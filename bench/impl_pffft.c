/* Adapter for PFFFT (complex, ordered output). PFFFT needs an aligned work buffer for n >= 16384. */
#include "impl.h"
#include <pffft.h>
#include <stdlib.h>

typedef struct {
    PFFFT_Setup *setup;
    float *work;
} pffft_plan;

static void *pffft_create(size_t n) {
    if (n < 16) return NULL;
    pffft_plan *p = malloc(sizeof(*p));
    p->setup = pffft_new_setup((int)n, PFFFT_COMPLEX);
    if (!p->setup) {
        free(p);
        return NULL;
    }
    p->work = pffft_aligned_malloc(n * 2 * sizeof(float));
    return p;
}

static void pffft_run(void *plan, const float *in, float *out) {
    pffft_plan *p = plan;
    pffft_transform_ordered(p->setup, in, out, p->work, PFFFT_FORWARD);
}

static void pffft_destroy(void *plan) {
    pffft_plan *p = plan;
    pffft_destroy_setup(p->setup);
    pffft_aligned_free(p->work);
    free(p);
}

const bench_impl impl_pffft = {"pffft", pffft_create, pffft_run, pffft_destroy};
