/*
 * Copyright 2021 Torben Thaysen
 */

#ifndef FFT_LOAD_H
#define FFT_LOAD_H

#define load_by_reg_single_(bank, shift0, shift1) \
"ld2 {" bank##_0r ".s," bank##_0i ".s}[0], [%[input]], " shift0 " \n\t" \
"ld2 {" bank##_0r ".s," bank##_0i ".s}[1], [%[input]], " shift0 " \n\t" \
"ld2 {" bank##_0r ".s," bank##_0i ".s}[2], [%[input]], " shift0 " \n\t" \
"ld2 {" bank##_0r ".s," bank##_0i ".s}[3], [%[input]], " shift1 " \n\t" \
 \
"ld2 {" bank##_1r ".s," bank##_1i ".s}[0], [%[input]], " shift0 " \n\t" \
"ld2 {" bank##_1r ".s," bank##_1i ".s}[1], [%[input]], " shift0 " \n\t" \
"ld2 {" bank##_1r ".s," bank##_1i ".s}[2], [%[input]], " shift0 " \n\t" \
"ld2 {" bank##_1r ".s," bank##_1i ".s}[3], [%[input]], " shift1 " \n\t" \
 \
"ld2 {" bank##_2r ".s," bank##_2i ".s}[0], [%[input]], " shift0 " \n\t" \
"ld2 {" bank##_2r ".s," bank##_2i ".s}[1], [%[input]], " shift0 " \n\t" \
"ld2 {" bank##_2r ".s," bank##_2i ".s}[2], [%[input]], " shift0 " \n\t" \
"ld2 {" bank##_2r ".s," bank##_2i ".s}[3], [%[input]], " shift1 " \n\t" \
 \
"ld2 {" bank##_3r ".s," bank##_3i ".s}[0], [%[input]], " shift0 " \n\t" \
"ld2 {" bank##_3r ".s," bank##_3i ".s}[1], [%[input]], " shift0 " \n\t" \
"ld2 {" bank##_3r ".s," bank##_3i ".s}[2], [%[input]], " shift0 " \n\t" \
"ld2 {" bank##_3r ".s," bank##_3i ".s}[3], [%[input]], " shift1 " \n\t" \

#define load_by_reg_single(bank, shift0) load_by_reg_single_(bank, shift0, shift0)

#define store_by_reg_single_(bank, shift0, shift1) \
"st2 {" bank##_0r ".s," bank##_0i ".s}[0], [%[output]], " shift0 " \n\t" \
"st2 {" bank##_0r ".s," bank##_0i ".s}[1], [%[output]], " shift0 " \n\t" \
"st2 {" bank##_0r ".s," bank##_0i ".s}[2], [%[output]], " shift0 " \n\t" \
"st2 {" bank##_0r ".s," bank##_0i ".s}[3], [%[output]], " shift1 " \n\t" \
 \
"st2 {" bank##_1r ".s," bank##_1i ".s}[0], [%[output]], " shift0 " \n\t" \
"st2 {" bank##_1r ".s," bank##_1i ".s}[1], [%[output]], " shift0 " \n\t" \
"st2 {" bank##_1r ".s," bank##_1i ".s}[2], [%[output]], " shift0 " \n\t" \
"st2 {" bank##_1r ".s," bank##_1i ".s}[3], [%[output]], " shift1 " \n\t" \
 \
"st2 {" bank##_2r ".s," bank##_2i ".s}[0], [%[output]], " shift0 " \n\t" \
"st2 {" bank##_2r ".s," bank##_2i ".s}[1], [%[output]], " shift0 " \n\t" \
"st2 {" bank##_2r ".s," bank##_2i ".s}[2], [%[output]], " shift0 " \n\t" \
"st2 {" bank##_2r ".s," bank##_2i ".s}[3], [%[output]], " shift1 " \n\t" \
 \
"st2 {" bank##_3r ".s," bank##_3i ".s}[0], [%[output]], " shift0 " \n\t" \
"st2 {" bank##_3r ".s," bank##_3i ".s}[1], [%[output]], " shift0 " \n\t" \
"st2 {" bank##_3r ".s," bank##_3i ".s}[2], [%[output]], " shift0 " \n\t" \
"st2 {" bank##_3r ".s," bank##_3i ".s}[3], [%[output]], " shift1 " \n\t" \

#define store_by_reg_single(bank, shift0) store_by_reg_single_(bank, shift0, shift0)

#define load_by_elm_single_(bank, shift0, shift1) \
"ld2 {" bank##_0r ".s," bank##_0i ".s}[0], [%[input]], " shift0 " \n\t" \
"ld2 {" bank##_1r ".s," bank##_1i ".s}[0], [%[input]], " shift0 " \n\t" \
"ld2 {" bank##_2r ".s," bank##_2i ".s}[0], [%[input]], " shift0 " \n\t" \
"ld2 {" bank##_3r ".s," bank##_3i ".s}[0], [%[input]], " shift1 " \n\t" \
 \
"ld2 {" bank##_0r ".s," bank##_0i ".s}[1], [%[input]], " shift0 " \n\t" \
"ld2 {" bank##_1r ".s," bank##_1i ".s}[1], [%[input]], " shift0 " \n\t" \
"ld2 {" bank##_2r ".s," bank##_2i ".s}[1], [%[input]], " shift0 " \n\t" \
"ld2 {" bank##_3r ".s," bank##_3i ".s}[1], [%[input]], " shift1 " \n\t" \
 \
"ld2 {" bank##_0r ".s," bank##_0i ".s}[2], [%[input]], " shift0 " \n\t" \
"ld2 {" bank##_1r ".s," bank##_1i ".s}[2], [%[input]], " shift0 " \n\t" \
"ld2 {" bank##_2r ".s," bank##_2i ".s}[2], [%[input]], " shift0 " \n\t" \
"ld2 {" bank##_3r ".s," bank##_3i ".s}[2], [%[input]], " shift1 " \n\t" \
 \
"ld2 {" bank##_0r ".s," bank##_0i ".s}[3], [%[input]], " shift0 " \n\t" \
"ld2 {" bank##_1r ".s," bank##_1i ".s}[3], [%[input]], " shift0 " \n\t" \
"ld2 {" bank##_2r ".s," bank##_2i ".s}[3], [%[input]], " shift0 " \n\t" \
"ld2 {" bank##_3r ".s," bank##_3i ".s}[3], [%[input]], " shift1 " \n\t" \

#define load_by_elm_single(bank, shift0) load_by_elm_single_(bank, shift0, shift0)

#define store_by_elm_single_(bank, shift0, shift1) \
"st2 {" bank##_0r ".s," bank##_0i ".s}[0], [%[output]], " shift0 " \n\t" \
"st2 {" bank##_1r ".s," bank##_1i ".s}[0], [%[output]], " shift0 " \n\t" \
"st2 {" bank##_2r ".s," bank##_2i ".s}[0], [%[output]], " shift0 " \n\t" \
"st2 {" bank##_3r ".s," bank##_3i ".s}[0], [%[output]], " shift1 " \n\t" \
 \
