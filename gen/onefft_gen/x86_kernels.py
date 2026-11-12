"""The x86-64 kernels for SSE2 and AVX2.

See docs/design.md for the algorithm. With 16 registers a kernel holds 4 complex vectors, so the
passes are radix 4:

* `f4`/`f2`  first pass. Reads the interleaved input, transforms the top two (one) index bits
             across the registers and writes split vectors to their final position.
* AVX2 `f8`  first pass for three index bits, used instead of `f2`. Its 8 complex vectors fill
             all 16 registers, so it computes in place (cplx.butterfly_in_place).
* `c4`       middle pass, in place on split vectors: one scalar twiddle per register, then a
             plain FFT across the 4 registers.
* SSE2 `l4`  last pass: transposes the two index bits in the lanes into the registers,
             transforms them and writes the interleaved result.
* AVX2 `la`, `lb`  the three index bits in the lanes take two passes. `la` transforms index
             bits 3 and 2, `lb` bits 1 and 0 and writes the interleaved result. `la2` replaces
             `la` for 64 points, where bit 3 has no partner.
* AVX2 `la8`  replaces `la` for an even number of index bits and transforms bits 4, 3 and 2.
             Like `f8` it fills all registers and computes in place; its twiddles are applied
             as shears (cplx.butterfly_shear) with table entries as memory operands. `la8x` is
             the variant for 256 points.
* `n8`, `n16`, `n32`  complete transforms in registers.

The `i` variants compute the inverse transform by exchanging real and imaginary parts when
reading the input and writing the output.

AVX2 lane order. Shuffles work within the 128 bit halves of a register, so the cheapest way to
split interleaved data puts element e of 8 consecutive values into lane LANE_OF[e] (elements
0 1 4 5 in the lower half). The same order is the cheapest to interleave again. A lane is
addressed by three bits: `h` (the half), `b1` and `b0` (the position in the half).
"""

import math
import os

from . import cplx
from .cplx import Cx, bit_reverse, butterfly, butterfly_shear, cmul, fft_plain, fft_staged
from .ir import AllocationError
from .passes import field, last_loops, middle_loops, middle_params
from .x86 import X86, Mem

#: Code generation choices, see neon_kernels.OPTIONS. Override with ONEFFT_GEN_X86.
OPTIONS = {
    "schedule": 3,    # reorder instructions for a core running this many SIMD operations per cycle
    "fp_latency": 4,  # latency assumed for floating point operations when scheduling
    "window": 64,     # how far instructions may be moved ahead when scheduling
}
for _item in filter(None, os.environ.get("ONEFFT_GEN_X86", "").split(",")):
    _key, _value = _item.split("=")
    assert _key in OPTIONS, _key
    OPTIONS[_key] = int(_value)

#: AVX2: ELEMENT_OF[lane] is the element (of 8 consecutive values) held by a lane.
ELEMENT_OF = (0, 1, 4, 5, 2, 3, 6, 7)


