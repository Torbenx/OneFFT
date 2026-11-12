#!/usr/bin/env bash
# Developer tool: builds bench/passes.c with experimental generator options and runs it.
#
#   scripts/experiment.sh <device> "<ONEFFT_GEN options>" <log2n>...
#
# <device> is an Android alias (see android_run.sh) or, with "remote:" in front, a machine of
# remote.sh, e.g. remote:m2.
# ONEFFT_GEN_X86 sets the options of the x86 kernels. The generated source in src/ is
# restored to the default options afterwards.
set -euo pipefail
device=$1; opts=$2; shift 2
root=$(cd "$(dirname "$0")/.." && pwd)
trap 'python3 "$root/gen/generate.py" >/dev/null' EXIT
ONEFFT_GEN="$opts" python3 "$root/gen/generate.py" >/dev/null
if [ "${device#remote:}" != "$device" ]; then
    "$root/scripts/remote.sh" "${device#remote:}" bench/passes.c "$@"
else
    cc=${ANDROID_NDK_HOME:-$root/.tools/android-ndk-r27c}/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android24-clang
    mkdir -p "$root/build/android-arm64"
    "$cc" -O2 ${EXTRA_CFLAGS:-} -I"$root/include" "$root/bench/passes.c" -lm -o "$root/build/android-arm64/passes_exp"
    "$root/scripts/android_run.sh" "$device" "$root/build/android-arm64/passes_exp" "$@"
fi
