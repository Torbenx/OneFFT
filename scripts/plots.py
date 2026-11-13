#!/usr/bin/env python3
"""Draws the benchmark charts in docs/plots from the result files in docs/results.

    scripts/plots.py [--png]

One chart per machine, as <machine>.svg for light and <machine>-dark.svg for dark pages. A chart
shows the speed of every library over the transform size, in the unit FFT benchmarks commonly
use: "mflops" = 5 n log2(n) / (time in microseconds), the number of floating point operations
of a textbook radix 2 transform per microsecond. Unlike the time itself this is comparable
across sizes: a library whose cost per operation does not change draws a horizontal line.

Needs matplotlib. --png also writes PNG files next to the SVGs, to look at the result.
"""

import math
import os
import sys

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt  # noqa: E402

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))

MACHINES = {  # file name: chart title
    "s24": "Cortex-X4 (Samsung Galaxy S24), AArch64",
    "m2": "Apple M2, AArch64",
    "pixel3a": "Kryo 360 Gold (Google Pixel 3a), AArch64",
    "a50": "Cortex-A73 (Samsung Galaxy A50), AArch64",
    "zen5": "AMD EPYC 9255 (Zen 5), AVX-512",
    "zen4": "AMD EPYC 9254 (Zen 4), AVX-512",
    "intel": "Intel Core i5-9600K (Skylake), AVX2",
    "epyc": "AMD EPYC 7443P (Zen 3), AVX2",
    "zen2": "AMD Ryzen 7 5700U (Zen 2), AVX2",
}

# Series in a fixed order with a fixed colour each: the colour follows the library, in every
# chart. Colours and their order are a palette validated for colour vision deficiencies.
SERIES = [  # result name, label, light colour, dark colour
    ("onefft", "OneFFT", "#2a78d6", "#3987e5"),
    ("onefft_avx2", "OneFFT, AVX2 kernels", "#2a78d6", "#3987e5"),  # a variant: OneFFT's colour, dashed
    ("fftw", "FFTW", "#eb6834", "#d95926"),
    ("ffmpeg", "FFmpeg", "#1baf7a", "#199e70"),
    ("pffft", "PFFFT", "#eda100", "#c98500"),
    ("vdsp", "vDSP", "#e87ba4", "#d55181"),
]

THEMES = {
    "": {"surface": "#fcfcfb", "primary": "#0b0b0b", "secondary": "#52514e", "muted": "#898781",
         "grid": "#e1e0d9", "axis": "#c3c2b7", "colour": 2},
    "-dark": {"surface": "#1a1a19", "primary": "#f0efec", "secondary": "#c3c2b7", "muted": "#898781",
              "grid": "#2c2c2a", "axis": "#383835", "colour": 3},
}

# Variants of a library are drawn dashed in its colour; the value is the short label at the line end.
VARIANTS = {"onefft_avx2": "AVX2"}
DASHED = (0, (2.5, 2.5))

FONT = "system-ui, -apple-system, 'Segoe UI', 'DejaVu Sans', sans-serif"
MIN_SIZE = 4  # smaller transforms are a handful of operations and say nothing about speed


def read(path):
    """{library: [(n, mflops), ...]}"""
    data = {}
    with open(path) as f:
        for line in f:
            fields = line.split()
            if len(fields) < 3:
                continue
            name, n, ns = fields[0], int(fields[1]), float(fields[2])
            if n >= MIN_SIZE:
                data.setdefault(name, []).append((n, 5.0 * n * math.log2(n) / (ns / 1000.0)))
    return data


