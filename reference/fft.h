/*
 * Copyright 2019 Torben Thaysen
 */

#include <pthread.h>
#include <stdio.h>
#include "log.h"
#include "cpu.h"

#define LOGD_COMPLEX_ARRAY(TAG,array,n) {char *temp__ = malloc(n * 100);int i__;int o__ = 0;for(i__=0;i__<n;i__++){o__ += sprintf(temp__+o__,"{%+f, %+f},",(array)[i__].r,(array)[i__].i);}LOGD(TAG,"%s",temp__);free(temp__);}

#define tcos(x) __builtin_cosf(x)
#define tsin(x) __builtin_sinf(x)
#define tsqrt(x) __builtin_sqrtf(x)

#define F_2PI (-6.283185307179586476925286766559f) //-2.0f * PI

#define FLAG_DIRECTION_FORWARDS 1
#define FLAG_DIRECTION_BACKWARDS 2
#define FLAG_DIRECTION_BOTH 3

#ifndef FFT_H
#define FFT_H

struct complex_s {
    float r;
    float i;
};
typedef struct complex_s complex;

typedef struct{
    complex *w,*b,*in,*out;
    int *reverse,m,n,radix,direction,forward;
    void (*transformFft)(void*);
    void *t;
    void (*freeWeights)(void*);
} FFT;

typedef struct{
    void (*preComputeWeights)(FFT*);
    void (*preComputeReverse)(FFT*);
    void (*transform)(void*);
    int radix;
    int minN;
    int maxN;
    int flags;
    void *next;
} FFT_Routine;

void transformFFT2(void *fft);
void transformFFT4(void *fft);
void transformFFT2_2(void *fft);
void transformFFT2_4(void *fft);
void transformFFT2_4_r(void *fft);
void transformFFT2_BUFFER(void *fft);
void transformFFT2_REC(void *fft);

void createWeightsRec(FFT *fft);
void createWeightsRec_r(FFT *fft);

#if HAVE_NEON
void transformFFT2_NEON_2(void *fft);
void transformFFT2_NEON_4(void *fftP);
void transformFFT2_NEON_4_r(void *fft);
void transformFFT2_NEON_8(void *fftP);
void transformFFT2_NEON_8_r(void *fft);
void transformFFT2_NEON_16(void *fft);
void transformFFT2_NEON_16_r(void *fft);
void transformFFT2_NEON_32(void *fft);
void transformFFT2_NEON_32_r(void *fft);
void transformFFT2_NEON(void *fft);
void transformFFT2_NEON_BUFFER(void *fft);
void transformFFT2_NEON_BUFFER_2(void *fft);
void transformFFT2_NEON_BUFFER_2_r(void *fft);
void transformFFT2_REC_NEON(void *fft);
void transformFFT2_REC_NEON_r(void *fft);
void transformFFT4_NEON(void *fft);
void transformFFT4_NEON_r(void *fft);

void createWeights_NEON_8(FFT *fft);
void createWeights_NEON_8_r(FFT *fft);
void createWeights_NEON_16(FFT *fft);
void createWeights_NEON_16_r(FFT *fft);
void createWeights_NEON_32(FFT *fft);
void createWeights_NEON_32_r(FFT *fft);
void createWeights_NEON_BUFFER(FFT *fft);
void createWeights_NEON_BUFFER(FFT *fft);
void createWeights_NEON_BUFFER_r(FFT *fft);
void createWeights_REC_NEON(FFT *fft);
void createWeights_REC_NEON_r(FFT *fft);
void createWeights4_NEON(FFT *fft);
void createWeights4_NEON_r(FFT *fft);
#endif
#if __aarch64__
void transformFFT16(void *fftP);
void transformFFT4_16_ASIMD(void *fftP);
void transformFFT_32_ASIMD(void *fftP);
void transformFFT_64_ASIMD(void *fftP);
void transformFFT_128_ASIMD(void *fftP);
void transformFFT16_256_ASIMD(void *fftP);
void transformFFT_512_ASIMD(void *fftP);
void transformFFT_1024_ASIMD(void *fftP);

void createWeightsFFT_64_ASIMD(FFT *fft);
void createWeightsFFT_128_ASIMD(FFT *fft);
void createWeightsFFT16_256_ASIMD(FFT *fft);
void createWeightsFFT_512_ASIMD(FFT *fft);
void createWeightsFFT_1024_ASIMD(FFT *fft);
#endif

FFT* createFFT(int n,int forward);
void freeFFT(FFT *fft);
int logi(int i,int p);
int powi(int x,int y);

#endif //FFT_H
