# OneFFT measurements

What was measured while building OneFFT, and what followed from it. [design.md](design.md)
describes the result; this document is the evidence behind its decisions, and the place to look
before trying to make something faster: "Open questions" at the end lists what is known to be
unmeasured or unbuilt, "Decisions and their effect" what was tried and what it gave.

## Setup

| machine | core | instruction set |
|---|---|---|
| Google Pixel 3a | Kryo 360 Gold (Cortex-A75 derivative), 2.0 GHz | AArch64 |
| Samsung Galaxy A50 | Cortex-A73, 2.3 GHz | AArch64 |
| Samsung Galaxy S24 | Cortex-X4, 3.2 GHz | AArch64 |
| Apple M2 | Avalanche, 3.5 GHz | AArch64 |
| Laptop | AMD Ryzen 7 5700U (Zen 2), 4.3 GHz | AVX2 |
| Desktop | Intel Core i5-9600K (Skylake), 4.5 GHz | AVX2 |
| Server | AMD EPYC 7443P (Zen 3), 2.85 GHz | AVX2 |
| Server | AMD EPYC 9254 (Zen 4), 4.15 GHz | AVX-512 |
| Server | AMD EPYC 9255 (Zen 5), 4.3 GHz | AVX-512 |
| Server | AMD Ryzen 9 9950X (Zen 5), 5.3 to 5.5 GHz | AVX-512 |
| Server | Intel Xeon Gold 6230R (Cascade Lake), 2.1 GHz | AVX-512 |

All times are for one forward transform, single thread, pinned to the fastest cores on the
phones, minimum over 15 samples of at least 5 ms each (`bench/bench.c`). Every library is called
through its out-of-place interface on interleaved data, except vDSP, which only has split real
and imaginary arrays and is given those (the conversion is not timed, which favours it).

* FFTW 3.3.10, single precision, NEON or AVX2 and AVX-512 enabled, `FFTW_PATIENT` plans up to
  4096 points and `FFTW_MEASURE` above (patient planning of large sizes takes minutes).
* FFmpeg 7.1, `av_tx` with `AV_TX_FLOAT_FFT`, with its assembly for the target.
* PFFFT (the marton78 fork), `pffft_transform_ordered`; SSE or NEON, it has no AVX kernels.
* vDSP from Accelerate, `vDSP_DFT_Execute`.
* KFR 6.3.1 through its C API, `kfr_dft_execute_f32`, built with Clang; it picks SSE2 to
  AVX-512 at run time.
* TwoFFT, a sibling library, compiled from its source tree next to this one (`../TwoFFT`, or
  `TWOFFT_DIR`) with the benchmark's flags; AArch64 only, its x86-64 backend is not finished.
  Two versions are in the tables: on the phones the current one (commit 398f3b5, headers with
  inline assembly kernels, `tw_a64_fft`), on the M2 the earlier one (branch `intrinsics-run`,
  commit bf2b65d, `twofft_forward`). On the phones the current one takes 0.68..0.77 times the
  time of the earlier one from 2^16 points up on the Pixel 3a and the A50 and 0.85..0.98 times
  on the S24, 0.85..1.08 times from 128 points to 2^15, 0.54..0.86 times at 2, 4, 32 and 64
  points, and 0.92..1.18 times at 8 and 16 points.

KFR and TwoFFT were measured later than the others and only on the phones and the M2, in a run
of OneFFT, TwoFFT and KFR with the same settings. OneFFT's times in that run were within 3% of
the ones in the tables at every size from 8 points up, except 6% at 2^19 points on the A50, and
within 16% below that, so the lines were added to the earlier results. The current TwoFFT was
measured again later on the phones, in a run of OneFFT and TwoFFT; OneFFT's times in that run
were within 7% of the ones in the tables from 4 points up (13% at 2 points on the Pixel 3a), and
TwoFFT's lines were replaced.

`scripts/third_party.sh` builds the libraries and `scripts/build_bench.sh` the benchmark.
Compilers: Clang 18 (Android NDK r27c), Apple Clang 21, GCC 15.

Turbo is switched off on the Xeon: everything runs at 2.1 GHz there. With turbo, processors of
its generation run 512 bit code at a lower frequency than other code; what that does to the
AVX-512 kernels against the AVX2 kernels was not measured.

The input buffer is at the start of a page and the output buffer 2048 bytes behind one. Where
the two sit relative to each other matters for small transforms on some cores: with the same
offset in their pages, loads of the next transform wait for stores of the previous one. On
Zen 4 FFTW then takes 56 instead of 29 ns for 64 points and OneFFT 56 instead of 47 ns for 128;
on Cascade Lake OneFFT is up to 7% slower from 32 to 128 points. The benchmark also works for
half a second before it measures: a core that was just woken up has not reached its frequency
yet, and the first size came out up to 1.5 times too slow on the phones. On Zen 2, Zen 5 and
the M2, measured both ways, the small sizes differ by at most 3% with it, on Zen 4 by at most
6% from 4 points up.

## Results

The charts show speed in "mflops": 5 n log2(n), the operation count of a textbook radix 2
transform of n points, divided by the time in microseconds. In this unit a library whose cost
per operation does not depend on the size draws a horizontal line, and the drop of every curve
towards the right is the transform outgrowing the caches. The tables under each chart have the
times in nanoseconds; their columns on the right are the time of the other library divided by
the time of OneFFT.

### Cortex-X4 (Galaxy S24)

<picture>
  <source media="(prefers-color-scheme: dark)" srcset="plots/s24-dark.svg">
  <img src="plots/s24.svg" alt="Line chart of the speed of OneFFT, FFTW, FFmpeg, PFFFT, TwoFFT and KFR over the transform size on a Cortex-X4; the numbers are in the tables below." width="760">
</picture>

<details>
<summary>Times in nanoseconds</summary>

<!-- results:s24 -->
| n | OneFFT | FFTW | FFmpeg | TwoFFT | KFR | PFFFT | vs FFTW | vs FFmpeg | vs TwoFFT | vs KFR | vs PFFFT |
|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|
| 2 | 1.1 | 3.4 | 0.9 | 2.3 | 8.2 | - | 3.09 | 0.82 | 2.09 | 7.45 | - |
| 4 | 1.9 | 3.8 | 1.2 | 2.8 | 8.2 | - | 2.00 | 0.63 | 1.47 | 4.32 | - |
| 8 | 4.4 | 7.5 | 4.8 | 4.6 | 8.9 | - | 1.70 | 1.09 | 1.05 | 2.02 | - |
| 16 | 8.7 | 18.7 | 12.6 | 9.6 | 12.5 | 19.0 | 2.15 | 1.45 | 1.10 | 1.44 | 2.18 |
| 32 | 16.4 | 24.2 | 26.9 | 18.5 | 25.3 | 33.2 | 1.48 | 1.64 | 1.13 | 1.54 | 2.02 |
| 64 | 45.1 | 51.0 | 67.9 | 45.5 | 55.9 | 73.9 | 1.13 | 1.51 | 1.01 | 1.24 | 1.64 |
| 128 | 97.7 | 109.4 | 154.2 | 106.8 | 177.5 | 150.7 | 1.12 | 1.58 | 1.09 | 1.82 | 1.54 |
| 256 | 220.5 | 234.2 | 362.5 | 246.6 | 290.5 | 317.8 | 1.06 | 1.64 | 1.12 | 1.32 | 1.44 |
| 512 | 488.0 | 538.4 | 811.8 | 480.0 | 610.3 | 696.7 | 1.10 | 1.66 | 0.98 | 1.25 | 1.43 |
| 1024 | 1065 | 1250 | 1879 | 1120 | 1456 | 1489 | 1.17 | 1.76 | 1.05 | 1.37 | 1.40 |
| 2048 | 2332 | 2718 | 4123 | 2591 | 2777 | 3362 | 1.17 | 1.77 | 1.11 | 1.19 | 1.44 |
| 4096 | 5077 | 5701 | 9004 | 4975 | 6593 | 7112 | 1.12 | 1.77 | 0.98 | 1.30 | 1.40 |
| 8192 | 11435 | 17572 | 24186 | 11651 | 13293 | 15494 | 1.54 | 2.12 | 1.02 | 1.16 | 1.36 |
| 16384 | 24322 | 41256 | 53966 | 29203 | 32044 | 34386 | 1.70 | 2.22 | 1.20 | 1.32 | 1.41 |
| 32768 | 53457 | 92352 | 117398 | 56611 | 64642 | 74132 | 1.73 | 2.20 | 1.06 | 1.21 | 1.39 |
| 65536 | 114614 | 187395 | 250918 | 124834 | 149020 | 151965 | 1.64 | 2.19 | 1.09 | 1.30 | 1.33 |
| 131072 | 244849 | 452910 | 561308 | 275938 | 298796 | 346826 | 1.85 | 2.29 | 1.13 | 1.22 | 1.42 |
| 262144 | 523408 | 1000362 | 2480664 | 573286 | 690127 | 783203 | 1.91 | 4.74 | 1.10 | 1.32 | 1.50 |
| 524288 | 1211797 | 3074492 | 5571563 | 1257031 | 1471289 | 2163047 | 2.54 | 4.60 | 1.04 | 1.21 | 1.78 |
| 1048576 | 2930782 | 6612149 | 12775937 | 2921797 | 3682812 | 8027657 | 2.26 | 4.36 | 1.00 | 1.26 | 2.74 |
<!-- end -->

</details>

