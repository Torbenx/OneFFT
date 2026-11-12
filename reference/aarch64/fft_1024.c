#include "../fft.h"
#include "fft_radix_16.h"
#include "fft_load.h"
#include "fft_weights.h"

/*
 * Copyright 2021 Torben Thaysen
 */

void freeWeightsFFT_1024_ASIMD(void *fftP){
    FFT *fft = fftP;
    free(fft->t);
}

void createWeightsFFT_1024_ASIMD(FFT *fft){
    float *t = malloc(31 * 32 * 2 * sizeof(float));
    for(int l=1;l<32;l++){
        for(int k=0;k<32;k+=4){
            float arg0 = F_2PI * l * (k+0) / 1024;
            float arg1 = F_2PI * l * (k+1) / 1024;
            float arg2 = F_2PI * l * (k+2) / 1024;
            float arg3 = F_2PI * l * (k+3) / 1024;
            t[(l-1)*32*2+k*2+0] = tcos(arg0);
            t[(l-1)*32*2+k*2+1] = tcos(arg1);
            t[(l-1)*32*2+k*2+2] = tcos(arg2);
            t[(l-1)*32*2+k*2+3] = tcos(arg3);
            t[(l-1)*32*2+k*2+4] = tsin(arg0);
            t[(l-1)*32*2+k*2+5] = tsin(arg1);
            t[(l-1)*32*2+k*2+6] = tsin(arg2);
            t[(l-1)*32*2+k*2+7] = tsin(arg3);
        }
    }
    fft->t = t;
    fft->freeWeights = freeWeightsFFT_1024_ASIMD;
}

#define store_load1(s_bank0, s_bank1, l_bank0, l_bank1) \
    store_load_by_reg_single(s_bank0, "#8", l_bank0, "%[shift]") \
    store_load_by_reg_single(s_bank1, "#8", l_bank1, "%[shift]") \
    "sub %[input], %[input], %[minus] \n\t" \

#define store_load_weights2 \
    store_load_by_reg_single(B0, "%[shift]", B2, "%[shift]") \
    do_weights(B2, B0) \
    store_load_by_reg_single(B1, "%[shift]", B2, "%[shift]") \
    do_weights(B2, B1) \
    "sub %[output], %[output], %[minus] \n\t" \
    "sub %[input], %[input], %[minus] \n\t" \

