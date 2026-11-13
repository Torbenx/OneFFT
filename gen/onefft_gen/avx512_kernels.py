"""The x86-64 kernels for AVX-512.

A register has 16 lanes, so four index bits sit in the lanes, and there are 32 registers, so a
kernel can hold 8 complex vectors with room to spare. The kernels:

* `f16`   first pass, four index bits: 16 complex vectors in all 32 registers.
* `f8`    first pass, three index bits.
* `c8`, `c4`  middle passes, three and two index bits (`c4` is the kernel of
          x86_kernels.middle_pass).
* `la3`/`la2`  pass before the last: index bit 4 across the registers (`la3` only), then bits 3
          and 2, each after an exchange that moves it from the lanes to the registers.
* `lb`    last pass: index bits 1 and 0 after their exchanges; writes the interleaved result.
* `n32`, `n64`, `n128`  complete transforms in registers.

What the kernels were tuned for (docs/measurements.md has the numbers, from Zen 4 and Zen 5;
on Intel's Cascade Lake, which runs all 512 bit arithmetic and shuffles on two units, none of
it makes a difference):

* Loads are scarce: two per cycle on Zen 5 and one on Zen 4 at 512 bits, and a memory operand,
  also an embedded broadcast, is a load. Twiddles and constants that are used more than once
  are therefore loaded into registers.
* Additions and multiplications run on different execution units on these cores. Where a
  kernel has many more additions, some butterflies are computed as (a + b, (a + b) - 2 b)
  with a fused multiply-add (cplx.butterfly_in_place) to even them out.
* The processor's scheduler holds few 512 bit operations, so a kernel whose operations all
  wait for the same loads stalls it: the last pass is software pipelined (`pipelined_loop`).

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

import math

from .cplx import butterfly, cmul, fft_plain, root, shear
from .passes import field, last_loops, middle_loops, middle_params
from .x86 import Mem
from .x86_kernels import Kernel, four_vectors, middle_pass, name, schedule

#: ELEMENT_OF[lane] is the element (of 16 consecutive values) held by a lane.
ELEMENT_OF = tuple((lane & 1) + 2 * (lane >> 2) + 8 * ((lane >> 1) & 1) for lane in range(16))


def low_bits(region, bits=3):
    """The number formed by output bits 0..bits-1 for a region of the output.

    The regions are the 2^bits equal parts of the output; from the top, the position bits hold
    output bits 0, 2, 1 (and 3).
    """
    order = (0, 2, 1, 3)[:bits]
    return sum(((region >> (bits - 1 - i)) & 1) << k for i, k in enumerate(order))


def region_of(low, bits=4):
    """The region whose low output bits are `low`: the inverse of low_bits()."""
    return next(region for region in range(1 << bits) if low_bits(region, bits) == low)


def first_pass_8(inverse):
    k = Kernel(name("avx512", "f8", inverse),
               """First pass, radix 8. For each of the `count` entries (a, b) of `table`: reads 8 interleaved
