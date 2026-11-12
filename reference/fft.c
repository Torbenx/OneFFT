/*
 * Copyright 2019 Torben Thaysen
 */

#include <stdlib.h>
#include "fft.h"
#include <cpu-features.h>

#include "fft2_4.c"
#include "fft2_rec.c"

#define BACKWARDS

#include "fft2_4.c"
#include "fft2_rec.c"

void createReverse(FFT *fft){
    fft->reverse = (int *) malloc(fft->n * sizeof(int));

    int i,j,in,out;
    for(i=0;i<fft->n;i++){
        in = i;
        out = 0;
        for(j=0;j<fft->m;j++){
            out *= fft->radix;
            out += in % fft->radix;
            in /= fft->radix;
        }
        fft->reverse[i] = out;
    }
}

void createReverse8(FFT *fft){
    fft->reverse = (int *) malloc(fft->n * sizeof(int));

    int i,j,in,out;
    for(i=0;i<fft->n;i++){
        in = i;
        out = 0;
        for(j=0;j<fft->m;j++){
            out *= fft->radix;
            out += in % fft->radix;
            in /= fft->radix;
        }
        fft->reverse[i] = out * 8;
    }
}

void freeWeightsDefault(void *fftP){
    FFT *fft = fftP;
    complex **t = fft->t;
    for(int i=0;i<fft->m;i++){
        free(t[i]);
    }
    free(t);
}

void createWeightsDefault(FFT *fft){
    int r1 = fft->radix-1;
    complex **t = malloc(fft->m * sizeof(complex*));
    float arg;
    int l = 1;
    int len = fft->radix;
    float f_2pi = F_2PI * fft->direction;
    int i,j,k;
    for(i=0;i<fft->m;i++){
        t[i] = malloc(l * r1 * sizeof(complex));
        for(j=0;j<l;j++){
            for(k=0;k<r1;k++){
                arg = f_2pi * j * (k+1) / len;
                t[i][j*r1+k].r = tcos(arg);
                t[i][j*r1+k].i = tsin(arg);
            }
        }
        len *= fft->radix;
        l *= fft->radix;
    }
    fft->t = t;
    fft->freeWeights = freeWeightsDefault;
}

void createWeightsDefaultFull(FFT *fft){
    int r = fft->radix;
    complex **t = malloc(fft->m * sizeof(complex*));
    float arg;
    int l = 1;
    int len = fft->radix;
    float f_2pi = F_2PI * fft->direction;
    int i,j,k;
    for(i=0;i<fft->m;i++){
        t[i] = malloc(l * r * sizeof(complex));
        for(j=0;j<l;j++){
            for(k=0;k<r;k++){
                arg = f_2pi * j * k / len;
                t[i][j*r+k].r = tcos(arg);
                t[i][j*r+k].i = tsin(arg);
            }
        }
        len *= fft->radix;
        l *= fft->radix;
    }
    fft->t = t;
    fft->freeWeights = freeWeightsDefault;
}

FFT_Routine *firstFFT_Routine = NULL;

void* addFFT_Routine(void *lastR,int radix,int minM,int maxM,void (*reverse)(FFT*),void (*weights)(FFT*),int flags,void (*transform)(void*)){
    FFT_Routine *r = malloc(sizeof(FFT_Routine));
    r->radix = radix;
    r->preComputeWeights = weights;
    r->preComputeReverse = reverse;
    r->flags = flags;
    r->transform = transform;
    r->minN = minM ? powi(radix,minM) : 0;
    r->maxN = maxM ? powi(radix,maxM) : 0;
    r->next = NULL;
    if(lastR){
        ((FFT_Routine*)lastR)->next = r;
    }else{
        firstFFT_Routine = r;
    }
    return r;
}