def size_label(n):
    for unit, suffix in ((1 << 20, "M"), (1 << 10, "K")):
        if n >= unit:
            return "%d%s" % (n // unit, suffix)
    return str(n)


def spread(positions, gap):
    """Moves label positions (sorted ascending) apart until neighbours are `gap` apart."""
    positions = list(positions)
    for _ in range(100):
        moved = False
        for i in range(len(positions) - 1):
            overlap = gap - (positions[i + 1] - positions[i])
            if overlap > 1e-9:
                positions[i] -= overlap / 2
                positions[i + 1] += overlap / 2
                moved = True
        if not moved:
            break
    return positions


def chart(title, data, theme):
    fig = plt.figure(figsize=(7.6, 3.9), dpi=100, facecolor=theme["surface"])
    ax = fig.add_axes([0.075, 0.115, 0.80, 0.66], facecolor=theme["surface"])
    series = [s for s in SERIES if s[0] in data]
    top = max(v for name in data for _, v in data[name])
    # round the axis up to a clean step
    step = 10 ** math.floor(math.log10(top))
    step = step / 2 if top / step < 2.5 else step * 2 if top / step > 7 else step
    y_max = step * math.ceil(top * 1.02 / step)

    ax.set_xscale("log", base=2)
    ax.set_xlim(MIN_SIZE / 1.25, (1 << 20) * 1.25)
    ax.set_ylim(0, y_max)
    ticks = [1 << k for k in range(2, 21, 2)]
    ax.set_xticks(ticks)
    ax.set_xticklabels([size_label(n) for n in ticks])
    ax.set_xticks([], minor=True)
    y_ticks = [step * i for i in range(int(round(y_max / step)) + 1)]
    ax.set_yticks(y_ticks)
    ax.set_yticklabels(["0" if v == 0 else ("%gK" % (v / 1000) if y_max >= 10000 else "%d" % v) for v in y_ticks])
    ax.tick_params(axis="both", length=0, labelsize=9, labelcolor=theme["muted"], pad=5)
    ax.grid(axis="y", color=theme["grid"], linewidth=1)
    ax.set_axisbelow(True)
    for side in ("top", "right", "left"):
        ax.spines[side].set_visible(False)
    ax.spines["bottom"].set_color(theme["axis"])
    ax.spines["bottom"].set_linewidth(1)

    # lines, OneFFT last so that it stays on top; an end dot with a ring in the surface colour
    for name, label, *colours in reversed(series):
        colour = colours[theme["colour"] - 2]
        xs, ys = zip(*data[name])
        ax.plot(xs, ys, color=colour, linewidth=2, solid_capstyle="round", solid_joinstyle="round",
                dash_capstyle="round", linestyle=DASHED if name in VARIANTS else "-", zorder=3 if name == "onefft" else 2)
        ax.plot(xs[-1:], ys[-1:], marker="o", markersize=8, markerfacecolor=colour, markeredgecolor=theme["surface"],
                markeredgewidth=2, zorder=4, clip_on=False)

    # direct labels at the line ends, moved apart where the lines end close together and then
    # tied to their line with a thin leader
    ends = sorted((data[name][-1][1], VARIANTS.get(name, label)) for name, label, *_ in series)
    gap = y_max * 0.058
    placed = spread([y for y, _ in ends], gap)
    x_end, x_text = 1 << 20, (1 << 20) * 2.1
    for (y, label), y_text in zip(ends, placed):
        if abs(y_text - y) > gap * 0.3:
            ax.plot([x_end * 1.3, x_text * 0.93], [y, y_text], color=theme["muted"], linewidth=0.8, clip_on=False, zorder=1)
        ax.text(x_text, y_text, label, color=theme["secondary"], fontsize=9.5, va="center", ha="left", clip_on=False)

    # title, subtitle and a legend row (text in ink colours, identity from the line key)
    fig.text(0.02, 0.945, title, color=theme["primary"], fontsize=12, fontweight="bold", va="center")
    fig.text(0.02, 0.885, "Speed in mflops = 5 n log₂(n) / (time in µs), by transform size n. Higher is better.",
             color=theme["secondary"], fontsize=9, va="center")
    x = 0.022
    for name, label, *colours in series:
        colour = colours[theme["colour"] - 2]
        fig.add_artist(plt.Line2D([x, x + 0.03], [0.825, 0.825], color=colour, linewidth=2, solid_capstyle="round",
                                  dash_capstyle="round", linestyle=(0, (1.2, 2.2)) if name in VARIANTS else "-"))
        fig.text(x + 0.038, 0.825, label, color=theme["secondary"], fontsize=9, va="center")
        x += 0.052 + (0.0108 if name in VARIANTS else 0.0125) * len(label)  # long labels run narrower per letter
    fig.text(0.875, 0.045, "n", color=theme["muted"], fontsize=9, va="center", ha="right", style="italic")
    return fig


def main():
    results = os.path.join(ROOT, "docs", "results")
    out = os.path.join(ROOT, "docs", "plots")
    os.makedirs(out, exist_ok=True)
    plt.rcParams["svg.fonttype"] = "none"  # keep text as text
    plt.rcParams["svg.hashsalt"] = "onefft"  # stable ids: unchanged data gives an unchanged file
    plt.rcParams["font.family"] = "DejaVu Sans"
    for machine, title in MACHINES.items():
        path = os.path.join(results, machine + ".txt")
        if not os.path.exists(path):
            continue
        data = read(path)
        for suffix, theme in THEMES.items():
            fig = chart(title, data, theme)
            target = os.path.join(out, machine + suffix + ".svg")
            fig.savefig(target, format="svg", facecolor=theme["surface"], metadata={"Date": None, "Creator": None})
            if "--png" in sys.argv:
                fig.savefig(target[:-4] + ".png", dpi=150, facecolor=theme["surface"])
            plt.close(fig)
            # the layout was computed with a font that is surely installed here; let the
            # viewer's system font take over
            with open(target) as f:
                svg = f.read()
            svg = svg.replace("'DejaVu Sans'", FONT)
            with open(target, "w") as f:
                f.write(svg)
        print("wrote", os.path.join("docs", "plots", machine + ".svg"), "and -dark.svg")
    return 0


if __name__ == "__main__":
    sys.exit(main())
