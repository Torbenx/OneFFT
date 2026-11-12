#include "../fft.h"
#include "fft_radix_16.h"
#include "fft_load.h"
#include "fft_weights.h"

/*
 * Copyright 2021 Torben Thaysen
 */

void freeWeightsFFT16_256_ASIMD(void *fftP){
    FFT *fft = fftP;
    free(fft->t);
}

void createWeightsFFT16_256_ASIMD(FFT *fft){
    float *t = malloc(16 * 16 * 2 * sizeof(float));
    for(int l=0;l<16;l+=2){
        for(int k=0;k<16;k+=2){
            float arg0 = F_2PI * (l+0) * (k+0) / 256;
            float arg1 = F_2PI * (l+1) * (k+0) / 256;
            float arg2 = F_2PI * (l+0) * (k+1) / 256;
            float arg3 = F_2PI * (l+1) * (k+1) / 256;
            t[l*16*2+k*4+0] = tcos(arg0);
            t[l*16*2+k*4+1] = tcos(arg1);
            t[l*16*2+k*4+2] = tcos(arg2);
            t[l*16*2+k*4+3] = tcos(arg3);
            t[l*16*2+k*4+4] = tsin(arg0);
            t[l*16*2+k*4+5] = tsin(arg1);
            t[l*16*2+k*4+6] = tsin(arg2);
            t[l*16*2+k*4+7] = tsin(arg3);
        }
    }
    fft->t = t;
    fft->freeWeights = freeWeightsFFT16_256_ASIMD;
}

#define store_load1(s_bank0, s_bank1, l_bank0, l_bank1) \
    store_by_reg_even_alt_load_by_reg_shift2_single(s_bank0, s_bank1, "#8", "#8", l_bank0, "#8", "%[shift]") \
    store_by_reg_odd_alt_load_by_reg_shift2_single(s_bank0, s_bank1, "#8", "#8", l_bank1, "#8", "%[shift]") \
    "sub %[input], %[input], #2032 \n\t" \

#define store_load_weights2 \
    store_by_reg_alt_shift2_load_by_reg_shift2_single(B0, "#8", "%[shift]", B2, "#8", "%[shift]") \
    do_weights(B2, B0) \
    store_by_reg_alt_shift2_load_by_reg_shift2_single(B1, "#8", "%[shift]", B2, "#8", "%[shift]") \
    do_weights(B2, B1) \
    "sub %[output], %[output], #2032 \n\t" \
    "sub %[input], %[input], #2032 \n\t" \

void transformFFT16_256_ASIMD(void *fftP) {
    FFT *fft = fftP;

    complex *x = fft->b;
    complex *in = fft->in;
    float *t = fft->t;
    fft->out = x;
    complex *out = x;

    asm volatile (
    "ld1 { v24.4s, v25.4s, v26.4s, v27.4s }, [%0], #64 \n\t"
    "ld1 { v28.4s, v29.4s, v30.4s, v31.4s }, [%0] \n\t"
    :
    :"r"(fft_ASIMD_data_f)
    :"v24", "v25", "v26", "v27", "v28", "v29", "v30", "v31"
    );


    asm volatile (
    load_by_reg_shift2_single(B0, "#8", "%[shift]")
    load_by_reg_shift2_single(B1, "#8", "%[shift]")
    "sub %[input], %[input], #2032 \n\t"
    do_fft16_x2(B0, B1, B2, B0, B1)
    store_load1(B0, B1, B2, B0)
    do_fft16_x2(B2, B0, B1, B0, B2)
    store_load1(B0, B2, B1, B0)
    do_fft16_x2(B1, B0, B2, B0, B1)
    store_load1(B0, B1, B2, B0)
    do_fft16_x2(B2, B0, B1, B0, B2)
    store_load1(B0, B2, B1, B0)
    do_fft16_x2(B1, B0, B2, B0, B1)
    store_load1(B0, B1, B2, B0)
    do_fft16_x2(B2, B0, B1, B0, B2)
    store_load1(B0, B2, B1, B0)
    do_fft16_x2(B1, B0, B2, B0, B1)
    store_load1(B0, B1, B2, B0)
    do_fft16_x2(B2, B0, B1, B2, B0)

    :[output]"+r"(out), [input]"+r"(in)
    :[shift]"r"((16-1)*sizeof(complex))
    :REGISTERS
    );

    in = x;
    asm volatile (
    store_by_reg_even_alt_load_by_reg_shift2_single(B2, B0, "#8", "#8", B1, "#8", "%[shift]")
    store_by_reg_odd_alt_load_by_reg_shift2_single(B2, B0, "#8", "#8", B2, "#8", "%[shift]")
    do_weights(B1, B0)
    do_weights(B2, B1)
    "sub %[input], %[input], #2032 \n\t"

    :[weights]"+r"(t), [input]"+r"(in), [output]"+r"(out)
    :[shift]"r"((16-1)*sizeof(complex))
    :REGISTERS
    );
    out = x;

    asm volatile (
    do_fft16_x2(B0, B1, B2, B0, B1)
    store_load_weights2
    do_fft16_x2(B0, B1, B2, B0, B1)
    store_load_weights2
    do_fft16_x2(B0, B1, B2, B0, B1)
    store_load_weights2
    do_fft16_x2(B0, B1, B2, B0, B1)
    store_load_weights2
    do_fft16_x2(B0, B1, B2, B0, B1)
    store_load_weights2
    do_fft16_x2(B0, B1, B2, B0, B1)
    store_load_weights2
    do_fft16_x2(B0, B1, B2, B0, B1)
    store_load_weights2
    do_fft16_x2(B0, B1, B2, B0, B1)
    store_by_reg_alt_shift2_single(B0, "#8", "%[shift]")
    store_by_reg_alt_shift2_single(B1, "#8", "%[shift]")

    :[weights]"+r"(t), [input]"+r"(in), [output]"+r"(out)
    :[shift]"r"((16-1)*sizeof(complex))
    :REGISTERS
    );
}