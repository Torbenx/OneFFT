// OneFFT: test of the C++ interface. Exits with a non-zero status if anything fails.
#include "onefft.h"

#include <cmath>
#include <complex>
#include <cstdio>
#include <utility>
#include <vector>

int main() {
    const std::size_t n = 256;
    std::vector<std::complex<float>> in(n), spectrum(n), back(n), again(n);
    for (std::size_t i = 0; i < n; i++) in[i] = {std::sin(0.3f * float(i)), std::cos(1.7f * float(i))};

    onefft::Plan plan(n);
    if (!plan || plan.size() != n) return std::puts("FAILED: plan creation"), 1;
    plan.forward(in.data(), spectrum.data());
    plan.inverse(spectrum.data(), back.data());
    float worst = 0;
    for (std::size_t i = 0; i < n; i++) worst = std::max(worst, std::abs(back[i] / float(n) - in[i]));
    if (worst > 1e-5f) return std::printf("FAILED: round trip error %g\n", double(worst)), 1;

    onefft::Plan moved(std::move(plan));
    if (plan || !moved) return std::puts("FAILED: move"), 1;
    moved.forward(in.data(), again.data());
    if (again != spectrum) return std::puts("FAILED: moved plan differs"), 1;
    moved.inverse(reinterpret_cast<const float *>(spectrum.data()), reinterpret_cast<float *>(again.data()));
    if (again != back) return std::puts("FAILED: moved plan differs"), 1;

    if (!onefft::forward(in.data(), again.data(), n) || again != spectrum) return std::puts("FAILED: forward"), 1;
    if (!onefft::inverse(spectrum.data(), again.data(), n) || again != back) return std::puts("FAILED: inverse"), 1;
    if (onefft::forward(in.data(), again.data(), 100)) return std::puts("FAILED: invalid size accepted"), 1;
    if (onefft::Plan(100)) return std::puts("FAILED: invalid plan"), 1;
    onefft_cache_clear();
    std::puts("C++ interface tests passed");
    return 0;
}