### Kryo 360 Gold (Pixel 3a)

<picture>
  <source media="(prefers-color-scheme: dark)" srcset="plots/pixel3a-dark.svg">
  <img src="plots/pixel3a.svg" alt="Line chart of the speed of OneFFT, FFTW, FFmpeg, PFFFT, TwoFFT and KFR over the transform size on a Pixel 3a; the numbers are in the tables below." width="760">
</picture>

<details>
<summary>Times in nanoseconds</summary>

<!-- results:pixel3a -->
| n | OneFFT | FFTW | FFmpeg | TwoFFT | KFR | PFFFT | vs FFTW | vs FFmpeg | vs TwoFFT | vs KFR | vs PFFFT |
|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|
| 2 | 4.0 | 11.5 | 4.0 | 9.1 | 43.6 | - | 2.88 | 1.00 | 2.27 | 10.90 | - |
| 4 | 8.9 | 15.9 | 5.7 | 10.4 | 44.0 | - | 1.79 | 0.64 | 1.17 | 4.94 | - |
| 8 | 22.2 | 28.9 | 21.6 | 26.4 | 61.2 | - | 1.30 | 0.97 | 1.19 | 2.76 | - |
| 16 | 43.1 | 67.7 | 54.2 | 48.8 | 84.3 | 89.4 | 1.57 | 1.26 | 1.13 | 1.96 | 2.07 |
| 32 | 93.5 | 119.9 | 144.5 | 105.0 | 144.1 | 156.6 | 1.28 | 1.55 | 1.12 | 1.54 | 1.67 |
| 64 | 238.2 | 250.8 | 335.8 | 243.9 | 290.0 | 344.8 | 1.05 | 1.41 | 1.02 | 1.22 | 1.45 |
| 128 | 559.0 | 538.8 | 795.8 | 577.5 | 867.0 | 741.1 | 0.96 | 1.42 | 1.03 | 1.55 | 1.33 |
| 256 | 1192 | 1151 | 1833 | 1262 | 1417 | 1539 | 0.97 | 1.54 | 1.06 | 1.19 | 1.29 |
| 512 | 2593 | 2579 | 4220 | 2663 | 2961 | 3381 | 0.99 | 1.63 | 1.03 | 1.14 | 1.30 |
| 1024 | 5834 | 5725 | 9482 | 6265 | 7135 | 7146 | 0.98 | 1.63 | 1.07 | 1.22 | 1.23 |
| 2048 | 12634 | 14316 | 21093 | 13407 | 14198 | 16119 | 1.13 | 1.67 | 1.06 | 1.12 | 1.28 |
| 4096 | 29180 | 35212 | 51932 | 31129 | 36427 | 41294 | 1.21 | 1.78 | 1.07 | 1.25 | 1.42 |
| 8192 | 62830 | 86675 | 121685 | 72165 | 76918 | 102853 | 1.38 | 1.94 | 1.15 | 1.22 | 1.64 |
| 16384 | 155495 | 211455 | 275615 | 180765 | 178294 | 220664 | 1.36 | 1.77 | 1.16 | 1.15 | 1.42 |
| 32768 | 347650 | 464128 | 655781 | 374460 | 368301 | 483145 | 1.34 | 1.89 | 1.08 | 1.06 | 1.39 |
| 65536 | 780820 | 1150286 | 1639922 | 880690 | 859388 | 1079974 | 1.47 | 2.10 | 1.13 | 1.10 | 1.38 |
| 131072 | 1786745 | 2790157 | 4698282 | 2169740 | 1871198 | 3437449 | 1.56 | 2.63 | 1.21 | 1.05 | 1.92 |
| 262144 | 4331251 | 5998699 | 14627710 | 4368751 | 5016250 | 9446250 | 1.38 | 3.38 | 1.01 | 1.16 | 2.18 |
| 524288 | 8992084 | 17438231 | 30991826 | 9294792 | 12180002 | 20560731 | 1.94 | 3.45 | 1.03 | 1.35 | 2.29 |
| 1048576 | 19480419 | 37079222 | 64838965 | 22631721 | 28067138 | 42778442 | 1.90 | 3.33 | 1.16 | 1.44 | 2.20 |
<!-- end -->

</details>

### Cortex-A73 (Galaxy A50)

<picture>
  <source media="(prefers-color-scheme: dark)" srcset="plots/a50-dark.svg">
  <img src="plots/a50.svg" alt="Line chart of the speed of OneFFT, FFTW, FFmpeg, PFFFT, TwoFFT and KFR over the transform size on a Galaxy A50; the numbers are in the tables below." width="760">
</picture>

<details>
<summary>Times in nanoseconds</summary>

<!-- results:a50 -->
| n | OneFFT | FFTW | FFmpeg | TwoFFT | KFR | PFFFT | vs FFTW | vs FFmpeg | vs TwoFFT | vs KFR | vs PFFFT |
|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|
| 2 | 4.8 | 15.2 | 4.8 | 10.5 | 44.5 | - | 3.17 | 1.00 | 2.19 | 9.27 | - |
| 4 | 10.6 | 20.8 | 7.6 | 15.6 | 69.6 | - | 1.96 | 0.72 | 1.47 | 6.57 | - |
| 8 | 27.5 | 37.6 | 24.2 | 33.4 | 85.5 | - | 1.37 | 0.88 | 1.21 | 3.11 | - |
| 16 | 46.7 | 87.8 | 59.5 | 52.5 | 105.5 | 106.4 | 1.88 | 1.27 | 1.12 | 2.26 | 2.28 |
| 32 | 93.4 | 135.4 | 140.6 | 100.6 | 154.9 | 176.7 | 1.45 | 1.51 | 1.08 | 1.66 | 1.89 |
| 64 | 247.6 | 261.8 | 332.2 | 266.7 | 323.6 | 406.4 | 1.06 | 1.34 | 1.08 | 1.31 | 1.64 |
| 128 | 605.0 | 543.0 | 788.1 | 598.4 | 880.5 | 885.5 | 0.90 | 1.30 | 0.99 | 1.46 | 1.46 |
| 256 | 1277 | 1171 | 1822 | 1265 | 1542 | 1887 | 0.92 | 1.43 | 0.99 | 1.21 | 1.48 |
| 512 | 2677 | 2774 | 4208 | 2831 | 3286 | 4227 | 1.04 | 1.57 | 1.06 | 1.23 | 1.58 |
| 1024 | 6407 | 6809 | 9518 | 6344 | 7856 | 8940 | 1.06 | 1.49 | 0.99 | 1.23 | 1.40 |
| 2048 | 14778 | 15745 | 23105 | 15150 | 18187 | 22889 | 1.07 | 1.56 | 1.03 | 1.23 | 1.55 |
| 4096 | 32760 | 37823 | 59516 | 35564 | 46927 | 53824 | 1.15 | 1.82 | 1.09 | 1.43 | 1.64 |
| 8192 | 81630 | 100630 | 141056 | 86068 | 99296 | 127062 | 1.23 | 1.73 | 1.05 | 1.22 | 1.56 |
| 16384 | 170288 | 226726 | 309072 | 190683 | 231139 | 250625 | 1.33 | 1.81 | 1.12 | 1.36 | 1.47 |
| 32768 | 374240 | 541457 | 682038 | 418914 | 510764 | 634452 | 1.45 | 1.82 | 1.12 | 1.36 | 1.70 |
| 65536 | 905154 | 1456980 | 2060020 | 1051279 | 1262019 | 1684346 | 1.61 | 2.28 | 1.16 | 1.39 | 1.86 |
| 131072 | 2036846 | 3425769 | 6123154 | 2557154 | 3288962 | 5838577 | 1.68 | 3.01 | 1.26 | 1.61 | 2.87 |
| 262144 | 4767000 | 7681847 | 17338769 | 5206192 | 8310731 | 10369846 | 1.61 | 3.64 | 1.09 | 1.74 | 2.18 |
| 524288 | 10013424 | 19744615 | 36122308 | 10956923 | 19915884 | 26964346 | 1.97 | 3.61 | 1.09 | 1.99 | 2.69 |
| 1048576 | 21215846 | 42221731 | 76520807 | 27158577 | 42628423 | 47696770 | 1.99 | 3.61 | 1.28 | 2.01 | 2.25 |
<!-- end -->

</details>

### Apple M2

<picture>
  <source media="(prefers-color-scheme: dark)" srcset="plots/m2-dark.svg">
  <img src="plots/m2.svg" alt="Line chart of the speed of OneFFT, FFTW, FFmpeg, PFFFT, vDSP, TwoFFT and KFR over the transform size on an Apple M2; the numbers are in the tables below." width="760">
</picture>

<details>
<summary>Times in nanoseconds</summary>

