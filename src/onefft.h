/*
 * OneFFT - a simple and fast SIMD FFT for power-of-two sizes.
 *
 * Single precision, complex to complex, out-of-place.
 */
#ifndef ONEFFT_H
#define ONEFFT_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct onefft_plan onefft_plan;

enum { ONEFFT_FORWARD = 0, ONEFFT_INVERSE = 1 };

/* Creates a plan for transforms of n complex values, n a power of two. Allocates and computes
 * the twiddle tables: O(n) time and memory. Returns NULL if n is not a power of two or the
 * allocation fails. */
onefft_plan *onefft_plan_create(size_t n, int direction);

void onefft_plan_destroy(onefft_plan *plan);

/* Transforms n interleaved complex values (re, im, re, im, ...) from `in` to `out`. The buffers
 * must not overlap. Neither needs any particular alignment. The inverse is not normalized. */
void onefft_execute(const onefft_plan *plan, const float *in, float *out);

#ifdef __cplusplus
}
#endif

#endif /* ONEFFT_H */
