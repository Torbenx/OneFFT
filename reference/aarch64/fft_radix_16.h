/*
 * Copyright 2021 Torben Thaysen
 */

#ifndef FFT_RADIX_16_H
#define FFT_RADIX_16_H

#include "../fft.h"

#define F_SQRT_1_2 0.707106781186547524400844362105f
#define F_COS_PI_8 0.923879532511286756128183189397f
#define F_SIN_PI_8 0.382683432365089771728459984030f
#define F_COS_PI_16 0.980785280403230449126182236134f
#define F_SIN_PI_16 0.195090322016128267848284868477f

extern float fft4_32_ASIMD_data_f[];
extern float fft_ASIMD_data_f[];

#define B0_0r  "v0"
#define B0_0i  "v1"
#define B0_1r  "v2"
#define B0_1i  "v3"
#define B0_2r  "v4"
#define B0_2i  "v5"
#define B0_3r  "v6"
#define B0_3i  "v7"

#define B1_0r  "v8"
#define B1_0i  "v9"
#define B1_1r "v10"
#define B1_1i "v11"
#define B1_2r "v12"
#define B1_2i "v13"
#define B1_3r "v14"
#define B1_3i "v15"

#define B2_0r "v16"
#define B2_0i "v17"
#define B2_1r "v18"
#define B2_1i "v19"
#define B2_2r "v20"
#define B2_2i "v21"
#define B2_3r "v22"
#define B2_3i "v23"

#define B3_0r "v24"
#define B3_0i "v25"
#define B3_1r "v26"
#define B3_1i "v27"
#define B3_2r "v28"
#define B3_2i "v29"
#define B3_3r "v30"
#define B3_3i "v31"

#define I0_0r  "v0"
#define I0_0i  "v1"
#define I0_1r  "v4"
#define I0_1i  "v5"
#define I0_2r  "v8"
#define I0_2i  "v9"
#define I0_3r "v12"
#define I0_3i "v13"

#define I1_0r  "v2"
#define I1_0i  "v3"
#define I1_1r  "v6"
#define I1_1i  "v7"
#define I1_2r "v10"
#define I1_2i "v11"
#define I1_3r "v14"
#define I1_3i "v15"

#define REGISTERS "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", "v10", "v11", "v12", "v13", "v14", "v15", "v16", "v17", "v18", "v19", "v20", "v21", "v22", "v23", "v24", "v25", "v26", "v27", "v28", "v29", "v30", "v31", "cc"

#define do_fft4_x4(in, tmp, out) \
"fadd " tmp##_0r ".4s, " in##_0r ".4s, " in##_2r ".4s \n\t" \
"fadd " tmp##_0i ".4s, " in##_0i ".4s, " in##_2i ".4s \n\t" \
"fsub " tmp##_1r ".4s, " in##_0r ".4s, " in##_2r ".4s \n\t" \
"fsub " tmp##_1i ".4s, " in##_0i ".4s, " in##_2i ".4s \n\t" \
 \
"fadd " tmp##_2r ".4s, " in##_1r ".4s, " in##_3r ".4s \n\t" \
"fadd " tmp##_2i ".4s, " in##_1i ".4s, " in##_3i ".4s \n\t" \
"fsub " tmp##_3r ".4s, " in##_1r ".4s, " in##_3r ".4s \n\t" \
"fsub " tmp##_3i ".4s, " in##_1i ".4s, " in##_3i ".4s \n\t" \
 \
"fadd " out##_0r ".4s, " tmp##_0r ".4s, " tmp##_2r ".4s \n\t" \
"fadd " out##_0i ".4s, " tmp##_0i ".4s, " tmp##_2i ".4s \n\t" \
 \
"fadd " out##_1r ".4s, " tmp##_1r ".4s, " tmp##_3i ".4s \n\t" \
"fsub " out##_1i ".4s, " tmp##_1i ".4s, " tmp##_3r ".4s \n\t" \
 \