<!-- results:m2 -->
| n | OneFFT | FFTW | FFmpeg | TwoFFT | KFR | PFFFT | vDSP | vs FFTW | vs FFmpeg | vs TwoFFT | vs KFR | vs PFFFT | vs vDSP |
|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|
| 2 | 1.4 | 2.6 | 1.1 | 3.4 | 5.3 | - | - | 1.86 | 0.79 | 2.43 | 3.79 | - | - |
| 4 | 1.9 | 3.7 | 1.1 | 4.1 | 5.6 | - | - | 1.95 | 0.58 | 2.16 | 2.95 | - | - |
| 8 | 3.4 | 6.6 | 3.7 | 4.9 | 7.1 | - | - | 1.94 | 1.09 | 1.44 | 2.09 | - | - |
| 16 | 6.9 | 14.6 | 8.6 | 8.3 | 11.2 | 17.5 | 11.2 | 2.12 | 1.25 | 1.20 | 1.62 | 2.54 | 1.62 |
| 32 | 14.3 | 18.1 | 20.9 | 23.7 | 20.6 | 28.6 | 17.7 | 1.27 | 1.46 | 1.66 | 1.44 | 2.00 | 1.24 |
| 64 | 34.9 | 34.5 | 50.0 | 44.0 | 44.8 | 57.4 | 34.7 | 0.99 | 1.43 | 1.26 | 1.28 | 1.64 | 0.99 |
| 128 | 76.7 | 73.3 | 113.8 | 92.2 | 139.0 | 117.2 | 68.2 | 0.96 | 1.48 | 1.20 | 1.81 | 1.53 | 0.89 |
| 256 | 166.6 | 156.4 | 257.3 | 182.9 | 215.4 | 234.7 | 154.2 | 0.94 | 1.54 | 1.10 | 1.29 | 1.41 | 0.93 |
| 512 | 361.3 | 346.8 | 584.7 | 385.1 | 425.4 | 545.9 | 328.1 | 0.96 | 1.62 | 1.07 | 1.18 | 1.51 | 0.91 |
| 1024 | 789.3 | 747.6 | 1306 | 848.9 | 1019 | 1103 | 536.0 | 0.95 | 1.65 | 1.08 | 1.29 | 1.40 | 0.68 |
| 2048 | 1718 | 1741 | 2890 | 1810 | 2000 | 2464 | 1022 | 1.01 | 1.68 | 1.05 | 1.16 | 1.43 | 0.59 |
| 4096 | 3693 | 3858 | 6330 | 3755 | 4631 | 5139 | 2322 | 1.04 | 1.71 | 1.02 | 1.25 | 1.39 | 0.63 |
| 8192 | 8250 | 9535 | 13922 | 8559 | 9856 | 13656 | 5332 | 1.16 | 1.69 | 1.04 | 1.19 | 1.66 | 0.65 |
| 16384 | 18699 | 22430 | 32625 | 19148 | 23898 | 29719 | 11594 | 1.20 | 1.74 | 1.02 | 1.28 | 1.59 | 0.62 |
| 32768 | 40984 | 56562 | 76906 | 58938 | 58922 | 66203 | 25016 | 1.38 | 1.88 | 1.44 | 1.44 | 1.62 | 0.61 |
| 65536 | 87250 | 125906 | 165812 | 110688 | 151625 | 130844 | 63484 | 1.44 | 1.90 | 1.27 | 1.74 | 1.50 | 0.73 |
| 131072 | 182562 | 258875 | 345875 | 200500 | 323375 | 293562 | 137344 | 1.42 | 1.89 | 1.10 | 1.77 | 1.61 | 0.75 |
| 262144 | 386750 | 564625 | 1537500 | 611250 | 722250 | 603500 | 283250 | 1.46 | 3.98 | 1.58 | 1.87 | 1.56 | 0.73 |
| 524288 | 821500 | 2058500 | 3229000 | 1064750 | 1468000 | 1293000 | 670000 | 2.51 | 3.93 | 1.30 | 1.79 | 1.57 | 0.82 |
| 1048576 | 1939000 | 4417000 | 6915000 | 2075500 | 3171000 | 2630000 | 2246000 | 2.28 | 3.57 | 1.07 | 1.64 | 1.36 | 1.16 |
<!-- end -->

</details>

### AMD EPYC 9255 (Zen 5), AVX-512

<picture>
  <source media="(prefers-color-scheme: dark)" srcset="plots/zen5-dark.svg">
  <img src="plots/zen5.svg" alt="Line chart of the speed of OneFFT with its AVX-512 and with its AVX2 kernels, FFTW, FFmpeg and PFFFT over the transform size on an AMD EPYC 9255; the numbers are in the tables below." width="760">
</picture>

"OneFFT (AVX2)" is OneFFT built with `-DONEFFT_NO_AVX512`. FFTW and FFmpeg use AVX-512 where
they have code for it. The times are the minimum of two runs, which agree within 5% from 8
points up except for FFTW at 64 points (18%). OneFFT's times are within 3% of the first
measurement, on another server of this model and before the warm-up; the other libraries'
within 8%, FFTW's within 11%.

<details>
<summary>Times in nanoseconds</summary>

<!-- results:zen5 -->
| n | OneFFT | OneFFT (AVX2) | FFTW | FFmpeg | PFFFT | vs OneFFT (AVX2) | vs FFTW | vs FFmpeg | vs PFFFT |
|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|
| 2 | 1.2 | 1.2 | 3.2 | 1.6 | - | 1.00 | 2.67 | 1.33 | - |
| 4 | 1.9 | 1.9 | 4.5 | 1.6 | - | 1.00 | 2.37 | 0.84 | - |
| 8 | 3.8 | 3.8 | 7.0 | 2.9 | - | 1.00 | 1.84 | 0.76 | - |
| 16 | 4.6 | 4.6 | 13.8 | 6.7 | 17.6 | 1.00 | 3.00 | 1.46 | 3.83 |
| 32 | 6.7 | 9.9 | 15.5 | 14.1 | 29.0 | 1.48 | 2.31 | 2.10 | 4.33 |
| 64 | 14.7 | 25.9 | 22.3 | 49.8 | 63.2 | 1.76 | 1.52 | 3.39 | 4.30 |
| 128 | 26.5 | 47.9 | 32.9 | 98.2 | 131.1 | 1.81 | 1.24 | 3.71 | 4.95 |
| 256 | 54.6 | 107.6 | 65.8 | 215.6 | 289.0 | 1.97 | 1.21 | 3.95 | 5.29 |
| 512 | 109.4 | 220.2 | 140.3 | 457.8 | 662.6 | 2.01 | 1.28 | 4.18 | 6.06 |
| 1024 | 239.1 | 500.3 | 347.5 | 972.6 | 1413 | 2.09 | 1.45 | 4.07 | 5.91 |
| 2048 | 500.5 | 1031 | 722.5 | 2053 | 3122 | 2.06 | 1.44 | 4.10 | 6.24 |
| 4096 | 1266 | 2372 | 1633 | 4647 | 6587 | 1.87 | 1.29 | 3.67 | 5.20 |
| 8192 | 3296 | 5225 | 5706 | 12848 | 14319 | 1.59 | 1.73 | 3.90 | 4.34 |
| 16384 | 6908 | 11943 | 14400 | 36581 | 29849 | 1.73 | 2.08 | 5.30 | 4.32 |
| 32768 | 14997 | 25074 | 31933 | 79000 | 65056 | 1.67 | 2.13 | 5.27 | 4.34 |
| 65536 | 36429 | 58458 | 85060 | 174360 | 140516 | 1.60 | 2.33 | 4.79 | 3.86 |
| 131072 | 86109 | 124020 | 187147 | 424714 | 317544 | 1.44 | 2.17 | 4.93 | 3.69 |
| 262144 | 168008 | 257220 | 399285 | 845551 | 621079 | 1.53 | 2.38 | 5.03 | 3.70 |
| 524288 | 355883 | 539617 | 2110219 | 1880318 | 1326076 | 1.52 | 5.93 | 5.28 | 3.73 |
| 1048576 | 930256 | 1500752 | 4218048 | 4762640 | 2816822 | 1.61 | 4.53 | 5.12 | 3.03 |
<!-- end -->

</details>

### AMD Ryzen 9 9950X (Zen 5), AVX-512

<picture>
  <source media="(prefers-color-scheme: dark)" srcset="plots/9950x-dark.svg">
  <img src="plots/9950x.svg" alt="Line chart of the speed of OneFFT with its AVX-512 and with its AVX2 kernels, FFTW, FFmpeg and PFFFT over the transform size on an AMD Ryzen 9 9950X; the numbers are in the tables below." width="760">
</picture>

The desktop processor with the cores of the EPYC 9255, in a server. The times are the minimum of
two runs, which agree within 6%. Every library is about 1.27 times as fast as on the EPYC 9255,
the ratio of the clocks (5.46 against 4.3 GHz), except OneFFT's AVX-512 kernels from 256 points
up: 1.18 to 1.25 times. With them the core runs at 5.29 GHz instead of 5.46. Against FFTW and
against its own AVX2 kernels OneFFT is ahead by as much as on the EPYC 9255.

<details>
<summary>Times in nanoseconds</summary>