vectors from in + 8*a + u*in_stride, transforms across them and writes output q as a split
vector to out + 8*b + r*out_stride, where q = low_bits(r).""", "avx512", inverse)
    src_base = k.param("const float *", "in")
    dst_base = k.param("float *", "out")
    table = k.param("const uint32_t *", "table")
    count = k.param("size_t", "count")
    in_stride = k.param("size_t", "in_stride")
    out_stride = k.param("size_t", "out_stride")
    src = k.gtemp("src", "const float *")
    dst = k.gtemp("dst", "float *")
    a, b = k.gtemp("a"), k.gtemp("b")
    # Butterflies of the form (a + b, (a + b) - 2 b): half of the operations are fused
    # multiply-adds, which run on other execution units than additions on AMD cores.
    consts = k.gconst("consts", "const float *", [2.0, math.sqrt(0.5), 0.0, 0.0])
    k.two, half = k.vreg(), k.vreg()
    k.broadcast(k.two, Mem(consts))
    k.broadcast(half, Mem(consts, 4))
    top = k.label()
    k.gload_pair32(a, b, table)
    k.gadd(src, src_base, a, shift=3)
    k.gadd(dst, dst_base, b, shift=3)
    xs = []
    for u in range(8):
        xs.append(k.load_interleaved(Mem(src)))
        if u < 7:
            k.gadd(src, src, in_stride)
    ys = fft_plain(k, xs, half, in_place=True)
    for r in range(8):
        k.store_split(ys[low_bits(r)], Mem(dst))
        if r < 7:
            k.gadd(dst, dst, out_stride)
    k.loop(count, top)
    k.finish()
    return k


def sixteen_vectors(k, base, stride, stride3, temps):
    """Addresses base + u*stride for u = 0..15; sets the three `temps` to base + (1..3)*stride."""
    p1, p2, p3 = temps
    k.gadd(p1, base, stride)
    k.gadd(p2, base, stride, shift=1)
    k.gadd(p3, base, stride3)
    where = [None] * 16
    for u, p in enumerate((base, p1, p2, p3)):
        where[u + 8] = Mem(p, index=stride, scale=8)
        where[u + 12] = Mem(p, index=stride3, scale=4)
    where[0], where[1], where[2], where[4] = (Mem(base), Mem(base, index=stride), Mem(base, index=stride, scale=2),
                                             Mem(base, index=stride, scale=4))
    where[3], where[6] = Mem(base, index=stride3), Mem(base, index=stride3, scale=2)
    where[5], where[7] = Mem(p1, index=stride, scale=4), Mem(p1, index=stride3, scale=2)
    return where


def first_pass_16(inverse):
    """Radix 16 in 32 registers.

    16 complex vectors occupy every register, so the transform is arranged as two layers of
    radix 4 that need all of them only for a moment: the first layer transforms four groups of
    four vectors as they are loaded, the second layer takes one result of each group and
    stores its own results at once. Butterflies compute in place (cplx.butterfly_in_place).
    Their constant 2 and the constants of the rotations are kept in registers, except where
    the last group of the first layer and the first group of the second leave none free:
    there the constant is a memory operand.
    """
    k = Kernel(name("avx512", "f16", inverse),
               """First pass, radix 16. For each of the `count` entries (a, b) of `table`: reads 16 interleaved
vectors from in + 8*a + u*in_stride, transforms across them and writes output q as a split
vector to out + 8*b + r*in_stride, where q = low_bits(r). (The strides of input and output are
the same.)""", "avx512", inverse)
    src_base = k.param("const float *", "in")
    dst_base = k.param("float *", "out")
    table = k.param("const uint32_t *", "table")
    count = k.param("size_t", "count")
    stride = k.param("size_t", "in_stride")
    k.param("size_t", "out_stride", used=False)
    p0 = k.gtemp("p0", "float *")
    temps = [k.gtemp("p%d" % i, "float *") for i in (1, 2, 3)]
    stride3, b = k.gtemp("stride3"), k.gtemp("b")
    # 2; then tan(angle / 2) and sin(angle) for the rotations by 1, 2 and 3 sixteenths of a turn
    angles = [math.pi * e / 8 for e in (1, 2, 3)]
    consts = k.gconst("consts", "const float *", [2.0] + [math.tan(x / 2) for x in angles] + [math.sin(x) for x in angles])
    k.gadd(stride3, stride, stride, shift=1)
    top = k.label()
    k.gload_pair32(p0, b, table)
    k.gadd(p0, src_base, p0, shift=3)
    src = sixteen_vectors(k, p0, stride, stride3, temps)
    two = k.vreg()
    k.broadcast(two, Mem(consts))
    # First layer: index bits n-1 and n-2 for each value g of bits n-3 and n-4.
    ys = []
    for g in range(4):
        k.two = two if g < 3 else Mem(consts, broadcast=True)
        ys.append(fft_plain(k, [k.load_interleaved(src[4 * h + g]) for h in range(4)], in_place=True))
    k.gadd(p0, dst_base, b, shift=3)
    dst = sixteen_vectors(k, p0, stride, stride3, temps)
    # Second layer: bits n-3 and n-4 for each value q of output bits 1 and 0, whose inputs
    # are turned by multiples of q sixteenths.
    for q in range(4):
        zs = [ys[g][q] for g in range(4)]
        if q == 1:
            k.two = k.vreg()
            k.broadcast(k.two, Mem(consts))
            rotation = [None]
            for e in (1, 2, 3):
                rotation.append((k.vreg(), k.vreg()))
                k.broadcast(rotation[e][0], Mem(consts, 4 * e))
                k.broadcast(rotation[e][1], Mem(consts, 12 + 4 * e))
        for g in range(1, 4):
            quarters, sixteenths = divmod(q * g, 4)
            if sixteenths:
                zs[g] = shear(k, zs[g], *rotation[sixteenths])
            zs[g] = zs[g].rotated(quarters)
        out = fft_plain(k, zs, in_place=q < 3)  # the last group has registers for plain butterflies
        for r in range(4):
            k.store_split(out[r], dst[region_of(q + 4 * r)])
    k.loop(count, top)
    k.finish()
    return k


def middle_pass_8():
    k = Kernel(name("avx512", "c8"),
               """Middle pass, radix 8, in place on split vectors. `data` holds `regions` regions of `blocks`