class Kernel(X86):
    def __init__(self, name, doc, isa, inverse=False):
        super().__init__(name, doc, isa)
        self.inverse = inverse
        self.vector = 2 * self.bytes  # bytes of one split complex vector

    def load_interleaved(self, mem):
        """Reads `lanes` interleaved values. AVX2 leaves them in the lane order ELEMENT_OF."""
        x = Cx(*self.cx())
        a = self.vreg()
        self.load(a, mem)
        if self.vex:
            # The second half as a memory operand of the shuffles: one instruction less. (The
            # two-operand SSE2 form would need it aligned, which the input need not be.)
            b = mem.shifted(self.bytes)
        else:
            b = self.vreg()
            self.load(b, mem.shifted(self.bytes))
        self.shufps(x.im if self.inverse else x.re, a, b, 0x88)
        self.shufps(x.re if self.inverse else x.im, a, b, 0xdd)
        return x

    def store_interleaved(self, x, mem):
        assert x.rot == 0
        a, b = self.vreg(), self.vreg()
        even, odd = (x.im, x.re) if self.inverse else (x.re, x.im)
        self.unpcklps(a, even, odd)
        self.unpckhps(b, even, odd)
        self.store(a, mem)
        self.store(b, mem.shifted(self.bytes))

    def load_split(self, mem):
        x = Cx(*self.cx())
        self.load(x.re, mem)
        self.load(x.im, mem.shifted(self.bytes))
        return x

    def store_split(self, x, mem):
        assert x.rot == 0
        self.store(x.re, mem)
        self.store(x.im, mem.shifted(self.bytes))

    def exchange(self, a, b, low, high):
        """Applies two shuffles to both parts of two complex vectors.

        Used to exchange one lane index bit with the register index: `low` collects the
        elements of a and b where that bit is clear, `high` those where it is set.
        """
        assert a.rot == 0 and b.rot == 0
        lo, hi = Cx(*self.cx()), Cx(*self.cx())
        for part in ("re", "im"):
            low(getattr(lo, part), getattr(a, part), getattr(b, part))
            high(getattr(hi, part), getattr(a, part), getattr(b, part))
        return lo, hi

    # The three exchanges of AVX2. Each names the lane bit that leaves, and where the register
    # index goes: see the table in docs/design.md.

    def exchange_b1_to_b0(self, a, b):
        """Lane bit b1 leaves; the register index becomes b0 and the old b0 becomes b1."""
        return self.exchange(a, b, self.unpcklps, self.unpckhps)

    def exchange_b1(self, a, b):
        """Lane bit b1 leaves; the register index takes its place."""
        return self.exchange(a, b, self.unpcklpd, self.unpckhpd)

    def exchange_b0_to_b1(self, a, b):
        """Lane bit b0 leaves; the register index becomes b1 and the old b1 becomes b0."""
        return self.exchange(a, b, lambda d, x, y: self.shufps(d, x, y, 0x88),
                             lambda d, x, y: self.shufps(d, x, y, 0xdd))

    def exchange_q1(self, a, b):
        """AVX-512: lane bit q1 (the upper two of the four 128 bit blocks) leaves; the register
        index takes its place."""
        return self.exchange(a, b, lambda d, x, y: self.shuff32x4(d, x, y, 0x44),
                             lambda d, x, y: self.shuff32x4(d, x, y, 0xee))

    def exchange_q0_to_q1(self, a, b):
        """AVX-512: lane bit q0 (the odd blocks) leaves; the register index becomes q1 and the
        old q1 becomes q0."""
        return self.exchange(a, b, lambda d, x, y: self.shuff32x4(d, x, y, 0x88),
                             lambda d, x, y: self.shuff32x4(d, x, y, 0xdd))

    def exchange_h(self, a, b):
        """Lane bit h (the 128 bit half) leaves; the register index takes its place."""
        return self.exchange(a, b, lambda d, x, y: self.perm2f128(d, x, y, 0x20),
                             lambda d, x, y: self.perm2f128(d, x, y, 0x31))

    def transpose(self, xs):
        """SSE2: exchanges the roles of lanes and registers: result[l] lane t = xs[t] lane l."""
        assert self.isa == "sse2"
        lo01, hi01 = self.exchange_b1_to_b0(xs[0], xs[1])
        lo23, hi23 = self.exchange_b1_to_b0(xs[2], xs[3])
        t0, t1 = self.exchange_b1(lo01, lo23)
        t2, t3 = self.exchange_b1(hi01, hi23)
        return [t0, t1, t2, t3]

    def twiddle_vector(self, x, table, offset):
        """Multiplies by the complex vector stored at table + offset (real parts, then imaginary)."""
        return cmul(self, x, Mem(table, offset), Mem(table, offset + self.bytes))

    def twiddle_scalar(self, x, table, re_offset, im_offset):
        """Multiplies all lanes by the complex number whose parts are stored at the offsets."""
        return cmul(self, x, self.multiplier(Mem(table, re_offset)), self.multiplier(Mem(table, im_offset)))


def name(isa, kernel, inverse=False):
    return "onefft_%s_%s%s" % (isa, kernel, "i" * inverse)


