#!/usr/bin/env python3
"""Formats the output of the benchmark as a Markdown table: one row per size, one column per
implementation with the time in nanoseconds, and the speed of OneFFT relative to the others.

    build/host/bench | scripts/bench_table.py
"""

import sys


def main():
    rows, names = {}, []
    for line in sys.stdin:
        fields = line.split()
        if len(fields) < 3:
            continue
        name, n, ns = fields[0], int(fields[1]), float(fields[2])
        rows.setdefault(n, {})[name] = ns
        if name not in names:
            names.append(name)
    others = [name for name in names if name != "onefft"]
    print("| n | " + " | ".join(names) + " | " + " | ".join("vs " + name for name in others) + " |")
    print("|--:|" + "--:|" * (len(names) + len(others)))
    for n in sorted(rows):
        row = rows[n]

        def time(name):
            if name not in row:
                return "-"
            return "%.1f" % row[name] if row[name] < 1000 else "%.0f" % row[name]

        ratios = ["%.2f" % (row[name] / row["onefft"]) if name in row else "-" for name in others]
        print("| %d | " % n + " | ".join(time(name) for name in names) + " | " + " | ".join(ratios) + " |")
    return 0


if __name__ == "__main__":
    sys.exit(main())