"st2 {" bank##_0r ".s," bank##_0i ".s}[1], [%[output]], " shift0 " \n\t" \
"st2 {" bank##_1r ".s," bank##_1i ".s}[1], [%[output]], " shift0 " \n\t" \
"st2 {" bank##_2r ".s," bank##_2i ".s}[1], [%[output]], " shift0 " \n\t" \
"st2 {" bank##_3r ".s," bank##_3i ".s}[1], [%[output]], " shift1 " \n\t" \
 \
"st2 {" bank##_0r ".s," bank##_0i ".s}[2], [%[output]], " shift0 " \n\t" \
"st2 {" bank##_1r ".s," bank##_1i ".s}[2], [%[output]], " shift0 " \n\t" \
"st2 {" bank##_2r ".s," bank##_2i ".s}[2], [%[output]], " shift0 " \n\t" \
"st2 {" bank##_3r ".s," bank##_3i ".s}[2], [%[output]], " shift1 " \n\t" \
 \
"st2 {" bank##_0r ".s," bank##_0i ".s}[3], [%[output]], " shift0 " \n\t" \
"st2 {" bank##_1r ".s," bank##_1i ".s}[3], [%[output]], " shift0 " \n\t" \
"st2 {" bank##_2r ".s," bank##_2i ".s}[3], [%[output]], " shift0 " \n\t" \
"st2 {" bank##_3r ".s," bank##_3i ".s}[3], [%[output]], " shift1 " \n\t" \

#define store_by_elm_single(bank, shift0) store_by_elm_single_(bank, shift0, shift0)

#define store_load_by_reg_single_(s_bank, s_shift0, s_shift1, l_bank, l_shift0, l_shift1) \
"st2 {" s_bank##_0r ".s," s_bank##_0i ".s}[0], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_0r ".s," l_bank##_0i ".s}[0], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank##_0r ".s," s_bank##_0i ".s}[1], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_0r ".s," l_bank##_0i ".s}[1], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank##_0r ".s," s_bank##_0i ".s}[2], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_0r ".s," l_bank##_0i ".s}[2], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank##_0r ".s," s_bank##_0i ".s}[3], [%[output]], " s_shift1 " \n\t" \
"ld2 {" l_bank##_0r ".s," l_bank##_0i ".s}[3], [%[input]], " l_shift1 " \n\t" \
 \
"st2 {" s_bank##_1r ".s," s_bank##_1i ".s}[0], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_1r ".s," l_bank##_1i ".s}[0], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank##_1r ".s," s_bank##_1i ".s}[1], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_1r ".s," l_bank##_1i ".s}[1], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank##_1r ".s," s_bank##_1i ".s}[2], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_1r ".s," l_bank##_1i ".s}[2], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank##_1r ".s," s_bank##_1i ".s}[3], [%[output]], " s_shift1 " \n\t" \
"ld2 {" l_bank##_1r ".s," l_bank##_1i ".s}[3], [%[input]], " l_shift1 " \n\t" \
 \
"st2 {" s_bank##_2r ".s," s_bank##_2i ".s}[0], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_2r ".s," l_bank##_2i ".s}[0], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank##_2r ".s," s_bank##_2i ".s}[1], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_2r ".s," l_bank##_2i ".s}[1], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank##_2r ".s," s_bank##_2i ".s}[2], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_2r ".s," l_bank##_2i ".s}[2], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank##_2r ".s," s_bank##_2i ".s}[3], [%[output]], " s_shift1 " \n\t" \
"ld2 {" l_bank##_2r ".s," l_bank##_2i ".s}[3], [%[input]], " l_shift1 " \n\t" \
 \
"st2 {" s_bank##_3r ".s," s_bank##_3i ".s}[0], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_3r ".s," l_bank##_3i ".s}[0], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank##_3r ".s," s_bank##_3i ".s}[1], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_3r ".s," l_bank##_3i ".s}[1], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank##_3r ".s," s_bank##_3i ".s}[2], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_3r ".s," l_bank##_3i ".s}[2], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank##_3r ".s," s_bank##_3i ".s}[3], [%[output]], " s_shift1 " \n\t" \
"ld2 {" l_bank##_3r ".s," l_bank##_3i ".s}[3], [%[input]], " l_shift1 " \n\t" \

#define store_load_by_reg_single(s_bank, s_shift0, l_bank, l_shift0) store_load_by_reg_single_(s_bank, s_shift0, s_shift0, l_bank, l_shift0, l_shift0)

#define store_load_by_elm_single_(s_bank, s_shift0, s_shift1, l_bank, l_shift0, l_shift1) \
"st2 {" s_bank##_0r ".s," s_bank##_0i ".s}[0], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_0r ".s," l_bank##_0i ".s}[0], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank##_1r ".s," s_bank##_1i ".s}[0], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_1r ".s," l_bank##_1i ".s}[0], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank##_2r ".s," s_bank##_2i ".s}[0], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_2r ".s," l_bank##_2i ".s}[0], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank##_3r ".s," s_bank##_3i ".s}[0], [%[output]], " s_shift1 " \n\t" \
"ld2 {" l_bank##_3r ".s," l_bank##_3i ".s}[0], [%[input]], " l_shift1 " \n\t" \
 \
"st2 {" s_bank##_0r ".s," s_bank##_0i ".s}[1], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_0r ".s," l_bank##_0i ".s}[1], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank##_1r ".s," s_bank##_1i ".s}[1], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_1r ".s," l_bank##_1i ".s}[1], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank##_2r ".s," s_bank##_2i ".s}[1], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_2r ".s," l_bank##_2i ".s}[1], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank##_3r ".s," s_bank##_3i ".s}[1], [%[output]], " s_shift1 " \n\t" \
"ld2 {" l_bank##_3r ".s," l_bank##_3i ".s}[1], [%[input]], " l_shift1 " \n\t" \
 \
"st2 {" s_bank##_0r ".s," s_bank##_0i ".s}[2], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_0r ".s," l_bank##_0i ".s}[2], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank##_1r ".s," s_bank##_1i ".s}[2], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_1r ".s," l_bank##_1i ".s}[2], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank##_2r ".s," s_bank##_2i ".s}[2], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_2r ".s," l_bank##_2i ".s}[2], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank##_3r ".s," s_bank##_3i ".s}[2], [%[output]], " s_shift1 " \n\t" \
"ld2 {" l_bank##_3r ".s," l_bank##_3i ".s}[2], [%[input]], " l_shift1 " \n\t" \
 \
"st2 {" s_bank##_0r ".s," s_bank##_0i ".s}[3], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_0r ".s," l_bank##_0i ".s}[3], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank##_1r ".s," s_bank##_1i ".s}[3], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_1r ".s," l_bank##_1i ".s}[3], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank##_2r ".s," s_bank##_2i ".s}[3], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_2r ".s," l_bank##_2i ".s}[3], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank##_3r ".s," s_bank##_3i ".s}[3], [%[output]], " s_shift1 " \n\t" \
"ld2 {" l_bank##_3r ".s," l_bank##_3i ".s}[3], [%[input]], " l_shift1 " \n\t" \

