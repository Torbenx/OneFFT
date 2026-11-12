/*
 * Developer tool: measures the throughput of individual AArch64 instructions in cycles, to guide
 * kernel design. Each test repeats one instruction 16 times with independent destinations.
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

static float buffer[4096] __attribute__((aligned(64)));

#define REGS "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", "v10", "v11", "v12", "v13", "v14", \
             "v15", "v16", "v17", "v18", "v19", "v20", "v21", "v22", "v23", "v24", "v25", "v26", "v27", "v28", \
             "v29", "v30", "v31", "cc", "memory"

/* B(a, b, o) is assembled 8 times with the register numbers (a, b) = (0, 1), (2, 3), .. (14, 15)
 * and the byte offset o = 0, 64, .. 448. */
#define TEST(name, B)                                                                                       \
    static void test_##name(size_t n, float *p) {                                                           \
        float *q = p + 1024;                                                                                \
        /* Start from ordinary values: garbage could be subnormal and take slow paths. */      \
        __asm__ volatile("fmov v16.4s, #1.0\n fmov v17.4s, #1.0\n fmov v0.4s, #1.0\n"                      \
                         "fmov v1.4s, #1.0\n"                                                               \
                         "1:\n" B("0", "1", "0") B("2", "3", "64") B("4", "5", "128") B("6", "7", "192")          \
                         B("8", "9", "256") B("10", "11", "320") B("12", "13", "384") B("14", "15", "448") "sub %[p], %[q], #4096\n subs %[n], %[n], #1\n b.ne 1b\n" \
                         : [n] "+r"(n), [p] "+r"(p), [q] "+r"(q) : : REGS);                                  \
    }

