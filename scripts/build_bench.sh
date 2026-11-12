#!/usr/bin/env bash
# Builds the benchmark with every comparison library available for the target.
#
#   scripts/build_bench.sh <target>      target: android-arm64 | host
#
# Output: build/<target>/bench. Run scripts/third_party.sh <target> first to get the libraries.
# EXTRA_CFLAGS is added to the compiler flags, e.g. -DONEFFT_NO_AVX2 to measure the SSE2 kernels.
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
if [ "$(uname -s)" = Darwin ]; then
    flags+=(-DBENCH_VDSP); srcs+=("$root/bench/impl_vdsp.c")
    libs+=(-framework Accelerate -framework CoreFoundation -framework CoreVideo -framework CoreMedia)
fi
case "$(uname -m)-$target" in aarch64-host | arm64-host | *-android-arm64) arm=1 ;; *) arm=0 ;; esac
if [ $arm = 1 ]; then
    flags+=(-DBENCH_REFERENCE -I"$root/bench/reference_shim" -Wno-everything)
    srcs+=("$root/bench/impl_reference.c" "$root"/reference/aarch64/fft_{1024,512,128,64,32}.c
           "$root/reference/aarch64/fft16_256.c" "$root/reference/aarch64/fft4_16.c")
fi
"$CC" "${flags[@]}" -I"$tp/include" "${srcs[@]}" -L"$tp/lib" ${libs[@]+"${libs[@]}"} -lm -o "$out/bench"
echo "$out/bench"
