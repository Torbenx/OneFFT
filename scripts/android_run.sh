#!/usr/bin/env bash
# Pushes a binary to an Android device and runs it pinned to the fastest core cluster.
#
#   scripts/android_run.sh <device> <binary> [args...]
#
# <device> is an alias from scripts/machines.local (see machines.example) or an adb serial.
set -euo pipefail
device=${1:?usage: android_run.sh <device> <binary> [args...]}
binary=${2:?}
shift 2
root=$(cd "$(dirname "$0")/.." && pwd)
serial=$(awk -v m="$device" '$1 == m {print $2}' "$root/scripts/machines.local" 2>/dev/null || true)
serial=${serial:-$device}
adb -s "$serial" push "$binary" /data/local/tmp/ >/dev/null
name=$(basename "$binary")
# Pin to the cores with the highest maximum frequency (the "big" or "prime" cluster).
mask=$(adb -s "$serial" shell 'best=0; mask=0; i=0
for f in /sys/devices/system/cpu/cpu[0-9]*/cpufreq/cpuinfo_max_freq; do
  c=${f#/sys/devices/system/cpu/cpu}; c=${c%%/*}; v=$(cat $f)
  if [ "$v" -gt "$best" ]; then best=$v; mask=$((1 << c)); elif [ "$v" -eq "$best" ]; then mask=$((mask | (1 << c))); fi
done; printf "%x" $mask')
adb -s "$serial" shell "cd /data/local/tmp && taskset $mask ./$name $*"