<!-- results:9950x -->
| n | OneFFT | OneFFT (AVX2) | FFTW | FFmpeg | PFFFT | vs OneFFT (AVX2) | vs FFTW | vs FFmpeg | vs PFFFT |
|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|
| 2 | 0.9 | 0.9 | 3.3 | 1.3 | - | 1.00 | 3.67 | 1.44 | - |
| 4 | 1.8 | 1.8 | 4.6 | 1.6 | - | 1.00 | 2.56 | 0.89 | - |
| 8 | 3.8 | 3.8 | 6.4 | 2.3 | - | 1.00 | 1.68 | 0.61 | - |
| 16 | 3.6 | 3.7 | 11.7 | 5.3 | 13.9 | 1.03 | 3.25 | 1.47 | 3.86 |
| 32 | 5.3 | 7.9 | 13.5 | 11.1 | 21.5 | 1.49 | 2.55 | 2.09 | 4.06 |
| 64 | 11.5 | 20.5 | 18.1 | 38.9 | 48.5 | 1.78 | 1.57 | 3.38 | 4.22 |
| 128 | 20.8 | 37.8 | 26.4 | 77.2 | 104.4 | 1.82 | 1.27 | 3.71 | 5.02 |
| 256 | 44.6 | 85.9 | 52.8 | 169.7 | 232.9 | 1.93 | 1.18 | 3.80 | 5.22 |
| 512 | 88.9 | 175.7 | 115.6 | 359.8 | 522.4 | 1.98 | 1.30 | 4.05 | 5.88 |
| 1024 | 196.0 | 399.5 | 270.7 | 769.0 | 1103 | 2.04 | 1.38 | 3.92 | 5.63 |
| 2048 | 415.0 | 827.8 | 599.4 | 1614 | 2475 | 1.99 | 1.44 | 3.89 | 5.96 |
| 4096 | 1050 | 1907 | 1356 | 3683 | 5231 | 1.82 | 1.29 | 3.51 | 4.98 |
| 8192 | 2796 | 4222 | 4747 | 10244 | 11362 | 1.51 | 1.70 | 3.66 | 4.06 |
| 16384 | 5786 | 9612 | 11757 | 28764 | 23583 | 1.66 | 2.03 | 4.97 | 4.08 |
| 32768 | 12439 | 20145 | 26225 | 62066 | 51579 | 1.62 | 2.11 | 4.99 | 4.15 |
| 65536 | 30052 | 46907 | 62019 | 135758 | 110294 | 1.56 | 2.06 | 4.52 | 3.67 |
| 131072 | 68612 | 99486 | 150506 | 337811 | 247502 | 1.45 | 2.19 | 4.92 | 3.61 |
| 262144 | 139285 | 206484 | 344178 | 668908 | 490949 | 1.48 | 2.47 | 4.80 | 3.52 |
| 524288 | 292175 | 435341 | 1859221 | 1480264 | 1055051 | 1.49 | 6.36 | 5.07 | 3.61 |
| 1048576 | 847900 | 1290282 | 3735304 | 3791519 | 2348670 | 1.52 | 4.41 | 4.47 | 2.77 |
<!-- end -->

</details>

### AMD EPYC 9254 (Zen 4), AVX-512

<picture>
  <source media="(prefers-color-scheme: dark)" srcset="plots/zen4-dark.svg">
  <img src="plots/zen4.svg" alt="Line chart of the speed of OneFFT with its AVX-512 and with its AVX2 kernels, FFTW, FFmpeg and PFFFT over the transform size on an AMD EPYC 9254; the numbers are in the tables below." width="760">
</picture>

The times are the minimum of two runs. From 32 points up they agree within 7%, except for FFTW
at 256 and 512 points (10 and 19%) and PFFFT at 16384 points (one run 1.6 times slower).
Below 32 points a time depends more on the process than on the run: started 20 times, FFmpeg
takes 1.4 or 2.2 ns for 2 points and OneFFT 2.4, 3.1 or 3.9 ns for 4 points. Two more runs
give the same times within 4% from 32 points up; the first measurement, on another server of
this model and before the warm-up, within 6% from 4 points up.

<details>
<summary>Times in nanoseconds</summary>

<!-- results:zen4 -->
| n | OneFFT | OneFFT (AVX2) | FFTW | FFmpeg | PFFFT | vs OneFFT (AVX2) | vs FFTW | vs FFmpeg | vs PFFFT |
|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|
| 2 | 1.7 | 1.7 | 4.3 | 1.4 | - | 1.00 | 2.53 | 0.82 | - |
| 4 | 3.1 | 3.1 | 5.3 | 1.9 | - | 1.00 | 1.71 | 0.61 | - |
| 8 | 4.6 | 4.6 | 9.2 | 3.9 | - | 1.00 | 2.00 | 0.85 | - |
| 16 | 5.1 | 5.2 | 17.1 | 8.2 | 21.3 | 1.02 | 3.35 | 1.61 | 4.18 |
| 32 | 10.8 | 12.2 | 19.6 | 16.8 | 33.4 | 1.13 | 1.81 | 1.56 | 3.09 |
| 64 | 25.7 | 30.4 | 28.6 | 41.1 | 73.7 | 1.18 | 1.11 | 1.60 | 2.87 |
| 128 | 46.3 | 53.8 | 50.9 | 84.9 | 155.5 | 1.16 | 1.10 | 1.83 | 3.36 |
| 256 | 95.9 | 116.3 | 99.4 | 188.1 | 314.4 | 1.21 | 1.04 | 1.96 | 3.28 |
| 512 | 209.8 | 242.1 | 210.1 | 409.2 | 692.4 | 1.15 | 1.00 | 1.95 | 3.30 |
| 1024 | 452.3 | 547.4 | 475.5 | 885.2 | 1444 | 1.21 | 1.05 | 1.96 | 3.19 |
| 2048 | 1006 | 1171 | 1138 | 1986 | 3378 | 1.16 | 1.13 | 1.97 | 3.36 |
| 4096 | 2342 | 2939 | 2800 | 5280 | 6854 | 1.25 | 1.20 | 2.25 | 2.93 |
| 8192 | 5464 | 6092 | 8863 | 17732 | 14805 | 1.12 | 1.62 | 3.25 | 2.71 |
| 16384 | 11638 | 13214 | 19539 | 45768 | 30755 | 1.14 | 1.68 | 3.93 | 2.64 |
| 32768 | 25937 | 29048 | 42120 | 102331 | 69201 | 1.12 | 1.62 | 3.95 | 2.67 |
| 65536 | 55187 | 67657 | 102831 | 223877 | 147343 | 1.23 | 1.86 | 4.06 | 2.67 |
| 131072 | 120770 | 142344 | 258757 | 515936 | 336653 | 1.18 | 2.14 | 4.27 | 2.79 |
| 262144 | 263913 | 310537 | 551899 | 1086459 | 682231 | 1.18 | 2.09 | 4.12 | 2.59 |
| 524288 | 554650 | 649643 | 2788124 | 2421998 | 1456012 | 1.17 | 5.03 | 4.37 | 2.63 |
| 1048576 | 1319667 | 1613787 | 5797148 | 5702257 | 3217524 | 1.22 | 4.39 | 4.32 | 2.44 |
<!-- end -->

</details>

### Intel Xeon Gold 6230R (Cascade Lake), AVX-512

<picture>
  <source media="(prefers-color-scheme: dark)" srcset="plots/clx-dark.svg">
  <img src="plots/clx.svg" alt="Line chart of the speed of OneFFT with its AVX-512 and with its AVX2 kernels, FFTW, FFmpeg and PFFFT over the transform size on an Intel Xeon Gold 6230R; the numbers are in the tables below." width="760">
</picture>

At 2.1 GHz, without turbo (see Setup). The times are the minimum of two runs of the benchmark:
on this machine a whole run is sometimes 5 to 15% slower than another at the small sizes.

<details>
<summary>Times in nanoseconds</summary>

<!-- results:clx -->
| n | OneFFT | OneFFT (AVX2) | FFTW | FFmpeg | PFFFT | vs OneFFT (AVX2) | vs FFTW | vs FFmpeg | vs PFFFT |
|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|
| 2 | 4.3 | 4.3 | 13.0 | 3.3 | - | 1.00 | 3.02 | 0.77 | - |
| 4 | 6.2 | 6.2 | 14.8 | 4.8 | - | 1.00 | 2.39 | 0.77 | - |
| 8 | 15.8 | 15.9 | 25.3 | 12.7 | - | 1.01 | 1.60 | 0.80 | - |
| 16 | 16.1 | 16.1 | 52.4 | 25.9 | 73.3 | 1.00 | 3.25 | 1.61 | 4.55 |
| 32 | 24.8 | 32.0 | 46.0 | 50.9 | 101.5 | 1.29 | 1.85 | 2.05 | 4.09 |
| 64 | 46.1 | 85.8 | 65.7 | 114.0 | 231.1 | 1.86 | 1.43 | 2.47 | 5.01 |
| 128 | 76.1 | 150.9 | 114.5 | 240.4 | 470.8 | 1.98 | 1.50 | 3.16 | 6.19 |
| 256 | 182.9 | 314.9 | 218.7 | 562.3 | 960.7 | 1.72 | 1.20 | 3.07 | 5.25 |
| 512 | 362.8 | 710.3 | 467.2 | 1215 | 2084 | 1.96 | 1.29 | 3.35 | 5.75 |
| 1024 | 855.3 | 1530 | 1086 | 2654 | 4267 | 1.79 | 1.27 | 3.10 | 4.99 |
| 2048 | 1951 | 3556 | 2621 | 6081 | 10227 | 1.82 | 1.34 | 3.12 | 5.24 |
| 4096 | 4530 | 8335 | 6290 | 14358 | 23336 | 1.84 | 1.39 | 3.17 | 5.15 |
| 8192 | 11531 | 19161 | 19086 | 33116 | 51648 | 1.66 | 1.66 | 2.87 | 4.48 |
| 16384 | 24431 | 40339 | 41490 | 75416 | 104778 | 1.65 | 1.70 | 3.09 | 4.29 |
| 32768 | 51405 | 87156 | 91970 | 158966 | 234557 | 1.70 | 1.79 | 3.09 | 4.56 |
| 65536 | 134502 | 200795 | 217772 | 364579 | 510007 | 1.49 | 1.62 | 2.71 | 3.79 |
| 131072 | 355474 | 482570 | 615795 | 1025818 | 1259966 | 1.36 | 1.73 | 2.89 | 3.54 |
| 262144 | 806249 | 1064157 | 1391384 | 2254765 | 2587401 | 1.32 | 1.73 | 2.80 | 3.21 |
| 524288 | 1678925 | 2244860 | 7821413 | 4680275 | 5664419 | 1.34 | 4.66 | 2.79 | 3.37 |
| 1048576 | 3722387 | 5085460 | 16320123 | 11973013 | 11764916 | 1.37 | 4.38 | 3.22 | 3.16 |
<!-- end -->

