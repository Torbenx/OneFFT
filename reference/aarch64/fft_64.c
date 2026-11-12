#include "../fft.h"
#include "fft_radix_16.h"
#include "fft_load.h"
#include "fft_weights.h"

/*
 * Copyright 2021 Torben Thaysen
 */

void freeWeightsFFT_64_ASIMD(void *fftP){
    FFT *fft = fftP;
    free(fft->t);
}

void createWeightsFFT_64_ASIMD(FFT *fft){
    float *t = malloc(8 * 8 * 2 * sizeof(float));
    for(int l=0;l<8;l++){
        int ll = l*4*2;
        float arg0 = F_2PI * l * 0 / 64;
        float arg1 = F_2PI * l * 1 / 64;
        float arg2 = F_2PI * l * 2 / 64;
        float arg3 = F_2PI * l * 3 / 64;
        t[ll+0] = tcos(arg0);
        t[ll+1] = tcos(arg1);
        t[ll+2] = tcos(arg2);
        t[ll+3] = tcos(arg3);
        t[ll+4] = tsin(arg0);
        t[ll+5] = tsin(arg1);
        t[ll+6] = tsin(arg2);
        t[ll+7] = tsin(arg3);
    }
    for(int l=0;l<8;l++){
        int ll = 8*4*2 + l*4*2;
        float arg0 = F_2PI * l * 4 / 64;
        float arg1 = F_2PI * l * 5 / 64;
        float arg2 = F_2PI * l * 6 / 64;
        float arg3 = F_2PI * l * 7 / 64;
        t[ll+0] = tcos(arg0);
        t[ll+1] = tcos(arg1);
        t[ll+2] = tcos(arg2);
        t[ll+3] = tcos(arg3);
        t[ll+4] = tsin(arg0);
        t[ll+5] = tsin(arg1);
        t[ll+6] = tsin(arg2);
        t[ll+7] = tsin(arg3);
    }
    fft->t = t;
    fft->freeWeights = freeWeightsFFT_64_ASIMD;
}

void transformFFT_64_ASIMD(void *fftP){
    FFT *fft = fftP;

    complex *in = fft->in;
    complex *x = fft->b;
    float *t = fft->t;
    complex *out = x;
    fft->out = x;

    asm volatile (
    "mov v31.s[3], %w0 \n\t"
    :
    :"r"(F_SQRT_1_2)
    :"v24", "v25", "v26", "v27", "v28", "v29", "v30", "v31"
    );

    asm volatile (
    "ld2 {  v0.4s,  v1.4s }, [%[input]], %[shift] \n\t"
    "ld2 {  v2.4s,  v3.4s }, [%[input]], %[shift] \n\t"
    "ld2 {  v4.4s,  v5.4s }, [%[input]], %[shift] \n\t"
    "ld2 {  v6.4s,  v7.4s }, [%[input]], %[shift] \n\t"
    "ld2 {  v8.4s,  v9.4s }, [%[input]], %[shift] \n\t"
    "ld2 { v10.4s, v11.4s }, [%[input]], %[shift] \n\t"
    "ld2 { v12.4s, v13.4s }, [%[input]], %[shift] \n\t"
    "ld2 { v14.4s, v15.4s }, [%[input]], %[shift] \n\t"

    "sub %[input], %[input], #480 \n\t" //(8*8 - 4)*sizeof(complex)

    do_fft8_x4(B0, B1, B2, B1, B0, "v31.s[3]")

    store_by_elm_load_by_reg_double_(B1, B0, "#8", "#8",B2, B1, "#8", "%[shift2]")

    do_fft8_x4(B2, B1, B0, B1, B2, "v31.s[3]")

    :[input]"+r"(in), [output]"+r"(out)
    :[shift]"r"(8*sizeof(complex)), [shift2]"r"(40)
    :REGISTERS
    );

    in = x;

    asm volatile (

    store_by_elm_load_by_reg_double_(B1, B2, "#8", "#8", B0, B1, "#8", "%[shift2]")
    "sub %[input], %[input], #480 \n\t" //(8*8 - 4)*sizeof(complex)

    :[input]"+r"(in), [output]"+r"(out)
    :[shift]"r"(8*sizeof(complex)), [shift2]"r"(40)
    :REGISTERS
    );

    out = x;

    asm volatile (

    do_weights(B0, B2)
    do_weights(B1, B0)

    do_fft8_x4(B2, B0, B1, B0, B2, "v31.s[3]")

    store_load_by_reg_double_(B0, B2, "#8", "%[shift2]", B0, B1, "#8", "%[shift2]")
    "sub %[output], %[output], #480 \n\t" //(8*8 - 4)*sizeof(complex)

    do_weights(B0, B2)
    do_weights(B1, B0)

    do_fft8_x4(B2, B0, B1, B0, B2, "v31.s[3]")

    "st2 {  v0.4s,  v1.4s }, [%[output]], %[shift] \n\t"
    "st2 {  v2.4s,  v3.4s }, [%[output]], %[shift] \n\t"
    "st2 {  v4.4s,  v5.4s }, [%[output]], %[shift] \n\t"
    "st2 {  v6.4s,  v7.4s }, [%[output]], %[shift] \n\t"

    "st2 { v16.4s, v17.4s }, [%[output]], %[shift] \n\t"
    "st2 { v18.4s, v19.4s }, [%[output]], %[shift] \n\t"
    "st2 { v20.4s, v21.4s }, [%[output]], %[shift] \n\t"
    "st2 { v22.4s, v23.4s }, [%[output]], %[shift] \n\t"

    :[input]"+r"(in), [output]"+r"(out), [weights]"+r"(t)
    :[shift]"r"(8*sizeof(complex)), [data]"r"(F_SQRT_1_2), [shift2]"r"(40)
    :REGISTERS
    );
}