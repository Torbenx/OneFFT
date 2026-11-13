# OneFFT measurements

What was measured while building OneFFT, and what followed from it. [design.md](design.md)
describes the result; this document is the evidence behind its decisions, and the place to look
before trying to make something faster.

## Setup

| machine | core | instruction set |
|---|---|---|
| Google Pixel 3a | Kryo 360 Gold (Cortex-A75 derivative), 2.0 GHz | AArch64 |
| Samsung Galaxy A50 | Cortex-A73, 2.3 GHz | AArch64 |
| Samsung Galaxy S24 | Cortex-X4, 3.2 GHz | AArch64 |
| Apple M2 | Avalanche, 3.5 GHz | AArch64 |
| Laptop | AMD Ryzen 7 5700U (Zen 2), 4.3 GHz | AVX2 |
| Desktop | Intel Core i5-9600K (Skylake), 4.5 GHz | AVX2 |
| Server | AMD EPYC 7443P (Zen 3) | AVX2 |
| Server | AMD EPYC 9254 (Zen 4), 4.1 GHz | AVX-512 |
| Server | AMD EPYC 9255 (Zen 5), 4.3 GHz | AVX-512 |

All times are for one forward transform, single thread, pinned to the fastest cores on the
phones, minimum over 15 samples of at least 5 ms each (`bench/bench.c`). Every library is called
through its out-of-place interface on interleaved data, except vDSP, which only has split real
and imaginary arrays and is given those (the conversion is not timed, which favours it).

* FFTW 3.3.10, single precision, NEON or AVX2 and AVX-512 enabled, `FFTW_PATIENT` plans up to
  4096 points and `FFTW_MEASURE` above (patient planning of large sizes takes minutes).
* FFmpeg 7.1, `av_tx` with `AV_TX_FLOAT_FFT`, with its assembly for the target.
* PFFFT (the marton78 fork), `pffft_transform_ordered`; SSE or NEON, it has no AVX kernels.
* vDSP from Accelerate, `vDSP_DFT_Execute`.

`scripts/third_party.sh` builds the libraries and `scripts/build_bench.sh` the benchmark.
Compilers: Clang 18 (Android NDK r27c), Apple Clang 21, GCC 15.

Zen 4, Zen 5, Zen 2 and the M2 were measured with the input buffer at the start of a page and
the output buffer 2048 bytes behind one. Where the two sit relative to each other matters for
small transforms on Zen 4: with the same offset in their pages, loads of the next transform wait
for stores of the previous one, and FFTW takes 56 instead of 29 ns for 64 points, OneFFT 56
instead of 47 ns for 128. The phones, Skylake and Zen 3 were measured earlier, with buffers
wherever the allocator put them (the Zen 3 server was not available again).

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
  <img src="plots/s24.svg" alt="Line chart of the speed of OneFFT, FFTW, FFmpeg and PFFFT over the transform size on a Cortex-X4; the numbers are in the tables below." width="760">
</picture>

<details>
<summary>Times in nanoseconds</summary>

<!-- results:s24 -->
| n | OneFFT | FFTW | FFmpeg | PFFFT | vs FFTW | vs FFmpeg | vs PFFFT |
|--:|--:|--:|--:|--:|--:|--:|--:|
| 2 | 1.1 | 3.1 | 0.9 | - | 2.82 | 0.82 | - |
| 4 | 1.9 | 3.7 | 1.6 | - | 1.95 | 0.84 | - |
| 8 | 4.4 | 7.5 | 4.8 | - | 1.70 | 1.09 | - |
| 16 | 8.7 | 18.9 | 12.6 | 19.1 | 2.17 | 1.45 | 2.20 |
| 32 | 16.9 | 24.7 | 26.9 | 32.5 | 1.46 | 1.59 | 1.92 |
| 64 | 47.0 | 50.7 | 67.7 | 73.6 | 1.08 | 1.44 | 1.57 |
| 128 | 97.8 | 109.0 | 154.7 | 150.5 | 1.11 | 1.58 | 1.54 |
| 256 | 221.0 | 234.1 | 364.3 | 317.1 | 1.06 | 1.65 | 1.43 |
| 512 | 489.9 | 561.9 | 847.8 | 714.1 | 1.15 | 1.73 | 1.46 |
| 1024 | 1088 | 1280 | 1877 | 1476 | 1.18 | 1.73 | 1.36 |
| 2048 | 2379 | 2772 | 4120 | 3269 | 1.16 | 1.73 | 1.37 |
| 4096 | 5125 | 6207 | 9047 | 7292 | 1.21 | 1.77 | 1.42 |
| 8192 | 11069 | 18286 | 24546 | 15996 | 1.65 | 2.22 | 1.45 |
| 16384 | 24730 | 41888 | 53323 | 34860 | 1.69 | 2.16 | 1.41 |
| 32768 | 53596 | 93390 | 117887 | 76394 | 1.74 | 2.20 | 1.43 |
| 65536 | 115218 | 192529 | 251765 | 153120 | 1.67 | 2.19 | 1.33 |
| 131072 | 248550 | 443613 | 558852 | 348188 | 1.78 | 2.25 | 1.40 |
| 262144 | 534678 | 1000225 | 2548672 | 804922 | 1.87 | 4.77 | 1.51 |
| 524288 | 1225722 | 3041602 | 5570117 | 2138848 | 2.48 | 4.54 | 1.74 |
| 1048576 | 2949024 | 6842500 | 12856132 | 8048164 | 2.32 | 4.36 | 2.73 |
<!-- end -->

