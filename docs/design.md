# OneFFT design

This document explains how OneFFT computes a transform, why it is built that way, and how the
code is organised. Measurements that led to the decisions are in [measurements.md](measurements.md).

## Goals

* Complex to complex, single precision, power-of-two sizes, out of place.
* Interleaved data (`re, im, re, im, ...`), the format nearly all callers have.
* As fast as the hardware allows on AArch64 and x86-64, without run time code generation and
  without a planner that has to measure.
* Small and simple enough to maintain: one generator, a few kernels per instruction set, one
  runtime file.

## The idea in one paragraph

A transform is a sequence of *passes* over the data. Each pass loads as many values as fit into
the SIMD registers, performs as many butterfly stages on them as possible while they are in
registers, and stores them again. Loads and stores are a large part of the cost of an FFT, so the
number of passes should be as small as possible, which means that a pass has to make full use of
the registers. The compiler cannot be trusted with this (it spills), so every pass is one
hand-scheduled block of inline assembly. The assembly is not written by hand but produced by a
small generator that also verifies it.

## Notation

`N = 2^n` points. The input index is `i` with bits `b[n-1] .. b[0]`, the output index is `k` with
bits `k[n-1] .. k[0]`. `w_N = exp(-2 pi i / N)`. A SIMD register holds `L` floats (*lanes*), 4 on
AArch64. Complex vectors are kept *split*: one register with `L` real parts, one with the `L`
imaginary parts. Multiplying by `-i`, `-1` or `i` is then free (exchange the registers, flip the
sign of the following addition), and a general complex multiplication is four instructions for
`L` values.

A *butterfly stage* for index bit `b[j]` combines pairs of values whose index differs in that
bit: `(a, b) -> (a + w b, a - w b)`. It turns the axis of input bit `b[j]` into the axis of
output bit `k[n-1-j]`, and its twiddle `w` depends on the output bits that already exist:
`w = w_N^(2^j * (k mod 2^(n-1-j)))`. Two facts follow and shape everything else:

1. **The stages have to run from the top input bit to the bottom one.** The twiddle of a stage
   depends on the outputs of all stages for higher input bits.
2. **A stage is cheap if its bit selects registers, and expensive if it selects lanes.** If the
   two values of a pair sit in different registers, a stage is plain register arithmetic on `L`
   independent columns. If they sit in different lanes of one register, they have to be shuffled
   apart first.

## Layout of a transform (AArch64, L = 4)

With 32 registers a pass can hold 8 complex vectors (16 registers) and still have room for
intermediate results and twiddles. 8 vectors means three stages per pass: radix 8.

```
first pass    in  -> out   bits b[n-1..n-3]   reads interleaved, writes split      no twiddles
middle pass   out -> out   3 (or 2) bits      split, in place                      scalar twiddles
   ...
last pass     out -> out   bits b[2..0]       reads split, writes interleaved      vector twiddles
```

### Where the values live

The input is read in natural order, so the lanes of a loaded register hold the two lowest input
bits `b[1], b[0]`. These are the *last* bits to be transformed (fact 1) and they are expensive
(fact 2). On the other side, the lanes of the stored result have to hold the two lowest output
bits `k[1], k[0]`, which the *first* two stages produce. The whole design follows from handling
this mismatch exactly once:

* The lanes keep holding `b[1], b[0]` through the first and all middle passes. These passes only
  transform bits that select registers. Every lane is an independent column, there is no
  shuffling at all, and the twiddles are the same for all lanes of a register: one scalar per
  register instead of a vector.
* The first pass is the only pass that does not work in place, so it is the one that moves data
  to where it belongs: it writes every vector to its final position. Position bit `p[j]` (for
  `2 <= j <= n-3`) receives the axis of input bit `b[n-1-j]`, which later passes transform in
  place into `k[j]`. This is the bit reversal of a classic FFT, done for free and with whole
  vectors.
* The two top position bits and the two lane bits are special. The lanes hold `b[1], b[0]`,
  whose results `k[n-2], k[n-1]` belong at the top. The top two position bits therefore receive
  `k[0], k[1]`, which belong in the lanes. The last pass loads the four vectors of the four
  quarters, exchanges lanes and registers with a 4x4 transpose (two shuffles per register), runs
  the two final stages across registers, and stores the result, now in natural order.

So of all `n` stages only the last two need shuffles, and they need them once.

