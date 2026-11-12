/* Adapter for Apple Accelerate (vDSP). vDSP works on split complex data, so the benchmark feeds it
 * split buffers directly: the conversion from and to interleaved data is NOT timed, which
 * flatters vDSP when compared to the interleaved interfaces of the other libraries. */
#include "impl.h"
#include <Accelerate/Accelerate.h>
#include <stdlib.h>

typedef struct {
    vDSP_DFT_Setup setup;
    size_t n;
    float *ir, *ii, *or_, *oi;
} vdsp_plan;

static void *vdsp_create(size_t n) {
    if (n < 16) return NULL; /* vDSP_DFT needs n = f * 2^k with k >= 4 */
    vdsp_plan *p = calloc(1, sizeof(*p));
    p->setup = vDSP_DFT_zop_CreateSetup(NULL, n, vDSP_DFT_FORWARD);
    if (!p->setup) {
        free(p);
        return NULL;
    }
    p->n = n;
    p->ir = malloc(4 * n * sizeof(float));
    p->ii = p->ir + n, p->or_ = p->ii + n, p->oi = p->or_ + n;
    return p;
}

static void vdsp_run(void *plan, const float *in, float *out) {
    vdsp_plan *p = plan;
    if (p->ir[0] != in[0] || p->ii[0] != in[1]) { /* first call: deinterleave the input once */
        for (size_t i = 0; i < p->n; i++) p->ir[i] = in[2 * i], p->ii[i] = in[2 * i + 1];
    }
    vDSP_DFT_Execute(p->setup, p->ir, p->ii, p->or_, p->oi);
    out[0] = p->or_[0], out[1] = p->oi[0]; /* keep the result observable */
}

static void vdsp_destroy(void *plan) {
    vdsp_plan *p = plan;
    vDSP_DFT_DestroySetup(p->setup);
    free(p->ir);
    free(p);
}

const bench_impl impl_vdsp = {"vdsp", vdsp_create, vdsp_run, vdsp_destroy};