"fsub " out##_2r ".4s, " tmp##_0r ".4s, " tmp##_2r ".4s \n\t" \
"fsub " out##_2i ".4s, " tmp##_0i ".4s, " tmp##_2i ".4s \n\t" \
 \
"fsub " out##_3r ".4s, " tmp##_1r ".4s, " tmp##_3i ".4s \n\t" \
"fadd " out##_3i ".4s, " tmp##_1i ".4s, " tmp##_3r ".4s \n\t" \

#define do_fft8_x4(in0, in1, tmp, out0, out1, sqrt12) \
"fadd " tmp##_0r ".4s, " in0##_0r ".4s, " in1##_0r ".4s \n\t" \
"fadd " tmp##_0i ".4s, " in0##_0i ".4s, " in1##_0i ".4s \n\t" \
"fadd " tmp##_1r ".4s, " in0##_1r ".4s, " in1##_1r ".4s \n\t" \
"fadd " tmp##_1i ".4s, " in0##_1i ".4s, " in1##_1i ".4s \n\t" \
"fadd " tmp##_2r ".4s, " in0##_2r ".4s, " in1##_2r ".4s \n\t" \
"fadd " tmp##_2i ".4s, " in0##_2i ".4s, " in1##_2i ".4s \n\t" \
"fadd " tmp##_3r ".4s, " in0##_3r ".4s, " in1##_3r ".4s \n\t" \
"fadd " tmp##_3i ".4s, " in0##_3i ".4s, " in1##_3i ".4s \n\t" \
 \
"fsub " out0##_0r ".4s, " in0##_0r ".4s, " in1##_0r ".4s \n\t" \
"fsub " out0##_0i ".4s, " in0##_0i ".4s, " in1##_0i ".4s \n\t" \
"fsub " out0##_1r ".4s, " in0##_1r ".4s, " in1##_1r ".4s \n\t" \
"fsub " out0##_1i ".4s, " in0##_1i ".4s, " in1##_1i ".4s \n\t" \
"fsub " out0##_2r ".4s, " in0##_2r ".4s, " in1##_2r ".4s \n\t" \
"fsub " out0##_2i ".4s, " in0##_2i ".4s, " in1##_2i ".4s \n\t" \
"fsub " out0##_3r ".4s, " in0##_3r ".4s, " in1##_3r ".4s \n\t" \
"fsub " out0##_3i ".4s, " in0##_3i ".4s, " in1##_3i ".4s \n\t" \
 \
"fadd " out1##_0r ".4s, " tmp##_0r ".4s, " tmp##_2r ".4s \n\t" \
"fadd " out1##_0i ".4s, " tmp##_0i ".4s, " tmp##_2i ".4s \n\t" \
"fadd " out1##_1r ".4s, " tmp##_1r ".4s, " tmp##_3r ".4s \n\t" \
"fadd " out1##_1i ".4s, " tmp##_1i ".4s, " tmp##_3i ".4s \n\t" \
"fsub " out1##_2r ".4s, " tmp##_0r ".4s, " tmp##_2r ".4s \n\t" \
"fsub " out1##_2i ".4s, " tmp##_0i ".4s, " tmp##_2i ".4s \n\t" \
"fsub " out1##_3r ".4s, " tmp##_1r ".4s, " tmp##_3r ".4s \n\t" \
"fsub " out1##_3i ".4s, " tmp##_1i ".4s, " tmp##_3i ".4s \n\t" \
 \
"fsub " tmp##_2r ".4s, " out0##_0r ".4s, " out0##_2i ".4s \n\t" \
"fadd " tmp##_2i ".4s, " out0##_0i ".4s, " out0##_2r ".4s \n\t" \
"fsub " tmp##_3r ".4s, " out0##_1r ".4s, " out0##_3i ".4s \n\t" \
"fadd " tmp##_3i ".4s, " out0##_1i ".4s, " out0##_3r ".4s \n\t" \
"fadd " tmp##_0r ".4s, " out0##_0r ".4s, " out0##_2i ".4s \n\t" \
"fsub " tmp##_0i ".4s, " out0##_0i ".4s, " out0##_2r ".4s \n\t" \
"fadd " tmp##_1r ".4s, " out0##_1r ".4s, " out0##_3i ".4s \n\t" \
"fsub " tmp##_1i ".4s, " out0##_1i ".4s, " out0##_3r ".4s \n\t" \
 \