### First pass (`f8`)

For each group of 8 input vectors at stride `N/8`: split real and imaginary parts, run a plain
8 point FFT across the registers (no twiddles, only `sqrt(1/2)`), and write the 8 results to the
output buffer. Result `q = k[0] + 2 k[1] + 4 k[2]` goes to quarter `q mod 4`, and within the
quarter the two results `q / 4 = 0, 1` are adjacent.

A table tells the pass which input group to read and where its results go. For small sizes the
table simply counts through the input; for large sizes it visits the data in an order that is
kind to the cache and the TLB (see "Large transforms").

### Middle passes (`c8`, `c4`)

A middle pass that starts after `a` bits are done works on blocks of `8 * 2^a` values
(`4 * 2^a` for radix 4) within one quarter. Butterfly `j` of a block uses the 8 vectors at
distance `2^a` values from each other. Its inputs are multiplied by powers of one twiddle,
`w^1 .. w^7` with `w = exp(-2 pi i K / 2^(a+3))`, where `K` is made of the finished output bits:
the quarter and `j`. Then a plain 8 point FFT across the registers follows.

Multiplying all inputs first and then running a twiddle-free FFT needs 7 complex multiplications
for 12 butterflies, the fewest of all ways to arrange a radix 8 pass. The twiddle tables depend
only on `a`, not on `N`, and they are small: `2^a` entries.

### Last pass (`l8`, `l4`)

`l8` loads the 8 vectors that differ in the top three position bits: 4 quarters times the two
values of `b[2]`. One stage with a scalar twiddle per quarter handles `b[2]`. Then, for each of
the two halves, the 4x4 transpose makes `b[1], b[0]` select the register and `k[0], k[1]` the
lane; the three registers with `b[1], b[0] != 0` are multiplied by their twiddle vectors and a
plain 4 point FFT across the registers finishes the transform. Results are interleaved and
stored. The twiddles of this pass are different for every value, so its table has `7N/8`
entries, the only large table of a plan.

`l4` is the same without the first stage. Per index bit `l8` is the cheaper one, so `l4` is only
used where the number of bits requires it.

### Pass structure

`n = 3 + (middle bits) + 3`, with as few passes as possible:

| n  | passes        | n  | passes           |
|----|---------------|----|------------------|
| 6  | 3 3           | 10 | 3 2 2 3          |
| 7  | 3 2 2         | 11 | 3 2 3 3          |
| 8  | 3 2 3         | 12 | 3 3 3 3          |
| 9  | 3 3 3         | 13 | 3 2 2 3 3        |

Sizes up to 32 are single kernels that keep the whole transform in registers (`n8`, `n16`,
`n32`); sizes 1, 2 and 4 are a few lines of C.

The output is handled in four *regions*, its quarters, which hold the four values of
`k[1], k[0]`. Their twiddles differ, so each region has its own part of a pass's table; a kernel
call processes all of them.

### The inverse transform

`ifft(x) = swap(fft(swap(x)))` where `swap` exchanges real and imaginary parts. The kernels that
read the input and write the output therefore exist in a second version that reads the real
parts into the "imaginary" registers and vice versa. This costs nothing at run time and the
middle passes and all twiddle tables are shared between the two directions.

### The second form of the passes (Apple)

A butterfly with a twiddle, `(a + w b, a - w b)`, takes 8 operations in the form above: 4 for
the product, 4 to add and subtract. Writing `w = c (1 + i t)` it takes 6 fused multiply-adds:
`u = b (1 + i t)` is two, and each of the four results `a +- c u` is one. AArch64 only has the
destructive multiply-add (`acc += x * y`), so this needs three register copies per butterfly.

Copies are free on Apple cores, cost as much as any other SIMD operation on the Cortex-X4, and
half an operation on Cortex-A73/A75. The middle and last passes therefore exist twice
(`c8_fma`, `l8_fma`, ...); the fused form is the default on Apple platforms and about 8% faster
on an M2, the standard form is used everywhere else. `ONEFFT_NEON_FMA=0/1` overrides the choice.

The fused passes use one twiddle per butterfly stage instead of multiplying all inputs up
front, stored as `(c, t)`. `t` is a tangent, so the twiddle angle has to stay below a quarter
turn. The first stage of a pass covers half a turn: the upper half of its butterflies stores the
twiddle a quarter turn back and the kernel has a second loop body for them that multiplies by
`-i`, which is free. As a side effect the tables of these passes are about half as large.

