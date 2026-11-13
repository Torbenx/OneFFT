"""A tiny assembler IR for OneFFT kernels.

A kernel is one C function whose body is a single inline assembly block. This module provides the
target independent parts of producing such a function:

* `Reg`      - a virtual register, either a SIMD register or a general purpose register.
* `Asm`      - an instruction list. Target backends derive from it and add one method per machine
               instruction. Every instruction carries its assembly text, the registers it reads
               and writes, and a Python closure that simulates it.
* allocation - `Asm.allocate` maps virtual SIMD registers to physical ones with a linear scan.
* simulation - `Asm.run` interprets the instruction list on a small memory model. The kernel
               tests use it to verify every kernel numerically without the target hardware.
* emission   - `Asm.c_function` renders the C function.

General purpose registers are never allocated here: each becomes a named inline assembly operand
and the C compiler picks the physical register. This keeps kernels independent of the calling
convention.
"""


class Reg:
    """A virtual register: kind 'v' for SIMD registers, 'g' for general purpose registers."""

    __slots__ = ("kind", "name", "ctype")

    def __init__(self, kind, name, ctype=None):
        self.kind = kind
        self.name = name
        self.ctype = ctype

    def __repr__(self):
        return self.name


class Ins:
    """One instruction: text with `{0}`, `{1}`... placeholders for `regs`."""

    __slots__ = ("text", "regs", "defs", "uses", "sim", "label", "target", "kind", "access", "late")

    def __init__(self, text, regs, defs, uses, sim, label=None, target=None, kind="int", access=None, late=()):
        self.late = late  # registers still read after the results are written (two-operand code)
        # "fp", "perm", "mov" (register copy), "load", "store" or "int", used by Asm.schedule;
        # "barrier" for an instruction that no other instruction may be moved across
        self.kind = kind
        # for every instruction that reads or writes memory: (address registers, first byte, end)
        self.access = access
        self.text = text
        self.regs = regs
        self.defs = defs
        self.uses = uses
        self.sim = sim
        self.label = label  # set for label pseudo instructions
        self.target = target  # set for backward branches: the label they may jump to


