"""x86-64 backend for SSE2, AVX2 and AVX-512: one method per machine instruction.

The three instruction sets share this class. They differ in the register width, in whether the
three-operand VEX/EVEX encoding is available, and in whether fused multiply-add exists:

    X86(name, doc, isa="sse2")     128 bit, two-operand instructions, no FMA
    X86(name, doc, isa="avx2")     256 bit, VEX, FMA3
    X86(name, doc, isa="avx512")   512 bit, EVEX, FMA, 32 registers, embedded broadcasts

Kernel code is written against three-operand instructions. For SSE2 they are lowered here to a
register copy plus a two-operand instruction (the copy is free on every x86-64 core).
Assembly is emitted in AT&T syntax.
"""

from .ir import Asm, Reg


class Mem:
    """An x86 address: offset(base, index, scale).

    With `broadcast` set the operand is a single float that is replicated to all lanes; only
    AVX-512 can use that directly as an instruction operand.

    Kernels must only form addresses that obey the rule `Asm.schedule` relies on: accesses
    through different (base, index) pairs never overlap.
    """

    __slots__ = ("base", "offset", "index", "scale", "broadcast")

    def __init__(self, base, offset=0, index=None, scale=1, broadcast=False):
        self.base = base
        self.offset = offset
        self.index = index
        self.scale = scale
        self.broadcast = broadcast

    def regs(self):
        return [self.base] if self.index is None else [self.base, self.index]

    def key(self):
        """Identifies the address registers for the scheduler's overlap rule."""
        return (self.base, self.index, self.scale)

    def shifted(self, offset):
        """The address `offset` bytes further on."""
        return Mem(self.base, self.offset + offset, self.index, self.scale, self.broadcast)

    def text(self, first):
        """Operand text; `first` is the placeholder number of the base register."""
        inner = "{%d}" % first if self.index is None else "{%d},{%d},%d" % (first, first + 1, self.scale)
        return "%s(%s)" % (self.offset or "", inner)

    def address(self, m):
        return m.reg[self.base] + self.offset + (m.reg[self.index] * self.scale if self.index is not None else 0)


ISAS = {
    # register prefix, bytes, physical registers, VEX/EVEX three-operand encoding, FMA
    "sse2": ("xmm", 16, 16, False, False),
    "avx2": ("ymm", 32, 16, True, True),
    "avx512": ("zmm", 64, 32, True, True),
}