"fmul " tmp##_1r ".4s, " tmp##_1r ".4s, " sqrt12 " \n\t" \
"fmul " tmp##_1i ".4s, " tmp##_1i ".4s, " sqrt12 " \n\t" \
"fmul " tmp##_3r ".4s, " tmp##_3r ".4s, " sqrt12 " \n\t" \
"fmul " tmp##_3i ".4s, " tmp##_3i ".4s, " sqrt12 " \n\t" \
 \
"fadd " out0##_0r ".4s, " out1##_0r ".4s, " out1##_1r ".4s \n\t" \
"fadd " out0##_0i ".4s, " out1##_0i ".4s, " out1##_1i ".4s \n\t" \
"fadd " out0##_2r ".4s, " out1##_2r ".4s, " out1##_3i ".4s \n\t" \
"fsub " out0##_2i ".4s, " out1##_2i ".4s, " out1##_3r ".4s \n\t" \
 \
"fadd " out0##_1r ".4s, " tmp##_0r ".4s, " tmp##_1r ".4s \n\t" \
"fadd " out0##_1i ".4s, " tmp##_0i ".4s, " tmp##_1i ".4s \n\t" \
"fadd " out0##_3r ".4s, " tmp##_2r ".4s, " tmp##_3i ".4s \n\t" \
"fsub " out0##_3i ".4s, " tmp##_2i ".4s, " tmp##_3r ".4s \n\t" \
"fadd " out0##_1r ".4s, " out0##_1r ".4s, " tmp##_1i ".4s \n\t" \
"fsub " out0##_1i ".4s, " out0##_1i ".4s, " tmp##_1r ".4s \n\t" \
"fsub " out0##_3r ".4s, " out0##_3r ".4s, " tmp##_3r ".4s \n\t" \
"fsub " out0##_3i ".4s, " out0##_3i ".4s, " tmp##_3i ".4s \n\t" \
 \
"fsub " out1##_0r ".4s, " out1##_0r ".4s, " out1##_1r ".4s \n\t" \
"fsub " out1##_0i ".4s, " out1##_0i ".4s, " out1##_1i ".4s \n\t" \
"fsub " out1##_2r ".4s, " out1##_2r ".4s, " out1##_3i ".4s \n\t" \
"fadd " out1##_2i ".4s, " out1##_2i ".4s, " out1##_3r ".4s \n\t" \
 \
"fsub " out1##_1r ".4s, " tmp##_0r ".4s, " tmp##_1r ".4s \n\t" \
"fsub " out1##_1i ".4s, " tmp##_0i ".4s, " tmp##_1i ".4s \n\t" \
"fsub " out1##_3r ".4s, " tmp##_2r ".4s, " tmp##_3i ".4s \n\t" \
"fadd " out1##_3i ".4s, " tmp##_2i ".4s, " tmp##_3r ".4s \n\t" \
"fsub " out1##_1r ".4s, " out1##_1r ".4s, " tmp##_1i ".4s \n\t" \
"fadd " out1##_1i ".4s, " out1##_1i ".4s, " tmp##_1r ".4s \n\t" \
"fadd " out1##_3r ".4s, " out1##_3r ".4s, " tmp##_3r ".4s \n\t" \
"fadd " out1##_3i ".4s, " out1##_3i ".4s, " tmp##_3i ".4s \n\t" \

