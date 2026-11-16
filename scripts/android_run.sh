#!/usr/bin/env bash
# Pushes a binary to an Android device and runs it pinned to the fastest core cluster.
#
#   scripts/android_run.sh <device> <binary> [args...]
#
# <device> is an alias from scripts/machines.local (see machines.example) or an adb serial. "-"
# takes it from ONEFFT_DEVICE, and without that adb picks the device (ANDROID_SERIAL, or the only
# one connected); this is how ctest runs the tests (the android preset in CMakePresets.json).
# The exit status is the binary's.
set -euo pipefail
device=${1:?usage: android_run.sh <device> <binary> [args...]}
binary=${2:?}
shift 2
root=$(cd "$(dirname "$0")/.." && pwd)
[ "$device" != - ] || device=${ONEFFT_DEVICE:-}
adb=(adb)
if [ -n "$device" ]; then
    serial=$(awk -v m="$device" '$1 == m {print $2}' "$root/scripts/machines.local" 2>/dev/null || true)
    adb+=(-s "${serial:-$device}")
fi
"${adb[@]}" push "$binary" /data/local/tmp/ >/dev/null
# shared libraries next to the binary (the benchmark's libkfr_capi.so) go along with it
for lib in "$(dirname "$binary")"/*.so; do
    [ -f "$lib" ] && "${adb[@]}" push "$lib" /data/local/tmp/ >/dev/null
done
name=$(basename "$binary")
# Pin to the cores with the highest maximum frequency (the "big" or "prime" cluster).
mask=$("${adb[@]}" shell 'best=0; mask=0; i=0
for f in /sys/devices/system/cpu/cpu[0-9]*/cpufreq/cpuinfo_max_freq; do
  c=${f#/sys/devices/system/cpu/cpu}; c=${c%%/*}; v=$(cat $f)
  if [ "$v" -gt "$best" ]; then best=$v; mask=$((1 << c)); elif [ "$v" -eq "$best" ]; then mask=$((mask | (1 << c))); fi
done; printf "%x" $mask')
"${adb[@]}" shell "cd /data/local/tmp && LD_LIBRARY_PATH=/data/local/tmp taskset $mask ./$name $*"
