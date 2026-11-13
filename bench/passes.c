/*
 * Developer tool: times every pass of a OneFFT plan separately and reports cycles per kernel
 * invocation ("butterfly"). Includes the library source directly to reach its internals.
 *
 *   passes <log2n>[:radices] ...
 *
 * `radices` (AArch64 only) overrides the pass structure: the radix bits of the middle passes and
 * the last pass as digits, e.g. "10:2223" for a radix 4, 4, 4 middle and a radix 8 last pass.
 *
 * A leading argument "cache=<log2 bytes>" overrides ONEFFT_CACHE_BYTES_LOG2 of onefft.c, and
 * "shift=<bytes>" moves the input buffer relative to the output buffer (buffers with the same
 * offset in their 4 KiB page are the worst case for some cores). For the AVX-512 plans:
 * "tuning=<f16_to>,<f16_from>,<ordered_from>,<ordered_to>" sets the size dependent choices, see
 * onefft_avx512_tuning in onefft.c.
 * The total time of complete transforms is printed as well.
 */
#define _POSIX_C_SOURCE 200809L
#define ONEFFT_DEVELOPER
#include "../src/onefft.c"

#include <stdio.h>
#include <time.h>

static double now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1e9 + (double)ts.tv_nsec;
}

/* One taken branch per cycle on every core we care about: measures the clock period. */
static double cycle_ns(void) {
    double best = 1e300;
    for (int s = 0; s < 30; s++) {
        size_t n = 20000000;
        double t0 = now_ns();
#if defined(__aarch64__)
        __asm__ volatile("1: subs %[n], %[n], #1\n b.ne 1b\n" : [n] "+r"(n) : : "cc");
#else
        __asm__ volatile("1: sub $1, %[n]\n jnz 1b\n" : [n] "+r"(n) : : "cc");
#endif
        double dt = (now_ns() - t0) / 20000000.0;
        if (dt < best) best = dt;
    }
    return best;
}

typedef void step_fn(const onefft_plan *plan, const float *in, float *out, int index);

static void step_first(const onefft_plan *p, const float *in, float *out, int index) {
    (void)index;
    p->first(in, out, p->first_table, p->first_count, p->first_in_stride, p->first_out_stride);
}
static void step_middle(const onefft_plan *p, const float *in, float *out, int index) {
    (void)in;
    const onefft_middle *m = &p->middle[index];
    m->run(out, m->tw, m->args, m->blocks, m->regions);
}
static void step_last(const onefft_plan *p, const float *in, float *out, int index) {
    (void)in;
    const onefft_last *l = &p->last[index];
    l->run(out, l->tw, l->stride, l->count, l->regions);
}
static void step_all(const onefft_plan *p, const float *in, float *out, int index) {
    (void)index;
    onefft_plan_execute(p, in, out);
}
static void step_copy(const onefft_plan *p, const float *in, float *out, int index) {
    (void)p, (void)in, (void)out, (void)index;
}

/* Minimum time of memcpy(out, saved) + step. The copy keeps the data finite and identical. */
static double time_step(step_fn *step, const onefft_plan *p, const float *in, float *out, const float *saved,
                        int index) {
    size_t bytes = p->n * 2 * sizeof(float);
    size_t iters = 1 + (size_t)(4e6 / (double)(p->n * 8 + 1000));
    double best = 1e300;
    for (int s = 0; s < 30; s++) {
        double t0 = now_ns();
        for (size_t i = 0; i < iters; i++) {
            memcpy(out, saved, bytes);
            step(p, in, out, index);
        }
        double dt = (now_ns() - t0) / (double)iters;
        if (dt < best) best = dt;
    }
    return best;
}

int main(int argc, char **argv) {
    size_t shift = 0; /* bytes between the page offsets of the input and the output buffer */
    for (; argc > 1 && strchr(argv[1], '='); argv++, argc--) {
        if (!strncmp(argv[1], "cache=", 6)) onefft_tune_cache_log2 = (unsigned)atoi(argv[1] + 6);
        if (!strncmp(argv[1], "shift=", 6)) shift = (size_t)atoi(argv[1] + 6);
#ifdef ONEFFT_AVX512
        if (!strncmp(argv[1], "tuning=", 7)) {
            static onefft_avx512_tuning tuning;
            sscanf(argv[1] + 7, "%hhu,%hhu,%hhu,%hhu", &tuning.f16_to, &tuning.f16_from, &tuning.ordered_from,
                   &tuning.ordered_to);
            onefft_developer_tuning = &tuning;
        }
#endif
    }
    cycle_ns(); /* warm up, lets the frequency governor settle */
    double cycle = cycle_ns();
    printf("clock: %.3f ns per cycle (%.0f MHz)\n", cycle, 1000.0 / cycle);
    for (int a = 1; a < argc; a++) {
        unsigned log2n = (unsigned)atoi(argv[a]);
        size_t n = (size_t)1 << log2n;
#ifdef ONEFFT_NEON
        unsigned radices[32] = {0};
        const char *colon = strchr(argv[a], ':');
        onefft_developer_radices = NULL;
        if (colon) {
            for (int i = 0; colon[1 + i] && i < 30; i++) radices[i] = (unsigned)(colon[1 + i] - '0');
            onefft_developer_radices = radices;
        }
#endif
        onefft_plan *p = onefft_plan_create(n, ONEFFT_FORWARD);
        if (!p || !p->first) {
            printf("n=%zu: no multi pass plan\n", n);
            continue;
        }
        float *in, *out, *saved;
        if (posix_memalign((void **)&in, 4096, n * 8 + 4096) || posix_memalign((void **)&out, 4096, n * 8) ||
            posix_memalign((void **)&saved, 64, n * 8))
            return 1;
        float *in_base = in;
        in = (float *)((char *)in + shift);
        for (size_t i = 0; i < 2 * n; i++) in[i] = (float)((i * 2654435761u) & 0xffff) / 65536.0f - 0.5f;
        step_first(p, in, out, 0);
        memcpy(saved, out, n * 8);

        double copy = time_step(step_copy, p, in, out, saved, 0);
        double total = 0;
        printf("n=%zu (copy %.0f ns)\n", n, copy);
        double t = time_step(step_first, p, in, out, saved, 0) - copy;
        total += t;
        printf("  first    : %9.0f ns  %6.1f cycles/butterfly\n", t, t / cycle / (double)p->first_count);
        for (int i = 0; i < p->middle_count; i++) {
            const onefft_middle *m = &p->middle[i];
            double count = (double)m->regions * (double)m->args[ONEFFT_INNER] * (double)m->blocks;
            t = time_step(step_middle, p, in, out, saved, i) - copy;
            total += t;
            printf("  middle %d : %9.0f ns  %6.1f cycles/butterfly  (strides %zu %zu, inner %zu)\n", i, t,
                   t / cycle / count, m->args[ONEFFT_S_HIGH], m->args[ONEFFT_S_LOW], m->args[ONEFFT_INNER]);
        }
        for (int i = 0; i < p->last_count; i++) {
            const onefft_last *l = &p->last[i];
            t = time_step(step_last, p, in, out, saved, i) - copy;
            total += t;
            printf("  last %d   : %9.0f ns  %6.1f cycles/butterfly\n", i, t,
                   t / cycle / ((double)l->regions * (double)l->count));
        }
        printf("  sum %.0f ns, complete transform %.0f ns\n", total, time_step(step_all, p, in, out, saved, 0) - copy);
        free(in_base), free(out), free(saved);
        onefft_plan_destroy(p);
    }
    return 0;
}
