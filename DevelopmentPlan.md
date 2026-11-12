# Development Plan

This is the rougth development plan for the project and should be modified and expanded by you while implementing.

## Plan

### A. ARM-64

A1. Build, validate and analyse the reference
A2. Benchmark reference against other implementations
A3. Design and plan OneFFT implementation
A4. Implement OneFFT for sizes matching the reference
A5. Review implementation and run tests and benchmarks
A6. Iteratively design, implement, test, benchmark and improve new sizes
A7. Review entire implementation and apply suite wide refactors/improvements

### X. X86-64

X1. Design and plan implementation for AVX2
X2. Implement AVX2 plan for an initial range of sizes and run tests
X3. Setup benchmark suite and compare against other libraries
X4. Run benchmark suite on extended hardware (ask user)
X5. Iteratively design, implement, test, benchmark and improve new sizes for AVX2
X6. Design and plan implementation for SSE variant (128 bit registers)
X7. Iteratively design, implement, test, benchmark and improve new sizes for SSE
X8. Design and plan implementation for AVX512 variant
X9. Iteratively design, implement, test, benchmark and improve new sizes for AVX512

### Release

R1. Review entire code base and apply refactors/improvements
R2. Design library iterface and distribution
    * Use describtive names and document potential additional overhead of call.
      For example "Invoking a size for the first time will allocate and compute the weight table"
      or "This will allocate and compute all weight tables".
    * Assume that users will use shared objects (such as a context holding weights) from multiple
      threads. Relay on single threaded-ness only when the library user has explicitly opted in.
    * Interface suggestion: C and C++ interfaces. (Large) C functions definded out-of-line in .c
      file, C++ functions inline and forward to C interface.
    * Distribution suggestion: Generate a single .c and .h (tracked in git) that
      can be copied directly or include via build scripts. Build scripts for cmake + bazel.
R3. Implement user iterface with documentation
R4. Setup github CI for correctness tests
R5. Write further documentation (REAME, etc)
R6. Final review and polish

## Status (written by the implementation sessions)

Done and verified. Tests pass on Pixel 3a, Galaxy A50, Galaxy S24, Apple M2, the Zen 2 laptop,
the Intel i5-9600K and the EPYC 7443P.

* A1/A2  Reference built, validated and benchmarked against FFTW, FFmpeg, PFFFT, KissFFT, vDSP.
* A3-A7  AArch64 for all power-of-two sizes, forward and inverse: kernel generator with
         simulation tests (gen/), instruction scheduling, cache blocking for large sizes, and a
         second form of the passes from fused multiply-add butterflies, the default on Apple.
* X1-X5  AVX2 (+FMA), measured and tuned on Zen 2, Zen 3 and Skylake: radix 8 first pass and
         radix 8 pass before the last, both computing in place in 16 registers.
* X6/X7  SSE2: complete and tested, not tuned (every x86-64 machine at hand has AVX2).
* R1-R3  C and C++ interface (include/onefft.h), single file distribution (dist/), CMake, Bazel.
* R4     .github/workflows/ci.yml is written but has never run.
* R5     README.md, docs/design.md, docs/measurements.md.

Open:

* X8/X9  AVX-512 is not implemented: none of the machines has it. Such processors run the AVX2
         kernels. docs/design.md sketches the layout; Intel SDE would do for correctness tests
         but tuning needs hardware (ideally one Intel and one AMD Zen 4 or later).
* Between 64 and 4096 points FFTW with patient planning is still up to 13% faster on some
  machines (see docs/measurements.md); Apple's vDSP is 1.2 to 2.3 times faster on the M2.
  Ideas that were not tried: vendor specific AVX2 kernels (the in-place kernels are limited by
  the multiply-add units on AMD and by instruction count on Intel), and the fused form of the
  passes on x86.
* No LICENSE file: the choice is the owner's.
* Windows (MinGW/Clang) is untested. MSVC is not supported by design.
* CI has to be run once on GitHub; the workflow may need small fixes.
