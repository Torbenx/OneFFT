/* Adapter for TwoFFT, a sibling library built from its source tree (TWOFFT_DIR in
 * scripts/build_bench.sh). A context caches the plans of every size; the first transform builds
 * the plan, so create() runs one. */
#include "impl.h"
#include <stdlib.h>
#include <twofft.h>

typedef struct {
    twofft_context *ctx;
    unsigned log2n;
} twofft_plan;

static void *two_create(size_t n) {
    unsigned lg = 0;
    while (((size_t)1 << lg) < n) lg++;
    twofft_plan *p = malloc(sizeof(*p));
    p->ctx = twofft_context_create();
    p->log2n = lg;
    float *buf = calloc(4 * n, sizeof(float));
    int err = !p->ctx || !buf || twofft_forward(p->ctx, buf, buf + 2 * n, lg);
    free(buf);
    if (err) {
        if (p->ctx) twofft_context_destroy(p->ctx);
        free(p);
        return NULL;
    }
    return p;
}

static void two_run(void *plan, const float *in, float *out) {
    twofft_plan *p = plan;
    twofft_forward(p->ctx, in, out, p->log2n);
}

static void two_destroy(void *plan) {
    twofft_plan *p = plan;
    twofft_context_destroy(p->ctx);
    free(p);
}

const bench_impl impl_twofft = {"twofft", two_create, two_run, two_destroy};
