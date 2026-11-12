/*
 * Copyright 2021 Torben Thaysen
 */

#include "fft_radix_16.h"
#include "../fft.h"
#include <android/log.h>
#include <math.h>

void transformFFT16(void *fftP){
    FFT *fft = fftP;
    int n = fft->n;
    int m = fft->m;
    int r = 4;
    complex **t = fft->t;

    complex *x = fft->b;
    fft->out = fft->b;

    for(int i=0;i<n;i++){
        x[i] = fft->in[fft->reverse[i]];
    }

    static const float fft4_data[] = {
            +1.0f, +1.0f, +1.0f, +1.0f,
            +1.0f, -1.0f, +1.0f, -1.0f,
            +1.0f, +0.0f, -1.0f, +0.0f,
            +0.0f, +1.0f, +0.0f, -1.0f
    };
    asm volatile (
    "ld1 { v16.4s, v17.4s, v18.4s, v19.4s }, [%0] \n\t"
    :
    :"r"(fft4_data)
    :"v16", "v17", "v18", "v19"
    );

    int l2 = 1;
    if((m & 1) == 0){
        for(int i=0;i<n;i+=16){
            //m even
            asm volatile (
            "ld2 {  v8.s,  v9.s }[0], [%0], #8 \n\t"
            "ld2 { v10.s, v11.s }[0], [%0], #8 \n\t"
            "ld2 { v12.s, v13.s }[0], [%0], #8 \n\t"
            "ld2 { v14.s, v15.s }[0], [%0], #8 \n\t"

            "ld2 {  v8.s,  v9.s }[1], [%0], #8 \n\t"
            "ld2 { v10.s, v11.s }[1], [%0], #8 \n\t"
            "ld2 { v12.s, v13.s }[1], [%0], #8 \n\t"
            "ld2 { v14.s, v15.s }[1], [%0], #8 \n\t"

            "ld2 {  v8.s,  v9.s }[2], [%0], #8 \n\t"
            "ld2 { v10.s, v11.s }[2], [%0], #8 \n\t"
            "ld2 { v12.s, v13.s }[2], [%0], #8 \n\t"
            "ld2 { v14.s, v15.s }[2], [%0], #8 \n\t"

            "ld2 {  v8.s,  v9.s }[3], [%0], #8 \n\t"
            "ld2 { v10.s, v11.s }[3], [%0], #8 \n\t"
            "ld2 { v12.s, v13.s }[3], [%0], #8 \n\t"
            "ld2 { v14.s, v15.s }[3], [%0], #8 \n\t"

            "sub %0, %0, #128 \n\t"


            do_fft4_x4(B1, B0, B1)


            "st2 {  v8.s,  v9.s }[0], [%0], #8 \n\t"
            "st2 { v10.s, v11.s }[0], [%0], #8 \n\t"
            "st2 { v12.s, v13.s }[0], [%0], #8 \n\t"
            "st2 { v14.s, v15.s }[0], [%0], #8 \n\t"

            "st2 {  v8.s,  v9.s }[1], [%0], #8 \n\t"
            "st2 { v10.s, v11.s }[1], [%0], #8 \n\t"
            "st2 { v12.s, v13.s }[1], [%0], #8 \n\t"
            "st2 { v14.s, v15.s }[1], [%0], #8 \n\t"

            "st2 {  v8.s,  v9.s }[2], [%0], #8 \n\t"
            "st2 { v10.s, v11.s }[2], [%0], #8 \n\t"
            "st2 { v12.s, v13.s }[2], [%0], #8 \n\t"
            "st2 { v14.s, v15.s }[2], [%0], #8 \n\t"

            "st2 {  v8.s,  v9.s }[3], [%0], #8 \n\t"
            "st2 { v10.s, v11.s }[3], [%0], #8 \n\t"
            "st2 { v12.s, v13.s }[3], [%0], #8 \n\t"
            "st2 { v14.s, v15.s }[3], [%0], #8 \n\t"

            "sub %0, %0, #128 \n\t"
            :
            :"r"(&x[i])
            :"v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", "v10", "v11", "v12", "v13", "v14", "v15"
            );
        }
        l2 *= 4;
        t += 1;
    }else{
        //m odd
        complex *t1 = t[1];
        asm volatile (

        "ld2 { v22.s, v23.s }[0], [%0], #8 \n\t"
        "ld2 { v24.s, v25.s }[0], [%0], #8 \n\t"
        "ld2 { v26.s, v27.s }[0], [%0], #8 \n\t"
        "ld2 { v28.s, v29.s }[0], [%0], #8 \n\t"

        "ld2 { v22.s, v23.s }[1], [%0], #8 \n\t"
        "ld2 { v24.s, v25.s }[1], [%0], #8 \n\t"
        "ld2 { v26.s, v27.s }[1], [%0], #8 \n\t"
        "ld2 { v28.s, v29.s }[1], [%0], #8 \n\t"

        "ld2 { v22.s, v23.s }[2], [%0], #8 \n\t"
        "ld2 { v24.s, v25.s }[2], [%0], #8 \n\t"
        "ld2 { v26.s, v27.s }[2], [%0], #8 \n\t"
        "ld2 { v28.s, v29.s }[2], [%0], #8 \n\t"

        "ld2 { v22.s, v23.s }[3], [%0], #8 \n\t"
        "ld2 { v24.s, v25.s }[3], [%0], #8 \n\t"
        "ld2 { v26.s, v27.s }[3], [%0], #8 \n\t"
        "ld2 { v28.s, v29.s }[3], [%0], #8 \n\t"

        "sub %0, %0, #128 \n\t"
        :
        :"r"(t1)
        :"v20", "v21", "v22", "v23", "v24", "v25", "v26", "v27", "v28", "v29", "v30", "v31"
        );
        for(int i=0;i<n;i+=16){
            asm volatile (
            "ld2 {  v8.s,  v9.s }[0], [%0], #8 \n\t"
            "ld2 { v10.s, v11.s }[0], [%0], #8 \n\t"
            "ld2 { v12.s, v13.s }[0], [%0], #8 \n\t"
            "ld2 { v14.s, v15.s }[0], [%0], #8 \n\t"

            "ld2 {  v8.s,  v9.s }[1], [%0], #8 \n\t"
            "ld2 { v10.s, v11.s }[1], [%0], #8 \n\t"
            "ld2 { v12.s, v13.s }[1], [%0], #8 \n\t"
            "ld2 { v14.s, v15.s }[1], [%0], #8 \n\t"

            "ld2 {  v8.s,  v9.s }[2], [%0], #8 \n\t"
            "ld2 { v10.s, v11.s }[2], [%0], #8 \n\t"
            "ld2 { v12.s, v13.s }[2], [%0], #8 \n\t"
            "ld2 { v14.s, v15.s }[2], [%0], #8 \n\t"

            "ld2 {  v8.s,  v9.s }[3], [%0], #8 \n\t"
            "ld2 { v10.s, v11.s }[3], [%0], #8 \n\t"
            "ld2 { v12.s, v13.s }[3], [%0], #8 \n\t"
            "ld2 { v14.s, v15.s }[3], [%0], #8 \n\t"

            "sub %0, %0, #128 \n\t"


            do_fft4_x4(B1, B0, B1)


            "uzp1 v0.4s,  v8.4s, v10.4s \n\t"
            "uzp2 v2.4s,  v8.4s, v10.4s \n\t"
            "uzp1 v1.4s,  v9.4s, v11.4s \n\t"
            "uzp2 v3.4s,  v9.4s, v11.4s \n\t"

            "uzp1 v4.4s, v12.4s, v14.4s \n\t"
            "uzp2 v6.4s, v12.4s, v14.4s \n\t"
            "uzp1 v5.4s, v13.4s, v15.4s \n\t"
            "uzp2 v7.4s, v13.4s, v15.4s \n\t"

            "uzp1  v8.4s, v0.4s, v4.4s \n\t"
            "uzp2 v10.4s, v0.4s, v4.4s \n\t"
            "uzp1  v9.4s, v1.4s, v5.4s \n\t"
            "uzp2 v11.4s, v1.4s, v5.4s \n\t"

            "uzp1 v12.4s, v2.4s, v6.4s \n\t"
            "uzp2 v14.4s, v2.4s, v6.4s \n\t"
            "uzp1 v13.4s, v3.4s, v7.4s \n\t"
            "uzp2 v15.4s, v3.4s, v7.4s \n\t"

            "fmul v0.4s,  v8.4s, v22.4s \n\t"
            "fmul v1.4s,  v9.4s, v22.4s \n\t"
            "fmul v2.4s, v12.4s, v24.4s \n\t"
            "fmul v3.4s, v13.4s, v24.4s \n\t"
            "fmul v4.4s, v10.4s, v26.4s \n\t"
            "fmul v5.4s, v11.4s, v26.4s \n\t"
            "fmul v6.4s, v14.4s, v28.4s \n\t"
            "fmul v7.4s, v15.4s, v28.4s \n\t"

            "fmls v0.4s,  v9.4s, v23.4s \n\t"
            "fmla v1.4s,  v8.4s, v23.4s \n\t"
            "fmls v2.4s, v13.4s, v25.4s \n\t"
            "fmla v3.4s, v12.4s, v25.4s \n\t"
            "fmls v4.4s, v11.4s, v27.4s \n\t"
            "fmla v5.4s, v10.4s, v27.4s \n\t"
            "fmls v6.4s, v15.4s, v29.4s \n\t"
            "fmla v7.4s, v14.4s, v29.4s \n\t"


            do_fft4_x4(B0, B1, B0)


            "st2 { v0.4s, v1.4s }, [%0], #32 \n\t"
            "st2 { v2.4s, v3.4s }, [%0], #32 \n\t"
            "st2 { v4.4s, v5.4s }, [%0], #32 \n\t"
            "st2 { v6.4s, v7.4s }, [%0], #32 \n\t"

            "sub %0, %0, #128 \n\t"
            :
            :"r"(&x[i])
            :"v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", "v10", "v11", "v12", "v13", "v14", "v15", "v16", "v17", "v18", "v19", "v20", "v21", "v22", "v23", "v24", "v25", "v26", "v27", "v28", "v29", "cc"
            );
        }
        l2 *= 16;
        t += 2;
    }
    while(l2 != n>>2){
        complex *t0 = t[0];
        complex *t1 = t[1];
        for(int j=0;j<l2;j++){
            asm volatile (
            "ld2 { v20.4s, v21.4s }, [%0] \n\t"

            "sub %2, %2, #24 \n\t"

            "ld2 { v22.s, v23.s }[0], [%1], #8 \n\t"
            "ld2 { v24.s, v25.s }[0], [%1], #8 \n\t"
            "ld2 { v26.s, v27.s }[0], [%1], #8 \n\t"
            "ld2 { v28.s, v29.s }[0], [%1], %2 \n\t"

            "ld2 { v22.s, v23.s }[1], [%1], #8 \n\t"
            "ld2 { v24.s, v25.s }[1], [%1], #8 \n\t"
            "ld2 { v26.s, v27.s }[1], [%1], #8 \n\t"
            "ld2 { v28.s, v29.s }[1], [%1], %2 \n\t"

            "ld2 { v22.s, v23.s }[2], [%1], #8 \n\t"
            "ld2 { v24.s, v25.s }[2], [%1], #8 \n\t"
            "ld2 { v26.s, v27.s }[2], [%1], #8 \n\t"
            "ld2 { v28.s, v29.s }[2], [%1], %2 \n\t"

            "ld2 { v22.s, v23.s }[3], [%1], #8 \n\t"
            "ld2 { v24.s, v25.s }[3], [%1], #8 \n\t"
            "ld2 { v26.s, v27.s }[3], [%1], #8 \n\t"
            "ld2 { v28.s, v29.s }[3], [%1], %2 \n\t"

            "add %2, %2, #24 \n\t"
            "sub %1, %1, %2, lsl 2 \n\t"
            :
            :"r"(&t0[j*4]), "r"(&t1[j*4]), "r"(l2*4*sizeof(complex))
            :"v20", "v21", "v22", "v23", "v24", "v25", "v26", "v27", "v28", "v29", "v30", "v31"
            );
            for(int i=0;i<n;i+=l2*16){
                asm volatile (
                "ld2 { v0.s, v1.s }[0], [%0], %1 \n\t"
                "ld2 { v2.s, v3.s }[0], [%0], %1 \n\t"
                "ld2 { v4.s, v5.s }[0], [%0], %1 \n\t"
                "ld2 { v6.s, v7.s }[0], [%0], %1 \n\t"

                "ld2 { v0.s, v1.s }[1], [%0], %1 \n\t"
                "ld2 { v2.s, v3.s }[1], [%0], %1 \n\t"
                "ld2 { v4.s, v5.s }[1], [%0], %1 \n\t"
                "ld2 { v6.s, v7.s }[1], [%0], %1 \n\t"

                "ld2 { v0.s, v1.s }[2], [%0], %1 \n\t"
                "ld2 { v2.s, v3.s }[2], [%0], %1 \n\t"
                "ld2 { v4.s, v5.s }[2], [%0], %1 \n\t"
                "ld2 { v6.s, v7.s }[2], [%0], %1 \n\t"

                "ld2 { v0.s, v1.s }[3], [%0], %1 \n\t"
                "ld2 { v2.s, v3.s }[3], [%0], %1 \n\t"
                "ld2 { v4.s, v5.s }[3], [%0], %1 \n\t"
                "ld2 { v6.s, v7.s }[3], [%0], %1 \n\t"

                "sub %0, %0, %1, lsl 4 \n\t"


                "fmul  v8.4s, v0.4s, v20.s[0] \n\t"
                "fmul  v9.4s, v1.4s, v20.s[0] \n\t"
                "fmul v10.4s, v2.4s, v20.s[1] \n\t"
                "fmul v11.4s, v3.4s, v20.s[1] \n\t"
                "fmul v12.4s, v4.4s, v20.s[2] \n\t"
                "fmul v13.4s, v5.4s, v20.s[2] \n\t"
                "fmul v14.4s, v6.4s, v20.s[3] \n\t"
                "fmul v15.4s, v7.4s, v20.s[3] \n\t"

                "fmls  v8.4s, v1.4s, v21.s[0] \n\t"
                "fmla  v9.4s, v0.4s, v21.s[0] \n\t"
                "fmls v10.4s, v3.4s, v21.s[1] \n\t"
                "fmla v11.4s, v2.4s, v21.s[1] \n\t"
                "fmls v12.4s, v5.4s, v21.s[2] \n\t"
                "fmla v13.4s, v4.4s, v21.s[2] \n\t"
                "fmls v14.4s, v7.4s, v21.s[3] \n\t"
                "fmla v15.4s, v6.4s, v21.s[3] \n\t"


                do_fft4_x4(B1, B0, B1)


                "uzp1 v0.4s,  v8.4s, v10.4s \n\t"
                "uzp2 v2.4s,  v8.4s, v10.4s \n\t"
                "uzp1 v1.4s,  v9.4s, v11.4s \n\t"
                "uzp2 v3.4s,  v9.4s, v11.4s \n\t"

                "uzp1 v4.4s, v12.4s, v14.4s \n\t"
                "uzp2 v6.4s, v12.4s, v14.4s \n\t"
                "uzp1 v5.4s, v13.4s, v15.4s \n\t"
                "uzp2 v7.4s, v13.4s, v15.4s \n\t"

                "uzp1  v8.4s, v0.4s, v4.4s \n\t"
                "uzp2 v10.4s, v0.4s, v4.4s \n\t"
                "uzp1  v9.4s, v1.4s, v5.4s \n\t"
                "uzp2 v11.4s, v1.4s, v5.4s \n\t"

                "uzp1 v12.4s, v2.4s, v6.4s \n\t"
                "uzp2 v14.4s, v2.4s, v6.4s \n\t"
                "uzp1 v13.4s, v3.4s, v7.4s \n\t"
                "uzp2 v15.4s, v3.4s, v7.4s \n\t"

                "fmul v0.4s,  v8.4s, v22.4s \n\t"
                "fmul v1.4s,  v9.4s, v22.4s \n\t"
                "fmul v2.4s, v12.4s, v24.4s \n\t"
                "fmul v3.4s, v13.4s, v24.4s \n\t"
                "fmul v4.4s, v10.4s, v26.4s \n\t"
                "fmul v5.4s, v11.4s, v26.4s \n\t"
                "fmul v6.4s, v14.4s, v28.4s \n\t"
                "fmul v7.4s, v15.4s, v28.4s \n\t"

                "fmls v0.4s,  v9.4s, v23.4s \n\t"
                "fmla v1.4s,  v8.4s, v23.4s \n\t"
                "fmls v2.4s, v13.4s, v25.4s \n\t"
                "fmla v3.4s, v12.4s, v25.4s \n\t"
                "fmls v4.4s, v11.4s, v27.4s \n\t"
                "fmla v5.4s, v10.4s, v27.4s \n\t"
                "fmls v6.4s, v15.4s, v29.4s \n\t"
                "fmla v7.4s, v14.4s, v29.4s \n\t"


                do_fft4_x4(B0, B1, B0)


                "st2 { v0.s, v1.s }[0], [%0], %1 \n\t"
                "st2 { v0.s, v1.s }[1], [%0], %1 \n\t"
                "st2 { v0.s, v1.s }[2], [%0], %1 \n\t"
                "st2 { v0.s, v1.s }[3], [%0], %1 \n\t"

                "st2 { v2.s, v3.s }[0], [%0], %1 \n\t"
                "st2 { v2.s, v3.s }[1], [%0], %1 \n\t"
                "st2 { v2.s, v3.s }[2], [%0], %1 \n\t"
                "st2 { v2.s, v3.s }[3], [%0], %1 \n\t"

                "st2 { v4.s, v5.s }[0], [%0], %1 \n\t"
                "st2 { v4.s, v5.s }[1], [%0], %1 \n\t"
                "st2 { v4.s, v5.s }[2], [%0], %1 \n\t"
                "st2 { v4.s, v5.s }[3], [%0], %1 \n\t"

                "st2 { v6.s, v7.s }[0], [%0], %1 \n\t"
                "st2 { v6.s, v7.s }[1], [%0], %1 \n\t"
                "st2 { v6.s, v7.s }[2], [%0], %1 \n\t"
                "st2 { v6.s, v7.s }[3], [%0], %1 \n\t"

                "sub %0, %0, %1, lsl 4 \n\t"
                :
                :"r"(&x[i+j]), "r"(l2*sizeof(complex))
                :"v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", "v10", "v11", "v12", "v13", "v14", "v15", "v16", "v17", "v18", "v19", "v20", "v21", "v22", "v23", "v24", "v25", "v26", "v27", "v28", "v29", "cc"
                );
            }
        }
        l2 *= 16;
        t += 2;
    }

    complex *tt = *t;
    for(int j=0;j<n>>2;j++){
        asm volatile (
        "ld2 { v20.4s, v21.4s }, [%2] \n\t"

        "ld2 { v2.s, v3.s }[0], [%0], %1 \n\t"
        "ld2 { v2.s, v3.s }[1], [%0], %1 \n\t"
        "ld2 { v2.s, v3.s }[2], [%0], %1 \n\t"
        "ld2 { v2.s, v3.s }[3], [%0], %1 \n\t"
        "sub %0, %0, %1, lsl 2 \n\t"

        "fmul v0.4s, v2.4s, v20.4s \n\t"
        "fmul v1.4s, v3.4s, v20.4s \n\t"
        "fmls v0.4s, v3.4s, v21.4s \n\t"
        "fmla v1.4s, v2.4s, v21.4s \n\t"

        "fmul v2.4s, v16.4s, v0.s[0] \n\t"
        "fmul v3.4s, v16.4s, v1.s[0] \n\t"

        "fmla v2.4s, v17.4s, v0.s[2] \n\t"
        "fmla v3.4s, v17.4s, v1.s[2] \n\t"

        "fmla v2.4s, v18.4s, v0.s[1] \n\t"
        "fmla v3.4s, v18.4s, v1.s[1] \n\t"
        "fmla v2.4s, v19.4s, v1.s[1] \n\t"
        "fmls v3.4s, v19.4s, v0.s[1] \n\t"

        "fmla v2.4s, v18.4s, v0.s[3] \n\t"
        "fmla v3.4s, v18.4s, v1.s[3] \n\t"
        "fmls v2.4s, v19.4s, v1.s[3] \n\t"
        "fmla v3.4s, v19.4s, v0.s[3] \n\t"

        "st2 { v2.s, v3.s }[0], [%0], %1 \n\t"
        "st2 { v2.s, v3.s }[1], [%0], %1 \n\t"
        "st2 { v2.s, v3.s }[2], [%0], %1 \n\t"
        "st2 { v2.s, v3.s }[3], [%0], %1 \n\t"
        "sub %0, %0, %1, lsl 2 \n\t"
        :
        :"r"(&x[j]), "r"((n>>2)*sizeof(complex)), "r"(&tt[j*4])
        :"v0", "v1", "v2", "v3", "v16", "v17", "v18", "v19", "v20", "v21", "cc"
        );
    }
}