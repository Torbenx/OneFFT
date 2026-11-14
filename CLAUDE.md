# OneFFT

A simple and fast SIMD FFT library: complex to complex, single precision, power-of-two sizes,
out of place, interleaved data. Kernels for AArch64 (Advanced SIMD) and x86-64 (AVX-512, AVX2
with FMA, SSE2), selected at run time, plus a portable C fallback for everything else. The
kernels are inline assembly in GCC/Clang syntax; MSVC's `cl.exe` is not supported, by design.
Users get one C file and one header with no dependencies and no compiler flags.

## Where things are

* `README.md`: the user's view (API, integration, performance summary, limits) and the
  repository layout under "Development".
* `docs/design.md`: how a transform is computed and why, the generator, the runtime. Read it
  before changing kernels, plans or the driver.
* `docs/measurements.md`: the evidence behind every decision, what was tried and dropped
  ("Decisions and their effect") and what is still open ("Open questions"). Read both before
  trying to make something faster: many ideas have been tried already.
* `include/onefft.h`: the public C and C++ API; every function documents what it costs.

## Rules that are easy to break

* **Generated files are never edited by hand.** `src/onefft_*.inc` come from `gen/onefft_gen`,
  and `dist/onefft.c` and `dist/onefft.h` are an amalgamation of `src/` and `include/`.
  Run `python3 gen/generate.py` after changing anything in `gen/`, `src/` or `include/`, and
  commit its output with the change. CI fails if the generated files are out of date.
* **After changing a kernel**, in this order:
  ```
  python3 gen/test_kernels.py      # every kernel in simulation against a reference FFT
  python3 gen/generate.py          # rewrites src/onefft_*.inc and dist/
  cmake --workflow --preset host   # warnings as errors, every code path of this machine
  ```
* **Every code path needs a test variant in `CMakeLists.txt`.** One build only exercises the
  instruction set and the plan choices of the machine it runs on; the others are tested by
  building again with `ONEFFT_NO_AVX512`, `ONEFFT_NO_AVX2`, `ONEFFT_NO_SIMD`,
  `ONEFFT_X86_TUNING=...`, `ONEFFT_NEON_FMA=0/1` and `ONEFFT_NEON_ORDERED=0/1`. The variants are
  listed once, with `onefft_add_test`, and every way of testing runs that list (CI, the host,
  Android devices, the remote machines). A new switch or kernel form that changes which code
  runs gets a variant there.
* **The version is in three places:** `CMakeLists.txt` (`project(... VERSION ...)`),
  `MODULE.bazel` and the `ONEFFT_VERSION_*` macros in `include/onefft.h` (then regenerate
  `dist/`).
* **CMake and Bazel both build the library** (`CMakeLists.txt`, `BUILD.bazel`). A new source
  or test file has to be added to both.
* Never commit host names, addresses, device serials or credentials. Test machines are listed
  in `scripts/machines.local`, which is not tracked (`scripts/machines.example` shows the
  format).

## Testing

* `cmake --workflow --preset host` (`CMakePresets.json`, CMake 3.25 or later) builds with
  warnings as errors and runs `ctest`: every variant, `dist/` and the C++ interface, on what the
  host processor can run; the code paths it cannot run are not tested by it. Bazel only builds
  the default variant (`bazel test //...`). `-DONEFFT_TEST_SIZE=<log2>` at configure time
  changes the largest size tested (18).
* AVX-512 without AVX-512 hardware: run the tests under Intel SDE with
  `cmake --preset host -B build/cmake-sde "-DCMAKE_TEST_LAUNCHER=<path>/sde64;-spr;--"
  -DONEFFT_TEST_SIZE=12`, then build and `ctest` in that directory (CMake 3.29 or later;
  `.tools/` is the conventional place for SDE). Slow, so keep the size small.
* AArch64: `ONEFFT_DEVICE=<device> cmake --workflow --preset android` builds with the Android
  NDK in `ANDROID_NDK_HOME` (for example `.tools/android-ndk-r27c`) and runs the tests on a
  device over adb, with `scripts/android_run.sh` as the cross-compiling emulator;
  `ONEFFT_DEVICE=<device> ctest --preset android` runs them again on another device.
  `scripts/remote.sh <machine> test` runs the host preset on another machine over ssh, for
  example a Mac (CMake from the PATH or `~/onefft-tools-src/bin`).
* CI (`.github/workflows/ci.yml`) runs the kernel simulation, checks the generated files and
  runs `ctest` on Linux x86-64 and AArch64 (GCC and Clang) and macOS (Apple silicon and x86-64),
  plus Bazel. Its runners cannot be relied on to have AVX-512.
* Say in the commit message what was tested where, and what was only compiled.

## Performance work

* Measure before and after, on the cores the change affects: `bench/bench.c` against the other
  libraries (`scripts/third_party.sh`, then `scripts/build_bench.sh`), `bench/passes.c` for the
  passes one by one, `bench/uops_*.c` for single instructions. `scripts/experiment.sh` runs
  `bench/passes.c` with generator options (`ONEFFT_GEN`, `ONEFFT_GEN_X86`).
* A change that is faster on one core and slower on another needs a run-time choice (see
  `onefft_x86_tuning` and `onefft_neon_ordered` in `src/onefft.c`), not a trade.
* Record the outcome: new benchmark results go to `docs/results/<machine>.txt`, then
  `scripts/update_docs.py` rewrites the tables in `docs/measurements.md` and the charts in
  `docs/plots/`. What was tried and what it gave goes to "Decisions and their effect" (under
  "Tried and dropped" if it did not pay off), and the item leaves "Open questions". Design
  changes go to `docs/design.md`, and the README's performance summary is kept in step.

## Style

* C99 with GCC/Clang extensions for the library, C++11 for the C++ wrappers in the header,
  Python 3 without dependencies for the generator and scripts.
* Commit messages: a short imperative subject; a body that explains why, with measured numbers
  where speed is concerned; what was tested.
* Documentation is plain and concrete: numbers with the machine they were measured on, no
  claims without a measurement behind them.