#define store_load_by_elm_single(s_bank, s_shift0, l_bank, l_shift0) store_load_by_elm_single_(s_bank, s_shift0, s_shift0, l_bank, l_shift0, l_shift0)

#define store_by_elm_load_by_reg_single_(s_bank, s_shift0, s_shift1, l_bank, l_shift0, l_shift1) \
"st2 {" s_bank##_0r ".s," s_bank##_0i ".s}[0], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_0r ".s," l_bank##_0i ".s}[0], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank##_1r ".s," s_bank##_1i ".s}[0], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_0r ".s," l_bank##_0i ".s}[1], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank##_2r ".s," s_bank##_2i ".s}[0], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_0r ".s," l_bank##_0i ".s}[2], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank##_3r ".s," s_bank##_3i ".s}[0], [%[output]], " s_shift1 " \n\t" \
"ld2 {" l_bank##_0r ".s," l_bank##_0i ".s}[3], [%[input]], " l_shift1 " \n\t" \
 \
"st2 {" s_bank##_0r ".s," s_bank##_0i ".s}[1], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_1r ".s," l_bank##_1i ".s}[0], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank##_1r ".s," s_bank##_1i ".s}[1], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_1r ".s," l_bank##_1i ".s}[1], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank##_2r ".s," s_bank##_2i ".s}[1], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_1r ".s," l_bank##_1i ".s}[2], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank##_3r ".s," s_bank##_3i ".s}[1], [%[output]], " s_shift1 " \n\t" \
"ld2 {" l_bank##_1r ".s," l_bank##_1i ".s}[3], [%[input]], " l_shift1 " \n\t" \
 \
"st2 {" s_bank##_0r ".s," s_bank##_0i ".s}[2], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_2r ".s," l_bank##_2i ".s}[0], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank##_1r ".s," s_bank##_1i ".s}[2], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_2r ".s," l_bank##_2i ".s}[1], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank##_2r ".s," s_bank##_2i ".s}[2], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_2r ".s," l_bank##_2i ".s}[2], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank##_3r ".s," s_bank##_3i ".s}[2], [%[output]], " s_shift1 " \n\t" \
"ld2 {" l_bank##_2r ".s," l_bank##_2i ".s}[3], [%[input]], " l_shift1 " \n\t" \
 \
"st2 {" s_bank##_0r ".s," s_bank##_0i ".s}[3], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_3r ".s," l_bank##_3i ".s}[0], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank##_1r ".s," s_bank##_1i ".s}[3], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_3r ".s," l_bank##_3i ".s}[1], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank##_2r ".s," s_bank##_2i ".s}[3], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_3r ".s," l_bank##_3i ".s}[2], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank##_3r ".s," s_bank##_3i ".s}[3], [%[output]], " s_shift1 " \n\t" \
"ld2 {" l_bank##_3r ".s," l_bank##_3i ".s}[3], [%[input]], " l_shift1 " \n\t" \

#define store_by_elm_load_by_reg_single(s_bank, s_shift0, l_bank, l_shift0) store_by_elm_load_by_reg_single_(s_bank, s_shift0, s_shift0, l_bank, l_shift0, l_shift0)

#define store_by_reg_load_by_elm_single_(s_bank, s_shift0, s_shift1, l_bank, l_shift0, l_shift1) \
"st2 {" s_bank##_0r ".s," s_bank##_0i ".s}[0], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_0r ".s," l_bank##_0i ".s}[0], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank##_0r ".s," s_bank##_0i ".s}[1], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_1r ".s," l_bank##_1i ".s}[0], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank##_0r ".s," s_bank##_0i ".s}[2], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_2r ".s," l_bank##_2i ".s}[0], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank##_0r ".s," s_bank##_0i ".s}[3], [%[output]], " s_shift1 " \n\t" \
"ld2 {" l_bank##_3r ".s," l_bank##_3i ".s}[0], [%[input]], " l_shift1 " \n\t" \
 \
"st2 {" s_bank##_1r ".s," s_bank##_1i ".s}[0], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_0r ".s," l_bank##_0i ".s}[1], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank##_1r ".s," s_bank##_1i ".s}[1], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_1r ".s," l_bank##_1i ".s}[1], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank##_1r ".s," s_bank##_1i ".s}[2], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_2r ".s," l_bank##_2i ".s}[1], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank##_1r ".s," s_bank##_1i ".s}[3], [%[output]], " s_shift1 " \n\t" \
"ld2 {" l_bank##_3r ".s," l_bank##_3i ".s}[1], [%[input]], " l_shift1 " \n\t" \
 \
"st2 {" s_bank##_2r ".s," s_bank##_2i ".s}[0], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_0r ".s," l_bank##_0i ".s}[2], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank##_2r ".s," s_bank##_2i ".s}[1], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_1r ".s," l_bank##_1i ".s}[2], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank##_2r ".s," s_bank##_2i ".s}[2], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_2r ".s," l_bank##_2i ".s}[2], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank##_2r ".s," s_bank##_2i ".s}[3], [%[output]], " s_shift1 " \n\t" \
"ld2 {" l_bank##_3r ".s," l_bank##_3i ".s}[2], [%[input]], " l_shift1 " \n\t" \
 \
"st2 {" s_bank##_3r ".s," s_bank##_3i ".s}[0], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_0r ".s," l_bank##_0i ".s}[3], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank##_3r ".s," s_bank##_3i ".s}[1], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_1r ".s," l_bank##_1i ".s}[3], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank##_3r ".s," s_bank##_3i ".s}[2], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_2r ".s," l_bank##_2i ".s}[3], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank##_3r ".s," s_bank##_3i ".s}[3], [%[output]], " s_shift1 " \n\t" \
"ld2 {" l_bank##_3r ".s," l_bank##_3i ".s}[3], [%[input]], " l_shift1 " \n\t" \

#define store_by_reg_load_by_elm_single(s_bank, s_shift0, l_bank, l_shift0) store_by_reg_load_by_elm_single_(s_bank, s_shift0, s_shift0, l_bank, l_shift0, l_shift0)

#define store_by_elm_double_(bank0, bank1, shift0, shift1) \
"st2 { " bank0##_0r ".s, " bank0##_0i ".s }[0], [%[output]], " shift0 " \n\t" \
"st2 { " bank0##_1r ".s, " bank0##_1i ".s }[0], [%[output]], " shift0 " \n\t" \
"st2 { " bank0##_2r ".s, " bank0##_2i ".s }[0], [%[output]], " shift0 " \n\t" \
"st2 { " bank0##_3r ".s, " bank0##_3i ".s }[0], [%[output]], " shift0 " \n\t" \
"st2 { " bank1##_0r ".s, " bank1##_0i ".s }[0], [%[output]], " shift0 " \n\t" \
"st2 { " bank1##_1r ".s, " bank1##_1i ".s }[0], [%[output]], " shift0 " \n\t" \
"st2 { " bank1##_2r ".s, " bank1##_2i ".s }[0], [%[output]], " shift0 " \n\t" \
"st2 { " bank1##_3r ".s, " bank1##_3i ".s }[0], [%[output]], " shift1 " \n\t" \
 \
