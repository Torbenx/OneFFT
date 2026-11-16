/* Adapter for KFR through its C API (kfr_dft_*_f32). KFR picks the instruction set at run time.
 * The plan needs a work buffer of kfr_dft_get_temp_size_f32 bytes. */
#define _POSIX_C_SOURCE 200809L
#include "impl.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <kfr/capi.h>

typedef struct {
    KFR_DFT_PLAN_F32 *plan;
    uint8_t *temp;
} kfr_plan;

static void *kfr_create(size_t n) {
    kfr_plan *p = malloc(sizeof(*p));
    p->plan = kfr_dft_create_plan_f32(n);
    if (!p->plan) {
        free(p);
        return NULL;
    }
    size_t bytes = kfr_dft_get_temp_size_f32(p->plan);
    if (posix_memalign((void **)&p->temp, 64, bytes ? bytes : 64)) abort();
    return p;
}

static void kfr_run(void *plan, const float *in, float *out) {
    kfr_plan *p = plan;
    kfr_dft_execute_f32(p->plan, (kfr_c32 *)out, (const kfr_c32 *)in, p->temp);
}

static void kfr_destroy(void *plan) {
    kfr_plan *p = plan;
    kfr_dft_delete_plan_f32(p->plan);
    free(p->temp);
    free(p);
}

const bench_impl impl_kfr = {"kfr", kfr_create, kfr_run, kfr_destroy};
