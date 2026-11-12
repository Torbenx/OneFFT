/*
 * Developer tool: measures how many AVX2 instructions of a kind an x86-64 core runs per cycle,
 * alone and in the mixes that occur in the kernels, to guide kernel design. Each test is a loop
 * of independent instructions; the result is cycles per loop iteration.
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static double now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1e9 + (double)ts.tv_nsec;
}

static float buffer[8192] __attribute__((aligned(64)));

#define REGS "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", \
             "xmm12", "xmm13", "xmm14", "xmm15", "cc", "memory"

/* The body is assembled 8 times, with register r = 0..7 and offset o = 0, 64, .. 448. p is read,
 * q = p + 16 KiB is written. ymm14 and ymm15 are zero. */
#define R8(M) M("0", "0") M("1", "64") M("2", "128") M("3", "192") M("4", "256") M("5", "320") M("6", "384") M("7", "448")
#define TEST(name, body)                                                                                    \
    static void test_##name(size_t n, float *p) {                                                           \
        float *q = p + 4096;                                                                                \
        __asm__ volatile("vxorps %%ymm14, %%ymm14, %%ymm14\n vxorps %%ymm15, %%ymm15, %%ymm15\n 1:\n"       \
                         R8(body) "sub $1, %[n]\n jnz 1b\n vzeroupper\n"                                    \
                         : [n] "+r"(n), [p] "+r"(p), [q] "+r"(q) : : REGS);                                 \
    }

#define OP(op, r) op " %%ymm14, %%ymm15, %%ymm" r "\n"
#define B_empty(r, o) ""
#define B_load(r, o) "vmovups " o "(%[p]), %%ymm" r "\n"
#define B_load2(r, o) B_load(r, o) "vmovups 32+" o "(%[p]), %%ymm8\n"
#define B_store(r, o) "vmovups %%ymm" r ", " o "(%[q])\n"
#define B_store2(r, o) B_store(r, o) "vmovups %%ymm" r ", 32+" o "(%[q])\n"
#define B_add2(r, o) OP("vaddps", r) OP("vaddps", "8")
#define B_mul2(r, o) OP("vmulps", r) OP("vmulps", "8")
#define B_add_mul(r, o) OP("vaddps", r) OP("vmulps", "8")
#define B_broadcast(r, o) "vbroadcastss " o "(%[p]), %%ymm" r "\n"
#define B_mul_mem(r, o) "vmulps " o "(%[p]), %%ymm15, %%ymm" r "\n"
#define B_unpck2(r, o) OP("vunpcklps", r) OP("vunpckhps", "8")
#define B_shufps2(r, o) "vshufps $0x88, %%ymm14, %%ymm15, %%ymm" r "\n vshufps $0xdd, %%ymm14, %%ymm15, %%ymm8\n"
#define B_shufps_mem(r, o) "vshufps $0x88, " o "(%[p]), %%ymm15, %%ymm" r "\n"
#define B_perm2f128(r, o) "vperm2f128 $0x20, %%ymm14, %%ymm15, %%ymm" r "\n vperm2f128 $0x31, %%ymm14, %%ymm15, %%ymm8\n"
#define B_insert_mem(r, o) "vmovups " o "(%[p]), %%xmm" r "\n vinsertf128 $1, 16+" o "(%[p]), %%ymm" r ", %%ymm" r "\n"
#define B_add2_store(r, o) B_add2(r, o) B_store(r, o)
#define B_mul2_store(r, o) B_mul2(r, o) B_store(r, o)
#define B_add2_mul2(r, o) B_add2(r, o) OP("vmulps", "9") OP("vmulps", "10")
#define B_add2_mul2_store(r, o) B_add2_mul2(r, o) B_store(r, o)
#define B_unpck2_add2(r, o) B_unpck2(r, o) OP("vaddps", "9") OP("vaddps", "10")
#define B_unpck2_mul2(r, o) B_unpck2(r, o) OP("vmulps", "9") OP("vmulps", "10")
#define B_unpck2_store(r, o) B_unpck2(r, o) B_store(r, o)
#define B_load_add_store(r, o) B_load(r, o) "vaddps %%ymm" r ", %%ymm15, %%ymm" r "\n" B_store(r, o)
#define T(name) TEST(name, B_##name)
T(empty) T(load) T(load2) T(store) T(store2) T(add2) T(mul2) T(add_mul) T(broadcast) T(mul_mem) T(unpck2) T(shufps2)
T(shufps_mem) T(perm2f128) T(insert_mem) T(add2_store) T(mul2_store) T(add2_mul2) T(add2_mul2_store) T(unpck2_add2)
T(unpck2_mul2) T(unpck2_store) T(load_add_store)

static double cycle;

static double measure(void (*fn)(size_t, float *)) {
    double best = 1e300;
    for (int s = 0; s < 20; s++) {
        double t0 = now_ns();
        fn(200000, buffer);
        double dt = (now_ns() - t0) / 200000.0;
        if (dt < best) best = dt;
    }
    return best / cycle;
}

int main(void) {
    for (int warm = 0; warm < 12; warm++) { /* spin until the frequency has settled, then measure it */
        size_t n = 50000000;
        double t0 = now_ns();
        __asm__ volatile("1: sub $1, %[n]\n jnz 1b\n" : [n] "+r"(n) : : "cc");
        cycle = (now_ns() - t0) / 5e7;
    }
    printf("clock %.0f MHz; cycles per loop of 8 times the named instructions\n", 1000.0 / cycle);
    double base = measure(test_empty);
#define RUN(name) printf("  %-18s %5.2f\n", #name, measure(test_##name) - base + 1.0)
    RUN(load); RUN(load2); RUN(store); RUN(store2); RUN(add2); RUN(mul2); RUN(add_mul); RUN(broadcast);
    RUN(mul_mem); RUN(unpck2); RUN(shufps2); RUN(shufps_mem); RUN(perm2f128); RUN(insert_mem); RUN(add2_store);
    RUN(mul2_store); RUN(add2_mul2); RUN(add2_mul2_store); RUN(unpck2_add2); RUN(unpck2_mul2); RUN(unpck2_store);
    RUN(load_add_store);
    return 0;
}