</details>

### Kryo 360 Gold (Pixel 3a)

<picture>
  <source media="(prefers-color-scheme: dark)" srcset="plots/pixel3a-dark.svg">
  <img src="plots/pixel3a.svg" alt="Line chart of the speed of OneFFT, FFTW, FFmpeg and PFFFT over the transform size on a Pixel 3a; the numbers are in the tables below." width="760">
</picture>

<details>
<summary>Times in nanoseconds</summary>

<!-- results:pixel3a -->
| n | OneFFT | FFTW | FFmpeg | PFFFT | vs FFTW | vs FFmpeg | vs PFFFT |
|--:|--:|--:|--:|--:|--:|--:|--:|
| 2 | 4.0 | 11.5 | 4.0 | - | 2.88 | 1.00 | - |
| 4 | 8.6 | 15.9 | 5.6 | - | 1.85 | 0.65 | - |
| 8 | 22.2 | 28.9 | 21.5 | - | 1.30 | 0.97 | - |
| 16 | 42.3 | 67.5 | 54.2 | 88.7 | 1.60 | 1.28 | 2.10 |
| 32 | 93.5 | 120.0 | 143.9 | 157.1 | 1.28 | 1.54 | 1.68 |
| 64 | 237.7 | 251.0 | 336.0 | 342.7 | 1.06 | 1.41 | 1.44 |
| 128 | 559.1 | 538.7 | 794.5 | 737.1 | 0.96 | 1.42 | 1.32 |
| 256 | 1196 | 1152 | 1833 | 1565 | 0.96 | 1.53 | 1.31 |
| 512 | 2591 | 2578 | 4208 | 3391 | 1.00 | 1.62 | 1.31 |
| 1024 | 5847 | 5742 | 9463 | 7152 | 0.98 | 1.62 | 1.22 |
| 2048 | 12633 | 14476 | 21033 | 16174 | 1.15 | 1.66 | 1.28 |
| 4096 | 29110 | 34785 | 51160 | 40749 | 1.19 | 1.76 | 1.40 |
| 8192 | 63061 | 86929 | 121626 | 102456 | 1.38 | 1.93 | 1.62 |
| 16384 | 155583 | 203591 | 275980 | 235306 | 1.31 | 1.77 | 1.51 |
| 32768 | 346634 | 461823 | 656719 | 513770 | 1.33 | 1.89 | 1.48 |
| 65536 | 813151 | 1130234 | 1572760 | 1113763 | 1.39 | 1.93 | 1.37 |
| 131072 | 1788724 | 2811303 | 4643125 | 3447292 | 1.57 | 2.60 | 1.93 |
| 262144 | 4403855 | 6090366 | 14824376 | 9449792 | 1.38 | 3.37 | 2.15 |
| 524288 | 8826928 | 17496824 | 31115524 | 20413023 | 1.98 | 3.53 | 2.31 |
| 1048576 | 19808283 | 37532399 | 64856048 | 42985056 | 1.89 | 3.27 | 2.17 |
<!-- end -->

</details>

### Cortex-A73 (Galaxy A50)

<picture>
  <source media="(prefers-color-scheme: dark)" srcset="plots/a50-dark.svg">
  <img src="plots/a50.svg" alt="Line chart of the speed of OneFFT, FFTW, FFmpeg and PFFFT over the transform size on a Galaxy A50; the numbers are in the tables below." width="760">
