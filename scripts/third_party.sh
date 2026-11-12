#!/usr/bin/env bash
# Fetches and builds the third-party FFT libraries the benchmark compares against.
# They are only needed for bench/, never for the library itself.
#
#   scripts/third_party.sh <target>      target: android-arm64 | host
#
# Sources go to .tools/src, build products to .tools/<target>/{include,lib}. Set ONEFFT_TOOLS to
# use another directory than .tools.
set -euo pipefail

target=${1:?usage: third_party.sh android-arm64|host}
root=$(cd "$(dirname "$0")/.." && pwd)
tools=${ONEFFT_TOOLS:-$root/.tools}
src=$tools/src
prefix=$tools/$target
mkdir -p "$src" "$prefix/include" "$prefix/lib"
jobs=$(getconf _NPROCESSORS_ONLN)

case $target in
android-arm64)
    ndk=${ANDROID_NDK_HOME:-$tools/android-ndk-r27c}/toolchains/llvm/prebuilt/linux-x86_64/bin
    CC=$ndk/aarch64-linux-android24-clang
    AR=$ndk/llvm-ar; RANLIB=$ndk/llvm-ranlib; NM=$ndk/llvm-nm; STRIP=$ndk/llvm-strip
    fftw_host=(--host=aarch64-linux-android --enable-neon --enable-armv8-cntvct-el0)
    ffmpeg_cross=(--enable-cross-compile --target-os=android --arch=aarch64 --nm="$NM" --ar="$AR"
                  --ranlib="$RANLIB" --strip="$STRIP")
    ;;
host)
    CC=${CC:-cc}; AR=${AR:-ar}; RANLIB=${RANLIB:-ranlib}
    case $(uname -m) in
    x86_64) fftw_host=(--enable-sse2 --enable-avx --enable-avx2 --enable-avx512) ;;
    *) fftw_host=(--enable-neon --enable-armv8-cntvct-el0) ;;
    esac
    ffmpeg_cross=()
    ;;
*) echo "unknown target $target" >&2; exit 2 ;;
esac

fetch_git() { # name url ref
    [ -d "$src/$1" ] || git clone --quiet --depth 1 --branch "$3" "$2" "$src/$1"
}

# --- PFFFT and KissFFT: a couple of C files, compiled straight into small static libraries ------
fetch_git pffft https://github.com/marton78/pffft.git master
fetch_git kissfft https://github.com/mborgerding/kissfft.git master
build=$prefix/build-small; mkdir -p "$build"
"$CC" -O3 -I"$src/pffft/include/pffft" -c "$src/pffft/src/pffft.c" -o "$build/pffft.o"
"$CC" -O3 -I"$src/pffft/include/pffft" -c "$src/pffft/src/pffft_common.c" -o "$build/pffft_common.o"
"$CC" -O3 -c "$src/kissfft/kiss_fft.c" -o "$build/kiss_fft.o"
"$AR" rcs "$prefix/lib/libpffft.a" "$build/pffft.o" "$build/pffft_common.o"
"$AR" rcs "$prefix/lib/libkissfft.a" "$build/kiss_fft.o"
cp "$src/pffft/include/pffft/pffft.h" "$src/kissfft/kiss_fft.h" "$prefix/include/"

# --- FFTW (single precision) --------------------------------------------------------------------
if [ ! -f "$prefix/lib/libfftw3f.a" ]; then
    [ -d "$src/fftw-3.3.10" ] || curl -sL https://www.fftw.org/fftw-3.3.10.tar.gz | tar xz -C "$src"
    build=$prefix/build-fftw; rm -rf "$build"; mkdir -p "$build"
    (cd "$build" && "$src/fftw-3.3.10/configure" --quiet --prefix="$prefix" --enable-float \
        --disable-fortran --disable-shared --disable-doc "${fftw_host[@]}" CC="$CC" AR="$AR" \
        RANLIB="$RANLIB" CFLAGS="-O3" >/dev/null && make -s -j"$jobs" >/dev/null && make -s install >/dev/null)
fi

# --- FFmpeg libavutil (av_tx) -------------------------------------------------------------------
# Its x86 assembly needs nasm. If there is none, build one into the tools directory.
if [ "$target" = host ] && [ "$(uname -m)" = x86_64 ] && ! command -v nasm >/dev/null && [ ! -x "$tools/bin/nasm" ]; then
    [ -d "$src/nasm-2.16.03" ] || curl -sL https://www.nasm.us/pub/nasm/releasebuilds/2.16.03/nasm-2.16.03.tar.gz | tar xz -C "$src"
    (cd "$src/nasm-2.16.03" && ./configure --quiet --prefix="$tools" >/dev/null && make -s -j"$jobs" >/dev/null 2>&1 &&
        mkdir -p "$tools/bin" && cp nasm "$tools/bin/")
fi
export PATH="$tools/bin:$PATH"
if [ ! -f "$prefix/lib/libavutil.a" ]; then
    fetch_git ffmpeg https://github.com/FFmpeg/FFmpeg.git n7.1
    build=$prefix/build-ffmpeg; rm -rf "$build"; mkdir -p "$build"
    (cd "$build" && "$src/ffmpeg/configure" --prefix="$prefix" --cc="$CC" ${ffmpeg_cross[@]+"${ffmpeg_cross[@]}"} \
        --disable-programs --disable-doc --disable-avcodec --disable-avformat --disable-avdevice \
        --disable-avfilter --disable-swscale --disable-swresample --disable-postproc \
        --disable-autodetect --disable-shared --enable-static --enable-pic >/dev/null &&
        make -s -j"$jobs" >/dev/null && make -s install >/dev/null)
fi
echo "third-party libraries for $target are in $prefix"