/*                       in0                      |                      in1
 * x0 y0 x4 y4|x8 y8 xc yc|x1 y1 x5 y5|x9 y9 xd yd|x2 y2 x6 y6|xa ya xe ye|x3 y3 x7 y7|xb yb xf yf
 *
 *                       tmp                      |                      in0
 * x0 y0 x1 y1|x8 y8 x9 y9|x2 y2 x3 y3|xa ya xb yb|x4 y4 x5 y5|xc yc xd yd|x6 y6 x7 y7|xe ye xf yf
 *
 *                       tmp                      |                      in1
 * x0 y0 x1 y1|x8 y8 x9 y9|x2 y2 x3 y3|xa ya xb yb|x4 y4 x5 y5|xc yc xd yd|x6 y6 x7 y7|xe ye xf yf
 *
 *
 * 0 4 8 c|1 5 9 d|2 6 a e|3 7 b f
 *
 * 0 8 1 9|4 c 5 9|2 a 3 b|6 e 7 f
 *
 * 0 1 2 3|4 5 6 7|8 9 a b|c d e f
 */

#define do_fft16_x2(in0, in1, tmp, out0, out1) \
do_fft8_x4(in0, in1, tmp, out0, out1, "v30.s[0]") \
 \
"zip1 " tmp##_0r ".2d, " out0##_0r ".2d, " out0##_2r ".2d \n\t" \
"zip1 " tmp##_0i ".2d, " out0##_0i ".2d, " out0##_2i ".2d \n\t" \
"zip1 " tmp##_2r ".2d, " out1##_0r ".2d, " out1##_2r ".2d \n\t" \
"zip1 " tmp##_2i ".2d, " out1##_0i ".2d, " out1##_2i ".2d \n\t" \
"zip1 " tmp##_1r ".2d, " out0##_1r ".2d, " out0##_3r ".2d \n\t" \
"zip1 " tmp##_1i ".2d, " out0##_1i ".2d, " out0##_3i ".2d \n\t" \
"zip1 " tmp##_3r ".2d, " out1##_1r ".2d, " out1##_3r ".2d \n\t" \
"zip1 " tmp##_3i ".2d, " out1##_1i ".2d, " out1##_3i ".2d \n\t" \
 \
"zip2 " out0##_0r ".2d, " out0##_0r ".2d, " out0##_2r ".2d \n\t" \
"zip2 " out0##_0i ".2d, " out0##_0i ".2d, " out0##_2i ".2d \n\t" \
"zip2 " out0##_2r ".2d, " out1##_0r ".2d, " out1##_2r ".2d \n\t" \
"zip2 " out0##_2i ".2d, " out1##_0i ".2d, " out1##_2i ".2d \n\t" \
"zip2 " out0##_1r ".2d, " out0##_1r ".2d, " out0##_3r ".2d \n\t" \
"zip2 " out0##_1i ".2d, " out0##_1i ".2d, " out0##_3i ".2d \n\t" \
"zip2 " out0##_3r ".2d, " out1##_1r ".2d, " out1##_3r ".2d \n\t" \
"zip2 " out0##_3i ".2d, " out1##_1i ".2d, " out1##_3i ".2d \n\t" \
 \
"fmul " out1##_0r ".4s, " out0##_0r ".4s, v24.4s \n\t" \
"fmul " out1##_0i ".4s, " out0##_0i ".4s, v24.4s \n\t" \
"fmls " out1##_0r ".4s, " out0##_0i ".4s, v25.4s \n\t" \
"fmla " out1##_0i ".4s, " out0##_0r ".4s, v25.4s \n\t" \
 \
"fmul " out1##_1r ".4s, " out0##_1r ".4s, v26.4s \n\t" \
"fmul " out1##_1i ".4s, " out0##_1i ".4s, v26.4s \n\t" \
"fmls " out1##_1r ".4s, " out0##_1i ".4s, v27.4s \n\t" \
"fmla " out1##_1i ".4s, " out0##_1r ".4s, v27.4s \n\t" \
 \
"fmul " out1##_2r ".4s, " out0##_2r ".4s, v25.4s \n\t" \
"fmul " out1##_2i ".4s, " out0##_2i ".4s, v25.4s \n\t" \
"fmla " out1##_2r ".4s, " out0##_2i ".4s, v24.4s \n\t" \
"fmls " out1##_2i ".4s, " out0##_2r ".4s, v24.4s \n\t" \
 \
