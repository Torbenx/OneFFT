/*
 * Benchmark and accuracy harness.
 *
 *   bench [-i impl[,impl...]] [-a min_log2] [-b max_log2] [-t ms_per_sample] [-s samples] [-o offset]
 *
 * The input buffer starts at a page boundary and the output buffer `offset` bytes after one
 * (default 2048, a multiple of 64).
 *
 * Prints one line per implementation and size:
 *   <impl> <n> <ns per transform> <ns / (n log2 n)> <relative rms error>
 * The time is the minimum over all samples, which is the most reproducible statistic on machines
 * with frequency scaling and background load.
 */
#define _POSIX_C_SOURCE 200809L
#include "impl.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static const bench_impl *const impls[] = {
#ifdef BENCH_ONEFFT
    &impl_onefft,
#endif
#ifdef BENCH_REFERENCE
    &impl_reference,
#endif
#ifdef BENCH_FFTW
    &impl_fftw,
#endif
#ifdef BENCH_FFMPEG
    &impl_ffmpeg,
#endif
#ifdef BENCH_PFFFT
    &impl_pffft,
#endif
#ifdef BENCH_VDSP
    &impl_vdsp,
#endif
#ifdef BENCH_POCKETFFT
    &impl_pocketfft,
#endif
#ifdef BENCH_KISSFFT
    &impl_kissfft,
#endif
};
enum { IMPL_COUNT = sizeof(impls) / sizeof(impls[0]) };

static double now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1e9 + (double)ts.tv_nsec;
}

static void *alloc_pages(size_t bytes) {
    void *p = NULL;
    if (posix_memalign(&p, 4096, bytes)) abort();
    return p;
}

static void *alloc64(size_t bytes) {
    void *p = NULL;
    if (posix_memalign(&p, 64, bytes ? bytes : 64)) abort();
    return p;
}

/* Double precision reference: in-place iterative radix-2, forward (e^{-2 pi i jk/n}). */
static void fft_double(double *x, size_t n) {
    for (size_t i = 1, j = 0; i < n; i++) {
        size_t bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) {
            double tr = x[2 * i], ti = x[2 * i + 1];
            x[2 * i] = x[2 * j], x[2 * i + 1] = x[2 * j + 1];
            x[2 * j] = tr, x[2 * j + 1] = ti;
        }
    }
    for (size_t len = 2; len <= n; len <<= 1) {
        for (size_t k = 0; k < len / 2; k++) {
            double ang = -2.0 * 3.14159265358979323846 * (double)k / (double)len;
            double wr = cos(ang), wi = sin(ang);
            for (size_t i = k; i < n; i += len) {
                size_t j = i + len / 2;
                double tr = x[2 * j] * wr - x[2 * j + 1] * wi;
                double ti = x[2 * j] * wi + x[2 * j + 1] * wr;
                x[2 * j] = x[2 * i] - tr, x[2 * j + 1] = x[2 * i + 1] - ti;
                x[2 * i] += tr, x[2 * i + 1] += ti;
            }
        }
    }
}

static uint32_t rng_state = 0x12345678u;
static float rng_float(void) { /* xorshift, uniform in [-1, 1) */
    rng_state ^= rng_state << 13, rng_state ^= rng_state >> 17, rng_state ^= rng_state << 5;
    return (float)(int32_t)rng_state * (1.0f / 2147483648.0f);
}

int main(int argc, char **argv) {
    const char *filter = NULL;
    int min_log2 = 1, max_log2 = 16, samples = 20;
    double sample_ms = 5.0;
    size_t out_offset = 2048;
    for (int i = 1; i + 1 < argc; i += 2) {
        if (!strcmp(argv[i], "-i")) filter = argv[i + 1];
        else if (!strcmp(argv[i], "-a")) min_log2 = atoi(argv[i + 1]);
        else if (!strcmp(argv[i], "-b")) max_log2 = atoi(argv[i + 1]);
        else if (!strcmp(argv[i], "-t")) sample_ms = atof(argv[i + 1]);
        else if (!strcmp(argv[i], "-s")) samples = atoi(argv[i + 1]);
        else if (!strcmp(argv[i], "-o")) out_offset = (size_t)atoi(argv[i + 1]);
        else { fprintf(stderr, "unknown option %s\n", argv[i]); return 2; }
    }

    for (int lg = min_log2; lg <= max_log2; lg++) {
        size_t n = (size_t)1 << lg;
        /* Both buffers start at a fixed place of their 4 KiB page. Otherwise the result depends
         * on where the allocator happens to put them: loads from the input wait for earlier
         * stores to the output on many cores if the two have the same offset in their page. */
        float *in_base = alloc_pages(n * 2 * sizeof(float) + 4096);
        float *out_base = alloc_pages(n * 2 * sizeof(float) + 4096);
        float *in = in_base, *out = (float *)((char *)out_base + out_offset);
        double *ref = alloc64(n * 2 * sizeof(double));
        for (size_t i = 0; i < 2 * n; i++) ref[i] = in[i] = rng_float();
        fft_double(ref, n);

        for (int k = 0; k < IMPL_COUNT; k++) {
            const bench_impl *impl = impls[k];
            if (filter) {
                const char *hit = strstr(filter, impl->name);
                size_t len = strlen(impl->name);
                if (!hit || (hit != filter && hit[-1] != ',') || (hit[len] && hit[len] != ',')) continue;
            }
            void *plan = impl->create(n);
            if (!plan) continue;

            memset(out, 0, n * 2 * sizeof(float));
            impl->run(plan, in, out);
            double err = 0, norm = 0;
            for (size_t i = 0; i < 2 * n; i++) {
                double d = (double)out[i] - ref[i];
                err += d * d, norm += ref[i] * ref[i];
            }
            err = sqrt(err / norm);

            /* Calibrate the iteration count for the requested sample duration. */
            size_t iters = 1;
            for (;;) {
                double t0 = now_ns();
                for (size_t i = 0; i < iters; i++) impl->run(plan, in, out);
                double dt = now_ns() - t0;
                if (dt >= sample_ms * 1e6 * 0.5 || iters >= ((size_t)1 << 30)) break;
                iters *= 2;
            }
            double best = 1e300;
            for (int s = 0; s < samples; s++) {
                double t0 = now_ns();
                for (size_t i = 0; i < iters; i++) impl->run(plan, in, out);
                double dt = (now_ns() - t0) / (double)iters;
                if (dt < best) best = dt;
            }
            printf("%-10s %9zu %12.1f %7.3f %9.2e\n", impl->name, n, best, best / ((double)n * lg), err);
            fflush(stdout);
            impl->destroy(plan);
        }
        free(in_base), free(out_base), free(ref);
    }
    return 0;
}