void transformFFT_1024_ASIMD(void *fftP) {
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
    load_by_reg_single(B0, "%[shift]")
    load_by_reg_single(B1, "%[shift]")
    "sub %[input], %[input], %[minus] \n\t"
    do_fft32(B0, B1, B2, B0, B1)
    store_load1(B0, B1, B2, B0)
    do_fft32(B2, B0, B1, B0, B2)
    store_load1(B0, B2, B1, B0)
    do_fft32(B1, B0, B2, B0, B1)
    store_load1(B0, B1, B2, B0)
    do_fft32(B2, B0, B1, B0, B2)
    store_load1(B0, B2, B1, B0)
    do_fft32(B1, B0, B2, B0, B1)
    store_load1(B0, B1, B2, B0)
    do_fft32(B2, B0, B1, B0, B2)
    store_load1(B0, B2, B1, B0)
    do_fft32(B1, B0, B2, B0, B1)
    store_load1(B0, B1, B2, B0)
    do_fft32(B2, B0, B1, B0, B2)
    store_load1(B0, B2, B1, B0)
    do_fft32(B1, B0, B2, B0, B1)
    store_load1(B0, B1, B2, B0)
    do_fft32(B2, B0, B1, B0, B2)
    store_load1(B0, B2, B1, B0)
    do_fft32(B1, B0, B2, B0, B1)
    store_load1(B0, B1, B2, B0)
    do_fft32(B2, B0, B1, B0, B2)
    store_load1(B0, B2, B1, B0)
    do_fft32(B1, B0, B2, B0, B1)
    store_load1(B0, B1, B2, B0)
    do_fft32(B2, B0, B1, B0, B2)
    store_load1(B0, B2, B1, B0)
    do_fft32(B1, B0, B2, B0, B1)
    store_load1(B0, B1, B2, B0)
    do_fft32(B2, B0, B1, B0, B2)
    store_load1(B0, B2, B1, B0)
    do_fft32(B1, B0, B2, B0, B1)
    store_load1(B0, B1, B2, B0)
    do_fft32(B2, B0, B1, B0, B2)
    store_load1(B0, B2, B1, B0)
    do_fft32(B1, B0, B2, B0, B1)
    store_load1(B0, B1, B2, B0)
    do_fft32(B2, B0, B1, B0, B2)
    store_load1(B0, B2, B1, B0)
    do_fft32(B1, B0, B2, B0, B1)
    store_load1(B0, B1, B2, B0)
    do_fft32(B2, B0, B1, B0, B2)
    store_load1(B0, B2, B1, B0)
    do_fft32(B1, B0, B2, B0, B1)
    store_load1(B0, B1, B2, B0)
    do_fft32(B2, B0, B1, B0, B2)
    store_load1(B0, B2, B1, B0)
    do_fft32(B1, B0, B2, B0, B1)
    store_load1(B0, B1, B2, B0)
    do_fft32(B2, B0, B1, B0, B2)
    store_load1(B0, B2, B1, B0)
    do_fft32(B1, B0, B2, B0, B1)
    store_load1(B0, B1, B2, B0)
    do_fft32(B2, B0, B1, B0, B2)
    store_load1(B0, B2, B1, B0)
    do_fft32(B1, B0, B2, B0, B1)
    store_load1(B0, B1, B2, B0)
    do_fft32(B2, B0, B1, B0, B2)
    store_load1(B0, B2, B1, B0)
    do_fft32(B1, B0, B2, B0, B1)
    store_load1(B0, B1, B2, B0)
    do_fft32(B2, B0, B1, B0, B2)

    :[output]"+r"(out), [input]"+r"(in)
    :[shift]"r"(32*sizeof(complex)), [minus]"r"((1024-1)*sizeof(complex))
    :REGISTERS
    );

    in = x;
    asm volatile (
    store_load_by_reg_single(B0, "#8", B1, "%[shift]")
    store_load_by_reg_single(B2, "#8", B0, "%[shift]")
    "sub %[input], %[input], %[minus] \n\t"
    :[weights]"+r"(t), [input]"+r"(in), [output]"+r"(out)
    :[shift]"r"(32*sizeof(complex)), [minus]"r"((1024-1)*sizeof(complex))
    :REGISTERS
    );
    out = x;

    asm volatile (
    do_fft32(B1, B0, B2, B0, B1)
    store_load_weights2
    do_fft32(B0, B1, B2, B0, B1)
    store_load_weights2
    do_fft32(B0, B1, B2, B0, B1)
    store_load_weights2
    do_fft32(B0, B1, B2, B0, B1)
    store_load_weights2
    do_fft32(B0, B1, B2, B0, B1)
    store_load_weights2
    do_fft32(B0, B1, B2, B0, B1)
    store_load_weights2
    do_fft32(B0, B1, B2, B0, B1)
    store_load_weights2
    do_fft32(B0, B1, B2, B0, B1)
    store_load_weights2
    do_fft32(B0, B1, B2, B0, B1)
    store_load_weights2
    do_fft32(B0, B1, B2, B0, B1)
    store_load_weights2
    do_fft32(B0, B1, B2, B0, B1)
    store_load_weights2
    do_fft32(B0, B1, B2, B0, B1)
    store_load_weights2
    do_fft32(B0, B1, B2, B0, B1)
    store_load_weights2
    do_fft32(B0, B1, B2, B0, B1)
    store_load_weights2
    do_fft32(B0, B1, B2, B0, B1)
    store_load_weights2
    do_fft32(B0, B1, B2, B0, B1)
    store_load_weights2
    do_fft32(B0, B1, B2, B0, B1)
    store_load_weights2
    do_fft32(B0, B1, B2, B0, B1)
    store_load_weights2
    do_fft32(B0, B1, B2, B0, B1)
    store_load_weights2
    do_fft32(B0, B1, B2, B0, B1)
    store_load_weights2
    do_fft32(B0, B1, B2, B0, B1)
    store_load_weights2
    do_fft32(B0, B1, B2, B0, B1)
    store_load_weights2
    do_fft32(B0, B1, B2, B0, B1)
    store_load_weights2
    do_fft32(B0, B1, B2, B0, B1)
    store_load_weights2
    do_fft32(B0, B1, B2, B0, B1)
    store_load_weights2
    do_fft32(B0, B1, B2, B0, B1)
    store_load_weights2
    do_fft32(B0, B1, B2, B0, B1)
    store_load_weights2
    do_fft32(B0, B1, B2, B0, B1)
    store_load_weights2
    do_fft32(B0, B1, B2, B0, B1)
    store_load_weights2
    do_fft32(B0, B1, B2, B0, B1)
    store_load_weights2
    do_fft32(B0, B1, B2, B0, B1)
    store_load_weights2
    do_fft32(B0, B1, B2, B0, B1)
    store_by_reg_single(B0, "%[shift]")
    store_by_reg_single(B1, "%[shift]")

    :[weights]"+r"(t), [input]"+r"(in), [output]"+r"(out)
    :[shift]"r"(32*sizeof(complex)), [minus]"r"((1024-1)*sizeof(complex))
    :REGISTERS
    );
}