</details>

### Intel Core i5-9600K (Skylake), AVX2

<picture>
  <source media="(prefers-color-scheme: dark)" srcset="plots/intel-dark.svg">
  <img src="plots/intel.svg" alt="Line chart of the speed of OneFFT, FFTW, FFmpeg and PFFFT over the transform size on an Intel i5-9600K; the numbers are in the tables below." width="760">
</picture>

<details>
<summary>Times in nanoseconds</summary>

<!-- results:intel -->
| n | OneFFT | FFTW | FFmpeg | PFFFT | vs FFTW | vs FFmpeg | vs PFFFT |
|--:|--:|--:|--:|--:|--:|--:|--:|
| 2 | 2.0 | 5.9 | 1.5 | - | 2.95 | 0.75 | - |
| 4 | 2.8 | 6.9 | 2.2 | - | 2.46 | 0.79 | - |
| 8 | 7.0 | 11.6 | 5.7 | - | 1.66 | 0.81 | - |
| 16 | 7.4 | 24.6 | 11.6 | 31.6 | 3.32 | 1.57 | 4.27 |
| 32 | 14.0 | 23.4 | 22.2 | 46.4 | 1.67 | 1.59 | 3.31 |
| 64 | 36.1 | 35.4 | 51.8 | 105.3 | 0.98 | 1.43 | 2.92 |
| 128 | 67.4 | 64.6 | 105.2 | 219.6 | 0.96 | 1.56 | 3.26 |
| 256 | 142.4 | 136.6 | 237.5 | 425.1 | 0.96 | 1.67 | 2.99 |
| 512 | 317.5 | 312.4 | 508.0 | 920.2 | 0.98 | 1.60 | 2.90 |
| 1024 | 684.5 | 648.1 | 1165 | 1871 | 0.95 | 1.70 | 2.73 |
| 2048 | 1576 | 1532 | 2560 | 4482 | 0.97 | 1.62 | 2.84 |
| 4096 | 3803 | 3601 | 6171 | 10690 | 0.95 | 1.62 | 2.81 |
| 8192 | 8728 | 11102 | 14892 | 24163 | 1.27 | 1.71 | 2.77 |
| 16384 | 19374 | 26721 | 35534 | 53057 | 1.38 | 1.83 | 2.74 |
| 32768 | 45276 | 69364 | 84821 | 129869 | 1.53 | 1.87 | 2.87 |
| 65536 | 96833 | 171633 | 191130 | 252980 | 1.77 | 1.97 | 2.61 |
| 131072 | 209243 | 385388 | 401638 | 625372 | 1.84 | 1.92 | 2.99 |
| 262144 | 459237 | 732377 | 841954 | 1157209 | 1.59 | 1.83 | 2.52 |
| 524288 | 1134748 | 3718173 | 2246893 | 3082268 | 3.28 | 1.98 | 2.72 |
| 1048576 | 3007179 | 8445645 | 8539022 | 9283227 | 2.81 | 2.84 | 3.09 |
<!-- end -->

</details>

### AMD EPYC 7443P (Zen 3), AVX2

<picture>
  <source media="(prefers-color-scheme: dark)" srcset="plots/epyc-dark.svg">
  <img src="plots/epyc.svg" alt="Line chart of the speed of OneFFT, FFTW, FFmpeg and PFFFT over the transform size on an AMD EPYC 7443P; the numbers are in the tables below." width="760">
</picture>

At 2.85 GHz: boost is off on this server. The times are the minimum of two runs, which agree
within 4% at most sizes.
The first measurement was on another server of this model, before the first pass learned to
write in order (see "Decisions and their effect"). Measured directly on this one, writing in
order makes 2^13 to 2^19 points 2 to 9% faster than reading in order.

<details>
<summary>Times in nanoseconds</summary>

<!-- results:epyc -->
| n | OneFFT | FFTW | FFmpeg | PFFFT | vs FFTW | vs FFmpeg | vs PFFFT |
|--:|--:|--:|--:|--:|--:|--:|--:|
| 2 | 2.5 | 7.8 | 2.1 | - | 3.12 | 0.84 | - |
| 4 | 4.6 | 8.8 | 3.9 | - | 1.91 | 0.85 | - |
| 8 | 7.6 | 14.8 | 7.0 | - | 1.95 | 0.92 | - |
| 16 | 8.8 | 27.9 | 14.8 | 33.2 | 3.17 | 1.68 | 3.77 |
| 32 | 19.3 | 30.0 | 30.0 | 51.8 | 1.55 | 1.55 | 2.68 |
| 64 | 46.4 | 47.2 | 73.8 | 112.9 | 1.02 | 1.59 | 2.43 |
| 128 | 85.8 | 87.1 | 142.8 | 239.0 | 1.02 | 1.66 | 2.79 |
| 256 | 180.6 | 171.3 | 319.2 | 478.2 | 0.95 | 1.77 | 2.65 |
| 512 | 392.1 | 362.4 | 686.8 | 1067 | 0.92 | 1.75 | 2.72 |
| 1024 | 848.3 | 847.9 | 1495 | 2180 | 1.00 | 1.76 | 2.57 |
| 2048 | 1872 | 2059 | 3323 | 5047 | 1.10 | 1.78 | 2.70 |
| 4096 | 4538 | 4906 | 8776 | 10615 | 1.08 | 1.93 | 2.34 |
| 8192 | 9885 | 15339 | 28122 | 23125 | 1.55 | 2.84 | 2.34 |
| 16384 | 21471 | 36019 | 72869 | 48572 | 1.68 | 3.39 | 2.26 |
| 32768 | 49378 | 89392 | 168966 | 110065 | 1.81 | 3.42 | 2.23 |
| 65536 | 108768 | 202012 | 361558 | 240494 | 1.86 | 3.32 | 2.21 |
| 131072 | 230823 | 454496 | 754874 | 533096 | 1.97 | 3.27 | 2.31 |
| 262144 | 499180 | 1030925 | 1627014 | 1121700 | 2.07 | 3.26 | 2.25 |
| 524288 | 1031362 | 4541193 | 3587519 | 2373702 | 4.40 | 3.48 | 2.30 |
| 1048576 | 2387352 | 9662379 | 8850475 | 5053905 | 4.05 | 3.71 | 2.12 |
<!-- end -->

</details>

### AMD Ryzen 7 5700U (Zen 2), AVX2

<picture>
  <source media="(prefers-color-scheme: dark)" srcset="plots/zen2-dark.svg">
  <img src="plots/zen2.svg" alt="Line chart of the speed of OneFFT, FFTW, FFmpeg and PFFFT over the transform size on an AMD Ryzen 7 5700U; the numbers are in the tables below." width="760">
</picture>

<details>
<summary>Times in nanoseconds</summary>

<!-- results:zen2 -->
| n | OneFFT | FFTW | FFmpeg | PFFFT | vs FFTW | vs FFmpeg | vs PFFFT |
|--:|--:|--:|--:|--:|--:|--:|--:|
| 2 | 1.8 | 6.5 | 1.9 | - | 3.61 | 1.06 | - |
| 4 | 3.6 | 7.7 | 2.8 | - | 2.14 | 0.78 | - |
| 8 | 6.7 | 12.5 | 6.3 | - | 1.87 | 0.94 | - |
| 16 | 8.0 | 24.8 | 11.6 | 27.2 | 3.10 | 1.45 | 3.40 |
| 32 | 15.3 | 22.7 | 23.3 | 43.0 | 1.48 | 1.52 | 2.81 |
| 64 | 39.8 | 37.0 | 62.0 | 93.8 | 0.93 | 1.56 | 2.36 |
| 128 | 74.0 | 70.9 | 120.1 | 198.8 | 0.96 | 1.62 | 2.69 |
| 256 | 166.9 | 140.7 | 270.3 | 399.2 | 0.84 | 1.62 | 2.39 |
| 512 | 334.7 | 315.9 | 583.0 | 900.4 | 0.94 | 1.74 | 2.69 |
| 1024 | 748.8 | 678.9 | 1264 | 1868 | 0.91 | 1.69 | 2.49 |
| 2048 | 1572 | 1608 | 2774 | 4372 | 1.02 | 1.76 | 2.78 |
| 4096 | 3679 | 3806 | 6471 | 9017 | 1.03 | 1.76 | 2.45 |
| 8192 | 7989 | 10956 | 19130 | 19349 | 1.37 | 2.39 | 2.42 |
| 16384 | 17707 | 26070 | 56750 | 40335 | 1.47 | 3.21 | 2.28 |
| 32768 | 38124 | 66908 | 142214 | 91088 | 1.76 | 3.73 | 2.39 |
| 65536 | 83164 | 154300 | 320574 | 192395 | 1.86 | 3.85 | 2.31 |
| 131072 | 208260 | 322824 | 679608 | 438005 | 1.55 | 3.26 | 2.10 |
| 262144 | 600170 | 1039278 | 1952800 | 1596096 | 1.73 | 3.25 | 2.66 |
| 524288 | 2188086 | 4853247 | 6678176 | 6663358 | 2.22 | 3.05 | 3.05 |
| 1048576 | 5652721 | 10737982 | 14803870 | 14071404 | 1.90 | 2.62 | 2.49 |
<!-- end -->