void *addNeon(void *last){
#if HAVE_NEON
    last = addFFT_Routine(last,2,1,1,NULL,NULL,FLAG_DIRECTION_BOTH,transformFFT2_NEON_2);

    last = addFFT_Routine(last,2,2,2,NULL,NULL,FLAG_DIRECTION_FORWARDS,transformFFT2_NEON_4);
    last = addFFT_Routine(last,2,2,2,NULL,NULL,FLAG_DIRECTION_BACKWARDS,transformFFT2_NEON_4_r);

    last = addFFT_Routine(last,2,3,3,NULL,createWeights_NEON_8,FLAG_DIRECTION_FORWARDS,transformFFT2_NEON_8);
    last = addFFT_Routine(last,2,3,3,NULL,createWeights_NEON_8_r,FLAG_DIRECTION_BACKWARDS,transformFFT2_NEON_8_r);

    last = addFFT_Routine(last,2,4,4,NULL,createWeights_NEON_16,FLAG_DIRECTION_FORWARDS,transformFFT2_NEON_16);
    last = addFFT_Routine(last,2,4,4,NULL,createWeights_NEON_16_r,FLAG_DIRECTION_BACKWARDS,transformFFT2_NEON_16_r);

    last = addFFT_Routine(last,2,5,5,NULL,createWeights_NEON_32,FLAG_DIRECTION_FORWARDS,transformFFT2_NEON_32);
    last = addFFT_Routine(last,2,5,5,NULL,createWeights_NEON_32_r,FLAG_DIRECTION_BACKWARDS,transformFFT2_NEON_32_r);

    last = addFFT_Routine(last,2,5,9,createReverse8,createWeights_NEON_BUFFER,FLAG_DIRECTION_FORWARDS,transformFFT2_NEON_BUFFER_2);
    last = addFFT_Routine(last,2,5,9,createReverse8,createWeights_NEON_BUFFER_r,FLAG_DIRECTION_BACKWARDS,transformFFT2_NEON_BUFFER_2_r);

    last = addFFT_Routine(last,2,10,21,NULL,createWeights_REC_NEON,FLAG_DIRECTION_FORWARDS,transformFFT2_REC_NEON);
    last = addFFT_Routine(last,2,10,21,NULL,createWeights_REC_NEON_r,FLAG_DIRECTION_BACKWARDS,transformFFT2_REC_NEON_r);

    last = addFFT_Routine(last,2,22,0,createReverse8,createWeightsDefault,FLAG_DIRECTION_FORWARDS,transformFFT2_NEON_BUFFER);

    last = addFFT_Routine(last,4,2,10,createReverse,createWeights4_NEON,FLAG_DIRECTION_FORWARDS,transformFFT4_NEON);
    //last = addFFT_Routine(last,4,1,10,createReverse,createWeights4_NEON_r,FLAG_DIRECTION_BACKWARDS,transformFFT4_NEON_r);
#endif
#ifdef __aarch64__
    last = addFFT_Routine(last,4,2,2,NULL,NULL,FLAG_DIRECTION_FORWARDS,transformFFT4_16_ASIMD);
    last = addFFT_Routine(last,2,5,5,NULL,NULL,FLAG_DIRECTION_FORWARDS,transformFFT_32_ASIMD);
    last = addFFT_Routine(last,8,2,2,NULL,createWeightsFFT_64_ASIMD,FLAG_DIRECTION_FORWARDS,transformFFT_64_ASIMD);
    last = addFFT_Routine(last,2,7,7,NULL,createWeightsFFT_128_ASIMD,FLAG_DIRECTION_FORWARDS,transformFFT_128_ASIMD);
    last = addFFT_Routine(last,16,2,2,NULL,createWeightsFFT16_256_ASIMD,FLAG_DIRECTION_FORWARDS,transformFFT16_256_ASIMD);
    last = addFFT_Routine(last,2,9,9,NULL,createWeightsFFT_512_ASIMD,FLAG_DIRECTION_FORWARDS,transformFFT_512_ASIMD);
    last = addFFT_Routine(last,32,2,2,NULL,createWeightsFFT_1024_ASIMD,FLAG_DIRECTION_FORWARDS,transformFFT_1024_ASIMD);
    last = addFFT_Routine(last,4,3,10,createReverse,createWeightsDefaultFull,FLAG_DIRECTION_FORWARDS,transformFFT16);
#endif
    return last;
}

