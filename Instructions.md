# Instructions

Your goal is to write OneFFT, a simple and fast SIMD FFT implementation for power-of-two
out-of-place tranformations on arm64 and x86-64. The result should be a maintainable, production
ready FFT library. The name is a reference to that the implementation will be one-shot by AI (you).
There will no further user input, work independently and resolve problems and decisions by
yourself.

A proof of concept implementation is available in the `reference` directory. It performs high-radix
transforms with as much data as possible in registers. This is the core structural building block
of OneFFT.

The reference partitions the SIMD registers into 4 banks and rotates bank 1-3 to perform the
transformation while bank 4 statically holds weights needed for the transformation. This could be
replaced with a proper register allocator in more sophisticated implementation if it becomes a
problem.

Writing custom instructions is necessary achieve the necessary control of register usage. To avoid
messing with calling conventions, the reference uses normal functions with a single big inline
assembly block. Inline assembly uses the GCC/Clang syntax, supporting MSVCs cl.exe is not required.

The reference was written 2021 and benchmarked at the time against the FFMPEG FFT implementation
and was about 2x faster (possibly one a Pixel 3a but not confirmed).

`DevelopmentPlan.md` holds a suggested development plan, edit and update it as you see fit.

`TestHardware.md` lists the benchmarking hardware available to you. Later stages will require
more X86 hardware, request it from the user when the time comes.