</details>

### SSE2

The SSE2 kernels on the i5-9600K (built with `-DONEFFT_NO_AVX2`) against PFFFT, which is also
SSE. They were not tuned.

<!-- results:intel_sse2 -->
| n | OneFFT | PFFFT | vs PFFFT |
|--:|--:|--:|--:|
| 8 | 7.0 | - | - |
| 16 | 13.0 | 31.8 | 2.45 |
| 32 | 40.6 | 45.9 | 1.13 |
| 64 | 80.2 | 105.1 | 1.31 |
| 128 | 191.1 | 213.3 | 1.12 |
| 256 | 385.2 | 416.3 | 1.08 |
| 512 | 935.5 | 921.8 | 0.99 |
| 1024 | 1865 | 1870 | 1.00 |
| 2048 | 4395 | 4478 | 1.02 |
| 4096 | 9716 | 10680 | 1.10 |
| 8192 | 22290 | 23980 | 1.08 |
| 16384 | 46730 | 53413 | 1.14 |
| 32768 | 107002 | 121941 | 1.14 |
| 65536 | 215848 | 250732 | 1.16 |
<!-- end -->

### Summary

* Against FFmpeg, OneFFT is 1.25 to 5.3 times faster from 16 points up, on every machine.
  Against PFFFT 1.2 to 6.2 times. At 4 and 8 points FFmpeg's dedicated kernels are up to 1.7
  times faster than OneFFT's.
* Against patient FFTW there are three ranges. Up to 32 points OneFFT's single kernels are 1.3 to
  3.7 times faster. From 8192 points up OneFFT is 1.2 to 6.4 times faster: its passes stay cache
  friendly, and FFTW's measured (not patient) plans are weaker. Between 64 and 4096 points both
  are close with NEON and AVX2, OneFFT being between 16% slower and 21% faster (ahead at every
  size on the Cortex-X4, behind at most on Zen 2); FFTW's patient planner picks large generated
  codelets there that need very few passes.
* With AVX-512, OneFFT is ahead of FFTW (which has AVX-512 codelets) at every size, also between
  64 and 4096 points, except at 512 points on Zen 4, where the two are level (210 ns): by 1.21
  to 1.52 times there on Zen 5 (1.18 to 1.57 on the Ryzen 9950X), by 1.2 to 1.5 times on
  Cascade Lake and by up to 1.2 times on Zen 4. Against its own AVX2 kernels on the same
  machine it is 1.44 to 2.1 times faster on Zen 5 from 32 points up, 1.3 to 2.0 times on
  Cascade Lake (1.7 to 2.0 times from 64 to 4096 points, 1.3 to 1.7 times above), and 1.12 to
  1.25 times on Zen 4 from 64 points up; a 512 bit operation is two 256 bit operations on Zen 4.
* The order in which the first pass goes through its butterflies decides large transforms on
  some cores (see "Decisions and their effect"). The M2 used to be the exception at the top,
  with FFTW up to 16% faster between 2^15 and 2^18 points; with the first pass writing in
  order OneFFT is 1.38 to 1.46 times faster than FFTW there.
* Apple's vDSP is the fastest library on the M2: 1.1 to 1.7 times faster than OneFFT from 128
  to 524288 points, and faster than FFTW (at 2^20 points OneFFT is ahead). It is the only other
  library here that is written for one specific family of cores, and the only one that does
  not have to split and interleave.
* The 2021 proof of concept (commit `4587999`, since removed) takes 6.9 us for 1024 points on
  the Pixel 3a, 6.6 us on the A50, 2.1 us on the S24 and 1.5 us on the M2: 1.04 to 1.9 times the
  time of OneFFT.

### The dip at 64 points

Several curves dip at 64 points: OneFFT with AVX2 (on Skylake and Zen 2, and its AVX2 kernels
on Zen 4, Zen 5 and Cascade Lake) and with NEON on the Cortex-X4, and FFmpeg on the Zen cores.
This is not the effect of where the buffers are (see Setup), which showed on Zen 4 (in FFTW
at 64 points and in OneFFT at 128) and by a few percent on Cascade Lake:

* The time for 64 points does not depend on the position of the output buffer in its page.
  With the buffer at 0, 64, 512, 1024, 2048, 3072 and 4032 bytes OneFFT takes 25.5 to 26.3 ns
  with AVX2 on Zen 5, 41.3 to 42.0 ns on Zen 2, 44.7 to 45.5 ns on the Cortex-X4 and 34.9 to
  36.4 ns on the M2, and FFmpeg 49.5 to 49.9 ns on Zen 5. Skylake and Cascade Lake vary more
  from run to run than with the position (36 to 40 and 46 to 48 ns).
* The phones and Skylake were first measured with buffers wherever the allocator put them.
  The tables above are from new runs with fixed buffers and differ from the first ones by a
  few percent, at 64 points by at most 4% (Skylake 9%).

It is the step from a kernel to passes. Up to 32 points a transform is one kernel that keeps
everything in registers. 64 points is the smallest size done in passes, and the size that gets
the least out of each: with AVX2 three kernels (`f4`, `la2`, `lb`) for 6 index bits, while 128
and 256 points transform 7 and 8 bits with three. The kernels themselves are as fast as at
other sizes (`bench/passes.c` on the Cortex-X4: `f8` and `l8` take 22 and 46 cycles per
butterfly at 64 points, 22 and 43 at 256), but splitting the input, interleaving the output
and the kernel calls cost the same for fewer bits. In the unit of the charts OneFFT with AVX2
is 20 to 25% below the level of 128 points on the x86 cores; with NEON, where 64 points are
two kernels, 7% on the Cortex-X4, 6% on the M2 and not at all on the older cores, whose curves
only stand out at 32 points.

With AVX-512, 64 and 128 points are single kernels and those curves have no dip at 64. The same
step is at 256 points there, the first size in passes (Cascade Lake: 5% below 128 points). A
64 point kernel for AVX2 was not built: 8 complex vectors fill the 16 registers, so that the
three lane exchanges would need memory for their intermediate results, which is most of what
the passes cost. FFTW's 64 point codelet is as fast as the three passes on Skylake (35 against
36 ns) and 8% faster on Zen 2 (37 against 40 ns).

FFmpeg's dip on Zen 5 (50 ns for 64 points, 14 ns for 32 on the EPYC 9255, 39 and 11 ns on the
Ryzen 9950X; Zen 4 takes 41 ns) is in its own code: the processor executes 2.0 instructions per
cycle there, against 3.8 at 32 points.

## What the cores can do

`bench/uops_aarch64.c` and `bench/uops_x86.c` run loops of independent instructions of one kind.
The numbers explain most of what the pass timings (`bench/passes.c`) show.

### AArch64: cycles per instruction

| | Kryo 360 | Cortex-A73 | Cortex-X4 | Apple M2 |
|---|--:|--:|--:|--:|
| `fadd`, `fmul`, `fmla`, 4 lanes | 0.9 | 0.9 | 0.25 | 0.25 |
| the same with 2 lanes or scalar | 0.37 | 0.47 | 0.25 | 0.25 |
| `trn`, `zip`, `uzp` | 0.55 | 0.6 | 0.25 | 0.25 |
| `mov` between vector registers | 0.55 | 0.6 | 0.25 | 0.1 (free) |
| `ldr q` | 0.9 | 1.0 | 0.27 | 0.29 |
| `ldp q, q` | 2.2 | 2.0 | 0.60 | 0.65 |
| `ld2` (4 lanes) | 2.2 | 2.6 | 0.93 | 0.94 |
| `str q` | 0.9 | 1.9 | 0.44 | 0.44 |
| `stp q, q` | 1.9 | 4.0 | 0.93 | 0.94 |
| `st2` (4 lanes) | 1.9 | 4.2 | 0.93 | 0.94 |
| `ld2` of one lane | 0.9 | 0.9 | 0.93 | 0.45 |

What follows:

* **The older cores execute one 128 bit floating point operation per cycle**, half the rate of
  their 64 bit operations: their SIMD units are 64 bits wide. A load or store of a register
  costs about as much as an arithmetic operation. On these cores the time of a kernel is close
  to the number of 128 bit operations it executes, counting loads and stores. A 1024 point
  transform is about 8200 arithmetic operations and 1000 shuffles in OneFFT; the two codelets of
  FFTW's plan (two radix 32 passes) come to about 7800 and 2000, counted in their disassembly.
  That is why the two are equal there, and why reordering the same arithmetic does not help.
* **The current cores execute four operations per cycle**, and OneFFT's passes reach 3 to 3.4.
  Loads are nearly free, stores are limited to two registers per cycle.
* **Loading one lane costs as much as loading a whole register** everywhere but on Apple. The
  proof of concept gathers its 32 values lane by lane; that is 4 times the load and store
  instructions per pass and the reason for the different structure of OneFFT.
* `ld2` and `st2` are no bargain: they run at one per cycle on the fast cores. `ldp` with two
  `uzp` is equally fast or faster.
* A `mov` is free only on Apple. This decides whether butterflies made from fused multiply-adds
  (6 operations and 3 copies instead of 8 operations) pay off. Measured on the M2: 876 to
  798 ns at 1024 points.
