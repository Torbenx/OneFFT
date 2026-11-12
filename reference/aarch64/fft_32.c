/*
 * Copyright 2021 Torben Thaysen
 */

#include "../fft.h"
#include "fft_radix_16.h"
#include "fft_load.h"

void transformFFT_32_ASIMD(void *fftP){
    FFT *fft = fftP;

    complex *in = fft->in;
    complex *out = fft->b;
    fft->out = out;

    asm volatile (
    "ld1 { v24.4s, v25.4s, v26.4s, v27.4s }, [%[data]], #64 \n\t"
    "ld1 { v28.4s, v29.4s, v30.4s, v31.4s }, [%[data]], #64 \n\t"

    "ld2 {  v0.4s,  v1.4s }, [%[input]], #32 \n\t"
    "ld2 {  v2.4s,  v3.4s }, [%[input]], #32 \n\t"
    "ld2 {  v4.4s,  v5.4s }, [%[input]], #32 \n\t"
    "ld2 {  v6.4s,  v7.4s }, [%[input]], #32 \n\t"
    "ld2 {  v8.4s,  v9.4s }, [%[input]], #32 \n\t"
    "ld2 { v10.4s, v11.4s }, [%[input]], #32 \n\t"
    "ld2 { v12.4s, v13.4s }, [%[input]], #32 \n\t"
    "ld2 { v14.4s, v15.4s }, [%[input]], #32 \n\t"

    do_fft32_bare(B0, B1, B2, B0, B1)

    "fadd " B0_0r ".4s, " B2_0r ".4s, " B1_0r ".4s \n\t"
    "fadd " B0_0i ".4s, " B2_0i ".4s, " B1_0i ".4s \n\t"
    "st2 {  v0.4s,  v1.4s }, [%[output]], #32 \n\t"
    "fadd " B0_1r ".4s, " B2_1r ".4s, " B1_1r ".4s \n\t"
    "fadd " B0_1i ".4s, " B2_1i ".4s, " B1_1i ".4s \n\t"
    "st2 {  v2.4s,  v3.4s }, [%[output]], #32 \n\t"
    "fadd " B0_2r ".4s, " B2_2r ".4s, " B1_2r ".4s \n\t"
    "fadd " B0_2i ".4s, " B2_2i ".4s, " B1_2i ".4s \n\t"
    "st2 {  v4.4s,  v5.4s }, [%[output]], #32 \n\t"
    "fadd " B0_3r ".4s, " B2_3r ".4s, " B1_3r ".4s \n\t"
    "fadd " B0_3i ".4s, " B2_3i ".4s, " B1_3i ".4s \n\t"
    "st2 {  v6.4s,  v7.4s }, [%[output]], #32 \n\t"

    "fsub " B1_0r ".4s, " B2_0r ".4s, " B1_0r ".4s \n\t"
    "fsub " B1_0i ".4s, " B2_0i ".4s, " B1_0i ".4s \n\t"
    "st2 {  v8.4s,  v9.4s }, [%[output]], #32 \n\t"
    "fsub " B1_1r ".4s, " B2_1r ".4s, " B1_1r ".4s \n\t"
    "fsub " B1_1i ".4s, " B2_1i ".4s, " B1_1i ".4s \n\t"
    "st2 { v10.4s, v11.4s }, [%[output]], #32 \n\t"
    "fsub " B1_2r ".4s, " B2_2r ".4s, " B1_2r ".4s \n\t"
    "fsub " B1_2i ".4s, " B2_2i ".4s, " B1_2i ".4s \n\t"
    "st2 { v12.4s, v13.4s }, [%[output]], #32 \n\t"
    "fsub " B1_3r ".4s, " B2_3r ".4s, " B1_3r ".4s \n\t"
    "fsub " B1_3i ".4s, " B2_3i ".4s, " B1_3i ".4s \n\t"
    "st2 { v14.4s, v15.4s }, [%[output]], #32 \n\t"

    :[input]"+r"(in), [output]"+r"(out)
    :[data]"r"(fft_ASIMD_data_f)
    :REGISTERS
    );
}