/* Adapter for FFTW 3 (single precision, out-of-place, FFTW_MEASURE or FFTW_PATIENT plan). */
#include "impl.h"
#include <fftw3.h>

static void *fftw_create(size_t n) {
    fftwf_complex *in = fftwf_malloc(n * sizeof(fftwf_complex));
    fftwf_complex *out = fftwf_malloc(n * sizeof(fftwf_complex));
    unsigned flags = n <= 4096 ? FFTW_PATIENT : FFTW_MEASURE;
    fftwf_plan plan = fftwf_plan_dft_1d((int)n, in, out, FFTW_FORWARD, flags);
    fftwf_free(in), fftwf_free(out);
    return plan;
}

static void fftw_run(void *plan, const float *in, float *out) {
    fftwf_execute_dft(plan, (fftwf_complex *)in, (fftwf_complex *)out);
}

static void fftw_destroy(void *plan) { fftwf_destroy_plan(plan); }

const bench_impl impl_fftw = {"fftw", fftw_create, fftw_run, fftw_destroy};