</picture>

<details>
<summary>Times in nanoseconds</summary>

<!-- results:a50 -->
| n | OneFFT | FFTW | FFmpeg | PFFFT | vs FFTW | vs FFmpeg | vs PFFFT |
|--:|--:|--:|--:|--:|--:|--:|--:|
| 2 | 5.0 | 15.3 | 4.9 | - | 3.06 | 0.98 | - |
| 4 | 10.6 | 20.8 | 7.6 | - | 1.96 | 0.72 | - |
| 8 | 27.5 | 37.6 | 24.1 | - | 1.37 | 0.88 | - |
| 16 | 47.1 | 89.1 | 59.2 | 104.6 | 1.89 | 1.26 | 2.22 |
| 32 | 95.2 | 135.0 | 140.3 | 175.1 | 1.42 | 1.47 | 1.84 |
| 64 | 247.1 | 261.1 | 333.1 | 402.1 | 1.06 | 1.35 | 1.63 |
| 128 | 604.8 | 544.9 | 790.5 | 884.2 | 0.90 | 1.31 | 1.46 |
| 256 | 1277 | 1174 | 1824 | 1894 | 0.92 | 1.43 | 1.48 |
| 512 | 2677 | 2779 | 4221 | 4216 | 1.04 | 1.58 | 1.57 |
| 1024 | 6408 | 6809 | 9552 | 8930 | 1.06 | 1.49 | 1.39 |
| 2048 | 14670 | 15752 | 23146 | 22690 | 1.07 | 1.58 | 1.55 |
| 4096 | 32741 | 37842 | 59062 | 53994 | 1.16 | 1.80 | 1.65 |
| 8192 | 81653 | 100835 | 140945 | 126844 | 1.23 | 1.73 | 1.55 |
| 16384 | 170394 | 227445 | 311411 | 254584 | 1.33 | 1.83 | 1.49 |
| 32768 | 378514 | 541486 | 696827 | 625337 | 1.43 | 1.84 | 1.65 |
| 65536 | 905317 | 1427673 | 2027846 | 1720788 | 1.58 | 2.24 | 1.90 |
| 131072 | 2043077 | 3168885 | 6200731 | 5690730 | 1.55 | 3.03 | 2.79 |
| 262144 | 4749154 | 7479308 | 17297346 | 10778423 | 1.57 | 3.64 | 2.27 |
| 524288 | 10035039 | 19788077 | 36264270 | 25622923 | 1.97 | 3.61 | 2.55 |
| 1048576 | 21079576 | 42665307 | 76883846 | 48047923 | 2.02 | 3.65 | 2.28 |
<!-- end -->

</details>

### Apple M2

<picture>
  <source media="(prefers-color-scheme: dark)" srcset="plots/m2-dark.svg">
  <img src="plots/m2.svg" alt="Line chart of the speed of OneFFT, FFTW, FFmpeg and PFFFT and vDSP over the transform size on an Apple M2; the numbers are in the tables below." width="760">
</picture>

<details>
<summary>Times in nanoseconds</summary>

