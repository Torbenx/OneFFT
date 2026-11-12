#include "../fft.h"
#include "fft_radix_16.h"
#include "fft_load.h"
#include "fft_weights.h"

/*
 * Copyright 2021 Torben Thaysen
 */

void freeWeightsFFT_128_ASIMD(void *fftP){
    FFT *fft = fftP;
    free(fft->t);
}

void createWeightsFFT_128_ASIMD(FFT *fft){
    float *t = malloc(8 * 16 * 2 * sizeof(float));
    for(int i=0;i<8;i+=2){
        for(int l=0;l<16;l+=2){
            float arg0 = F_2PI * (i+0) * (l+0) / 128;
            float arg1 = F_2PI * (i+1) * (l+0) / 128;
            float arg2 = F_2PI * (i+0) * (l+1) / 128;
            float arg3 = F_2PI * (i+1) * (l+1) / 128;
            t[i*16*2+l*4+0] = tcos(arg0);
            t[i*16*2+l*4+1] = tcos(arg1);
            t[i*16*2+l*4+2] = tcos(arg2);
            t[i*16*2+l*4+3] = tcos(arg3);
            t[i*16*2+l*4+4] = tsin(arg0);
            t[i*16*2+l*4+5] = tsin(arg1);
            t[i*16*2+l*4+6] = tsin(arg2);
            t[i*16*2+l*4+7] = tsin(arg3);
        }
    }
    fft->t = t;
    fft->freeWeights = freeWeightsFFT_128_ASIMD;
}

#define store_load2 \
    store_load_by_reg_single(B0, "%[shift]", B0, "%[shift]") \
    "sub %[output], %[output], #1016 \n\t" \
    "sub %[input], %[input], #1016 \n\t" \

