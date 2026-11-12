/*
 * OneFFT correctness tests. Exits with a non-zero status if anything fails.
 *
 *   test_onefft [max_log2]        default 20
 */
#define _POSIX_C_SOURCE 200809L
#include "onefft.h"

#include <math.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;

#define CHECK(condition, ...)                                                                              \
    do {                                                                                                   \
        if (!(condition)) {                                                                                \
            failures++;                                                                                    \
            printf("FAILED %s:%d: ", __FILE__, __LINE__);                                                  \
            printf(__VA_ARGS__);                                                                           \
            printf("\n");                                                                                  \
        }                                                                                                  \
    } while (0)

static const double two_pi = 6.283185307179586476925286766559;

/* Double precision reference: in-place iterative radix 2, sign -1 forward, +1 inverse. */
static void reference_fft(double *x, size_t n, int inverse) {
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
            double angle = (inverse ? two_pi : -two_pi) * (double)k / (double)len;
            double wr = cos(angle), wi = sin(angle);
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

static uint32_t rng_state = 0x9e3779b9u;
static float random_float(void) { /* xorshift, uniform in [-1, 1) */
    rng_state ^= rng_state << 13, rng_state ^= rng_state >> 17, rng_state ^= rng_state << 5;
    return (float)(int32_t)rng_state * (1.0f / 2147483648.0f);
}

/* Relative root mean square difference between a float and a double vector. */
static double relative_error(const float *got, const double *want, size_t count) {
    double error = 0, norm = 0;
    for (size_t i = 0; i < count; i++) {
        double d = (double)got[i] - want[i];
        error += d * d, norm += want[i] * want[i];
    }
    return norm > 0 ? sqrt(error / norm) : sqrt(error);
}

enum { GUARD = 64 }; /* floats before and after the output that must stay untouched */

typedef struct {
    float *base;  /* allocation */
    float *data;  /* base + GUARD + misalignment */
    size_t count; /* floats */
} guarded;

static guarded guarded_alloc(size_t count, size_t misalign_floats) {
    guarded g;
    if (posix_memalign((void **)&g.base, 64, (count + 2 * GUARD + 16) * sizeof(float))) abort();
    g.data = g.base + GUARD + misalign_floats;
    g.count = count;
    for (size_t i = 0; i < GUARD + misalign_floats; i++) g.base[i] = 12345.0f;
    for (size_t i = 0; i < GUARD; i++) g.data[count + i] = 12345.0f;
    return g;
}

static int guarded_intact(const guarded *g) {
    for (float *p = g->base; p < g->data; p++)
        if (*p != 12345.0f) return 0;
    for (size_t i = 0; i < GUARD; i++)
        if (g->data[g->count + i] != 12345.0f) return 0;
    return 1;
}

/* Error bound for a single precision transform of 2^log2n points. Rounding errors accumulate
 * over log2n butterfly stages; measured errors are about 3e-8 * sqrt(log2n). */
static double tolerance(unsigned log2n) { return 4e-7 + 1e-7 * (double)log2n; }

static void test_size(unsigned log2n) {
    size_t n = (size_t)1 << log2n;
    onefft_plan *forward = onefft_plan_create(n, ONEFFT_FORWARD);
    onefft_plan *inverse = onefft_plan_create(n, ONEFFT_INVERSE);
    CHECK(forward && inverse, "n=%zu: plan creation failed", n);
    if (!forward || !inverse) return;

    double *want = malloc(2 * n * sizeof(double));
    /* The input is misaligned by one float and the output by three, to cover unaligned use. */
    guarded in = guarded_alloc(2 * n, 1), out = guarded_alloc(2 * n, 3), back = guarded_alloc(2 * n, 0);

    /* random input, forward and inverse against the reference */
    for (size_t i = 0; i < 2 * n; i++) in.data[i] = random_float();
    for (int direction = 0; direction < 2; direction++) {
        for (size_t i = 0; i < 2 * n; i++) want[i] = in.data[i];
        reference_fft(want, n, direction);
        memset(out.data, 0, 2 * n * sizeof(float));
        onefft_plan_execute(direction ? inverse : forward, in.data, out.data);
        double error = relative_error(out.data, want, 2 * n);
        CHECK(error < tolerance(log2n), "n=%zu %s: relative error %.3g", n, direction ? "inverse" : "forward", error);
        CHECK(guarded_intact(&out), "n=%zu %s: wrote outside of the output", n, direction ? "inverse" : "forward");
        CHECK(guarded_intact(&in), "n=%zu: input modified", n);
    }

    /* round trip: inverse(forward(x)) = n x */
    onefft_plan_execute(forward, in.data, out.data);
    onefft_plan_execute(inverse, out.data, back.data);
    for (size_t i = 0; i < 2 * n; i++) want[i] = (double)in.data[i] * (double)n;
    double error = relative_error(back.data, want, 2 * n);
    CHECK(error < 2 * tolerance(log2n), "n=%zu round trip: relative error %.3g", n, error);
    CHECK(guarded_intact(&back), "n=%zu: wrote outside of the output", n);

    /* a unit impulse at position j transforms to exp(-2 pi i j k / n) */
    size_t j = n > 1 ? 1 + (n / 3) % (n - 1) : 0;
    memset(in.data, 0, 2 * n * sizeof(float));
    in.data[2 * j] = 1.0f;
    onefft_plan_execute(forward, in.data, out.data);
    double worst = 0;
    for (size_t k = 0; k < n; k++) {
        /* reduce j*k modulo n in integers to keep the angle accurate */
        double angle = -two_pi * (double)((uint64_t)j * k % n) / (double)n;
        double d = hypot(out.data[2 * k] - cos(angle), out.data[2 * k + 1] - sin(angle));
        if (d > worst) worst = d;
    }
    CHECK(worst < 4e-6, "n=%zu impulse: error %.3g", n, worst);

    /* a constant transforms to an impulse at 0 */
    for (size_t i = 0; i < n; i++) in.data[2 * i] = 0.5f, in.data[2 * i + 1] = -0.25f;
    onefft_plan_execute(forward, in.data, out.data);
    worst = hypot(out.data[0] - 0.5 * (double)n, out.data[1] + 0.25 * (double)n) / (double)n;
    for (size_t k = 1; k < n; k++) {
        double d = hypot(out.data[2 * k], out.data[2 * k + 1]) / (double)n;
        if (d > worst) worst = d;
    }
    CHECK(worst < 1e-6, "n=%zu constant: error %.3g", n, worst);

    free(want), free(in.base), free(out.base), free(back.base);
    onefft_plan_destroy(forward), onefft_plan_destroy(inverse);
}

static void test_invalid_sizes(void) {
    static const size_t invalid[] = {0, 3, 5, 6, 7, 12, 1000, 1023, 1025, (size_t)-1, (size_t)1 << 31};
    for (size_t i = 0; i < sizeof(invalid) / sizeof(invalid[0]); i++) {
        onefft_plan *plan = onefft_plan_create(invalid[i], ONEFFT_FORWARD);
        CHECK(plan == NULL, "n=%zu: expected no plan", invalid[i]);
        onefft_plan_destroy(plan);
    }
    onefft_plan_destroy(NULL); /* must be accepted */
    CHECK(onefft_plan_create(8, 2) == NULL, "invalid direction: expected no plan");
    float in[6] = {0}, out[6] = {7, 7, 7, 7, 7, 7};
    CHECK(onefft_forward(in, out, 3) == -1 && onefft_inverse(in, out, 0) == -1, "expected failure");
    CHECK(out[0] == 7 && out[5] == 7, "output touched for an invalid size");
    CHECK(onefft_cache_prepare(12, ONEFFT_FORWARD) == -1, "expected failure");
    CHECK(onefft_cache_prepare(16, 5) == -1, "expected failure");
}

/* The functions without plans give exactly the result of a plan. */
static void test_cache(void) {
    for (size_t n = 1; n <= 4096; n *= 2) {
        float *in = malloc(2 * n * sizeof(float)), *want = malloc(2 * n * sizeof(float));
        float *got = malloc(2 * n * sizeof(float));
        for (size_t i = 0; i < 2 * n; i++) in[i] = random_float();
        for (int direction = 0; direction < 2; direction++) {
            onefft_plan *plan = onefft_plan_create(n, direction);
            CHECK(onefft_plan_size(plan) == n, "n=%zu: plan size", n);
            onefft_plan_execute(plan, in, want);
            if (n == 64) CHECK(onefft_cache_prepare(n, direction) == 0, "prepare failed");
            int status = direction ? onefft_inverse(in, got, n) : onefft_forward(in, got, n);
            CHECK(status == 0, "n=%zu: transform failed", n);
            CHECK(!memcmp(got, want, 2 * n * sizeof(float)), "n=%zu: cached plan differs", n);
            onefft_plan_destroy(plan);
        }
        free(in), free(want), free(got);
    }
    onefft_cache_clear();
    onefft_cache_clear(); /* clearing an empty cache is fine */
}

/* One plan used from several threads at once must give every thread the same result. */
typedef struct {
    const onefft_plan *plan;
    const float *in;
    const float *expected;
    size_t n;
    int mismatches;
} thread_job;

static void *thread_main(void *arg) {
    thread_job *job = arg;
    float *out = malloc(2 * job->n * sizeof(float));
    for (int round = 0; round < 200; round++) {
        /* Odd rounds go through the cache, whose first use races between the threads. */
        if (round & 1) {
            if (onefft_forward(job->in, out, job->n)) job->mismatches++;
        } else {
            onefft_plan_execute(job->plan, job->in, out);
        }
        if (memcmp(out, job->expected, 2 * job->n * sizeof(float))) job->mismatches++;
    }
    free(out);
    return NULL;
}

static void test_threads(void) {
    enum { THREADS = 4 };
    size_t n = 4096;
    onefft_plan *plan = onefft_plan_create(n, ONEFFT_FORWARD);
    float *in = malloc(2 * n * sizeof(float)), *expected = malloc(2 * n * sizeof(float));
    for (size_t i = 0; i < 2 * n; i++) in[i] = random_float();
    onefft_plan_execute(plan, in, expected);
    pthread_t threads[THREADS];
    thread_job jobs[THREADS];
    for (int t = 0; t < THREADS; t++) {
        jobs[t] = (thread_job){plan, in, expected, n, 0};
        pthread_create(&threads[t], NULL, thread_main, &jobs[t]);
    }
    for (int t = 0; t < THREADS; t++) {
        pthread_join(threads[t], NULL);
        CHECK(jobs[t].mismatches == 0, "thread %d: %d results differ", t, jobs[t].mismatches);
    }
    free(in), free(expected);
    onefft_plan_destroy(plan);
    onefft_cache_clear();
}

int main(int argc, char **argv) {
    unsigned max_log2 = argc > 1 ? (unsigned)atoi(argv[1]) : 20;
    for (unsigned log2n = 0; log2n <= max_log2; log2n++) test_size(log2n);
    test_invalid_sizes();
    test_cache();
    test_threads();
    if (failures) {
        printf("%d checks FAILED\n", failures);
        return 1;
    }
    printf("all tests passed (sizes 1 to 2^%u, %s kernels)\n", max_log2, onefft_implementation());
    return 0;
}
