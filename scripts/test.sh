#!/usr/bin/env bash
# Builds and runs the correctness tests.
#
#   scripts/test.sh host [args]          on this machine
#   scripts/test.sh <device> [args]      on an Android device (alias or serial, see android_run.sh)
#
# For the other test machines use scripts/remote.sh <machine> test.
set -euo pipefail
target=${1:?usage: test.sh host|<android device> [args]}
shift
root=$(cd "$(dirname "$0")/.." && pwd)
warnings="-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wno-sign-conversion -Werror"
case $target in
host)
    # Every code path this machine can run: the best instruction set, then the builds without
    # the better ones, down to portable C.
    mkdir -p "$root/build/host"
    case $(uname -m) in
    x86_64) variants=("" -DONEFFT_NO_AVX512 -DONEFFT_NO_AVX2 -DONEFFT_NO_SIMD) ;;
    *) variants=("" -DONEFFT_NEON_FMA=0 -DONEFFT_NEON_FMA=1 -DONEFFT_NO_SIMD) ;; # both forms of the passes
    esac
    for variant in "${variants[@]}"; do
        ${CC:-cc} -O2 $warnings $variant -I"$root/include" "$root/tests/test_onefft.c" "$root/src/onefft.c" -lm -lpthread -o "$root/build/host/test_onefft"
        echo "[${variant:-default}]"
        "$root/build/host/test_onefft" "$@"
    done
    ${CC:-cc} -O2 -c -I"$root/include" "$root/src/onefft.c" -o "$root/build/host/onefft.o"
    ${CXX:-c++} -std=c++11 -O2 -Wall -Wextra -Wpedantic -Werror -I"$root/include" "$root/tests/test_cpp.cpp" \
        "$root/build/host/onefft.o" -lm -o "$root/build/host/test_cpp"
    "$root/build/host/test_cpp"
    ;;
*)
    cc=${ANDROID_NDK_HOME:-$root/.tools/android-ndk-r27c}/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android24-clang
    mkdir -p "$root/build/android-arm64"
    for v in 0 1; do # both forms of the middle and last passes
        "$cc" -O2 $warnings -DONEFFT_NEON_FMA=$v -I"$root/include" "$root/tests/test_onefft.c" "$root/src/onefft.c" -lm -o "$root/build/android-arm64/test_onefft"
        "$root/scripts/android_run.sh" "$target" "$root/build/android-arm64/test_onefft" "$@"
    done
    ;;
esac