"fmul " out1##_3r ".4s, " out0##_3r ".4s, v27.4s \n\t" \
"fmul " out1##_3i ".4s, " out0##_3i ".4s, v27.4s \n\t" \
"fmla " out1##_3r ".4s, " out0##_3i ".4s, v26.4s \n\t" \
"fmls " out1##_3i ".4s, " out0##_3r ".4s, v26.4s \n\t" \
 \
"fadd " out0##_0r ".4s, " tmp##_0r ".4s, " out1##_0r ".4s \n\t" \
"fadd " out0##_0i ".4s, " tmp##_0i ".4s, " out1##_0i ".4s \n\t" \
"fadd " out0##_1r ".4s, " tmp##_1r ".4s, " out1##_1r ".4s \n\t" \
"fadd " out0##_1i ".4s, " tmp##_1i ".4s, " out1##_1i ".4s \n\t" \
"fadd " out0##_2r ".4s, " tmp##_2r ".4s, " out1##_2r ".4s \n\t" \
"fadd " out0##_2i ".4s, " tmp##_2i ".4s, " out1##_2i ".4s \n\t" \
"fadd " out0##_3r ".4s, " tmp##_3r ".4s, " out1##_3r ".4s \n\t" \
"fadd " out0##_3i ".4s, " tmp##_3i ".4s, " out1##_3i ".4s \n\t" \
 \
"fsub " out1##_0r ".4s, " tmp##_0r ".4s, " out1##_0r ".4s \n\t" \
"fsub " out1##_0i ".4s, " tmp##_0i ".4s, " out1##_0i ".4s \n\t" \
"fsub " out1##_1r ".4s, " tmp##_1r ".4s, " out1##_1r ".4s \n\t" \
"fsub " out1##_1i ".4s, " tmp##_1i ".4s, " out1##_1i ".4s \n\t" \
"fsub " out1##_2r ".4s, " tmp##_2r ".4s, " out1##_2r ".4s \n\t" \
"fsub " out1##_2i ".4s, " tmp##_2i ".4s, " out1##_2i ".4s \n\t" \
"fsub " out1##_3r ".4s, " tmp##_3r ".4s, " out1##_3r ".4s \n\t" \
"fsub " out1##_3i ".4s, " tmp##_3i ".4s, " out1##_3i ".4s \n\t" \

#define do_fft32_bare(in0, in1, tmp, out0, out1) \
do_fft16_x2(in0, in1, tmp, out0, out1) \
 \
"trn1 " tmp##_0r ".4s, " out0##_0r ".4s, " out0##_1r ".4s \n\t" \
"trn1 " tmp##_0i ".4s, " out0##_0i ".4s, " out0##_1i ".4s \n\t" \
"trn1 " tmp##_1r ".4s, " out0##_2r ".4s, " out0##_3r ".4s \n\t" \
"trn1 " tmp##_1i ".4s, " out0##_2i ".4s, " out0##_3i ".4s \n\t" \
"trn1 " tmp##_2r ".4s, " out1##_0r ".4s, " out1##_1r ".4s \n\t" \
"trn1 " tmp##_2i ".4s, " out1##_0i ".4s, " out1##_1i ".4s \n\t" \
"trn1 " tmp##_3r ".4s, " out1##_2r ".4s, " out1##_3r ".4s \n\t" \
"trn1 " tmp##_3i ".4s, " out1##_2i ".4s, " out1##_3i ".4s \n\t" \
 \
"trn2 " out0##_0r ".4s, " out0##_0r ".4s, " out0##_1r ".4s \n\t" \
"trn2 " out0##_0i ".4s, " out0##_0i ".4s, " out0##_1i ".4s \n\t" \
"trn2 " out0##_1r ".4s, " out0##_2r ".4s, " out0##_3r ".4s \n\t" \
"trn2 " out0##_1i ".4s, " out0##_2i ".4s, " out0##_3i ".4s \n\t" \
"trn2 " out0##_2r ".4s, " out1##_0r ".4s, " out1##_1r ".4s \n\t" \
"trn2 " out0##_2i ".4s, " out1##_0i ".4s, " out1##_1i ".4s \n\t" \
"trn2 " out0##_3r ".4s, " out1##_2r ".4s, " out1##_3r ".4s \n\t" \
"trn2 " out0##_3i ".4s, " out1##_2i ".4s, " out1##_3i ".4s \n\t" \
 \
