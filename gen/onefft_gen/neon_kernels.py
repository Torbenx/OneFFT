"""The AArch64 kernels.

See docs/design.md for the algorithm. In short, with 4 float lanes per register and 32 registers:

* `f8`      first pass. Reads the interleaved input, transforms the top three index bits across
            8 registers and writes split vectors (4 real parts, then 4 imaginary parts) to the
            output buffer at their final position. No twiddles.
* `c8`/`c4` middle passes, in place on split vectors: 8 (4) registers, one scalar twiddle per
            register, then a plain FFT across the registers.
* `l8`/`l4` last pass, in place: handles the two index bits that live in the lanes by transposing
            them into registers. Reads split vectors, writes the interleaved result.
* `n8`, `n16`, `n32`  complete transforms that fit in the registers.
* `c8_fma`, `c4_fma`, `l8_fma`, `l4_fma`  variants of the middle and last passes built from
            fused multiply-add butterflies (cplx.butterfly_tan), for cores where register copies
            are free. They have their own table format: twiddles as (cos, tan) pairs.

The `i` variants compute the inverse transform by exchanging the roles of the real and imaginary
parts when reading the input and writing the output, which costs nothing.
"""

import math
import os
import struct

from . import cplx
from .cplx import Cx, bit_reverse, butterfly, butterfly_tan, cmul, fft_plain, fft_staged
from .ir import AllocationError
from .neon import Mem, Neon
from .passes import field, middle_loops, middle_params

#: Code generation choices. The defaults are the measured best over the test devices (see
#: docs/design.md). They can be overridden for experiments: ONEFFT_GEN="unroll=1,schedule=0".
OPTIONS = {
    "unroll": 2,      # butterflies per loop iteration; scheduling overlaps their instructions
    "schedule": 2,    # reorder instructions for a core running this many SIMD operations per cycle
    "fp_latency": 3,  # latency assumed for floating point operations when scheduling
    "window": 64,     # how far instructions may be moved ahead when scheduling
}
for _item in filter(None, os.environ.get("ONEFFT_GEN", "").split(",")):
    _key, _value = _item.split("=")
    assert _key in OPTIONS, _key
    OPTIONS[_key] = int(_value)


