#!/usr/bin/env bash
# Builds the benchmark with every comparison library available for the target.
#
#   scripts/build_bench.sh <target>      target: android-arm64 | host
#
# Output: build/<target>/bench. Run scripts/third_party.sh <target> first to get the libraries.
# EXTRA_CFLAGS is added to the compiler flags, e.g. -DONEFFT_NO_AVX2 to measure the SSE2 kernels.
# TwoFFT (headers only) is compiled from its source tree if there is one at TWOFFT_DIR
# (default ../TwoFFT), on AArch64 only: its x86-64 backend is not finished.
set -euo pipefail
target=${1:?usage: build_bench.sh android-arm64|host}
root=$(cd "$(dirname "$0")/.." && pwd)
tools=${ONEFFT_TOOLS:-$root/.tools}
tp=$tools/$target
out=$root/build/$target
mkdir -p "$out"
case $target in
android-arm64) CC=${ANDROID_NDK_HOME:-$tools/android-ndk-r27c}/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android24-clang ;;
host) CC=${CC:-cc} ;;
esac
flags=(-O2 -Wall -Wextra -DBENCH_ONEFFT -I"$root/include" ${EXTRA_CFLAGS:-})
srcs=("$root/bench/bench.c" "$root/bench/impl_onefft.c" "$root/src/onefft.c")
libs=()
add() { # define, adapter, library
    if [ -f "$tp/lib/lib$3.a" ]; then
        flags+=("-D$1"); srcs+=("$root/bench/$2"); libs+=("-l$3")
    fi
}
add BENCH_FFTW impl_fftw.c fftw3f
add BENCH_FFMPEG impl_ffmpeg.c avutil
add BENCH_PFFFT impl_pffft.c pffft
add BENCH_KISSFFT impl_kissfft.c kissfft
# KFR's C API is a shared library only; the benchmark finds it through its rpath.
if [ -f "$tp/lib/libkfr_capi.so" ] || [ -f "$tp/lib/libkfr_capi.dylib" ]; then
    flags+=(-DBENCH_KFR); srcs+=("$root/bench/impl_kfr.c"); libs+=(-lkfr_capi "-Wl,-rpath,$tp/lib")
    [ "$target" = android-arm64 ] && cp "$tp/lib/libkfr_capi.so" "$out/"
fi
twofft=${TWOFFT_DIR:-$root/../TwoFFT}
if [ -f "$twofft/src/tw_arm64.h" ] && "$CC" -dumpmachine | grep -qE '^(aarch64|arm64)'; then
    flags+=(-DBENCH_TWOFFT -I"$twofft/src"); srcs+=("$root/bench/impl_twofft.c")
fi
if [ "$(uname -s)" = Darwin ]; then
    flags+=(-DBENCH_VDSP); srcs+=("$root/bench/impl_vdsp.c")
    libs+=(-framework Accelerate -framework CoreFoundation -framework CoreVideo -framework CoreMedia)
fi
"$CC" "${flags[@]}" -I"$tp/include" "${srcs[@]}" -L"$tp/lib" ${libs[@]+"${libs[@]}"} -lm -o "$out/bench"
echo "$out/bench"