* On the Cortex-X4, registers that hold subnormal numbers or NaNs make arithmetic 2.3 times
  slower. Benchmarks must not run on uninitialised registers or on data that has decayed to
  subnormals.

### x86-64: 256 bit instructions per cycle

| | Skylake, Cascade Lake | Zen 2 | Zen 3 |
|---|--:|--:|--:|
| loads | 2 | 2 | 2 |
| stores | 1 | 1 | 1 |
| additions | 2 | 2 | 2 |
| multiplications, fused multiply-adds | 2 | 2 | 2 |
| additions and multiplications together | 2 | 4 | 4 |
| `unpck`, `shufps` | 1 | 2 | 2 |
| `vperm2f128`, `vinsertf128` between registers | 1 | 1 | 1 |

What follows:

* **One store per cycle** on all three. A radix 4 pass over 32 values is 8 stores, as long as
  its arithmetic takes on a good day. Fewer passes matter more than on AArch64, hence the two
  radix 8 kernels that work in place (design.md).
* **Skylake** shares two units between additions and multiplications and has one for shuffles.
  Its pipeline is 4 instructions wide, and the kernels run close to that limit as well: the
  instruction count matters, and memory operands (a load fused with the operation) are worth
  using everywhere.
* **Zen** has separate units for additions and multiplications. Standard butterflies use both
  evenly. The in-place kernels lean on fused multiply-adds and are limited by those units; the
  radix 8 pass before the last gained 6 to 10% at even sizes on Skylake and nothing on Zen 3.
  On Zen 2, stores additionally occupy one of the two addition units.
* Moving the 128 bit halves of registers around is slow everywhere. `lb` does it while loading.

### x86-64: 512 bit instructions per cycle

| | Zen 4 | Zen 5 | Cascade Lake |
|---|--:|--:|--:|
| loads | 1 | 2 | 2 |
| stores | 0.5 | 1 | 1 |
| additions | 1 | 2 | 2 |
| multiplications, fused multiply-adds | 1 | 2 | 2 |
| additions and multiplications together | 2 | 4 | 2 |
| `unpck`, `shufps` | 1.5 | 4 | 1 |
| `vshuff32x4` | 1 | 2 | 1 |
| shuffles and additions together | - | 4 | 2 |
| cycles for a load, two multiplications by a twiddle vector and a store: twiddle as memory operands / loaded into a register | 3.2 / 2.2 | 1.5 / 1.0 | 1.9 / 1.4 |

(`bench/uops_x86.c` built with `-DWIDE -mavx512f`. The buffer it reads and the one it writes
must not have the same offset in their pages: a load then waits for the stores before it, and
the tests with loads and stores came out up to 1.8 times too slow on Cascade Lake.) What
follows:

* **Zen 4 executes a 512 bit operation as two of 256 bits.** It moves no more data per cycle
  than with AVX2, and stores one register every other cycle. What AVX-512 gives it is 32
  registers: higher radix, so fewer passes and fewer stores, and twiddles in registers.
* **Zen 5 has the full width**, twice the arithmetic of its 256 bit operations per cycle. It
  does not store more data per cycle than with AVX2 either (one 512 bit register or two of 256
  bits), so stores weigh twice as much relative to arithmetic.
* **A memory operand is a load**, an embedded broadcast of a scalar as well, and loads are
  scarce: the last row of the table. The kernels as first written (scalar twiddles as embedded
  broadcasts, twiddle vectors as memory operands, as in the AVX2 kernels that have no registers
  to spare) were limited by loads. A radix 4 middle pass has 8 loads of data and used 12 for its
  twiddles; with the twiddles broadcast into registers it has 6.
* **Additions and multiplications run on separate units**, two each. A butterfly with a twiddle
  is four of each. A butterfly without is four additions, or two additions and two fused
  multiply-adds as `(a + b, (a + b) - 2 b)`: the first passes are built from those. The radix 8
  first pass went from 66 to 44 cycles per butterfly on Zen 4. On Zen 5 it only paid off
  together with the constant in a register (34 to 27 cycles; with the constant as an embedded
  broadcast the 24 additional loads ate the gain).
* **The kernels reach 2.7 to 3.2 of the 4 operations per cycle on Zen 5.** The processor's
  counters say why: in the last passes the floating point scheduler is full during 35 to 40% of
  the cycles (20% in the first pass, never in the radix 4 middle pass), and no further
  instructions are accepted then. A butterfly is dispatched as a whole and its operations wait
  for each other. Knocking parts out of the last pass `lb` shows no single culprit: it takes
  19.2 cycles per butterfly, 13.3 without its twiddle multiplications, 12.9 without its
  butterflies, 13.5 without its exchanges, and 8.1 with loads and stores alone. Overlapping
  consecutive butterflies in the instruction stream (software pipelining) helps: 16.1 cycles.
* **Cascade Lake has two units for everything.** All 512 bit additions, multiplications and
  fused multiply-adds share them, and one of the two is also the only one that shuffles. Loads
  and stores have units of their own. A kernel cannot take fewer cycles there than half its
  arithmetic and shuffle operations, and the kernels reach 1.3 to 1.8 operations per cycle
  ("Where the time goes"). What was done for the Zen cores is neither needed nor harmful:
  with twiddles as memory operands instead of in registers, whole transforms are 0 to 3%
  slower; the order of the instructions within a butterfly matters by 5 to 8% (the generator's
  scheduling against none), but not for how many operations per cycle it schedules. The
  kernels are therefore the same for all processors.
* **Stores that miss the cache are handled one at a time, loads in parallel.** The radix 8
  first pass takes 27 cycles per butterfly on Zen 5 while everything is in the first level
  cache. At 2^17 points it takes 285 if it reads in order and writes its 16 cache lines to
  scattered places, and 100 if it writes in order and reads scattered. Zen 4: 44 in the cache,
  then 147 and 123.

## Decisions and their effect

Times in ns for 1024 points unless stated.

| change | effect |
|---|---|
| OneFFT instead of the proof of concept (AArch64) | Pixel 3a 6860 to 5850, A50 6640 to 6410, S24 2060 to 1090, M2 1460 to 790 |
| Instruction scheduling and 2x unrolling in the generator (AArch64) | Pixel 3a 6380 to 5650, A50 7110 to 6040, S24 1030 to 1010, M2 870 to 815 |
| Fused multiply-add form of the passes | M2 876 to 798; not used elsewhere |
| Cache blocking of the middle passes, 2^21 points | Pixel 3a 52.5 ms to 42.8 ms, A50 56.5 to 47.2, S24 unchanged |
| Region loops inside the kernels instead of one call per region (AVX2) | Skylake 834 to 717, Zen 3 906 to 841; 256 points on Skylake 166 to 152 |
| Radix 8 first pass (AVX2, odd number of index bits), 512 points | Skylake 369 to 319, Zen 3 426 to 385 |
| Radix 8 pass before the last (AVX2, even number of index bits) | Skylake 745 to 680, Zen 3 unchanged, Zen 2 767 to 749 |

AVX-512, measured when the kernels first ran on hardware. Zen 5 / Zen 4, in ns:

| change | effect |
|---|---|
| Twiddles and constants in registers instead of memory operands | 1024 points: 285 to 262 / unchanged |
| Radix 16 first pass (up to 4096 points) | 1024 points: 258 to 231 / 474 to 453; 4096 points: 1405 to 1318 / 2720 to 2325 |
| Software pipelined last pass | 19.2 to 16.1 / 36 to 31 cycles per butterfly, 4% of a transform |
| 256 points in AVX-512 passes instead of the AVX2 plan | 107 to 54 / 117 to 96 |
| Single kernels for 32, 64, 128 points | 64 points: 25.6 to 14.7 / 30.3 to 25.7; 128 points: 48.5 to 26.3 / 54.4 to 46.6 |
| Radix 8 middle pass | up to 5% from 4096 points up |
| First pass writes in order, 2^14 to 2^20 points (Zen 4: to 2^19) | 2^15 points: 22.9 to 15.6 us / 27.7 to 26.7 us; 2^18 points: 297 to 175 us / 291 to 268 us |
| The same with the AVX2 kernels, 2^13 points up | 2^18 points: 505 to 257 us / 345 to 315 us; Zen 2 at 2^16: 100 to 86 us; Zen 3 at 2^16 and 2^18: 117 to 109 us and 530 to 488 us |
| All of it | 1024 points: 285 to 234 / 502 to 452; 2^18 points: 295 to 164 us / 277 to 262 us |

The same change of the first pass on AArch64, 2^16 and 2^18 points: Apple M2 143 to 87 us and
638 to 387 us (used on Apple platforms while the data fits the second level cache of the
performance cores, 16 MiB on the M2); Cortex-X4 108 to 116 us and 515 to 627 us; Cortex-A73
0.93 to 1.48 ms and 4.8 to 7.8 ms; Kryo 360 0.83 to 1.47 ms and 4.4 to 7.4 ms (not used there).
Intel's Skylake (i5-9600K, AVX2) is 1 to 2% slower writing in order from 2^13 to 2^18 points and
23 to 30% slower at 2^19 and 2^20: it reads in order, as does every x86 processor that is not
AMD's.