## x86-64

16 registers hold 4 complex vectors and leave 8 for intermediate results, so the natural pass is
radix 4. A register has 4 lanes with SSE2 and 8 with AVX2. The structure is the one described
above; what differs is how the lanes are handled.

**Lane order.** Interleaved data is split with one `shufps` per register. On AVX2 shuffles work
within the two 128 bit halves of a register, so this leaves 8 consecutive values in the order
0 1 4 5 | 2 3 6 7. The same order is the one that interleaves again with one `unpcklps` /
`unpckhps` per register, so input and output cost one shuffle per register each and nothing
ever has to cross the halves for them.

**Three lane bits.** With 8 lanes the three lowest index bits sit in the lanes, but a kernel
with 4 vectors has only two register index bits to exchange them with. The exchange is spread
over the last two passes:

```
la   bits 3, 2   bit 3 across registers; exchange bit 2 with output bit 2; bit 2
lb   bits 1, 0   exchange bit 1 (the half) and bit 0 with output bits 1 and 0; bits 1, 0
```

Each exchange is a single shuffle per register (`unpcklps`/`unpckhps`), chosen so that the
output bits end up in exactly the lane order above. The exchange of the halves in `lb` is done
while loading (`vinsertf128` with a memory operand) and needs no shuffle at all. SSE2 has two
lane bits and a single last pass, `l4`, like AArch64.

**Radix 8 in 16 registers.** Stores are a bottleneck on x86 (one 256 bit store per cycle on most
cores), so a pass more or less matters. Two AVX2 kernels transform three bits although 8 complex
vectors occupy every register:

* `f8`, the first pass, has no twiddles. It computes its butterflies in place: the sum
  overwrites `a`, and the difference is formed from it as `(a + b) - 2 b`, one fused
  multiply-add that overwrites `b`. Its two constants are memory operands.
* `la8` replaces `la` and takes bits 4, 3 and 2. It exchanges bit 2 while loading (a shuffle
  with a memory operand produces two registers from one register and one memory vector), and
  applies each twiddle as three shears, `re += T im; im -= S re; re += T im` with
  `T = tan(angle/2)`, `S = sin(angle)`, again with the table entries as memory operands. It uses
  the same trick as the fused AArch64 passes to keep the angles below a quarter turn.

With these, AVX2 needs `n/2 - 1` passes for an even number of bits `n` and `(n-1)/2` for an odd
one. SSE2 has no fused multiply-add and stays with radix 4 everywhere (and a radix 2 first pass
for odd `n`); it exists for the few x86-64 processors without AVX2 and was not tuned.

### AVX-512

16 lanes and 32 registers: four index bits sit in the lanes, a kernel holds 8 complex vectors
with room to spare, and scalar twiddles are embedded broadcast operands. The passes are a radix
8 first pass, radix 4 middle passes, and two last passes that exchange two lane bits each:

```
la3 / la2   bits (4,) 3, 2   (bit 4 across registers;) exchange and transform bit 3, then bit 2
lb          bits 1, 0        exchange both, then transform them as in the other last passes
```

Each exchange is again one shuffle per register (`unpcklps`/`unpckhps` within the 128 bit blocks,
`vshuff32x4` between them), which works out in exactly one arrangement: index bits 3, 2, 1, 0
trade places with output bits 3, 1, 2, 0. The top four position bits of the output therefore
hold output bits 0, 2, 1, 3 until the last passes, and the first pass leaves the index bit that
becomes output bit 3 at its place instead of bit reversing it.

These kernels are verified in simulation and under Intel's emulator (SDE), but they have not
run on hardware and are not tuned: no radix 8 middle pass yet, and nothing is known about how
they compare to the AVX2 kernels on a given processor. They are used from 512 points up on
processors with AVX-512F; `-DONEFFT_NO_AVX512` leaves them out.

## Large transforms

Up to a few thousand points everything is in the first level cache and a pass is one kernel
call over the whole output. Beyond that:

* The middle passes work on blocks that are nested: a block of one pass consists of whole
  blocks of the pass before. The driver therefore does not run pass after pass over the whole
  array. It finishes all passes whose blocks fit the cache on one piece (32 KiB) before moving
  to the next piece, and then recursively the same for the larger blocks. Every pass finds its
  data in the cache level that its block size fits into. This is worth 17% at 2^21 points on the
  older phones and nothing on cores with large caches.