def first_pass(isa, radix, inverse, region_bits=None):
    k = Kernel(name(isa, "f%d" % radix, inverse),
               """First pass, radix %d. For each of the `count` entries (a, b) of `table`: reads %d interleaved
vectors from in + 8*a + u*in_stride, transforms across them and writes output q as a split
vector to out + 8*b + r*out_stride, where r is q with its bits reversed%s.""" % (
                   radix, radix, "" if region_bits is None else " (and the lower two exchanged)"), isa, inverse)
    src_base = k.param("const float *", "in")
    dst_base = k.param("float *", "out")
    table = k.param("const uint32_t *", "table")
    count = k.param("size_t", "count")
    in_stride = k.param("size_t", "in_stride")
    out_stride = k.param("size_t", "out_stride")
    src = k.gtemp("src", "const float *")
    dst = k.gtemp("dst", "float *")
    a, b = k.gtemp("a"), k.gtemp("b")
    half = None
    in_place = radix == 8 and isa == "avx2"
    if in_place:
        # 8 complex vectors occupy all 16 registers: the transform runs in place, with its two
        # constants as memory operands.
        consts = k.gconst("consts", "const float *", [2.0] * k.lanes + [math.sqrt(0.5)] * k.lanes)
        k.two = Mem(consts)
        half = Mem(consts, k.bytes)
    elif radix == 8:
        consts = k.gconst("consts", "const float *", [math.sqrt(0.5)] * 4)
        half = k.multiplier(Mem(consts))
    top = k.label()
    k.gload_pair32(a, b, table)
    k.gadd(src, src_base, a, shift=3)
    k.gadd(dst, dst_base, b, shift=3)
    xs = []
    for u in range(radix):
        xs.append(k.load_interleaved(Mem(src)))
        if u < radix - 1:
            k.gadd(src, src, in_stride)
    ys = fft_plain(k, xs, half, in_place=in_place)
    bits = radix.bit_length() - 1
    for r in range(radix):
        # Region r of the output takes the result whose low output bits, reversed, are r
        # (AVX-512 orders the regions differently, see avx512_kernels.low_bits).
        q = bit_reverse(r, bits) if region_bits is None else region_bits(r)
        k.store_split(ys[q], Mem(dst))
        if r < radix - 1:
            k.gadd(dst, dst, out_stride)
    k.loop(count, top)
    k.finish()
    return k


def middle_pass(isa):
    k = Kernel(name(isa, "c4"),
               """Middle pass, radix 4, in place on split vectors. `data` holds `regions` regions of `blocks`
blocks. Input i of butterfly j of a block is the vector at (i/2)*args.s_high + (i%2)*args.s_low
+ j*V (V = bytes per vector), j < args.inner; see passes.py for how the blocks and regions follow
each other. Butterfly j multiplies input i by w_j^i and transforms across the vectors; output q
goes to (q%2)*s_high + (q/2)*s_low. The table `tw` holds per butterfly the real parts of w^1..w^3
and one unused float, then the imaginary parts.""", isa)
    data, tw_base, args, blocks, regions = middle_params(k)
    upper = k.gtemp("upper", "float *")
    s_low = k.gtemp("s_low")
    k.gload(s_low, args, field("s_high"))
    k.gadd(upper, data, s_low)
    k.gload(s_low, args, field("s_low"))

    def inner_loops(tw, j, scratch):
        k.gload(j, args, field("inner"))
        top = k.label()
        where = [Mem(data), Mem(data, index=s_low), Mem(upper), Mem(upper, index=s_low)]
        xs = [k.load_split(m) for m in where]
        for i in range(1, 4):
            xs[i] = k.twiddle_scalar(xs[i], tw, 4 * (i - 1), 16 + 4 * (i - 1))
        ys = fft_plain(k, xs)
        for q in range(4):
            k.store_split(ys[q], where[(q & 1) * 2 + (q >> 1)])
        k.gadd(data, data, k.vector)
        k.gadd(upper, upper, k.vector)
        k.gadd(tw, tw, 32)
        k.loop(j, top)

    middle_loops(k, [data, upper], tw_base, args, blocks, regions, inner_loops)
    k.finish()
    return k


def four_vectors(k, data, stride):
    """Addresses data + u*stride for u = 0..3."""
    third = k.gtemp("third", "float *")
    k.gadd(third, data, stride, shift=1)
    k.gadd(third, third, stride)
    return third, [Mem(data), Mem(data, index=stride), Mem(data, index=stride, scale=2), Mem(third)]


