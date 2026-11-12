"""Complex arithmetic and FFT butterfly networks on top of a target backend.

Complex vectors are kept in split form: one SIMD register holds the real parts, another the
imaginary parts. Everything here is target independent. It only needs these backend methods:

    cx()                 -> fresh (re, im) register pair
    add(d, a, b)         d = a + b
    sub(d, a, b)         d = a - b
    mul(d, a, w)         d = a * w
    mla(d, a, w)         d += a * w
    mls(d, a, w)         d -= a * w

where `w` is whatever the backend accepts as a multiplier: a register, or for example a
(register, lane) tuple on targets with multiply-by-element instructions.
"""

import cmath
import math


class Cx:
    """A complex vector in two registers, times a pending rotation.

    The represented value is (re + i*im) * (-i)**rot. Multiplications by -i, -1 and i are the
    trivial twiddles of an FFT. Tracking them symbolically instead of executing them makes them
    free: the next addition or subtraction simply picks the matching operands and signs.
    """

    __slots__ = ("re", "im", "rot")

    def __init__(self, re, im, rot=0):
        self.re = re
        self.im = im
        self.rot = rot & 3

    def rotated(self, quarter_turns):
        """This value times (-i)**quarter_turns."""
        return Cx(self.re, self.im, self.rot + quarter_turns)


def butterfly(be, a, b):
    """Returns (a + b, a - b)."""
    # Express b relative to a's pending rotation: b * (-i)**d has the components
    #   d=0: (re, im)   d=1: (im, -re)   d=2: (-re, -im)   d=3: (-im, re)
    d = (b.rot - a.rot) & 3
    b_re, re_negative = ((b.re, False), (b.im, False), (b.re, True), (b.im, True))[d]
    b_im, im_negative = ((b.im, False), (b.re, True), (b.im, True), (b.re, False))[d]
    s_re, s_im = be.cx()
    d_re, d_im = be.cx()
    (be.sub if re_negative else be.add)(s_re, a.re, b_re)
    (be.sub if im_negative else be.add)(s_im, a.im, b_im)
    (be.add if re_negative else be.sub)(d_re, a.re, b_re)
    (be.add if im_negative else be.sub)(d_im, a.im, b_im)
    return Cx(s_re, s_im, a.rot), Cx(d_re, d_im, a.rot)


def butterfly_in_place(be, a, b):
    """Returns (a + b, a - b), computed in the registers of a and b without any other register.

    The difference is formed from the sum: a - b = (a + b) - 2 b. Needs the backend method
    difference_from_sum(), a fused multiply-add. Used where a kernel has no register to spare.
    """
    d = (b.rot - a.rot) & 3
    b_re, re_negative = ((b.re, False), (b.im, False), (b.re, True), (b.im, True))[d]
    b_im, im_negative = ((b.im, False), (b.re, True), (b.im, True), (b.re, False))[d]
    (be.sub if re_negative else be.add)(a.re, a.re, b_re)
    (be.sub if im_negative else be.add)(a.im, a.im, b_im)
    be.difference_from_sum(b_re, a.re, plus=re_negative)
    be.difference_from_sum(b_im, a.im, plus=im_negative)
    return Cx(a.re, a.im, a.rot), Cx(b_re, b_im, a.rot)


def cmul_w8_in_place(be, x, sqrt_half):
    """x * exp(-i*pi/4) in the registers of x; needs the backend method double_minus()."""
    be.add(x.re, x.re, x.im)  # re + im
    be.double_minus(x.im, x.re)  # 2 im - (re + im) = im - re
    be.mul(x.re, x.re, sqrt_half)
    be.mul(x.im, x.im, sqrt_half)
    return Cx(x.re, x.im, x.rot)


def butterfly_shear(be, a, b, tan_half, sin):
    """Returns (a + w b, a - w b) for w = exp(-i phi), entirely in the registers of a and b.

    b is rotated by three shears (the lifting scheme), which need tan(phi/2) and sin(phi) as
    multiplier operands; then butterfly_in_place() follows. Seven fused operations and no
    register besides the four of a and b, for kernels that have none to spare. |phi| should
    not exceed a quarter turn by much: the error grows with tan(phi/2).
    """
    be.mla(b.re, b.im, tan_half)
    be.mls(b.im, b.re, sin)
    be.mla(b.re, b.im, tan_half)
    return butterfly_in_place(be, a, b)


def butterfly_tan(be, a, b, c, t):
    """Returns (a + w b, a - w b) for the twiddle w = c * (1 + i*t), reusing the registers of a and b.

    Six fused multiply-adds instead of the eight operations of cmul() and butterfly(): first
    u = b (1 + i t), then a +- c u. With the destructive multiply-add of AArch64 this takes three
    register copies, so it only pays off where copies are free (Apple cores).

    `c` and `t` are multiplier operands. The twiddle must have a nonzero real part.
    """
    saved = be.vreg()
    be.mov(saved, b.re)
    be.mls(b.re, b.im, t)  # u.re = b.re - t b.im
    be.mla(b.im, saved, t)  # u.im = b.im + t b.re
    # u carries the pending rotation of b; see butterfly() for the component table.
    d = (b.rot - a.rot) & 3
    u_re, re_negative = ((b.re, False), (b.im, False), (b.re, True), (b.im, True))[d]
    u_im, im_negative = ((b.im, False), (b.re, True), (b.im, True), (b.re, False))[d]
    d_re, d_im = be.cx()
    be.mov(d_re, a.re)
    be.mov(d_im, a.im)
    (be.mls if re_negative else be.mla)(a.re, u_re, c)
    (be.mls if im_negative else be.mla)(a.im, u_im, c)
    (be.mla if re_negative else be.mls)(d_re, u_re, c)
    (be.mla if im_negative else be.mls)(d_im, u_im, c)
    return Cx(a.re, a.im, a.rot), Cx(d_re, d_im, a.rot)