#define I3(op, a, b) op " v" a ".4s, v16.4s, v17.4s\n " op " v" b ".4s, v16.4s, v17.4s\n"
#define B_empty(a, b, o) ""
#define B_fadd(a, b, o) I3("fadd", a, b)
#define B_fmul(a, b, o) I3("fmul", a, b)
#define B_fmla(a, b, o) I3("fmla", a, b)
#define B_fmul_elem(a, b, o) "fmul v" a ".4s, v16.4s, v17.s[1]\n fmul v" b ".4s, v16.4s, v17.s[2]\n"
#define B_fmla_elem(a, b, o) "fmla v" a ".4s, v16.4s, v17.s[1]\n fmla v" b ".4s, v16.4s, v17.s[2]\n"
#define B_fmul_fadd(a, b, o) "fmul v" a ".4s, v16.4s, v17.4s\n fadd v" b ".4s, v16.4s, v17.4s\n"
#define B_trn(a, b, o) "trn1 v" a ".4s, v16.4s, v17.4s\n trn2 v" b ".4s, v16.4s, v17.4s\n"
#define B_zip2d(a, b, o) "zip1 v" a ".2d, v16.2d, v17.2d\n zip2 v" b ".2d, v16.2d, v17.2d\n"
#define B_mov(a, b, o) "mov v" a ".16b, v16.16b\n mov v" b ".16b, v17.16b\n"
#define B_ld2(a, b, o) "ld2 {v" a ".4s, v" b ".4s}, [%[p]]\n ld2 {v" a ".4s, v" b ".4s}, [%[q]]\n"
#define B_st2(a, b, o) "st2 {v" a ".4s, v" b ".4s}, [%[p]]\n st2 {v" a ".4s, v" b ".4s}, [%[q]]\n"
#define B_ldp(a, b, o) "ldp q" a ", q" b ", [%[p], #" o "]\n ldp q" a ", q" b ", [%[p], #" o "+32]\n"
#define B_stp(a, b, o) "stp q" a ", q" b ", [%[q], #" o "]\n stp q" a ", q" b ", [%[q], #" o "+32]\n"
#define B_ldr(a, b, o) "ldr q" a ", [%[p], #" o "]\n ldr q" b ", [%[p], #" o "+16]\n"
#define B_str(a, b, o) "str q" a ", [%[q], #" o "]\n str q" b ", [%[q], #" o "+16]\n"
#define B_st2_inc(a, b, o) "st2 {v" a ".4s, v" b ".4s}, [%[p]], #32\n st2 {v" a ".4s, v" b ".4s}, [%[p]], #32\n"
#define B_ld2_inc(a, b, o) "ld2 {v" a ".4s, v" b ".4s}, [%[p]], #32\n ld2 {v" a ".4s, v" b ".4s}, [%[p]], #32\n"
#define B_fadd_2s(a, b, o) "fadd v" a ".2s, v16.2s, v17.2s\n fadd v" b ".2s, v16.2s, v17.2s\n"
#define B_fadd_s(a, b, o) "fadd s" a ", s16, s17\n fadd s" b ", s16, s17\n"
#define B_add_4s(a, b, o) "add v" a ".4s, v16.4s, v17.4s\n add v" b ".4s, v16.4s, v17.4s\n"
#define B_fadd_add(a, b, o) "fadd v" a ".4s, v16.4s, v17.4s\n add v" b ".4s, v16.4s, v17.4s\n"
#define B_fadd_mov(a, b, o) "fadd v" a ".4s, v16.4s, v17.4s\n mov v" b ".16b, v" a ".16b\n"
#define B_fadd_str(a, b, o) "fadd v" a ".4s, v16.4s, v17.4s\n str q16, [%[q], #" o "]\n"
/* store patterns: 8 or 16 stores per loop */
#define B_str_line(a, b, o) "str q" a ", [%[q], #" o "]\n"
#define B_str_line2(a, b, o) "str q" a ", [%[q], #" o "]\n str q" b ", [%[q], #" o "+16]\n"
#define B_str_line4(a, b, o) "str q" a ", [%[q], #" o "]\n str q" b ", [%[q], #" o "+16]\n str q" a ", [%[q], #" o "+32]\n str q" b ", [%[q], #" o "+48]\n"
#define B_stp_line(a, b, o) "stp q" a ", q" b ", [%[q], #" o "]\n"
#define B_stp_line2(a, b, o) "stp q" a ", q" b ", [%[q], #" o "]\n stp q" a ", q" b ", [%[q], #" o "+32]\n"
#define B_str16(a, b, o) "str q16, [%[q], #" o "]\n"
#define B_fadd8(a, b, o) "fadd v" a ".4s, v16.4s, v17.4s\n"
#define B_fadd_stp(a, b, o) "fadd v" a ".4s, v16.4s, v17.4s\n fadd v" b ".4s, v16.4s, v17.4s\n stp q" a ", q" b ", [%[q], #" o "]\n"
#define B_fadd_st2(a, b, o) "fadd v" a ".4s, v16.4s, v17.4s\n fadd v" b ".4s, v16.4s, v17.4s\n st2 {v" a ".4s, v" b ".4s}, [%[p]], #32\n"
#define B_fadd_ldp(a, b, o) "ldp q" a ", q" b ", [%[p], #" o "]\n fadd v" a ".4s, v" a ".4s, v17.4s\n fadd v" b ".4s, v" b ".4s, v17.4s\n"
#define B_fadd_ld2(a, b, o) "ld2 {v" a ".4s, v" b ".4s}, [%[p]], #32\n fadd v" a ".4s, v" a ".4s, v17.4s\n fadd v" b ".4s, v" b ".4s, v17.4s\n"
#define B_fadd_trn(a, b, o) "fadd v" a ".4s, v16.4s, v17.4s\n trn1 v" b ".4s, v16.4s, v17.4s\n"
#define B_ld1x4(a, b, o) "ld1 {v16.4s, v17.4s, v18.4s, v19.4s}, [%[p]]\n ld1 {v20.4s, v21.4s, v22.4s, v23.4s}, [%[q]]\n"
#define B_st1x4(a, b, o) "st1 {v16.4s, v17.4s, v18.4s, v19.4s}, [%[p]]\n st1 {v20.4s, v21.4s, v22.4s, v23.4s}, [%[q]]\n"
#define B_ld2_lane(a, b, o) "ld2 {v" a ".s, v" b ".s}[1], [%[p]]\n ld2 {v" a ".s, v" b ".s}[2], [%[q]]\n"
#define B_st2_lane(a, b, o) "st2 {v" a ".s, v" b ".s}[1], [%[p]]\n st2 {v" a ".s, v" b ".s}[2], [%[q]]\n"
#define B_ld4_2d(a, b, o) "ld4 {v16.2d, v17.2d, v18.2d, v19.2d}, [%[p]]\n ld4 {v20.2d, v21.2d, v22.2d, v23.2d}, [%[q]]\n"
#define B_st4_2d(a, b, o) "st4 {v16.2d, v17.2d, v18.2d, v19.2d}, [%[p]]\n st4 {v20.2d, v21.2d, v22.2d, v23.2d}, [%[q]]\n"
#define B_ld1r(a, b, o) "ld1r {v" a ".4s}, [%[p]]\n ld1r {v" b ".4s}, [%[q]]\n"
#define B_fcmla(a, b, o) ".arch armv8.3-a\n fcmla v" a ".4s, v16.4s, v17.4s, #0\n fcmla v" b ".4s, v16.4s, v17.4s, #90\n"
/* latency chains: one register, 16 dependent operations */
#define B_lat_fadd(a, b, o) "fadd v0.4s, v0.4s, v17.4s\n fadd v0.4s, v0.4s, v17.4s\n"
#define B_lat_fmul(a, b, o) "fmul v0.4s, v0.4s, v17.4s\n fmul v0.4s, v0.4s, v17.4s\n"
#define B_lat_fmla(a, b, o) "fmla v0.4s, v16.4s, v17.4s\n fmla v0.4s, v16.4s, v17.4s\n"
#define B_lat_mul_fmla(a, b, o) "fmul v1.4s, v0.4s, v17.4s\n fmla v0.4s, v1.4s, v17.4s\n"
#define B_lat_trn(a, b, o) "trn1 v0.4s, v0.4s, v17.4s\n trn2 v0.4s, v0.4s, v17.4s\n"
#define T(name) TEST(name, B_##name)
T(empty) T(fadd) T(fmul) T(fmla) T(fmul_elem) T(fmla_elem) T(fmul_fadd) T(trn) T(zip2d) T(mov) T(ld2) T(st2)
T(ldp) T(stp) T(ldr) T(str) T(ld1x4) T(st1x4) T(ld2_lane) T(st2_lane) T(ld4_2d) T(st4_2d) T(ld1r) T(fcmla)
T(fadd_mov) T(fadd_str) T(str_line) T(str_line2) T(str_line4) T(stp_line) T(stp_line2) T(str16) T(fadd8)
T(fadd_stp) T(fadd_st2) T(fadd_ldp) T(fadd_ld2) T(st2_inc) T(ld2_inc) T(fadd_2s) T(fadd_s) T(add_4s) T(fadd_add) T(fadd_trn)
T(lat_fadd) T(lat_fmul) T(lat_fmla) T(lat_mul_fmla) T(lat_trn)