* The first pass reads the input in order and writes every butterfly to the bit reversed
  position, that is, to scattered cache lines once the output exceeds the cache. Two
  alternatives were measured and are slower: reading the input with a stride so that
  consecutive butterflies write to neighbouring positions (up to 60% slower), and prefetching
  the lines to be written (a few percent either way).
* The last pass walks 8 (4) streams and the twiddle table in order.

## Why not ...

**... the structure of the 2021 proof of concept (`reference/`)?** It transforms 32 values that
are all in the registers in one go: 5 stages per pass instead of 3. To get 32 values of *one*
column into the registers it has to gather and scatter them one lane at a time (`ld2`/`st2` of
single lanes), and it needs the lane/register exchange in every pass. On every core measured,
lane-wise loads and stores cost about as much as a full register load, so a pass moves the data
with four times as many instructions. Loading whole vectors and treating the lanes as
independent columns is cheaper per stage even though it needs more passes.

**... radix 16 passes on AArch64?** 16 complex vectors are 32 registers. Nothing would be left
for intermediate results, and the in-place tricks of the AVX2 kernels need memory operands.

**... LD2/ST2 to split and interleave?** `ldp` + `uzp1`/`uzp2` and `zip1`/`zip2` + `stp` are
never slower on the cores measured and faster on some (LD2 and ST2 run at one per cycle on
Cortex-X4 and Apple M2), and they have no addressing restrictions.

## The generator

`gen/onefft_gen` is a few hundred lines of Python:

* `ir.py` holds a kernel as a list of instructions on virtual registers. Every instruction
  carries its assembly text, the registers it reads and writes, and a Python function that
  simulates it. The module allocates SIMD registers (linear scan), reorders instructions
  (below), interprets kernels on a small memory model, and prints the C function.
* `neon.py` and `x86.py` add one method per machine instruction. `x86.py` serves SSE2 and AVX2
  from one description and lowers three-operand code to two-operand SSE2 instructions.
* `cplx.py` builds complex arithmetic and FFT networks from those methods, independent of the
  target. It tracks pending multiplications by `-i` symbolically so that they cost nothing.
* `passes.py` has the loop structure shared by the pass kernels of all targets.
* `neon_kernels.py` and `x86_kernels.py` describe the kernels in terms of the above: a kernel is
  typically 30 lines.
* `model.py` is a Python model of the driver in `src/onefft.c`; `gen/test_kernels.py` uses it to
  run complete transforms in simulation against a reference FFT. A kernel bug is found in a
  second on any machine, with values that can be inspected, before anything is compiled.

General purpose registers are not allocated by the generator. They become named operands of the
inline assembly block and the C compiler assigns them, which keeps the kernels independent of the
calling convention. The compiler also saves and restores callee-saved SIMD registers because they
are listed as clobbered.

### Instruction scheduling

Kernels are written stage by stage. Executed in that order they are slow on cores with a small
out-of-order window: after the loads at the top the window fills with instructions that all wait
for the same few values. The generator therefore reorders each loop body the way an idealised
core would execute it: a list scheduler picks, cycle by cycle, the ready instructions with the
longest remaining dependency chain. Loop bodies are also unrolled twice, so that the scheduler
can overlap the end of one butterfly with the start of the next. Both together are worth about
10% on the Cortex-A73/A75 class and a few percent on current cores.

The scheduler needs to know which loads and stores may touch the same memory. Kernels follow one
rule that makes this trivial: accesses through different address registers never overlap.

On x86-64 the order of the instructions made no measurable difference (the out-of-order windows
are large), and the loops are not unrolled there.

## The runtime

`src/onefft.c` contains the plan (pass structure and tables), the table builders, the driver
that calls the kernels, a portable C implementation for targets without kernels, and the public
interface. Every instruction set provides kernels of the same four kinds (complete small
transform, first, middle and last pass) with the same signatures, so one driver serves all of
them; only the code that decides on the passes and fills the tables is specific. On x86-64 the
instruction set is chosen when a plan is created.

Plans are immutable after creation, so one plan can be used from any number of threads at the
same time. The functions without plans keep one plan per size and direction in a table of
atomic pointers that are written once with a compare-and-swap.
