"""Python models of the transform drivers in src/, used to test the kernels in simulation.

The models build the same tables as the C code and run the kernels through `Asm.run`. They exist
so that kernel changes can be verified on any machine in seconds, before anything is compiled
for the target. Keep them in sync with src/onefft.c.
"""

import cmath
import math

from .cplx import bit_reverse, root
from .ir import Machine
from .passes import ARGS


def split(values):
    """[z0, z1, ...] -> [re0, re1, ..., im0, im1, ...]"""
    return [z.real for z in values] + [z.imag for z in values]


def middle_entry(k, size, radix):
    """Table entry of a middle pass: the powers 1..radix-1 of exp(-2 pi i k / size), padded."""
    pad = 4 * (radix // 4)
    powers = [complex(root(k * i, size)) for i in range(1, radix)] + [0j] * (pad - radix + 1)
    return split(powers)


def middle_args(m, address, **fields):
    """Stores a onefft_middle_args struct (8 byte fields) in the simulated memory."""
    cells = []
    for name in ARGS:
        cells += [fields.get(name, 0), 0]
    m.store(address, cells)
    return address


# ---- AArch64 ---------------------------------------------------------------------------------


def neon_radices(log2n):
    """Radix bits per pass: first pass 3, then the middle passes, then the last pass.

    As few passes as possible, and a radix 8 last pass where the size allows it: per index bit
    it is the cheaper of the two last passes.
    """
    assert log2n >= 6
    if log2n == 7:
        return [3, 2, 2]
    rest = log2n - 6
    twos = (3 - rest % 3) % 3  # number of radix 4 passes among the middle passes
    return [3] + [2] * twos + [3] * ((rest - 2 * twos) // 3) + [3]


def cos_tan(values):
    """Table form of twiddles for the fused multiply-add kernels: real parts, then im / re."""
    values = [complex(z) for z in values]
    return [z.real for z in values] + [z.imag / z.real for z in values]


def neon_transform(kernels, log2n, flat, inverse=False, radices=None, fma=False):
    """Runs a complete transform of the interleaved float list in simulation.

    `fma` selects the passes built from fused multiply-add butterflies and their tables.
    """
    n = 1 << log2n
    kernel = {k.name[len("onefft_neon_"):]: k for k in kernels}
    suffix = "i" if inverse else ""
    m = Machine(64 * n + 4096)
    src, dst, table = 0, 8 * n, 16 * n
    m.store(src, flat)
    if log2n <= 5:
        kernel["n%d%s" % (n, suffix)].run(m, **{"in": src, "out": dst})
        return m.load(dst, 2 * n)
    radices = radices or neon_radices(log2n)
    first = []
    for a in range(n >> 5):
        first += [4 * a, 8 * bit_reverse(a, log2n - 5)]
    m.store(table, first)
    kernel["f8" + suffix].run(m, **{"in": src, "out": dst, "table": table, "count": n >> 5,
                                    "in_stride": n, "out_stride": 2 * n})
    done = 3
    variant = "_fma" if fma else ""
    args = 60 * n  # the struct is placed behind the tables
    for bits in radices[1:-1]:
        inner = 1 << (done - 2)
        tw = []
        for t in range(4):
            for lo in range(inner):
                k = t + 4 * lo
                if fma:
                    # stage twiddles; the first is stored a quarter turn back in the upper half
                    w = [root(k % (1 << (done - 1)), 1 << (done + 1)), root(k, 1 << (done + 2))]
                    if bits == 3:
                        w += [root(k, 1 << (done + 3)), root(k + (1 << done), 1 << (done + 3))]
                    tw += cos_tan(w)
                else:
                    tw += middle_entry(k, 1 << (done + bits), 1 << bits)
        m.store(table, tw)
        middle_args(m, args, s_high=32 * inner, inner=inner, skip=32 * inner * ((1 << bits) - 1),
                    tw_region=4 * len(tw) // 4)
        kernel["c%d%s" % (1 << bits, variant)].run(m, data=dst, tw=table, args=args, blocks=n >> (2 + done + bits),
                                                  regions=4)
        done += bits
    bits = radices[-1]
    assert done + bits == log2n
    tw = []
    count = n >> (bits + 2)
    for lo in range(count):
        if fma:
            # Twiddles as (cos, tan). Those of the first stage of the pass and of index bit 1
            # are stored a quarter turn back where their angle would reach pi/2.
            if bits == 3:
                tw += cos_tan([root(4 * ((t + 4 * lo) % (n >> 4)), n) for t in range(4)])
            tw += cos_tan([root(2 * ((t + 4 * lo) % (n >> 3)), n) for t in range(4)])
            for g in range(1 << (bits - 2)):
                tw += cos_tan([root(t + 4 * lo + (n >> 3) * g, n) for t in range(4)])
        else:
            if bits == 3:
                tw += split([root(4 * (t + 4 * lo), n) for t in range(4)])
            for g in range(1 << (bits - 2)):
                for l in range(1, 4):
                    tw += split([root(l * (t + 4 * lo + (n >> 3) * g), n) for t in range(4)])
    m.store(table, tw)
    kernel["l%d%s%s" % (1 << bits, variant, suffix)].run(m, data=dst, tw=table, stride=8 * n >> bits, count=count,
                                                        regions=1)
    return m.load(dst, 2 * n)


# ---- x86-64 ----------------------------------------------------------------------------------

ELEMENT_OF = (0, 1, 4, 5, 2, 3, 6, 7)  # AVX2: the element held by each lane, see x86_kernels


def x86_transform(kernels, isa, log2n, flat, inverse=False):
    """Runs a complete SSE2 or AVX2 transform in simulation.

    `kernels` maps the instruction set to its kernel list; AVX2 uses the SSE2 kernel for 8 points.
    """
    n = 1 << log2n
    kernel = {k.name[len("onefft_"):]: k for ks in kernels.values() for k in ks}
    suffix = "i" if inverse else ""
    lane_bits = 2 if isa == "sse2" else 3
    vector = 8 << lane_bits  # bytes of a split vector
    m = Machine(64 * n + 4096)
    src, dst, table = 0, 8 * n, 16 * n
    m.store(src, flat)

    def run(name, **params):
        kernel[name].run(m, **params)

    smallest = 5 if isa == "sse2" else 6
    if log2n < smallest:
        codelet_isa = isa if (1 << log2n) >= 4 << lane_bits or isa == "sse2" else "sse2"
        run("%s_n%d%s" % (codelet_isa, n, suffix), **{"in": src, "out": dst})
        return m.load(dst, 2 * n)

    # SSE2: all later passes transform two index bits, so the first pass is radix 4 for an even
    # number of bits and radix 2 for an odd one. AVX2: radix 8 (radix 4 for 64 points); its
    # second to last pass takes 2 or 3 bits to make the total fit. The top index bits that the
    # lane exchanges of the last passes need at the top of the output keep their place (unless
    # the first pass transforms them itself), the others are bit reversed.
    if isa == "sse2":
        first_bits = 2 if log2n % 2 == 0 else 1
    else:
        first_bits = 3 if log2n > 6 else 2
    kept = max(0, lane_bits - first_bits)  # top bits of the first pass counter that stay
    count_bits = log2n - first_bits - lane_bits
    first = []
    for a in range(1 << count_bits):
        low_bits = count_bits - kept
        low = bit_reverse(a & ((1 << low_bits) - 1), low_bits)
        first += [a << lane_bits, ((a >> low_bits << low_bits) | low) << lane_bits]
    m.store(table, first)
    run("%s_f%d%s" % (isa, 1 << first_bits, suffix), **{
        "in": src, "out": dst, "table": table, "count": 1 << count_bits,
        "in_stride": 8 * n >> first_bits, "out_stride": 8 * n >> first_bits})
    done = first_bits
    args = 60 * n  # the struct is placed behind the tables

    def middle(regions, region_bytes, s_high, s_low, inner, blocks, skip, k_of):
        nonlocal done
        tw = []
        for r in range(regions):
            for j in range(inner):
                tw += middle_entry(k_of(r, j), 1 << (done + 2), 4)
        m.store(table, tw)
        walked = blocks * (inner * vector + skip)  # bytes the data pointer moves within a region
        middle_args(m, args, s_high=s_high, s_low=s_low, inner=inner, skip=skip, region_skip=region_bytes - walked,
                    tw_region=4 * len(tw) // regions)
        run(isa + "_c4", data=dst, tw=table, args=args, blocks=blocks, regions=regions)
        done += 2

    # index bits left to the last passes
    if isa == "sse2":
        last_bits = 2
    else:
        last_bits = 5 if log2n % 2 == 0 and log2n > 6 else 4
    if isa == "sse2":
        if first_bits == 1:
            # index bits n-2 (at the second highest position) and n-3 (next to the lanes)
            middle(2, 4 * n, 2 * n, vector, 1, n >> 5, vector, lambda r, j: r)
        while done < log2n - last_bits:
            s_high = 8 << done
            middle(4, 2 * n, s_high, 2 * s_high, 1 << (done - 2), (2 * n) // (4 * s_high), 3 * s_high,
                   lambda r, j: bit_reverse(r, 2) + 4 * j)
        tw = []
        for j in range(n >> 4):
            for l in range(1, 4):
                tw += split([root(l * (t + 4 * j), n) for t in range(4)])
        m.store(table, tw)
        run("sse2_l4" + suffix, data=dst, tw=table, stride=2 * n, count=n >> 4, regions=1)
        return m.load(dst, 2 * n)

    while done < log2n - last_bits:
        s_high = 8 << done
        middle(8, n, s_high, 2 * s_high, 1 << (done - 3), n // (4 * s_high), 3 * s_high,
               lambda r, j: bit_reverse(r, 3) + 8 * j)
    if last_bits == 5:
        # la8: index bits 4, 3, 2. Lane bit b0 holds output bit 2 after the exchange.
        def shear(w):
            return [-z.imag / (1 + z.real) for z in w] + [-z.imag for z in w]

        count = n >> 8
        tw = []
        for r in range(4):
            for j in range(count):
                ks = [bit_reverse(r, 2) + 4 * (lane & 1) + 8 * j for lane in range(8)]
                first = ks if count == 1 else [k % (n >> 6) for k in ks]  # a quarter turn back in the upper half
                tw += shear([complex(root(k, n >> 4)) for k in first])
                tw += shear([complex(root(k, n >> 3)) for k in ks])
                tw += shear([complex(root(k, n >> 2)) for k in ks])
                tw += shear([complex(root(k + (n >> 5), n >> 2)) for k in ks])
        m.store(table, tw)
        run("avx2_la8" if count > 1 else "avx2_la8x", data=dst, tw=table, stride=n // 4, count=count, regions=4)
        tw = []
        for j in range(n >> 5):
            for l in range(1, 4):
                tw += split([root(l * (ELEMENT_OF[lane] + 8 * j), n) for lane in range(8)])
        m.store(table, tw)
        run("avx2_lb" + suffix, data=dst, tw=table, stride=2 * n, count=n >> 5, regions=1)
        return m.load(dst, 2 * n)
    tw = []
    for r in range(4):
        k_low = bit_reverse(r, 2)
        for j in range(max(1, n >> 7)):
            if log2n > 6:
                scalars = [root(8 * (k_low + 4 * k2 + 8 * j), n) for k2 in range(2)]
            else:
                scalars = [root(8 * k_low, n), 0j]
            tw += split(scalars)
            tw += split([root(4 * (k_low + 4 * (lane & 1) + 8 * j), n) for lane in range(8)])
    m.store(table, tw)
    if log2n > 6:
        run("avx2_la", data=dst, tw=table, stride=n // 2, count=n >> 7, regions=4)
    else:
        run("avx2_la2", data=dst, tw=table, stride=vector, count=1, regions=4)
    tw = []
    for j in range(n >> 5):
        for l in range(1, 4):
            tw += split([root(l * (ELEMENT_OF[lane] + 8 * j), n) for lane in range(8)])
    m.store(table, tw)
    run("avx2_lb" + suffix, data=dst, tw=table, stride=2 * n, count=n >> 5, regions=1)
    return m.load(dst, 2 * n)


def dft(x, inverse=False):
    """Reference transform (iterative radix 2, double precision)."""
    n = len(x)
    bits = n.bit_length() - 1
    a = [x[bit_reverse(i, bits)] for i in range(n)]
    size = 2
    while size <= n:
        for start in range(0, n, size):
            for k in range(size // 2):
                w = cmath.exp((2j if inverse else -2j) * math.pi * k / size)
                u, v = a[start + k], a[start + k + size // 2] * w
                a[start + k], a[start + k + size // 2] = u + v, u - v
        size *= 2
    return a
