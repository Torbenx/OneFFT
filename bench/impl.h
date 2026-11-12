/* Common adapter interface used by the benchmark and accuracy harness. */
#ifndef ONEFFT_BENCH_IMPL_H
#define ONEFFT_BENCH_IMPL_H

#include <stddef.h>

typedef struct {
    const char *name;
    /* Returns NULL if the size is not supported. Data is interleaved re/im float, n complex. */
    void *(*create)(size_t n);
    void (*run)(void *plan, const float *in, float *out);
    void (*destroy)(void *plan);
} bench_impl;

/* Each adapter translation unit defines one of these; bench.c lists the ones compiled in. */
extern const bench_impl impl_reference;
extern const bench_impl impl_onefft;
extern const bench_impl impl_fftw;
extern const bench_impl impl_pffft;
extern const bench_impl impl_kissfft;
extern const bench_impl impl_ffmpeg;
extern const bench_impl impl_vdsp;
extern const bench_impl impl_pocketfft;

#endif