blocks. Input i of butterfly j of a block is the vector at (i/4)*args.s_high + (i/2%2)*args.s_low
+ (i%2)*2*args.s_low + 128*j, j < args.inner; see passes.py for how the blocks and regions follow
each other. Butterfly j multiplies input i by w_j^i and transforms across the vectors; output q
goes to (q%2)*s_high + (q/2%2)*s_low + (q/4)*2*s_low. The table `tw` holds per butterfly the real
parts of w^1..w^7 and one unused float, then the imaginary parts.""", "avx512")
    data, tw_base, args, blocks, regions = middle_params(k)
    upper = k.gtemp("upper", "float *")
    s_low, s_low3 = k.gtemp("s_low"), k.gtemp("s_low3")
    k.gload(s_low, args, field("s_high"))
    k.gadd(upper, data, s_low)
    k.gload(s_low, args, field("s_low"))
    k.gadd(s_low3, s_low, s_low, shift=1)
    consts = k.gconst("consts", "const float *", [2.0, math.sqrt(0.5), 0.0, 0.0])
    k.two, half = k.vreg(), k.vreg()
    k.broadcast(k.two, Mem(consts))
    k.broadcast(half, Mem(consts, 4))
    # by (bit for s_high, bit for s_low, bit for 2 s_low)
    where = {(0, 0, 0): Mem(data), (0, 1, 0): Mem(data, index=s_low), (0, 0, 1): Mem(data, index=s_low, scale=2),
             (0, 1, 1): Mem(data, index=s_low3), (1, 0, 0): Mem(upper), (1, 1, 0): Mem(upper, index=s_low),
             (1, 0, 1): Mem(upper, index=s_low, scale=2), (1, 1, 1): Mem(upper, index=s_low3)}

    def inner_loops(tw, j, scratch):
        k.gload(j, args, field("inner"))
        top = k.label()
        xs = [k.load_split(where[i >> 2, (i >> 1) & 1, i & 1]) for i in range(8)]
        for i in range(1, 8):
            xs[i] = k.twiddle_scalar(xs[i], tw, 4 * (i - 1), 32 + 4 * (i - 1))
        # One of the three stages with fused butterflies: 44 additions and 40 multiplications
        # (or multiply-adds) instead of 52 and 32.
        ys = fft_plain(k, xs, half, fused_stages=(1,))
        for q in range(8):
            k.store_split(ys[q], where[q & 1, (q >> 1) & 1, q >> 2])
        k.gadd(data, data, k.vector)
        k.gadd(upper, upper, k.vector)
        k.gadd(tw, tw, 64)
        k.loop(j, top)

    middle_loops(k, [data, upper], tw_base, args, blocks, regions, inner_loops)
    k.finish()
    return k


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
        w3 = [k.twiddle_operands(tw, offset + 128 * k1) for k1 in range(2)]
        for c in range(2 if three else 1):
            z = [None, None]  # z[output bit 1][the output bit that index bit 3 gives]
            for k1 in range(2):
                # index bit 3 (lane b1) <-> output bit 3
                lo, hi = k.exchange_b1_to_b0(y[k1][0][c], y[k1][1][c])
                z[k1] = butterfly(k, lo, cmul(k, hi, *w3[k1]).rotated(c))
            w2 = k.twiddle_operands(tw, offset + 256 + 128 * c)
            for d in range(2):
                # index bit 2 (lane q1) <-> output bit 1
                lo, hi = k.exchange_q1(z[0][d], z[1][d])
                out[0, d, c], out[1, d, c] = butterfly(k, lo, cmul(k, hi, *w2).rotated(d))
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
    """The loop is software pipelined: an iteration finishes one butterfly (the transform across
    the registers and the stores) while it begins the next (loads, exchanges, twiddles). The
    instructions of both are interleaved by the scheduler. Without this, everything that a
    butterfly consists of waits in the processor's scheduler for the same loads, and the
    scheduler is too small for that: 15% on Zen 4 and Zen 5."""
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

    def begin():
        xs = [k.load_split(m) for m in where]  # region r holds output bits (0, 2) = (r/2, r%2)
        # index bit 1 (lane q0) <-> output bit 2
        u = [k.exchange_q0_to_q1(xs[2 * k0], xs[2 * k0 + 1]) for k0 in range(2)]  # u[output bit 0][index bit 1]
        # index bit 0 (lane b1 since `la`) <-> output bit 0
        t = [k.exchange_b1_to_b0(u[0][b1], u[1][b1]) for b1 in range(2)]  # t[index bit 1][index bit 0]
        zs = [t[l >> 1][l & 1] for l in range(4)]
        for l in range(1, 4):
            zs[l] = k.twiddle_vector(zs[l], tw, 128 * (l - 1))
        k.gadd(tw, tw, 384)
        return zs

    def finish(zs, where):
        ys = fft_plain(k, zs)
        for q in range(4):
            k.store_interleaved(ys[q], where[q])

    pipelined_loop(k, count, [data, third], begin, lambda zs: finish(zs, [m.shifted(-k.vector) for m in where]),
                   lambda zs: finish(zs, where))
    k.finish()
    return k


def pipelined_loop(k, count, pointers, begin, finish, finish_last):
    """Emits a loop over `count` butterflies in which an iteration finishes one butterfly and
    begins the next.

    `begin()` emits the first part of a butterfly and returns its complex vectors;
    `finish(vectors)` emits the rest for the previous butterfly, whose data is one vector
    before the `pointers` by then, and `finish_last(vectors)` the same after the loop.
    """
    carried = begin()
    last = k.forward_label()
    k.skip_if_last(count, last)
    top = k.label()
    for p in pointers:
        k.gadd(p, p, k.vector)
    following = begin()
    finish(carried)
    for x, y in zip(carried, following):
        assert x.rot == y.rot
        k.mov(x.re, y.re)
        k.mov(x.im, y.im)
    k.loop(count, top)
    k.place(last)
    finish_last(carried)


#: The lane exchanges of a complete transform in registers, in the order of the stages: the
#: index bit, the exchange, the lane bit that the index bit leaves, and the lane bits afterwards
#: as a function of the lane bits before (P is the bit that comes in from the register index).
#: Found by searching all sequences of single shuffles: it is one of two that end with output
#: bits 0..3 in the lane order of the interleaved result, and it needs the partners in this order.
CODELET_STAGES = (
    (3, "exchange_b1_to_b0", "b1", {"b0": "P", "b1": "b0"}),
    (2, "exchange_q1", "q1", {"q1": "P"}),
    (1, "exchange_q0_to_q1", "q0", {"q1": "P", "q0": "q1"}),
    (0, "exchange_b1", "b1", {"b1": "P"}),
)
LANE_BIT = {"b0": 0, "b1": 1, "q0": 2, "q1": 3}  # the bit of the lane number that addresses each lane bit


def codelet(n, inverse):
    """Complete transform of 32, 64 or 128 points in 2, 4 or 8 registers.

    The index bits above the lanes are transformed across the registers. Then each of the four
    index bits in the lanes is exchanged with an output bit in the register index and
    transformed: index bit 3 with output bit 0, bit 2 with 1, bit 1 with 2 and bit 0 with
    output bit 3, which is itself the result of index bit 3 for 32 points, and so on.

    A register is identified by the output bits that select it, as (bit number, value) pairs.
    """
    log2n = n.bit_length() - 1
    k = Kernel(name("avx512", "n%d" % n, inverse),
               "Complete %d point transform from `in` to `out`, both interleaved." % n, "avx512", inverse)
    src = k.param("const float *", "in")
    dst = k.param("float *", "out")
    consts = [math.sqrt(0.5)] * 16
    table = k.gconst("consts", "const float *", consts)
    vectors = n // 16
    half = None
    if vectors == 8:
        half = k.vreg()
        k.broadcast(half, Mem(table))
    ys = fft_plain(k, [k.load_interleaved(Mem(src, 128 * u)) for u in range(vectors)], half)
    known = log2n - 4  # output bits that exist so far
    regs = {tuple((t, (q >> t) & 1) for t in range(known)): y for q, y in enumerate(ys)}
    lanes = {"b0": ("index", 0), "q0": ("index", 1), "q1": ("index", 2), "b1": ("index", 3)}

    for partner, (j, exchange, leaves, moves) in enumerate(CODELET_STAGES):
        assert lanes[leaves] == ("index", j)
        before = dict(lanes)
        for slot, source in moves.items():
            lanes[slot] = ("output", partner) if source == "P" else before[source]
        loaded = {}  # twiddle vectors of this stage, by their phases

        def twiddled(x, phases):
            # Vectors that differ by quarter turns share a table entry: those are free.
            quarters = phases[0] * 4 // n
            phases = tuple((p - quarters * n // 4) % n for p in phases)
            if phases not in loaded:
                offset = 4 * len(consts)
                w = [complex(root(p, n)) for p in phases]
                consts.extend([z.real for z in w] + [z.imag for z in w])
                loaded[phases] = k.vreg(), k.vreg()
                k.load(loaded[phases][0], Mem(table, offset))
                k.load(loaded[phases][1], Mem(table, offset + 64))
            return cmul(k, x, *loaded[phases]).rotated(quarters)

        result = {}
        for key in sorted(regs):
            if (partner, 0) not in key:
                continue
            others = tuple(item for item in key if item[0] != partner)
            lo, hi = getattr(k, exchange)(regs[key], regs[tuple(sorted(others + ((partner, 1),)))])
            # The twiddle of index bit j is the root of unity with the phase K << j, where K is
            # the number that the output bits known so far form: those of the register and
            # those in the lanes.
            phases = []
            for lane in range(16):
                low = sum(bit << t for t, bit in others)
                low += sum(((lane >> LANE_BIT[slot]) & 1) << t for slot, (kind, t) in lanes.items() if kind == "output")
                phases.append((low << j) % n)
            lo, hi = butterfly(k, lo, twiddled(hi, phases))
            result[tuple(sorted(others + ((known, 0),)))] = lo
            result[tuple(sorted(others + ((known, 1),)))] = hi
        regs = result
        known += 1
    assert lanes == {"b0": ("output", 0), "q0": ("output", 1), "q1": ("output", 2), "b1": ("output", 3)}
    for key, x in sorted(regs.items()):
        k.store_interleaved(x, Mem(dst, 128 * sum(bit << (t - 4) for t, bit in key)))
    k.finish()
    return k


def all_kernels():
    kernels = []
    for inverse in (False, True):
        kernels.append(first_pass_8(inverse))
        kernels.append(first_pass_16(inverse))
        kernels.append(last_pass_b(inverse))
        for n in (32, 64, 128):
            kernels.append(codelet(n, inverse))
    kernels.append(middle_pass("avx512"))
    kernels.append(middle_pass_8())
    kernels.append(last_pass_a(True))
    kernels.append(last_pass_a(False))
    for k in kernels:
        schedule(k)
    return kernels