def last_pass_sse2(inverse):
    k = Kernel(name("sse2", "l4", inverse),
               """Last pass, radix 4, in place: reads split vectors and writes the interleaved result.
Butterfly j < count uses the vectors at data + r*stride + 32*j. The table holds per butterfly,
for l = 1..3, the real and the imaginary parts of the 4 twiddles of the vector with index bits
(1, 0) = l.""", "sse2", inverse)
    data = k.param("float *", "data")
    tw = k.param("const float *", "tw")
    stride = k.param("size_t", "stride")
    count = k.param("size_t", "count")
    k.param("size_t", "regions", used=False)  # all last pass kernels share a signature
    third, where = four_vectors(k, data, stride)
    top = k.label()
    xs = [k.load_split(m) for m in where]
    # Region r holds output bits (0, 1) = (r/2, r%2); the lanes hold index bits 1 and 0.
    zs = k.transpose([xs[bit_reverse(t, 2)] for t in range(4)])
    for l in range(1, 4):
        zs[l] = k.twiddle_vector(zs[l], tw, 32 * (l - 1))
    ys = fft_plain(k, zs)
    for q in range(4):
        k.store_interleaved(ys[q], where[q])
    k.gadd(data, data, k.vector)
    k.gadd(third, third, k.vector)
    k.gadd(tw, tw, 96)
    k.loop(count, top)
    k.finish()
    return k


def last_pass_a(pairs):
    """AVX2: index bits 3 and 2. `pairs` False gives the variant for 64 points."""
    k = Kernel(name("avx2", "la" if pairs else "la2"),
               """Second to last pass, in place on split vectors. Butterfly j < count uses the vectors at
data + u*stride + 64*j, u < %d, for each of the `regions` regions that follow each other. %sThen index bit 2 is moved from the lanes to the registers and
transformed. The table holds per butterfly 4 floats (real parts of the scalar twiddles for
u/2 = 0 and 1, then their imaginary parts) and the real and imaginary parts of the twiddle
vector of index bit 2.""" % (4 if pairs else 2, "Index bit 3 selects u%2 and is transformed first. " if pairs else
                           "The two vectors are the two values of index bit 3, which is transformed first. "),
               "avx2")
    data = k.param("float *", "data")
    tw = k.param("const float *", "tw")
    stride = k.param("size_t", "stride")
    count = k.param("size_t", "count")
    regions = k.param("size_t", "regions")
    pointers = [data]
    if pairs:
        third, where = four_vectors(k, data, stride)
        pointers.append(third)
    else:
        where = [Mem(data), Mem(data, index=stride)]
    # The regions follow each other: after `count` butterflies the first vector of the next
    # region is where the second vector of this one began, plus the vectors in between.
    region_skip = k.gtemp("region_skip")
    k.gadd(region_skip, stride, stride, shift=1) if pairs else k.gmov(region_skip, stride)
    last_loops(k, pointers, count, regions, region_skip, lambda j: butterflies(k, j, count, tw, pairs, where, pointers))
    k.finish()
    return k


