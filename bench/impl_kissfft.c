/* Adapter for KissFFT (float build), the usual scalar baseline. */
#include "impl.h"
#include <kiss_fft.h>

static void *kiss_create(size_t n) { return kiss_fft_alloc((int)n, 0, NULL, NULL); }
static void kiss_run(void *plan, const float *in, float *out) {
    kiss_fft(plan, (const kiss_fft_cpx *)in, (kiss_fft_cpx *)out);
}
static void kiss_destroy(void *plan) { kiss_fft_free(plan); }

const bench_impl impl_kissfft = {"kissfft", kiss_create, kiss_run, kiss_destroy};
