/*
 * Copyright 2021 Torben Thaysen
 */

#ifndef FFT_WEIGHTS_H
#define FFT_WEIGHTS_H

#define do_complex_multiply(in0, in1, out) \
"fmul " out##_0r ".4s, " in0##_0r ".4s, " in1##_0r ".4s \n\t" \
"fmul " out##_0i ".4s, " in0##_0i ".4s, " in1##_0r ".4s \n\t" \
"fmul " out##_1r ".4s, " in0##_1r ".4s, " in1##_1r ".4s \n\t" \
"fmul " out##_1i ".4s, " in0##_1i ".4s, " in1##_1r ".4s \n\t" \
"fmul " out##_2r ".4s, " in0##_2r ".4s, " in1##_2r ".4s \n\t" \
"fmul " out##_2i ".4s, " in0##_2i ".4s, " in1##_2r ".4s \n\t" \
"fmul " out##_3r ".4s, " in0##_3r ".4s, " in1##_3r ".4s \n\t" \
"fmul " out##_3i ".4s, " in0##_3i ".4s, " in1##_3r ".4s \n\t" \
 \
"fmls " out##_0r ".4s, " in0##_0i ".4s, " in1##_0i ".4s \n\t" \
"fmla " out##_0i ".4s, " in0##_0r ".4s, " in1##_0i ".4s \n\t" \
"fmls " out##_1r ".4s, " in0##_1i ".4s, " in1##_1i ".4s \n\t" \
"fmla " out##_1i ".4s, " in0##_1r ".4s, " in1##_1i ".4s \n\t" \
"fmls " out##_2r ".4s, " in0##_2i ".4s, " in1##_2i ".4s \n\t" \
"fmla " out##_2i ".4s, " in0##_2r ".4s, " in1##_2i ".4s \n\t" \
"fmls " out##_3r ".4s, " in0##_3i ".4s, " in1##_3i ".4s \n\t" \
"fmla " out##_3i ".4s, " in0##_3r ".4s, " in1##_3i ".4s \n\t" \

#define do_weights(in, out) \
"ld1 { " out##_2r ".4s, " out##_2i ".4s, " out##_3r ".4s, " out##_3i ".4s }, [%[weights]], #64 \n\t" \
"fmul " out##_0r ".4s, " in##_0r ".4s, " out##_2r ".4s \n\t" \
"fmul " out##_0i ".4s, " in##_0i ".4s, " out##_2r ".4s \n\t" \
"fmul " out##_1r ".4s, " in##_1r ".4s, " out##_3r ".4s \n\t" \
"fmul " out##_1i ".4s, " in##_1i ".4s, " out##_3r ".4s \n\t" \
"fmls " out##_0r ".4s, " in##_0i ".4s, " out##_2i ".4s \n\t" \
"fmla " out##_0i ".4s, " in##_0r ".4s, " out##_2i ".4s \n\t" \
"fmls " out##_1r ".4s, " in##_1i ".4s, " out##_3i ".4s \n\t" \
"fmla " out##_1i ".4s, " in##_1r ".4s, " out##_3i ".4s \n\t" \
 \
"ld1 { " in##_0r ".4s, " in##_0i ".4s, " in##_1r ".4s, " in##_1i ".4s }, [%[weights]], #64 \n\t" \
"fmul " out##_2r ".4s, " in##_2r ".4s, " in##_0r ".4s \n\t" \
"fmul " out##_2i ".4s, " in##_2i ".4s, " in##_0r ".4s \n\t" \
"fmul " out##_3r ".4s, " in##_3r ".4s, " in##_1r ".4s \n\t" \
"fmul " out##_3i ".4s, " in##_3i ".4s, " in##_1r ".4s \n\t" \
"fmls " out##_2r ".4s, " in##_2i ".4s, " in##_0i ".4s \n\t" \
"fmla " out##_2i ".4s, " in##_2r ".4s, " in##_0i ".4s \n\t" \
"fmls " out##_3r ".4s, " in##_3i ".4s, " in##_1i ".4s \n\t" \
"fmla " out##_3i ".4s, " in##_3r ".4s, " in##_1i ".4s \n\t" \

#endif //FFT_WEIGHTS_H