<!-- results:m2 -->
| n | OneFFT | FFTW | FFmpeg | PFFFT | vDSP | vs FFTW | vs FFmpeg | vs PFFFT | vs vDSP |
|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|
| 2 | 1.4 | 2.6 | 1.1 | - | - | 1.86 | 0.79 | - | - |
| 4 | 1.9 | 3.8 | 1.2 | - | - | 2.00 | 0.63 | - | - |
| 8 | 3.5 | 6.8 | 3.8 | - | - | 1.94 | 1.09 | - | - |
| 16 | 6.9 | 15.0 | 8.6 | 17.8 | 11.2 | 2.17 | 1.25 | 2.58 | 1.62 |
| 32 | 14.3 | 18.2 | 20.9 | 29.4 | 17.7 | 1.27 | 1.46 | 2.06 | 1.24 |
| 64 | 35.9 | 35.5 | 50.0 | 58.7 | 34.6 | 0.99 | 1.39 | 1.64 | 0.96 |
| 128 | 78.9 | 75.3 | 114.3 | 120.6 | 68.2 | 0.95 | 1.45 | 1.53 | 0.86 |
| 256 | 171.3 | 161.0 | 257.7 | 241.5 | 154.3 | 0.94 | 1.50 | 1.41 | 0.90 |
| 512 | 371.5 | 353.4 | 585.0 | 560.9 | 328.1 | 0.95 | 1.57 | 1.51 | 0.88 |
| 1024 | 811.5 | 768.8 | 1307 | 1134 | 536.5 | 0.95 | 1.61 | 1.40 | 0.66 |
| 2048 | 1714 | 1790 | 2890 | 2554 | 1023 | 1.04 | 1.69 | 1.49 | 0.60 |
| 4096 | 3693 | 3949 | 6332 | 5230 | 2319 | 1.07 | 1.71 | 1.42 | 0.63 |
| 8192 | 8273 | 9789 | 14312 | 14023 | 5330 | 1.18 | 1.73 | 1.69 | 0.64 |
| 16384 | 18695 | 23062 | 33555 | 29680 | 11594 | 1.23 | 1.79 | 1.59 | 0.62 |
| 32768 | 40984 | 58078 | 79062 | 66453 | 25016 | 1.42 | 1.93 | 1.62 | 0.61 |
| 65536 | 87406 | 126375 | 170500 | 131750 | 63266 | 1.45 | 1.95 | 1.51 | 0.72 |
| 131072 | 182688 | 266062 | 355375 | 295000 | 137344 | 1.46 | 1.95 | 1.61 | 0.75 |
| 262144 | 386875 | 599375 | 1580500 | 590750 | 284812 | 1.55 | 4.09 | 1.53 | 0.74 |
| 524288 | 821000 | 2122500 | 3282000 | 1288000 | 673000 | 2.59 | 4.00 | 1.57 | 0.82 |
| 1048576 | 1894000 | 4549000 | 7052000 | 2696000 | 2368000 | 2.40 | 3.72 | 1.42 | 1.25 |
<!-- end -->

</details>

### AMD EPYC 9255 (Zen 5), AVX-512

<picture>
  <source media="(prefers-color-scheme: dark)" srcset="plots/zen5-dark.svg">
  <img src="plots/zen5.svg" alt="Line chart of the speed of OneFFT with its AVX-512 and with its AVX2 kernels, FFTW, FFmpeg and PFFFT over the transform size on an AMD EPYC 9255; the numbers are in the tables below." width="760">
</picture>

"OneFFT (AVX2)" is OneFFT built with `-DONEFFT_NO_AVX512`. FFTW and FFmpeg use AVX-512 where
they have code for it.

<details>
<summary>Times in nanoseconds</summary>

<!-- results:zen5 -->
| n | OneFFT | OneFFT (AVX2) | FFTW | FFmpeg | PFFFT | vs OneFFT (AVX2) | vs FFTW | vs FFmpeg | vs PFFFT |
|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|
| 2 | 1.1 | 1.1 | 3.2 | 1.6 | - | 1.00 | 2.91 | 1.45 | - |
| 4 | 1.8 | 1.8 | 4.5 | 1.6 | - | 1.00 | 2.50 | 0.89 | - |
| 8 | 3.8 | 3.8 | 6.9 | 2.9 | - | 1.00 | 1.82 | 0.76 | - |
| 16 | 4.6 | 4.6 | 13.5 | 6.7 | 17.4 | 1.00 | 2.93 | 1.46 | 3.78 |
| 32 | 6.7 | 9.8 | 15.3 | 14.0 | 26.8 | 1.46 | 2.28 | 2.09 | 4.00 |
| 64 | 14.6 | 25.9 | 21.6 | 49.3 | 59.7 | 1.77 | 1.48 | 3.38 | 4.09 |
| 128 | 26.2 | 48.5 | 32.4 | 97.4 | 129.5 | 1.85 | 1.24 | 3.72 | 4.94 |
| 256 | 52.8 | 106.3 | 67.0 | 214.5 | 286.3 | 2.01 | 1.27 | 4.06 | 5.42 |
| 512 | 109.9 | 219.8 | 140.3 | 453.3 | 651.1 | 2.00 | 1.28 | 4.12 | 5.92 |
| 1024 | 234.4 | 494.4 | 341.1 | 964.9 | 1395 | 2.11 | 1.46 | 4.12 | 5.95 |
| 2048 | 491.8 | 1017 | 718.5 | 2031 | 3108 | 2.07 | 1.46 | 4.13 | 6.32 |
| 4096 | 1267 | 2345 | 1639 | 4599 | 6545 | 1.85 | 1.29 | 3.63 | 5.16 |
| 8192 | 3324 | 5192 | 5701 | 12746 | 14185 | 1.56 | 1.72 | 3.84 | 4.27 |
| 16384 | 6880 | 12183 | 12957 | 36176 | 29410 | 1.77 | 1.88 | 5.26 | 4.27 |
| 32768 | 14928 | 24674 | 34258 | 78244 | 64482 | 1.65 | 2.29 | 5.24 | 4.32 |
| 65536 | 36592 | 57979 | 83225 | 173332 | 135790 | 1.58 | 2.27 | 4.74 | 3.71 |
| 131072 | 84447 | 122116 | 187263 | 430886 | 312061 | 1.45 | 2.22 | 5.10 | 3.70 |
| 262144 | 164064 | 254913 | 406242 | 846404 | 667342 | 1.55 | 2.48 | 5.16 | 4.07 |
| 524288 | 356344 | 536425 | 2092234 | 1872943 | 1314524 | 1.51 | 5.87 | 5.26 | 3.69 |
| 1048576 | 951749 | 1470756 | 4200779 | 4741443 | 2767490 | 1.55 | 4.41 | 4.98 | 2.91 |
<!-- end -->

