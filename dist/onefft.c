/*
 * OneFFT, single file distribution: compile this file and include onefft.h.
 *
 * GENERATED FILE, DO NOT EDIT. It is src/onefft.c with the generated kernel sources included,
 * produced by gen/generate.py. The repository has the sources, tests and documentation.
 */
/*
 * OneFFT - runtime: plans, twiddle tables and the transform drivers.
 *
 * The SIMD kernels are generated (onefft_*.inc, see gen/). This file decides which kernels a
 * transform size uses, builds their tables and calls them. docs/design.md explains the
 * algorithm; the comments here assume it has been read.
 */
#include "onefft.h"

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <malloc.h>
#endif

#define ONEFFT_ALIGNED(bytes) __attribute__((aligned(bytes)))

#if defined(__aarch64__) && !defined(ONEFFT_NO_SIMD)
#define ONEFFT_NEON 1
#ifdef __APPLE__
#include <sys/sysctl.h>
#endif
/*
 * OneFFT AArch64 Advanced SIMD kernels.
 *
 * GENERATED FILE, DO NOT EDIT. Change gen/onefft_gen and run gen/generate.py instead.
 *
 * Every kernel is a normal C function whose body is one inline assembly block. The SIMD registers
 * are assigned by the generator, the general purpose registers by the C compiler.
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverlength-strings"

/* First pass, radix 8. For each of the `count` entries (a, b) of `table`: reads 8 interleaved
 * vectors from in + 8*a + u*in_stride, transforms across them and writes output q as a split
 * vector to out + 8*b + (q%4)*out_stride + 32*(q/4). */
static void onefft_neon_f8(const float *in, float *out, const uint32_t *table, size_t count, size_t in_stride, size_t out_stride) {
    size_t bits;
    const float *src0;
    float *dst0;
    size_t a0;
    size_t b0;
    const float *src1;
    float *dst1;
    size_t a1;
    size_t b1;
    __asm__ volatile(
        "    mov %w[bits], #1267\n"
        "    lsr %[count], %[count], #1\n"
        "    movk %w[bits], #16181, lsl #16\n"
        "    dup v0.4s, %w[bits]\n"
        "1:\n"
        "    ldp %w[a0], %w[b0], [%[table]], #8\n"
        "    add %[src0], %[in], %[a0], lsl #3\n"
        "    add %[dst0], %[out], %[b0], lsl #3\n"
        "    ldp q1, q2, [%[src0]]\n"
        "    add %[src0], %[src0], %[in_stride]\n"
        "    ldp q3, q4, [%[src0]]\n"
        "    add %[src0], %[src0], %[in_stride]\n"
        "    ldp q5, q6, [%[src0]]\n"
        "    uzp1 v7.4s, v1.4s, v2.4s\n"
        "    uzp2 v1.4s, v1.4s, v2.4s\n"
        "    add %[src0], %[src0], %[in_stride]\n"
        "    ldp q2, q8, [%[src0]]\n"
        "    uzp1 v9.4s, v3.4s, v4.4s\n"
        "    uzp2 v3.4s, v3.4s, v4.4s\n"
        "    add %[src0], %[src0], %[in_stride]\n"
        "    ldp q4, q10, [%[src0]]\n"
        "    uzp1 v11.4s, v5.4s, v6.4s\n"
        "    uzp2 v5.4s, v5.4s, v6.4s\n"
        "    add %[src0], %[src0], %[in_stride]\n"
        "    ldp q6, q12, [%[src0]]\n"
        "    uzp1 v13.4s, v2.4s, v8.4s\n"
        "    uzp2 v2.4s, v2.4s, v8.4s\n"
        "    add %[src0], %[src0], %[in_stride]\n"
        "    ldp q8, q14, [%[src0]]\n"
        "    uzp1 v15.4s, v4.4s, v10.4s\n"
        "    uzp2 v4.4s, v4.4s, v10.4s\n"
        "    add %[src0], %[src0], %[in_stride]\n"
        "    ldp q10, q16, [%[src0]]\n"
        "    uzp1 v17.4s, v6.4s, v12.4s\n"
        "    uzp2 v6.4s, v6.4s, v12.4s\n"
        "    fadd v12.4s, v7.4s, v15.4s\n"
        "    fadd v18.4s, v1.4s, v4.4s\n"
        "    fsub v19.4s, v9.4s, v17.4s\n"
        "    fsub v20.4s, v3.4s, v6.4s\n"
        "    uzp1 v21.4s, v8.4s, v14.4s\n"
        "    uzp2 v8.4s, v8.4s, v14.4s\n"
        "    uzp1 v14.4s, v10.4s, v16.4s\n"
        "    uzp2 v10.4s, v10.4s, v16.4s\n"
        "    fadd v16.4s, v11.4s, v21.4s\n"
        "    fadd v22.4s, v5.4s, v8.4s\n"
        "    fsub v23.4s, v13.4s, v14.4s\n"
        "    fsub v24.4s, v2.4s, v10.4s\n"
        "    fadd v9.4s, v9.4s, v17.4s\n"
        "    fadd v3.4s, v3.4s, v6.4s\n"
        "    fadd v6.4s, v13.4s, v14.4s\n"
        "    fadd v2.4s, v2.4s, v10.4s\n"
        "    fadd v10.4s, v19.4s, v24.4s\n"
        "    fsub v13.4s, v20.4s, v23.4s\n"
        "    fsub v14.4s, v19.4s, v24.4s\n"
        "    fadd v17.4s, v20.4s, v23.4s\n"
        "    fsub v7.4s, v7.4s, v15.4s\n"
        "    fsub v1.4s, v1.4s, v4.4s\n"
        "    fsub v4.4s, v11.4s, v21.4s\n"
        "    fsub v5.4s, v5.4s, v8.4s\n"
        "    fadd v8.4s, v10.4s, v13.4s\n"
        "    fsub v10.4s, v13.4s, v10.4s\n"
        "    fadd v11.4s, v12.4s, v16.4s\n"
        "    fadd v13.4s, v18.4s, v22.4s\n"
        "    fadd v15.4s, v9.4s, v6.4s\n"
        "    fadd v19.4s, v3.4s, v2.4s\n"
        "    fadd v20.4s, v14.4s, v17.4s\n"
        "    fsub v14.4s, v17.4s, v14.4s\n"
        "    fadd v17.4s, v7.4s, v5.4s\n"
        "    fsub v21.4s, v1.4s, v4.4s\n"
        "    fmul v8.4s, v8.4s, v0.4s\n"
        "    fmul v10.4s, v10.4s, v0.4s\n"
        "    fsub v12.4s, v12.4s, v16.4s\n"
        "    fsub v16.4s, v18.4s, v22.4s\n"
        "    ldp %w[a1], %w[b1], [%[table]], #8\n"
        "    fsub v6.4s, v9.4s, v6.4s\n"
        "    fsub v2.4s, v3.4s, v2.4s\n"
        "    add %[src1], %[in], %[a1], lsl #3\n"
        "    fsub v3.4s, v7.4s, v5.4s\n"
        "    fadd v1.4s, v1.4s, v4.4s\n"
        "    add %[dst1], %[out], %[b1], lsl #3\n"
        "    ldp q4, q5, [%[src1]]\n"
        "    fmul v7.4s, v20.4s, v0.4s\n"
        "    fmul v9.4s, v14.4s, v0.4s\n"
        "    add %[src1], %[src1], %[in_stride]\n"
        "    fadd v14.4s, v11.4s, v15.4s\n"
        "    fadd v18.4s, v13.4s, v19.4s\n"
        "    ldp q20, q22, [%[src1]]\n"
        "    fsub v11.4s, v11.4s, v15.4s\n"
        "    fsub v13.4s, v13.4s, v19.4s\n"
        "    add %[src1], %[src1], %[in_stride]\n"
        "    fadd v15.4s, v17.4s, v8.4s\n"
        "    fadd v19.4s, v21.4s, v10.4s\n"
        "    ldp q23, q24, [%[src1]]\n"
        "    uzp1 v25.4s, v4.4s, v5.4s\n"
        "    uzp2 v4.4s, v4.4s, v5.4s\n"
        "    stp q14, q18, [%[dst0]]\n"
        "    add %[src1], %[src1], %[in_stride]\n"
        "    fsub v5.4s, v17.4s, v8.4s\n"
        "    fsub v8.4s, v21.4s, v10.4s\n"
        "    stp q11, q13, [%[dst0], #32]\n"
        "    ldp q10, q11, [%[src1]]\n"
        "    uzp1 v13.4s, v20.4s, v22.4s\n"
        "    uzp2 v14.4s, v20.4s, v22.4s\n"
        "    add %[dst0], %[dst0], %[out_stride]\n"
        "    add %[src1], %[src1], %[in_stride]\n"
        "    fadd v17.4s, v12.4s, v2.4s\n"
        "    fsub v18.4s, v16.4s, v6.4s\n"
        "    stp q15, q19, [%[dst0]]\n"
        "    ldp q15, q19, [%[src1]]\n"
        "    uzp1 v20.4s, v23.4s, v24.4s\n"
        "    uzp2 v21.4s, v23.4s, v24.4s\n"
        "    stp q5, q8, [%[dst0], #32]\n"
        "    add %[src1], %[src1], %[in_stride]\n"
        "    fsub v2.4s, v12.4s, v2.4s\n"
        "    fadd v5.4s, v16.4s, v6.4s\n"
        "    add %[dst0], %[dst0], %[out_stride]\n"
        "    ldp q6, q8, [%[src1]]\n"
        "    uzp1 v12.4s, v10.4s, v11.4s\n"
        "    uzp2 v10.4s, v10.4s, v11.4s\n"
        "    stp q17, q18, [%[dst0]]\n"
        "    add %[src1], %[src1], %[in_stride]\n"
        "    fadd v11.4s, v3.4s, v9.4s\n"
        "    fsub v16.4s, v1.4s, v7.4s\n"
        "    ldp q17, q18, [%[src1]]\n"
        "    uzp1 v22.4s, v15.4s, v19.4s\n"
        "    uzp2 v15.4s, v15.4s, v19.4s\n"
        "    stp q2, q5, [%[dst0], #32]\n"
        "    add %[src1], %[src1], %[in_stride]\n"
        "    fsub v2.4s, v3.4s, v9.4s\n"
        "    fadd v1.4s, v1.4s, v7.4s\n"
        "    add %[dst0], %[dst0], %[out_stride]\n"
        "    ldp q3, q5, [%[src1]]\n"
        "    uzp1 v7.4s, v6.4s, v8.4s\n"
        "    uzp2 v6.4s, v6.4s, v8.4s\n"
        "    stp q11, q16, [%[dst0]]\n"
        "    fadd v8.4s, v25.4s, v22.4s\n"
        "    fadd v9.4s, v4.4s, v15.4s\n"
        "    fsub v11.4s, v13.4s, v7.4s\n"
        "    fsub v16.4s, v14.4s, v6.4s\n"
        "    stp q2, q1, [%[dst0], #32]\n"
        "    uzp1 v1.4s, v17.4s, v18.4s\n"
        "    uzp2 v2.4s, v17.4s, v18.4s\n"
        "    uzp1 v17.4s, v3.4s, v5.4s\n"
        "    uzp2 v3.4s, v3.4s, v5.4s\n"
        "    fadd v5.4s, v20.4s, v1.4s\n"
        "    fadd v18.4s, v21.4s, v2.4s\n"
        "    fsub v19.4s, v12.4s, v17.4s\n"
        "    fsub v23.4s, v10.4s, v3.4s\n"
        "    fadd v7.4s, v13.4s, v7.4s\n"
        "    fadd v6.4s, v14.4s, v6.4s\n"
        "    fadd v12.4s, v12.4s, v17.4s\n"
        "    fadd v3.4s, v10.4s, v3.4s\n"
        "    fadd v10.4s, v11.4s, v23.4s\n"
        "    fsub v13.4s, v16.4s, v19.4s\n"
        "    fsub v11.4s, v11.4s, v23.4s\n"
        "    fadd v14.4s, v16.4s, v19.4s\n"
        "    fsub v16.4s, v25.4s, v22.4s\n"
        "    fsub v4.4s, v4.4s, v15.4s\n"
        "    fsub v1.4s, v20.4s, v1.4s\n"
        "    fsub v2.4s, v21.4s, v2.4s\n"
        "    fadd v15.4s, v10.4s, v13.4s\n"
        "    fsub v10.4s, v13.4s, v10.4s\n"
        "    fadd v13.4s, v8.4s, v5.4s\n"
        "    fadd v17.4s, v9.4s, v18.4s\n"
        "    fadd v19.4s, v7.4s, v12.4s\n"
        "    fadd v20.4s, v6.4s, v3.4s\n"
        "    fadd v21.4s, v11.4s, v14.4s\n"
        "    fsub v11.4s, v14.4s, v11.4s\n"
        "    fadd v14.4s, v16.4s, v2.4s\n"
        "    fsub v22.4s, v4.4s, v1.4s\n"
        "    fmul v15.4s, v15.4s, v0.4s\n"
        "    fmul v10.4s, v10.4s, v0.4s\n"
        "    fsub v5.4s, v8.4s, v5.4s\n"
        "    fsub v8.4s, v9.4s, v18.4s\n"
        "    fsub v7.4s, v7.4s, v12.4s\n"
        "    fsub v3.4s, v6.4s, v3.4s\n"
        "    subs %[count], %[count], #1\n"
        "    fsub v2.4s, v16.4s, v2.4s\n"
        "    fadd v1.4s, v4.4s, v1.4s\n"
        "    fmul v4.4s, v21.4s, v0.4s\n"
        "    fmul v6.4s, v11.4s, v0.4s\n"
        "    fadd v9.4s, v13.4s, v19.4s\n"
        "    fadd v11.4s, v17.4s, v20.4s\n"
        "    fsub v12.4s, v13.4s, v19.4s\n"
        "    fsub v13.4s, v17.4s, v20.4s\n"
        "    fadd v16.4s, v14.4s, v15.4s\n"
        "    fadd v17.4s, v22.4s, v10.4s\n"
        "    fsub v14.4s, v14.4s, v15.4s\n"
        "    fsub v10.4s, v22.4s, v10.4s\n"
        "    stp q9, q11, [%[dst1]]\n"
        "    fadd v9.4s, v5.4s, v3.4s\n"
        "    fsub v11.4s, v8.4s, v7.4s\n"
        "    stp q12, q13, [%[dst1], #32]\n"
        "    fsub v3.4s, v5.4s, v3.4s\n"
        "    fadd v5.4s, v8.4s, v7.4s\n"
        "    add %[dst1], %[dst1], %[out_stride]\n"
        "    fadd v7.4s, v2.4s, v6.4s\n"
        "    fsub v8.4s, v1.4s, v4.4s\n"
        "    stp q16, q17, [%[dst1]]\n"
        "    stp q14, q10, [%[dst1], #32]\n"
        "    fsub v2.4s, v2.4s, v6.4s\n"
        "    fadd v1.4s, v1.4s, v4.4s\n"
        "    add %[dst1], %[dst1], %[out_stride]\n"
        "    stp q9, q11, [%[dst1]]\n"
        "    stp q3, q5, [%[dst1], #32]\n"
        "    add %[dst1], %[dst1], %[out_stride]\n"
        "    stp q7, q8, [%[dst1]]\n"
        "    stp q2, q1, [%[dst1], #32]\n"
        "    b.ne 1b\n"
        : [in] "+r"(in), [out] "+r"(out), [table] "+r"(table), [count] "+r"(count), [in_stride] "+r"(in_stride), [out_stride] "+r"(out_stride), [bits] "=&r"(bits), [src0] "=&r"(src0), [dst0] "=&r"(dst0), [a0] "=&r"(a0), [b0] "=&r"(b0), [src1] "=&r"(src1), [dst1] "=&r"(dst1), [a1] "=&r"(a1), [b1] "=&r"(b1)
        :
        : "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", "v10", "v11", "v12", "v13", "v14", "v15", "v16", "v17", "v18", "v19", "v20", "v21", "v22", "v23", "v24", "v25", "cc", "memory");
}

/* Last pass, radix 8, in place: reads split vectors and writes the interleaved result.
 * Butterfly j < count uses the vectors at data + u*stride + 32*j. The last parameter is not used: it
 * exists because the last pass kernels of all targets share a signature. See the table builder in
 * onefft.c for the twiddle layout. */
static void onefft_neon_l8(float *data, const float *tw, size_t stride, size_t count, size_t regions) {
    (void)regions;
    size_t j;
    float *p1;
    float *p2;
    float *p3;
    float *p4;
    float *p5;
    float *p6;
    float *p7;
    __asm__ volatile(
        "    add %[p1], %[data], %[stride]\n"
        "    lsr %[j], %[count], #1\n"
        "    add %[p2], %[p1], %[stride]\n"
        "    add %[p3], %[p2], %[stride]\n"
        "    add %[p4], %[p3], %[stride]\n"
        "    add %[p5], %[p4], %[stride]\n"
        "    add %[p6], %[p5], %[stride]\n"
        "    add %[p7], %[p6], %[stride]\n"
        "1:\n"
        "    ldp q1, q0, [%[p1]]\n"
        "    ldp q3, q2, [%[p3]]\n"
        "    ldp q5, q4, [%[p5]]\n"
        "    ldp q7, q6, [%[p7]]\n"
        "    ldp q8, q9, [%[tw]]\n"
        "    ldp q10, q11, [%[data]]\n"
        "    ldp q12, q13, [%[p2]]\n"
        "    ldp q14, q15, [%[p4]]\n"
        "    ldp q16, q17, [%[p6]]\n"
        "    ldp q18, q19, [%[tw], #32]\n"
        "    ldp q20, q21, [%[tw], #64]\n"
        "    ldp q22, q23, [%[tw], #96]\n"
        "    fmul v24.4s, v1.4s, v8.s[0]\n"
        "    fmul v25.4s, v0.4s, v8.s[0]\n"
        "    fmul v26.4s, v3.4s, v8.s[1]\n"
        "    fmul v27.4s, v2.4s, v8.s[1]\n"
        "    fmul v28.4s, v5.4s, v8.s[2]\n"
        "    fmul v29.4s, v4.4s, v8.s[2]\n"
        "    fmul v30.4s, v7.4s, v8.s[3]\n"
        "    fmul v8.4s, v6.4s, v8.s[3]\n"
        "    fmls v24.4s, v0.4s, v9.s[0]\n"
        "    fmla v25.4s, v1.4s, v9.s[0]\n"
        "    fmls v26.4s, v2.4s, v9.s[1]\n"
        "    fmla v27.4s, v3.4s, v9.s[1]\n"
        "    fmls v28.4s, v4.4s, v9.s[2]\n"
        "    fmla v29.4s, v5.4s, v9.s[2]\n"
        "    fmls v30.4s, v6.4s, v9.s[3]\n"
        "    fmla v8.4s, v7.4s, v9.s[3]\n"
        "    fadd v0.4s, v10.4s, v24.4s\n"
        "    fadd v1.4s, v11.4s, v25.4s\n"
        "    fsub v2.4s, v10.4s, v24.4s\n"
        "    fsub v3.4s, v11.4s, v25.4s\n"
        "    fadd v4.4s, v12.4s, v26.4s\n"
        "    fadd v5.4s, v13.4s, v27.4s\n"
        "    fsub v6.4s, v12.4s, v26.4s\n"
        "    fsub v7.4s, v13.4s, v27.4s\n"
        "    fadd v9.4s, v14.4s, v28.4s\n"
        "    fadd v10.4s, v15.4s, v29.4s\n"
        "    fsub v11.4s, v14.4s, v28.4s\n"
        "    fsub v12.4s, v15.4s, v29.4s\n"
        "    fadd v13.4s, v16.4s, v30.4s\n"
        "    fadd v14.4s, v17.4s, v8.4s\n"
        "    fsub v15.4s, v16.4s, v30.4s\n"
        "    fsub v8.4s, v17.4s, v8.4s\n"
        "    trn1 v16.4s, v0.4s, v4.4s\n"
        "    trn2 v0.4s, v0.4s, v4.4s\n"
        "    trn1 v4.4s, v9.4s, v13.4s\n"
        "    trn2 v9.4s, v9.4s, v13.4s\n"
        "    trn1 v13.4s, v1.4s, v5.4s\n"
        "    trn2 v1.4s, v1.4s, v5.4s\n"
        "    trn1 v5.4s, v10.4s, v14.4s\n"
        "    trn2 v10.4s, v10.4s, v14.4s\n"
        "    trn1 v14.4s, v2.4s, v6.4s\n"
        "    trn2 v2.4s, v2.4s, v6.4s\n"
        "    trn1 v6.4s, v11.4s, v15.4s\n"
        "    trn2 v11.4s, v11.4s, v15.4s\n"
        "    trn1 v15.4s, v3.4s, v7.4s\n"
        "    trn2 v3.4s, v3.4s, v7.4s\n"
        "    zip1 v7.2d, v0.2d, v9.2d\n"
        "    zip2 v17.2d, v16.2d, v4.2d\n"
        "    zip2 v0.2d, v0.2d, v9.2d\n"
        "    zip1 v9.2d, v1.2d, v10.2d\n"
        "    zip2 v24.2d, v13.2d, v5.2d\n"
        "    zip2 v1.2d, v1.2d, v10.2d\n"
        "    zip1 v10.2d, v2.2d, v11.2d\n"
        "    zip2 v25.2d, v14.2d, v6.2d\n"
        "    zip2 v2.2d, v2.2d, v11.2d\n"
        "    fmul v11.4s, v7.4s, v18.4s\n"
        "    fmul v18.4s, v9.4s, v18.4s\n"
        "    fmul v26.4s, v17.4s, v20.4s\n"
        "    fmul v20.4s, v24.4s, v20.4s\n"
        "    fmul v27.4s, v0.4s, v22.4s\n"
        "    fmul v22.4s, v1.4s, v22.4s\n"
        "    fmls v11.4s, v9.4s, v19.4s\n"
        "    fmla v18.4s, v7.4s, v19.4s\n"
        "    fmls v26.4s, v24.4s, v21.4s\n"
        "    fmla v20.4s, v17.4s, v21.4s\n"
        "    fmls v27.4s, v1.4s, v23.4s\n"
        "    fmla v22.4s, v0.4s, v23.4s\n"
        "    zip1 v0.2d, v16.2d, v4.2d\n"
        "    trn1 v1.4s, v12.4s, v8.4s\n"
        "    trn2 v4.4s, v12.4s, v8.4s\n"
        "    ldp q7, q8, [%[tw], #128]\n"
        "    zip1 v5.2d, v13.2d, v5.2d\n"
        "    zip1 v6.2d, v14.2d, v6.2d\n"
        "    ldp q9, q12, [%[tw], #160]\n"
        "    ldp q13, q14, [%[tw], #192]\n"
        "    zip1 v16.2d, v3.2d, v4.2d\n"
        "    zip2 v17.2d, v15.2d, v1.2d\n"
        "    zip2 v3.2d, v3.2d, v4.2d\n"
        "    zip1 v1.2d, v15.2d, v1.2d\n"
        "    fmul v4.4s, v10.4s, v7.4s\n"
        "    fmul v7.4s, v16.4s, v7.4s\n"
        "    fadd v15.4s, v0.4s, v26.4s\n"
        "    fadd v19.4s, v5.4s, v20.4s\n"
        "    fmul v21.4s, v25.4s, v9.4s\n"
        "    fmul v9.4s, v17.4s, v9.4s\n"
        "    fmul v23.4s, v2.4s, v13.4s\n"
        "    fmul v13.4s, v3.4s, v13.4s\n"
        "    fmls v4.4s, v16.4s, v8.4s\n"
        "    fmla v7.4s, v10.4s, v8.4s\n"
        "    fmls v21.4s, v17.4s, v12.4s\n"
        "    fmla v9.4s, v25.4s, v12.4s\n"
        "    fmls v23.4s, v3.4s, v14.4s\n"
        "    fmla v13.4s, v2.4s, v14.4s\n"
        "    fsub v0.4s, v0.4s, v26.4s\n"
        "    fsub v2.4s, v5.4s, v20.4s\n"
        "    fadd v3.4s, v11.4s, v27.4s\n"
        "    fadd v5.4s, v18.4s, v22.4s\n"
        "    fsub v8.4s, v11.4s, v27.4s\n"
        "    fsub v10.4s, v18.4s, v22.4s\n"
        "    fadd v11.4s, v6.4s, v21.4s\n"
        "    fadd v12.4s, v1.4s, v9.4s\n"
        "    fsub v6.4s, v6.4s, v21.4s\n"
        "    fsub v1.4s, v1.4s, v9.4s\n"
        "    fadd v9.4s, v4.4s, v23.4s\n"
        "    fadd v14.4s, v7.4s, v13.4s\n"
        "    fsub v4.4s, v4.4s, v23.4s\n"
        "    fsub v7.4s, v7.4s, v13.4s\n"
        "    fadd v13.4s, v15.4s, v3.4s\n"
        "    fadd v16.4s, v19.4s, v5.4s\n"
        "    fsub v3.4s, v15.4s, v3.4s\n"
        "    fsub v5.4s, v19.4s, v5.4s\n"
        "    fadd v15.4s, v0.4s, v10.4s\n"
        "    fsub v17.4s, v2.4s, v8.4s\n"
        "    fsub v0.4s, v0.4s, v10.4s\n"
        "    fadd v2.4s, v2.4s, v8.4s\n"
        "    fadd v8.4s, v11.4s, v9.4s\n"
        "    fadd v10.4s, v12.4s, v14.4s\n"
        "    fsub v9.4s, v11.4s, v9.4s\n"
        "    fsub v11.4s, v12.4s, v14.4s\n"
        "    fadd v12.4s, v6.4s, v7.4s\n"
        "    fsub v14.4s, v1.4s, v4.4s\n"
        "    fsub v6.4s, v6.4s, v7.4s\n"
        "    fadd v1.4s, v1.4s, v4.4s\n"
        "    zip1 v4.4s, v13.4s, v16.4s\n"
        "    zip2 v7.4s, v13.4s, v16.4s\n"
        "    zip1 v13.4s, v15.4s, v17.4s\n"
        "    zip2 v15.4s, v15.4s, v17.4s\n"
        "    zip1 v16.4s, v3.4s, v5.4s\n"
        "    zip2 v3.4s, v3.4s, v5.4s\n"
        "    stp q4, q7, [%[data]]\n"
        "    zip1 v4.4s, v0.4s, v2.4s\n"
        "    zip2 v0.4s, v0.4s, v2.4s\n"
        "    stp q13, q15, [%[p2]]\n"
        "    ldp q5, q2, [%[p1], #32]\n"
        "    ldp q7, q13, [%[data], #32]\n"
        "    zip1 v15.4s, v8.4s, v10.4s\n"
        "    zip2 v8.4s, v8.4s, v10.4s\n"
        "    stp q16, q3, [%[p4]]\n"
        "    ldp q10, q3, [%[p3], #32]\n"
        "    ldp q16, q17, [%[p2], #32]\n"
        "    ldp q18, q19, [%[p4], #32]\n"
        "    zip1 v20.4s, v12.4s, v14.4s\n"
        "    zip2 v12.4s, v12.4s, v14.4s\n"
        "    stp q4, q0, [%[p6]]\n"
        "    ldp q4, q0, [%[p5], #32]\n"
        "    ldp q21, q14, [%[p7], #32]\n"
        "    ldp q22, q23, [%[tw], #224]\n"
        "    ldp q24, q25, [%[p6], #32]\n"
        "    zip1 v26.4s, v9.4s, v11.4s\n"
        "    zip2 v9.4s, v9.4s, v11.4s\n"
        "    stp q15, q8, [%[p1]]\n"
        "    ldp q8, q11, [%[tw], #256]\n"
        "    zip1 v15.4s, v6.4s, v1.4s\n"
        "    zip2 v1.4s, v6.4s, v1.4s\n"
        "    stp q20, q12, [%[p3]]\n"
        "    stp q26, q9, [%[p5]]\n"
        "    ldp q6, q9, [%[tw], #288]\n"
        "    stp q15, q1, [%[p7]]\n"
        "    fmul v1.4s, v5.4s, v22.s[0]\n"
        "    fmul v12.4s, v2.4s, v22.s[0]\n"
        "    ldp q15, q20, [%[tw], #320]\n"
        "    fmul v26.4s, v10.4s, v22.s[1]\n"
        "    fmul v27.4s, v3.4s, v22.s[1]\n"
        "    fmul v28.4s, v4.4s, v22.s[2]\n"
        "    fmul v29.4s, v0.4s, v22.s[2]\n"
        "    fmul v30.4s, v21.4s, v22.s[3]\n"
        "    fmul v22.4s, v14.4s, v22.s[3]\n"
        "    fmls v1.4s, v2.4s, v23.s[0]\n"
        "    fmla v12.4s, v5.4s, v23.s[0]\n"
        "    fmls v26.4s, v3.4s, v23.s[1]\n"
        "    fmla v27.4s, v10.4s, v23.s[1]\n"
        "    fmls v28.4s, v0.4s, v23.s[2]\n"
        "    fmla v29.4s, v4.4s, v23.s[2]\n"
        "    fmls v30.4s, v14.4s, v23.s[3]\n"
        "    fmla v22.4s, v21.4s, v23.s[3]\n"
        "    fadd v0.4s, v7.4s, v1.4s\n"
        "    fadd v2.4s, v13.4s, v12.4s\n"
        "    fsub v1.4s, v7.4s, v1.4s\n"
        "    fsub v3.4s, v13.4s, v12.4s\n"
        "    fadd v4.4s, v16.4s, v26.4s\n"
        "    fadd v5.4s, v17.4s, v27.4s\n"
        "    fsub v7.4s, v16.4s, v26.4s\n"
        "    fsub v10.4s, v17.4s, v27.4s\n"
        "    fadd v12.4s, v18.4s, v28.4s\n"
        "    fadd v13.4s, v19.4s, v29.4s\n"
        "    fsub v14.4s, v18.4s, v28.4s\n"
        "    fsub v16.4s, v19.4s, v29.4s\n"
        "    fadd v17.4s, v24.4s, v30.4s\n"
        "    fadd v18.4s, v25.4s, v22.4s\n"
        "    fsub v19.4s, v24.4s, v30.4s\n"
        "    fsub v21.4s, v25.4s, v22.4s\n"
        "    trn1 v22.4s, v0.4s, v4.4s\n"
        "    trn2 v0.4s, v0.4s, v4.4s\n"
        "    trn1 v4.4s, v12.4s, v17.4s\n"
        "    trn2 v12.4s, v12.4s, v17.4s\n"
        "    trn1 v17.4s, v2.4s, v5.4s\n"
        "    trn2 v2.4s, v2.4s, v5.4s\n"
        "    trn1 v5.4s, v13.4s, v18.4s\n"
        "    trn2 v13.4s, v13.4s, v18.4s\n"
        "    trn1 v18.4s, v1.4s, v7.4s\n"
        "    trn2 v1.4s, v1.4s, v7.4s\n"
        "    trn1 v7.4s, v14.4s, v19.4s\n"
        "    trn2 v14.4s, v14.4s, v19.4s\n"
        "    trn1 v19.4s, v3.4s, v10.4s\n"
        "    trn2 v3.4s, v3.4s, v10.4s\n"
        "    zip1 v10.2d, v0.2d, v12.2d\n"
        "    zip2 v23.2d, v22.2d, v4.2d\n"
        "    zip2 v0.2d, v0.2d, v12.2d\n"
        "    zip1 v12.2d, v2.2d, v13.2d\n"
        "    zip2 v24.2d, v17.2d, v5.2d\n"
        "    zip2 v2.2d, v2.2d, v13.2d\n"
        "    zip1 v13.2d, v1.2d, v14.2d\n"
        "    zip2 v25.2d, v18.2d, v7.2d\n"
        "    zip2 v1.2d, v1.2d, v14.2d\n"
        "    fmul v14.4s, v10.4s, v8.4s\n"
        "    fmul v8.4s, v12.4s, v8.4s\n"
        "    fmul v26.4s, v23.4s, v6.4s\n"
        "    fmul v6.4s, v24.4s, v6.4s\n"
        "    fmul v27.4s, v0.4s, v15.4s\n"
        "    fmul v15.4s, v2.4s, v15.4s\n"
        "    fmls v14.4s, v12.4s, v11.4s\n"
        "    fmla v8.4s, v10.4s, v11.4s\n"
        "    fmls v26.4s, v24.4s, v9.4s\n"
        "    fmla v6.4s, v23.4s, v9.4s\n"
        "    fmls v27.4s, v2.4s, v20.4s\n"
        "    fmla v15.4s, v0.4s, v20.4s\n"
        "    zip1 v0.2d, v22.2d, v4.2d\n"
        "    trn1 v2.4s, v16.4s, v21.4s\n"
        "    trn2 v4.4s, v16.4s, v21.4s\n"
        "    ldp q9, q10, [%[tw], #352]\n"
        "    zip1 v5.2d, v17.2d, v5.2d\n"
        "    zip1 v7.2d, v18.2d, v7.2d\n"
        "    ldp q11, q12, [%[tw], #384]\n"
        "    ldp q16, q17, [%[tw], #416]\n"
        "    zip1 v18.2d, v3.2d, v4.2d\n"
        "    zip2 v20.2d, v19.2d, v2.2d\n"
        "    zip2 v3.2d, v3.2d, v4.2d\n"
        "    zip1 v2.2d, v19.2d, v2.2d\n"
        "    fmul v4.4s, v13.4s, v9.4s\n"
        "    fmul v9.4s, v18.4s, v9.4s\n"
        "    fadd v19.4s, v0.4s, v26.4s\n"
        "    fadd v21.4s, v5.4s, v6.4s\n"
        "    fmul v22.4s, v25.4s, v11.4s\n"
        "    fmul v11.4s, v20.4s, v11.4s\n"
        "    fmul v23.4s, v1.4s, v16.4s\n"
        "    fmul v16.4s, v3.4s, v16.4s\n"
        "    fmls v4.4s, v18.4s, v10.4s\n"
        "    fmla v9.4s, v13.4s, v10.4s\n"
        "    fmls v22.4s, v20.4s, v12.4s\n"
        "    fmla v11.4s, v25.4s, v12.4s\n"
        "    fmls v23.4s, v3.4s, v17.4s\n"
        "    fmla v16.4s, v1.4s, v17.4s\n"
        "    fsub v0.4s, v0.4s, v26.4s\n"
        "    fsub v1.4s, v5.4s, v6.4s\n"
        "    fadd v3.4s, v14.4s, v27.4s\n"
        "    fadd v5.4s, v8.4s, v15.4s\n"
        "    fsub v6.4s, v14.4s, v27.4s\n"
        "    fsub v8.4s, v8.4s, v15.4s\n"
        "    fadd v10.4s, v7.4s, v22.4s\n"
        "    fadd v12.4s, v2.4s, v11.4s\n"
        "    fsub v7.4s, v7.4s, v22.4s\n"
        "    fsub v2.4s, v2.4s, v11.4s\n"
        "    fadd v11.4s, v4.4s, v23.4s\n"
        "    fadd v13.4s, v9.4s, v16.4s\n"
        "    fsub v4.4s, v4.4s, v23.4s\n"
        "    fsub v9.4s, v9.4s, v16.4s\n"
        "    fadd v14.4s, v19.4s, v3.4s\n"
        "    fadd v15.4s, v21.4s, v5.4s\n"
        "    fsub v3.4s, v19.4s, v3.4s\n"
        "    fsub v5.4s, v21.4s, v5.4s\n"
        "    fadd v16.4s, v0.4s, v8.4s\n"
        "    fsub v17.4s, v1.4s, v6.4s\n"
        "    fsub v0.4s, v0.4s, v8.4s\n"
        "    fadd v1.4s, v1.4s, v6.4s\n"
        "    fadd v6.4s, v10.4s, v11.4s\n"
        "    fadd v8.4s, v12.4s, v13.4s\n"
        "    fsub v10.4s, v10.4s, v11.4s\n"
        "    fsub v11.4s, v12.4s, v13.4s\n"
        "    fadd v12.4s, v7.4s, v9.4s\n"
        "    fsub v13.4s, v2.4s, v4.4s\n"
        "    fsub v7.4s, v7.4s, v9.4s\n"
        "    fadd v2.4s, v2.4s, v4.4s\n"
        "    zip1 v4.4s, v14.4s, v15.4s\n"
        "    zip2 v9.4s, v14.4s, v15.4s\n"
        "    zip1 v14.4s, v16.4s, v17.4s\n"
        "    zip2 v15.4s, v16.4s, v17.4s\n"
        "    zip1 v16.4s, v3.4s, v5.4s\n"
        "    zip2 v3.4s, v3.4s, v5.4s\n"
        "    stp q4, q9, [%[data], #32]\n"
        "    zip1 v4.4s, v0.4s, v1.4s\n"
        "    zip2 v0.4s, v0.4s, v1.4s\n"
        "    stp q14, q15, [%[p2], #32]\n"
        "    zip1 v1.4s, v6.4s, v8.4s\n"
        "    zip2 v5.4s, v6.4s, v8.4s\n"
        "    stp q16, q3, [%[p4], #32]\n"
        "    add %[data], %[data], #64\n"
        "    zip1 v3.4s, v12.4s, v13.4s\n"
        "    zip2 v6.4s, v12.4s, v13.4s\n"
        "    stp q4, q0, [%[p6], #32]\n"
        "    add %[p2], %[p2], #64\n"
        "    add %[p4], %[p4], #64\n"
        "    zip1 v0.4s, v10.4s, v11.4s\n"
        "    zip2 v4.4s, v10.4s, v11.4s\n"
        "    stp q1, q5, [%[p1], #32]\n"
        "    add %[p6], %[p6], #64\n"
        "    add %[tw], %[tw], #448\n"
        "    subs %[j], %[j], #1\n"
        "    zip1 v1.4s, v7.4s, v2.4s\n"
        "    zip2 v2.4s, v7.4s, v2.4s\n"
        "    stp q3, q6, [%[p3], #32]\n"
        "    add %[p1], %[p1], #64\n"
        "    stp q0, q4, [%[p5], #32]\n"
        "    add %[p3], %[p3], #64\n"
        "    stp q1, q2, [%[p7], #32]\n"
        "    add %[p5], %[p5], #64\n"
        "    add %[p7], %[p7], #64\n"
        "    b.ne 1b\n"
        : [data] "+r"(data), [tw] "+r"(tw), [stride] "+r"(stride), [count] "+r"(count), [j] "=&r"(j), [p1] "=&r"(p1), [p2] "=&r"(p2), [p3] "=&r"(p3), [p4] "=&r"(p4), [p5] "=&r"(p5), [p6] "=&r"(p6), [p7] "=&r"(p7)
        :
        : "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", "v10", "v11", "v12", "v13", "v14", "v15", "v16", "v17", "v18", "v19", "v20", "v21", "v22", "v23", "v24", "v25", "v26", "v27", "v28", "v29", "v30", "cc", "memory");
}

/* Last pass, radix 4, in place: reads split vectors and writes the interleaved result.
 * Butterfly j < count uses the vectors at data + u*stride + 32*j. The last parameter is not used: it
 * exists because the last pass kernels of all targets share a signature. See the table builder in
 * onefft.c for the twiddle layout. */
static void onefft_neon_l4(float *data, const float *tw, size_t stride, size_t count, size_t regions) {
    (void)regions;
    size_t j;
    float *p1;
    float *p2;
    float *p3;
    __asm__ volatile(
        "    add %[p1], %[data], %[stride]\n"
        "    lsr %[j], %[count], #1\n"
        "    add %[p2], %[p1], %[stride]\n"
        "    add %[p3], %[p2], %[stride]\n"
        "1:\n"
        "    ldp q0, q1, [%[data]]\n"
        "    ldp q2, q3, [%[p1]]\n"
        "    ldp q4, q5, [%[p2]]\n"
        "    ldp q6, q7, [%[p3]]\n"
        "    ldp q8, q9, [%[data], #32]\n"
        "    ldp q10, q11, [%[p1], #32]\n"
        "    ldp q12, q13, [%[tw]]\n"
        "    ldp q14, q15, [%[tw], #32]\n"
        "    ldp q16, q17, [%[tw], #64]\n"
        "    ldp q18, q19, [%[p2], #32]\n"
        "    ldp q20, q21, [%[p3], #32]\n"
        "    trn1 v22.4s, v0.4s, v2.4s\n"
        "    trn2 v0.4s, v0.4s, v2.4s\n"
        "    trn1 v2.4s, v4.4s, v6.4s\n"
        "    trn2 v4.4s, v4.4s, v6.4s\n"
        "    trn1 v6.4s, v1.4s, v3.4s\n"
        "    trn2 v1.4s, v1.4s, v3.4s\n"
        "    trn1 v3.4s, v5.4s, v7.4s\n"
        "    trn2 v5.4s, v5.4s, v7.4s\n"
        "    trn1 v7.4s, v8.4s, v10.4s\n"
        "    trn2 v8.4s, v8.4s, v10.4s\n"
        "    trn1 v10.4s, v18.4s, v20.4s\n"
        "    trn2 v18.4s, v18.4s, v20.4s\n"
        "    zip1 v20.2d, v0.2d, v4.2d\n"
        "    zip2 v23.2d, v22.2d, v2.2d\n"
        "    zip2 v0.2d, v0.2d, v4.2d\n"
        "    zip1 v4.2d, v1.2d, v5.2d\n"
        "    zip2 v24.2d, v6.2d, v3.2d\n"
        "    zip2 v1.2d, v1.2d, v5.2d\n"
        "    zip1 v5.2d, v8.2d, v18.2d\n"
        "    fmul v25.4s, v20.4s, v12.4s\n"
        "    fmul v12.4s, v4.4s, v12.4s\n"
        "    fmul v26.4s, v23.4s, v14.4s\n"
        "    fmul v14.4s, v24.4s, v14.4s\n"
        "    fmul v27.4s, v0.4s, v16.4s\n"
        "    fmul v16.4s, v1.4s, v16.4s\n"
        "    fmls v25.4s, v4.4s, v13.4s\n"
        "    fmla v12.4s, v20.4s, v13.4s\n"
        "    fmls v26.4s, v24.4s, v15.4s\n"
        "    fmla v14.4s, v23.4s, v15.4s\n"
        "    fmls v27.4s, v1.4s, v17.4s\n"
        "    fmla v16.4s, v0.4s, v17.4s\n"
        "    zip1 v0.2d, v22.2d, v2.2d\n"
        "    trn1 v1.4s, v9.4s, v11.4s\n"
        "    trn2 v2.4s, v9.4s, v11.4s\n"
        "    trn1 v4.4s, v19.4s, v21.4s\n"
        "    trn2 v9.4s, v19.4s, v21.4s\n"
        "    zip2 v11.2d, v7.2d, v10.2d\n"
        "    zip2 v8.2d, v8.2d, v18.2d\n"
        "    zip1 v13.2d, v2.2d, v9.2d\n"
        "    zip1 v3.2d, v6.2d, v3.2d\n"
        "    ldp q6, q15, [%[tw], #96]\n"
        "    ldp q17, q18, [%[tw], #128]\n"
        "    ldp q19, q20, [%[tw], #160]\n"
        "    zip2 v21.2d, v1.2d, v4.2d\n"
        "    zip2 v2.2d, v2.2d, v9.2d\n"
        "    zip1 v7.2d, v7.2d, v10.2d\n"
        "    zip1 v1.2d, v1.2d, v4.2d\n"
        "    fadd v4.4s, v0.4s, v26.4s\n"
        "    fadd v9.4s, v3.4s, v14.4s\n"
        "    fsub v0.4s, v0.4s, v26.4s\n"
        "    fsub v3.4s, v3.4s, v14.4s\n"
        "    fmul v10.4s, v5.4s, v6.4s\n"
        "    fmul v6.4s, v13.4s, v6.4s\n"
        "    fmul v14.4s, v11.4s, v17.4s\n"
        "    fmul v17.4s, v21.4s, v17.4s\n"
        "    fmul v22.4s, v8.4s, v19.4s\n"
        "    fmul v19.4s, v2.4s, v19.4s\n"
        "    fmls v10.4s, v13.4s, v15.4s\n"
        "    fmla v6.4s, v5.4s, v15.4s\n"
        "    fmls v14.4s, v21.4s, v18.4s\n"
        "    fmla v17.4s, v11.4s, v18.4s\n"
        "    fmls v22.4s, v2.4s, v20.4s\n"
        "    fmla v19.4s, v8.4s, v20.4s\n"
        "    fadd v2.4s, v25.4s, v27.4s\n"
        "    fadd v5.4s, v12.4s, v16.4s\n"
        "    fsub v8.4s, v25.4s, v27.4s\n"
        "    fsub v11.4s, v12.4s, v16.4s\n"
        "    fadd v12.4s, v7.4s, v14.4s\n"
        "    fadd v13.4s, v1.4s, v17.4s\n"
        "    fsub v7.4s, v7.4s, v14.4s\n"
        "    fsub v1.4s, v1.4s, v17.4s\n"
        "    fadd v14.4s, v10.4s, v22.4s\n"
        "    fadd v15.4s, v6.4s, v19.4s\n"
        "    fsub v10.4s, v10.4s, v22.4s\n"
        "    fsub v6.4s, v6.4s, v19.4s\n"
        "    fadd v16.4s, v4.4s, v2.4s\n"
        "    fadd v17.4s, v9.4s, v5.4s\n"
        "    fsub v2.4s, v4.4s, v2.4s\n"
        "    fsub v4.4s, v9.4s, v5.4s\n"
        "    fadd v5.4s, v0.4s, v11.4s\n"
        "    fsub v9.4s, v3.4s, v8.4s\n"
        "    fsub v0.4s, v0.4s, v11.4s\n"
        "    fadd v3.4s, v3.4s, v8.4s\n"
        "    fadd v8.4s, v12.4s, v14.4s\n"
        "    fadd v11.4s, v13.4s, v15.4s\n"
        "    fsub v12.4s, v12.4s, v14.4s\n"
        "    fsub v13.4s, v13.4s, v15.4s\n"
        "    fadd v14.4s, v7.4s, v6.4s\n"
        "    fsub v15.4s, v1.4s, v10.4s\n"
        "    fsub v6.4s, v7.4s, v6.4s\n"
        "    fadd v1.4s, v1.4s, v10.4s\n"
        "    zip1 v7.4s, v16.4s, v17.4s\n"
        "    zip2 v10.4s, v16.4s, v17.4s\n"
        "    zip1 v16.4s, v5.4s, v9.4s\n"
        "    zip2 v5.4s, v5.4s, v9.4s\n"
        "    zip1 v9.4s, v2.4s, v4.4s\n"
        "    zip2 v2.4s, v2.4s, v4.4s\n"
        "    stp q7, q10, [%[data]]\n"
        "    zip1 v4.4s, v0.4s, v3.4s\n"
        "    zip2 v0.4s, v0.4s, v3.4s\n"
        "    stp q16, q5, [%[p1]]\n"
        "    zip1 v3.4s, v8.4s, v11.4s\n"
        "    zip2 v5.4s, v8.4s, v11.4s\n"
        "    stp q9, q2, [%[p2]]\n"
        "    zip1 v2.4s, v14.4s, v15.4s\n"
        "    zip2 v7.4s, v14.4s, v15.4s\n"
        "    stp q4, q0, [%[p3]]\n"
        "    zip1 v0.4s, v12.4s, v13.4s\n"
        "    zip2 v4.4s, v12.4s, v13.4s\n"
        "    stp q3, q5, [%[data], #32]\n"
        "    add %[tw], %[tw], #192\n"
        "    subs %[j], %[j], #1\n"
        "    zip1 v3.4s, v6.4s, v1.4s\n"
        "    zip2 v1.4s, v6.4s, v1.4s\n"
        "    stp q2, q7, [%[p1], #32]\n"
        "    add %[data], %[data], #64\n"
        "    stp q0, q4, [%[p2], #32]\n"
        "    add %[p1], %[p1], #64\n"
        "    stp q3, q1, [%[p3], #32]\n"
        "    add %[p2], %[p2], #64\n"
        "    add %[p3], %[p3], #64\n"
        "    b.ne 1b\n"
        : [data] "+r"(data), [tw] "+r"(tw), [stride] "+r"(stride), [count] "+r"(count), [j] "=&r"(j), [p1] "=&r"(p1), [p2] "=&r"(p2), [p3] "=&r"(p3)
        :
        : "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", "v10", "v11", "v12", "v13", "v14", "v15", "v16", "v17", "v18", "v19", "v20", "v21", "v22", "v23", "v24", "v25", "v26", "v27", "cc", "memory");
}

static const float onefft_neon_n8_consts[16] ONEFFT_ALIGNED(64) = {
    0x1.0000000000000p+0f, 0x1.0000000000000p+0f, -0x0.0p+0f, -0x0.0p+0f,
    0x0.0p+0f, 0x0.0p+0f, -0x1.0000000000000p+0f, -0x1.0000000000000p+0f,
    0x1.0000000000000p+0f, 0x1.6a09e667f3bcdp-1f, -0x0.0p+0f, -0x1.6a09e667f3bccp-1f,
    0x0.0p+0f, -0x1.6a09e667f3bccp-1f, -0x1.0000000000000p+0f, -0x1.6a09e667f3bcdp-1f,
};
/* Complete 8 point transform from `in` to `out`, both interleaved. */
static void onefft_neon_n8(const float *in, float *out) {
    const float *consts = onefft_neon_n8_consts;
    __asm__ volatile(
        "    ldp q0, q1, [%[in]]\n"
        "    ldp q2, q3, [%[in], #32]\n"
        "    ldp q4, q5, [%[consts]]\n"
        "    ldp q6, q7, [%[consts], #32]\n"
        "    uzp1 v8.4s, v0.4s, v1.4s\n"
        "    uzp2 v0.4s, v0.4s, v1.4s\n"
        "    uzp1 v1.4s, v2.4s, v3.4s\n"
        "    uzp2 v2.4s, v2.4s, v3.4s\n"
        "    fadd v3.4s, v8.4s, v1.4s\n"
        "    fadd v9.4s, v0.4s, v2.4s\n"
        "    fsub v1.4s, v8.4s, v1.4s\n"
        "    fsub v0.4s, v0.4s, v2.4s\n"
        "    zip2 v2.2d, v3.2d, v1.2d\n"
        "    zip2 v8.2d, v9.2d, v0.2d\n"
        "    zip1 v1.2d, v3.2d, v1.2d\n"
        "    zip1 v0.2d, v9.2d, v0.2d\n"
        "    fmul v3.4s, v2.4s, v4.4s\n"
        "    fmul v4.4s, v8.4s, v4.4s\n"
        "    fmls v3.4s, v8.4s, v5.4s\n"
        "    fmla v4.4s, v2.4s, v5.4s\n"
        "    fadd v2.4s, v1.4s, v3.4s\n"
        "    fadd v5.4s, v0.4s, v4.4s\n"
        "    fsub v1.4s, v1.4s, v3.4s\n"
        "    fsub v0.4s, v0.4s, v4.4s\n"
        "    uzp2 v3.4s, v2.4s, v1.4s\n"
        "    uzp2 v4.4s, v5.4s, v0.4s\n"
        "    uzp1 v1.4s, v2.4s, v1.4s\n"
        "    uzp1 v0.4s, v5.4s, v0.4s\n"
        "    fmul v2.4s, v3.4s, v6.4s\n"
        "    fmul v5.4s, v4.4s, v6.4s\n"
        "    fmls v2.4s, v4.4s, v7.4s\n"
        "    fmla v5.4s, v3.4s, v7.4s\n"
        "    fadd v3.4s, v1.4s, v2.4s\n"
        "    fadd v4.4s, v0.4s, v5.4s\n"
        "    fsub v1.4s, v1.4s, v2.4s\n"
        "    fsub v0.4s, v0.4s, v5.4s\n"
        "    zip1 v2.4s, v3.4s, v4.4s\n"
        "    zip2 v3.4s, v3.4s, v4.4s\n"
        "    zip1 v4.4s, v1.4s, v0.4s\n"
        "    zip2 v0.4s, v1.4s, v0.4s\n"
        "    stp q2, q3, [%[out]]\n"
        "    stp q4, q0, [%[out], #32]\n"
        : [in] "+r"(in), [out] "+r"(out), [consts] "+r"(consts)
        :
        : "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", "cc", "memory");
}

static const float onefft_neon_n16_consts[24] ONEFFT_ALIGNED(64) = {
    0x1.0000000000000p+0f, 0x1.d906bcf328d46p-1f, 0x1.6a09e667f3bcdp-1f, 0x1.87de2a6aea964p-2f,
    0x0.0p+0f, -0x1.87de2a6aea963p-2f, -0x1.6a09e667f3bccp-1f, -0x1.d906bcf328d46p-1f,
    0x1.0000000000000p+0f, 0x1.6a09e667f3bcdp-1f, -0x0.0p+0f, -0x1.6a09e667f3bccp-1f,
    0x0.0p+0f, -0x1.6a09e667f3bccp-1f, -0x1.0000000000000p+0f, -0x1.6a09e667f3bcdp-1f,
    0x1.0000000000000p+0f, 0x1.87de2a6aea964p-2f, -0x1.6a09e667f3bccp-1f, -0x1.d906bcf328d47p-1f,
    0x0.0p+0f, -0x1.d906bcf328d46p-1f, -0x1.6a09e667f3bcdp-1f, 0x1.87de2a6aea961p-2f,
};
/* Complete 16 point transform from `in` to `out`, both interleaved. */
static void onefft_neon_n16(const float *in, float *out) {
    const float *consts = onefft_neon_n16_consts;
    __asm__ volatile(
        "    ldp q0, q1, [%[in]]\n"
        "    ldp q2, q3, [%[in], #32]\n"
        "    ldp q4, q5, [%[in], #64]\n"
        "    ldp q6, q7, [%[in], #96]\n"
        "    ldp q8, q9, [%[consts]]\n"
        "    ldp q10, q11, [%[consts], #32]\n"
        "    ldp q12, q13, [%[consts], #64]\n"
        "    uzp1 v14.4s, v0.4s, v1.4s\n"
        "    uzp2 v0.4s, v0.4s, v1.4s\n"
        "    uzp1 v1.4s, v2.4s, v3.4s\n"
        "    uzp2 v2.4s, v2.4s, v3.4s\n"
        "    uzp1 v3.4s, v4.4s, v5.4s\n"
        "    uzp2 v4.4s, v4.4s, v5.4s\n"
        "    uzp1 v5.4s, v6.4s, v7.4s\n"
        "    uzp2 v6.4s, v6.4s, v7.4s\n"
        "    fadd v7.4s, v14.4s, v3.4s\n"
        "    fadd v15.4s, v0.4s, v4.4s\n"
        "    fsub v3.4s, v14.4s, v3.4s\n"
        "    fsub v0.4s, v0.4s, v4.4s\n"
        "    fadd v4.4s, v1.4s, v5.4s\n"
        "    fadd v14.4s, v2.4s, v6.4s\n"
        "    fsub v1.4s, v1.4s, v5.4s\n"
        "    fsub v2.4s, v2.4s, v6.4s\n"
        "    fadd v5.4s, v7.4s, v4.4s\n"
        "    fadd v6.4s, v15.4s, v14.4s\n"
        "    fsub v4.4s, v7.4s, v4.4s\n"
        "    fsub v7.4s, v15.4s, v14.4s\n"
        "    fadd v14.4s, v3.4s, v2.4s\n"
        "    fsub v15.4s, v0.4s, v1.4s\n"
        "    fsub v2.4s, v3.4s, v2.4s\n"
        "    fadd v0.4s, v0.4s, v1.4s\n"
        "    trn1 v1.4s, v5.4s, v14.4s\n"
        "    trn2 v3.4s, v5.4s, v14.4s\n"
        "    trn1 v5.4s, v4.4s, v2.4s\n"
        "    trn2 v2.4s, v4.4s, v2.4s\n"
        "    trn1 v4.4s, v6.4s, v15.4s\n"
        "    trn2 v6.4s, v6.4s, v15.4s\n"
        "    trn1 v14.4s, v7.4s, v0.4s\n"
        "    trn2 v0.4s, v7.4s, v0.4s\n"
        "    zip1 v7.2d, v3.2d, v2.2d\n"
        "    zip2 v15.2d, v1.2d, v5.2d\n"
        "    zip2 v2.2d, v3.2d, v2.2d\n"
        "    zip1 v3.2d, v6.2d, v0.2d\n"
        "    zip2 v16.2d, v4.2d, v14.2d\n"
        "    zip2 v0.2d, v6.2d, v0.2d\n"
        "    fmul v6.4s, v7.4s, v8.4s\n"
        "    fmul v8.4s, v3.4s, v8.4s\n"
        "    fmul v17.4s, v15.4s, v10.4s\n"
        "    fmul v10.4s, v16.4s, v10.4s\n"
        "    fmul v18.4s, v2.4s, v12.4s\n"
        "    fmul v12.4s, v0.4s, v12.4s\n"
        "    fmls v6.4s, v3.4s, v9.4s\n"
        "    fmla v8.4s, v7.4s, v9.4s\n"
        "    fmls v17.4s, v16.4s, v11.4s\n"
        "    fmla v10.4s, v15.4s, v11.4s\n"
        "    fmls v18.4s, v0.4s, v13.4s\n"
        "    fmla v12.4s, v2.4s, v13.4s\n"
        "    zip1 v0.2d, v1.2d, v5.2d\n"
        "    zip1 v1.2d, v4.2d, v14.2d\n"
        "    fadd v2.4s, v0.4s, v17.4s\n"
        "    fadd v3.4s, v1.4s, v10.4s\n"
        "    fsub v0.4s, v0.4s, v17.4s\n"
        "    fsub v1.4s, v1.4s, v10.4s\n"
        "    fadd v4.4s, v6.4s, v18.4s\n"
        "    fadd v5.4s, v8.4s, v12.4s\n"
        "    fsub v6.4s, v6.4s, v18.4s\n"
        "    fsub v7.4s, v8.4s, v12.4s\n"
        "    fadd v8.4s, v2.4s, v4.4s\n"
        "    fadd v9.4s, v3.4s, v5.4s\n"
        "    fsub v2.4s, v2.4s, v4.4s\n"
        "    fsub v3.4s, v3.4s, v5.4s\n"
        "    fadd v4.4s, v0.4s, v7.4s\n"
        "    fsub v5.4s, v1.4s, v6.4s\n"
        "    fsub v0.4s, v0.4s, v7.4s\n"
        "    fadd v1.4s, v1.4s, v6.4s\n"
        "    zip1 v6.4s, v8.4s, v9.4s\n"
        "    zip2 v7.4s, v8.4s, v9.4s\n"
        "    zip1 v8.4s, v4.4s, v5.4s\n"
        "    zip2 v4.4s, v4.4s, v5.4s\n"
        "    zip1 v5.4s, v2.4s, v3.4s\n"
        "    zip2 v2.4s, v2.4s, v3.4s\n"
        "    stp q6, q7, [%[out]]\n"
        "    zip1 v3.4s, v0.4s, v1.4s\n"
        "    zip2 v0.4s, v0.4s, v1.4s\n"
        "    stp q8, q4, [%[out], #32]\n"
        "    stp q5, q2, [%[out], #64]\n"
        "    stp q3, q0, [%[out], #96]\n"
        : [in] "+r"(in), [out] "+r"(out), [consts] "+r"(consts)
        :
        : "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", "v10", "v11", "v12", "v13", "v14", "v15", "v16", "v17", "v18", "cc", "memory");
}

static const float onefft_neon_n32_consts[48] ONEFFT_ALIGNED(64) = {
    0x1.0000000000000p+0f, 0x1.f6297cff75cb0p-1f, 0x1.d906bcf328d46p-1f, 0x1.a9b66290ea1a3p-1f,
    0x0.0p+0f, -0x1.8f8b83c69a60ap-3f, -0x1.87de2a6aea963p-2f, -0x1.1c73b39ae68c8p-1f,
    0x1.0000000000000p+0f, 0x1.d906bcf328d46p-1f, 0x1.6a09e667f3bcdp-1f, 0x1.87de2a6aea964p-2f,
    0x0.0p+0f, -0x1.87de2a6aea963p-2f, -0x1.6a09e667f3bccp-1f, -0x1.d906bcf328d46p-1f,
    0x1.0000000000000p+0f, 0x1.a9b66290ea1a3p-1f, 0x1.87de2a6aea964p-2f, -0x1.8f8b83c69a608p-3f,
    0x0.0p+0f, -0x1.1c73b39ae68c8p-1f, -0x1.d906bcf328d46p-1f, -0x1.f6297cff75cb0p-1f,
    0x1.6a09e667f3bcdp-1f, 0x1.1c73b39ae68c9p-1f, 0x1.87de2a6aea964p-2f, 0x1.8f8b83c69a60dp-3f,
    -0x1.6a09e667f3bccp-1f, -0x1.a9b66290ea1a3p-1f, -0x1.d906bcf328d46p-1f, -0x1.f6297cff75cb0p-1f,
    -0x0.0p+0f, -0x1.87de2a6aea962p-2f, -0x1.6a09e667f3bccp-1f, -0x1.d906bcf328d46p-1f,
    -0x1.0000000000000p+0f, -0x1.d906bcf328d46p-1f, -0x1.6a09e667f3bcdp-1f, -0x1.87de2a6aea965p-2f,
    -0x1.6a09e667f3bccp-1f, -0x1.f6297cff75cb0p-1f, -0x1.d906bcf328d47p-1f, -0x1.1c73b39ae68c8p-1f,
    -0x1.6a09e667f3bcdp-1f, -0x1.8f8b83c69a617p-3f, 0x1.87de2a6aea961p-2f, 0x1.a9b66290ea1a3p-1f,
};
/* Complete 32 point transform from `in` to `out`, both interleaved. */
static void onefft_neon_n32(const float *in, float *out) {
    size_t bits;
    const float *consts = onefft_neon_n32_consts;
    __asm__ volatile(
        "    ldp q0, q1, [%[in], #32]\n"
        "    ldp q2, q3, [%[in], #96]\n"
        "    ldp q4, q5, [%[in], #160]\n"
        "    ldp q6, q7, [%[in], #224]\n"
        "    ldp q8, q9, [%[in]]\n"
        "    ldp q10, q11, [%[in], #64]\n"
        "    ldp q12, q13, [%[in], #128]\n"
        "    ldp q14, q15, [%[in], #192]\n"
        "    mov %w[bits], #1267\n"
        "    movk %w[bits], #16181, lsl #16\n"
        "    dup v16.4s, %w[bits]\n"
        "    uzp1 v17.4s, v0.4s, v1.4s\n"
        "    uzp2 v0.4s, v0.4s, v1.4s\n"
        "    uzp1 v1.4s, v2.4s, v3.4s\n"
        "    uzp2 v2.4s, v2.4s, v3.4s\n"
        "    uzp1 v3.4s, v4.4s, v5.4s\n"
        "    uzp2 v4.4s, v4.4s, v5.4s\n"
        "    uzp1 v5.4s, v6.4s, v7.4s\n"
        "    uzp2 v6.4s, v6.4s, v7.4s\n"
        "    fsub v7.4s, v17.4s, v3.4s\n"
        "    fsub v18.4s, v0.4s, v4.4s\n"
        "    fsub v19.4s, v1.4s, v5.4s\n"
        "    fsub v20.4s, v2.4s, v6.4s\n"
        "    uzp1 v21.4s, v8.4s, v9.4s\n"
        "    uzp2 v8.4s, v8.4s, v9.4s\n"
        "    uzp1 v9.4s, v10.4s, v11.4s\n"
        "    uzp2 v10.4s, v10.4s, v11.4s\n"
        "    fadd v11.4s, v7.4s, v20.4s\n"
        "    fsub v22.4s, v18.4s, v19.4s\n"
        "    fsub v7.4s, v7.4s, v20.4s\n"
        "    fadd v18.4s, v18.4s, v19.4s\n"
        "    uzp1 v19.4s, v12.4s, v13.4s\n"
        "    uzp2 v12.4s, v12.4s, v13.4s\n"
        "    uzp1 v13.4s, v14.4s, v15.4s\n"
        "    uzp2 v14.4s, v14.4s, v15.4s\n"
        "    fadd v15.4s, v21.4s, v19.4s\n"
        "    fadd v20.4s, v8.4s, v12.4s\n"
        "    fsub v19.4s, v21.4s, v19.4s\n"
        "    fsub v8.4s, v8.4s, v12.4s\n"
        "    fadd v12.4s, v9.4s, v13.4s\n"
        "    fadd v21.4s, v10.4s, v14.4s\n"
        "    fsub v9.4s, v9.4s, v13.4s\n"
        "    fsub v10.4s, v10.4s, v14.4s\n"
        "    fadd v3.4s, v17.4s, v3.4s\n"
        "    fadd v0.4s, v0.4s, v4.4s\n"
        "    ldp q4, q13, [%[consts]]\n"
        "    fadd v1.4s, v1.4s, v5.4s\n"
        "    fadd v2.4s, v2.4s, v6.4s\n"
        "    fadd v5.4s, v11.4s, v22.4s\n"
        "    fsub v6.4s, v22.4s, v11.4s\n"
        "    ldp q11, q14, [%[consts], #32]\n"
        "    fadd v17.4s, v7.4s, v18.4s\n"
        "    fsub v7.4s, v18.4s, v7.4s\n"
        "    fadd v18.4s, v15.4s, v12.4s\n"
        "    fadd v22.4s, v20.4s, v21.4s\n"
        "    fsub v12.4s, v15.4s, v12.4s\n"
        "    fsub v15.4s, v20.4s, v21.4s\n"
        "    ldp q20, q21, [%[consts], #64]\n"
        "    fadd v23.4s, v19.4s, v10.4s\n"
        "    fsub v24.4s, v8.4s, v9.4s\n"
        "    fsub v10.4s, v19.4s, v10.4s\n"
        "    fadd v8.4s, v8.4s, v9.4s\n"
        "    fadd v9.4s, v3.4s, v1.4s\n"
        "    fadd v19.4s, v0.4s, v2.4s\n"
        "    fsub v1.4s, v3.4s, v1.4s\n"
        "    fsub v0.4s, v0.4s, v2.4s\n"
        "    fmul v2.4s, v5.4s, v16.4s\n"
        "    fmul v3.4s, v6.4s, v16.4s\n"
        "    fmul v5.4s, v17.4s, v16.4s\n"
        "    fmul v6.4s, v7.4s, v16.4s\n"
        "    fadd v7.4s, v18.4s, v9.4s\n"
        "    fadd v16.4s, v22.4s, v19.4s\n"
        "    fsub v9.4s, v18.4s, v9.4s\n"
        "    fsub v17.4s, v22.4s, v19.4s\n"
        "    fadd v18.4s, v23.4s, v2.4s\n"
        "    fadd v19.4s, v24.4s, v3.4s\n"
        "    fsub v2.4s, v23.4s, v2.4s\n"
        "    fsub v3.4s, v24.4s, v3.4s\n"
        "    fadd v22.4s, v12.4s, v0.4s\n"
        "    fsub v23.4s, v15.4s, v1.4s\n"
        "    fsub v0.4s, v12.4s, v0.4s\n"
        "    fadd v1.4s, v15.4s, v1.4s\n"
        "    fadd v12.4s, v10.4s, v6.4s\n"
        "    fsub v15.4s, v8.4s, v5.4s\n"
        "    fsub v6.4s, v10.4s, v6.4s\n"
        "    fadd v5.4s, v8.4s, v5.4s\n"
        "    trn1 v8.4s, v7.4s, v18.4s\n"
        "    trn2 v7.4s, v7.4s, v18.4s\n"
        "    trn1 v10.4s, v22.4s, v12.4s\n"
        "    trn2 v12.4s, v22.4s, v12.4s\n"
        "    trn1 v18.4s, v16.4s, v19.4s\n"
        "    trn2 v16.4s, v16.4s, v19.4s\n"
        "    trn1 v19.4s, v23.4s, v15.4s\n"
        "    trn2 v15.4s, v23.4s, v15.4s\n"
        "    trn1 v22.4s, v9.4s, v2.4s\n"
        "    trn2 v2.4s, v9.4s, v2.4s\n"
        "    trn1 v9.4s, v0.4s, v6.4s\n"
        "    trn2 v0.4s, v0.4s, v6.4s\n"
        "    trn1 v6.4s, v17.4s, v3.4s\n"
        "    trn2 v3.4s, v17.4s, v3.4s\n"
        "    zip1 v17.2d, v7.2d, v12.2d\n"
        "    zip2 v23.2d, v8.2d, v10.2d\n"
        "    zip2 v7.2d, v7.2d, v12.2d\n"
        "    zip1 v12.2d, v16.2d, v15.2d\n"
        "    zip2 v24.2d, v18.2d, v19.2d\n"
        "    zip2 v15.2d, v16.2d, v15.2d\n"
        "    zip1 v16.2d, v2.2d, v0.2d\n"
        "    zip2 v25.2d, v22.2d, v9.2d\n"
        "    zip2 v0.2d, v2.2d, v0.2d\n"
        "    fmul v2.4s, v17.4s, v4.4s\n"
        "    fmul v4.4s, v12.4s, v4.4s\n"
        "    fmul v26.4s, v23.4s, v11.4s\n"
        "    fmul v11.4s, v24.4s, v11.4s\n"
        "    fmul v27.4s, v7.4s, v20.4s\n"
        "    fmul v20.4s, v15.4s, v20.4s\n"
        "    fmls v2.4s, v12.4s, v13.4s\n"
        "    fmla v4.4s, v17.4s, v13.4s\n"
        "    fmls v26.4s, v24.4s, v14.4s\n"
        "    fmla v11.4s, v23.4s, v14.4s\n"
        "    fmls v27.4s, v15.4s, v21.4s\n"
        "    fmla v20.4s, v7.4s, v21.4s\n"
        "    zip1 v7.2d, v8.2d, v10.2d\n"
        "    trn1 v8.4s, v1.4s, v5.4s\n"
        "    trn2 v1.4s, v1.4s, v5.4s\n"
        "    ldp q5, q10, [%[consts], #96]\n"
        "    zip1 v12.2d, v18.2d, v19.2d\n"
        "    zip1 v9.2d, v22.2d, v9.2d\n"
        "    ldp q13, q14, [%[consts], #128]\n"
        "    ldp q15, q17, [%[consts], #160]\n"
        "    zip1 v18.2d, v3.2d, v1.2d\n"
        "    zip2 v19.2d, v6.2d, v8.2d\n"
        "    zip2 v1.2d, v3.2d, v1.2d\n"
        "    zip1 v3.2d, v6.2d, v8.2d\n"
        "    fmul v6.4s, v16.4s, v5.4s\n"
        "    fmul v5.4s, v18.4s, v5.4s\n"
        "    fadd v8.4s, v7.4s, v26.4s\n"
        "    fadd v21.4s, v12.4s, v11.4s\n"
        "    fmul v22.4s, v25.4s, v13.4s\n"
        "    fmul v13.4s, v19.4s, v13.4s\n"
        "    fmul v23.4s, v0.4s, v15.4s\n"
        "    fmul v15.4s, v1.4s, v15.4s\n"
        "    fmls v6.4s, v18.4s, v10.4s\n"
        "    fmla v5.4s, v16.4s, v10.4s\n"
        "    fmls v22.4s, v19.4s, v14.4s\n"
        "    fmla v13.4s, v25.4s, v14.4s\n"
        "    fmls v23.4s, v1.4s, v17.4s\n"
        "    fmla v15.4s, v0.4s, v17.4s\n"
        "    fsub v0.4s, v7.4s, v26.4s\n"
        "    fsub v1.4s, v12.4s, v11.4s\n"
        "    fadd v7.4s, v2.4s, v27.4s\n"
        "    fadd v10.4s, v4.4s, v20.4s\n"
        "    fsub v2.4s, v2.4s, v27.4s\n"
        "    fsub v4.4s, v4.4s, v20.4s\n"
        "    fadd v11.4s, v9.4s, v22.4s\n"
        "    fadd v12.4s, v3.4s, v13.4s\n"
        "    fsub v9.4s, v9.4s, v22.4s\n"
        "    fsub v3.4s, v3.4s, v13.4s\n"
        "    fadd v13.4s, v6.4s, v23.4s\n"
        "    fadd v14.4s, v5.4s, v15.4s\n"
        "    fsub v6.4s, v6.4s, v23.4s\n"
        "    fsub v5.4s, v5.4s, v15.4s\n"
        "    fadd v15.4s, v8.4s, v7.4s\n"
        "    fadd v16.4s, v21.4s, v10.4s\n"
        "    fsub v7.4s, v8.4s, v7.4s\n"
        "    fsub v8.4s, v21.4s, v10.4s\n"
        "    fadd v10.4s, v0.4s, v4.4s\n"
        "    fsub v17.4s, v1.4s, v2.4s\n"
        "    fsub v0.4s, v0.4s, v4.4s\n"
        "    fadd v1.4s, v1.4s, v2.4s\n"
        "    fadd v2.4s, v11.4s, v13.4s\n"
        "    fadd v4.4s, v12.4s, v14.4s\n"
        "    fsub v11.4s, v11.4s, v13.4s\n"
        "    fsub v12.4s, v12.4s, v14.4s\n"
        "    fadd v13.4s, v9.4s, v5.4s\n"
        "    fsub v14.4s, v3.4s, v6.4s\n"
        "    fsub v5.4s, v9.4s, v5.4s\n"
        "    fadd v3.4s, v3.4s, v6.4s\n"
        "    zip1 v6.4s, v15.4s, v16.4s\n"
        "    zip2 v9.4s, v15.4s, v16.4s\n"
        "    zip1 v15.4s, v10.4s, v17.4s\n"
        "    zip2 v10.4s, v10.4s, v17.4s\n"
        "    zip1 v16.4s, v7.4s, v8.4s\n"
        "    zip2 v7.4s, v7.4s, v8.4s\n"
        "    stp q6, q9, [%[out]]\n"
        "    zip1 v6.4s, v0.4s, v1.4s\n"
        "    zip2 v0.4s, v0.4s, v1.4s\n"
        "    stp q15, q10, [%[out], #64]\n"
        "    zip1 v1.4s, v2.4s, v4.4s\n"
        "    zip2 v2.4s, v2.4s, v4.4s\n"
        "    stp q16, q7, [%[out], #128]\n"
        "    zip1 v4.4s, v13.4s, v14.4s\n"
        "    zip2 v7.4s, v13.4s, v14.4s\n"
        "    stp q6, q0, [%[out], #192]\n"
        "    zip1 v0.4s, v11.4s, v12.4s\n"
        "    zip2 v6.4s, v11.4s, v12.4s\n"
        "    stp q1, q2, [%[out], #32]\n"
        "    zip1 v1.4s, v5.4s, v3.4s\n"
        "    zip2 v2.4s, v5.4s, v3.4s\n"
        "    stp q4, q7, [%[out], #96]\n"
        "    stp q0, q6, [%[out], #160]\n"
        "    stp q1, q2, [%[out], #224]\n"
        : [in] "+r"(in), [out] "+r"(out), [bits] "=&r"(bits), [consts] "+r"(consts)
        :
        : "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", "v10", "v11", "v12", "v13", "v14", "v15", "v16", "v17", "v18", "v19", "v20", "v21", "v22", "v23", "v24", "v25", "v26", "v27", "cc", "memory");
}

/* Last pass, radix 8, in place, from fused multiply-add butterflies: reads split vectors and writes the interleaved result.
 * Butterfly j < count uses the vectors at data + u*stride + 32*j. The last parameter is not used: it
 * exists because the last pass kernels of all targets share a signature. See the table builder in
 * onefft.c for the twiddle layout. */
static void onefft_neon_l8_fma(float *data, const float *tw, size_t stride, size_t count, size_t regions) {
    (void)regions;
    size_t j;
    float *p1;
    float *p2;
    float *p3;
    float *p4;
    float *p5;
    float *p6;
    float *p7;
    __asm__ volatile(
        "    add %[p1], %[data], %[stride]\n"
        "    lsr %[j], %[count], #1\n"
        "    add %[p2], %[p1], %[stride]\n"
        "    add %[p3], %[p2], %[stride]\n"
        "    add %[p4], %[p3], %[stride]\n"
        "    add %[p5], %[p4], %[stride]\n"
        "    add %[p6], %[p5], %[stride]\n"
        "    add %[p7], %[p6], %[stride]\n"
        "1:\n"
        "    ldp q0, q1, [%[p1]]\n"
        "    ldp q2, q3, [%[p3]]\n"
        "    ldp q4, q5, [%[p5]]\n"
        "    ldp q6, q7, [%[p7]]\n"
        "    ldp q9, q8, [%[tw]]\n"
        "    ldp q10, q11, [%[data]]\n"
        "    ldp q12, q13, [%[p2]]\n"
        "    ldp q14, q15, [%[p4]]\n"
        "    ldp q16, q17, [%[p6]]\n"
        "    ldp q19, q18, [%[tw], #32]\n"
        "    mov v20.16b, v0.16b\n"
        "    mov v21.16b, v2.16b\n"
        "    mov v22.16b, v4.16b\n"
        "    mov v23.16b, v6.16b\n"
        "    mov v24.16b, v10.16b\n"
        "    mov v25.16b, v11.16b\n"
        "    mov v26.16b, v12.16b\n"
        "    mov v27.16b, v13.16b\n"
        "    mov v28.16b, v14.16b\n"
        "    mov v29.16b, v15.16b\n"
        "    mov v30.16b, v16.16b\n"
        "    mov v31.16b, v17.16b\n"
        "    fmls v0.4s, v1.4s, v8.s[0]\n"
        "    fmls v2.4s, v3.4s, v8.s[1]\n"
        "    fmla v1.4s, v20.4s, v8.s[0]\n"
        "    fmla v3.4s, v21.4s, v8.s[1]\n"
        "    fmls v4.4s, v5.4s, v8.s[2]\n"
        "    fmls v6.4s, v7.4s, v8.s[3]\n"
        "    ldp q21, q20, [%[tw], #64]\n"
        "    fmla v5.4s, v22.4s, v8.s[2]\n"
        "    fmla v7.4s, v23.4s, v8.s[3]\n"
        "    fmla v10.4s, v0.4s, v9.s[0]\n"
        "    fmla v11.4s, v1.4s, v9.s[0]\n"
        "    fmls v24.4s, v0.4s, v9.s[0]\n"
        "    fmls v25.4s, v1.4s, v9.s[0]\n"
        "    fmla v12.4s, v2.4s, v9.s[1]\n"
        "    fmla v13.4s, v3.4s, v9.s[1]\n"
        "    fmls v26.4s, v2.4s, v9.s[1]\n"
        "    fmls v27.4s, v3.4s, v9.s[1]\n"
        "    fmla v14.4s, v4.4s, v9.s[2]\n"
        "    fmla v15.4s, v5.4s, v9.s[2]\n"
        "    fmls v28.4s, v4.4s, v9.s[2]\n"
        "    fmls v29.4s, v5.4s, v9.s[2]\n"
        "    fmla v16.4s, v6.4s, v9.s[3]\n"
        "    fmla v17.4s, v7.4s, v9.s[3]\n"
        "    fmls v30.4s, v6.4s, v9.s[3]\n"
        "    fmls v31.4s, v7.4s, v9.s[3]\n"
        "    trn2 v0.4s, v10.4s, v12.4s\n"
        "    trn2 v1.4s, v11.4s, v13.4s\n"
        "    trn2 v2.4s, v14.4s, v16.4s\n"
        "    trn2 v3.4s, v15.4s, v17.4s\n"
        "    trn1 v4.4s, v10.4s, v12.4s\n"
        "    trn1 v5.4s, v14.4s, v16.4s\n"
        "    zip2 v6.2d, v0.2d, v2.2d\n"
        "    zip2 v7.2d, v1.2d, v3.2d\n"
        "    trn1 v8.4s, v11.4s, v13.4s\n"
        "    trn1 v9.4s, v15.4s, v17.4s\n"
        "    mov v10.16b, v6.16b\n"
        "    zip1 v0.2d, v0.2d, v2.2d\n"
        "    zip2 v2.2d, v4.2d, v5.2d\n"
        "    fmls v6.4s, v7.4s, v18.4s\n"
        "    zip1 v1.2d, v1.2d, v3.2d\n"
        "    fmla v7.4s, v10.4s, v18.4s\n"
        "    zip2 v3.2d, v8.2d, v9.2d\n"
        "    mov v10.16b, v2.16b\n"
        "    mov v11.16b, v0.16b\n"
        "    mov v12.16b, v1.16b\n"
        "    zip1 v4.2d, v4.2d, v5.2d\n"
        "    zip1 v5.2d, v8.2d, v9.2d\n"
        "    trn2 v8.4s, v24.4s, v26.4s\n"
        "    trn1 v9.4s, v24.4s, v26.4s\n"
        "    fmls v2.4s, v3.4s, v18.4s\n"
        "    fmla v0.4s, v6.4s, v19.4s\n"
        "    mov v13.16b, v4.16b\n"
        "    mov v14.16b, v5.16b\n"
        "    trn1 v15.4s, v28.4s, v30.4s\n"
        "    fmla v3.4s, v10.4s, v18.4s\n"
        "    trn2 v10.4s, v28.4s, v30.4s\n"
        "    fmla v1.4s, v7.4s, v19.4s\n"
        "    fmls v11.4s, v6.4s, v19.4s\n"
        "    fmls v12.4s, v7.4s, v19.4s\n"
        "    mov v6.16b, v0.16b\n"
        "    zip1 v7.2d, v8.2d, v10.2d\n"
        "    zip1 v16.2d, v9.2d, v15.2d\n"
        "    fmla v4.4s, v2.4s, v19.4s\n"
        "    fmla v5.4s, v3.4s, v19.4s\n"
        "    zip2 v8.2d, v8.2d, v10.2d\n"
        "    zip2 v9.2d, v9.2d, v15.2d\n"
        "    mov v10.16b, v11.16b\n"
        "    fmls v13.4s, v2.4s, v19.4s\n"
        "    fmls v14.4s, v3.4s, v19.4s\n"
        "    trn2 v2.4s, v25.4s, v27.4s\n"
        "    trn2 v3.4s, v29.4s, v31.4s\n"
        "    ldp q17, q15, [%[tw], #96]\n"
        "    mov v22.16b, v9.16b\n"
        "    mov v23.16b, v4.16b\n"
        "    mov v24.16b, v5.16b\n"
        "    trn1 v25.4s, v25.4s, v27.4s\n"
        "    trn1 v26.4s, v29.4s, v31.4s\n"
        "    zip2 v27.2d, v2.2d, v3.2d\n"
        "    zip1 v2.2d, v2.2d, v3.2d\n"
        "    mov v3.16b, v13.16b\n"
        "    mov v28.16b, v14.16b\n"
        "    zip2 v29.2d, v25.2d, v26.2d\n"
        "    zip1 v25.2d, v25.2d, v26.2d\n"
        "    fmls v0.4s, v1.4s, v20.4s\n"
        "    fmls v11.4s, v12.4s, v20.4s\n"
        "    fmls v9.4s, v29.4s, v18.4s\n"
        "    fmla v1.4s, v6.4s, v20.4s\n"
        "    mov v6.16b, v16.16b\n"
        "    fmla v29.4s, v22.4s, v18.4s\n"
        "    fmla v12.4s, v10.4s, v20.4s\n"
        "    mov v10.16b, v25.16b\n"
        "    fmla v4.4s, v0.4s, v21.4s\n"
        "    fmls v23.4s, v0.4s, v21.4s\n"
        "    fmls v25.4s, v9.4s, v19.4s\n"
        "    fmla v5.4s, v1.4s, v21.4s\n"
        "    mov v0.16b, v8.16b\n"
        "    fmla v16.4s, v29.4s, v19.4s\n"
        "    fmls v6.4s, v29.4s, v19.4s\n"
        "    fmla v10.4s, v9.4s, v19.4s\n"
        "    fmls v24.4s, v1.4s, v21.4s\n"
        "    fmls v8.4s, v27.4s, v18.4s\n"
        "    mov v1.16b, v7.16b\n"
        "    mov v9.16b, v2.16b\n"
        "    fmla v13.4s, v12.4s, v21.4s\n"
        "    fmla v27.4s, v0.4s, v18.4s\n"
        "    fmls v14.4s, v11.4s, v21.4s\n"
        "    fmls v3.4s, v12.4s, v21.4s\n"
        "    fmla v28.4s, v11.4s, v21.4s\n"
        "    fmls v2.4s, v8.4s, v19.4s\n"
        "    fmla v9.4s, v8.4s, v19.4s\n"
        "    fmla v7.4s, v27.4s, v19.4s\n"
        "    fmls v1.4s, v27.4s, v19.4s\n"
        "    zip1 v0.4s, v4.4s, v5.4s\n"
        "    zip2 v4.4s, v4.4s, v5.4s\n"
        "    mov v5.16b, v16.16b\n"
        "    zip1 v8.4s, v13.4s, v14.4s\n"
        "    zip2 v11.4s, v13.4s, v14.4s\n"
        "    mov v12.16b, v7.16b\n"
        "    zip1 v13.4s, v23.4s, v24.4s\n"
        "    zip2 v14.4s, v23.4s, v24.4s\n"
        "    stp q0, q4, [%[data]]\n"
        "    fmls v7.4s, v2.4s, v15.4s\n"
        "    mov v0.16b, v25.16b\n"
        "    zip1 v4.4s, v3.4s, v28.4s\n"
        "    stp q8, q11, [%[p2]]\n"
        "    fmla v2.4s, v12.4s, v15.4s\n"
        "    mov v8.16b, v1.16b\n"
        "    zip2 v3.4s, v3.4s, v28.4s\n"
        "    stp q13, q14, [%[p4]]\n"
        "    fmls v1.4s, v9.4s, v15.4s\n"
        "    mov v11.16b, v6.16b\n"
        "    fmla v9.4s, v8.4s, v15.4s\n"
        "    fmla v16.4s, v7.4s, v17.4s\n"
        "    stp q4, q3, [%[p6]]\n"
        "    fmla v25.4s, v2.4s, v17.4s\n"
        "    fmls v5.4s, v7.4s, v17.4s\n"
        "    mov v3.16b, v10.16b\n"
        "    add %[data], %[data], #32\n"
        "    add %[p2], %[p2], #32\n"
        "    add %[p4], %[p4], #32\n"
        "    add %[p6], %[p6], #32\n"
        "    add %[tw], %[tw], #128\n"
        "    subs %[j], %[j], #1\n"
        "    fmls v0.4s, v2.4s, v17.4s\n"
        "    fmls v10.4s, v1.4s, v17.4s\n"
        "    fmla v6.4s, v9.4s, v17.4s\n"
        "    fmls v11.4s, v9.4s, v17.4s\n"
        "    fmla v3.4s, v1.4s, v17.4s\n"
        "    zip1 v1.4s, v16.4s, v25.4s\n"
        "    zip2 v2.4s, v16.4s, v25.4s\n"
        "    zip1 v4.4s, v5.4s, v0.4s\n"
        "    zip1 v7.4s, v6.4s, v10.4s\n"
        "    zip2 v6.4s, v6.4s, v10.4s\n"
        "    zip2 v0.4s, v5.4s, v0.4s\n"
        "    zip1 v5.4s, v11.4s, v3.4s\n"
        "    stp q1, q2, [%[p1]]\n"
        "    zip2 v1.4s, v11.4s, v3.4s\n"
        "    stp q7, q6, [%[p3]]\n"
        "    add %[p1], %[p1], #32\n"
        "    stp q4, q0, [%[p5]]\n"
        "    add %[p3], %[p3], #32\n"
        "    stp q5, q1, [%[p7]]\n"
        "    add %[p5], %[p5], #32\n"
        "    add %[p7], %[p7], #32\n"
        "    b.ne 1b\n"
        "    lsr %[j], %[count], #1\n"
        "2:\n"
        "    ldp q1, q0, [%[p1]]\n"
        "    ldp q3, q2, [%[p3]]\n"
        "    ldp q5, q4, [%[p5]]\n"
        "    ldp q7, q6, [%[p7]]\n"
        "    ldp q9, q8, [%[tw]]\n"
        "    ldp q10, q11, [%[data]]\n"
        "    ldp q12, q13, [%[p2]]\n"
        "    ldp q14, q15, [%[p4]]\n"
        "    ldp q16, q17, [%[p6]]\n"
        "    ldp q19, q18, [%[tw], #32]\n"
        "    mov v20.16b, v1.16b\n"
        "    mov v21.16b, v3.16b\n"
        "    mov v22.16b, v5.16b\n"
        "    mov v23.16b, v7.16b\n"
        "    mov v24.16b, v10.16b\n"
        "    mov v25.16b, v11.16b\n"
        "    mov v26.16b, v12.16b\n"
        "    mov v27.16b, v13.16b\n"
        "    mov v28.16b, v14.16b\n"
        "    mov v29.16b, v15.16b\n"
        "    mov v30.16b, v16.16b\n"
        "    mov v31.16b, v17.16b\n"
        "    fmls v1.4s, v0.4s, v8.s[0]\n"
        "    fmls v3.4s, v2.4s, v8.s[1]\n"
        "    fmla v0.4s, v20.4s, v8.s[0]\n"
        "    fmla v2.4s, v21.4s, v8.s[1]\n"
        "    fmls v5.4s, v4.4s, v8.s[2]\n"
        "    fmls v7.4s, v6.4s, v8.s[3]\n"
        "    ldp q21, q20, [%[tw], #64]\n"
        "    fmla v4.4s, v22.4s, v8.s[2]\n"
        "    fmla v6.4s, v23.4s, v8.s[3]\n"
        "    fmla v10.4s, v0.4s, v9.s[0]\n"
        "    fmls v11.4s, v1.4s, v9.s[0]\n"
        "    fmls v24.4s, v0.4s, v9.s[0]\n"
        "    fmla v25.4s, v1.4s, v9.s[0]\n"
        "    fmla v12.4s, v2.4s, v9.s[1]\n"
        "    fmls v13.4s, v3.4s, v9.s[1]\n"
        "    fmls v26.4s, v2.4s, v9.s[1]\n"
        "    fmla v27.4s, v3.4s, v9.s[1]\n"
        "    fmla v14.4s, v4.4s, v9.s[2]\n"
        "    fmls v15.4s, v5.4s, v9.s[2]\n"
        "    fmls v28.4s, v4.4s, v9.s[2]\n"
        "    fmla v29.4s, v5.4s, v9.s[2]\n"
        "    fmla v16.4s, v6.4s, v9.s[3]\n"
        "    fmls v17.4s, v7.4s, v9.s[3]\n"
        "    fmls v30.4s, v6.4s, v9.s[3]\n"
        "    fmla v31.4s, v7.4s, v9.s[3]\n"
        "    trn2 v0.4s, v10.4s, v12.4s\n"
        "    trn2 v1.4s, v11.4s, v13.4s\n"
        "    trn2 v2.4s, v14.4s, v16.4s\n"
        "    trn2 v3.4s, v15.4s, v17.4s\n"
        "    trn1 v4.4s, v10.4s, v12.4s\n"
        "    trn1 v5.4s, v14.4s, v16.4s\n"
        "    zip2 v6.2d, v0.2d, v2.2d\n"
        "    zip2 v7.2d, v1.2d, v3.2d\n"
        "    trn1 v8.4s, v11.4s, v13.4s\n"
        "    trn1 v9.4s, v15.4s, v17.4s\n"
        "    mov v10.16b, v6.16b\n"
        "    zip1 v0.2d, v0.2d, v2.2d\n"
        "    zip2 v2.2d, v4.2d, v5.2d\n"
        "    fmls v6.4s, v7.4s, v18.4s\n"
        "    zip1 v1.2d, v1.2d, v3.2d\n"
        "    fmla v7.4s, v10.4s, v18.4s\n"
        "    zip2 v3.2d, v8.2d, v9.2d\n"
        "    mov v10.16b, v2.16b\n"
        "    mov v11.16b, v0.16b\n"
        "    mov v12.16b, v1.16b\n"
        "    zip1 v4.2d, v4.2d, v5.2d\n"
        "    zip1 v5.2d, v8.2d, v9.2d\n"
        "    trn2 v8.4s, v24.4s, v26.4s\n"
        "    trn1 v9.4s, v24.4s, v26.4s\n"
        "    fmls v2.4s, v3.4s, v18.4s\n"
        "    fmla v0.4s, v6.4s, v19.4s\n"
        "    mov v13.16b, v4.16b\n"
        "    mov v14.16b, v5.16b\n"
        "    trn1 v15.4s, v28.4s, v30.4s\n"
        "    fmla v3.4s, v10.4s, v18.4s\n"
        "    trn2 v10.4s, v28.4s, v30.4s\n"
        "    fmla v1.4s, v7.4s, v19.4s\n"
        "    fmls v11.4s, v6.4s, v19.4s\n"
        "    fmls v12.4s, v7.4s, v19.4s\n"
        "    mov v6.16b, v0.16b\n"
        "    zip1 v7.2d, v8.2d, v10.2d\n"
        "    zip1 v16.2d, v9.2d, v15.2d\n"
        "    fmla v4.4s, v2.4s, v19.4s\n"
        "    fmla v5.4s, v3.4s, v19.4s\n"
        "    zip2 v8.2d, v8.2d, v10.2d\n"
        "    zip2 v9.2d, v9.2d, v15.2d\n"
        "    mov v10.16b, v11.16b\n"
        "    fmls v13.4s, v2.4s, v19.4s\n"
        "    fmls v14.4s, v3.4s, v19.4s\n"
        "    trn2 v2.4s, v25.4s, v27.4s\n"
        "    trn2 v3.4s, v29.4s, v31.4s\n"
        "    ldp q17, q15, [%[tw], #96]\n"
        "    mov v22.16b, v9.16b\n"
        "    mov v23.16b, v4.16b\n"
        "    mov v24.16b, v5.16b\n"
        "    trn1 v25.4s, v25.4s, v27.4s\n"
        "    trn1 v26.4s, v29.4s, v31.4s\n"
        "    zip2 v27.2d, v2.2d, v3.2d\n"
        "    zip1 v2.2d, v2.2d, v3.2d\n"
        "    mov v3.16b, v13.16b\n"
        "    mov v28.16b, v14.16b\n"
        "    zip2 v29.2d, v25.2d, v26.2d\n"
        "    zip1 v25.2d, v25.2d, v26.2d\n"
        "    fmls v0.4s, v1.4s, v20.4s\n"
        "    fmls v11.4s, v12.4s, v20.4s\n"
        "    fmls v9.4s, v29.4s, v18.4s\n"
        "    fmla v1.4s, v6.4s, v20.4s\n"
        "    mov v6.16b, v16.16b\n"
        "    fmla v29.4s, v22.4s, v18.4s\n"
        "    fmla v12.4s, v10.4s, v20.4s\n"
        "    mov v10.16b, v25.16b\n"
        "    fmla v4.4s, v0.4s, v21.4s\n"
        "    fmls v23.4s, v0.4s, v21.4s\n"
        "    fmls v25.4s, v9.4s, v19.4s\n"
        "    fmla v5.4s, v1.4s, v21.4s\n"
        "    mov v0.16b, v8.16b\n"
        "    fmla v16.4s, v29.4s, v19.4s\n"
        "    fmls v6.4s, v29.4s, v19.4s\n"
        "    fmla v10.4s, v9.4s, v19.4s\n"
        "    fmls v24.4s, v1.4s, v21.4s\n"
        "    fmls v8.4s, v27.4s, v18.4s\n"
        "    mov v1.16b, v7.16b\n"
        "    mov v9.16b, v2.16b\n"
        "    fmla v13.4s, v12.4s, v21.4s\n"
        "    fmla v27.4s, v0.4s, v18.4s\n"
        "    fmls v14.4s, v11.4s, v21.4s\n"
        "    fmls v3.4s, v12.4s, v21.4s\n"
        "    fmla v28.4s, v11.4s, v21.4s\n"
        "    fmls v2.4s, v8.4s, v19.4s\n"
        "    fmla v9.4s, v8.4s, v19.4s\n"
        "    fmla v7.4s, v27.4s, v19.4s\n"
        "    fmls v1.4s, v27.4s, v19.4s\n"
        "    zip1 v0.4s, v4.4s, v5.4s\n"
        "    zip2 v4.4s, v4.4s, v5.4s\n"
        "    mov v5.16b, v16.16b\n"
        "    zip1 v8.4s, v13.4s, v14.4s\n"
        "    zip2 v11.4s, v13.4s, v14.4s\n"
        "    mov v12.16b, v7.16b\n"
        "    zip1 v13.4s, v23.4s, v24.4s\n"
        "    zip2 v14.4s, v23.4s, v24.4s\n"
        "    stp q0, q4, [%[data]]\n"
        "    fmls v7.4s, v2.4s, v15.4s\n"
        "    mov v0.16b, v25.16b\n"
        "    zip1 v4.4s, v3.4s, v28.4s\n"
        "    stp q8, q11, [%[p2]]\n"
        "    fmla v2.4s, v12.4s, v15.4s\n"
        "    mov v8.16b, v1.16b\n"
        "    zip2 v3.4s, v3.4s, v28.4s\n"
        "    stp q13, q14, [%[p4]]\n"
        "    fmls v1.4s, v9.4s, v15.4s\n"
        "    mov v11.16b, v6.16b\n"
        "    fmla v9.4s, v8.4s, v15.4s\n"
        "    fmla v16.4s, v7.4s, v17.4s\n"
        "    stp q4, q3, [%[p6]]\n"
        "    fmla v25.4s, v2.4s, v17.4s\n"
        "    fmls v5.4s, v7.4s, v17.4s\n"
        "    mov v3.16b, v10.16b\n"
        "    add %[data], %[data], #32\n"
        "    add %[p2], %[p2], #32\n"
        "    add %[p4], %[p4], #32\n"
        "    add %[p6], %[p6], #32\n"
        "    add %[tw], %[tw], #128\n"
        "    subs %[j], %[j], #1\n"
        "    fmls v0.4s, v2.4s, v17.4s\n"
        "    fmls v10.4s, v1.4s, v17.4s\n"
        "    fmla v6.4s, v9.4s, v17.4s\n"
        "    fmls v11.4s, v9.4s, v17.4s\n"
        "    fmla v3.4s, v1.4s, v17.4s\n"
        "    zip1 v1.4s, v16.4s, v25.4s\n"
        "    zip2 v2.4s, v16.4s, v25.4s\n"
        "    zip1 v4.4s, v5.4s, v0.4s\n"
        "    zip1 v7.4s, v6.4s, v10.4s\n"
        "    zip2 v6.4s, v6.4s, v10.4s\n"
        "    zip2 v0.4s, v5.4s, v0.4s\n"
        "    zip1 v5.4s, v11.4s, v3.4s\n"
        "    stp q1, q2, [%[p1]]\n"
        "    zip2 v1.4s, v11.4s, v3.4s\n"
        "    stp q7, q6, [%[p3]]\n"
        "    add %[p1], %[p1], #32\n"
        "    stp q4, q0, [%[p5]]\n"
        "    add %[p3], %[p3], #32\n"
        "    stp q5, q1, [%[p7]]\n"
        "    add %[p5], %[p5], #32\n"
        "    add %[p7], %[p7], #32\n"
        "    b.ne 2b\n"
        : [data] "+r"(data), [tw] "+r"(tw), [stride] "+r"(stride), [count] "+r"(count), [j] "=&r"(j), [p1] "=&r"(p1), [p2] "=&r"(p2), [p3] "=&r"(p3), [p4] "=&r"(p4), [p5] "=&r"(p5), [p6] "=&r"(p6), [p7] "=&r"(p7)
        :
        : "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", "v10", "v11", "v12", "v13", "v14", "v15", "v16", "v17", "v18", "v19", "v20", "v21", "v22", "v23", "v24", "v25", "v26", "v27", "v28", "v29", "v30", "v31", "cc", "memory");
}

/* Last pass, radix 4, in place, from fused multiply-add butterflies: reads split vectors and writes the interleaved result.
 * Butterfly j < count uses the vectors at data + u*stride + 32*j. The last parameter is not used: it
 * exists because the last pass kernels of all targets share a signature. See the table builder in
 * onefft.c for the twiddle layout. */
static void onefft_neon_l4_fma(float *data, const float *tw, size_t stride, size_t count, size_t regions) {
    (void)regions;
    size_t j;
    float *p1;
    float *p2;
    float *p3;
    __asm__ volatile(
        "    add %[p1], %[data], %[stride]\n"
        "    lsr %[j], %[count], #1\n"
        "    add %[p2], %[p1], %[stride]\n"
        "    add %[p3], %[p2], %[stride]\n"
        "1:\n"
        "    ldp q0, q1, [%[data]]\n"
        "    ldp q2, q3, [%[p1]]\n"
        "    ldp q4, q5, [%[p2]]\n"
        "    ldp q6, q7, [%[p3]]\n"
        "    ldp q9, q8, [%[tw]]\n"
        "    ldp q11, q10, [%[tw], #32]\n"
        "    trn2 v12.4s, v0.4s, v2.4s\n"
        "    trn2 v13.4s, v4.4s, v6.4s\n"
        "    trn2 v14.4s, v1.4s, v3.4s\n"
        "    trn2 v15.4s, v5.4s, v7.4s\n"
        "    zip2 v16.2d, v12.2d, v13.2d\n"
        "    trn1 v0.4s, v0.4s, v2.4s\n"
        "    zip2 v2.2d, v14.2d, v15.2d\n"
        "    trn1 v4.4s, v4.4s, v6.4s\n"
        "    trn1 v1.4s, v1.4s, v3.4s\n"
        "    trn1 v3.4s, v5.4s, v7.4s\n"
        "    mov v5.16b, v16.16b\n"
        "    fmls v16.4s, v2.4s, v8.4s\n"
        "    zip1 v6.2d, v12.2d, v13.2d\n"
        "    fmla v2.4s, v5.4s, v8.4s\n"
        "    zip2 v5.2d, v0.2d, v4.2d\n"
        "    zip1 v7.2d, v14.2d, v15.2d\n"
        "    zip2 v12.2d, v1.2d, v3.2d\n"
        "    mov v13.16b, v6.16b\n"
        "    mov v14.16b, v5.16b\n"
        "    fmla v6.4s, v16.4s, v9.4s\n"
        "    fmls v13.4s, v16.4s, v9.4s\n"
        "    fmls v5.4s, v12.4s, v8.4s\n"
        "    mov v15.16b, v7.16b\n"
        "    zip1 v0.2d, v0.2d, v4.2d\n"
        "    fmla v12.4s, v14.4s, v8.4s\n"
        "    fmla v7.4s, v2.4s, v9.4s\n"
        "    add %[tw], %[tw], #64\n"
        "    subs %[j], %[j], #1\n"
        "    fmls v15.4s, v2.4s, v9.4s\n"
        "    zip1 v1.2d, v1.2d, v3.2d\n"
        "    mov v2.16b, v0.16b\n"
        "    mov v3.16b, v6.16b\n"
        "    mov v4.16b, v13.16b\n"
        "    fmla v0.4s, v5.4s, v9.4s\n"
        "    fmls v2.4s, v5.4s, v9.4s\n"
        "    mov v5.16b, v1.16b\n"
        "    fmls v6.4s, v7.4s, v10.4s\n"
        "    fmla v1.4s, v12.4s, v9.4s\n"
        "    fmls v5.4s, v12.4s, v9.4s\n"
        "    fmla v7.4s, v3.4s, v10.4s\n"
        "    fmls v13.4s, v15.4s, v10.4s\n"
        "    mov v3.16b, v0.16b\n"
        "    mov v8.16b, v2.16b\n"
        "    fmla v15.4s, v4.4s, v10.4s\n"
        "    fmla v0.4s, v6.4s, v11.4s\n"
        "    mov v4.16b, v1.16b\n"
        "    fmls v3.4s, v6.4s, v11.4s\n"
        "    mov v6.16b, v5.16b\n"
        "    fmla v1.4s, v7.4s, v11.4s\n"
        "    fmls v4.4s, v7.4s, v11.4s\n"
        "    fmla v2.4s, v15.4s, v11.4s\n"
        "    fmls v5.4s, v13.4s, v11.4s\n"
        "    fmls v8.4s, v15.4s, v11.4s\n"
        "    fmla v6.4s, v13.4s, v11.4s\n"
        "    zip1 v7.4s, v0.4s, v1.4s\n"
        "    zip2 v0.4s, v0.4s, v1.4s\n"
        "    zip1 v1.4s, v2.4s, v5.4s\n"
        "    zip2 v2.4s, v2.4s, v5.4s\n"
        "    zip1 v5.4s, v3.4s, v4.4s\n"
        "    zip2 v3.4s, v3.4s, v4.4s\n"
        "    stp q7, q0, [%[data]]\n"
        "    zip1 v0.4s, v8.4s, v6.4s\n"
        "    zip2 v4.4s, v8.4s, v6.4s\n"
        "    stp q1, q2, [%[p1]]\n"
        "    add %[data], %[data], #32\n"
        "    stp q5, q3, [%[p2]]\n"
        "    add %[p1], %[p1], #32\n"
        "    stp q0, q4, [%[p3]]\n"
        "    add %[p2], %[p2], #32\n"
        "    add %[p3], %[p3], #32\n"
        "    b.ne 1b\n"
        "    lsr %[j], %[count], #1\n"
        "2:\n"
        "    ldp q0, q1, [%[data]]\n"
        "    ldp q2, q3, [%[p1]]\n"
        "    ldp q4, q5, [%[p2]]\n"
        "    ldp q6, q7, [%[p3]]\n"
        "    ldp q9, q8, [%[tw]]\n"
        "    ldp q11, q10, [%[tw], #32]\n"
        "    trn2 v12.4s, v0.4s, v2.4s\n"
        "    trn2 v13.4s, v4.4s, v6.4s\n"
        "    trn2 v14.4s, v1.4s, v3.4s\n"
        "    trn2 v15.4s, v5.4s, v7.4s\n"
        "    zip2 v16.2d, v12.2d, v13.2d\n"
        "    trn1 v0.4s, v0.4s, v2.4s\n"
        "    zip2 v2.2d, v14.2d, v15.2d\n"
        "    trn1 v4.4s, v4.4s, v6.4s\n"
        "    trn1 v1.4s, v1.4s, v3.4s\n"
        "    trn1 v3.4s, v5.4s, v7.4s\n"
        "    mov v5.16b, v16.16b\n"
        "    fmls v16.4s, v2.4s, v8.4s\n"
        "    zip1 v6.2d, v12.2d, v13.2d\n"
        "    fmla v2.4s, v5.4s, v8.4s\n"
        "    zip2 v5.2d, v0.2d, v4.2d\n"
        "    zip1 v7.2d, v14.2d, v15.2d\n"
        "    zip2 v12.2d, v1.2d, v3.2d\n"
        "    mov v13.16b, v6.16b\n"
        "    mov v14.16b, v5.16b\n"
        "    zip1 v0.2d, v0.2d, v4.2d\n"
        "    zip1 v1.2d, v1.2d, v3.2d\n"
        "    fmls v5.4s, v12.4s, v8.4s\n"
        "    mov v3.16b, v7.16b\n"
        "    fmla v6.4s, v2.4s, v9.4s\n"
        "    add %[tw], %[tw], #64\n"
        "    subs %[j], %[j], #1\n"
        "    fmla v12.4s, v14.4s, v8.4s\n"
        "    fmls v7.4s, v16.4s, v9.4s\n"
        "    mov v4.16b, v0.16b\n"
        "    mov v8.16b, v1.16b\n"
        "    fmls v13.4s, v2.4s, v9.4s\n"
        "    fmla v3.4s, v16.4s, v9.4s\n"
        "    fmls v1.4s, v5.4s, v9.4s\n"
        "    fmla v8.4s, v5.4s, v9.4s\n"
        "    mov v2.16b, v6.16b\n"
        "    fmla v0.4s, v12.4s, v9.4s\n"
        "    fmls v4.4s, v12.4s, v9.4s\n"
        "    fmls v6.4s, v7.4s, v10.4s\n"
        "    mov v5.16b, v13.16b\n"
        "    fmla v7.4s, v2.4s, v10.4s\n"
        "    fmls v13.4s, v3.4s, v10.4s\n"
        "    mov v2.16b, v1.16b\n"
        "    mov v9.16b, v8.16b\n"
        "    fmla v3.4s, v5.4s, v10.4s\n"
        "    mov v5.16b, v0.16b\n"
        "    mov v10.16b, v4.16b\n"
        "    fmla v0.4s, v6.4s, v11.4s\n"
        "    fmls v5.4s, v6.4s, v11.4s\n"
        "    fmla v1.4s, v7.4s, v11.4s\n"
        "    fmls v2.4s, v7.4s, v11.4s\n"
        "    fmla v4.4s, v3.4s, v11.4s\n"
        "    fmls v8.4s, v13.4s, v11.4s\n"
        "    fmls v10.4s, v3.4s, v11.4s\n"
        "    fmla v9.4s, v13.4s, v11.4s\n"
        "    zip1 v3.4s, v0.4s, v1.4s\n"
        "    zip2 v0.4s, v0.4s, v1.4s\n"
        "    zip1 v1.4s, v4.4s, v8.4s\n"
        "    zip2 v4.4s, v4.4s, v8.4s\n"
        "    zip1 v6.4s, v5.4s, v2.4s\n"
        "    zip2 v2.4s, v5.4s, v2.4s\n"
        "    stp q3, q0, [%[data]]\n"
        "    zip1 v0.4s, v10.4s, v9.4s\n"
        "    zip2 v3.4s, v10.4s, v9.4s\n"
        "    stp q1, q4, [%[p1]]\n"
        "    add %[data], %[data], #32\n"
        "    stp q6, q2, [%[p2]]\n"
        "    add %[p1], %[p1], #32\n"
        "    stp q0, q3, [%[p3]]\n"
        "    add %[p2], %[p2], #32\n"
        "    add %[p3], %[p3], #32\n"
        "    b.ne 2b\n"
        : [data] "+r"(data), [tw] "+r"(tw), [stride] "+r"(stride), [count] "+r"(count), [j] "=&r"(j), [p1] "=&r"(p1), [p2] "=&r"(p2), [p3] "=&r"(p3)
        :
        : "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", "v10", "v11", "v12", "v13", "v14", "v15", "v16", "cc", "memory");
}

/* First pass, radix 8. For each of the `count` entries (a, b) of `table`: reads 8 interleaved
 * vectors from in + 8*a + u*in_stride, transforms across them and writes output q as a split
 * vector to out + 8*b + (q%4)*out_stride + 32*(q/4). */
static void onefft_neon_f8i(const float *in, float *out, const uint32_t *table, size_t count, size_t in_stride, size_t out_stride) {
    size_t bits;
    const float *src0;
    float *dst0;
    size_t a0;
    size_t b0;
    const float *src1;
    float *dst1;
    size_t a1;
    size_t b1;
    __asm__ volatile(
        "    mov %w[bits], #1267\n"
        "    lsr %[count], %[count], #1\n"
        "    movk %w[bits], #16181, lsl #16\n"
        "    dup v0.4s, %w[bits]\n"
        "1:\n"
        "    ldp %w[a0], %w[b0], [%[table]], #8\n"
        "    add %[src0], %[in], %[a0], lsl #3\n"
        "    add %[dst0], %[out], %[b0], lsl #3\n"
        "    ldp q1, q2, [%[src0]]\n"
        "    add %[src0], %[src0], %[in_stride]\n"
        "    ldp q3, q4, [%[src0]]\n"
        "    add %[src0], %[src0], %[in_stride]\n"
        "    ldp q5, q6, [%[src0]]\n"
        "    uzp1 v7.4s, v1.4s, v2.4s\n"
        "    uzp2 v1.4s, v1.4s, v2.4s\n"
        "    add %[src0], %[src0], %[in_stride]\n"
        "    ldp q2, q8, [%[src0]]\n"
        "    uzp1 v9.4s, v3.4s, v4.4s\n"
        "    uzp2 v3.4s, v3.4s, v4.4s\n"
        "    add %[src0], %[src0], %[in_stride]\n"
        "    ldp q4, q10, [%[src0]]\n"
        "    uzp1 v11.4s, v5.4s, v6.4s\n"
        "    uzp2 v5.4s, v5.4s, v6.4s\n"
        "    add %[src0], %[src0], %[in_stride]\n"
        "    ldp q6, q12, [%[src0]]\n"
        "    uzp1 v13.4s, v2.4s, v8.4s\n"
        "    uzp2 v2.4s, v2.4s, v8.4s\n"
        "    add %[src0], %[src0], %[in_stride]\n"
        "    ldp q8, q14, [%[src0]]\n"
        "    uzp1 v15.4s, v4.4s, v10.4s\n"
        "    uzp2 v4.4s, v4.4s, v10.4s\n"
        "    add %[src0], %[src0], %[in_stride]\n"
        "    ldp q10, q16, [%[src0]]\n"
        "    uzp1 v17.4s, v6.4s, v12.4s\n"
        "    uzp2 v6.4s, v6.4s, v12.4s\n"
        "    fadd v12.4s, v1.4s, v4.4s\n"
        "    fadd v18.4s, v7.4s, v15.4s\n"
        "    fsub v19.4s, v3.4s, v6.4s\n"
        "    fsub v20.4s, v9.4s, v17.4s\n"
        "    uzp1 v21.4s, v8.4s, v14.4s\n"
        "    uzp2 v8.4s, v8.4s, v14.4s\n"
        "    uzp1 v14.4s, v10.4s, v16.4s\n"
        "    uzp2 v10.4s, v10.4s, v16.4s\n"
        "    fadd v16.4s, v5.4s, v8.4s\n"
        "    fadd v22.4s, v11.4s, v21.4s\n"
        "    fsub v23.4s, v2.4s, v10.4s\n"
        "    fsub v24.4s, v13.4s, v14.4s\n"
        "    fadd v3.4s, v3.4s, v6.4s\n"
        "    fadd v6.4s, v9.4s, v17.4s\n"
        "    fadd v2.4s, v2.4s, v10.4s\n"
        "    fadd v9.4s, v13.4s, v14.4s\n"
        "    fadd v10.4s, v19.4s, v24.4s\n"
        "    fsub v13.4s, v20.4s, v23.4s\n"
        "    fsub v14.4s, v19.4s, v24.4s\n"
        "    fadd v17.4s, v20.4s, v23.4s\n"
        "    fsub v1.4s, v1.4s, v4.4s\n"
        "    fsub v4.4s, v7.4s, v15.4s\n"
        "    fsub v5.4s, v5.4s, v8.4s\n"
        "    fsub v7.4s, v11.4s, v21.4s\n"
        "    fadd v8.4s, v10.4s, v13.4s\n"
        "    fsub v10.4s, v13.4s, v10.4s\n"
        "    fadd v11.4s, v12.4s, v16.4s\n"
        "    fadd v13.4s, v18.4s, v22.4s\n"
        "    fadd v15.4s, v3.4s, v2.4s\n"
        "    fadd v19.4s, v6.4s, v9.4s\n"
        "    fadd v20.4s, v14.4s, v17.4s\n"
        "    fsub v14.4s, v17.4s, v14.4s\n"
        "    fadd v17.4s, v1.4s, v7.4s\n"
        "    fsub v21.4s, v4.4s, v5.4s\n"
        "    fmul v8.4s, v8.4s, v0.4s\n"
        "    fmul v10.4s, v10.4s, v0.4s\n"
        "    fsub v12.4s, v12.4s, v16.4s\n"
        "    fsub v16.4s, v18.4s, v22.4s\n"
        "    ldp %w[a1], %w[b1], [%[table]], #8\n"
        "    fsub v2.4s, v3.4s, v2.4s\n"
        "    fsub v3.4s, v6.4s, v9.4s\n"
        "    add %[src1], %[in], %[a1], lsl #3\n"
        "    fsub v1.4s, v1.4s, v7.4s\n"
        "    fadd v4.4s, v4.4s, v5.4s\n"
        "    add %[dst1], %[out], %[b1], lsl #3\n"
        "    ldp q5, q6, [%[src1]]\n"
        "    fmul v7.4s, v20.4s, v0.4s\n"
        "    fmul v9.4s, v14.4s, v0.4s\n"
        "    add %[src1], %[src1], %[in_stride]\n"
        "    fadd v14.4s, v11.4s, v15.4s\n"
        "    fadd v18.4s, v13.4s, v19.4s\n"
        "    ldp q20, q22, [%[src1]]\n"
        "    fsub v11.4s, v11.4s, v15.4s\n"
        "    fsub v13.4s, v13.4s, v19.4s\n"
        "    add %[src1], %[src1], %[in_stride]\n"
        "    fadd v15.4s, v17.4s, v8.4s\n"
        "    fadd v19.4s, v21.4s, v10.4s\n"
        "    ldp q23, q24, [%[src1]]\n"
        "    uzp1 v25.4s, v5.4s, v6.4s\n"
        "    uzp2 v5.4s, v5.4s, v6.4s\n"
        "    stp q14, q18, [%[dst0]]\n"
        "    add %[src1], %[src1], %[in_stride]\n"
        "    fsub v6.4s, v17.4s, v8.4s\n"
        "    fsub v8.4s, v21.4s, v10.4s\n"
        "    stp q11, q13, [%[dst0], #32]\n"
        "    ldp q10, q11, [%[src1]]\n"
        "    uzp1 v13.4s, v20.4s, v22.4s\n"
        "    uzp2 v14.4s, v20.4s, v22.4s\n"
        "    add %[dst0], %[dst0], %[out_stride]\n"
        "    add %[src1], %[src1], %[in_stride]\n"
        "    fadd v17.4s, v12.4s, v3.4s\n"
        "    fsub v18.4s, v16.4s, v2.4s\n"
        "    stp q15, q19, [%[dst0]]\n"
        "    ldp q15, q19, [%[src1]]\n"
        "    uzp1 v20.4s, v23.4s, v24.4s\n"
        "    uzp2 v21.4s, v23.4s, v24.4s\n"
        "    stp q6, q8, [%[dst0], #32]\n"
        "    add %[src1], %[src1], %[in_stride]\n"
        "    fsub v3.4s, v12.4s, v3.4s\n"
        "    fadd v2.4s, v16.4s, v2.4s\n"
        "    add %[dst0], %[dst0], %[out_stride]\n"
        "    ldp q6, q8, [%[src1]]\n"
        "    uzp1 v12.4s, v10.4s, v11.4s\n"
        "    uzp2 v10.4s, v10.4s, v11.4s\n"
        "    stp q17, q18, [%[dst0]]\n"
        "    add %[src1], %[src1], %[in_stride]\n"
        "    fadd v11.4s, v1.4s, v9.4s\n"
        "    fsub v16.4s, v4.4s, v7.4s\n"
        "    ldp q17, q18, [%[src1]]\n"
        "    uzp1 v22.4s, v15.4s, v19.4s\n"
        "    uzp2 v15.4s, v15.4s, v19.4s\n"
        "    stp q3, q2, [%[dst0], #32]\n"
        "    add %[src1], %[src1], %[in_stride]\n"
        "    fsub v1.4s, v1.4s, v9.4s\n"
        "    fadd v2.4s, v4.4s, v7.4s\n"
        "    add %[dst0], %[dst0], %[out_stride]\n"
        "    ldp q3, q4, [%[src1]]\n"
        "    uzp1 v7.4s, v6.4s, v8.4s\n"
        "    uzp2 v6.4s, v6.4s, v8.4s\n"
        "    stp q11, q16, [%[dst0]]\n"
        "    fadd v8.4s, v5.4s, v15.4s\n"
        "    fadd v9.4s, v25.4s, v22.4s\n"
        "    fsub v11.4s, v14.4s, v6.4s\n"
        "    fsub v16.4s, v13.4s, v7.4s\n"
        "    stp q1, q2, [%[dst0], #32]\n"
        "    uzp1 v1.4s, v17.4s, v18.4s\n"
        "    uzp2 v2.4s, v17.4s, v18.4s\n"
        "    uzp1 v17.4s, v3.4s, v4.4s\n"
        "    uzp2 v3.4s, v3.4s, v4.4s\n"
        "    fadd v4.4s, v21.4s, v2.4s\n"
        "    fadd v18.4s, v20.4s, v1.4s\n"
        "    fsub v19.4s, v10.4s, v3.4s\n"
        "    fsub v23.4s, v12.4s, v17.4s\n"
        "    fadd v6.4s, v14.4s, v6.4s\n"
        "    fadd v7.4s, v13.4s, v7.4s\n"
        "    fadd v3.4s, v10.4s, v3.4s\n"
        "    fadd v10.4s, v12.4s, v17.4s\n"
        "    fadd v12.4s, v11.4s, v23.4s\n"
        "    fsub v13.4s, v16.4s, v19.4s\n"
        "    fsub v11.4s, v11.4s, v23.4s\n"
        "    fadd v14.4s, v16.4s, v19.4s\n"
        "    fsub v5.4s, v5.4s, v15.4s\n"
        "    fsub v15.4s, v25.4s, v22.4s\n"
        "    fsub v2.4s, v21.4s, v2.4s\n"
        "    fsub v1.4s, v20.4s, v1.4s\n"
        "    fadd v16.4s, v12.4s, v13.4s\n"
        "    fsub v12.4s, v13.4s, v12.4s\n"
        "    fadd v13.4s, v8.4s, v4.4s\n"
        "    fadd v17.4s, v9.4s, v18.4s\n"
        "    fadd v19.4s, v6.4s, v3.4s\n"
        "    fadd v20.4s, v7.4s, v10.4s\n"
        "    fadd v21.4s, v11.4s, v14.4s\n"
        "    fsub v11.4s, v14.4s, v11.4s\n"
        "    fadd v14.4s, v5.4s, v1.4s\n"
        "    fsub v22.4s, v15.4s, v2.4s\n"
        "    fmul v16.4s, v16.4s, v0.4s\n"
        "    fmul v12.4s, v12.4s, v0.4s\n"
        "    fsub v4.4s, v8.4s, v4.4s\n"
        "    fsub v8.4s, v9.4s, v18.4s\n"
        "    fsub v3.4s, v6.4s, v3.4s\n"
        "    fsub v6.4s, v7.4s, v10.4s\n"
        "    subs %[count], %[count], #1\n"
        "    fsub v1.4s, v5.4s, v1.4s\n"
        "    fadd v2.4s, v15.4s, v2.4s\n"
        "    fmul v5.4s, v21.4s, v0.4s\n"
        "    fmul v7.4s, v11.4s, v0.4s\n"
        "    fadd v9.4s, v13.4s, v19.4s\n"
        "    fadd v10.4s, v17.4s, v20.4s\n"
        "    fsub v11.4s, v13.4s, v19.4s\n"
        "    fsub v13.4s, v17.4s, v20.4s\n"
        "    fadd v15.4s, v14.4s, v16.4s\n"
        "    fadd v17.4s, v22.4s, v12.4s\n"
        "    fsub v14.4s, v14.4s, v16.4s\n"
        "    fsub v12.4s, v22.4s, v12.4s\n"
        "    stp q9, q10, [%[dst1]]\n"
        "    fadd v9.4s, v4.4s, v6.4s\n"
        "    fsub v10.4s, v8.4s, v3.4s\n"
        "    stp q11, q13, [%[dst1], #32]\n"
        "    fsub v4.4s, v4.4s, v6.4s\n"
        "    fadd v3.4s, v8.4s, v3.4s\n"
        "    add %[dst1], %[dst1], %[out_stride]\n"
        "    fadd v6.4s, v1.4s, v7.4s\n"
        "    fsub v8.4s, v2.4s, v5.4s\n"
        "    stp q15, q17, [%[dst1]]\n"
        "    stp q14, q12, [%[dst1], #32]\n"
        "    fsub v1.4s, v1.4s, v7.4s\n"
        "    fadd v2.4s, v2.4s, v5.4s\n"
        "    add %[dst1], %[dst1], %[out_stride]\n"
        "    stp q9, q10, [%[dst1]]\n"
        "    stp q4, q3, [%[dst1], #32]\n"
        "    add %[dst1], %[dst1], %[out_stride]\n"
        "    stp q6, q8, [%[dst1]]\n"
        "    stp q1, q2, [%[dst1], #32]\n"
        "    b.ne 1b\n"
        : [in] "+r"(in), [out] "+r"(out), [table] "+r"(table), [count] "+r"(count), [in_stride] "+r"(in_stride), [out_stride] "+r"(out_stride), [bits] "=&r"(bits), [src0] "=&r"(src0), [dst0] "=&r"(dst0), [a0] "=&r"(a0), [b0] "=&r"(b0), [src1] "=&r"(src1), [dst1] "=&r"(dst1), [a1] "=&r"(a1), [b1] "=&r"(b1)
        :
        : "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", "v10", "v11", "v12", "v13", "v14", "v15", "v16", "v17", "v18", "v19", "v20", "v21", "v22", "v23", "v24", "v25", "cc", "memory");
}

/* Last pass, radix 8, in place: reads split vectors and writes the interleaved result.
 * Butterfly j < count uses the vectors at data + u*stride + 32*j. The last parameter is not used: it
 * exists because the last pass kernels of all targets share a signature. See the table builder in
 * onefft.c for the twiddle layout. */
static void onefft_neon_l8i(float *data, const float *tw, size_t stride, size_t count, size_t regions) {
    (void)regions;
    size_t j;
    float *p1;
    float *p2;
    float *p3;
    float *p4;
    float *p5;
    float *p6;
    float *p7;
    __asm__ volatile(
        "    add %[p1], %[data], %[stride]\n"
        "    lsr %[j], %[count], #1\n"
        "    add %[p2], %[p1], %[stride]\n"
        "    add %[p3], %[p2], %[stride]\n"
        "    add %[p4], %[p3], %[stride]\n"
        "    add %[p5], %[p4], %[stride]\n"
        "    add %[p6], %[p5], %[stride]\n"
        "    add %[p7], %[p6], %[stride]\n"
        "1:\n"
        "    ldp q1, q0, [%[p1]]\n"
        "    ldp q3, q2, [%[p3]]\n"
        "    ldp q5, q4, [%[p5]]\n"
        "    ldp q7, q6, [%[p7]]\n"
        "    ldp q8, q9, [%[tw]]\n"
        "    ldp q10, q11, [%[data]]\n"
        "    ldp q12, q13, [%[p2]]\n"
        "    ldp q14, q15, [%[p4]]\n"
        "    ldp q16, q17, [%[p6]]\n"
        "    ldp q18, q19, [%[tw], #32]\n"
        "    ldp q20, q21, [%[tw], #64]\n"
        "    ldp q22, q23, [%[tw], #96]\n"
        "    fmul v24.4s, v1.4s, v8.s[0]\n"
        "    fmul v25.4s, v0.4s, v8.s[0]\n"
        "    fmul v26.4s, v3.4s, v8.s[1]\n"
        "    fmul v27.4s, v2.4s, v8.s[1]\n"
        "    fmul v28.4s, v5.4s, v8.s[2]\n"
        "    fmul v29.4s, v4.4s, v8.s[2]\n"
        "    fmul v30.4s, v7.4s, v8.s[3]\n"
        "    fmul v8.4s, v6.4s, v8.s[3]\n"
        "    fmls v24.4s, v0.4s, v9.s[0]\n"
        "    fmla v25.4s, v1.4s, v9.s[0]\n"
        "    fmls v26.4s, v2.4s, v9.s[1]\n"
        "    fmla v27.4s, v3.4s, v9.s[1]\n"
        "    fmls v28.4s, v4.4s, v9.s[2]\n"
        "    fmla v29.4s, v5.4s, v9.s[2]\n"
        "    fmls v30.4s, v6.4s, v9.s[3]\n"
        "    fmla v8.4s, v7.4s, v9.s[3]\n"
        "    fadd v0.4s, v10.4s, v24.4s\n"
        "    fadd v1.4s, v11.4s, v25.4s\n"
        "    fsub v2.4s, v10.4s, v24.4s\n"
        "    fsub v3.4s, v11.4s, v25.4s\n"
        "    fadd v4.4s, v12.4s, v26.4s\n"
        "    fadd v5.4s, v13.4s, v27.4s\n"
        "    fsub v6.4s, v12.4s, v26.4s\n"
        "    fsub v7.4s, v13.4s, v27.4s\n"
        "    fadd v9.4s, v14.4s, v28.4s\n"
        "    fadd v10.4s, v15.4s, v29.4s\n"
        "    fsub v11.4s, v14.4s, v28.4s\n"
        "    fsub v12.4s, v15.4s, v29.4s\n"
        "    fadd v13.4s, v16.4s, v30.4s\n"
        "    fadd v14.4s, v17.4s, v8.4s\n"
        "    fsub v15.4s, v16.4s, v30.4s\n"
        "    fsub v8.4s, v17.4s, v8.4s\n"
        "    trn1 v16.4s, v0.4s, v4.4s\n"
        "    trn2 v0.4s, v0.4s, v4.4s\n"
        "    trn1 v4.4s, v9.4s, v13.4s\n"
        "    trn2 v9.4s, v9.4s, v13.4s\n"
        "    trn1 v13.4s, v1.4s, v5.4s\n"
        "    trn2 v1.4s, v1.4s, v5.4s\n"
        "    trn1 v5.4s, v10.4s, v14.4s\n"
        "    trn2 v10.4s, v10.4s, v14.4s\n"
        "    trn1 v14.4s, v2.4s, v6.4s\n"
        "    trn2 v2.4s, v2.4s, v6.4s\n"
        "    trn1 v6.4s, v11.4s, v15.4s\n"
        "    trn2 v11.4s, v11.4s, v15.4s\n"
        "    trn1 v15.4s, v3.4s, v7.4s\n"
        "    trn2 v3.4s, v3.4s, v7.4s\n"
        "    zip1 v7.2d, v0.2d, v9.2d\n"
        "    zip2 v17.2d, v16.2d, v4.2d\n"
        "    zip2 v0.2d, v0.2d, v9.2d\n"
        "    zip1 v9.2d, v1.2d, v10.2d\n"
        "    zip2 v24.2d, v13.2d, v5.2d\n"
        "    zip2 v1.2d, v1.2d, v10.2d\n"
        "    zip1 v10.2d, v2.2d, v11.2d\n"
        "    zip2 v25.2d, v14.2d, v6.2d\n"
        "    zip2 v2.2d, v2.2d, v11.2d\n"
        "    fmul v11.4s, v7.4s, v18.4s\n"
        "    fmul v18.4s, v9.4s, v18.4s\n"
        "    fmul v26.4s, v17.4s, v20.4s\n"
        "    fmul v20.4s, v24.4s, v20.4s\n"
        "    fmul v27.4s, v0.4s, v22.4s\n"
        "    fmul v22.4s, v1.4s, v22.4s\n"
        "    fmls v11.4s, v9.4s, v19.4s\n"
        "    fmla v18.4s, v7.4s, v19.4s\n"
        "    fmls v26.4s, v24.4s, v21.4s\n"
        "    fmla v20.4s, v17.4s, v21.4s\n"
        "    fmls v27.4s, v1.4s, v23.4s\n"
        "    fmla v22.4s, v0.4s, v23.4s\n"
        "    zip1 v0.2d, v16.2d, v4.2d\n"
        "    trn1 v1.4s, v12.4s, v8.4s\n"
        "    trn2 v4.4s, v12.4s, v8.4s\n"
        "    ldp q7, q8, [%[tw], #128]\n"
        "    zip1 v5.2d, v13.2d, v5.2d\n"
        "    zip1 v6.2d, v14.2d, v6.2d\n"
        "    ldp q9, q12, [%[tw], #160]\n"
        "    ldp q13, q14, [%[tw], #192]\n"
        "    zip1 v16.2d, v3.2d, v4.2d\n"
        "    zip2 v17.2d, v15.2d, v1.2d\n"
        "    zip2 v3.2d, v3.2d, v4.2d\n"
        "    zip1 v1.2d, v15.2d, v1.2d\n"
        "    fmul v4.4s, v10.4s, v7.4s\n"
        "    fmul v7.4s, v16.4s, v7.4s\n"
        "    fadd v15.4s, v0.4s, v26.4s\n"
        "    fadd v19.4s, v5.4s, v20.4s\n"
        "    fmul v21.4s, v25.4s, v9.4s\n"
        "    fmul v9.4s, v17.4s, v9.4s\n"
        "    fmul v23.4s, v2.4s, v13.4s\n"
        "    fmul v13.4s, v3.4s, v13.4s\n"
        "    fmls v4.4s, v16.4s, v8.4s\n"
        "    fmla v7.4s, v10.4s, v8.4s\n"
        "    fmls v21.4s, v17.4s, v12.4s\n"
        "    fmla v9.4s, v25.4s, v12.4s\n"
        "    fmls v23.4s, v3.4s, v14.4s\n"
        "    fmla v13.4s, v2.4s, v14.4s\n"
        "    fsub v0.4s, v0.4s, v26.4s\n"
        "    fsub v2.4s, v5.4s, v20.4s\n"
        "    fadd v3.4s, v11.4s, v27.4s\n"
        "    fadd v5.4s, v18.4s, v22.4s\n"
        "    fsub v8.4s, v11.4s, v27.4s\n"
        "    fsub v10.4s, v18.4s, v22.4s\n"
        "    fadd v11.4s, v6.4s, v21.4s\n"
        "    fadd v12.4s, v1.4s, v9.4s\n"
        "    fsub v6.4s, v6.4s, v21.4s\n"
        "    fsub v1.4s, v1.4s, v9.4s\n"
        "    fadd v9.4s, v4.4s, v23.4s\n"
        "    fadd v14.4s, v7.4s, v13.4s\n"
        "    fsub v4.4s, v4.4s, v23.4s\n"
        "    fsub v7.4s, v7.4s, v13.4s\n"
        "    fadd v13.4s, v15.4s, v3.4s\n"
        "    fadd v16.4s, v19.4s, v5.4s\n"
        "    fsub v3.4s, v15.4s, v3.4s\n"
        "    fsub v5.4s, v19.4s, v5.4s\n"
        "    fadd v15.4s, v0.4s, v10.4s\n"
        "    fsub v17.4s, v2.4s, v8.4s\n"
        "    fsub v0.4s, v0.4s, v10.4s\n"
        "    fadd v2.4s, v2.4s, v8.4s\n"
        "    fadd v8.4s, v11.4s, v9.4s\n"
        "    fadd v10.4s, v12.4s, v14.4s\n"
        "    fsub v9.4s, v11.4s, v9.4s\n"
        "    fsub v11.4s, v12.4s, v14.4s\n"
        "    fadd v12.4s, v6.4s, v7.4s\n"
        "    fsub v14.4s, v1.4s, v4.4s\n"
        "    fsub v6.4s, v6.4s, v7.4s\n"
        "    fadd v1.4s, v1.4s, v4.4s\n"
        "    zip1 v4.4s, v16.4s, v13.4s\n"
        "    zip2 v7.4s, v16.4s, v13.4s\n"
        "    zip1 v13.4s, v17.4s, v15.4s\n"
        "    zip2 v15.4s, v17.4s, v15.4s\n"
        "    zip1 v16.4s, v5.4s, v3.4s\n"
        "    zip2 v3.4s, v5.4s, v3.4s\n"
        "    stp q4, q7, [%[data]]\n"
        "    zip1 v4.4s, v2.4s, v0.4s\n"
        "    zip2 v0.4s, v2.4s, v0.4s\n"
        "    stp q13, q15, [%[p2]]\n"
        "    ldp q5, q2, [%[p1], #32]\n"
        "    ldp q7, q13, [%[data], #32]\n"
        "    zip1 v15.4s, v10.4s, v8.4s\n"
        "    zip2 v8.4s, v10.4s, v8.4s\n"
        "    stp q16, q3, [%[p4]]\n"
        "    ldp q10, q3, [%[p3], #32]\n"
        "    ldp q16, q17, [%[p2], #32]\n"
        "    ldp q18, q19, [%[p4], #32]\n"
        "    zip1 v20.4s, v14.4s, v12.4s\n"
        "    zip2 v12.4s, v14.4s, v12.4s\n"
        "    stp q4, q0, [%[p6]]\n"
        "    ldp q4, q0, [%[p5], #32]\n"
        "    ldp q21, q14, [%[p7], #32]\n"
        "    ldp q22, q23, [%[tw], #224]\n"
        "    ldp q24, q25, [%[p6], #32]\n"
        "    zip1 v26.4s, v11.4s, v9.4s\n"
        "    zip2 v9.4s, v11.4s, v9.4s\n"
        "    stp q15, q8, [%[p1]]\n"
        "    ldp q8, q11, [%[tw], #256]\n"
        "    zip1 v15.4s, v1.4s, v6.4s\n"
        "    zip2 v1.4s, v1.4s, v6.4s\n"
        "    stp q20, q12, [%[p3]]\n"
        "    stp q26, q9, [%[p5]]\n"
        "    ldp q6, q9, [%[tw], #288]\n"
        "    stp q15, q1, [%[p7]]\n"
        "    fmul v1.4s, v5.4s, v22.s[0]\n"
        "    fmul v12.4s, v2.4s, v22.s[0]\n"
        "    ldp q15, q20, [%[tw], #320]\n"
        "    fmul v26.4s, v10.4s, v22.s[1]\n"
        "    fmul v27.4s, v3.4s, v22.s[1]\n"
        "    fmul v28.4s, v4.4s, v22.s[2]\n"
        "    fmul v29.4s, v0.4s, v22.s[2]\n"
        "    fmul v30.4s, v21.4s, v22.s[3]\n"
        "    fmul v22.4s, v14.4s, v22.s[3]\n"
        "    fmls v1.4s, v2.4s, v23.s[0]\n"
        "    fmla v12.4s, v5.4s, v23.s[0]\n"
        "    fmls v26.4s, v3.4s, v23.s[1]\n"
        "    fmla v27.4s, v10.4s, v23.s[1]\n"
        "    fmls v28.4s, v0.4s, v23.s[2]\n"
        "    fmla v29.4s, v4.4s, v23.s[2]\n"
        "    fmls v30.4s, v14.4s, v23.s[3]\n"
        "    fmla v22.4s, v21.4s, v23.s[3]\n"
        "    fadd v0.4s, v7.4s, v1.4s\n"
        "    fadd v2.4s, v13.4s, v12.4s\n"
        "    fsub v1.4s, v7.4s, v1.4s\n"
        "    fsub v3.4s, v13.4s, v12.4s\n"
        "    fadd v4.4s, v16.4s, v26.4s\n"
        "    fadd v5.4s, v17.4s, v27.4s\n"
        "    fsub v7.4s, v16.4s, v26.4s\n"
        "    fsub v10.4s, v17.4s, v27.4s\n"
        "    fadd v12.4s, v18.4s, v28.4s\n"
        "    fadd v13.4s, v19.4s, v29.4s\n"
        "    fsub v14.4s, v18.4s, v28.4s\n"
        "    fsub v16.4s, v19.4s, v29.4s\n"
        "    fadd v17.4s, v24.4s, v30.4s\n"
        "    fadd v18.4s, v25.4s, v22.4s\n"
        "    fsub v19.4s, v24.4s, v30.4s\n"
        "    fsub v21.4s, v25.4s, v22.4s\n"
        "    trn1 v22.4s, v0.4s, v4.4s\n"
        "    trn2 v0.4s, v0.4s, v4.4s\n"
        "    trn1 v4.4s, v12.4s, v17.4s\n"
        "    trn2 v12.4s, v12.4s, v17.4s\n"
        "    trn1 v17.4s, v2.4s, v5.4s\n"
        "    trn2 v2.4s, v2.4s, v5.4s\n"
        "    trn1 v5.4s, v13.4s, v18.4s\n"
        "    trn2 v13.4s, v13.4s, v18.4s\n"
        "    trn1 v18.4s, v1.4s, v7.4s\n"
        "    trn2 v1.4s, v1.4s, v7.4s\n"
        "    trn1 v7.4s, v14.4s, v19.4s\n"
        "    trn2 v14.4s, v14.4s, v19.4s\n"
        "    trn1 v19.4s, v3.4s, v10.4s\n"
        "    trn2 v3.4s, v3.4s, v10.4s\n"
        "    zip1 v10.2d, v0.2d, v12.2d\n"
        "    zip2 v23.2d, v22.2d, v4.2d\n"
        "    zip2 v0.2d, v0.2d, v12.2d\n"
        "    zip1 v12.2d, v2.2d, v13.2d\n"
        "    zip2 v24.2d, v17.2d, v5.2d\n"
        "    zip2 v2.2d, v2.2d, v13.2d\n"
        "    zip1 v13.2d, v1.2d, v14.2d\n"
        "    zip2 v25.2d, v18.2d, v7.2d\n"
        "    zip2 v1.2d, v1.2d, v14.2d\n"
        "    fmul v14.4s, v10.4s, v8.4s\n"
        "    fmul v8.4s, v12.4s, v8.4s\n"
        "    fmul v26.4s, v23.4s, v6.4s\n"
        "    fmul v6.4s, v24.4s, v6.4s\n"
        "    fmul v27.4s, v0.4s, v15.4s\n"
        "    fmul v15.4s, v2.4s, v15.4s\n"
        "    fmls v14.4s, v12.4s, v11.4s\n"
        "    fmla v8.4s, v10.4s, v11.4s\n"
        "    fmls v26.4s, v24.4s, v9.4s\n"
        "    fmla v6.4s, v23.4s, v9.4s\n"
        "    fmls v27.4s, v2.4s, v20.4s\n"
        "    fmla v15.4s, v0.4s, v20.4s\n"
        "    zip1 v0.2d, v22.2d, v4.2d\n"
        "    trn1 v2.4s, v16.4s, v21.4s\n"
        "    trn2 v4.4s, v16.4s, v21.4s\n"
        "    ldp q9, q10, [%[tw], #352]\n"
        "    zip1 v5.2d, v17.2d, v5.2d\n"
        "    zip1 v7.2d, v18.2d, v7.2d\n"
        "    ldp q11, q12, [%[tw], #384]\n"
        "    ldp q16, q17, [%[tw], #416]\n"
        "    zip1 v18.2d, v3.2d, v4.2d\n"
        "    zip2 v20.2d, v19.2d, v2.2d\n"
        "    zip2 v3.2d, v3.2d, v4.2d\n"
        "    zip1 v2.2d, v19.2d, v2.2d\n"
        "    fmul v4.4s, v13.4s, v9.4s\n"
        "    fmul v9.4s, v18.4s, v9.4s\n"
        "    fadd v19.4s, v0.4s, v26.4s\n"
        "    fadd v21.4s, v5.4s, v6.4s\n"
        "    fmul v22.4s, v25.4s, v11.4s\n"
        "    fmul v11.4s, v20.4s, v11.4s\n"
        "    fmul v23.4s, v1.4s, v16.4s\n"
        "    fmul v16.4s, v3.4s, v16.4s\n"
        "    fmls v4.4s, v18.4s, v10.4s\n"
        "    fmla v9.4s, v13.4s, v10.4s\n"
        "    fmls v22.4s, v20.4s, v12.4s\n"
        "    fmla v11.4s, v25.4s, v12.4s\n"
        "    fmls v23.4s, v3.4s, v17.4s\n"
        "    fmla v16.4s, v1.4s, v17.4s\n"
        "    fsub v0.4s, v0.4s, v26.4s\n"
        "    fsub v1.4s, v5.4s, v6.4s\n"
        "    fadd v3.4s, v14.4s, v27.4s\n"
        "    fadd v5.4s, v8.4s, v15.4s\n"
        "    fsub v6.4s, v14.4s, v27.4s\n"
        "    fsub v8.4s, v8.4s, v15.4s\n"
        "    fadd v10.4s, v7.4s, v22.4s\n"
        "    fadd v12.4s, v2.4s, v11.4s\n"
        "    fsub v7.4s, v7.4s, v22.4s\n"
        "    fsub v2.4s, v2.4s, v11.4s\n"
        "    fadd v11.4s, v4.4s, v23.4s\n"
        "    fadd v13.4s, v9.4s, v16.4s\n"
        "    fsub v4.4s, v4.4s, v23.4s\n"
        "    fsub v9.4s, v9.4s, v16.4s\n"
        "    fadd v14.4s, v19.4s, v3.4s\n"
        "    fadd v15.4s, v21.4s, v5.4s\n"
        "    fsub v3.4s, v19.4s, v3.4s\n"
        "    fsub v5.4s, v21.4s, v5.4s\n"
        "    fadd v16.4s, v0.4s, v8.4s\n"
        "    fsub v17.4s, v1.4s, v6.4s\n"
        "    fsub v0.4s, v0.4s, v8.4s\n"
        "    fadd v1.4s, v1.4s, v6.4s\n"
        "    fadd v6.4s, v10.4s, v11.4s\n"
        "    fadd v8.4s, v12.4s, v13.4s\n"
        "    fsub v10.4s, v10.4s, v11.4s\n"
        "    fsub v11.4s, v12.4s, v13.4s\n"
        "    fadd v12.4s, v7.4s, v9.4s\n"
        "    fsub v13.4s, v2.4s, v4.4s\n"
        "    fsub v7.4s, v7.4s, v9.4s\n"
        "    fadd v2.4s, v2.4s, v4.4s\n"
        "    zip1 v4.4s, v15.4s, v14.4s\n"
        "    zip2 v9.4s, v15.4s, v14.4s\n"
        "    zip1 v14.4s, v17.4s, v16.4s\n"
        "    zip2 v15.4s, v17.4s, v16.4s\n"
        "    zip1 v16.4s, v5.4s, v3.4s\n"
        "    zip2 v3.4s, v5.4s, v3.4s\n"
        "    stp q4, q9, [%[data], #32]\n"
        "    zip1 v4.4s, v1.4s, v0.4s\n"
        "    zip2 v0.4s, v1.4s, v0.4s\n"
        "    stp q14, q15, [%[p2], #32]\n"
        "    zip1 v1.4s, v8.4s, v6.4s\n"
        "    zip2 v5.4s, v8.4s, v6.4s\n"
        "    stp q16, q3, [%[p4], #32]\n"
        "    add %[data], %[data], #64\n"
        "    zip1 v3.4s, v13.4s, v12.4s\n"
        "    zip2 v6.4s, v13.4s, v12.4s\n"
        "    stp q4, q0, [%[p6], #32]\n"
        "    add %[p2], %[p2], #64\n"
        "    add %[p4], %[p4], #64\n"
        "    zip1 v0.4s, v11.4s, v10.4s\n"
        "    zip2 v4.4s, v11.4s, v10.4s\n"
        "    stp q1, q5, [%[p1], #32]\n"
        "    add %[p6], %[p6], #64\n"
        "    add %[tw], %[tw], #448\n"
        "    subs %[j], %[j], #1\n"
        "    zip1 v1.4s, v2.4s, v7.4s\n"
        "    zip2 v2.4s, v2.4s, v7.4s\n"
        "    stp q3, q6, [%[p3], #32]\n"
        "    add %[p1], %[p1], #64\n"
        "    stp q0, q4, [%[p5], #32]\n"
        "    add %[p3], %[p3], #64\n"
        "    stp q1, q2, [%[p7], #32]\n"
        "    add %[p5], %[p5], #64\n"
        "    add %[p7], %[p7], #64\n"
        "    b.ne 1b\n"
        : [data] "+r"(data), [tw] "+r"(tw), [stride] "+r"(stride), [count] "+r"(count), [j] "=&r"(j), [p1] "=&r"(p1), [p2] "=&r"(p2), [p3] "=&r"(p3), [p4] "=&r"(p4), [p5] "=&r"(p5), [p6] "=&r"(p6), [p7] "=&r"(p7)
        :
        : "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", "v10", "v11", "v12", "v13", "v14", "v15", "v16", "v17", "v18", "v19", "v20", "v21", "v22", "v23", "v24", "v25", "v26", "v27", "v28", "v29", "v30", "cc", "memory");
}

/* Last pass, radix 4, in place: reads split vectors and writes the interleaved result.
 * Butterfly j < count uses the vectors at data + u*stride + 32*j. The last parameter is not used: it
 * exists because the last pass kernels of all targets share a signature. See the table builder in
 * onefft.c for the twiddle layout. */
static void onefft_neon_l4i(float *data, const float *tw, size_t stride, size_t count, size_t regions) {
    (void)regions;
    size_t j;
    float *p1;
    float *p2;
    float *p3;
    __asm__ volatile(
        "    add %[p1], %[data], %[stride]\n"
        "    lsr %[j], %[count], #1\n"
        "    add %[p2], %[p1], %[stride]\n"
        "    add %[p3], %[p2], %[stride]\n"
        "1:\n"
        "    ldp q0, q1, [%[data]]\n"
        "    ldp q2, q3, [%[p1]]\n"
        "    ldp q4, q5, [%[p2]]\n"
        "    ldp q6, q7, [%[p3]]\n"
        "    ldp q8, q9, [%[data], #32]\n"
        "    ldp q10, q11, [%[p1], #32]\n"
        "    ldp q12, q13, [%[tw]]\n"
        "    ldp q14, q15, [%[tw], #32]\n"
        "    ldp q16, q17, [%[tw], #64]\n"
        "    ldp q18, q19, [%[p2], #32]\n"
        "    ldp q20, q21, [%[p3], #32]\n"
        "    trn1 v22.4s, v0.4s, v2.4s\n"
        "    trn2 v0.4s, v0.4s, v2.4s\n"
        "    trn1 v2.4s, v4.4s, v6.4s\n"
        "    trn2 v4.4s, v4.4s, v6.4s\n"
        "    trn1 v6.4s, v1.4s, v3.4s\n"
        "    trn2 v1.4s, v1.4s, v3.4s\n"
        "    trn1 v3.4s, v5.4s, v7.4s\n"
        "    trn2 v5.4s, v5.4s, v7.4s\n"
        "    trn1 v7.4s, v8.4s, v10.4s\n"
        "    trn2 v8.4s, v8.4s, v10.4s\n"
        "    trn1 v10.4s, v18.4s, v20.4s\n"
        "    trn2 v18.4s, v18.4s, v20.4s\n"
        "    zip1 v20.2d, v0.2d, v4.2d\n"
        "    zip2 v23.2d, v22.2d, v2.2d\n"
        "    zip2 v0.2d, v0.2d, v4.2d\n"
        "    zip1 v4.2d, v1.2d, v5.2d\n"
        "    zip2 v24.2d, v6.2d, v3.2d\n"
        "    zip2 v1.2d, v1.2d, v5.2d\n"
        "    zip1 v5.2d, v8.2d, v18.2d\n"
        "    fmul v25.4s, v20.4s, v12.4s\n"
        "    fmul v12.4s, v4.4s, v12.4s\n"
        "    fmul v26.4s, v23.4s, v14.4s\n"
        "    fmul v14.4s, v24.4s, v14.4s\n"
        "    fmul v27.4s, v0.4s, v16.4s\n"
        "    fmul v16.4s, v1.4s, v16.4s\n"
        "    fmls v25.4s, v4.4s, v13.4s\n"
        "    fmla v12.4s, v20.4s, v13.4s\n"
        "    fmls v26.4s, v24.4s, v15.4s\n"
        "    fmla v14.4s, v23.4s, v15.4s\n"
        "    fmls v27.4s, v1.4s, v17.4s\n"
        "    fmla v16.4s, v0.4s, v17.4s\n"
        "    zip1 v0.2d, v22.2d, v2.2d\n"
        "    trn1 v1.4s, v9.4s, v11.4s\n"
        "    trn2 v2.4s, v9.4s, v11.4s\n"
        "    trn1 v4.4s, v19.4s, v21.4s\n"
        "    trn2 v9.4s, v19.4s, v21.4s\n"
        "    zip2 v11.2d, v7.2d, v10.2d\n"
        "    zip2 v8.2d, v8.2d, v18.2d\n"
        "    zip1 v13.2d, v2.2d, v9.2d\n"
        "    zip1 v3.2d, v6.2d, v3.2d\n"
        "    ldp q6, q15, [%[tw], #96]\n"
        "    ldp q17, q18, [%[tw], #128]\n"
        "    ldp q19, q20, [%[tw], #160]\n"
        "    zip2 v21.2d, v1.2d, v4.2d\n"
        "    zip2 v2.2d, v2.2d, v9.2d\n"
        "    zip1 v7.2d, v7.2d, v10.2d\n"
        "    zip1 v1.2d, v1.2d, v4.2d\n"
        "    fadd v4.4s, v0.4s, v26.4s\n"
        "    fadd v9.4s, v3.4s, v14.4s\n"
        "    fsub v0.4s, v0.4s, v26.4s\n"
        "    fsub v3.4s, v3.4s, v14.4s\n"
        "    fmul v10.4s, v5.4s, v6.4s\n"
        "    fmul v6.4s, v13.4s, v6.4s\n"
        "    fmul v14.4s, v11.4s, v17.4s\n"
        "    fmul v17.4s, v21.4s, v17.4s\n"
        "    fmul v22.4s, v8.4s, v19.4s\n"
        "    fmul v19.4s, v2.4s, v19.4s\n"
        "    fmls v10.4s, v13.4s, v15.4s\n"
        "    fmla v6.4s, v5.4s, v15.4s\n"
        "    fmls v14.4s, v21.4s, v18.4s\n"
        "    fmla v17.4s, v11.4s, v18.4s\n"
        "    fmls v22.4s, v2.4s, v20.4s\n"
        "    fmla v19.4s, v8.4s, v20.4s\n"
        "    fadd v2.4s, v25.4s, v27.4s\n"
        "    fadd v5.4s, v12.4s, v16.4s\n"
        "    fsub v8.4s, v25.4s, v27.4s\n"
        "    fsub v11.4s, v12.4s, v16.4s\n"
        "    fadd v12.4s, v7.4s, v14.4s\n"
        "    fadd v13.4s, v1.4s, v17.4s\n"
        "    fsub v7.4s, v7.4s, v14.4s\n"
        "    fsub v1.4s, v1.4s, v17.4s\n"
        "    fadd v14.4s, v10.4s, v22.4s\n"
        "    fadd v15.4s, v6.4s, v19.4s\n"
        "    fsub v10.4s, v10.4s, v22.4s\n"
        "    fsub v6.4s, v6.4s, v19.4s\n"
        "    fadd v16.4s, v4.4s, v2.4s\n"
        "    fadd v17.4s, v9.4s, v5.4s\n"
        "    fsub v2.4s, v4.4s, v2.4s\n"
        "    fsub v4.4s, v9.4s, v5.4s\n"
        "    fadd v5.4s, v0.4s, v11.4s\n"
        "    fsub v9.4s, v3.4s, v8.4s\n"
        "    fsub v0.4s, v0.4s, v11.4s\n"
        "    fadd v3.4s, v3.4s, v8.4s\n"
        "    fadd v8.4s, v12.4s, v14.4s\n"
        "    fadd v11.4s, v13.4s, v15.4s\n"
        "    fsub v12.4s, v12.4s, v14.4s\n"
        "    fsub v13.4s, v13.4s, v15.4s\n"
        "    fadd v14.4s, v7.4s, v6.4s\n"
        "    fsub v15.4s, v1.4s, v10.4s\n"
        "    fsub v6.4s, v7.4s, v6.4s\n"
        "    fadd v1.4s, v1.4s, v10.4s\n"
        "    zip1 v7.4s, v17.4s, v16.4s\n"
        "    zip2 v10.4s, v17.4s, v16.4s\n"
        "    zip1 v16.4s, v9.4s, v5.4s\n"
        "    zip2 v5.4s, v9.4s, v5.4s\n"
        "    zip1 v9.4s, v4.4s, v2.4s\n"
        "    zip2 v2.4s, v4.4s, v2.4s\n"
        "    stp q7, q10, [%[data]]\n"
        "    zip1 v4.4s, v3.4s, v0.4s\n"
        "    zip2 v0.4s, v3.4s, v0.4s\n"
        "    stp q16, q5, [%[p1]]\n"
        "    zip1 v3.4s, v11.4s, v8.4s\n"
        "    zip2 v5.4s, v11.4s, v8.4s\n"
        "    stp q9, q2, [%[p2]]\n"
        "    zip1 v2.4s, v15.4s, v14.4s\n"
        "    zip2 v7.4s, v15.4s, v14.4s\n"
        "    stp q4, q0, [%[p3]]\n"
        "    zip1 v0.4s, v13.4s, v12.4s\n"
        "    zip2 v4.4s, v13.4s, v12.4s\n"
        "    stp q3, q5, [%[data], #32]\n"
        "    add %[tw], %[tw], #192\n"
        "    subs %[j], %[j], #1\n"
        "    zip1 v3.4s, v1.4s, v6.4s\n"
        "    zip2 v1.4s, v1.4s, v6.4s\n"
        "    stp q2, q7, [%[p1], #32]\n"
        "    add %[data], %[data], #64\n"
        "    stp q0, q4, [%[p2], #32]\n"
        "    add %[p1], %[p1], #64\n"
        "    stp q3, q1, [%[p3], #32]\n"
        "    add %[p2], %[p2], #64\n"
        "    add %[p3], %[p3], #64\n"
        "    b.ne 1b\n"
        : [data] "+r"(data), [tw] "+r"(tw), [stride] "+r"(stride), [count] "+r"(count), [j] "=&r"(j), [p1] "=&r"(p1), [p2] "=&r"(p2), [p3] "=&r"(p3)
        :
        : "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", "v10", "v11", "v12", "v13", "v14", "v15", "v16", "v17", "v18", "v19", "v20", "v21", "v22", "v23", "v24", "v25", "v26", "v27", "cc", "memory");
}

static const float onefft_neon_n8i_consts[16] ONEFFT_ALIGNED(64) = {
    0x1.0000000000000p+0f, 0x1.0000000000000p+0f, -0x0.0p+0f, -0x0.0p+0f,
    0x0.0p+0f, 0x0.0p+0f, -0x1.0000000000000p+0f, -0x1.0000000000000p+0f,
    0x1.0000000000000p+0f, 0x1.6a09e667f3bcdp-1f, -0x0.0p+0f, -0x1.6a09e667f3bccp-1f,
    0x0.0p+0f, -0x1.6a09e667f3bccp-1f, -0x1.0000000000000p+0f, -0x1.6a09e667f3bcdp-1f,
};
/* Complete 8 point transform from `in` to `out`, both interleaved. */
static void onefft_neon_n8i(const float *in, float *out) {
    const float *consts = onefft_neon_n8i_consts;
    __asm__ volatile(
        "    ldp q0, q1, [%[in]]\n"
        "    ldp q2, q3, [%[in], #32]\n"
        "    ldp q4, q5, [%[consts]]\n"
        "    ldp q6, q7, [%[consts], #32]\n"
        "    uzp1 v8.4s, v0.4s, v1.4s\n"
        "    uzp2 v0.4s, v0.4s, v1.4s\n"
        "    uzp1 v1.4s, v2.4s, v3.4s\n"
        "    uzp2 v2.4s, v2.4s, v3.4s\n"
        "    fadd v3.4s, v0.4s, v2.4s\n"
        "    fadd v9.4s, v8.4s, v1.4s\n"
        "    fsub v0.4s, v0.4s, v2.4s\n"
        "    fsub v1.4s, v8.4s, v1.4s\n"
        "    zip2 v2.2d, v3.2d, v0.2d\n"
        "    zip2 v8.2d, v9.2d, v1.2d\n"
        "    zip1 v0.2d, v3.2d, v0.2d\n"
        "    zip1 v1.2d, v9.2d, v1.2d\n"
        "    fmul v3.4s, v2.4s, v4.4s\n"
        "    fmul v4.4s, v8.4s, v4.4s\n"
        "    fmls v3.4s, v8.4s, v5.4s\n"
        "    fmla v4.4s, v2.4s, v5.4s\n"
        "    fadd v2.4s, v0.4s, v3.4s\n"
        "    fadd v5.4s, v1.4s, v4.4s\n"
        "    fsub v0.4s, v0.4s, v3.4s\n"
        "    fsub v1.4s, v1.4s, v4.4s\n"
        "    uzp2 v3.4s, v2.4s, v0.4s\n"
        "    uzp2 v4.4s, v5.4s, v1.4s\n"
        "    uzp1 v0.4s, v2.4s, v0.4s\n"
        "    uzp1 v1.4s, v5.4s, v1.4s\n"
        "    fmul v2.4s, v3.4s, v6.4s\n"
        "    fmul v5.4s, v4.4s, v6.4s\n"
        "    fmls v2.4s, v4.4s, v7.4s\n"
        "    fmla v5.4s, v3.4s, v7.4s\n"
        "    fadd v3.4s, v0.4s, v2.4s\n"
        "    fadd v4.4s, v1.4s, v5.4s\n"
        "    fsub v0.4s, v0.4s, v2.4s\n"
        "    fsub v1.4s, v1.4s, v5.4s\n"
        "    zip1 v2.4s, v4.4s, v3.4s\n"
        "    zip2 v3.4s, v4.4s, v3.4s\n"
        "    zip1 v4.4s, v1.4s, v0.4s\n"
        "    zip2 v0.4s, v1.4s, v0.4s\n"
        "    stp q2, q3, [%[out]]\n"
        "    stp q4, q0, [%[out], #32]\n"
        : [in] "+r"(in), [out] "+r"(out), [consts] "+r"(consts)
        :
        : "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", "cc", "memory");
}

static const float onefft_neon_n16i_consts[24] ONEFFT_ALIGNED(64) = {
    0x1.0000000000000p+0f, 0x1.d906bcf328d46p-1f, 0x1.6a09e667f3bcdp-1f, 0x1.87de2a6aea964p-2f,
    0x0.0p+0f, -0x1.87de2a6aea963p-2f, -0x1.6a09e667f3bccp-1f, -0x1.d906bcf328d46p-1f,
    0x1.0000000000000p+0f, 0x1.6a09e667f3bcdp-1f, -0x0.0p+0f, -0x1.6a09e667f3bccp-1f,
    0x0.0p+0f, -0x1.6a09e667f3bccp-1f, -0x1.0000000000000p+0f, -0x1.6a09e667f3bcdp-1f,
    0x1.0000000000000p+0f, 0x1.87de2a6aea964p-2f, -0x1.6a09e667f3bccp-1f, -0x1.d906bcf328d47p-1f,
    0x0.0p+0f, -0x1.d906bcf328d46p-1f, -0x1.6a09e667f3bcdp-1f, 0x1.87de2a6aea961p-2f,
};
/* Complete 16 point transform from `in` to `out`, both interleaved. */
static void onefft_neon_n16i(const float *in, float *out) {
    const float *consts = onefft_neon_n16i_consts;
    __asm__ volatile(
        "    ldp q0, q1, [%[in]]\n"
        "    ldp q2, q3, [%[in], #32]\n"
        "    ldp q4, q5, [%[in], #64]\n"
        "    ldp q6, q7, [%[in], #96]\n"
        "    ldp q8, q9, [%[consts]]\n"
        "    ldp q10, q11, [%[consts], #32]\n"
        "    ldp q12, q13, [%[consts], #64]\n"
        "    uzp1 v14.4s, v0.4s, v1.4s\n"
        "    uzp2 v0.4s, v0.4s, v1.4s\n"
        "    uzp1 v1.4s, v2.4s, v3.4s\n"
        "    uzp2 v2.4s, v2.4s, v3.4s\n"
        "    uzp1 v3.4s, v4.4s, v5.4s\n"
        "    uzp2 v4.4s, v4.4s, v5.4s\n"
        "    uzp1 v5.4s, v6.4s, v7.4s\n"
        "    uzp2 v6.4s, v6.4s, v7.4s\n"
        "    fadd v7.4s, v0.4s, v4.4s\n"
        "    fadd v15.4s, v14.4s, v3.4s\n"
        "    fsub v0.4s, v0.4s, v4.4s\n"
        "    fsub v3.4s, v14.4s, v3.4s\n"
        "    fadd v4.4s, v2.4s, v6.4s\n"
        "    fadd v14.4s, v1.4s, v5.4s\n"
        "    fsub v2.4s, v2.4s, v6.4s\n"
        "    fsub v1.4s, v1.4s, v5.4s\n"
        "    fadd v5.4s, v7.4s, v4.4s\n"
        "    fadd v6.4s, v15.4s, v14.4s\n"
        "    fsub v4.4s, v7.4s, v4.4s\n"
        "    fsub v7.4s, v15.4s, v14.4s\n"
        "    fadd v14.4s, v0.4s, v1.4s\n"
        "    fsub v15.4s, v3.4s, v2.4s\n"
        "    fsub v0.4s, v0.4s, v1.4s\n"
        "    fadd v1.4s, v3.4s, v2.4s\n"
        "    trn1 v2.4s, v5.4s, v14.4s\n"
        "    trn2 v3.4s, v5.4s, v14.4s\n"
        "    trn1 v5.4s, v4.4s, v0.4s\n"
        "    trn2 v0.4s, v4.4s, v0.4s\n"
        "    trn1 v4.4s, v6.4s, v15.4s\n"
        "    trn2 v6.4s, v6.4s, v15.4s\n"
        "    trn1 v14.4s, v7.4s, v1.4s\n"
        "    trn2 v1.4s, v7.4s, v1.4s\n"
        "    zip1 v7.2d, v3.2d, v0.2d\n"
        "    zip2 v15.2d, v2.2d, v5.2d\n"
        "    zip2 v0.2d, v3.2d, v0.2d\n"
        "    zip1 v3.2d, v6.2d, v1.2d\n"
        "    zip2 v16.2d, v4.2d, v14.2d\n"
        "    zip2 v1.2d, v6.2d, v1.2d\n"
        "    fmul v6.4s, v7.4s, v8.4s\n"
        "    fmul v8.4s, v3.4s, v8.4s\n"
        "    fmul v17.4s, v15.4s, v10.4s\n"
        "    fmul v10.4s, v16.4s, v10.4s\n"
        "    fmul v18.4s, v0.4s, v12.4s\n"
        "    fmul v12.4s, v1.4s, v12.4s\n"
        "    fmls v6.4s, v3.4s, v9.4s\n"
        "    fmla v8.4s, v7.4s, v9.4s\n"
        "    fmls v17.4s, v16.4s, v11.4s\n"
        "    fmla v10.4s, v15.4s, v11.4s\n"
        "    fmls v18.4s, v1.4s, v13.4s\n"
        "    fmla v12.4s, v0.4s, v13.4s\n"
        "    zip1 v0.2d, v2.2d, v5.2d\n"
        "    zip1 v1.2d, v4.2d, v14.2d\n"
        "    fadd v2.4s, v0.4s, v17.4s\n"
        "    fadd v3.4s, v1.4s, v10.4s\n"
        "    fsub v0.4s, v0.4s, v17.4s\n"
        "    fsub v1.4s, v1.4s, v10.4s\n"
        "    fadd v4.4s, v6.4s, v18.4s\n"
        "    fadd v5.4s, v8.4s, v12.4s\n"
        "    fsub v6.4s, v6.4s, v18.4s\n"
        "    fsub v7.4s, v8.4s, v12.4s\n"
        "    fadd v8.4s, v2.4s, v4.4s\n"
        "    fadd v9.4s, v3.4s, v5.4s\n"
        "    fsub v2.4s, v2.4s, v4.4s\n"
        "    fsub v3.4s, v3.4s, v5.4s\n"
        "    fadd v4.4s, v0.4s, v7.4s\n"
        "    fsub v5.4s, v1.4s, v6.4s\n"
        "    fsub v0.4s, v0.4s, v7.4s\n"
        "    fadd v1.4s, v1.4s, v6.4s\n"
        "    zip1 v6.4s, v9.4s, v8.4s\n"
        "    zip2 v7.4s, v9.4s, v8.4s\n"
        "    zip1 v8.4s, v5.4s, v4.4s\n"
        "    zip2 v4.4s, v5.4s, v4.4s\n"
        "    zip1 v5.4s, v3.4s, v2.4s\n"
        "    zip2 v2.4s, v3.4s, v2.4s\n"
        "    stp q6, q7, [%[out]]\n"
        "    zip1 v3.4s, v1.4s, v0.4s\n"
        "    zip2 v0.4s, v1.4s, v0.4s\n"
        "    stp q8, q4, [%[out], #32]\n"
        "    stp q5, q2, [%[out], #64]\n"
        "    stp q3, q0, [%[out], #96]\n"
        : [in] "+r"(in), [out] "+r"(out), [consts] "+r"(consts)
        :
        : "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", "v10", "v11", "v12", "v13", "v14", "v15", "v16", "v17", "v18", "cc", "memory");
}

static const float onefft_neon_n32i_consts[48] ONEFFT_ALIGNED(64) = {
    0x1.0000000000000p+0f, 0x1.f6297cff75cb0p-1f, 0x1.d906bcf328d46p-1f, 0x1.a9b66290ea1a3p-1f,
    0x0.0p+0f, -0x1.8f8b83c69a60ap-3f, -0x1.87de2a6aea963p-2f, -0x1.1c73b39ae68c8p-1f,
    0x1.0000000000000p+0f, 0x1.d906bcf328d46p-1f, 0x1.6a09e667f3bcdp-1f, 0x1.87de2a6aea964p-2f,
    0x0.0p+0f, -0x1.87de2a6aea963p-2f, -0x1.6a09e667f3bccp-1f, -0x1.d906bcf328d46p-1f,
    0x1.0000000000000p+0f, 0x1.a9b66290ea1a3p-1f, 0x1.87de2a6aea964p-2f, -0x1.8f8b83c69a608p-3f,
    0x0.0p+0f, -0x1.1c73b39ae68c8p-1f, -0x1.d906bcf328d46p-1f, -0x1.f6297cff75cb0p-1f,
    0x1.6a09e667f3bcdp-1f, 0x1.1c73b39ae68c9p-1f, 0x1.87de2a6aea964p-2f, 0x1.8f8b83c69a60dp-3f,
    -0x1.6a09e667f3bccp-1f, -0x1.a9b66290ea1a3p-1f, -0x1.d906bcf328d46p-1f, -0x1.f6297cff75cb0p-1f,
    -0x0.0p+0f, -0x1.87de2a6aea962p-2f, -0x1.6a09e667f3bccp-1f, -0x1.d906bcf328d46p-1f,
    -0x1.0000000000000p+0f, -0x1.d906bcf328d46p-1f, -0x1.6a09e667f3bcdp-1f, -0x1.87de2a6aea965p-2f,
    -0x1.6a09e667f3bccp-1f, -0x1.f6297cff75cb0p-1f, -0x1.d906bcf328d47p-1f, -0x1.1c73b39ae68c8p-1f,
    -0x1.6a09e667f3bcdp-1f, -0x1.8f8b83c69a617p-3f, 0x1.87de2a6aea961p-2f, 0x1.a9b66290ea1a3p-1f,
};
/* Complete 32 point transform from `in` to `out`, both interleaved. */
static void onefft_neon_n32i(const float *in, float *out) {
    size_t bits;
    const float *consts = onefft_neon_n32i_consts;
    __asm__ volatile(
        "    ldp q0, q1, [%[in], #32]\n"
        "    ldp q2, q3, [%[in], #96]\n"
        "    ldp q4, q5, [%[in], #160]\n"
        "    ldp q6, q7, [%[in], #224]\n"
        "    ldp q8, q9, [%[in]]\n"
        "    ldp q10, q11, [%[in], #64]\n"
        "    ldp q12, q13, [%[in], #128]\n"
        "    ldp q14, q15, [%[in], #192]\n"
        "    mov %w[bits], #1267\n"
        "    movk %w[bits], #16181, lsl #16\n"
        "    dup v16.4s, %w[bits]\n"
        "    uzp1 v17.4s, v0.4s, v1.4s\n"
        "    uzp2 v0.4s, v0.4s, v1.4s\n"
        "    uzp1 v1.4s, v2.4s, v3.4s\n"
        "    uzp2 v2.4s, v2.4s, v3.4s\n"
        "    uzp1 v3.4s, v4.4s, v5.4s\n"
        "    uzp2 v4.4s, v4.4s, v5.4s\n"
        "    uzp1 v5.4s, v6.4s, v7.4s\n"
        "    uzp2 v6.4s, v6.4s, v7.4s\n"
        "    fsub v7.4s, v0.4s, v4.4s\n"
        "    fsub v18.4s, v17.4s, v3.4s\n"
        "    fsub v19.4s, v2.4s, v6.4s\n"
        "    fsub v20.4s, v1.4s, v5.4s\n"
        "    uzp1 v21.4s, v8.4s, v9.4s\n"
        "    uzp2 v8.4s, v8.4s, v9.4s\n"
        "    uzp1 v9.4s, v10.4s, v11.4s\n"
        "    uzp2 v10.4s, v10.4s, v11.4s\n"
        "    fadd v11.4s, v7.4s, v20.4s\n"
        "    fsub v22.4s, v18.4s, v19.4s\n"
        "    fsub v7.4s, v7.4s, v20.4s\n"
        "    fadd v18.4s, v18.4s, v19.4s\n"
        "    uzp1 v19.4s, v12.4s, v13.4s\n"
        "    uzp2 v12.4s, v12.4s, v13.4s\n"
        "    uzp1 v13.4s, v14.4s, v15.4s\n"
        "    uzp2 v14.4s, v14.4s, v15.4s\n"
        "    fadd v15.4s, v8.4s, v12.4s\n"
        "    fadd v20.4s, v21.4s, v19.4s\n"
        "    fsub v8.4s, v8.4s, v12.4s\n"
        "    fsub v12.4s, v21.4s, v19.4s\n"
        "    fadd v19.4s, v10.4s, v14.4s\n"
        "    fadd v21.4s, v9.4s, v13.4s\n"
        "    fsub v10.4s, v10.4s, v14.4s\n"
        "    fsub v9.4s, v9.4s, v13.4s\n"
        "    fadd v0.4s, v0.4s, v4.4s\n"
        "    fadd v3.4s, v17.4s, v3.4s\n"
        "    ldp q4, q13, [%[consts]]\n"
        "    fadd v2.4s, v2.4s, v6.4s\n"
        "    fadd v1.4s, v1.4s, v5.4s\n"
        "    fadd v5.4s, v11.4s, v22.4s\n"
        "    fsub v6.4s, v22.4s, v11.4s\n"
        "    ldp q11, q14, [%[consts], #32]\n"
        "    fadd v17.4s, v7.4s, v18.4s\n"
        "    fsub v7.4s, v18.4s, v7.4s\n"
        "    fadd v18.4s, v15.4s, v19.4s\n"
        "    fadd v22.4s, v20.4s, v21.4s\n"
        "    fsub v15.4s, v15.4s, v19.4s\n"
        "    fsub v19.4s, v20.4s, v21.4s\n"
        "    ldp q20, q21, [%[consts], #64]\n"
        "    fadd v23.4s, v8.4s, v9.4s\n"
        "    fsub v24.4s, v12.4s, v10.4s\n"
        "    fsub v8.4s, v8.4s, v9.4s\n"
        "    fadd v9.4s, v12.4s, v10.4s\n"
        "    fadd v10.4s, v0.4s, v2.4s\n"
        "    fadd v12.4s, v3.4s, v1.4s\n"
        "    fsub v0.4s, v0.4s, v2.4s\n"
        "    fsub v1.4s, v3.4s, v1.4s\n"
        "    fmul v2.4s, v5.4s, v16.4s\n"
        "    fmul v3.4s, v6.4s, v16.4s\n"
        "    fmul v5.4s, v17.4s, v16.4s\n"
        "    fmul v6.4s, v7.4s, v16.4s\n"
        "    fadd v7.4s, v18.4s, v10.4s\n"
        "    fadd v16.4s, v22.4s, v12.4s\n"
        "    fsub v10.4s, v18.4s, v10.4s\n"
        "    fsub v12.4s, v22.4s, v12.4s\n"
        "    fadd v17.4s, v23.4s, v2.4s\n"
        "    fadd v18.4s, v24.4s, v3.4s\n"
        "    fsub v2.4s, v23.4s, v2.4s\n"
        "    fsub v3.4s, v24.4s, v3.4s\n"
        "    fadd v22.4s, v15.4s, v1.4s\n"
        "    fsub v23.4s, v19.4s, v0.4s\n"
        "    fsub v1.4s, v15.4s, v1.4s\n"
        "    fadd v0.4s, v19.4s, v0.4s\n"
        "    fadd v15.4s, v8.4s, v6.4s\n"
        "    fsub v19.4s, v9.4s, v5.4s\n"
        "    fsub v6.4s, v8.4s, v6.4s\n"
        "    fadd v5.4s, v9.4s, v5.4s\n"
        "    trn1 v8.4s, v7.4s, v17.4s\n"
        "    trn2 v7.4s, v7.4s, v17.4s\n"
        "    trn1 v9.4s, v22.4s, v15.4s\n"
        "    trn2 v15.4s, v22.4s, v15.4s\n"
        "    trn1 v17.4s, v16.4s, v18.4s\n"
        "    trn2 v16.4s, v16.4s, v18.4s\n"
        "    trn1 v18.4s, v23.4s, v19.4s\n"
        "    trn2 v19.4s, v23.4s, v19.4s\n"
        "    trn1 v22.4s, v10.4s, v2.4s\n"
        "    trn2 v2.4s, v10.4s, v2.4s\n"
        "    trn1 v10.4s, v1.4s, v6.4s\n"
        "    trn2 v1.4s, v1.4s, v6.4s\n"
        "    trn1 v6.4s, v12.4s, v3.4s\n"
        "    trn2 v3.4s, v12.4s, v3.4s\n"
        "    zip1 v12.2d, v7.2d, v15.2d\n"
        "    zip2 v23.2d, v8.2d, v9.2d\n"
        "    zip2 v7.2d, v7.2d, v15.2d\n"
        "    zip1 v15.2d, v16.2d, v19.2d\n"
        "    zip2 v24.2d, v17.2d, v18.2d\n"
        "    zip2 v16.2d, v16.2d, v19.2d\n"
        "    zip1 v19.2d, v2.2d, v1.2d\n"
        "    zip2 v25.2d, v22.2d, v10.2d\n"
        "    zip2 v1.2d, v2.2d, v1.2d\n"
        "    fmul v2.4s, v12.4s, v4.4s\n"
        "    fmul v4.4s, v15.4s, v4.4s\n"
        "    fmul v26.4s, v23.4s, v11.4s\n"
        "    fmul v11.4s, v24.4s, v11.4s\n"
        "    fmul v27.4s, v7.4s, v20.4s\n"
        "    fmul v20.4s, v16.4s, v20.4s\n"
        "    fmls v2.4s, v15.4s, v13.4s\n"
        "    fmla v4.4s, v12.4s, v13.4s\n"
        "    fmls v26.4s, v24.4s, v14.4s\n"
        "    fmla v11.4s, v23.4s, v14.4s\n"
        "    fmls v27.4s, v16.4s, v21.4s\n"
        "    fmla v20.4s, v7.4s, v21.4s\n"
        "    zip1 v7.2d, v8.2d, v9.2d\n"
        "    trn1 v8.4s, v0.4s, v5.4s\n"
        "    trn2 v0.4s, v0.4s, v5.4s\n"
        "    ldp q5, q9, [%[consts], #96]\n"
        "    zip1 v12.2d, v17.2d, v18.2d\n"
        "    zip1 v10.2d, v22.2d, v10.2d\n"
        "    ldp q13, q14, [%[consts], #128]\n"
        "    ldp q15, q16, [%[consts], #160]\n"
        "    zip1 v17.2d, v3.2d, v0.2d\n"
        "    zip2 v18.2d, v6.2d, v8.2d\n"
        "    zip2 v0.2d, v3.2d, v0.2d\n"
        "    zip1 v3.2d, v6.2d, v8.2d\n"
        "    fmul v6.4s, v19.4s, v5.4s\n"
        "    fmul v5.4s, v17.4s, v5.4s\n"
        "    fadd v8.4s, v7.4s, v26.4s\n"
        "    fadd v21.4s, v12.4s, v11.4s\n"
        "    fmul v22.4s, v25.4s, v13.4s\n"
        "    fmul v13.4s, v18.4s, v13.4s\n"
        "    fmul v23.4s, v1.4s, v15.4s\n"
        "    fmul v15.4s, v0.4s, v15.4s\n"
        "    fmls v6.4s, v17.4s, v9.4s\n"
        "    fmla v5.4s, v19.4s, v9.4s\n"
        "    fmls v22.4s, v18.4s, v14.4s\n"
        "    fmla v13.4s, v25.4s, v14.4s\n"
        "    fmls v23.4s, v0.4s, v16.4s\n"
        "    fmla v15.4s, v1.4s, v16.4s\n"
        "    fsub v0.4s, v7.4s, v26.4s\n"
        "    fsub v1.4s, v12.4s, v11.4s\n"
        "    fadd v7.4s, v2.4s, v27.4s\n"
        "    fadd v9.4s, v4.4s, v20.4s\n"
        "    fsub v2.4s, v2.4s, v27.4s\n"
        "    fsub v4.4s, v4.4s, v20.4s\n"
        "    fadd v11.4s, v10.4s, v22.4s\n"
        "    fadd v12.4s, v3.4s, v13.4s\n"
        "    fsub v10.4s, v10.4s, v22.4s\n"
        "    fsub v3.4s, v3.4s, v13.4s\n"
        "    fadd v13.4s, v6.4s, v23.4s\n"
        "    fadd v14.4s, v5.4s, v15.4s\n"
        "    fsub v6.4s, v6.4s, v23.4s\n"
        "    fsub v5.4s, v5.4s, v15.4s\n"
        "    fadd v15.4s, v8.4s, v7.4s\n"
        "    fadd v16.4s, v21.4s, v9.4s\n"
        "    fsub v7.4s, v8.4s, v7.4s\n"
        "    fsub v8.4s, v21.4s, v9.4s\n"
        "    fadd v9.4s, v0.4s, v4.4s\n"
        "    fsub v17.4s, v1.4s, v2.4s\n"
        "    fsub v0.4s, v0.4s, v4.4s\n"
        "    fadd v1.4s, v1.4s, v2.4s\n"
        "    fadd v2.4s, v11.4s, v13.4s\n"
        "    fadd v4.4s, v12.4s, v14.4s\n"
        "    fsub v11.4s, v11.4s, v13.4s\n"
        "    fsub v12.4s, v12.4s, v14.4s\n"
        "    fadd v13.4s, v10.4s, v5.4s\n"
        "    fsub v14.4s, v3.4s, v6.4s\n"
        "    fsub v5.4s, v10.4s, v5.4s\n"
        "    fadd v3.4s, v3.4s, v6.4s\n"
        "    zip1 v6.4s, v16.4s, v15.4s\n"
        "    zip2 v10.4s, v16.4s, v15.4s\n"
        "    zip1 v15.4s, v17.4s, v9.4s\n"
        "    zip2 v9.4s, v17.4s, v9.4s\n"
        "    zip1 v16.4s, v8.4s, v7.4s\n"
        "    zip2 v7.4s, v8.4s, v7.4s\n"
        "    stp q6, q10, [%[out]]\n"
        "    zip1 v6.4s, v1.4s, v0.4s\n"
        "    zip2 v0.4s, v1.4s, v0.4s\n"
        "    stp q15, q9, [%[out], #64]\n"
        "    zip1 v1.4s, v4.4s, v2.4s\n"
        "    zip2 v2.4s, v4.4s, v2.4s\n"
        "    stp q16, q7, [%[out], #128]\n"
        "    zip1 v4.4s, v14.4s, v13.4s\n"
        "    zip2 v7.4s, v14.4s, v13.4s\n"
        "    stp q6, q0, [%[out], #192]\n"
        "    zip1 v0.4s, v12.4s, v11.4s\n"
        "    zip2 v6.4s, v12.4s, v11.4s\n"
        "    stp q1, q2, [%[out], #32]\n"
        "    zip1 v1.4s, v3.4s, v5.4s\n"
        "    zip2 v2.4s, v3.4s, v5.4s\n"
        "    stp q4, q7, [%[out], #96]\n"
        "    stp q0, q6, [%[out], #160]\n"
        "    stp q1, q2, [%[out], #224]\n"
        : [in] "+r"(in), [out] "+r"(out), [bits] "=&r"(bits), [consts] "+r"(consts)
        :
        : "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", "v10", "v11", "v12", "v13", "v14", "v15", "v16", "v17", "v18", "v19", "v20", "v21", "v22", "v23", "v24", "v25", "v26", "v27", "cc", "memory");
}

/* Last pass, radix 8, in place, from fused multiply-add butterflies: reads split vectors and writes the interleaved result.
 * Butterfly j < count uses the vectors at data + u*stride + 32*j. The last parameter is not used: it
 * exists because the last pass kernels of all targets share a signature. See the table builder in
 * onefft.c for the twiddle layout. */
static void onefft_neon_l8_fmai(float *data, const float *tw, size_t stride, size_t count, size_t regions) {
    (void)regions;
    size_t j;
    float *p1;
    float *p2;
    float *p3;
    float *p4;
    float *p5;
    float *p6;
    float *p7;
    __asm__ volatile(
        "    add %[p1], %[data], %[stride]\n"
        "    lsr %[j], %[count], #1\n"
        "    add %[p2], %[p1], %[stride]\n"
        "    add %[p3], %[p2], %[stride]\n"
        "    add %[p4], %[p3], %[stride]\n"
        "    add %[p5], %[p4], %[stride]\n"
        "    add %[p6], %[p5], %[stride]\n"
        "    add %[p7], %[p6], %[stride]\n"
        "1:\n"
        "    ldp q0, q1, [%[p1]]\n"
        "    ldp q2, q3, [%[p3]]\n"
        "    ldp q4, q5, [%[p5]]\n"
        "    ldp q6, q7, [%[p7]]\n"
        "    ldp q9, q8, [%[tw]]\n"
        "    ldp q10, q11, [%[data]]\n"
        "    ldp q12, q13, [%[p2]]\n"
        "    ldp q14, q15, [%[p4]]\n"
        "    ldp q16, q17, [%[p6]]\n"
        "    ldp q19, q18, [%[tw], #32]\n"
        "    mov v20.16b, v0.16b\n"
        "    mov v21.16b, v2.16b\n"
        "    mov v22.16b, v4.16b\n"
        "    mov v23.16b, v6.16b\n"
        "    mov v24.16b, v10.16b\n"
        "    mov v25.16b, v11.16b\n"
        "    mov v26.16b, v12.16b\n"
        "    mov v27.16b, v13.16b\n"
        "    mov v28.16b, v14.16b\n"
        "    mov v29.16b, v15.16b\n"
        "    mov v30.16b, v16.16b\n"
        "    mov v31.16b, v17.16b\n"
        "    fmls v0.4s, v1.4s, v8.s[0]\n"
        "    fmls v2.4s, v3.4s, v8.s[1]\n"
        "    fmla v1.4s, v20.4s, v8.s[0]\n"
        "    fmla v3.4s, v21.4s, v8.s[1]\n"
        "    fmls v4.4s, v5.4s, v8.s[2]\n"
        "    fmls v6.4s, v7.4s, v8.s[3]\n"
        "    ldp q21, q20, [%[tw], #64]\n"
        "    fmla v5.4s, v22.4s, v8.s[2]\n"
        "    fmla v7.4s, v23.4s, v8.s[3]\n"
        "    fmla v10.4s, v0.4s, v9.s[0]\n"
        "    fmla v11.4s, v1.4s, v9.s[0]\n"
        "    fmls v24.4s, v0.4s, v9.s[0]\n"
        "    fmls v25.4s, v1.4s, v9.s[0]\n"
        "    fmla v12.4s, v2.4s, v9.s[1]\n"
        "    fmla v13.4s, v3.4s, v9.s[1]\n"
        "    fmls v26.4s, v2.4s, v9.s[1]\n"
        "    fmls v27.4s, v3.4s, v9.s[1]\n"
        "    fmla v14.4s, v4.4s, v9.s[2]\n"
        "    fmla v15.4s, v5.4s, v9.s[2]\n"
        "    fmls v28.4s, v4.4s, v9.s[2]\n"
        "    fmls v29.4s, v5.4s, v9.s[2]\n"
        "    fmla v16.4s, v6.4s, v9.s[3]\n"
        "    fmla v17.4s, v7.4s, v9.s[3]\n"
        "    fmls v30.4s, v6.4s, v9.s[3]\n"
        "    fmls v31.4s, v7.4s, v9.s[3]\n"
        "    trn2 v0.4s, v10.4s, v12.4s\n"
        "    trn2 v1.4s, v11.4s, v13.4s\n"
        "    trn2 v2.4s, v14.4s, v16.4s\n"
        "    trn2 v3.4s, v15.4s, v17.4s\n"
        "    trn1 v4.4s, v10.4s, v12.4s\n"
        "    trn1 v5.4s, v14.4s, v16.4s\n"
        "    zip2 v6.2d, v0.2d, v2.2d\n"
        "    zip2 v7.2d, v1.2d, v3.2d\n"
        "    trn1 v8.4s, v11.4s, v13.4s\n"
        "    trn1 v9.4s, v15.4s, v17.4s\n"
        "    mov v10.16b, v6.16b\n"
        "    zip1 v0.2d, v0.2d, v2.2d\n"
        "    zip2 v2.2d, v4.2d, v5.2d\n"
        "    fmls v6.4s, v7.4s, v18.4s\n"
        "    zip1 v1.2d, v1.2d, v3.2d\n"
        "    fmla v7.4s, v10.4s, v18.4s\n"
        "    zip2 v3.2d, v8.2d, v9.2d\n"
        "    mov v10.16b, v2.16b\n"
        "    mov v11.16b, v0.16b\n"
        "    mov v12.16b, v1.16b\n"
        "    zip1 v4.2d, v4.2d, v5.2d\n"
        "    zip1 v5.2d, v8.2d, v9.2d\n"
        "    trn2 v8.4s, v24.4s, v26.4s\n"
        "    trn1 v9.4s, v24.4s, v26.4s\n"
        "    fmls v2.4s, v3.4s, v18.4s\n"
        "    fmla v0.4s, v6.4s, v19.4s\n"
        "    mov v13.16b, v4.16b\n"
        "    mov v14.16b, v5.16b\n"
        "    trn1 v15.4s, v28.4s, v30.4s\n"
        "    fmla v3.4s, v10.4s, v18.4s\n"
        "    trn2 v10.4s, v28.4s, v30.4s\n"
        "    fmla v1.4s, v7.4s, v19.4s\n"
        "    fmls v11.4s, v6.4s, v19.4s\n"
        "    fmls v12.4s, v7.4s, v19.4s\n"
        "    mov v6.16b, v0.16b\n"
        "    zip1 v7.2d, v8.2d, v10.2d\n"
        "    zip1 v16.2d, v9.2d, v15.2d\n"
        "    fmla v4.4s, v2.4s, v19.4s\n"
        "    fmla v5.4s, v3.4s, v19.4s\n"
        "    zip2 v8.2d, v8.2d, v10.2d\n"
        "    zip2 v9.2d, v9.2d, v15.2d\n"
        "    mov v10.16b, v11.16b\n"
        "    fmls v13.4s, v2.4s, v19.4s\n"
        "    fmls v14.4s, v3.4s, v19.4s\n"
        "    trn2 v2.4s, v25.4s, v27.4s\n"
        "    trn2 v3.4s, v29.4s, v31.4s\n"
        "    ldp q17, q15, [%[tw], #96]\n"
        "    mov v22.16b, v9.16b\n"
        "    mov v23.16b, v4.16b\n"
        "    mov v24.16b, v5.16b\n"
        "    trn1 v25.4s, v25.4s, v27.4s\n"
        "    trn1 v26.4s, v29.4s, v31.4s\n"
        "    zip2 v27.2d, v2.2d, v3.2d\n"
        "    zip1 v2.2d, v2.2d, v3.2d\n"
        "    mov v3.16b, v13.16b\n"
        "    mov v28.16b, v14.16b\n"
        "    zip2 v29.2d, v25.2d, v26.2d\n"
        "    zip1 v25.2d, v25.2d, v26.2d\n"
        "    fmls v0.4s, v1.4s, v20.4s\n"
        "    fmls v11.4s, v12.4s, v20.4s\n"
        "    fmls v9.4s, v29.4s, v18.4s\n"
        "    fmla v1.4s, v6.4s, v20.4s\n"
        "    mov v6.16b, v16.16b\n"
        "    fmla v29.4s, v22.4s, v18.4s\n"
        "    fmla v12.4s, v10.4s, v20.4s\n"
        "    mov v10.16b, v25.16b\n"
        "    fmla v4.4s, v0.4s, v21.4s\n"
        "    fmls v23.4s, v0.4s, v21.4s\n"
        "    fmls v25.4s, v9.4s, v19.4s\n"
        "    fmla v5.4s, v1.4s, v21.4s\n"
        "    mov v0.16b, v8.16b\n"
        "    fmla v16.4s, v29.4s, v19.4s\n"
        "    fmls v6.4s, v29.4s, v19.4s\n"
        "    fmla v10.4s, v9.4s, v19.4s\n"
        "    fmls v24.4s, v1.4s, v21.4s\n"
        "    fmls v8.4s, v27.4s, v18.4s\n"
        "    mov v1.16b, v7.16b\n"
        "    mov v9.16b, v2.16b\n"
        "    fmla v13.4s, v12.4s, v21.4s\n"
        "    fmla v27.4s, v0.4s, v18.4s\n"
        "    fmls v14.4s, v11.4s, v21.4s\n"
        "    fmls v3.4s, v12.4s, v21.4s\n"
        "    fmla v28.4s, v11.4s, v21.4s\n"
        "    fmls v2.4s, v8.4s, v19.4s\n"
        "    fmla v9.4s, v8.4s, v19.4s\n"
        "    fmla v7.4s, v27.4s, v19.4s\n"
        "    fmls v1.4s, v27.4s, v19.4s\n"
        "    zip1 v0.4s, v5.4s, v4.4s\n"
        "    zip2 v4.4s, v5.4s, v4.4s\n"
        "    mov v5.16b, v16.16b\n"
        "    zip1 v8.4s, v14.4s, v13.4s\n"
        "    zip2 v11.4s, v14.4s, v13.4s\n"
        "    mov v12.16b, v7.16b\n"
        "    zip1 v13.4s, v24.4s, v23.4s\n"
        "    zip2 v14.4s, v24.4s, v23.4s\n"
        "    stp q0, q4, [%[data]]\n"
        "    fmls v7.4s, v2.4s, v15.4s\n"
        "    mov v0.16b, v25.16b\n"
        "    zip1 v4.4s, v28.4s, v3.4s\n"
        "    stp q8, q11, [%[p2]]\n"
        "    fmla v2.4s, v12.4s, v15.4s\n"
        "    mov v8.16b, v1.16b\n"
        "    zip2 v3.4s, v28.4s, v3.4s\n"
        "    stp q13, q14, [%[p4]]\n"
        "    fmls v1.4s, v9.4s, v15.4s\n"
        "    mov v11.16b, v6.16b\n"
        "    fmla v9.4s, v8.4s, v15.4s\n"
        "    fmla v16.4s, v7.4s, v17.4s\n"
        "    stp q4, q3, [%[p6]]\n"
        "    fmla v25.4s, v2.4s, v17.4s\n"
        "    fmls v5.4s, v7.4s, v17.4s\n"
        "    mov v3.16b, v10.16b\n"
        "    add %[data], %[data], #32\n"
        "    add %[p2], %[p2], #32\n"
        "    add %[p4], %[p4], #32\n"
        "    add %[p6], %[p6], #32\n"
        "    add %[tw], %[tw], #128\n"
        "    subs %[j], %[j], #1\n"
        "    fmls v0.4s, v2.4s, v17.4s\n"
        "    fmls v10.4s, v1.4s, v17.4s\n"
        "    fmla v6.4s, v9.4s, v17.4s\n"
        "    fmls v11.4s, v9.4s, v17.4s\n"
        "    fmla v3.4s, v1.4s, v17.4s\n"
        "    zip1 v1.4s, v25.4s, v16.4s\n"
        "    zip2 v2.4s, v25.4s, v16.4s\n"
        "    zip1 v4.4s, v0.4s, v5.4s\n"
        "    zip1 v7.4s, v10.4s, v6.4s\n"
        "    zip2 v6.4s, v10.4s, v6.4s\n"
        "    zip2 v0.4s, v0.4s, v5.4s\n"
        "    zip1 v5.4s, v3.4s, v11.4s\n"
        "    stp q1, q2, [%[p1]]\n"
        "    zip2 v1.4s, v3.4s, v11.4s\n"
        "    stp q7, q6, [%[p3]]\n"
        "    add %[p1], %[p1], #32\n"
        "    stp q4, q0, [%[p5]]\n"
        "    add %[p3], %[p3], #32\n"
        "    stp q5, q1, [%[p7]]\n"
        "    add %[p5], %[p5], #32\n"
        "    add %[p7], %[p7], #32\n"
        "    b.ne 1b\n"
        "    lsr %[j], %[count], #1\n"
        "2:\n"
        "    ldp q1, q0, [%[p1]]\n"
        "    ldp q3, q2, [%[p3]]\n"
        "    ldp q5, q4, [%[p5]]\n"
        "    ldp q7, q6, [%[p7]]\n"
        "    ldp q9, q8, [%[tw]]\n"
        "    ldp q10, q11, [%[data]]\n"
        "    ldp q12, q13, [%[p2]]\n"
        "    ldp q14, q15, [%[p4]]\n"
        "    ldp q16, q17, [%[p6]]\n"
        "    ldp q19, q18, [%[tw], #32]\n"
        "    mov v20.16b, v1.16b\n"
        "    mov v21.16b, v3.16b\n"
        "    mov v22.16b, v5.16b\n"
        "    mov v23.16b, v7.16b\n"
        "    mov v24.16b, v10.16b\n"
        "    mov v25.16b, v11.16b\n"
        "    mov v26.16b, v12.16b\n"
        "    mov v27.16b, v13.16b\n"
        "    mov v28.16b, v14.16b\n"
        "    mov v29.16b, v15.16b\n"
        "    mov v30.16b, v16.16b\n"
        "    mov v31.16b, v17.16b\n"
        "    fmls v1.4s, v0.4s, v8.s[0]\n"
        "    fmls v3.4s, v2.4s, v8.s[1]\n"
        "    fmla v0.4s, v20.4s, v8.s[0]\n"
        "    fmla v2.4s, v21.4s, v8.s[1]\n"
        "    fmls v5.4s, v4.4s, v8.s[2]\n"
        "    fmls v7.4s, v6.4s, v8.s[3]\n"
        "    ldp q21, q20, [%[tw], #64]\n"
        "    fmla v4.4s, v22.4s, v8.s[2]\n"
        "    fmla v6.4s, v23.4s, v8.s[3]\n"
        "    fmla v10.4s, v0.4s, v9.s[0]\n"
        "    fmls v11.4s, v1.4s, v9.s[0]\n"
        "    fmls v24.4s, v0.4s, v9.s[0]\n"
        "    fmla v25.4s, v1.4s, v9.s[0]\n"
        "    fmla v12.4s, v2.4s, v9.s[1]\n"
        "    fmls v13.4s, v3.4s, v9.s[1]\n"
        "    fmls v26.4s, v2.4s, v9.s[1]\n"
        "    fmla v27.4s, v3.4s, v9.s[1]\n"
        "    fmla v14.4s, v4.4s, v9.s[2]\n"
        "    fmls v15.4s, v5.4s, v9.s[2]\n"
        "    fmls v28.4s, v4.4s, v9.s[2]\n"
        "    fmla v29.4s, v5.4s, v9.s[2]\n"
        "    fmla v16.4s, v6.4s, v9.s[3]\n"
        "    fmls v17.4s, v7.4s, v9.s[3]\n"
        "    fmls v30.4s, v6.4s, v9.s[3]\n"
        "    fmla v31.4s, v7.4s, v9.s[3]\n"
        "    trn2 v0.4s, v10.4s, v12.4s\n"
        "    trn2 v1.4s, v11.4s, v13.4s\n"
        "    trn2 v2.4s, v14.4s, v16.4s\n"
        "    trn2 v3.4s, v15.4s, v17.4s\n"
        "    trn1 v4.4s, v10.4s, v12.4s\n"
        "    trn1 v5.4s, v14.4s, v16.4s\n"
        "    zip2 v6.2d, v0.2d, v2.2d\n"
        "    zip2 v7.2d, v1.2d, v3.2d\n"
        "    trn1 v8.4s, v11.4s, v13.4s\n"
        "    trn1 v9.4s, v15.4s, v17.4s\n"
        "    mov v10.16b, v6.16b\n"
        "    zip1 v0.2d, v0.2d, v2.2d\n"
        "    zip2 v2.2d, v4.2d, v5.2d\n"
        "    fmls v6.4s, v7.4s, v18.4s\n"
        "    zip1 v1.2d, v1.2d, v3.2d\n"
        "    fmla v7.4s, v10.4s, v18.4s\n"
        "    zip2 v3.2d, v8.2d, v9.2d\n"
        "    mov v10.16b, v2.16b\n"
        "    mov v11.16b, v0.16b\n"
        "    mov v12.16b, v1.16b\n"
        "    zip1 v4.2d, v4.2d, v5.2d\n"
        "    zip1 v5.2d, v8.2d, v9.2d\n"
        "    trn2 v8.4s, v24.4s, v26.4s\n"
        "    trn1 v9.4s, v24.4s, v26.4s\n"
        "    fmls v2.4s, v3.4s, v18.4s\n"
        "    fmla v0.4s, v6.4s, v19.4s\n"
        "    mov v13.16b, v4.16b\n"
        "    mov v14.16b, v5.16b\n"
        "    trn1 v15.4s, v28.4s, v30.4s\n"
        "    fmla v3.4s, v10.4s, v18.4s\n"
        "    trn2 v10.4s, v28.4s, v30.4s\n"
        "    fmla v1.4s, v7.4s, v19.4s\n"
        "    fmls v11.4s, v6.4s, v19.4s\n"
        "    fmls v12.4s, v7.4s, v19.4s\n"
        "    mov v6.16b, v0.16b\n"
        "    zip1 v7.2d, v8.2d, v10.2d\n"
        "    zip1 v16.2d, v9.2d, v15.2d\n"
        "    fmla v4.4s, v2.4s, v19.4s\n"
        "    fmla v5.4s, v3.4s, v19.4s\n"
        "    zip2 v8.2d, v8.2d, v10.2d\n"
        "    zip2 v9.2d, v9.2d, v15.2d\n"
        "    mov v10.16b, v11.16b\n"
        "    fmls v13.4s, v2.4s, v19.4s\n"
        "    fmls v14.4s, v3.4s, v19.4s\n"
        "    trn2 v2.4s, v25.4s, v27.4s\n"
        "    trn2 v3.4s, v29.4s, v31.4s\n"
        "    ldp q17, q15, [%[tw], #96]\n"
        "    mov v22.16b, v9.16b\n"
        "    mov v23.16b, v4.16b\n"
        "    mov v24.16b, v5.16b\n"
        "    trn1 v25.4s, v25.4s, v27.4s\n"
        "    trn1 v26.4s, v29.4s, v31.4s\n"
        "    zip2 v27.2d, v2.2d, v3.2d\n"
        "    zip1 v2.2d, v2.2d, v3.2d\n"
        "    mov v3.16b, v13.16b\n"
        "    mov v28.16b, v14.16b\n"
        "    zip2 v29.2d, v25.2d, v26.2d\n"
        "    zip1 v25.2d, v25.2d, v26.2d\n"
        "    fmls v0.4s, v1.4s, v20.4s\n"
        "    fmls v11.4s, v12.4s, v20.4s\n"
        "    fmls v9.4s, v29.4s, v18.4s\n"
        "    fmla v1.4s, v6.4s, v20.4s\n"
        "    mov v6.16b, v16.16b\n"
        "    fmla v29.4s, v22.4s, v18.4s\n"
        "    fmla v12.4s, v10.4s, v20.4s\n"
        "    mov v10.16b, v25.16b\n"
        "    fmla v4.4s, v0.4s, v21.4s\n"
        "    fmls v23.4s, v0.4s, v21.4s\n"
        "    fmls v25.4s, v9.4s, v19.4s\n"
        "    fmla v5.4s, v1.4s, v21.4s\n"
        "    mov v0.16b, v8.16b\n"
        "    fmla v16.4s, v29.4s, v19.4s\n"
        "    fmls v6.4s, v29.4s, v19.4s\n"
        "    fmla v10.4s, v9.4s, v19.4s\n"
        "    fmls v24.4s, v1.4s, v21.4s\n"
        "    fmls v8.4s, v27.4s, v18.4s\n"
        "    mov v1.16b, v7.16b\n"
        "    mov v9.16b, v2.16b\n"
        "    fmla v13.4s, v12.4s, v21.4s\n"
        "    fmla v27.4s, v0.4s, v18.4s\n"
        "    fmls v14.4s, v11.4s, v21.4s\n"
        "    fmls v3.4s, v12.4s, v21.4s\n"
        "    fmla v28.4s, v11.4s, v21.4s\n"
        "    fmls v2.4s, v8.4s, v19.4s\n"
        "    fmla v9.4s, v8.4s, v19.4s\n"
        "    fmla v7.4s, v27.4s, v19.4s\n"
        "    fmls v1.4s, v27.4s, v19.4s\n"
        "    zip1 v0.4s, v5.4s, v4.4s\n"
        "    zip2 v4.4s, v5.4s, v4.4s\n"
        "    mov v5.16b, v16.16b\n"
        "    zip1 v8.4s, v14.4s, v13.4s\n"
        "    zip2 v11.4s, v14.4s, v13.4s\n"
        "    mov v12.16b, v7.16b\n"
        "    zip1 v13.4s, v24.4s, v23.4s\n"
        "    zip2 v14.4s, v24.4s, v23.4s\n"
        "    stp q0, q4, [%[data]]\n"
        "    fmls v7.4s, v2.4s, v15.4s\n"
        "    mov v0.16b, v25.16b\n"
        "    zip1 v4.4s, v28.4s, v3.4s\n"
        "    stp q8, q11, [%[p2]]\n"
        "    fmla v2.4s, v12.4s, v15.4s\n"
        "    mov v8.16b, v1.16b\n"
        "    zip2 v3.4s, v28.4s, v3.4s\n"
        "    stp q13, q14, [%[p4]]\n"
        "    fmls v1.4s, v9.4s, v15.4s\n"
        "    mov v11.16b, v6.16b\n"
        "    fmla v9.4s, v8.4s, v15.4s\n"
        "    fmla v16.4s, v7.4s, v17.4s\n"
        "    stp q4, q3, [%[p6]]\n"
        "    fmla v25.4s, v2.4s, v17.4s\n"
        "    fmls v5.4s, v7.4s, v17.4s\n"
        "    mov v3.16b, v10.16b\n"
        "    add %[data], %[data], #32\n"
        "    add %[p2], %[p2], #32\n"
        "    add %[p4], %[p4], #32\n"
        "    add %[p6], %[p6], #32\n"
        "    add %[tw], %[tw], #128\n"
        "    subs %[j], %[j], #1\n"
        "    fmls v0.4s, v2.4s, v17.4s\n"
        "    fmls v10.4s, v1.4s, v17.4s\n"
        "    fmla v6.4s, v9.4s, v17.4s\n"
        "    fmls v11.4s, v9.4s, v17.4s\n"
        "    fmla v3.4s, v1.4s, v17.4s\n"
        "    zip1 v1.4s, v25.4s, v16.4s\n"
        "    zip2 v2.4s, v25.4s, v16.4s\n"
        "    zip1 v4.4s, v0.4s, v5.4s\n"
        "    zip1 v7.4s, v10.4s, v6.4s\n"
        "    zip2 v6.4s, v10.4s, v6.4s\n"
        "    zip2 v0.4s, v0.4s, v5.4s\n"
        "    zip1 v5.4s, v3.4s, v11.4s\n"
        "    stp q1, q2, [%[p1]]\n"
        "    zip2 v1.4s, v3.4s, v11.4s\n"
        "    stp q7, q6, [%[p3]]\n"
        "    add %[p1], %[p1], #32\n"
        "    stp q4, q0, [%[p5]]\n"
        "    add %[p3], %[p3], #32\n"
        "    stp q5, q1, [%[p7]]\n"
        "    add %[p5], %[p5], #32\n"
        "    add %[p7], %[p7], #32\n"
        "    b.ne 2b\n"
        : [data] "+r"(data), [tw] "+r"(tw), [stride] "+r"(stride), [count] "+r"(count), [j] "=&r"(j), [p1] "=&r"(p1), [p2] "=&r"(p2), [p3] "=&r"(p3), [p4] "=&r"(p4), [p5] "=&r"(p5), [p6] "=&r"(p6), [p7] "=&r"(p7)
        :
        : "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", "v10", "v11", "v12", "v13", "v14", "v15", "v16", "v17", "v18", "v19", "v20", "v21", "v22", "v23", "v24", "v25", "v26", "v27", "v28", "v29", "v30", "v31", "cc", "memory");
}

/* Last pass, radix 4, in place, from fused multiply-add butterflies: reads split vectors and writes the interleaved result.
 * Butterfly j < count uses the vectors at data + u*stride + 32*j. The last parameter is not used: it
 * exists because the last pass kernels of all targets share a signature. See the table builder in
 * onefft.c for the twiddle layout. */
static void onefft_neon_l4_fmai(float *data, const float *tw, size_t stride, size_t count, size_t regions) {
    (void)regions;
    size_t j;
    float *p1;
    float *p2;
    float *p3;
    __asm__ volatile(
        "    add %[p1], %[data], %[stride]\n"
        "    lsr %[j], %[count], #1\n"
        "    add %[p2], %[p1], %[stride]\n"
        "    add %[p3], %[p2], %[stride]\n"
        "1:\n"
        "    ldp q0, q1, [%[data]]\n"
        "    ldp q2, q3, [%[p1]]\n"
        "    ldp q4, q5, [%[p2]]\n"
        "    ldp q6, q7, [%[p3]]\n"
        "    ldp q9, q8, [%[tw]]\n"
        "    ldp q11, q10, [%[tw], #32]\n"
        "    trn2 v12.4s, v0.4s, v2.4s\n"
        "    trn2 v13.4s, v4.4s, v6.4s\n"
        "    trn2 v14.4s, v1.4s, v3.4s\n"
        "    trn2 v15.4s, v5.4s, v7.4s\n"
        "    zip2 v16.2d, v12.2d, v13.2d\n"
        "    trn1 v0.4s, v0.4s, v2.4s\n"
        "    zip2 v2.2d, v14.2d, v15.2d\n"
        "    trn1 v4.4s, v4.4s, v6.4s\n"
        "    trn1 v1.4s, v1.4s, v3.4s\n"
        "    trn1 v3.4s, v5.4s, v7.4s\n"
        "    mov v5.16b, v16.16b\n"
        "    fmls v16.4s, v2.4s, v8.4s\n"
        "    zip1 v6.2d, v12.2d, v13.2d\n"
        "    fmla v2.4s, v5.4s, v8.4s\n"
        "    zip2 v5.2d, v0.2d, v4.2d\n"
        "    zip1 v7.2d, v14.2d, v15.2d\n"
        "    zip2 v12.2d, v1.2d, v3.2d\n"
        "    mov v13.16b, v6.16b\n"
        "    mov v14.16b, v5.16b\n"
        "    fmla v6.4s, v16.4s, v9.4s\n"
        "    fmls v13.4s, v16.4s, v9.4s\n"
        "    fmls v5.4s, v12.4s, v8.4s\n"
        "    mov v15.16b, v7.16b\n"
        "    zip1 v0.2d, v0.2d, v4.2d\n"
        "    fmla v12.4s, v14.4s, v8.4s\n"
        "    fmla v7.4s, v2.4s, v9.4s\n"
        "    add %[tw], %[tw], #64\n"
        "    subs %[j], %[j], #1\n"
        "    fmls v15.4s, v2.4s, v9.4s\n"
        "    zip1 v1.2d, v1.2d, v3.2d\n"
        "    mov v2.16b, v0.16b\n"
        "    mov v3.16b, v6.16b\n"
        "    mov v4.16b, v13.16b\n"
        "    fmla v0.4s, v5.4s, v9.4s\n"
        "    fmls v2.4s, v5.4s, v9.4s\n"
        "    mov v5.16b, v1.16b\n"
        "    fmls v6.4s, v7.4s, v10.4s\n"
        "    fmla v1.4s, v12.4s, v9.4s\n"
        "    fmls v5.4s, v12.4s, v9.4s\n"
        "    fmla v7.4s, v3.4s, v10.4s\n"
        "    fmls v13.4s, v15.4s, v10.4s\n"
        "    mov v3.16b, v0.16b\n"
        "    mov v8.16b, v2.16b\n"
        "    fmla v15.4s, v4.4s, v10.4s\n"
        "    fmla v0.4s, v6.4s, v11.4s\n"
        "    mov v4.16b, v1.16b\n"
        "    fmls v3.4s, v6.4s, v11.4s\n"
        "    mov v6.16b, v5.16b\n"
        "    fmla v1.4s, v7.4s, v11.4s\n"
        "    fmls v4.4s, v7.4s, v11.4s\n"
        "    fmla v2.4s, v15.4s, v11.4s\n"
        "    fmls v5.4s, v13.4s, v11.4s\n"
        "    fmls v8.4s, v15.4s, v11.4s\n"
        "    fmla v6.4s, v13.4s, v11.4s\n"
        "    zip1 v7.4s, v1.4s, v0.4s\n"
        "    zip2 v0.4s, v1.4s, v0.4s\n"
        "    zip1 v1.4s, v5.4s, v2.4s\n"
        "    zip2 v2.4s, v5.4s, v2.4s\n"
        "    zip1 v5.4s, v4.4s, v3.4s\n"
        "    zip2 v3.4s, v4.4s, v3.4s\n"
        "    stp q7, q0, [%[data]]\n"
        "    zip1 v0.4s, v6.4s, v8.4s\n"
        "    zip2 v4.4s, v6.4s, v8.4s\n"
        "    stp q1, q2, [%[p1]]\n"
        "    add %[data], %[data], #32\n"
        "    stp q5, q3, [%[p2]]\n"
        "    add %[p1], %[p1], #32\n"
        "    stp q0, q4, [%[p3]]\n"
        "    add %[p2], %[p2], #32\n"
        "    add %[p3], %[p3], #32\n"
        "    b.ne 1b\n"
        "    lsr %[j], %[count], #1\n"
        "2:\n"
        "    ldp q0, q1, [%[data]]\n"
        "    ldp q2, q3, [%[p1]]\n"
        "    ldp q4, q5, [%[p2]]\n"
        "    ldp q6, q7, [%[p3]]\n"
        "    ldp q9, q8, [%[tw]]\n"
        "    ldp q11, q10, [%[tw], #32]\n"
        "    trn2 v12.4s, v0.4s, v2.4s\n"
        "    trn2 v13.4s, v4.4s, v6.4s\n"
        "    trn2 v14.4s, v1.4s, v3.4s\n"
        "    trn2 v15.4s, v5.4s, v7.4s\n"
        "    zip2 v16.2d, v12.2d, v13.2d\n"
        "    trn1 v0.4s, v0.4s, v2.4s\n"
        "    zip2 v2.2d, v14.2d, v15.2d\n"
        "    trn1 v4.4s, v4.4s, v6.4s\n"
        "    trn1 v1.4s, v1.4s, v3.4s\n"
        "    trn1 v3.4s, v5.4s, v7.4s\n"
        "    mov v5.16b, v16.16b\n"
        "    fmls v16.4s, v2.4s, v8.4s\n"
        "    zip1 v6.2d, v12.2d, v13.2d\n"
        "    fmla v2.4s, v5.4s, v8.4s\n"
        "    zip2 v5.2d, v0.2d, v4.2d\n"
        "    zip1 v7.2d, v14.2d, v15.2d\n"
        "    zip2 v12.2d, v1.2d, v3.2d\n"
        "    mov v13.16b, v6.16b\n"
        "    mov v14.16b, v5.16b\n"
        "    zip1 v0.2d, v0.2d, v4.2d\n"
        "    zip1 v1.2d, v1.2d, v3.2d\n"
        "    fmls v5.4s, v12.4s, v8.4s\n"
        "    mov v3.16b, v7.16b\n"
        "    fmla v6.4s, v2.4s, v9.4s\n"
        "    add %[tw], %[tw], #64\n"
        "    subs %[j], %[j], #1\n"
        "    fmla v12.4s, v14.4s, v8.4s\n"
        "    fmls v7.4s, v16.4s, v9.4s\n"
        "    mov v4.16b, v0.16b\n"
        "    mov v8.16b, v1.16b\n"
        "    fmls v13.4s, v2.4s, v9.4s\n"
        "    fmla v3.4s, v16.4s, v9.4s\n"
        "    fmls v1.4s, v5.4s, v9.4s\n"
        "    fmla v8.4s, v5.4s, v9.4s\n"
        "    mov v2.16b, v6.16b\n"
        "    fmla v0.4s, v12.4s, v9.4s\n"
        "    fmls v4.4s, v12.4s, v9.4s\n"
        "    fmls v6.4s, v7.4s, v10.4s\n"
        "    mov v5.16b, v13.16b\n"
        "    fmla v7.4s, v2.4s, v10.4s\n"
        "    fmls v13.4s, v3.4s, v10.4s\n"
        "    mov v2.16b, v1.16b\n"
        "    mov v9.16b, v8.16b\n"
        "    fmla v3.4s, v5.4s, v10.4s\n"
        "    mov v5.16b, v0.16b\n"
        "    mov v10.16b, v4.16b\n"
        "    fmla v0.4s, v6.4s, v11.4s\n"
        "    fmls v5.4s, v6.4s, v11.4s\n"
        "    fmla v1.4s, v7.4s, v11.4s\n"
        "    fmls v2.4s, v7.4s, v11.4s\n"
        "    fmla v4.4s, v3.4s, v11.4s\n"
        "    fmls v8.4s, v13.4s, v11.4s\n"
        "    fmls v10.4s, v3.4s, v11.4s\n"
        "    fmla v9.4s, v13.4s, v11.4s\n"
        "    zip1 v3.4s, v1.4s, v0.4s\n"
        "    zip2 v0.4s, v1.4s, v0.4s\n"
        "    zip1 v1.4s, v8.4s, v4.4s\n"
        "    zip2 v4.4s, v8.4s, v4.4s\n"
        "    zip1 v6.4s, v2.4s, v5.4s\n"
        "    zip2 v2.4s, v2.4s, v5.4s\n"
        "    stp q3, q0, [%[data]]\n"
        "    zip1 v0.4s, v9.4s, v10.4s\n"
        "    zip2 v3.4s, v9.4s, v10.4s\n"
        "    stp q1, q4, [%[p1]]\n"
        "    add %[data], %[data], #32\n"
        "    stp q6, q2, [%[p2]]\n"
        "    add %[p1], %[p1], #32\n"
        "    stp q0, q3, [%[p3]]\n"
        "    add %[p2], %[p2], #32\n"
        "    add %[p3], %[p3], #32\n"
        "    b.ne 2b\n"
        : [data] "+r"(data), [tw] "+r"(tw), [stride] "+r"(stride), [count] "+r"(count), [j] "=&r"(j), [p1] "=&r"(p1), [p2] "=&r"(p2), [p3] "=&r"(p3)
        :
        : "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", "v10", "v11", "v12", "v13", "v14", "v15", "v16", "cc", "memory");
}

/* Middle pass, radix 8, in place on split vectors. `data` holds `regions` regions of `blocks`
 * blocks. Vector u of butterfly j of a block is at u*args.s_high + 32*j, for j < args.inner; see
 * passes.py for how the blocks and regions follow each other. Butterfly j multiplies input i (bit
 * reversed u) by w_j^i and transforms across the vectors. The table `tw` holds per butterfly the real
 * parts of w^1..w^7 padded to 8 floats, then the imaginary parts. */
static void onefft_neon_c8(float *data, const float *tw, const size_t *args, size_t blocks, size_t regions) {
    size_t stride;
    float *p1;
    float *p2;
    float *p3;
    float *p4;
    float *p5;
    float *p6;
    float *p7;
    size_t bits;
    const float *twp;
    size_t j;
    size_t b;
    size_t scratch;
    __asm__ volatile(
        "    ldr %[stride], [%[args], #0]\n"
        "    mov %w[bits], #1267\n"
        "    add %[p1], %[data], %[stride]\n"
        "    movk %w[bits], #16181, lsl #16\n"
        "    add %[p2], %[p1], %[stride]\n"
        "    dup v0.4s, %w[bits]\n"
        "    add %[p3], %[p2], %[stride]\n"
        "    add %[p4], %[p3], %[stride]\n"
        "    add %[p5], %[p4], %[stride]\n"
        "    add %[p6], %[p5], %[stride]\n"
        "    add %[p7], %[p6], %[stride]\n"
        "1:\n"
        "    mov %[b], %[blocks]\n"
        "2:\n"
        "    ldr %[scratch], [%[args], #16]\n"
        "    mov %[twp], %[tw]\n"
        "    lsr %[j], %[scratch], #1\n"
        "3:\n"
        "    ldp q2, q1, [%[p4]]\n"
        "    ldp q4, q3, [%[p5]]\n"
        "    ldp q6, q5, [%[p6]]\n"
        "    ldp q8, q7, [%[p7]]\n"
        "    ldp q9, q10, [%[twp]]\n"
        "    ldp q11, q12, [%[twp], #32]\n"
        "    ldp q14, q13, [%[p1]]\n"
        "    ldp q16, q15, [%[p2]]\n"
        "    ldp q18, q17, [%[p3]]\n"
        "    ldp q19, q20, [%[data]]\n"
        "    fmul v21.4s, v2.4s, v9.s[0]\n"
        "    fmul v22.4s, v1.4s, v9.s[0]\n"
        "    fmul v23.4s, v6.4s, v9.s[2]\n"
        "    fmul v24.4s, v5.4s, v9.s[2]\n"
        "    fmul v25.4s, v4.4s, v10.s[0]\n"
        "    fmul v26.4s, v3.4s, v10.s[0]\n"
        "    fmls v21.4s, v1.4s, v11.s[0]\n"
        "    fmla v22.4s, v2.4s, v11.s[0]\n"
        "    fmls v23.4s, v5.4s, v11.s[2]\n"
        "    fmla v24.4s, v6.4s, v11.s[2]\n"
        "    fmls v25.4s, v3.4s, v12.s[0]\n"
        "    fmla v26.4s, v4.4s, v12.s[0]\n"
        "    fmul v1.4s, v16.4s, v9.s[1]\n"
        "    fmul v2.4s, v15.4s, v9.s[1]\n"
        "    fmul v3.4s, v14.4s, v9.s[3]\n"
        "    fmul v4.4s, v13.4s, v9.s[3]\n"
        "    fmul v5.4s, v18.4s, v10.s[1]\n"
        "    fmul v6.4s, v17.4s, v10.s[1]\n"
        "    fmls v1.4s, v15.4s, v11.s[1]\n"
        "    fmla v2.4s, v16.4s, v11.s[1]\n"
        "    fmul v9.4s, v8.4s, v10.s[2]\n"
        "    fmul v10.4s, v7.4s, v10.s[2]\n"
        "    fmls v3.4s, v13.4s, v11.s[3]\n"
        "    fmla v4.4s, v14.4s, v11.s[3]\n"
        "    fsub v11.4s, v21.4s, v25.4s\n"
        "    fmls v5.4s, v17.4s, v12.s[1]\n"
        "    fmls v9.4s, v7.4s, v12.s[2]\n"
        "    fmla v10.4s, v8.4s, v12.s[2]\n"
        "    fsub v7.4s, v22.4s, v26.4s\n"
        "    fmla v6.4s, v18.4s, v12.s[1]\n"
        "    fadd v8.4s, v19.4s, v3.4s\n"
        "    fadd v12.4s, v20.4s, v4.4s\n"
        "    fsub v13.4s, v23.4s, v9.4s\n"
        "    fsub v14.4s, v24.4s, v10.4s\n"
        "    fsub v3.4s, v19.4s, v3.4s\n"
        "    fsub v4.4s, v20.4s, v4.4s\n"
        "    fadd v15.4s, v1.4s, v5.4s\n"
        "    fadd v16.4s, v2.4s, v6.4s\n"
        "    fsub v1.4s, v1.4s, v5.4s\n"
        "    fsub v2.4s, v2.4s, v6.4s\n"
        "    fadd v5.4s, v21.4s, v25.4s\n"
        "    fadd v6.4s, v22.4s, v26.4s\n"
        "    fadd v17.4s, v11.4s, v14.4s\n"
        "    fadd v9.4s, v23.4s, v9.4s\n"
        "    fsub v18.4s, v7.4s, v13.4s\n"
        "    fadd v10.4s, v24.4s, v10.4s\n"
        "    fsub v11.4s, v11.4s, v14.4s\n"
        "    fadd v7.4s, v7.4s, v13.4s\n"
        "    fadd v13.4s, v8.4s, v15.4s\n"
        "    fadd v14.4s, v12.4s, v16.4s\n"
        "    fadd v19.4s, v17.4s, v18.4s\n"
        "    fsub v17.4s, v18.4s, v17.4s\n"
        "    fsub v8.4s, v8.4s, v15.4s\n"
        "    fsub v12.4s, v12.4s, v16.4s\n"
        "    fadd v15.4s, v11.4s, v7.4s\n"
        "    fadd v16.4s, v3.4s, v2.4s\n"
        "    fsub v7.4s, v7.4s, v11.4s\n"
        "    fsub v11.4s, v4.4s, v1.4s\n"
        "    fsub v2.4s, v3.4s, v2.4s\n"
        "    fadd v1.4s, v4.4s, v1.4s\n"
        "    fadd v3.4s, v5.4s, v9.4s\n"
        "    fadd v4.4s, v6.4s, v10.4s\n"
        "    fsub v5.4s, v5.4s, v9.4s\n"
        "    fsub v6.4s, v6.4s, v10.4s\n"
        "    fmul v9.4s, v19.4s, v0.4s\n"
        "    fmul v10.4s, v17.4s, v0.4s\n"
        "    fmul v15.4s, v15.4s, v0.4s\n"
        "    fmul v7.4s, v7.4s, v0.4s\n"
        "    fadd v17.4s, v13.4s, v3.4s\n"
        "    fadd v18.4s, v14.4s, v4.4s\n"
        "    fsub v3.4s, v13.4s, v3.4s\n"
        "    fsub v4.4s, v14.4s, v4.4s\n"
        "    fadd v13.4s, v16.4s, v9.4s\n"
        "    fadd v14.4s, v11.4s, v10.4s\n"
        "    fsub v9.4s, v16.4s, v9.4s\n"
        "    fsub v10.4s, v11.4s, v10.4s\n"
        "    stp q17, q18, [%[data]]\n"
        "    ldp q11, q16, [%[data], #32]\n"
        "    fadd v17.4s, v8.4s, v6.4s\n"
        "    fsub v18.4s, v12.4s, v5.4s\n"
        "    stp q3, q4, [%[p4]]\n"
        "    ldp q4, q3, [%[p1], #32]\n"
        "    ldp q20, q19, [%[p2], #32]\n"
        "    fsub v6.4s, v8.4s, v6.4s\n"
        "    fadd v5.4s, v12.4s, v5.4s\n"
        "    stp q13, q14, [%[p1]]\n"
        "    ldp q12, q8, [%[p4], #32]\n"
        "    ldp q14, q13, [%[p3], #32]\n"
        "    fadd v21.4s, v2.4s, v7.4s\n"
        "    fsub v22.4s, v1.4s, v15.4s\n"
        "    stp q9, q10, [%[p5]]\n"
        "    ldp q10, q9, [%[p5], #32]\n"
        "    ldp q24, q23, [%[p6], #32]\n"
        "    fsub v2.4s, v2.4s, v7.4s\n"
        "    fadd v1.4s, v1.4s, v15.4s\n"
        "    stp q17, q18, [%[p2]]\n"
        "    ldp q15, q7, [%[p7], #32]\n"
        "    ldp q17, q18, [%[twp], #64]\n"
        "    ldp q25, q26, [%[twp], #96]\n"
        "    stp q6, q5, [%[p6]]\n"
        "    stp q21, q22, [%[p3]]\n"
        "    stp q2, q1, [%[p7]]\n"
        "    fmul v1.4s, v12.4s, v17.s[0]\n"
        "    fmul v2.4s, v8.4s, v17.s[0]\n"
        "    fmul v5.4s, v24.4s, v17.s[2]\n"
        "    fmul v6.4s, v23.4s, v17.s[2]\n"
        "    fmul v21.4s, v10.4s, v18.s[0]\n"
        "    fmul v22.4s, v9.4s, v18.s[0]\n"
        "    fmls v1.4s, v8.4s, v25.s[0]\n"
        "    fmla v2.4s, v12.4s, v25.s[0]\n"
        "    fmls v5.4s, v23.4s, v25.s[2]\n"
        "    fmla v6.4s, v24.4s, v25.s[2]\n"
        "    fmls v21.4s, v9.4s, v26.s[0]\n"
        "    fmla v22.4s, v10.4s, v26.s[0]\n"
        "    fmul v8.4s, v20.4s, v17.s[1]\n"
        "    fmul v9.4s, v19.4s, v17.s[1]\n"
        "    fmul v10.4s, v4.4s, v17.s[3]\n"
        "    fmul v12.4s, v3.4s, v17.s[3]\n"
        "    fmul v17.4s, v14.4s, v18.s[1]\n"
        "    fmul v23.4s, v13.4s, v18.s[1]\n"
        "    fmls v8.4s, v19.4s, v25.s[1]\n"
        "    fmla v9.4s, v20.4s, v25.s[1]\n"
        "    fmul v19.4s, v15.4s, v18.s[2]\n"
        "    fmul v18.4s, v7.4s, v18.s[2]\n"
        "    fmls v10.4s, v3.4s, v25.s[3]\n"
        "    fmla v12.4s, v4.4s, v25.s[3]\n"
        "    fsub v3.4s, v1.4s, v21.4s\n"
        "    fmls v17.4s, v13.4s, v26.s[1]\n"
        "    fmls v19.4s, v7.4s, v26.s[2]\n"
        "    fmla v18.4s, v15.4s, v26.s[2]\n"
        "    fsub v4.4s, v2.4s, v22.4s\n"
        "    fmla v23.4s, v14.4s, v26.s[1]\n"
        "    fadd v7.4s, v11.4s, v10.4s\n"
        "    fadd v13.4s, v16.4s, v12.4s\n"
        "    fsub v14.4s, v5.4s, v19.4s\n"
        "    fsub v15.4s, v6.4s, v18.4s\n"
        "    fsub v10.4s, v11.4s, v10.4s\n"
        "    fsub v11.4s, v16.4s, v12.4s\n"
        "    fadd v12.4s, v8.4s, v17.4s\n"
        "    fadd v16.4s, v9.4s, v23.4s\n"
        "    fsub v8.4s, v8.4s, v17.4s\n"
        "    fsub v9.4s, v9.4s, v23.4s\n"
        "    fadd v1.4s, v1.4s, v21.4s\n"
        "    fadd v2.4s, v2.4s, v22.4s\n"
        "    fadd v17.4s, v3.4s, v15.4s\n"
        "    fadd v5.4s, v5.4s, v19.4s\n"
        "    fsub v19.4s, v4.4s, v14.4s\n"
        "    fadd v6.4s, v6.4s, v18.4s\n"
        "    fsub v3.4s, v3.4s, v15.4s\n"
        "    fadd v4.4s, v4.4s, v14.4s\n"
        "    fadd v14.4s, v7.4s, v12.4s\n"
        "    fadd v15.4s, v13.4s, v16.4s\n"
        "    fadd v18.4s, v17.4s, v19.4s\n"
        "    fsub v17.4s, v19.4s, v17.4s\n"
        "    fsub v7.4s, v7.4s, v12.4s\n"
        "    fsub v12.4s, v13.4s, v16.4s\n"
        "    fadd v13.4s, v3.4s, v4.4s\n"
        "    fadd v16.4s, v10.4s, v9.4s\n"
        "    fsub v3.4s, v4.4s, v3.4s\n"
        "    fsub v4.4s, v11.4s, v8.4s\n"
        "    fsub v9.4s, v10.4s, v9.4s\n"
        "    fadd v8.4s, v11.4s, v8.4s\n"
        "    fadd v10.4s, v1.4s, v5.4s\n"
        "    fadd v11.4s, v2.4s, v6.4s\n"
        "    fsub v1.4s, v1.4s, v5.4s\n"
        "    fsub v2.4s, v2.4s, v6.4s\n"
        "    fmul v5.4s, v18.4s, v0.4s\n"
        "    fmul v6.4s, v17.4s, v0.4s\n"
        "    fmul v13.4s, v13.4s, v0.4s\n"
        "    fmul v3.4s, v3.4s, v0.4s\n"
        "    fadd v17.4s, v14.4s, v10.4s\n"
        "    fadd v18.4s, v15.4s, v11.4s\n"
        "    fsub v10.4s, v14.4s, v10.4s\n"
        "    fsub v11.4s, v15.4s, v11.4s\n"
        "    fadd v14.4s, v16.4s, v5.4s\n"
        "    fadd v15.4s, v4.4s, v6.4s\n"
        "    fsub v5.4s, v16.4s, v5.4s\n"
        "    fsub v4.4s, v4.4s, v6.4s\n"
        "    stp q17, q18, [%[data], #32]\n"
        "    fadd v6.4s, v7.4s, v2.4s\n"
        "    fsub v16.4s, v12.4s, v1.4s\n"
        "    stp q10, q11, [%[p4], #32]\n"
        "    add %[data], %[data], #64\n"
        "    fsub v2.4s, v7.4s, v2.4s\n"
        "    fadd v1.4s, v12.4s, v1.4s\n"
        "    stp q14, q15, [%[p1], #32]\n"
        "    fadd v7.4s, v9.4s, v3.4s\n"
        "    fsub v10.4s, v8.4s, v13.4s\n"
        "    stp q5, q4, [%[p5], #32]\n"
        "    add %[p1], %[p1], #64\n"
        "    add %[p4], %[p4], #64\n"
        "    fsub v3.4s, v9.4s, v3.4s\n"
        "    fadd v4.4s, v8.4s, v13.4s\n"
        "    stp q6, q16, [%[p2], #32]\n"
        "    add %[p5], %[p5], #64\n"
        "    stp q2, q1, [%[p6], #32]\n"
        "    add %[p2], %[p2], #64\n"
        "    add %[twp], %[twp], #128\n"
        "    subs %[j], %[j], #1\n"
        "    stp q7, q10, [%[p3], #32]\n"
        "    add %[p6], %[p6], #64\n"
        "    stp q3, q4, [%[p7], #32]\n"
        "    add %[p3], %[p3], #64\n"
        "    add %[p7], %[p7], #64\n"
        "    b.ne 3b\n"
        "    ldr %[scratch], [%[args], #24]\n"
        "    subs %[b], %[b], #1\n"
        "    add %[data], %[data], %[scratch]\n"
        "    add %[p1], %[p1], %[scratch]\n"
        "    add %[p2], %[p2], %[scratch]\n"
        "    add %[p3], %[p3], %[scratch]\n"
        "    add %[p4], %[p4], %[scratch]\n"
        "    add %[p5], %[p5], %[scratch]\n"
        "    add %[p6], %[p6], %[scratch]\n"
        "    add %[p7], %[p7], %[scratch]\n"
        "    b.ne 2b\n"
        "    ldr %[scratch], [%[args], #32]\n"
        "    subs %[regions], %[regions], #1\n"
        "    add %[data], %[data], %[scratch]\n"
        "    add %[p1], %[p1], %[scratch]\n"
        "    add %[p2], %[p2], %[scratch]\n"
        "    add %[p3], %[p3], %[scratch]\n"
        "    add %[p4], %[p4], %[scratch]\n"
        "    add %[p5], %[p5], %[scratch]\n"
        "    add %[p6], %[p6], %[scratch]\n"
        "    add %[p7], %[p7], %[scratch]\n"
        "    ldr %[scratch], [%[args], #40]\n"
        "    add %[tw], %[tw], %[scratch]\n"
        "    b.ne 1b\n"
        : [data] "+r"(data), [tw] "+r"(tw), [args] "+r"(args), [blocks] "+r"(blocks), [regions] "+r"(regions), [stride] "=&r"(stride), [p1] "=&r"(p1), [p2] "=&r"(p2), [p3] "=&r"(p3), [p4] "=&r"(p4), [p5] "=&r"(p5), [p6] "=&r"(p6), [p7] "=&r"(p7), [bits] "=&r"(bits), [twp] "=&r"(twp), [j] "=&r"(j), [b] "=&r"(b), [scratch] "=&r"(scratch)
        :
        : "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", "v10", "v11", "v12", "v13", "v14", "v15", "v16", "v17", "v18", "v19", "v20", "v21", "v22", "v23", "v24", "v25", "v26", "cc", "memory");
}

/* Middle pass, radix 4, in place on split vectors. `data` holds `regions` regions of `blocks`
 * blocks. Vector u of butterfly j of a block is at u*args.s_high + 32*j, for j < args.inner; see
 * passes.py for how the blocks and regions follow each other. Butterfly j multiplies input i (bit
 * reversed u) by w_j^i and transforms across the vectors. The table `tw` holds per butterfly the real
 * parts of w^1..w^3 padded to 4 floats, then the imaginary parts. */
static void onefft_neon_c4(float *data, const float *tw, const size_t *args, size_t blocks, size_t regions) {
    size_t stride;
    float *p1;
    float *p2;
    float *p3;
    const float *twp;
    size_t j;
    size_t b;
    size_t scratch;
    __asm__ volatile(
        "    ldr %[stride], [%[args], #0]\n"
        "    add %[p1], %[data], %[stride]\n"
        "    add %[p2], %[p1], %[stride]\n"
        "    add %[p3], %[p2], %[stride]\n"
        "1:\n"
        "    mov %[b], %[blocks]\n"
        "2:\n"
        "    ldr %[scratch], [%[args], #16]\n"
        "    mov %[twp], %[tw]\n"
        "    lsr %[j], %[scratch], #1\n"
        "3:\n"
        "    ldp q1, q0, [%[p1]]\n"
        "    ldp q3, q2, [%[p2]]\n"
        "    ldp q5, q4, [%[p3]]\n"
        "    ldp q6, q7, [%[twp]]\n"
        "    ldp q9, q8, [%[p1], #32]\n"
        "    ldp q11, q10, [%[p2], #32]\n"
        "    ldp q13, q12, [%[p3], #32]\n"
        "    ldp q14, q15, [%[twp], #32]\n"
        "    ldp q16, q17, [%[data]]\n"
        "    ldp q18, q19, [%[data], #32]\n"
        "    fmul v20.4s, v3.4s, v6.s[0]\n"
        "    fmul v21.4s, v2.4s, v6.s[0]\n"
        "    fmul v22.4s, v1.4s, v6.s[1]\n"
        "    fmul v23.4s, v0.4s, v6.s[1]\n"
        "    fmul v24.4s, v5.4s, v6.s[2]\n"
        "    fmul v6.4s, v4.4s, v6.s[2]\n"
        "    fmul v25.4s, v11.4s, v14.s[0]\n"
        "    fmul v26.4s, v10.4s, v14.s[0]\n"
        "    fmul v27.4s, v9.4s, v14.s[1]\n"
        "    fmul v28.4s, v8.4s, v14.s[1]\n"
        "    fmul v29.4s, v13.4s, v14.s[2]\n"
        "    fmul v14.4s, v12.4s, v14.s[2]\n"
        "    fmls v20.4s, v2.4s, v7.s[0]\n"
        "    fmla v21.4s, v3.4s, v7.s[0]\n"
        "    fmls v22.4s, v0.4s, v7.s[1]\n"
        "    fmla v23.4s, v1.4s, v7.s[1]\n"
        "    fmls v24.4s, v4.4s, v7.s[2]\n"
        "    fmla v6.4s, v5.4s, v7.s[2]\n"
        "    add %[twp], %[twp], #64\n"
        "    subs %[j], %[j], #1\n"
        "    fmls v25.4s, v10.4s, v15.s[0]\n"
        "    fmla v26.4s, v11.4s, v15.s[0]\n"
        "    fmls v27.4s, v8.4s, v15.s[1]\n"
        "    fmla v28.4s, v9.4s, v15.s[1]\n"
        "    fmls v29.4s, v12.4s, v15.s[2]\n"
        "    fmla v14.4s, v13.4s, v15.s[2]\n"
        "    fadd v0.4s, v16.4s, v22.4s\n"
        "    fadd v1.4s, v17.4s, v23.4s\n"
        "    fsub v2.4s, v16.4s, v22.4s\n"
        "    fsub v3.4s, v17.4s, v23.4s\n"
        "    fadd v4.4s, v20.4s, v24.4s\n"
        "    fadd v5.4s, v21.4s, v6.4s\n"
        "    fsub v7.4s, v20.4s, v24.4s\n"
        "    fsub v6.4s, v21.4s, v6.4s\n"
        "    fadd v8.4s, v18.4s, v27.4s\n"
        "    fadd v9.4s, v19.4s, v28.4s\n"
        "    fsub v10.4s, v18.4s, v27.4s\n"
        "    fsub v11.4s, v19.4s, v28.4s\n"
        "    fadd v12.4s, v25.4s, v29.4s\n"
        "    fadd v13.4s, v26.4s, v14.4s\n"
        "    fsub v15.4s, v25.4s, v29.4s\n"
        "    fsub v14.4s, v26.4s, v14.4s\n"
        "    fadd v16.4s, v0.4s, v4.4s\n"
        "    fadd v17.4s, v1.4s, v5.4s\n"
        "    fsub v0.4s, v0.4s, v4.4s\n"
        "    fsub v1.4s, v1.4s, v5.4s\n"
        "    fadd v4.4s, v2.4s, v6.4s\n"
        "    fsub v5.4s, v3.4s, v7.4s\n"
        "    fsub v2.4s, v2.4s, v6.4s\n"
        "    fadd v3.4s, v3.4s, v7.4s\n"
        "    stp q16, q17, [%[data]]\n"
        "    fadd v6.4s, v8.4s, v12.4s\n"
        "    fadd v7.4s, v9.4s, v13.4s\n"
        "    stp q0, q1, [%[p2]]\n"
        "    fsub v0.4s, v8.4s, v12.4s\n"
        "    fsub v1.4s, v9.4s, v13.4s\n"
        "    stp q4, q5, [%[p1]]\n"
        "    fadd v4.4s, v10.4s, v14.4s\n"
        "    fsub v5.4s, v11.4s, v15.4s\n"
        "    stp q2, q3, [%[p3]]\n"
        "    fsub v2.4s, v10.4s, v14.4s\n"
        "    fadd v3.4s, v11.4s, v15.4s\n"
        "    stp q6, q7, [%[data], #32]\n"
        "    stp q0, q1, [%[p2], #32]\n"
        "    add %[data], %[data], #64\n"
        "    stp q4, q5, [%[p1], #32]\n"
        "    add %[p2], %[p2], #64\n"
        "    stp q2, q3, [%[p3], #32]\n"
        "    add %[p1], %[p1], #64\n"
        "    add %[p3], %[p3], #64\n"
        "    b.ne 3b\n"
        "    ldr %[scratch], [%[args], #24]\n"
        "    subs %[b], %[b], #1\n"
        "    add %[data], %[data], %[scratch]\n"
        "    add %[p1], %[p1], %[scratch]\n"
        "    add %[p2], %[p2], %[scratch]\n"
        "    add %[p3], %[p3], %[scratch]\n"
        "    b.ne 2b\n"
        "    ldr %[scratch], [%[args], #32]\n"
        "    subs %[regions], %[regions], #1\n"
        "    add %[data], %[data], %[scratch]\n"
        "    add %[p1], %[p1], %[scratch]\n"
        "    add %[p2], %[p2], %[scratch]\n"
        "    add %[p3], %[p3], %[scratch]\n"
        "    ldr %[scratch], [%[args], #40]\n"
        "    add %[tw], %[tw], %[scratch]\n"
        "    b.ne 1b\n"
        : [data] "+r"(data), [tw] "+r"(tw), [args] "+r"(args), [blocks] "+r"(blocks), [regions] "+r"(regions), [stride] "=&r"(stride), [p1] "=&r"(p1), [p2] "=&r"(p2), [p3] "=&r"(p3), [twp] "=&r"(twp), [j] "=&r"(j), [b] "=&r"(b), [scratch] "=&r"(scratch)
        :
        : "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", "v10", "v11", "v12", "v13", "v14", "v15", "v16", "v17", "v18", "v19", "v20", "v21", "v22", "v23", "v24", "v25", "v26", "v27", "v28", "v29", "cc", "memory");
}

/* Middle pass, radix 8, in place on split vectors, from fused multiply-add butterflies. `data` holds `regions` regions of `blocks`
 * blocks. Vector u of butterfly j of a block is at u*args.s_high + 32*j, for j < args.inner; see
 * passes.py for how the blocks and regions follow each other. 
 * The table `tw` holds per butterfly the cosines of its 4 twiddles, then their tangents, see the
 * table builder in onefft.c. */
static void onefft_neon_c8_fma(float *data, const float *tw, const size_t *args, size_t blocks, size_t regions) {
    size_t stride;
    float *p1;
    float *p2;
    float *p3;
    float *p4;
    float *p5;
    float *p6;
    float *p7;
    const float *twp;
    size_t j;
    size_t b;
    size_t scratch;
    __asm__ volatile(
        "    ldr %[stride], [%[args], #0]\n"
        "    add %[p1], %[data], %[stride]\n"
        "    add %[p2], %[p1], %[stride]\n"
        "    add %[p3], %[p2], %[stride]\n"
        "    add %[p4], %[p3], %[stride]\n"
        "    add %[p5], %[p4], %[stride]\n"
        "    add %[p6], %[p5], %[stride]\n"
        "    add %[p7], %[p6], %[stride]\n"
        "1:\n"
        "    mov %[b], %[blocks]\n"
        "2:\n"
        "    ldr %[scratch], [%[args], #16]\n"
        "    mov %[twp], %[tw]\n"
        "    lsr %[j], %[scratch], #1\n"
        "3:\n"
        "    ldp q0, q1, [%[p7]]\n"
        "    ldp q3, q2, [%[twp]]\n"
        "    ldp q4, q5, [%[p3]]\n"
        "    ldp q6, q7, [%[p5]]\n"
        "    ldp q8, q9, [%[p6]]\n"
        "    ldp q10, q11, [%[p1]]\n"
        "    ldp q12, q13, [%[p2]]\n"
        "    ldp q14, q15, [%[p4]]\n"
        "    ldp q16, q17, [%[data]]\n"
        "    mov v18.16b, v0.16b\n"
        "    mov v19.16b, v6.16b\n"
        "    mov v20.16b, v4.16b\n"
        "    mov v21.16b, v8.16b\n"
        "    mov v22.16b, v9.16b\n"
        "    mov v23.16b, v10.16b\n"
        "    mov v24.16b, v14.16b\n"
        "    mov v25.16b, v15.16b\n"
        "    mov v26.16b, v12.16b\n"
        "    mov v27.16b, v13.16b\n"
        "    mov v28.16b, v16.16b\n"
        "    mov v29.16b, v17.16b\n"
        "    fmls v0.4s, v1.4s, v2.s[0]\n"
        "    fmls v6.4s, v7.4s, v2.s[0]\n"
        "    fmla v1.4s, v18.4s, v2.s[0]\n"
        "    fmla v7.4s, v19.4s, v2.s[0]\n"
        "    fmls v4.4s, v5.4s, v2.s[0]\n"
        "    fmls v10.4s, v11.4s, v2.s[0]\n"
        "    fmla v5.4s, v20.4s, v2.s[0]\n"
        "    fmla v8.4s, v0.4s, v3.s[0]\n"
        "    fmla v9.4s, v1.4s, v3.s[0]\n"
        "    fmls v21.4s, v0.4s, v3.s[0]\n"
        "    fmls v22.4s, v1.4s, v3.s[0]\n"
        "    fmla v11.4s, v23.4s, v2.s[0]\n"
        "    fmla v14.4s, v6.4s, v3.s[0]\n"
        "    fmla v15.4s, v7.4s, v3.s[0]\n"
        "    mov v0.16b, v8.16b\n"
        "    fmls v24.4s, v6.4s, v3.s[0]\n"
        "    fmls v25.4s, v7.4s, v3.s[0]\n"
        "    mov v1.16b, v21.16b\n"
        "    fmla v12.4s, v4.4s, v3.s[0]\n"
        "    fmla v13.4s, v5.4s, v3.s[0]\n"
        "    fmls v26.4s, v4.4s, v3.s[0]\n"
        "    fmls v27.4s, v5.4s, v3.s[0]\n"
        "    mov v4.16b, v14.16b\n"
        "    mov v5.16b, v15.16b\n"
        "    fmls v8.4s, v9.4s, v2.s[1]\n"
        "    fmls v21.4s, v22.4s, v2.s[1]\n"
        "    mov v6.16b, v24.16b\n"
        "    mov v7.16b, v25.16b\n"
        "    fmla v9.4s, v0.4s, v2.s[1]\n"
        "    fmla v22.4s, v1.4s, v2.s[1]\n"
        "    mov v0.16b, v12.16b\n"
        "    fmla v16.4s, v10.4s, v3.s[0]\n"
        "    fmla v17.4s, v11.4s, v3.s[0]\n"
        "    mov v1.16b, v26.16b\n"
        "    fmls v28.4s, v10.4s, v3.s[0]\n"
        "    fmls v29.4s, v11.4s, v3.s[0]\n"
        "    fmls v12.4s, v13.4s, v2.s[1]\n"
        "    fmla v14.4s, v8.4s, v3.s[1]\n"
        "    fmla v13.4s, v0.4s, v2.s[1]\n"
        "    fmla v15.4s, v9.4s, v3.s[1]\n"
        "    mov v0.16b, v16.16b\n"
        "    mov v10.16b, v17.16b\n"
        "    fmls v4.4s, v8.4s, v3.s[1]\n"
        "    fmls v5.4s, v9.4s, v3.s[1]\n"
        "    mov v8.16b, v28.16b\n"
        "    mov v9.16b, v29.16b\n"
        "    fmls v26.4s, v27.4s, v2.s[1]\n"
        "    fmla v24.4s, v22.4s, v3.s[1]\n"
        "    mov v11.16b, v14.16b\n"
        "    fmla v27.4s, v1.4s, v2.s[1]\n"
        "    fmls v25.4s, v21.4s, v3.s[1]\n"
        "    fmls v6.4s, v22.4s, v3.s[1]\n"
        "    fmla v7.4s, v21.4s, v3.s[1]\n"
        "    mov v1.16b, v4.16b\n"
        "    fmla v16.4s, v12.4s, v3.s[1]\n"
        "    fmla v17.4s, v13.4s, v3.s[1]\n"
        "    mov v18.16b, v24.16b\n"
        "    fmls v0.4s, v12.4s, v3.s[1]\n"
        "    fmls v10.4s, v13.4s, v3.s[1]\n"
        "    fmla v28.4s, v27.4s, v3.s[1]\n"
        "    fmls v29.4s, v26.4s, v3.s[1]\n"
        "    mov v12.16b, v6.16b\n"
        "    fmls v8.4s, v27.4s, v3.s[1]\n"
        "    fmla v9.4s, v26.4s, v3.s[1]\n"
        "    mov v13.16b, v16.16b\n"
        "    mov v19.16b, v17.16b\n"
        "    add %[twp], %[twp], #32\n"
        "    subs %[j], %[j], #1\n"
        "    fmls v14.4s, v15.4s, v2.s[2]\n"
        "    fmls v4.4s, v5.4s, v2.s[2]\n"
        "    mov v20.16b, v0.16b\n"
        "    mov v21.16b, v10.16b\n"
        "    fmla v15.4s, v11.4s, v2.s[2]\n"
        "    fmla v5.4s, v1.4s, v2.s[2]\n"
        "    mov v1.16b, v28.16b\n"
        "    mov v11.16b, v29.16b\n"
        "    fmls v24.4s, v25.4s, v2.s[3]\n"
        "    fmls v6.4s, v7.4s, v2.s[3]\n"
        "    mov v22.16b, v8.16b\n"
        "    mov v23.16b, v9.16b\n"
        "    fmla v25.4s, v18.4s, v2.s[3]\n"
        "    fmla v7.4s, v12.4s, v2.s[3]\n"
        "    fmla v16.4s, v14.4s, v3.s[2]\n"
        "    fmla v17.4s, v15.4s, v3.s[2]\n"
        "    fmls v13.4s, v14.4s, v3.s[2]\n"
        "    fmls v19.4s, v15.4s, v3.s[2]\n"
        "    fmla v0.4s, v5.4s, v3.s[2]\n"
        "    fmls v10.4s, v4.4s, v3.s[2]\n"
        "    fmls v20.4s, v5.4s, v3.s[2]\n"
        "    fmla v21.4s, v4.4s, v3.s[2]\n"
        "    stp q16, q17, [%[data]]\n"
        "    fmla v28.4s, v24.4s, v3.s[3]\n"
        "    fmla v29.4s, v25.4s, v3.s[3]\n"
        "    stp q13, q19, [%[p4]]\n"
        "    add %[data], %[data], #32\n"
        "    fmls v1.4s, v24.4s, v3.s[3]\n"
        "    fmls v11.4s, v25.4s, v3.s[3]\n"
        "    stp q0, q10, [%[p2]]\n"
        "    add %[p4], %[p4], #32\n"
        "    fmla v8.4s, v7.4s, v3.s[3]\n"
        "    fmls v9.4s, v6.4s, v3.s[3]\n"
        "    stp q20, q21, [%[p6]]\n"
        "    add %[p2], %[p2], #32\n"
        "    fmls v22.4s, v7.4s, v3.s[3]\n"
        "    fmla v23.4s, v6.4s, v3.s[3]\n"
        "    stp q28, q29, [%[p1]]\n"
        "    add %[p6], %[p6], #32\n"
        "    stp q1, q11, [%[p5]]\n"
        "    add %[p1], %[p1], #32\n"
        "    stp q8, q9, [%[p3]]\n"
        "    add %[p5], %[p5], #32\n"
        "    stp q22, q23, [%[p7]]\n"
        "    add %[p3], %[p3], #32\n"
        "    add %[p7], %[p7], #32\n"
        "    b.ne 3b\n"
        "    lsr %[j], %[scratch], #1\n"
        "4:\n"
        "    ldp q1, q0, [%[p7]]\n"
        "    ldp q3, q2, [%[twp]]\n"
        "    ldp q5, q4, [%[p3]]\n"
        "    ldp q7, q6, [%[p5]]\n"
        "    ldp q8, q9, [%[p6]]\n"
        "    ldp q11, q10, [%[p1]]\n"
        "    ldp q12, q13, [%[p2]]\n"
        "    ldp q14, q15, [%[p4]]\n"
        "    ldp q16, q17, [%[data]]\n"
        "    mov v18.16b, v1.16b\n"
        "    mov v19.16b, v7.16b\n"
        "    mov v20.16b, v5.16b\n"
        "    mov v21.16b, v8.16b\n"
        "    mov v22.16b, v9.16b\n"
        "    mov v23.16b, v11.16b\n"
        "    mov v24.16b, v14.16b\n"
        "    mov v25.16b, v15.16b\n"
        "    mov v26.16b, v12.16b\n"
        "    mov v27.16b, v13.16b\n"
        "    mov v28.16b, v16.16b\n"
        "    mov v29.16b, v17.16b\n"
        "    fmls v1.4s, v0.4s, v2.s[0]\n"
        "    fmls v7.4s, v6.4s, v2.s[0]\n"
        "    fmla v0.4s, v18.4s, v2.s[0]\n"
        "    fmla v6.4s, v19.4s, v2.s[0]\n"
        "    fmls v5.4s, v4.4s, v2.s[0]\n"
        "    fmls v11.4s, v10.4s, v2.s[0]\n"
        "    fmla v4.4s, v20.4s, v2.s[0]\n"
        "    fmls v9.4s, v1.4s, v3.s[0]\n"
        "    fmla v8.4s, v0.4s, v3.s[0]\n"
        "    fmls v21.4s, v0.4s, v3.s[0]\n"
        "    fmla v22.4s, v1.4s, v3.s[0]\n"
        "    fmla v10.4s, v23.4s, v2.s[0]\n"
        "    fmla v14.4s, v6.4s, v3.s[0]\n"
        "    fmls v15.4s, v7.4s, v3.s[0]\n"
        "    fmls v24.4s, v6.4s, v3.s[0]\n"
        "    fmla v25.4s, v7.4s, v3.s[0]\n"
        "    mov v0.16b, v8.16b\n"
        "    mov v1.16b, v21.16b\n"
        "    fmla v12.4s, v4.4s, v3.s[0]\n"
        "    fmls v13.4s, v5.4s, v3.s[0]\n"
        "    fmls v26.4s, v4.4s, v3.s[0]\n"
        "    fmla v27.4s, v5.4s, v3.s[0]\n"
        "    mov v4.16b, v14.16b\n"
        "    mov v5.16b, v15.16b\n"
        "    fmls v8.4s, v9.4s, v2.s[1]\n"
        "    fmls v21.4s, v22.4s, v2.s[1]\n"
        "    mov v6.16b, v24.16b\n"
        "    mov v7.16b, v25.16b\n"
        "    fmla v9.4s, v0.4s, v2.s[1]\n"
        "    fmla v22.4s, v1.4s, v2.s[1]\n"
        "    mov v0.16b, v12.16b\n"
        "    fmla v16.4s, v10.4s, v3.s[0]\n"
        "    fmls v17.4s, v11.4s, v3.s[0]\n"
        "    mov v1.16b, v26.16b\n"
        "    fmls v28.4s, v10.4s, v3.s[0]\n"
        "    fmla v29.4s, v11.4s, v3.s[0]\n"
        "    fmls v12.4s, v13.4s, v2.s[1]\n"
        "    fmla v14.4s, v8.4s, v3.s[1]\n"
        "    fmla v13.4s, v0.4s, v2.s[1]\n"
        "    fmla v15.4s, v9.4s, v3.s[1]\n"
        "    mov v0.16b, v16.16b\n"
        "    mov v10.16b, v17.16b\n"
        "    fmls v4.4s, v8.4s, v3.s[1]\n"
        "    fmls v5.4s, v9.4s, v3.s[1]\n"
        "    mov v8.16b, v28.16b\n"
        "    mov v9.16b, v29.16b\n"
        "    fmls v26.4s, v27.4s, v2.s[1]\n"
        "    fmla v24.4s, v22.4s, v3.s[1]\n"
        "    mov v11.16b, v14.16b\n"
        "    fmla v27.4s, v1.4s, v2.s[1]\n"
        "    fmls v25.4s, v21.4s, v3.s[1]\n"
        "    fmls v6.4s, v22.4s, v3.s[1]\n"
        "    fmla v7.4s, v21.4s, v3.s[1]\n"
        "    mov v1.16b, v4.16b\n"
        "    fmla v16.4s, v12.4s, v3.s[1]\n"
        "    fmla v17.4s, v13.4s, v3.s[1]\n"
        "    mov v18.16b, v24.16b\n"
        "    fmls v0.4s, v12.4s, v3.s[1]\n"
        "    fmls v10.4s, v13.4s, v3.s[1]\n"
        "    fmla v28.4s, v27.4s, v3.s[1]\n"
        "    fmls v29.4s, v26.4s, v3.s[1]\n"
        "    mov v12.16b, v6.16b\n"
        "    fmls v8.4s, v27.4s, v3.s[1]\n"
        "    fmla v9.4s, v26.4s, v3.s[1]\n"
        "    mov v13.16b, v16.16b\n"
        "    mov v19.16b, v17.16b\n"
        "    add %[twp], %[twp], #32\n"
        "    subs %[j], %[j], #1\n"
        "    fmls v14.4s, v15.4s, v2.s[2]\n"
        "    fmls v4.4s, v5.4s, v2.s[2]\n"
        "    mov v20.16b, v0.16b\n"
        "    mov v21.16b, v10.16b\n"
        "    fmla v15.4s, v11.4s, v2.s[2]\n"
        "    fmla v5.4s, v1.4s, v2.s[2]\n"
        "    mov v1.16b, v28.16b\n"
        "    mov v11.16b, v29.16b\n"
        "    fmls v24.4s, v25.4s, v2.s[3]\n"
        "    fmls v6.4s, v7.4s, v2.s[3]\n"
        "    mov v22.16b, v8.16b\n"
        "    mov v23.16b, v9.16b\n"
        "    fmla v25.4s, v18.4s, v2.s[3]\n"
        "    fmla v7.4s, v12.4s, v2.s[3]\n"
        "    fmla v16.4s, v14.4s, v3.s[2]\n"
        "    fmla v17.4s, v15.4s, v3.s[2]\n"
        "    fmls v13.4s, v14.4s, v3.s[2]\n"
        "    fmls v19.4s, v15.4s, v3.s[2]\n"
        "    fmla v0.4s, v5.4s, v3.s[2]\n"
        "    fmls v10.4s, v4.4s, v3.s[2]\n"
        "    fmls v20.4s, v5.4s, v3.s[2]\n"
        "    fmla v21.4s, v4.4s, v3.s[2]\n"
        "    stp q16, q17, [%[data]]\n"
        "    fmla v28.4s, v24.4s, v3.s[3]\n"
        "    fmla v29.4s, v25.4s, v3.s[3]\n"
        "    stp q13, q19, [%[p4]]\n"
        "    add %[data], %[data], #32\n"
        "    fmls v1.4s, v24.4s, v3.s[3]\n"
        "    fmls v11.4s, v25.4s, v3.s[3]\n"
        "    stp q0, q10, [%[p2]]\n"
        "    add %[p4], %[p4], #32\n"
        "    fmla v8.4s, v7.4s, v3.s[3]\n"
        "    fmls v9.4s, v6.4s, v3.s[3]\n"
        "    stp q20, q21, [%[p6]]\n"
        "    add %[p2], %[p2], #32\n"
        "    fmls v22.4s, v7.4s, v3.s[3]\n"
        "    fmla v23.4s, v6.4s, v3.s[3]\n"
        "    stp q28, q29, [%[p1]]\n"
        "    add %[p6], %[p6], #32\n"
        "    stp q1, q11, [%[p5]]\n"
        "    add %[p1], %[p1], #32\n"
        "    stp q8, q9, [%[p3]]\n"
        "    add %[p5], %[p5], #32\n"
        "    stp q22, q23, [%[p7]]\n"
        "    add %[p3], %[p3], #32\n"
        "    add %[p7], %[p7], #32\n"
        "    b.ne 4b\n"
        "    ldr %[scratch], [%[args], #24]\n"
        "    subs %[b], %[b], #1\n"
        "    add %[data], %[data], %[scratch]\n"
        "    add %[p1], %[p1], %[scratch]\n"
        "    add %[p2], %[p2], %[scratch]\n"
        "    add %[p3], %[p3], %[scratch]\n"
        "    add %[p4], %[p4], %[scratch]\n"
        "    add %[p5], %[p5], %[scratch]\n"
        "    add %[p6], %[p6], %[scratch]\n"
        "    add %[p7], %[p7], %[scratch]\n"
        "    b.ne 2b\n"
        "    ldr %[scratch], [%[args], #32]\n"
        "    subs %[regions], %[regions], #1\n"
        "    add %[data], %[data], %[scratch]\n"
        "    add %[p1], %[p1], %[scratch]\n"
        "    add %[p2], %[p2], %[scratch]\n"
        "    add %[p3], %[p3], %[scratch]\n"
        "    add %[p4], %[p4], %[scratch]\n"
        "    add %[p5], %[p5], %[scratch]\n"
        "    add %[p6], %[p6], %[scratch]\n"
        "    add %[p7], %[p7], %[scratch]\n"
        "    ldr %[scratch], [%[args], #40]\n"
        "    add %[tw], %[tw], %[scratch]\n"
        "    b.ne 1b\n"
        : [data] "+r"(data), [tw] "+r"(tw), [args] "+r"(args), [blocks] "+r"(blocks), [regions] "+r"(regions), [stride] "=&r"(stride), [p1] "=&r"(p1), [p2] "=&r"(p2), [p3] "=&r"(p3), [p4] "=&r"(p4), [p5] "=&r"(p5), [p6] "=&r"(p6), [p7] "=&r"(p7), [twp] "=&r"(twp), [j] "=&r"(j), [b] "=&r"(b), [scratch] "=&r"(scratch)
        :
        : "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", "v10", "v11", "v12", "v13", "v14", "v15", "v16", "v17", "v18", "v19", "v20", "v21", "v22", "v23", "v24", "v25", "v26", "v27", "v28", "v29", "cc", "memory");
}

/* Middle pass, radix 4, in place on split vectors, from fused multiply-add butterflies. `data` holds `regions` regions of `blocks`
 * blocks. Vector u of butterfly j of a block is at u*args.s_high + 32*j, for j < args.inner; see
 * passes.py for how the blocks and regions follow each other. 
 * The table `tw` holds per butterfly the cosines of its 2 twiddles, then their tangents, see the
 * table builder in onefft.c. */
static void onefft_neon_c4_fma(float *data, const float *tw, const size_t *args, size_t blocks, size_t regions) {
    size_t stride;
    float *p1;
    float *p2;
    float *p3;
    const float *twp;
    size_t j;
    size_t b;
    size_t scratch;
    __asm__ volatile(
        "    ldr %[stride], [%[args], #0]\n"
        "    add %[p1], %[data], %[stride]\n"
        "    add %[p2], %[p1], %[stride]\n"
        "    add %[p3], %[p2], %[stride]\n"
        "1:\n"
        "    mov %[b], %[blocks]\n"
        "2:\n"
        "    ldr %[scratch], [%[args], #16]\n"
        "    mov %[twp], %[tw]\n"
        "    lsr %[j], %[scratch], #1\n"
        "3:\n"
        "    ldp q0, q1, [%[p3]]\n"
        "    ldr q2, [%[twp]]\n"
        "    ldp q3, q4, [%[p1]]\n"
        "    ldp q5, q6, [%[p2]]\n"
        "    ldp q7, q8, [%[data]]\n"
        "    subs %[j], %[j], #1\n"
        "    add %[twp], %[twp], #16\n"
        "    mov v9.16b, v0.16b\n"
        "    mov v10.16b, v3.16b\n"
        "    mov v11.16b, v5.16b\n"
        "    mov v12.16b, v6.16b\n"
        "    mov v13.16b, v7.16b\n"
        "    mov v14.16b, v8.16b\n"
        "    fmls v0.4s, v1.4s, v2.s[2]\n"
        "    fmls v3.4s, v4.4s, v2.s[2]\n"
        "    fmla v1.4s, v9.4s, v2.s[2]\n"
        "    fmla v4.4s, v10.4s, v2.s[2]\n"
        "    fmla v5.4s, v0.4s, v2.s[0]\n"
        "    fmls v11.4s, v0.4s, v2.s[0]\n"
        "    fmla v6.4s, v1.4s, v2.s[0]\n"
        "    fmls v12.4s, v1.4s, v2.s[0]\n"
        "    fmla v7.4s, v3.4s, v2.s[0]\n"
        "    fmla v8.4s, v4.4s, v2.s[0]\n"
        "    fmls v13.4s, v3.4s, v2.s[0]\n"
        "    fmls v14.4s, v4.4s, v2.s[0]\n"
        "    mov v0.16b, v5.16b\n"
        "    mov v1.16b, v11.16b\n"
        "    fmls v5.4s, v6.4s, v2.s[3]\n"
        "    fmls v11.4s, v12.4s, v2.s[3]\n"
        "    fmla v6.4s, v0.4s, v2.s[3]\n"
        "    fmla v12.4s, v1.4s, v2.s[3]\n"
        "    mov v0.16b, v7.16b\n"
        "    mov v1.16b, v8.16b\n"
        "    mov v3.16b, v13.16b\n"
        "    mov v4.16b, v14.16b\n"
        "    fmla v7.4s, v5.4s, v2.s[1]\n"
        "    fmls v0.4s, v5.4s, v2.s[1]\n"
        "    fmla v8.4s, v6.4s, v2.s[1]\n"
        "    fmls v1.4s, v6.4s, v2.s[1]\n"
        "    fmla v13.4s, v12.4s, v2.s[1]\n"
        "    fmls v14.4s, v11.4s, v2.s[1]\n"
        "    fmls v3.4s, v12.4s, v2.s[1]\n"
        "    fmla v4.4s, v11.4s, v2.s[1]\n"
        "    stp q7, q8, [%[data]]\n"
        "    stp q0, q1, [%[p2]]\n"
        "    stp q13, q14, [%[p1]]\n"
        "    add %[data], %[data], #32\n"
        "    add %[p2], %[p2], #32\n"
        "    stp q3, q4, [%[p3]]\n"
        "    add %[p1], %[p1], #32\n"
        "    add %[p3], %[p3], #32\n"
        "    b.ne 3b\n"
        "    lsr %[j], %[scratch], #1\n"
        "4:\n"
        "    ldp q0, q1, [%[p3]]\n"
        "    ldr q2, [%[twp]]\n"
        "    ldp q4, q3, [%[p1]]\n"
        "    ldp q5, q6, [%[p2]]\n"
        "    ldp q7, q8, [%[data]]\n"
        "    subs %[j], %[j], #1\n"
        "    add %[twp], %[twp], #16\n"
        "    mov v9.16b, v0.16b\n"
        "    mov v10.16b, v4.16b\n"
        "    mov v11.16b, v5.16b\n"
        "    mov v12.16b, v6.16b\n"
        "    mov v13.16b, v7.16b\n"
        "    mov v14.16b, v8.16b\n"
        "    fmls v0.4s, v1.4s, v2.s[2]\n"
        "    fmls v4.4s, v3.4s, v2.s[2]\n"
        "    fmla v1.4s, v9.4s, v2.s[2]\n"
        "    fmla v3.4s, v10.4s, v2.s[2]\n"
        "    fmls v6.4s, v0.4s, v2.s[0]\n"
        "    fmla v12.4s, v0.4s, v2.s[0]\n"
        "    fmla v5.4s, v1.4s, v2.s[0]\n"
        "    fmls v11.4s, v1.4s, v2.s[0]\n"
        "    fmla v7.4s, v3.4s, v2.s[0]\n"
        "    fmls v8.4s, v4.4s, v2.s[0]\n"
        "    fmls v13.4s, v3.4s, v2.s[0]\n"
        "    fmla v14.4s, v4.4s, v2.s[0]\n"
        "    mov v0.16b, v5.16b\n"
        "    mov v1.16b, v11.16b\n"
        "    fmls v5.4s, v6.4s, v2.s[3]\n"
        "    fmls v11.4s, v12.4s, v2.s[3]\n"
        "    mov v3.16b, v7.16b\n"
        "    mov v4.16b, v8.16b\n"
        "    fmla v6.4s, v0.4s, v2.s[3]\n"
        "    fmla v12.4s, v1.4s, v2.s[3]\n"
        "    mov v0.16b, v13.16b\n"
        "    mov v1.16b, v14.16b\n"
        "    fmla v7.4s, v5.4s, v2.s[1]\n"
        "    fmls v3.4s, v5.4s, v2.s[1]\n"
        "    fmla v8.4s, v6.4s, v2.s[1]\n"
        "    fmls v4.4s, v6.4s, v2.s[1]\n"
        "    fmla v13.4s, v12.4s, v2.s[1]\n"
        "    fmls v14.4s, v11.4s, v2.s[1]\n"
        "    fmls v0.4s, v12.4s, v2.s[1]\n"
        "    fmla v1.4s, v11.4s, v2.s[1]\n"
        "    stp q7, q8, [%[data]]\n"
        "    stp q3, q4, [%[p2]]\n"
        "    stp q13, q14, [%[p1]]\n"
        "    add %[data], %[data], #32\n"
        "    add %[p2], %[p2], #32\n"
        "    stp q0, q1, [%[p3]]\n"
        "    add %[p1], %[p1], #32\n"
        "    add %[p3], %[p3], #32\n"
        "    b.ne 4b\n"
        "    ldr %[scratch], [%[args], #24]\n"
        "    subs %[b], %[b], #1\n"
        "    add %[data], %[data], %[scratch]\n"
        "    add %[p1], %[p1], %[scratch]\n"
        "    add %[p2], %[p2], %[scratch]\n"
        "    add %[p3], %[p3], %[scratch]\n"
        "    b.ne 2b\n"
        "    ldr %[scratch], [%[args], #32]\n"
        "    subs %[regions], %[regions], #1\n"
        "    add %[data], %[data], %[scratch]\n"
        "    add %[p1], %[p1], %[scratch]\n"
        "    add %[p2], %[p2], %[scratch]\n"
        "    add %[p3], %[p3], %[scratch]\n"
        "    ldr %[scratch], [%[args], #40]\n"
        "    add %[tw], %[tw], %[scratch]\n"
        "    b.ne 1b\n"
        : [data] "+r"(data), [tw] "+r"(tw), [args] "+r"(args), [blocks] "+r"(blocks), [regions] "+r"(regions), [stride] "=&r"(stride), [p1] "=&r"(p1), [p2] "=&r"(p2), [p3] "=&r"(p3), [twp] "=&r"(twp), [j] "=&r"(j), [b] "=&r"(b), [scratch] "=&r"(scratch)
        :
        : "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", "v10", "v11", "v12", "v13", "v14", "cc", "memory");
}

#pragma GCC diagnostic pop
#endif

#if defined(__x86_64__) && !defined(ONEFFT_NO_SIMD)
#define ONEFFT_X86 1
#include <cpuid.h>
/*
 * OneFFT x86-64 SSE2 kernels.
 *
 * GENERATED FILE, DO NOT EDIT. Change gen/onefft_gen and run gen/generate.py instead.
 *
 * Every kernel is a normal C function whose body is one inline assembly block. The SIMD registers
 * are assigned by the generator, the general purpose registers by the C compiler.
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverlength-strings"

/* First pass, radix 4. For each of the `count` entries (a, b) of `table`: reads 4 interleaved
 * vectors from in + 8*a + u*in_stride, transforms across them and writes output q as a split
 * vector to out + 8*b + r*out_stride, where r is q with its bits reversed. */
static void onefft_sse2_f4(const float *in, float *out, const uint32_t *table, size_t count, size_t in_stride, size_t out_stride) {
    const float *src;
    float *dst;
    size_t a;
    size_t b;
    __asm__ volatile(
        "1:\n"
        "    movl (%[table]), %k[a]\n"
        "    movl 4(%[table]), %k[b]\n"
        "    lea 8(%[table]), %[table]\n"
        "    lea (%[in],%[a],8), %[src]\n"
        "    lea (%[out],%[b],8), %[dst]\n"
        "    movups (%[src]), %%xmm0\n"
        "    movups 16(%[src]), %%xmm1\n"
        "    lea (%[src],%[in_stride],1), %[src]\n"
        "    movups (%[src]), %%xmm2\n"
        "    movups 16(%[src]), %%xmm3\n"
        "    lea (%[src],%[in_stride],1), %[src]\n"
        "    movups (%[src]), %%xmm4\n"
        "    movups 16(%[src]), %%xmm5\n"
        "    lea (%[src],%[in_stride],1), %[src]\n"
        "    movaps %%xmm0, %%xmm6\n"
        "    shufps $136, %%xmm1, %%xmm6\n"
        "    movaps %%xmm0, %%xmm0\n"
        "    shufps $221, %%xmm1, %%xmm0\n"
        "    movups (%[src]), %%xmm1\n"
        "    movups 16(%[src]), %%xmm7\n"
        "    movaps %%xmm2, %%xmm8\n"
        "    shufps $136, %%xmm3, %%xmm8\n"
        "    movaps %%xmm2, %%xmm2\n"
        "    shufps $221, %%xmm3, %%xmm2\n"
        "    movaps %%xmm4, %%xmm3\n"
        "    shufps $136, %%xmm5, %%xmm3\n"
        "    movaps %%xmm4, %%xmm4\n"
        "    shufps $221, %%xmm5, %%xmm4\n"
        "    movaps %%xmm6, %%xmm5\n"
        "    addps %%xmm3, %%xmm5\n"
        "    movaps %%xmm0, %%xmm9\n"
        "    addps %%xmm4, %%xmm9\n"
        "    movaps %%xmm6, %%xmm6\n"
        "    subps %%xmm3, %%xmm6\n"
        "    movaps %%xmm1, %%xmm3\n"
        "    shufps $136, %%xmm7, %%xmm3\n"
        "    movaps %%xmm1, %%xmm1\n"
        "    shufps $221, %%xmm7, %%xmm1\n"
        "    movaps %%xmm0, %%xmm0\n"
        "    subps %%xmm4, %%xmm0\n"
        "    movaps %%xmm8, %%xmm4\n"
        "    addps %%xmm3, %%xmm4\n"
        "    movaps %%xmm2, %%xmm7\n"
        "    addps %%xmm1, %%xmm7\n"
        "    movaps %%xmm8, %%xmm8\n"
        "    subps %%xmm3, %%xmm8\n"
        "    movaps %%xmm2, %%xmm2\n"
        "    subps %%xmm1, %%xmm2\n"
        "    movaps %%xmm5, %%xmm1\n"
        "    addps %%xmm4, %%xmm1\n"
        "    movaps %%xmm9, %%xmm3\n"
        "    addps %%xmm7, %%xmm3\n"
        "    movaps %%xmm5, %%xmm5\n"
        "    subps %%xmm4, %%xmm5\n"
        "    movaps %%xmm9, %%xmm4\n"
        "    subps %%xmm7, %%xmm4\n"
        "    movaps %%xmm6, %%xmm7\n"
        "    addps %%xmm2, %%xmm7\n"
        "    movaps %%xmm0, %%xmm9\n"
        "    subps %%xmm8, %%xmm9\n"
        "    movaps %%xmm6, %%xmm6\n"
        "    subps %%xmm2, %%xmm6\n"
        "    movaps %%xmm0, %%xmm0\n"
        "    addps %%xmm8, %%xmm0\n"
        "    movups %%xmm1, (%[dst])\n"
        "    movups %%xmm3, 16(%[dst])\n"
        "    lea (%[dst],%[out_stride],1), %[dst]\n"
        "    movups %%xmm5, (%[dst])\n"
        "    movups %%xmm4, 16(%[dst])\n"
        "    lea (%[dst],%[out_stride],1), %[dst]\n"
        "    movups %%xmm7, (%[dst])\n"
        "    movups %%xmm9, 16(%[dst])\n"
        "    lea (%[dst],%[out_stride],1), %[dst]\n"
        "    movups %%xmm6, (%[dst])\n"
        "    movups %%xmm0, 16(%[dst])\n"
        "    sub $1, %[count]\n"
        "    jnz 1b\n"
        : [in] "+r"(in), [out] "+r"(out), [table] "+r"(table), [count] "+r"(count), [in_stride] "+r"(in_stride), [out_stride] "+r"(out_stride), [src] "=&r"(src), [dst] "=&r"(dst), [a] "=&r"(a), [b] "=&r"(b)
        :
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "cc", "memory");
}

/* First pass, radix 2. For each of the `count` entries (a, b) of `table`: reads 2 interleaved
 * vectors from in + 8*a + u*in_stride, transforms across them and writes output q as a split
 * vector to out + 8*b + r*out_stride, where r is q with its bits reversed. */
static void onefft_sse2_f2(const float *in, float *out, const uint32_t *table, size_t count, size_t in_stride, size_t out_stride) {
    const float *src;
    float *dst;
    size_t a;
    size_t b;
    __asm__ volatile(
        "1:\n"
        "    movl (%[table]), %k[a]\n"
        "    movl 4(%[table]), %k[b]\n"
        "    lea 8(%[table]), %[table]\n"
        "    lea (%[in],%[a],8), %[src]\n"
        "    lea (%[out],%[b],8), %[dst]\n"
        "    movups (%[src]), %%xmm0\n"
        "    movups 16(%[src]), %%xmm1\n"
        "    lea (%[src],%[in_stride],1), %[src]\n"
        "    movups (%[src]), %%xmm2\n"
        "    movups 16(%[src]), %%xmm3\n"
        "    movaps %%xmm0, %%xmm4\n"
        "    shufps $136, %%xmm1, %%xmm4\n"
        "    movaps %%xmm0, %%xmm0\n"
        "    shufps $221, %%xmm1, %%xmm0\n"
        "    movaps %%xmm2, %%xmm1\n"
        "    shufps $136, %%xmm3, %%xmm1\n"
        "    movaps %%xmm2, %%xmm2\n"
        "    shufps $221, %%xmm3, %%xmm2\n"
        "    movaps %%xmm4, %%xmm3\n"
        "    addps %%xmm1, %%xmm3\n"
        "    movaps %%xmm0, %%xmm5\n"
        "    addps %%xmm2, %%xmm5\n"
        "    movaps %%xmm4, %%xmm4\n"
        "    subps %%xmm1, %%xmm4\n"
        "    movaps %%xmm0, %%xmm0\n"
        "    subps %%xmm2, %%xmm0\n"
        "    movups %%xmm3, (%[dst])\n"
        "    movups %%xmm5, 16(%[dst])\n"
        "    lea (%[dst],%[out_stride],1), %[dst]\n"
        "    movups %%xmm4, (%[dst])\n"
        "    movups %%xmm0, 16(%[dst])\n"
        "    sub $1, %[count]\n"
        "    jnz 1b\n"
        : [in] "+r"(in), [out] "+r"(out), [table] "+r"(table), [count] "+r"(count), [in_stride] "+r"(in_stride), [out_stride] "+r"(out_stride), [src] "=&r"(src), [dst] "=&r"(dst), [a] "=&r"(a), [b] "=&r"(b)
        :
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "cc", "memory");
}

/* Last pass, radix 4, in place: reads split vectors and writes the interleaved result.
 * Butterfly j < count uses the vectors at data + r*stride + 32*j. The table holds per butterfly,
 * for l = 1..3, the real and the imaginary parts of the 4 twiddles of the vector with index bits
 * (1, 0) = l. */
static void onefft_sse2_l4(float *data, const float *tw, size_t stride, size_t count, size_t regions) {
    (void)regions;
    float *third;
    __asm__ volatile(
        "    lea (%[data],%[stride],2), %[third]\n"
        "    lea (%[third],%[stride],1), %[third]\n"
        "1:\n"
        "    movups (%[data]), %%xmm0\n"
        "    movups 16(%[data]), %%xmm1\n"
        "    movups (%[data],%[stride],1), %%xmm2\n"
        "    movups 16(%[data],%[stride],1), %%xmm3\n"
        "    movups (%[data],%[stride],2), %%xmm4\n"
        "    movups 16(%[data],%[stride],2), %%xmm5\n"
        "    movups (%[third]), %%xmm6\n"
        "    movups 16(%[third]), %%xmm7\n"
        "    movaps %%xmm0, %%xmm8\n"
        "    unpcklps %%xmm4, %%xmm8\n"
        "    movaps %%xmm0, %%xmm0\n"
        "    unpckhps %%xmm4, %%xmm0\n"
        "    movaps %%xmm1, %%xmm4\n"
        "    unpcklps %%xmm5, %%xmm4\n"
        "    movaps %%xmm1, %%xmm1\n"
        "    unpckhps %%xmm5, %%xmm1\n"
        "    movaps %%xmm2, %%xmm5\n"
        "    unpcklps %%xmm6, %%xmm5\n"
        "    movaps %%xmm2, %%xmm2\n"
        "    unpckhps %%xmm6, %%xmm2\n"
        "    movaps %%xmm3, %%xmm6\n"
        "    unpcklps %%xmm7, %%xmm6\n"
        "    movaps %%xmm3, %%xmm3\n"
        "    unpckhps %%xmm7, %%xmm3\n"
        "    movaps %%xmm8, %%xmm7\n"
        "    unpckhpd %%xmm5, %%xmm7\n"
        "    movaps %%xmm4, %%xmm9\n"
        "    unpckhpd %%xmm6, %%xmm9\n"
        "    movaps %%xmm0, %%xmm10\n"
        "    unpcklpd %%xmm2, %%xmm10\n"
        "    movaps %%xmm0, %%xmm0\n"
        "    unpckhpd %%xmm2, %%xmm0\n"
        "    movaps %%xmm1, %%xmm2\n"
        "    unpcklpd %%xmm3, %%xmm2\n"
        "    movaps %%xmm1, %%xmm1\n"
        "    unpckhpd %%xmm3, %%xmm1\n"
        "    movaps %%xmm7, %%xmm3\n"
        "    mulps (%[tw]), %%xmm3\n"
        "    movaps %%xmm9, %%xmm11\n"
        "    mulps (%[tw]), %%xmm11\n"
        "    movaps %%xmm9, %%xmm9\n"
        "    mulps 16(%[tw]), %%xmm9\n"
        "    movaps %%xmm7, %%xmm7\n"
        "    mulps 16(%[tw]), %%xmm7\n"
        "    movaps %%xmm10, %%xmm12\n"
        "    mulps 32(%[tw]), %%xmm12\n"
        "    movaps %%xmm2, %%xmm13\n"
        "    mulps 32(%[tw]), %%xmm13\n"
        "    movaps %%xmm2, %%xmm2\n"
        "    mulps 48(%[tw]), %%xmm2\n"
        "    movaps %%xmm10, %%xmm10\n"
        "    mulps 48(%[tw]), %%xmm10\n"
        "    movaps %%xmm0, %%xmm14\n"
        "    mulps 64(%[tw]), %%xmm14\n"
        "    movaps %%xmm1, %%xmm15\n"
        "    mulps 64(%[tw]), %%xmm15\n"
        "    movaps %%xmm1, %%xmm1\n"
        "    mulps 80(%[tw]), %%xmm1\n"
        "    movaps %%xmm0, %%xmm0\n"
        "    mulps 80(%[tw]), %%xmm0\n"
        "    movaps %%xmm8, %%xmm8\n"
        "    unpcklpd %%xmm5, %%xmm8\n"
        "    subps %%xmm9, %%xmm3\n"
        "    addps %%xmm7, %%xmm11\n"
        "    movaps %%xmm4, %%xmm4\n"
        "    unpcklpd %%xmm6, %%xmm4\n"
        "    lea 96(%[tw]), %[tw]\n"
        "    subps %%xmm2, %%xmm12\n"
        "    addps %%xmm10, %%xmm13\n"
        "    subps %%xmm1, %%xmm14\n"
        "    addps %%xmm0, %%xmm15\n"
        "    movaps %%xmm8, %%xmm0\n"
        "    addps %%xmm12, %%xmm0\n"
        "    movaps %%xmm8, %%xmm1\n"
        "    subps %%xmm12, %%xmm1\n"
        "    movaps %%xmm4, %%xmm2\n"
        "    addps %%xmm13, %%xmm2\n"
        "    movaps %%xmm4, %%xmm4\n"
        "    subps %%xmm13, %%xmm4\n"
        "    movaps %%xmm3, %%xmm5\n"
        "    addps %%xmm14, %%xmm5\n"
        "    movaps %%xmm11, %%xmm6\n"
        "    addps %%xmm15, %%xmm6\n"
        "    movaps %%xmm3, %%xmm3\n"
        "    subps %%xmm14, %%xmm3\n"
        "    movaps %%xmm11, %%xmm7\n"
        "    subps %%xmm15, %%xmm7\n"
        "    movaps %%xmm0, %%xmm8\n"
        "    addps %%xmm5, %%xmm8\n"
        "    movaps %%xmm2, %%xmm9\n"
        "    addps %%xmm6, %%xmm9\n"
        "    movaps %%xmm0, %%xmm0\n"
        "    subps %%xmm5, %%xmm0\n"
        "    movaps %%xmm2, %%xmm2\n"
        "    subps %%xmm6, %%xmm2\n"
        "    movaps %%xmm1, %%xmm5\n"
        "    addps %%xmm7, %%xmm5\n"
        "    movaps %%xmm4, %%xmm6\n"
        "    subps %%xmm3, %%xmm6\n"
        "    movaps %%xmm1, %%xmm1\n"
        "    subps %%xmm7, %%xmm1\n"
        "    movaps %%xmm4, %%xmm4\n"
        "    addps %%xmm3, %%xmm4\n"
        "    movaps %%xmm8, %%xmm3\n"
        "    unpcklps %%xmm9, %%xmm3\n"
        "    movaps %%xmm8, %%xmm7\n"
        "    unpckhps %%xmm9, %%xmm7\n"
        "    movaps %%xmm5, %%xmm8\n"
        "    unpcklps %%xmm6, %%xmm8\n"
        "    movaps %%xmm5, %%xmm5\n"
        "    unpckhps %%xmm6, %%xmm5\n"
        "    movaps %%xmm0, %%xmm6\n"
        "    unpcklps %%xmm2, %%xmm6\n"
        "    movups %%xmm3, (%[data])\n"
        "    movups %%xmm7, 16(%[data])\n"
        "    movaps %%xmm0, %%xmm0\n"
        "    unpckhps %%xmm2, %%xmm0\n"
        "    movaps %%xmm1, %%xmm2\n"
        "    unpcklps %%xmm4, %%xmm2\n"
        "    movaps %%xmm1, %%xmm1\n"
        "    unpckhps %%xmm4, %%xmm1\n"
        "    movups %%xmm8, (%[data],%[stride],1)\n"
        "    movups %%xmm5, 16(%[data],%[stride],1)\n"
        "    movups %%xmm6, (%[data],%[stride],2)\n"
        "    movups %%xmm0, 16(%[data],%[stride],2)\n"
        "    movups %%xmm2, (%[third])\n"
        "    movups %%xmm1, 16(%[third])\n"
        "    lea 32(%[data]), %[data]\n"
        "    lea 32(%[third]), %[third]\n"
        "    sub $1, %[count]\n"
        "    jnz 1b\n"
        : [data] "+r"(data), [tw] "+r"(tw), [stride] "+r"(stride), [count] "+r"(count), [third] "=&r"(third)
        :
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "cc", "memory");
}

static const float onefft_sse2_n8_consts[16] ONEFFT_ALIGNED(64) = {
    0x1.0000000000000p+0f, 0x1.0000000000000p+0f, -0x0.0p+0f, -0x0.0p+0f,
    0x0.0p+0f, 0x0.0p+0f, -0x1.0000000000000p+0f, -0x1.0000000000000p+0f,
    0x1.0000000000000p+0f, 0x1.6a09e667f3bcdp-1f, -0x0.0p+0f, -0x1.6a09e667f3bccp-1f,
    0x0.0p+0f, -0x1.6a09e667f3bccp-1f, -0x1.0000000000000p+0f, -0x1.6a09e667f3bcdp-1f,
};
/* Complete 8 point transform from `in` to `out`, both interleaved. */
static void onefft_sse2_n8(const float *in, float *out) {
    const float *consts = onefft_sse2_n8_consts;
    __asm__ volatile(
        "    movups (%[in]), %%xmm0\n"
        "    movups 16(%[in]), %%xmm1\n"
        "    movups 32(%[in]), %%xmm2\n"
        "    movups 48(%[in]), %%xmm3\n"
        "    movaps %%xmm0, %%xmm4\n"
        "    shufps $136, %%xmm1, %%xmm4\n"
        "    movaps %%xmm0, %%xmm0\n"
        "    shufps $221, %%xmm1, %%xmm0\n"
        "    movaps %%xmm2, %%xmm1\n"
        "    shufps $136, %%xmm3, %%xmm1\n"
        "    movaps %%xmm2, %%xmm2\n"
        "    shufps $221, %%xmm3, %%xmm2\n"
        "    movaps %%xmm4, %%xmm3\n"
        "    addps %%xmm1, %%xmm3\n"
        "    movaps %%xmm4, %%xmm4\n"
        "    subps %%xmm1, %%xmm4\n"
        "    movaps %%xmm0, %%xmm1\n"
        "    addps %%xmm2, %%xmm1\n"
        "    movaps %%xmm0, %%xmm0\n"
        "    subps %%xmm2, %%xmm0\n"
        "    movaps %%xmm3, %%xmm2\n"
        "    unpckhpd %%xmm4, %%xmm2\n"
        "    movaps %%xmm3, %%xmm3\n"
        "    unpcklpd %%xmm4, %%xmm3\n"
        "    movaps %%xmm1, %%xmm4\n"
        "    unpckhpd %%xmm0, %%xmm4\n"
        "    movaps %%xmm2, %%xmm5\n"
        "    mulps (%[consts]), %%xmm5\n"
        "    movaps %%xmm2, %%xmm2\n"
        "    mulps 16(%[consts]), %%xmm2\n"
        "    movaps %%xmm4, %%xmm6\n"
        "    mulps (%[consts]), %%xmm6\n"
        "    movaps %%xmm4, %%xmm4\n"
        "    mulps 16(%[consts]), %%xmm4\n"
        "    movaps %%xmm1, %%xmm1\n"
        "    unpcklpd %%xmm0, %%xmm1\n"
        "    subps %%xmm4, %%xmm5\n"
        "    addps %%xmm2, %%xmm6\n"
        "    movaps %%xmm3, %%xmm0\n"
        "    addps %%xmm5, %%xmm0\n"
        "    movaps %%xmm1, %%xmm2\n"
        "    addps %%xmm6, %%xmm2\n"
        "    movaps %%xmm3, %%xmm3\n"
        "    subps %%xmm5, %%xmm3\n"
        "    movaps %%xmm1, %%xmm1\n"
        "    subps %%xmm6, %%xmm1\n"
        "    movaps %%xmm0, %%xmm4\n"
        "    shufps $221, %%xmm3, %%xmm4\n"
        "    movaps %%xmm0, %%xmm0\n"
        "    shufps $136, %%xmm3, %%xmm0\n"
        "    movaps %%xmm2, %%xmm3\n"
        "    shufps $221, %%xmm1, %%xmm3\n"
        "    movaps %%xmm4, %%xmm5\n"
        "    mulps 32(%[consts]), %%xmm5\n"
        "    movaps %%xmm4, %%xmm4\n"
        "    mulps 48(%[consts]), %%xmm4\n"
        "    movaps %%xmm3, %%xmm6\n"
        "    mulps 32(%[consts]), %%xmm6\n"
        "    movaps %%xmm3, %%xmm3\n"
        "    mulps 48(%[consts]), %%xmm3\n"
        "    movaps %%xmm2, %%xmm2\n"
        "    shufps $136, %%xmm1, %%xmm2\n"
        "    subps %%xmm3, %%xmm5\n"
        "    addps %%xmm4, %%xmm6\n"
        "    movaps %%xmm0, %%xmm1\n"
        "    addps %%xmm5, %%xmm1\n"
        "    movaps %%xmm2, %%xmm3\n"
        "    addps %%xmm6, %%xmm3\n"
        "    movaps %%xmm0, %%xmm0\n"
        "    subps %%xmm5, %%xmm0\n"
        "    movaps %%xmm2, %%xmm2\n"
        "    subps %%xmm6, %%xmm2\n"
        "    movaps %%xmm1, %%xmm4\n"
        "    unpcklps %%xmm3, %%xmm4\n"
        "    movaps %%xmm1, %%xmm1\n"
        "    unpckhps %%xmm3, %%xmm1\n"
        "    movaps %%xmm0, %%xmm3\n"
        "    unpcklps %%xmm2, %%xmm3\n"
        "    movaps %%xmm0, %%xmm0\n"
        "    unpckhps %%xmm2, %%xmm0\n"
        "    movups %%xmm4, (%[out])\n"
        "    movups %%xmm1, 16(%[out])\n"
        "    movups %%xmm3, 32(%[out])\n"
        "    movups %%xmm0, 48(%[out])\n"
        : [in] "+r"(in), [out] "+r"(out), [consts] "+r"(consts)
        :
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "cc", "memory");
}

static const float onefft_sse2_n16_consts[24] ONEFFT_ALIGNED(64) = {
    0x1.0000000000000p+0f, 0x1.d906bcf328d46p-1f, 0x1.6a09e667f3bcdp-1f, 0x1.87de2a6aea964p-2f,
    0x0.0p+0f, -0x1.87de2a6aea963p-2f, -0x1.6a09e667f3bccp-1f, -0x1.d906bcf328d46p-1f,
    0x1.0000000000000p+0f, 0x1.6a09e667f3bcdp-1f, -0x0.0p+0f, -0x1.6a09e667f3bccp-1f,
    0x0.0p+0f, -0x1.6a09e667f3bccp-1f, -0x1.0000000000000p+0f, -0x1.6a09e667f3bcdp-1f,
    0x1.0000000000000p+0f, 0x1.87de2a6aea964p-2f, -0x1.6a09e667f3bccp-1f, -0x1.d906bcf328d47p-1f,
    0x0.0p+0f, -0x1.d906bcf328d46p-1f, -0x1.6a09e667f3bcdp-1f, 0x1.87de2a6aea961p-2f,
};
/* Complete 16 point transform from `in` to `out`, both interleaved. */
static void onefft_sse2_n16(const float *in, float *out) {
    const float *consts = onefft_sse2_n16_consts;
    __asm__ volatile(
        "    movups (%[in]), %%xmm0\n"
        "    movups 16(%[in]), %%xmm1\n"
        "    movups 32(%[in]), %%xmm2\n"
        "    movups 48(%[in]), %%xmm3\n"
        "    movups 64(%[in]), %%xmm4\n"
        "    movups 80(%[in]), %%xmm5\n"
        "    movups 96(%[in]), %%xmm6\n"
        "    movups 112(%[in]), %%xmm7\n"
        "    movaps %%xmm0, %%xmm8\n"
        "    shufps $136, %%xmm1, %%xmm8\n"
        "    movaps %%xmm0, %%xmm0\n"
        "    shufps $221, %%xmm1, %%xmm0\n"
        "    movaps %%xmm2, %%xmm1\n"
        "    shufps $136, %%xmm3, %%xmm1\n"
        "    movaps %%xmm2, %%xmm2\n"
        "    shufps $221, %%xmm3, %%xmm2\n"
        "    movaps %%xmm4, %%xmm3\n"
        "    shufps $136, %%xmm5, %%xmm3\n"
        "    movaps %%xmm4, %%xmm4\n"
        "    shufps $221, %%xmm5, %%xmm4\n"
        "    movaps %%xmm6, %%xmm5\n"
        "    shufps $136, %%xmm7, %%xmm5\n"
        "    movaps %%xmm6, %%xmm6\n"
        "    shufps $221, %%xmm7, %%xmm6\n"
        "    movaps %%xmm8, %%xmm7\n"
        "    addps %%xmm3, %%xmm7\n"
        "    movaps %%xmm0, %%xmm9\n"
        "    addps %%xmm4, %%xmm9\n"
        "    movaps %%xmm8, %%xmm8\n"
        "    subps %%xmm3, %%xmm8\n"
        "    movaps %%xmm0, %%xmm0\n"
        "    subps %%xmm4, %%xmm0\n"
        "    movaps %%xmm1, %%xmm3\n"
        "    addps %%xmm5, %%xmm3\n"
        "    movaps %%xmm2, %%xmm4\n"
        "    addps %%xmm6, %%xmm4\n"
        "    movaps %%xmm1, %%xmm1\n"
        "    subps %%xmm5, %%xmm1\n"
        "    movaps %%xmm2, %%xmm2\n"
        "    subps %%xmm6, %%xmm2\n"
        "    movaps %%xmm7, %%xmm5\n"
        "    addps %%xmm3, %%xmm5\n"
        "    movaps %%xmm9, %%xmm6\n"
        "    addps %%xmm4, %%xmm6\n"
        "    movaps %%xmm7, %%xmm7\n"
        "    subps %%xmm3, %%xmm7\n"
        "    movaps %%xmm9, %%xmm3\n"
        "    subps %%xmm4, %%xmm3\n"
        "    movaps %%xmm8, %%xmm4\n"
        "    addps %%xmm2, %%xmm4\n"
        "    movaps %%xmm0, %%xmm9\n"
        "    subps %%xmm1, %%xmm9\n"
        "    movaps %%xmm8, %%xmm8\n"
        "    subps %%xmm2, %%xmm8\n"
        "    movaps %%xmm0, %%xmm0\n"
        "    addps %%xmm1, %%xmm0\n"
        "    movaps %%xmm5, %%xmm1\n"
        "    unpcklps %%xmm4, %%xmm1\n"
        "    movaps %%xmm5, %%xmm2\n"
        "    unpckhps %%xmm4, %%xmm2\n"
        "    movaps %%xmm6, %%xmm4\n"
        "    unpcklps %%xmm9, %%xmm4\n"
        "    movaps %%xmm6, %%xmm5\n"
        "    unpckhps %%xmm9, %%xmm5\n"
        "    movaps %%xmm7, %%xmm6\n"
        "    unpcklps %%xmm8, %%xmm6\n"
        "    movaps %%xmm7, %%xmm7\n"
        "    unpckhps %%xmm8, %%xmm7\n"
        "    movaps %%xmm3, %%xmm8\n"
        "    unpcklps %%xmm0, %%xmm8\n"
        "    movaps %%xmm3, %%xmm3\n"
        "    unpckhps %%xmm0, %%xmm3\n"
        "    movaps %%xmm1, %%xmm0\n"
        "    unpckhpd %%xmm6, %%xmm0\n"
        "    movaps %%xmm4, %%xmm9\n"
        "    unpckhpd %%xmm8, %%xmm9\n"
        "    movaps %%xmm2, %%xmm10\n"
        "    unpcklpd %%xmm7, %%xmm10\n"
        "    movaps %%xmm2, %%xmm2\n"
        "    unpckhpd %%xmm7, %%xmm2\n"
        "    movaps %%xmm5, %%xmm7\n"
        "    unpcklpd %%xmm3, %%xmm7\n"
        "    movaps %%xmm5, %%xmm5\n"
        "    unpckhpd %%xmm3, %%xmm5\n"
        "    movaps %%xmm0, %%xmm3\n"
        "    mulps (%[consts]), %%xmm3\n"
        "    movaps %%xmm9, %%xmm11\n"
        "    mulps (%[consts]), %%xmm11\n"
        "    movaps %%xmm9, %%xmm9\n"
        "    mulps 16(%[consts]), %%xmm9\n"
        "    movaps %%xmm0, %%xmm0\n"
        "    mulps 16(%[consts]), %%xmm0\n"
        "    movaps %%xmm10, %%xmm12\n"
        "    mulps 32(%[consts]), %%xmm12\n"
        "    movaps %%xmm7, %%xmm13\n"
        "    mulps 32(%[consts]), %%xmm13\n"
        "    movaps %%xmm7, %%xmm7\n"
        "    mulps 48(%[consts]), %%xmm7\n"
        "    movaps %%xmm10, %%xmm10\n"
        "    mulps 48(%[consts]), %%xmm10\n"
        "    movaps %%xmm2, %%xmm14\n"
        "    mulps 64(%[consts]), %%xmm14\n"
        "    movaps %%xmm5, %%xmm15\n"
        "    mulps 64(%[consts]), %%xmm15\n"
        "    movaps %%xmm5, %%xmm5\n"
        "    mulps 80(%[consts]), %%xmm5\n"
        "    movaps %%xmm2, %%xmm2\n"
        "    mulps 80(%[consts]), %%xmm2\n"
        "    movaps %%xmm1, %%xmm1\n"
        "    unpcklpd %%xmm6, %%xmm1\n"
        "    subps %%xmm9, %%xmm3\n"
        "    addps %%xmm0, %%xmm11\n"
        "    movaps %%xmm4, %%xmm0\n"
        "    unpcklpd %%xmm8, %%xmm0\n"
        "    subps %%xmm7, %%xmm12\n"
        "    addps %%xmm10, %%xmm13\n"
        "    subps %%xmm5, %%xmm14\n"
        "    addps %%xmm2, %%xmm15\n"
        "    movaps %%xmm1, %%xmm2\n"
        "    addps %%xmm12, %%xmm2\n"
        "    movaps %%xmm1, %%xmm1\n"
        "    subps %%xmm12, %%xmm1\n"
        "    movaps %%xmm0, %%xmm4\n"
        "    addps %%xmm13, %%xmm4\n"
        "    movaps %%xmm0, %%xmm0\n"
        "    subps %%xmm13, %%xmm0\n"
        "    movaps %%xmm3, %%xmm5\n"
        "    addps %%xmm14, %%xmm5\n"
        "    movaps %%xmm11, %%xmm6\n"
        "    addps %%xmm15, %%xmm6\n"
        "    movaps %%xmm3, %%xmm3\n"
        "    subps %%xmm14, %%xmm3\n"
        "    movaps %%xmm11, %%xmm7\n"
        "    subps %%xmm15, %%xmm7\n"
        "    movaps %%xmm2, %%xmm8\n"
        "    addps %%xmm5, %%xmm8\n"
        "    movaps %%xmm4, %%xmm9\n"
        "    addps %%xmm6, %%xmm9\n"
        "    movaps %%xmm2, %%xmm2\n"
        "    subps %%xmm5, %%xmm2\n"
        "    movaps %%xmm4, %%xmm4\n"
        "    subps %%xmm6, %%xmm4\n"
        "    movaps %%xmm1, %%xmm5\n"
        "    addps %%xmm7, %%xmm5\n"
        "    movaps %%xmm0, %%xmm6\n"
        "    subps %%xmm3, %%xmm6\n"
        "    movaps %%xmm1, %%xmm1\n"
        "    subps %%xmm7, %%xmm1\n"
        "    movaps %%xmm0, %%xmm0\n"
        "    addps %%xmm3, %%xmm0\n"
        "    movaps %%xmm8, %%xmm3\n"
        "    unpcklps %%xmm9, %%xmm3\n"
        "    movaps %%xmm8, %%xmm7\n"
        "    unpckhps %%xmm9, %%xmm7\n"
        "    movaps %%xmm5, %%xmm8\n"
        "    unpcklps %%xmm6, %%xmm8\n"
        "    movaps %%xmm5, %%xmm5\n"
        "    unpckhps %%xmm6, %%xmm5\n"
        "    movaps %%xmm2, %%xmm6\n"
        "    unpcklps %%xmm4, %%xmm6\n"
        "    movups %%xmm3, (%[out])\n"
        "    movups %%xmm7, 16(%[out])\n"
        "    movaps %%xmm2, %%xmm2\n"
        "    unpckhps %%xmm4, %%xmm2\n"
        "    movaps %%xmm1, %%xmm3\n"
        "    unpcklps %%xmm0, %%xmm3\n"
        "    movaps %%xmm1, %%xmm1\n"
        "    unpckhps %%xmm0, %%xmm1\n"
        "    movups %%xmm8, 32(%[out])\n"
        "    movups %%xmm5, 48(%[out])\n"
        "    movups %%xmm6, 64(%[out])\n"
        "    movups %%xmm2, 80(%[out])\n"
        "    movups %%xmm3, 96(%[out])\n"
        "    movups %%xmm1, 112(%[out])\n"
        : [in] "+r"(in), [out] "+r"(out), [consts] "+r"(consts)
        :
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "cc", "memory");
}

/* First pass, radix 4. For each of the `count` entries (a, b) of `table`: reads 4 interleaved
 * vectors from in + 8*a + u*in_stride, transforms across them and writes output q as a split
 * vector to out + 8*b + r*out_stride, where r is q with its bits reversed. */
static void onefft_sse2_f4i(const float *in, float *out, const uint32_t *table, size_t count, size_t in_stride, size_t out_stride) {
    const float *src;
    float *dst;
    size_t a;
    size_t b;
    __asm__ volatile(
        "1:\n"
        "    movl (%[table]), %k[a]\n"
        "    movl 4(%[table]), %k[b]\n"
        "    lea 8(%[table]), %[table]\n"
        "    lea (%[in],%[a],8), %[src]\n"
        "    lea (%[out],%[b],8), %[dst]\n"
        "    movups (%[src]), %%xmm0\n"
        "    movups 16(%[src]), %%xmm1\n"
        "    lea (%[src],%[in_stride],1), %[src]\n"
        "    movups (%[src]), %%xmm2\n"
        "    movups 16(%[src]), %%xmm3\n"
        "    lea (%[src],%[in_stride],1), %[src]\n"
        "    movups (%[src]), %%xmm4\n"
        "    movups 16(%[src]), %%xmm5\n"
        "    lea (%[src],%[in_stride],1), %[src]\n"
        "    movaps %%xmm0, %%xmm6\n"
        "    shufps $136, %%xmm1, %%xmm6\n"
        "    movaps %%xmm0, %%xmm0\n"
        "    shufps $221, %%xmm1, %%xmm0\n"
        "    movups (%[src]), %%xmm1\n"
        "    movups 16(%[src]), %%xmm7\n"
        "    movaps %%xmm2, %%xmm8\n"
        "    shufps $136, %%xmm3, %%xmm8\n"
        "    movaps %%xmm2, %%xmm2\n"
        "    shufps $221, %%xmm3, %%xmm2\n"
        "    movaps %%xmm4, %%xmm3\n"
        "    shufps $136, %%xmm5, %%xmm3\n"
        "    movaps %%xmm4, %%xmm4\n"
        "    shufps $221, %%xmm5, %%xmm4\n"
        "    movaps %%xmm0, %%xmm5\n"
        "    addps %%xmm4, %%xmm5\n"
        "    movaps %%xmm6, %%xmm9\n"
        "    addps %%xmm3, %%xmm9\n"
        "    movaps %%xmm0, %%xmm0\n"
        "    subps %%xmm4, %%xmm0\n"
        "    movaps %%xmm1, %%xmm4\n"
        "    shufps $136, %%xmm7, %%xmm4\n"
        "    movaps %%xmm1, %%xmm1\n"
        "    shufps $221, %%xmm7, %%xmm1\n"
        "    movaps %%xmm6, %%xmm6\n"
        "    subps %%xmm3, %%xmm6\n"
        "    movaps %%xmm2, %%xmm3\n"
        "    addps %%xmm1, %%xmm3\n"
        "    movaps %%xmm8, %%xmm7\n"
        "    addps %%xmm4, %%xmm7\n"
        "    movaps %%xmm2, %%xmm2\n"
        "    subps %%xmm1, %%xmm2\n"
        "    movaps %%xmm8, %%xmm1\n"
        "    subps %%xmm4, %%xmm1\n"
        "    movaps %%xmm5, %%xmm4\n"
        "    addps %%xmm3, %%xmm4\n"
        "    movaps %%xmm9, %%xmm8\n"
        "    addps %%xmm7, %%xmm8\n"
        "    movaps %%xmm5, %%xmm5\n"
        "    subps %%xmm3, %%xmm5\n"
        "    movaps %%xmm9, %%xmm3\n"
        "    subps %%xmm7, %%xmm3\n"
        "    movaps %%xmm0, %%xmm7\n"
        "    addps %%xmm1, %%xmm7\n"
        "    movaps %%xmm6, %%xmm9\n"
        "    subps %%xmm2, %%xmm9\n"
        "    movaps %%xmm0, %%xmm0\n"
        "    subps %%xmm1, %%xmm0\n"
        "    movaps %%xmm6, %%xmm1\n"
        "    addps %%xmm2, %%xmm1\n"
        "    movups %%xmm4, (%[dst])\n"
        "    movups %%xmm8, 16(%[dst])\n"
        "    lea (%[dst],%[out_stride],1), %[dst]\n"
        "    movups %%xmm5, (%[dst])\n"
        "    movups %%xmm3, 16(%[dst])\n"
        "    lea (%[dst],%[out_stride],1), %[dst]\n"
        "    movups %%xmm7, (%[dst])\n"
        "    movups %%xmm9, 16(%[dst])\n"
        "    lea (%[dst],%[out_stride],1), %[dst]\n"
        "    movups %%xmm0, (%[dst])\n"
        "    movups %%xmm1, 16(%[dst])\n"
        "    sub $1, %[count]\n"
        "    jnz 1b\n"
        : [in] "+r"(in), [out] "+r"(out), [table] "+r"(table), [count] "+r"(count), [in_stride] "+r"(in_stride), [out_stride] "+r"(out_stride), [src] "=&r"(src), [dst] "=&r"(dst), [a] "=&r"(a), [b] "=&r"(b)
        :
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "cc", "memory");
}

/* First pass, radix 2. For each of the `count` entries (a, b) of `table`: reads 2 interleaved
 * vectors from in + 8*a + u*in_stride, transforms across them and writes output q as a split
 * vector to out + 8*b + r*out_stride, where r is q with its bits reversed. */
static void onefft_sse2_f2i(const float *in, float *out, const uint32_t *table, size_t count, size_t in_stride, size_t out_stride) {
    const float *src;
    float *dst;
    size_t a;
    size_t b;
    __asm__ volatile(
        "1:\n"
        "    movl (%[table]), %k[a]\n"
        "    movl 4(%[table]), %k[b]\n"
        "    lea 8(%[table]), %[table]\n"
        "    lea (%[in],%[a],8), %[src]\n"
        "    lea (%[out],%[b],8), %[dst]\n"
        "    movups (%[src]), %%xmm0\n"
        "    movups 16(%[src]), %%xmm1\n"
        "    lea (%[src],%[in_stride],1), %[src]\n"
        "    movups (%[src]), %%xmm2\n"
        "    movups 16(%[src]), %%xmm3\n"
        "    movaps %%xmm0, %%xmm4\n"
        "    shufps $136, %%xmm1, %%xmm4\n"
        "    movaps %%xmm0, %%xmm0\n"
        "    shufps $221, %%xmm1, %%xmm0\n"
        "    movaps %%xmm2, %%xmm1\n"
        "    shufps $136, %%xmm3, %%xmm1\n"
        "    movaps %%xmm2, %%xmm2\n"
        "    shufps $221, %%xmm3, %%xmm2\n"
        "    movaps %%xmm0, %%xmm3\n"
        "    addps %%xmm2, %%xmm3\n"
        "    movaps %%xmm4, %%xmm5\n"
        "    addps %%xmm1, %%xmm5\n"
        "    movaps %%xmm0, %%xmm0\n"
        "    subps %%xmm2, %%xmm0\n"
        "    movaps %%xmm4, %%xmm2\n"
        "    subps %%xmm1, %%xmm2\n"
        "    movups %%xmm3, (%[dst])\n"
        "    movups %%xmm5, 16(%[dst])\n"
        "    lea (%[dst],%[out_stride],1), %[dst]\n"
        "    movups %%xmm0, (%[dst])\n"
        "    movups %%xmm2, 16(%[dst])\n"
        "    sub $1, %[count]\n"
        "    jnz 1b\n"
        : [in] "+r"(in), [out] "+r"(out), [table] "+r"(table), [count] "+r"(count), [in_stride] "+r"(in_stride), [out_stride] "+r"(out_stride), [src] "=&r"(src), [dst] "=&r"(dst), [a] "=&r"(a), [b] "=&r"(b)
        :
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "cc", "memory");
}

/* Last pass, radix 4, in place: reads split vectors and writes the interleaved result.
 * Butterfly j < count uses the vectors at data + r*stride + 32*j. The table holds per butterfly,
 * for l = 1..3, the real and the imaginary parts of the 4 twiddles of the vector with index bits
 * (1, 0) = l. */
static void onefft_sse2_l4i(float *data, const float *tw, size_t stride, size_t count, size_t regions) {
    (void)regions;
    float *third;
    __asm__ volatile(
        "    lea (%[data],%[stride],2), %[third]\n"
        "    lea (%[third],%[stride],1), %[third]\n"
        "1:\n"
        "    movups (%[data]), %%xmm0\n"
        "    movups 16(%[data]), %%xmm1\n"
        "    movups (%[data],%[stride],1), %%xmm2\n"
        "    movups 16(%[data],%[stride],1), %%xmm3\n"
        "    movups (%[data],%[stride],2), %%xmm4\n"
        "    movups 16(%[data],%[stride],2), %%xmm5\n"
        "    movups (%[third]), %%xmm6\n"
        "    movups 16(%[third]), %%xmm7\n"
        "    movaps %%xmm0, %%xmm8\n"
        "    unpcklps %%xmm4, %%xmm8\n"
        "    movaps %%xmm0, %%xmm0\n"
        "    unpckhps %%xmm4, %%xmm0\n"
        "    movaps %%xmm1, %%xmm4\n"
        "    unpcklps %%xmm5, %%xmm4\n"
        "    movaps %%xmm1, %%xmm1\n"
        "    unpckhps %%xmm5, %%xmm1\n"
        "    movaps %%xmm2, %%xmm5\n"
        "    unpcklps %%xmm6, %%xmm5\n"
        "    movaps %%xmm2, %%xmm2\n"
        "    unpckhps %%xmm6, %%xmm2\n"
        "    movaps %%xmm3, %%xmm6\n"
        "    unpcklps %%xmm7, %%xmm6\n"
        "    movaps %%xmm3, %%xmm3\n"
        "    unpckhps %%xmm7, %%xmm3\n"
        "    movaps %%xmm8, %%xmm7\n"
        "    unpckhpd %%xmm5, %%xmm7\n"
        "    movaps %%xmm4, %%xmm9\n"
        "    unpckhpd %%xmm6, %%xmm9\n"
        "    movaps %%xmm0, %%xmm10\n"
        "    unpcklpd %%xmm2, %%xmm10\n"
        "    movaps %%xmm0, %%xmm0\n"
        "    unpckhpd %%xmm2, %%xmm0\n"
        "    movaps %%xmm1, %%xmm2\n"
        "    unpcklpd %%xmm3, %%xmm2\n"
        "    movaps %%xmm1, %%xmm1\n"
        "    unpckhpd %%xmm3, %%xmm1\n"
        "    movaps %%xmm7, %%xmm3\n"
        "    mulps (%[tw]), %%xmm3\n"
        "    movaps %%xmm9, %%xmm11\n"
        "    mulps (%[tw]), %%xmm11\n"
        "    movaps %%xmm9, %%xmm9\n"
        "    mulps 16(%[tw]), %%xmm9\n"
        "    movaps %%xmm7, %%xmm7\n"
        "    mulps 16(%[tw]), %%xmm7\n"
        "    movaps %%xmm10, %%xmm12\n"
        "    mulps 32(%[tw]), %%xmm12\n"
        "    movaps %%xmm2, %%xmm13\n"
        "    mulps 32(%[tw]), %%xmm13\n"
        "    movaps %%xmm2, %%xmm2\n"
        "    mulps 48(%[tw]), %%xmm2\n"
        "    movaps %%xmm10, %%xmm10\n"
        "    mulps 48(%[tw]), %%xmm10\n"
        "    movaps %%xmm0, %%xmm14\n"
        "    mulps 64(%[tw]), %%xmm14\n"
        "    movaps %%xmm1, %%xmm15\n"
        "    mulps 64(%[tw]), %%xmm15\n"
        "    movaps %%xmm1, %%xmm1\n"
        "    mulps 80(%[tw]), %%xmm1\n"
        "    movaps %%xmm0, %%xmm0\n"
        "    mulps 80(%[tw]), %%xmm0\n"
        "    movaps %%xmm8, %%xmm8\n"
        "    unpcklpd %%xmm5, %%xmm8\n"
        "    subps %%xmm9, %%xmm3\n"
        "    addps %%xmm7, %%xmm11\n"
        "    movaps %%xmm4, %%xmm4\n"
        "    unpcklpd %%xmm6, %%xmm4\n"
        "    lea 96(%[tw]), %[tw]\n"
        "    subps %%xmm2, %%xmm12\n"
        "    addps %%xmm10, %%xmm13\n"
        "    subps %%xmm1, %%xmm14\n"
        "    addps %%xmm0, %%xmm15\n"
        "    movaps %%xmm8, %%xmm0\n"
        "    addps %%xmm12, %%xmm0\n"
        "    movaps %%xmm8, %%xmm1\n"
        "    subps %%xmm12, %%xmm1\n"
        "    movaps %%xmm4, %%xmm2\n"
        "    addps %%xmm13, %%xmm2\n"
        "    movaps %%xmm4, %%xmm4\n"
        "    subps %%xmm13, %%xmm4\n"
        "    movaps %%xmm3, %%xmm5\n"
        "    addps %%xmm14, %%xmm5\n"
        "    movaps %%xmm11, %%xmm6\n"
        "    addps %%xmm15, %%xmm6\n"
        "    movaps %%xmm3, %%xmm3\n"
        "    subps %%xmm14, %%xmm3\n"
        "    movaps %%xmm11, %%xmm7\n"
        "    subps %%xmm15, %%xmm7\n"
        "    movaps %%xmm0, %%xmm8\n"
        "    addps %%xmm5, %%xmm8\n"
        "    movaps %%xmm2, %%xmm9\n"
        "    addps %%xmm6, %%xmm9\n"
        "    movaps %%xmm0, %%xmm0\n"
        "    subps %%xmm5, %%xmm0\n"
        "    movaps %%xmm2, %%xmm2\n"
        "    subps %%xmm6, %%xmm2\n"
        "    movaps %%xmm1, %%xmm5\n"
        "    addps %%xmm7, %%xmm5\n"
        "    movaps %%xmm4, %%xmm6\n"
        "    subps %%xmm3, %%xmm6\n"
        "    movaps %%xmm1, %%xmm1\n"
        "    subps %%xmm7, %%xmm1\n"
        "    movaps %%xmm4, %%xmm4\n"
        "    addps %%xmm3, %%xmm4\n"
        "    movaps %%xmm9, %%xmm3\n"
        "    unpcklps %%xmm8, %%xmm3\n"
        "    movaps %%xmm9, %%xmm7\n"
        "    unpckhps %%xmm8, %%xmm7\n"
        "    movaps %%xmm6, %%xmm8\n"
        "    unpcklps %%xmm5, %%xmm8\n"
        "    movaps %%xmm6, %%xmm6\n"
        "    unpckhps %%xmm5, %%xmm6\n"
        "    movaps %%xmm2, %%xmm5\n"
        "    unpcklps %%xmm0, %%xmm5\n"
        "    movups %%xmm3, (%[data])\n"
        "    movups %%xmm7, 16(%[data])\n"
        "    movaps %%xmm2, %%xmm2\n"
        "    unpckhps %%xmm0, %%xmm2\n"
        "    movaps %%xmm4, %%xmm0\n"
        "    unpcklps %%xmm1, %%xmm0\n"
        "    movaps %%xmm4, %%xmm3\n"
        "    unpckhps %%xmm1, %%xmm3\n"
        "    movups %%xmm8, (%[data],%[stride],1)\n"
        "    movups %%xmm6, 16(%[data],%[stride],1)\n"
        "    movups %%xmm5, (%[data],%[stride],2)\n"
        "    movups %%xmm2, 16(%[data],%[stride],2)\n"
        "    movups %%xmm0, (%[third])\n"
        "    movups %%xmm3, 16(%[third])\n"
        "    lea 32(%[data]), %[data]\n"
        "    lea 32(%[third]), %[third]\n"
        "    sub $1, %[count]\n"
        "    jnz 1b\n"
        : [data] "+r"(data), [tw] "+r"(tw), [stride] "+r"(stride), [count] "+r"(count), [third] "=&r"(third)
        :
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "cc", "memory");
}

static const float onefft_sse2_n8i_consts[16] ONEFFT_ALIGNED(64) = {
    0x1.0000000000000p+0f, 0x1.0000000000000p+0f, -0x0.0p+0f, -0x0.0p+0f,
    0x0.0p+0f, 0x0.0p+0f, -0x1.0000000000000p+0f, -0x1.0000000000000p+0f,
    0x1.0000000000000p+0f, 0x1.6a09e667f3bcdp-1f, -0x0.0p+0f, -0x1.6a09e667f3bccp-1f,
    0x0.0p+0f, -0x1.6a09e667f3bccp-1f, -0x1.0000000000000p+0f, -0x1.6a09e667f3bcdp-1f,
};
/* Complete 8 point transform from `in` to `out`, both interleaved. */
static void onefft_sse2_n8i(const float *in, float *out) {
    const float *consts = onefft_sse2_n8i_consts;
    __asm__ volatile(
        "    movups (%[in]), %%xmm0\n"
        "    movups 16(%[in]), %%xmm1\n"
        "    movups 32(%[in]), %%xmm2\n"
        "    movups 48(%[in]), %%xmm3\n"
        "    movaps %%xmm0, %%xmm4\n"
        "    shufps $136, %%xmm1, %%xmm4\n"
        "    movaps %%xmm0, %%xmm0\n"
        "    shufps $221, %%xmm1, %%xmm0\n"
        "    movaps %%xmm2, %%xmm1\n"
        "    shufps $136, %%xmm3, %%xmm1\n"
        "    movaps %%xmm2, %%xmm2\n"
        "    shufps $221, %%xmm3, %%xmm2\n"
        "    movaps %%xmm4, %%xmm3\n"
        "    addps %%xmm1, %%xmm3\n"
        "    movaps %%xmm4, %%xmm4\n"
        "    subps %%xmm1, %%xmm4\n"
        "    movaps %%xmm0, %%xmm1\n"
        "    addps %%xmm2, %%xmm1\n"
        "    movaps %%xmm0, %%xmm0\n"
        "    subps %%xmm2, %%xmm0\n"
        "    movaps %%xmm3, %%xmm2\n"
        "    unpckhpd %%xmm4, %%xmm2\n"
        "    movaps %%xmm3, %%xmm3\n"
        "    unpcklpd %%xmm4, %%xmm3\n"
        "    movaps %%xmm1, %%xmm4\n"
        "    unpckhpd %%xmm0, %%xmm4\n"
        "    movaps %%xmm2, %%xmm5\n"
        "    mulps (%[consts]), %%xmm5\n"
        "    movaps %%xmm2, %%xmm2\n"
        "    mulps 16(%[consts]), %%xmm2\n"
        "    movaps %%xmm4, %%xmm6\n"
        "    mulps (%[consts]), %%xmm6\n"
        "    movaps %%xmm4, %%xmm4\n"
        "    mulps 16(%[consts]), %%xmm4\n"
        "    movaps %%xmm1, %%xmm1\n"
        "    unpcklpd %%xmm0, %%xmm1\n"
        "    subps %%xmm2, %%xmm6\n"
        "    addps %%xmm4, %%xmm5\n"
        "    movaps %%xmm1, %%xmm0\n"
        "    addps %%xmm6, %%xmm0\n"
        "    movaps %%xmm3, %%xmm2\n"
        "    addps %%xmm5, %%xmm2\n"
        "    movaps %%xmm1, %%xmm1\n"
        "    subps %%xmm6, %%xmm1\n"
        "    movaps %%xmm3, %%xmm3\n"
        "    subps %%xmm5, %%xmm3\n"
        "    movaps %%xmm0, %%xmm4\n"
        "    shufps $221, %%xmm1, %%xmm4\n"
        "    movaps %%xmm0, %%xmm0\n"
        "    shufps $136, %%xmm1, %%xmm0\n"
        "    movaps %%xmm2, %%xmm1\n"
        "    shufps $221, %%xmm3, %%xmm1\n"
        "    movaps %%xmm4, %%xmm5\n"
        "    mulps 32(%[consts]), %%xmm5\n"
        "    movaps %%xmm4, %%xmm4\n"
        "    mulps 48(%[consts]), %%xmm4\n"
        "    movaps %%xmm1, %%xmm6\n"
        "    mulps 32(%[consts]), %%xmm6\n"
        "    movaps %%xmm1, %%xmm1\n"
        "    mulps 48(%[consts]), %%xmm1\n"
        "    movaps %%xmm2, %%xmm2\n"
        "    shufps $136, %%xmm3, %%xmm2\n"
        "    subps %%xmm1, %%xmm5\n"
        "    addps %%xmm4, %%xmm6\n"
        "    movaps %%xmm0, %%xmm1\n"
        "    addps %%xmm5, %%xmm1\n"
        "    movaps %%xmm2, %%xmm3\n"
        "    addps %%xmm6, %%xmm3\n"
        "    movaps %%xmm0, %%xmm0\n"
        "    subps %%xmm5, %%xmm0\n"
        "    movaps %%xmm2, %%xmm2\n"
        "    subps %%xmm6, %%xmm2\n"
        "    movaps %%xmm3, %%xmm4\n"
        "    unpcklps %%xmm1, %%xmm4\n"
        "    movaps %%xmm3, %%xmm3\n"
        "    unpckhps %%xmm1, %%xmm3\n"
        "    movaps %%xmm2, %%xmm1\n"
        "    unpcklps %%xmm0, %%xmm1\n"
        "    movaps %%xmm2, %%xmm2\n"
        "    unpckhps %%xmm0, %%xmm2\n"
        "    movups %%xmm4, (%[out])\n"
        "    movups %%xmm3, 16(%[out])\n"
        "    movups %%xmm1, 32(%[out])\n"
        "    movups %%xmm2, 48(%[out])\n"
        : [in] "+r"(in), [out] "+r"(out), [consts] "+r"(consts)
        :
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "cc", "memory");
}

static const float onefft_sse2_n16i_consts[24] ONEFFT_ALIGNED(64) = {
    0x1.0000000000000p+0f, 0x1.d906bcf328d46p-1f, 0x1.6a09e667f3bcdp-1f, 0x1.87de2a6aea964p-2f,
    0x0.0p+0f, -0x1.87de2a6aea963p-2f, -0x1.6a09e667f3bccp-1f, -0x1.d906bcf328d46p-1f,
    0x1.0000000000000p+0f, 0x1.6a09e667f3bcdp-1f, -0x0.0p+0f, -0x1.6a09e667f3bccp-1f,
    0x0.0p+0f, -0x1.6a09e667f3bccp-1f, -0x1.0000000000000p+0f, -0x1.6a09e667f3bcdp-1f,
    0x1.0000000000000p+0f, 0x1.87de2a6aea964p-2f, -0x1.6a09e667f3bccp-1f, -0x1.d906bcf328d47p-1f,
    0x0.0p+0f, -0x1.d906bcf328d46p-1f, -0x1.6a09e667f3bcdp-1f, 0x1.87de2a6aea961p-2f,
};
/* Complete 16 point transform from `in` to `out`, both interleaved. */
static void onefft_sse2_n16i(const float *in, float *out) {
    const float *consts = onefft_sse2_n16i_consts;
    __asm__ volatile(
        "    movups (%[in]), %%xmm0\n"
        "    movups 16(%[in]), %%xmm1\n"
        "    movups 32(%[in]), %%xmm2\n"
        "    movups 48(%[in]), %%xmm3\n"
        "    movups 64(%[in]), %%xmm4\n"
        "    movups 80(%[in]), %%xmm5\n"
        "    movups 96(%[in]), %%xmm6\n"
        "    movups 112(%[in]), %%xmm7\n"
        "    movaps %%xmm0, %%xmm8\n"
        "    shufps $136, %%xmm1, %%xmm8\n"
        "    movaps %%xmm0, %%xmm0\n"
        "    shufps $221, %%xmm1, %%xmm0\n"
        "    movaps %%xmm2, %%xmm1\n"
        "    shufps $136, %%xmm3, %%xmm1\n"
        "    movaps %%xmm2, %%xmm2\n"
        "    shufps $221, %%xmm3, %%xmm2\n"
        "    movaps %%xmm4, %%xmm3\n"
        "    shufps $136, %%xmm5, %%xmm3\n"
        "    movaps %%xmm4, %%xmm4\n"
        "    shufps $221, %%xmm5, %%xmm4\n"
        "    movaps %%xmm6, %%xmm5\n"
        "    shufps $136, %%xmm7, %%xmm5\n"
        "    movaps %%xmm6, %%xmm6\n"
        "    shufps $221, %%xmm7, %%xmm6\n"
        "    movaps %%xmm0, %%xmm7\n"
        "    addps %%xmm4, %%xmm7\n"
        "    movaps %%xmm8, %%xmm9\n"
        "    addps %%xmm3, %%xmm9\n"
        "    movaps %%xmm0, %%xmm0\n"
        "    subps %%xmm4, %%xmm0\n"
        "    movaps %%xmm8, %%xmm4\n"
        "    subps %%xmm3, %%xmm4\n"
        "    movaps %%xmm2, %%xmm3\n"
        "    addps %%xmm6, %%xmm3\n"
        "    movaps %%xmm1, %%xmm8\n"
        "    addps %%xmm5, %%xmm8\n"
        "    movaps %%xmm2, %%xmm2\n"
        "    subps %%xmm6, %%xmm2\n"
        "    movaps %%xmm1, %%xmm1\n"
        "    subps %%xmm5, %%xmm1\n"
        "    movaps %%xmm7, %%xmm5\n"
        "    addps %%xmm3, %%xmm5\n"
        "    movaps %%xmm9, %%xmm6\n"
        "    addps %%xmm8, %%xmm6\n"
        "    movaps %%xmm7, %%xmm7\n"
        "    subps %%xmm3, %%xmm7\n"
        "    movaps %%xmm9, %%xmm3\n"
        "    subps %%xmm8, %%xmm3\n"
        "    movaps %%xmm0, %%xmm8\n"
        "    addps %%xmm1, %%xmm8\n"
        "    movaps %%xmm4, %%xmm9\n"
        "    subps %%xmm2, %%xmm9\n"
        "    movaps %%xmm0, %%xmm0\n"
        "    subps %%xmm1, %%xmm0\n"
        "    movaps %%xmm4, %%xmm1\n"
        "    addps %%xmm2, %%xmm1\n"
        "    movaps %%xmm5, %%xmm2\n"
        "    unpcklps %%xmm8, %%xmm2\n"
        "    movaps %%xmm5, %%xmm4\n"
        "    unpckhps %%xmm8, %%xmm4\n"
        "    movaps %%xmm6, %%xmm5\n"
        "    unpcklps %%xmm9, %%xmm5\n"
        "    movaps %%xmm6, %%xmm6\n"
        "    unpckhps %%xmm9, %%xmm6\n"
        "    movaps %%xmm7, %%xmm8\n"
        "    unpcklps %%xmm0, %%xmm8\n"
        "    movaps %%xmm7, %%xmm7\n"
        "    unpckhps %%xmm0, %%xmm7\n"
        "    movaps %%xmm3, %%xmm0\n"
        "    unpcklps %%xmm1, %%xmm0\n"
        "    movaps %%xmm3, %%xmm3\n"
        "    unpckhps %%xmm1, %%xmm3\n"
        "    movaps %%xmm2, %%xmm1\n"
        "    unpckhpd %%xmm8, %%xmm1\n"
        "    movaps %%xmm5, %%xmm9\n"
        "    unpckhpd %%xmm0, %%xmm9\n"
        "    movaps %%xmm4, %%xmm10\n"
        "    unpcklpd %%xmm7, %%xmm10\n"
        "    movaps %%xmm4, %%xmm4\n"
        "    unpckhpd %%xmm7, %%xmm4\n"
        "    movaps %%xmm6, %%xmm7\n"
        "    unpcklpd %%xmm3, %%xmm7\n"
        "    movaps %%xmm6, %%xmm6\n"
        "    unpckhpd %%xmm3, %%xmm6\n"
        "    movaps %%xmm1, %%xmm3\n"
        "    mulps (%[consts]), %%xmm3\n"
        "    movaps %%xmm9, %%xmm11\n"
        "    mulps (%[consts]), %%xmm11\n"
        "    movaps %%xmm9, %%xmm9\n"
        "    mulps 16(%[consts]), %%xmm9\n"
        "    movaps %%xmm1, %%xmm1\n"
        "    mulps 16(%[consts]), %%xmm1\n"
        "    movaps %%xmm10, %%xmm12\n"
        "    mulps 32(%[consts]), %%xmm12\n"
        "    movaps %%xmm7, %%xmm13\n"
        "    mulps 32(%[consts]), %%xmm13\n"
        "    movaps %%xmm7, %%xmm7\n"
        "    mulps 48(%[consts]), %%xmm7\n"
        "    movaps %%xmm10, %%xmm10\n"
        "    mulps 48(%[consts]), %%xmm10\n"
        "    movaps %%xmm4, %%xmm14\n"
        "    mulps 64(%[consts]), %%xmm14\n"
        "    movaps %%xmm6, %%xmm15\n"
        "    mulps 64(%[consts]), %%xmm15\n"
        "    movaps %%xmm6, %%xmm6\n"
        "    mulps 80(%[consts]), %%xmm6\n"
        "    movaps %%xmm4, %%xmm4\n"
        "    mulps 80(%[consts]), %%xmm4\n"
        "    movaps %%xmm2, %%xmm2\n"
        "    unpcklpd %%xmm8, %%xmm2\n"
        "    subps %%xmm9, %%xmm3\n"
        "    addps %%xmm1, %%xmm11\n"
        "    movaps %%xmm5, %%xmm1\n"
        "    unpcklpd %%xmm0, %%xmm1\n"
        "    subps %%xmm7, %%xmm12\n"
        "    addps %%xmm10, %%xmm13\n"
        "    subps %%xmm6, %%xmm14\n"
        "    addps %%xmm4, %%xmm15\n"
        "    movaps %%xmm2, %%xmm0\n"
        "    addps %%xmm12, %%xmm0\n"
        "    movaps %%xmm2, %%xmm2\n"
        "    subps %%xmm12, %%xmm2\n"
        "    movaps %%xmm1, %%xmm4\n"
        "    addps %%xmm13, %%xmm4\n"
        "    movaps %%xmm1, %%xmm1\n"
        "    subps %%xmm13, %%xmm1\n"
        "    movaps %%xmm3, %%xmm5\n"
        "    addps %%xmm14, %%xmm5\n"
        "    movaps %%xmm11, %%xmm6\n"
        "    addps %%xmm15, %%xmm6\n"
        "    movaps %%xmm3, %%xmm3\n"
        "    subps %%xmm14, %%xmm3\n"
        "    movaps %%xmm11, %%xmm7\n"
        "    subps %%xmm15, %%xmm7\n"
        "    movaps %%xmm0, %%xmm8\n"
        "    addps %%xmm5, %%xmm8\n"
        "    movaps %%xmm4, %%xmm9\n"
        "    addps %%xmm6, %%xmm9\n"
        "    movaps %%xmm0, %%xmm0\n"
        "    subps %%xmm5, %%xmm0\n"
        "    movaps %%xmm4, %%xmm4\n"
        "    subps %%xmm6, %%xmm4\n"
        "    movaps %%xmm2, %%xmm5\n"
        "    addps %%xmm7, %%xmm5\n"
        "    movaps %%xmm1, %%xmm6\n"
        "    subps %%xmm3, %%xmm6\n"
        "    movaps %%xmm2, %%xmm2\n"
        "    subps %%xmm7, %%xmm2\n"
        "    movaps %%xmm1, %%xmm1\n"
        "    addps %%xmm3, %%xmm1\n"
        "    movaps %%xmm9, %%xmm3\n"
        "    unpcklps %%xmm8, %%xmm3\n"
        "    movaps %%xmm9, %%xmm7\n"
        "    unpckhps %%xmm8, %%xmm7\n"
        "    movaps %%xmm6, %%xmm8\n"
        "    unpcklps %%xmm5, %%xmm8\n"
        "    movaps %%xmm6, %%xmm6\n"
        "    unpckhps %%xmm5, %%xmm6\n"
        "    movaps %%xmm4, %%xmm5\n"
        "    unpcklps %%xmm0, %%xmm5\n"
        "    movups %%xmm3, (%[out])\n"
        "    movups %%xmm7, 16(%[out])\n"
        "    movaps %%xmm4, %%xmm3\n"
        "    unpckhps %%xmm0, %%xmm3\n"
        "    movaps %%xmm1, %%xmm0\n"
        "    unpcklps %%xmm2, %%xmm0\n"
        "    movaps %%xmm1, %%xmm1\n"
        "    unpckhps %%xmm2, %%xmm1\n"
        "    movups %%xmm8, 32(%[out])\n"
        "    movups %%xmm6, 48(%[out])\n"
        "    movups %%xmm5, 64(%[out])\n"
        "    movups %%xmm3, 80(%[out])\n"
        "    movups %%xmm0, 96(%[out])\n"
        "    movups %%xmm1, 112(%[out])\n"
        : [in] "+r"(in), [out] "+r"(out), [consts] "+r"(consts)
        :
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "cc", "memory");
}

/* Middle pass, radix 4, in place on split vectors. `data` holds `regions` regions of `blocks`
 * blocks. Input i of butterfly j of a block is the vector at (i/2)*args.s_high + (i%2)*args.s_low
 * + j*V (V = bytes per vector), j < args.inner; see passes.py for how the blocks and regions follow
 * each other. Butterfly j multiplies input i by w_j^i and transforms across the vectors; output q
 * goes to (q%2)*s_high + (q/2)*s_low. The table `tw` holds per butterfly the real parts of w^1..w^3
 * and one unused float, then the imaginary parts. */
static void onefft_sse2_c4(float *data, const float *tw, const size_t *args, size_t blocks, size_t regions) {
    float *upper;
    size_t s_low;
    const float *twp;
    size_t j;
    size_t b;
    size_t scratch;
    __asm__ volatile(
        "    mov 0(%[args]), %[s_low]\n"
        "    lea (%[data],%[s_low],1), %[upper]\n"
        "    mov 8(%[args]), %[s_low]\n"
        "1:\n"
        "    mov %[blocks], %[b]\n"
        "2:\n"
        "    mov %[tw], %[twp]\n"
        "    mov 16(%[args]), %[j]\n"
        "3:\n"
        "    movups (%[data],%[s_low],1), %%xmm0\n"
        "    movups 16(%[data],%[s_low],1), %%xmm1\n"
        "    movups (%[upper]), %%xmm2\n"
        "    movups 16(%[upper]), %%xmm3\n"
        "    movups (%[upper],%[s_low],1), %%xmm4\n"
        "    movups 16(%[upper],%[s_low],1), %%xmm5\n"
        "    movss (%[twp]), %%xmm6\n"
        "    shufps $0, %%xmm6, %%xmm6\n"
        "    movss 16(%[twp]), %%xmm7\n"
        "    shufps $0, %%xmm7, %%xmm7\n"
        "    movss 4(%[twp]), %%xmm8\n"
        "    shufps $0, %%xmm8, %%xmm8\n"
        "    movss 20(%[twp]), %%xmm9\n"
        "    shufps $0, %%xmm9, %%xmm9\n"
        "    movss 8(%[twp]), %%xmm10\n"
        "    shufps $0, %%xmm10, %%xmm10\n"
        "    movss 24(%[twp]), %%xmm11\n"
        "    shufps $0, %%xmm11, %%xmm11\n"
        "    movups (%[data]), %%xmm12\n"
        "    movups 16(%[data]), %%xmm13\n"
        "    lea 32(%[twp]), %[twp]\n"
        "    movaps %%xmm0, %%xmm14\n"
        "    mulps %%xmm6, %%xmm14\n"
        "    movaps %%xmm1, %%xmm15\n"
        "    mulps %%xmm6, %%xmm15\n"
        "    movaps %%xmm1, %%xmm1\n"
        "    mulps %%xmm7, %%xmm1\n"
        "    movaps %%xmm0, %%xmm0\n"
        "    mulps %%xmm7, %%xmm0\n"
        "    movaps %%xmm2, %%xmm6\n"
        "    mulps %%xmm8, %%xmm6\n"
        "    movaps %%xmm3, %%xmm7\n"
        "    mulps %%xmm8, %%xmm7\n"
        "    movaps %%xmm3, %%xmm3\n"
        "    mulps %%xmm9, %%xmm3\n"
        "    movaps %%xmm2, %%xmm2\n"
        "    mulps %%xmm9, %%xmm2\n"
        "    movaps %%xmm4, %%xmm8\n"
        "    mulps %%xmm10, %%xmm8\n"
        "    movaps %%xmm5, %%xmm9\n"
        "    mulps %%xmm10, %%xmm9\n"
        "    movaps %%xmm5, %%xmm5\n"
        "    mulps %%xmm11, %%xmm5\n"
        "    movaps %%xmm4, %%xmm4\n"
        "    mulps %%xmm11, %%xmm4\n"
        "    subps %%xmm1, %%xmm14\n"
        "    addps %%xmm0, %%xmm15\n"
        "    subps %%xmm3, %%xmm6\n"
        "    addps %%xmm2, %%xmm7\n"
        "    subps %%xmm5, %%xmm8\n"
        "    addps %%xmm4, %%xmm9\n"
        "    movaps %%xmm12, %%xmm0\n"
        "    addps %%xmm6, %%xmm0\n"
        "    movaps %%xmm13, %%xmm1\n"
        "    addps %%xmm7, %%xmm1\n"
        "    movaps %%xmm12, %%xmm2\n"
        "    subps %%xmm6, %%xmm2\n"
        "    movaps %%xmm13, %%xmm3\n"
        "    subps %%xmm7, %%xmm3\n"
        "    movaps %%xmm14, %%xmm4\n"
        "    addps %%xmm8, %%xmm4\n"
        "    movaps %%xmm15, %%xmm5\n"
        "    addps %%xmm9, %%xmm5\n"
        "    movaps %%xmm14, %%xmm6\n"
        "    subps %%xmm8, %%xmm6\n"
        "    movaps %%xmm15, %%xmm7\n"
        "    subps %%xmm9, %%xmm7\n"
        "    movaps %%xmm0, %%xmm8\n"
        "    addps %%xmm4, %%xmm8\n"
        "    movaps %%xmm1, %%xmm9\n"
        "    addps %%xmm5, %%xmm9\n"
        "    movaps %%xmm0, %%xmm0\n"
        "    subps %%xmm4, %%xmm0\n"
        "    movaps %%xmm1, %%xmm1\n"
        "    subps %%xmm5, %%xmm1\n"
        "    movaps %%xmm2, %%xmm4\n"
        "    addps %%xmm7, %%xmm4\n"
        "    movaps %%xmm3, %%xmm5\n"
        "    subps %%xmm6, %%xmm5\n"
        "    movaps %%xmm2, %%xmm2\n"
        "    subps %%xmm7, %%xmm2\n"
        "    movaps %%xmm3, %%xmm3\n"
        "    addps %%xmm6, %%xmm3\n"
        "    movups %%xmm8, (%[data])\n"
        "    movups %%xmm9, 16(%[data])\n"
        "    movups %%xmm0, (%[data],%[s_low],1)\n"
        "    movups %%xmm4, (%[upper])\n"
        "    movups %%xmm5, 16(%[upper])\n"
        "    movups %%xmm1, 16(%[data],%[s_low],1)\n"
        "    movups %%xmm2, (%[upper],%[s_low],1)\n"
        "    movups %%xmm3, 16(%[upper],%[s_low],1)\n"
        "    lea 32(%[data]), %[data]\n"
        "    lea 32(%[upper]), %[upper]\n"
        "    sub $1, %[j]\n"
        "    jnz 3b\n"
        "    mov 24(%[args]), %[scratch]\n"
        "    lea (%[data],%[scratch],1), %[data]\n"
        "    lea (%[upper],%[scratch],1), %[upper]\n"
        "    sub $1, %[b]\n"
        "    jnz 2b\n"
        "    mov 32(%[args]), %[scratch]\n"
        "    lea (%[data],%[scratch],1), %[data]\n"
        "    lea (%[upper],%[scratch],1), %[upper]\n"
        "    mov 40(%[args]), %[scratch]\n"
        "    lea (%[tw],%[scratch],1), %[tw]\n"
        "    sub $1, %[regions]\n"
        "    jnz 1b\n"
        : [data] "+r"(data), [tw] "+r"(tw), [args] "+r"(args), [blocks] "+r"(blocks), [regions] "+r"(regions), [upper] "=&r"(upper), [s_low] "=&r"(s_low), [twp] "=&r"(twp), [j] "=&r"(j), [b] "=&r"(b), [scratch] "=&r"(scratch)
        :
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "cc", "memory");
}

#pragma GCC diagnostic pop
#ifndef ONEFFT_NO_AVX2
#define ONEFFT_AVX2 1
/*
 * OneFFT x86-64 AVX2 and FMA kernels.
 *
 * GENERATED FILE, DO NOT EDIT. Change gen/onefft_gen and run gen/generate.py instead.
 *
 * Every kernel is a normal C function whose body is one inline assembly block. The SIMD registers
 * are assigned by the generator, the general purpose registers by the C compiler.
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverlength-strings"

/* First pass, radix 4. For each of the `count` entries (a, b) of `table`: reads 4 interleaved
 * vectors from in + 8*a + u*in_stride, transforms across them and writes output q as a split
 * vector to out + 8*b + r*out_stride, where r is q with its bits reversed. */
__attribute__((target("avx2,fma"))) static void onefft_avx2_f4(const float *in, float *out, const uint32_t *table, size_t count, size_t in_stride, size_t out_stride) {
    const float *src;
    float *dst;
    size_t a;
    size_t b;
    __asm__ volatile(
        "1:\n"
        "    movl (%[table]), %k[a]\n"
        "    movl 4(%[table]), %k[b]\n"
        "    lea 8(%[table]), %[table]\n"
        "    lea (%[in],%[a],8), %[src]\n"
        "    lea (%[out],%[b],8), %[dst]\n"
        "    vmovups (%[src]), %%ymm0\n"
        "    vshufps $136, 32(%[src]), %%ymm0, %%ymm1\n"
        "    vshufps $221, 32(%[src]), %%ymm0, %%ymm0\n"
        "    lea (%[src],%[in_stride],1), %[src]\n"
        "    vmovups (%[src]), %%ymm2\n"
        "    vshufps $136, 32(%[src]), %%ymm2, %%ymm3\n"
        "    vshufps $221, 32(%[src]), %%ymm2, %%ymm2\n"
        "    lea (%[src],%[in_stride],1), %[src]\n"
        "    vmovups (%[src]), %%ymm4\n"
        "    vshufps $136, 32(%[src]), %%ymm4, %%ymm5\n"
        "    vshufps $221, 32(%[src]), %%ymm4, %%ymm4\n"
        "    lea (%[src],%[in_stride],1), %[src]\n"
        "    vaddps %%ymm5, %%ymm1, %%ymm6\n"
        "    vaddps %%ymm4, %%ymm0, %%ymm7\n"
        "    vsubps %%ymm5, %%ymm1, %%ymm1\n"
        "    vmovups (%[src]), %%ymm5\n"
        "    vsubps %%ymm4, %%ymm0, %%ymm0\n"
        "    vshufps $136, 32(%[src]), %%ymm5, %%ymm4\n"
        "    vshufps $221, 32(%[src]), %%ymm5, %%ymm5\n"
        "    vaddps %%ymm4, %%ymm3, %%ymm8\n"
        "    vaddps %%ymm5, %%ymm2, %%ymm9\n"
        "    vsubps %%ymm4, %%ymm3, %%ymm3\n"
        "    vsubps %%ymm5, %%ymm2, %%ymm2\n"
        "    vaddps %%ymm8, %%ymm6, %%ymm4\n"
        "    vaddps %%ymm9, %%ymm7, %%ymm5\n"
        "    vsubps %%ymm8, %%ymm6, %%ymm6\n"
        "    vsubps %%ymm9, %%ymm7, %%ymm7\n"
        "    vaddps %%ymm2, %%ymm1, %%ymm8\n"
        "    vsubps %%ymm3, %%ymm0, %%ymm9\n"
        "    vsubps %%ymm2, %%ymm1, %%ymm1\n"
        "    vaddps %%ymm3, %%ymm0, %%ymm0\n"
        "    vmovups %%ymm4, (%[dst])\n"
        "    vmovups %%ymm5, 32(%[dst])\n"
        "    lea (%[dst],%[out_stride],1), %[dst]\n"
        "    vmovups %%ymm6, (%[dst])\n"
        "    vmovups %%ymm7, 32(%[dst])\n"
        "    lea (%[dst],%[out_stride],1), %[dst]\n"
        "    vmovups %%ymm8, (%[dst])\n"
        "    vmovups %%ymm9, 32(%[dst])\n"
        "    lea (%[dst],%[out_stride],1), %[dst]\n"
        "    vmovups %%ymm1, (%[dst])\n"
        "    vmovups %%ymm0, 32(%[dst])\n"
        "    sub $1, %[count]\n"
        "    jnz 1b\n"
        "    vzeroupper\n"
        : [in] "+r"(in), [out] "+r"(out), [table] "+r"(table), [count] "+r"(count), [in_stride] "+r"(in_stride), [out_stride] "+r"(out_stride), [src] "=&r"(src), [dst] "=&r"(dst), [a] "=&r"(a), [b] "=&r"(b)
        :
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "cc", "memory");
}

static const float onefft_avx2_f8_consts[16] ONEFFT_ALIGNED(64) = {
    0x1.0000000000000p+1f, 0x1.0000000000000p+1f, 0x1.0000000000000p+1f, 0x1.0000000000000p+1f,
    0x1.0000000000000p+1f, 0x1.0000000000000p+1f, 0x1.0000000000000p+1f, 0x1.0000000000000p+1f,
    0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f,
    0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f,
};
/* First pass, radix 8. For each of the `count` entries (a, b) of `table`: reads 8 interleaved
 * vectors from in + 8*a + u*in_stride, transforms across them and writes output q as a split
 * vector to out + 8*b + r*out_stride, where r is q with its bits reversed. */
__attribute__((target("avx2,fma"))) static void onefft_avx2_f8(const float *in, float *out, const uint32_t *table, size_t count, size_t in_stride, size_t out_stride) {
    const float *src;
    float *dst;
    size_t a;
    size_t b;
    const float *consts = onefft_avx2_f8_consts;
    __asm__ volatile(
        "1:\n"
        "    movl (%[table]), %k[a]\n"
        "    movl 4(%[table]), %k[b]\n"
        "    lea 8(%[table]), %[table]\n"
        "    lea (%[in],%[a],8), %[src]\n"
        "    lea (%[out],%[b],8), %[dst]\n"
        "    vmovups (%[src]), %%ymm0\n"
        "    vshufps $136, 32(%[src]), %%ymm0, %%ymm1\n"
        "    vshufps $221, 32(%[src]), %%ymm0, %%ymm0\n"
        "    lea (%[src],%[in_stride],1), %[src]\n"
        "    vmovups (%[src]), %%ymm2\n"
        "    vshufps $136, 32(%[src]), %%ymm2, %%ymm3\n"
        "    vshufps $221, 32(%[src]), %%ymm2, %%ymm2\n"
        "    lea (%[src],%[in_stride],1), %[src]\n"
        "    vmovups (%[src]), %%ymm4\n"
        "    vshufps $136, 32(%[src]), %%ymm4, %%ymm5\n"
        "    vshufps $221, 32(%[src]), %%ymm4, %%ymm4\n"
        "    lea (%[src],%[in_stride],1), %[src]\n"
        "    vmovups (%[src]), %%ymm6\n"
        "    vshufps $136, 32(%[src]), %%ymm6, %%ymm7\n"
        "    vshufps $221, 32(%[src]), %%ymm6, %%ymm6\n"
        "    lea (%[src],%[in_stride],1), %[src]\n"
        "    vmovups (%[src]), %%ymm8\n"
        "    vshufps $136, 32(%[src]), %%ymm8, %%ymm9\n"
        "    vshufps $221, 32(%[src]), %%ymm8, %%ymm8\n"
        "    lea (%[src],%[in_stride],1), %[src]\n"
        "    vaddps %%ymm9, %%ymm1, %%ymm1\n"
        "    vaddps %%ymm8, %%ymm0, %%ymm0\n"
        "    vmovups (%[src]), %%ymm10\n"
        "    vfnmadd132ps (%[consts]), %%ymm1, %%ymm9\n"
        "    vfnmadd132ps (%[consts]), %%ymm0, %%ymm8\n"
        "    vshufps $136, 32(%[src]), %%ymm10, %%ymm11\n"
        "    vshufps $221, 32(%[src]), %%ymm10, %%ymm10\n"
        "    lea (%[src],%[in_stride],1), %[src]\n"
        "    vaddps %%ymm11, %%ymm3, %%ymm3\n"
        "    vaddps %%ymm10, %%ymm2, %%ymm2\n"
        "    vmovups (%[src]), %%ymm12\n"
        "    vfnmadd132ps (%[consts]), %%ymm3, %%ymm11\n"
        "    vfnmadd132ps (%[consts]), %%ymm2, %%ymm10\n"
        "    vshufps $136, 32(%[src]), %%ymm12, %%ymm13\n"
        "    vshufps $221, 32(%[src]), %%ymm12, %%ymm12\n"
        "    lea (%[src],%[in_stride],1), %[src]\n"
        "    vaddps %%ymm13, %%ymm5, %%ymm5\n"
        "    vaddps %%ymm12, %%ymm4, %%ymm4\n"
        "    vmovups (%[src]), %%ymm14\n"
        "    vfnmadd132ps (%[consts]), %%ymm5, %%ymm13\n"
        "    vfnmadd132ps (%[consts]), %%ymm4, %%ymm12\n"
        "    vaddps %%ymm5, %%ymm1, %%ymm1\n"
        "    vaddps %%ymm4, %%ymm0, %%ymm0\n"
        "    vshufps $136, 32(%[src]), %%ymm14, %%ymm15\n"
        "    vshufps $221, 32(%[src]), %%ymm14, %%ymm14\n"
        "    vaddps %%ymm15, %%ymm7, %%ymm7\n"
        "    vaddps %%ymm14, %%ymm6, %%ymm6\n"
        "    vfnmadd132ps (%[consts]), %%ymm1, %%ymm5\n"
        "    vaddps %%ymm12, %%ymm9, %%ymm9\n"
        "    vsubps %%ymm13, %%ymm8, %%ymm8\n"
        "    vfnmadd132ps (%[consts]), %%ymm0, %%ymm4\n"
        "    vfnmadd132ps (%[consts]), %%ymm7, %%ymm15\n"
        "    vfnmadd132ps (%[consts]), %%ymm6, %%ymm14\n"
        "    vaddps %%ymm7, %%ymm3, %%ymm3\n"
        "    vaddps %%ymm6, %%ymm2, %%ymm2\n"
        "    vfnmadd132ps (%[consts]), %%ymm9, %%ymm12\n"
        "    vfmadd132ps (%[consts]), %%ymm8, %%ymm13\n"
        "    vaddps %%ymm14, %%ymm11, %%ymm11\n"
        "    vsubps %%ymm15, %%ymm10, %%ymm10\n"
        "    vfnmadd132ps (%[consts]), %%ymm3, %%ymm7\n"
        "    vfnmadd132ps (%[consts]), %%ymm2, %%ymm6\n"
        "    vaddps %%ymm3, %%ymm1, %%ymm1\n"
        "    vaddps %%ymm2, %%ymm0, %%ymm0\n"
        "    vfnmadd132ps (%[consts]), %%ymm11, %%ymm14\n"
        "    vfmadd132ps (%[consts]), %%ymm10, %%ymm15\n"
        "    vsubps %%ymm7, %%ymm4, %%ymm4\n"
        "    vaddps %%ymm10, %%ymm11, %%ymm11\n"
        "    vaddps %%ymm6, %%ymm5, %%ymm5\n"
        "    vfnmadd132ps (%[consts]), %%ymm1, %%ymm3\n"
        "    vmovups %%ymm1, (%[dst])\n"
        "    vmovups %%ymm0, 32(%[dst])\n"
        "    vfnmadd132ps (%[consts]), %%ymm0, %%ymm2\n"
        "    lea (%[dst],%[out_stride],1), %[dst]\n"
        "    vaddps %%ymm15, %%ymm14, %%ymm14\n"
        "    vfmadd132ps (%[consts]), %%ymm4, %%ymm7\n"
        "    vfmsub132ps (%[consts]), %%ymm11, %%ymm10\n"
        "    vfnmadd132ps (%[consts]), %%ymm5, %%ymm6\n"
        "    vmovups %%ymm3, (%[dst])\n"
        "    vmulps 32(%[consts]), %%ymm11, %%ymm11\n"
        "    vmovups %%ymm2, 32(%[dst])\n"
        "    lea (%[dst],%[out_stride],1), %[dst]\n"
        "    vfmsub132ps (%[consts]), %%ymm14, %%ymm15\n"
        "    vmovups %%ymm5, (%[dst])\n"
        "    vmovups %%ymm4, 32(%[dst])\n"
        "    vmulps 32(%[consts]), %%ymm10, %%ymm10\n"
        "    vmulps 32(%[consts]), %%ymm14, %%ymm14\n"
        "    lea (%[dst],%[out_stride],1), %[dst]\n"
        "    vaddps %%ymm11, %%ymm9, %%ymm9\n"
        "    vmovups %%ymm6, (%[dst])\n"
        "    vmovups %%ymm7, 32(%[dst])\n"
        "    lea (%[dst],%[out_stride],1), %[dst]\n"
        "    vmulps 32(%[consts]), %%ymm15, %%ymm15\n"
        "    vaddps %%ymm10, %%ymm8, %%ymm8\n"
        "    vsubps %%ymm14, %%ymm13, %%ymm13\n"
        "    vfnmadd132ps (%[consts]), %%ymm9, %%ymm11\n"
        "    vmovups %%ymm9, (%[dst])\n"
        "    vaddps %%ymm15, %%ymm12, %%ymm12\n"
        "    vfnmadd132ps (%[consts]), %%ymm8, %%ymm10\n"
        "    vfmadd132ps (%[consts]), %%ymm13, %%ymm14\n"
        "    vmovups %%ymm8, 32(%[dst])\n"
        "    lea (%[dst],%[out_stride],1), %[dst]\n"
        "    vmovups %%ymm11, (%[dst])\n"
        "    vfnmadd132ps (%[consts]), %%ymm12, %%ymm15\n"
        "    vmovups %%ymm10, 32(%[dst])\n"
        "    lea (%[dst],%[out_stride],1), %[dst]\n"
        "    vmovups %%ymm12, (%[dst])\n"
        "    vmovups %%ymm13, 32(%[dst])\n"
        "    lea (%[dst],%[out_stride],1), %[dst]\n"
        "    vmovups %%ymm15, (%[dst])\n"
        "    vmovups %%ymm14, 32(%[dst])\n"
        "    sub $1, %[count]\n"
        "    jnz 1b\n"
        "    vzeroupper\n"
        : [in] "+r"(in), [out] "+r"(out), [table] "+r"(table), [count] "+r"(count), [in_stride] "+r"(in_stride), [out_stride] "+r"(out_stride), [src] "=&r"(src), [dst] "=&r"(dst), [a] "=&r"(a), [b] "=&r"(b), [consts] "+r"(consts)
        :
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "cc", "memory");
}

/* Last pass, in place: reads split vectors and writes the interleaved result. Butterfly
 * j < count uses the vectors at data + r*stride + 64*j. Index bits 1 and 0 are moved from the
 * lanes to the registers and transformed. The table holds per butterfly, for l = 1..3, the real
 * and the imaginary parts of the 8 twiddles of the vector with index bits (1, 0) = l. */
__attribute__((target("avx2,fma"))) static void onefft_avx2_lb(float *data, const float *tw, size_t stride, size_t count, size_t regions) {
    (void)regions;
    float *third;
    __asm__ volatile(
        "    lea (%[data],%[stride],2), %[third]\n"
        "    lea (%[third],%[stride],1), %[third]\n"
        "1:\n"
        "    vmovups (%[data]), %%xmm0\n"
        "    vmovups 32(%[data]), %%xmm1\n"
        "    vmovups 16(%[data]), %%xmm2\n"
        "    vmovups 48(%[data]), %%xmm3\n"
        "    vmovups (%[data],%[stride],2), %%xmm4\n"
        "    vmovups 32(%[data],%[stride],2), %%xmm5\n"
        "    vmovups 16(%[data],%[stride],2), %%xmm6\n"
        "    vmovups 48(%[data],%[stride],2), %%xmm7\n"
        "    vinsertf128 $1, (%[data],%[stride],1), %%ymm0, %%ymm0\n"
        "    vinsertf128 $1, 32(%[data],%[stride],1), %%ymm1, %%ymm1\n"
        "    vinsertf128 $1, 16(%[data],%[stride],1), %%ymm2, %%ymm2\n"
        "    vinsertf128 $1, 48(%[data],%[stride],1), %%ymm3, %%ymm3\n"
        "    vinsertf128 $1, (%[third]), %%ymm4, %%ymm4\n"
        "    vinsertf128 $1, 32(%[third]), %%ymm5, %%ymm5\n"
        "    vinsertf128 $1, 16(%[third]), %%ymm6, %%ymm6\n"
        "    vinsertf128 $1, 48(%[third]), %%ymm7, %%ymm7\n"
        "    vunpckhps %%ymm4, %%ymm0, %%ymm8\n"
        "    vunpckhps %%ymm5, %%ymm1, %%ymm9\n"
        "    vunpcklps %%ymm6, %%ymm2, %%ymm10\n"
        "    vunpckhps %%ymm6, %%ymm2, %%ymm2\n"
        "    vunpcklps %%ymm7, %%ymm3, %%ymm6\n"
        "    vunpckhps %%ymm7, %%ymm3, %%ymm3\n"
        "    vmulps (%[tw]), %%ymm8, %%ymm7\n"
        "    vmulps (%[tw]), %%ymm9, %%ymm11\n"
        "    vmulps 64(%[tw]), %%ymm10, %%ymm12\n"
        "    vmulps 64(%[tw]), %%ymm6, %%ymm13\n"
        "    vmulps 128(%[tw]), %%ymm2, %%ymm14\n"
        "    vmulps 128(%[tw]), %%ymm3, %%ymm15\n"
        "    vunpcklps %%ymm4, %%ymm0, %%ymm0\n"
        "    vunpcklps %%ymm5, %%ymm1, %%ymm1\n"
        "    vfnmadd231ps 32(%[tw]), %%ymm9, %%ymm7\n"
        "    vfmadd231ps 32(%[tw]), %%ymm8, %%ymm11\n"
        "    vfnmadd231ps 96(%[tw]), %%ymm6, %%ymm12\n"
        "    vfmadd231ps 96(%[tw]), %%ymm10, %%ymm13\n"
        "    vfnmadd231ps 160(%[tw]), %%ymm3, %%ymm14\n"
        "    vfmadd231ps 160(%[tw]), %%ymm2, %%ymm15\n"
        "    lea 192(%[tw]), %[tw]\n"
        "    vaddps %%ymm12, %%ymm0, %%ymm2\n"
        "    vsubps %%ymm12, %%ymm0, %%ymm0\n"
        "    vaddps %%ymm13, %%ymm1, %%ymm3\n"
        "    vsubps %%ymm13, %%ymm1, %%ymm1\n"
        "    vaddps %%ymm14, %%ymm7, %%ymm4\n"
        "    vaddps %%ymm15, %%ymm11, %%ymm5\n"
        "    vsubps %%ymm14, %%ymm7, %%ymm6\n"
        "    vsubps %%ymm15, %%ymm11, %%ymm7\n"
        "    vaddps %%ymm4, %%ymm2, %%ymm8\n"
        "    vsubps %%ymm4, %%ymm2, %%ymm2\n"
        "    vaddps %%ymm5, %%ymm3, %%ymm4\n"
        "    vsubps %%ymm5, %%ymm3, %%ymm3\n"
        "    vaddps %%ymm7, %%ymm0, %%ymm5\n"
        "    vsubps %%ymm6, %%ymm1, %%ymm9\n"
        "    vsubps %%ymm7, %%ymm0, %%ymm0\n"
        "    vaddps %%ymm6, %%ymm1, %%ymm1\n"
        "    vunpcklps %%ymm4, %%ymm8, %%ymm6\n"
        "    vunpckhps %%ymm4, %%ymm8, %%ymm4\n"
        "    vunpcklps %%ymm3, %%ymm2, %%ymm7\n"
        "    vunpcklps %%ymm9, %%ymm5, %%ymm8\n"
        "    vunpckhps %%ymm9, %%ymm5, %%ymm5\n"
        "    vunpckhps %%ymm3, %%ymm2, %%ymm2\n"
        "    vmovups %%ymm6, (%[data])\n"
        "    vmovups %%ymm4, 32(%[data])\n"
        "    vmovups %%ymm7, (%[data],%[stride],2)\n"
        "    vunpcklps %%ymm1, %%ymm0, %%ymm3\n"
        "    vunpckhps %%ymm1, %%ymm0, %%ymm0\n"
        "    vmovups %%ymm8, (%[data],%[stride],1)\n"
        "    vmovups %%ymm5, 32(%[data],%[stride],1)\n"
        "    vmovups %%ymm2, 32(%[data],%[stride],2)\n"
        "    vmovups %%ymm3, (%[third])\n"
        "    vmovups %%ymm0, 32(%[third])\n"
        "    lea 64(%[data]), %[data]\n"
        "    lea 64(%[third]), %[third]\n"
        "    sub $1, %[count]\n"
        "    jnz 1b\n"
        "    vzeroupper\n"
        : [data] "+r"(data), [tw] "+r"(tw), [stride] "+r"(stride), [count] "+r"(count), [third] "=&r"(third)
        :
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "cc", "memory");
}

static const float onefft_avx2_n16_consts[48] ONEFFT_ALIGNED(64) = {
    0x1.0000000000000p+0f, -0x0.0p+0f, 0x1.0000000000000p+0f, -0x0.0p+0f,
    0x1.0000000000000p+0f, -0x0.0p+0f, 0x1.0000000000000p+0f, -0x0.0p+0f,
    0x0.0p+0f, -0x1.0000000000000p+0f, 0x0.0p+0f, -0x1.0000000000000p+0f,
    0x0.0p+0f, -0x1.0000000000000p+0f, 0x0.0p+0f, -0x1.0000000000000p+0f,
    0x1.0000000000000p+0f, 0x1.6a09e667f3bcdp-1f, 0x1.0000000000000p+0f, 0x1.6a09e667f3bcdp-1f,
    -0x0.0p+0f, -0x1.6a09e667f3bccp-1f, -0x0.0p+0f, -0x1.6a09e667f3bccp-1f,
    0x0.0p+0f, -0x1.6a09e667f3bccp-1f, 0x0.0p+0f, -0x1.6a09e667f3bccp-1f,
    -0x1.0000000000000p+0f, -0x1.6a09e667f3bcdp-1f, -0x1.0000000000000p+0f, -0x1.6a09e667f3bcdp-1f,
    0x1.0000000000000p+0f, 0x1.d906bcf328d46p-1f, -0x0.0p+0f, -0x1.87de2a6aea962p-2f,
    0x1.6a09e667f3bcdp-1f, 0x1.87de2a6aea964p-2f, -0x1.6a09e667f3bccp-1f, -0x1.d906bcf328d46p-1f,
    0x0.0p+0f, -0x1.87de2a6aea963p-2f, -0x1.0000000000000p+0f, -0x1.d906bcf328d46p-1f,
    -0x1.6a09e667f3bccp-1f, -0x1.d906bcf328d46p-1f, -0x1.6a09e667f3bcdp-1f, -0x1.87de2a6aea965p-2f,
};
/* Complete 16 point transform from `in` to `out`, both interleaved. */
__attribute__((target("avx2,fma"))) static void onefft_avx2_n16(const float *in, float *out) {
    const float *consts = onefft_avx2_n16_consts;
    __asm__ volatile(
        "    vmovups (%[in]), %%ymm0\n"
        "    vmovups 64(%[in]), %%ymm1\n"
        "    vshufps $136, 32(%[in]), %%ymm0, %%ymm2\n"
        "    vshufps $221, 32(%[in]), %%ymm0, %%ymm0\n"
        "    vshufps $136, 96(%[in]), %%ymm1, %%ymm3\n"
        "    vshufps $221, 96(%[in]), %%ymm1, %%ymm1\n"
        "    vaddps %%ymm3, %%ymm2, %%ymm4\n"
        "    vsubps %%ymm3, %%ymm2, %%ymm2\n"
        "    vaddps %%ymm1, %%ymm0, %%ymm3\n"
        "    vsubps %%ymm1, %%ymm0, %%ymm0\n"
        "    vunpckhps %%ymm2, %%ymm4, %%ymm1\n"
        "    vunpcklps %%ymm2, %%ymm4, %%ymm2\n"
        "    vunpckhps %%ymm0, %%ymm3, %%ymm4\n"
        "    vmulps (%[consts]), %%ymm1, %%ymm5\n"
        "    vunpcklps %%ymm0, %%ymm3, %%ymm0\n"
        "    vmulps (%[consts]), %%ymm4, %%ymm3\n"
        "    vfnmadd231ps 32(%[consts]), %%ymm4, %%ymm5\n"
        "    vfmadd231ps 32(%[consts]), %%ymm1, %%ymm3\n"
        "    vaddps %%ymm5, %%ymm2, %%ymm1\n"
        "    vsubps %%ymm5, %%ymm2, %%ymm2\n"
        "    vaddps %%ymm3, %%ymm0, %%ymm4\n"
        "    vsubps %%ymm3, %%ymm0, %%ymm0\n"
        "    vperm2f128 $49, %%ymm2, %%ymm1, %%ymm3\n"
        "    vperm2f128 $32, %%ymm2, %%ymm1, %%ymm1\n"
        "    vperm2f128 $49, %%ymm0, %%ymm4, %%ymm2\n"
        "    vmulps 64(%[consts]), %%ymm3, %%ymm5\n"
        "    vperm2f128 $32, %%ymm0, %%ymm4, %%ymm0\n"
        "    vmulps 64(%[consts]), %%ymm2, %%ymm4\n"
        "    vfnmadd231ps 96(%[consts]), %%ymm2, %%ymm5\n"
        "    vfmadd231ps 96(%[consts]), %%ymm3, %%ymm4\n"
        "    vaddps %%ymm5, %%ymm1, %%ymm2\n"
        "    vsubps %%ymm5, %%ymm1, %%ymm1\n"
        "    vaddps %%ymm4, %%ymm0, %%ymm3\n"
        "    vsubps %%ymm4, %%ymm0, %%ymm0\n"
        "    vunpckhpd %%ymm1, %%ymm2, %%ymm4\n"
        "    vunpcklpd %%ymm1, %%ymm2, %%ymm1\n"
        "    vunpckhpd %%ymm0, %%ymm3, %%ymm2\n"
        "    vmulps 128(%[consts]), %%ymm4, %%ymm5\n"
        "    vunpcklpd %%ymm0, %%ymm3, %%ymm0\n"
        "    vmulps 128(%[consts]), %%ymm2, %%ymm3\n"
        "    vfnmadd231ps 160(%[consts]), %%ymm2, %%ymm5\n"
        "    vfmadd231ps 160(%[consts]), %%ymm4, %%ymm3\n"
        "    vaddps %%ymm5, %%ymm1, %%ymm2\n"
        "    vsubps %%ymm5, %%ymm1, %%ymm1\n"
        "    vaddps %%ymm3, %%ymm0, %%ymm4\n"
        "    vsubps %%ymm3, %%ymm0, %%ymm0\n"
        "    vunpcklps %%ymm4, %%ymm2, %%ymm3\n"
        "    vunpckhps %%ymm4, %%ymm2, %%ymm2\n"
        "    vunpcklps %%ymm0, %%ymm1, %%ymm4\n"
        "    vunpckhps %%ymm0, %%ymm1, %%ymm0\n"
        "    vmovups %%ymm3, (%[out])\n"
        "    vmovups %%ymm2, 32(%[out])\n"
        "    vmovups %%ymm4, 64(%[out])\n"
        "    vmovups %%ymm0, 96(%[out])\n"
        "    vzeroupper\n"
        : [in] "+r"(in), [out] "+r"(out), [consts] "+r"(consts)
        :
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "cc", "memory");
}

static const float onefft_avx2_n32_consts[96] ONEFFT_ALIGNED(64) = {
    0x1.0000000000000p+0f, 0x1.6a09e667f3bcdp-1f, 0x1.0000000000000p+0f, 0x1.6a09e667f3bcdp-1f,
    0x1.0000000000000p+0f, 0x1.6a09e667f3bcdp-1f, 0x1.0000000000000p+0f, 0x1.6a09e667f3bcdp-1f,
    0x0.0p+0f, -0x1.6a09e667f3bccp-1f, 0x0.0p+0f, -0x1.6a09e667f3bccp-1f,
    0x0.0p+0f, -0x1.6a09e667f3bccp-1f, 0x0.0p+0f, -0x1.6a09e667f3bccp-1f,
    -0x0.0p+0f, -0x1.6a09e667f3bccp-1f, -0x0.0p+0f, -0x1.6a09e667f3bccp-1f,
    -0x0.0p+0f, -0x1.6a09e667f3bccp-1f, -0x0.0p+0f, -0x1.6a09e667f3bccp-1f,
    -0x1.0000000000000p+0f, -0x1.6a09e667f3bcdp-1f, -0x1.0000000000000p+0f, -0x1.6a09e667f3bcdp-1f,
    -0x1.0000000000000p+0f, -0x1.6a09e667f3bcdp-1f, -0x1.0000000000000p+0f, -0x1.6a09e667f3bcdp-1f,
    0x1.0000000000000p+0f, 0x1.d906bcf328d46p-1f, 0x1.0000000000000p+0f, 0x1.d906bcf328d46p-1f,
    0x1.6a09e667f3bcdp-1f, 0x1.87de2a6aea964p-2f, 0x1.6a09e667f3bcdp-1f, 0x1.87de2a6aea964p-2f,
    0x0.0p+0f, -0x1.87de2a6aea963p-2f, 0x0.0p+0f, -0x1.87de2a6aea963p-2f,
    -0x1.6a09e667f3bccp-1f, -0x1.d906bcf328d46p-1f, -0x1.6a09e667f3bccp-1f, -0x1.d906bcf328d46p-1f,
    -0x0.0p+0f, -0x1.87de2a6aea962p-2f, -0x0.0p+0f, -0x1.87de2a6aea962p-2f,
    -0x1.6a09e667f3bccp-1f, -0x1.d906bcf328d46p-1f, -0x1.6a09e667f3bccp-1f, -0x1.d906bcf328d46p-1f,
    -0x1.0000000000000p+0f, -0x1.d906bcf328d46p-1f, -0x1.0000000000000p+0f, -0x1.d906bcf328d46p-1f,
    -0x1.6a09e667f3bcdp-1f, -0x1.87de2a6aea965p-2f, -0x1.6a09e667f3bcdp-1f, -0x1.87de2a6aea965p-2f,
    0x1.0000000000000p+0f, 0x1.f6297cff75cb0p-1f, 0x1.6a09e667f3bcdp-1f, 0x1.1c73b39ae68c9p-1f,
    0x1.d906bcf328d46p-1f, 0x1.a9b66290ea1a3p-1f, 0x1.87de2a6aea964p-2f, 0x1.8f8b83c69a60dp-3f,
    0x0.0p+0f, -0x1.8f8b83c69a60ap-3f, -0x1.6a09e667f3bccp-1f, -0x1.a9b66290ea1a3p-1f,
    -0x1.87de2a6aea963p-2f, -0x1.1c73b39ae68c8p-1f, -0x1.d906bcf328d46p-1f, -0x1.f6297cff75cb0p-1f,
    -0x0.0p+0f, -0x1.8f8b83c69a608p-3f, -0x1.6a09e667f3bccp-1f, -0x1.a9b66290ea1a4p-1f,
    -0x1.87de2a6aea962p-2f, -0x1.1c73b39ae68c6p-1f, -0x1.d906bcf328d46p-1f, -0x1.f6297cff75cb0p-1f,
    -0x1.0000000000000p+0f, -0x1.f6297cff75cb0p-1f, -0x1.6a09e667f3bcdp-1f, -0x1.1c73b39ae68c8p-1f,
    -0x1.d906bcf328d46p-1f, -0x1.a9b66290ea1a5p-1f, -0x1.87de2a6aea965p-2f, -0x1.8f8b83c69a617p-3f,
};
/* Complete 32 point transform from `in` to `out`, both interleaved. */
__attribute__((target("avx2,fma"))) static void onefft_avx2_n32(const float *in, float *out) {
    const float *consts = onefft_avx2_n32_consts;
    __asm__ volatile(
        "    vmovups (%[in]), %%ymm0\n"
        "    vmovups 64(%[in]), %%ymm1\n"
        "    vmovups 128(%[in]), %%ymm2\n"
        "    vmovups 192(%[in]), %%ymm3\n"
        "    vshufps $136, 32(%[in]), %%ymm0, %%ymm4\n"
        "    vshufps $221, 32(%[in]), %%ymm0, %%ymm0\n"
        "    vshufps $136, 96(%[in]), %%ymm1, %%ymm5\n"
        "    vshufps $221, 96(%[in]), %%ymm1, %%ymm1\n"
        "    vshufps $136, 160(%[in]), %%ymm2, %%ymm6\n"
        "    vshufps $221, 160(%[in]), %%ymm2, %%ymm2\n"
        "    vshufps $136, 224(%[in]), %%ymm3, %%ymm7\n"
        "    vshufps $221, 224(%[in]), %%ymm3, %%ymm3\n"
        "    vaddps %%ymm6, %%ymm4, %%ymm8\n"
        "    vaddps %%ymm2, %%ymm0, %%ymm9\n"
        "    vsubps %%ymm6, %%ymm4, %%ymm4\n"
        "    vsubps %%ymm2, %%ymm0, %%ymm0\n"
        "    vaddps %%ymm7, %%ymm5, %%ymm2\n"
        "    vaddps %%ymm3, %%ymm1, %%ymm6\n"
        "    vsubps %%ymm7, %%ymm5, %%ymm5\n"
        "    vsubps %%ymm3, %%ymm1, %%ymm1\n"
        "    vaddps %%ymm2, %%ymm8, %%ymm3\n"
        "    vaddps %%ymm6, %%ymm9, %%ymm7\n"
        "    vsubps %%ymm2, %%ymm8, %%ymm2\n"
        "    vsubps %%ymm6, %%ymm9, %%ymm6\n"
        "    vaddps %%ymm1, %%ymm4, %%ymm8\n"
        "    vsubps %%ymm5, %%ymm0, %%ymm9\n"
        "    vsubps %%ymm1, %%ymm4, %%ymm1\n"
        "    vaddps %%ymm5, %%ymm0, %%ymm0\n"
        "    vunpckhps %%ymm8, %%ymm3, %%ymm4\n"
        "    vunpckhps %%ymm9, %%ymm7, %%ymm5\n"
        "    vunpcklps %%ymm8, %%ymm3, %%ymm3\n"
        "    vunpckhps %%ymm1, %%ymm2, %%ymm8\n"
        "    vunpckhps %%ymm0, %%ymm6, %%ymm10\n"
        "    vmulps (%[consts]), %%ymm4, %%ymm11\n"
        "    vmulps (%[consts]), %%ymm5, %%ymm12\n"
        "    vmulps 64(%[consts]), %%ymm8, %%ymm13\n"
        "    vmulps 64(%[consts]), %%ymm10, %%ymm14\n"
        "    vunpcklps %%ymm9, %%ymm7, %%ymm7\n"
        "    vunpcklps %%ymm1, %%ymm2, %%ymm1\n"
        "    vunpcklps %%ymm0, %%ymm6, %%ymm0\n"
        "    vfnmadd231ps 32(%[consts]), %%ymm5, %%ymm11\n"
        "    vfmadd231ps 32(%[consts]), %%ymm4, %%ymm12\n"
        "    vfnmadd231ps 96(%[consts]), %%ymm10, %%ymm13\n"
        "    vfmadd231ps 96(%[consts]), %%ymm8, %%ymm14\n"
        "    vaddps %%ymm11, %%ymm3, %%ymm2\n"
        "    vsubps %%ymm11, %%ymm3, %%ymm3\n"
        "    vaddps %%ymm12, %%ymm7, %%ymm4\n"
        "    vsubps %%ymm12, %%ymm7, %%ymm5\n"
        "    vaddps %%ymm13, %%ymm1, %%ymm6\n"
        "    vaddps %%ymm14, %%ymm0, %%ymm7\n"
        "    vsubps %%ymm13, %%ymm1, %%ymm1\n"
        "    vsubps %%ymm14, %%ymm0, %%ymm0\n"
        "    vperm2f128 $49, %%ymm6, %%ymm2, %%ymm8\n"
        "    vperm2f128 $32, %%ymm6, %%ymm2, %%ymm2\n"
        "    vperm2f128 $49, %%ymm7, %%ymm4, %%ymm6\n"
        "    vperm2f128 $49, %%ymm1, %%ymm3, %%ymm9\n"
        "    vperm2f128 $49, %%ymm0, %%ymm5, %%ymm10\n"
        "    vmulps 128(%[consts]), %%ymm8, %%ymm11\n"
        "    vmulps 128(%[consts]), %%ymm6, %%ymm12\n"
        "    vmulps 192(%[consts]), %%ymm9, %%ymm13\n"
        "    vmulps 192(%[consts]), %%ymm10, %%ymm14\n"
        "    vperm2f128 $32, %%ymm7, %%ymm4, %%ymm4\n"
        "    vperm2f128 $32, %%ymm1, %%ymm3, %%ymm1\n"
        "    vperm2f128 $32, %%ymm0, %%ymm5, %%ymm0\n"
        "    vfnmadd231ps 160(%[consts]), %%ymm6, %%ymm11\n"
        "    vfmadd231ps 160(%[consts]), %%ymm8, %%ymm12\n"
        "    vfnmadd231ps 224(%[consts]), %%ymm10, %%ymm13\n"
        "    vfmadd231ps 224(%[consts]), %%ymm9, %%ymm14\n"
        "    vaddps %%ymm11, %%ymm2, %%ymm3\n"
        "    vaddps %%ymm12, %%ymm4, %%ymm5\n"
        "    vsubps %%ymm11, %%ymm2, %%ymm2\n"
        "    vsubps %%ymm12, %%ymm4, %%ymm4\n"
        "    vaddps %%ymm13, %%ymm1, %%ymm6\n"
        "    vaddps %%ymm14, %%ymm0, %%ymm7\n"
        "    vsubps %%ymm13, %%ymm1, %%ymm1\n"
        "    vsubps %%ymm14, %%ymm0, %%ymm0\n"
        "    vunpckhpd %%ymm6, %%ymm3, %%ymm8\n"
        "    vunpckhpd %%ymm7, %%ymm5, %%ymm9\n"
        "    vunpcklpd %%ymm6, %%ymm3, %%ymm3\n"
        "    vunpckhpd %%ymm1, %%ymm2, %%ymm6\n"
        "    vunpckhpd %%ymm0, %%ymm4, %%ymm10\n"
        "    vmulps 256(%[consts]), %%ymm8, %%ymm11\n"
        "    vmulps 256(%[consts]), %%ymm9, %%ymm12\n"
        "    vmulps 320(%[consts]), %%ymm6, %%ymm13\n"
        "    vmulps 320(%[consts]), %%ymm10, %%ymm14\n"
        "    vunpcklpd %%ymm7, %%ymm5, %%ymm5\n"
        "    vunpcklpd %%ymm1, %%ymm2, %%ymm1\n"
        "    vunpcklpd %%ymm0, %%ymm4, %%ymm0\n"
        "    vfnmadd231ps 288(%[consts]), %%ymm9, %%ymm11\n"
        "    vfmadd231ps 288(%[consts]), %%ymm8, %%ymm12\n"
        "    vfnmadd231ps 352(%[consts]), %%ymm10, %%ymm13\n"
        "    vfmadd231ps 352(%[consts]), %%ymm6, %%ymm14\n"
        "    vaddps %%ymm11, %%ymm3, %%ymm2\n"
        "    vsubps %%ymm11, %%ymm3, %%ymm3\n"
        "    vaddps %%ymm12, %%ymm5, %%ymm4\n"
        "    vsubps %%ymm12, %%ymm5, %%ymm5\n"
        "    vaddps %%ymm13, %%ymm1, %%ymm6\n"
        "    vaddps %%ymm14, %%ymm0, %%ymm7\n"
        "    vsubps %%ymm13, %%ymm1, %%ymm1\n"
        "    vsubps %%ymm14, %%ymm0, %%ymm0\n"
        "    vunpcklps %%ymm4, %%ymm2, %%ymm8\n"
        "    vunpckhps %%ymm4, %%ymm2, %%ymm2\n"
        "    vunpcklps %%ymm5, %%ymm3, %%ymm4\n"
        "    vunpcklps %%ymm7, %%ymm6, %%ymm9\n"
        "    vunpckhps %%ymm7, %%ymm6, %%ymm6\n"
        "    vunpckhps %%ymm5, %%ymm3, %%ymm3\n"
        "    vmovups %%ymm8, (%[out])\n"
        "    vmovups %%ymm2, 32(%[out])\n"
        "    vmovups %%ymm4, 128(%[out])\n"
        "    vunpcklps %%ymm0, %%ymm1, %%ymm2\n"
        "    vunpckhps %%ymm0, %%ymm1, %%ymm0\n"
        "    vmovups %%ymm9, 64(%[out])\n"
        "    vmovups %%ymm6, 96(%[out])\n"
        "    vmovups %%ymm3, 160(%[out])\n"
        "    vmovups %%ymm2, 192(%[out])\n"
        "    vmovups %%ymm0, 224(%[out])\n"
        "    vzeroupper\n"
        : [in] "+r"(in), [out] "+r"(out), [consts] "+r"(consts)
        :
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "cc", "memory");
}

/* First pass, radix 4. For each of the `count` entries (a, b) of `table`: reads 4 interleaved
 * vectors from in + 8*a + u*in_stride, transforms across them and writes output q as a split
 * vector to out + 8*b + r*out_stride, where r is q with its bits reversed. */
__attribute__((target("avx2,fma"))) static void onefft_avx2_f4i(const float *in, float *out, const uint32_t *table, size_t count, size_t in_stride, size_t out_stride) {
    const float *src;
    float *dst;
    size_t a;
    size_t b;
    __asm__ volatile(
        "1:\n"
        "    movl (%[table]), %k[a]\n"
        "    movl 4(%[table]), %k[b]\n"
        "    lea 8(%[table]), %[table]\n"
        "    lea (%[in],%[a],8), %[src]\n"
        "    lea (%[out],%[b],8), %[dst]\n"
        "    vmovups (%[src]), %%ymm0\n"
        "    vshufps $136, 32(%[src]), %%ymm0, %%ymm1\n"
        "    vshufps $221, 32(%[src]), %%ymm0, %%ymm0\n"
        "    lea (%[src],%[in_stride],1), %[src]\n"
        "    vmovups (%[src]), %%ymm2\n"
        "    vshufps $136, 32(%[src]), %%ymm2, %%ymm3\n"
        "    vshufps $221, 32(%[src]), %%ymm2, %%ymm2\n"
        "    lea (%[src],%[in_stride],1), %[src]\n"
        "    vmovups (%[src]), %%ymm4\n"
        "    vshufps $136, 32(%[src]), %%ymm4, %%ymm5\n"
        "    vshufps $221, 32(%[src]), %%ymm4, %%ymm4\n"
        "    lea (%[src],%[in_stride],1), %[src]\n"
        "    vaddps %%ymm4, %%ymm0, %%ymm6\n"
        "    vaddps %%ymm5, %%ymm1, %%ymm7\n"
        "    vsubps %%ymm4, %%ymm0, %%ymm0\n"
        "    vmovups (%[src]), %%ymm4\n"
        "    vsubps %%ymm5, %%ymm1, %%ymm1\n"
        "    vshufps $136, 32(%[src]), %%ymm4, %%ymm5\n"
        "    vshufps $221, 32(%[src]), %%ymm4, %%ymm4\n"
        "    vaddps %%ymm4, %%ymm2, %%ymm8\n"
        "    vaddps %%ymm5, %%ymm3, %%ymm9\n"
        "    vsubps %%ymm4, %%ymm2, %%ymm2\n"
        "    vsubps %%ymm5, %%ymm3, %%ymm3\n"
        "    vaddps %%ymm8, %%ymm6, %%ymm4\n"
        "    vaddps %%ymm9, %%ymm7, %%ymm5\n"
        "    vsubps %%ymm8, %%ymm6, %%ymm6\n"
        "    vsubps %%ymm9, %%ymm7, %%ymm7\n"
        "    vaddps %%ymm3, %%ymm0, %%ymm8\n"
        "    vsubps %%ymm2, %%ymm1, %%ymm9\n"
        "    vsubps %%ymm3, %%ymm0, %%ymm0\n"
        "    vaddps %%ymm2, %%ymm1, %%ymm1\n"
        "    vmovups %%ymm4, (%[dst])\n"
        "    vmovups %%ymm5, 32(%[dst])\n"
        "    lea (%[dst],%[out_stride],1), %[dst]\n"
        "    vmovups %%ymm6, (%[dst])\n"
        "    vmovups %%ymm7, 32(%[dst])\n"
        "    lea (%[dst],%[out_stride],1), %[dst]\n"
        "    vmovups %%ymm8, (%[dst])\n"
        "    vmovups %%ymm9, 32(%[dst])\n"
        "    lea (%[dst],%[out_stride],1), %[dst]\n"
        "    vmovups %%ymm0, (%[dst])\n"
        "    vmovups %%ymm1, 32(%[dst])\n"
        "    sub $1, %[count]\n"
        "    jnz 1b\n"
        "    vzeroupper\n"
        : [in] "+r"(in), [out] "+r"(out), [table] "+r"(table), [count] "+r"(count), [in_stride] "+r"(in_stride), [out_stride] "+r"(out_stride), [src] "=&r"(src), [dst] "=&r"(dst), [a] "=&r"(a), [b] "=&r"(b)
        :
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "cc", "memory");
}

static const float onefft_avx2_f8i_consts[16] ONEFFT_ALIGNED(64) = {
    0x1.0000000000000p+1f, 0x1.0000000000000p+1f, 0x1.0000000000000p+1f, 0x1.0000000000000p+1f,
    0x1.0000000000000p+1f, 0x1.0000000000000p+1f, 0x1.0000000000000p+1f, 0x1.0000000000000p+1f,
    0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f,
    0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f,
};
/* First pass, radix 8. For each of the `count` entries (a, b) of `table`: reads 8 interleaved
 * vectors from in + 8*a + u*in_stride, transforms across them and writes output q as a split
 * vector to out + 8*b + r*out_stride, where r is q with its bits reversed. */
__attribute__((target("avx2,fma"))) static void onefft_avx2_f8i(const float *in, float *out, const uint32_t *table, size_t count, size_t in_stride, size_t out_stride) {
    const float *src;
    float *dst;
    size_t a;
    size_t b;
    const float *consts = onefft_avx2_f8i_consts;
    __asm__ volatile(
        "1:\n"
        "    movl (%[table]), %k[a]\n"
        "    movl 4(%[table]), %k[b]\n"
        "    lea 8(%[table]), %[table]\n"
        "    lea (%[in],%[a],8), %[src]\n"
        "    lea (%[out],%[b],8), %[dst]\n"
        "    vmovups (%[src]), %%ymm0\n"
        "    vshufps $136, 32(%[src]), %%ymm0, %%ymm1\n"
        "    vshufps $221, 32(%[src]), %%ymm0, %%ymm0\n"
        "    lea (%[src],%[in_stride],1), %[src]\n"
        "    vmovups (%[src]), %%ymm2\n"
        "    vshufps $136, 32(%[src]), %%ymm2, %%ymm3\n"
        "    vshufps $221, 32(%[src]), %%ymm2, %%ymm2\n"
        "    lea (%[src],%[in_stride],1), %[src]\n"
        "    vmovups (%[src]), %%ymm4\n"
        "    vshufps $136, 32(%[src]), %%ymm4, %%ymm5\n"
        "    vshufps $221, 32(%[src]), %%ymm4, %%ymm4\n"
        "    lea (%[src],%[in_stride],1), %[src]\n"
        "    vmovups (%[src]), %%ymm6\n"
        "    vshufps $136, 32(%[src]), %%ymm6, %%ymm7\n"
        "    vshufps $221, 32(%[src]), %%ymm6, %%ymm6\n"
        "    lea (%[src],%[in_stride],1), %[src]\n"
        "    vmovups (%[src]), %%ymm8\n"
        "    vshufps $136, 32(%[src]), %%ymm8, %%ymm9\n"
        "    vshufps $221, 32(%[src]), %%ymm8, %%ymm8\n"
        "    lea (%[src],%[in_stride],1), %[src]\n"
        "    vaddps %%ymm8, %%ymm0, %%ymm0\n"
        "    vaddps %%ymm9, %%ymm1, %%ymm1\n"
        "    vmovups (%[src]), %%ymm10\n"
        "    vfnmadd132ps (%[consts]), %%ymm0, %%ymm8\n"
        "    vfnmadd132ps (%[consts]), %%ymm1, %%ymm9\n"
        "    vshufps $136, 32(%[src]), %%ymm10, %%ymm11\n"
        "    vshufps $221, 32(%[src]), %%ymm10, %%ymm10\n"
        "    lea (%[src],%[in_stride],1), %[src]\n"
        "    vaddps %%ymm10, %%ymm2, %%ymm2\n"
        "    vaddps %%ymm11, %%ymm3, %%ymm3\n"
        "    vmovups (%[src]), %%ymm12\n"
        "    vfnmadd132ps (%[consts]), %%ymm2, %%ymm10\n"
        "    vfnmadd132ps (%[consts]), %%ymm3, %%ymm11\n"
        "    vshufps $136, 32(%[src]), %%ymm12, %%ymm13\n"
        "    vshufps $221, 32(%[src]), %%ymm12, %%ymm12\n"
        "    lea (%[src],%[in_stride],1), %[src]\n"
        "    vaddps %%ymm12, %%ymm4, %%ymm4\n"
        "    vaddps %%ymm13, %%ymm5, %%ymm5\n"
        "    vmovups (%[src]), %%ymm14\n"
        "    vfnmadd132ps (%[consts]), %%ymm4, %%ymm12\n"
        "    vfnmadd132ps (%[consts]), %%ymm5, %%ymm13\n"
        "    vaddps %%ymm4, %%ymm0, %%ymm0\n"
        "    vaddps %%ymm5, %%ymm1, %%ymm1\n"
        "    vshufps $136, 32(%[src]), %%ymm14, %%ymm15\n"
        "    vshufps $221, 32(%[src]), %%ymm14, %%ymm14\n"
        "    vaddps %%ymm14, %%ymm6, %%ymm6\n"
        "    vaddps %%ymm15, %%ymm7, %%ymm7\n"
        "    vfnmadd132ps (%[consts]), %%ymm0, %%ymm4\n"
        "    vaddps %%ymm13, %%ymm8, %%ymm8\n"
        "    vsubps %%ymm12, %%ymm9, %%ymm9\n"
        "    vfnmadd132ps (%[consts]), %%ymm1, %%ymm5\n"
        "    vfnmadd132ps (%[consts]), %%ymm6, %%ymm14\n"
        "    vfnmadd132ps (%[consts]), %%ymm7, %%ymm15\n"
        "    vaddps %%ymm6, %%ymm2, %%ymm2\n"
        "    vaddps %%ymm7, %%ymm3, %%ymm3\n"
        "    vfnmadd132ps (%[consts]), %%ymm8, %%ymm13\n"
        "    vfmadd132ps (%[consts]), %%ymm9, %%ymm12\n"
        "    vaddps %%ymm15, %%ymm10, %%ymm10\n"
        "    vsubps %%ymm14, %%ymm11, %%ymm11\n"
        "    vfnmadd132ps (%[consts]), %%ymm2, %%ymm6\n"
        "    vfnmadd132ps (%[consts]), %%ymm3, %%ymm7\n"
        "    vaddps %%ymm2, %%ymm0, %%ymm0\n"
        "    vaddps %%ymm3, %%ymm1, %%ymm1\n"
        "    vfnmadd132ps (%[consts]), %%ymm10, %%ymm15\n"
        "    vfmadd132ps (%[consts]), %%ymm11, %%ymm14\n"
        "    vsubps %%ymm6, %%ymm5, %%ymm5\n"
        "    vaddps %%ymm11, %%ymm10, %%ymm10\n"
        "    vaddps %%ymm7, %%ymm4, %%ymm4\n"
        "    vfnmadd132ps (%[consts]), %%ymm0, %%ymm2\n"
        "    vmovups %%ymm0, (%[dst])\n"
        "    vmovups %%ymm1, 32(%[dst])\n"
        "    vfnmadd132ps (%[consts]), %%ymm1, %%ymm3\n"
        "    lea (%[dst],%[out_stride],1), %[dst]\n"
        "    vaddps %%ymm14, %%ymm15, %%ymm15\n"
        "    vfmadd132ps (%[consts]), %%ymm5, %%ymm6\n"
        "    vfmsub132ps (%[consts]), %%ymm10, %%ymm11\n"
        "    vfnmadd132ps (%[consts]), %%ymm4, %%ymm7\n"
        "    vmovups %%ymm2, (%[dst])\n"
        "    vmulps 32(%[consts]), %%ymm10, %%ymm10\n"
        "    vmovups %%ymm3, 32(%[dst])\n"
        "    lea (%[dst],%[out_stride],1), %[dst]\n"
        "    vfmsub132ps (%[consts]), %%ymm15, %%ymm14\n"
        "    vmovups %%ymm4, (%[dst])\n"
        "    vmovups %%ymm5, 32(%[dst])\n"
        "    vmulps 32(%[consts]), %%ymm11, %%ymm11\n"
        "    vmulps 32(%[consts]), %%ymm15, %%ymm15\n"
        "    lea (%[dst],%[out_stride],1), %[dst]\n"
        "    vaddps %%ymm10, %%ymm8, %%ymm8\n"
        "    vmovups %%ymm7, (%[dst])\n"
        "    vmovups %%ymm6, 32(%[dst])\n"
        "    lea (%[dst],%[out_stride],1), %[dst]\n"
        "    vmulps 32(%[consts]), %%ymm14, %%ymm14\n"
        "    vaddps %%ymm11, %%ymm9, %%ymm9\n"
        "    vsubps %%ymm15, %%ymm12, %%ymm12\n"
        "    vfnmadd132ps (%[consts]), %%ymm8, %%ymm10\n"
        "    vmovups %%ymm8, (%[dst])\n"
        "    vaddps %%ymm14, %%ymm13, %%ymm13\n"
        "    vfnmadd132ps (%[consts]), %%ymm9, %%ymm11\n"
        "    vfmadd132ps (%[consts]), %%ymm12, %%ymm15\n"
        "    vmovups %%ymm9, 32(%[dst])\n"
        "    lea (%[dst],%[out_stride],1), %[dst]\n"
        "    vmovups %%ymm10, (%[dst])\n"
        "    vfnmadd132ps (%[consts]), %%ymm13, %%ymm14\n"
        "    vmovups %%ymm11, 32(%[dst])\n"
        "    lea (%[dst],%[out_stride],1), %[dst]\n"
        "    vmovups %%ymm13, (%[dst])\n"
        "    vmovups %%ymm12, 32(%[dst])\n"
        "    lea (%[dst],%[out_stride],1), %[dst]\n"
        "    vmovups %%ymm14, (%[dst])\n"
        "    vmovups %%ymm15, 32(%[dst])\n"
        "    sub $1, %[count]\n"
        "    jnz 1b\n"
        "    vzeroupper\n"
        : [in] "+r"(in), [out] "+r"(out), [table] "+r"(table), [count] "+r"(count), [in_stride] "+r"(in_stride), [out_stride] "+r"(out_stride), [src] "=&r"(src), [dst] "=&r"(dst), [a] "=&r"(a), [b] "=&r"(b), [consts] "+r"(consts)
        :
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "cc", "memory");
}

/* Last pass, in place: reads split vectors and writes the interleaved result. Butterfly
 * j < count uses the vectors at data + r*stride + 64*j. Index bits 1 and 0 are moved from the
 * lanes to the registers and transformed. The table holds per butterfly, for l = 1..3, the real
 * and the imaginary parts of the 8 twiddles of the vector with index bits (1, 0) = l. */
__attribute__((target("avx2,fma"))) static void onefft_avx2_lbi(float *data, const float *tw, size_t stride, size_t count, size_t regions) {
    (void)regions;
    float *third;
    __asm__ volatile(
        "    lea (%[data],%[stride],2), %[third]\n"
        "    lea (%[third],%[stride],1), %[third]\n"
        "1:\n"
        "    vmovups (%[data]), %%xmm0\n"
        "    vmovups 32(%[data]), %%xmm1\n"
        "    vmovups 16(%[data]), %%xmm2\n"
        "    vmovups 48(%[data]), %%xmm3\n"
        "    vmovups (%[data],%[stride],2), %%xmm4\n"
        "    vmovups 32(%[data],%[stride],2), %%xmm5\n"
        "    vmovups 16(%[data],%[stride],2), %%xmm6\n"
        "    vmovups 48(%[data],%[stride],2), %%xmm7\n"
        "    vinsertf128 $1, (%[data],%[stride],1), %%ymm0, %%ymm0\n"
        "    vinsertf128 $1, 32(%[data],%[stride],1), %%ymm1, %%ymm1\n"
        "    vinsertf128 $1, 16(%[data],%[stride],1), %%ymm2, %%ymm2\n"
        "    vinsertf128 $1, 48(%[data],%[stride],1), %%ymm3, %%ymm3\n"
        "    vinsertf128 $1, (%[third]), %%ymm4, %%ymm4\n"
        "    vinsertf128 $1, 32(%[third]), %%ymm5, %%ymm5\n"
        "    vinsertf128 $1, 16(%[third]), %%ymm6, %%ymm6\n"
        "    vinsertf128 $1, 48(%[third]), %%ymm7, %%ymm7\n"
        "    vunpckhps %%ymm4, %%ymm0, %%ymm8\n"
        "    vunpckhps %%ymm5, %%ymm1, %%ymm9\n"
        "    vunpcklps %%ymm6, %%ymm2, %%ymm10\n"
        "    vunpckhps %%ymm6, %%ymm2, %%ymm2\n"
        "    vunpcklps %%ymm7, %%ymm3, %%ymm6\n"
        "    vunpckhps %%ymm7, %%ymm3, %%ymm3\n"
        "    vmulps (%[tw]), %%ymm8, %%ymm7\n"
        "    vmulps (%[tw]), %%ymm9, %%ymm11\n"
        "    vmulps 64(%[tw]), %%ymm10, %%ymm12\n"
        "    vmulps 64(%[tw]), %%ymm6, %%ymm13\n"
        "    vmulps 128(%[tw]), %%ymm2, %%ymm14\n"
        "    vmulps 128(%[tw]), %%ymm3, %%ymm15\n"
        "    vunpcklps %%ymm4, %%ymm0, %%ymm0\n"
        "    vunpcklps %%ymm5, %%ymm1, %%ymm1\n"
        "    vfnmadd231ps 32(%[tw]), %%ymm9, %%ymm7\n"
        "    vfmadd231ps 32(%[tw]), %%ymm8, %%ymm11\n"
        "    vfnmadd231ps 96(%[tw]), %%ymm6, %%ymm12\n"
        "    vfmadd231ps 96(%[tw]), %%ymm10, %%ymm13\n"
        "    vfnmadd231ps 160(%[tw]), %%ymm3, %%ymm14\n"
        "    vfmadd231ps 160(%[tw]), %%ymm2, %%ymm15\n"
        "    lea 192(%[tw]), %[tw]\n"
        "    vaddps %%ymm12, %%ymm0, %%ymm2\n"
        "    vsubps %%ymm12, %%ymm0, %%ymm0\n"
        "    vaddps %%ymm13, %%ymm1, %%ymm3\n"
        "    vsubps %%ymm13, %%ymm1, %%ymm1\n"
        "    vaddps %%ymm14, %%ymm7, %%ymm4\n"
        "    vaddps %%ymm15, %%ymm11, %%ymm5\n"
        "    vsubps %%ymm14, %%ymm7, %%ymm6\n"
        "    vsubps %%ymm15, %%ymm11, %%ymm7\n"
        "    vaddps %%ymm4, %%ymm2, %%ymm8\n"
        "    vsubps %%ymm4, %%ymm2, %%ymm2\n"
        "    vaddps %%ymm5, %%ymm3, %%ymm4\n"
        "    vsubps %%ymm5, %%ymm3, %%ymm3\n"
        "    vaddps %%ymm7, %%ymm0, %%ymm5\n"
        "    vsubps %%ymm6, %%ymm1, %%ymm9\n"
        "    vsubps %%ymm7, %%ymm0, %%ymm0\n"
        "    vaddps %%ymm6, %%ymm1, %%ymm1\n"
        "    vunpcklps %%ymm8, %%ymm4, %%ymm6\n"
        "    vunpckhps %%ymm8, %%ymm4, %%ymm4\n"
        "    vunpcklps %%ymm2, %%ymm3, %%ymm7\n"
        "    vunpcklps %%ymm5, %%ymm9, %%ymm8\n"
        "    vunpckhps %%ymm5, %%ymm9, %%ymm5\n"
        "    vunpckhps %%ymm2, %%ymm3, %%ymm2\n"
        "    vmovups %%ymm6, (%[data])\n"
        "    vmovups %%ymm4, 32(%[data])\n"
        "    vmovups %%ymm7, (%[data],%[stride],2)\n"
        "    vunpcklps %%ymm0, %%ymm1, %%ymm3\n"
        "    vunpckhps %%ymm0, %%ymm1, %%ymm0\n"
        "    vmovups %%ymm8, (%[data],%[stride],1)\n"
        "    vmovups %%ymm5, 32(%[data],%[stride],1)\n"
        "    vmovups %%ymm2, 32(%[data],%[stride],2)\n"
        "    vmovups %%ymm3, (%[third])\n"
        "    vmovups %%ymm0, 32(%[third])\n"
        "    lea 64(%[data]), %[data]\n"
        "    lea 64(%[third]), %[third]\n"
        "    sub $1, %[count]\n"
        "    jnz 1b\n"
        "    vzeroupper\n"
        : [data] "+r"(data), [tw] "+r"(tw), [stride] "+r"(stride), [count] "+r"(count), [third] "=&r"(third)
        :
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "cc", "memory");
}

static const float onefft_avx2_n16i_consts[48] ONEFFT_ALIGNED(64) = {
    0x1.0000000000000p+0f, -0x0.0p+0f, 0x1.0000000000000p+0f, -0x0.0p+0f,
    0x1.0000000000000p+0f, -0x0.0p+0f, 0x1.0000000000000p+0f, -0x0.0p+0f,
    0x0.0p+0f, -0x1.0000000000000p+0f, 0x0.0p+0f, -0x1.0000000000000p+0f,
    0x0.0p+0f, -0x1.0000000000000p+0f, 0x0.0p+0f, -0x1.0000000000000p+0f,
    0x1.0000000000000p+0f, 0x1.6a09e667f3bcdp-1f, 0x1.0000000000000p+0f, 0x1.6a09e667f3bcdp-1f,
    -0x0.0p+0f, -0x1.6a09e667f3bccp-1f, -0x0.0p+0f, -0x1.6a09e667f3bccp-1f,
    0x0.0p+0f, -0x1.6a09e667f3bccp-1f, 0x0.0p+0f, -0x1.6a09e667f3bccp-1f,
    -0x1.0000000000000p+0f, -0x1.6a09e667f3bcdp-1f, -0x1.0000000000000p+0f, -0x1.6a09e667f3bcdp-1f,
    0x1.0000000000000p+0f, 0x1.d906bcf328d46p-1f, -0x0.0p+0f, -0x1.87de2a6aea962p-2f,
    0x1.6a09e667f3bcdp-1f, 0x1.87de2a6aea964p-2f, -0x1.6a09e667f3bccp-1f, -0x1.d906bcf328d46p-1f,
    0x0.0p+0f, -0x1.87de2a6aea963p-2f, -0x1.0000000000000p+0f, -0x1.d906bcf328d46p-1f,
    -0x1.6a09e667f3bccp-1f, -0x1.d906bcf328d46p-1f, -0x1.6a09e667f3bcdp-1f, -0x1.87de2a6aea965p-2f,
};
/* Complete 16 point transform from `in` to `out`, both interleaved. */
__attribute__((target("avx2,fma"))) static void onefft_avx2_n16i(const float *in, float *out) {
    const float *consts = onefft_avx2_n16i_consts;
    __asm__ volatile(
        "    vmovups (%[in]), %%ymm0\n"
        "    vmovups 64(%[in]), %%ymm1\n"
        "    vshufps $136, 32(%[in]), %%ymm0, %%ymm2\n"
        "    vshufps $221, 32(%[in]), %%ymm0, %%ymm0\n"
        "    vshufps $136, 96(%[in]), %%ymm1, %%ymm3\n"
        "    vshufps $221, 96(%[in]), %%ymm1, %%ymm1\n"
        "    vaddps %%ymm3, %%ymm2, %%ymm4\n"
        "    vsubps %%ymm3, %%ymm2, %%ymm2\n"
        "    vaddps %%ymm1, %%ymm0, %%ymm3\n"
        "    vsubps %%ymm1, %%ymm0, %%ymm0\n"
        "    vunpckhps %%ymm2, %%ymm4, %%ymm1\n"
        "    vunpcklps %%ymm2, %%ymm4, %%ymm2\n"
        "    vunpckhps %%ymm0, %%ymm3, %%ymm4\n"
        "    vmulps (%[consts]), %%ymm1, %%ymm5\n"
        "    vunpcklps %%ymm0, %%ymm3, %%ymm0\n"
        "    vmulps (%[consts]), %%ymm4, %%ymm3\n"
        "    vfmadd231ps 32(%[consts]), %%ymm4, %%ymm5\n"
        "    vfnmadd231ps 32(%[consts]), %%ymm1, %%ymm3\n"
        "    vaddps %%ymm5, %%ymm2, %%ymm1\n"
        "    vsubps %%ymm5, %%ymm2, %%ymm2\n"
        "    vaddps %%ymm3, %%ymm0, %%ymm4\n"
        "    vsubps %%ymm3, %%ymm0, %%ymm0\n"
        "    vperm2f128 $49, %%ymm2, %%ymm1, %%ymm3\n"
        "    vperm2f128 $32, %%ymm2, %%ymm1, %%ymm1\n"
        "    vperm2f128 $49, %%ymm0, %%ymm4, %%ymm2\n"
        "    vmulps 64(%[consts]), %%ymm3, %%ymm5\n"
        "    vperm2f128 $32, %%ymm0, %%ymm4, %%ymm0\n"
        "    vmulps 64(%[consts]), %%ymm2, %%ymm4\n"
        "    vfmadd231ps 96(%[consts]), %%ymm2, %%ymm5\n"
        "    vfnmadd231ps 96(%[consts]), %%ymm3, %%ymm4\n"
        "    vaddps %%ymm5, %%ymm1, %%ymm2\n"
        "    vsubps %%ymm5, %%ymm1, %%ymm1\n"
        "    vaddps %%ymm4, %%ymm0, %%ymm3\n"
        "    vsubps %%ymm4, %%ymm0, %%ymm0\n"
        "    vunpckhpd %%ymm1, %%ymm2, %%ymm4\n"
        "    vunpcklpd %%ymm1, %%ymm2, %%ymm1\n"
        "    vunpckhpd %%ymm0, %%ymm3, %%ymm2\n"
        "    vmulps 128(%[consts]), %%ymm4, %%ymm5\n"
        "    vunpcklpd %%ymm0, %%ymm3, %%ymm0\n"
        "    vmulps 128(%[consts]), %%ymm2, %%ymm3\n"
        "    vfmadd231ps 160(%[consts]), %%ymm2, %%ymm5\n"
        "    vfnmadd231ps 160(%[consts]), %%ymm4, %%ymm3\n"
        "    vaddps %%ymm5, %%ymm1, %%ymm2\n"
        "    vsubps %%ymm5, %%ymm1, %%ymm1\n"
        "    vaddps %%ymm3, %%ymm0, %%ymm4\n"
        "    vsubps %%ymm3, %%ymm0, %%ymm0\n"
        "    vunpcklps %%ymm4, %%ymm2, %%ymm3\n"
        "    vunpckhps %%ymm4, %%ymm2, %%ymm2\n"
        "    vunpcklps %%ymm0, %%ymm1, %%ymm4\n"
        "    vunpckhps %%ymm0, %%ymm1, %%ymm0\n"
        "    vmovups %%ymm3, (%[out])\n"
        "    vmovups %%ymm2, 32(%[out])\n"
        "    vmovups %%ymm4, 64(%[out])\n"
        "    vmovups %%ymm0, 96(%[out])\n"
        "    vzeroupper\n"
        : [in] "+r"(in), [out] "+r"(out), [consts] "+r"(consts)
        :
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "cc", "memory");
}

static const float onefft_avx2_n32i_consts[96] ONEFFT_ALIGNED(64) = {
    0x1.0000000000000p+0f, 0x1.6a09e667f3bcdp-1f, 0x1.0000000000000p+0f, 0x1.6a09e667f3bcdp-1f,
    0x1.0000000000000p+0f, 0x1.6a09e667f3bcdp-1f, 0x1.0000000000000p+0f, 0x1.6a09e667f3bcdp-1f,
    0x0.0p+0f, -0x1.6a09e667f3bccp-1f, 0x0.0p+0f, -0x1.6a09e667f3bccp-1f,
    0x0.0p+0f, -0x1.6a09e667f3bccp-1f, 0x0.0p+0f, -0x1.6a09e667f3bccp-1f,
    -0x0.0p+0f, -0x1.6a09e667f3bccp-1f, -0x0.0p+0f, -0x1.6a09e667f3bccp-1f,
    -0x0.0p+0f, -0x1.6a09e667f3bccp-1f, -0x0.0p+0f, -0x1.6a09e667f3bccp-1f,
    -0x1.0000000000000p+0f, -0x1.6a09e667f3bcdp-1f, -0x1.0000000000000p+0f, -0x1.6a09e667f3bcdp-1f,
    -0x1.0000000000000p+0f, -0x1.6a09e667f3bcdp-1f, -0x1.0000000000000p+0f, -0x1.6a09e667f3bcdp-1f,
    0x1.0000000000000p+0f, 0x1.d906bcf328d46p-1f, 0x1.0000000000000p+0f, 0x1.d906bcf328d46p-1f,
    0x1.6a09e667f3bcdp-1f, 0x1.87de2a6aea964p-2f, 0x1.6a09e667f3bcdp-1f, 0x1.87de2a6aea964p-2f,
    0x0.0p+0f, -0x1.87de2a6aea963p-2f, 0x0.0p+0f, -0x1.87de2a6aea963p-2f,
    -0x1.6a09e667f3bccp-1f, -0x1.d906bcf328d46p-1f, -0x1.6a09e667f3bccp-1f, -0x1.d906bcf328d46p-1f,
    -0x0.0p+0f, -0x1.87de2a6aea962p-2f, -0x0.0p+0f, -0x1.87de2a6aea962p-2f,
    -0x1.6a09e667f3bccp-1f, -0x1.d906bcf328d46p-1f, -0x1.6a09e667f3bccp-1f, -0x1.d906bcf328d46p-1f,
    -0x1.0000000000000p+0f, -0x1.d906bcf328d46p-1f, -0x1.0000000000000p+0f, -0x1.d906bcf328d46p-1f,
    -0x1.6a09e667f3bcdp-1f, -0x1.87de2a6aea965p-2f, -0x1.6a09e667f3bcdp-1f, -0x1.87de2a6aea965p-2f,
    0x1.0000000000000p+0f, 0x1.f6297cff75cb0p-1f, 0x1.6a09e667f3bcdp-1f, 0x1.1c73b39ae68c9p-1f,
    0x1.d906bcf328d46p-1f, 0x1.a9b66290ea1a3p-1f, 0x1.87de2a6aea964p-2f, 0x1.8f8b83c69a60dp-3f,
    0x0.0p+0f, -0x1.8f8b83c69a60ap-3f, -0x1.6a09e667f3bccp-1f, -0x1.a9b66290ea1a3p-1f,
    -0x1.87de2a6aea963p-2f, -0x1.1c73b39ae68c8p-1f, -0x1.d906bcf328d46p-1f, -0x1.f6297cff75cb0p-1f,
    -0x0.0p+0f, -0x1.8f8b83c69a608p-3f, -0x1.6a09e667f3bccp-1f, -0x1.a9b66290ea1a4p-1f,
    -0x1.87de2a6aea962p-2f, -0x1.1c73b39ae68c6p-1f, -0x1.d906bcf328d46p-1f, -0x1.f6297cff75cb0p-1f,
    -0x1.0000000000000p+0f, -0x1.f6297cff75cb0p-1f, -0x1.6a09e667f3bcdp-1f, -0x1.1c73b39ae68c8p-1f,
    -0x1.d906bcf328d46p-1f, -0x1.a9b66290ea1a5p-1f, -0x1.87de2a6aea965p-2f, -0x1.8f8b83c69a617p-3f,
};
/* Complete 32 point transform from `in` to `out`, both interleaved. */
__attribute__((target("avx2,fma"))) static void onefft_avx2_n32i(const float *in, float *out) {
    const float *consts = onefft_avx2_n32i_consts;
    __asm__ volatile(
        "    vmovups (%[in]), %%ymm0\n"
        "    vmovups 64(%[in]), %%ymm1\n"
        "    vmovups 128(%[in]), %%ymm2\n"
        "    vmovups 192(%[in]), %%ymm3\n"
        "    vshufps $136, 32(%[in]), %%ymm0, %%ymm4\n"
        "    vshufps $221, 32(%[in]), %%ymm0, %%ymm0\n"
        "    vshufps $136, 96(%[in]), %%ymm1, %%ymm5\n"
        "    vshufps $221, 96(%[in]), %%ymm1, %%ymm1\n"
        "    vshufps $136, 160(%[in]), %%ymm2, %%ymm6\n"
        "    vshufps $221, 160(%[in]), %%ymm2, %%ymm2\n"
        "    vshufps $136, 224(%[in]), %%ymm3, %%ymm7\n"
        "    vshufps $221, 224(%[in]), %%ymm3, %%ymm3\n"
        "    vaddps %%ymm2, %%ymm0, %%ymm8\n"
        "    vaddps %%ymm6, %%ymm4, %%ymm9\n"
        "    vsubps %%ymm2, %%ymm0, %%ymm0\n"
        "    vsubps %%ymm6, %%ymm4, %%ymm2\n"
        "    vaddps %%ymm3, %%ymm1, %%ymm4\n"
        "    vaddps %%ymm7, %%ymm5, %%ymm6\n"
        "    vsubps %%ymm3, %%ymm1, %%ymm1\n"
        "    vsubps %%ymm7, %%ymm5, %%ymm3\n"
        "    vaddps %%ymm4, %%ymm8, %%ymm5\n"
        "    vaddps %%ymm6, %%ymm9, %%ymm7\n"
        "    vsubps %%ymm4, %%ymm8, %%ymm4\n"
        "    vsubps %%ymm6, %%ymm9, %%ymm6\n"
        "    vaddps %%ymm3, %%ymm0, %%ymm8\n"
        "    vsubps %%ymm1, %%ymm2, %%ymm9\n"
        "    vsubps %%ymm3, %%ymm0, %%ymm0\n"
        "    vaddps %%ymm1, %%ymm2, %%ymm1\n"
        "    vunpckhps %%ymm8, %%ymm5, %%ymm2\n"
        "    vunpckhps %%ymm9, %%ymm7, %%ymm3\n"
        "    vunpcklps %%ymm8, %%ymm5, %%ymm5\n"
        "    vunpckhps %%ymm0, %%ymm4, %%ymm8\n"
        "    vunpckhps %%ymm1, %%ymm6, %%ymm10\n"
        "    vmulps (%[consts]), %%ymm2, %%ymm11\n"
        "    vmulps (%[consts]), %%ymm3, %%ymm12\n"
        "    vmulps 64(%[consts]), %%ymm8, %%ymm13\n"
        "    vmulps 64(%[consts]), %%ymm10, %%ymm14\n"
        "    vunpcklps %%ymm9, %%ymm7, %%ymm7\n"
        "    vunpcklps %%ymm0, %%ymm4, %%ymm0\n"
        "    vunpcklps %%ymm1, %%ymm6, %%ymm1\n"
        "    vfnmadd231ps 32(%[consts]), %%ymm3, %%ymm11\n"
        "    vfmadd231ps 32(%[consts]), %%ymm2, %%ymm12\n"
        "    vfnmadd231ps 96(%[consts]), %%ymm10, %%ymm13\n"
        "    vfmadd231ps 96(%[consts]), %%ymm8, %%ymm14\n"
        "    vaddps %%ymm11, %%ymm5, %%ymm2\n"
        "    vsubps %%ymm11, %%ymm5, %%ymm3\n"
        "    vaddps %%ymm12, %%ymm7, %%ymm4\n"
        "    vsubps %%ymm12, %%ymm7, %%ymm5\n"
        "    vaddps %%ymm13, %%ymm0, %%ymm6\n"
        "    vaddps %%ymm14, %%ymm1, %%ymm7\n"
        "    vsubps %%ymm13, %%ymm0, %%ymm0\n"
        "    vsubps %%ymm14, %%ymm1, %%ymm1\n"
        "    vperm2f128 $49, %%ymm6, %%ymm2, %%ymm8\n"
        "    vperm2f128 $32, %%ymm6, %%ymm2, %%ymm2\n"
        "    vperm2f128 $49, %%ymm7, %%ymm4, %%ymm6\n"
        "    vperm2f128 $49, %%ymm0, %%ymm3, %%ymm9\n"
        "    vperm2f128 $49, %%ymm1, %%ymm5, %%ymm10\n"
        "    vmulps 128(%[consts]), %%ymm8, %%ymm11\n"
        "    vmulps 128(%[consts]), %%ymm6, %%ymm12\n"
        "    vmulps 192(%[consts]), %%ymm9, %%ymm13\n"
        "    vmulps 192(%[consts]), %%ymm10, %%ymm14\n"
        "    vperm2f128 $32, %%ymm7, %%ymm4, %%ymm4\n"
        "    vperm2f128 $32, %%ymm0, %%ymm3, %%ymm0\n"
        "    vperm2f128 $32, %%ymm1, %%ymm5, %%ymm1\n"
        "    vfnmadd231ps 160(%[consts]), %%ymm6, %%ymm11\n"
        "    vfmadd231ps 160(%[consts]), %%ymm8, %%ymm12\n"
        "    vfnmadd231ps 224(%[consts]), %%ymm10, %%ymm13\n"
        "    vfmadd231ps 224(%[consts]), %%ymm9, %%ymm14\n"
        "    vaddps %%ymm11, %%ymm2, %%ymm3\n"
        "    vaddps %%ymm12, %%ymm4, %%ymm5\n"
        "    vsubps %%ymm11, %%ymm2, %%ymm2\n"
        "    vsubps %%ymm12, %%ymm4, %%ymm4\n"
        "    vaddps %%ymm13, %%ymm0, %%ymm6\n"
        "    vaddps %%ymm14, %%ymm1, %%ymm7\n"
        "    vsubps %%ymm13, %%ymm0, %%ymm0\n"
        "    vsubps %%ymm14, %%ymm1, %%ymm1\n"
        "    vunpckhpd %%ymm6, %%ymm3, %%ymm8\n"
        "    vunpckhpd %%ymm7, %%ymm5, %%ymm9\n"
        "    vunpcklpd %%ymm6, %%ymm3, %%ymm3\n"
        "    vunpckhpd %%ymm0, %%ymm2, %%ymm6\n"
        "    vunpckhpd %%ymm1, %%ymm4, %%ymm10\n"
        "    vmulps 256(%[consts]), %%ymm8, %%ymm11\n"
        "    vmulps 256(%[consts]), %%ymm9, %%ymm12\n"
        "    vmulps 320(%[consts]), %%ymm6, %%ymm13\n"
        "    vmulps 320(%[consts]), %%ymm10, %%ymm14\n"
        "    vunpcklpd %%ymm7, %%ymm5, %%ymm5\n"
        "    vunpcklpd %%ymm0, %%ymm2, %%ymm0\n"
        "    vunpcklpd %%ymm1, %%ymm4, %%ymm1\n"
        "    vfnmadd231ps 288(%[consts]), %%ymm9, %%ymm11\n"
        "    vfmadd231ps 288(%[consts]), %%ymm8, %%ymm12\n"
        "    vfnmadd231ps 352(%[consts]), %%ymm10, %%ymm13\n"
        "    vfmadd231ps 352(%[consts]), %%ymm6, %%ymm14\n"
        "    vaddps %%ymm11, %%ymm3, %%ymm2\n"
        "    vsubps %%ymm11, %%ymm3, %%ymm3\n"
        "    vaddps %%ymm12, %%ymm5, %%ymm4\n"
        "    vsubps %%ymm12, %%ymm5, %%ymm5\n"
        "    vaddps %%ymm13, %%ymm0, %%ymm6\n"
        "    vaddps %%ymm14, %%ymm1, %%ymm7\n"
        "    vsubps %%ymm13, %%ymm0, %%ymm0\n"
        "    vsubps %%ymm14, %%ymm1, %%ymm1\n"
        "    vunpcklps %%ymm2, %%ymm4, %%ymm8\n"
        "    vunpckhps %%ymm2, %%ymm4, %%ymm2\n"
        "    vunpcklps %%ymm3, %%ymm5, %%ymm4\n"
        "    vunpcklps %%ymm6, %%ymm7, %%ymm9\n"
        "    vunpckhps %%ymm6, %%ymm7, %%ymm6\n"
        "    vunpckhps %%ymm3, %%ymm5, %%ymm3\n"
        "    vmovups %%ymm8, (%[out])\n"
        "    vmovups %%ymm2, 32(%[out])\n"
        "    vmovups %%ymm4, 128(%[out])\n"
        "    vunpcklps %%ymm0, %%ymm1, %%ymm2\n"
        "    vunpckhps %%ymm0, %%ymm1, %%ymm0\n"
        "    vmovups %%ymm9, 64(%[out])\n"
        "    vmovups %%ymm6, 96(%[out])\n"
        "    vmovups %%ymm3, 160(%[out])\n"
        "    vmovups %%ymm2, 192(%[out])\n"
        "    vmovups %%ymm0, 224(%[out])\n"
        "    vzeroupper\n"
        : [in] "+r"(in), [out] "+r"(out), [consts] "+r"(consts)
        :
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "cc", "memory");
}

/* Middle pass, radix 4, in place on split vectors. `data` holds `regions` regions of `blocks`
 * blocks. Input i of butterfly j of a block is the vector at (i/2)*args.s_high + (i%2)*args.s_low
 * + j*V (V = bytes per vector), j < args.inner; see passes.py for how the blocks and regions follow
 * each other. Butterfly j multiplies input i by w_j^i and transforms across the vectors; output q
 * goes to (q%2)*s_high + (q/2)*s_low. The table `tw` holds per butterfly the real parts of w^1..w^3
 * and one unused float, then the imaginary parts. */
__attribute__((target("avx2,fma"))) static void onefft_avx2_c4(float *data, const float *tw, const size_t *args, size_t blocks, size_t regions) {
    float *upper;
    size_t s_low;
    const float *twp;
    size_t j;
    size_t b;
    size_t scratch;
    __asm__ volatile(
        "    mov 0(%[args]), %[s_low]\n"
        "    lea (%[data],%[s_low],1), %[upper]\n"
        "    mov 8(%[args]), %[s_low]\n"
        "1:\n"
        "    mov %[blocks], %[b]\n"
        "2:\n"
        "    mov %[tw], %[twp]\n"
        "    mov 16(%[args]), %[j]\n"
        "3:\n"
        "    vmovups (%[data],%[s_low],1), %%ymm0\n"
        "    vmovups 32(%[data],%[s_low],1), %%ymm1\n"
        "    vmovups (%[upper]), %%ymm2\n"
        "    vmovups 32(%[upper]), %%ymm3\n"
        "    vmovups (%[upper],%[s_low],1), %%ymm4\n"
        "    vmovups 32(%[upper],%[s_low],1), %%ymm5\n"
        "    vbroadcastss (%[twp]), %%ymm6\n"
        "    vmovups (%[data]), %%ymm7\n"
        "    vmovups 32(%[data]), %%ymm8\n"
        "    vbroadcastss 4(%[twp]), %%ymm9\n"
        "    vbroadcastss 16(%[twp]), %%ymm10\n"
        "    vbroadcastss 20(%[twp]), %%ymm11\n"
        "    vmulps %%ymm6, %%ymm0, %%ymm12\n"
        "    vmulps %%ymm6, %%ymm1, %%ymm6\n"
        "    vbroadcastss 8(%[twp]), %%ymm13\n"
        "    vmulps %%ymm9, %%ymm2, %%ymm14\n"
        "    vmulps %%ymm9, %%ymm3, %%ymm9\n"
        "    vfnmadd231ps %%ymm10, %%ymm1, %%ymm12\n"
        "    vfmadd231ps %%ymm10, %%ymm0, %%ymm6\n"
        "    vbroadcastss 24(%[twp]), %%ymm0\n"
        "    vfnmadd231ps %%ymm11, %%ymm3, %%ymm14\n"
        "    vfmadd231ps %%ymm11, %%ymm2, %%ymm9\n"
        "    vmulps %%ymm13, %%ymm4, %%ymm1\n"
        "    vmulps %%ymm13, %%ymm5, %%ymm2\n"
        "    vaddps %%ymm14, %%ymm7, %%ymm3\n"
        "    vaddps %%ymm9, %%ymm8, %%ymm10\n"
        "    vsubps %%ymm14, %%ymm7, %%ymm7\n"
        "    vfnmadd231ps %%ymm0, %%ymm5, %%ymm1\n"
        "    vfmadd231ps %%ymm0, %%ymm4, %%ymm2\n"
        "    vsubps %%ymm9, %%ymm8, %%ymm0\n"
        "    vaddps %%ymm1, %%ymm12, %%ymm4\n"
        "    vaddps %%ymm2, %%ymm6, %%ymm5\n"
        "    vsubps %%ymm1, %%ymm12, %%ymm1\n"
        "    vsubps %%ymm2, %%ymm6, %%ymm2\n"
        "    vaddps %%ymm4, %%ymm3, %%ymm6\n"
        "    vaddps %%ymm5, %%ymm10, %%ymm8\n"
        "    vsubps %%ymm4, %%ymm3, %%ymm3\n"
        "    vsubps %%ymm5, %%ymm10, %%ymm4\n"
        "    vaddps %%ymm2, %%ymm7, %%ymm5\n"
        "    vsubps %%ymm1, %%ymm0, %%ymm9\n"
        "    vsubps %%ymm2, %%ymm7, %%ymm2\n"
        "    vaddps %%ymm1, %%ymm0, %%ymm0\n"
        "    vmovups %%ymm6, (%[data])\n"
        "    vmovups %%ymm8, 32(%[data])\n"
        "    vmovups %%ymm3, (%[data],%[s_low],1)\n"
        "    vmovups %%ymm5, (%[upper])\n"
        "    vmovups %%ymm9, 32(%[upper])\n"
        "    vmovups %%ymm4, 32(%[data],%[s_low],1)\n"
        "    lea 32(%[twp]), %[twp]\n"
        "    vmovups %%ymm2, (%[upper],%[s_low],1)\n"
        "    vmovups %%ymm0, 32(%[upper],%[s_low],1)\n"
        "    lea 64(%[data]), %[data]\n"
        "    lea 64(%[upper]), %[upper]\n"
        "    sub $1, %[j]\n"
        "    jnz 3b\n"
        "    mov 24(%[args]), %[scratch]\n"
        "    lea (%[data],%[scratch],1), %[data]\n"
        "    lea (%[upper],%[scratch],1), %[upper]\n"
        "    sub $1, %[b]\n"
        "    jnz 2b\n"
        "    mov 32(%[args]), %[scratch]\n"
        "    lea (%[data],%[scratch],1), %[data]\n"
        "    lea (%[upper],%[scratch],1), %[upper]\n"
        "    mov 40(%[args]), %[scratch]\n"
        "    lea (%[tw],%[scratch],1), %[tw]\n"
        "    sub $1, %[regions]\n"
        "    jnz 1b\n"
        "    vzeroupper\n"
        : [data] "+r"(data), [tw] "+r"(tw), [args] "+r"(args), [blocks] "+r"(blocks), [regions] "+r"(regions), [upper] "=&r"(upper), [s_low] "=&r"(s_low), [twp] "=&r"(twp), [j] "=&r"(j), [b] "=&r"(b), [scratch] "=&r"(scratch)
        :
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "cc", "memory");
}

/* Second to last pass, in place on split vectors. Butterfly j < count uses the vectors at
 * data + u*stride + 64*j, u < 4, for each of the `regions` regions that follow each other. Index bit 3 selects u%2 and is transformed first. Then index bit 2 is moved from the lanes to the registers and
 * transformed. The table holds per butterfly 4 floats (real parts of the scalar twiddles for
 * u/2 = 0 and 1, then their imaginary parts) and the real and imaginary parts of the twiddle
 * vector of index bit 2. */
__attribute__((target("avx2,fma"))) static void onefft_avx2_la(float *data, const float *tw, size_t stride, size_t count, size_t regions) {
    float *third;
    size_t region_skip;
    size_t j;
    __asm__ volatile(
        "    lea (%[data],%[stride],2), %[third]\n"
        "    lea (%[stride],%[stride],2), %[region_skip]\n"
        "    lea (%[third],%[stride],1), %[third]\n"
        "1:\n"
        "    mov %[count], %[j]\n"
        "2:\n"
        "    vmovups (%[data],%[stride],1), %%ymm0\n"
        "    vmovups 32(%[data],%[stride],1), %%ymm1\n"
        "    vmovups (%[third]), %%ymm2\n"
        "    vmovups 32(%[third]), %%ymm3\n"
        "    vbroadcastss (%[tw]), %%ymm4\n"
        "    vbroadcastss 4(%[tw]), %%ymm5\n"
        "    vbroadcastss 8(%[tw]), %%ymm6\n"
        "    vbroadcastss 12(%[tw]), %%ymm7\n"
        "    vmovups (%[data]), %%ymm8\n"
        "    vmovups 32(%[data]), %%ymm9\n"
        "    vmovups (%[data],%[stride],2), %%ymm10\n"
        "    vmovups 32(%[data],%[stride],2), %%ymm11\n"
        "    vmulps %%ymm4, %%ymm0, %%ymm12\n"
        "    vmulps %%ymm4, %%ymm1, %%ymm4\n"
        "    vmulps %%ymm5, %%ymm2, %%ymm13\n"
        "    vmulps %%ymm5, %%ymm3, %%ymm5\n"
        "    vfnmadd231ps %%ymm6, %%ymm1, %%ymm12\n"
        "    vfmadd231ps %%ymm6, %%ymm0, %%ymm4\n"
        "    vfnmadd231ps %%ymm7, %%ymm3, %%ymm13\n"
        "    vfmadd231ps %%ymm7, %%ymm2, %%ymm5\n"
        "    vaddps %%ymm12, %%ymm8, %%ymm0\n"
        "    vaddps %%ymm4, %%ymm9, %%ymm1\n"
        "    vsubps %%ymm12, %%ymm8, %%ymm2\n"
        "    vsubps %%ymm4, %%ymm9, %%ymm3\n"
        "    vaddps %%ymm13, %%ymm10, %%ymm4\n"
        "    vaddps %%ymm5, %%ymm11, %%ymm6\n"
        "    vsubps %%ymm13, %%ymm10, %%ymm7\n"
        "    vsubps %%ymm5, %%ymm11, %%ymm5\n"
        "    vunpckhps %%ymm4, %%ymm0, %%ymm8\n"
        "    vunpckhps %%ymm6, %%ymm1, %%ymm9\n"
        "    vunpcklps %%ymm4, %%ymm0, %%ymm0\n"
        "    vunpckhps %%ymm7, %%ymm2, %%ymm4\n"
        "    vunpckhps %%ymm5, %%ymm3, %%ymm10\n"
        "    vmulps 16(%[tw]), %%ymm8, %%ymm11\n"
        "    vmulps 16(%[tw]), %%ymm9, %%ymm12\n"
        "    vmulps 16(%[tw]), %%ymm4, %%ymm13\n"
        "    vmulps 16(%[tw]), %%ymm10, %%ymm14\n"
        "    vunpcklps %%ymm6, %%ymm1, %%ymm1\n"
        "    vunpcklps %%ymm7, %%ymm2, %%ymm2\n"
        "    vunpcklps %%ymm5, %%ymm3, %%ymm3\n"
        "    vfnmadd231ps 48(%[tw]), %%ymm9, %%ymm11\n"
        "    vfmadd231ps 48(%[tw]), %%ymm8, %%ymm12\n"
        "    vfnmadd231ps 48(%[tw]), %%ymm10, %%ymm13\n"
        "    vfmadd231ps 48(%[tw]), %%ymm4, %%ymm14\n"
        "    lea 80(%[tw]), %[tw]\n"
        "    vaddps %%ymm11, %%ymm0, %%ymm4\n"
        "    vsubps %%ymm11, %%ymm0, %%ymm0\n"
        "    vaddps %%ymm12, %%ymm1, %%ymm5\n"
        "    vsubps %%ymm12, %%ymm1, %%ymm1\n"
        "    vaddps %%ymm14, %%ymm2, %%ymm6\n"
        "    vsubps %%ymm13, %%ymm3, %%ymm7\n"
        "    vsubps %%ymm14, %%ymm2, %%ymm2\n"
        "    vaddps %%ymm13, %%ymm3, %%ymm3\n"
        "    vmovups %%ymm4, (%[data])\n"
        "    vmovups %%ymm0, (%[data],%[stride],2)\n"
        "    vmovups %%ymm5, 32(%[data])\n"
        "    vmovups %%ymm6, (%[data],%[stride],1)\n"
        "    vmovups %%ymm1, 32(%[data],%[stride],2)\n"
        "    vmovups %%ymm7, 32(%[data],%[stride],1)\n"
        "    vmovups %%ymm2, (%[third])\n"
        "    vmovups %%ymm3, 32(%[third])\n"
        "    lea 64(%[data]), %[data]\n"
        "    lea 64(%[third]), %[third]\n"
        "    sub $1, %[j]\n"
        "    jnz 2b\n"
        "    lea (%[data],%[region_skip],1), %[data]\n"
        "    lea (%[third],%[region_skip],1), %[third]\n"
        "    sub $1, %[regions]\n"
        "    jnz 1b\n"
        "    vzeroupper\n"
        : [data] "+r"(data), [tw] "+r"(tw), [stride] "+r"(stride), [count] "+r"(count), [regions] "+r"(regions), [third] "=&r"(third), [region_skip] "=&r"(region_skip), [j] "=&r"(j)
        :
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "cc", "memory");
}

/* Second to last pass, in place on split vectors. Butterfly j < count uses the vectors at
 * data + u*stride + 64*j, u < 2, for each of the `regions` regions that follow each other. The two vectors are the two values of index bit 3, which is transformed first. Then index bit 2 is moved from the lanes to the registers and
 * transformed. The table holds per butterfly 4 floats (real parts of the scalar twiddles for
 * u/2 = 0 and 1, then their imaginary parts) and the real and imaginary parts of the twiddle
 * vector of index bit 2. */
__attribute__((target("avx2,fma"))) static void onefft_avx2_la2(float *data, const float *tw, size_t stride, size_t count, size_t regions) {
    size_t region_skip;
    size_t j;
    __asm__ volatile(
        "    mov %[stride], %[region_skip]\n"
        "1:\n"
        "    mov %[count], %[j]\n"
        "2:\n"
        "    vmovups (%[data],%[stride],1), %%ymm0\n"
        "    vmovups 32(%[data],%[stride],1), %%ymm1\n"
        "    vbroadcastss (%[tw]), %%ymm2\n"
        "    vbroadcastss 8(%[tw]), %%ymm3\n"
        "    vmovups (%[data]), %%ymm4\n"
        "    vmovups 32(%[data]), %%ymm5\n"
        "    vmulps %%ymm2, %%ymm0, %%ymm6\n"
        "    vmulps %%ymm2, %%ymm1, %%ymm2\n"
        "    vfnmadd231ps %%ymm3, %%ymm1, %%ymm6\n"
        "    vfmadd231ps %%ymm3, %%ymm0, %%ymm2\n"
        "    vaddps %%ymm6, %%ymm4, %%ymm0\n"
        "    vaddps %%ymm2, %%ymm5, %%ymm1\n"
        "    vsubps %%ymm6, %%ymm4, %%ymm3\n"
        "    vsubps %%ymm2, %%ymm5, %%ymm2\n"
        "    vunpckhps %%ymm3, %%ymm0, %%ymm4\n"
        "    vunpcklps %%ymm3, %%ymm0, %%ymm0\n"
        "    vunpckhps %%ymm2, %%ymm1, %%ymm3\n"
        "    vmulps 16(%[tw]), %%ymm4, %%ymm5\n"
        "    vunpcklps %%ymm2, %%ymm1, %%ymm1\n"
        "    vmulps 16(%[tw]), %%ymm3, %%ymm2\n"
        "    vfnmadd231ps 48(%[tw]), %%ymm3, %%ymm5\n"
        "    vfmadd231ps 48(%[tw]), %%ymm4, %%ymm2\n"
        "    lea 80(%[tw]), %[tw]\n"
        "    vaddps %%ymm5, %%ymm0, %%ymm3\n"
        "    vsubps %%ymm5, %%ymm0, %%ymm0\n"
        "    vaddps %%ymm2, %%ymm1, %%ymm4\n"
        "    vsubps %%ymm2, %%ymm1, %%ymm1\n"
        "    vmovups %%ymm3, (%[data])\n"
        "    vmovups %%ymm0, (%[data],%[stride],1)\n"
        "    vmovups %%ymm4, 32(%[data])\n"
        "    vmovups %%ymm1, 32(%[data],%[stride],1)\n"
        "    lea 64(%[data]), %[data]\n"
        "    sub $1, %[j]\n"
        "    jnz 2b\n"
        "    lea (%[data],%[region_skip],1), %[data]\n"
        "    sub $1, %[regions]\n"
        "    jnz 1b\n"
        "    vzeroupper\n"
        : [data] "+r"(data), [tw] "+r"(tw), [stride] "+r"(stride), [count] "+r"(count), [regions] "+r"(regions), [region_skip] "=&r"(region_skip), [j] "=&r"(j)
        :
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "cc", "memory");
}

static const float onefft_avx2_la8_consts[8] ONEFFT_ALIGNED(64) = {
    0x1.0000000000000p+1f, 0x1.0000000000000p+1f, 0x1.0000000000000p+1f, 0x1.0000000000000p+1f,
    0x1.0000000000000p+1f, 0x1.0000000000000p+1f, 0x1.0000000000000p+1f, 0x1.0000000000000p+1f,
};
/* Second to last pass for three index bits, in place on split vectors. Butterfly j < count
 * of a region uses the vectors at data + u*stride + 64*j, u < 8; the `regions` regions follow each
 * other. Index bit 2 is moved from the lanes to the registers while loading. Then index bits 4, 3
 * and 2 are transformed, each stage with twiddles that differ between the lanes: the table holds
 * per butterfly 4 twiddle vectors as tan(angle/2) and sin(angle), see the table builder in
 * onefft.c. */
__attribute__((target("avx2,fma"))) static void onefft_avx2_la8(float *data, const float *tw, size_t stride, size_t count, size_t regions) {
    float *third;
    float *fifth;
    size_t region_skip;
    size_t j;
    const float *consts = onefft_avx2_la8_consts;
    __asm__ volatile(
        "    mov %[stride], %[region_skip]\n"
        "    lea (%[data],%[stride],2), %[third]\n"
        "    lea (%[data],%[stride],4), %[fifth]\n"
        "    shl $3, %[region_skip]\n"
        "    lea (%[third],%[stride],1), %[third]\n"
        "    lea (%[fifth],%[stride],1), %[fifth]\n"
        "    sub %[stride], %[region_skip]\n"
        "1:\n"
        "    mov %[count], %[j]\n"
        "    shr $1, %[j]\n"
        "2:\n"
        "    vmovups (%[third]), %%ymm0\n"
        "    vmovups 32(%[third]), %%ymm1\n"
        "    vmovups (%[data],%[stride],1), %%ymm2\n"
        "    vmovups 32(%[data],%[stride],1), %%ymm3\n"
        "    vmovups (%[data],%[stride],2), %%ymm4\n"
        "    vmovups 32(%[data],%[stride],2), %%ymm5\n"
        "    vmovups (%[data]), %%ymm6\n"
        "    vmovups 32(%[data]), %%ymm7\n"
        "    vunpcklps (%[third],%[stride],4), %%ymm0, %%ymm8\n"
        "    vunpcklps 32(%[third],%[stride],4), %%ymm1, %%ymm9\n"
        "    vunpcklps (%[fifth]), %%ymm2, %%ymm10\n"
        "    vunpckhps (%[third],%[stride],4), %%ymm0, %%ymm0\n"
        "    vunpckhps 32(%[third],%[stride],4), %%ymm1, %%ymm1\n"
        "    vunpckhps (%[fifth]), %%ymm2, %%ymm2\n"
        "    vfmadd231ps (%[tw]), %%ymm1, %%ymm0\n"
        "    vunpcklps 32(%[fifth]), %%ymm3, %%ymm11\n"
        "    vunpcklps (%[fifth],%[stride],1), %%ymm4, %%ymm12\n"
        "    vunpckhps 32(%[fifth]), %%ymm3, %%ymm3\n"
        "    vunpckhps (%[fifth],%[stride],1), %%ymm4, %%ymm4\n"
        "    vunpcklps 32(%[fifth],%[stride],1), %%ymm5, %%ymm13\n"
        "    vunpckhps 32(%[fifth],%[stride],1), %%ymm5, %%ymm5\n"
        "    vfmadd231ps (%[tw]), %%ymm3, %%ymm2\n"
        "    vfmadd231ps (%[tw]), %%ymm9, %%ymm8\n"
        "    vunpcklps (%[data],%[stride],4), %%ymm6, %%ymm14\n"
        "    vunpcklps 32(%[data],%[stride],4), %%ymm7, %%ymm15\n"
        "    vfmadd231ps (%[tw]), %%ymm11, %%ymm10\n"
        "    vfnmadd231ps 32(%[tw]), %%ymm0, %%ymm1\n"
        "    vunpckhps (%[data],%[stride],4), %%ymm6, %%ymm6\n"
        "    vunpckhps 32(%[data],%[stride],4), %%ymm7, %%ymm7\n"
        "    vfnmadd231ps 32(%[tw]), %%ymm2, %%ymm3\n"
        "    vfnmadd231ps 32(%[tw]), %%ymm8, %%ymm9\n"
        "    vfnmadd231ps 32(%[tw]), %%ymm10, %%ymm11\n"
        "    vfmadd231ps (%[tw]), %%ymm1, %%ymm0\n"
        "    vaddps %%ymm1, %%ymm5, %%ymm5\n"
        "    vfmadd231ps (%[tw]), %%ymm3, %%ymm2\n"
        "    vfmadd231ps (%[tw]), %%ymm9, %%ymm8\n"
        "    vaddps %%ymm3, %%ymm7, %%ymm7\n"
        "    vaddps %%ymm9, %%ymm13, %%ymm13\n"
        "    vfmadd231ps (%[tw]), %%ymm11, %%ymm10\n"
        "    vaddps %%ymm11, %%ymm15, %%ymm15\n"
        "    vaddps %%ymm0, %%ymm4, %%ymm4\n"
        "    vfnmadd132ps (%[consts]), %%ymm5, %%ymm1\n"
        "    vaddps %%ymm2, %%ymm6, %%ymm6\n"
        "    vaddps %%ymm8, %%ymm12, %%ymm12\n"
        "    vfnmadd132ps (%[consts]), %%ymm7, %%ymm3\n"
        "    vfnmadd132ps (%[consts]), %%ymm13, %%ymm9\n"
        "    vaddps %%ymm10, %%ymm14, %%ymm14\n"
        "    vfnmadd132ps (%[consts]), %%ymm15, %%ymm11\n"
        "    vfnmadd132ps (%[consts]), %%ymm4, %%ymm0\n"
        "    vfmadd231ps 64(%[tw]), %%ymm5, %%ymm4\n"
        "    vfnmadd132ps (%[consts]), %%ymm6, %%ymm2\n"
        "    vfnmadd132ps (%[consts]), %%ymm12, %%ymm8\n"
        "    vfmadd231ps 64(%[tw]), %%ymm13, %%ymm12\n"
        "    vfnmadd132ps (%[consts]), %%ymm14, %%ymm10\n"
        "    vfmadd231ps 64(%[tw]), %%ymm1, %%ymm0\n"
        "    vfnmadd231ps 96(%[tw]), %%ymm4, %%ymm5\n"
        "    vfmadd231ps 64(%[tw]), %%ymm9, %%ymm8\n"
        "    vfnmadd231ps 96(%[tw]), %%ymm12, %%ymm13\n"
        "    vfnmadd231ps 96(%[tw]), %%ymm0, %%ymm1\n"
        "    vfmadd231ps 64(%[tw]), %%ymm5, %%ymm4\n"
        "    vaddps %%ymm5, %%ymm7, %%ymm7\n"
        "    vfnmadd231ps 96(%[tw]), %%ymm8, %%ymm9\n"
        "    vfmadd231ps 64(%[tw]), %%ymm13, %%ymm12\n"
        "    vaddps %%ymm13, %%ymm15, %%ymm15\n"
        "    vfmadd231ps 64(%[tw]), %%ymm1, %%ymm0\n"
        "    vaddps %%ymm1, %%ymm2, %%ymm2\n"
        "    vaddps %%ymm4, %%ymm6, %%ymm6\n"
        "    vfnmadd132ps (%[consts]), %%ymm7, %%ymm5\n"
        "    vfmadd231ps 64(%[tw]), %%ymm9, %%ymm8\n"
        "    vaddps %%ymm9, %%ymm10, %%ymm10\n"
        "    vaddps %%ymm12, %%ymm14, %%ymm14\n"
        "    vfnmadd132ps (%[consts]), %%ymm15, %%ymm13\n"
        "    vsubps %%ymm0, %%ymm3, %%ymm3\n"
        "    vfnmadd132ps (%[consts]), %%ymm2, %%ymm1\n"
        "    vfnmadd132ps (%[consts]), %%ymm6, %%ymm4\n"
        "    vfmadd231ps 128(%[tw]), %%ymm7, %%ymm6\n"
        "    vsubps %%ymm8, %%ymm11, %%ymm11\n"
        "    vfnmadd132ps (%[consts]), %%ymm10, %%ymm9\n"
        "    vfnmadd132ps (%[consts]), %%ymm14, %%ymm12\n"
        "    vfmadd132ps (%[consts]), %%ymm3, %%ymm0\n"
        "    vfmadd231ps 192(%[tw]), %%ymm3, %%ymm2\n"
        "    vfmadd231ps 128(%[tw]), %%ymm5, %%ymm4\n"
        "    vfnmadd231ps 160(%[tw]), %%ymm6, %%ymm7\n"
        "    vfmadd132ps (%[consts]), %%ymm11, %%ymm8\n"
        "    vfmadd231ps 192(%[tw]), %%ymm0, %%ymm1\n"
        "    vfnmadd231ps 224(%[tw]), %%ymm2, %%ymm3\n"
        "    vfnmadd231ps 160(%[tw]), %%ymm4, %%ymm5\n"
        "    vfmadd231ps 128(%[tw]), %%ymm7, %%ymm6\n"
        "    vaddps %%ymm7, %%ymm15, %%ymm15\n"
        "    vfnmadd231ps 224(%[tw]), %%ymm1, %%ymm0\n"
        "    vfmadd231ps 192(%[tw]), %%ymm3, %%ymm2\n"
        "    vaddps %%ymm3, %%ymm11, %%ymm11\n"
        "    vfmadd231ps 128(%[tw]), %%ymm5, %%ymm4\n"
        "    vaddps %%ymm5, %%ymm12, %%ymm12\n"
        "    vaddps %%ymm6, %%ymm14, %%ymm14\n"
        "    vfnmadd132ps (%[consts]), %%ymm15, %%ymm7\n"
        "    vmovups %%ymm15, 32(%[data])\n"
        "    vfmadd231ps 192(%[tw]), %%ymm0, %%ymm1\n"
        "    vaddps %%ymm2, %%ymm10, %%ymm10\n"
        "    vaddps %%ymm0, %%ymm9, %%ymm9\n"
        "    vmovups %%ymm11, 32(%[data],%[stride],1)\n"
        "    vsubps %%ymm4, %%ymm13, %%ymm13\n"
        "    vfnmadd132ps (%[consts]), %%ymm12, %%ymm5\n"
        "    vfnmadd132ps (%[consts]), %%ymm11, %%ymm3\n"
        "    vmovups %%ymm12, (%[data],%[stride],2)\n"
        "    lea 256(%[tw]), %[tw]\n"
        "    vfnmadd132ps (%[consts]), %%ymm14, %%ymm6\n"
        "    vmovups %%ymm14, (%[data])\n"
        "    vmovups %%ymm7, 32(%[data],%[stride],4)\n"
        "    vsubps %%ymm1, %%ymm8, %%ymm8\n"
        "    vfnmadd132ps (%[consts]), %%ymm10, %%ymm2\n"
        "    vfnmadd132ps (%[consts]), %%ymm9, %%ymm0\n"
        "    vmovups %%ymm10, (%[data],%[stride],1)\n"
        "    vmovups %%ymm9, (%[third])\n"
        "    vfmadd132ps (%[consts]), %%ymm13, %%ymm4\n"
        "    vmovups %%ymm13, 32(%[data],%[stride],2)\n"
        "    vmovups %%ymm3, 32(%[fifth])\n"
        "    vmovups %%ymm5, (%[fifth],%[stride],1)\n"
        "    vmovups %%ymm6, (%[data],%[stride],4)\n"
        "    lea 64(%[data]), %[data]\n"
        "    vfmadd132ps (%[consts]), %%ymm8, %%ymm1\n"
        "    vmovups %%ymm8, 32(%[third])\n"
        "    vmovups %%ymm2, (%[fifth])\n"
        "    vmovups %%ymm0, (%[third],%[stride],4)\n"
        "    vmovups %%ymm4, 32(%[fifth],%[stride],1)\n"
        "    lea 64(%[fifth]), %[fifth]\n"
        "    vmovups %%ymm1, 32(%[third],%[stride],4)\n"
        "    lea 64(%[third]), %[third]\n"
        "    sub $1, %[j]\n"
        "    jnz 2b\n"
        "    mov %[count], %[j]\n"
        "    shr $1, %[j]\n"
        "3:\n"
        "    vmovups (%[third]), %%ymm0\n"
        "    vmovups 32(%[third]), %%ymm1\n"
        "    vmovups (%[data],%[stride],1), %%ymm2\n"
        "    vmovups 32(%[data],%[stride],1), %%ymm3\n"
        "    vmovups (%[data],%[stride],2), %%ymm4\n"
        "    vmovups 32(%[data],%[stride],2), %%ymm5\n"
        "    vmovups (%[data]), %%ymm6\n"
        "    vmovups 32(%[data]), %%ymm7\n"
        "    vunpcklps (%[third],%[stride],4), %%ymm0, %%ymm8\n"
        "    vunpcklps 32(%[third],%[stride],4), %%ymm1, %%ymm9\n"
        "    vunpcklps (%[fifth]), %%ymm2, %%ymm10\n"
        "    vunpckhps (%[third],%[stride],4), %%ymm0, %%ymm0\n"
        "    vunpckhps 32(%[third],%[stride],4), %%ymm1, %%ymm1\n"
        "    vunpckhps (%[fifth]), %%ymm2, %%ymm2\n"
        "    vfmadd231ps (%[tw]), %%ymm1, %%ymm0\n"
        "    vunpcklps 32(%[fifth]), %%ymm3, %%ymm11\n"
        "    vunpcklps (%[fifth],%[stride],1), %%ymm4, %%ymm12\n"
        "    vunpckhps 32(%[fifth]), %%ymm3, %%ymm3\n"
        "    vunpckhps (%[fifth],%[stride],1), %%ymm4, %%ymm4\n"
        "    vunpcklps 32(%[fifth],%[stride],1), %%ymm5, %%ymm13\n"
        "    vunpckhps 32(%[fifth],%[stride],1), %%ymm5, %%ymm5\n"
        "    vfmadd231ps (%[tw]), %%ymm3, %%ymm2\n"
        "    vfmadd231ps (%[tw]), %%ymm9, %%ymm8\n"
        "    vunpcklps (%[data],%[stride],4), %%ymm6, %%ymm14\n"
        "    vunpcklps 32(%[data],%[stride],4), %%ymm7, %%ymm15\n"
        "    vfmadd231ps (%[tw]), %%ymm11, %%ymm10\n"
        "    vfnmadd231ps 32(%[tw]), %%ymm0, %%ymm1\n"
        "    vunpckhps (%[data],%[stride],4), %%ymm6, %%ymm6\n"
        "    vunpckhps 32(%[data],%[stride],4), %%ymm7, %%ymm7\n"
        "    vfnmadd231ps 32(%[tw]), %%ymm2, %%ymm3\n"
        "    vfnmadd231ps 32(%[tw]), %%ymm8, %%ymm9\n"
        "    vfnmadd231ps 32(%[tw]), %%ymm10, %%ymm11\n"
        "    vfmadd231ps (%[tw]), %%ymm1, %%ymm0\n"
        "    vaddps %%ymm1, %%ymm4, %%ymm4\n"
        "    vfmadd231ps (%[tw]), %%ymm3, %%ymm2\n"
        "    vfmadd231ps (%[tw]), %%ymm9, %%ymm8\n"
        "    vaddps %%ymm3, %%ymm6, %%ymm6\n"
        "    vaddps %%ymm9, %%ymm12, %%ymm12\n"
        "    vfmadd231ps (%[tw]), %%ymm11, %%ymm10\n"
        "    vaddps %%ymm11, %%ymm14, %%ymm14\n"
        "    vsubps %%ymm0, %%ymm5, %%ymm5\n"
        "    vfnmadd132ps (%[consts]), %%ymm4, %%ymm1\n"
        "    vsubps %%ymm2, %%ymm7, %%ymm7\n"
        "    vsubps %%ymm8, %%ymm13, %%ymm13\n"
        "    vfnmadd132ps (%[consts]), %%ymm6, %%ymm3\n"
        "    vfnmadd132ps (%[consts]), %%ymm12, %%ymm9\n"
        "    vsubps %%ymm10, %%ymm15, %%ymm15\n"
        "    vfnmadd132ps (%[consts]), %%ymm14, %%ymm11\n"
        "    vfmadd132ps (%[consts]), %%ymm5, %%ymm0\n"
        "    vfmadd231ps 64(%[tw]), %%ymm5, %%ymm4\n"
        "    vfmadd132ps (%[consts]), %%ymm7, %%ymm2\n"
        "    vfmadd132ps (%[consts]), %%ymm13, %%ymm8\n"
        "    vfmadd231ps 64(%[tw]), %%ymm13, %%ymm12\n"
        "    vfmadd132ps (%[consts]), %%ymm15, %%ymm10\n"
        "    vfmadd231ps 64(%[tw]), %%ymm0, %%ymm1\n"
        "    vfnmadd231ps 96(%[tw]), %%ymm4, %%ymm5\n"
        "    vfmadd231ps 64(%[tw]), %%ymm8, %%ymm9\n"
        "    vfnmadd231ps 96(%[tw]), %%ymm12, %%ymm13\n"
        "    vfnmadd231ps 96(%[tw]), %%ymm1, %%ymm0\n"
        "    vfmadd231ps 64(%[tw]), %%ymm5, %%ymm4\n"
        "    vaddps %%ymm5, %%ymm7, %%ymm7\n"
        "    vfnmadd231ps 96(%[tw]), %%ymm9, %%ymm8\n"
        "    vfmadd231ps 64(%[tw]), %%ymm13, %%ymm12\n"
        "    vaddps %%ymm13, %%ymm15, %%ymm15\n"
        "    vfmadd231ps 64(%[tw]), %%ymm0, %%ymm1\n"
        "    vaddps %%ymm4, %%ymm6, %%ymm6\n"
        "    vaddps %%ymm0, %%ymm3, %%ymm3\n"
        "    vfnmadd132ps (%[consts]), %%ymm7, %%ymm5\n"
        "    vfmadd231ps 64(%[tw]), %%ymm8, %%ymm9\n"
        "    vaddps %%ymm12, %%ymm14, %%ymm14\n"
        "    vaddps %%ymm8, %%ymm11, %%ymm11\n"
        "    vfnmadd132ps (%[consts]), %%ymm15, %%ymm13\n"
        "    vsubps %%ymm1, %%ymm2, %%ymm2\n"
        "    vfnmadd132ps (%[consts]), %%ymm6, %%ymm4\n"
        "    vfnmadd132ps (%[consts]), %%ymm3, %%ymm0\n"
        "    vfmadd231ps 128(%[tw]), %%ymm7, %%ymm6\n"
        "    vsubps %%ymm9, %%ymm10, %%ymm10\n"
        "    vfnmadd132ps (%[consts]), %%ymm14, %%ymm12\n"
        "    vfnmadd132ps (%[consts]), %%ymm11, %%ymm8\n"
        "    vfmadd132ps (%[consts]), %%ymm2, %%ymm1\n"
        "    vfmadd231ps 128(%[tw]), %%ymm5, %%ymm4\n"
        "    vfmadd231ps 192(%[tw]), %%ymm2, %%ymm3\n"
        "    vfnmadd231ps 160(%[tw]), %%ymm6, %%ymm7\n"
        "    vfmadd132ps (%[consts]), %%ymm10, %%ymm9\n"
        "    vfmadd231ps 192(%[tw]), %%ymm1, %%ymm0\n"
        "    vfnmadd231ps 160(%[tw]), %%ymm4, %%ymm5\n"
        "    vfnmadd231ps 224(%[tw]), %%ymm3, %%ymm2\n"
        "    vfmadd231ps 128(%[tw]), %%ymm7, %%ymm6\n"
        "    vaddps %%ymm7, %%ymm15, %%ymm15\n"
        "    vfnmadd231ps 224(%[tw]), %%ymm0, %%ymm1\n"
        "    vfmadd231ps 128(%[tw]), %%ymm5, %%ymm4\n"
        "    vfmadd231ps 192(%[tw]), %%ymm2, %%ymm3\n"
        "    vaddps %%ymm6, %%ymm14, %%ymm14\n"
        "    vaddps %%ymm5, %%ymm12, %%ymm12\n"
        "    vaddps %%ymm2, %%ymm10, %%ymm10\n"
        "    vmovups %%ymm15, 32(%[data])\n"
        "    vfnmadd132ps (%[consts]), %%ymm15, %%ymm7\n"
        "    vfmadd231ps 192(%[tw]), %%ymm1, %%ymm0\n"
        "    vsubps %%ymm4, %%ymm13, %%ymm13\n"
        "    vaddps %%ymm3, %%ymm11, %%ymm11\n"
        "    vaddps %%ymm1, %%ymm8, %%ymm8\n"
        "    vfnmadd132ps (%[consts]), %%ymm14, %%ymm6\n"
        "    vfnmadd132ps (%[consts]), %%ymm12, %%ymm5\n"
        "    vmovups %%ymm14, (%[data])\n"
        "    vmovups %%ymm10, 32(%[data],%[stride],1)\n"
        "    vmovups %%ymm12, (%[data],%[stride],2)\n"
        "    lea 256(%[tw]), %[tw]\n"
        "    vfnmadd132ps (%[consts]), %%ymm10, %%ymm2\n"
        "    vmovups %%ymm7, 32(%[data],%[stride],4)\n"
        "    vsubps %%ymm0, %%ymm9, %%ymm9\n"
        "    vfmadd132ps (%[consts]), %%ymm13, %%ymm4\n"
        "    vfnmadd132ps (%[consts]), %%ymm11, %%ymm3\n"
        "    vmovups %%ymm11, (%[data],%[stride],1)\n"
        "    vmovups %%ymm13, 32(%[data],%[stride],2)\n"
        "    vfnmadd132ps (%[consts]), %%ymm8, %%ymm1\n"
        "    vmovups %%ymm8, (%[third])\n"
        "    vmovups %%ymm6, (%[data],%[stride],4)\n"
        "    vmovups %%ymm5, (%[fifth],%[stride],1)\n"
        "    vmovups %%ymm2, 32(%[fifth])\n"
        "    lea 64(%[data]), %[data]\n"
        "    vfmadd132ps (%[consts]), %%ymm9, %%ymm0\n"
        "    vmovups %%ymm9, 32(%[third])\n"
        "    vmovups %%ymm3, (%[fifth])\n"
        "    vmovups %%ymm4, 32(%[fifth],%[stride],1)\n"
        "    vmovups %%ymm1, (%[third],%[stride],4)\n"
        "    lea 64(%[fifth]), %[fifth]\n"
        "    vmovups %%ymm0, 32(%[third],%[stride],4)\n"
        "    lea 64(%[third]), %[third]\n"
        "    sub $1, %[j]\n"
        "    jnz 3b\n"
        "    lea (%[data],%[region_skip],1), %[data]\n"
        "    lea (%[third],%[region_skip],1), %[third]\n"
        "    lea (%[fifth],%[region_skip],1), %[fifth]\n"
        "    sub $1, %[regions]\n"
        "    jnz 1b\n"
        "    vzeroupper\n"
        : [data] "+r"(data), [tw] "+r"(tw), [stride] "+r"(stride), [count] "+r"(count), [regions] "+r"(regions), [third] "=&r"(third), [fifth] "=&r"(fifth), [region_skip] "=&r"(region_skip), [j] "=&r"(j), [consts] "+r"(consts)
        :
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "cc", "memory");
}

static const float onefft_avx2_la8x_consts[8] ONEFFT_ALIGNED(64) = {
    0x1.0000000000000p+1f, 0x1.0000000000000p+1f, 0x1.0000000000000p+1f, 0x1.0000000000000p+1f,
    0x1.0000000000000p+1f, 0x1.0000000000000p+1f, 0x1.0000000000000p+1f, 0x1.0000000000000p+1f,
};
/* Second to last pass for three index bits, in place on split vectors. Butterfly j < count
 * of a region uses the vectors at data + u*stride + 64*j, u < 8; the `regions` regions follow each
 * other. Index bit 2 is moved from the lanes to the registers while loading. Then index bits 4, 3
 * and 2 are transformed, each stage with twiddles that differ between the lanes: the table holds
 * per butterfly 4 twiddle vectors as tan(angle/2) and sin(angle), see the table builder in
 * onefft.c. This variant is for a single butterfly per region (256 points). */
__attribute__((target("avx2,fma"))) static void onefft_avx2_la8x(float *data, const float *tw, size_t stride, size_t count, size_t regions) {
    float *third;
    float *fifth;
    size_t region_skip;
    size_t j;
    const float *consts = onefft_avx2_la8x_consts;
    __asm__ volatile(
        "    mov %[stride], %[region_skip]\n"
        "    lea (%[data],%[stride],2), %[third]\n"
        "    lea (%[data],%[stride],4), %[fifth]\n"
        "    shl $3, %[region_skip]\n"
        "    lea (%[third],%[stride],1), %[third]\n"
        "    lea (%[fifth],%[stride],1), %[fifth]\n"
        "    sub %[stride], %[region_skip]\n"
        "1:\n"
        "    mov %[count], %[j]\n"
        "2:\n"
        "    vmovups (%[third]), %%ymm0\n"
        "    vmovups 32(%[third]), %%ymm1\n"
        "    vmovups (%[data],%[stride],1), %%ymm2\n"
        "    vmovups 32(%[data],%[stride],1), %%ymm3\n"
        "    vmovups (%[data],%[stride],2), %%ymm4\n"
        "    vmovups 32(%[data],%[stride],2), %%ymm5\n"
        "    vmovups (%[data]), %%ymm6\n"
        "    vmovups 32(%[data]), %%ymm7\n"
        "    vunpcklps (%[third],%[stride],4), %%ymm0, %%ymm8\n"
        "    vunpcklps 32(%[third],%[stride],4), %%ymm1, %%ymm9\n"
        "    vunpcklps (%[fifth]), %%ymm2, %%ymm10\n"
        "    vunpckhps (%[third],%[stride],4), %%ymm0, %%ymm0\n"
        "    vunpckhps 32(%[third],%[stride],4), %%ymm1, %%ymm1\n"
        "    vunpckhps (%[fifth]), %%ymm2, %%ymm2\n"
        "    vfmadd231ps (%[tw]), %%ymm1, %%ymm0\n"
        "    vunpcklps 32(%[fifth]), %%ymm3, %%ymm11\n"
        "    vunpcklps (%[fifth],%[stride],1), %%ymm4, %%ymm12\n"
        "    vunpckhps 32(%[fifth]), %%ymm3, %%ymm3\n"
        "    vunpckhps (%[fifth],%[stride],1), %%ymm4, %%ymm4\n"
        "    vunpcklps 32(%[fifth],%[stride],1), %%ymm5, %%ymm13\n"
        "    vunpckhps 32(%[fifth],%[stride],1), %%ymm5, %%ymm5\n"
        "    vfmadd231ps (%[tw]), %%ymm3, %%ymm2\n"
        "    vfmadd231ps (%[tw]), %%ymm9, %%ymm8\n"
        "    vunpcklps (%[data],%[stride],4), %%ymm6, %%ymm14\n"
        "    vunpcklps 32(%[data],%[stride],4), %%ymm7, %%ymm15\n"
        "    vfmadd231ps (%[tw]), %%ymm11, %%ymm10\n"
        "    vfnmadd231ps 32(%[tw]), %%ymm0, %%ymm1\n"
        "    vunpckhps (%[data],%[stride],4), %%ymm6, %%ymm6\n"
        "    vunpckhps 32(%[data],%[stride],4), %%ymm7, %%ymm7\n"
        "    vfnmadd231ps 32(%[tw]), %%ymm2, %%ymm3\n"
        "    vfnmadd231ps 32(%[tw]), %%ymm8, %%ymm9\n"
        "    vfnmadd231ps 32(%[tw]), %%ymm10, %%ymm11\n"
        "    vfmadd231ps (%[tw]), %%ymm1, %%ymm0\n"
        "    vaddps %%ymm1, %%ymm5, %%ymm5\n"
        "    vfmadd231ps (%[tw]), %%ymm3, %%ymm2\n"
        "    vfmadd231ps (%[tw]), %%ymm9, %%ymm8\n"
        "    vaddps %%ymm3, %%ymm7, %%ymm7\n"
        "    vaddps %%ymm9, %%ymm13, %%ymm13\n"
        "    vfmadd231ps (%[tw]), %%ymm11, %%ymm10\n"
        "    vaddps %%ymm11, %%ymm15, %%ymm15\n"
        "    vaddps %%ymm0, %%ymm4, %%ymm4\n"
        "    vfnmadd132ps (%[consts]), %%ymm5, %%ymm1\n"
        "    vaddps %%ymm2, %%ymm6, %%ymm6\n"
        "    vaddps %%ymm8, %%ymm12, %%ymm12\n"
        "    vfnmadd132ps (%[consts]), %%ymm7, %%ymm3\n"
        "    vfnmadd132ps (%[consts]), %%ymm13, %%ymm9\n"
        "    vaddps %%ymm10, %%ymm14, %%ymm14\n"
        "    vfnmadd132ps (%[consts]), %%ymm15, %%ymm11\n"
        "    vfnmadd132ps (%[consts]), %%ymm4, %%ymm0\n"
        "    vfmadd231ps 64(%[tw]), %%ymm5, %%ymm4\n"
        "    vfnmadd132ps (%[consts]), %%ymm6, %%ymm2\n"
        "    vfnmadd132ps (%[consts]), %%ymm12, %%ymm8\n"
        "    vfmadd231ps 64(%[tw]), %%ymm13, %%ymm12\n"
        "    vfnmadd132ps (%[consts]), %%ymm14, %%ymm10\n"
        "    vfmadd231ps 64(%[tw]), %%ymm1, %%ymm0\n"
        "    vfnmadd231ps 96(%[tw]), %%ymm4, %%ymm5\n"
        "    vfmadd231ps 64(%[tw]), %%ymm9, %%ymm8\n"
        "    vfnmadd231ps 96(%[tw]), %%ymm12, %%ymm13\n"
        "    vfnmadd231ps 96(%[tw]), %%ymm0, %%ymm1\n"
        "    vfmadd231ps 64(%[tw]), %%ymm5, %%ymm4\n"
        "    vaddps %%ymm5, %%ymm7, %%ymm7\n"
        "    vfnmadd231ps 96(%[tw]), %%ymm8, %%ymm9\n"
        "    vfmadd231ps 64(%[tw]), %%ymm13, %%ymm12\n"
        "    vaddps %%ymm13, %%ymm15, %%ymm15\n"
        "    vfmadd231ps 64(%[tw]), %%ymm1, %%ymm0\n"
        "    vaddps %%ymm1, %%ymm2, %%ymm2\n"
        "    vaddps %%ymm4, %%ymm6, %%ymm6\n"
        "    vfnmadd132ps (%[consts]), %%ymm7, %%ymm5\n"
        "    vfmadd231ps 64(%[tw]), %%ymm9, %%ymm8\n"
        "    vaddps %%ymm9, %%ymm10, %%ymm10\n"
        "    vaddps %%ymm12, %%ymm14, %%ymm14\n"
        "    vfnmadd132ps (%[consts]), %%ymm15, %%ymm13\n"
        "    vsubps %%ymm0, %%ymm3, %%ymm3\n"
        "    vfnmadd132ps (%[consts]), %%ymm2, %%ymm1\n"
        "    vfnmadd132ps (%[consts]), %%ymm6, %%ymm4\n"
        "    vfmadd231ps 128(%[tw]), %%ymm7, %%ymm6\n"
        "    vsubps %%ymm8, %%ymm11, %%ymm11\n"
        "    vfnmadd132ps (%[consts]), %%ymm10, %%ymm9\n"
        "    vfnmadd132ps (%[consts]), %%ymm14, %%ymm12\n"
        "    vfmadd132ps (%[consts]), %%ymm3, %%ymm0\n"
        "    vfmadd231ps 192(%[tw]), %%ymm3, %%ymm2\n"
        "    vfmadd231ps 128(%[tw]), %%ymm5, %%ymm4\n"
        "    vfnmadd231ps 160(%[tw]), %%ymm6, %%ymm7\n"
        "    vfmadd132ps (%[consts]), %%ymm11, %%ymm8\n"
        "    vfmadd231ps 192(%[tw]), %%ymm0, %%ymm1\n"
        "    vfnmadd231ps 224(%[tw]), %%ymm2, %%ymm3\n"
        "    vfnmadd231ps 160(%[tw]), %%ymm4, %%ymm5\n"
        "    vfmadd231ps 128(%[tw]), %%ymm7, %%ymm6\n"
        "    vaddps %%ymm7, %%ymm15, %%ymm15\n"
        "    vfnmadd231ps 224(%[tw]), %%ymm1, %%ymm0\n"
        "    vfmadd231ps 192(%[tw]), %%ymm3, %%ymm2\n"
        "    vaddps %%ymm3, %%ymm11, %%ymm11\n"
        "    vfmadd231ps 128(%[tw]), %%ymm5, %%ymm4\n"
        "    vaddps %%ymm5, %%ymm12, %%ymm12\n"
        "    vaddps %%ymm6, %%ymm14, %%ymm14\n"
        "    vfnmadd132ps (%[consts]), %%ymm15, %%ymm7\n"
        "    vmovups %%ymm15, 32(%[data])\n"
        "    vfmadd231ps 192(%[tw]), %%ymm0, %%ymm1\n"
        "    vaddps %%ymm2, %%ymm10, %%ymm10\n"
        "    vaddps %%ymm0, %%ymm9, %%ymm9\n"
        "    vmovups %%ymm11, 32(%[data],%[stride],1)\n"
        "    vsubps %%ymm4, %%ymm13, %%ymm13\n"
        "    vfnmadd132ps (%[consts]), %%ymm12, %%ymm5\n"
        "    vfnmadd132ps (%[consts]), %%ymm11, %%ymm3\n"
        "    vmovups %%ymm12, (%[data],%[stride],2)\n"
        "    lea 256(%[tw]), %[tw]\n"
        "    vfnmadd132ps (%[consts]), %%ymm14, %%ymm6\n"
        "    vmovups %%ymm14, (%[data])\n"
        "    vmovups %%ymm7, 32(%[data],%[stride],4)\n"
        "    vsubps %%ymm1, %%ymm8, %%ymm8\n"
        "    vfnmadd132ps (%[consts]), %%ymm10, %%ymm2\n"
        "    vfnmadd132ps (%[consts]), %%ymm9, %%ymm0\n"
        "    vmovups %%ymm10, (%[data],%[stride],1)\n"
        "    vmovups %%ymm9, (%[third])\n"
        "    vfmadd132ps (%[consts]), %%ymm13, %%ymm4\n"
        "    vmovups %%ymm13, 32(%[data],%[stride],2)\n"
        "    vmovups %%ymm3, 32(%[fifth])\n"
        "    vmovups %%ymm5, (%[fifth],%[stride],1)\n"
        "    vmovups %%ymm6, (%[data],%[stride],4)\n"
        "    lea 64(%[data]), %[data]\n"
        "    vfmadd132ps (%[consts]), %%ymm8, %%ymm1\n"
        "    vmovups %%ymm8, 32(%[third])\n"
        "    vmovups %%ymm2, (%[fifth])\n"
        "    vmovups %%ymm0, (%[third],%[stride],4)\n"
        "    vmovups %%ymm4, 32(%[fifth],%[stride],1)\n"
        "    lea 64(%[fifth]), %[fifth]\n"
        "    vmovups %%ymm1, 32(%[third],%[stride],4)\n"
        "    lea 64(%[third]), %[third]\n"
        "    sub $1, %[j]\n"
        "    jnz 2b\n"
        "    lea (%[data],%[region_skip],1), %[data]\n"
        "    lea (%[third],%[region_skip],1), %[third]\n"
        "    lea (%[fifth],%[region_skip],1), %[fifth]\n"
        "    sub $1, %[regions]\n"
        "    jnz 1b\n"
        "    vzeroupper\n"
        : [data] "+r"(data), [tw] "+r"(tw), [stride] "+r"(stride), [count] "+r"(count), [regions] "+r"(regions), [third] "=&r"(third), [fifth] "=&r"(fifth), [region_skip] "=&r"(region_skip), [j] "=&r"(j), [consts] "+r"(consts)
        :
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "cc", "memory");
}

#pragma GCC diagnostic pop
#ifndef ONEFFT_NO_AVX512
#define ONEFFT_AVX512 1
/*
 * OneFFT x86-64 AVX-512 kernels.
 *
 * GENERATED FILE, DO NOT EDIT. Change gen/onefft_gen and run gen/generate.py instead.
 *
 * Every kernel is a normal C function whose body is one inline assembly block. The SIMD registers
 * are assigned by the generator, the general purpose registers by the C compiler.
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverlength-strings"

static const float onefft_avx512_f8_consts[4] ONEFFT_ALIGNED(64) = {
    0x1.0000000000000p+1f, 0x1.6a09e667f3bcdp-1f, 0x0.0p+0f, 0x0.0p+0f,
};
/* First pass, radix 8. For each of the `count` entries (a, b) of `table`: reads 8 interleaved
 * vectors from in + 8*a + u*in_stride, transforms across them and writes output q as a split
 * vector to out + 8*b + r*out_stride, where q = low_bits(r). */
__attribute__((target("avx512f"))) static void onefft_avx512_f8(const float *in, float *out, const uint32_t *table, size_t count, size_t in_stride, size_t out_stride) {
    const float *src;
    float *dst;
    size_t a;
    size_t b;
    const float *consts = onefft_avx512_f8_consts;
    __asm__ volatile(
        "    vbroadcastss (%[consts]), %%zmm0\n"
        "    vbroadcastss 4(%[consts]), %%zmm1\n"
        "1:\n"
        "    movl (%[table]), %k[a]\n"
        "    movl 4(%[table]), %k[b]\n"
        "    lea 8(%[table]), %[table]\n"
        "    lea (%[in],%[a],8), %[src]\n"
        "    lea (%[out],%[b],8), %[dst]\n"
        "    vmovups (%[src]), %%zmm2\n"
        "    vshufps $136, 64(%[src]), %%zmm2, %%zmm3\n"
        "    vshufps $221, 64(%[src]), %%zmm2, %%zmm2\n"
        "    lea (%[src],%[in_stride],1), %[src]\n"
        "    vmovups (%[src]), %%zmm4\n"
        "    vshufps $136, 64(%[src]), %%zmm4, %%zmm5\n"
        "    vshufps $221, 64(%[src]), %%zmm4, %%zmm4\n"
        "    lea (%[src],%[in_stride],1), %[src]\n"
        "    vmovups (%[src]), %%zmm6\n"
        "    vshufps $136, 64(%[src]), %%zmm6, %%zmm7\n"
        "    vshufps $221, 64(%[src]), %%zmm6, %%zmm6\n"
        "    lea (%[src],%[in_stride],1), %[src]\n"
        "    vmovups (%[src]), %%zmm8\n"
        "    vshufps $136, 64(%[src]), %%zmm8, %%zmm9\n"
        "    vshufps $221, 64(%[src]), %%zmm8, %%zmm8\n"
        "    lea (%[src],%[in_stride],1), %[src]\n"
        "    vmovups (%[src]), %%zmm10\n"
        "    vshufps $136, 64(%[src]), %%zmm10, %%zmm11\n"
        "    vshufps $221, 64(%[src]), %%zmm10, %%zmm10\n"
        "    lea (%[src],%[in_stride],1), %[src]\n"
        "    vaddps %%zmm11, %%zmm3, %%zmm3\n"
        "    vaddps %%zmm10, %%zmm2, %%zmm2\n"
        "    vmovups (%[src]), %%zmm12\n"
        "    vfnmadd132ps %%zmm0, %%zmm3, %%zmm11\n"
        "    vfnmadd132ps %%zmm0, %%zmm2, %%zmm10\n"
        "    vshufps $136, 64(%[src]), %%zmm12, %%zmm13\n"
        "    vshufps $221, 64(%[src]), %%zmm12, %%zmm12\n"
        "    lea (%[src],%[in_stride],1), %[src]\n"
        "    vaddps %%zmm13, %%zmm5, %%zmm5\n"
        "    vaddps %%zmm12, %%zmm4, %%zmm4\n"
        "    vmovups (%[src]), %%zmm14\n"
        "    vfnmadd132ps %%zmm0, %%zmm5, %%zmm13\n"
        "    vfnmadd132ps %%zmm0, %%zmm4, %%zmm12\n"
        "    vshufps $136, 64(%[src]), %%zmm14, %%zmm15\n"
        "    vshufps $221, 64(%[src]), %%zmm14, %%zmm14\n"
        "    lea (%[src],%[in_stride],1), %[src]\n"
        "    vaddps %%zmm15, %%zmm7, %%zmm7\n"
        "    vaddps %%zmm14, %%zmm6, %%zmm6\n"
        "    vmovups (%[src]), %%zmm16\n"
        "    vfnmadd132ps %%zmm0, %%zmm7, %%zmm15\n"
        "    vfnmadd132ps %%zmm0, %%zmm6, %%zmm14\n"
        "    vaddps %%zmm7, %%zmm3, %%zmm3\n"
        "    vaddps %%zmm6, %%zmm2, %%zmm2\n"
        "    vshufps $136, 64(%[src]), %%zmm16, %%zmm17\n"
        "    vshufps $221, 64(%[src]), %%zmm16, %%zmm16\n"
        "    vaddps %%zmm17, %%zmm9, %%zmm9\n"
        "    vaddps %%zmm16, %%zmm8, %%zmm8\n"
        "    vfnmadd132ps %%zmm0, %%zmm3, %%zmm7\n"
        "    vaddps %%zmm14, %%zmm11, %%zmm11\n"
        "    vsubps %%zmm15, %%zmm10, %%zmm10\n"
        "    vfnmadd132ps %%zmm0, %%zmm2, %%zmm6\n"
        "    vfnmadd132ps %%zmm0, %%zmm9, %%zmm17\n"
        "    vfnmadd132ps %%zmm0, %%zmm8, %%zmm16\n"
        "    vaddps %%zmm9, %%zmm5, %%zmm5\n"
        "    vaddps %%zmm8, %%zmm4, %%zmm4\n"
        "    vfnmadd132ps %%zmm0, %%zmm11, %%zmm14\n"
        "    vfmadd132ps %%zmm0, %%zmm10, %%zmm15\n"
        "    vaddps %%zmm16, %%zmm13, %%zmm13\n"
        "    vsubps %%zmm17, %%zmm12, %%zmm12\n"
        "    vfnmadd132ps %%zmm0, %%zmm5, %%zmm9\n"
        "    vfnmadd132ps %%zmm0, %%zmm4, %%zmm8\n"
        "    vaddps %%zmm5, %%zmm3, %%zmm3\n"
        "    vaddps %%zmm4, %%zmm2, %%zmm2\n"
        "    vfnmadd132ps %%zmm0, %%zmm13, %%zmm16\n"
        "    vfmadd132ps %%zmm0, %%zmm12, %%zmm17\n"
        "    vsubps %%zmm9, %%zmm6, %%zmm6\n"
        "    vaddps %%zmm12, %%zmm13, %%zmm13\n"
        "    vaddps %%zmm8, %%zmm7, %%zmm7\n"
        "    vfnmadd132ps %%zmm0, %%zmm3, %%zmm5\n"
        "    vmovups %%zmm3, (%[dst])\n"
        "    vmovups %%zmm2, 64(%[dst])\n"
        "    vfnmadd132ps %%zmm0, %%zmm2, %%zmm4\n"
        "    lea (%[dst],%[out_stride],1), %[dst]\n"
        "    vaddps %%zmm17, %%zmm16, %%zmm16\n"
        "    vfmadd132ps %%zmm0, %%zmm6, %%zmm9\n"
        "    vmovups %%zmm6, 64(%[dst])\n"
        "    vfmsub132ps %%zmm0, %%zmm13, %%zmm12\n"
        "    vfnmadd132ps %%zmm0, %%zmm7, %%zmm8\n"
        "    vmovups %%zmm7, (%[dst])\n"
        "    vmulps %%zmm1, %%zmm13, %%zmm13\n"
        "    lea (%[dst],%[out_stride],1), %[dst]\n"
        "    vmovups %%zmm5, (%[dst])\n"
        "    vmovups %%zmm4, 64(%[dst])\n"
        "    vfmsub132ps %%zmm0, %%zmm16, %%zmm17\n"
        "    lea (%[dst],%[out_stride],1), %[dst]\n"
        "    vmulps %%zmm1, %%zmm12, %%zmm12\n"
        "    vmulps %%zmm1, %%zmm16, %%zmm16\n"
        "    vmovups %%zmm8, (%[dst])\n"
        "    vmovups %%zmm9, 64(%[dst])\n"
        "    vaddps %%zmm13, %%zmm11, %%zmm11\n"
        "    lea (%[dst],%[out_stride],1), %[dst]\n"
        "    vmulps %%zmm1, %%zmm17, %%zmm17\n"
        "    vaddps %%zmm12, %%zmm10, %%zmm10\n"
        "    vsubps %%zmm16, %%zmm15, %%zmm15\n"
        "    vfnmadd132ps %%zmm0, %%zmm11, %%zmm13\n"
        "    vmovups %%zmm11, (%[dst])\n"
        "    vaddps %%zmm17, %%zmm14, %%zmm14\n"
        "    vfnmadd132ps %%zmm0, %%zmm10, %%zmm12\n"
        "    vfmadd132ps %%zmm0, %%zmm15, %%zmm16\n"
        "    vmovups %%zmm10, 64(%[dst])\n"
        "    lea (%[dst],%[out_stride],1), %[dst]\n"
        "    vmovups %%zmm15, 64(%[dst])\n"
        "    vfnmadd132ps %%zmm0, %%zmm14, %%zmm17\n"
        "    vmovups %%zmm14, (%[dst])\n"
        "    lea (%[dst],%[out_stride],1), %[dst]\n"
        "    vmovups %%zmm13, (%[dst])\n"
        "    vmovups %%zmm12, 64(%[dst])\n"
        "    lea (%[dst],%[out_stride],1), %[dst]\n"
        "    vmovups %%zmm17, (%[dst])\n"
        "    vmovups %%zmm16, 64(%[dst])\n"
        "    sub $1, %[count]\n"
        "    jnz 1b\n"
        "    vzeroupper\n"
        : [in] "+r"(in), [out] "+r"(out), [table] "+r"(table), [count] "+r"(count), [in_stride] "+r"(in_stride), [out_stride] "+r"(out_stride), [src] "=&r"(src), [dst] "=&r"(dst), [a] "=&r"(a), [b] "=&r"(b), [consts] "+r"(consts)
        :
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "xmm16", "xmm17", "cc", "memory");
}

static const float onefft_avx512_f16_consts[7] ONEFFT_ALIGNED(64) = {
    0x1.0000000000000p+1f, 0x1.975f5e0553158p-3f, 0x1.a827999fcef32p-2f, 0x1.561b82ab7f990p-1f,
    0x1.87de2a6aea963p-2f, 0x1.6a09e667f3bccp-1f, 0x1.d906bcf328d46p-1f,
};
/* First pass, radix 16. For each of the `count` entries (a, b) of `table`: reads 16 interleaved
 * vectors from in + 8*a + u*in_stride, transforms across them and writes output q as a split
 * vector to out + 8*b + r*in_stride, where q = low_bits(r). (The strides of input and output are
 * the same.) */
__attribute__((target("avx512f"))) static void onefft_avx512_f16(const float *in, float *out, const uint32_t *table, size_t count, size_t in_stride, size_t out_stride) {
    (void)out_stride;
    float *p0;
    float *p1;
    float *p2;
    float *p3;
    size_t stride3;
    size_t b;
    const float *consts = onefft_avx512_f16_consts;
    __asm__ volatile(
        "    lea (%[in_stride],%[in_stride],2), %[stride3]\n"
        "1:\n"
        "    movl (%[table]), %k[p0]\n"
        "    movl 4(%[table]), %k[b]\n"
        "    lea 8(%[table]), %[table]\n"
        "    lea (%[in],%[p0],8), %[p0]\n"
        "    vbroadcastss (%[consts]), %%zmm0\n"
        "    lea (%[p0],%[in_stride],1), %[p1]\n"
        "    lea (%[p0],%[in_stride],2), %[p2]\n"
        "    lea (%[p0],%[stride3],1), %[p3]\n"
        "    vmovups (%[p0]), %%zmm1\n"
        "    vmovups (%[p0],%[in_stride],4), %%zmm2\n"
        "    vshufps $136, 64(%[p0]), %%zmm1, %%zmm3\n"
        "    vshufps $221, 64(%[p0]), %%zmm1, %%zmm1\n"
        "    vmovups (%[p0],%[in_stride],8), %%zmm4\n"
        "    vshufps $136, 64(%[p0],%[in_stride],4), %%zmm2, %%zmm5\n"
        "    vshufps $221, 64(%[p0],%[in_stride],4), %%zmm2, %%zmm2\n"
        "    vmovups (%[p0],%[stride3],4), %%zmm6\n"
        "    vshufps $136, 64(%[p0],%[in_stride],8), %%zmm4, %%zmm7\n"
        "    vshufps $221, 64(%[p0],%[in_stride],8), %%zmm4, %%zmm4\n"
        "    vshufps $136, 64(%[p0],%[stride3],4), %%zmm6, %%zmm8\n"
        "    vshufps $221, 64(%[p0],%[stride3],4), %%zmm6, %%zmm6\n"
        "    vaddps %%zmm7, %%zmm3, %%zmm3\n"
        "    vaddps %%zmm4, %%zmm1, %%zmm1\n"
        "    vaddps %%zmm8, %%zmm5, %%zmm5\n"
        "    vaddps %%zmm6, %%zmm2, %%zmm2\n"
        "    vfnmadd132ps %%zmm0, %%zmm3, %%zmm7\n"
        "    vfnmadd132ps %%zmm0, %%zmm1, %%zmm4\n"
        "    vfnmadd132ps %%zmm0, %%zmm5, %%zmm8\n"
        "    vfnmadd132ps %%zmm0, %%zmm2, %%zmm6\n"
        "    vaddps %%zmm5, %%zmm3, %%zmm3\n"
        "    vaddps %%zmm2, %%zmm1, %%zmm1\n"
        "    vaddps %%zmm6, %%zmm7, %%zmm7\n"
        "    vsubps %%zmm8, %%zmm4, %%zmm4\n"
        "    vfnmadd132ps %%zmm0, %%zmm3, %%zmm5\n"
        "    vfnmadd132ps %%zmm0, %%zmm1, %%zmm2\n"
        "    vmovups (%[p0],%[in_stride],1), %%zmm9\n"
        "    vfnmadd132ps %%zmm0, %%zmm7, %%zmm6\n"
        "    vfmadd132ps %%zmm0, %%zmm4, %%zmm8\n"
        "    vmovups (%[p1],%[in_stride],4), %%zmm10\n"
        "    vshufps $136, 64(%[p0],%[in_stride],1), %%zmm9, %%zmm11\n"
        "    vshufps $221, 64(%[p0],%[in_stride],1), %%zmm9, %%zmm9\n"
        "    vmovups (%[p1],%[in_stride],8), %%zmm12\n"
        "    vshufps $136, 64(%[p1],%[in_stride],4), %%zmm10, %%zmm13\n"
        "    vshufps $221, 64(%[p1],%[in_stride],4), %%zmm10, %%zmm10\n"
        "    vmovups (%[p1],%[stride3],4), %%zmm14\n"
        "    vshufps $136, 64(%[p1],%[in_stride],8), %%zmm12, %%zmm15\n"
        "    vshufps $221, 64(%[p1],%[in_stride],8), %%zmm12, %%zmm12\n"
        "    vaddps %%zmm15, %%zmm11, %%zmm11\n"
        "    vaddps %%zmm12, %%zmm9, %%zmm9\n"
        "    vshufps $136, 64(%[p1],%[stride3],4), %%zmm14, %%zmm16\n"
        "    vshufps $221, 64(%[p1],%[stride3],4), %%zmm14, %%zmm14\n"
        "    vaddps %%zmm16, %%zmm13, %%zmm13\n"
        "    vaddps %%zmm14, %%zmm10, %%zmm10\n"
        "    vfnmadd132ps %%zmm0, %%zmm11, %%zmm15\n"
        "    vfnmadd132ps %%zmm0, %%zmm9, %%zmm12\n"
        "    vfnmadd132ps %%zmm0, %%zmm13, %%zmm16\n"
        "    vfnmadd132ps %%zmm0, %%zmm10, %%zmm14\n"
        "    vaddps %%zmm13, %%zmm11, %%zmm11\n"
        "    vaddps %%zmm10, %%zmm9, %%zmm9\n"
        "    vfnmadd132ps %%zmm0, %%zmm11, %%zmm13\n"
        "    vaddps %%zmm14, %%zmm15, %%zmm15\n"
        "    vsubps %%zmm16, %%zmm12, %%zmm12\n"
        "    vfnmadd132ps %%zmm0, %%zmm9, %%zmm10\n"
        "    vmovups (%[p0],%[in_stride],2), %%zmm17\n"
        "    vfnmadd132ps %%zmm0, %%zmm15, %%zmm14\n"
        "    vfmadd132ps %%zmm0, %%zmm12, %%zmm16\n"
        "    vmovups (%[p0],%[stride3],2), %%zmm18\n"
        "    vshufps $136, 64(%[p0],%[in_stride],2), %%zmm17, %%zmm19\n"
        "    vshufps $221, 64(%[p0],%[in_stride],2), %%zmm17, %%zmm17\n"
        "    vmovups (%[p2],%[in_stride],8), %%zmm20\n"
        "    vshufps $136, 64(%[p0],%[stride3],2), %%zmm18, %%zmm21\n"
        "    vshufps $221, 64(%[p0],%[stride3],2), %%zmm18, %%zmm18\n"
        "    vmovups (%[p2],%[stride3],4), %%zmm22\n"
        "    vshufps $136, 64(%[p2],%[in_stride],8), %%zmm20, %%zmm23\n"
        "    vshufps $221, 64(%[p2],%[in_stride],8), %%zmm20, %%zmm20\n"
        "    vaddps %%zmm23, %%zmm19, %%zmm19\n"
        "    vaddps %%zmm20, %%zmm17, %%zmm17\n"
        "    vshufps $136, 64(%[p2],%[stride3],4), %%zmm22, %%zmm24\n"
        "    vshufps $221, 64(%[p2],%[stride3],4), %%zmm22, %%zmm22\n"
        "    vaddps %%zmm24, %%zmm21, %%zmm21\n"
        "    vaddps %%zmm22, %%zmm18, %%zmm18\n"
        "    vfnmadd132ps %%zmm0, %%zmm19, %%zmm23\n"
        "    vfnmadd132ps %%zmm0, %%zmm17, %%zmm20\n"
        "    vfnmadd132ps %%zmm0, %%zmm21, %%zmm24\n"
        "    vfnmadd132ps %%zmm0, %%zmm18, %%zmm22\n"
        "    vaddps %%zmm21, %%zmm19, %%zmm19\n"
        "    vaddps %%zmm18, %%zmm17, %%zmm17\n"
        "    vaddps %%zmm22, %%zmm23, %%zmm23\n"
        "    vsubps %%zmm24, %%zmm20, %%zmm20\n"
        "    vfnmadd132ps %%zmm0, %%zmm19, %%zmm21\n"
        "    vfnmadd132ps %%zmm0, %%zmm17, %%zmm18\n"
        "    vmovups (%[p0],%[stride3],1), %%zmm25\n"
        "    vfnmadd132ps %%zmm0, %%zmm23, %%zmm22\n"
        "    vfmadd132ps %%zmm0, %%zmm20, %%zmm24\n"
        "    vmovups (%[p1],%[stride3],2), %%zmm0\n"
        "    vshufps $136, 64(%[p0],%[stride3],1), %%zmm25, %%zmm26\n"
        "    vshufps $221, 64(%[p0],%[stride3],1), %%zmm25, %%zmm25\n"
        "    vmovups (%[p3],%[in_stride],8), %%zmm27\n"
        "    vshufps $136, 64(%[p1],%[stride3],2), %%zmm0, %%zmm28\n"
        "    vshufps $221, 64(%[p1],%[stride3],2), %%zmm0, %%zmm0\n"
        "    vmovups (%[p3],%[stride3],4), %%zmm29\n"
        "    vshufps $136, 64(%[p3],%[in_stride],8), %%zmm27, %%zmm30\n"
        "    vshufps $221, 64(%[p3],%[in_stride],8), %%zmm27, %%zmm27\n"
        "    vaddps %%zmm30, %%zmm26, %%zmm26\n"
        "    vaddps %%zmm27, %%zmm25, %%zmm25\n"
        "    vshufps $136, 64(%[p3],%[stride3],4), %%zmm29, %%zmm31\n"
        "    vshufps $221, 64(%[p3],%[stride3],4), %%zmm29, %%zmm29\n"
        "    vaddps %%zmm31, %%zmm28, %%zmm28\n"
        "    vaddps %%zmm29, %%zmm0, %%zmm0\n"
        "    vfnmadd132ps (%[consts])%{1to16%}, %%zmm26, %%zmm30\n"
        "    vfnmadd132ps (%[consts])%{1to16%}, %%zmm25, %%zmm27\n"
        "    vfnmadd132ps (%[consts])%{1to16%}, %%zmm28, %%zmm31\n"
        "    vfnmadd132ps (%[consts])%{1to16%}, %%zmm0, %%zmm29\n"
        "    vaddps %%zmm28, %%zmm26, %%zmm26\n"
        "    vaddps %%zmm0, %%zmm25, %%zmm25\n"
        "    vfnmadd132ps (%[consts])%{1to16%}, %%zmm26, %%zmm28\n"
        "    vaddps %%zmm29, %%zmm30, %%zmm30\n"
        "    vsubps %%zmm31, %%zmm27, %%zmm27\n"
        "    vfnmadd132ps (%[consts])%{1to16%}, %%zmm25, %%zmm0\n"
        "    lea (%[out],%[b],8), %[p0]\n"
        "    lea (%[p0],%[in_stride],1), %[p1]\n"
        "    lea (%[p0],%[in_stride],2), %[p2]\n"
        "    vfnmadd132ps (%[consts])%{1to16%}, %%zmm30, %%zmm29\n"
        "    vfmadd132ps (%[consts])%{1to16%}, %%zmm27, %%zmm31\n"
        "    vaddps %%zmm19, %%zmm3, %%zmm3\n"
        "    vaddps %%zmm17, %%zmm1, %%zmm1\n"
        "    lea (%[p0],%[stride3],1), %[p3]\n"
        "    vaddps %%zmm26, %%zmm11, %%zmm11\n"
        "    vaddps %%zmm25, %%zmm9, %%zmm9\n"
        "    vfnmadd132ps (%[consts])%{1to16%}, %%zmm3, %%zmm19\n"
        "    vfnmadd132ps (%[consts])%{1to16%}, %%zmm1, %%zmm17\n"
        "    vfnmadd132ps (%[consts])%{1to16%}, %%zmm11, %%zmm26\n"
        "    vfnmadd132ps (%[consts])%{1to16%}, %%zmm9, %%zmm25\n"
        "    vaddps %%zmm11, %%zmm3, %%zmm3\n"
        "    vaddps %%zmm9, %%zmm1, %%zmm1\n"
        "    vaddps %%zmm25, %%zmm19, %%zmm19\n"
        "    vsubps %%zmm26, %%zmm17, %%zmm17\n"
        "    vfnmadd132ps (%[consts])%{1to16%}, %%zmm3, %%zmm11\n"
        "    vfnmadd132ps (%[consts])%{1to16%}, %%zmm1, %%zmm9\n"
        "    vmovups %%zmm3, (%[p0])\n"
        "    vmovups %%zmm1, 64(%[p0])\n"
        "    vfnmadd132ps (%[consts])%{1to16%}, %%zmm19, %%zmm25\n"
        "    vfmadd132ps (%[consts])%{1to16%}, %%zmm17, %%zmm26\n"
        "    vmovups %%zmm19, (%[p0],%[in_stride],4)\n"
        "    vmovups %%zmm17, 64(%[p0],%[in_stride],4)\n"
        "    vmovups %%zmm11, (%[p0],%[in_stride],1)\n"
        "    vmovups %%zmm9, 64(%[p0],%[in_stride],1)\n"
        "    vbroadcastss 4(%[consts]), %%zmm1\n"
        "    vbroadcastss 16(%[consts]), %%zmm3\n"
        "    vbroadcastss (%[consts]), %%zmm9\n"
        "    vmovups %%zmm25, (%[p1],%[in_stride],4)\n"
        "    vmovups %%zmm26, 64(%[p1],%[in_stride],4)\n"
        "    vbroadcastss 8(%[consts]), %%zmm11\n"
        "    vbroadcastss 12(%[consts]), %%zmm17\n"
        "    vbroadcastss 20(%[consts]), %%zmm19\n"
        "    vbroadcastss 24(%[consts]), %%zmm25\n"
        "    vfmadd231ps %%zmm1, %%zmm12, %%zmm15\n"
        "    vfmadd231ps %%zmm11, %%zmm20, %%zmm23\n"
        "    vfnmadd231ps %%zmm3, %%zmm15, %%zmm12\n"
        "    vfmadd231ps %%zmm17, %%zmm27, %%zmm30\n"
        "    vfnmadd231ps %%zmm19, %%zmm23, %%zmm20\n"
        "    vfmadd231ps %%zmm1, %%zmm12, %%zmm15\n"
        "    vfnmadd231ps %%zmm25, %%zmm30, %%zmm27\n"
        "    vfmadd231ps %%zmm11, %%zmm20, %%zmm23\n"
        "    vaddps %%zmm20, %%zmm4, %%zmm4\n"
        "    vfmadd231ps %%zmm17, %%zmm27, %%zmm30\n"
        "    vaddps %%zmm23, %%zmm7, %%zmm7\n"
        "    vaddps %%zmm27, %%zmm12, %%zmm12\n"
        "    vfnmadd132ps %%zmm9, %%zmm4, %%zmm20\n"
        "    vaddps %%zmm30, %%zmm15, %%zmm15\n"
        "    vfnmadd132ps %%zmm9, %%zmm7, %%zmm23\n"
        "    vfnmadd132ps %%zmm9, %%zmm12, %%zmm27\n"
        "    vaddps %%zmm12, %%zmm4, %%zmm4\n"
        "    vfnmadd132ps %%zmm9, %%zmm15, %%zmm30\n"
        "    vaddps %%zmm15, %%zmm7, %%zmm7\n"
        "    vaddps %%zmm27, %%zmm23, %%zmm23\n"
        "    vfnmadd132ps %%zmm9, %%zmm4, %%zmm12\n"
        "    vsubps %%zmm30, %%zmm20, %%zmm20\n"
        "    vfnmadd132ps %%zmm9, %%zmm7, %%zmm15\n"
        "    vmovups %%zmm7, (%[p0],%[in_stride],8)\n"
        "    vmovups %%zmm4, 64(%[p0],%[in_stride],8)\n"
        "    vfnmadd132ps %%zmm9, %%zmm23, %%zmm27\n"
        "    vmovups %%zmm23, (%[p0],%[stride3],4)\n"
        "    vfmadd132ps %%zmm9, %%zmm20, %%zmm30\n"
        "    vmovups %%zmm20, 64(%[p0],%[stride3],4)\n"
        "    vfmadd231ps %%zmm11, %%zmm10, %%zmm13\n"
        "    vmovups %%zmm15, (%[p1],%[in_stride],8)\n"
        "    vmovups %%zmm12, 64(%[p1],%[in_stride],8)\n"
        "    vmovups %%zmm27, (%[p1],%[stride3],4)\n"
        "    vfmadd231ps %%zmm11, %%zmm0, %%zmm28\n"
        "    vmovups %%zmm30, 64(%[p1],%[stride3],4)\n"
        "    vfnmadd231ps %%zmm19, %%zmm13, %%zmm10\n"
        "    vaddps %%zmm18, %%zmm5, %%zmm5\n"
        "    vfnmadd231ps %%zmm19, %%zmm28, %%zmm0\n"
        "    vfmadd231ps %%zmm11, %%zmm10, %%zmm13\n"
        "    vsubps %%zmm21, %%zmm2, %%zmm2\n"
        "    vfnmadd132ps %%zmm9, %%zmm5, %%zmm18\n"
        "    vfmadd231ps %%zmm11, %%zmm0, %%zmm28\n"
        "    vaddps %%zmm0, %%zmm13, %%zmm13\n"
        "    vfmadd132ps %%zmm9, %%zmm2, %%zmm21\n"
        "    vsubps %%zmm28, %%zmm10, %%zmm10\n"
        "    vfnmadd132ps %%zmm9, %%zmm13, %%zmm0\n"
        "    vaddps %%zmm13, %%zmm5, %%zmm5\n"
        "    vfmadd132ps %%zmm9, %%zmm10, %%zmm28\n"
        "    vaddps %%zmm10, %%zmm2, %%zmm2\n"
        "    vsubps %%zmm0, %%zmm21, %%zmm21\n"
        "    vfnmadd132ps %%zmm9, %%zmm5, %%zmm13\n"
        "    vaddps %%zmm28, %%zmm18, %%zmm18\n"
        "    vfnmadd132ps %%zmm9, %%zmm2, %%zmm10\n"
        "    vfmadd132ps %%zmm9, %%zmm21, %%zmm0\n"
        "    vmovups %%zmm5, (%[p0],%[in_stride],2)\n"
        "    vmovups %%zmm2, 64(%[p0],%[in_stride],2)\n"
        "    vfnmadd132ps %%zmm9, %%zmm18, %%zmm28\n"
        "    vmovups %%zmm18, (%[p0],%[stride3],2)\n"
        "    vmovups %%zmm21, 64(%[p0],%[stride3],2)\n"
        "    vmovups %%zmm13, (%[p0],%[stride3],1)\n"
        "    vmovups %%zmm10, 64(%[p0],%[stride3],1)\n"
        "    vmovups %%zmm0, 64(%[p1],%[stride3],2)\n"
        "    vfmadd231ps %%zmm17, %%zmm16, %%zmm14\n"
        "    vmovups %%zmm28, (%[p1],%[stride3],2)\n"
        "    vfmadd231ps %%zmm11, %%zmm24, %%zmm22\n"
        "    vfnmadd231ps %%zmm25, %%zmm14, %%zmm16\n"
        "    vfmadd231ps %%zmm1, %%zmm31, %%zmm29\n"
        "    vfnmadd231ps %%zmm19, %%zmm22, %%zmm24\n"
        "    vfmadd231ps %%zmm17, %%zmm16, %%zmm14\n"
        "    vfnmadd231ps %%zmm3, %%zmm29, %%zmm31\n"
        "    vfmadd231ps %%zmm11, %%zmm24, %%zmm22\n"
        "    vaddps %%zmm24, %%zmm6, %%zmm0\n"
        "    vsubps %%zmm24, %%zmm6, %%zmm2\n"
        "    vfmadd231ps %%zmm1, %%zmm31, %%zmm29\n"
        "    vsubps %%zmm31, %%zmm16, %%zmm1\n"
        "    vsubps %%zmm22, %%zmm8, %%zmm3\n"
        "    vaddps %%zmm22, %%zmm8, %%zmm4\n"
        "    vaddps %%zmm31, %%zmm16, %%zmm5\n"
        "    vsubps %%zmm29, %%zmm14, %%zmm6\n"
        "    vaddps %%zmm29, %%zmm14, %%zmm7\n"
        "    vaddps %%zmm1, %%zmm3, %%zmm8\n"
        "    vsubps %%zmm1, %%zmm3, %%zmm1\n"
        "    vaddps %%zmm5, %%zmm2, %%zmm3\n"
        "    vaddps %%zmm6, %%zmm0, %%zmm9\n"
        "    vsubps %%zmm6, %%zmm0, %%zmm0\n"
        "    vsubps %%zmm7, %%zmm4, %%zmm6\n"
        "    vsubps %%zmm5, %%zmm2, %%zmm2\n"
        "    vaddps %%zmm7, %%zmm4, %%zmm4\n"
        "    vmovups %%zmm8, 64(%[p2],%[in_stride],8)\n"
        "    vmovups %%zmm3, (%[p2],%[stride3],4)\n"
        "    vmovups %%zmm9, (%[p2],%[in_stride],8)\n"
        "    vmovups %%zmm0, (%[p3],%[in_stride],8)\n"
        "    vmovups %%zmm6, 64(%[p2],%[stride3],4)\n"
        "    vmovups %%zmm1, 64(%[p3],%[in_stride],8)\n"
        "    vmovups %%zmm2, (%[p3],%[stride3],4)\n"
        "    vmovups %%zmm4, 64(%[p3],%[stride3],4)\n"
        "    sub $1, %[count]\n"
        "    jnz 1b\n"
        "    vzeroupper\n"
        : [in] "+r"(in), [out] "+r"(out), [table] "+r"(table), [count] "+r"(count), [in_stride] "+r"(in_stride), [p0] "=&r"(p0), [p1] "=&r"(p1), [p2] "=&r"(p2), [p3] "=&r"(p3), [stride3] "=&r"(stride3), [b] "=&r"(b), [consts] "+r"(consts)
        :
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "xmm16", "xmm17", "xmm18", "xmm19", "xmm20", "xmm21", "xmm22", "xmm23", "xmm24", "xmm25", "xmm26", "xmm27", "xmm28", "xmm29", "xmm30", "xmm31", "cc", "memory");
}

/* Last pass, in place: reads split vectors and writes the interleaved result. Butterfly
 * j < count uses the vectors at data + r*stride + 128*j. Index bits 1 and 0 are moved from the lanes
 * to the registers and transformed. The table holds per butterfly, for l = 1..3, the real and the
 * imaginary parts of the 16 twiddles of the vector with index bits (1, 0) = l. */
__attribute__((target("avx512f"))) static void onefft_avx512_lb(float *data, const float *tw, size_t stride, size_t count, size_t regions) {
    (void)regions;
    float *third;
    __asm__ volatile(
        "    lea (%[data],%[stride],2), %[third]\n"
        "    vmovups (%[data]), %%zmm0\n"
        "    vmovups 64(%[data]), %%zmm1\n"
        "    vmovups (%[data],%[stride],1), %%zmm2\n"
        "    vmovups 64(%[data],%[stride],1), %%zmm3\n"
        "    vmovups (%[data],%[stride],2), %%zmm4\n"
        "    vmovups 64(%[data],%[stride],2), %%zmm5\n"
        "    lea (%[third],%[stride],1), %[third]\n"
        "    vmovups (%[third]), %%zmm6\n"
        "    vmovups 64(%[third]), %%zmm7\n"
        "    vmovups (%[tw]), %%zmm8\n"
        "    vshuff32x4 $136, %%zmm2, %%zmm0, %%zmm9\n"
        "    vshuff32x4 $221, %%zmm2, %%zmm0, %%zmm0\n"
        "    vshuff32x4 $136, %%zmm3, %%zmm1, %%zmm2\n"
        "    vshuff32x4 $221, %%zmm3, %%zmm1, %%zmm1\n"
        "    vmovups 64(%[tw]), %%zmm3\n"
        "    vshuff32x4 $136, %%zmm6, %%zmm4, %%zmm10\n"
        "    vshuff32x4 $221, %%zmm6, %%zmm4, %%zmm4\n"
        "    vshuff32x4 $136, %%zmm7, %%zmm5, %%zmm6\n"
        "    vmovups 128(%[tw]), %%zmm11\n"
        "    vshuff32x4 $221, %%zmm7, %%zmm5, %%zmm5\n"
        "    vunpckhps %%zmm10, %%zmm9, %%zmm7\n"
        "    vunpckhps %%zmm6, %%zmm2, %%zmm12\n"
        "    vmovups 192(%[tw]), %%zmm13\n"
        "    vunpcklps %%zmm4, %%zmm0, %%zmm14\n"
        "    vunpckhps %%zmm4, %%zmm0, %%zmm0\n"
        "    vunpcklps %%zmm5, %%zmm1, %%zmm4\n"
        "    vunpckhps %%zmm5, %%zmm1, %%zmm1\n"
        "    vmulps %%zmm8, %%zmm7, %%zmm5\n"
        "    vmulps %%zmm8, %%zmm12, %%zmm8\n"
        "    vunpcklps %%zmm10, %%zmm9, %%zmm9\n"
        "    vunpcklps %%zmm6, %%zmm2, %%zmm2\n"
        "    vmovups 256(%[tw]), %%zmm6\n"
        "    vmovups 320(%[tw]), %%zmm10\n"
        "    vmulps %%zmm11, %%zmm14, %%zmm15\n"
        "    vmulps %%zmm11, %%zmm4, %%zmm11\n"
        "    lea 384(%[tw]), %[tw]\n"
        "    vfnmadd231ps %%zmm3, %%zmm12, %%zmm5\n"
        "    vfmadd231ps %%zmm3, %%zmm7, %%zmm8\n"
        "    vmulps %%zmm6, %%zmm0, %%zmm3\n"
        "    vmulps %%zmm6, %%zmm1, %%zmm6\n"
        "    vfnmadd231ps %%zmm13, %%zmm4, %%zmm15\n"
        "    vfmadd231ps %%zmm13, %%zmm14, %%zmm11\n"
        "    vfnmadd231ps %%zmm10, %%zmm1, %%zmm3\n"
        "    vfmadd231ps %%zmm10, %%zmm0, %%zmm6\n"
        "    sub $1, %[count]\n"
        "    jz 1f\n"
        "2:\n"
        "    lea 128(%[data]), %[data]\n"
        "    lea 128(%[third]), %[third]\n"
        "    vmovups (%[data]), %%zmm0\n"
        "    vmovups 64(%[data]), %%zmm1\n"
        "    vmovups (%[data],%[stride],1), %%zmm4\n"
        "    vmovups 64(%[data],%[stride],1), %%zmm7\n"
        "    vmovups (%[data],%[stride],2), %%zmm10\n"
        "    vmovups 64(%[data],%[stride],2), %%zmm12\n"
        "    vmovups (%[third]), %%zmm13\n"
        "    vmovups 64(%[third]), %%zmm14\n"
        "    vmovups (%[tw]), %%zmm16\n"
        "    vshuff32x4 $136, %%zmm4, %%zmm0, %%zmm17\n"
        "    vshuff32x4 $221, %%zmm4, %%zmm0, %%zmm0\n"
        "    vshuff32x4 $136, %%zmm7, %%zmm1, %%zmm4\n"
        "    vshuff32x4 $221, %%zmm7, %%zmm1, %%zmm1\n"
        "    vshuff32x4 $136, %%zmm13, %%zmm10, %%zmm7\n"
        "    vshuff32x4 $221, %%zmm13, %%zmm10, %%zmm10\n"
        "    vmovups 64(%[tw]), %%zmm13\n"
        "    vmovups 128(%[tw]), %%zmm18\n"
        "    vshuff32x4 $136, %%zmm14, %%zmm12, %%zmm19\n"
        "    vshuff32x4 $221, %%zmm14, %%zmm12, %%zmm12\n"
        "    vunpckhps %%zmm7, %%zmm17, %%zmm14\n"
        "    vunpckhps %%zmm19, %%zmm4, %%zmm20\n"
        "    vunpcklps %%zmm10, %%zmm0, %%zmm21\n"
        "    vunpckhps %%zmm10, %%zmm0, %%zmm0\n"
        "    vmovups 192(%[tw]), %%zmm10\n"
        "    vunpcklps %%zmm12, %%zmm1, %%zmm22\n"
        "    vunpckhps %%zmm12, %%zmm1, %%zmm1\n"
        "    vmulps %%zmm16, %%zmm14, %%zmm12\n"
        "    vmulps %%zmm16, %%zmm20, %%zmm16\n"
        "    vunpcklps %%zmm7, %%zmm17, %%zmm7\n"
        "    vunpcklps %%zmm19, %%zmm4, %%zmm4\n"
        "    vmovups 256(%[tw]), %%zmm17\n"
        "    vmovups 320(%[tw]), %%zmm19\n"
        "    vaddps %%zmm15, %%zmm9, %%zmm23\n"
        "    vaddps %%zmm11, %%zmm2, %%zmm24\n"
        "    vmulps %%zmm18, %%zmm21, %%zmm25\n"
        "    vmulps %%zmm18, %%zmm22, %%zmm18\n"
        "    lea 384(%[tw]), %[tw]\n"
        "    vfnmadd231ps %%zmm13, %%zmm20, %%zmm12\n"
        "    vsubps %%zmm15, %%zmm9, %%zmm20\n"
        "    vfmadd231ps %%zmm13, %%zmm14, %%zmm16\n"
        "    vsubps %%zmm11, %%zmm2, %%zmm13\n"
        "    vaddps %%zmm3, %%zmm5, %%zmm14\n"
        "    vaddps %%zmm6, %%zmm8, %%zmm26\n"
        "    vsubps %%zmm3, %%zmm5, %%zmm27\n"
        "    vsubps %%zmm6, %%zmm8, %%zmm28\n"
        "    vmulps %%zmm17, %%zmm0, %%zmm29\n"
        "    vmulps %%zmm17, %%zmm1, %%zmm17\n"
        "    vfnmadd231ps %%zmm10, %%zmm22, %%zmm25\n"
        "    vfmadd231ps %%zmm10, %%zmm21, %%zmm18\n"
        "    vaddps %%zmm14, %%zmm23, %%zmm10\n"
        "    vaddps %%zmm26, %%zmm24, %%zmm21\n"
        "    vsubps %%zmm14, %%zmm23, %%zmm14\n"
        "    vsubps %%zmm26, %%zmm24, %%zmm22\n"
        "    vaddps %%zmm28, %%zmm20, %%zmm23\n"
        "    vsubps %%zmm27, %%zmm13, %%zmm24\n"
        "    vfnmadd231ps %%zmm19, %%zmm1, %%zmm29\n"
        "    vfmadd231ps %%zmm19, %%zmm0, %%zmm17\n"
        "    vsubps %%zmm28, %%zmm20, %%zmm0\n"
        "    vaddps %%zmm27, %%zmm13, %%zmm1\n"
        "    vunpcklps %%zmm21, %%zmm10, %%zmm13\n"
        "    vunpckhps %%zmm21, %%zmm10, %%zmm10\n"
        "    vmovaps %%zmm7, %%zmm9\n"
        "    vunpcklps %%zmm24, %%zmm23, %%zmm7\n"
        "    vunpckhps %%zmm24, %%zmm23, %%zmm19\n"
        "    vunpcklps %%zmm22, %%zmm14, %%zmm20\n"
        "    vmovups %%zmm13, -128(%[data])\n"
        "    vmovups %%zmm10, -64(%[data])\n"
        "    vmovaps %%zmm4, %%zmm2\n"
        "    vmovaps %%zmm12, %%zmm5\n"
        "    vunpckhps %%zmm22, %%zmm14, %%zmm4\n"
        "    vmovups %%zmm7, -128(%[data],%[stride],1)\n"
        "    vmovups %%zmm19, -64(%[data],%[stride],1)\n"
        "    vmovups %%zmm20, -128(%[data],%[stride],2)\n"
        "    vmovaps %%zmm16, %%zmm8\n"
        "    vmovaps %%zmm25, %%zmm15\n"
        "    vmovaps %%zmm18, %%zmm11\n"
        "    vmovaps %%zmm29, %%zmm3\n"
        "    vunpcklps %%zmm1, %%zmm0, %%zmm7\n"
        "    vunpckhps %%zmm1, %%zmm0, %%zmm0\n"
        "    vmovups %%zmm4, -64(%[data],%[stride],2)\n"
        "    vmovaps %%zmm17, %%zmm6\n"
        "    vmovups %%zmm7, -128(%[third])\n"
        "    vmovups %%zmm0, -64(%[third])\n"
        "    sub $1, %[count]\n"
        "    jnz 2b\n"
        "1:\n"
        "    vaddps %%zmm15, %%zmm9, %%zmm0\n"
        "    vaddps %%zmm11, %%zmm2, %%zmm1\n"
        "    vsubps %%zmm15, %%zmm9, %%zmm4\n"
        "    vsubps %%zmm11, %%zmm2, %%zmm2\n"
        "    vaddps %%zmm3, %%zmm5, %%zmm7\n"
        "    vaddps %%zmm6, %%zmm8, %%zmm9\n"
        "    vsubps %%zmm3, %%zmm5, %%zmm3\n"
        "    vsubps %%zmm6, %%zmm8, %%zmm5\n"
        "    vaddps %%zmm7, %%zmm0, %%zmm6\n"
        "    vaddps %%zmm9, %%zmm1, %%zmm8\n"
        "    vsubps %%zmm7, %%zmm0, %%zmm0\n"
        "    vsubps %%zmm9, %%zmm1, %%zmm1\n"
        "    vaddps %%zmm5, %%zmm4, %%zmm7\n"
        "    vsubps %%zmm3, %%zmm2, %%zmm9\n"
        "    vsubps %%zmm5, %%zmm4, %%zmm4\n"
        "    vaddps %%zmm3, %%zmm2, %%zmm2\n"
        "    vunpcklps %%zmm8, %%zmm6, %%zmm3\n"
        "    vunpckhps %%zmm8, %%zmm6, %%zmm5\n"
        "    vunpcklps %%zmm9, %%zmm7, %%zmm6\n"
        "    vunpckhps %%zmm9, %%zmm7, %%zmm7\n"
        "    vunpcklps %%zmm1, %%zmm0, %%zmm8\n"
        "    vmovups %%zmm3, (%[data])\n"
        "    vmovups %%zmm5, 64(%[data])\n"
        "    vunpckhps %%zmm1, %%zmm0, %%zmm0\n"
        "    vunpcklps %%zmm2, %%zmm4, %%zmm1\n"
        "    vunpckhps %%zmm2, %%zmm4, %%zmm2\n"
        "    vmovups %%zmm6, (%[data],%[stride],1)\n"
        "    vmovups %%zmm7, 64(%[data],%[stride],1)\n"
        "    vmovups %%zmm8, (%[data],%[stride],2)\n"
        "    vmovups %%zmm0, 64(%[data],%[stride],2)\n"
        "    vmovups %%zmm1, (%[third])\n"
        "    vmovups %%zmm2, 64(%[third])\n"
        "    vzeroupper\n"
        : [data] "+r"(data), [tw] "+r"(tw), [stride] "+r"(stride), [count] "+r"(count), [third] "=&r"(third)
        :
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "xmm16", "xmm17", "xmm18", "xmm19", "xmm20", "xmm21", "xmm22", "xmm23", "xmm24", "xmm25", "xmm26", "xmm27", "xmm28", "xmm29", "cc", "memory");
}

static const float onefft_avx512_n32_consts[144] ONEFFT_ALIGNED(64) = {
    0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f,
    0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f,
    0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f,
    0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f,
    0x1.0000000000000p+0f, -0x0.0p+0f, 0x1.0000000000000p+0f, -0x0.0p+0f,
    0x1.0000000000000p+0f, -0x0.0p+0f, 0x1.0000000000000p+0f, -0x0.0p+0f,
    0x1.0000000000000p+0f, -0x0.0p+0f, 0x1.0000000000000p+0f, -0x0.0p+0f,
    0x1.0000000000000p+0f, -0x0.0p+0f, 0x1.0000000000000p+0f, -0x0.0p+0f,
    0x0.0p+0f, -0x1.0000000000000p+0f, 0x0.0p+0f, -0x1.0000000000000p+0f,
    0x0.0p+0f, -0x1.0000000000000p+0f, 0x0.0p+0f, -0x1.0000000000000p+0f,
    0x0.0p+0f, -0x1.0000000000000p+0f, 0x0.0p+0f, -0x1.0000000000000p+0f,
    0x0.0p+0f, -0x1.0000000000000p+0f, 0x0.0p+0f, -0x1.0000000000000p+0f,
    0x1.0000000000000p+0f, 0x1.6a09e667f3bcdp-1f, 0x1.0000000000000p+0f, 0x1.6a09e667f3bcdp-1f,
    0x1.0000000000000p+0f, 0x1.6a09e667f3bcdp-1f, 0x1.0000000000000p+0f, 0x1.6a09e667f3bcdp-1f,
    -0x0.0p+0f, -0x1.6a09e667f3bccp-1f, -0x0.0p+0f, -0x1.6a09e667f3bccp-1f,
    -0x0.0p+0f, -0x1.6a09e667f3bccp-1f, -0x0.0p+0f, -0x1.6a09e667f3bccp-1f,
    0x0.0p+0f, -0x1.6a09e667f3bccp-1f, 0x0.0p+0f, -0x1.6a09e667f3bccp-1f,
    0x0.0p+0f, -0x1.6a09e667f3bccp-1f, 0x0.0p+0f, -0x1.6a09e667f3bccp-1f,
    -0x1.0000000000000p+0f, -0x1.6a09e667f3bcdp-1f, -0x1.0000000000000p+0f, -0x1.6a09e667f3bcdp-1f,
    -0x1.0000000000000p+0f, -0x1.6a09e667f3bcdp-1f, -0x1.0000000000000p+0f, -0x1.6a09e667f3bcdp-1f,
    0x1.0000000000000p+0f, 0x1.d906bcf328d46p-1f, 0x1.0000000000000p+0f, 0x1.d906bcf328d46p-1f,
    0x1.6a09e667f3bcdp-1f, 0x1.87de2a6aea964p-2f, 0x1.6a09e667f3bcdp-1f, 0x1.87de2a6aea964p-2f,
    -0x0.0p+0f, -0x1.87de2a6aea962p-2f, -0x0.0p+0f, -0x1.87de2a6aea962p-2f,
    -0x1.6a09e667f3bccp-1f, -0x1.d906bcf328d46p-1f, -0x1.6a09e667f3bccp-1f, -0x1.d906bcf328d46p-1f,
    0x0.0p+0f, -0x1.87de2a6aea963p-2f, 0x0.0p+0f, -0x1.87de2a6aea963p-2f,
    -0x1.6a09e667f3bccp-1f, -0x1.d906bcf328d46p-1f, -0x1.6a09e667f3bccp-1f, -0x1.d906bcf328d46p-1f,
    -0x1.0000000000000p+0f, -0x1.d906bcf328d46p-1f, -0x1.0000000000000p+0f, -0x1.d906bcf328d46p-1f,
    -0x1.6a09e667f3bcdp-1f, -0x1.87de2a6aea965p-2f, -0x1.6a09e667f3bcdp-1f, -0x1.87de2a6aea965p-2f,
    0x1.0000000000000p+0f, 0x1.f6297cff75cb0p-1f, -0x0.0p+0f, -0x1.8f8b83c69a608p-3f,
    0x1.d906bcf328d46p-1f, 0x1.a9b66290ea1a3p-1f, -0x1.87de2a6aea962p-2f, -0x1.1c73b39ae68c6p-1f,
    0x1.6a09e667f3bcdp-1f, 0x1.1c73b39ae68c9p-1f, -0x1.6a09e667f3bccp-1f, -0x1.a9b66290ea1a4p-1f,
    0x1.87de2a6aea964p-2f, 0x1.8f8b83c69a60dp-3f, -0x1.d906bcf328d46p-1f, -0x1.f6297cff75cb0p-1f,
    0x0.0p+0f, -0x1.8f8b83c69a60ap-3f, -0x1.0000000000000p+0f, -0x1.f6297cff75cb0p-1f,
    -0x1.87de2a6aea963p-2f, -0x1.1c73b39ae68c8p-1f, -0x1.d906bcf328d46p-1f, -0x1.a9b66290ea1a5p-1f,
    -0x1.6a09e667f3bccp-1f, -0x1.a9b66290ea1a3p-1f, -0x1.6a09e667f3bcdp-1f, -0x1.1c73b39ae68c8p-1f,
    -0x1.d906bcf328d46p-1f, -0x1.f6297cff75cb0p-1f, -0x1.87de2a6aea965p-2f, -0x1.8f8b83c69a617p-3f,
};
/* Complete 32 point transform from `in` to `out`, both interleaved. */
__attribute__((target("avx512f"))) static void onefft_avx512_n32(const float *in, float *out) {
    const float *consts = onefft_avx512_n32_consts;
    __asm__ volatile(
        "    vmovups (%[in]), %%zmm0\n"
        "    vmovups 128(%[in]), %%zmm1\n"
        "    vmovups 64(%[consts]), %%zmm2\n"
        "    vmovups 128(%[consts]), %%zmm3\n"
        "    vmovups 192(%[consts]), %%zmm4\n"
        "    vmovups 256(%[consts]), %%zmm5\n"
        "    vmovups 320(%[consts]), %%zmm6\n"
        "    vmovups 384(%[consts]), %%zmm7\n"
        "    vmovups 448(%[consts]), %%zmm8\n"
        "    vmovups 512(%[consts]), %%zmm9\n"
        "    vshufps $136, 64(%[in]), %%zmm0, %%zmm10\n"
        "    vshufps $221, 64(%[in]), %%zmm0, %%zmm0\n"
        "    vshufps $136, 192(%[in]), %%zmm1, %%zmm11\n"
        "    vshufps $221, 192(%[in]), %%zmm1, %%zmm1\n"
        "    vaddps %%zmm11, %%zmm10, %%zmm12\n"
        "    vsubps %%zmm11, %%zmm10, %%zmm10\n"
        "    vaddps %%zmm1, %%zmm0, %%zmm11\n"
        "    vsubps %%zmm1, %%zmm0, %%zmm0\n"
        "    vunpckhps %%zmm10, %%zmm12, %%zmm1\n"
        "    vunpcklps %%zmm10, %%zmm12, %%zmm10\n"
        "    vunpckhps %%zmm0, %%zmm11, %%zmm12\n"
        "    vmulps %%zmm2, %%zmm1, %%zmm13\n"
        "    vunpcklps %%zmm0, %%zmm11, %%zmm0\n"
        "    vmulps %%zmm2, %%zmm12, %%zmm2\n"
        "    vfnmadd231ps %%zmm3, %%zmm12, %%zmm13\n"
        "    vfmadd231ps %%zmm3, %%zmm1, %%zmm2\n"
        "    vaddps %%zmm13, %%zmm10, %%zmm1\n"
        "    vsubps %%zmm13, %%zmm10, %%zmm3\n"
        "    vaddps %%zmm2, %%zmm0, %%zmm10\n"
        "    vsubps %%zmm2, %%zmm0, %%zmm0\n"
        "    vshuff32x4 $238, %%zmm3, %%zmm1, %%zmm2\n"
        "    vshuff32x4 $68, %%zmm3, %%zmm1, %%zmm1\n"
        "    vshuff32x4 $238, %%zmm0, %%zmm10, %%zmm3\n"
        "    vmulps %%zmm4, %%zmm2, %%zmm11\n"
        "    vshuff32x4 $68, %%zmm0, %%zmm10, %%zmm0\n"
        "    vmulps %%zmm4, %%zmm3, %%zmm4\n"
        "    vfnmadd231ps %%zmm5, %%zmm3, %%zmm11\n"
        "    vfmadd231ps %%zmm5, %%zmm2, %%zmm4\n"
        "    vaddps %%zmm11, %%zmm1, %%zmm2\n"
        "    vsubps %%zmm11, %%zmm1, %%zmm1\n"
        "    vaddps %%zmm4, %%zmm0, %%zmm3\n"
        "    vsubps %%zmm4, %%zmm0, %%zmm0\n"
        "    vshuff32x4 $221, %%zmm1, %%zmm2, %%zmm4\n"
        "    vshuff32x4 $136, %%zmm1, %%zmm2, %%zmm1\n"
        "    vshuff32x4 $221, %%zmm0, %%zmm3, %%zmm2\n"
        "    vmulps %%zmm6, %%zmm4, %%zmm5\n"
        "    vshuff32x4 $136, %%zmm0, %%zmm3, %%zmm0\n"
        "    vmulps %%zmm6, %%zmm2, %%zmm3\n"
        "    vfnmadd231ps %%zmm7, %%zmm2, %%zmm5\n"
        "    vfmadd231ps %%zmm7, %%zmm4, %%zmm3\n"
        "    vaddps %%zmm5, %%zmm1, %%zmm2\n"
        "    vsubps %%zmm5, %%zmm1, %%zmm1\n"
        "    vaddps %%zmm3, %%zmm0, %%zmm4\n"
        "    vsubps %%zmm3, %%zmm0, %%zmm0\n"
        "    vunpckhpd %%zmm1, %%zmm2, %%zmm3\n"
        "    vunpcklpd %%zmm1, %%zmm2, %%zmm1\n"
        "    vunpckhpd %%zmm0, %%zmm4, %%zmm2\n"
        "    vmulps %%zmm8, %%zmm3, %%zmm5\n"
        "    vunpcklpd %%zmm0, %%zmm4, %%zmm0\n"
        "    vmulps %%zmm8, %%zmm2, %%zmm4\n"
        "    vfnmadd231ps %%zmm9, %%zmm2, %%zmm5\n"
        "    vfmadd231ps %%zmm9, %%zmm3, %%zmm4\n"
        "    vaddps %%zmm5, %%zmm1, %%zmm2\n"
        "    vsubps %%zmm5, %%zmm1, %%zmm1\n"
        "    vaddps %%zmm4, %%zmm0, %%zmm3\n"
        "    vsubps %%zmm4, %%zmm0, %%zmm0\n"
        "    vunpcklps %%zmm3, %%zmm2, %%zmm4\n"
        "    vunpckhps %%zmm3, %%zmm2, %%zmm2\n"
        "    vunpcklps %%zmm0, %%zmm1, %%zmm3\n"
        "    vunpckhps %%zmm0, %%zmm1, %%zmm0\n"
        "    vmovups %%zmm4, (%[out])\n"
        "    vmovups %%zmm2, 64(%[out])\n"
        "    vmovups %%zmm3, 128(%[out])\n"
        "    vmovups %%zmm0, 192(%[out])\n"
        "    vzeroupper\n"
        : [in] "+r"(in), [out] "+r"(out), [consts] "+r"(consts)
        :
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "cc", "memory");
}

static const float onefft_avx512_n64_consts[144] ONEFFT_ALIGNED(64) = {
    0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f,
    0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f,
    0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f,
    0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f,
    0x1.0000000000000p+0f, 0x1.6a09e667f3bcdp-1f, 0x1.0000000000000p+0f, 0x1.6a09e667f3bcdp-1f,
    0x1.0000000000000p+0f, 0x1.6a09e667f3bcdp-1f, 0x1.0000000000000p+0f, 0x1.6a09e667f3bcdp-1f,
    0x1.0000000000000p+0f, 0x1.6a09e667f3bcdp-1f, 0x1.0000000000000p+0f, 0x1.6a09e667f3bcdp-1f,
    0x1.0000000000000p+0f, 0x1.6a09e667f3bcdp-1f, 0x1.0000000000000p+0f, 0x1.6a09e667f3bcdp-1f,
    0x0.0p+0f, -0x1.6a09e667f3bccp-1f, 0x0.0p+0f, -0x1.6a09e667f3bccp-1f,
    0x0.0p+0f, -0x1.6a09e667f3bccp-1f, 0x0.0p+0f, -0x1.6a09e667f3bccp-1f,
    0x0.0p+0f, -0x1.6a09e667f3bccp-1f, 0x0.0p+0f, -0x1.6a09e667f3bccp-1f,
    0x0.0p+0f, -0x1.6a09e667f3bccp-1f, 0x0.0p+0f, -0x1.6a09e667f3bccp-1f,
    0x1.0000000000000p+0f, 0x1.d906bcf328d46p-1f, 0x1.0000000000000p+0f, 0x1.d906bcf328d46p-1f,
    0x1.0000000000000p+0f, 0x1.d906bcf328d46p-1f, 0x1.0000000000000p+0f, 0x1.d906bcf328d46p-1f,
    0x1.6a09e667f3bcdp-1f, 0x1.87de2a6aea964p-2f, 0x1.6a09e667f3bcdp-1f, 0x1.87de2a6aea964p-2f,
    0x1.6a09e667f3bcdp-1f, 0x1.87de2a6aea964p-2f, 0x1.6a09e667f3bcdp-1f, 0x1.87de2a6aea964p-2f,
    0x0.0p+0f, -0x1.87de2a6aea963p-2f, 0x0.0p+0f, -0x1.87de2a6aea963p-2f,
    0x0.0p+0f, -0x1.87de2a6aea963p-2f, 0x0.0p+0f, -0x1.87de2a6aea963p-2f,
    -0x1.6a09e667f3bccp-1f, -0x1.d906bcf328d46p-1f, -0x1.6a09e667f3bccp-1f, -0x1.d906bcf328d46p-1f,
    -0x1.6a09e667f3bccp-1f, -0x1.d906bcf328d46p-1f, -0x1.6a09e667f3bccp-1f, -0x1.d906bcf328d46p-1f,
    0x1.0000000000000p+0f, 0x1.f6297cff75cb0p-1f, 0x1.0000000000000p+0f, 0x1.f6297cff75cb0p-1f,
    0x1.d906bcf328d46p-1f, 0x1.a9b66290ea1a3p-1f, 0x1.d906bcf328d46p-1f, 0x1.a9b66290ea1a3p-1f,
    0x1.6a09e667f3bcdp-1f, 0x1.1c73b39ae68c9p-1f, 0x1.6a09e667f3bcdp-1f, 0x1.1c73b39ae68c9p-1f,
    0x1.87de2a6aea964p-2f, 0x1.8f8b83c69a60dp-3f, 0x1.87de2a6aea964p-2f, 0x1.8f8b83c69a60dp-3f,
    0x0.0p+0f, -0x1.8f8b83c69a60ap-3f, 0x0.0p+0f, -0x1.8f8b83c69a60ap-3f,
    -0x1.87de2a6aea963p-2f, -0x1.1c73b39ae68c8p-1f, -0x1.87de2a6aea963p-2f, -0x1.1c73b39ae68c8p-1f,
    -0x1.6a09e667f3bccp-1f, -0x1.a9b66290ea1a3p-1f, -0x1.6a09e667f3bccp-1f, -0x1.a9b66290ea1a3p-1f,
    -0x1.d906bcf328d46p-1f, -0x1.f6297cff75cb0p-1f, -0x1.d906bcf328d46p-1f, -0x1.f6297cff75cb0p-1f,
    0x1.0000000000000p+0f, 0x1.fd88da3d12526p-1f, 0x1.6a09e667f3bcdp-1f, 0x1.44cf325091dd6p-1f,
    0x1.f6297cff75cb0p-1f, 0x1.e9f4156c62ddap-1f, 0x1.1c73b39ae68c9p-1f, 0x1.e2b5d3806f63ep-2f,
    0x1.d906bcf328d46p-1f, 0x1.c38b2f180bdb1p-1f, 0x1.87de2a6aea964p-2f, 0x1.294062ed59f05p-2f,
    0x1.a9b66290ea1a3p-1f, 0x1.8bc806b151741p-1f, 0x1.8f8b83c69a60dp-3f, 0x1.917a6bc29b438p-4f,
    0x0.0p+0f, -0x1.917a6bc29b42cp-4f, -0x1.6a09e667f3bccp-1f, -0x1.8bc806b151741p-1f,
    -0x1.8f8b83c69a60ap-3f, -0x1.294062ed59f05p-2f, -0x1.a9b66290ea1a3p-1f, -0x1.c38b2f180bdb0p-1f,
    -0x1.87de2a6aea963p-2f, -0x1.e2b5d3806f63bp-2f, -0x1.d906bcf328d46p-1f, -0x1.e9f4156c62ddbp-1f,
    -0x1.1c73b39ae68c8p-1f, -0x1.44cf325091dd6p-1f, -0x1.f6297cff75cb0p-1f, -0x1.fd88da3d12525p-1f,
};
/* Complete 64 point transform from `in` to `out`, both interleaved. */
__attribute__((target("avx512f"))) static void onefft_avx512_n64(const float *in, float *out) {
    const float *consts = onefft_avx512_n64_consts;
    __asm__ volatile(
        "    vmovups (%[in]), %%zmm0\n"
        "    vmovups 128(%[in]), %%zmm1\n"
        "    vmovups 256(%[in]), %%zmm2\n"
        "    vmovups 384(%[in]), %%zmm3\n"
        "    vmovups 64(%[consts]), %%zmm4\n"
        "    vmovups 128(%[consts]), %%zmm5\n"
        "    vmovups 192(%[consts]), %%zmm6\n"
        "    vmovups 256(%[consts]), %%zmm7\n"
        "    vshufps $136, 64(%[in]), %%zmm0, %%zmm8\n"
        "    vshufps $221, 64(%[in]), %%zmm0, %%zmm0\n"
        "    vshufps $136, 192(%[in]), %%zmm1, %%zmm9\n"
        "    vshufps $221, 192(%[in]), %%zmm1, %%zmm1\n"
        "    vshufps $136, 320(%[in]), %%zmm2, %%zmm10\n"
        "    vshufps $221, 320(%[in]), %%zmm2, %%zmm2\n"
        "    vshufps $136, 448(%[in]), %%zmm3, %%zmm11\n"
        "    vshufps $221, 448(%[in]), %%zmm3, %%zmm3\n"
        "    vaddps %%zmm10, %%zmm8, %%zmm12\n"
        "    vaddps %%zmm2, %%zmm0, %%zmm13\n"
        "    vsubps %%zmm10, %%zmm8, %%zmm8\n"
        "    vsubps %%zmm2, %%zmm0, %%zmm0\n"
        "    vaddps %%zmm11, %%zmm9, %%zmm2\n"
        "    vaddps %%zmm3, %%zmm1, %%zmm10\n"
        "    vsubps %%zmm11, %%zmm9, %%zmm9\n"
        "    vsubps %%zmm3, %%zmm1, %%zmm1\n"
        "    vmovups 320(%[consts]), %%zmm3\n"
        "    vaddps %%zmm2, %%zmm12, %%zmm11\n"
        "    vaddps %%zmm10, %%zmm13, %%zmm14\n"
        "    vsubps %%zmm2, %%zmm12, %%zmm2\n"
        "    vsubps %%zmm10, %%zmm13, %%zmm10\n"
        "    vaddps %%zmm1, %%zmm8, %%zmm12\n"
        "    vsubps %%zmm9, %%zmm0, %%zmm13\n"
        "    vmovups 384(%[consts]), %%zmm15\n"
        "    vsubps %%zmm1, %%zmm8, %%zmm1\n"
        "    vaddps %%zmm9, %%zmm0, %%zmm0\n"
        "    vunpckhps %%zmm12, %%zmm11, %%zmm8\n"
        "    vunpckhps %%zmm13, %%zmm14, %%zmm9\n"
        "    vunpcklps %%zmm12, %%zmm11, %%zmm11\n"
        "    vunpckhps %%zmm1, %%zmm2, %%zmm12\n"
        "    vunpckhps %%zmm0, %%zmm10, %%zmm16\n"
        "    vmulps %%zmm4, %%zmm8, %%zmm17\n"
        "    vmulps %%zmm4, %%zmm9, %%zmm18\n"
        "    vmulps %%zmm4, %%zmm12, %%zmm19\n"
        "    vmulps %%zmm4, %%zmm16, %%zmm4\n"
        "    vunpcklps %%zmm13, %%zmm14, %%zmm13\n"
        "    vunpcklps %%zmm1, %%zmm2, %%zmm1\n"
        "    vunpcklps %%zmm0, %%zmm10, %%zmm0\n"
        "    vfnmadd231ps %%zmm5, %%zmm9, %%zmm17\n"
        "    vfmadd231ps %%zmm5, %%zmm8, %%zmm18\n"
        "    vfnmadd231ps %%zmm5, %%zmm16, %%zmm19\n"
        "    vfmadd231ps %%zmm5, %%zmm12, %%zmm4\n"
        "    vaddps %%zmm17, %%zmm11, %%zmm2\n"
        "    vsubps %%zmm17, %%zmm11, %%zmm5\n"
        "    vaddps %%zmm18, %%zmm13, %%zmm8\n"
        "    vsubps %%zmm18, %%zmm13, %%zmm9\n"
        "    vaddps %%zmm4, %%zmm1, %%zmm10\n"
        "    vsubps %%zmm19, %%zmm0, %%zmm11\n"
        "    vsubps %%zmm4, %%zmm1, %%zmm1\n"
        "    vaddps %%zmm19, %%zmm0, %%zmm0\n"
        "    vmovups 448(%[consts]), %%zmm4\n"
        "    vmovups 512(%[consts]), %%zmm12\n"
        "    vshuff32x4 $238, %%zmm10, %%zmm2, %%zmm13\n"
        "    vshuff32x4 $68, %%zmm10, %%zmm2, %%zmm2\n"
        "    vshuff32x4 $238, %%zmm11, %%zmm8, %%zmm10\n"
        "    vshuff32x4 $238, %%zmm1, %%zmm5, %%zmm14\n"
        "    vshuff32x4 $238, %%zmm0, %%zmm9, %%zmm16\n"
        "    vmulps %%zmm6, %%zmm13, %%zmm17\n"
        "    vmulps %%zmm6, %%zmm10, %%zmm18\n"
        "    vmulps %%zmm6, %%zmm14, %%zmm19\n"
        "    vmulps %%zmm6, %%zmm16, %%zmm6\n"
        "    vshuff32x4 $68, %%zmm11, %%zmm8, %%zmm8\n"
        "    vshuff32x4 $68, %%zmm1, %%zmm5, %%zmm1\n"
        "    vshuff32x4 $68, %%zmm0, %%zmm9, %%zmm0\n"
        "    vfnmadd231ps %%zmm7, %%zmm10, %%zmm17\n"
        "    vfmadd231ps %%zmm7, %%zmm13, %%zmm18\n"
        "    vfnmadd231ps %%zmm7, %%zmm16, %%zmm19\n"
        "    vfmadd231ps %%zmm7, %%zmm14, %%zmm6\n"
        "    vaddps %%zmm17, %%zmm2, %%zmm5\n"
        "    vaddps %%zmm18, %%zmm8, %%zmm7\n"
        "    vsubps %%zmm17, %%zmm2, %%zmm2\n"
        "    vsubps %%zmm18, %%zmm8, %%zmm8\n"
        "    vaddps %%zmm6, %%zmm1, %%zmm9\n"
        "    vsubps %%zmm19, %%zmm0, %%zmm10\n"
        "    vsubps %%zmm6, %%zmm1, %%zmm1\n"
        "    vaddps %%zmm19, %%zmm0, %%zmm0\n"
        "    vshuff32x4 $221, %%zmm9, %%zmm5, %%zmm6\n"
        "    vshuff32x4 $221, %%zmm10, %%zmm7, %%zmm11\n"
        "    vshuff32x4 $136, %%zmm9, %%zmm5, %%zmm5\n"
        "    vshuff32x4 $221, %%zmm1, %%zmm2, %%zmm9\n"
        "    vshuff32x4 $221, %%zmm0, %%zmm8, %%zmm13\n"
        "    vmulps %%zmm3, %%zmm6, %%zmm14\n"
        "    vmulps %%zmm3, %%zmm11, %%zmm16\n"
        "    vmulps %%zmm3, %%zmm9, %%zmm17\n"
        "    vmulps %%zmm3, %%zmm13, %%zmm3\n"
        "    vshuff32x4 $136, %%zmm10, %%zmm7, %%zmm7\n"
        "    vshuff32x4 $136, %%zmm1, %%zmm2, %%zmm1\n"
        "    vshuff32x4 $136, %%zmm0, %%zmm8, %%zmm0\n"
        "    vfnmadd231ps %%zmm15, %%zmm11, %%zmm14\n"
        "    vfmadd231ps %%zmm15, %%zmm6, %%zmm16\n"
        "    vfnmadd231ps %%zmm15, %%zmm13, %%zmm17\n"
        "    vfmadd231ps %%zmm15, %%zmm9, %%zmm3\n"
        "    vaddps %%zmm14, %%zmm5, %%zmm2\n"
        "    vsubps %%zmm14, %%zmm5, %%zmm5\n"
        "    vaddps %%zmm16, %%zmm7, %%zmm6\n"
        "    vsubps %%zmm16, %%zmm7, %%zmm7\n"
        "    vaddps %%zmm3, %%zmm1, %%zmm8\n"
        "    vsubps %%zmm17, %%zmm0, %%zmm9\n"
        "    vsubps %%zmm3, %%zmm1, %%zmm1\n"
        "    vaddps %%zmm17, %%zmm0, %%zmm0\n"
        "    vunpckhpd %%zmm8, %%zmm2, %%zmm3\n"
        "    vunpcklpd %%zmm8, %%zmm2, %%zmm2\n"
        "    vunpckhpd %%zmm9, %%zmm6, %%zmm8\n"
        "    vunpckhpd %%zmm1, %%zmm5, %%zmm10\n"
        "    vunpckhpd %%zmm0, %%zmm7, %%zmm11\n"
        "    vmulps %%zmm4, %%zmm3, %%zmm13\n"
        "    vmulps %%zmm4, %%zmm8, %%zmm14\n"
        "    vmulps %%zmm4, %%zmm10, %%zmm15\n"
        "    vmulps %%zmm4, %%zmm11, %%zmm4\n"
        "    vunpcklpd %%zmm9, %%zmm6, %%zmm6\n"
        "    vunpcklpd %%zmm1, %%zmm5, %%zmm1\n"
        "    vunpcklpd %%zmm0, %%zmm7, %%zmm0\n"
        "    vfnmadd231ps %%zmm12, %%zmm8, %%zmm13\n"
        "    vfmadd231ps %%zmm12, %%zmm3, %%zmm14\n"
        "    vfnmadd231ps %%zmm12, %%zmm11, %%zmm15\n"
        "    vfmadd231ps %%zmm12, %%zmm10, %%zmm4\n"
        "    vaddps %%zmm13, %%zmm2, %%zmm3\n"
        "    vaddps %%zmm14, %%zmm6, %%zmm5\n"
        "    vsubps %%zmm13, %%zmm2, %%zmm2\n"
        "    vsubps %%zmm14, %%zmm6, %%zmm6\n"
        "    vaddps %%zmm4, %%zmm1, %%zmm7\n"
        "    vsubps %%zmm15, %%zmm0, %%zmm8\n"
        "    vsubps %%zmm4, %%zmm1, %%zmm1\n"
        "    vaddps %%zmm15, %%zmm0, %%zmm0\n"
        "    vunpcklps %%zmm5, %%zmm3, %%zmm4\n"
        "    vunpckhps %%zmm5, %%zmm3, %%zmm3\n"
        "    vunpcklps %%zmm6, %%zmm2, %%zmm5\n"
        "    vunpckhps %%zmm6, %%zmm2, %%zmm2\n"
        "    vunpcklps %%zmm8, %%zmm7, %%zmm6\n"
        "    vmovups %%zmm4, (%[out])\n"
        "    vmovups %%zmm3, 64(%[out])\n"
        "    vunpckhps %%zmm8, %%zmm7, %%zmm3\n"
        "    vunpcklps %%zmm0, %%zmm1, %%zmm4\n"
        "    vunpckhps %%zmm0, %%zmm1, %%zmm0\n"
        "    vmovups %%zmm5, 256(%[out])\n"
        "    vmovups %%zmm2, 320(%[out])\n"
        "    vmovups %%zmm6, 128(%[out])\n"
        "    vmovups %%zmm3, 192(%[out])\n"
        "    vmovups %%zmm4, 384(%[out])\n"
        "    vmovups %%zmm0, 448(%[out])\n"
        "    vzeroupper\n"
        : [in] "+r"(in), [out] "+r"(out), [consts] "+r"(consts)
        :
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "xmm16", "xmm17", "xmm18", "xmm19", "cc", "memory");
}

static const float onefft_avx512_n128_consts[272] ONEFFT_ALIGNED(64) = {
    0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f,
    0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f,
    0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f,
    0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f,
    0x1.0000000000000p+0f, 0x1.d906bcf328d46p-1f, 0x1.0000000000000p+0f, 0x1.d906bcf328d46p-1f,
    0x1.0000000000000p+0f, 0x1.d906bcf328d46p-1f, 0x1.0000000000000p+0f, 0x1.d906bcf328d46p-1f,
    0x1.0000000000000p+0f, 0x1.d906bcf328d46p-1f, 0x1.0000000000000p+0f, 0x1.d906bcf328d46p-1f,
    0x1.0000000000000p+0f, 0x1.d906bcf328d46p-1f, 0x1.0000000000000p+0f, 0x1.d906bcf328d46p-1f,
    0x0.0p+0f, -0x1.87de2a6aea963p-2f, 0x0.0p+0f, -0x1.87de2a6aea963p-2f,
    0x0.0p+0f, -0x1.87de2a6aea963p-2f, 0x0.0p+0f, -0x1.87de2a6aea963p-2f,
    0x0.0p+0f, -0x1.87de2a6aea963p-2f, 0x0.0p+0f, -0x1.87de2a6aea963p-2f,
    0x0.0p+0f, -0x1.87de2a6aea963p-2f, 0x0.0p+0f, -0x1.87de2a6aea963p-2f,
    0x1.6a09e667f3bcdp-1f, 0x1.87de2a6aea964p-2f, 0x1.6a09e667f3bcdp-1f, 0x1.87de2a6aea964p-2f,
    0x1.6a09e667f3bcdp-1f, 0x1.87de2a6aea964p-2f, 0x1.6a09e667f3bcdp-1f, 0x1.87de2a6aea964p-2f,
    0x1.6a09e667f3bcdp-1f, 0x1.87de2a6aea964p-2f, 0x1.6a09e667f3bcdp-1f, 0x1.87de2a6aea964p-2f,
    0x1.6a09e667f3bcdp-1f, 0x1.87de2a6aea964p-2f, 0x1.6a09e667f3bcdp-1f, 0x1.87de2a6aea964p-2f,
    -0x1.6a09e667f3bccp-1f, -0x1.d906bcf328d46p-1f, -0x1.6a09e667f3bccp-1f, -0x1.d906bcf328d46p-1f,
    -0x1.6a09e667f3bccp-1f, -0x1.d906bcf328d46p-1f, -0x1.6a09e667f3bccp-1f, -0x1.d906bcf328d46p-1f,
    -0x1.6a09e667f3bccp-1f, -0x1.d906bcf328d46p-1f, -0x1.6a09e667f3bccp-1f, -0x1.d906bcf328d46p-1f,
    -0x1.6a09e667f3bccp-1f, -0x1.d906bcf328d46p-1f, -0x1.6a09e667f3bccp-1f, -0x1.d906bcf328d46p-1f,
    0x1.0000000000000p+0f, 0x1.f6297cff75cb0p-1f, 0x1.0000000000000p+0f, 0x1.f6297cff75cb0p-1f,
    0x1.0000000000000p+0f, 0x1.f6297cff75cb0p-1f, 0x1.0000000000000p+0f, 0x1.f6297cff75cb0p-1f,
    0x1.d906bcf328d46p-1f, 0x1.a9b66290ea1a3p-1f, 0x1.d906bcf328d46p-1f, 0x1.a9b66290ea1a3p-1f,
    0x1.d906bcf328d46p-1f, 0x1.a9b66290ea1a3p-1f, 0x1.d906bcf328d46p-1f, 0x1.a9b66290ea1a3p-1f,
    0x0.0p+0f, -0x1.8f8b83c69a60ap-3f, 0x0.0p+0f, -0x1.8f8b83c69a60ap-3f,
    0x0.0p+0f, -0x1.8f8b83c69a60ap-3f, 0x0.0p+0f, -0x1.8f8b83c69a60ap-3f,
    -0x1.87de2a6aea963p-2f, -0x1.1c73b39ae68c8p-1f, -0x1.87de2a6aea963p-2f, -0x1.1c73b39ae68c8p-1f,
    -0x1.87de2a6aea963p-2f, -0x1.1c73b39ae68c8p-1f, -0x1.87de2a6aea963p-2f, -0x1.1c73b39ae68c8p-1f,
    0x1.6a09e667f3bcdp-1f, 0x1.1c73b39ae68c9p-1f, 0x1.6a09e667f3bcdp-1f, 0x1.1c73b39ae68c9p-1f,
    0x1.6a09e667f3bcdp-1f, 0x1.1c73b39ae68c9p-1f, 0x1.6a09e667f3bcdp-1f, 0x1.1c73b39ae68c9p-1f,
    0x1.87de2a6aea964p-2f, 0x1.8f8b83c69a60dp-3f, 0x1.87de2a6aea964p-2f, 0x1.8f8b83c69a60dp-3f,
    0x1.87de2a6aea964p-2f, 0x1.8f8b83c69a60dp-3f, 0x1.87de2a6aea964p-2f, 0x1.8f8b83c69a60dp-3f,
    -0x1.6a09e667f3bccp-1f, -0x1.a9b66290ea1a3p-1f, -0x1.6a09e667f3bccp-1f, -0x1.a9b66290ea1a3p-1f,
    -0x1.6a09e667f3bccp-1f, -0x1.a9b66290ea1a3p-1f, -0x1.6a09e667f3bccp-1f, -0x1.a9b66290ea1a3p-1f,
    -0x1.d906bcf328d46p-1f, -0x1.f6297cff75cb0p-1f, -0x1.d906bcf328d46p-1f, -0x1.f6297cff75cb0p-1f,
    -0x1.d906bcf328d46p-1f, -0x1.f6297cff75cb0p-1f, -0x1.d906bcf328d46p-1f, -0x1.f6297cff75cb0p-1f,
    0x1.0000000000000p+0f, 0x1.fd88da3d12526p-1f, 0x1.0000000000000p+0f, 0x1.fd88da3d12526p-1f,
    0x1.f6297cff75cb0p-1f, 0x1.e9f4156c62ddap-1f, 0x1.f6297cff75cb0p-1f, 0x1.e9f4156c62ddap-1f,
    0x1.d906bcf328d46p-1f, 0x1.c38b2f180bdb1p-1f, 0x1.d906bcf328d46p-1f, 0x1.c38b2f180bdb1p-1f,
    0x1.a9b66290ea1a3p-1f, 0x1.8bc806b151741p-1f, 0x1.a9b66290ea1a3p-1f, 0x1.8bc806b151741p-1f,
    0x0.0p+0f, -0x1.917a6bc29b42cp-4f, 0x0.0p+0f, -0x1.917a6bc29b42cp-4f,
    -0x1.8f8b83c69a60ap-3f, -0x1.294062ed59f05p-2f, -0x1.8f8b83c69a60ap-3f, -0x1.294062ed59f05p-2f,
    -0x1.87de2a6aea963p-2f, -0x1.e2b5d3806f63bp-2f, -0x1.87de2a6aea963p-2f, -0x1.e2b5d3806f63bp-2f,
    -0x1.1c73b39ae68c8p-1f, -0x1.44cf325091dd6p-1f, -0x1.1c73b39ae68c8p-1f, -0x1.44cf325091dd6p-1f,
    0x1.6a09e667f3bcdp-1f, 0x1.44cf325091dd6p-1f, 0x1.6a09e667f3bcdp-1f, 0x1.44cf325091dd6p-1f,
    0x1.1c73b39ae68c9p-1f, 0x1.e2b5d3806f63ep-2f, 0x1.1c73b39ae68c9p-1f, 0x1.e2b5d3806f63ep-2f,
    0x1.87de2a6aea964p-2f, 0x1.294062ed59f05p-2f, 0x1.87de2a6aea964p-2f, 0x1.294062ed59f05p-2f,
    0x1.8f8b83c69a60dp-3f, 0x1.917a6bc29b438p-4f, 0x1.8f8b83c69a60dp-3f, 0x1.917a6bc29b438p-4f,
    -0x1.6a09e667f3bccp-1f, -0x1.8bc806b151741p-1f, -0x1.6a09e667f3bccp-1f, -0x1.8bc806b151741p-1f,
    -0x1.a9b66290ea1a3p-1f, -0x1.c38b2f180bdb0p-1f, -0x1.a9b66290ea1a3p-1f, -0x1.c38b2f180bdb0p-1f,
    -0x1.d906bcf328d46p-1f, -0x1.e9f4156c62ddbp-1f, -0x1.d906bcf328d46p-1f, -0x1.e9f4156c62ddbp-1f,
    -0x1.f6297cff75cb0p-1f, -0x1.fd88da3d12525p-1f, -0x1.f6297cff75cb0p-1f, -0x1.fd88da3d12525p-1f,
    0x1.0000000000000p+0f, 0x1.ff621e3796d7ep-1f, 0x1.d906bcf328d46p-1f, 0x1.ced7af43cc773p-1f,
    0x1.fd88da3d12526p-1f, 0x1.fa7557f08a517p-1f, 0x1.c38b2f180bdb1p-1f, 0x1.b728345196e3ep-1f,
    0x1.f6297cff75cb0p-1f, 0x1.f0a7efb9230d7p-1f, 0x1.a9b66290ea1a3p-1f, 0x1.9b3e047f38741p-1f,
    0x1.e9f4156c62ddap-1f, 0x1.e212104f686e5p-1f, 0x1.8bc806b151741p-1f, 0x1.7b5df226aafafp-1f,
    0x0.0p+0f, -0x1.91f65f10dd814p-5f, -0x1.87de2a6aea963p-2f, -0x1.b5d1009e15cc0p-2f,
    -0x1.917a6bc29b42cp-4f, -0x1.2c8106e8e613ap-3f, -0x1.e2b5d3806f63bp-2f, -0x1.073879922ffedp-1f,
    -0x1.8f8b83c69a60ap-3f, -0x1.f19f97b215f1ap-3f, -0x1.1c73b39ae68c8p-1f, -0x1.30ff7fce17035p-1f,
    -0x1.294062ed59f05p-2f, -0x1.58f9a75ab1fddp-2f, -0x1.44cf325091dd6p-1f, -0x1.57d69348cec9fp-1f,
    0x1.6a09e667f3bcdp-1f, 0x1.57d69348cec9fp-1f, 0x1.87de2a6aea964p-2f, 0x1.58f9a75ab1fddp-2f,
    0x1.44cf325091dd6p-1f, 0x1.30ff7fce17036p-1f, 0x1.294062ed59f05p-2f, 0x1.f19f97b215f1ep-3f,
    0x1.1c73b39ae68c9p-1f, 0x1.073879922ffedp-1f, 0x1.8f8b83c69a60dp-3f, 0x1.2c8106e8e613ap-3f,
    0x1.e2b5d3806f63ep-2f, 0x1.b5d1009e15cc2p-2f, 0x1.917a6bc29b438p-4f, 0x1.91f65f10dd824p-5f,
    -0x1.6a09e667f3bccp-1f, -0x1.7b5df226aafafp-1f, -0x1.d906bcf328d46p-1f, -0x1.e212104f686e5p-1f,
    -0x1.8bc806b151741p-1f, -0x1.9b3e047f38740p-1f, -0x1.e9f4156c62ddbp-1f, -0x1.f0a7efb9230d7p-1f,
    -0x1.a9b66290ea1a3p-1f, -0x1.b728345196e3ep-1f, -0x1.f6297cff75cb0p-1f, -0x1.fa7557f08a517p-1f,
    -0x1.c38b2f180bdb0p-1f, -0x1.ced7af43cc773p-1f, -0x1.fd88da3d12525p-1f, -0x1.ff621e3796d7ep-1f,
};
/* Complete 128 point transform from `in` to `out`, both interleaved. */
__attribute__((target("avx512f"))) static void onefft_avx512_n128(const float *in, float *out) {
    const float *consts = onefft_avx512_n128_consts;
    __asm__ volatile(
        "    vmovups 128(%[in]), %%zmm0\n"
        "    vmovups 384(%[in]), %%zmm1\n"
        "    vmovups 640(%[in]), %%zmm2\n"
        "    vmovups 896(%[in]), %%zmm3\n"
        "    vmovups (%[in]), %%zmm4\n"
        "    vmovups 256(%[in]), %%zmm5\n"
        "    vmovups 512(%[in]), %%zmm6\n"
        "    vmovups 768(%[in]), %%zmm7\n"
        "    vbroadcastss (%[consts]), %%zmm8\n"
        "    vshufps $136, 192(%[in]), %%zmm0, %%zmm9\n"
        "    vshufps $221, 192(%[in]), %%zmm0, %%zmm0\n"
        "    vshufps $136, 448(%[in]), %%zmm1, %%zmm10\n"
        "    vshufps $221, 448(%[in]), %%zmm1, %%zmm1\n"
        "    vshufps $136, 704(%[in]), %%zmm2, %%zmm11\n"
        "    vshufps $221, 704(%[in]), %%zmm2, %%zmm2\n"
        "    vshufps $136, 960(%[in]), %%zmm3, %%zmm12\n"
        "    vshufps $221, 960(%[in]), %%zmm3, %%zmm3\n"
        "    vshufps $136, 64(%[in]), %%zmm4, %%zmm13\n"
        "    vsubps %%zmm11, %%zmm9, %%zmm14\n"
        "    vshufps $221, 64(%[in]), %%zmm4, %%zmm4\n"
        "    vshufps $136, 320(%[in]), %%zmm5, %%zmm15\n"
        "    vsubps %%zmm2, %%zmm0, %%zmm16\n"
        "    vsubps %%zmm12, %%zmm10, %%zmm17\n"
        "    vsubps %%zmm3, %%zmm1, %%zmm18\n"
        "    vshufps $221, 320(%[in]), %%zmm5, %%zmm5\n"
        "    vshufps $136, 576(%[in]), %%zmm6, %%zmm19\n"
        "    vshufps $221, 576(%[in]), %%zmm6, %%zmm6\n"
        "    vshufps $136, 832(%[in]), %%zmm7, %%zmm20\n"
        "    vshufps $221, 832(%[in]), %%zmm7, %%zmm7\n"
        "    vaddps %%zmm19, %%zmm13, %%zmm21\n"
        "    vaddps %%zmm6, %%zmm4, %%zmm22\n"
        "    vsubps %%zmm19, %%zmm13, %%zmm13\n"
        "    vsubps %%zmm6, %%zmm4, %%zmm4\n"
        "    vaddps %%zmm18, %%zmm14, %%zmm6\n"
        "    vsubps %%zmm17, %%zmm16, %%zmm19\n"
        "    vsubps %%zmm18, %%zmm14, %%zmm14\n"
        "    vaddps %%zmm17, %%zmm16, %%zmm16\n"
        "    vaddps %%zmm20, %%zmm15, %%zmm17\n"
        "    vaddps %%zmm7, %%zmm5, %%zmm18\n"
        "    vsubps %%zmm20, %%zmm15, %%zmm15\n"
        "    vsubps %%zmm7, %%zmm5, %%zmm5\n"
        "    vaddps %%zmm11, %%zmm9, %%zmm7\n"
        "    vaddps %%zmm2, %%zmm0, %%zmm0\n"
        "    vaddps %%zmm12, %%zmm10, %%zmm2\n"
        "    vaddps %%zmm3, %%zmm1, %%zmm1\n"
        "    vaddps %%zmm19, %%zmm6, %%zmm3\n"
        "    vsubps %%zmm6, %%zmm19, %%zmm6\n"
        "    vaddps %%zmm16, %%zmm14, %%zmm9\n"
        "    vsubps %%zmm14, %%zmm16, %%zmm10\n"
        "    vaddps %%zmm17, %%zmm21, %%zmm11\n"
        "    vaddps %%zmm18, %%zmm22, %%zmm12\n"
        "    vsubps %%zmm17, %%zmm21, %%zmm14\n"
        "    vsubps %%zmm18, %%zmm22, %%zmm16\n"
        "    vaddps %%zmm5, %%zmm13, %%zmm17\n"
        "    vsubps %%zmm15, %%zmm4, %%zmm18\n"
        "    vsubps %%zmm5, %%zmm13, %%zmm5\n"
        "    vaddps %%zmm15, %%zmm4, %%zmm4\n"
        "    vaddps %%zmm2, %%zmm7, %%zmm13\n"
        "    vaddps %%zmm1, %%zmm0, %%zmm15\n"
        "    vsubps %%zmm2, %%zmm7, %%zmm2\n"
        "    vsubps %%zmm1, %%zmm0, %%zmm0\n"
        "    vmulps %%zmm8, %%zmm3, %%zmm1\n"
        "    vmulps %%zmm8, %%zmm6, %%zmm3\n"
        "    vmulps %%zmm8, %%zmm9, %%zmm6\n"
        "    vmulps %%zmm8, %%zmm10, %%zmm7\n"
        "    vmovups 64(%[consts]), %%zmm8\n"
        "    vmovups 128(%[consts]), %%zmm9\n"
        "    vaddps %%zmm13, %%zmm11, %%zmm10\n"
        "    vaddps %%zmm15, %%zmm12, %%zmm19\n"
        "    vsubps %%zmm13, %%zmm11, %%zmm11\n"
        "    vsubps %%zmm15, %%zmm12, %%zmm12\n"
        "    vaddps %%zmm1, %%zmm17, %%zmm13\n"
        "    vsubps %%zmm1, %%zmm17, %%zmm1\n"
        "    vaddps %%zmm3, %%zmm18, %%zmm15\n"
        "    vsubps %%zmm3, %%zmm18, %%zmm3\n"
        "    vaddps %%zmm0, %%zmm14, %%zmm17\n"
        "    vsubps %%zmm2, %%zmm16, %%zmm18\n"
        "    vsubps %%zmm0, %%zmm14, %%zmm0\n"
        "    vaddps %%zmm2, %%zmm16, %%zmm2\n"
        "    vaddps %%zmm7, %%zmm5, %%zmm14\n"
        "    vsubps %%zmm6, %%zmm4, %%zmm16\n"
        "    vsubps %%zmm7, %%zmm5, %%zmm5\n"
        "    vaddps %%zmm6, %%zmm4, %%zmm4\n"
        "    vmovups 192(%[consts]), %%zmm6\n"
        "    vunpckhps %%zmm13, %%zmm10, %%zmm7\n"
        "    vunpckhps %%zmm1, %%zmm11, %%zmm20\n"
        "    vmovups 256(%[consts]), %%zmm21\n"
        "    vunpckhps %%zmm15, %%zmm19, %%zmm22\n"
        "    vunpckhps %%zmm3, %%zmm12, %%zmm23\n"
        "    vmulps %%zmm8, %%zmm7, %%zmm24\n"
        "    vmulps %%zmm8, %%zmm22, %%zmm25\n"
        "    vmulps %%zmm8, %%zmm20, %%zmm26\n"
        "    vmulps %%zmm8, %%zmm23, %%zmm8\n"
        "    vunpckhps %%zmm14, %%zmm17, %%zmm27\n"
        "    vunpckhps %%zmm16, %%zmm18, %%zmm28\n"
        "    vunpcklps %%zmm13, %%zmm10, %%zmm10\n"
        "    vunpcklps %%zmm15, %%zmm19, %%zmm13\n"
        "    vunpcklps %%zmm1, %%zmm11, %%zmm1\n"
        "    vunpcklps %%zmm3, %%zmm12, %%zmm3\n"
        "    vmulps %%zmm6, %%zmm27, %%zmm11\n"
        "    vmulps %%zmm6, %%zmm28, %%zmm12\n"
        "    vfnmadd231ps %%zmm9, %%zmm22, %%zmm24\n"
        "    vunpckhps %%zmm5, %%zmm0, %%zmm15\n"
        "    vfmadd231ps %%zmm9, %%zmm7, %%zmm25\n"
        "    vfnmadd231ps %%zmm9, %%zmm23, %%zmm26\n"
        "    vfmadd231ps %%zmm9, %%zmm20, %%zmm8\n"
        "    vunpcklps %%zmm14, %%zmm17, %%zmm7\n"
        "    vunpcklps %%zmm16, %%zmm18, %%zmm9\n"
        "    vunpcklps %%zmm5, %%zmm0, %%zmm0\n"
        "    vunpcklps %%zmm4, %%zmm2, %%zmm5\n"
        "    vfnmadd231ps %%zmm21, %%zmm28, %%zmm11\n"
        "    vfmadd231ps %%zmm21, %%zmm27, %%zmm12\n"
        "    vaddps %%zmm24, %%zmm10, %%zmm14\n"
        "    vunpckhps %%zmm4, %%zmm2, %%zmm2\n"
        "    vaddps %%zmm25, %%zmm13, %%zmm4\n"
        "    vsubps %%zmm24, %%zmm10, %%zmm10\n"
        "    vmulps %%zmm6, %%zmm15, %%zmm16\n"
        "    vmulps %%zmm6, %%zmm2, %%zmm6\n"
        "    vsubps %%zmm25, %%zmm13, %%zmm13\n"
        "    vaddps %%zmm8, %%zmm1, %%zmm17\n"
        "    vsubps %%zmm26, %%zmm3, %%zmm18\n"
        "    vsubps %%zmm8, %%zmm1, %%zmm1\n"
        "    vaddps %%zmm26, %%zmm3, %%zmm3\n"
        "    vaddps %%zmm11, %%zmm7, %%zmm8\n"
        "    vaddps %%zmm12, %%zmm9, %%zmm19\n"
        "    vmovups 320(%[consts]), %%zmm20\n"
        "    vmovups 384(%[consts]), %%zmm22\n"
        "    vsubps %%zmm11, %%zmm7, %%zmm7\n"
        "    vsubps %%zmm12, %%zmm9, %%zmm9\n"
        "    vfnmadd231ps %%zmm21, %%zmm2, %%zmm16\n"
        "    vfmadd231ps %%zmm21, %%zmm15, %%zmm6\n"
        "    vshuff32x4 $238, %%zmm8, %%zmm14, %%zmm2\n"
        "    vshuff32x4 $238, %%zmm19, %%zmm4, %%zmm11\n"
        "    vshuff32x4 $68, %%zmm8, %%zmm14, %%zmm8\n"
        "    vshuff32x4 $238, %%zmm7, %%zmm10, %%zmm12\n"
        "    vshuff32x4 $238, %%zmm9, %%zmm13, %%zmm14\n"
        "    vmulps %%zmm20, %%zmm2, %%zmm15\n"
        "    vaddps %%zmm6, %%zmm0, %%zmm21\n"
        "    vsubps %%zmm16, %%zmm5, %%zmm23\n"
        "    vsubps %%zmm6, %%zmm0, %%zmm0\n"
        "    vaddps %%zmm16, %%zmm5, %%zmm5\n"
        "    vmovups 448(%[consts]), %%zmm6\n"
        "    vmovups 512(%[consts]), %%zmm16\n"
        "    vmulps %%zmm20, %%zmm11, %%zmm24\n"
        "    vmulps %%zmm20, %%zmm12, %%zmm25\n"
        "    vmulps %%zmm20, %%zmm14, %%zmm20\n"
        "    vshuff32x4 $68, %%zmm19, %%zmm4, %%zmm4\n"
        "    vshuff32x4 $68, %%zmm7, %%zmm10, %%zmm7\n"
        "    vfnmadd231ps %%zmm22, %%zmm11, %%zmm15\n"
        "    vshuff32x4 $68, %%zmm9, %%zmm13, %%zmm9\n"
        "    vshuff32x4 $238, %%zmm21, %%zmm17, %%zmm10\n"
        "    vshuff32x4 $238, %%zmm23, %%zmm18, %%zmm11\n"
        "    vshuff32x4 $238, %%zmm0, %%zmm1, %%zmm13\n"
        "    vfmadd231ps %%zmm22, %%zmm2, %%zmm24\n"
        "    vfnmadd231ps %%zmm22, %%zmm14, %%zmm25\n"
        "    vshuff32x4 $68, %%zmm21, %%zmm17, %%zmm2\n"
        "    vmulps %%zmm6, %%zmm10, %%zmm14\n"
        "    vmulps %%zmm6, %%zmm11, %%zmm17\n"
        "    vfmadd231ps %%zmm22, %%zmm12, %%zmm20\n"
        "    vshuff32x4 $68, %%zmm23, %%zmm18, %%zmm12\n"
        "    vshuff32x4 $68, %%zmm0, %%zmm1, %%zmm0\n"
        "    vshuff32x4 $68, %%zmm5, %%zmm3, %%zmm1\n"
        "    vaddps %%zmm15, %%zmm8, %%zmm18\n"
        "    vsubps %%zmm15, %%zmm8, %%zmm8\n"
        "    vshuff32x4 $238, %%zmm5, %%zmm3, %%zmm3\n"
        "    vaddps %%zmm24, %%zmm4, %%zmm5\n"
        "    vsubps %%zmm24, %%zmm4, %%zmm4\n"
        "    vmulps %%zmm6, %%zmm13, %%zmm15\n"
        "    vmulps %%zmm6, %%zmm3, %%zmm6\n"
        "    vfnmadd231ps %%zmm16, %%zmm11, %%zmm14\n"
        "    vfmadd231ps %%zmm16, %%zmm10, %%zmm17\n"
        "    vaddps %%zmm20, %%zmm7, %%zmm10\n"
        "    vsubps %%zmm25, %%zmm9, %%zmm11\n"
        "    vsubps %%zmm20, %%zmm7, %%zmm7\n"
        "    vaddps %%zmm25, %%zmm9, %%zmm9\n"
        "    vmovups 576(%[consts]), %%zmm19\n"
        "    vmovups 640(%[consts]), %%zmm20\n"
        "    vfnmadd231ps %%zmm16, %%zmm3, %%zmm15\n"
        "    vfmadd231ps %%zmm16, %%zmm13, %%zmm6\n"
        "    vaddps %%zmm14, %%zmm2, %%zmm3\n"
        "    vaddps %%zmm17, %%zmm12, %%zmm13\n"
        "    vsubps %%zmm14, %%zmm2, %%zmm2\n"
        "    vsubps %%zmm17, %%zmm12, %%zmm12\n"
        "    vaddps %%zmm6, %%zmm0, %%zmm14\n"
        "    vsubps %%zmm15, %%zmm1, %%zmm16\n"
        "    vsubps %%zmm6, %%zmm0, %%zmm0\n"
        "    vaddps %%zmm15, %%zmm1, %%zmm1\n"
        "    vmovups 704(%[consts]), %%zmm6\n"
        "    vshuff32x4 $221, %%zmm3, %%zmm18, %%zmm15\n"
        "    vshuff32x4 $221, %%zmm13, %%zmm5, %%zmm17\n"
        "    vmovups 768(%[consts]), %%zmm21\n"
        "    vshuff32x4 $221, %%zmm2, %%zmm8, %%zmm22\n"
        "    vshuff32x4 $221, %%zmm12, %%zmm4, %%zmm23\n"
        "    vmulps %%zmm19, %%zmm15, %%zmm24\n"
        "    vmulps %%zmm19, %%zmm17, %%zmm25\n"
        "    vmulps %%zmm19, %%zmm22, %%zmm26\n"
        "    vmulps %%zmm19, %%zmm23, %%zmm19\n"
        "    vshuff32x4 $221, %%zmm14, %%zmm10, %%zmm27\n"
        "    vshuff32x4 $221, %%zmm16, %%zmm11, %%zmm28\n"
        "    vshuff32x4 $136, %%zmm3, %%zmm18, %%zmm3\n"
        "    vshuff32x4 $136, %%zmm13, %%zmm5, %%zmm5\n"
        "    vshuff32x4 $136, %%zmm2, %%zmm8, %%zmm2\n"
        "    vshuff32x4 $136, %%zmm12, %%zmm4, %%zmm4\n"
        "    vmulps %%zmm6, %%zmm27, %%zmm8\n"
        "    vmulps %%zmm6, %%zmm28, %%zmm12\n"
        "    vfnmadd231ps %%zmm20, %%zmm17, %%zmm24\n"
        "    vshuff32x4 $221, %%zmm0, %%zmm7, %%zmm13\n"
        "    vfmadd231ps %%zmm20, %%zmm15, %%zmm25\n"
        "    vfnmadd231ps %%zmm20, %%zmm23, %%zmm26\n"
        "    vfmadd231ps %%zmm20, %%zmm22, %%zmm19\n"
        "    vshuff32x4 $136, %%zmm14, %%zmm10, %%zmm10\n"
        "    vshuff32x4 $136, %%zmm16, %%zmm11, %%zmm11\n"
        "    vshuff32x4 $136, %%zmm0, %%zmm7, %%zmm0\n"
        "    vshuff32x4 $136, %%zmm1, %%zmm9, %%zmm7\n"
        "    vfnmadd231ps %%zmm21, %%zmm28, %%zmm8\n"
        "    vfmadd231ps %%zmm21, %%zmm27, %%zmm12\n"
        "    vaddps %%zmm24, %%zmm3, %%zmm14\n"
        "    vshuff32x4 $221, %%zmm1, %%zmm9, %%zmm1\n"
        "    vaddps %%zmm25, %%zmm5, %%zmm9\n"
        "    vsubps %%zmm24, %%zmm3, %%zmm3\n"
        "    vmulps %%zmm6, %%zmm13, %%zmm15\n"
        "    vmulps %%zmm6, %%zmm1, %%zmm6\n"
        "    vsubps %%zmm25, %%zmm5, %%zmm5\n"
        "    vaddps %%zmm19, %%zmm2, %%zmm16\n"
        "    vsubps %%zmm26, %%zmm4, %%zmm17\n"
        "    vsubps %%zmm19, %%zmm2, %%zmm2\n"
        "    vaddps %%zmm26, %%zmm4, %%zmm4\n"
        "    vaddps %%zmm8, %%zmm10, %%zmm18\n"
        "    vaddps %%zmm12, %%zmm11, %%zmm19\n"
        "    vmovups 832(%[consts]), %%zmm20\n"
        "    vmovups 896(%[consts]), %%zmm22\n"
        "    vsubps %%zmm8, %%zmm10, %%zmm8\n"
        "    vsubps %%zmm12, %%zmm11, %%zmm10\n"
        "    vfnmadd231ps %%zmm21, %%zmm1, %%zmm15\n"
        "    vfmadd231ps %%zmm21, %%zmm13, %%zmm6\n"
        "    vunpckhpd %%zmm18, %%zmm14, %%zmm1\n"
        "    vunpckhpd %%zmm19, %%zmm9, %%zmm11\n"
        "    vunpcklpd %%zmm18, %%zmm14, %%zmm12\n"
        "    vunpckhpd %%zmm8, %%zmm3, %%zmm13\n"
        "    vunpckhpd %%zmm10, %%zmm5, %%zmm14\n"
        "    vmulps %%zmm20, %%zmm1, %%zmm18\n"
        "    vaddps %%zmm6, %%zmm0, %%zmm21\n"
        "    vsubps %%zmm15, %%zmm7, %%zmm23\n"
        "    vsubps %%zmm6, %%zmm0, %%zmm0\n"
        "    vaddps %%zmm15, %%zmm7, %%zmm6\n"
        "    vmovups 960(%[consts]), %%zmm7\n"
        "    vmovups 1024(%[consts]), %%zmm15\n"
        "    vmulps %%zmm20, %%zmm11, %%zmm24\n"
        "    vmulps %%zmm20, %%zmm13, %%zmm25\n"
        "    vmulps %%zmm20, %%zmm14, %%zmm20\n"
        "    vunpcklpd %%zmm19, %%zmm9, %%zmm9\n"
        "    vunpcklpd %%zmm8, %%zmm3, %%zmm3\n"
        "    vfnmadd231ps %%zmm22, %%zmm11, %%zmm18\n"
        "    vunpcklpd %%zmm10, %%zmm5, %%zmm5\n"
        "    vunpckhpd %%zmm21, %%zmm16, %%zmm8\n"
        "    vunpckhpd %%zmm23, %%zmm17, %%zmm10\n"
        "    vunpckhpd %%zmm0, %%zmm2, %%zmm11\n"
        "    vfmadd231ps %%zmm22, %%zmm1, %%zmm24\n"
        "    vfnmadd231ps %%zmm22, %%zmm14, %%zmm25\n"
        "    vunpcklpd %%zmm21, %%zmm16, %%zmm1\n"
        "    vmulps %%zmm7, %%zmm8, %%zmm14\n"
        "    vmulps %%zmm7, %%zmm10, %%zmm16\n"
        "    vfmadd231ps %%zmm22, %%zmm13, %%zmm20\n"
        "    vunpcklpd %%zmm23, %%zmm17, %%zmm13\n"
        "    vunpcklpd %%zmm0, %%zmm2, %%zmm0\n"
        "    vunpcklpd %%zmm6, %%zmm4, %%zmm2\n"
        "    vaddps %%zmm18, %%zmm12, %%zmm17\n"
        "    vsubps %%zmm18, %%zmm12, %%zmm12\n"
        "    vunpckhpd %%zmm6, %%zmm4, %%zmm4\n"
        "    vaddps %%zmm24, %%zmm9, %%zmm6\n"
        "    vsubps %%zmm24, %%zmm9, %%zmm9\n"
        "    vmulps %%zmm7, %%zmm11, %%zmm18\n"
        "    vmulps %%zmm7, %%zmm4, %%zmm7\n"
        "    vfnmadd231ps %%zmm15, %%zmm10, %%zmm14\n"
        "    vfmadd231ps %%zmm15, %%zmm8, %%zmm16\n"
        "    vaddps %%zmm20, %%zmm3, %%zmm8\n"
        "    vsubps %%zmm25, %%zmm5, %%zmm10\n"
        "    vsubps %%zmm20, %%zmm3, %%zmm3\n"
        "    vaddps %%zmm25, %%zmm5, %%zmm5\n"
        "    vunpcklps %%zmm6, %%zmm17, %%zmm19\n"
        "    vunpckhps %%zmm6, %%zmm17, %%zmm6\n"
        "    vunpcklps %%zmm9, %%zmm12, %%zmm17\n"
        "    vfnmadd231ps %%zmm15, %%zmm4, %%zmm18\n"
        "    vfmadd231ps %%zmm15, %%zmm11, %%zmm7\n"
        "    vaddps %%zmm14, %%zmm1, %%zmm4\n"
        "    vmovups %%zmm19, (%[out])\n"
        "    vmovups %%zmm6, 64(%[out])\n"
        "    vmovups %%zmm17, 512(%[out])\n"
        "    vaddps %%zmm16, %%zmm13, %%zmm6\n"
        "    vsubps %%zmm14, %%zmm1, %%zmm1\n"
        "    vsubps %%zmm16, %%zmm13, %%zmm11\n"
        "    vunpckhps %%zmm9, %%zmm12, %%zmm9\n"
        "    vunpcklps %%zmm10, %%zmm8, %%zmm12\n"
        "    vunpckhps %%zmm10, %%zmm8, %%zmm8\n"
        "    vunpcklps %%zmm5, %%zmm3, %%zmm10\n"
        "    vunpckhps %%zmm5, %%zmm3, %%zmm3\n"
        "    vmovups %%zmm9, 576(%[out])\n"
        "    vmovups %%zmm12, 256(%[out])\n"
        "    vmovups %%zmm8, 320(%[out])\n"
        "    vaddps %%zmm7, %%zmm0, %%zmm5\n"
        "    vsubps %%zmm18, %%zmm2, %%zmm8\n"
        "    vsubps %%zmm7, %%zmm0, %%zmm0\n"
        "    vmovups %%zmm10, 768(%[out])\n"
        "    vmovups %%zmm3, 832(%[out])\n"
        "    vaddps %%zmm18, %%zmm2, %%zmm2\n"
        "    vunpcklps %%zmm6, %%zmm4, %%zmm3\n"
        "    vunpckhps %%zmm6, %%zmm4, %%zmm4\n"
        "    vunpcklps %%zmm11, %%zmm1, %%zmm6\n"
        "    vunpckhps %%zmm11, %%zmm1, %%zmm1\n"
        "    vmovups %%zmm3, 128(%[out])\n"
        "    vmovups %%zmm4, 192(%[out])\n"
        "    vmovups %%zmm6, 640(%[out])\n"
        "    vmovups %%zmm1, 704(%[out])\n"
        "    vunpcklps %%zmm8, %%zmm5, %%zmm1\n"
        "    vunpckhps %%zmm8, %%zmm5, %%zmm3\n"
        "    vunpcklps %%zmm2, %%zmm0, %%zmm4\n"
        "    vunpckhps %%zmm2, %%zmm0, %%zmm0\n"
        "    vmovups %%zmm1, 384(%[out])\n"
        "    vmovups %%zmm3, 448(%[out])\n"
        "    vmovups %%zmm4, 896(%[out])\n"
        "    vmovups %%zmm0, 960(%[out])\n"
        "    vzeroupper\n"
        : [in] "+r"(in), [out] "+r"(out), [consts] "+r"(consts)
        :
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "xmm16", "xmm17", "xmm18", "xmm19", "xmm20", "xmm21", "xmm22", "xmm23", "xmm24", "xmm25", "xmm26", "xmm27", "xmm28", "cc", "memory");
}

static const float onefft_avx512_f8i_consts[4] ONEFFT_ALIGNED(64) = {
    0x1.0000000000000p+1f, 0x1.6a09e667f3bcdp-1f, 0x0.0p+0f, 0x0.0p+0f,
};
/* First pass, radix 8. For each of the `count` entries (a, b) of `table`: reads 8 interleaved
 * vectors from in + 8*a + u*in_stride, transforms across them and writes output q as a split
 * vector to out + 8*b + r*out_stride, where q = low_bits(r). */
__attribute__((target("avx512f"))) static void onefft_avx512_f8i(const float *in, float *out, const uint32_t *table, size_t count, size_t in_stride, size_t out_stride) {
    const float *src;
    float *dst;
    size_t a;
    size_t b;
    const float *consts = onefft_avx512_f8i_consts;
    __asm__ volatile(
        "    vbroadcastss (%[consts]), %%zmm0\n"
        "    vbroadcastss 4(%[consts]), %%zmm1\n"
        "1:\n"
        "    movl (%[table]), %k[a]\n"
        "    movl 4(%[table]), %k[b]\n"
        "    lea 8(%[table]), %[table]\n"
        "    lea (%[in],%[a],8), %[src]\n"
        "    lea (%[out],%[b],8), %[dst]\n"
        "    vmovups (%[src]), %%zmm2\n"
        "    vshufps $136, 64(%[src]), %%zmm2, %%zmm3\n"
        "    vshufps $221, 64(%[src]), %%zmm2, %%zmm2\n"
        "    lea (%[src],%[in_stride],1), %[src]\n"
        "    vmovups (%[src]), %%zmm4\n"
        "    vshufps $136, 64(%[src]), %%zmm4, %%zmm5\n"
        "    vshufps $221, 64(%[src]), %%zmm4, %%zmm4\n"
        "    lea (%[src],%[in_stride],1), %[src]\n"
        "    vmovups (%[src]), %%zmm6\n"
        "    vshufps $136, 64(%[src]), %%zmm6, %%zmm7\n"
        "    vshufps $221, 64(%[src]), %%zmm6, %%zmm6\n"
        "    lea (%[src],%[in_stride],1), %[src]\n"
        "    vmovups (%[src]), %%zmm8\n"
        "    vshufps $136, 64(%[src]), %%zmm8, %%zmm9\n"
        "    vshufps $221, 64(%[src]), %%zmm8, %%zmm8\n"
        "    lea (%[src],%[in_stride],1), %[src]\n"
        "    vmovups (%[src]), %%zmm10\n"
        "    vshufps $136, 64(%[src]), %%zmm10, %%zmm11\n"
        "    vshufps $221, 64(%[src]), %%zmm10, %%zmm10\n"
        "    lea (%[src],%[in_stride],1), %[src]\n"
        "    vaddps %%zmm10, %%zmm2, %%zmm2\n"
        "    vaddps %%zmm11, %%zmm3, %%zmm3\n"
        "    vmovups (%[src]), %%zmm12\n"
        "    vfnmadd132ps %%zmm0, %%zmm2, %%zmm10\n"
        "    vfnmadd132ps %%zmm0, %%zmm3, %%zmm11\n"
        "    vshufps $136, 64(%[src]), %%zmm12, %%zmm13\n"
        "    vshufps $221, 64(%[src]), %%zmm12, %%zmm12\n"
        "    lea (%[src],%[in_stride],1), %[src]\n"
        "    vaddps %%zmm12, %%zmm4, %%zmm4\n"
        "    vaddps %%zmm13, %%zmm5, %%zmm5\n"
        "    vmovups (%[src]), %%zmm14\n"
        "    vfnmadd132ps %%zmm0, %%zmm4, %%zmm12\n"
        "    vfnmadd132ps %%zmm0, %%zmm5, %%zmm13\n"
        "    vshufps $136, 64(%[src]), %%zmm14, %%zmm15\n"
        "    vshufps $221, 64(%[src]), %%zmm14, %%zmm14\n"
        "    lea (%[src],%[in_stride],1), %[src]\n"
        "    vaddps %%zmm14, %%zmm6, %%zmm6\n"
        "    vaddps %%zmm15, %%zmm7, %%zmm7\n"
        "    vmovups (%[src]), %%zmm16\n"
        "    vfnmadd132ps %%zmm0, %%zmm6, %%zmm14\n"
        "    vfnmadd132ps %%zmm0, %%zmm7, %%zmm15\n"
        "    vaddps %%zmm6, %%zmm2, %%zmm2\n"
        "    vaddps %%zmm7, %%zmm3, %%zmm3\n"
        "    vshufps $136, 64(%[src]), %%zmm16, %%zmm17\n"
        "    vshufps $221, 64(%[src]), %%zmm16, %%zmm16\n"
        "    vaddps %%zmm16, %%zmm8, %%zmm8\n"
        "    vaddps %%zmm17, %%zmm9, %%zmm9\n"
        "    vfnmadd132ps %%zmm0, %%zmm2, %%zmm6\n"
        "    vaddps %%zmm15, %%zmm10, %%zmm10\n"
        "    vsubps %%zmm14, %%zmm11, %%zmm11\n"
        "    vfnmadd132ps %%zmm0, %%zmm3, %%zmm7\n"
        "    vfnmadd132ps %%zmm0, %%zmm8, %%zmm16\n"
        "    vfnmadd132ps %%zmm0, %%zmm9, %%zmm17\n"
        "    vaddps %%zmm8, %%zmm4, %%zmm4\n"
        "    vaddps %%zmm9, %%zmm5, %%zmm5\n"
        "    vfnmadd132ps %%zmm0, %%zmm10, %%zmm15\n"
        "    vfmadd132ps %%zmm0, %%zmm11, %%zmm14\n"
        "    vaddps %%zmm17, %%zmm12, %%zmm12\n"
        "    vsubps %%zmm16, %%zmm13, %%zmm13\n"
        "    vfnmadd132ps %%zmm0, %%zmm4, %%zmm8\n"
        "    vfnmadd132ps %%zmm0, %%zmm5, %%zmm9\n"
        "    vaddps %%zmm4, %%zmm2, %%zmm2\n"
        "    vaddps %%zmm5, %%zmm3, %%zmm3\n"
        "    vfnmadd132ps %%zmm0, %%zmm12, %%zmm17\n"
        "    vfmadd132ps %%zmm0, %%zmm13, %%zmm16\n"
        "    vsubps %%zmm8, %%zmm7, %%zmm7\n"
        "    vaddps %%zmm13, %%zmm12, %%zmm12\n"
        "    vaddps %%zmm9, %%zmm6, %%zmm6\n"
        "    vfnmadd132ps %%zmm0, %%zmm2, %%zmm4\n"
        "    vmovups %%zmm2, (%[dst])\n"
        "    vmovups %%zmm3, 64(%[dst])\n"
        "    vfnmadd132ps %%zmm0, %%zmm3, %%zmm5\n"
        "    lea (%[dst],%[out_stride],1), %[dst]\n"
        "    vaddps %%zmm16, %%zmm17, %%zmm17\n"
        "    vfmadd132ps %%zmm0, %%zmm7, %%zmm8\n"
        "    vmovups %%zmm7, 64(%[dst])\n"
        "    vfmsub132ps %%zmm0, %%zmm12, %%zmm13\n"
        "    vfnmadd132ps %%zmm0, %%zmm6, %%zmm9\n"
        "    vmovups %%zmm6, (%[dst])\n"
        "    vmulps %%zmm1, %%zmm12, %%zmm12\n"
        "    lea (%[dst],%[out_stride],1), %[dst]\n"
        "    vmovups %%zmm4, (%[dst])\n"
        "    vmovups %%zmm5, 64(%[dst])\n"
        "    vfmsub132ps %%zmm0, %%zmm17, %%zmm16\n"
        "    lea (%[dst],%[out_stride],1), %[dst]\n"
        "    vmulps %%zmm1, %%zmm13, %%zmm13\n"
        "    vmulps %%zmm1, %%zmm17, %%zmm17\n"
        "    vmovups %%zmm9, (%[dst])\n"
        "    vmovups %%zmm8, 64(%[dst])\n"
        "    vaddps %%zmm12, %%zmm10, %%zmm10\n"
        "    lea (%[dst],%[out_stride],1), %[dst]\n"
        "    vmulps %%zmm1, %%zmm16, %%zmm16\n"
        "    vaddps %%zmm13, %%zmm11, %%zmm11\n"
        "    vsubps %%zmm17, %%zmm14, %%zmm14\n"
        "    vfnmadd132ps %%zmm0, %%zmm10, %%zmm12\n"
        "    vmovups %%zmm10, (%[dst])\n"
        "    vaddps %%zmm16, %%zmm15, %%zmm15\n"
        "    vfnmadd132ps %%zmm0, %%zmm11, %%zmm13\n"
        "    vfmadd132ps %%zmm0, %%zmm14, %%zmm17\n"
        "    vmovups %%zmm11, 64(%[dst])\n"
        "    lea (%[dst],%[out_stride],1), %[dst]\n"
        "    vmovups %%zmm14, 64(%[dst])\n"
        "    vfnmadd132ps %%zmm0, %%zmm15, %%zmm16\n"
        "    vmovups %%zmm15, (%[dst])\n"
        "    lea (%[dst],%[out_stride],1), %[dst]\n"
        "    vmovups %%zmm12, (%[dst])\n"
        "    vmovups %%zmm13, 64(%[dst])\n"
        "    lea (%[dst],%[out_stride],1), %[dst]\n"
        "    vmovups %%zmm16, (%[dst])\n"
        "    vmovups %%zmm17, 64(%[dst])\n"
        "    sub $1, %[count]\n"
        "    jnz 1b\n"
        "    vzeroupper\n"
        : [in] "+r"(in), [out] "+r"(out), [table] "+r"(table), [count] "+r"(count), [in_stride] "+r"(in_stride), [out_stride] "+r"(out_stride), [src] "=&r"(src), [dst] "=&r"(dst), [a] "=&r"(a), [b] "=&r"(b), [consts] "+r"(consts)
        :
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "xmm16", "xmm17", "cc", "memory");
}

static const float onefft_avx512_f16i_consts[7] ONEFFT_ALIGNED(64) = {
    0x1.0000000000000p+1f, 0x1.975f5e0553158p-3f, 0x1.a827999fcef32p-2f, 0x1.561b82ab7f990p-1f,
    0x1.87de2a6aea963p-2f, 0x1.6a09e667f3bccp-1f, 0x1.d906bcf328d46p-1f,
};
/* First pass, radix 16. For each of the `count` entries (a, b) of `table`: reads 16 interleaved
 * vectors from in + 8*a + u*in_stride, transforms across them and writes output q as a split
 * vector to out + 8*b + r*in_stride, where q = low_bits(r). (The strides of input and output are
 * the same.) */
__attribute__((target("avx512f"))) static void onefft_avx512_f16i(const float *in, float *out, const uint32_t *table, size_t count, size_t in_stride, size_t out_stride) {
    (void)out_stride;
    float *p0;
    float *p1;
    float *p2;
    float *p3;
    size_t stride3;
    size_t b;
    const float *consts = onefft_avx512_f16i_consts;
    __asm__ volatile(
        "    lea (%[in_stride],%[in_stride],2), %[stride3]\n"
        "1:\n"
        "    movl (%[table]), %k[p0]\n"
        "    movl 4(%[table]), %k[b]\n"
        "    lea 8(%[table]), %[table]\n"
        "    lea (%[in],%[p0],8), %[p0]\n"
        "    vbroadcastss (%[consts]), %%zmm0\n"
        "    lea (%[p0],%[in_stride],1), %[p1]\n"
        "    lea (%[p0],%[in_stride],2), %[p2]\n"
        "    lea (%[p0],%[stride3],1), %[p3]\n"
        "    vmovups (%[p0]), %%zmm1\n"
        "    vmovups (%[p0],%[in_stride],4), %%zmm2\n"
        "    vshufps $136, 64(%[p0]), %%zmm1, %%zmm3\n"
        "    vshufps $221, 64(%[p0]), %%zmm1, %%zmm1\n"
        "    vmovups (%[p0],%[in_stride],8), %%zmm4\n"
        "    vshufps $136, 64(%[p0],%[in_stride],4), %%zmm2, %%zmm5\n"
        "    vshufps $221, 64(%[p0],%[in_stride],4), %%zmm2, %%zmm2\n"
        "    vmovups (%[p0],%[stride3],4), %%zmm6\n"
        "    vshufps $136, 64(%[p0],%[in_stride],8), %%zmm4, %%zmm7\n"
        "    vshufps $221, 64(%[p0],%[in_stride],8), %%zmm4, %%zmm4\n"
        "    vshufps $136, 64(%[p0],%[stride3],4), %%zmm6, %%zmm8\n"
        "    vshufps $221, 64(%[p0],%[stride3],4), %%zmm6, %%zmm6\n"
        "    vaddps %%zmm4, %%zmm1, %%zmm1\n"
        "    vaddps %%zmm7, %%zmm3, %%zmm3\n"
        "    vaddps %%zmm6, %%zmm2, %%zmm2\n"
        "    vaddps %%zmm8, %%zmm5, %%zmm5\n"
        "    vfnmadd132ps %%zmm0, %%zmm1, %%zmm4\n"
        "    vfnmadd132ps %%zmm0, %%zmm3, %%zmm7\n"
        "    vfnmadd132ps %%zmm0, %%zmm2, %%zmm6\n"
        "    vfnmadd132ps %%zmm0, %%zmm5, %%zmm8\n"
        "    vaddps %%zmm2, %%zmm1, %%zmm1\n"
        "    vaddps %%zmm5, %%zmm3, %%zmm3\n"
        "    vaddps %%zmm8, %%zmm4, %%zmm4\n"
        "    vsubps %%zmm6, %%zmm7, %%zmm7\n"
        "    vfnmadd132ps %%zmm0, %%zmm1, %%zmm2\n"
        "    vfnmadd132ps %%zmm0, %%zmm3, %%zmm5\n"
        "    vmovups (%[p0],%[in_stride],1), %%zmm9\n"
        "    vfnmadd132ps %%zmm0, %%zmm4, %%zmm8\n"
        "    vfmadd132ps %%zmm0, %%zmm7, %%zmm6\n"
        "    vmovups (%[p1],%[in_stride],4), %%zmm10\n"
        "    vshufps $136, 64(%[p0],%[in_stride],1), %%zmm9, %%zmm11\n"
        "    vshufps $221, 64(%[p0],%[in_stride],1), %%zmm9, %%zmm9\n"
        "    vmovups (%[p1],%[in_stride],8), %%zmm12\n"
        "    vshufps $136, 64(%[p1],%[in_stride],4), %%zmm10, %%zmm13\n"
        "    vshufps $221, 64(%[p1],%[in_stride],4), %%zmm10, %%zmm10\n"
        "    vmovups (%[p1],%[stride3],4), %%zmm14\n"
        "    vshufps $136, 64(%[p1],%[in_stride],8), %%zmm12, %%zmm15\n"
        "    vshufps $221, 64(%[p1],%[in_stride],8), %%zmm12, %%zmm12\n"
        "    vaddps %%zmm12, %%zmm9, %%zmm9\n"
        "    vaddps %%zmm15, %%zmm11, %%zmm11\n"
        "    vshufps $136, 64(%[p1],%[stride3],4), %%zmm14, %%zmm16\n"
        "    vshufps $221, 64(%[p1],%[stride3],4), %%zmm14, %%zmm14\n"
        "    vaddps %%zmm14, %%zmm10, %%zmm10\n"
        "    vaddps %%zmm16, %%zmm13, %%zmm13\n"
        "    vfnmadd132ps %%zmm0, %%zmm9, %%zmm12\n"
        "    vfnmadd132ps %%zmm0, %%zmm11, %%zmm15\n"
        "    vfnmadd132ps %%zmm0, %%zmm10, %%zmm14\n"
        "    vfnmadd132ps %%zmm0, %%zmm13, %%zmm16\n"
        "    vaddps %%zmm10, %%zmm9, %%zmm9\n"
        "    vaddps %%zmm13, %%zmm11, %%zmm11\n"
        "    vfnmadd132ps %%zmm0, %%zmm9, %%zmm10\n"
        "    vaddps %%zmm16, %%zmm12, %%zmm12\n"
        "    vsubps %%zmm14, %%zmm15, %%zmm15\n"
        "    vfnmadd132ps %%zmm0, %%zmm11, %%zmm13\n"
        "    vmovups (%[p0],%[in_stride],2), %%zmm17\n"
        "    vfnmadd132ps %%zmm0, %%zmm12, %%zmm16\n"
        "    vfmadd132ps %%zmm0, %%zmm15, %%zmm14\n"
        "    vmovups (%[p0],%[stride3],2), %%zmm18\n"
        "    vshufps $136, 64(%[p0],%[in_stride],2), %%zmm17, %%zmm19\n"
        "    vshufps $221, 64(%[p0],%[in_stride],2), %%zmm17, %%zmm17\n"
        "    vmovups (%[p2],%[in_stride],8), %%zmm20\n"
        "    vshufps $136, 64(%[p0],%[stride3],2), %%zmm18, %%zmm21\n"
        "    vshufps $221, 64(%[p0],%[stride3],2), %%zmm18, %%zmm18\n"
        "    vmovups (%[p2],%[stride3],4), %%zmm22\n"
        "    vshufps $136, 64(%[p2],%[in_stride],8), %%zmm20, %%zmm23\n"
        "    vshufps $221, 64(%[p2],%[in_stride],8), %%zmm20, %%zmm20\n"
        "    vaddps %%zmm20, %%zmm17, %%zmm17\n"
        "    vaddps %%zmm23, %%zmm19, %%zmm19\n"
        "    vshufps $136, 64(%[p2],%[stride3],4), %%zmm22, %%zmm24\n"
        "    vshufps $221, 64(%[p2],%[stride3],4), %%zmm22, %%zmm22\n"
        "    vaddps %%zmm22, %%zmm18, %%zmm18\n"
        "    vaddps %%zmm24, %%zmm21, %%zmm21\n"
        "    vfnmadd132ps %%zmm0, %%zmm17, %%zmm20\n"
        "    vfnmadd132ps %%zmm0, %%zmm19, %%zmm23\n"
        "    vfnmadd132ps %%zmm0, %%zmm18, %%zmm22\n"
        "    vfnmadd132ps %%zmm0, %%zmm21, %%zmm24\n"
        "    vaddps %%zmm18, %%zmm17, %%zmm17\n"
        "    vaddps %%zmm21, %%zmm19, %%zmm19\n"
        "    vaddps %%zmm24, %%zmm20, %%zmm20\n"
        "    vsubps %%zmm22, %%zmm23, %%zmm23\n"
        "    vfnmadd132ps %%zmm0, %%zmm17, %%zmm18\n"
        "    vfnmadd132ps %%zmm0, %%zmm19, %%zmm21\n"
        "    vmovups (%[p0],%[stride3],1), %%zmm25\n"
        "    vfnmadd132ps %%zmm0, %%zmm20, %%zmm24\n"
        "    vfmadd132ps %%zmm0, %%zmm23, %%zmm22\n"
        "    vmovups (%[p1],%[stride3],2), %%zmm0\n"
        "    vshufps $136, 64(%[p0],%[stride3],1), %%zmm25, %%zmm26\n"
        "    vshufps $221, 64(%[p0],%[stride3],1), %%zmm25, %%zmm25\n"
        "    vmovups (%[p3],%[in_stride],8), %%zmm27\n"
        "    vshufps $136, 64(%[p1],%[stride3],2), %%zmm0, %%zmm28\n"
        "    vshufps $221, 64(%[p1],%[stride3],2), %%zmm0, %%zmm0\n"
        "    vmovups (%[p3],%[stride3],4), %%zmm29\n"
        "    vshufps $136, 64(%[p3],%[in_stride],8), %%zmm27, %%zmm30\n"
        "    vshufps $221, 64(%[p3],%[in_stride],8), %%zmm27, %%zmm27\n"
        "    vaddps %%zmm27, %%zmm25, %%zmm25\n"
        "    vaddps %%zmm30, %%zmm26, %%zmm26\n"
        "    vshufps $136, 64(%[p3],%[stride3],4), %%zmm29, %%zmm31\n"
        "    vshufps $221, 64(%[p3],%[stride3],4), %%zmm29, %%zmm29\n"
        "    vaddps %%zmm29, %%zmm0, %%zmm0\n"
        "    vaddps %%zmm31, %%zmm28, %%zmm28\n"
        "    vfnmadd132ps (%[consts])%{1to16%}, %%zmm25, %%zmm27\n"
        "    vfnmadd132ps (%[consts])%{1to16%}, %%zmm26, %%zmm30\n"
        "    vfnmadd132ps (%[consts])%{1to16%}, %%zmm0, %%zmm29\n"
        "    vfnmadd132ps (%[consts])%{1to16%}, %%zmm28, %%zmm31\n"
        "    vaddps %%zmm0, %%zmm25, %%zmm25\n"
        "    vaddps %%zmm28, %%zmm26, %%zmm26\n"
        "    vfnmadd132ps (%[consts])%{1to16%}, %%zmm25, %%zmm0\n"
        "    vaddps %%zmm31, %%zmm27, %%zmm27\n"
        "    vsubps %%zmm29, %%zmm30, %%zmm30\n"
        "    vfnmadd132ps (%[consts])%{1to16%}, %%zmm26, %%zmm28\n"
        "    lea (%[out],%[b],8), %[p0]\n"
        "    lea (%[p0],%[in_stride],1), %[p1]\n"
        "    lea (%[p0],%[in_stride],2), %[p2]\n"
        "    vfnmadd132ps (%[consts])%{1to16%}, %%zmm27, %%zmm31\n"
        "    vfmadd132ps (%[consts])%{1to16%}, %%zmm30, %%zmm29\n"
        "    vaddps %%zmm17, %%zmm1, %%zmm1\n"
        "    vaddps %%zmm19, %%zmm3, %%zmm3\n"
        "    lea (%[p0],%[stride3],1), %[p3]\n"
        "    vaddps %%zmm25, %%zmm9, %%zmm9\n"
        "    vaddps %%zmm26, %%zmm11, %%zmm11\n"
        "    vfnmadd132ps (%[consts])%{1to16%}, %%zmm1, %%zmm17\n"
        "    vfnmadd132ps (%[consts])%{1to16%}, %%zmm3, %%zmm19\n"
        "    vfnmadd132ps (%[consts])%{1to16%}, %%zmm9, %%zmm25\n"
        "    vfnmadd132ps (%[consts])%{1to16%}, %%zmm11, %%zmm26\n"
        "    vaddps %%zmm9, %%zmm1, %%zmm1\n"
        "    vaddps %%zmm11, %%zmm3, %%zmm3\n"
        "    vaddps %%zmm26, %%zmm17, %%zmm17\n"
        "    vsubps %%zmm25, %%zmm19, %%zmm19\n"
        "    vfnmadd132ps (%[consts])%{1to16%}, %%zmm1, %%zmm9\n"
        "    vfnmadd132ps (%[consts])%{1to16%}, %%zmm3, %%zmm11\n"
        "    vmovups %%zmm1, (%[p0])\n"
        "    vmovups %%zmm3, 64(%[p0])\n"
        "    vfnmadd132ps (%[consts])%{1to16%}, %%zmm17, %%zmm26\n"
        "    vfmadd132ps (%[consts])%{1to16%}, %%zmm19, %%zmm25\n"
        "    vmovups %%zmm17, (%[p0],%[in_stride],4)\n"
        "    vmovups %%zmm19, 64(%[p0],%[in_stride],4)\n"
        "    vmovups %%zmm9, (%[p0],%[in_stride],1)\n"
        "    vmovups %%zmm11, 64(%[p0],%[in_stride],1)\n"
        "    vbroadcastss 4(%[consts]), %%zmm1\n"
        "    vbroadcastss 16(%[consts]), %%zmm3\n"
        "    vbroadcastss (%[consts]), %%zmm9\n"
        "    vmovups %%zmm26, (%[p1],%[in_stride],4)\n"
        "    vmovups %%zmm25, 64(%[p1],%[in_stride],4)\n"
        "    vbroadcastss 8(%[consts]), %%zmm11\n"
        "    vbroadcastss 12(%[consts]), %%zmm17\n"
        "    vbroadcastss 20(%[consts]), %%zmm19\n"
        "    vbroadcastss 24(%[consts]), %%zmm25\n"
        "    vfmadd231ps %%zmm1, %%zmm15, %%zmm12\n"
        "    vfmadd231ps %%zmm11, %%zmm23, %%zmm20\n"
        "    vfnmadd231ps %%zmm3, %%zmm12, %%zmm15\n"
        "    vfmadd231ps %%zmm17, %%zmm30, %%zmm27\n"
        "    vfnmadd231ps %%zmm19, %%zmm20, %%zmm23\n"
        "    vfmadd231ps %%zmm1, %%zmm15, %%zmm12\n"
        "    vfnmadd231ps %%zmm25, %%zmm27, %%zmm30\n"
        "    vfmadd231ps %%zmm11, %%zmm23, %%zmm20\n"
        "    vaddps %%zmm23, %%zmm7, %%zmm7\n"
        "    vfmadd231ps %%zmm17, %%zmm30, %%zmm27\n"
        "    vaddps %%zmm20, %%zmm4, %%zmm4\n"
        "    vaddps %%zmm30, %%zmm15, %%zmm15\n"
        "    vfnmadd132ps %%zmm9, %%zmm7, %%zmm23\n"
        "    vaddps %%zmm27, %%zmm12, %%zmm12\n"
        "    vfnmadd132ps %%zmm9, %%zmm4, %%zmm20\n"
        "    vfnmadd132ps %%zmm9, %%zmm15, %%zmm30\n"
        "    vaddps %%zmm15, %%zmm7, %%zmm7\n"
        "    vfnmadd132ps %%zmm9, %%zmm12, %%zmm27\n"
        "    vaddps %%zmm12, %%zmm4, %%zmm4\n"
        "    vaddps %%zmm30, %%zmm20, %%zmm20\n"
        "    vfnmadd132ps %%zmm9, %%zmm7, %%zmm15\n"
        "    vsubps %%zmm27, %%zmm23, %%zmm23\n"
        "    vfnmadd132ps %%zmm9, %%zmm4, %%zmm12\n"
        "    vmovups %%zmm4, (%[p0],%[in_stride],8)\n"
        "    vmovups %%zmm7, 64(%[p0],%[in_stride],8)\n"
        "    vfnmadd132ps %%zmm9, %%zmm20, %%zmm30\n"
        "    vmovups %%zmm20, (%[p0],%[stride3],4)\n"
        "    vfmadd132ps %%zmm9, %%zmm23, %%zmm27\n"
        "    vmovups %%zmm23, 64(%[p0],%[stride3],4)\n"
        "    vfmadd231ps %%zmm11, %%zmm13, %%zmm10\n"
        "    vmovups %%zmm12, (%[p1],%[in_stride],8)\n"
        "    vmovups %%zmm15, 64(%[p1],%[in_stride],8)\n"
        "    vmovups %%zmm30, (%[p1],%[stride3],4)\n"
        "    vfmadd231ps %%zmm11, %%zmm28, %%zmm0\n"
        "    vmovups %%zmm27, 64(%[p1],%[stride3],4)\n"
        "    vfnmadd231ps %%zmm19, %%zmm10, %%zmm13\n"
        "    vaddps %%zmm21, %%zmm2, %%zmm2\n"
        "    vfnmadd231ps %%zmm19, %%zmm0, %%zmm28\n"
        "    vfmadd231ps %%zmm11, %%zmm13, %%zmm10\n"
        "    vsubps %%zmm18, %%zmm5, %%zmm5\n"
        "    vfnmadd132ps %%zmm9, %%zmm2, %%zmm21\n"
        "    vfmadd231ps %%zmm11, %%zmm28, %%zmm0\n"
        "    vaddps %%zmm28, %%zmm10, %%zmm10\n"
        "    vfmadd132ps %%zmm9, %%zmm5, %%zmm18\n"
        "    vsubps %%zmm0, %%zmm13, %%zmm13\n"
        "    vfnmadd132ps %%zmm9, %%zmm10, %%zmm28\n"
        "    vaddps %%zmm10, %%zmm2, %%zmm2\n"
        "    vfmadd132ps %%zmm9, %%zmm13, %%zmm0\n"
        "    vaddps %%zmm13, %%zmm5, %%zmm5\n"
        "    vsubps %%zmm28, %%zmm18, %%zmm18\n"
        "    vfnmadd132ps %%zmm9, %%zmm2, %%zmm10\n"
        "    vaddps %%zmm0, %%zmm21, %%zmm21\n"
        "    vfnmadd132ps %%zmm9, %%zmm5, %%zmm13\n"
        "    vfmadd132ps %%zmm9, %%zmm18, %%zmm28\n"
        "    vmovups %%zmm2, (%[p0],%[in_stride],2)\n"
        "    vmovups %%zmm5, 64(%[p0],%[in_stride],2)\n"
        "    vfnmadd132ps %%zmm9, %%zmm21, %%zmm0\n"
        "    vmovups %%zmm21, (%[p0],%[stride3],2)\n"
        "    vmovups %%zmm18, 64(%[p0],%[stride3],2)\n"
        "    vmovups %%zmm10, (%[p0],%[stride3],1)\n"
        "    vmovups %%zmm13, 64(%[p0],%[stride3],1)\n"
        "    vmovups %%zmm28, 64(%[p1],%[stride3],2)\n"
        "    vfmadd231ps %%zmm17, %%zmm14, %%zmm16\n"
        "    vmovups %%zmm0, (%[p1],%[stride3],2)\n"
        "    vfmadd231ps %%zmm11, %%zmm22, %%zmm24\n"
        "    vfnmadd231ps %%zmm25, %%zmm16, %%zmm14\n"
        "    vfmadd231ps %%zmm1, %%zmm29, %%zmm31\n"
        "    vfnmadd231ps %%zmm19, %%zmm24, %%zmm22\n"
        "    vfmadd231ps %%zmm17, %%zmm14, %%zmm16\n"
        "    vfnmadd231ps %%zmm3, %%zmm31, %%zmm29\n"
        "    vfmadd231ps %%zmm11, %%zmm22, %%zmm24\n"
        "    vaddps %%zmm22, %%zmm8, %%zmm0\n"
        "    vsubps %%zmm22, %%zmm8, %%zmm2\n"
        "    vfmadd231ps %%zmm1, %%zmm29, %%zmm31\n"
        "    vsubps %%zmm29, %%zmm14, %%zmm1\n"
        "    vsubps %%zmm24, %%zmm6, %%zmm3\n"
        "    vaddps %%zmm24, %%zmm6, %%zmm4\n"
        "    vaddps %%zmm29, %%zmm14, %%zmm5\n"
        "    vsubps %%zmm31, %%zmm16, %%zmm6\n"
        "    vaddps %%zmm31, %%zmm16, %%zmm7\n"
        "    vaddps %%zmm1, %%zmm3, %%zmm8\n"
        "    vsubps %%zmm1, %%zmm3, %%zmm1\n"
        "    vaddps %%zmm5, %%zmm2, %%zmm3\n"
        "    vaddps %%zmm6, %%zmm0, %%zmm9\n"
        "    vsubps %%zmm6, %%zmm0, %%zmm0\n"
        "    vsubps %%zmm7, %%zmm4, %%zmm6\n"
        "    vsubps %%zmm5, %%zmm2, %%zmm2\n"
        "    vaddps %%zmm7, %%zmm4, %%zmm4\n"
        "    vmovups %%zmm8, 64(%[p2],%[in_stride],8)\n"
        "    vmovups %%zmm3, (%[p2],%[stride3],4)\n"
        "    vmovups %%zmm9, (%[p2],%[in_stride],8)\n"
        "    vmovups %%zmm0, (%[p3],%[in_stride],8)\n"
        "    vmovups %%zmm6, 64(%[p2],%[stride3],4)\n"
        "    vmovups %%zmm1, 64(%[p3],%[in_stride],8)\n"
        "    vmovups %%zmm2, (%[p3],%[stride3],4)\n"
        "    vmovups %%zmm4, 64(%[p3],%[stride3],4)\n"
        "    sub $1, %[count]\n"
        "    jnz 1b\n"
        "    vzeroupper\n"
        : [in] "+r"(in), [out] "+r"(out), [table] "+r"(table), [count] "+r"(count), [in_stride] "+r"(in_stride), [p0] "=&r"(p0), [p1] "=&r"(p1), [p2] "=&r"(p2), [p3] "=&r"(p3), [stride3] "=&r"(stride3), [b] "=&r"(b), [consts] "+r"(consts)
        :
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "xmm16", "xmm17", "xmm18", "xmm19", "xmm20", "xmm21", "xmm22", "xmm23", "xmm24", "xmm25", "xmm26", "xmm27", "xmm28", "xmm29", "xmm30", "xmm31", "cc", "memory");
}

/* Last pass, in place: reads split vectors and writes the interleaved result. Butterfly
 * j < count uses the vectors at data + r*stride + 128*j. Index bits 1 and 0 are moved from the lanes
 * to the registers and transformed. The table holds per butterfly, for l = 1..3, the real and the
 * imaginary parts of the 16 twiddles of the vector with index bits (1, 0) = l. */
__attribute__((target("avx512f"))) static void onefft_avx512_lbi(float *data, const float *tw, size_t stride, size_t count, size_t regions) {
    (void)regions;
    float *third;
    __asm__ volatile(
        "    lea (%[data],%[stride],2), %[third]\n"
        "    vmovups (%[data]), %%zmm0\n"
        "    vmovups 64(%[data]), %%zmm1\n"
        "    vmovups (%[data],%[stride],1), %%zmm2\n"
        "    vmovups 64(%[data],%[stride],1), %%zmm3\n"
        "    vmovups (%[data],%[stride],2), %%zmm4\n"
        "    vmovups 64(%[data],%[stride],2), %%zmm5\n"
        "    lea (%[third],%[stride],1), %[third]\n"
        "    vmovups (%[third]), %%zmm6\n"
        "    vmovups 64(%[third]), %%zmm7\n"
        "    vmovups (%[tw]), %%zmm8\n"
        "    vshuff32x4 $136, %%zmm2, %%zmm0, %%zmm9\n"
        "    vshuff32x4 $221, %%zmm2, %%zmm0, %%zmm0\n"
        "    vshuff32x4 $136, %%zmm3, %%zmm1, %%zmm2\n"
        "    vshuff32x4 $221, %%zmm3, %%zmm1, %%zmm1\n"
        "    vmovups 64(%[tw]), %%zmm3\n"
        "    vshuff32x4 $136, %%zmm6, %%zmm4, %%zmm10\n"
        "    vshuff32x4 $221, %%zmm6, %%zmm4, %%zmm4\n"
        "    vshuff32x4 $136, %%zmm7, %%zmm5, %%zmm6\n"
        "    vmovups 128(%[tw]), %%zmm11\n"
        "    vshuff32x4 $221, %%zmm7, %%zmm5, %%zmm5\n"
        "    vunpckhps %%zmm10, %%zmm9, %%zmm7\n"
        "    vunpckhps %%zmm6, %%zmm2, %%zmm12\n"
        "    vmovups 192(%[tw]), %%zmm13\n"
        "    vunpcklps %%zmm4, %%zmm0, %%zmm14\n"
        "    vunpckhps %%zmm4, %%zmm0, %%zmm0\n"
        "    vunpcklps %%zmm5, %%zmm1, %%zmm4\n"
        "    vunpckhps %%zmm5, %%zmm1, %%zmm1\n"
        "    vmulps %%zmm8, %%zmm7, %%zmm5\n"
        "    vmulps %%zmm8, %%zmm12, %%zmm8\n"
        "    vunpcklps %%zmm10, %%zmm9, %%zmm9\n"
        "    vunpcklps %%zmm6, %%zmm2, %%zmm2\n"
        "    vmovups 256(%[tw]), %%zmm6\n"
        "    vmovups 320(%[tw]), %%zmm10\n"
        "    vmulps %%zmm11, %%zmm14, %%zmm15\n"
        "    vmulps %%zmm11, %%zmm4, %%zmm11\n"
        "    lea 384(%[tw]), %[tw]\n"
        "    vfnmadd231ps %%zmm3, %%zmm12, %%zmm5\n"
        "    vfmadd231ps %%zmm3, %%zmm7, %%zmm8\n"
        "    vmulps %%zmm6, %%zmm0, %%zmm3\n"
        "    vmulps %%zmm6, %%zmm1, %%zmm6\n"
        "    vfnmadd231ps %%zmm13, %%zmm4, %%zmm15\n"
        "    vfmadd231ps %%zmm13, %%zmm14, %%zmm11\n"
        "    vfnmadd231ps %%zmm10, %%zmm1, %%zmm3\n"
        "    vfmadd231ps %%zmm10, %%zmm0, %%zmm6\n"
        "    sub $1, %[count]\n"
        "    jz 1f\n"
        "2:\n"
        "    lea 128(%[data]), %[data]\n"
        "    lea 128(%[third]), %[third]\n"
        "    vmovups (%[data]), %%zmm0\n"
        "    vmovups 64(%[data]), %%zmm1\n"
        "    vmovups (%[data],%[stride],1), %%zmm4\n"
        "    vmovups 64(%[data],%[stride],1), %%zmm7\n"
        "    vmovups (%[data],%[stride],2), %%zmm10\n"
        "    vmovups 64(%[data],%[stride],2), %%zmm12\n"
        "    vmovups (%[third]), %%zmm13\n"
        "    vmovups 64(%[third]), %%zmm14\n"
        "    vmovups (%[tw]), %%zmm16\n"
        "    vshuff32x4 $136, %%zmm4, %%zmm0, %%zmm17\n"
        "    vshuff32x4 $221, %%zmm4, %%zmm0, %%zmm0\n"
        "    vshuff32x4 $136, %%zmm7, %%zmm1, %%zmm4\n"
        "    vshuff32x4 $221, %%zmm7, %%zmm1, %%zmm1\n"
        "    vshuff32x4 $136, %%zmm13, %%zmm10, %%zmm7\n"
        "    vshuff32x4 $221, %%zmm13, %%zmm10, %%zmm10\n"
        "    vmovups 64(%[tw]), %%zmm13\n"
        "    vmovups 128(%[tw]), %%zmm18\n"
        "    vshuff32x4 $136, %%zmm14, %%zmm12, %%zmm19\n"
        "    vshuff32x4 $221, %%zmm14, %%zmm12, %%zmm12\n"
        "    vunpckhps %%zmm7, %%zmm17, %%zmm14\n"
        "    vunpckhps %%zmm19, %%zmm4, %%zmm20\n"
        "    vunpcklps %%zmm10, %%zmm0, %%zmm21\n"
        "    vunpckhps %%zmm10, %%zmm0, %%zmm0\n"
        "    vmovups 192(%[tw]), %%zmm10\n"
        "    vunpcklps %%zmm12, %%zmm1, %%zmm22\n"
        "    vunpckhps %%zmm12, %%zmm1, %%zmm1\n"
        "    vmulps %%zmm16, %%zmm14, %%zmm12\n"
        "    vmulps %%zmm16, %%zmm20, %%zmm16\n"
        "    vunpcklps %%zmm7, %%zmm17, %%zmm7\n"
        "    vunpcklps %%zmm19, %%zmm4, %%zmm4\n"
        "    vmovups 256(%[tw]), %%zmm17\n"
        "    vmovups 320(%[tw]), %%zmm19\n"
        "    vaddps %%zmm15, %%zmm9, %%zmm23\n"
        "    vaddps %%zmm11, %%zmm2, %%zmm24\n"
        "    vmulps %%zmm18, %%zmm21, %%zmm25\n"
        "    vmulps %%zmm18, %%zmm22, %%zmm18\n"
        "    lea 384(%[tw]), %[tw]\n"
        "    vfnmadd231ps %%zmm13, %%zmm20, %%zmm12\n"
        "    vsubps %%zmm15, %%zmm9, %%zmm20\n"
        "    vfmadd231ps %%zmm13, %%zmm14, %%zmm16\n"
        "    vsubps %%zmm11, %%zmm2, %%zmm13\n"
        "    vaddps %%zmm3, %%zmm5, %%zmm14\n"
        "    vaddps %%zmm6, %%zmm8, %%zmm26\n"
        "    vsubps %%zmm3, %%zmm5, %%zmm27\n"
        "    vsubps %%zmm6, %%zmm8, %%zmm28\n"
        "    vmulps %%zmm17, %%zmm0, %%zmm29\n"
        "    vmulps %%zmm17, %%zmm1, %%zmm17\n"
        "    vfnmadd231ps %%zmm10, %%zmm22, %%zmm25\n"
        "    vfmadd231ps %%zmm10, %%zmm21, %%zmm18\n"
        "    vaddps %%zmm14, %%zmm23, %%zmm10\n"
        "    vaddps %%zmm26, %%zmm24, %%zmm21\n"
        "    vsubps %%zmm14, %%zmm23, %%zmm14\n"
        "    vsubps %%zmm26, %%zmm24, %%zmm22\n"
        "    vaddps %%zmm28, %%zmm20, %%zmm23\n"
        "    vsubps %%zmm27, %%zmm13, %%zmm24\n"
        "    vfnmadd231ps %%zmm19, %%zmm1, %%zmm29\n"
        "    vfmadd231ps %%zmm19, %%zmm0, %%zmm17\n"
        "    vsubps %%zmm28, %%zmm20, %%zmm0\n"
        "    vaddps %%zmm27, %%zmm13, %%zmm1\n"
        "    vunpcklps %%zmm10, %%zmm21, %%zmm13\n"
        "    vunpckhps %%zmm10, %%zmm21, %%zmm10\n"
        "    vmovaps %%zmm7, %%zmm9\n"
        "    vunpcklps %%zmm23, %%zmm24, %%zmm7\n"
        "    vunpckhps %%zmm23, %%zmm24, %%zmm19\n"
        "    vunpcklps %%zmm14, %%zmm22, %%zmm20\n"
        "    vmovups %%zmm13, -128(%[data])\n"
        "    vmovups %%zmm10, -64(%[data])\n"
        "    vmovaps %%zmm4, %%zmm2\n"
        "    vmovaps %%zmm12, %%zmm5\n"
        "    vunpckhps %%zmm14, %%zmm22, %%zmm4\n"
        "    vmovups %%zmm7, -128(%[data],%[stride],1)\n"
        "    vmovups %%zmm19, -64(%[data],%[stride],1)\n"
        "    vmovups %%zmm20, -128(%[data],%[stride],2)\n"
        "    vmovaps %%zmm16, %%zmm8\n"
        "    vmovaps %%zmm25, %%zmm15\n"
        "    vmovaps %%zmm18, %%zmm11\n"
        "    vmovaps %%zmm29, %%zmm3\n"
        "    vunpcklps %%zmm0, %%zmm1, %%zmm7\n"
        "    vunpckhps %%zmm0, %%zmm1, %%zmm0\n"
        "    vmovups %%zmm4, -64(%[data],%[stride],2)\n"
        "    vmovaps %%zmm17, %%zmm6\n"
        "    vmovups %%zmm7, -128(%[third])\n"
        "    vmovups %%zmm0, -64(%[third])\n"
        "    sub $1, %[count]\n"
        "    jnz 2b\n"
        "1:\n"
        "    vaddps %%zmm15, %%zmm9, %%zmm0\n"
        "    vaddps %%zmm11, %%zmm2, %%zmm1\n"
        "    vsubps %%zmm15, %%zmm9, %%zmm4\n"
        "    vsubps %%zmm11, %%zmm2, %%zmm2\n"
        "    vaddps %%zmm3, %%zmm5, %%zmm7\n"
        "    vaddps %%zmm6, %%zmm8, %%zmm9\n"
        "    vsubps %%zmm3, %%zmm5, %%zmm3\n"
        "    vsubps %%zmm6, %%zmm8, %%zmm5\n"
        "    vaddps %%zmm7, %%zmm0, %%zmm6\n"
        "    vaddps %%zmm9, %%zmm1, %%zmm8\n"
        "    vsubps %%zmm7, %%zmm0, %%zmm0\n"
        "    vsubps %%zmm9, %%zmm1, %%zmm1\n"
        "    vaddps %%zmm5, %%zmm4, %%zmm7\n"
        "    vsubps %%zmm3, %%zmm2, %%zmm9\n"
        "    vsubps %%zmm5, %%zmm4, %%zmm4\n"
        "    vaddps %%zmm3, %%zmm2, %%zmm2\n"
        "    vunpcklps %%zmm6, %%zmm8, %%zmm3\n"
        "    vunpckhps %%zmm6, %%zmm8, %%zmm5\n"
        "    vunpcklps %%zmm7, %%zmm9, %%zmm6\n"
        "    vunpckhps %%zmm7, %%zmm9, %%zmm7\n"
        "    vunpcklps %%zmm0, %%zmm1, %%zmm8\n"
        "    vmovups %%zmm3, (%[data])\n"
        "    vmovups %%zmm5, 64(%[data])\n"
        "    vunpckhps %%zmm0, %%zmm1, %%zmm0\n"
        "    vunpcklps %%zmm4, %%zmm2, %%zmm1\n"
        "    vunpckhps %%zmm4, %%zmm2, %%zmm2\n"
        "    vmovups %%zmm6, (%[data],%[stride],1)\n"
        "    vmovups %%zmm7, 64(%[data],%[stride],1)\n"
        "    vmovups %%zmm8, (%[data],%[stride],2)\n"
        "    vmovups %%zmm0, 64(%[data],%[stride],2)\n"
        "    vmovups %%zmm1, (%[third])\n"
        "    vmovups %%zmm2, 64(%[third])\n"
        "    vzeroupper\n"
        : [data] "+r"(data), [tw] "+r"(tw), [stride] "+r"(stride), [count] "+r"(count), [third] "=&r"(third)
        :
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "xmm16", "xmm17", "xmm18", "xmm19", "xmm20", "xmm21", "xmm22", "xmm23", "xmm24", "xmm25", "xmm26", "xmm27", "xmm28", "xmm29", "cc", "memory");
}

static const float onefft_avx512_n32i_consts[144] ONEFFT_ALIGNED(64) = {
    0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f,
    0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f,
    0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f,
    0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f,
    0x1.0000000000000p+0f, -0x0.0p+0f, 0x1.0000000000000p+0f, -0x0.0p+0f,
    0x1.0000000000000p+0f, -0x0.0p+0f, 0x1.0000000000000p+0f, -0x0.0p+0f,
    0x1.0000000000000p+0f, -0x0.0p+0f, 0x1.0000000000000p+0f, -0x0.0p+0f,
    0x1.0000000000000p+0f, -0x0.0p+0f, 0x1.0000000000000p+0f, -0x0.0p+0f,
    0x0.0p+0f, -0x1.0000000000000p+0f, 0x0.0p+0f, -0x1.0000000000000p+0f,
    0x0.0p+0f, -0x1.0000000000000p+0f, 0x0.0p+0f, -0x1.0000000000000p+0f,
    0x0.0p+0f, -0x1.0000000000000p+0f, 0x0.0p+0f, -0x1.0000000000000p+0f,
    0x0.0p+0f, -0x1.0000000000000p+0f, 0x0.0p+0f, -0x1.0000000000000p+0f,
    0x1.0000000000000p+0f, 0x1.6a09e667f3bcdp-1f, 0x1.0000000000000p+0f, 0x1.6a09e667f3bcdp-1f,
    0x1.0000000000000p+0f, 0x1.6a09e667f3bcdp-1f, 0x1.0000000000000p+0f, 0x1.6a09e667f3bcdp-1f,
    -0x0.0p+0f, -0x1.6a09e667f3bccp-1f, -0x0.0p+0f, -0x1.6a09e667f3bccp-1f,
    -0x0.0p+0f, -0x1.6a09e667f3bccp-1f, -0x0.0p+0f, -0x1.6a09e667f3bccp-1f,
    0x0.0p+0f, -0x1.6a09e667f3bccp-1f, 0x0.0p+0f, -0x1.6a09e667f3bccp-1f,
    0x0.0p+0f, -0x1.6a09e667f3bccp-1f, 0x0.0p+0f, -0x1.6a09e667f3bccp-1f,
    -0x1.0000000000000p+0f, -0x1.6a09e667f3bcdp-1f, -0x1.0000000000000p+0f, -0x1.6a09e667f3bcdp-1f,
    -0x1.0000000000000p+0f, -0x1.6a09e667f3bcdp-1f, -0x1.0000000000000p+0f, -0x1.6a09e667f3bcdp-1f,
    0x1.0000000000000p+0f, 0x1.d906bcf328d46p-1f, 0x1.0000000000000p+0f, 0x1.d906bcf328d46p-1f,
    0x1.6a09e667f3bcdp-1f, 0x1.87de2a6aea964p-2f, 0x1.6a09e667f3bcdp-1f, 0x1.87de2a6aea964p-2f,
    -0x0.0p+0f, -0x1.87de2a6aea962p-2f, -0x0.0p+0f, -0x1.87de2a6aea962p-2f,
    -0x1.6a09e667f3bccp-1f, -0x1.d906bcf328d46p-1f, -0x1.6a09e667f3bccp-1f, -0x1.d906bcf328d46p-1f,
    0x0.0p+0f, -0x1.87de2a6aea963p-2f, 0x0.0p+0f, -0x1.87de2a6aea963p-2f,
    -0x1.6a09e667f3bccp-1f, -0x1.d906bcf328d46p-1f, -0x1.6a09e667f3bccp-1f, -0x1.d906bcf328d46p-1f,
    -0x1.0000000000000p+0f, -0x1.d906bcf328d46p-1f, -0x1.0000000000000p+0f, -0x1.d906bcf328d46p-1f,
    -0x1.6a09e667f3bcdp-1f, -0x1.87de2a6aea965p-2f, -0x1.6a09e667f3bcdp-1f, -0x1.87de2a6aea965p-2f,
    0x1.0000000000000p+0f, 0x1.f6297cff75cb0p-1f, -0x0.0p+0f, -0x1.8f8b83c69a608p-3f,
    0x1.d906bcf328d46p-1f, 0x1.a9b66290ea1a3p-1f, -0x1.87de2a6aea962p-2f, -0x1.1c73b39ae68c6p-1f,
    0x1.6a09e667f3bcdp-1f, 0x1.1c73b39ae68c9p-1f, -0x1.6a09e667f3bccp-1f, -0x1.a9b66290ea1a4p-1f,
    0x1.87de2a6aea964p-2f, 0x1.8f8b83c69a60dp-3f, -0x1.d906bcf328d46p-1f, -0x1.f6297cff75cb0p-1f,
    0x0.0p+0f, -0x1.8f8b83c69a60ap-3f, -0x1.0000000000000p+0f, -0x1.f6297cff75cb0p-1f,
    -0x1.87de2a6aea963p-2f, -0x1.1c73b39ae68c8p-1f, -0x1.d906bcf328d46p-1f, -0x1.a9b66290ea1a5p-1f,
    -0x1.6a09e667f3bccp-1f, -0x1.a9b66290ea1a3p-1f, -0x1.6a09e667f3bcdp-1f, -0x1.1c73b39ae68c8p-1f,
    -0x1.d906bcf328d46p-1f, -0x1.f6297cff75cb0p-1f, -0x1.87de2a6aea965p-2f, -0x1.8f8b83c69a617p-3f,
};
/* Complete 32 point transform from `in` to `out`, both interleaved. */
__attribute__((target("avx512f"))) static void onefft_avx512_n32i(const float *in, float *out) {
    const float *consts = onefft_avx512_n32i_consts;
    __asm__ volatile(
        "    vmovups (%[in]), %%zmm0\n"
        "    vmovups 128(%[in]), %%zmm1\n"
        "    vmovups 64(%[consts]), %%zmm2\n"
        "    vmovups 128(%[consts]), %%zmm3\n"
        "    vmovups 192(%[consts]), %%zmm4\n"
        "    vmovups 256(%[consts]), %%zmm5\n"
        "    vmovups 320(%[consts]), %%zmm6\n"
        "    vmovups 384(%[consts]), %%zmm7\n"
        "    vmovups 448(%[consts]), %%zmm8\n"
        "    vmovups 512(%[consts]), %%zmm9\n"
        "    vshufps $136, 64(%[in]), %%zmm0, %%zmm10\n"
        "    vshufps $221, 64(%[in]), %%zmm0, %%zmm0\n"
        "    vshufps $136, 192(%[in]), %%zmm1, %%zmm11\n"
        "    vshufps $221, 192(%[in]), %%zmm1, %%zmm1\n"
        "    vaddps %%zmm11, %%zmm10, %%zmm12\n"
        "    vsubps %%zmm11, %%zmm10, %%zmm10\n"
        "    vaddps %%zmm1, %%zmm0, %%zmm11\n"
        "    vsubps %%zmm1, %%zmm0, %%zmm0\n"
        "    vunpckhps %%zmm10, %%zmm12, %%zmm1\n"
        "    vunpcklps %%zmm10, %%zmm12, %%zmm10\n"
        "    vunpckhps %%zmm0, %%zmm11, %%zmm12\n"
        "    vmulps %%zmm2, %%zmm1, %%zmm13\n"
        "    vunpcklps %%zmm0, %%zmm11, %%zmm0\n"
        "    vmulps %%zmm2, %%zmm12, %%zmm2\n"
        "    vfmadd231ps %%zmm3, %%zmm12, %%zmm13\n"
        "    vfnmadd231ps %%zmm3, %%zmm1, %%zmm2\n"
        "    vaddps %%zmm13, %%zmm10, %%zmm1\n"
        "    vsubps %%zmm13, %%zmm10, %%zmm3\n"
        "    vaddps %%zmm2, %%zmm0, %%zmm10\n"
        "    vsubps %%zmm2, %%zmm0, %%zmm0\n"
        "    vshuff32x4 $238, %%zmm3, %%zmm1, %%zmm2\n"
        "    vshuff32x4 $68, %%zmm3, %%zmm1, %%zmm1\n"
        "    vshuff32x4 $238, %%zmm0, %%zmm10, %%zmm3\n"
        "    vmulps %%zmm4, %%zmm2, %%zmm11\n"
        "    vshuff32x4 $68, %%zmm0, %%zmm10, %%zmm0\n"
        "    vmulps %%zmm4, %%zmm3, %%zmm4\n"
        "    vfmadd231ps %%zmm5, %%zmm3, %%zmm11\n"
        "    vfnmadd231ps %%zmm5, %%zmm2, %%zmm4\n"
        "    vaddps %%zmm11, %%zmm1, %%zmm2\n"
        "    vsubps %%zmm11, %%zmm1, %%zmm1\n"
        "    vaddps %%zmm4, %%zmm0, %%zmm3\n"
        "    vsubps %%zmm4, %%zmm0, %%zmm0\n"
        "    vshuff32x4 $221, %%zmm1, %%zmm2, %%zmm4\n"
        "    vshuff32x4 $136, %%zmm1, %%zmm2, %%zmm1\n"
        "    vshuff32x4 $221, %%zmm0, %%zmm3, %%zmm2\n"
        "    vmulps %%zmm6, %%zmm4, %%zmm5\n"
        "    vshuff32x4 $136, %%zmm0, %%zmm3, %%zmm0\n"
        "    vmulps %%zmm6, %%zmm2, %%zmm3\n"
        "    vfmadd231ps %%zmm7, %%zmm2, %%zmm5\n"
        "    vfnmadd231ps %%zmm7, %%zmm4, %%zmm3\n"
        "    vaddps %%zmm5, %%zmm1, %%zmm2\n"
        "    vsubps %%zmm5, %%zmm1, %%zmm1\n"
        "    vaddps %%zmm3, %%zmm0, %%zmm4\n"
        "    vsubps %%zmm3, %%zmm0, %%zmm0\n"
        "    vunpckhpd %%zmm1, %%zmm2, %%zmm3\n"
        "    vunpcklpd %%zmm1, %%zmm2, %%zmm1\n"
        "    vunpckhpd %%zmm0, %%zmm4, %%zmm2\n"
        "    vmulps %%zmm8, %%zmm3, %%zmm5\n"
        "    vunpcklpd %%zmm0, %%zmm4, %%zmm0\n"
        "    vmulps %%zmm8, %%zmm2, %%zmm4\n"
        "    vfmadd231ps %%zmm9, %%zmm2, %%zmm5\n"
        "    vfnmadd231ps %%zmm9, %%zmm3, %%zmm4\n"
        "    vaddps %%zmm5, %%zmm1, %%zmm2\n"
        "    vsubps %%zmm5, %%zmm1, %%zmm1\n"
        "    vaddps %%zmm4, %%zmm0, %%zmm3\n"
        "    vsubps %%zmm4, %%zmm0, %%zmm0\n"
        "    vunpcklps %%zmm3, %%zmm2, %%zmm4\n"
        "    vunpckhps %%zmm3, %%zmm2, %%zmm2\n"
        "    vunpcklps %%zmm0, %%zmm1, %%zmm3\n"
        "    vunpckhps %%zmm0, %%zmm1, %%zmm0\n"
        "    vmovups %%zmm4, (%[out])\n"
        "    vmovups %%zmm2, 64(%[out])\n"
        "    vmovups %%zmm3, 128(%[out])\n"
        "    vmovups %%zmm0, 192(%[out])\n"
        "    vzeroupper\n"
        : [in] "+r"(in), [out] "+r"(out), [consts] "+r"(consts)
        :
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "cc", "memory");
}

static const float onefft_avx512_n64i_consts[144] ONEFFT_ALIGNED(64) = {
    0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f,
    0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f,
    0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f,
    0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f,
    0x1.0000000000000p+0f, 0x1.6a09e667f3bcdp-1f, 0x1.0000000000000p+0f, 0x1.6a09e667f3bcdp-1f,
    0x1.0000000000000p+0f, 0x1.6a09e667f3bcdp-1f, 0x1.0000000000000p+0f, 0x1.6a09e667f3bcdp-1f,
    0x1.0000000000000p+0f, 0x1.6a09e667f3bcdp-1f, 0x1.0000000000000p+0f, 0x1.6a09e667f3bcdp-1f,
    0x1.0000000000000p+0f, 0x1.6a09e667f3bcdp-1f, 0x1.0000000000000p+0f, 0x1.6a09e667f3bcdp-1f,
    0x0.0p+0f, -0x1.6a09e667f3bccp-1f, 0x0.0p+0f, -0x1.6a09e667f3bccp-1f,
    0x0.0p+0f, -0x1.6a09e667f3bccp-1f, 0x0.0p+0f, -0x1.6a09e667f3bccp-1f,
    0x0.0p+0f, -0x1.6a09e667f3bccp-1f, 0x0.0p+0f, -0x1.6a09e667f3bccp-1f,
    0x0.0p+0f, -0x1.6a09e667f3bccp-1f, 0x0.0p+0f, -0x1.6a09e667f3bccp-1f,
    0x1.0000000000000p+0f, 0x1.d906bcf328d46p-1f, 0x1.0000000000000p+0f, 0x1.d906bcf328d46p-1f,
    0x1.0000000000000p+0f, 0x1.d906bcf328d46p-1f, 0x1.0000000000000p+0f, 0x1.d906bcf328d46p-1f,
    0x1.6a09e667f3bcdp-1f, 0x1.87de2a6aea964p-2f, 0x1.6a09e667f3bcdp-1f, 0x1.87de2a6aea964p-2f,
    0x1.6a09e667f3bcdp-1f, 0x1.87de2a6aea964p-2f, 0x1.6a09e667f3bcdp-1f, 0x1.87de2a6aea964p-2f,
    0x0.0p+0f, -0x1.87de2a6aea963p-2f, 0x0.0p+0f, -0x1.87de2a6aea963p-2f,
    0x0.0p+0f, -0x1.87de2a6aea963p-2f, 0x0.0p+0f, -0x1.87de2a6aea963p-2f,
    -0x1.6a09e667f3bccp-1f, -0x1.d906bcf328d46p-1f, -0x1.6a09e667f3bccp-1f, -0x1.d906bcf328d46p-1f,
    -0x1.6a09e667f3bccp-1f, -0x1.d906bcf328d46p-1f, -0x1.6a09e667f3bccp-1f, -0x1.d906bcf328d46p-1f,
    0x1.0000000000000p+0f, 0x1.f6297cff75cb0p-1f, 0x1.0000000000000p+0f, 0x1.f6297cff75cb0p-1f,
    0x1.d906bcf328d46p-1f, 0x1.a9b66290ea1a3p-1f, 0x1.d906bcf328d46p-1f, 0x1.a9b66290ea1a3p-1f,
    0x1.6a09e667f3bcdp-1f, 0x1.1c73b39ae68c9p-1f, 0x1.6a09e667f3bcdp-1f, 0x1.1c73b39ae68c9p-1f,
    0x1.87de2a6aea964p-2f, 0x1.8f8b83c69a60dp-3f, 0x1.87de2a6aea964p-2f, 0x1.8f8b83c69a60dp-3f,
    0x0.0p+0f, -0x1.8f8b83c69a60ap-3f, 0x0.0p+0f, -0x1.8f8b83c69a60ap-3f,
    -0x1.87de2a6aea963p-2f, -0x1.1c73b39ae68c8p-1f, -0x1.87de2a6aea963p-2f, -0x1.1c73b39ae68c8p-1f,
    -0x1.6a09e667f3bccp-1f, -0x1.a9b66290ea1a3p-1f, -0x1.6a09e667f3bccp-1f, -0x1.a9b66290ea1a3p-1f,
    -0x1.d906bcf328d46p-1f, -0x1.f6297cff75cb0p-1f, -0x1.d906bcf328d46p-1f, -0x1.f6297cff75cb0p-1f,
    0x1.0000000000000p+0f, 0x1.fd88da3d12526p-1f, 0x1.6a09e667f3bcdp-1f, 0x1.44cf325091dd6p-1f,
    0x1.f6297cff75cb0p-1f, 0x1.e9f4156c62ddap-1f, 0x1.1c73b39ae68c9p-1f, 0x1.e2b5d3806f63ep-2f,
    0x1.d906bcf328d46p-1f, 0x1.c38b2f180bdb1p-1f, 0x1.87de2a6aea964p-2f, 0x1.294062ed59f05p-2f,
    0x1.a9b66290ea1a3p-1f, 0x1.8bc806b151741p-1f, 0x1.8f8b83c69a60dp-3f, 0x1.917a6bc29b438p-4f,
    0x0.0p+0f, -0x1.917a6bc29b42cp-4f, -0x1.6a09e667f3bccp-1f, -0x1.8bc806b151741p-1f,
    -0x1.8f8b83c69a60ap-3f, -0x1.294062ed59f05p-2f, -0x1.a9b66290ea1a3p-1f, -0x1.c38b2f180bdb0p-1f,
    -0x1.87de2a6aea963p-2f, -0x1.e2b5d3806f63bp-2f, -0x1.d906bcf328d46p-1f, -0x1.e9f4156c62ddbp-1f,
    -0x1.1c73b39ae68c8p-1f, -0x1.44cf325091dd6p-1f, -0x1.f6297cff75cb0p-1f, -0x1.fd88da3d12525p-1f,
};
/* Complete 64 point transform from `in` to `out`, both interleaved. */
__attribute__((target("avx512f"))) static void onefft_avx512_n64i(const float *in, float *out) {
    const float *consts = onefft_avx512_n64i_consts;
    __asm__ volatile(
        "    vmovups (%[in]), %%zmm0\n"
        "    vmovups 128(%[in]), %%zmm1\n"
        "    vmovups 256(%[in]), %%zmm2\n"
        "    vmovups 384(%[in]), %%zmm3\n"
        "    vmovups 64(%[consts]), %%zmm4\n"
        "    vmovups 128(%[consts]), %%zmm5\n"
        "    vmovups 192(%[consts]), %%zmm6\n"
        "    vmovups 256(%[consts]), %%zmm7\n"
        "    vshufps $136, 64(%[in]), %%zmm0, %%zmm8\n"
        "    vshufps $221, 64(%[in]), %%zmm0, %%zmm0\n"
        "    vshufps $136, 192(%[in]), %%zmm1, %%zmm9\n"
        "    vshufps $221, 192(%[in]), %%zmm1, %%zmm1\n"
        "    vshufps $136, 320(%[in]), %%zmm2, %%zmm10\n"
        "    vshufps $221, 320(%[in]), %%zmm2, %%zmm2\n"
        "    vshufps $136, 448(%[in]), %%zmm3, %%zmm11\n"
        "    vshufps $221, 448(%[in]), %%zmm3, %%zmm3\n"
        "    vaddps %%zmm2, %%zmm0, %%zmm12\n"
        "    vaddps %%zmm10, %%zmm8, %%zmm13\n"
        "    vsubps %%zmm2, %%zmm0, %%zmm0\n"
        "    vsubps %%zmm10, %%zmm8, %%zmm2\n"
        "    vaddps %%zmm3, %%zmm1, %%zmm8\n"
        "    vaddps %%zmm11, %%zmm9, %%zmm10\n"
        "    vsubps %%zmm3, %%zmm1, %%zmm1\n"
        "    vsubps %%zmm11, %%zmm9, %%zmm3\n"
        "    vmovups 320(%[consts]), %%zmm9\n"
        "    vaddps %%zmm8, %%zmm12, %%zmm11\n"
        "    vaddps %%zmm10, %%zmm13, %%zmm14\n"
        "    vsubps %%zmm8, %%zmm12, %%zmm8\n"
        "    vsubps %%zmm10, %%zmm13, %%zmm10\n"
        "    vaddps %%zmm3, %%zmm0, %%zmm12\n"
        "    vsubps %%zmm1, %%zmm2, %%zmm13\n"
        "    vmovups 384(%[consts]), %%zmm15\n"
        "    vsubps %%zmm3, %%zmm0, %%zmm0\n"
        "    vaddps %%zmm1, %%zmm2, %%zmm1\n"
        "    vunpckhps %%zmm12, %%zmm11, %%zmm2\n"
        "    vunpckhps %%zmm13, %%zmm14, %%zmm3\n"
        "    vunpcklps %%zmm12, %%zmm11, %%zmm11\n"
        "    vunpckhps %%zmm0, %%zmm8, %%zmm12\n"
        "    vunpckhps %%zmm1, %%zmm10, %%zmm16\n"
        "    vmulps %%zmm4, %%zmm2, %%zmm17\n"
        "    vmulps %%zmm4, %%zmm3, %%zmm18\n"
        "    vmulps %%zmm4, %%zmm12, %%zmm19\n"
        "    vmulps %%zmm4, %%zmm16, %%zmm4\n"
        "    vunpcklps %%zmm13, %%zmm14, %%zmm13\n"
        "    vunpcklps %%zmm0, %%zmm8, %%zmm0\n"
        "    vunpcklps %%zmm1, %%zmm10, %%zmm1\n"
        "    vfnmadd231ps %%zmm5, %%zmm3, %%zmm17\n"
        "    vfmadd231ps %%zmm5, %%zmm2, %%zmm18\n"
        "    vfnmadd231ps %%zmm5, %%zmm16, %%zmm19\n"
        "    vfmadd231ps %%zmm5, %%zmm12, %%zmm4\n"
        "    vaddps %%zmm17, %%zmm11, %%zmm2\n"
        "    vsubps %%zmm17, %%zmm11, %%zmm3\n"
        "    vaddps %%zmm18, %%zmm13, %%zmm5\n"
        "    vsubps %%zmm18, %%zmm13, %%zmm8\n"
        "    vaddps %%zmm4, %%zmm0, %%zmm10\n"
        "    vsubps %%zmm19, %%zmm1, %%zmm11\n"
        "    vsubps %%zmm4, %%zmm0, %%zmm0\n"
        "    vaddps %%zmm19, %%zmm1, %%zmm1\n"
        "    vmovups 448(%[consts]), %%zmm4\n"
        "    vmovups 512(%[consts]), %%zmm12\n"
        "    vshuff32x4 $238, %%zmm10, %%zmm2, %%zmm13\n"
        "    vshuff32x4 $68, %%zmm10, %%zmm2, %%zmm2\n"
        "    vshuff32x4 $238, %%zmm11, %%zmm5, %%zmm10\n"
        "    vshuff32x4 $238, %%zmm0, %%zmm3, %%zmm14\n"
        "    vshuff32x4 $238, %%zmm1, %%zmm8, %%zmm16\n"
        "    vmulps %%zmm6, %%zmm13, %%zmm17\n"
        "    vmulps %%zmm6, %%zmm10, %%zmm18\n"
        "    vmulps %%zmm6, %%zmm14, %%zmm19\n"
        "    vmulps %%zmm6, %%zmm16, %%zmm6\n"
        "    vshuff32x4 $68, %%zmm11, %%zmm5, %%zmm5\n"
        "    vshuff32x4 $68, %%zmm0, %%zmm3, %%zmm0\n"
        "    vshuff32x4 $68, %%zmm1, %%zmm8, %%zmm1\n"
        "    vfnmadd231ps %%zmm7, %%zmm10, %%zmm17\n"
        "    vfmadd231ps %%zmm7, %%zmm13, %%zmm18\n"
        "    vfnmadd231ps %%zmm7, %%zmm16, %%zmm19\n"
        "    vfmadd231ps %%zmm7, %%zmm14, %%zmm6\n"
        "    vaddps %%zmm17, %%zmm2, %%zmm3\n"
        "    vaddps %%zmm18, %%zmm5, %%zmm7\n"
        "    vsubps %%zmm17, %%zmm2, %%zmm2\n"
        "    vsubps %%zmm18, %%zmm5, %%zmm5\n"
        "    vaddps %%zmm6, %%zmm0, %%zmm8\n"
        "    vsubps %%zmm19, %%zmm1, %%zmm10\n"
        "    vsubps %%zmm6, %%zmm0, %%zmm0\n"
        "    vaddps %%zmm19, %%zmm1, %%zmm1\n"
        "    vshuff32x4 $221, %%zmm8, %%zmm3, %%zmm6\n"
        "    vshuff32x4 $221, %%zmm10, %%zmm7, %%zmm11\n"
        "    vshuff32x4 $136, %%zmm8, %%zmm3, %%zmm3\n"
        "    vshuff32x4 $221, %%zmm0, %%zmm2, %%zmm8\n"
        "    vshuff32x4 $221, %%zmm1, %%zmm5, %%zmm13\n"
        "    vmulps %%zmm9, %%zmm6, %%zmm14\n"
        "    vmulps %%zmm9, %%zmm11, %%zmm16\n"
        "    vmulps %%zmm9, %%zmm8, %%zmm17\n"
        "    vmulps %%zmm9, %%zmm13, %%zmm9\n"
        "    vshuff32x4 $136, %%zmm10, %%zmm7, %%zmm7\n"
        "    vshuff32x4 $136, %%zmm0, %%zmm2, %%zmm0\n"
        "    vshuff32x4 $136, %%zmm1, %%zmm5, %%zmm1\n"
        "    vfnmadd231ps %%zmm15, %%zmm11, %%zmm14\n"
        "    vfmadd231ps %%zmm15, %%zmm6, %%zmm16\n"
        "    vfnmadd231ps %%zmm15, %%zmm13, %%zmm17\n"
        "    vfmadd231ps %%zmm15, %%zmm8, %%zmm9\n"
        "    vaddps %%zmm14, %%zmm3, %%zmm2\n"
        "    vsubps %%zmm14, %%zmm3, %%zmm3\n"
        "    vaddps %%zmm16, %%zmm7, %%zmm5\n"
        "    vsubps %%zmm16, %%zmm7, %%zmm6\n"
        "    vaddps %%zmm9, %%zmm0, %%zmm7\n"
        "    vsubps %%zmm17, %%zmm1, %%zmm8\n"
        "    vsubps %%zmm9, %%zmm0, %%zmm0\n"
        "    vaddps %%zmm17, %%zmm1, %%zmm1\n"
        "    vunpckhpd %%zmm7, %%zmm2, %%zmm9\n"
        "    vunpcklpd %%zmm7, %%zmm2, %%zmm2\n"
        "    vunpckhpd %%zmm8, %%zmm5, %%zmm7\n"
        "    vunpckhpd %%zmm0, %%zmm3, %%zmm10\n"
        "    vunpckhpd %%zmm1, %%zmm6, %%zmm11\n"
        "    vmulps %%zmm4, %%zmm9, %%zmm13\n"
        "    vmulps %%zmm4, %%zmm7, %%zmm14\n"
        "    vmulps %%zmm4, %%zmm10, %%zmm15\n"
        "    vmulps %%zmm4, %%zmm11, %%zmm4\n"
        "    vunpcklpd %%zmm8, %%zmm5, %%zmm5\n"
        "    vunpcklpd %%zmm0, %%zmm3, %%zmm0\n"
        "    vunpcklpd %%zmm1, %%zmm6, %%zmm1\n"
        "    vfnmadd231ps %%zmm12, %%zmm7, %%zmm13\n"
        "    vfmadd231ps %%zmm12, %%zmm9, %%zmm14\n"
        "    vfnmadd231ps %%zmm12, %%zmm11, %%zmm15\n"
        "    vfmadd231ps %%zmm12, %%zmm10, %%zmm4\n"
        "    vaddps %%zmm13, %%zmm2, %%zmm3\n"
        "    vaddps %%zmm14, %%zmm5, %%zmm6\n"
        "    vsubps %%zmm13, %%zmm2, %%zmm2\n"
        "    vsubps %%zmm14, %%zmm5, %%zmm5\n"
        "    vaddps %%zmm4, %%zmm0, %%zmm7\n"
        "    vsubps %%zmm15, %%zmm1, %%zmm8\n"
        "    vsubps %%zmm4, %%zmm0, %%zmm0\n"
        "    vaddps %%zmm15, %%zmm1, %%zmm1\n"
        "    vunpcklps %%zmm3, %%zmm6, %%zmm4\n"
        "    vunpckhps %%zmm3, %%zmm6, %%zmm3\n"
        "    vunpcklps %%zmm2, %%zmm5, %%zmm6\n"
        "    vunpckhps %%zmm2, %%zmm5, %%zmm2\n"
        "    vunpcklps %%zmm7, %%zmm8, %%zmm5\n"
        "    vmovups %%zmm4, (%[out])\n"
        "    vmovups %%zmm3, 64(%[out])\n"
        "    vunpckhps %%zmm7, %%zmm8, %%zmm3\n"
        "    vunpcklps %%zmm0, %%zmm1, %%zmm4\n"
        "    vunpckhps %%zmm0, %%zmm1, %%zmm0\n"
        "    vmovups %%zmm6, 256(%[out])\n"
        "    vmovups %%zmm2, 320(%[out])\n"
        "    vmovups %%zmm5, 128(%[out])\n"
        "    vmovups %%zmm3, 192(%[out])\n"
        "    vmovups %%zmm4, 384(%[out])\n"
        "    vmovups %%zmm0, 448(%[out])\n"
        "    vzeroupper\n"
        : [in] "+r"(in), [out] "+r"(out), [consts] "+r"(consts)
        :
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "xmm16", "xmm17", "xmm18", "xmm19", "cc", "memory");
}

static const float onefft_avx512_n128i_consts[272] ONEFFT_ALIGNED(64) = {
    0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f,
    0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f,
    0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f,
    0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f, 0x1.6a09e667f3bcdp-1f,
    0x1.0000000000000p+0f, 0x1.d906bcf328d46p-1f, 0x1.0000000000000p+0f, 0x1.d906bcf328d46p-1f,
    0x1.0000000000000p+0f, 0x1.d906bcf328d46p-1f, 0x1.0000000000000p+0f, 0x1.d906bcf328d46p-1f,
    0x1.0000000000000p+0f, 0x1.d906bcf328d46p-1f, 0x1.0000000000000p+0f, 0x1.d906bcf328d46p-1f,
    0x1.0000000000000p+0f, 0x1.d906bcf328d46p-1f, 0x1.0000000000000p+0f, 0x1.d906bcf328d46p-1f,
    0x0.0p+0f, -0x1.87de2a6aea963p-2f, 0x0.0p+0f, -0x1.87de2a6aea963p-2f,
    0x0.0p+0f, -0x1.87de2a6aea963p-2f, 0x0.0p+0f, -0x1.87de2a6aea963p-2f,
    0x0.0p+0f, -0x1.87de2a6aea963p-2f, 0x0.0p+0f, -0x1.87de2a6aea963p-2f,
    0x0.0p+0f, -0x1.87de2a6aea963p-2f, 0x0.0p+0f, -0x1.87de2a6aea963p-2f,
    0x1.6a09e667f3bcdp-1f, 0x1.87de2a6aea964p-2f, 0x1.6a09e667f3bcdp-1f, 0x1.87de2a6aea964p-2f,
    0x1.6a09e667f3bcdp-1f, 0x1.87de2a6aea964p-2f, 0x1.6a09e667f3bcdp-1f, 0x1.87de2a6aea964p-2f,
    0x1.6a09e667f3bcdp-1f, 0x1.87de2a6aea964p-2f, 0x1.6a09e667f3bcdp-1f, 0x1.87de2a6aea964p-2f,
    0x1.6a09e667f3bcdp-1f, 0x1.87de2a6aea964p-2f, 0x1.6a09e667f3bcdp-1f, 0x1.87de2a6aea964p-2f,
    -0x1.6a09e667f3bccp-1f, -0x1.d906bcf328d46p-1f, -0x1.6a09e667f3bccp-1f, -0x1.d906bcf328d46p-1f,
    -0x1.6a09e667f3bccp-1f, -0x1.d906bcf328d46p-1f, -0x1.6a09e667f3bccp-1f, -0x1.d906bcf328d46p-1f,
    -0x1.6a09e667f3bccp-1f, -0x1.d906bcf328d46p-1f, -0x1.6a09e667f3bccp-1f, -0x1.d906bcf328d46p-1f,
    -0x1.6a09e667f3bccp-1f, -0x1.d906bcf328d46p-1f, -0x1.6a09e667f3bccp-1f, -0x1.d906bcf328d46p-1f,
    0x1.0000000000000p+0f, 0x1.f6297cff75cb0p-1f, 0x1.0000000000000p+0f, 0x1.f6297cff75cb0p-1f,
    0x1.0000000000000p+0f, 0x1.f6297cff75cb0p-1f, 0x1.0000000000000p+0f, 0x1.f6297cff75cb0p-1f,
    0x1.d906bcf328d46p-1f, 0x1.a9b66290ea1a3p-1f, 0x1.d906bcf328d46p-1f, 0x1.a9b66290ea1a3p-1f,
    0x1.d906bcf328d46p-1f, 0x1.a9b66290ea1a3p-1f, 0x1.d906bcf328d46p-1f, 0x1.a9b66290ea1a3p-1f,
    0x0.0p+0f, -0x1.8f8b83c69a60ap-3f, 0x0.0p+0f, -0x1.8f8b83c69a60ap-3f,
    0x0.0p+0f, -0x1.8f8b83c69a60ap-3f, 0x0.0p+0f, -0x1.8f8b83c69a60ap-3f,
    -0x1.87de2a6aea963p-2f, -0x1.1c73b39ae68c8p-1f, -0x1.87de2a6aea963p-2f, -0x1.1c73b39ae68c8p-1f,
    -0x1.87de2a6aea963p-2f, -0x1.1c73b39ae68c8p-1f, -0x1.87de2a6aea963p-2f, -0x1.1c73b39ae68c8p-1f,
    0x1.6a09e667f3bcdp-1f, 0x1.1c73b39ae68c9p-1f, 0x1.6a09e667f3bcdp-1f, 0x1.1c73b39ae68c9p-1f,
    0x1.6a09e667f3bcdp-1f, 0x1.1c73b39ae68c9p-1f, 0x1.6a09e667f3bcdp-1f, 0x1.1c73b39ae68c9p-1f,
    0x1.87de2a6aea964p-2f, 0x1.8f8b83c69a60dp-3f, 0x1.87de2a6aea964p-2f, 0x1.8f8b83c69a60dp-3f,
    0x1.87de2a6aea964p-2f, 0x1.8f8b83c69a60dp-3f, 0x1.87de2a6aea964p-2f, 0x1.8f8b83c69a60dp-3f,
    -0x1.6a09e667f3bccp-1f, -0x1.a9b66290ea1a3p-1f, -0x1.6a09e667f3bccp-1f, -0x1.a9b66290ea1a3p-1f,
    -0x1.6a09e667f3bccp-1f, -0x1.a9b66290ea1a3p-1f, -0x1.6a09e667f3bccp-1f, -0x1.a9b66290ea1a3p-1f,
    -0x1.d906bcf328d46p-1f, -0x1.f6297cff75cb0p-1f, -0x1.d906bcf328d46p-1f, -0x1.f6297cff75cb0p-1f,
    -0x1.d906bcf328d46p-1f, -0x1.f6297cff75cb0p-1f, -0x1.d906bcf328d46p-1f, -0x1.f6297cff75cb0p-1f,
    0x1.0000000000000p+0f, 0x1.fd88da3d12526p-1f, 0x1.0000000000000p+0f, 0x1.fd88da3d12526p-1f,
    0x1.f6297cff75cb0p-1f, 0x1.e9f4156c62ddap-1f, 0x1.f6297cff75cb0p-1f, 0x1.e9f4156c62ddap-1f,
    0x1.d906bcf328d46p-1f, 0x1.c38b2f180bdb1p-1f, 0x1.d906bcf328d46p-1f, 0x1.c38b2f180bdb1p-1f,
    0x1.a9b66290ea1a3p-1f, 0x1.8bc806b151741p-1f, 0x1.a9b66290ea1a3p-1f, 0x1.8bc806b151741p-1f,
    0x0.0p+0f, -0x1.917a6bc29b42cp-4f, 0x0.0p+0f, -0x1.917a6bc29b42cp-4f,
    -0x1.8f8b83c69a60ap-3f, -0x1.294062ed59f05p-2f, -0x1.8f8b83c69a60ap-3f, -0x1.294062ed59f05p-2f,
    -0x1.87de2a6aea963p-2f, -0x1.e2b5d3806f63bp-2f, -0x1.87de2a6aea963p-2f, -0x1.e2b5d3806f63bp-2f,
    -0x1.1c73b39ae68c8p-1f, -0x1.44cf325091dd6p-1f, -0x1.1c73b39ae68c8p-1f, -0x1.44cf325091dd6p-1f,
    0x1.6a09e667f3bcdp-1f, 0x1.44cf325091dd6p-1f, 0x1.6a09e667f3bcdp-1f, 0x1.44cf325091dd6p-1f,
    0x1.1c73b39ae68c9p-1f, 0x1.e2b5d3806f63ep-2f, 0x1.1c73b39ae68c9p-1f, 0x1.e2b5d3806f63ep-2f,
    0x1.87de2a6aea964p-2f, 0x1.294062ed59f05p-2f, 0x1.87de2a6aea964p-2f, 0x1.294062ed59f05p-2f,
    0x1.8f8b83c69a60dp-3f, 0x1.917a6bc29b438p-4f, 0x1.8f8b83c69a60dp-3f, 0x1.917a6bc29b438p-4f,
    -0x1.6a09e667f3bccp-1f, -0x1.8bc806b151741p-1f, -0x1.6a09e667f3bccp-1f, -0x1.8bc806b151741p-1f,
    -0x1.a9b66290ea1a3p-1f, -0x1.c38b2f180bdb0p-1f, -0x1.a9b66290ea1a3p-1f, -0x1.c38b2f180bdb0p-1f,
    -0x1.d906bcf328d46p-1f, -0x1.e9f4156c62ddbp-1f, -0x1.d906bcf328d46p-1f, -0x1.e9f4156c62ddbp-1f,
    -0x1.f6297cff75cb0p-1f, -0x1.fd88da3d12525p-1f, -0x1.f6297cff75cb0p-1f, -0x1.fd88da3d12525p-1f,
    0x1.0000000000000p+0f, 0x1.ff621e3796d7ep-1f, 0x1.d906bcf328d46p-1f, 0x1.ced7af43cc773p-1f,
    0x1.fd88da3d12526p-1f, 0x1.fa7557f08a517p-1f, 0x1.c38b2f180bdb1p-1f, 0x1.b728345196e3ep-1f,
    0x1.f6297cff75cb0p-1f, 0x1.f0a7efb9230d7p-1f, 0x1.a9b66290ea1a3p-1f, 0x1.9b3e047f38741p-1f,
    0x1.e9f4156c62ddap-1f, 0x1.e212104f686e5p-1f, 0x1.8bc806b151741p-1f, 0x1.7b5df226aafafp-1f,
    0x0.0p+0f, -0x1.91f65f10dd814p-5f, -0x1.87de2a6aea963p-2f, -0x1.b5d1009e15cc0p-2f,
    -0x1.917a6bc29b42cp-4f, -0x1.2c8106e8e613ap-3f, -0x1.e2b5d3806f63bp-2f, -0x1.073879922ffedp-1f,
    -0x1.8f8b83c69a60ap-3f, -0x1.f19f97b215f1ap-3f, -0x1.1c73b39ae68c8p-1f, -0x1.30ff7fce17035p-1f,
    -0x1.294062ed59f05p-2f, -0x1.58f9a75ab1fddp-2f, -0x1.44cf325091dd6p-1f, -0x1.57d69348cec9fp-1f,
    0x1.6a09e667f3bcdp-1f, 0x1.57d69348cec9fp-1f, 0x1.87de2a6aea964p-2f, 0x1.58f9a75ab1fddp-2f,
    0x1.44cf325091dd6p-1f, 0x1.30ff7fce17036p-1f, 0x1.294062ed59f05p-2f, 0x1.f19f97b215f1ep-3f,
    0x1.1c73b39ae68c9p-1f, 0x1.073879922ffedp-1f, 0x1.8f8b83c69a60dp-3f, 0x1.2c8106e8e613ap-3f,
    0x1.e2b5d3806f63ep-2f, 0x1.b5d1009e15cc2p-2f, 0x1.917a6bc29b438p-4f, 0x1.91f65f10dd824p-5f,
    -0x1.6a09e667f3bccp-1f, -0x1.7b5df226aafafp-1f, -0x1.d906bcf328d46p-1f, -0x1.e212104f686e5p-1f,
    -0x1.8bc806b151741p-1f, -0x1.9b3e047f38740p-1f, -0x1.e9f4156c62ddbp-1f, -0x1.f0a7efb9230d7p-1f,
    -0x1.a9b66290ea1a3p-1f, -0x1.b728345196e3ep-1f, -0x1.f6297cff75cb0p-1f, -0x1.fa7557f08a517p-1f,
    -0x1.c38b2f180bdb0p-1f, -0x1.ced7af43cc773p-1f, -0x1.fd88da3d12525p-1f, -0x1.ff621e3796d7ep-1f,
};
/* Complete 128 point transform from `in` to `out`, both interleaved. */
__attribute__((target("avx512f"))) static void onefft_avx512_n128i(const float *in, float *out) {
    const float *consts = onefft_avx512_n128i_consts;
    __asm__ volatile(
        "    vmovups 128(%[in]), %%zmm0\n"
        "    vmovups 384(%[in]), %%zmm1\n"
        "    vmovups 640(%[in]), %%zmm2\n"
        "    vmovups 896(%[in]), %%zmm3\n"
        "    vmovups (%[in]), %%zmm4\n"
        "    vmovups 256(%[in]), %%zmm5\n"
        "    vmovups 512(%[in]), %%zmm6\n"
        "    vmovups 768(%[in]), %%zmm7\n"
        "    vbroadcastss (%[consts]), %%zmm8\n"
        "    vshufps $136, 192(%[in]), %%zmm0, %%zmm9\n"
        "    vshufps $221, 192(%[in]), %%zmm0, %%zmm0\n"
        "    vshufps $136, 448(%[in]), %%zmm1, %%zmm10\n"
        "    vshufps $221, 448(%[in]), %%zmm1, %%zmm1\n"
        "    vshufps $136, 704(%[in]), %%zmm2, %%zmm11\n"
        "    vshufps $221, 704(%[in]), %%zmm2, %%zmm2\n"
        "    vshufps $136, 960(%[in]), %%zmm3, %%zmm12\n"
        "    vshufps $221, 960(%[in]), %%zmm3, %%zmm3\n"
        "    vshufps $136, 64(%[in]), %%zmm4, %%zmm13\n"
        "    vsubps %%zmm2, %%zmm0, %%zmm14\n"
        "    vshufps $221, 64(%[in]), %%zmm4, %%zmm4\n"
        "    vshufps $136, 320(%[in]), %%zmm5, %%zmm15\n"
        "    vsubps %%zmm11, %%zmm9, %%zmm16\n"
        "    vsubps %%zmm3, %%zmm1, %%zmm17\n"
        "    vsubps %%zmm12, %%zmm10, %%zmm18\n"
        "    vshufps $221, 320(%[in]), %%zmm5, %%zmm5\n"
        "    vshufps $136, 576(%[in]), %%zmm6, %%zmm19\n"
        "    vshufps $221, 576(%[in]), %%zmm6, %%zmm6\n"
        "    vshufps $136, 832(%[in]), %%zmm7, %%zmm20\n"
        "    vshufps $221, 832(%[in]), %%zmm7, %%zmm7\n"
        "    vaddps %%zmm6, %%zmm4, %%zmm21\n"
        "    vaddps %%zmm19, %%zmm13, %%zmm22\n"
        "    vsubps %%zmm6, %%zmm4, %%zmm4\n"
        "    vsubps %%zmm19, %%zmm13, %%zmm6\n"
        "    vaddps %%zmm18, %%zmm14, %%zmm13\n"
        "    vsubps %%zmm17, %%zmm16, %%zmm19\n"
        "    vsubps %%zmm18, %%zmm14, %%zmm14\n"
        "    vaddps %%zmm17, %%zmm16, %%zmm16\n"
        "    vaddps %%zmm7, %%zmm5, %%zmm17\n"
        "    vaddps %%zmm20, %%zmm15, %%zmm18\n"
        "    vsubps %%zmm7, %%zmm5, %%zmm5\n"
        "    vsubps %%zmm20, %%zmm15, %%zmm7\n"
        "    vaddps %%zmm2, %%zmm0, %%zmm0\n"
        "    vaddps %%zmm11, %%zmm9, %%zmm2\n"
        "    vaddps %%zmm3, %%zmm1, %%zmm1\n"
        "    vaddps %%zmm12, %%zmm10, %%zmm3\n"
        "    vaddps %%zmm19, %%zmm13, %%zmm9\n"
        "    vsubps %%zmm13, %%zmm19, %%zmm10\n"
        "    vaddps %%zmm16, %%zmm14, %%zmm11\n"
        "    vsubps %%zmm14, %%zmm16, %%zmm12\n"
        "    vaddps %%zmm17, %%zmm21, %%zmm13\n"
        "    vaddps %%zmm18, %%zmm22, %%zmm14\n"
        "    vsubps %%zmm17, %%zmm21, %%zmm15\n"
        "    vsubps %%zmm18, %%zmm22, %%zmm16\n"
        "    vaddps %%zmm7, %%zmm4, %%zmm17\n"
        "    vsubps %%zmm5, %%zmm6, %%zmm18\n"
        "    vsubps %%zmm7, %%zmm4, %%zmm4\n"
        "    vaddps %%zmm5, %%zmm6, %%zmm5\n"
        "    vaddps %%zmm1, %%zmm0, %%zmm6\n"
        "    vaddps %%zmm3, %%zmm2, %%zmm7\n"
        "    vsubps %%zmm1, %%zmm0, %%zmm0\n"
        "    vsubps %%zmm3, %%zmm2, %%zmm1\n"
        "    vmulps %%zmm8, %%zmm9, %%zmm2\n"
        "    vmulps %%zmm8, %%zmm10, %%zmm3\n"
        "    vmulps %%zmm8, %%zmm11, %%zmm9\n"
        "    vmulps %%zmm8, %%zmm12, %%zmm8\n"
        "    vmovups 64(%[consts]), %%zmm10\n"
        "    vmovups 128(%[consts]), %%zmm11\n"
        "    vaddps %%zmm6, %%zmm13, %%zmm12\n"
        "    vaddps %%zmm7, %%zmm14, %%zmm19\n"
        "    vsubps %%zmm6, %%zmm13, %%zmm6\n"
        "    vsubps %%zmm7, %%zmm14, %%zmm7\n"
        "    vaddps %%zmm2, %%zmm17, %%zmm13\n"
        "    vsubps %%zmm2, %%zmm17, %%zmm2\n"
        "    vaddps %%zmm3, %%zmm18, %%zmm14\n"
        "    vsubps %%zmm3, %%zmm18, %%zmm3\n"
        "    vaddps %%zmm1, %%zmm15, %%zmm17\n"
        "    vsubps %%zmm0, %%zmm16, %%zmm18\n"
        "    vsubps %%zmm1, %%zmm15, %%zmm1\n"
        "    vaddps %%zmm0, %%zmm16, %%zmm0\n"
        "    vaddps %%zmm8, %%zmm4, %%zmm15\n"
        "    vsubps %%zmm9, %%zmm5, %%zmm16\n"
        "    vsubps %%zmm8, %%zmm4, %%zmm4\n"
        "    vaddps %%zmm9, %%zmm5, %%zmm5\n"
        "    vmovups 192(%[consts]), %%zmm8\n"
        "    vunpckhps %%zmm13, %%zmm12, %%zmm9\n"
        "    vunpckhps %%zmm2, %%zmm6, %%zmm20\n"
        "    vmovups 256(%[consts]), %%zmm21\n"
        "    vunpckhps %%zmm14, %%zmm19, %%zmm22\n"
        "    vunpckhps %%zmm3, %%zmm7, %%zmm23\n"
        "    vmulps %%zmm10, %%zmm9, %%zmm24\n"
        "    vmulps %%zmm10, %%zmm22, %%zmm25\n"
        "    vmulps %%zmm10, %%zmm20, %%zmm26\n"
        "    vmulps %%zmm10, %%zmm23, %%zmm10\n"
        "    vunpckhps %%zmm15, %%zmm17, %%zmm27\n"
        "    vunpckhps %%zmm16, %%zmm18, %%zmm28\n"
        "    vunpcklps %%zmm13, %%zmm12, %%zmm12\n"
        "    vunpcklps %%zmm14, %%zmm19, %%zmm13\n"
        "    vunpcklps %%zmm2, %%zmm6, %%zmm2\n"
        "    vunpcklps %%zmm3, %%zmm7, %%zmm3\n"
        "    vmulps %%zmm8, %%zmm27, %%zmm6\n"
        "    vmulps %%zmm8, %%zmm28, %%zmm7\n"
        "    vfnmadd231ps %%zmm11, %%zmm22, %%zmm24\n"
        "    vunpckhps %%zmm4, %%zmm1, %%zmm14\n"
        "    vfmadd231ps %%zmm11, %%zmm9, %%zmm25\n"
        "    vfnmadd231ps %%zmm11, %%zmm23, %%zmm26\n"
        "    vfmadd231ps %%zmm11, %%zmm20, %%zmm10\n"
        "    vunpcklps %%zmm15, %%zmm17, %%zmm9\n"
        "    vunpcklps %%zmm16, %%zmm18, %%zmm11\n"
        "    vunpcklps %%zmm4, %%zmm1, %%zmm1\n"
        "    vunpcklps %%zmm5, %%zmm0, %%zmm4\n"
        "    vfnmadd231ps %%zmm21, %%zmm28, %%zmm6\n"
        "    vfmadd231ps %%zmm21, %%zmm27, %%zmm7\n"
        "    vaddps %%zmm24, %%zmm12, %%zmm15\n"
        "    vunpckhps %%zmm5, %%zmm0, %%zmm0\n"
        "    vaddps %%zmm25, %%zmm13, %%zmm5\n"
        "    vsubps %%zmm24, %%zmm12, %%zmm12\n"
        "    vmulps %%zmm8, %%zmm14, %%zmm16\n"
        "    vmulps %%zmm8, %%zmm0, %%zmm8\n"
        "    vsubps %%zmm25, %%zmm13, %%zmm13\n"
        "    vaddps %%zmm10, %%zmm2, %%zmm17\n"
        "    vsubps %%zmm26, %%zmm3, %%zmm18\n"
        "    vsubps %%zmm10, %%zmm2, %%zmm2\n"
        "    vaddps %%zmm26, %%zmm3, %%zmm3\n"
        "    vaddps %%zmm6, %%zmm9, %%zmm10\n"
        "    vaddps %%zmm7, %%zmm11, %%zmm19\n"
        "    vmovups 320(%[consts]), %%zmm20\n"
        "    vmovups 384(%[consts]), %%zmm22\n"
        "    vsubps %%zmm6, %%zmm9, %%zmm6\n"
        "    vsubps %%zmm7, %%zmm11, %%zmm7\n"
        "    vfnmadd231ps %%zmm21, %%zmm0, %%zmm16\n"
        "    vfmadd231ps %%zmm21, %%zmm14, %%zmm8\n"
        "    vshuff32x4 $238, %%zmm10, %%zmm15, %%zmm0\n"
        "    vshuff32x4 $238, %%zmm19, %%zmm5, %%zmm9\n"
        "    vshuff32x4 $68, %%zmm10, %%zmm15, %%zmm10\n"
        "    vshuff32x4 $238, %%zmm6, %%zmm12, %%zmm11\n"
        "    vshuff32x4 $238, %%zmm7, %%zmm13, %%zmm14\n"
        "    vmulps %%zmm20, %%zmm0, %%zmm15\n"
        "    vaddps %%zmm8, %%zmm1, %%zmm21\n"
        "    vsubps %%zmm16, %%zmm4, %%zmm23\n"
        "    vsubps %%zmm8, %%zmm1, %%zmm1\n"
        "    vaddps %%zmm16, %%zmm4, %%zmm4\n"
        "    vmovups 448(%[consts]), %%zmm8\n"
        "    vmovups 512(%[consts]), %%zmm16\n"
        "    vmulps %%zmm20, %%zmm9, %%zmm24\n"
        "    vmulps %%zmm20, %%zmm11, %%zmm25\n"
        "    vmulps %%zmm20, %%zmm14, %%zmm20\n"
        "    vshuff32x4 $68, %%zmm19, %%zmm5, %%zmm5\n"
        "    vshuff32x4 $68, %%zmm6, %%zmm12, %%zmm6\n"
        "    vfnmadd231ps %%zmm22, %%zmm9, %%zmm15\n"
        "    vshuff32x4 $68, %%zmm7, %%zmm13, %%zmm7\n"
        "    vshuff32x4 $238, %%zmm21, %%zmm17, %%zmm9\n"
        "    vshuff32x4 $238, %%zmm23, %%zmm18, %%zmm12\n"
        "    vshuff32x4 $238, %%zmm1, %%zmm2, %%zmm13\n"
        "    vfmadd231ps %%zmm22, %%zmm0, %%zmm24\n"
        "    vfnmadd231ps %%zmm22, %%zmm14, %%zmm25\n"
        "    vshuff32x4 $68, %%zmm21, %%zmm17, %%zmm0\n"
        "    vmulps %%zmm8, %%zmm9, %%zmm14\n"
        "    vmulps %%zmm8, %%zmm12, %%zmm17\n"
        "    vfmadd231ps %%zmm22, %%zmm11, %%zmm20\n"
        "    vshuff32x4 $68, %%zmm23, %%zmm18, %%zmm11\n"
        "    vshuff32x4 $68, %%zmm1, %%zmm2, %%zmm1\n"
        "    vshuff32x4 $68, %%zmm4, %%zmm3, %%zmm2\n"
        "    vaddps %%zmm15, %%zmm10, %%zmm18\n"
        "    vsubps %%zmm15, %%zmm10, %%zmm10\n"
        "    vshuff32x4 $238, %%zmm4, %%zmm3, %%zmm3\n"
        "    vaddps %%zmm24, %%zmm5, %%zmm4\n"
        "    vsubps %%zmm24, %%zmm5, %%zmm5\n"
        "    vmulps %%zmm8, %%zmm13, %%zmm15\n"
        "    vmulps %%zmm8, %%zmm3, %%zmm8\n"
        "    vfnmadd231ps %%zmm16, %%zmm12, %%zmm14\n"
        "    vfmadd231ps %%zmm16, %%zmm9, %%zmm17\n"
        "    vaddps %%zmm20, %%zmm6, %%zmm9\n"
        "    vsubps %%zmm25, %%zmm7, %%zmm12\n"
        "    vsubps %%zmm20, %%zmm6, %%zmm6\n"
        "    vaddps %%zmm25, %%zmm7, %%zmm7\n"
        "    vmovups 576(%[consts]), %%zmm19\n"
        "    vmovups 640(%[consts]), %%zmm20\n"
        "    vfnmadd231ps %%zmm16, %%zmm3, %%zmm15\n"
        "    vfmadd231ps %%zmm16, %%zmm13, %%zmm8\n"
        "    vaddps %%zmm14, %%zmm0, %%zmm3\n"
        "    vaddps %%zmm17, %%zmm11, %%zmm13\n"
        "    vsubps %%zmm14, %%zmm0, %%zmm0\n"
        "    vsubps %%zmm17, %%zmm11, %%zmm11\n"
        "    vaddps %%zmm8, %%zmm1, %%zmm14\n"
        "    vsubps %%zmm15, %%zmm2, %%zmm16\n"
        "    vsubps %%zmm8, %%zmm1, %%zmm1\n"
        "    vaddps %%zmm15, %%zmm2, %%zmm2\n"
        "    vmovups 704(%[consts]), %%zmm8\n"
        "    vshuff32x4 $221, %%zmm3, %%zmm18, %%zmm15\n"
        "    vshuff32x4 $221, %%zmm13, %%zmm4, %%zmm17\n"
        "    vmovups 768(%[consts]), %%zmm21\n"
        "    vshuff32x4 $221, %%zmm0, %%zmm10, %%zmm22\n"
        "    vshuff32x4 $221, %%zmm11, %%zmm5, %%zmm23\n"
        "    vmulps %%zmm19, %%zmm15, %%zmm24\n"
        "    vmulps %%zmm19, %%zmm17, %%zmm25\n"
        "    vmulps %%zmm19, %%zmm22, %%zmm26\n"
        "    vmulps %%zmm19, %%zmm23, %%zmm19\n"
        "    vshuff32x4 $221, %%zmm14, %%zmm9, %%zmm27\n"
        "    vshuff32x4 $221, %%zmm16, %%zmm12, %%zmm28\n"
        "    vshuff32x4 $136, %%zmm3, %%zmm18, %%zmm3\n"
        "    vshuff32x4 $136, %%zmm13, %%zmm4, %%zmm4\n"
        "    vshuff32x4 $136, %%zmm0, %%zmm10, %%zmm0\n"
        "    vshuff32x4 $136, %%zmm11, %%zmm5, %%zmm5\n"
        "    vmulps %%zmm8, %%zmm27, %%zmm10\n"
        "    vmulps %%zmm8, %%zmm28, %%zmm11\n"
        "    vfnmadd231ps %%zmm20, %%zmm17, %%zmm24\n"
        "    vshuff32x4 $221, %%zmm1, %%zmm6, %%zmm13\n"
        "    vfmadd231ps %%zmm20, %%zmm15, %%zmm25\n"
        "    vfnmadd231ps %%zmm20, %%zmm23, %%zmm26\n"
        "    vfmadd231ps %%zmm20, %%zmm22, %%zmm19\n"
        "    vshuff32x4 $136, %%zmm14, %%zmm9, %%zmm9\n"
        "    vshuff32x4 $136, %%zmm16, %%zmm12, %%zmm12\n"
        "    vshuff32x4 $136, %%zmm1, %%zmm6, %%zmm1\n"
        "    vshuff32x4 $136, %%zmm2, %%zmm7, %%zmm6\n"
        "    vfnmadd231ps %%zmm21, %%zmm28, %%zmm10\n"
        "    vfmadd231ps %%zmm21, %%zmm27, %%zmm11\n"
        "    vaddps %%zmm24, %%zmm3, %%zmm14\n"
        "    vshuff32x4 $221, %%zmm2, %%zmm7, %%zmm2\n"
        "    vaddps %%zmm25, %%zmm4, %%zmm7\n"
        "    vsubps %%zmm24, %%zmm3, %%zmm3\n"
        "    vmulps %%zmm8, %%zmm13, %%zmm15\n"
        "    vmulps %%zmm8, %%zmm2, %%zmm8\n"
        "    vsubps %%zmm25, %%zmm4, %%zmm4\n"
        "    vaddps %%zmm19, %%zmm0, %%zmm16\n"
        "    vsubps %%zmm26, %%zmm5, %%zmm17\n"
        "    vsubps %%zmm19, %%zmm0, %%zmm0\n"
        "    vaddps %%zmm26, %%zmm5, %%zmm5\n"
        "    vaddps %%zmm10, %%zmm9, %%zmm18\n"
        "    vaddps %%zmm11, %%zmm12, %%zmm19\n"
        "    vmovups 832(%[consts]), %%zmm20\n"
        "    vmovups 896(%[consts]), %%zmm22\n"
        "    vsubps %%zmm10, %%zmm9, %%zmm9\n"
        "    vsubps %%zmm11, %%zmm12, %%zmm10\n"
        "    vfnmadd231ps %%zmm21, %%zmm2, %%zmm15\n"
        "    vfmadd231ps %%zmm21, %%zmm13, %%zmm8\n"
        "    vunpckhpd %%zmm18, %%zmm14, %%zmm2\n"
        "    vunpckhpd %%zmm19, %%zmm7, %%zmm11\n"
        "    vunpcklpd %%zmm18, %%zmm14, %%zmm12\n"
        "    vunpckhpd %%zmm9, %%zmm3, %%zmm13\n"
        "    vunpckhpd %%zmm10, %%zmm4, %%zmm14\n"
        "    vmulps %%zmm20, %%zmm2, %%zmm18\n"
        "    vaddps %%zmm8, %%zmm1, %%zmm21\n"
        "    vsubps %%zmm15, %%zmm6, %%zmm23\n"
        "    vsubps %%zmm8, %%zmm1, %%zmm1\n"
        "    vaddps %%zmm15, %%zmm6, %%zmm6\n"
        "    vmovups 960(%[consts]), %%zmm8\n"
        "    vmovups 1024(%[consts]), %%zmm15\n"
        "    vmulps %%zmm20, %%zmm11, %%zmm24\n"
        "    vmulps %%zmm20, %%zmm13, %%zmm25\n"
        "    vmulps %%zmm20, %%zmm14, %%zmm20\n"
        "    vunpcklpd %%zmm19, %%zmm7, %%zmm7\n"
        "    vunpcklpd %%zmm9, %%zmm3, %%zmm3\n"
        "    vfnmadd231ps %%zmm22, %%zmm11, %%zmm18\n"
        "    vunpcklpd %%zmm10, %%zmm4, %%zmm4\n"
        "    vunpckhpd %%zmm21, %%zmm16, %%zmm9\n"
        "    vunpckhpd %%zmm23, %%zmm17, %%zmm10\n"
        "    vunpckhpd %%zmm1, %%zmm0, %%zmm11\n"
        "    vfmadd231ps %%zmm22, %%zmm2, %%zmm24\n"
        "    vfnmadd231ps %%zmm22, %%zmm14, %%zmm25\n"
        "    vunpcklpd %%zmm21, %%zmm16, %%zmm2\n"
        "    vmulps %%zmm8, %%zmm9, %%zmm14\n"
        "    vmulps %%zmm8, %%zmm10, %%zmm16\n"
        "    vfmadd231ps %%zmm22, %%zmm13, %%zmm20\n"
        "    vunpcklpd %%zmm23, %%zmm17, %%zmm13\n"
        "    vunpcklpd %%zmm1, %%zmm0, %%zmm0\n"
        "    vunpcklpd %%zmm6, %%zmm5, %%zmm1\n"
        "    vaddps %%zmm18, %%zmm12, %%zmm17\n"
        "    vsubps %%zmm18, %%zmm12, %%zmm12\n"
        "    vunpckhpd %%zmm6, %%zmm5, %%zmm5\n"
        "    vaddps %%zmm24, %%zmm7, %%zmm6\n"
        "    vsubps %%zmm24, %%zmm7, %%zmm7\n"
        "    vmulps %%zmm8, %%zmm11, %%zmm18\n"
        "    vmulps %%zmm8, %%zmm5, %%zmm8\n"
        "    vfnmadd231ps %%zmm15, %%zmm10, %%zmm14\n"
        "    vfmadd231ps %%zmm15, %%zmm9, %%zmm16\n"
        "    vaddps %%zmm20, %%zmm3, %%zmm9\n"
        "    vsubps %%zmm25, %%zmm4, %%zmm10\n"
        "    vsubps %%zmm20, %%zmm3, %%zmm3\n"
        "    vaddps %%zmm25, %%zmm4, %%zmm4\n"
        "    vunpcklps %%zmm17, %%zmm6, %%zmm19\n"
        "    vunpckhps %%zmm17, %%zmm6, %%zmm6\n"
        "    vunpcklps %%zmm12, %%zmm7, %%zmm17\n"
        "    vfnmadd231ps %%zmm15, %%zmm5, %%zmm18\n"
        "    vfmadd231ps %%zmm15, %%zmm11, %%zmm8\n"
        "    vaddps %%zmm14, %%zmm2, %%zmm5\n"
        "    vmovups %%zmm19, (%[out])\n"
        "    vmovups %%zmm6, 64(%[out])\n"
        "    vmovups %%zmm17, 512(%[out])\n"
        "    vaddps %%zmm16, %%zmm13, %%zmm6\n"
        "    vsubps %%zmm14, %%zmm2, %%zmm2\n"
        "    vsubps %%zmm16, %%zmm13, %%zmm11\n"
        "    vunpckhps %%zmm12, %%zmm7, %%zmm7\n"
        "    vunpcklps %%zmm9, %%zmm10, %%zmm12\n"
        "    vunpckhps %%zmm9, %%zmm10, %%zmm9\n"
        "    vunpcklps %%zmm3, %%zmm4, %%zmm10\n"
        "    vunpckhps %%zmm3, %%zmm4, %%zmm3\n"
        "    vmovups %%zmm7, 576(%[out])\n"
        "    vmovups %%zmm12, 256(%[out])\n"
        "    vmovups %%zmm9, 320(%[out])\n"
        "    vaddps %%zmm8, %%zmm0, %%zmm4\n"
        "    vsubps %%zmm18, %%zmm1, %%zmm7\n"
        "    vsubps %%zmm8, %%zmm0, %%zmm0\n"
        "    vmovups %%zmm10, 768(%[out])\n"
        "    vmovups %%zmm3, 832(%[out])\n"
        "    vaddps %%zmm18, %%zmm1, %%zmm1\n"
        "    vunpcklps %%zmm5, %%zmm6, %%zmm3\n"
        "    vunpckhps %%zmm5, %%zmm6, %%zmm5\n"
        "    vunpcklps %%zmm2, %%zmm11, %%zmm6\n"
        "    vunpckhps %%zmm2, %%zmm11, %%zmm2\n"
        "    vmovups %%zmm3, 128(%[out])\n"
        "    vmovups %%zmm5, 192(%[out])\n"
        "    vmovups %%zmm6, 640(%[out])\n"
        "    vmovups %%zmm2, 704(%[out])\n"
        "    vunpcklps %%zmm4, %%zmm7, %%zmm2\n"
        "    vunpckhps %%zmm4, %%zmm7, %%zmm3\n"
        "    vunpcklps %%zmm0, %%zmm1, %%zmm4\n"
        "    vunpckhps %%zmm0, %%zmm1, %%zmm0\n"
        "    vmovups %%zmm2, 384(%[out])\n"
        "    vmovups %%zmm3, 448(%[out])\n"
        "    vmovups %%zmm4, 896(%[out])\n"
        "    vmovups %%zmm0, 960(%[out])\n"
        "    vzeroupper\n"
        : [in] "+r"(in), [out] "+r"(out), [consts] "+r"(consts)
        :
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "xmm16", "xmm17", "xmm18", "xmm19", "xmm20", "xmm21", "xmm22", "xmm23", "xmm24", "xmm25", "xmm26", "xmm27", "xmm28", "cc", "memory");
}

/* Middle pass, radix 4, in place on split vectors. `data` holds `regions` regions of `blocks`
 * blocks. Input i of butterfly j of a block is the vector at (i/2)*args.s_high + (i%2)*args.s_low
 * + j*V (V = bytes per vector), j < args.inner; see passes.py for how the blocks and regions follow
 * each other. Butterfly j multiplies input i by w_j^i and transforms across the vectors; output q
 * goes to (q%2)*s_high + (q/2)*s_low. The table `tw` holds per butterfly the real parts of w^1..w^3
 * and one unused float, then the imaginary parts. */
__attribute__((target("avx512f"))) static void onefft_avx512_c4(float *data, const float *tw, const size_t *args, size_t blocks, size_t regions) {
    float *upper;
    size_t s_low;
    const float *twp;
    size_t j;
    size_t b;
    size_t scratch;
    __asm__ volatile(
        "    mov 0(%[args]), %[s_low]\n"
        "    lea (%[data],%[s_low],1), %[upper]\n"
        "    mov 8(%[args]), %[s_low]\n"
        "1:\n"
        "    mov %[blocks], %[b]\n"
        "2:\n"
        "    mov %[tw], %[twp]\n"
        "    mov 16(%[args]), %[j]\n"
        "3:\n"
        "    vmovups (%[data],%[s_low],1), %%zmm0\n"
        "    vmovups 64(%[data],%[s_low],1), %%zmm1\n"
        "    vmovups (%[upper]), %%zmm2\n"
        "    vmovups 64(%[upper]), %%zmm3\n"
        "    vmovups (%[upper],%[s_low],1), %%zmm4\n"
        "    vmovups 64(%[upper],%[s_low],1), %%zmm5\n"
        "    vbroadcastss (%[twp]), %%zmm6\n"
        "    vbroadcastss 4(%[twp]), %%zmm7\n"
        "    vbroadcastss 8(%[twp]), %%zmm8\n"
        "    vbroadcastss 16(%[twp]), %%zmm9\n"
        "    vbroadcastss 20(%[twp]), %%zmm10\n"
        "    vbroadcastss 24(%[twp]), %%zmm11\n"
        "    vmovups (%[data]), %%zmm12\n"
        "    vmovups 64(%[data]), %%zmm13\n"
        "    lea 32(%[twp]), %[twp]\n"
        "    vmulps %%zmm6, %%zmm0, %%zmm14\n"
        "    vmulps %%zmm6, %%zmm1, %%zmm6\n"
        "    vmulps %%zmm7, %%zmm2, %%zmm15\n"
        "    vmulps %%zmm7, %%zmm3, %%zmm7\n"
        "    vmulps %%zmm8, %%zmm4, %%zmm16\n"
        "    vmulps %%zmm8, %%zmm5, %%zmm8\n"
        "    vfnmadd231ps %%zmm9, %%zmm1, %%zmm14\n"
        "    vfmadd231ps %%zmm9, %%zmm0, %%zmm6\n"
        "    vfnmadd231ps %%zmm10, %%zmm3, %%zmm15\n"
        "    vfmadd231ps %%zmm10, %%zmm2, %%zmm7\n"
        "    vfnmadd231ps %%zmm11, %%zmm5, %%zmm16\n"
        "    vfmadd231ps %%zmm11, %%zmm4, %%zmm8\n"
        "    vaddps %%zmm15, %%zmm12, %%zmm0\n"
        "    vsubps %%zmm15, %%zmm12, %%zmm1\n"
        "    vaddps %%zmm7, %%zmm13, %%zmm2\n"
        "    vsubps %%zmm7, %%zmm13, %%zmm3\n"
        "    vaddps %%zmm16, %%zmm14, %%zmm4\n"
        "    vaddps %%zmm8, %%zmm6, %%zmm5\n"
        "    vsubps %%zmm16, %%zmm14, %%zmm7\n"
        "    vsubps %%zmm8, %%zmm6, %%zmm6\n"
        "    vaddps %%zmm4, %%zmm0, %%zmm8\n"
        "    vsubps %%zmm4, %%zmm0, %%zmm0\n"
        "    vaddps %%zmm5, %%zmm2, %%zmm4\n"
        "    vsubps %%zmm5, %%zmm2, %%zmm2\n"
        "    vaddps %%zmm6, %%zmm1, %%zmm5\n"
        "    vsubps %%zmm7, %%zmm3, %%zmm9\n"
        "    vsubps %%zmm6, %%zmm1, %%zmm1\n"
        "    vaddps %%zmm7, %%zmm3, %%zmm3\n"
        "    vmovups %%zmm8, (%[data])\n"
        "    vmovups %%zmm0, (%[data],%[s_low],1)\n"
        "    vmovups %%zmm4, 64(%[data])\n"
        "    vmovups %%zmm5, (%[upper])\n"
        "    vmovups %%zmm2, 64(%[data],%[s_low],1)\n"
        "    vmovups %%zmm9, 64(%[upper])\n"
        "    vmovups %%zmm1, (%[upper],%[s_low],1)\n"
        "    vmovups %%zmm3, 64(%[upper],%[s_low],1)\n"
        "    lea 128(%[data]), %[data]\n"
        "    lea 128(%[upper]), %[upper]\n"
        "    sub $1, %[j]\n"
        "    jnz 3b\n"
        "    mov 24(%[args]), %[scratch]\n"
        "    lea (%[data],%[scratch],1), %[data]\n"
        "    lea (%[upper],%[scratch],1), %[upper]\n"
        "    sub $1, %[b]\n"
        "    jnz 2b\n"
        "    mov 32(%[args]), %[scratch]\n"
        "    lea (%[data],%[scratch],1), %[data]\n"
        "    lea (%[upper],%[scratch],1), %[upper]\n"
        "    mov 40(%[args]), %[scratch]\n"
        "    lea (%[tw],%[scratch],1), %[tw]\n"
        "    sub $1, %[regions]\n"
        "    jnz 1b\n"
        "    vzeroupper\n"
        : [data] "+r"(data), [tw] "+r"(tw), [args] "+r"(args), [blocks] "+r"(blocks), [regions] "+r"(regions), [upper] "=&r"(upper), [s_low] "=&r"(s_low), [twp] "=&r"(twp), [j] "=&r"(j), [b] "=&r"(b), [scratch] "=&r"(scratch)
        :
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "xmm16", "cc", "memory");
}

static const float onefft_avx512_c8_consts[4] ONEFFT_ALIGNED(64) = {
    0x1.0000000000000p+1f, 0x1.6a09e667f3bcdp-1f, 0x0.0p+0f, 0x0.0p+0f,
};
/* Middle pass, radix 8, in place on split vectors. `data` holds `regions` regions of `blocks`
 * blocks. Input i of butterfly j of a block is the vector at (i/4)*args.s_high + (i/2%2)*args.s_low
 * + (i%2)*2*args.s_low + 128*j, j < args.inner; see passes.py for how the blocks and regions follow
 * each other. Butterfly j multiplies input i by w_j^i and transforms across the vectors; output q
 * goes to (q%2)*s_high + (q/2%2)*s_low + (q/4)*2*s_low. The table `tw` holds per butterfly the real
 * parts of w^1..w^7 and one unused float, then the imaginary parts. */
__attribute__((target("avx512f"))) static void onefft_avx512_c8(float *data, const float *tw, const size_t *args, size_t blocks, size_t regions) {
    float *upper;
    size_t s_low;
    size_t s_low3;
    const float *twp;
    size_t j;
    size_t b;
    size_t scratch;
    const float *consts = onefft_avx512_c8_consts;
    __asm__ volatile(
        "    vbroadcastss (%[consts]), %%zmm0\n"
        "    vbroadcastss 4(%[consts]), %%zmm1\n"
        "    mov 0(%[args]), %[s_low]\n"
        "    lea (%[data],%[s_low],1), %[upper]\n"
        "    mov 8(%[args]), %[s_low]\n"
        "    lea (%[s_low],%[s_low],2), %[s_low3]\n"
        "1:\n"
        "    mov %[blocks], %[b]\n"
        "2:\n"
        "    mov %[tw], %[twp]\n"
        "    mov 16(%[args]), %[j]\n"
        "3:\n"
        "    vmovups (%[data],%[s_low],2), %%zmm2\n"
        "    vmovups 64(%[data],%[s_low],2), %%zmm3\n"
        "    vmovups (%[data],%[s_low3],1), %%zmm4\n"
        "    vmovups 64(%[data],%[s_low3],1), %%zmm5\n"
        "    vmovups (%[upper],%[s_low],2), %%zmm6\n"
        "    vmovups 64(%[upper],%[s_low],2), %%zmm7\n"
        "    vmovups (%[upper],%[s_low3],1), %%zmm8\n"
        "    vmovups 64(%[upper],%[s_low3],1), %%zmm9\n"
        "    vbroadcastss (%[twp]), %%zmm10\n"
        "    vmovups (%[data],%[s_low],1), %%zmm11\n"
        "    vmovups 64(%[data],%[s_low],1), %%zmm12\n"
        "    vmovups (%[upper]), %%zmm13\n"
        "    vmovups 64(%[upper]), %%zmm14\n"
        "    vmovups (%[upper],%[s_low],1), %%zmm15\n"
        "    vmovups 64(%[upper],%[s_low],1), %%zmm16\n"
        "    vmovups (%[data]), %%zmm17\n"
        "    vmovups 64(%[data]), %%zmm18\n"
        "    vbroadcastss 8(%[twp]), %%zmm19\n"
        "    vbroadcastss 32(%[twp]), %%zmm20\n"
        "    vbroadcastss 40(%[twp]), %%zmm21\n"
        "    vbroadcastss 4(%[twp]), %%zmm22\n"
        "    vbroadcastss 36(%[twp]), %%zmm23\n"
        "    vbroadcastss 12(%[twp]), %%zmm24\n"
        "    vmulps %%zmm10, %%zmm2, %%zmm25\n"
        "    vmulps %%zmm10, %%zmm3, %%zmm10\n"
        "    vmulps %%zmm19, %%zmm4, %%zmm26\n"
        "    vmulps %%zmm19, %%zmm5, %%zmm19\n"
        "    vbroadcastss 44(%[twp]), %%zmm27\n"
        "    vmulps %%zmm22, %%zmm11, %%zmm28\n"
        "    vmulps %%zmm22, %%zmm12, %%zmm22\n"
        "    vmulps %%zmm24, %%zmm13, %%zmm29\n"
        "    vfnmadd231ps %%zmm20, %%zmm3, %%zmm25\n"
        "    vfmadd231ps %%zmm20, %%zmm2, %%zmm10\n"
        "    vbroadcastss 16(%[twp]), %%zmm2\n"
        "    vbroadcastss 48(%[twp]), %%zmm3\n"
        "    vfnmadd231ps %%zmm21, %%zmm5, %%zmm26\n"
        "    vfmadd231ps %%zmm21, %%zmm4, %%zmm19\n"
        "    vmulps %%zmm24, %%zmm14, %%zmm4\n"
        "    vfnmadd231ps %%zmm23, %%zmm12, %%zmm28\n"
        "    vfmadd231ps %%zmm23, %%zmm11, %%zmm22\n"
        "    vfnmadd231ps %%zmm27, %%zmm14, %%zmm29\n"
        "    vbroadcastss 24(%[twp]), %%zmm5\n"
        "    vbroadcastss 56(%[twp]), %%zmm11\n"
        "    vbroadcastss 20(%[twp]), %%zmm12\n"
        "    vbroadcastss 52(%[twp]), %%zmm14\n"
        "    vfmadd231ps %%zmm27, %%zmm13, %%zmm4\n"
        "    vmulps %%zmm2, %%zmm6, %%zmm13\n"
        "    vmulps %%zmm2, %%zmm7, %%zmm2\n"
        "    vaddps %%zmm29, %%zmm17, %%zmm20\n"
        "    vsubps %%zmm29, %%zmm17, %%zmm17\n"
        "    vmulps %%zmm5, %%zmm8, %%zmm21\n"
        "    vmulps %%zmm5, %%zmm9, %%zmm5\n"
        "    vmulps %%zmm12, %%zmm15, %%zmm23\n"
        "    vmulps %%zmm12, %%zmm16, %%zmm12\n"
        "    vaddps %%zmm4, %%zmm18, %%zmm24\n"
        "    vfnmadd231ps %%zmm3, %%zmm7, %%zmm13\n"
        "    vfmadd231ps %%zmm3, %%zmm6, %%zmm2\n"
        "    vsubps %%zmm4, %%zmm18, %%zmm3\n"
        "    vfnmadd231ps %%zmm11, %%zmm9, %%zmm21\n"
        "    vfmadd231ps %%zmm11, %%zmm8, %%zmm5\n"
        "    vfnmadd231ps %%zmm14, %%zmm16, %%zmm23\n"
        "    vfmadd231ps %%zmm14, %%zmm15, %%zmm12\n"
        "    vsubps %%zmm13, %%zmm25, %%zmm4\n"
        "    vsubps %%zmm2, %%zmm10, %%zmm6\n"
        "    vaddps %%zmm13, %%zmm25, %%zmm7\n"
        "    vaddps %%zmm2, %%zmm10, %%zmm2\n"
        "    vsubps %%zmm21, %%zmm26, %%zmm8\n"
        "    vsubps %%zmm5, %%zmm19, %%zmm9\n"
        "    vaddps %%zmm23, %%zmm28, %%zmm10\n"
        "    vaddps %%zmm12, %%zmm22, %%zmm11\n"
        "    vsubps %%zmm23, %%zmm28, %%zmm13\n"
        "    vsubps %%zmm12, %%zmm22, %%zmm12\n"
        "    vaddps %%zmm21, %%zmm26, %%zmm14\n"
        "    vaddps %%zmm5, %%zmm19, %%zmm5\n"
        "    vaddps %%zmm9, %%zmm4, %%zmm4\n"
        "    vsubps %%zmm8, %%zmm6, %%zmm6\n"
        "    vaddps %%zmm10, %%zmm20, %%zmm20\n"
        "    vaddps %%zmm11, %%zmm24, %%zmm24\n"
        "    vaddps %%zmm12, %%zmm17, %%zmm17\n"
        "    vsubps %%zmm13, %%zmm3, %%zmm3\n"
        "    vaddps %%zmm14, %%zmm7, %%zmm7\n"
        "    vaddps %%zmm5, %%zmm2, %%zmm2\n"
        "    vfnmadd132ps %%zmm0, %%zmm4, %%zmm9\n"
        "    vfmadd132ps %%zmm0, %%zmm6, %%zmm8\n"
        "    vaddps %%zmm6, %%zmm4, %%zmm15\n"
        "    vsubps %%zmm4, %%zmm6, %%zmm4\n"
        "    vfnmadd132ps %%zmm0, %%zmm20, %%zmm10\n"
        "    vfnmadd132ps %%zmm0, %%zmm24, %%zmm11\n"
        "    vfnmadd132ps %%zmm0, %%zmm17, %%zmm12\n"
        "    vfmadd132ps %%zmm0, %%zmm3, %%zmm13\n"
        "    vfnmadd132ps %%zmm0, %%zmm7, %%zmm14\n"
        "    vfnmadd132ps %%zmm0, %%zmm2, %%zmm5\n"
        "    vaddps %%zmm7, %%zmm20, %%zmm6\n"
        "    vaddps %%zmm2, %%zmm24, %%zmm16\n"
        "    vaddps %%zmm8, %%zmm9, %%zmm18\n"
        "    vsubps %%zmm9, %%zmm8, %%zmm8\n"
        "    vmulps %%zmm1, %%zmm15, %%zmm9\n"
        "    vmulps %%zmm1, %%zmm4, %%zmm4\n"
        "    vsubps %%zmm7, %%zmm20, %%zmm7\n"
        "    vsubps %%zmm2, %%zmm24, %%zmm2\n"
        "    vsubps %%zmm14, %%zmm11, %%zmm15\n"
        "    vaddps %%zmm14, %%zmm11, %%zmm11\n"
        "    vaddps %%zmm5, %%zmm10, %%zmm14\n"
        "    vsubps %%zmm5, %%zmm10, %%zmm5\n"
        "    vmulps %%zmm1, %%zmm18, %%zmm10\n"
        "    vmulps %%zmm1, %%zmm8, %%zmm8\n"
        "    vaddps %%zmm9, %%zmm17, %%zmm18\n"
        "    vaddps %%zmm4, %%zmm3, %%zmm19\n"
        "    vsubps %%zmm9, %%zmm17, %%zmm9\n"
        "    vsubps %%zmm4, %%zmm3, %%zmm3\n"
        "    vmovups %%zmm6, (%[data])\n"
        "    vmovups %%zmm16, 64(%[data])\n"
        "    vmovups %%zmm15, 64(%[data],%[s_low],1)\n"
        "    vmovups %%zmm7, (%[data],%[s_low],2)\n"
        "    vmovups %%zmm2, 64(%[data],%[s_low],2)\n"
        "    vmovups %%zmm14, (%[data],%[s_low],1)\n"
        "    vmovups %%zmm5, (%[data],%[s_low3],1)\n"
        "    vaddps %%zmm8, %%zmm12, %%zmm2\n"
        "    vsubps %%zmm10, %%zmm13, %%zmm4\n"
        "    vsubps %%zmm8, %%zmm12, %%zmm5\n"
        "    vmovups %%zmm18, (%[upper])\n"
        "    vaddps %%zmm10, %%zmm13, %%zmm6\n"
        "    vmovups %%zmm19, 64(%[upper])\n"
        "    vmovups %%zmm9, (%[upper],%[s_low],2)\n"
        "    vmovups %%zmm3, 64(%[upper],%[s_low],2)\n"
        "    vmovups %%zmm11, 64(%[data],%[s_low3],1)\n"
        "    lea 128(%[data]), %[data]\n"
        "    lea 64(%[twp]), %[twp]\n"
        "    vmovups %%zmm2, (%[upper],%[s_low],1)\n"
        "    vmovups %%zmm4, 64(%[upper],%[s_low],1)\n"
        "    vmovups %%zmm5, (%[upper],%[s_low3],1)\n"
        "    vmovups %%zmm6, 64(%[upper],%[s_low3],1)\n"
        "    lea 128(%[upper]), %[upper]\n"
        "    sub $1, %[j]\n"
        "    jnz 3b\n"
        "    mov 24(%[args]), %[scratch]\n"
        "    lea (%[data],%[scratch],1), %[data]\n"
        "    lea (%[upper],%[scratch],1), %[upper]\n"
        "    sub $1, %[b]\n"
        "    jnz 2b\n"
        "    mov 32(%[args]), %[scratch]\n"
        "    lea (%[data],%[scratch],1), %[data]\n"
        "    lea (%[upper],%[scratch],1), %[upper]\n"
        "    mov 40(%[args]), %[scratch]\n"
        "    lea (%[tw],%[scratch],1), %[tw]\n"
        "    sub $1, %[regions]\n"
        "    jnz 1b\n"
        "    vzeroupper\n"
        : [data] "+r"(data), [tw] "+r"(tw), [args] "+r"(args), [blocks] "+r"(blocks), [regions] "+r"(regions), [upper] "=&r"(upper), [s_low] "=&r"(s_low), [s_low3] "=&r"(s_low3), [twp] "=&r"(twp), [j] "=&r"(j), [b] "=&r"(b), [scratch] "=&r"(scratch), [consts] "+r"(consts)
        :
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "xmm16", "xmm17", "xmm18", "xmm19", "xmm20", "xmm21", "xmm22", "xmm23", "xmm24", "xmm25", "xmm26", "xmm27", "xmm28", "xmm29", "cc", "memory");
}

/* Pass before the last, in place on split vectors. Butterfly j < count of a region uses the
 * vectors at data + u*stride + 128*j, u < 8; the `regions` regions follow each other. Index bit 4 selects u%2 and is transformed first. Index bits 3
 * and 2 are moved from the lanes to the registers and transformed. See the table builder in
 * onefft.c for the twiddle layout. */
__attribute__((target("avx512f"))) static void onefft_avx512_la3(float *data, const float *tw, size_t stride, size_t count, size_t regions) {
    float *third;
    size_t region_skip;
    float *fifth;
    size_t j;
    __asm__ volatile(
        "    mov %[stride], %[region_skip]\n"
        "    lea (%[data],%[stride],2), %[third]\n"
        "    lea (%[data],%[stride],4), %[fifth]\n"
        "    shl $3, %[region_skip]\n"
        "    lea (%[third],%[stride],1), %[third]\n"
        "    lea (%[fifth],%[stride],1), %[fifth]\n"
        "    sub %[stride], %[region_skip]\n"
        "1:\n"
        "    mov %[count], %[j]\n"
        "2:\n"
        "    vmovups (%[data],%[stride],1), %%zmm0\n"
        "    vmovups 64(%[data],%[stride],1), %%zmm1\n"
        "    vmovups (%[third]), %%zmm2\n"
        "    vmovups 64(%[third]), %%zmm3\n"
        "    vmovups (%[fifth]), %%zmm4\n"
        "    vmovups 64(%[fifth]), %%zmm5\n"
        "    vmovups (%[third],%[stride],4), %%zmm6\n"
        "    vmovups 64(%[third],%[stride],4), %%zmm7\n"
        "    vbroadcastss (%[tw]), %%zmm8\n"
        "    vbroadcastss 4(%[tw]), %%zmm9\n"
        "    vbroadcastss 8(%[tw]), %%zmm10\n"
        "    vbroadcastss 12(%[tw]), %%zmm11\n"
        "    vbroadcastss 16(%[tw]), %%zmm12\n"
        "    vbroadcastss 20(%[tw]), %%zmm13\n"
        "    vbroadcastss 24(%[tw]), %%zmm14\n"
        "    vbroadcastss 28(%[tw]), %%zmm15\n"
        "    vmovups (%[data]), %%zmm16\n"
        "    vmovups 64(%[data]), %%zmm17\n"
        "    vmovups (%[data],%[stride],2), %%zmm18\n"
        "    vmovups 64(%[data],%[stride],2), %%zmm19\n"
        "    vmovups (%[data],%[stride],4), %%zmm20\n"
        "    vmovups 64(%[data],%[stride],4), %%zmm21\n"
        "    vmovups (%[fifth],%[stride],1), %%zmm22\n"
        "    vmovups 64(%[fifth],%[stride],1), %%zmm23\n"
        "    vmovups 32(%[tw]), %%zmm24\n"
        "    vmovups 160(%[tw]), %%zmm25\n"
        "    vmovups 96(%[tw]), %%zmm26\n"
        "    vmovups 224(%[tw]), %%zmm27\n"
        "    vmulps %%zmm8, %%zmm0, %%zmm28\n"
        "    vmulps %%zmm8, %%zmm1, %%zmm8\n"
        "    vmulps %%zmm9, %%zmm2, %%zmm29\n"
        "    vmulps %%zmm9, %%zmm3, %%zmm9\n"
        "    vmulps %%zmm10, %%zmm4, %%zmm30\n"
        "    vmulps %%zmm10, %%zmm5, %%zmm10\n"
        "    vmulps %%zmm11, %%zmm6, %%zmm31\n"
        "    vmulps %%zmm11, %%zmm7, %%zmm11\n"
        "    vfnmadd231ps %%zmm12, %%zmm1, %%zmm28\n"
        "    vfmadd231ps %%zmm12, %%zmm0, %%zmm8\n"
        "    vfnmadd231ps %%zmm13, %%zmm3, %%zmm29\n"
        "    vfmadd231ps %%zmm13, %%zmm2, %%zmm9\n"
        "    vfnmadd231ps %%zmm14, %%zmm5, %%zmm30\n"
        "    vfmadd231ps %%zmm14, %%zmm4, %%zmm10\n"
        "    vfnmadd231ps %%zmm15, %%zmm7, %%zmm31\n"
        "    vfmadd231ps %%zmm15, %%zmm6, %%zmm11\n"
        "    vaddps %%zmm28, %%zmm16, %%zmm0\n"
        "    vaddps %%zmm8, %%zmm17, %%zmm1\n"
        "    vsubps %%zmm28, %%zmm16, %%zmm2\n"
        "    vsubps %%zmm8, %%zmm17, %%zmm3\n"
        "    vaddps %%zmm29, %%zmm18, %%zmm4\n"
        "    vaddps %%zmm9, %%zmm19, %%zmm5\n"
        "    vsubps %%zmm29, %%zmm18, %%zmm6\n"
        "    vsubps %%zmm9, %%zmm19, %%zmm7\n"
        "    vaddps %%zmm30, %%zmm20, %%zmm8\n"
        "    vaddps %%zmm10, %%zmm21, %%zmm9\n"
        "    vsubps %%zmm30, %%zmm20, %%zmm12\n"
        "    vsubps %%zmm10, %%zmm21, %%zmm10\n"
        "    vmovups 288(%[tw]), %%zmm13\n"
        "    vmovups 352(%[tw]), %%zmm14\n"
        "    vaddps %%zmm31, %%zmm22, %%zmm15\n"
        "    vaddps %%zmm11, %%zmm23, %%zmm16\n"
        "    vsubps %%zmm31, %%zmm22, %%zmm17\n"
        "    vsubps %%zmm11, %%zmm23, %%zmm11\n"
        "    vunpckhps %%zmm4, %%zmm0, %%zmm18\n"
        "    vunpckhps %%zmm5, %%zmm1, %%zmm19\n"
        "    vmulps %%zmm24, %%zmm18, %%zmm20\n"
        "    vmulps %%zmm24, %%zmm19, %%zmm21\n"
        "    vunpcklps %%zmm4, %%zmm0, %%zmm0\n"
        "    vunpcklps %%zmm5, %%zmm1, %%zmm1\n"
        "    vunpcklps %%zmm6, %%zmm2, %%zmm4\n"
        "    vunpckhps %%zmm15, %%zmm8, %%zmm5\n"
        "    vunpckhps %%zmm16, %%zmm9, %%zmm22\n"
        "    vunpckhps %%zmm6, %%zmm2, %%zmm2\n"
        "    vunpckhps %%zmm7, %%zmm3, %%zmm6\n"
        "    vmulps %%zmm25, %%zmm5, %%zmm23\n"
        "    vmulps %%zmm25, %%zmm22, %%zmm28\n"
        "    vmulps %%zmm24, %%zmm2, %%zmm29\n"
        "    vfnmadd231ps %%zmm26, %%zmm19, %%zmm20\n"
        "    vfmadd231ps %%zmm26, %%zmm18, %%zmm21\n"
        "    vmulps %%zmm24, %%zmm6, %%zmm18\n"
        "    vunpcklps %%zmm15, %%zmm8, %%zmm8\n"
        "    vunpcklps %%zmm16, %%zmm9, %%zmm9\n"
        "    vunpcklps %%zmm7, %%zmm3, %%zmm3\n"
        "    vfnmadd231ps %%zmm27, %%zmm22, %%zmm23\n"
        "    vfmadd231ps %%zmm27, %%zmm5, %%zmm28\n"
        "    vfnmadd231ps %%zmm26, %%zmm6, %%zmm29\n"
        "    vaddps %%zmm20, %%zmm0, %%zmm5\n"
        "    vaddps %%zmm21, %%zmm1, %%zmm6\n"
        "    vfmadd231ps %%zmm26, %%zmm2, %%zmm18\n"
        "    vsubps %%zmm20, %%zmm0, %%zmm0\n"
        "    vsubps %%zmm21, %%zmm1, %%zmm1\n"
        "    vunpckhps %%zmm17, %%zmm12, %%zmm2\n"
        "    vunpckhps %%zmm11, %%zmm10, %%zmm7\n"
        "    vunpcklps %%zmm17, %%zmm12, %%zmm12\n"
        "    vmulps %%zmm25, %%zmm2, %%zmm15\n"
        "    vmulps %%zmm25, %%zmm7, %%zmm16\n"
        "    vunpcklps %%zmm11, %%zmm10, %%zmm10\n"
        "    vaddps %%zmm23, %%zmm8, %%zmm11\n"
        "    vaddps %%zmm28, %%zmm9, %%zmm17\n"
        "    vsubps %%zmm23, %%zmm8, %%zmm8\n"
        "    vsubps %%zmm28, %%zmm9, %%zmm9\n"
        "    vaddps %%zmm18, %%zmm4, %%zmm19\n"
        "    vsubps %%zmm29, %%zmm3, %%zmm20\n"
        "    vsubps %%zmm18, %%zmm4, %%zmm4\n"
        "    vaddps %%zmm29, %%zmm3, %%zmm3\n"
        "    vmovups 416(%[tw]), %%zmm18\n"
        "    vfnmadd231ps %%zmm27, %%zmm7, %%zmm15\n"
        "    vfmadd231ps %%zmm27, %%zmm2, %%zmm16\n"
        "    vshuff32x4 $238, %%zmm11, %%zmm5, %%zmm2\n"
        "    vshuff32x4 $238, %%zmm17, %%zmm6, %%zmm7\n"
        "    vshuff32x4 $238, %%zmm8, %%zmm0, %%zmm21\n"
        "    vshuff32x4 $238, %%zmm9, %%zmm1, %%zmm22\n"
        "    vmulps %%zmm13, %%zmm2, %%zmm23\n"
        "    vmulps %%zmm13, %%zmm7, %%zmm24\n"
        "    vmulps %%zmm13, %%zmm21, %%zmm25\n"
        "    vmulps %%zmm13, %%zmm22, %%zmm13\n"
        "    vshuff32x4 $68, %%zmm11, %%zmm5, %%zmm5\n"
        "    vaddps %%zmm16, %%zmm12, %%zmm11\n"
        "    vsubps %%zmm15, %%zmm10, %%zmm26\n"
        "    vsubps %%zmm16, %%zmm12, %%zmm12\n"
        "    vmovups 480(%[tw]), %%zmm16\n"
        "    vaddps %%zmm15, %%zmm10, %%zmm10\n"
        "    vshuff32x4 $68, %%zmm17, %%zmm6, %%zmm6\n"
        "    vshuff32x4 $68, %%zmm8, %%zmm0, %%zmm0\n"
        "    vfnmadd231ps %%zmm14, %%zmm7, %%zmm23\n"
        "    vfmadd231ps %%zmm14, %%zmm2, %%zmm24\n"
        "    vshuff32x4 $68, %%zmm9, %%zmm1, %%zmm1\n"
        "    vfnmadd231ps %%zmm14, %%zmm22, %%zmm25\n"
        "    vfmadd231ps %%zmm14, %%zmm21, %%zmm13\n"
        "    vshuff32x4 $238, %%zmm11, %%zmm19, %%zmm2\n"
        "    vshuff32x4 $238, %%zmm26, %%zmm20, %%zmm7\n"
        "    vshuff32x4 $68, %%zmm11, %%zmm19, %%zmm8\n"
        "    vmulps %%zmm18, %%zmm2, %%zmm9\n"
        "    vmulps %%zmm18, %%zmm7, %%zmm11\n"
        "    vshuff32x4 $68, %%zmm26, %%zmm20, %%zmm14\n"
        "    vaddps %%zmm23, %%zmm5, %%zmm15\n"
        "    vaddps %%zmm24, %%zmm6, %%zmm17\n"
        "    vsubps %%zmm23, %%zmm5, %%zmm5\n"
        "    vsubps %%zmm24, %%zmm6, %%zmm6\n"
        "    vaddps %%zmm13, %%zmm0, %%zmm19\n"
        "    vsubps %%zmm25, %%zmm1, %%zmm20\n"
        "    vshuff32x4 $238, %%zmm12, %%zmm4, %%zmm21\n"
        "    vshuff32x4 $238, %%zmm10, %%zmm3, %%zmm22\n"
        "    vshuff32x4 $68, %%zmm12, %%zmm4, %%zmm4\n"
        "    vmulps %%zmm18, %%zmm21, %%zmm12\n"
        "    vmulps %%zmm18, %%zmm22, %%zmm18\n"
        "    vfnmadd231ps %%zmm16, %%zmm7, %%zmm9\n"
        "    vfmadd231ps %%zmm16, %%zmm2, %%zmm11\n"
        "    vshuff32x4 $68, %%zmm10, %%zmm3, %%zmm2\n"
        "    vsubps %%zmm13, %%zmm0, %%zmm0\n"
        "    vaddps %%zmm25, %%zmm1, %%zmm1\n"
        "    vmovups %%zmm15, (%[data])\n"
        "    vmovups %%zmm17, 64(%[data])\n"
        "    vmovups %%zmm19, (%[data],%[stride],2)\n"
        "    vmovups %%zmm20, 64(%[data],%[stride],2)\n"
        "    vmovups %%zmm5, (%[data],%[stride],4)\n"
        "    vmovups %%zmm6, 64(%[data],%[stride],4)\n"
        "    lea 544(%[tw]), %[tw]\n"
        "    vfnmadd231ps %%zmm16, %%zmm22, %%zmm12\n"
        "    vfmadd231ps %%zmm16, %%zmm21, %%zmm18\n"
        "    vaddps %%zmm9, %%zmm8, %%zmm3\n"
        "    vaddps %%zmm11, %%zmm14, %%zmm5\n"
        "    vsubps %%zmm9, %%zmm8, %%zmm6\n"
        "    vsubps %%zmm11, %%zmm14, %%zmm7\n"
        "    vmovups %%zmm0, (%[fifth],%[stride],1)\n"
        "    vmovups %%zmm1, 64(%[fifth],%[stride],1)\n"
        "    vaddps %%zmm18, %%zmm4, %%zmm0\n"
        "    vsubps %%zmm12, %%zmm2, %%zmm1\n"
        "    vsubps %%zmm18, %%zmm4, %%zmm4\n"
        "    vmovups %%zmm3, (%[data],%[stride],1)\n"
        "    vaddps %%zmm12, %%zmm2, %%zmm2\n"
        "    vmovups %%zmm5, 64(%[data],%[stride],1)\n"
        "    vmovups %%zmm6, (%[fifth])\n"
        "    vmovups %%zmm7, 64(%[fifth])\n"
        "    lea 128(%[data]), %[data]\n"
        "    lea 128(%[fifth]), %[fifth]\n"
        "    vmovups %%zmm0, (%[third])\n"
        "    vmovups %%zmm1, 64(%[third])\n"
        "    vmovups %%zmm4, (%[third],%[stride],4)\n"
        "    vmovups %%zmm2, 64(%[third],%[stride],4)\n"
        "    lea 128(%[third]), %[third]\n"
        "    sub $1, %[j]\n"
        "    jnz 2b\n"
        "    lea (%[data],%[region_skip],1), %[data]\n"
        "    lea (%[third],%[region_skip],1), %[third]\n"
        "    lea (%[fifth],%[region_skip],1), %[fifth]\n"
        "    sub $1, %[regions]\n"
        "    jnz 1b\n"
        "    vzeroupper\n"
        : [data] "+r"(data), [tw] "+r"(tw), [stride] "+r"(stride), [count] "+r"(count), [regions] "+r"(regions), [third] "=&r"(third), [region_skip] "=&r"(region_skip), [fifth] "=&r"(fifth), [j] "=&r"(j)
        :
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "xmm16", "xmm17", "xmm18", "xmm19", "xmm20", "xmm21", "xmm22", "xmm23", "xmm24", "xmm25", "xmm26", "xmm27", "xmm28", "xmm29", "xmm30", "xmm31", "cc", "memory");
}

/* Pass before the last, in place on split vectors. Butterfly j < count of a region uses the
 * vectors at data + u*stride + 128*j, u < 4; the `regions` regions follow each other. Index bits 3
 * and 2 are moved from the lanes to the registers and transformed. See the table builder in
 * onefft.c for the twiddle layout. */
__attribute__((target("avx512f"))) static void onefft_avx512_la2(float *data, const float *tw, size_t stride, size_t count, size_t regions) {
    float *third;
    size_t region_skip;
    size_t j;
    __asm__ volatile(
        "    lea (%[data],%[stride],2), %[third]\n"
        "    lea (%[stride],%[stride],2), %[region_skip]\n"
        "    lea (%[third],%[stride],1), %[third]\n"
        "1:\n"
        "    mov %[count], %[j]\n"
        "2:\n"
        "    vmovups (%[data]), %%zmm0\n"
        "    vmovups 64(%[data]), %%zmm1\n"
        "    vmovups (%[data],%[stride],1), %%zmm2\n"
        "    vmovups 64(%[data],%[stride],1), %%zmm3\n"
        "    vmovups (%[data],%[stride],2), %%zmm4\n"
        "    vmovups 64(%[data],%[stride],2), %%zmm5\n"
        "    vmovups (%[third]), %%zmm6\n"
        "    vmovups 64(%[third]), %%zmm7\n"
        "    vmovups (%[tw]), %%zmm8\n"
        "    vmovups 128(%[tw]), %%zmm9\n"
        "    vmovups 64(%[tw]), %%zmm10\n"
        "    vmovups 192(%[tw]), %%zmm11\n"
        "    vmovups 256(%[tw]), %%zmm12\n"
        "    vmovups 320(%[tw]), %%zmm13\n"
        "    lea 384(%[tw]), %[tw]\n"
        "    vunpckhps %%zmm2, %%zmm0, %%zmm14\n"
        "    vunpckhps %%zmm3, %%zmm1, %%zmm15\n"
        "    vunpckhps %%zmm6, %%zmm4, %%zmm16\n"
        "    vunpckhps %%zmm7, %%zmm5, %%zmm17\n"
        "    vmulps %%zmm8, %%zmm14, %%zmm18\n"
        "    vmulps %%zmm8, %%zmm15, %%zmm8\n"
        "    vmulps %%zmm9, %%zmm16, %%zmm19\n"
        "    vmulps %%zmm9, %%zmm17, %%zmm9\n"
        "    vunpcklps %%zmm2, %%zmm0, %%zmm0\n"
        "    vunpcklps %%zmm3, %%zmm1, %%zmm1\n"
        "    vunpcklps %%zmm6, %%zmm4, %%zmm2\n"
        "    vunpcklps %%zmm7, %%zmm5, %%zmm3\n"
        "    vfnmadd231ps %%zmm10, %%zmm15, %%zmm18\n"
        "    vfmadd231ps %%zmm10, %%zmm14, %%zmm8\n"
        "    vfnmadd231ps %%zmm11, %%zmm17, %%zmm19\n"
        "    vfmadd231ps %%zmm11, %%zmm16, %%zmm9\n"
        "    vaddps %%zmm18, %%zmm0, %%zmm4\n"
        "    vaddps %%zmm8, %%zmm1, %%zmm5\n"
        "    vsubps %%zmm18, %%zmm0, %%zmm0\n"
        "    vsubps %%zmm8, %%zmm1, %%zmm1\n"
        "    vaddps %%zmm19, %%zmm2, %%zmm6\n"
        "    vaddps %%zmm9, %%zmm3, %%zmm7\n"
        "    vsubps %%zmm19, %%zmm2, %%zmm2\n"
        "    vsubps %%zmm9, %%zmm3, %%zmm3\n"
        "    vshuff32x4 $238, %%zmm6, %%zmm4, %%zmm8\n"
        "    vshuff32x4 $238, %%zmm7, %%zmm5, %%zmm9\n"
        "    vshuff32x4 $68, %%zmm6, %%zmm4, %%zmm4\n"
        "    vshuff32x4 $238, %%zmm2, %%zmm0, %%zmm6\n"
        "    vshuff32x4 $238, %%zmm3, %%zmm1, %%zmm10\n"
        "    vmulps %%zmm12, %%zmm8, %%zmm11\n"
        "    vmulps %%zmm12, %%zmm9, %%zmm14\n"
        "    vmulps %%zmm12, %%zmm6, %%zmm15\n"
        "    vmulps %%zmm12, %%zmm10, %%zmm12\n"
        "    vshuff32x4 $68, %%zmm7, %%zmm5, %%zmm5\n"
        "    vshuff32x4 $68, %%zmm2, %%zmm0, %%zmm0\n"
        "    vshuff32x4 $68, %%zmm3, %%zmm1, %%zmm1\n"
        "    vfnmadd231ps %%zmm13, %%zmm9, %%zmm11\n"
        "    vfmadd231ps %%zmm13, %%zmm8, %%zmm14\n"
        "    vfnmadd231ps %%zmm13, %%zmm10, %%zmm15\n"
        "    vfmadd231ps %%zmm13, %%zmm6, %%zmm12\n"
        "    vaddps %%zmm11, %%zmm4, %%zmm2\n"
        "    vsubps %%zmm11, %%zmm4, %%zmm3\n"
        "    vaddps %%zmm14, %%zmm5, %%zmm4\n"
        "    vsubps %%zmm14, %%zmm5, %%zmm5\n"
        "    vaddps %%zmm12, %%zmm0, %%zmm6\n"
        "    vsubps %%zmm15, %%zmm1, %%zmm7\n"
        "    vsubps %%zmm12, %%zmm0, %%zmm0\n"
        "    vaddps %%zmm15, %%zmm1, %%zmm1\n"
        "    vmovups %%zmm2, (%[data])\n"
        "    vmovups %%zmm3, (%[data],%[stride],2)\n"
        "    vmovups %%zmm4, 64(%[data])\n"
        "    vmovups %%zmm6, (%[data],%[stride],1)\n"
        "    vmovups %%zmm5, 64(%[data],%[stride],2)\n"
        "    vmovups %%zmm7, 64(%[data],%[stride],1)\n"
        "    vmovups %%zmm0, (%[third])\n"
        "    vmovups %%zmm1, 64(%[third])\n"
        "    lea 128(%[data]), %[data]\n"
        "    lea 128(%[third]), %[third]\n"
        "    sub $1, %[j]\n"
        "    jnz 2b\n"
        "    lea (%[data],%[region_skip],1), %[data]\n"
        "    lea (%[third],%[region_skip],1), %[third]\n"
        "    sub $1, %[regions]\n"
        "    jnz 1b\n"
        "    vzeroupper\n"
        : [data] "+r"(data), [tw] "+r"(tw), [stride] "+r"(stride), [count] "+r"(count), [regions] "+r"(regions), [third] "=&r"(third), [region_skip] "=&r"(region_skip), [j] "=&r"(j)
        :
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "xmm16", "xmm17", "xmm18", "xmm19", "cc", "memory");
}

#pragma GCC diagnostic pop
#endif
#endif
#endif

/* ---- plan ---------------------------------------------------------------------------------- */

/* Kernel signatures. Every instruction set provides kernels of these four kinds. */
typedef void onefft_small_fn(const float *in, float *out);
typedef void onefft_first_fn(const float *in, float *out, const uint32_t *table, size_t count, size_t in_stride,
                             size_t out_stride);
typedef void onefft_middle_fn(float *data, const float *tw, const size_t *args, size_t blocks, size_t regions);
typedef void onefft_last_fn(float *data, const float *tw, size_t stride, size_t count, size_t regions);

/* What a middle pass kernel reads through its `args` pointer, as an array of size_t in this
 * order (gen/onefft_gen/passes.py has the same list). See there for the loop structure. */
enum {
    ONEFFT_S_HIGH,      /* bytes between the vectors of a butterfly */
    ONEFFT_S_LOW,       /* x86: the second such distance */
    ONEFFT_INNER,       /* butterflies per block */
    ONEFFT_SKIP,        /* bytes from the end of the last butterfly of a block to the next block */
    ONEFFT_REGION_SKIP, /* additional bytes from the last block of a region to the next region */
    ONEFFT_TW_REGION,   /* bytes of twiddle table per region */
    ONEFFT_ARGS
};

/* The output buffer is divided into `regions` equal parts that a pass handles one by one, each
 * with its own part of the twiddle table: the parts differ in output bits that are already
 * known, and the twiddles depend on those. */
typedef struct {
    onefft_middle_fn *run;
    const float *tw;     /* table of region 0, the tables of the other regions follow */
    size_t args[ONEFFT_ARGS];
    size_t regions, region_bytes;
    size_t blocks;       /* blocks per region */
    size_t block_bytes;  /* nested passes: bytes that one block spans */
} onefft_middle;

typedef struct {
    onefft_last_fn *run;
    const float *tw;
    size_t regions;
    size_t stride, count;
} onefft_last;

enum { ONEFFT_MAX_MIDDLE = 32, ONEFFT_MAX_LOG2 = 30 };

struct onefft_plan {
    size_t n;
    unsigned log2n;
    int inverse;
    void (*execute)(const onefft_plan *plan, const float *in, float *out);
    void *memory; /* the one allocation holding all tables */

    /* transforms that fit the registers */
    onefft_small_fn *small;

    /* transforms in passes */
    onefft_first_fn *first;
    const uint32_t *first_table;
    size_t first_count, first_in_stride, first_out_stride;
    int middle_count;
    int nested_from; /* middle passes from this index on work on nested blocks, see below */
    onefft_middle middle[ONEFFT_MAX_MIDDLE];
    int last_count;
    onefft_last last[2];

    /* portable transform */
    const float *scalar_tw;
};

/* exp(-2 pi i k / 2^log2size). The angle is folded into the first octant before calling the
 * math library, which makes the tables exactly symmetric and exact at multiples of an eighth
 * turn. */
static void onefft_root_double(uint64_t k, unsigned log2size, double *re, double *im) {
    uint64_t phase = log2size ? k << (64 - log2size) : 0; /* fraction of a full turn, 0.64 fixed point */
    unsigned quadrant = (unsigned)(phase >> 62);
    uint64_t rest = phase & (((uint64_t)1 << 62) - 1);
    const double scale = 6.283185307179586476925286766559 / 18446744073709551616.0;
    double c, s;
    if (rest > ((uint64_t)1 << 61)) {
        double angle = (double)(((uint64_t)1 << 62) - rest) * scale;
        c = sin(angle), s = cos(angle);
    } else {
        double angle = (double)rest * scale;
        c = cos(angle), s = sin(angle);
    }
    /* (c - i s) * (-i)^quadrant */
    switch (quadrant) {
    case 0: *re = c, *im = -s; break;
    case 1: *re = -s, *im = -c; break;
    case 2: *re = -c, *im = s; break;
    default: *re = s, *im = c; break;
    }
}

static void onefft_root(uint64_t k, unsigned log2size, float *re, float *im) {
    double c, s;
    onefft_root_double(k, log2size, &c, &s);
    *re = (float)c, *im = (float)s;
}

static size_t onefft_bit_reverse(size_t value, unsigned bits) {
    size_t result = 0;
    for (unsigned i = 0; i < bits; i++) {
        result = (result << 1) | (value & 1);
        value >>= 1;
    }
    return result;
}

/* ---- portable transforms --------------------------------------------------------------------
 *
 * Sizes 1, 2 and 4 are written out. Everything else that has no SIMD kernels uses a plain
 * radix 2 transform.
 */

static void onefft_execute_1(const onefft_plan *plan, const float *in, float *out) {
    (void)plan;
    out[0] = in[0], out[1] = in[1];
}

static void onefft_execute_2(const onefft_plan *plan, const float *in, float *out) {
    (void)plan;
    float ar = in[0], ai = in[1], br = in[2], bi = in[3];
    out[0] = ar + br, out[1] = ai + bi;
    out[2] = ar - br, out[3] = ai - bi;
}

static void onefft_execute_4(const onefft_plan *plan, const float *in, float *out) {
    float ar = in[0] + in[4], ai = in[1] + in[5]; /* x0 + x2 */
    float br = in[0] - in[4], bi = in[1] - in[5]; /* x0 - x2 */
    float cr = in[2] + in[6], ci = in[3] + in[7]; /* x1 + x3 */
    float dr = in[2] - in[6], di = in[3] - in[7]; /* x1 - x3 */
    if (plan->inverse) dr = -dr, di = -di;
    out[0] = ar + cr, out[1] = ai + ci;
    out[2] = br + di, out[3] = bi - dr; /* b - i d */
    out[4] = ar - cr, out[5] = ai - ci;
    out[6] = br - di, out[7] = bi + dr; /* b + i d */
}

static void onefft_scalar_execute(const onefft_plan *plan, const float *in, float *out) {
    size_t n = plan->n;
    unsigned log2n = plan->log2n;
    int inv = plan->inverse; /* the inverse exchanges real and imaginary parts on the way in and out */
    for (size_t i = 0; i < n; i++) {
        size_t j = onefft_bit_reverse(i, log2n);
        out[2 * j + inv] = in[2 * i];
        out[2 * j + 1 - inv] = in[2 * i + 1];
    }
    const float *tw = plan->scalar_tw;
    for (size_t half = 1; half < n; half *= 2) {
        size_t step = n / (2 * half);
        for (size_t start = 0; start < n; start += 2 * half) {
            for (size_t k = 0; k < half; k++) {
                float wr = tw[2 * k * step], wi = tw[2 * k * step + 1];
                float *a = out + 2 * (start + k), *b = a + 2 * half;
                float tr = b[0] * wr - b[1] * wi;
                float ti = b[0] * wi + b[1] * wr;
                b[0] = a[0] - tr, b[1] = a[1] - ti;
                a[0] += tr, a[1] += ti;
            }
        }
    }
    if (inv) {
        for (size_t i = 0; i < n; i++) {
            float t = out[2 * i];
            out[2 * i] = out[2 * i + 1], out[2 * i + 1] = t;
        }
    }
}

/* All plan builders are called twice: first with memory == NULL to learn how many bytes of
 * tables the plan needs, then with the allocation to fill in. */
static size_t onefft_scalar_plan(onefft_plan *plan, float *memory) {
    if (plan->log2n <= 2) {
        if (memory) {
            plan->execute = plan->log2n == 0 ? onefft_execute_1 : plan->log2n == 1 ? onefft_execute_2 : onefft_execute_4;
        }
        return 0;
    }
    size_t count = plan->n / 2;
    if (memory) {
        for (size_t k = 0; k < count; k++) onefft_root(k, plan->log2n, memory + 2 * k, memory + 2 * k + 1);
        plan->scalar_tw = memory;
        plan->execute = onefft_scalar_execute;
    }
    return 2 * count * sizeof(float);
}

/* ---- transforms in passes, common to all instruction sets ---------------------------------- */

#if defined(ONEFFT_NEON) || defined(ONEFFT_X86)

/* Middle passes run pass by pass on ranges of up to 2^ONEFFT_CACHE_BYTES_LOG2 bytes. */
enum { ONEFFT_CACHE_BYTES_LOG2 = 15 };
#ifdef ONEFFT_DEVELOPER
static unsigned onefft_tune_cache_log2 = ONEFFT_CACHE_BYTES_LOG2;
#else
static const unsigned onefft_tune_cache_log2 = ONEFFT_CACHE_BYTES_LOG2;
#endif

/* Applies the nested middle passes up to `level` to `bytes` bytes at `data` within region
 * `region`. The blocks of these passes are nested: a block of one pass is made of whole blocks
 * of the pass before. `bytes` is a multiple of the block size of pass `level`.
 *
 * Ranges that fit the cache are processed pass by pass. A larger range is cut into pieces that
 * are finished one after the other, so that every pass finds its data in the cache that its
 * block size fits into. */
static void onefft_nested(const onefft_plan *plan, int level, size_t region, float *data, size_t bytes) {
    const onefft_middle *m = &plan->middle[level];
    size_t block = m->block_bytes, cache = (size_t)1 << onefft_tune_cache_log2;
    if (bytes <= cache || (level == plan->nested_from && bytes == block)) {
        for (int i = plan->nested_from; i <= level; i++) {
            const onefft_middle *p = &plan->middle[i];
            const float *tw = (const float *)((const char *)p->tw + region * p->args[ONEFFT_TW_REGION]);
            p->run(data, tw, p->args, bytes / p->block_bytes, 1);
        }
    } else if (block <= cache) {
        for (size_t offset = 0; offset < bytes; offset += cache) {
            onefft_nested(plan, level, region, (float *)((char *)data + offset), cache);
        }
    } else {
        const float *tw = (const float *)((const char *)m->tw + region * m->args[ONEFFT_TW_REGION]);
        for (size_t offset = 0; offset < bytes; offset += block) {
            float *part = (float *)((char *)data + offset);
            if (level > plan->nested_from) onefft_nested(plan, level - 1, region, part, block);
            m->run(part, tw, m->args, 1, 1);
        }
    }
}

static void onefft_execute_passes(const onefft_plan *plan, const float *in, float *out) {
    plan->first(in, out, plan->first_table, plan->first_count, plan->first_in_stride, plan->first_out_stride);
    /* A pass over the whole output is one kernel call. Only when the output does not fit the
     * cache are the nested passes applied piece by piece instead. */
    int whole = plan->n * 8 <= (size_t)1 << onefft_tune_cache_log2 ? plan->middle_count : plan->nested_from;
    for (int i = 0; i < whole; i++) {
        const onefft_middle *m = &plan->middle[i];
        m->run(out, m->tw, m->args, m->blocks, m->regions);
    }
    if (whole < plan->middle_count) {
        const onefft_middle *m = &plan->middle[plan->middle_count - 1];
        for (size_t r = 0; r < m->regions; r++) {
            onefft_nested(plan, plan->middle_count - 1, r, (float *)((char *)out + r * m->region_bytes), m->region_bytes);
        }
    }
    for (int i = 0; i < plan->last_count; i++) {
        const onefft_last *l = &plan->last[i];
        l->run(out, l->tw, l->stride, l->count, l->regions);
    }
}

static void onefft_execute_small(const onefft_plan *plan, const float *in, float *out) { plan->small(in, out); }

/* Table entry of a middle pass: the real parts of the powers 1..radix-1 of
 * exp(-2 pi i k / 2^log2size), padded to a multiple of 4 floats, then the imaginary parts.
 * Returns the number of floats. */
static size_t onefft_middle_entry(float *entry, uint64_t k, unsigned log2size, unsigned radix) {
    size_t pad = 4 * (radix / 4);
    if (entry) {
        memset(entry, 0, 2 * pad * sizeof(float));
        for (unsigned i = 1; i < radix; i++) onefft_root(k * i, log2size, &entry[i - 1], &entry[pad + i - 1]);
    }
    return 2 * pad;
}

/* A complex vector in a table: first the real parts of all lanes, then the imaginary parts. */
static void onefft_table_root(float *table, size_t lanes, size_t lane, uint64_t k, unsigned log2size) {
    if (table) onefft_root(k, log2size, &table[lane], &table[lanes + lane]);
}

#endif

/* ---- AArch64 --------------------------------------------------------------------------------
 *
 * A register holds 4 lanes, a butterfly works on 8 (or 4) registers.
 *
 *   first pass   transforms the top 3 index bits and moves every vector to its final position
 *   middle       transform 3 (or 2) more bits each, in place, small strides first
 *   last pass    transforms the remaining 3 (or 2) bits including the two that sit in the lanes
 *
 * The output is handled in 4 regions: its quarters hold the 4 values of output bits (1, 0).
 */
#ifdef ONEFFT_NEON

#ifdef ONEFFT_DEVELOPER
static const unsigned *onefft_developer_radices; /* overrides the pass structure, see below */
static int onefft_developer_ordered = -1;        /* 0 or 1 overrides onefft_neon_ordered() */
#endif

/* The same for the kernels built from fused multiply-add butterflies, which take a twiddle
 * w = c (1 + i t) as its real part c and the ratio t: first c of all lanes, then t. The real
 * part must not be zero; the callers keep the angle below a quarter turn. */
static void onefft_table_cos_tan(float *table, size_t lanes, size_t lane, uint64_t k, unsigned log2size) {
    if (table) {
        double c, s;
        onefft_root_double(k, log2size, &c, &s);
        table[lane] = (float)c, table[lanes + lane] = (float)(s / c);
    }
}

/* The middle and last passes exist in two forms. The standard one multiplies by twiddles and
 * then adds and subtracts. The other one builds every butterfly from six fused multiply-adds
 * (see cplx.butterfly_tan in the generator), which is fewer operations but needs three register
 * copies per butterfly. That is a clear gain on Apple cores, where copies cost nothing, and a
 * loss on the Cortex cores measured. */
#ifndef ONEFFT_NEON_FMA
#ifdef __APPLE__
#define ONEFFT_NEON_FMA 1
#else
#define ONEFFT_NEON_FMA 0
#endif
#endif

/* Whether the first pass of a transform of 2^log2n points handles its butterflies in the order
 * of the output: it then writes in order and reads scattered cache lines, instead of reading in
 * order and writing scattered lines (once the data exceeds the first level cache).
 *
 * Apple's cores are much faster that way while input and output fit their second level cache,
 * the last one they have: the M2 by 10% at 2^14 points and by 29 to 40% from 2^15 to 2^20. The
 * Cortex-X4 is 7 to 25% slower with it from 2^15 to 2^20 points, the Cortex-A73 and Kryo 360
 * 20 to 80%. ONEFFT_NEON_ORDERED=0/1 overrides the choice for all sizes. */
static int onefft_neon_ordered(unsigned log2n) {
#ifdef ONEFFT_DEVELOPER
    if (onefft_developer_ordered >= 0) return onefft_developer_ordered;
#endif
#if defined(ONEFFT_NEON_ORDERED)
    (void)log2n;
    return ONEFFT_NEON_ORDERED;
#elif defined(__APPLE__)
    /* The cache of the performance cores, where the system distinguishes them. The value is a
     * 32 or 64 bit integer, depending on the name and the system. */
    uint64_t cache = 0;
    size_t size = sizeof(cache);
    if (sysctlbyname("hw.perflevel0.l2cachesize", &cache, &size, NULL, 0) != 0 || cache == 0) {
        cache = 0, size = sizeof(cache);
        if (sysctlbyname("hw.l2cachesize", &cache, &size, NULL, 0) != 0) cache = 0;
    }
    return log2n >= 14 && log2n <= 40 && ((uint64_t)16 << log2n) <= cache;
#else
    (void)log2n;
    return 0;
#endif
}

static size_t onefft_neon_plan(onefft_plan *plan, float *memory) {
    unsigned log2n = plan->log2n;
    int inverse = plan->inverse;
    if (log2n < 3) return onefft_scalar_plan(plan, memory);
    if (log2n <= 5) {
        static onefft_small_fn *const small[3][2] = {
            {onefft_neon_n8, onefft_neon_n8i}, {onefft_neon_n16, onefft_neon_n16i}, {onefft_neon_n32, onefft_neon_n32i}};
        if (memory) {
            plan->small = small[log2n - 3][inverse];
            plan->execute = onefft_execute_small;
        }
        return 0;
    }
    size_t n = plan->n, floats = 0;

    /* Radix bits per pass: 3 for the first pass, then the middle passes, then the last pass.
     * As few passes as possible, and a radix 8 last pass where the size allows it: per index
     * bit it is the cheaper of the two last passes. */
    unsigned last = 3, middle[ONEFFT_MAX_MIDDLE], middle_count = 0;
    if (log2n == 7) {
        last = 2;
        middle[middle_count++] = 2;
    } else {
        unsigned rest = log2n - 6, twos = (3 - rest % 3) % 3;
        for (unsigned i = 0; i < twos; i++) middle[middle_count++] = 2;
        for (unsigned i = 0; i < (rest - 2 * twos) / 3; i++) middle[middle_count++] = 3;
    }
#ifdef ONEFFT_DEVELOPER
    if (onefft_developer_radices) { /* 0 terminated list: middle passes, then the last pass */
        middle_count = 0;
        const unsigned *r = onefft_developer_radices;
        for (; r[1]; r++) middle[middle_count++] = r[0];
        last = r[0];
    }
#endif

    /* First pass: the butterfly that reads input vector a writes to the bit reversed position.
     * The butterflies are handled in the order of the input or in the order of the output, see
     * onefft_neon_ordered(). Prefetching the scattered lines did not help. */
    size_t first_count = n >> 5;
    if (memory) {
        uint32_t *table = (uint32_t *)(memory + floats);
        int ordered = onefft_neon_ordered(log2n);
        for (size_t a = 0; a < first_count; a++) {
            size_t b = onefft_bit_reverse(a, log2n - 5); /* the mapping is its own inverse */
            table[2 * a] = (uint32_t)(4 * (ordered ? b : a));
            table[2 * a + 1] = (uint32_t)(8 * (ordered ? a : b));
        }
        plan->first = inverse ? onefft_neon_f8i : onefft_neon_f8;
        plan->first_table = table;
        plan->first_count = first_count;
        plan->first_in_stride = n;      /* an eighth of the input, in bytes */
        plan->first_out_stride = 2 * n; /* a quarter of the output */
    }
    floats += 2 * first_count;
    floats = (floats + 15) & ~(size_t)15;

    /* Middle passes. After `done` bits, butterfly j of a block in region t has the low output
     * bits K = t + 4 j: t are output bits (1, 0), j the rest. */
    const int fma = ONEFFT_NEON_FMA;
    unsigned done = 3;
    for (unsigned i = 0; i < middle_count; i++) {
        unsigned bits = middle[i];
        size_t inner = (size_t)1 << (done - 2), start = floats;
        for (size_t t = 0; t < 4; t++) {
            for (size_t j = 0; j < inner; j++) {
                uint64_t k = t + 4 * j;
                float *entry = memory ? memory + floats : NULL;
                if (!fma) {
                    /* the powers of exp(-2 pi i K / 2^(done + bits)) */
                    floats += onefft_middle_entry(entry, k, done + bits, 1u << bits);
                    continue;
                }
                /* One twiddle per stage m: exp(-2 pi i K / 2^(done + m)). The angle of the
                 * first reaches a quarter turn in the upper half of the K range; there it is
                 * stored a quarter turn back and the kernel multiplies by -i. Radix 8 needs
                 * the third also turned by an eighth. */
                size_t lanes = bits == 3 ? 4 : 2;
                onefft_table_cos_tan(entry, lanes, 0, k & (((uint64_t)1 << (done - 1)) - 1), done + 1);
                onefft_table_cos_tan(entry, lanes, 1, k, done + 2);
                if (bits == 3) {
                    onefft_table_cos_tan(entry, lanes, 2, k, done + 3);
                    onefft_table_cos_tan(entry, lanes, 3, k + ((uint64_t)1 << done), done + 3);
                }
                floats += 2 * lanes;
            }
        }
        if (memory) {
            onefft_middle *m = &plan->middle[i];
            m->run = bits == 3 ? (fma ? onefft_neon_c8_fma : onefft_neon_c8) : (fma ? onefft_neon_c4_fma : onefft_neon_c4);
            m->tw = memory + start;
            m->regions = 4;
            m->region_bytes = 2 * n;
            m->args[ONEFFT_S_HIGH] = 32 * inner;
            m->args[ONEFFT_INNER] = inner;
            m->args[ONEFFT_SKIP] = 32 * inner * (((size_t)1 << bits) - 1);
            m->args[ONEFFT_REGION_SKIP] = 0;
            m->args[ONEFFT_TW_REGION] = (floats - start) / 4 * sizeof(float);
            m->block_bytes = (32 * inner) << bits;
            m->blocks = m->region_bytes / m->block_bytes;
        }
        done += bits;
    }
    floats = (floats + 15) & ~(size_t)15;

    /* Last pass. Butterfly j handles the output indices K + (N/8 or N/4) * q with K = t + 4 j
     * in lane t. Radix 8 first transforms index bit 2 with one scalar twiddle per region t,
     * which gives the new output bit g. Then index bits 1 and 0 are transformed with twiddle
     * vectors.
     *
     * Standard form: for each g, the twiddles of the registers l = 1..3, l being index bits
     * (1, 0): the powers l of exp(-2 pi i (K + g N/8) / N).
     * Fused form: the twiddle of index bit 1, exp(-2 pi i 2 K / N), shared by both g, then for
     * each g that of index bit 0, exp(-2 pi i (K + g N/8) / N). Angles that would reach a
     * quarter turn are stored a quarter turn back, as in the middle passes. */
    size_t start = floats;
    for (size_t j = 0; j < n >> (last + 2); j++) {
        float *table = memory ? memory + floats : NULL;
        if (fma) {
            if (last == 3) {
                for (size_t t = 0; t < 4; t++) onefft_table_cos_tan(table, 4, t, 4 * ((t + 4 * j) % (n >> 4)), log2n);
                floats += 8;
            }
            table = memory ? memory + floats : NULL;
            for (size_t t = 0; t < 4; t++) onefft_table_cos_tan(table, 4, t, 2 * ((t + 4 * j) % (n >> 3)), log2n);
            floats += 8;
            for (size_t g = 0; g < (size_t)1 << (last - 2); g++) {
                table = memory ? memory + floats : NULL;
                for (size_t t = 0; t < 4; t++) onefft_table_cos_tan(table, 4, t, t + 4 * j + (n >> 3) * g, log2n);
                floats += 8;
            }
            continue;
        }
        if (last == 3) {
            for (size_t t = 0; t < 4; t++) onefft_table_root(table, 4, t, 4 * (t + 4 * j), log2n);
            floats += 8;
        }
        for (size_t g = 0; g < (size_t)1 << (last - 2); g++) {
            for (size_t l = 1; l < 4; l++) {
                table = memory ? memory + floats : NULL;
                for (size_t t = 0; t < 4; t++) onefft_table_root(table, 4, t, l * (t + 4 * j + (n >> 3) * g), log2n);
                floats += 8;
            }
        }
    }
    if (memory) {
        plan->middle_count = (int)middle_count;
        plan->nested_from = 0;
        onefft_last *l = &plan->last[0];
        static onefft_last_fn *const kernels[2][2][2] = { /* by fused form, radix 8, inverse */
            {{onefft_neon_l4, onefft_neon_l4i}, {onefft_neon_l8, onefft_neon_l8i}},
            {{onefft_neon_l4_fma, onefft_neon_l4_fmai}, {onefft_neon_l8_fma, onefft_neon_l8_fmai}}};
        l->run = kernels[fma][last == 3][inverse];
        l->tw = memory + start;
        l->regions = 1;
        l->stride = (8 * n) >> last;
        l->count = n >> (last + 2);
        plan->last_count = 1;
        plan->execute = onefft_execute_passes;
    }
    return floats * sizeof(float);
}

#endif /* ONEFFT_NEON */

/* ---- x86-64 ---------------------------------------------------------------------------------
 *
 * 16 registers hold 4 complex vectors with room for intermediate results: the passes are radix
 * 4. Two AVX2 kernels are radix 8 nevertheless; they compute in place in all 16 registers, with
 * constants and twiddles as memory operands. A register has 4 lanes with SSE2 and 8 with AVX2.
 *
 *   first pass      transforms the top 2 (1, 3) index bits, moves vectors to their final position
 *   middle passes   2 bits each
 *   SSE2  l4        index bits 1 and 0, which sit in the lanes
 *   AVX2  la, lb    index bits 3 and 2, then 1 and 0; bits 2..0 sit in the lanes
 *         la8, lb   index bits 4, 3 and 2, then 1 and 0
 *
 * Region r of the output holds output bits (0, 1) = (r/2, r%2) when it is divided in 4, and
 * output bits (0, 1, 2) = (r/4, r/2%2, r%2) when it is divided in 8. In that order the
 * exchanges of the last passes leave everything in its place.
 */
#ifdef ONEFFT_X86

typedef struct {
    unsigned lane_bits;
    size_t (*low_bits)(size_t region, unsigned bits); /* the low output bits of a region */
    onefft_first_fn *f4[2], *f_odd[2]; /* radix 4; radix 2 (SSE2) or 8 (AVX2, AVX-512) */
    onefft_middle_fn *c4, *c8;         /* c8: AVX-512 */
    onefft_last_fn *l4[2];             /* SSE2 */
    onefft_last_fn *la, *la2, *la8, *la8x, *lb[2]; /* AVX2 */
    onefft_small_fn *small[6][2];      /* by log2n; NULL where the passes are used */
} onefft_x86_kernels;

static const onefft_x86_kernels onefft_sse2_kernels = {
    2, onefft_bit_reverse, {onefft_sse2_f4, onefft_sse2_f4i}, {onefft_sse2_f2, onefft_sse2_f2i}, onefft_sse2_c4, NULL,
    {onefft_sse2_l4, onefft_sse2_l4i}, NULL, NULL, NULL, NULL, {NULL, NULL},
    {{0}, {0}, {0}, {onefft_sse2_n8, onefft_sse2_n8i}, {onefft_sse2_n16, onefft_sse2_n16i}, {0}}};

#ifdef ONEFFT_AVX2
static const onefft_x86_kernels onefft_avx2_kernels = {
    3, onefft_bit_reverse, {onefft_avx2_f4, onefft_avx2_f4i}, {onefft_avx2_f8, onefft_avx2_f8i}, onefft_avx2_c4, NULL,
    {NULL, NULL}, onefft_avx2_la, onefft_avx2_la2, onefft_avx2_la8, onefft_avx2_la8x,
    {onefft_avx2_lb, onefft_avx2_lbi},
    {{0}, {0}, {0}, {onefft_sse2_n8, onefft_sse2_n8i}, {onefft_avx2_n16, onefft_avx2_n16i},
     {onefft_avx2_n32, onefft_avx2_n32i}}};
#endif

/* AVX2 lane order: the element of 8 consecutive values that each lane holds. */
static const unsigned char onefft_avx2_element[8] = {0, 1, 4, 5, 2, 3, 6, 7};

/* Adds a middle pass that transforms `bits` index bits (2, or 3 with AVX-512) after `done` are
 * finished. The twiddle of butterfly j in region r is exp(-2 pi i K / 2^(done + bits)),
 * K = low + (j << region_bits) where `low` are the output bits that the region stands for.
 * Returns the new table size. */
static size_t onefft_x86_middle(onefft_plan *plan, float *memory, size_t floats, const onefft_x86_kernels *kernels,
                                unsigned done, unsigned bits, unsigned region_bits, size_t s_high, size_t s_low,
                                size_t inner, size_t skip, size_t blocks) {
    size_t start = floats, regions = (size_t)1 << region_bits;
    for (size_t r = 0; r < regions; r++) {
        for (size_t j = 0; j < inner; j++) {
            uint64_t k = kernels->low_bits(r, region_bits) + (j << region_bits);
            floats += onefft_middle_entry(memory ? memory + floats : NULL, k, done + bits, 1u << bits);
        }
    }
    if (memory) {
        onefft_middle *m = &plan->middle[plan->middle_count++];
        m->run = bits == 3 ? kernels->c8 : kernels->c4;
        size_t vector = (size_t)8 << kernels->lane_bits;
        m->tw = memory + start;
        m->regions = regions;
        m->region_bytes = (8 * plan->n) >> region_bits;
        m->args[ONEFFT_S_HIGH] = s_high;
        m->args[ONEFFT_S_LOW] = s_low;
        m->args[ONEFFT_INNER] = inner;
        m->args[ONEFFT_SKIP] = skip;
        /* the data pointer moves by inner * vector + skip per block */
        m->args[ONEFFT_REGION_SKIP] = m->region_bytes - blocks * (inner * vector + skip);
        m->args[ONEFFT_TW_REGION] = (floats - start) / regions * sizeof(float);
        m->blocks = blocks;
        m->block_bytes = m->region_bytes / blocks; /* only meaningful, and exact, for nested passes */
    }
    return floats;
}

/* Choices that depend on the transform size and on the processor. All sizes are log2 of the
 * number of points. docs/measurements.md has the numbers behind them.
 *
 * The first pass moves every vector to its final position, which is bit reversed. It handles
 * its butterflies in the order of the input, and then writes to scattered cache lines once the
 * data exceeds the first level cache, or in the order of the output, and then reads scattered
 * lines. Stores that miss the cache are handled one at a time by AMD's cores, loads in
 * parallel: writing in order is 3 to 15% faster on Zen 2 and Zen 4 and up to twice as fast on
 * Zen 5, as long as input and output fit a part of the third level cache. Beyond that reading
 * in order is faster. Intel's cores gain nothing from writing in order (Skylake with AVX2,
 * Cascade Lake with AVX2 and AVX-512) and read in order, as does every processor that does not
 * report that cache the AMD way.
 *
 * AVX-512 has a radix 16 first pass. It reads and writes 16 streams whose cache lines compete
 * for the same places of the first level cache when the transform is large, so that the radix 8
 * pass is faster then on AMD's cores (Zen 4, Zen 5). When the data does not fit the third level
 * cache either, a pass over it costs more than that and radix 16 is used again, because it
 * saves a pass. Intel's Cascade Lake is as fast or faster with radix 16 at every size, by up to
 * 10% from 2^14 points up: Intel processors always use it. */
typedef struct {
    unsigned char f16_to, f16_from;         /* AVX-512: radix 16 up to and from these sizes, radix 8 between */
    unsigned char ordered_from, ordered_to; /* sizes whose first pass writes in order */
} onefft_x86_tuning;

#ifdef ONEFFT_DEVELOPER
static const onefft_x86_tuning *onefft_developer_tuning; /* overrides the choice below */
#endif

static onefft_x86_tuning onefft_x86_tune(unsigned lane_bits) {
#ifdef ONEFFT_DEVELOPER
    if (onefft_developer_tuning) return *onefft_developer_tuning;
#endif
#ifdef ONEFFT_X86_TUNING /* the four numbers, for tests: -DONEFFT_X86_TUNING=99,0,99,0 */
    static const onefft_x86_tuning fixed = {ONEFFT_X86_TUNING};
    (void)lane_bits;
    return fixed;
#else
    onefft_x86_tuning tuning = {12, 22, 99, 0};
    unsigned eax = 0, ebx = 0, ecx = 0, edx = 0, family = 0;
    if (__get_cpuid(0, &eax, &ebx, &ecx, &edx) && ebx == 0x756e6547 && edx == 0x49656e69 && ecx == 0x6c65746e) {
        tuning.f16_to = 99; /* "GenuineIntel" */
        return tuning;
    }
    if (__get_cpuid(1, &eax, &ebx, &ecx, &edx)) family = ((eax >> 8) & 15) + ((eax >> 20) & 255);
    /* The third level cache that a core has access to, from AMD's list of caches. */
    uint64_t bytes = 0;
    for (unsigned i = 0; i < 8 && __get_cpuid_count(0x8000001d, i, &eax, &ebx, &ecx, &edx) && (eax & 31); i++) {
        if (((eax >> 5) & 7) == 3) {
            bytes = (uint64_t)((ebx >> 22) + 1) * (((ebx >> 12) & 1023) + 1) * ((ebx & 4095) + 1) * ((uint64_t)ecx + 1);
        }
    }
    if (bytes < ((uint64_t)1 << 20)) return tuning;
    unsigned cache = 20; /* log2 of its size, rounded down */
    while (bytes >> (cache + 1)) cache++;
    /* A transform of 2^k points works on 2^(k+4) bytes: it writes in order while that is at
     * most a quarter of the cache (half on Zen 5 and later, but never beyond what the TLB of
     * these cores covers). */
    int zen5 = family >= 0x1a;
    unsigned limit = zen5 ? 20 : 19, to = cache - (zen5 ? 5 : 6);
    tuning.ordered_from = lane_bits == 4 ? 14 : 13;
    tuning.ordered_to = (unsigned char)(to < limit ? to : limit);
    tuning.f16_from = (unsigned char)(cache - 4);
    return tuning;
#endif
}

static size_t onefft_x86_plan(onefft_plan *plan, float *memory, const onefft_x86_kernels *kernels) {
    unsigned log2n = plan->log2n, lane_bits = kernels->lane_bits;
    int inverse = plan->inverse, avx2 = lane_bits == 3;
    if (log2n < 3) return onefft_scalar_plan(plan, memory);
    if (log2n < 6 && kernels->small[log2n][0]) {
        if (memory) {
            plan->small = kernels->small[log2n][inverse];
            plan->execute = onefft_execute_small;
        }
        return 0;
    }
    size_t n = plan->n, floats = 0, vector = (size_t)8 << lane_bits; /* bytes of a split vector */
    if (memory) plan->middle_count = 0;

    /* Which passes. SSE2: all passes after the first transform two index bits, so the first
     * is radix 4 for an even number of bits and radix 2 for an odd one. AVX2: the first pass is
     * radix 8 (its kernel has no twiddles to hold and manages with 16 registers), and the pass
     * before the last takes 3 bits (la8) or 2 (la) to make the total fit. 64 points are too
     * few for that: radix 4, la2, lb. */
    unsigned first_bits, last_bits; /* last_bits: index bits left to the last pass(es) */
    if (!avx2) {
        first_bits = log2n % 2 == 0 ? 2 : 1, last_bits = 2;
    } else if (log2n == 6) {
        first_bits = 2, last_bits = 4;
    } else {
        first_bits = 3, last_bits = log2n % 2 == 0 ? 5 : 4;
    }

    /* First pass. Its butterflies count through the remaining index bits above the lanes. The
     * top `kept` of those bits stay where they are: their results are what the last passes
     * exchange with the lanes, unless the first pass transforms those bits itself. The others
     * are bit reversed. The butterflies are handled in the order of the input or in the order
     * of the output, see onefft_x86_tuning. */
    unsigned count_bits = log2n - first_bits - lane_bits, kept = lane_bits > first_bits ? lane_bits - first_bits : 0;
    size_t first_count = (size_t)1 << count_bits;
    if (memory) {
        uint32_t *table = (uint32_t *)(memory + floats);
        unsigned low_bits = count_bits - kept;
        onefft_x86_tuning tuning = onefft_x86_tune(lane_bits);
        int ordered = log2n >= tuning.ordered_from && log2n <= tuning.ordered_to;
        for (size_t a = 0; a < first_count; a++) {
            size_t low = onefft_bit_reverse(a & (((size_t)1 << low_bits) - 1), low_bits);
            size_t b = (a >> low_bits << low_bits) | low; /* the mapping is its own inverse */
            table[2 * a] = (uint32_t)((ordered ? b : a) << lane_bits);
            table[2 * a + 1] = (uint32_t)((ordered ? a : b) << lane_bits);
        }
        plan->first = first_bits == 2 ? kernels->f4[inverse] : kernels->f_odd[inverse];
        plan->first_table = table;
        plan->first_count = first_count;
        plan->first_in_stride = plan->first_out_stride = (8 * n) >> first_bits;
    }
    floats += 2 * first_count;
    floats = (floats + 15) & ~(size_t)15;

    /* A middle pass that is not nested: SSE2 with a radix 2 first pass has a kept index bit,
     * n-2, at the second highest position. It is transformed together with bit n-3, which is
     * next to the lanes. */
    unsigned done = first_bits;
    if (!avx2 && first_bits == 1) {
        floats = onefft_x86_middle(plan, memory, floats, kernels, done, 2, 1, 2 * n, vector, 1, vector, n >> 5);
        done += 2;
    }
    if (memory) plan->nested_from = plan->middle_count;

    /* Nested middle passes, small strides first. */
    for (; done + last_bits < log2n; done += 2) {
        size_t s_high = (size_t)8 << done;
        floats = onefft_x86_middle(plan, memory, floats, kernels, done, 2, lane_bits, s_high, 2 * s_high,
                                   (size_t)1 << (done - lane_bits), 3 * s_high, (n << (3 - lane_bits)) / (4 * s_high));
    }

    /* AVX2, pass before the last. Butterfly j of region r has the low output bits
     * K = k_low + 4 k2 + 8 j, where k_low is r reversed (output bits 0 and 1). */
    if (avx2 && last_bits == 5) {
        /* la8: index bit 2 is exchanged with output bit 2 (k2) while loading, so k2 is lane bit
         * b0 and every twiddle is a vector. Stage m = 1..3 (index bits 4, 3, 2) needs
         * exp(-2 pi i K / 2^(n - 5 + m)), the last also turned by an eighth. The first reaches
         * a quarter turn in the upper half of the butterflies; there it is stored a quarter
         * turn back and the kernel multiplies by -i (la8x, 256 points, has one butterfly per
         * region and stores the full angle). The kernel applies a twiddle as three shears and
         * takes tan(angle/2) and sin(angle). */
        size_t start = floats, count = n >> 8;
        for (size_t r = 0; r < 4; r++) {
            for (size_t j = 0; j < count; j++) {
                for (unsigned t = 0; t < 4; t++) {
                    float *table = memory ? memory + floats : NULL;
                    for (size_t lane = 0; table && lane < 8; lane++) {
                        uint64_t k = onefft_bit_reverse(r, 2) + 4 * (lane & 1) + 8 * j;
                        unsigned log2size = log2n - 4 + (t < 3 ? t : 2);
                        if (t == 0 && count > 1) k &= (n >> 6) - 1;
                        if (t == 3) k += n >> 5;
                        double c, s;
                        onefft_root_double(k, log2size, &c, &s);
                        table[lane] = (float)(-s / (1.0 + c)), table[8 + lane] = (float)-s;
                    }
                    floats += 16;
                }
            }
        }
        if (memory) {
            onefft_last *l = &plan->last[0];
            l->run = count > 1 ? kernels->la8 : kernels->la8x;
            l->tw = memory + start;
            l->regions = 4;
            l->stride = n / 4;
            l->count = count;
        }
    } else if (avx2) {
        /* la: index bit 3 uses a scalar twiddle for each of the two values of k2, which selects
         * a pair of vectors. Then k2 moves into lane bit b0, and index bit 2 uses a twiddle
         * vector. For 64 points (la2) there is one butterfly per region and no k2 yet. */
        size_t start = floats, count = log2n > 6 ? n >> 7 : 1;
        for (size_t r = 0; r < 4; r++) {
            uint64_t k_low = onefft_bit_reverse(r, 2);
            for (size_t j = 0; j < count; j++) {
                float *table = memory ? memory + floats : NULL;
                if (table) memset(table, 0, 4 * sizeof(float));
                for (size_t k2 = 0; k2 < (log2n > 6 ? 2 : 1); k2++) {
                    onefft_table_root(table, 2, k2, 8 * (k_low + 4 * k2 + 8 * j), log2n);
                }
                for (size_t lane = 0; lane < 8; lane++) {
                    onefft_table_root(table ? table + 4 : NULL, 8, lane, 4 * (k_low + 4 * (lane & 1) + 8 * j), log2n);
                }
                floats += 20;
            }
        }
        if (memory) {
            onefft_last *l = &plan->last[0];
            l->run = log2n > 6 ? kernels->la : kernels->la2;
            l->tw = memory + start;
            l->regions = 4;
            l->stride = log2n > 6 ? n / 2 : vector;
            l->count = count;
        }
    }
    floats = (floats + 15) & ~(size_t)15;

    /* l4 / lb: butterfly j handles the output indices K + (N/4) q, K = element + lanes * j. */
    size_t start = floats, lanes = (size_t)1 << lane_bits;
    for (size_t j = 0; j < n >> (lane_bits + 2); j++) {
        for (size_t l = 1; l < 4; l++) {
            float *table = memory ? memory + floats : NULL;
            for (size_t lane = 0; lane < lanes; lane++) {
                size_t element = avx2 ? onefft_avx2_element[lane] : lane;
                onefft_table_root(table, lanes, lane, l * (element + lanes * j), log2n);
            }
            floats += 2 * lanes;
        }
    }
    if (memory) {
        onefft_last *l = &plan->last[avx2 ? 1 : 0];
        l->run = avx2 ? kernels->lb[inverse] : kernels->l4[inverse];
        l->tw = memory + start;
        l->regions = 1;
        l->stride = 2 * n;
        l->count = n >> (lane_bits + 2);
        plan->last_count = avx2 ? 2 : 1;
        plan->execute = onefft_execute_passes;
    }
    return floats * sizeof(float);
}

/* ---- AVX-512 --------------------------------------------------------------------------------
 *
 * 16 lanes and 32 registers: four index bits sit in the lanes, a kernel holds 8 complex vectors,
 * or 16 if it computes in place. 32 to 128 points are single kernels; below, the AVX2 plan is
 * used. Larger transforms:
 *
 *   f16         the top 4 index bits
 *     or f8     the top 3 index bits, followed by a c8 for bit n-4 (see below) and the two bits
 *               next to the lanes
 *   c8, c4      nested passes, 3 and 2 bits each
 *   la3 / la2   index bits (4,) 3, 2
 *   lb          index bits 1, 0
 *
 * The four exchanges of lanes and registers in la and lb are one shuffle per register each
 * only in one arrangement (gen/onefft_gen/avx512_kernels.py): the top four position bits of the
 * output hold, from the top, output bits 0, 2, 1, 3. The radix 16 first pass writes its results
 * accordingly. The radix 8 first pass produces only three of them: output bit 3 comes from index
 * bit n-4, which it therefore leaves at that position.
 */
#ifdef ONEFFT_AVX512

static size_t onefft_avx512_low_bits(size_t region, unsigned bits) {
    static const unsigned char order[4] = {0, 2, 1, 3};
    size_t low = 0;
    for (unsigned i = 0; i < bits; i++) low |= ((region >> (bits - 1 - i)) & 1) << order[i];
    return low;
}

static const onefft_x86_kernels onefft_avx512_kernels = {
    4, onefft_avx512_low_bits, {NULL, NULL}, {NULL, NULL}, onefft_avx512_c4, onefft_avx512_c8,
    {NULL, NULL}, onefft_avx512_la3, onefft_avx512_la2, NULL, NULL, {onefft_avx512_lb, onefft_avx512_lbi}, {{0}}};

/* The element of 16 consecutive values that each lane holds. */
static const unsigned char onefft_avx512_element[16] = {0, 1, 8, 9, 2, 3, 10, 11, 4, 5, 12, 13, 6, 7, 14, 15};

static size_t onefft_avx512_plan(onefft_plan *plan, float *memory) {
    const onefft_x86_kernels *kernels = &onefft_avx512_kernels;
    unsigned log2n = plan->log2n;
    int inverse = plan->inverse;
    if (log2n < 5) return onefft_x86_plan(plan, memory, &onefft_avx2_kernels);
    if (log2n < 8) { /* 32 to 128 points fit the registers */
        static onefft_small_fn *const small[3][2] = {{onefft_avx512_n32, onefft_avx512_n32i},
                                                     {onefft_avx512_n64, onefft_avx512_n64i},
                                                     {onefft_avx512_n128, onefft_avx512_n128i}};
        if (memory) {
            plan->small = small[log2n - 5][inverse];
            plan->execute = onefft_execute_small;
        }
        return 0;
    }
    size_t n = plan->n, floats = 0, vector = 128;
    if (memory) plan->middle_count = 0;

    /* Which passes. The first is radix 16 or radix 8. A radix 8 first pass is followed by a
     * pass for 3 bits that is not nested (`special`, see below). The nested middle passes take
     * 3 or 2 bits each, and the two last passes 5 or 4 together: whichever needs fewer middle
     * passes, and among equals 4 if the data fits the first level cache and 5 otherwise. (One
     * bit cannot be left to the middle passes.) */
    onefft_x86_tuning tuning = onefft_x86_tune(4);
    unsigned first_bits = log2n <= tuning.f16_to || log2n >= tuning.f16_from || log2n < 10 ? 4 : 3;
    unsigned special = first_bits == 3 ? 3 : 0;
    unsigned rest = log2n - first_bits - special - 4; /* bits of the middle passes if the last take 4 */
    unsigned last_bits = rest == 1 || (rest >= 3 && (rest % 3 == 1 || log2n > 12)) ? 5 : 4;
    unsigned nested = log2n - first_bits - special - last_bits;

    /* First pass. It counts through the index bits between its own and the lanes and writes to
     * the bit reversed position; after the radix 8 pass the top one of those bits, n-4, keeps
     * its place instead. */
    unsigned count_bits = log2n - first_bits - 4, low_bits = count_bits - (4 - first_bits);
    size_t first_count = (size_t)1 << count_bits;
    if (memory) {
        uint32_t *table = (uint32_t *)(memory + floats);
        int ordered = log2n >= tuning.ordered_from && log2n <= tuning.ordered_to;
        for (size_t a = 0; a < first_count; a++) {
            size_t low = onefft_bit_reverse(a & (((size_t)1 << low_bits) - 1), low_bits);
            size_t b = (a >> low_bits << low_bits) | low; /* the mapping is its own inverse */
            table[2 * a] = (uint32_t)((ordered ? b : a) << 4);
            table[2 * a + 1] = (uint32_t)((ordered ? a : b) << 4);
        }
        plan->first = first_bits == 4 ? (inverse ? onefft_avx512_f16i : onefft_avx512_f16)
                                      : (inverse ? onefft_avx512_f8i : onefft_avx512_f8);
        plan->first_table = table;
        plan->first_count = first_count;
        plan->first_in_stride = plan->first_out_stride = (8 * n) >> first_bits;
    }
    floats += 2 * first_count;
    floats = (floats + 15) & ~(size_t)15;

    /* After a radix 8 first pass: index bit n-4 at the fourth highest position with the two
     * bits next to the lanes, not nested. */
    unsigned done = first_bits;
    if (special) {
        floats = onefft_x86_middle(plan, memory, floats, kernels, done, 3, 3, n / 2, vector, 1, 3 * vector, n >> 10);
        done += 3;
    }
    if (memory) plan->nested_from = plan->middle_count;

    /* Nested passes, small strides first. */
    unsigned twos = (3 - nested % 3) % 3;
    for (unsigned i = 0; done + last_bits < log2n; i++) {
        unsigned bits = i < (nested - 2 * twos) / 3 ? 3 : 2;
        size_t s_high = (size_t)8 << done;
        floats = onefft_x86_middle(plan, memory, floats, kernels, done, bits, 4, s_high, 2 * s_high,
                                   (size_t)1 << (done - 4), (((size_t)1 << bits) - 1) * s_high,
                                   (n / 2) / (s_high << bits));
        done += bits;
    }
    int three = last_bits == 5; /* la3 or la2 */

    /* la: butterfly j of region r (output bits 0 and 2) has the low output bits
     * K = k0 + 2 k1 + 4 k2 + 8 k3 + 16 j. Output bits 1 and 3 (k1, k3) select the vector at
     * first and move into the lanes with the two exchanges: k3 to lane bit b0, k1 to q1.
     *   la3 only: index bit 4, a scalar twiddle exp(-2 pi i 16 K / N) per (k1, k3)
     *   index bit 3: a twiddle vector exp(-2 pi i 8 K / N) per k1, k3 varying over the lanes
     *   index bit 2: a twiddle vector exp(-2 pi i 4 K / N), for la3 a second one turned by an
     *                eighth for the upper output of index bit 4 */
    size_t start = floats, count = n >> (three ? 9 : 8);
    for (size_t r = 0; r < 4; r++) {
        for (size_t j = 0; j < count; j++) {
            uint64_t base = (r >> 1) + 4 * (r & 1) + 16 * j;
            float *table = memory ? memory + floats : NULL;
            if (three) {
                for (size_t s = 0; s < 4; s++) onefft_table_root(table, 4, s, 16 * (base + 2 * (s >> 1) + 8 * (s & 1)), log2n);
                floats += 8;
            }
            for (size_t k1 = 0; k1 < 2; k1++) {
                table = memory ? memory + floats : NULL;
                for (size_t lane = 0; lane < 16; lane++) {
                    onefft_table_root(table, 16, lane, 8 * (base + 2 * k1 + 8 * (lane & 1)), log2n);
                }
                floats += 32;
            }
            for (size_t c = 0; c < (three ? 2u : 1u); c++) {
                table = memory ? memory + floats : NULL;
                for (size_t lane = 0; lane < 16; lane++) {
                    onefft_table_root(table, 16, lane, 4 * (base + 2 * (lane >> 3) + 8 * (lane & 1)) + (n >> 3) * c, log2n);
                }
                floats += 32;
            }
        }
    }
    if (memory) {
        onefft_last *l = &plan->last[0];
        l->run = three ? kernels->la : kernels->la2;
        l->tw = memory + start;
        l->regions = 4;
        l->stride = three ? n / 4 : n / 2;
        l->count = count;
    }
    floats = (floats + 15) & ~(size_t)15;

    /* lb: butterfly j handles the output indices K + (N/4) q, K = element + 16 j. */
    start = floats;
    for (size_t j = 0; j < n >> 6; j++) {
        for (size_t l = 1; l < 4; l++) {
            float *table = memory ? memory + floats : NULL;
            for (size_t lane = 0; lane < 16; lane++) {
                onefft_table_root(table, 16, lane, l * (onefft_avx512_element[lane] + 16 * j), log2n);
            }
            floats += 32;
        }
    }
    if (memory) {
        onefft_last *l = &plan->last[1];
        l->run = kernels->lb[inverse];
        l->tw = memory + start;
        l->regions = 1;
        l->stride = 2 * n;
        l->count = n >> 6;
        plan->last_count = 2;
        plan->execute = onefft_execute_passes;
    }
    return floats * sizeof(float);
}

#endif /* ONEFFT_AVX512 */

/* The best instruction set that this build contains and the processor and operating system
 * support: 4 for AVX-512, 3 for AVX2 with FMA, 2 for SSE2 (the number of lane bits). */
static unsigned onefft_x86_select(void) {
#ifdef ONEFFT_AVX2
    __builtin_cpu_init();
    if (__builtin_cpu_supports("avx2") && __builtin_cpu_supports("fma")) {
#ifdef ONEFFT_AVX512
        if (__builtin_cpu_supports("avx512f")) return 4;
#endif
        return 3;
    }
#endif
    return 2;
}

static size_t onefft_x86_build(onefft_plan *plan, float *memory) {
    switch (onefft_x86_select()) {
#ifdef ONEFFT_AVX512
    case 4: return onefft_avx512_plan(plan, memory);
#endif
#ifdef ONEFFT_AVX2
    case 3: return onefft_x86_plan(plan, memory, &onefft_avx2_kernels);
#endif
    default: return onefft_x86_plan(plan, memory, &onefft_sse2_kernels);
    }
}

#endif /* ONEFFT_X86 */

/* ---- public interface ---------------------------------------------------------------------- */

static size_t onefft_build(onefft_plan *plan, float *memory) {
#if defined(ONEFFT_NEON)
    return onefft_neon_plan(plan, memory);
#elif defined(ONEFFT_X86)
    return onefft_x86_build(plan, memory);
#else
    return onefft_scalar_plan(plan, memory);
#endif
}

const char *onefft_implementation(void) {
#if defined(ONEFFT_NEON)
    return ONEFFT_NEON_FMA ? "neon (fused)" : "neon";
#elif defined(ONEFFT_X86)
    switch (onefft_x86_select()) {
    case 4: return "avx512";
    case 3: return "avx2";
    default: return "sse2";
    }
#else
    return "portable";
#endif
}

static void *onefft_aligned_alloc(size_t bytes) {
#ifdef _WIN32
    return _aligned_malloc(bytes, 64);
#else
    void *memory = NULL;
    return posix_memalign(&memory, 64, bytes) ? NULL : memory;
#endif
}

static void onefft_aligned_free(void *memory) {
#ifdef _WIN32
    _aligned_free(memory);
#else
    free(memory);
#endif
}

onefft_plan *onefft_plan_create(size_t n, int direction) {
    if (n == 0 || (n & (n - 1)) != 0) return NULL;
    if (direction != ONEFFT_FORWARD && direction != ONEFFT_INVERSE) return NULL;
    onefft_plan *plan = (onefft_plan *)calloc(1, sizeof(onefft_plan));
    if (!plan) return NULL;
    plan->n = n;
    while (((size_t)1 << plan->log2n) < n) plan->log2n++;
    plan->inverse = direction == ONEFFT_INVERSE;
    /* Tables address the data with 32 bit offsets, in units of complex values. */
    if (plan->log2n > ONEFFT_MAX_LOG2) {
        free(plan);
        return NULL;
    }
    size_t bytes = onefft_build(plan, NULL);
    plan->memory = onefft_aligned_alloc(bytes + 64);
    if (!plan->memory) {
        free(plan);
        return NULL;
    }
    onefft_build(plan, (float *)plan->memory);
    return plan;
}

void onefft_plan_destroy(onefft_plan *plan) {
    if (!plan) return;
    onefft_aligned_free(plan->memory);
    free(plan);
}

void onefft_plan_execute(const onefft_plan *plan, const float *in, float *out) { plan->execute(plan, in, out); }

size_t onefft_plan_size(const onefft_plan *plan) { return plan->n; }

/* ---- the plan cache behind onefft_forward() and onefft_inverse() ----------------------------
 *
 * One slot per direction and size. A slot is written once, with a compare-and-swap, and only
 * read afterwards, so transforms need no lock. If two threads create the plan for a slot at the
 * same time, one of them wins and the other frees its copy.
 */

static onefft_plan *onefft_cache[2][8 * sizeof(size_t)];

static const onefft_plan *onefft_cache_get(size_t n, int direction) {
    if (n == 0 || (n & (n - 1)) != 0) return NULL;
    unsigned log2n = (unsigned)__builtin_ctzll((unsigned long long)n);
    onefft_plan **slot = &onefft_cache[direction][log2n];
    onefft_plan *plan = __atomic_load_n(slot, __ATOMIC_ACQUIRE);
    if (plan) return plan;
    plan = onefft_plan_create(n, direction);
    if (!plan) return NULL;
    onefft_plan *expected = NULL;
    if (!__atomic_compare_exchange_n(slot, &expected, plan, 0, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE)) {
        onefft_plan_destroy(plan);
        plan = expected;
    }
    return plan;
}

int onefft_forward(const float *in, float *out, size_t n) {
    const onefft_plan *plan = onefft_cache_get(n, ONEFFT_FORWARD);
    if (!plan) return -1;
    plan->execute(plan, in, out);
    return 0;
}

int onefft_inverse(const float *in, float *out, size_t n) {
    const onefft_plan *plan = onefft_cache_get(n, ONEFFT_INVERSE);
    if (!plan) return -1;
    plan->execute(plan, in, out);
    return 0;
}

int onefft_cache_prepare(size_t n, int direction) {
    if (direction != ONEFFT_FORWARD && direction != ONEFFT_INVERSE) return -1;
    return onefft_cache_get(n, direction) ? 0 : -1;
}

void onefft_cache_clear(void) {
    for (int direction = 0; direction < 2; direction++) {
        for (size_t i = 0; i < 8 * sizeof(size_t); i++) {
            onefft_plan_destroy(__atomic_exchange_n(&onefft_cache[direction][i], NULL, __ATOMIC_ACQ_REL));
        }
    }
}