The order of the first pass and the sizes with a radix 16 first pass depend on the processor's
caches. Writing in order stops paying off when input and output together exceed a quarter of
the third level cache (Zen 2 with 4 MiB: beyond 2^16 points; Zen 3 and Zen 4 with 32 MiB:
beyond 2^19), on Zen 5 half of it (2^20). Beyond that, reading in order is up to 25% faster.
The plan reads the cache size from the processor (`onefft_x86_tune` in `onefft.c`). The radix
16 first pass loses against radix 8 from 8192 points up (by up to 25%: its 16 input and 16
output streams are a multiple of 4096 bytes apart then) and wins again by 3 to 10% when the
data no longer fits the third level cache.

That is so on the Zen cores. On Cascade Lake the radix 16 first pass is the faster one at every
size: equal at 8192 points, 1 to 6% faster from 2^14 to 2^17 and 5 to 10% from 2^18 to 2^23
(2^18 points: 875 to 792 us), so Intel processors always use it. Writing in order gains nothing
there at any size, with AVX-512 or with AVX2 (AVX2: equal up to 2^19 points, 6 to 9% slower
beyond).

Tried and dropped:

* **First pass reading with a stride** so that consecutive butterflies write next to each other
  instead of to bit reversed positions: 2^21 points on the Pixel 3a took 64 ms instead of 43 ms.
  Dropped for the phones only: it is what AMD's and Apple's cores want for medium sizes, see
  above.
* **Prefetching the scattered output lines of the first pass:** 3% faster on the Pixel 3a at
  2^21 points, 4 to 10% slower on the S24. With AVX-512, prefetching the lines to be written
  (reading in order) or to be read (writing in order) one to eight butterflies ahead: no gain
  on Zen 5, 10 to 30% slower first pass on Zen 4.
* **Embedded broadcasts for scalar twiddles (AVX-512):** the middle pass takes 15.5 instead of
  12.7 cycles per butterfly on Zen 5. On Zen 4 it is the other way round, 19.6 against 22.3, but
  a whole transform takes the same time there either way.
* **Loading the next butterfly's data an iteration ahead** without pipelining anything else: 3%.
* **Splitting the interleaved input with two loads and register shuffles** instead of one load
  and two shuffles with a memory operand (AVX-512): no gain on Zen 5, slower on Zen 4.
* **Kernels of their own for Intel (AVX-512):** twiddles as memory operands, and instruction
  orders for two or four operations per cycle instead of three: nothing gained on Cascade Lake
  (see "What the cores can do").
* **Other pass structures with AVX-512:** radix 4 middle passes before the radix 8 ones instead
  of after, and 4 against 5 index bits for the last two passes where both need the same number
  of passes: within 2%, except at 2048 points, where radix 8 with `la2` is 5% faster than radix 4
  with `la3` on both machines (and is used).
* **`ldr`/`str` instead of `ldp`/`stp`:** `ldp` looks very slow in isolation on the Kryo
  (table above), but in the kernels the pair forms are equal or better on every core.
* **A radix 4 last pass where possible** (it has fewer twiddle multiplications): slower. Per
  index bit the radix 8 last pass is cheaper because the transposition and the interleaving
  are paid once for three bits instead of two.
* **Butterfly differences by fused multiply-add** (`a - b = (a + b) - 2b`) in the standard AVX2
  kernels, to move work from the adders to the multipliers on Zen: no gain on Zen 2.
* **Scheduling and unrolling on x86:** no measurable effect.

## Where the time goes

Cycles per butterfly (one kernel iteration, 32 values) at 1024 points, from `bench/passes.c`:

| | first | middle | last |
|---|--:|--:|--:|
| Kryo 360, radix 8 / 4 / 8 | 75 | 40 per radix 4 | 150 |
| Cortex-X4, radix 8 / 4 / 8 | 22 | 10 per radix 4 | 47 |
| Apple M2 (fused), radix 8 / 4 / 8 | 19 | 8 per radix 4 | 35 |

The radix 8 kernels execute 56 (first), 84 (middle) and 120 (last) arithmetic and shuffle
operations. On the Kryo that is the time, plus loads and stores. On the X4 and the M2 the bound
of four operations per cycle would be 14, 21 and 30 cycles; the first and last pass also
shuffle to split and interleave the data.

The AVX-512 kernels, in cycles per butterfly while the data is in the first level cache
(`bench/passes.c plain=1`, 1024 and 4096 points):

| | `f16` | `f8` | `c8` | `c4` | `la3` | `la2` | `lb` |
|---|--:|--:|--:|--:|--:|--:|--:|
| complex vectors of 16 values, index bits | 16, 4 | 8, 3 | 8, 3 | 4, 2 | 8, 3 | 4, 2 | 4, 2 |
| arithmetic and shuffle operations | 184 | 72 | 84 | 28 | 128 | 48 | 52 |
| loads (twiddles and constants included), stores | 72, 32 | 24, 16 | 30, 16 | 14, 8 | 32, 16 | 14, 8 | 14, 8 |
| Zen 5, cycles | 61 | 27 | 32 | 11 to 13 | 50 | 17 | 16 |
| Zen 4, cycles | 125 | 44 | 64 | 23 | 91 | 32 | 31 |
| Cascade Lake, cycles | 113 | 56 | 57 | 21 | 74 | 32 | 29 |
| Zen 5, cycles per vector and index bit | 0.95 | 1.1 | 1.3 | 1.5 | 2.1 | 2.1 | 2.0 |
| Zen 4, cycles per vector and index bit | 2.0 | 1.8 | 2.7 | 2.9 | 3.8 | 4.0 | 3.9 |
| Cascade Lake, cycles per vector and index bit | 1.8 | 2.3 | 2.4 | 2.6 | 3.1 | 4.0 | 3.7 |

On Zen 4 the cycles are close to loads plus twice the stores: it is limited by memory
operations. On Zen 5 they follow the operation count at about 3 per cycle, on Cascade Lake at
1.3 to 1.8 per cycle, of the 2 that its units allow. On all three, the index bits in the lanes
cost more than the others, because every exchange is two shuffles per vector.

What would be needed to be substantially faster:

* On the 64 bit wide cores: fewer operations, and the count is already at the level of FFTW's
  generated codelets. A radix 16 or 32 pass does not fit 32 registers without spilling.
* On the current AArch64 cores: closing the gap between 3.2 and 4 operations per cycle, and the
  cost of split and interleave, which vDSP does not have.
* On x86 with AVX2: kernels per vendor (see above).
* With AVX-512, what was not built: a pair of last passes for 6 index bits (the lane exchange of
  index bit 3 after two bits across the registers, then three exchanges in one pass of 8
  vectors). It would save a pass at every third size, 1024 points among them; estimated from
  the table above 15% on Zen 4 and 5% on Zen 5 there. Middle passes whose blocks are a single
  butterfly (the first one of every transform) load the same twiddles for every butterfly of a
  region; a variant that keeps them in registers would save 6 or 14 loads per butterfly, about
  5% of a transform on Zen 4. Neither is likely to do much on Cascade Lake, whose kernels run
  close to what its two arithmetic units allow whatever the loads and the number of passes:
  there only fewer operations help, and a transform already needs few more than the
  butterflies themselves (two shuffles per vector to split, two to interleave, eight for the
  four exchanges).

## Open questions

What was not measured or not built. Each item points to the section with the evidence. When an
item is measured or tried, its result goes to that section (or to "Decisions and their effect",
under "Tried and dropped" if it did not pay off) and the item is removed here.

Not measured:

* **AVX-512 on Intel with turbo, and on Intel cores after Cascade Lake.** The one Intel
  processor with AVX-512 ran with turbo switched off (see "Setup"). With turbo, processors of
  that generation clock 512 bit code lower than 256 bit code; whether the AVX-512 kernels still
  beat the AVX2 kernels by the 1.3 to 2.0 times measured is unknown. Ice Lake and later were not
  measured at all.
* **The order of the first pass on Intel cores after Cascade Lake.** Skylake and Cascade Lake
  gain nothing from writing in order, so every x86 processor that is not AMD's reads in order
  (see "Decisions and their effect"). Newer Intel cores may behave like AMD's.
* **The SSE2 kernels were not tuned** (see "SSE2"): every x86-64 machine measured has AVX2.

Not built or not tried:

* **AVX-512: last passes for 6 index bits** (one pass less at every third size, estimated 15%
  on Zen 4 and 5% on Zen 5 at 1024 points) and **middle passes that keep the twiddles of a
  region in registers** when its blocks are single butterflies (about 5% on Zen 4). See "Where
  the time goes"; neither is likely to help Cascade Lake.
* **A 64 point kernel for AVX2.** 64 points are 20 to 25% below the level of 128 points with
  AVX2; 8 complex vectors leave no register free for the lane exchanges (see "The dip at 64
  points").
* **AVX2 kernels per vendor.** Between 64 and 4096 points patient FFTW is up to 16% faster than
  OneFFT on some machines with AVX2 and NEON (see "Summary"). The AVX2 kernels are limited by
  the multiply-add units on AMD's cores and by the instruction count on Intel's (see "What the
  cores can do" and "Where the time goes").
* **The fused multiply-add form of the middle and last passes on x86** ("The second form of the
  passes" in design.md; about 8% faster on the M2). It was not tried with AVX2 or AVX-512, whose
  multiply-adds are not destructive, so the register copies it costs on AArch64 would not be
  needed.
* **Closing the gap to vDSP on Apple platforms**, 1.1 to 1.7 times from 128 points up (see
  "Summary"). vDSP works on split real and imaginary arrays; how much of the gap is the split
  and interleave that OneFFT has to do was not measured.
