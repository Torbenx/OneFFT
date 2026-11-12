#!/usr/bin/env python3
"""Rewrites the benchmark tables in docs/measurements.md from the result files in docs/results
and redraws the charts in docs/plots (scripts/plots.py).

    scripts/update_docs.py

docs/results holds the output of bench/bench.c per machine: pixel3a.txt, a50.txt, s24.txt,
m2.txt, zen2.txt, intel.txt, epyc.txt, intel_sse2.txt. A table is placed between the markers
<!-- results:NAME --> and <!-- end -->.
"""

import os
import re
import subprocess
import sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
TITLES = {"onefft": "OneFFT", "fftw": "FFTW", "ffmpeg": "FFmpeg", "pffft": "PFFFT", "vdsp": "vDSP"}


def table(path, sizes):
    with open(path) as f:
        out = subprocess.run([os.path.join(ROOT, "scripts", "bench_table.py")], stdin=f, capture_output=True,
                             text=True, check=True).stdout.splitlines()
    header = out[0]
    for name, title in TITLES.items():
        header = header.replace(" %s " % name, " %s " % title)
    rows = [line for line in out[2:] if sizes is None or int(line.split("|")[1]) in sizes]
    return "\n".join([header, out[1]] + rows)


def update(document, results, sizes):
    path = os.path.join(ROOT, document)
    with open(path) as f:
        text = f.read()

    def replace(match):
        source = os.path.join(results, match.group(1) + ".txt")
        if not os.path.exists(source):
            return match.group(0)
        return "<!-- results:%s -->\n%s\n<!-- end -->" % (match.group(1), table(source, sizes))

    text = re.sub(r"<!-- results:(\w+) -->.*?<!-- end -->", replace, text, flags=re.S)
    with open(path, "w") as f:
        f.write(text)


def main():
    update(os.path.join("docs", "measurements.md"), os.path.join(ROOT, "docs", "results"), None)
    return subprocess.run([sys.executable, os.path.join(ROOT, "scripts", "plots.py")]).returncode


if __name__ == "__main__":
    sys.exit(main())
