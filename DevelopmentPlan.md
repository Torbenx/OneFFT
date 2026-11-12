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