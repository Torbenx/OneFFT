#!/usr/bin/env python3
"""Regenerates everything that is generated and tracked in git:

* the SIMD kernel sources src/onefft_*.inc, from gen/onefft_gen
* the single file distribution dist/onefft.c and dist/onefft.h, from src/ and include/

Run it after changing anything in gen/, src/ or include/. CI fails if its output differs from
what is checked in.
"""

import os
import re
import sys

from onefft_gen import avx512_kernels, neon_kernels, x86_kernels

HEADER = """/*
 * OneFFT %s kernels.
 *
 * GENERATED FILE, DO NOT EDIT. Change gen/onefft_gen and run gen/generate.py instead.
 *
 * Every kernel is a normal C function whose body is one inline assembly block. The SIMD registers
 * are assigned by the generator, the general purpose registers by the C compiler.
 */
"""


# The assembly blocks are longer than the 4095 characters ISO C guarantees for string literals.
PROLOGUE = "\n".join([
    "",
    "#pragma GCC diagnostic push",
    '#pragma GCC diagnostic ignored "-Woverlength-strings"',
])

EPILOGUE = "#pragma GCC diagnostic pop"


def write(path, text):
    old = open(path).read() if os.path.exists(path) else None
    if old != text:
        with open(path, "w") as f:
            f.write(text)
        print("wrote", path)


def emit(path, title, kernels, attributes=""):
    parts = [HEADER % title + PROLOGUE]
    parts += [kernel.c_function(attributes) for kernel in kernels]
    parts.append(EPILOGUE)
    write(path, "\n\n".join(parts) + "\n")


DIST_HEADER = """/*
 * OneFFT, single file distribution: compile this file and include onefft.h.
 *
 * GENERATED FILE, DO NOT EDIT. It is src/onefft.c with the generated kernel sources included,
 * produced by gen/generate.py. The repository has the sources, tests and documentation.
 */
"""


def amalgamate(root):
    """Writes dist/onefft.c (src/onefft.c with its kernel includes expanded) and dist/onefft.h."""
    src = os.path.join(root, "src")

    def include(match):
        with open(os.path.join(src, match.group(1))) as f:
            return f.read().rstrip("\n")

    with open(os.path.join(src, "onefft.c")) as f:
        text = re.sub(r'#include "(onefft_\w+\.inc)"', include, f.read())
    os.makedirs(os.path.join(root, "dist"), exist_ok=True)
    write(os.path.join(root, "dist", "onefft.c"), DIST_HEADER + text)
    with open(os.path.join(root, "include", "onefft.h")) as f:
        write(os.path.join(root, "dist", "onefft.h"), f.read())


def main():
    root = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
    src = os.path.join(root, "src")
    emit(os.path.join(src, "onefft_neon.inc"), "AArch64 Advanced SIMD", neon_kernels.all_kernels())
    emit(os.path.join(src, "onefft_sse2.inc"), "x86-64 SSE2", x86_kernels.all_kernels("sse2"))
    emit(os.path.join(src, "onefft_avx2.inc"), "x86-64 AVX2 and FMA", x86_kernels.all_kernels("avx2"),
         '__attribute__((target("avx2,fma"))) ')
    emit(os.path.join(src, "onefft_avx512.inc"), "x86-64 AVX-512", avx512_kernels.all_kernels(),
         '__attribute__((target("avx512f"))) ')
    amalgamate(root)
    return 0


if __name__ == "__main__":
    sys.exit(main())