"st2 { " bank0##_0r ".s, " bank0##_0i ".s }[1], [%[output]], " shift0 " \n\t" \
"st2 { " bank0##_1r ".s, " bank0##_1i ".s }[1], [%[output]], " shift0 " \n\t" \
"st2 { " bank0##_2r ".s, " bank0##_2i ".s }[1], [%[output]], " shift0 " \n\t" \
"st2 { " bank0##_3r ".s, " bank0##_3i ".s }[1], [%[output]], " shift0 " \n\t" \
"st2 { " bank1##_0r ".s, " bank1##_0i ".s }[1], [%[output]], " shift0 " \n\t" \
"st2 { " bank1##_1r ".s, " bank1##_1i ".s }[1], [%[output]], " shift0 " \n\t" \
"st2 { " bank1##_2r ".s, " bank1##_2i ".s }[1], [%[output]], " shift0 " \n\t" \
"st2 { " bank1##_3r ".s, " bank1##_3i ".s }[1], [%[output]], " shift1 " \n\t" \
 \
"st2 { " bank0##_0r ".s, " bank0##_0i ".s }[2], [%[output]], " shift0 " \n\t" \
"st2 { " bank0##_1r ".s, " bank0##_1i ".s }[2], [%[output]], " shift0 " \n\t" \
"st2 { " bank0##_2r ".s, " bank0##_2i ".s }[2], [%[output]], " shift0 " \n\t" \
"st2 { " bank0##_3r ".s, " bank0##_3i ".s }[2], [%[output]], " shift0 " \n\t" \
"st2 { " bank1##_0r ".s, " bank1##_0i ".s }[2], [%[output]], " shift0 " \n\t" \
"st2 { " bank1##_1r ".s, " bank1##_1i ".s }[2], [%[output]], " shift0 " \n\t" \
"st2 { " bank1##_2r ".s, " bank1##_2i ".s }[2], [%[output]], " shift0 " \n\t" \
"st2 { " bank1##_3r ".s, " bank1##_3i ".s }[2], [%[output]], " shift1 " \n\t" \
 \
"st2 { " bank0##_0r ".s, " bank0##_0i ".s }[3], [%[output]], " shift0 " \n\t" \
"st2 { " bank0##_1r ".s, " bank0##_1i ".s }[3], [%[output]], " shift0 " \n\t" \
"st2 { " bank0##_2r ".s, " bank0##_2i ".s }[3], [%[output]], " shift0 " \n\t" \
"st2 { " bank0##_3r ".s, " bank0##_3i ".s }[3], [%[output]], " shift0 " \n\t" \
"st2 { " bank1##_0r ".s, " bank1##_0i ".s }[3], [%[output]], " shift0 " \n\t" \
"st2 { " bank1##_1r ".s, " bank1##_1i ".s }[3], [%[output]], " shift0 " \n\t" \
"st2 { " bank1##_2r ".s, " bank1##_2i ".s }[3], [%[output]], " shift0 " \n\t" \
"st2 { " bank1##_3r ".s, " bank1##_3i ".s }[3], [%[output]], " shift1 " \n\t" \

#define load_by_reg_double_(bank0, bank1, shift0, shift1) \
load_by_reg_single_(bank0, shift0, shift1) \
load_by_reg_single_(bank1, shift0, shift1) \

#define store_by_reg_double_(bank0, bank1, shift0, shift1) \
store_by_reg_single_(bank0, shift0, shift1) \
store_by_reg_single_(bank1, shift0, shift1) \

#define store_load_by_reg_double_(s_bank0, s_bank1, s_shift0, s_shift1, l_bank0, l_bank1, l_shift0, l_shift1) \
store_load_by_reg_single_(s_bank0, s_shift0, s_shift1, l_bank0, l_shift0, l_shift1) \
store_load_by_reg_single_(s_bank1, s_shift0, s_shift1, l_bank1, l_shift0, l_shift1) \

#define store_by_elm_load_by_reg_double_(s_bank0, s_bank1, s_shift0, s_shift1, l_bank0, l_bank1, l_shift0, l_shift1) \
"st2 { " s_bank0##_0r ".s, " s_bank0##_0i ".s }[0], [%[output]], " s_shift0 " \n\t" \
"ld2 { " l_bank0##_0r ".s, " l_bank0##_0i ".s }[0], [%[input]], " l_shift0 " \n\t" \
"st2 { " s_bank0##_1r ".s, " s_bank0##_1i ".s }[0], [%[output]], " s_shift0 " \n\t" \
"ld2 { " l_bank0##_0r ".s, " l_bank0##_0i ".s }[1], [%[input]], " l_shift0 " \n\t" \
"st2 { " s_bank0##_2r ".s, " s_bank0##_2i ".s }[0], [%[output]], " s_shift0 " \n\t" \
"ld2 { " l_bank0##_0r ".s, " l_bank0##_0i ".s }[2], [%[input]], " l_shift0 " \n\t" \
"st2 { " s_bank0##_3r ".s, " s_bank0##_3i ".s }[0], [%[output]], " s_shift0 " \n\t" \
"ld2 { " l_bank0##_0r ".s, " l_bank0##_0i ".s }[3], [%[input]], " l_shift1 " \n\t" \
"st2 { " s_bank1##_0r ".s, " s_bank1##_0i ".s }[0], [%[output]], " s_shift0 " \n\t" \
"ld2 { " l_bank0##_1r ".s, " l_bank0##_1i ".s }[0], [%[input]], " l_shift0 " \n\t" \
"st2 { " s_bank1##_1r ".s, " s_bank1##_1i ".s }[0], [%[output]], " s_shift0 " \n\t" \
"ld2 { " l_bank0##_1r ".s, " l_bank0##_1i ".s }[1], [%[input]], " l_shift0 " \n\t" \
"st2 { " s_bank1##_2r ".s, " s_bank1##_2i ".s }[0], [%[output]], " s_shift0 " \n\t" \
"ld2 { " l_bank0##_1r ".s, " l_bank0##_1i ".s }[2], [%[input]], " l_shift0 " \n\t" \
"st2 { " s_bank1##_3r ".s, " s_bank1##_3i ".s }[0], [%[output]], " s_shift1 " \n\t" \
"ld2 { " l_bank0##_1r ".s, " l_bank0##_1i ".s }[3], [%[input]], " l_shift1 " \n\t" \
 \