"fmul " out1##_0r ".4s, " out0##_0r ".4s, v28.4s \n\t" \
"fmul " out1##_0i ".4s, " out0##_0i ".4s, v28.4s \n\t" \
"fmul " out1##_1r ".4s, " out0##_1r ".4s, v30.4s \n\t" \
"fmul " out1##_1i ".4s, " out0##_1i ".4s, v30.4s \n\t" \
"fmul " out1##_2r ".4s, " out0##_2r ".4s, v29.4s \n\t" \
"fmul " out1##_2i ".4s, " out0##_2i ".4s, v29.4s \n\t" \
"fmul " out1##_3r ".4s, " out0##_3r ".4s, v31.4s \n\t" \
"fmul " out1##_3i ".4s, " out0##_3i ".4s, v31.4s \n\t" \
 \
"fmls " out1##_0r ".4s, " out0##_0i ".4s, v29.4s \n\t" \
"fmla " out1##_0i ".4s, " out0##_0r ".4s, v29.4s \n\t" \
"fmls " out1##_1r ".4s, " out0##_1i ".4s, v31.4s \n\t" \
"fmla " out1##_1i ".4s, " out0##_1r ".4s, v31.4s \n\t" \
"fmla " out1##_2r ".4s, " out0##_2i ".4s, v28.4s \n\t" \
"fmls " out1##_2i ".4s, " out0##_2r ".4s, v28.4s \n\t" \
"fmla " out1##_3r ".4s, " out0##_3i ".4s, v30.4s \n\t" \
"fmls " out1##_3i ".4s, " out0##_3r ".4s, v30.4s \n\t" \

#define do_fft32(in0, in1, tmp, out0, out1) \
do_fft32_bare(in0, in1, tmp, out0, out1) \
 \
"fadd " out0##_0r ".4s, " tmp##_0r ".4s, " out1##_0r ".4s \n\t" \
"fadd " out0##_0i ".4s, " tmp##_0i ".4s, " out1##_0i ".4s \n\t" \
"fadd " out0##_1r ".4s, " tmp##_1r ".4s, " out1##_1r ".4s \n\t" \
"fadd " out0##_1i ".4s, " tmp##_1i ".4s, " out1##_1i ".4s \n\t" \
"fadd " out0##_2r ".4s, " tmp##_2r ".4s, " out1##_2r ".4s \n\t" \
"fadd " out0##_2i ".4s, " tmp##_2i ".4s, " out1##_2i ".4s \n\t" \
"fadd " out0##_3r ".4s, " tmp##_3r ".4s, " out1##_3r ".4s \n\t" \
"fadd " out0##_3i ".4s, " tmp##_3i ".4s, " out1##_3i ".4s \n\t" \
 \
"fsub " out1##_0r ".4s, " tmp##_0r ".4s, " out1##_0r ".4s \n\t" \
"fsub " out1##_0i ".4s, " tmp##_0i ".4s, " out1##_0i ".4s \n\t" \
"fsub " out1##_1r ".4s, " tmp##_1r ".4s, " out1##_1r ".4s \n\t" \
"fsub " out1##_1i ".4s, " tmp##_1i ".4s, " out1##_1i ".4s \n\t" \
"fsub " out1##_2r ".4s, " tmp##_2r ".4s, " out1##_2r ".4s \n\t" \
"fsub " out1##_2i ".4s, " tmp##_2i ".4s, " out1##_2i ".4s \n\t" \
"fsub " out1##_3r ".4s, " tmp##_3r ".4s, " out1##_3r ".4s \n\t" \
"fsub " out1##_3i ".4s, " tmp##_3i ".4s, " out1##_3i ".4s \n\t" \

#endif //FFT_RADIX_16_H