def last_pass_a8(halved):
    """AVX2: index bits 4, 3 and 2 in one pass, in place in all 16 registers."""
    k = Kernel(name("avx2", "la8" if halved else "la8x"),
               """Second to last pass for three index bits, in place on split vectors. Butterfly j < count
of a region uses the vectors at data + u*stride + 64*j, u < 8; the `regions` regions follow each
other. Index bit 2 is moved from the lanes to the registers while loading. Then index bits 4, 3
and 2 are transformed, each stage with twiddles that differ between the lanes: the table holds
per butterfly 4 twiddle vectors as tan(angle/2) and sin(angle), see the table builder in
onefft.c.%s""" % ("" if halved else " This variant is for a single butterfly per region (256 points)."), "avx2")
    data = k.param("float *", "data")
    tw = k.param("const float *", "tw")
    stride = k.param("size_t", "stride")
    count = k.param("size_t", "count")
    regions = k.param("size_t", "regions")
    consts = k.gconst("consts", "const float *", [2.0] * k.lanes)
    k.two = Mem(consts)
    third, fifth = k.gtemp("third", "float *"), k.gtemp("fifth", "float *")
    k.gadd(third, data, stride, shift=1)
    k.gadd(third, third, stride)
    k.gadd(fifth, data, stride, shift=2)
    k.gadd(fifth, fifth, stride)
    where = [Mem(data), Mem(data, index=stride), Mem(data, index=stride, scale=2), Mem(third),
             Mem(data, index=stride, scale=4), Mem(fifth), Mem(fifth, index=stride), Mem(third, index=stride, scale=4)]
    pointers = [data, third, fifth]
    region_skip = k.gtemp("region_skip")
    k.glsl(region_skip, stride, 3)
    k.gsub(region_skip, region_skip, stride)

    def body(upper):
        # Vectors u and u + 4 differ in output bit 2. Exchanging it with index bit 2 (lane bit
        # b1) needs only shuffles with the second vector as a memory operand: two registers in,
        # two out.
        xs = [None] * 8
        for u in range(4):
            lo, hi = Cx(k.vreg(), k.vreg()), Cx(k.vreg(), k.vreg())
            for part, offset in (("re", 0), ("im", k.bytes)):
                first = getattr(hi, part)
                k.load(first, where[u].shifted(offset))
                k.unpcklps(getattr(lo, part), first, where[u + 4].shifted(offset))
                k.unpckhps(first, first, where[u + 4].shifted(offset))
            # u = 2 * (index bit 3) + (index bit 4); the transform wants bit 4 on top
            i = 4 * (u & 1) + 2 * (u >> 1)
            xs[i], xs[i + 1] = lo, hi
        ys = fft_staged(k, xs, lambda m, eighth: (Mem(tw, 64 * (m - 1 + eighth)), Mem(tw, 64 * (m - 1 + eighth) + 32)),
                        first_quarter_turn=upper, combine=butterfly_shear)
        for q in range(8):
            k.store_split(ys[q], where[q])
        for p in pointers:
            k.gadd(p, p, k.vector)
        k.gadd(tw, tw, 256)

    def inner_loops(j):
        for upper in ((False, True) if halved else (False,)):
            # see the same construction in neon_kernels.halved_loop
            k.glsr(j, count, 1) if halved else k.gmov(j, count)
            top = k.label()
            body(upper)
            k.loop(j, top)

    last_loops(k, pointers, count, regions, region_skip, inner_loops)
    k.finish()
    return k


def butterflies(k, j, count, tw, pairs, where, pointers):
    """The loop over the butterflies of one region of `la` / `la2`."""
    k.gmov(j, count)
    top = k.label()
    xs = [k.load_split(m) for m in where]
    # Index bit 3 across the registers. Its partner in the register index is output bit 2.
    groups = 2 if pairs else 1
    y = []  # y[output bit 2][new output bit]
    for g in range(groups):
        t = k.twiddle_scalar(xs[2 * g + 1], tw, 4 * g, 8 + 4 * g)
        y.append(butterfly(k, xs[2 * g], t))
    if pairs:
        # Output bit 2 goes into the lanes, index bit 2 comes out and is transformed.
        out = [None] * 4
        for c in range(2):
            lo, hi = k.exchange_b1_to_b0(y[0][c], y[1][c])
            hi = k.twiddle_vector(hi, tw, 16).rotated(c)
            out[c], out[2 + c] = butterfly(k, lo, hi)
    else:
        # The transform of index bit 3 is output bit 2 itself.
        lo, hi = k.exchange_b1_to_b0(y[0][0], y[0][1])
        hi = k.twiddle_vector(hi, tw, 16)
        out = list(butterfly(k, lo, hi))
    for u, x in enumerate(out):
        k.store_split(x, where[u])
    for p in pointers:
        k.gadd(p, p, k.vector)
    k.gadd(tw, tw, 80)
    k.loop(j, top)