class Kernel(Neon):
    def __init__(self, name, doc, inverse=False):
        super().__init__(name, doc)
        self.inverse = inverse
        self._table_offset = 0

    # Interleaved data is read with LDP and split with UZP1/UZP2 rather than with LD2, and
    # written with ZIP1/ZIP2 and STP rather than ST2: never slower on the cores measured,
    # faster on some, and free of the addressing restrictions of LD2/ST2.

    def load_interleaved(self, mem):
        x = Cx(*self.cx())
        a, b = self.vreg(), self.vreg()
        self.ldp(a, b, mem)
        self.uzp1(x.im if self.inverse else x.re, a, b)
        self.uzp2(x.re if self.inverse else x.im, a, b)
        return x

    def store_interleaved(self, x, mem):
        assert x.rot == 0
        a, b = self.vreg(), self.vreg()
        even, odd = (x.im, x.re) if self.inverse else (x.re, x.im)
        self.zip1(a, even, odd)
        self.zip2(b, even, odd)
        self.stp(a, b, mem)

    def load_split(self, base, offset=0):
        x = Cx(*self.cx())
        self.ldp(x.re, x.im, Mem(base, offset))
        return x

    def store_split(self, x, base, offset=0):
        assert x.rot == 0
        self.stp(x.re, x.im, Mem(base, offset))

    def table_load(self, table, count):
        """Loads the next `count` registers from a table pointer; see table_done()."""
        regs = [self.vreg() for _ in range(count)]
        for i in range(0, count - 1, 2):
            self.ldp(regs[i], regs[i + 1], Mem(table, self._table_offset + 16 * i))
        if count % 2:
            self.ldr(regs[-1], Mem(table, self._table_offset + 16 * (count - 1)))
        self._table_offset += 16 * count
        return regs

    def table_done(self, table):
        """Advances the table pointer past everything table_load() has read."""
        self.gadd(table, table, self._table_offset)
        self._table_offset = 0

    def sqrt_half(self):
        """Loads sqrt(1/2) into all lanes of a register and returns it as a multiplier."""
        scratch = self.gtemp("bits")
        reg = self.vreg()
        value = math.sqrt(0.5)
        self.gmovi32(scratch, struct.unpack("<I", struct.pack("<f", value))[0])
        self.dup(reg, scratch, value)
        return reg

    def transpose(self, xs):
        """Exchanges the roles of lanes and registers: result[l] lane t = xs[t] lane l."""
        assert len(xs) == 4 and all(x.rot == 0 for x in xs)
        out = [Cx(*self.cx()) for _ in range(4)]
        for part in ("re", "im"):
            a = [getattr(x, part) for x in xs]
            b = [getattr(x, part) for x in out]
            t = [self.vreg() for _ in range(4)]
            self.trn1(t[0], a[0], a[1])
            self.trn2(t[1], a[0], a[1])
            self.trn1(t[2], a[2], a[3])
            self.trn2(t[3], a[2], a[3])
            self.zip1d(b[0], t[0], t[2])
            self.zip1d(b[1], t[1], t[3])
            self.zip2d(b[2], t[0], t[2])
            self.zip2d(b[3], t[1], t[3])
        return out

    def exchange(self, a, b, lane_bit):
        """Exchanges one lane index bit of two vectors with the register index.

        Returns (lo, hi): the elements of a and b whose lane index has `lane_bit` clear (set).
        The other lane bit moves to lane bit 0, and lane bit 1 now tells a from b.
        """
        assert a.rot == 0 and b.rot == 0
        lo, hi = Cx(*self.cx()), Cx(*self.cx())
        first, second = (self.uzp1, self.uzp2) if lane_bit == 0 else (self.zip1d, self.zip2d)
        for part in ("re", "im"):
            first(getattr(lo, part), getattr(a, part), getattr(b, part))
            second(getattr(hi, part), getattr(a, part), getattr(b, part))
        return lo, hi

    def pointers(self, base, stride, count):
        """Returns `count` pointers base + u*stride, the first being `base` itself."""
        ps = [base]
        for u in range(1, count):
            p = self.gtemp("p%d" % u, "float *")
            self.gadd(p, ps[-1], stride)
            ps.append(p)
        return ps

    def unrolled_loop(self, counter, count, body):
        """Emits a loop that runs body(copy) OPTIONS["unroll"] times per iteration, `count`
        butterflies in total (a multiple of the unroll factor). `counter` is a free register;
        it may be `count` itself. Returns the loop label for the closing loop()."""
        unroll = OPTIONS["unroll"]
        shift = unroll.bit_length() - 1
        assert unroll == 1 << shift
        if shift:
            self.glsr(counter, count, shift)
        elif counter is not count:
            self.gmov(counter, count)
        top = self.label()
        for copy in range(unroll):
            body(copy)
        return top


def first_pass(inverse):
    k = Kernel("onefft_neon_f8" + "i" * inverse,
               """First pass, radix 8. For each of the `count` entries (a, b) of `table`: reads 8 interleaved
vectors from in + 8*a + u*in_stride, transforms across them and writes output q as a split
vector to out + 8*b + (q%4)*out_stride + 32*(q/4).""", inverse)
    src_base = k.param("const float *", "in")
    dst_base = k.param("float *", "out")
    table = k.param("const uint32_t *", "table")
    count = k.param("size_t", "count")
    in_stride = k.param("size_t", "in_stride")
    out_stride = k.param("size_t", "out_stride")
    half = k.sqrt_half()

    def body(copy):
        src = k.gtemp("src%d" % copy, "const float *")
        dst = k.gtemp("dst%d" % copy, "float *")
        a, b = k.gtemp("a%d" % copy), k.gtemp("b%d" % copy)
        k.gload_pair32(a, b, table)
        k.gadd(src, src_base, a, shift=3)
        k.gadd(dst, dst_base, b, shift=3)
        xs = []
        for u in range(8):
            xs.append(k.load_interleaved(Mem(src)))
            if u < 7:
                k.gadd(src, src, in_stride)
        ys = fft_plain(k, xs, half)
        for t in range(4):
            k.store_split(ys[t], dst)
            k.store_split(ys[t + 4], dst, 32)
            if t < 3:
                k.gadd(dst, dst, out_stride)

    top = k.unrolled_loop(count, count, body)
    k.loop(count, top)
    return k


