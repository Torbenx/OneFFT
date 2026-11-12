"""AArch64 Advanced SIMD (NEON) backend: one method per machine instruction."""

from .ir import Asm, Reg


class Mem:
    """An AArch64 address: [base, #offset], or [base] with post-increment by `post` bytes.

    Kernels must only form addresses that obey the rule `Asm.schedule` relies on: accesses
    through different base registers never overlap.
    """

    __slots__ = ("base", "offset", "post")

    def __init__(self, base, offset=0, post=None):
        assert not (offset and post is not None)
        self.base = base
        self.offset = offset
        self.post = post


class Neon(Asm):
    simd_count = 32
    lanes = 4

    def clobbers(self, used):
        return ["v%d" % p for p in used] + ["cc", "memory"]

    def format_gpr(self, reg):
        return _Gpr(reg.name)

    def cx(self):
        """Two fresh registers for the real and imaginary parts of a complex vector."""
        return self.vreg(), self.vreg()

    # ---- SIMD arithmetic -------------------------------------------------------------------

    def _arith(self, mnemonic, fn, d, a, w, accumulate):
        """d = fn(d, a, w) where w is a register or a (register, lane) tuple."""
        uses = [a] + ([d] if accumulate else [])
        if isinstance(w, tuple):
            wreg, lane = w
            text = "%s v{0}.4s, v{1}.4s, v{2}.s[%d]" % (mnemonic, lane)

            def sim(m):
                s = m.reg[wreg][lane]
                m.reg[d] = [fn(m.reg[d][i] if accumulate else 0.0, m.reg[a][i], s) for i in range(4)]
        else:
            wreg = w
            text = "%s v{0}.4s, v{1}.4s, v{2}.4s" % mnemonic

            def sim(m):
                m.reg[d] = [fn(m.reg[d][i] if accumulate else 0.0, m.reg[a][i], m.reg[wreg][i]) for i in range(4)]
        self.emit(text, (d, a, wreg), (d,), uses + [wreg], sim, kind="fp")

    def add(self, d, a, b):
        self._arith("fadd", lambda acc, x, y: x + y, d, a, b, False)

    def sub(self, d, a, b):
        self._arith("fsub", lambda acc, x, y: x - y, d, a, b, False)

    def mul(self, d, a, w):
        self._arith("fmul", lambda acc, x, y: x * y, d, a, w, False)

    def mla(self, d, a, w):
        self._arith("fmla", lambda acc, x, y: acc + x * y, d, a, w, True)

    def mls(self, d, a, w):
        self._arith("fmls", lambda acc, x, y: acc - x * y, d, a, w, True)

    def mov(self, d, a):
        def sim(m):
            m.reg[d] = list(m.reg[a])
        self.emit("mov v{0}.16b, v{1}.16b", (d, a), (d,), (a,), sim, kind="mov")

    def _permute(self, mnemonic, arrangement, pick, d, a, b):
        def sim(m):
            x, y = m.reg[a], m.reg[b]
            m.reg[d] = [(x, y)[src][lane] for src, lane in pick]
        self.emit("%s v{0}.%s, v{1}.%s, v{2}.%s" % (mnemonic, arrangement, arrangement, arrangement),
                  (d, a, b), (d,), (a, b), sim, kind="perm")

    # Each permutation is described by (source, lane) for the four result lanes.
    def trn1(self, d, a, b):
        self._permute("trn1", "4s", ((0, 0), (1, 0), (0, 2), (1, 2)), d, a, b)

    def trn2(self, d, a, b):
        self._permute("trn2", "4s", ((0, 1), (1, 1), (0, 3), (1, 3)), d, a, b)

    def zip1(self, d, a, b):
        self._permute("zip1", "4s", ((0, 0), (1, 0), (0, 1), (1, 1)), d, a, b)

    def zip2(self, d, a, b):
        self._permute("zip2", "4s", ((0, 2), (1, 2), (0, 3), (1, 3)), d, a, b)

    def uzp1(self, d, a, b):
        self._permute("uzp1", "4s", ((0, 0), (0, 2), (1, 0), (1, 2)), d, a, b)

    def uzp2(self, d, a, b):
        self._permute("uzp2", "4s", ((0, 1), (0, 3), (1, 1), (1, 3)), d, a, b)

    def zip1d(self, d, a, b):
        self._permute("zip1", "2d", ((0, 0), (0, 1), (1, 0), (1, 1)), d, a, b)

    def zip2d(self, d, a, b):
        self._permute("zip2", "2d", ((0, 2), (0, 3), (1, 2), (1, 3)), d, a, b)

    def dup(self, d, gpr, value):
        """Sets all lanes to the 32 bit pattern in a general purpose register (`value` as float)."""
        def sim(m):
            m.reg[d] = [value] * 4
        self.emit("dup v{0}.4s, {1:w}", (d, gpr), (d,), (gpr,), sim, kind="perm")

    # ---- SIMD loads and stores -------------------------------------------------------------

    def _address(self, mem, index):
        """Returns (text, registers, post-increment function, written registers) of an operand.

        `index` is the placeholder index of the base register in the instruction text.
        """
        if mem.post is None:
            text = "[{%d}, #%d]" % (index, mem.offset) if mem.offset else "[{%d}]" % index
            return text, [mem.base], lambda m: None, []

        def advance(m, base=mem.base, post=mem.post):
            m.reg[base] += post
        return "[{%d}], #%d" % (index, mem.post), [mem.base], advance, [mem.base]

    def ldp(self, d0, d1, mem):
        """Loads two consecutive registers (32 bytes)."""
        text, regs, advance, written = self._address(mem, 2)

        def sim(m):
            data = m.load(m.reg[mem.base] + mem.offset, 8)
            m.reg[d0], m.reg[d1] = data[:4], data[4:]
            advance(m)
        self.emit("ldp q{0}, q{1}, " + text, [d0, d1] + regs, [d0, d1] + written, regs, sim, kind="load",
                  access=((mem.base,), mem.offset, mem.offset + 32))

    def stp(self, s0, s1, mem):
        text, regs, advance, written = self._address(mem, 2)

        def sim(m):
            m.store(m.reg[mem.base] + mem.offset, m.reg[s0] + m.reg[s1])
            advance(m)
        self.emit("stp q{0}, q{1}, " + text, [s0, s1] + regs, written, [s0, s1] + regs, sim, kind="store",
                  access=((mem.base,), mem.offset, mem.offset + 32))

    def ldr(self, d, mem):
        """Loads one register (16 bytes)."""
        text, regs, advance, written = self._address(mem, 1)

        def sim(m):
            m.reg[d] = m.load(m.reg[mem.base] + mem.offset, 4)
            advance(m)
        self.emit("ldr q{0}, " + text, [d] + regs, [d] + written, regs, sim, kind="load",
                  access=((mem.base,), mem.offset, mem.offset + 16))

    # ---- general purpose instructions ------------------------------------------------------

    def gmov(self, d, a):
        def sim(m):
            m.reg[d] = m.reg[a]
        self.emit("mov {0}, {1}", (d, a), (d,), (a,), sim)

    def gmovi32(self, d, value):
        """Loads a 32 bit constant."""
        def sim(m):
            m.reg[d] = value
        self.emit("mov {0:w}, #%d" % (value & 0xffff), (d,), (d,), (), None)
        self.emit("movk {0:w}, #%d, lsl #16" % (value >> 16), (d,), (d,), (d,), sim)

    def gadd(self, d, a, b, shift=0):
        """d = a + (b << shift); b is a register or, with shift 0, an immediate."""
        if isinstance(b, Reg):
            def sim(m):
                m.reg[d] = m.reg[a] + (m.reg[b] << shift)
            suffix = ", lsl #%d" % shift if shift else ""
            self.emit("add {0}, {1}, {2}" + suffix, (d, a, b), (d,), (a, b), sim)
        else:
            assert shift == 0 and 0 <= b < 4096

            def sim(m):
                m.reg[d] = m.reg[a] + b
            self.emit("add {0}, {1}, #%d" % b, (d, a), (d,), (a,), sim)

    def gsub(self, d, a, b):
        def sim(m):
            m.reg[d] = m.reg[a] - m.reg[b]
        self.emit("sub {0}, {1}, {2}", (d, a, b), (d,), (a, b), sim)

    def glsl(self, d, a, shift):
        def sim(m):
            m.reg[d] = m.reg[a] << shift
        self.emit("lsl {0}, {1}, #%d" % shift, (d, a), (d,), (a,), sim)

    def glsr(self, d, a, shift):
        def sim(m):
            m.reg[d] = m.reg[a] >> shift
        self.emit("lsr {0}, {1}, #%d" % shift, (d, a), (d,), (a,), sim)

    def gload(self, d, base, offset):
        """Loads a 64 bit value from base + offset."""
        def sim(m):
            m.reg[d] = m.load(m.reg[base] + offset, 1)[0]
        self.emit("ldr {0}, [{1}, #%d]" % offset, (d, base), (d,), (base,), sim)

    def gload_pair32(self, d0, d1, base):
        """Loads two unsigned 32 bit values into d0, d1 and advances base by 8."""
        def sim(m):
            m.reg[d0], m.reg[d1] = m.load(m.reg[base], 2)
            m.reg[base] += 8
        self.emit("ldp {0:w}, {1:w}, [{2}], #8", (d0, d1, base), (d0, d1, base), (base,), sim)

    def loop(self, counter, label):
        """Decrements counter and branches back to label while it is not zero."""
        def sim(m):
            m.reg[counter] -= 1
            return m.reg[counter] != 0
        self.emit("subs {0}, {0}, #1", (counter,), (counter,), (counter,), None)
        self.branch("b.ne %sb" % label, label, sim)


class _Gpr(str):
    """Formats as the 64 bit operand by default and as the 32 bit view with the `w` spec."""

    def __format__(self, spec):
        return ("%w[" if spec == "w" else "%[") + str(self) + "]"