def last_pass_b(inverse):
    k = Kernel(name("avx2", "lb", inverse),
               """Last pass, in place: reads split vectors and writes the interleaved result. Butterfly
j < count uses the vectors at data + r*stride + 64*j. Index bits 1 and 0 are moved from the
lanes to the registers and transformed. The table holds per butterfly, for l = 1..3, the real
and the imaginary parts of the 8 twiddles of the vector with index bits (1, 0) = l.""", "avx2", inverse)
    data = k.param("float *", "data")
    tw = k.param("const float *", "tw")
    stride = k.param("size_t", "stride")
    count = k.param("size_t", "count")
    k.param("size_t", "regions", used=False)  # all last pass kernels share a signature
    third, where = four_vectors(k, data, stride)
    top = k.label()
    # Region r holds output bits (0, 1) = (r/2, r%2). Index bit 1 is lane bit h, the 128 bit
    # half: exchange it with output bit 1. That is done while loading, by reading the two halves
    # of a register from the two regions; no shuffle is needed.
    u = [[None, None], [None, None]]  # u[output bit 0][index bit 1]
    for k0 in range(2):
        for b1 in range(2):
            x = Cx(*k.cx())
            for part, offset in ((x.re, 16 * b1), (x.im, k.bytes + 16 * b1)):
                k.load_halves(part, where[2 * k0].shifted(offset), where[2 * k0 + 1].shifted(offset))
            u[k0][b1] = x
    # Index bit 0 is lane bit b1 since `la`: exchange it with output bit 0.
    t = [k.exchange_b1_to_b0(u[0][b1], u[1][b1]) for b1 in range(2)]  # t[index bit 1][index bit 0]
    zs = [t[l >> 1][l & 1] for l in range(4)]
    for l in range(1, 4):
        zs[l] = k.twiddle_vector(zs[l], tw, 64 * (l - 1))
    ys = fft_plain(k, zs)
    for q in range(4):
        k.store_interleaved(ys[q], where[q])
    k.gadd(data, data, k.vector)
    k.gadd(third, third, k.vector)
    k.gadd(tw, tw, 192)
    k.loop(count, top)
    k.finish()
    return k