</details>

### AMD EPYC 9254 (Zen 4), AVX-512

<picture>
  <source media="(prefers-color-scheme: dark)" srcset="plots/zen4-dark.svg">
  <img src="plots/zen4.svg" alt="Line chart of the speed of OneFFT with its AVX-512 and with its AVX2 kernels, FFTW, FFmpeg and PFFFT over the transform size on an AMD EPYC 9254; the numbers are in the tables below." width="760">
</picture>

<details>
<summary>Times in nanoseconds</summary>

<!-- results:zen4 -->
| n | OneFFT | OneFFT (AVX2) | FFTW | FFmpeg | PFFFT | vs OneFFT (AVX2) | vs FFTW | vs FFmpeg | vs PFFFT |
|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|
| 2 | 1.7 | 1.7 | 4.8 | 2.2 | - | 1.00 | 2.82 | 1.29 | - |
| 4 | 3.1 | 3.1 | 5.3 | 1.9 | - | 1.00 | 1.71 | 0.61 | - |
| 8 | 4.6 | 4.6 | 9.2 | 3.9 | - | 1.00 | 2.00 | 0.85 | - |
| 16 | 5.1 | 5.4 | 17.7 | 8.3 | 21.3 | 1.06 | 3.47 | 1.63 | 4.18 |
| 32 | 11.2 | 11.5 | 20.2 | 16.7 | 33.4 | 1.03 | 1.80 | 1.49 | 2.98 |
| 64 | 25.7 | 30.3 | 28.9 | 41.8 | 73.9 | 1.18 | 1.12 | 1.63 | 2.88 |
| 128 | 46.7 | 53.9 | 50.6 | 85.2 | 156.4 | 1.15 | 1.08 | 1.82 | 3.35 |
| 256 | 95.4 | 116.1 | 100.5 | 194.7 | 315.2 | 1.22 | 1.05 | 2.04 | 3.30 |
| 512 | 207.9 | 242.3 | 220.7 | 408.9 | 695.3 | 1.17 | 1.06 | 1.97 | 3.34 |
| 1024 | 452.2 | 547.6 | 500.4 | 885.9 | 1446 | 1.21 | 1.11 | 1.96 | 3.20 |
| 2048 | 1007 | 1167 | 1151 | 1984 | 3456 | 1.16 | 1.14 | 1.97 | 3.43 |
| 4096 | 2328 | 2947 | 2730 | 5303 | 6857 | 1.27 | 1.17 | 2.28 | 2.95 |
| 8192 | 5588 | 6115 | 8582 | 17762 | 14772 | 1.09 | 1.54 | 3.18 | 2.64 |
| 16384 | 11767 | 13132 | 19124 | 45654 | 30717 | 1.12 | 1.63 | 3.88 | 2.61 |
| 32768 | 25478 | 29001 | 43110 | 102702 | 69102 | 1.14 | 1.69 | 4.03 | 2.71 |
| 65536 | 56942 | 68772 | 102740 | 229704 | 156611 | 1.21 | 1.80 | 4.03 | 2.75 |
| 131072 | 121054 | 142349 | 269067 | 516227 | 333202 | 1.18 | 2.22 | 4.26 | 2.75 |
| 262144 | 262310 | 311763 | 550616 | 1085104 | 690566 | 1.19 | 2.10 | 4.14 | 2.63 |
| 524288 | 552103 | 660058 | 2826413 | 2429432 | 1464796 | 1.20 | 5.12 | 4.40 | 2.65 |
| 1048576 | 1356126 | 1588827 | 5843846 | 5781116 | 3243614 | 1.17 | 4.31 | 4.26 | 2.39 |
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
| 2 | 2.0 | 6.0 | 1.8 | - | 3.00 | 0.90 | - |
| 4 | 2.8 | 6.5 | 2.4 | - | 2.32 | 0.86 | - |
| 8 | 7.3 | 12.0 | 5.7 | - | 1.64 | 0.78 | - |
| 16 | 7.4 | 24.1 | 11.6 | 33.7 | 3.26 | 1.57 | 4.55 |
| 32 | 14.0 | 21.9 | 21.5 | 46.4 | 1.56 | 1.54 | 3.31 |
| 64 | 39.5 | 41.2 | 52.9 | 109.0 | 1.04 | 1.34 | 2.76 |
| 128 | 70.4 | 67.7 | 106.2 | 215.1 | 0.96 | 1.51 | 3.06 |
| 256 | 145.4 | 136.3 | 240.6 | 417.1 | 0.94 | 1.65 | 2.87 |
| 512 | 328.0 | 291.7 | 511.5 | 920.8 | 0.89 | 1.56 | 2.81 |
| 1024 | 683.9 | 664.7 | 1138 | 1927 | 0.97 | 1.66 | 2.82 |
| 2048 | 1595 | 1551 | 2695 | 4687 | 0.97 | 1.69 | 2.94 |
| 4096 | 4353 | 3990 | 6333 | 11274 | 0.92 | 1.45 | 2.59 |
| 8192 | 8824 | 11371 | 16068 | 25441 | 1.29 | 1.82 | 2.88 |
| 16384 | 19583 | 26499 | 35949 | 54309 | 1.35 | 1.84 | 2.77 |
| 32768 | 44906 | 67506 | 84453 | 125816 | 1.50 | 1.88 | 2.80 |
| 65536 | 96871 | 177583 | 190030 | 261951 | 1.83 | 1.96 | 2.70 |
| 131072 | 211784 | 353387 | 398592 | 566408 | 1.67 | 1.88 | 2.67 |
| 262144 | 456699 | 724082 | 840236 | 1218783 | 1.59 | 1.84 | 2.67 |
| 524288 | 1139242 | 3731023 | 2148882 | 3134970 | 3.28 | 1.89 | 2.75 |
| 1048576 | 2919645 | 8346325 | 8577686 | 9414169 | 2.86 | 2.94 | 3.22 |
<!-- end -->