class Machine:
    """Simulation state: register values and a flat memory of 4 byte cells."""

    def __init__(self, cells):
        self.mem = [0.0] * cells
        self.reg = {}
        self.executed = 0

    def load(self, addr, count):
        assert addr % 4 == 0 and addr >= 0, addr
        return self.mem[addr // 4:addr // 4 + count]

    def store(self, addr, values):
        assert addr % 4 == 0 and addr >= 0, addr
        assert addr // 4 + len(values) <= len(self.mem), "store out of bounds"
        self.mem[addr // 4:addr // 4 + len(values)] = values


class AllocationError(Exception):
    pass


class Asm:
    """Instruction list of one kernel. Subclassed by the target backends."""

    #: number of physical SIMD registers, set by the backend
    simd_count = 0
    #: number of float lanes of a full SIMD register, set by the backend
    lanes = 0

    def __init__(self, name, doc):
        self.name = name
        self.doc = doc
        self.ins = []
        self.params = []  # general purpose registers bound to the C parameters, in order
        self.unused = []  # parameters that are not referenced by the assembly
        self.temps = []  # scratch general purpose registers
        self.consts = []  # (register, float list): pointers to constant tables of this kernel
        self._vcount = 0
        self._labels = 0
        self.assignment = None

    # ---- registers -------------------------------------------------------------------------

    def param(self, ctype, name, used=True):
        """A C parameter. Kernels of one kind share a signature; `used` is False for a parameter
        that this kernel has no use for."""
        reg = Reg("g", name, ctype)
        self.params.append(reg)
        if not used:
            self.unused.append(reg)
        return reg

    def gtemp(self, name, ctype="size_t"):
        reg = Reg("g", name, ctype)
        self.temps.append(reg)
        return reg

    def gconst(self, name, ctype, values):
        """A register that initially points to a constant float table.

        `values` is read when the function is emitted, so it may be filled while building.
        """
        reg = Reg("g", name, ctype)
        self.consts.append((reg, values))
        return reg

    def vreg(self):
        self._vcount += 1
        return Reg("v", "t%d" % self._vcount)

    # ---- building --------------------------------------------------------------------------

    def emit(self, text, regs=(), defs=(), uses=(), sim=None, kind="int", access=None, late=()):
        assert access is not None or kind not in ("load", "store")
        self.ins.append(Ins(text, tuple(regs), tuple(defs), tuple(uses), sim, kind=kind, access=access,
                            late=tuple(late)))

    def branch(self, text, label, sim, regs=()):
        """A conditional backward branch to `label`; `sim` returns whether it is taken.

        `regs` are general purpose registers the branch instruction reads and writes.
        """
        self.ins.append(Ins(text, tuple(regs), tuple(regs), tuple(regs), sim, target=label))

    def label(self):
        """Places a local label here and returns it, for use as a backward branch target."""
        return self.place(self.forward_label())

    def forward_label(self):
        """Reserves a label for a forward branch; `place` puts it into the instruction list."""
        self._labels += 1
        return str(self._labels)

    def place(self, name):
        self.ins.append(Ins(name + ":", (), (), (), None, label=name))
        return name

    # ---- instruction scheduling ------------------------------------------------------------

    def schedule(self, width, latency, window):
        """Reorders instructions into the order an idealized out-of-order core would run them.

        Kernels are built stage by stage, which is easy to read but gives a core with a small
        scheduling window little to overlap. This pass list-schedules every straight-line region
        for a machine that executes `width` SIMD operations per cycle with the given `latency`
        per instruction kind, preferring the instructions on the longest remaining dependency
        chain, and emits them in that order.

        An instruction is moved ahead of at most `window` earlier instructions. This keeps the
        number of live values close to that of the original order, so that register allocation
        still succeeds; callers shrink the window until it does.

        Memory dependencies are derived from a rule every kernel has to obey: accesses through
        different address registers (base, or base and index) never overlap. Accesses through
        the same address registers are compared by their offsets, as long as the registers were
        not modified in between.
        """
        result, region = [], []
        for ins in self.ins + [None]:
            if ins is None or ins.label is not None or ins.target is not None or ins.kind == "barrier":
                result += self._schedule_region(region, width, latency, window)
                region = []
                if ins is not None:
                    result.append(ins)
            else:
                region.append(ins)
        self.ins = result

    def _schedule_region(self, region, width, latency, window):
        n = len(region)
        if n < 3:
            return region
        deps = [[] for _ in range(n)]  # (predecessor, delay)
        last_def, readers, accesses = {}, {}, []
        for i, ins in enumerate(region):
            for r in ins.uses:
                if r in last_def:
                    deps[i].append((last_def[r], latency[region[last_def[r]].kind]))
            for r in ins.defs:
                # A redefinition must follow the previous value's definition and all its readers.
                for j in readers.get(r, []):
                    if j != i:
                        deps[i].append((j, 0))
                if r in last_def:
                    deps[i].append((last_def[r], 0))
            if ins.access is not None:
                address, first, end = ins.access
                version = tuple(last_def.get(r) for r in address if isinstance(r, Reg))
                for j, other_store, other_address, other_version, other_first, other_end in accesses:
                    if (ins.kind == "store" or other_store) and other_address == address and (
                            other_version != version or (first < other_end and other_first < end)):
                        deps[i].append((j, 0))
                accesses.append((i, ins.kind == "store", address, version, first, end))
            for r in ins.uses:
                readers.setdefault(r, []).append(i)
            for r in ins.defs:
                last_def[r] = i
                readers[r] = []
        # Priority: length of the longest dependency chain that starts at the instruction.
        users = [[] for _ in range(n)]
        for i in range(n):
            for j, delay in deps[i]:
                users[j].append((i, delay))
        height = [0] * n
        for i in reversed(range(n)):
            height[i] = max([latency[region[i].kind]] + [delay + height[u] for u, delay in users[i]])

        start, order, cycle = [None] * n, [], 0
        remaining = set(range(n))
        while remaining:
            limit = min(remaining) + window
            ready = [i for i in remaining if i <= limit and all(
                start[j] is not None and start[j] + max(delay, 1) <= cycle for j, delay in deps[i])]
            ready.sort(key=lambda i: (-height[i], i))
            slots = width
            for i in ready:
                if region[i].kind in ("fp", "perm"):
                    if slots == 0:
                        continue
                    slots -= 1
                start[i] = cycle
                order.append(i)
                remaining.discard(i)
            cycle += 1
        return [region[i] for i in order]

    # ---- register allocation ---------------------------------------------------------------

    def _intervals(self):
        """Live interval (first definition, last use) of every SIMD register."""
        start, end = {}, {}
        for i, ins in enumerate(self.ins):
            for r in ins.uses:
                if r.kind == "v":
                    if r not in start:
                        raise AllocationError("%s: %s is read before it is written" % (self.name, r))
                    # A late use keeps the register busy past the instruction's own results.
                    end[r] = i + 0.5 if r in ins.late else i
            for r in ins.defs:
                if r.kind == "v":
                    start.setdefault(r, i)
                    end[r] = max(end.get(r, i), i)
        # A value defined before a loop and used inside it stays live until the loop ends.
        labels = {ins.label: i for i, ins in enumerate(self.ins) if ins.label}
        for j, ins in enumerate(self.ins):
            if ins.target is not None:
                i = labels[ins.target]
                for r in start:
                    if start[r] < i <= end[r]:
                        end[r] = max(end[r], j)
        return start, end

    def allocate(self):
        """Assigns physical SIMD registers. Raises AllocationError if the kernel needs too many."""
        start, end = self._intervals()
        order = sorted(start, key=lambda r: (start[r], end[r]))
        free = [True] * self.simd_count
        active = []  # (end, physical register)
        assignment = {}
        peak = 0
        for r in order:
            # A register whose last use is the instruction that defines the new value can be
            # reused by it: all instructions read their inputs before writing outputs.
            for item in [a for a in active if a[0] <= start[r]]:
                active.remove(item)
                free[item[1]] = True
            p = next((p for p in range(self.simd_count) if free[p]), None)
            if p is None:
                raise AllocationError("%s: out of SIMD registers at instruction %d" % (self.name, start[r]))
            free[p] = False
            assignment[r] = p
            active.append((end[r], p))
            peak = max(peak, self.simd_count - sum(free))
        self.assignment = assignment
        self.peak = peak
        return assignment

    # ---- simulation ------------------------------------------------------------------------

    def run(self, machine, **params):
        """Interprets the kernel. `params` gives the integer values of the C parameters."""
        for reg in self.params:
            machine.reg[reg] = params[reg.name]
        for reg, values in self.consts:
            # Constant tables are placed at the very end of the simulated memory.
            machine.reg[reg] = 4 * (len(machine.mem) - len(values))
            machine.store(machine.reg[reg], list(values))
        labels = {ins.label: i for i, ins in enumerate(self.ins) if ins.label}
        pc = 0
        while pc < len(self.ins):
            ins = self.ins[pc]
            pc += 1
            if ins.sim is not None:
                machine.executed += 1
                if ins.sim(machine):
                    pc = labels[ins.target]
        return machine

    # ---- C emission ------------------------------------------------------------------------

    def format_gpr(self, reg):
        return "%[" + reg.name + "]"

    def clobbers(self, used):
        raise NotImplementedError

    def c_function(self, attributes=""):
        if self.assignment is None:
            self.allocate()
        lines = []
        for ins in self.ins:
            names = []
            for r in ins.regs:
                names.append(self.assignment[r] if r.kind == "v" else self.format_gpr(r))
            lines.append(ins.text.format(*names))
        used = sorted(set(self.assignment.values()))
        out = []
        for reg, values in self.consts:
            out.append("static const float %s_%s[%d] ONEFFT_ALIGNED(64) = {" % (self.name, reg.name, len(values)))
            for i in range(0, len(values), 4):
                out.append("    " + " ".join("%sf," % float.hex(float(v)) for v in values[i:i + 4]))
            out.append("};")
        out.append("/* " + self.doc.strip().replace("\n", "\n * ") + " */")
        args = ", ".join("%s%s%s" % (p.ctype, "" if p.ctype.endswith("*") else " ", p.name) for p in self.params)
        out.append("%sstatic void %s(%s) {" % (attributes, self.name, args))
        for p in self.unused:
            out.append("    (void)%s;" % p.name)
        for t in self.temps:
            out.append("    %s%s%s;" % (t.ctype, "" if t.ctype.endswith("*") else " ", t.name))
        for reg, _ in self.consts:
            out.append("    %s%s = %s_%s;" % (reg.ctype, reg.name, self.name, reg.name))
        out.append("    __asm__ volatile(")
        for line in "\n".join(lines).split("\n"):
            pad = "" if line.endswith(":") else "    "
            out.append('        "%s%s\\n"' % (pad, line))
        operands = ['[%s] "+r"(%s)' % (p.name, p.name) for p in self.params if p not in self.unused]
        operands += ['[%s] "=&r"(%s)' % (t.name, t.name) for t in self.temps]
        operands += ['[%s] "+r"(%s)' % (reg.name, reg.name) for reg, _ in self.consts]
        out.append("        : " + ", ".join(operands))
        out.append("        :")
        out.append("        : " + ", ".join('"%s"' % c for c in self.clobbers(used)) + ");")
        out.append("}")
        return "\n".join(out)