class X86(Asm):
    def __init__(self, name, doc, isa):
        super().__init__(name, doc)
        self.isa = isa
        self.prefix, self.bytes, self.simd_count, self.vex, self.fma = ISAS[isa]
        self.lanes = self.bytes // 4
        self.two = None  # memory operand holding 2.0 in all lanes, for the in-place operations

    def clobbers(self, used):
        return ["xmm%d" % p for p in used] + ["cc", "memory"]

    def format_gpr(self, reg):
        return _Gpr(reg.name)

    def cx(self):
        """Two fresh registers for the real and imaginary parts of a complex vector."""
        return self.vreg(), self.vreg()

    def _r(self, index):
        """Placeholder text for SIMD register number `index` of an instruction."""
        return "%%%%%s{%d}" % (self.prefix, index)

    # ---- generic three-operand instruction -------------------------------------------------

    def _op(self, name, d, a, b, fn, commutative=False, kind="fp", imm=None):
        """d = fn(a, b), computed lane block by lane block.

        `b` is a register or a Mem. `fn` maps two lists (the full registers) to the result list.
        `name` is the SSE mnemonic; the VEX form is the same with a `v` prefix.
        """
        immediate = "$%d, " % imm if imm is not None else ""
        if isinstance(b, Mem):
            assert not b.broadcast or self.isa == "avx512"
            b_regs, b_text = b.regs(), b.text(2) + ("%{{1to16%}}" if b.broadcast else "")

            def value(m):
                data = m.load(b.address(m), 1 if b.broadcast else self.lanes)
                return data * self.lanes if b.broadcast else data
        else:
            b_regs, b_text = [b], self._r(2)

            def value(m):
                return m.reg[b]

        def sim(m):
            m.reg[d] = fn(m.reg[a], value(m))

        uses = [a] + b_regs
        access = self._operand_access(b)
        if self.vex:
            self.emit("v%s %s%s, %s, %s" % (name, immediate, b_text, self._r(1), self._r(0)),
                      [d, a] + b_regs, (d,), uses, sim, kind=kind, access=access)
        elif d is a:
            self.emit("%s %s%s, %s" % (name, immediate, b_text, self._r(0)), [d, a] + b_regs, (d,), uses, sim,
                      kind=kind, access=access)
        else:
            # Two-operand form: copy a to d first. d must not share a register with b, which is
            # still read afterwards; `late` tells the register allocator.
            self.emit("movaps %s, %s\n%s %s%s, %s" % (self._r(1), self._r(0), name, immediate, b_text, self._r(0)),
                      [d, a] + b_regs, (d,), uses, sim, kind=kind, late=b_regs, access=access)

    def _operand_access(self, operand):
        """The memory read by an instruction operand, for the scheduler; None for a register."""
        if not isinstance(operand, Mem):
            return None
        return (operand.key(), operand.offset, operand.offset + (4 if operand.broadcast else self.bytes))

    def _lanewise(self, op):
        return lambda x, y: [op(p, q) for p, q in zip(x, y)]

    def _blockwise(self, pick):
        """Result block = [(x, y)[source][lane] for source, lane in pick], per 128 bit block."""
        def fn(x, y):
            out = []
            for block in range(0, self.lanes, 4):
                out += [(x, y)[source][block + lane] for source, lane in pick]
            return out
        return fn

    # ---- SIMD arithmetic -------------------------------------------------------------------

    def add(self, d, a, b):
        self._op("addps", d, a, b, self._lanewise(lambda p, q: p + q), commutative=True)

    def sub(self, d, a, b):
        self._op("subps", d, a, b, self._lanewise(lambda p, q: p - q))

    def mul(self, d, a, w):
        self._op("mulps", d, a, w, self._lanewise(lambda p, q: p * q), commutative=True)

    def _accumulate(self, fma_name, sse_name, sign, d, a, w):
        """d += sign * a * w."""
        if self.fma:
            if isinstance(w, Mem):
                w_regs, w_text = w.regs(), w.text(2) + ("%{{1to16%}}" if w.broadcast else "")

                def value(m):
                    data = m.load(w.address(m), 1 if w.broadcast else self.lanes)
                    return data * self.lanes if w.broadcast else data
            else:
                w_regs, w_text = [w], self._r(2)

                def value(m):
                    return m.reg[w]

            def sim(m):
                m.reg[d] = [acc + sign * p * q for acc, p, q in zip(m.reg[d], m.reg[a], value(m))]
            self.emit("%s %s, %s, %s" % (fma_name, w_text, self._r(1), self._r(0)), [d, a] + w_regs, (d,),
                      [d, a] + w_regs, sim, kind="fp", access=self._operand_access(w))
        else:
            product = self.vreg()
            self.mul(product, a, w)
            self._op(sse_name, d, d, product, self._lanewise(lambda p, q: p + sign * q))

    def mla(self, d, a, w):
        self._accumulate("vfmadd231ps", "addps", 1.0, d, a, w)

    def mls(self, d, a, w):
        self._accumulate("vfnmadd231ps", "subps", -1.0, d, a, w)

    # ---- operations that overwrite an input, for kernels without a register to spare ----------

    def _fused132(self, name, d, y, fn):
        """d = fn(d, y) by a fused multiply-add of the form d = +-(d * two) +- y."""
        assert self.fma and self.two is not None
        two = self.two

        def sim(m):
            m.reg[d] = [fn(p, q) for p, q in zip(m.reg[d], m.reg[y])]
        self.emit("%s %s, %s, %s" % (name, two.text(2), self._r(1), self._r(0)), [d, y] + two.regs(), (d,),
                  [d, y] + two.regs(), sim, kind="fp", access=self._operand_access(two))

    def difference_from_sum(self, b, s, plus):
        """b = s - 2 b, or s + 2 b with `plus`: the second result of a butterfly from the first."""
        if plus:
            self._fused132("vfmadd132ps", b, s, lambda p, q: q + 2.0 * p)
        else:
            self._fused132("vfnmadd132ps", b, s, lambda p, q: q - 2.0 * p)

    def double_minus(self, x, y):
        """x = 2 x - y."""
        self._fused132("vfmsub132ps", x, y, lambda p, q: 2.0 * p - q)

    # ---- permutations (all work within 128 bit blocks unless noted) --------------------------

    def shufps(self, d, a, b, imm):
        """Per block: [a[i0], a[i1], b[i2], b[i3]] with the four 2 bit indices packed in imm."""
        pick = ((0, imm & 3), (0, (imm >> 2) & 3), (1, (imm >> 4) & 3), (1, (imm >> 6) & 3))
        self._op("shufps", d, a, b, self._blockwise(pick), kind="perm", imm=imm)

    def unpcklps(self, d, a, b):
        self._op("unpcklps", d, a, b, self._blockwise(((0, 0), (1, 0), (0, 1), (1, 1))), kind="perm")

    def unpckhps(self, d, a, b):
        self._op("unpckhps", d, a, b, self._blockwise(((0, 2), (1, 2), (0, 3), (1, 3))), kind="perm")

    def unpcklpd(self, d, a, b):
        self._op("unpcklpd", d, a, b, self._blockwise(((0, 0), (0, 1), (1, 0), (1, 1))), kind="perm")

    def unpckhpd(self, d, a, b):
        self._op("unpckhpd", d, a, b, self._blockwise(((0, 2), (0, 3), (1, 2), (1, 3))), kind="perm")

    def perm2f128(self, d, a, b, imm):
        """AVX: each 128 bit half of d is one of a.low, a.high, b.low, b.high (2 bit selectors)."""
        assert self.isa == "avx2"

        def fn(x, y):
            halves = (x[:4], x[4:], y[:4], y[4:])
            return halves[imm & 3] + halves[(imm >> 4) & 3]
        self._op("perm2f128", d, a, b, fn, kind="perm", imm=imm)

    def shuff32x4(self, d, a, b, imm):
        """AVX-512: blocks of d are a[i0], a[i1], b[i2], b[i3] with 128 bit blocks as elements."""
        assert self.isa == "avx512"

        def fn(x, y):
            out = []
            for position, source in enumerate((x, x, y, y)):
                block = (imm >> (2 * position)) & 3
                out += source[4 * block:4 * block + 4]
            return out
        self._op("shuff32x4", d, a, b, fn, kind="perm", imm=imm)

    # ---- SIMD loads and stores -------------------------------------------------------------

    def _mov(self):
        return "vmovups" if self.vex else "movups"

    def load(self, d, mem):
        def sim(m):
            m.reg[d] = m.load(mem.address(m), self.lanes)
        self.emit("%s %s, %s" % (self._mov(), mem.text(1), self._r(0)), [d] + mem.regs(), (d,), mem.regs(), sim,
                  kind="load", access=(mem.key(), mem.offset, mem.offset + self.bytes))

    def load_halves(self, d, low, high):
        """AVX2: fills the lower 128 bits of d from `low` and the upper from `high`.

        Combines halves of two vectors while loading, which needs no shuffle operation.
        """
        assert self.isa == "avx2"

        def sim_low(m):
            m.reg[d] = m.load(low.address(m), 4) + [0.0] * 4

        def sim_high(m):
            m.reg[d] = m.reg[d][:4] + m.load(high.address(m), 4)
        self.emit("vmovups %s, %%%%xmm{0}" % low.text(1), [d] + low.regs(), (d,), low.regs(), sim_low, kind="load",
                  access=(low.key(), low.offset, low.offset + 16))
        self.emit("vinsertf128 $1, %s, %s, %s" % (high.text(1), self._r(0), self._r(0)), [d] + high.regs(), (d,),
                  [d] + high.regs(), sim_high, kind="load", access=(high.key(), high.offset, high.offset + 16))

    def store(self, s, mem):
        def sim(m):
            m.store(mem.address(m), m.reg[s])
        self.emit("%s %s, %s" % (self._mov(), self._r(0), mem.text(1)), [s] + mem.regs(), (), [s] + mem.regs(), sim,
                  kind="store", access=(mem.key(), mem.offset, mem.offset + self.bytes))

    def broadcast(self, d, mem):
        """Loads one float and replicates it to all lanes."""
        def sim(m):
            m.reg[d] = m.load(mem.address(m), 1) * self.lanes
        access = (mem.key(), mem.offset, mem.offset + 4)
        if self.vex:
            self.emit("vbroadcastss %s, %s" % (mem.text(1), self._r(0)), [d] + mem.regs(), (d,), mem.regs(), sim,
                      kind="load", access=access)
        else:
            self.emit("movss %s, %s\nshufps $0, %s, %s" % (mem.text(1), self._r(0), self._r(0), self._r(0)),
                      [d] + mem.regs(), (d,), mem.regs(), sim, kind="load", access=access)

    def multiplier(self, mem):
        """A scalar from memory as a multiplier operand: the cheapest form the target offers."""
        if self.isa == "avx512":
            return Mem(mem.base, mem.offset, mem.index, mem.scale, broadcast=True)
        reg = self.vreg()
        self.broadcast(reg, mem)
        return reg

    def finish(self):
        """Ends a kernel. After 256 or 512 bit code the upper register halves have to be cleared
        to avoid penalties in SSE code that runs later."""
        if self.vex:
            self.emit("vzeroupper", (), (), (), None, kind="barrier")

    # ---- general purpose instructions ------------------------------------------------------

    def gmov(self, d, a):
        def sim(m):
            m.reg[d] = m.reg[a]
        self.emit("mov {1}, {0}", (d, a), (d,), (a,), sim)

    def gadd(self, d, a, b, shift=0):
        """d = a + (b << shift); b is a register or, with shift 0, an immediate. Keeps flags."""
        if isinstance(b, Reg):
            assert shift <= 3

            def sim(m):
                m.reg[d] = m.reg[a] + (m.reg[b] << shift)
            self.emit("lea ({1},{2},%d), {0}" % (1 << shift), (d, a, b), (d,), (a, b), sim)
        else:
            assert shift == 0

            def sim(m):
                m.reg[d] = m.reg[a] + b
            self.emit("lea %d({1}), {0}" % b, (d, a), (d,), (a,), sim)

    def gsub(self, d, a, b):
        assert d is a

        def sim(m):
            m.reg[d] = m.reg[a] - m.reg[b]
        self.emit("sub {1}, {0}", (d, b), (d,), (a, b), sim)

    def glsl(self, d, a, shift):
        if d is not a:
            self.gmov(d, a)

        def sim(m):
            m.reg[d] = m.reg[d] << shift
        self.emit("shl $%d, {0}" % shift, (d,), (d,), (d,), sim)

    def glsr(self, d, a, shift):
        if d is not a:
            self.gmov(d, a)

        def sim(m):
            m.reg[d] = m.reg[d] >> shift
        self.emit("shr $%d, {0}" % shift, (d,), (d,), (d,), sim)

    def gload(self, d, base, offset):
        """Loads a 64 bit value from base + offset."""
        def sim(m):
            m.reg[d] = m.load(m.reg[base] + offset, 1)[0]
        self.emit("mov %d({1}), {0}" % offset, (d, base), (d,), (base,), sim)

    def gload_pair32(self, d0, d1, base):
        """Loads two unsigned 32 bit values into d0, d1 and advances base by 8. Keeps flags."""
        def sim(m):
            m.reg[d0], m.reg[d1] = m.load(m.reg[base], 2)
            m.reg[base] += 8
        self.emit("movl ({2}), {0:k}\nmovl 4({2}), {1:k}\nlea 8({2}), {2}", (d0, d1, base), (d0, d1, base), (base,),
                  sim)

    def loop(self, counter, label):
        """Decrements counter and branches back to label while it is not zero."""
        def sim(m):
            m.reg[counter] -= 1
            return m.reg[counter] != 0
        # One unit, so that nothing that changes the flags can be scheduled in between.
        self.branch("sub $1, {0}\njnz %sb" % label, label, sim, (counter,))


class _Gpr(str):
    """Formats as the 64 bit operand by default and as the 32 bit view with the `k` spec."""

    def __format__(self, spec):
        return ("%k[" if spec == "k" else "%[") + str(self) + "]"
