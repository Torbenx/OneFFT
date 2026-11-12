/*
 * OneFFT - a simple and fast SIMD FFT for power-of-two sizes.
 *
 * Complex to complex, single precision, out of place. Data is interleaved: n complex values are
 * 2n floats (re, im, re, im, ...), which is also the layout of `float complex` in C and of
 * `std::complex<float>` in C++.
 *
 *     forward:  out[k] = sum over j of in[j] * exp(-2 pi i j k / n)
 *     inverse:  out[k] = sum over j of in[j] * exp(+2 pi i j k / n)
 *
 * Neither direction is normalized: a forward transform followed by an inverse one multiplies
 * the data by n.
 *
 * There are two ways to use the library, both safe to use from any number of threads at once:
 *
 *  - onefft_forward() / onefft_inverse(): just transform. The tables for a size are created
 *    when the size is first used and are kept until onefft_cache_clear().
 *  - onefft_plan_*: create the tables for a size explicitly and decide yourself when they are
 *    created and freed.
 *
 * SIMD kernels exist for AArch64 (Advanced SIMD) and x86-64 (AVX2 with FMA, chosen at run time
 * if the processor has them, otherwise SSE2). Other targets use portable C.
 */
#ifndef ONEFFT_H
#define ONEFFT_H

#include <stddef.h>

#define ONEFFT_VERSION_MAJOR 0
#define ONEFFT_VERSION_MINOR 1
#define ONEFFT_VERSION_PATCH 0

#ifdef __cplusplus
extern "C" {
#endif

enum { ONEFFT_FORWARD = 0, ONEFFT_INVERSE = 1 };

/* ---- transforms without plan management -----------------------------------------------------
 *
 * Transform n complex values from `in` to `out`. n has to be a power of two from 1 to 2^30, and
 * the buffers must not overlap. No alignment is required.
 *
 * The first call for a size and direction allocates and computes the tables for it, which
 * takes O(n) time and memory, several times longer than the transform itself. All later calls
 * with that size find the tables with a single atomic read and neither allocate nor lock.
 * Call onefft_cache_prepare() first where that one-time cost must not occur later, for example
 * before entering a real-time thread.
 *
 * Returns 0 on success. Returns -1, leaving `out` untouched, if n is not such a size or if the
 * tables could not be allocated.
 */
int onefft_forward(const float *in, float *out, size_t n);
int onefft_inverse(const float *in, float *out, size_t n);

/* Allocates and computes the tables for size n and the given direction now, so that later
 * onefft_forward() / onefft_inverse() calls with that size do not have to. Does nothing if
 * they exist already. Returns 0 on success, -1 if n is not a power of two or on allocation
 * failure. */
int onefft_cache_prepare(size_t n, int direction);

/* Frees all tables that the functions above have created. Must not be called while another
 * thread is inside one of them. The cache refills on demand afterwards. */
void onefft_cache_clear(void);

/* ---- plans ----------------------------------------------------------------------------------
 *
 * A plan holds the tables for one size and direction.
 */
typedef struct onefft_plan onefft_plan;

/* Allocates a plan and computes its tables: O(n) time and memory (about 1.5 times the size of
 * one input buffer). `direction` is ONEFFT_FORWARD or ONEFFT_INVERSE. Returns NULL if n is not
 * a power of two from 1 to 2^30 or if the allocation fails. */
onefft_plan *onefft_plan_create(size_t n, int direction);

/* Frees a plan. Passing NULL is allowed. */
void onefft_plan_destroy(onefft_plan *plan);

/* Transforms from `in` to `out`, which must not overlap and need no alignment. Does not
 * allocate, lock or modify the plan: one plan can be used by any number of threads at the same
 * time. */
void onefft_plan_execute(const onefft_plan *plan, const float *in, float *out);

/* The number of complex values a plan transforms. */
size_t onefft_plan_size(const onefft_plan *plan);

/* ---- information ----------------------------------------------------------------------------
 */

/* The kernels in use on this machine: "neon", "neon (fused)", "avx2", "sse2" or "portable". */
const char *onefft_implementation(void);

#ifdef __cplusplus
} /* extern "C" */

#include <complex>

namespace onefft {

/* Transforms without plan management; see onefft_forward() above for when tables are created.
 * Return false if n is not a power of two or the tables could not be allocated. */
inline bool forward(const std::complex<float> *in, std::complex<float> *out, std::size_t n) {
    return onefft_forward(reinterpret_cast<const float *>(in), reinterpret_cast<float *>(out), n) == 0;
}
inline bool inverse(const std::complex<float> *in, std::complex<float> *out, std::size_t n) {
    return onefft_inverse(reinterpret_cast<const float *>(in), reinterpret_cast<float *>(out), n) == 0;
}

/* Owns a plan. The constructor allocates and computes the tables: O(n) time and memory.
 * operator() only transforms and can be called from several threads at once. */
class Plan {
public:
    Plan(std::size_t n, int direction) : plan_(onefft_plan_create(n, direction)) {}
    ~Plan() { onefft_plan_destroy(plan_); }
    Plan(Plan &&other) noexcept : plan_(other.plan_) { other.plan_ = nullptr; }
    Plan &operator=(Plan &&other) noexcept {
        if (this != &other) {
            onefft_plan_destroy(plan_);
            plan_ = other.plan_;
            other.plan_ = nullptr;
        }
        return *this;
    }
    Plan(const Plan &) = delete;
    Plan &operator=(const Plan &) = delete;

    /* False if n was not a power of two or the allocation failed; the plan must not be used. */
    explicit operator bool() const { return plan_ != nullptr; }
    std::size_t size() const { return onefft_plan_size(plan_); }

    void operator()(const std::complex<float> *in, std::complex<float> *out) const {
        onefft_plan_execute(plan_, reinterpret_cast<const float *>(in), reinterpret_cast<float *>(out));
    }
    void operator()(const float *in, float *out) const { onefft_plan_execute(plan_, in, out); }

private:
    onefft_plan *plan_;
};

} /* namespace onefft */
#endif /* __cplusplus */

#endif /* ONEFFT_H */