</details>

### AMD EPYC 7443P (Zen 3), AVX2

<picture>
  <source media="(prefers-color-scheme: dark)" srcset="plots/epyc-dark.svg">
  <img src="plots/epyc.svg" alt="Line chart of the speed of OneFFT, FFTW, FFmpeg and PFFFT over the transform size on an AMD EPYC 7443P; the numbers are in the tables below." width="760">
</picture>

Measured before the first pass learned to write in order (see "Decisions and their effect").
On Zen 2 and Zen 4 that made 8192 to 524288 points 5 to 15% faster with the AVX2 kernels; the
same is to be expected here.

<details>
<summary>Times in nanoseconds</summary>

<!-- results:epyc -->
| n | OneFFT | FFTW | FFmpeg | PFFFT | vs FFTW | vs FFmpeg | vs PFFFT |
|--:|--:|--:|--:|--:|--:|--:|--:|
| 2 | 3.4 | 7.4 | 2.1 | - | 2.18 | 0.62 | - |
| 4 | 4.6 | 8.5 | 3.9 | - | 1.85 | 0.85 | - |
| 8 | 7.5 | 14.5 | 7.1 | - | 1.93 | 0.95 | - |
| 16 | 8.8 | 31.4 | 14.9 | 33.4 | 3.57 | 1.69 | 3.80 |
| 32 | 19.3 | 30.3 | 30.0 | 54.3 | 1.57 | 1.55 | 2.81 |
| 64 | 46.5 | 47.2 | 74.0 | 113.4 | 1.02 | 1.59 | 2.44 |
| 128 | 88.9 | 89.0 | 142.9 | 240.2 | 1.00 | 1.61 | 2.70 |
| 256 | 185.1 | 169.1 | 318.8 | 478.5 | 0.91 | 1.72 | 2.59 |
| 512 | 394.9 | 441.0 | 692.2 | 1101 | 1.12 | 1.75 | 2.79 |
| 1024 | 840.3 | 849.0 | 1498 | 2174 | 1.01 | 1.78 | 2.59 |
| 2048 | 1947 | 2045 | 3334 | 4992 | 1.05 | 1.71 | 2.56 |
| 4096 | 4505 | 4892 | 8685 | 10788 | 1.09 | 1.93 | 2.39 |
| 8192 | 10946 | 15419 | 28277 | 23188 | 1.41 | 2.58 | 2.12 |
| 16384 | 22531 | 39125 | 72073 | 49746 | 1.74 | 3.20 | 2.21 |
| 32768 | 52796 | 101118 | 167429 | 124081 | 1.92 | 3.17 | 2.35 |
| 65536 | 121752 | 209993 | 396611 | 265136 | 1.72 | 3.26 | 2.18 |
| 131072 | 257086 | 468810 | 833370 | 545456 | 1.82 | 3.24 | 2.12 |
| 262144 | 535243 | 1090049 | 1781685 | 1129582 | 2.04 | 3.33 | 2.11 |
| 524288 | 1120759 | 4546586 | 3991483 | 2439349 | 4.06 | 3.56 | 2.18 |
| 1048576 | 2383984 | 9825527 | 8988031 | 5199750 | 4.12 | 3.77 | 2.18 |
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
| 16 | 8.1 | 25.0 | 11.5 | 27.2 | 3.09 | 1.42 | 3.36 |
| 32 | 15.4 | 25.4 | 23.0 | 42.9 | 1.65 | 1.49 | 2.79 |
| 64 | 40.3 | 37.6 | 61.8 | 93.2 | 0.93 | 1.53 | 2.31 |
| 128 | 75.3 | 74.7 | 119.1 | 199.8 | 0.99 | 1.58 | 2.65 |
| 256 | 168.8 | 144.8 | 269.9 | 404.7 | 0.86 | 1.60 | 2.40 |
| 512 | 340.1 | 324.4 | 593.3 | 903.2 | 0.95 | 1.74 | 2.66 |
| 1024 | 754.1 | 709.4 | 1272 | 1885 | 0.94 | 1.69 | 2.50 |
| 2048 | 1569 | 1637 | 2808 | 4302 | 1.04 | 1.79 | 2.74 |
| 4096 | 3787 | 4006 | 6616 | 9199 | 1.06 | 1.75 | 2.43 |
| 8192 | 8115 | 11569 | 19444 | 19730 | 1.43 | 2.40 | 2.43 |
| 16384 | 18999 | 27932 | 54369 | 40927 | 1.47 | 2.86 | 2.15 |
| 32768 | 39941 | 69636 | 155828 | 93463 | 1.74 | 3.90 | 2.34 |
| 65536 | 86095 | 160752 | 316536 | 199715 | 1.87 | 3.68 | 2.32 |
| 131072 | 221772 | 345542 | 666672 | 483800 | 1.56 | 3.01 | 2.18 |
| 262144 | 620910 | 991851 | 1981086 | 1621658 | 1.60 | 3.19 | 2.61 |
| 524288 | 2279444 | 5008482 | 6511931 | 6622131 | 2.20 | 2.86 | 2.91 |
| 1048576 | 5409455 | 10718990 | 14779559 | 13809826 | 1.98 | 2.73 | 2.55 |
<!-- end -->

