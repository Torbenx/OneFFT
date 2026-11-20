/* Adapter for TwoFFT, a sibling library built from its source tree (TWOFFT_DIR in
 * scripts/build_bench.sh). TwoFFT is a set of headers, one per backend; the caller fills a
 * weight table for one size and passes it to every transform of that size. Only the AArch64
 * backend is finished, so scripts/build_bench.sh compiles this adapter for AArch64 only. */
#define _POSIX_C_SOURCE 200809L
#include "impl.h"
#include <stdlib.h>

#if !defined(__aarch64__)
#error "TwoFFT is benchmarked on AArch64 only"
#endif

/* The header defines every function static; the benchmark uses only some of them. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#include <tw_arm64.h>
#pragma GCC diagnostic pop

typedef struct {
    unsigned log2n;
    void *weights;
} twofft_plan;

static void *two_create(size_t n) {
    unsigned lg = 0;
    while (((size_t)1 << lg) < n) lg++;
    const size_t bytes = tw_weights_floats(lg) * sizeof(float);
    twofft_plan *p = malloc(sizeof(*p));
    if (!p) return NULL;
    p->log2n = lg;
    p->weights = NULL;
    if (posix_memalign(&p->weights, 64, bytes ? bytes : 64)) {
        free(p);
        return NULL;
    }
    tw_weights_fill(lg, p->weights);
    return p;
}

static void two_run(void *plan, const float *in, float *out) {
    const twofft_plan *p = plan;
    tw_a64_fft(p->log2n, p->weights, in, out, 0);
}

static void two_destroy(void *plan) {
    twofft_plan *p = plan;
    free(p->weights);
    free(p);
}

const bench_impl impl_twofft = {"twofft", two_create, two_run, two_destroy};
