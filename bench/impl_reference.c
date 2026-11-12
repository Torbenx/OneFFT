/* Adapter for the 2021 proof of concept in reference/ (aarch64 only, forward, N = 16..1024). */
#include "impl.h"
#include "../reference/fft.h"
#include <string.h>

static void *ref_create(size_t n) {
    FFT *fft = calloc(1, sizeof(FFT));
    fft->n = (int)n;
    fft->direction = 1;
    fft->forward = 1;
    switch (n) {
    case 16: fft->transformFft = transformFFT4_16_ASIMD; break;
    case 32: fft->transformFft = transformFFT_32_ASIMD; break;
    case 64: createWeightsFFT_64_ASIMD(fft); fft->transformFft = transformFFT_64_ASIMD; break;
    case 128: createWeightsFFT_128_ASIMD(fft); fft->transformFft = transformFFT_128_ASIMD; break;
    case 256: createWeightsFFT16_256_ASIMD(fft); fft->transformFft = transformFFT16_256_ASIMD; break;
    case 512: createWeightsFFT_512_ASIMD(fft); fft->transformFft = transformFFT_512_ASIMD; break;
    case 1024: createWeightsFFT_1024_ASIMD(fft); fft->transformFft = transformFFT_1024_ASIMD; break;
    default: free(fft); return NULL;
    }
    return fft;
}

static void ref_run(void *plan, const float *in, float *out) {
    FFT *fft = plan;
    /* The reference always writes to its internal buffer; redirect it to the caller's output. */
    fft->in = (complex *)in;
    fft->b = (complex *)out;
    fft->transformFft(fft);
}

static void ref_destroy(void *plan) {
    FFT *fft = plan;
    if (fft->freeWeights) fft->freeWeights(fft);
    free(fft);
}

const bench_impl impl_reference = {"reference", ref_create, ref_run, ref_destroy};