</details>

### SSE2

The SSE2 kernels on the i5-9600K (built with `-DONEFFT_NO_AVX2`) against PFFFT, which is also
SSE. They were not tuned.

<!-- results:intel_sse2 -->
| n | OneFFT | PFFFT | vs PFFFT |
|--:|--:|--:|--:|
| 8 | 7.2 | - | - |
| 16 | 13.3 | 34.2 | 2.57 |
| 32 | 41.3 | 46.1 | 1.12 |
| 64 | 80.0 | 105.1 | 1.31 |
| 128 | 194.6 | 217.5 | 1.12 |
| 256 | 390.8 | 425.4 | 1.09 |
| 512 | 920.3 | 944.7 | 1.03 |
| 1024 | 1889 | 1927 | 1.02 |
| 2048 | 4426 | 4589 | 1.04 |
| 4096 | 9836 | 10823 | 1.10 |
| 8192 | 22466 | 24931 | 1.11 |
| 16384 | 46670 | 54980 | 1.18 |
| 32768 | 106882 | 127050 | 1.19 |
| 65536 | 216638 | 258271 | 1.19 |
<!-- end -->

### Summary

* Against FFmpeg, OneFFT is 1.25 to 5.3 times faster from 16 points up, on every machine.
  Against PFFFT 1.2 to 6.3 times. At 4 and 8 points FFmpeg's dedicated kernels are up to 1.6
  times faster than OneFFT's.