def fft_staged(be, xs, twiddle, first_quarter_turn=False, combine=None):
    """Forward FFT across 2, 4 or 8 complex vectors with a twiddle in every stage.

    xs[i] is input i; the result is in natural order. This is the radix 2 network of a pass
    that transforms len(xs) = 2^r index bits when K, the number formed by the output bits that
    are already known, has `a` bits: stage m = 1..r uses, for the group whose new output bits
    so far form the number g < 2^(m-1), the twiddle

        exp(-2 pi i (K + 2^a g) / 2^(a + m)) = exp(-2 pi i K / 2^(a + m)) * exp(-2 pi i g / 2^m)

    `twiddle(m, eighth)` returns the two multipliers that describe exp(-2 pi i K / 2^(a+m)),
    times exp(-i pi/4) if `eighth`, to `combine`: butterfly_tan (the default) or
    butterfly_shear. The remaining quarter turns of the second factor are free. If
    `first_quarter_turn`, the twiddle of stage 1 is additionally multiplied by -i (the caller
    uses this for the upper half of the K range, to keep real parts away from zero).
    """
    n = len(xs)
    r = n.bit_length() - 1
    assert n in (2, 4, 8)
    combine = combine or butterfly_tan
    # groups[g] holds the values with new output bits g, ordered by the remaining input bits
    groups = {0: list(xs)}
    for m in range(1, r + 1):
        result = {}
        for g, values in groups.items():
            half = len(values) // 2
            turns = g * 8 >> m  # the second factor above in eighths of a turn
            c, t = twiddle(m, turns % 2 == 1)
            quarter = turns // 2 + (1 if m == 1 and first_quarter_turn else 0)
            lows, highs = [], []
            for j in range(half):
                low, high = combine(be, values[j], values[j + half].rotated(quarter), c, t)
                lows.append(low)
                highs.append(high)
            result[g], result[g + (1 << (m - 1))] = lows, highs
        groups = result
    out = [groups[q][0] for q in range(n)]
    assert all(x.rot == 0 for x in out)
    return out


def cmul(be, x, w_re, w_im):
    """Returns x * (w_re + i*w_im). The pending rotation of x carries over."""
    t_re, t_im = be.cx()
    be.mul(t_re, x.re, w_re)
    be.mul(t_im, x.im, w_re)
    be.mls(t_re, x.im, w_im)
    be.mla(t_im, x.re, w_im)
    return Cx(t_re, t_im, x.rot)


def cmul_w8(be, x, sqrt_half):
    """Returns x * exp(-i*pi/4) = x * (1 - i) * sqrt(1/2). `sqrt_half` is a multiplier operand."""
    s_re, s_im = be.cx()
    be.add(s_re, x.re, x.im)
    be.sub(s_im, x.im, x.re)
    t_re, t_im = be.cx()
    be.mul(t_re, s_re, sqrt_half)
    be.mul(t_im, s_im, sqrt_half)
    return Cx(t_re, t_im, x.rot)


def bit_reverse(value, bits):
    result = 0
    for _ in range(bits):
        result = (result << 1) | (value & 1)
        value >>= 1
    return result


def fft_plain(be, xs, sqrt_half=None, in_place=False):
    """Forward FFT across a list of 2, 4 or 8 complex vectors, natural order in and out.

    Every lane is transformed independently. Only trivial twiddles and exp(-i*pi/4) occur, the
    latter needs the `sqrt_half` multiplier operand for 8 points. With `in_place` the result
    is computed in the registers of the input and no further register is used.
    """
    n = len(xs)
    bits = n.bit_length() - 1
    assert n in (2, 4, 8)
    a = [xs[bit_reverse(i, bits)] for i in range(n)]
    size = 2
    while size <= n:
        half = size // 2
        # First all twiddles of the stage, then all butterflies: independent instructions end up
        # next to each other, which suits both in-order and out-of-order cores.
        for start in range(0, n, size):
            for k in range(1, half):
                eighths = k * 8 // size  # the twiddle is exp(-2*pi*i * eighths/8)
                y = a[start + half + k]
                if eighths % 2:
                    y = (cmul_w8_in_place if in_place else cmul_w8)(be, y, sqrt_half)
                a[start + half + k] = y.rotated(eighths // 2)
        for start in range(0, n, size):
            for k in range(half):
                a[start + k], a[start + half + k] = (butterfly_in_place if in_place else butterfly)(
                    be, a[start + k], a[start + half + k])
        size *= 2
    assert all(x.rot == 0 for x in a)
    return a


def root(k, n):
    """exp(-2*pi*i*k/n), exact for multiples of a quarter turn."""
    k %= n
    if (4 * k) % n == 0:
        return (1, -1j, -1, 1j)[4 * k // n]
    return cmath.exp(-2j * math.pi * k / n)
