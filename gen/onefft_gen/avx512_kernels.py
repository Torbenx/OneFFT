"""The x86-64 kernels for AVX-512.

Status: correct (tested in simulation and under an emulator), not tuned on hardware. Only
transforms of 512 points and more use these kernels; smaller ones use the AVX2 kernels.

A register has 16 lanes, so four index bits sit in the lanes, and there are 32 registers, so a
kernel can hold 8 complex vectors with room to spare. Scalar twiddles are embedded broadcast
operands. The passes:

* `f8`    first pass, three index bits.
* `c4`    middle pass, two index bits (the kernel of x86_kernels.middle_pass).
* `la3`/`la2`  pass before the last: index bit 4 across the registers (`la3` only), then bits 3
          and 2, each after an exchange that moves it from the lanes to the registers.
* `lb`    last pass: index bits 1 and 0 after their exchanges; writes the interleaved result.

Lane order. A lane is addressed by four bits: `q1`, `q0` (which of the four 128 bit blocks) and
`b1`, `b0` (the position in the block). Splitting interleaved data with one shuffle per register
puts element e of 16 consecutive values into the lane with b0 + 2*(2*q1 + q0) + 8*b1 = e, and
that is also the order that interleaves again with one shuffle per register. Each of the four
exchanges is one shuffle per register, too, which fixes what has to be where:

    index bit 3 (lane b1) <-> output bit 3      la, unpcklps/unpckhps: goes to b0, b0 to b1
    index bit 2 (lane q1) <-> output bit 1      la, vshuff32x4: stays at q1
    index bit 1 (lane q0) <-> output bit 2      lb, vshuff32x4: goes to q1, q1 to q0
    index bit 0 (lane b1) <-> output bit 0      lb, unpcklps/unpckhps: goes to b0, b0 to b1

The result of an index bit lands where its exchange partner was. For the output to be in
natural order the top four positions of the output must therefore hold, from the top, output
bits 0, 2, 1 and 3 before the last passes: see `low_bits`.
"""

from . import cplx
from .cplx import Cx, butterfly, fft_plain
from .passes import last_loops
from .x86 import Mem
from .x86_kernels import Kernel, first_pass, four_vectors, middle_pass, name, schedule

#: ELEMENT_OF[lane] is the element (of 16 consecutive values) held by a lane.
ELEMENT_OF = tuple((lane & 1) + 2 * (lane >> 2) + 8 * ((lane >> 1) & 1) for lane in range(16))


def low_bits(region, bits=3):
    """The number formed by output bits 0..bits-1 for a region of the output.

    The regions are the 2^bits equal parts of the output; from the top, the position bits hold
    output bits 0, 2, 1 (and 3).
    """
    order = (0, 2, 1, 3)[:bits]
    return sum(((region >> (bits - 1 - i)) & 1) << k for i, k in enumerate(order))