"st2 { " s_bank0##_0r ".s, " s_bank0##_0i ".s }[1], [%[output]], " s_shift0 " \n\t" \
"ld2 { " l_bank0##_2r ".s, " l_bank0##_2i ".s }[0], [%[input]], " l_shift0 " \n\t" \
"st2 { " s_bank0##_1r ".s, " s_bank0##_1i ".s }[1], [%[output]], " s_shift0 " \n\t" \
"ld2 { " l_bank0##_2r ".s, " l_bank0##_2i ".s }[1], [%[input]], " l_shift0 " \n\t" \
"st2 { " s_bank0##_2r ".s, " s_bank0##_2i ".s }[1], [%[output]], " s_shift0 " \n\t" \
"ld2 { " l_bank0##_2r ".s, " l_bank0##_2i ".s }[2], [%[input]], " l_shift0 " \n\t" \
"st2 { " s_bank0##_3r ".s, " s_bank0##_3i ".s }[1], [%[output]], " s_shift0 " \n\t" \
"ld2 { " l_bank0##_2r ".s, " l_bank0##_2i ".s }[3], [%[input]], " l_shift1 " \n\t" \
"st2 { " s_bank1##_0r ".s, " s_bank1##_0i ".s }[1], [%[output]], " s_shift0 " \n\t" \
"ld2 { " l_bank0##_3r ".s, " l_bank0##_3i ".s }[0], [%[input]], " l_shift0 " \n\t" \
"st2 { " s_bank1##_1r ".s, " s_bank1##_1i ".s }[1], [%[output]], " s_shift0 " \n\t" \
"ld2 { " l_bank0##_3r ".s, " l_bank0##_3i ".s }[1], [%[input]], " l_shift0 " \n\t" \
"st2 { " s_bank1##_2r ".s, " s_bank1##_2i ".s }[1], [%[output]], " s_shift0 " \n\t" \
"ld2 { " l_bank0##_3r ".s, " l_bank0##_3i ".s }[2], [%[input]], " l_shift0 " \n\t" \
"st2 { " s_bank1##_3r ".s, " s_bank1##_3i ".s }[1], [%[output]], " s_shift1 " \n\t" \
"ld2 { " l_bank0##_3r ".s, " l_bank0##_3i ".s }[3], [%[input]], " l_shift1 " \n\t" \
 \
"st2 { " s_bank0##_0r ".s, " s_bank0##_0i ".s }[2], [%[output]], " s_shift0 " \n\t" \
"ld2 { " l_bank1##_0r ".s, " l_bank1##_0i ".s }[0], [%[input]], " l_shift0 " \n\t" \
"st2 { " s_bank0##_1r ".s, " s_bank0##_1i ".s }[2], [%[output]], " s_shift0 " \n\t" \
"ld2 { " l_bank1##_0r ".s, " l_bank1##_0i ".s }[1], [%[input]], " l_shift0 " \n\t" \
"st2 { " s_bank0##_2r ".s, " s_bank0##_2i ".s }[2], [%[output]], " s_shift0 " \n\t" \
"ld2 { " l_bank1##_0r ".s, " l_bank1##_0i ".s }[2], [%[input]], " l_shift0 " \n\t" \
"st2 { " s_bank0##_3r ".s, " s_bank0##_3i ".s }[2], [%[output]], " s_shift0 " \n\t" \
"st2 { " s_bank1##_0r ".s, " s_bank1##_0i ".s }[2], [%[output]], " s_shift0 " \n\t" \
"st2 { " s_bank1##_1r ".s, " s_bank1##_1i ".s }[2], [%[output]], " s_shift0 " \n\t" \
"st2 { " s_bank1##_2r ".s, " s_bank1##_2i ".s }[2], [%[output]], " s_shift0 " \n\t" \
"st2 { " s_bank1##_3r ".s, " s_bank1##_3i ".s }[2], [%[output]], " s_shift1 " \n\t" \
 \
"st2 { " s_bank0##_0r ".s, " s_bank0##_0i ".s }[3], [%[output]], " s_shift0 " \n\t" \
"ld2 { " l_bank1##_0r ".s, " l_bank1##_0i ".s }[3], [%[input]], " l_shift1 " \n\t" \
"st2 { " s_bank0##_1r ".s, " s_bank0##_1i ".s }[3], [%[output]], " s_shift0 " \n\t" \
"ld2 { " l_bank1##_1r ".s, " l_bank1##_1i ".s }[0], [%[input]], " l_shift0 " \n\t" \
"st2 { " s_bank0##_2r ".s, " s_bank0##_2i ".s }[3], [%[output]], " s_shift0 " \n\t" \
"ld2 { " l_bank1##_1r ".s, " l_bank1##_1i ".s }[1], [%[input]], " l_shift0 " \n\t" \
"st2 { " s_bank0##_3r ".s, " s_bank0##_3i ".s }[3], [%[output]], " s_shift0 " \n\t" \
"ld2 { " l_bank1##_1r ".s, " l_bank1##_1i ".s }[2], [%[input]], " l_shift0 " \n\t" \
"st2 { " s_bank1##_0r ".s, " s_bank1##_0i ".s }[3], [%[output]], " s_shift0 " \n\t" \
"ld2 { " l_bank1##_1r ".s, " l_bank1##_1i ".s }[3], [%[input]], " l_shift1 " \n\t" \
"st2 { " s_bank1##_1r ".s, " s_bank1##_1i ".s }[3], [%[output]], " s_shift0 " \n\t" \
"ld2 { " l_bank1##_2r ".s, " l_bank1##_2i ".s }[0], [%[input]], " l_shift0 " \n\t" \
"st2 { " s_bank1##_2r ".s, " s_bank1##_2i ".s }[3], [%[output]], " s_shift0 " \n\t" \
"ld2 { " l_bank1##_2r ".s, " l_bank1##_2i ".s }[1], [%[input]], " l_shift0 " \n\t" \
"st2 { " s_bank1##_3r ".s, " s_bank1##_3i ".s }[3], [%[output]], " s_shift1 " \n\t" \
"ld2 { " l_bank1##_2r ".s, " l_bank1##_2i ".s }[2], [%[input]], " l_shift0 " \n\t" \
 \
"ld2 { " l_bank1##_2r ".s, " l_bank1##_2i ".s }[3], [%[input]], " l_shift1 " \n\t" \
"ld2 { " l_bank1##_3r ".s, " l_bank1##_3i ".s }[0], [%[input]], " l_shift0 " \n\t" \
"ld2 { " l_bank1##_3r ".s, " l_bank1##_3i ".s }[1], [%[input]], " l_shift0 " \n\t" \
"ld2 { " l_bank1##_3r ".s, " l_bank1##_3i ".s }[2], [%[input]], " l_shift0 " \n\t" \
"ld2 { " l_bank1##_3r ".s, " l_bank1##_3i ".s }[3], [%[input]], " l_shift1 " \n\t" \

#define load_by_reg_shift2_single(bank, shift0, shift1) \
"ld2 {" bank##_0r ".s," bank##_0i ".s}[0], [%[input]], " shift0 " \n\t" \
"ld2 {" bank##_0r ".s," bank##_0i ".s}[1], [%[input]], " shift1 " \n\t" \
"ld2 {" bank##_0r ".s," bank##_0i ".s}[2], [%[input]], " shift0 " \n\t" \
"ld2 {" bank##_0r ".s," bank##_0i ".s}[3], [%[input]], " shift1 " \n\t" \
 \
