#!/usr/bin/env bash
# Syncs the sources to a remote test machine (see TestHardware.md), builds there natively and runs.
#
#   scripts/remote.sh <machine> test [args]            the tests, every code path of the machine
#   scripts/remote.sh <machine> bench [args]           the benchmark with all comparison libraries
#   scripts/remote.sh <machine> third_party            fetches and builds the comparison libraries
#   scripts/remote.sh <machine> <source.c> [args]      one program, e.g. bench/passes.c
#
# <machine> is an alias from scripts/machines.local (see machines.example) or user@host.
# EXTRA_CFLAGS is passed to the compiler for the last form.
set -euo pipefail
machine=${1:?usage: remote.sh <machine> test|bench|third_party|<source.c> [args]}
what=${2:?}
shift 2
root=$(cd "$(dirname "$0")/.." && pwd)
# an alias from scripts/machines.local (see machines.example), or user@host itself
host=$(awk -v m="$machine" '$1 == m {print $2}' "$root/scripts/machines.local" 2>/dev/null || true)
host=${host:-$machine}
rsync -a --delete --exclude .git --exclude .tools --exclude '/build' --exclude '/build-*' --exclude '/bazel-*' --exclude '/scripts/machines.local' "$root/" "$host:onefft/"
tools='ONEFFT_TOOLS=$HOME/onefft-tools-src'
case $what in
test) run="bash scripts/test.sh host $*" ;;
bench) run="$tools bash scripts/build_bench.sh host >/dev/null && ./build/host/bench $*" ;;
third_party) run="$tools bash scripts/third_party.sh host 2>&1 | tail -2" ;;
*) run="cc -O2 ${EXTRA_CFLAGS:-} -Iinclude $what -lm -o build/tool && ./build/tool $*" ;;
esac
ssh -o BatchMode=yes "$host" "cd onefft && mkdir -p build && $run"
