#!/usr/bin/env python3
"""Verifies the generated kernels by simulation against a double precision reference FFT.

    test_kernels.py [neon] [sse2] [avx2] [avx512]      default: all
"""

import random
import sys

from onefft_gen import avx512_kernels, model, neon_kernels, x86_kernels


def check(label, transform, log2n, inverse):
    n = 1 << log2n
    rng = random.Random(log2n)
    x = [complex(rng.uniform(-1, 1), rng.uniform(-1, 1)) for _ in range(n)]
    flat = [part for z in x for part in (z.real, z.imag)]
    out = transform(flat)
    got = [complex(out[2 * i], out[2 * i + 1]) for i in range(n)]
    want = model.dft(x, inverse)
    scale = max(abs(z) for z in want)
    error = max(abs(g - w) for g, w in zip(got, want)) / scale
    label = "%s n=%d %s" % (label, n, "inverse" if inverse else "forward")
    # The simulation runs in double precision, so the result has to be exact up to rounding.
    assert error < 1e-12, "%s: relative error %g" % (label, error)
    print("ok   %-44s error %.1e" % (label, error))


def main():
    targets = sys.argv[1:] or ["neon", "sse2", "avx2", "avx512"]
    if "neon" in targets:
        kernels = neon_kernels.all_kernels()
        for inverse in (False, True):
            for log2n in range(3, 13):
                check("neon", lambda flat: model.neon_transform(kernels, log2n, flat, inverse), log2n, inverse)
            for log2n in range(6, 13):
                check("neon fma", lambda flat: model.neon_transform(kernels, log2n, flat, inverse, fma=True), log2n,
                      inverse)
        # Every way of splitting the middle and last passes.
        for radices in ([3, 2, 2], [3, 3, 2], [3, 2, 3], [3, 2, 2, 2], [3, 2, 2, 3], [3, 3, 2, 2], [3, 2, 3, 2]):
            for fma in (False, True):
                check("neon %s%s" % (radices, " fma" if fma else ""),
                      lambda flat: model.neon_transform(kernels, sum(radices), flat, False, radices, fma), sum(radices),
                      False)
    x86 = {isa: x86_kernels.all_kernels(isa) for isa in ("sse2", "avx2") if isa in targets or "avx2" in targets}
    for isa in ("sse2", "avx2"):
        if isa in targets:
            for inverse in (False, True):
                for log2n in range(3, 13):
                    check(isa, lambda flat: model.x86_transform(x86, isa, log2n, flat, inverse), log2n, inverse)
    if "avx512" in targets:
        kernels = avx512_kernels.all_kernels()
        for inverse in (False, True):
            for log2n in (5, 6, 7):
                check("avx512", lambda flat: model.avx512_transform(kernels, log2n, flat, inverse), log2n, inverse)
            for first_bits in (4, 3):  # radix 16 and radix 8 first pass
                for log2n in range(8 if first_bits == 4 else 10, 15):
                    check("avx512 f%d %s" % (1 << first_bits, model.avx512_passes(log2n, first_bits)),
                          lambda flat: model.avx512_transform(kernels, log2n, flat, inverse, first_bits, log2n % 2 == 1),
                          log2n, inverse)
    return 0


if __name__ == "__main__":
    sys.exit(main())