def codelet_sse2(n, inverse):
    """SSE2: complete transform of 8 or 16 points."""
    k = Kernel(name("sse2", "n%d" % n, inverse),
               "Complete %d point transform from `in` to `out`, both interleaved." % n, "sse2", inverse)
    src = k.param("const float *", "in")
    dst = k.param("float *", "out")
    consts = []
    table = k.gconst("consts", "const float *", consts)

    def twiddled(x, w):
        offset = 4 * len(consts)
        consts.extend([z.real for z in w] + [z.imag for z in w])
        return k.twiddle_vector(x, table, offset)

    xs = [k.load_interleaved(Mem(src, 32 * u)) for u in range(n // 4)]
    if n == 16:
        xs = fft_plain(k, xs)
        zs = k.transpose(xs)
        for l in range(1, 4):
            zs[l] = twiddled(zs[l], [cplx.root(l * t, 16) for t in range(4)])
        ys = fft_plain(k, zs)
    else:
        # Two registers: index bit 2 selects the register, bits 1 and 0 the lane.
        a, b = butterfly(k, xs[0], xs[1])
        a, b = k.exchange_b1(a, b)  # lanes: (output bit 0, index bit 0)
        b = twiddled(b, [cplx.root(2 * (lane >> 1), 8) for lane in range(4)])
        a, b = butterfly(k, a, b)
        a, b = k.exchange_b0_to_b1(a, b)  # lanes: (output bit 1, output bit 0)
        b = twiddled(b, [cplx.root(lane, 8) for lane in range(4)])
        ys = butterfly(k, a, b)
    for q, y in enumerate(ys):
        k.store_interleaved(y, Mem(dst, 32 * q))
    k.finish()
    return k


def codelet_avx2(n, inverse):
    """AVX2: complete transform of 16 or 32 points in 2 or 4 registers."""
    vectors = n // 8
    k = Kernel(name("avx2", "n%d" % n, inverse),
               "Complete %d point transform from `in` to `out`, both interleaved." % n, "avx2", inverse)
    src = k.param("const float *", "in")
    dst = k.param("float *", "out")
    consts = []
    table = k.gconst("consts", "const float *", consts)

    def twiddled(x, w):
        offset = 4 * len(consts)
        consts.extend([z.real for z in w] + [z.imag for z in w])
        return k.twiddle_vector(x, table, offset)

    def stage(xs, exchange, twiddle):
        """One index bit that sits in the lanes, for every value `other` of the second register
        index bit: exchange with the first register index bit, twiddle, butterfly.

        xs[first][other] -> result[new output bit][other]. `twiddle(other)` gives the vector.
        """
        result = [[None] * len(xs[0]) for _ in range(2)]
        for other in range(len(xs[0])):
            lo, hi = exchange(xs[0][other], xs[1][other])
            hi = twiddled(hi, twiddle(other))
            result[0][other], result[1][other] = butterfly(k, lo, hi)
        return result

    def flip(xs):
        return [[xs[i][j] for i in range(len(xs))] for j in range(len(xs[0]))]

    # Lane bits after loading: h = index bit 1, b1 = index bit 2, b0 = index bit 0.
    loaded = [k.load_interleaved(Mem(src, 64 * u)) for u in range(vectors)]
    lanes = range(8)
    if vectors == 4:
        ys = fft_plain(k, loaded)  # output bits 0 and 1
        x = [[ys[k0 + 2 * k1] for k1 in range(2)] for k0 in range(2)]  # x[k0][k1]
        # index bit 2 (lane b1) <-> output bit 0, which ends up in lane b0
        x = stage(x, k.exchange_b1_to_b0, lambda k1: [cplx.root(4 * ((l & 1) + 2 * k1), 32) for l in lanes])
        # x[k2][k1]: index bit 1 (lane h) <-> output bit 1
        x = stage(flip(x), k.exchange_h,
                  lambda k2: [cplx.root(2 * ((l & 1) + 2 * (l >> 2) + 4 * k2), 32) for l in lanes])
        # x[k3][k2]: index bit 0 (now lane b1) <-> output bit 2; the lanes are in their final order
        x = stage(flip(x), k.exchange_b1, lambda k3: [cplx.root(ELEMENT_OF[l] + 8 * k3, 32) for l in lanes])
        out = [x[k4][k3] for k4 in range(2) for k3 in range(2)]
    else:
        x = [[y] for y in butterfly(k, *loaded)]  # x[output bit 0][0]
        x = stage(x, k.exchange_b1_to_b0, lambda _: [cplx.root(4 * (l & 1), 16) for l in lanes])
        x = stage(x, k.exchange_h, lambda _: [cplx.root(2 * ((l & 1) + 2 * (l >> 2)), 16) for l in lanes])
        x = stage(x, k.exchange_b1, lambda _: [cplx.root(ELEMENT_OF[l], 16) for l in lanes])
        out = [x[0][0], x[1][0]]
    for q, y in enumerate(out):
        k.store_interleaved(y, Mem(dst, 64 * q))
    k.finish()
    return k


def schedule(kernel):
    """Schedules with the largest reordering window that still allows register allocation."""
    latency = {"fp": OPTIONS["fp_latency"], "perm": 1, "load": 5, "store": 1, "int": 1}
    original = kernel.ins
    for window in (64, 48, 32, 24, 16, 12, 8, 4, 2, 0):
        if window > OPTIONS["window"]:
            continue
        kernel.ins = original
        kernel.schedule(OPTIONS["schedule"], latency, window)
        try:
            kernel.allocate()
            return
        except AllocationError:
            continue
    raise AllocationError("%s: no schedule fits the registers" % kernel.name)


def all_kernels(isa):
    kernels = []
    for inverse in (False, True):
        kernels.append(first_pass(isa, 4, inverse))
        kernels.append(first_pass(isa, 2 if isa == "sse2" else 8, inverse))
        if isa == "sse2":
            kernels.append(last_pass_sse2(inverse))
            kernels.append(codelet_sse2(8, inverse))
            kernels.append(codelet_sse2(16, inverse))
        else:
            kernels.append(last_pass_b(inverse))
            kernels.append(codelet_avx2(16, inverse))
            kernels.append(codelet_avx2(32, inverse))
    kernels.append(middle_pass(isa))
    if isa == "avx2":
        kernels.append(last_pass_a(True))
        kernels.append(last_pass_a(False))
        kernels.append(last_pass_a8(True))
        kernels.append(last_pass_a8(False))
    for k in kernels:
        if OPTIONS["schedule"]:
            schedule(k)
        else:
            k.allocate()
    return kernels