* Against patient FFTW there are three ranges. Up to 32 points OneFFT's single kernels are 1.3 to
  3.6 times faster. From 8192 points up OneFFT is 1.2 to 5.9 times faster: its passes stay cache
  friendly, and FFTW's measured (not patient) plans are weaker. Between 64 and 4096 points both
  are close with NEON and AVX2, OneFFT being between 14% slower and 21% faster (ahead at every
  size on the Cortex-X4, behind at most on Zen 2); FFTW's patient planner picks large generated
  codelets there that need very few passes.
* With AVX-512, OneFFT is ahead of FFTW (which has AVX-512 codelets) at every size, also between
  64 and 4096 points: by 1.24 to 1.5 times there on Zen 5 and by 1.05 to 1.17 times on Zen 4.
  Against its own AVX2 kernels on the same machine it is 1.45 to 2.1 times faster on Zen 5 from
  32 points up, and 1.1 to 1.27 times on Zen 4 from 64 points up; a 512 bit operation is two 256
  bit operations on Zen 4.
* The order in which the first pass goes through its butterflies decides large transforms on
  some cores (see "Decisions and their effect"). The M2 used to be the exception at the top,
  with FFTW up to 16% faster between 2^15 and 2^18 points; with the first pass writing in
  order OneFFT is 1.4 to 1.55 times faster than FFTW there.
* Apple's vDSP is the fastest library on the M2: 1.1 to 1.7 times faster than OneFFT from 128
  to 524288 points, and faster than FFTW (at 2^20 points OneFFT is ahead). It is the only other
  library here that is written for one specific family of cores, and the only one that does
  not have to split and interleave.
* The 2021 proof of concept in `reference/` takes 6.9 us for 1024 points on the Pixel 3a, 6.6 us
  on the A50, 2.1 us on the S24 and 1.5 us on the M2: 1.04 to 1.9 times the time of OneFFT.

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

| | Skylake | Zen 2 | Zen 3 |
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

| | Zen 4 | Zen 5 |
|---|--:|--:|
| loads | 1 | 2 |
| stores | 0.5 | 1 |
| additions | 1 | 2 |
| multiplications, fused multiply-adds | 1 | 2 |
| additions and multiplications together | 2 | 4 |
| `unpck`, `shufps` | 1.5 | 4 |
| `vshuff32x4` | 1 | 2 |
| cycles for a load, two multiplications by a twiddle vector and a store: twiddle as memory operands / loaded into a register | 3.2 / 2.2 | 1.5 / 1.0 |

(`bench/uops_x86.c` built with `-DWIDE -mavx512f`.) What follows:

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
| The same with the AVX2 kernels, 2^13 points up | 2^18 points: 505 to 257 us / 345 to 315 us; Zen 2 at 2^16: 100 to 86 us |
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
the third level cache (Zen 2 with 4 MiB: beyond 2^16 points; Zen 4 with 32 MiB: beyond 2^19),
on Zen 5 half of it (2^20). Beyond that, reading in order is up to 25% faster. The plan reads
the cache size from the processor (`onefft_x86_tune` in `onefft.c`). The radix 16 first pass
loses against radix 8 from 8192 points up (by up to 25%: its 16 input and 16 output streams
are a multiple of 4096 bytes apart then) and wins again by 3 to 10% when the data no longer
fits the third level cache.

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
| Zen 5, cycles per vector and index bit | 0.95 | 1.1 | 1.3 | 1.5 | 2.1 | 2.1 | 2.0 |
| Zen 4, cycles per vector and index bit | 2.0 | 1.8 | 2.7 | 2.9 | 3.8 | 4.0 | 3.9 |

On Zen 4 the cycles are close to loads plus twice the stores: it is limited by memory
operations. On Zen 5 they follow the operation count at about 3 per cycle. On both, the index
bits in the lanes cost twice as much as the others, because every exchange is two shuffles per
vector.

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
  5% of a transform on Zen 4. And nothing is known about Intel's processors with AVX-512, which
  have one shuffle unit and share two units between additions and multiplications: the kernels
  were tuned without one.