"ld2 {" bank##_1r ".s," bank##_1i ".s}[0], [%[input]], " shift0 " \n\t" \
"ld2 {" bank##_1r ".s," bank##_1i ".s}[1], [%[input]], " shift1 " \n\t" \
"ld2 {" bank##_1r ".s," bank##_1i ".s}[2], [%[input]], " shift0 " \n\t" \
"ld2 {" bank##_1r ".s," bank##_1i ".s}[3], [%[input]], " shift1 " \n\t" \
 \
"ld2 {" bank##_2r ".s," bank##_2i ".s}[0], [%[input]], " shift0 " \n\t" \
"ld2 {" bank##_2r ".s," bank##_2i ".s}[1], [%[input]], " shift1 " \n\t" \
"ld2 {" bank##_2r ".s," bank##_2i ".s}[2], [%[input]], " shift0 " \n\t" \
"ld2 {" bank##_2r ".s," bank##_2i ".s}[3], [%[input]], " shift1 " \n\t" \
 \
"ld2 {" bank##_3r ".s," bank##_3i ".s}[0], [%[input]], " shift0 " \n\t" \
"ld2 {" bank##_3r ".s," bank##_3i ".s}[1], [%[input]], " shift1 " \n\t" \
"ld2 {" bank##_3r ".s," bank##_3i ".s}[2], [%[input]], " shift0 " \n\t" \
"ld2 {" bank##_3r ".s," bank##_3i ".s}[3], [%[input]], " shift1 " \n\t" \

#define store_by_reg_alt_shift2_single(bank, shift0, shift1) \
"st2 {" bank##_0r ".s," bank##_0i ".s}[0], [%[output]], " shift0 " \n\t" \
"st2 {" bank##_0r ".s," bank##_0i ".s}[1], [%[output]], " shift1 " \n\t" \
"st2 {" bank##_1r ".s," bank##_1i ".s}[0], [%[output]], " shift0 " \n\t" \
"st2 {" bank##_1r ".s," bank##_1i ".s}[1], [%[output]], " shift1 " \n\t" \
"st2 {" bank##_0r ".s," bank##_0i ".s}[2], [%[output]], " shift0 " \n\t" \
"st2 {" bank##_0r ".s," bank##_0i ".s}[3], [%[output]], " shift1 " \n\t" \
"st2 {" bank##_1r ".s," bank##_1i ".s}[2], [%[output]], " shift0 " \n\t" \
"st2 {" bank##_1r ".s," bank##_1i ".s}[3], [%[output]], " shift1 " \n\t" \
 \
"st2 {" bank##_2r ".s," bank##_2i ".s}[0], [%[output]], " shift0 " \n\t" \
"st2 {" bank##_2r ".s," bank##_2i ".s}[1], [%[output]], " shift1 " \n\t" \
"st2 {" bank##_3r ".s," bank##_3i ".s}[0], [%[output]], " shift0 " \n\t" \
"st2 {" bank##_3r ".s," bank##_3i ".s}[1], [%[output]], " shift1 " \n\t" \
"st2 {" bank##_2r ".s," bank##_2i ".s}[2], [%[output]], " shift0 " \n\t" \
"st2 {" bank##_2r ".s," bank##_2i ".s}[3], [%[output]], " shift1 " \n\t" \
"st2 {" bank##_3r ".s," bank##_3i ".s}[2], [%[output]], " shift0 " \n\t" \
"st2 {" bank##_3r ".s," bank##_3i ".s}[3], [%[output]], " shift1 " \n\t" \

#define store_by_reg_alt_shift2_load_by_reg_shift2_single(s_bank, s_shift0, s_shift1, l_bank, l_shift0, l_shift1) \
"st2 {" s_bank##_0r ".s," s_bank##_0i ".s}[0], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_0r ".s," l_bank##_0i ".s}[0], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank##_0r ".s," s_bank##_0i ".s}[1], [%[output]], " s_shift1 " \n\t" \
"ld2 {" l_bank##_0r ".s," l_bank##_0i ".s}[1], [%[input]], " l_shift1 " \n\t" \
"st2 {" s_bank##_1r ".s," s_bank##_1i ".s}[0], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_0r ".s," l_bank##_0i ".s}[2], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank##_1r ".s," s_bank##_1i ".s}[1], [%[output]], " s_shift1 " \n\t" \
"ld2 {" l_bank##_0r ".s," l_bank##_0i ".s}[3], [%[input]], " l_shift1 " \n\t" \
 \
"st2 {" s_bank##_0r ".s," s_bank##_0i ".s}[2], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_1r ".s," l_bank##_1i ".s}[0], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank##_0r ".s," s_bank##_0i ".s}[3], [%[output]], " s_shift1 " \n\t" \
"ld2 {" l_bank##_1r ".s," l_bank##_1i ".s}[1], [%[input]], " l_shift1 " \n\t" \
"st2 {" s_bank##_1r ".s," s_bank##_1i ".s}[2], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_1r ".s," l_bank##_1i ".s}[2], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank##_1r ".s," s_bank##_1i ".s}[3], [%[output]], " s_shift1 " \n\t" \
"ld2 {" l_bank##_1r ".s," l_bank##_1i ".s}[3], [%[input]], " l_shift1 " \n\t" \
 \
"st2 {" s_bank##_2r ".s," s_bank##_2i ".s}[0], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_2r ".s," l_bank##_2i ".s}[0], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank##_2r ".s," s_bank##_2i ".s}[1], [%[output]], " s_shift1 " \n\t" \
"ld2 {" l_bank##_2r ".s," l_bank##_2i ".s}[1], [%[input]], " l_shift1 " \n\t" \
"st2 {" s_bank##_3r ".s," s_bank##_3i ".s}[0], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_2r ".s," l_bank##_2i ".s}[2], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank##_3r ".s," s_bank##_3i ".s}[1], [%[output]], " s_shift1 " \n\t" \
"ld2 {" l_bank##_2r ".s," l_bank##_2i ".s}[3], [%[input]], " l_shift1 " \n\t" \
 \
"st2 {" s_bank##_2r ".s," s_bank##_2i ".s}[2], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_3r ".s," l_bank##_3i ".s}[0], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank##_2r ".s," s_bank##_2i ".s}[3], [%[output]], " s_shift1 " \n\t" \
"ld2 {" l_bank##_3r ".s," l_bank##_3i ".s}[1], [%[input]], " l_shift1 " \n\t" \
"st2 {" s_bank##_3r ".s," s_bank##_3i ".s}[2], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_3r ".s," l_bank##_3i ".s}[2], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank##_3r ".s," s_bank##_3i ".s}[3], [%[output]], " s_shift1 " \n\t" \
"ld2 {" l_bank##_3r ".s," l_bank##_3i ".s}[3], [%[input]], " l_shift1 " \n\t" \