FFT* createFFT(int n,int forward){
    FFT *fft = (FFT*)malloc(sizeof(FFT));
    fft->direction = forward ? 1 : -1;
    fft->n = n;
    fft->forward = forward;
    fft->in = NULL;
    fft->out = NULL;
    fft->reverse = NULL;
    fft->t = NULL;
    fft->w = memalign(64,n * sizeof(complex));
    fft->b = memalign(64,n * sizeof(complex));
    fft->freeWeights = NULL;

    if(!firstFFT_Routine){
        void *last = NULL;

        uint64_t features = android_getCpuFeatures();
        switch (android_getCpuFamily()){
            case ANDROID_CPU_FAMILY_ARM64:{
                if(features & ANDROID_CPU_ARM64_FEATURE_ASIMD){
                    last = addNeon(last);
                }
                break;
            }
            case ANDROID_CPU_FAMILY_ARM:{
                if (features & ANDROID_CPU_ARM_FEATURE_NEON) {
                    last = addNeon(last);
                }
                break;
            }
            default: {
                break;
            }
        }
        last = addFFT_Routine(last,2,1,1,NULL,NULL,FLAG_DIRECTION_BOTH,transformFFT2_2);

        last = addFFT_Routine(last,2,2,2,NULL,NULL,FLAG_DIRECTION_FORWARDS,transformFFT2_4);
        last = addFFT_Routine(last,2,2,2,NULL,NULL,FLAG_DIRECTION_BACKWARDS,transformFFT2_4_r);

        last = addFFT_Routine(last,4,1,6,createReverse,createWeightsDefault,FLAG_DIRECTION_BOTH,transformFFT4);

        last = addFFT_Routine(last,2,3,12,createReverse,createWeightsDefault,FLAG_DIRECTION_BOTH,transformFFT2);

        last = addFFT_Routine(last,2,13,21,createReverse,createWeightsRec,FLAG_DIRECTION_FORWARDS,transformFFT2_REC);
        last = addFFT_Routine(last,2,13,21,createReverse,createWeightsRec_r,FLAG_DIRECTION_BACKWARDS,transformFFT2_REC_r);
    }

    FFT_Routine *r = firstFFT_Routine;
    int directionFlag = forward ? FLAG_DIRECTION_FORWARDS : FLAG_DIRECTION_BACKWARDS;
    while (r){
        if(
                (r->flags & directionFlag) &&
                ((!r->minN) || (n >= r->minN)) &&
                ((!r->maxN) || (n <= r->maxN)) &&
                ((fft->m = logi(n,r->radix)) >= 0)
        ){
            fft->radix = r->radix;
            fft->transformFft = r->transform;
            if(r->preComputeReverse)r->preComputeReverse(fft);
            if(r->preComputeWeights)r->preComputeWeights(fft);
            break;
        }
        r = r->next;
    }

    return fft;
}

void freeFFT(FFT *fft){
    if(fft->reverse){
        free(fft->reverse);
    }
    if(fft->freeWeights){
        fft->freeWeights(fft);
    }
    free(fft->w);
    free(fft->b);
    free(fft);

}

int logi(int s,int p){
    if(p == 1){
        return s;
    }
    if(p < 1 || s <= 1){
        return -1;
    }
    int r = 0;
    int i = s;
    int l;
    while (i > 1){
        l = i;
        i /= p;
        r++;
    }
    if(l % p){
        return -1;
    }
    return r;
}

int powi(int x,int y){
    if(y == 0)return 1;
    int r = 1;
    for(int i=0;i<y;i++){
        r *= x;
    }
    return r;
}