def last_pass_a(three):
    vectors = 8 if three else 4
    k = Kernel(name("avx512", "la3" if three else "la2"),
               """Pass before the last, in place on split vectors. Butterfly j < count of a region uses the
vectors at data + u*stride + 128*j, u < %d; the `regions` regions follow each other. %sIndex bits 3
and 2 are moved from the lanes to the registers and transformed. See the table builder in
onefft.c for the twiddle layout.""" % (vectors, "Index bit 4 selects u%2 and is transformed first. " if three else ""),
               "avx512")
    data = k.param("float *", "data")
    tw = k.param("const float *", "tw")
    stride = k.param("size_t", "stride")
    count = k.param("size_t", "count")
    regions = k.param("size_t", "regions")
    third, where = four_vectors(k, data, stride)
    pointers = [data, third]
    region_skip = k.gtemp("region_skip")
    if three:
        fifth = k.gtemp("fifth", "float *")
        k.gadd(fifth, data, stride, shift=2)
        k.gadd(fifth, fifth, stride)
        where += [Mem(data, index=stride, scale=4), Mem(fifth), Mem(fifth, index=stride), Mem(third, index=stride, scale=4)]
        pointers.append(fifth)
        k.glsl(region_skip, stride, 3)
        k.gsub(region_skip, region_skip, stride)
    else:
        k.gadd(region_skip, stride, stride, shift=1)

    def inner_loops(j):
        k.gmov(j, count)
        top = k.label()
        xs = [k.load_split(m) for m in where]
        offset = 0
        # y[output bit 1][output bit 3][c]; c is the output bit that index bit 4 gives
        if three:
            y = [[None, None], [None, None]]
            for k1 in range(2):
                for k3 in range(2):
                    s = 2 * k1 + k3
                    t = k.twiddle_scalar(xs[2 * s + 1], tw, 4 * s, 16 + 4 * s)
                    y[k1][k3] = butterfly(k, xs[2 * s], t)
            offset = 32
        else:
            y = [[(xs[2 * k1 + k3],) for k3 in range(2)] for k1 in range(2)]
        out = {}
        for c in range(2 if three else 1):
            z = [None, None]  # z[output bit 1][the output bit that index bit 3 gives]
            for k1 in range(2):
                # index bit 3 (lane b1) <-> output bit 3
                lo, hi = k.exchange_b1_to_b0(y[k1][0][c], y[k1][1][c])
                hi = k.twiddle_vector(hi, tw, offset + 128 * k1).rotated(c)
                z[k1] = butterfly(k, lo, hi)
            for d in range(2):
                # index bit 2 (lane q1) <-> output bit 1
                lo, hi = k.exchange_q1(z[0][d], z[1][d])
                hi = k.twiddle_vector(hi, tw, offset + 256 + 128 * c).rotated(d)
                out[0, d, c], out[1, d, c] = butterfly(k, lo, hi)
        for (e, d, c), x in sorted(out.items()):
            k.store_split(x, where[4 * e + 2 * d + c if three else 2 * e + d])
        for p in pointers:
            k.gadd(p, p, k.vector)
        k.gadd(tw, tw, offset + (512 if three else 384))
        k.loop(j, top)

    last_loops(k, pointers, count, regions, region_skip, inner_loops)
    k.finish()
    return k


def last_pass_b(inverse):
    k = Kernel(name("avx512", "lb", inverse),
               """Last pass, in place: reads split vectors and writes the interleaved result. Butterfly
j < count uses the vectors at data + r*stride + 128*j. Index bits 1 and 0 are moved from the lanes
to the registers and transformed. The table holds per butterfly, for l = 1..3, the real and the
imaginary parts of the 16 twiddles of the vector with index bits (1, 0) = l.""", "avx512", inverse)
    data = k.param("float *", "data")
    tw = k.param("const float *", "tw")
    stride = k.param("size_t", "stride")
    count = k.param("size_t", "count")
    k.param("size_t", "regions", used=False)  # all last pass kernels share a signature
    third, where = four_vectors(k, data, stride)
    top = k.label()
    xs = [k.load_split(m) for m in where]  # region r holds output bits (0, 2) = (r/2, r%2)
    # index bit 1 (lane q0) <-> output bit 2
    u = [k.exchange_q0_to_q1(xs[2 * k0], xs[2 * k0 + 1]) for k0 in range(2)]  # u[output bit 0][index bit 1]
    # index bit 0 (lane b1 since `la`) <-> output bit 0
    t = [k.exchange_b1_to_b0(u[0][b1], u[1][b1]) for b1 in range(2)]  # t[index bit 1][index bit 0]
    zs = [t[l >> 1][l & 1] for l in range(4)]
    for l in range(1, 4):
        zs[l] = k.twiddle_vector(zs[l], tw, 128 * (l - 1))
    ys = fft_plain(k, zs)
    for q in range(4):
        k.store_interleaved(ys[q], where[q])
    k.gadd(data, data, k.vector)
    k.gadd(third, third, k.vector)
    k.gadd(tw, tw, 384)
    k.loop(count, top)
    k.finish()
    return k


def all_kernels():
    kernels = []
    for inverse in (False, True):
        kernels.append(first_pass("avx512", 8, inverse, region_bits=low_bits))
        kernels.append(last_pass_b(inverse))
    kernels.append(middle_pass("avx512"))
    kernels.append(last_pass_a(True))
    kernels.append(last_pass_a(False))
    for k in kernels:
        schedule(k)
    return kernels