#define store_by_reg_even_alt_half(bank, shift0, shift1) \
"st2 {" bank##_0r ".s," bank##_0i ".s}[0], [%[output]], " shift0 " \n\t" \
"st2 {" bank##_1r ".s," bank##_1i ".s}[0], [%[output]], " shift0 " \n\t" \
"st2 {" bank##_0r ".s," bank##_0i ".s}[2], [%[output]], " shift0 " \n\t" \
"st2 {" bank##_1r ".s," bank##_1i ".s}[2], [%[output]], " shift0 " \n\t" \
"st2 {" bank##_2r ".s," bank##_2i ".s}[0], [%[output]], " shift0 " \n\t" \
"st2 {" bank##_3r ".s," bank##_3i ".s}[0], [%[output]], " shift0 " \n\t" \
"st2 {" bank##_2r ".s," bank##_2i ".s}[2], [%[output]], " shift0 " \n\t" \
"st2 {" bank##_3r ".s," bank##_3i ".s}[2], [%[output]], " shift1 " \n\t" \

#define store_by_reg_odd_alt_half(bank, shift0, shift1) \
"st2 {" bank##_0r ".s," bank##_0i ".s}[1], [%[output]], " shift0 " \n\t" \
"st2 {" bank##_1r ".s," bank##_1i ".s}[1], [%[output]], " shift0 " \n\t" \
"st2 {" bank##_0r ".s," bank##_0i ".s}[3], [%[output]], " shift0 " \n\t" \
"st2 {" bank##_1r ".s," bank##_1i ".s}[3], [%[output]], " shift0 " \n\t" \
"st2 {" bank##_2r ".s," bank##_2i ".s}[1], [%[output]], " shift0 " \n\t" \
"st2 {" bank##_3r ".s," bank##_3i ".s}[1], [%[output]], " shift0 " \n\t" \
"st2 {" bank##_2r ".s," bank##_2i ".s}[3], [%[output]], " shift0 " \n\t" \
"st2 {" bank##_3r ".s," bank##_3i ".s}[3], [%[output]], " shift1 " \n\t" \

#define store_by_reg_even_alt_single(bank0, bank1, shift0, shift1) \
store_by_reg_even_alt_half(bank0, shift0, shift1) \
store_by_reg_even_alt_half(bank1, shift0, shift1) \

#define store_by_reg_odd_alt_single(bank0, bank1, shift0, shift1) \
store_by_reg_odd_alt_half(bank0, shift0, shift1) \
store_by_reg_odd_alt_half(bank1, shift0, shift1) \

#define store_by_reg_even_alt_load_by_reg_shift2_single(s_bank0, s_bank1, s_shift0, s_shift1, l_bank, l_shift0, l_shift1) \
"st2 {" s_bank0##_0r ".s," s_bank0##_0i ".s}[0], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_0r ".s," l_bank##_0i ".s}[0], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank0##_1r ".s," s_bank0##_1i ".s}[0], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_0r ".s," l_bank##_0i ".s}[1], [%[input]], " l_shift1 " \n\t" \
"st2 {" s_bank0##_0r ".s," s_bank0##_0i ".s}[2], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_0r ".s," l_bank##_0i ".s}[2], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank0##_1r ".s," s_bank0##_1i ".s}[2], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_0r ".s," l_bank##_0i ".s}[3], [%[input]], " l_shift1 " \n\t" \
"st2 {" s_bank0##_2r ".s," s_bank0##_2i ".s}[0], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_1r ".s," l_bank##_1i ".s}[0], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank0##_3r ".s," s_bank0##_3i ".s}[0], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_1r ".s," l_bank##_1i ".s}[1], [%[input]], " l_shift1 " \n\t" \
"st2 {" s_bank0##_2r ".s," s_bank0##_2i ".s}[2], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_1r ".s," l_bank##_1i ".s}[2], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank0##_3r ".s," s_bank0##_3i ".s}[2], [%[output]], " s_shift1 " \n\t" \
"ld2 {" l_bank##_1r ".s," l_bank##_1i ".s}[3], [%[input]], " l_shift1 " \n\t" \
 \
"st2 {" s_bank1##_0r ".s," s_bank1##_0i ".s}[0], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_2r ".s," l_bank##_2i ".s}[0], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank1##_1r ".s," s_bank1##_1i ".s}[0], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_2r ".s," l_bank##_2i ".s}[1], [%[input]], " l_shift1 " \n\t" \
"st2 {" s_bank1##_0r ".s," s_bank1##_0i ".s}[2], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_2r ".s," l_bank##_2i ".s}[2], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank1##_1r ".s," s_bank1##_1i ".s}[2], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_2r ".s," l_bank##_2i ".s}[3], [%[input]], " l_shift1 " \n\t" \
"st2 {" s_bank1##_2r ".s," s_bank1##_2i ".s}[0], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_3r ".s," l_bank##_3i ".s}[0], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank1##_3r ".s," s_bank1##_3i ".s}[0], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_3r ".s," l_bank##_3i ".s}[1], [%[input]], " l_shift1 " \n\t" \
"st2 {" s_bank1##_2r ".s," s_bank1##_2i ".s}[2], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_3r ".s," l_bank##_3i ".s}[2], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank1##_3r ".s," s_bank1##_3i ".s}[2], [%[output]], " s_shift1 " \n\t" \
"ld2 {" l_bank##_3r ".s," l_bank##_3i ".s}[3], [%[input]], " l_shift1 " \n\t" \

#define store_by_reg_odd_alt_load_by_reg_shift2_single(s_bank0, s_bank1, s_shift0, s_shift1, l_bank, l_shift0, l_shift1) \
"st2 {" s_bank0##_0r ".s," s_bank0##_0i ".s}[1], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_0r ".s," l_bank##_0i ".s}[0], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank0##_1r ".s," s_bank0##_1i ".s}[1], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_0r ".s," l_bank##_0i ".s}[1], [%[input]], " l_shift1 " \n\t" \
"st2 {" s_bank0##_0r ".s," s_bank0##_0i ".s}[3], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_0r ".s," l_bank##_0i ".s}[2], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank0##_1r ".s," s_bank0##_1i ".s}[3], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_0r ".s," l_bank##_0i ".s}[3], [%[input]], " l_shift1 " \n\t" \
"st2 {" s_bank0##_2r ".s," s_bank0##_2i ".s}[1], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_1r ".s," l_bank##_1i ".s}[0], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank0##_3r ".s," s_bank0##_3i ".s}[1], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_1r ".s," l_bank##_1i ".s}[1], [%[input]], " l_shift1 " \n\t" \
"st2 {" s_bank0##_2r ".s," s_bank0##_2i ".s}[3], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_1r ".s," l_bank##_1i ".s}[2], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank0##_3r ".s," s_bank0##_3i ".s}[3], [%[output]], " s_shift1 " \n\t" \
"ld2 {" l_bank##_1r ".s," l_bank##_1i ".s}[3], [%[input]], " l_shift1 " \n\t" \
 \
