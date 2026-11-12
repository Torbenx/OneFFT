/*
 * Copyright 2021 Torben Thaysen
 */

#include "fft_radix_16.h"
#include "../fft.h"

float fft4_16_ASIMD_data_f[] = {
        1.0f, +F_COS_PI_8, +F_SQRT_1_2, +F_SIN_PI_8,
        0.0f, -F_SIN_PI_8, -F_SQRT_1_2, -F_COS_PI_8,

        1.0f, +F_SQRT_1_2, +0.0f      , -F_SQRT_1_2,
        0.0f, -F_SQRT_1_2, -1.0f      , -F_SQRT_1_2,

        1.0f, +F_SIN_PI_8, -F_SQRT_1_2, -F_COS_PI_8,
        0.0f, -F_COS_PI_8, -F_SQRT_1_2, +F_SIN_PI_8,

        +F_COS_PI_16, (+F_COS_PI_16+F_SIN_PI_16)*F_SQRT_1_2, (+F_COS_PI_16-F_SIN_PI_16)*F_SQRT_1_2, +F_SIN_PI_16,
        -F_SIN_PI_16, (-F_COS_PI_16+F_SIN_PI_16)*F_SQRT_1_2, (-F_COS_PI_16-F_SIN_PI_16)*F_SQRT_1_2, -F_COS_PI_16
};
float fft4_32_ASIMD_data_f[] = {
        1.0f, +F_COS_PI_16, +F_COS_PI_8, (+F_COS_PI_16+F_SIN_PI_16)*F_SQRT_1_2,
        0.0f, -F_SIN_PI_16, -F_SIN_PI_8, (-F_COS_PI_16+F_SIN_PI_16)*F_SQRT_1_2,

        1.0f, +F_SQRT_1_2, +0.0f      , -F_SQRT_1_2,
        0.0f, -F_SQRT_1_2, -1.0f      , -F_SQRT_1_2,

        1.0f, +F_SIN_PI_8, -F_SQRT_1_2, -F_COS_PI_8,
        0.0f, -F_COS_PI_8, -F_SQRT_1_2, +F_SIN_PI_8,

        +F_SQRT_1_2, (+F_COS_PI_16-F_SIN_PI_16)*F_SQRT_1_2, +F_SIN_PI_8, +F_SIN_PI_16,
        -F_SQRT_1_2, (-F_COS_PI_16-F_SIN_PI_16)*F_SQRT_1_2, -F_COS_PI_8, -F_COS_PI_16
};
/*float fft_ASIMD_data_f[] = {
        1.0f, +F_COS_PI_16, +F_COS_PI_8, (+F_COS_PI_16+F_SIN_PI_16)*F_SQRT_1_2,
        0.0f, -F_SIN_PI_16, -F_SIN_PI_8, (-F_COS_PI_16+F_SIN_PI_16)*F_SQRT_1_2,

        +F_SQRT_1_2, (+F_COS_PI_16-F_SIN_PI_16)*F_SQRT_1_2, +F_SIN_PI_8, +F_SIN_PI_16,
        -F_SQRT_1_2, (-F_COS_PI_16-F_SIN_PI_16)*F_SQRT_1_2, -F_COS_PI_8, -F_COS_PI_16,

        +0.0f, +0.0f, -F_SQRT_1_2, -F_SQRT_1_2,
        -1.0f, -1.0f, -F_SQRT_1_2, -F_SQRT_1_2,

        -F_SQRT_1_2, -F_SQRT_1_2, F_SIN_PI_8, F_SIN_PI_8
};*/

float fft_ASIMD_data_f[] = {
        +1.0f, +1.0f, +F_SQRT_1_2, +F_SQRT_1_2,
        +0.0f, +0.0f, -F_SQRT_1_2, -F_SQRT_1_2,

        +F_COS_PI_8, +F_COS_PI_8, +F_SIN_PI_8, +F_SIN_PI_8,
        -F_SIN_PI_8, -F_SIN_PI_8, -F_COS_PI_8, -F_COS_PI_8,

        1.0f, +F_COS_PI_16, +F_COS_PI_8, (+F_COS_PI_16+F_SIN_PI_16)*F_SQRT_1_2,
        0.0f, -F_SIN_PI_16, -F_SIN_PI_8, (-F_COS_PI_16+F_SIN_PI_16)*F_SQRT_1_2,

        +F_SQRT_1_2, (+F_COS_PI_16-F_SIN_PI_16)*F_SQRT_1_2, +F_SIN_PI_8, +F_SIN_PI_16,
        -F_SQRT_1_2, (-F_COS_PI_16-F_SIN_PI_16)*F_SQRT_1_2, -F_COS_PI_8, -F_COS_PI_16,
};