void transformFFT_128_ASIMD(void *fftP){
    FFT *fft = fftP;

    complex *in = fft->in;
    complex *x = fft->b;
    complex *out = x;
    complex *t = fft->t;
    fft->out = out;

    asm volatile (
    "ld1 { v24.4s, v25.4s, v26.4s, v27.4s }, [%0], #64 \n\t"
    "ld1 { v28.4s, v29.4s, v30.4s, v31.4s }, [%0] \n\t"
    :
    :"r"(fft_ASIMD_data_f)
    :"v24", "v25", "v26", "v27", "v28", "v29", "v30", "v31"
    );

    asm volatile (
    "ld2 {  v0.4s,  v1.4s }, [%[input]], %[shiftFull] \n\t"
    "ld2 {  v2.4s,  v3.4s }, [%[input]], %[shiftFull] \n\t"
    "ld2 {  v4.4s,  v5.4s }, [%[input]], %[shiftFull] \n\t"
    "ld2 {  v6.4s,  v7.4s }, [%[input]], %[shiftFull] \n\t"
    "ld2 {  v8.4s,  v9.4s }, [%[input]], %[shiftFull] \n\t"
    "ld2 { v10.4s, v11.4s }, [%[input]], %[shiftFull] \n\t"
    "ld2 { v12.4s, v13.4s }, [%[input]], %[shiftFull] \n\t"
    "ld2 { v14.4s, v15.4s }, [%[input]], %[shiftFull] \n\t"
    "sub %[input], %[input], #992 \n\t"
    do_fft8_x4(B0, B1, B2, B0, B1, "v30.s[0]")
    store_by_elm_load_by_reg_double_(B0, B1, "#8", "#8", B2, B0, "#8", "%[shift]")
    "sub %[input], %[input], #992 \n\t"
    do_fft8_x4(B2, B0, B1, B0, B2, "v30.s[0]")
    store_by_elm_load_by_reg_double_(B0, B2, "#8", "#8", B1, B0, "#8", "%[shift]")
    "sub %[input], %[input], #992 \n\t"
    do_fft8_x4(B1, B0, B2, B0, B1, "v30.s[0]")
    store_by_elm_load_by_reg_double_(B0, B1, "#8", "#8", B2, B0, "#8", "%[shift]")
    "sub %[input], %[input], #992 \n\t"
    do_fft8_x4(B2, B0, B1, B0, B2, "v30.s[0]")

    :[input]"+r"(in), [output]"+r"(out)
    :[shift]"r"((16-3)*sizeof(complex)), [shiftFull]"r"(16*sizeof(complex))
    :REGISTERS
    );

    in = x;
    asm volatile (
    "st2 { " B0_0r ".s, " B0_0i ".s }[0], [%[output]], #8 \n\t"
    "st2 { " B0_1r ".s, " B0_1i ".s }[0], [%[output]], #8 \n\t"
    "ld2 {" B1_0r ".s," B1_0i ".s}[0], [%[input]], #8 \n\t"
    "st2 { " B0_2r ".s, " B0_2i ".s }[0], [%[output]], #8 \n\t"
    "st2 { " B0_3r ".s, " B0_3i ".s }[0], [%[output]], #8 \n\t"
    "ld2 {" B1_0r ".s," B1_0i ".s}[1], [%[input]], %[shift] \n\t"
    "st2 { " B2_0r ".s, " B2_0i ".s }[0], [%[output]], #8 \n\t"
    "st2 { " B2_1r ".s, " B2_1i ".s }[0], [%[output]], #8 \n\t"
    "ld2 {" B1_0r ".s," B1_0i ".s}[2], [%[input]], #8 \n\t"
    "st2 { " B2_2r ".s, " B2_2i ".s }[0], [%[output]], #8 \n\t"
    "st2 { " B2_3r ".s, " B2_3i ".s }[0], [%[output]], #8 \n\t"
    "ld2 {" B1_0r ".s," B1_0i ".s}[3], [%[input]], %[shift] \n\t"
    "st2 { " B0_0r ".s, " B0_0i ".s }[1], [%[output]], #8 \n\t"
    "st2 { " B0_1r ".s, " B0_1i ".s }[1], [%[output]], #8 \n\t"
    "ld2 {" B1_1r ".s," B1_1i ".s}[0], [%[input]], #8 \n\t"
    "st2 { " B0_2r ".s, " B0_2i ".s }[1], [%[output]], #8 \n\t"
    "st2 { " B0_3r ".s, " B0_3i ".s }[1], [%[output]], #8 \n\t"
    "ld2 {" B1_1r ".s," B1_1i ".s}[1], [%[input]], %[shift] \n\t"
    "st2 { " B2_0r ".s, " B2_0i ".s }[1], [%[output]], #8 \n\t"
    "st2 { " B2_1r ".s, " B2_1i ".s }[1], [%[output]], #8 \n\t"
    "ld2 {" B1_1r ".s," B1_1i ".s}[2], [%[input]], #8 \n\t"
    "st2 { " B2_2r ".s, " B2_2i ".s }[1], [%[output]], #8 \n\t"
    "st2 { " B2_3r ".s, " B2_3i ".s }[1], [%[output]], #8 \n\t"
    "ld2 {" B1_1r ".s," B1_1i ".s}[3], [%[input]], %[shift] \n\t"
    "st2 { " B0_0r ".s, " B0_0i ".s }[2], [%[output]], #8 \n\t"
    "st2 { " B0_1r ".s, " B0_1i ".s }[2], [%[output]], #8 \n\t"
    "ld2 {" B1_2r ".s," B1_2i ".s}[0], [%[input]], #8 \n\t"
    "st2 { " B0_2r ".s, " B0_2i ".s }[2], [%[output]], #8 \n\t"
    "st2 { " B0_3r ".s, " B0_3i ".s }[2], [%[output]], #8 \n\t"
    "ld2 {" B1_2r ".s," B1_2i ".s}[1], [%[input]], %[shift] \n\t"
    "st2 { " B2_0r ".s, " B2_0i ".s }[2], [%[output]], #8 \n\t"
    "st2 { " B2_1r ".s, " B2_1i ".s }[2], [%[output]], #8 \n\t"
    "ld2 {" B1_2r ".s," B1_2i ".s}[2], [%[input]], #8 \n\t"
    "st2 { " B2_2r ".s, " B2_2i ".s }[2], [%[output]], #8 \n\t"
    "st2 { " B2_3r ".s, " B2_3i ".s }[2], [%[output]], #8 \n\t"
    "ld2 {" B1_2r ".s," B1_2i ".s}[3], [%[input]], %[shift] \n\t"
    "st2 { " B0_0r ".s, " B0_0i ".s }[3], [%[output]], #8 \n\t"
    "st2 { " B0_1r ".s, " B0_1i ".s }[3], [%[output]], #8 \n\t"
    "ld2 {" B1_3r ".s," B1_3i ".s}[0], [%[input]], #8 \n\t"
    "st2 { " B0_2r ".s, " B0_2i ".s }[3], [%[output]], #8 \n\t"
    "st2 { " B0_3r ".s, " B0_3i ".s }[3], [%[output]], #8 \n\t"
    "ld2 {" B1_3r ".s," B1_3i ".s}[1], [%[input]], %[shift] \n\t"
    "st2 { " B2_0r ".s, " B2_0i ".s }[3], [%[output]], #8 \n\t"
    "st2 { " B2_1r ".s, " B2_1i ".s }[3], [%[output]], #8 \n\t"
    "ld2 {" B1_3r ".s," B1_3i ".s}[2], [%[input]], #8 \n\t"
    "st2 { " B2_2r ".s, " B2_2i ".s }[3], [%[output]], #8 \n\t"
    "st2 { " B2_3r ".s, " B2_3i ".s }[3], [%[output]], #8 \n\t"
    "ld2 {" B1_3r ".s," B1_3i ".s}[3], [%[input]], %[shift] \n\t"
    do_weights(B1, B0)
    load_by_reg_shift2_single(B2, "#8", "%[shift]")
    do_weights(B2, B1)
    "sub %[input], %[input], #1008 \n\t"
    :[input]"+r"(in), [output]"+r"(out), [weights]"+r"(t)
    :[shift]"r"((8-1)*sizeof(complex))
    );
    out = x;

    asm volatile (

    do_fft16_x2(B0, B1, B2, B0, B1)

    store_by_reg_alt_shift2_load_by_reg_shift2_single(B0, "#8", "%[shift]", B2, "#8", "%[shift]")
    do_weights(B2, B0)
    store_by_reg_alt_shift2_load_by_reg_shift2_single(B1, "#8", "%[shift]", B2, "#8", "%[shift]")
    do_weights(B2, B1)
    "sub %[output], %[output], #1008 \n\t"
    "sub %[input], %[input], #1008 \n\t"

    do_fft16_x2(B0, B1, B2, B0, B1)

    store_by_reg_alt_shift2_load_by_reg_shift2_single(B0, "#8", "%[shift]", B2, "#8", "%[shift]")
    do_weights(B2, B0)
    store_by_reg_alt_shift2_load_by_reg_shift2_single(B1, "#8", "%[shift]", B2, "#8", "%[shift]")
    do_weights(B2, B1)
    "sub %[output], %[output], #1008 \n\t"
    "sub %[input], %[input], #1008 \n\t"

    do_fft16_x2(B0, B1, B2, B0, B1)

    store_by_reg_alt_shift2_load_by_reg_shift2_single(B0, "#8", "%[shift]", B2, "#8", "%[shift]")
    do_weights(B2, B0)
    store_by_reg_alt_shift2_load_by_reg_shift2_single(B1, "#8", "%[shift]", B2, "#8", "%[shift]")
    do_weights(B2, B1)
    "sub %[output], %[output], #1008 \n\t"
    "sub %[input], %[input], #1008 \n\t"

    do_fft16_x2(B0, B1, B2, B0, B1)

    store_by_reg_alt_shift2_single(B0, "#8", "%[shift]")
    store_by_reg_alt_shift2_single(B1, "#8", "%[shift]")
    "sub %[output], %[output], #1008 \n\t"

    :[input]"+r"(in), [output]"+r"(out), [weights]"+r"(t)
    :[shift]"r"((8-1)*sizeof(complex))
    );
}