"st2 {" s_bank1##_0r ".s," s_bank1##_0i ".s}[1], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_2r ".s," l_bank##_2i ".s}[0], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank1##_1r ".s," s_bank1##_1i ".s}[1], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_2r ".s," l_bank##_2i ".s}[1], [%[input]], " l_shift1 " \n\t" \
"st2 {" s_bank1##_0r ".s," s_bank1##_0i ".s}[3], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_2r ".s," l_bank##_2i ".s}[2], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank1##_1r ".s," s_bank1##_1i ".s}[3], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_2r ".s," l_bank##_2i ".s}[3], [%[input]], " l_shift1 " \n\t" \
"st2 {" s_bank1##_2r ".s," s_bank1##_2i ".s}[1], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_3r ".s," l_bank##_3i ".s}[0], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank1##_3r ".s," s_bank1##_3i ".s}[1], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_3r ".s," l_bank##_3i ".s}[1], [%[input]], " l_shift1 " \n\t" \
"st2 {" s_bank1##_2r ".s," s_bank1##_2i ".s}[3], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_3r ".s," l_bank##_3i ".s}[2], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank1##_3r ".s," s_bank1##_3i ".s}[3], [%[output]], " s_shift1 " \n\t" \
"ld2 {" l_bank##_3r ".s," l_bank##_3i ".s}[3], [%[input]], " l_shift1 " \n\t" \

#define store_by_reg_even_alt_load_by_reg_single(s_bank0, s_bank1, s_shift0, s_shift1, l_bank, l_shift0, l_shift1) \
"st2 {" s_bank0##_0r ".s," s_bank0##_0i ".s}[0], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_0r ".s," l_bank##_0i ".s}[0], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank0##_1r ".s," s_bank0##_1i ".s}[0], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_0r ".s," l_bank##_0i ".s}[1], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank0##_0r ".s," s_bank0##_0i ".s}[2], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_0r ".s," l_bank##_0i ".s}[2], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank0##_1r ".s," s_bank0##_1i ".s}[2], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_0r ".s," l_bank##_0i ".s}[3], [%[input]], " l_shift1 " \n\t" \
"st2 {" s_bank0##_2r ".s," s_bank0##_2i ".s}[0], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_1r ".s," l_bank##_1i ".s}[0], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank0##_3r ".s," s_bank0##_3i ".s}[0], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_1r ".s," l_bank##_1i ".s}[1], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank0##_2r ".s," s_bank0##_2i ".s}[2], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_1r ".s," l_bank##_1i ".s}[2], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank0##_3r ".s," s_bank0##_3i ".s}[2], [%[output]], " s_shift1 " \n\t" \
"ld2 {" l_bank##_1r ".s," l_bank##_1i ".s}[3], [%[input]], " l_shift1 " \n\t" \
 \
"st2 {" s_bank1##_0r ".s," s_bank1##_0i ".s}[0], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_2r ".s," l_bank##_2i ".s}[0], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank1##_1r ".s," s_bank1##_1i ".s}[0], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_2r ".s," l_bank##_2i ".s}[1], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank1##_0r ".s," s_bank1##_0i ".s}[2], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_2r ".s," l_bank##_2i ".s}[2], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank1##_1r ".s," s_bank1##_1i ".s}[2], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_2r ".s," l_bank##_2i ".s}[3], [%[input]], " l_shift1 " \n\t" \
"st2 {" s_bank1##_2r ".s," s_bank1##_2i ".s}[0], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_3r ".s," l_bank##_3i ".s}[0], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank1##_3r ".s," s_bank1##_3i ".s}[0], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_3r ".s," l_bank##_3i ".s}[1], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank1##_2r ".s," s_bank1##_2i ".s}[2], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_3r ".s," l_bank##_3i ".s}[2], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank1##_3r ".s," s_bank1##_3i ".s}[2], [%[output]], " s_shift1 " \n\t" \
"ld2 {" l_bank##_3r ".s," l_bank##_3i ".s}[3], [%[input]], " l_shift1 " \n\t" \

#define store_by_reg_odd_alt_load_by_reg_single(s_bank0, s_bank1, s_shift0, s_shift1, l_bank, l_shift0, l_shift1) \
"st2 {" s_bank0##_0r ".s," s_bank0##_0i ".s}[1], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_0r ".s," l_bank##_0i ".s}[0], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank0##_1r ".s," s_bank0##_1i ".s}[1], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_0r ".s," l_bank##_0i ".s}[1], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank0##_0r ".s," s_bank0##_0i ".s}[3], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_0r ".s," l_bank##_0i ".s}[2], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank0##_1r ".s," s_bank0##_1i ".s}[3], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_0r ".s," l_bank##_0i ".s}[3], [%[input]], " l_shift1 " \n\t" \
"st2 {" s_bank0##_2r ".s," s_bank0##_2i ".s}[1], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_1r ".s," l_bank##_1i ".s}[0], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank0##_3r ".s," s_bank0##_3i ".s}[1], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_1r ".s," l_bank##_1i ".s}[1], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank0##_2r ".s," s_bank0##_2i ".s}[3], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_1r ".s," l_bank##_1i ".s}[2], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank0##_3r ".s," s_bank0##_3i ".s}[3], [%[output]], " s_shift1 " \n\t" \
"ld2 {" l_bank##_1r ".s," l_bank##_1i ".s}[3], [%[input]], " l_shift1 " \n\t" \
 \
"st2 {" s_bank1##_0r ".s," s_bank1##_0i ".s}[1], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_2r ".s," l_bank##_2i ".s}[0], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank1##_1r ".s," s_bank1##_1i ".s}[1], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_2r ".s," l_bank##_2i ".s}[1], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank1##_0r ".s," s_bank1##_0i ".s}[3], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_2r ".s," l_bank##_2i ".s}[2], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank1##_1r ".s," s_bank1##_1i ".s}[3], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_2r ".s," l_bank##_2i ".s}[3], [%[input]], " l_shift1 " \n\t" \
"st2 {" s_bank1##_2r ".s," s_bank1##_2i ".s}[1], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_3r ".s," l_bank##_3i ".s}[0], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank1##_3r ".s," s_bank1##_3i ".s}[1], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_3r ".s," l_bank##_3i ".s}[1], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank1##_2r ".s," s_bank1##_2i ".s}[3], [%[output]], " s_shift0 " \n\t" \
"ld2 {" l_bank##_3r ".s," l_bank##_3i ".s}[2], [%[input]], " l_shift0 " \n\t" \
"st2 {" s_bank1##_3r ".s," s_bank1##_3i ".s}[3], [%[output]], " s_shift1 " \n\t" \
"ld2 {" l_bank##_3r ".s," l_bank##_3i ".s}[3], [%[input]], " l_shift1 " \n\t" \

#endif //FFT_LOAD_H