void transformFFT4_16_ASIMD(void *fftP){
    FFT *fft = fftP;

    complex *x = fft->b;
    complex *in = fft->in;
    fft->out = x;

    /*
     *  0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
     * 00 01 02 03 10 11 12 13 20 21 22 23 30 31 32 33
     *
     * 00 10 20 30 01 11 21 31 02 12 22 32 03 13 23 33
     *  0  4  8  c  1  5  9  d  2  6  a  e  3  7  b  f
     */

    /*
        0 1 2 3 4 5 6 7 8 9 a b c d e f

        0 4 8 c|1 5 9 d|2 6 a e|3 7 b f

        0 2 8 a|4 6 c e|1 3 9 b|5 7 d f

        0 1 2 3|4 5 6 7|8 9 a b|c d e f


        0 4 8 c|1 5 9 d|2 6 a e|3 7 b f

        0 8 1 9|4 c 5 9|2 a 3 b|6 e 7 f

        0 1 2 3|4 5 6 7|8 9 a b|c d e f
    */

    asm volatile (


    "ld2 {  v8.4s,  v9.4s }, [%0], #32 \n\t"
    "ld2 { v10.4s, v11.4s }, [%0], #32 \n\t"
    "ld2 { v12.4s, v13.4s }, [%0], #32 \n\t"
    "fadd " B0_0r ".4s, " B1_0r ".4s, " B1_2r ".4s \n\t"
    "fadd " B0_0i ".4s, " B1_0i ".4s, " B1_2i ".4s \n\t"
    "fsub " B0_1r ".4s, " B1_0r ".4s, " B1_2r ".4s \n\t"
    "fsub " B0_1i ".4s, " B1_0i ".4s, " B1_2i ".4s \n\t"

    "ld2 { v14.4s, v15.4s }, [%0] \n\t"
    "fadd " B0_2r ".4s, " B1_1r ".4s, " B1_3r ".4s \n\t"
    "fadd " B0_2i ".4s, " B1_1i ".4s, " B1_3i ".4s \n\t"
    "fsub " B0_3r ".4s, " B1_1r ".4s, " B1_3r ".4s \n\t"
    "fsub " B0_3i ".4s, " B1_1i ".4s, " B1_3i ".4s \n\t"

    "ld1 { v24.4s, v25.4s }, [%2], #32 \n\t"
    "fadd " B1_0r ".4s, " B0_0r ".4s, " B0_2r ".4s \n\t"
    "fadd " B1_0i ".4s, " B0_0i ".4s, " B0_2i ".4s \n\t"

    "ld1 { v26.4s, v27.4s }, [%2], #32 \n\t"
    "fadd " B1_1r ".4s, " B0_1r ".4s, " B0_3i ".4s \n\t"
    "fsub " B1_1i ".4s, " B0_1i ".4s, " B0_3r ".4s \n\t"

    "ld1 { v28.4s, v29.4s }, [%2] \n\t"
    "fsub " B1_2r ".4s, " B0_0r ".4s, " B0_2r ".4s \n\t"
    "fsub " B1_2i ".4s, " B0_0i ".4s, " B0_2i ".4s \n\t"

    "fsub " B1_3r ".4s, " B0_1r ".4s, " B0_3i ".4s \n\t"
    "fadd " B1_3i ".4s, " B0_1i ".4s, " B0_3r ".4s \n\t"

    "uzp1 " B0_1r ".4s, " B1_0r ".4s, " B1_1r ".4s \n\t"
    "uzp2 " B0_0r ".4s, " B1_0r ".4s, " B1_1r ".4s \n\t"
    "uzp1 " B0_1i ".4s, " B1_0i ".4s, " B1_1i ".4s \n\t"
    "uzp2 " B0_0i ".4s, " B1_0i ".4s, " B1_1i ".4s \n\t"
    
    "uzp1 " B0_3r ".4s, " B1_2r ".4s, " B1_3r ".4s \n\t"
    "uzp2 " B0_2r ".4s, " B1_2r ".4s, " B1_3r ".4s \n\t"
    "uzp1 " B0_3i ".4s, " B1_2i ".4s, " B1_3i ".4s \n\t"
    "uzp2 " B0_2i ".4s, " B1_2i ".4s, " B1_3i ".4s \n\t"
    
    "uzp1 " B1_1r ".4s, " B0_0r ".4s, " B0_2r ".4s \n\t"
    "uzp2 " B1_3r ".4s, " B0_0r ".4s, " B0_2r ".4s \n\t"
    "uzp1 " B1_1i ".4s, " B0_0i ".4s, " B0_2i ".4s \n\t"
    "uzp2 " B1_3i ".4s, " B0_0i ".4s, " B0_2i ".4s \n\t"
    
    "uzp1  " B0_0r ".4s, " B0_1r ".4s, " B0_3r ".4s \n\t"
    "uzp2 " B1_2r ".4s, " B0_1r ".4s, " B0_3r ".4s \n\t"
    "uzp1  " B0_0i ".4s, " B0_1i ".4s, " B0_3i ".4s \n\t"
    "uzp2 " B1_2i ".4s, " B0_1i ".4s, " B0_3i ".4s \n\t"
    
    "fmul " B0_1r ".4s, " B1_1r ".4s, v24.4s \n\t"
    "fmul " B0_1i ".4s, " B1_1i ".4s, v24.4s \n\t"
    "fmul " B0_2r ".4s, " B1_2r ".4s, v26.4s \n\t"
    "fmul " B0_2i ".4s, " B1_2i ".4s, v26.4s \n\t"
    "fmul " B0_3r ".4s, " B1_3r ".4s, v28.4s \n\t"
    "fmul " B0_3i ".4s, " B1_3i ".4s, v28.4s \n\t"
    
    "fmls " B0_1r ".4s, " B1_1i ".4s, v25.4s \n\t"
    "fmla " B0_1i ".4s, " B1_1r ".4s, v25.4s \n\t"
    "fmls " B0_2r ".4s, " B1_2i ".4s, v27.4s \n\t"
    "fmla " B0_2i ".4s, " B1_2r ".4s, v27.4s \n\t"
    "fmls " B0_3r ".4s, " B1_3i ".4s, v29.4s \n\t"
    "fmla " B0_3i ".4s, " B1_3r ".4s, v29.4s \n\t"

    "fadd " B1_0r ".4s, " B0_0r ".4s, " B0_2r ".4s \n\t"
    "fadd " B1_0i ".4s, " B0_0i ".4s, " B0_2i ".4s \n\t"
    "fsub " B1_1r ".4s, " B0_0r ".4s, " B0_2r ".4s \n\t"
    "fsub " B1_1i ".4s, " B0_0i ".4s, " B0_2i ".4s \n\t"

    "fadd " B1_2r ".4s, " B0_1r ".4s, " B0_3r ".4s \n\t"
    "fadd " B1_2i ".4s, " B0_1i ".4s, " B0_3i ".4s \n\t"
    "fsub " B1_3r ".4s, " B0_1r ".4s, " B0_3r ".4s \n\t"
    "fsub " B1_3i ".4s, " B0_1i ".4s, " B0_3i ".4s \n\t"

    "fadd " B0_0r ".4s, " B1_0r ".4s, " B1_2r ".4s \n\t"
    "fadd " B0_0i ".4s, " B1_0i ".4s, " B1_2i ".4s \n\t"

    "fadd " B0_1r ".4s, " B1_1r ".4s, " B1_3i ".4s \n\t"
    "fsub " B0_1i ".4s, " B1_1i ".4s, " B1_3r ".4s \n\t"
    "st2 { v0.4s, v1.4s }, [%1], #32 \n\t"

    "fsub " B0_2r ".4s, " B1_0r ".4s, " B1_2r ".4s \n\t"
    "fsub " B0_2i ".4s, " B1_0i ".4s, " B1_2i ".4s \n\t"
    "st2 { v2.4s, v3.4s }, [%1], #32 \n\t"

    "fsub " B0_3r ".4s, " B1_1r ".4s, " B1_3i ".4s \n\t"
    "fadd " B0_3i ".4s, " B1_1i ".4s, " B1_3r ".4s \n\t"
    "st2 { v4.4s, v5.4s }, [%1], #32 \n\t"
    "st2 { v6.4s, v7.4s }, [%1] \n\t"

    :"+r"(in), "+r"(x)
    :"r"(fft4_16_ASIMD_data_f)
    :REGISTERS
    );
}