MIDDLE_DOC = """Middle pass, radix %d, in place on split vectors%s. `data` holds `regions` regions of `blocks`
blocks. Vector u of butterfly j of a block is at u*args.s_high + 32*j, for j < args.inner; see
passes.py for how the blocks and regions follow each other. %s"""


def middle_pass(radix):
    bits = radix.bit_length() - 1
    k = Kernel("onefft_neon_c%d" % radix, MIDDLE_DOC % (radix, "", """Butterfly j multiplies input i (bit
reversed u) by w_j^i and transforms across the vectors. The table `tw` holds per butterfly the real
parts of w^1..w^%d padded to %d floats, then the imaginary parts.""" % (radix - 1, 4 * (radix // 4))))
    data, tw_base, args, blocks, regions = middle_params(k)
    stride = k.gtemp("stride")
    k.gload(stride, args, field("s_high"))
    ps = k.pointers(data, stride, radix)
    half = k.sqrt_half() if radix == 8 else None

    def inner_loops(tw, j, scratch):
        def body(copy):
            xs = [k.load_split(p, 32 * copy) for p in ps]
            w = k.table_load(tw, radix // 2)
            w_re, w_im = w[:radix // 4], w[radix // 4:]
            ins = [xs[bit_reverse(i, bits)] for i in range(radix)]
            for i in range(1, radix):
                reg, lane = divmod(i - 1, 4)
                ins[i] = cmul(k, ins[i], (w_re[reg], lane), (w_im[reg], lane))
            ys = fft_plain(k, ins, half)
            for u in range(radix):
                k.store_split(ys[u], ps[u], 32 * copy)

        k.gload(scratch, args, field("inner"))
        top = k.unrolled_loop(j, scratch, body)
        for p in ps:
            k.gadd(p, p, 32 * OPTIONS["unroll"])
        k.table_done(tw)
        k.loop(j, top)

    middle_loops(k, ps, tw_base, args, blocks, regions, inner_loops)
    return k


LAST_DOC = """Last pass, radix %d, in place%s: reads split vectors and writes the interleaved result.
Butterfly j < count uses the vectors at data + u*stride + 32*j. The last parameter is not used: it
exists because the last pass kernels of all targets share a signature. See the table builder in
onefft.c for the twiddle layout."""


def last_params(k, radix):
    data = k.param("float *", "data")
    tw = k.param("const float *", "tw")
    stride = k.param("size_t", "stride")
    count = k.param("size_t", "count")
    k.param("size_t", "regions", used=False)
    j = k.gtemp("j")
    ps = k.pointers(data, stride, radix)
    return ps, tw, count, j


def last_pass(radix, inverse):
    k = Kernel("onefft_neon_l%d" % radix + "i" * inverse, LAST_DOC % (radix, ""), inverse)
    ps, tw, count, j = last_params(k, radix)
    groups = radix // 4

    def body(copy):
        xs = [k.load_split(p, 32 * copy) for p in ps]
        if radix == 8:
            # Index bit 2 sits in the registers: vectors 2t and 2t+1 hold its two values. Lane t
            # of the scalar twiddle register belongs to that pair.
            s_re, s_im = k.table_load(tw, 2)
            ys = [cmul(k, xs[2 * t + 1], (s_re, t), (s_im, t)) for t in range(4)]
            for t in range(4):
                xs[2 * t], xs[2 * t + 1] = butterfly(k, xs[2 * t], ys[t])
        for g in range(groups):
            # Index bits 1 and 0 sit in the lanes. After transposing they select the register,
            # and the lanes hold the two lowest bits of the output index.
            zs = k.transpose([xs[groups * t + g] for t in range(4)])
            for l in range(1, 4):
                w_re, w_im = k.table_load(tw, 2)
                zs[l] = cmul(k, zs[l], w_re, w_im)
            ys = fft_plain(k, zs)
            for q in range(4):
                k.store_interleaved(ys[q], Mem(ps[groups * q + g], 32 * copy))

    top = k.unrolled_loop(j, count, body)
    for p in ps:
        k.gadd(p, p, 32 * OPTIONS["unroll"])
    k.table_done(tw)
    k.loop(j, top)
    return k


def halved_loop(k, counter, count, body):
    """Runs body(False) for the first half of `count` butterflies and body(True) for the second.

    The fused multiply-add passes need this: their first stage twiddle exp(-i phi) has phi in
    [0, pi), but is stored as (cos, tan), which has a pole at pi/2. The second half of the
    butterflies has phi >= pi/2; there the table holds the twiddle for phi - pi/2 and the code
    multiplies by -i, which is free.
    """
    for upper in (False, True):
        k.glsr(counter, count, 1)
        top = k.label()
        body(upper)
        k.loop(counter, top)


def middle_pass_fma(radix):
    bits = radix.bit_length() - 1
    k = Kernel("onefft_neon_c%d_fma" % radix, MIDDLE_DOC % (radix, ", from fused multiply-add butterflies", """
The table `tw` holds per butterfly the cosines of its %d twiddles, then their tangents, see the
table builder in onefft.c.""" % (radix // 2)))
    data, tw_base, args, blocks, regions = middle_params(k)
    stride = k.gtemp("stride")
    k.gload(stride, args, field("s_high"))
    ps = k.pointers(data, stride, radix)

    def inner_loops(tw, j, scratch):
        def body(upper):
            xs = [k.load_split(p) for p in ps]
            if radix == 8:
                c, t = k.table_load(tw, 2)
            else:
                (c,) = k.table_load(tw, 1)
                t = c
            # lanes of c (and t): stage 1, stage 2, stage 3, stage 3 turned by an eighth;
            # radix 4 keeps (c1, c2, t1, t2) in one register
            lane = (lambda m, eighth: m - 1 + eighth) if radix == 8 else (lambda m, eighth: m - 1)
            offset = 0 if radix == 8 else 2
            ys = fft_staged(k, [xs[bit_reverse(i, bits)] for i in range(radix)],
                            lambda m, eighth: ((c, lane(m, eighth)), (t, lane(m, eighth) + offset)), upper)
            for u in range(radix):
                k.store_split(ys[u], ps[u])
            for p in ps:
                k.gadd(p, p, 32)
            k.table_done(tw)

        k.gload(scratch, args, field("inner"))
        halved_loop(k, j, scratch, body)

    middle_loops(k, ps, tw_base, args, blocks, regions, inner_loops)
    return k


def last_pass_fma(radix, inverse):
    k = Kernel("onefft_neon_l%d_fma" % radix + "i" * inverse,
               LAST_DOC % (radix, ", from fused multiply-add butterflies"), inverse)
    ps, tw, count, j = last_params(k, radix)
    groups = radix // 4

    def body(upper):
        xs = [k.load_split(p) for p in ps]
        if radix == 8:
            # Index bit 2: one scalar twiddle per pair of vectors, lane t of (s_c, s_t).
            s_c, s_t = k.table_load(tw, 2)
            for t in range(4):
                xs[2 * t], xs[2 * t + 1] = butterfly_tan(k, xs[2 * t], xs[2 * t + 1].rotated(1 if upper else 0),
                                                         (s_c, t), (s_t, t))
        # Index bits 1 and 0, from the lanes. The twiddle of bit 1 is shared by the two groups
        # of a radix 8 butterfly up to a factor -i; radix 4 gets that factor in the upper half
        # of its butterflies, like the first stages elsewhere (see halved_loop).
        w1 = k.table_load(tw, 2)
        for g in range(groups):
            zs = k.transpose([xs[groups * t + g] for t in range(4)])
            w0 = k.table_load(tw, 2)
            ys = fft_staged(k, zs, lambda m, eighth: w1 if m == 1 else w0,
                            first_quarter_turn=(g == 1) if radix == 8 else upper)
            for q in range(4):
                k.store_interleaved(ys[q], Mem(ps[groups * q + g]))
        for p in ps:
            k.gadd(p, p, 32)
        k.table_done(tw)

    halved_loop(k, j, count, body)
    return k


def codelet(n, inverse):
    """Complete transform of 16 or 32 points in registers."""
    vectors = n // 4
    k = Kernel("onefft_neon_n%d" % n + "i" * inverse,
               "Complete %d point transform from `in` to `out`, both interleaved." % n, inverse)
    src = k.param("const float *", "in")
    dst = k.param("float *", "out")
    consts = []
    table = k.gconst("consts", "const float *", consts)
    half = k.sqrt_half() if vectors == 8 else None
    xs = [k.load_interleaved(Mem(src, 32 * u)) for u in range(vectors)]
    xs = fft_plain(k, xs, half)
    groups = vectors // 4
    for g in range(groups):
        zs = k.transpose([xs[t + 4 * g] for t in range(4)])
        for l in range(1, 4):
            w = [cplx.root(l * (t + 4 * g), n) for t in range(4)]
            consts.extend([z.real for z in w] + [z.imag for z in w])
            w_re, w_im = k.table_load(table, 2)
            zs[l] = cmul(k, zs[l], w_re, w_im)
        ys = fft_plain(k, zs)
        for q in range(4):
            k.store_interleaved(ys[q], Mem(dst, 32 * (g + groups * q)))
    return k


def codelet8(inverse):
    """Complete transform of 8 points: two registers, one index bit across them and two in the lanes."""
    k = Kernel("onefft_neon_n8" + "i" * inverse,
               "Complete 8 point transform from `in` to `out`, both interleaved.", inverse)
    src = k.param("const float *", "in")
    dst = k.param("float *", "out")
    consts = []
    table = k.gconst("consts", "const float *", consts)

    def twiddled(x, w):
        consts.extend([z.real for z in w] + [z.imag for z in w])
        w_re, w_im = k.table_load(table, 2)
        return cmul(k, x, w_re, w_im)

    a = k.load_interleaved(Mem(src))
    b = k.load_interleaved(Mem(src, 32))
    # Index bit 2 selects the register. Its butterfly yields output bit 0.
    a, b = butterfly(k, a, b)
    # Move index bit 1 from the lanes to the registers: lanes are now (index bit 0, output bit 0).
    a, b = k.exchange(a, b, 1)
    b = twiddled(b, [cplx.root(2 * (lane >> 1), 8) for lane in range(4)])
    a, b = butterfly(k, a, b)
    # Move index bit 0 to the registers: lanes are now (output bit 0, output bit 1).
    a, b = k.exchange(a, b, 0)
    b = twiddled(b, [cplx.root(lane, 8) for lane in range(4)])
    a, b = butterfly(k, a, b)
    k.store_interleaved(a, Mem(dst))
    k.store_interleaved(b, Mem(dst, 32))
    return k


def schedule(kernel):
    """Schedules with the largest reordering window that still allows register allocation."""
    latency = {"fp": OPTIONS["fp_latency"], "perm": 2, "mov": 0, "load": 4, "store": 1, "int": 1}
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


def all_kernels():
    kernels = []
    for inverse in (False, True):
        kernels.append(first_pass(inverse))
        kernels.append(last_pass(8, inverse))
        kernels.append(last_pass(4, inverse))
        kernels.append(codelet8(inverse))
        kernels.append(codelet(16, inverse))
        kernels.append(codelet(32, inverse))
        kernels.append(last_pass_fma(8, inverse))
        kernels.append(last_pass_fma(4, inverse))
    kernels.append(middle_pass(8))
    kernels.append(middle_pass(4))
    kernels.append(middle_pass_fma(8))
    kernels.append(middle_pass_fma(4))
    for k in kernels:
        if OPTIONS["schedule"]:
            schedule(k)
        else:
            k.allocate()
    return kernels