static double cycle;

static double measure(void (*fn)(size_t, float *)) {
    double best = 1e300;
    size_t n = 200000;
    for (int s = 0; s < 15; s++) {
        memset(buffer, 0, sizeof(buffer));
        double t0 = now_ns();
        fn(n, buffer);
        double dt = (now_ns() - t0) / (double)n;
        if (dt < best) best = dt;
    }
    return best / cycle;
}

int main(int argc, char **argv) {
    int fcma = argc > 1 && !strcmp(argv[1], "fcma");
    /* Spin first so that the frequency governor has settled before the clock is measured. */
    for (int warm = 0; warm < 12; warm++) {
        size_t n = 50000000;
        double t0 = now_ns();
        __asm__ volatile("1: subs %[n], %[n], #1\n b.ne 1b\n" : [n] "+r"(n) : : "cc");
        cycle = (now_ns() - t0) / 50000000.0;
    }
    printf("clock %.0f MHz; cycles per instruction (16 per loop)\n", 1000.0 / cycle);
    double base = measure(test_empty);
#define RAW(name) printf("  %-12s %6.2f cycles per loop\n", #name, measure(test_##name) - base)
    printf("patterns (8 lines per loop):\n");
    RAW(fadd8); RAW(str_line); RAW(str16); RAW(str_line2); RAW(str_line4); RAW(stp_line); RAW(stp_line2);
    RAW(fadd_str); RAW(fadd_stp); RAW(fadd_st2); RAW(fadd_ldp); RAW(fadd_ld2);
    printf("  cycles per loop: empty %.2f; 16 fadd %.2f; 8 fadd + 8 dependent mov %.2f; 8 fadd + 8 str %.2f\n", base,
           measure(test_fadd), measure(test_fadd_mov), measure(test_fadd_str));
#define RUN(name) printf("  %-14s %5.2f\n", #name, (measure(test_##name) - base) / 16.0)
    RUN(fadd); RUN(fmul); RUN(fmul_elem); RUN(fmla); RUN(fmla_elem); RUN(fmul_fadd); RUN(trn); RUN(zip2d);
    RUN(mov); RUN(fadd_2s); RUN(fadd_s); RUN(add_4s); RUN(fadd_add); RUN(fadd_trn); RUN(ld2); RUN(st2); RUN(ldp); RUN(stp); RUN(ldr); RUN(str); RUN(ld1x4);
    RUN(st1x4); RUN(ld2_inc); RUN(st2_inc); RUN(ld2_lane); RUN(st2_lane); RUN(ld4_2d); RUN(st4_2d); RUN(ld1r);
    if (fcma) RUN(fcmla);
    printf("latency:\n");
    RUN(lat_fadd); RUN(lat_fmul); RUN(lat_fmla); RUN(lat_mul_fmla); RUN(lat_trn);
    return 0;
}
