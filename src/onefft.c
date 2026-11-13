/*
 * OneFFT - runtime: plans, twiddle tables and the transform drivers.
 *
 * The SIMD kernels are generated (onefft_*.inc, see gen/). This file decides which kernels a
 * transform size uses, builds their tables and calls them. docs/design.md explains the
 * algorithm; the comments here assume it has been read.
 */
#include "onefft.h"

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <malloc.h>
#endif

#define ONEFFT_ALIGNED(bytes) __attribute__((aligned(bytes)))

#if defined(__aarch64__) && !defined(ONEFFT_NO_SIMD)
#define ONEFFT_NEON 1
#ifdef __APPLE__
#include <sys/sysctl.h>
#endif
#include "onefft_neon.inc"
#endif

#if defined(__x86_64__) && !defined(ONEFFT_NO_SIMD)
#define ONEFFT_X86 1
#include <cpuid.h>
#include "onefft_sse2.inc"
#ifndef ONEFFT_NO_AVX2
#define ONEFFT_AVX2 1
#include "onefft_avx2.inc"
#ifndef ONEFFT_NO_AVX512
#define ONEFFT_AVX512 1
#include "onefft_avx512.inc"
#endif
#endif
#endif

/* ---- plan ---------------------------------------------------------------------------------- */

/* Kernel signatures. Every instruction set provides kernels of these four kinds. */
typedef void onefft_small_fn(const float *in, float *out);
typedef void onefft_first_fn(const float *in, float *out, const uint32_t *table, size_t count, size_t in_stride,
                             size_t out_stride);
typedef void onefft_middle_fn(float *data, const float *tw, const size_t *args, size_t blocks, size_t regions);
typedef void onefft_last_fn(float *data, const float *tw, size_t stride, size_t count, size_t regions);

/* What a middle pass kernel reads through its `args` pointer, as an array of size_t in this
 * order (gen/onefft_gen/passes.py has the same list). See there for the loop structure. */
enum {
    ONEFFT_S_HIGH,      /* bytes between the vectors of a butterfly */
    ONEFFT_S_LOW,       /* x86: the second such distance */
    ONEFFT_INNER,       /* butterflies per block */
    ONEFFT_SKIP,        /* bytes from the end of the last butterfly of a block to the next block */
    ONEFFT_REGION_SKIP, /* additional bytes from the last block of a region to the next region */
    ONEFFT_TW_REGION,   /* bytes of twiddle table per region */
    ONEFFT_ARGS
};

/* The output buffer is divided into `regions` equal parts that a pass handles one by one, each
 * with its own part of the twiddle table: the parts differ in output bits that are already
 * known, and the twiddles depend on those. */
typedef struct {
    onefft_middle_fn *run;
    const float *tw;     /* table of region 0, the tables of the other regions follow */
    size_t args[ONEFFT_ARGS];
    size_t regions, region_bytes;
    size_t blocks;       /* blocks per region */
    size_t block_bytes;  /* nested passes: bytes that one block spans */
} onefft_middle;

typedef struct {
    onefft_last_fn *run;
    const float *tw;
    size_t regions;
    size_t stride, count;
} onefft_last;

enum { ONEFFT_MAX_MIDDLE = 32, ONEFFT_MAX_LOG2 = 30 };

struct onefft_plan {
    size_t n;
    unsigned log2n;
    int inverse;
    void (*execute)(const onefft_plan *plan, const float *in, float *out);
    void *memory; /* the one allocation holding all tables */

    /* transforms that fit the registers */
    onefft_small_fn *small;

    /* transforms in passes */
    onefft_first_fn *first;
    const uint32_t *first_table;
    size_t first_count, first_in_stride, first_out_stride;
    int middle_count;
    int nested_from; /* middle passes from this index on work on nested blocks, see below */
    onefft_middle middle[ONEFFT_MAX_MIDDLE];
    int last_count;
    onefft_last last[2];

    /* portable transform */
    const float *scalar_tw;
};

/* exp(-2 pi i k / 2^log2size). The angle is folded into the first octant before calling the
 * math library, which makes the tables exactly symmetric and exact at multiples of an eighth
 * turn. */
static void onefft_root_double(uint64_t k, unsigned log2size, double *re, double *im) {
    uint64_t phase = log2size ? k << (64 - log2size) : 0; /* fraction of a full turn, 0.64 fixed point */
    unsigned quadrant = (unsigned)(phase >> 62);
    uint64_t rest = phase & (((uint64_t)1 << 62) - 1);
    const double scale = 6.283185307179586476925286766559 / 18446744073709551616.0;
    double c, s;
    if (rest > ((uint64_t)1 << 61)) {
        double angle = (double)(((uint64_t)1 << 62) - rest) * scale;
        c = sin(angle), s = cos(angle);
    } else {
        double angle = (double)rest * scale;
        c = cos(angle), s = sin(angle);
    }
    /* (c - i s) * (-i)^quadrant */
    switch (quadrant) {
    case 0: *re = c, *im = -s; break;
    case 1: *re = -s, *im = -c; break;
    case 2: *re = -c, *im = s; break;
    default: *re = s, *im = c; break;
    }
}

static void onefft_root(uint64_t k, unsigned log2size, float *re, float *im) {
    double c, s;
    onefft_root_double(k, log2size, &c, &s);
    *re = (float)c, *im = (float)s;
}

static size_t onefft_bit_reverse(size_t value, unsigned bits) {
    size_t result = 0;
    for (unsigned i = 0; i < bits; i++) {
        result = (result << 1) | (value & 1);
        value >>= 1;
    }
    return result;
}

/* ---- portable transforms --------------------------------------------------------------------
 *
 * Sizes 1, 2 and 4 are written out. Everything else that has no SIMD kernels uses a plain
 * radix 2 transform.
 */

static void onefft_execute_1(const onefft_plan *plan, const float *in, float *out) {
    (void)plan;
    out[0] = in[0], out[1] = in[1];
}

static void onefft_execute_2(const onefft_plan *plan, const float *in, float *out) {
    (void)plan;
    float ar = in[0], ai = in[1], br = in[2], bi = in[3];
    out[0] = ar + br, out[1] = ai + bi;
    out[2] = ar - br, out[3] = ai - bi;
}

static void onefft_execute_4(const onefft_plan *plan, const float *in, float *out) {
    float ar = in[0] + in[4], ai = in[1] + in[5]; /* x0 + x2 */
    float br = in[0] - in[4], bi = in[1] - in[5]; /* x0 - x2 */
    float cr = in[2] + in[6], ci = in[3] + in[7]; /* x1 + x3 */
    float dr = in[2] - in[6], di = in[3] - in[7]; /* x1 - x3 */
    if (plan->inverse) dr = -dr, di = -di;
    out[0] = ar + cr, out[1] = ai + ci;
    out[2] = br + di, out[3] = bi - dr; /* b - i d */
    out[4] = ar - cr, out[5] = ai - ci;
    out[6] = br - di, out[7] = bi + dr; /* b + i d */
}

static void onefft_scalar_execute(const onefft_plan *plan, const float *in, float *out) {
    size_t n = plan->n;
    unsigned log2n = plan->log2n;
    int inv = plan->inverse; /* the inverse exchanges real and imaginary parts on the way in and out */
    for (size_t i = 0; i < n; i++) {
        size_t j = onefft_bit_reverse(i, log2n);
        out[2 * j + inv] = in[2 * i];
        out[2 * j + 1 - inv] = in[2 * i + 1];
    }
    const float *tw = plan->scalar_tw;
    for (size_t half = 1; half < n; half *= 2) {
        size_t step = n / (2 * half);
        for (size_t start = 0; start < n; start += 2 * half) {
            for (size_t k = 0; k < half; k++) {
                float wr = tw[2 * k * step], wi = tw[2 * k * step + 1];
                float *a = out + 2 * (start + k), *b = a + 2 * half;
                float tr = b[0] * wr - b[1] * wi;
                float ti = b[0] * wi + b[1] * wr;
                b[0] = a[0] - tr, b[1] = a[1] - ti;
                a[0] += tr, a[1] += ti;
            }
        }
    }
    if (inv) {
        for (size_t i = 0; i < n; i++) {
            float t = out[2 * i];
            out[2 * i] = out[2 * i + 1], out[2 * i + 1] = t;
        }
    }
}

/* All plan builders are called twice: first with memory == NULL to learn how many bytes of
 * tables the plan needs, then with the allocation to fill in. */
static size_t onefft_scalar_plan(onefft_plan *plan, float *memory) {
    if (plan->log2n <= 2) {
        if (memory) {
            plan->execute = plan->log2n == 0 ? onefft_execute_1 : plan->log2n == 1 ? onefft_execute_2 : onefft_execute_4;
        }
        return 0;
    }
    size_t count = plan->n / 2;
    if (memory) {
        for (size_t k = 0; k < count; k++) onefft_root(k, plan->log2n, memory + 2 * k, memory + 2 * k + 1);
        plan->scalar_tw = memory;
        plan->execute = onefft_scalar_execute;
    }
    return 2 * count * sizeof(float);
}

/* ---- transforms in passes, common to all instruction sets ---------------------------------- */

#if defined(ONEFFT_NEON) || defined(ONEFFT_X86)

/* Middle passes run pass by pass on ranges of up to 2^ONEFFT_CACHE_BYTES_LOG2 bytes. */
enum { ONEFFT_CACHE_BYTES_LOG2 = 15 };
#ifdef ONEFFT_DEVELOPER
static unsigned onefft_tune_cache_log2 = ONEFFT_CACHE_BYTES_LOG2;
#else
static const unsigned onefft_tune_cache_log2 = ONEFFT_CACHE_BYTES_LOG2;
#endif

/* Applies the nested middle passes up to `level` to `bytes` bytes at `data` within region
 * `region`. The blocks of these passes are nested: a block of one pass is made of whole blocks
 * of the pass before. `bytes` is a multiple of the block size of pass `level`.
 *
 * Ranges that fit the cache are processed pass by pass. A larger range is cut into pieces that
 * are finished one after the other, so that every pass finds its data in the cache that its
 * block size fits into. */
static void onefft_nested(const onefft_plan *plan, int level, size_t region, float *data, size_t bytes) {
    const onefft_middle *m = &plan->middle[level];
    size_t block = m->block_bytes, cache = (size_t)1 << onefft_tune_cache_log2;
    if (bytes <= cache || (level == plan->nested_from && bytes == block)) {
        for (int i = plan->nested_from; i <= level; i++) {
            const onefft_middle *p = &plan->middle[i];
            const float *tw = (const float *)((const char *)p->tw + region * p->args[ONEFFT_TW_REGION]);
            p->run(data, tw, p->args, bytes / p->block_bytes, 1);
        }
    } else if (block <= cache) {
        for (size_t offset = 0; offset < bytes; offset += cache) {
            onefft_nested(plan, level, region, (float *)((char *)data + offset), cache);
        }
    } else {
        const float *tw = (const float *)((const char *)m->tw + region * m->args[ONEFFT_TW_REGION]);
        for (size_t offset = 0; offset < bytes; offset += block) {
            float *part = (float *)((char *)data + offset);
            if (level > plan->nested_from) onefft_nested(plan, level - 1, region, part, block);
            m->run(part, tw, m->args, 1, 1);
        }
    }
}

static void onefft_execute_passes(const onefft_plan *plan, const float *in, float *out) {
    plan->first(in, out, plan->first_table, plan->first_count, plan->first_in_stride, plan->first_out_stride);
    /* A pass over the whole output is one kernel call. Only when the output does not fit the
     * cache are the nested passes applied piece by piece instead. */
    int whole = plan->n * 8 <= (size_t)1 << onefft_tune_cache_log2 ? plan->middle_count : plan->nested_from;
    for (int i = 0; i < whole; i++) {
        const onefft_middle *m = &plan->middle[i];
        m->run(out, m->tw, m->args, m->blocks, m->regions);
    }
    if (whole < plan->middle_count) {
        const onefft_middle *m = &plan->middle[plan->middle_count - 1];
        for (size_t r = 0; r < m->regions; r++) {
            onefft_nested(plan, plan->middle_count - 1, r, (float *)((char *)out + r * m->region_bytes), m->region_bytes);
        }
    }
    for (int i = 0; i < plan->last_count; i++) {
        const onefft_last *l = &plan->last[i];
        l->run(out, l->tw, l->stride, l->count, l->regions);
    }
}

static void onefft_execute_small(const onefft_plan *plan, const float *in, float *out) { plan->small(in, out); }

/* Table entry of a middle pass: the real parts of the powers 1..radix-1 of
 * exp(-2 pi i k / 2^log2size), padded to a multiple of 4 floats, then the imaginary parts.
 * Returns the number of floats. */
static size_t onefft_middle_entry(float *entry, uint64_t k, unsigned log2size, unsigned radix) {
    size_t pad = 4 * (radix / 4);
    if (entry) {
        memset(entry, 0, 2 * pad * sizeof(float));
        for (unsigned i = 1; i < radix; i++) onefft_root(k * i, log2size, &entry[i - 1], &entry[pad + i - 1]);
    }
    return 2 * pad;
}

/* A complex vector in a table: first the real parts of all lanes, then the imaginary parts. */
static void onefft_table_root(float *table, size_t lanes, size_t lane, uint64_t k, unsigned log2size) {
    if (table) onefft_root(k, log2size, &table[lane], &table[lanes + lane]);
}

#endif

/* ---- AArch64 --------------------------------------------------------------------------------
 *
 * A register holds 4 lanes, a butterfly works on 8 (or 4) registers.
 *
 *   first pass   transforms the top 3 index bits and moves every vector to its final position
 *   middle       transform 3 (or 2) more bits each, in place, small strides first
 *   last pass    transforms the remaining 3 (or 2) bits including the two that sit in the lanes
 *
 * The output is handled in 4 regions: its quarters hold the 4 values of output bits (1, 0).
 */
#ifdef ONEFFT_NEON

#ifdef ONEFFT_DEVELOPER
static const unsigned *onefft_developer_radices; /* overrides the pass structure, see below */
static int onefft_developer_ordered = -1;        /* 0 or 1 overrides onefft_neon_ordered() */
#endif

/* The same for the kernels built from fused multiply-add butterflies, which take a twiddle
 * w = c (1 + i t) as its real part c and the ratio t: first c of all lanes, then t. The real
 * part must not be zero; the callers keep the angle below a quarter turn. */
static void onefft_table_cos_tan(float *table, size_t lanes, size_t lane, uint64_t k, unsigned log2size) {
    if (table) {
        double c, s;
        onefft_root_double(k, log2size, &c, &s);
        table[lane] = (float)c, table[lanes + lane] = (float)(s / c);
    }
}

/* The middle and last passes exist in two forms. The standard one multiplies by twiddles and
 * then adds and subtracts. The other one builds every butterfly from six fused multiply-adds
 * (see cplx.butterfly_tan in the generator), which is fewer operations but needs three register
 * copies per butterfly. That is a clear gain on Apple cores, where copies cost nothing, and a
 * loss on the Cortex cores measured. */
#ifndef ONEFFT_NEON_FMA
#ifdef __APPLE__
#define ONEFFT_NEON_FMA 1
#else
#define ONEFFT_NEON_FMA 0
#endif
#endif

/* Whether the first pass of a transform of 2^log2n points handles its butterflies in the order
 * of the output: it then writes in order and reads scattered cache lines, instead of reading in
 * order and writing scattered lines (once the data exceeds the first level cache).
 *
 * Apple's cores are much faster that way while input and output fit their second level cache,
 * the last one they have: the M2 by 10% at 2^14 points and by 29 to 40% from 2^15 to 2^20. The
 * Cortex-X4 is 7 to 25% slower with it from 2^15 to 2^20 points, the Cortex-A73 and Kryo 360
 * 20 to 80%. ONEFFT_NEON_ORDERED=0/1 overrides the choice for all sizes. */
static int onefft_neon_ordered(unsigned log2n) {
#ifdef ONEFFT_DEVELOPER
    if (onefft_developer_ordered >= 0) return onefft_developer_ordered;
#endif
#if defined(ONEFFT_NEON_ORDERED)
    (void)log2n;
    return ONEFFT_NEON_ORDERED;
#elif defined(__APPLE__)
    /* The cache of the performance cores, where the system distinguishes them. The value is a
     * 32 or 64 bit integer, depending on the name and the system. */
    uint64_t cache = 0;
    size_t size = sizeof(cache);
    if (sysctlbyname("hw.perflevel0.l2cachesize", &cache, &size, NULL, 0) != 0 || cache == 0) {
        cache = 0, size = sizeof(cache);
        if (sysctlbyname("hw.l2cachesize", &cache, &size, NULL, 0) != 0) cache = 0;
    }
    return log2n >= 14 && log2n <= 40 && ((uint64_t)16 << log2n) <= cache;
#else
    (void)log2n;
    return 0;
#endif
}

static size_t onefft_neon_plan(onefft_plan *plan, float *memory) {
    unsigned log2n = plan->log2n;
    int inverse = plan->inverse;
    if (log2n < 3) return onefft_scalar_plan(plan, memory);
    if (log2n <= 5) {
        static onefft_small_fn *const small[3][2] = {
            {onefft_neon_n8, onefft_neon_n8i}, {onefft_neon_n16, onefft_neon_n16i}, {onefft_neon_n32, onefft_neon_n32i}};
        if (memory) {
            plan->small = small[log2n - 3][inverse];
            plan->execute = onefft_execute_small;
        }
        return 0;
    }
    size_t n = plan->n, floats = 0;

    /* Radix bits per pass: 3 for the first pass, then the middle passes, then the last pass.
     * As few passes as possible, and a radix 8 last pass where the size allows it: per index
     * bit it is the cheaper of the two last passes. */
    unsigned last = 3, middle[ONEFFT_MAX_MIDDLE], middle_count = 0;
    if (log2n == 7) {
        last = 2;
        middle[middle_count++] = 2;
    } else {
        unsigned rest = log2n - 6, twos = (3 - rest % 3) % 3;
        for (unsigned i = 0; i < twos; i++) middle[middle_count++] = 2;
        for (unsigned i = 0; i < (rest - 2 * twos) / 3; i++) middle[middle_count++] = 3;
    }
#ifdef ONEFFT_DEVELOPER
    if (onefft_developer_radices) { /* 0 terminated list: middle passes, then the last pass */
        middle_count = 0;
        const unsigned *r = onefft_developer_radices;
        for (; r[1]; r++) middle[middle_count++] = r[0];
        last = r[0];
    }
#endif

    /* First pass: the butterfly that reads input vector a writes to the bit reversed position.
     * The butterflies are handled in the order of the input or in the order of the output, see
     * onefft_neon_ordered(). Prefetching the scattered lines did not help. */
    size_t first_count = n >> 5;
    if (memory) {
        uint32_t *table = (uint32_t *)(memory + floats);
        int ordered = onefft_neon_ordered(log2n);
        for (size_t a = 0; a < first_count; a++) {
            size_t b = onefft_bit_reverse(a, log2n - 5); /* the mapping is its own inverse */
            table[2 * a] = (uint32_t)(4 * (ordered ? b : a));
            table[2 * a + 1] = (uint32_t)(8 * (ordered ? a : b));
        }
        plan->first = inverse ? onefft_neon_f8i : onefft_neon_f8;
        plan->first_table = table;
        plan->first_count = first_count;
        plan->first_in_stride = n;      /* an eighth of the input, in bytes */
        plan->first_out_stride = 2 * n; /* a quarter of the output */
    }
    floats += 2 * first_count;
    floats = (floats + 15) & ~(size_t)15;

    /* Middle passes. After `done` bits, butterfly j of a block in region t has the low output
     * bits K = t + 4 j: t are output bits (1, 0), j the rest. */
    const int fma = ONEFFT_NEON_FMA;
    unsigned done = 3;
    for (unsigned i = 0; i < middle_count; i++) {
        unsigned bits = middle[i];
        size_t inner = (size_t)1 << (done - 2), start = floats;
        for (size_t t = 0; t < 4; t++) {
            for (size_t j = 0; j < inner; j++) {
                uint64_t k = t + 4 * j;
                float *entry = memory ? memory + floats : NULL;
                if (!fma) {
                    /* the powers of exp(-2 pi i K / 2^(done + bits)) */
                    floats += onefft_middle_entry(entry, k, done + bits, 1u << bits);
                    continue;
                }
                /* One twiddle per stage m: exp(-2 pi i K / 2^(done + m)). The angle of the
                 * first reaches a quarter turn in the upper half of the K range; there it is
                 * stored a quarter turn back and the kernel multiplies by -i. Radix 8 needs
                 * the third also turned by an eighth. */
                size_t lanes = bits == 3 ? 4 : 2;
                onefft_table_cos_tan(entry, lanes, 0, k & (((uint64_t)1 << (done - 1)) - 1), done + 1);
                onefft_table_cos_tan(entry, lanes, 1, k, done + 2);
                if (bits == 3) {
                    onefft_table_cos_tan(entry, lanes, 2, k, done + 3);
                    onefft_table_cos_tan(entry, lanes, 3, k + ((uint64_t)1 << done), done + 3);
                }
                floats += 2 * lanes;
            }
        }
        if (memory) {
            onefft_middle *m = &plan->middle[i];
            m->run = bits == 3 ? (fma ? onefft_neon_c8_fma : onefft_neon_c8) : (fma ? onefft_neon_c4_fma : onefft_neon_c4);
            m->tw = memory + start;
            m->regions = 4;
            m->region_bytes = 2 * n;
            m->args[ONEFFT_S_HIGH] = 32 * inner;
            m->args[ONEFFT_INNER] = inner;
            m->args[ONEFFT_SKIP] = 32 * inner * (((size_t)1 << bits) - 1);
            m->args[ONEFFT_REGION_SKIP] = 0;
            m->args[ONEFFT_TW_REGION] = (floats - start) / 4 * sizeof(float);
            m->block_bytes = (32 * inner) << bits;
            m->blocks = m->region_bytes / m->block_bytes;
        }
        done += bits;
    }
    floats = (floats + 15) & ~(size_t)15;

    /* Last pass. Butterfly j handles the output indices K + (N/8 or N/4) * q with K = t + 4 j
     * in lane t. Radix 8 first transforms index bit 2 with one scalar twiddle per region t,
     * which gives the new output bit g. Then index bits 1 and 0 are transformed with twiddle
     * vectors.
     *
     * Standard form: for each g, the twiddles of the registers l = 1..3, l being index bits
     * (1, 0): the powers l of exp(-2 pi i (K + g N/8) / N).
     * Fused form: the twiddle of index bit 1, exp(-2 pi i 2 K / N), shared by both g, then for
     * each g that of index bit 0, exp(-2 pi i (K + g N/8) / N). Angles that would reach a
     * quarter turn are stored a quarter turn back, as in the middle passes. */
    size_t start = floats;
    for (size_t j = 0; j < n >> (last + 2); j++) {
        float *table = memory ? memory + floats : NULL;
        if (fma) {
            if (last == 3) {
                for (size_t t = 0; t < 4; t++) onefft_table_cos_tan(table, 4, t, 4 * ((t + 4 * j) % (n >> 4)), log2n);
                floats += 8;
            }
            table = memory ? memory + floats : NULL;
            for (size_t t = 0; t < 4; t++) onefft_table_cos_tan(table, 4, t, 2 * ((t + 4 * j) % (n >> 3)), log2n);
            floats += 8;
            for (size_t g = 0; g < (size_t)1 << (last - 2); g++) {
                table = memory ? memory + floats : NULL;
                for (size_t t = 0; t < 4; t++) onefft_table_cos_tan(table, 4, t, t + 4 * j + (n >> 3) * g, log2n);
                floats += 8;
            }
            continue;
        }
        if (last == 3) {
            for (size_t t = 0; t < 4; t++) onefft_table_root(table, 4, t, 4 * (t + 4 * j), log2n);
            floats += 8;
        }
        for (size_t g = 0; g < (size_t)1 << (last - 2); g++) {
            for (size_t l = 1; l < 4; l++) {
                table = memory ? memory + floats : NULL;
                for (size_t t = 0; t < 4; t++) onefft_table_root(table, 4, t, l * (t + 4 * j + (n >> 3) * g), log2n);
                floats += 8;
            }
        }
    }
    if (memory) {
        plan->middle_count = (int)middle_count;
        plan->nested_from = 0;
        onefft_last *l = &plan->last[0];
        static onefft_last_fn *const kernels[2][2][2] = { /* by fused form, radix 8, inverse */
            {{onefft_neon_l4, onefft_neon_l4i}, {onefft_neon_l8, onefft_neon_l8i}},
            {{onefft_neon_l4_fma, onefft_neon_l4_fmai}, {onefft_neon_l8_fma, onefft_neon_l8_fmai}}};
        l->run = kernels[fma][last == 3][inverse];
        l->tw = memory + start;
        l->regions = 1;
        l->stride = (8 * n) >> last;
        l->count = n >> (last + 2);
        plan->last_count = 1;
        plan->execute = onefft_execute_passes;
    }
    return floats * sizeof(float);
}

#endif /* ONEFFT_NEON */

/* ---- x86-64 ---------------------------------------------------------------------------------
 *
 * 16 registers hold 4 complex vectors with room for intermediate results: the passes are radix
 * 4. Two AVX2 kernels are radix 8 nevertheless; they compute in place in all 16 registers, with
 * constants and twiddles as memory operands. A register has 4 lanes with SSE2 and 8 with AVX2.
 *
 *   first pass      transforms the top 2 (1, 3) index bits, moves vectors to their final position
 *   middle passes   2 bits each
 *   SSE2  l4        index bits 1 and 0, which sit in the lanes
 *   AVX2  la, lb    index bits 3 and 2, then 1 and 0; bits 2..0 sit in the lanes
 *         la8, lb   index bits 4, 3 and 2, then 1 and 0
 *
 * Region r of the output holds output bits (0, 1) = (r/2, r%2) when it is divided in 4, and
 * output bits (0, 1, 2) = (r/4, r/2%2, r%2) when it is divided in 8. In that order the
 * exchanges of the last passes leave everything in its place.
 */
#ifdef ONEFFT_X86

typedef struct {
    unsigned lane_bits;
    size_t (*low_bits)(size_t region, unsigned bits); /* the low output bits of a region */
    onefft_first_fn *f4[2], *f_odd[2]; /* radix 4; radix 2 (SSE2) or 8 (AVX2, AVX-512) */
    onefft_middle_fn *c4, *c8;         /* c8: AVX-512 */
    onefft_last_fn *l4[2];             /* SSE2 */
    onefft_last_fn *la, *la2, *la8, *la8x, *lb[2]; /* AVX2 */
    onefft_small_fn *small[6][2];      /* by log2n; NULL where the passes are used */
} onefft_x86_kernels;

static const onefft_x86_kernels onefft_sse2_kernels = {
    2, onefft_bit_reverse, {onefft_sse2_f4, onefft_sse2_f4i}, {onefft_sse2_f2, onefft_sse2_f2i}, onefft_sse2_c4, NULL,
    {onefft_sse2_l4, onefft_sse2_l4i}, NULL, NULL, NULL, NULL, {NULL, NULL},
    {{0}, {0}, {0}, {onefft_sse2_n8, onefft_sse2_n8i}, {onefft_sse2_n16, onefft_sse2_n16i}, {0}}};

#ifdef ONEFFT_AVX2
static const onefft_x86_kernels onefft_avx2_kernels = {
    3, onefft_bit_reverse, {onefft_avx2_f4, onefft_avx2_f4i}, {onefft_avx2_f8, onefft_avx2_f8i}, onefft_avx2_c4, NULL,
    {NULL, NULL}, onefft_avx2_la, onefft_avx2_la2, onefft_avx2_la8, onefft_avx2_la8x,
    {onefft_avx2_lb, onefft_avx2_lbi},
    {{0}, {0}, {0}, {onefft_sse2_n8, onefft_sse2_n8i}, {onefft_avx2_n16, onefft_avx2_n16i},
     {onefft_avx2_n32, onefft_avx2_n32i}}};
#endif

/* AVX2 lane order: the element of 8 consecutive values that each lane holds. */
static const unsigned char onefft_avx2_element[8] = {0, 1, 4, 5, 2, 3, 6, 7};

/* Adds a middle pass that transforms `bits` index bits (2, or 3 with AVX-512) after `done` are
 * finished. The twiddle of butterfly j in region r is exp(-2 pi i K / 2^(done + bits)),
 * K = low + (j << region_bits) where `low` are the output bits that the region stands for.
 * Returns the new table size. */
static size_t onefft_x86_middle(onefft_plan *plan, float *memory, size_t floats, const onefft_x86_kernels *kernels,
                                unsigned done, unsigned bits, unsigned region_bits, size_t s_high, size_t s_low,
                                size_t inner, size_t skip, size_t blocks) {
    size_t start = floats, regions = (size_t)1 << region_bits;
    for (size_t r = 0; r < regions; r++) {
        for (size_t j = 0; j < inner; j++) {
            uint64_t k = kernels->low_bits(r, region_bits) + (j << region_bits);
            floats += onefft_middle_entry(memory ? memory + floats : NULL, k, done + bits, 1u << bits);
        }
    }
    if (memory) {
        onefft_middle *m = &plan->middle[plan->middle_count++];
        m->run = bits == 3 ? kernels->c8 : kernels->c4;
        size_t vector = (size_t)8 << kernels->lane_bits;
        m->tw = memory + start;
        m->regions = regions;
        m->region_bytes = (8 * plan->n) >> region_bits;
        m->args[ONEFFT_S_HIGH] = s_high;
        m->args[ONEFFT_S_LOW] = s_low;
        m->args[ONEFFT_INNER] = inner;
        m->args[ONEFFT_SKIP] = skip;
        /* the data pointer moves by inner * vector + skip per block */
        m->args[ONEFFT_REGION_SKIP] = m->region_bytes - blocks * (inner * vector + skip);
        m->args[ONEFFT_TW_REGION] = (floats - start) / regions * sizeof(float);
        m->blocks = blocks;
        m->block_bytes = m->region_bytes / blocks; /* only meaningful, and exact, for nested passes */
    }
    return floats;
}

/* Choices that depend on the transform size and on the processor. All sizes are log2 of the
 * number of points. docs/measurements.md has the numbers behind them.
 *
 * The first pass moves every vector to its final position, which is bit reversed. It handles
 * its butterflies in the order of the input, and then writes to scattered cache lines once the
 * data exceeds the first level cache, or in the order of the output, and then reads scattered
 * lines. Stores that miss the cache are handled one at a time by AMD's cores, loads in
 * parallel: writing in order is 3 to 15% faster on Zen 2 and Zen 4 and up to twice as fast on
 * Zen 5, as long as input and output fit a part of the third level cache. Beyond that reading
 * in order is faster. Processors that do not report that cache the AMD way (all others, none
 * of which was measured) read in order.
 *
 * AVX-512 has a radix 16 first pass. It reads and writes 16 streams whose cache lines compete
 * for the same places of the first level cache when the transform is large, so that the radix 8
 * pass is faster then. When the data does not fit the third level cache either, a pass over it
 * costs more than that and radix 16 is used again, because it saves a pass. */
typedef struct {
    unsigned char f16_to, f16_from;         /* AVX-512: radix 16 up to and from these sizes, radix 8 between */
    unsigned char ordered_from, ordered_to; /* sizes whose first pass writes in order */
} onefft_x86_tuning;

#ifdef ONEFFT_DEVELOPER
static const onefft_x86_tuning *onefft_developer_tuning; /* overrides the choice below */
#endif

static onefft_x86_tuning onefft_x86_tune(unsigned lane_bits) {
#ifdef ONEFFT_DEVELOPER
    if (onefft_developer_tuning) return *onefft_developer_tuning;
#endif
#ifdef ONEFFT_X86_TUNING /* the four numbers, for tests: -DONEFFT_X86_TUNING=99,0,99,0 */
    static const onefft_x86_tuning fixed = {ONEFFT_X86_TUNING};
    (void)lane_bits;
    return fixed;
#else
    onefft_x86_tuning tuning = {12, 22, 99, 0};
    unsigned eax = 0, ebx = 0, ecx = 0, edx = 0, family = 0;
    if (__get_cpuid(1, &eax, &ebx, &ecx, &edx)) family = ((eax >> 8) & 15) + ((eax >> 20) & 255);
    /* The third level cache that a core has access to, from AMD's list of caches. */
    uint64_t bytes = 0;
    for (unsigned i = 0; i < 8 && __get_cpuid_count(0x8000001d, i, &eax, &ebx, &ecx, &edx) && (eax & 31); i++) {
        if (((eax >> 5) & 7) == 3) {
            bytes = (uint64_t)((ebx >> 22) + 1) * (((ebx >> 12) & 1023) + 1) * ((ebx & 4095) + 1) * ((uint64_t)ecx + 1);
        }
    }
    if (bytes < ((uint64_t)1 << 20)) return tuning;
    unsigned cache = 20; /* log2 of its size, rounded down */
    while (bytes >> (cache + 1)) cache++;
    /* A transform of 2^k points works on 2^(k+4) bytes: it writes in order while that is at
     * most a quarter of the cache (half on Zen 5 and later, but never beyond what the TLB of
     * these cores covers). */
    int zen5 = family >= 0x1a;
    unsigned limit = zen5 ? 20 : 19, to = cache - (zen5 ? 5 : 6);
    tuning.ordered_from = lane_bits == 4 ? 14 : 13;
    tuning.ordered_to = (unsigned char)(to < limit ? to : limit);
    tuning.f16_from = (unsigned char)(cache - 4);
    return tuning;
#endif
}

static size_t onefft_x86_plan(onefft_plan *plan, float *memory, const onefft_x86_kernels *kernels) {
    unsigned log2n = plan->log2n, lane_bits = kernels->lane_bits;
    int inverse = plan->inverse, avx2 = lane_bits == 3;
    if (log2n < 3) return onefft_scalar_plan(plan, memory);
    if (log2n < 6 && kernels->small[log2n][0]) {
        if (memory) {
            plan->small = kernels->small[log2n][inverse];
            plan->execute = onefft_execute_small;
        }
        return 0;
    }
    size_t n = plan->n, floats = 0, vector = (size_t)8 << lane_bits; /* bytes of a split vector */
    if (memory) plan->middle_count = 0;

    /* Which passes. SSE2: all passes after the first transform two index bits, so the first
     * is radix 4 for an even number of bits and radix 2 for an odd one. AVX2: the first pass is
     * radix 8 (its kernel has no twiddles to hold and manages with 16 registers), and the pass
     * before the last takes 3 bits (la8) or 2 (la) to make the total fit. 64 points are too
     * few for that: radix 4, la2, lb. */
    unsigned first_bits, last_bits; /* last_bits: index bits left to the last pass(es) */
    if (!avx2) {
        first_bits = log2n % 2 == 0 ? 2 : 1, last_bits = 2;
    } else if (log2n == 6) {
        first_bits = 2, last_bits = 4;
    } else {
        first_bits = 3, last_bits = log2n % 2 == 0 ? 5 : 4;
    }

    /* First pass. Its butterflies count through the remaining index bits above the lanes. The
     * top `kept` of those bits stay where they are: their results are what the last passes
     * exchange with the lanes, unless the first pass transforms those bits itself. The others
     * are bit reversed. The butterflies are handled in the order of the input or in the order
     * of the output, see onefft_x86_tuning. */
    unsigned count_bits = log2n - first_bits - lane_bits, kept = lane_bits > first_bits ? lane_bits - first_bits : 0;
    size_t first_count = (size_t)1 << count_bits;
    if (memory) {
        uint32_t *table = (uint32_t *)(memory + floats);
        unsigned low_bits = count_bits - kept;
        onefft_x86_tuning tuning = onefft_x86_tune(lane_bits);
        int ordered = log2n >= tuning.ordered_from && log2n <= tuning.ordered_to;
        for (size_t a = 0; a < first_count; a++) {
            size_t low = onefft_bit_reverse(a & (((size_t)1 << low_bits) - 1), low_bits);
            size_t b = (a >> low_bits << low_bits) | low; /* the mapping is its own inverse */
            table[2 * a] = (uint32_t)((ordered ? b : a) << lane_bits);
            table[2 * a + 1] = (uint32_t)((ordered ? a : b) << lane_bits);
        }
        plan->first = first_bits == 2 ? kernels->f4[inverse] : kernels->f_odd[inverse];
        plan->first_table = table;
        plan->first_count = first_count;
        plan->first_in_stride = plan->first_out_stride = (8 * n) >> first_bits;
    }
    floats += 2 * first_count;
    floats = (floats + 15) & ~(size_t)15;

    /* A middle pass that is not nested: SSE2 with a radix 2 first pass has a kept index bit,
     * n-2, at the second highest position. It is transformed together with bit n-3, which is
     * next to the lanes. */
    unsigned done = first_bits;
    if (!avx2 && first_bits == 1) {
        floats = onefft_x86_middle(plan, memory, floats, kernels, done, 2, 1, 2 * n, vector, 1, vector, n >> 5);
        done += 2;
    }
    if (memory) plan->nested_from = plan->middle_count;

    /* Nested middle passes, small strides first. */
    for (; done + last_bits < log2n; done += 2) {
        size_t s_high = (size_t)8 << done;
        floats = onefft_x86_middle(plan, memory, floats, kernels, done, 2, lane_bits, s_high, 2 * s_high,
                                   (size_t)1 << (done - lane_bits), 3 * s_high, (n << (3 - lane_bits)) / (4 * s_high));
    }

    /* AVX2, pass before the last. Butterfly j of region r has the low output bits
     * K = k_low + 4 k2 + 8 j, where k_low is r reversed (output bits 0 and 1). */
    if (avx2 && last_bits == 5) {
        /* la8: index bit 2 is exchanged with output bit 2 (k2) while loading, so k2 is lane bit
         * b0 and every twiddle is a vector. Stage m = 1..3 (index bits 4, 3, 2) needs
         * exp(-2 pi i K / 2^(n - 5 + m)), the last also turned by an eighth. The first reaches
         * a quarter turn in the upper half of the butterflies; there it is stored a quarter
         * turn back and the kernel multiplies by -i (la8x, 256 points, has one butterfly per
         * region and stores the full angle). The kernel applies a twiddle as three shears and
         * takes tan(angle/2) and sin(angle). */
        size_t start = floats, count = n >> 8;
        for (size_t r = 0; r < 4; r++) {
            for (size_t j = 0; j < count; j++) {
                for (unsigned t = 0; t < 4; t++) {
                    float *table = memory ? memory + floats : NULL;
                    for (size_t lane = 0; table && lane < 8; lane++) {
                        uint64_t k = onefft_bit_reverse(r, 2) + 4 * (lane & 1) + 8 * j;
                        unsigned log2size = log2n - 4 + (t < 3 ? t : 2);
                        if (t == 0 && count > 1) k &= (n >> 6) - 1;
                        if (t == 3) k += n >> 5;
                        double c, s;
                        onefft_root_double(k, log2size, &c, &s);
                        table[lane] = (float)(-s / (1.0 + c)), table[8 + lane] = (float)-s;
                    }
                    floats += 16;
                }
            }
        }
        if (memory) {
            onefft_last *l = &plan->last[0];
            l->run = count > 1 ? kernels->la8 : kernels->la8x;
            l->tw = memory + start;
            l->regions = 4;
            l->stride = n / 4;
            l->count = count;
        }
    } else if (avx2) {
        /* la: index bit 3 uses a scalar twiddle for each of the two values of k2, which selects
         * a pair of vectors. Then k2 moves into lane bit b0, and index bit 2 uses a twiddle
         * vector. For 64 points (la2) there is one butterfly per region and no k2 yet. */
        size_t start = floats, count = log2n > 6 ? n >> 7 : 1;
        for (size_t r = 0; r < 4; r++) {
            uint64_t k_low = onefft_bit_reverse(r, 2);
            for (size_t j = 0; j < count; j++) {
                float *table = memory ? memory + floats : NULL;
                if (table) memset(table, 0, 4 * sizeof(float));
                for (size_t k2 = 0; k2 < (log2n > 6 ? 2 : 1); k2++) {
                    onefft_table_root(table, 2, k2, 8 * (k_low + 4 * k2 + 8 * j), log2n);
                }
                for (size_t lane = 0; lane < 8; lane++) {
                    onefft_table_root(table ? table + 4 : NULL, 8, lane, 4 * (k_low + 4 * (lane & 1) + 8 * j), log2n);
                }
                floats += 20;
            }
        }
        if (memory) {
            onefft_last *l = &plan->last[0];
            l->run = log2n > 6 ? kernels->la : kernels->la2;
            l->tw = memory + start;
            l->regions = 4;
            l->stride = log2n > 6 ? n / 2 : vector;
            l->count = count;
        }
    }
    floats = (floats + 15) & ~(size_t)15;

    /* l4 / lb: butterfly j handles the output indices K + (N/4) q, K = element + lanes * j. */
    size_t start = floats, lanes = (size_t)1 << lane_bits;
    for (size_t j = 0; j < n >> (lane_bits + 2); j++) {
        for (size_t l = 1; l < 4; l++) {
            float *table = memory ? memory + floats : NULL;
            for (size_t lane = 0; lane < lanes; lane++) {
                size_t element = avx2 ? onefft_avx2_element[lane] : lane;
                onefft_table_root(table, lanes, lane, l * (element + lanes * j), log2n);
            }
            floats += 2 * lanes;
        }
    }
    if (memory) {
        onefft_last *l = &plan->last[avx2 ? 1 : 0];
        l->run = avx2 ? kernels->lb[inverse] : kernels->l4[inverse];
        l->tw = memory + start;
        l->regions = 1;
        l->stride = 2 * n;
        l->count = n >> (lane_bits + 2);
        plan->last_count = avx2 ? 2 : 1;
        plan->execute = onefft_execute_passes;
    }
    return floats * sizeof(float);
}

/* ---- AVX-512 --------------------------------------------------------------------------------
 *
 * 16 lanes and 32 registers: four index bits sit in the lanes, a kernel holds 8 complex vectors,
 * or 16 if it computes in place. 32 to 128 points are single kernels; below, the AVX2 plan is
 * used. Larger transforms:
 *
 *   f16         the top 4 index bits
 *     or f8     the top 3 index bits, followed by a c8 for bit n-4 (see below) and the two bits
 *               next to the lanes
 *   c8, c4      nested passes, 3 and 2 bits each
 *   la3 / la2   index bits (4,) 3, 2
 *   lb          index bits 1, 0
 *
 * The four exchanges of lanes and registers in la and lb are one shuffle per register each
 * only in one arrangement (gen/onefft_gen/avx512_kernels.py): the top four position bits of the
 * output hold, from the top, output bits 0, 2, 1, 3. The radix 16 first pass writes its results
 * accordingly. The radix 8 first pass produces only three of them: output bit 3 comes from index
 * bit n-4, which it therefore leaves at that position.
 */
#ifdef ONEFFT_AVX512

static size_t onefft_avx512_low_bits(size_t region, unsigned bits) {
    static const unsigned char order[4] = {0, 2, 1, 3};
    size_t low = 0;
    for (unsigned i = 0; i < bits; i++) low |= ((region >> (bits - 1 - i)) & 1) << order[i];
    return low;
}

static const onefft_x86_kernels onefft_avx512_kernels = {
    4, onefft_avx512_low_bits, {NULL, NULL}, {NULL, NULL}, onefft_avx512_c4, onefft_avx512_c8,
    {NULL, NULL}, onefft_avx512_la3, onefft_avx512_la2, NULL, NULL, {onefft_avx512_lb, onefft_avx512_lbi}, {{0}}};

/* The element of 16 consecutive values that each lane holds. */
static const unsigned char onefft_avx512_element[16] = {0, 1, 8, 9, 2, 3, 10, 11, 4, 5, 12, 13, 6, 7, 14, 15};

static size_t onefft_avx512_plan(onefft_plan *plan, float *memory) {
    const onefft_x86_kernels *kernels = &onefft_avx512_kernels;
    unsigned log2n = plan->log2n;
    int inverse = plan->inverse;
    if (log2n < 5) return onefft_x86_plan(plan, memory, &onefft_avx2_kernels);
    if (log2n < 8) { /* 32 to 128 points fit the registers */
        static onefft_small_fn *const small[3][2] = {{onefft_avx512_n32, onefft_avx512_n32i},
                                                     {onefft_avx512_n64, onefft_avx512_n64i},
                                                     {onefft_avx512_n128, onefft_avx512_n128i}};
        if (memory) {
            plan->small = small[log2n - 5][inverse];
            plan->execute = onefft_execute_small;
        }
        return 0;
    }
    size_t n = plan->n, floats = 0, vector = 128;
    if (memory) plan->middle_count = 0;

    /* Which passes. The first is radix 16 or radix 8. A radix 8 first pass is followed by a
     * pass for 3 bits that is not nested (`special`, see below). The nested middle passes take
     * 3 or 2 bits each, and the two last passes 5 or 4 together: whichever needs fewer middle
     * passes, and among equals 4 if the data fits the first level cache and 5 otherwise. (One
     * bit cannot be left to the middle passes.) */
    onefft_x86_tuning tuning = onefft_x86_tune(4);
    unsigned first_bits = log2n <= tuning.f16_to || log2n >= tuning.f16_from || log2n < 10 ? 4 : 3;
    unsigned special = first_bits == 3 ? 3 : 0;
    unsigned rest = log2n - first_bits - special - 4; /* bits of the middle passes if the last take 4 */
    unsigned last_bits = rest == 1 || (rest >= 3 && (rest % 3 == 1 || log2n > 12)) ? 5 : 4;
    unsigned nested = log2n - first_bits - special - last_bits;

    /* First pass. It counts through the index bits between its own and the lanes and writes to
     * the bit reversed position; after the radix 8 pass the top one of those bits, n-4, keeps
     * its place instead. */
    unsigned count_bits = log2n - first_bits - 4, low_bits = count_bits - (4 - first_bits);
    size_t first_count = (size_t)1 << count_bits;
    if (memory) {
        uint32_t *table = (uint32_t *)(memory + floats);
        int ordered = log2n >= tuning.ordered_from && log2n <= tuning.ordered_to;
        for (size_t a = 0; a < first_count; a++) {
            size_t low = onefft_bit_reverse(a & (((size_t)1 << low_bits) - 1), low_bits);
            size_t b = (a >> low_bits << low_bits) | low; /* the mapping is its own inverse */
            table[2 * a] = (uint32_t)((ordered ? b : a) << 4);
            table[2 * a + 1] = (uint32_t)((ordered ? a : b) << 4);
        }
        plan->first = first_bits == 4 ? (inverse ? onefft_avx512_f16i : onefft_avx512_f16)
                                      : (inverse ? onefft_avx512_f8i : onefft_avx512_f8);
        plan->first_table = table;
        plan->first_count = first_count;
        plan->first_in_stride = plan->first_out_stride = (8 * n) >> first_bits;
    }
    floats += 2 * first_count;
    floats = (floats + 15) & ~(size_t)15;

    /* After a radix 8 first pass: index bit n-4 at the fourth highest position with the two
     * bits next to the lanes, not nested. */
    unsigned done = first_bits;
    if (special) {
        floats = onefft_x86_middle(plan, memory, floats, kernels, done, 3, 3, n / 2, vector, 1, 3 * vector, n >> 10);
        done += 3;
    }
    if (memory) plan->nested_from = plan->middle_count;

    /* Nested passes, small strides first. */
    unsigned twos = (3 - nested % 3) % 3;
    for (unsigned i = 0; done + last_bits < log2n; i++) {
        unsigned bits = i < (nested - 2 * twos) / 3 ? 3 : 2;
        size_t s_high = (size_t)8 << done;
        floats = onefft_x86_middle(plan, memory, floats, kernels, done, bits, 4, s_high, 2 * s_high,
                                   (size_t)1 << (done - 4), (((size_t)1 << bits) - 1) * s_high,
                                   (n / 2) / (s_high << bits));
        done += bits;
    }
    int three = last_bits == 5; /* la3 or la2 */

    /* la: butterfly j of region r (output bits 0 and 2) has the low output bits
     * K = k0 + 2 k1 + 4 k2 + 8 k3 + 16 j. Output bits 1 and 3 (k1, k3) select the vector at
     * first and move into the lanes with the two exchanges: k3 to lane bit b0, k1 to q1.
     *   la3 only: index bit 4, a scalar twiddle exp(-2 pi i 16 K / N) per (k1, k3)
     *   index bit 3: a twiddle vector exp(-2 pi i 8 K / N) per k1, k3 varying over the lanes
     *   index bit 2: a twiddle vector exp(-2 pi i 4 K / N), for la3 a second one turned by an
     *                eighth for the upper output of index bit 4 */
    size_t start = floats, count = n >> (three ? 9 : 8);
    for (size_t r = 0; r < 4; r++) {
        for (size_t j = 0; j < count; j++) {
            uint64_t base = (r >> 1) + 4 * (r & 1) + 16 * j;
            float *table = memory ? memory + floats : NULL;
            if (three) {
                for (size_t s = 0; s < 4; s++) onefft_table_root(table, 4, s, 16 * (base + 2 * (s >> 1) + 8 * (s & 1)), log2n);
                floats += 8;
            }
            for (size_t k1 = 0; k1 < 2; k1++) {
                table = memory ? memory + floats : NULL;
                for (size_t lane = 0; lane < 16; lane++) {
                    onefft_table_root(table, 16, lane, 8 * (base + 2 * k1 + 8 * (lane & 1)), log2n);
                }
                floats += 32;
            }
            for (size_t c = 0; c < (three ? 2u : 1u); c++) {
                table = memory ? memory + floats : NULL;
                for (size_t lane = 0; lane < 16; lane++) {
                    onefft_table_root(table, 16, lane, 4 * (base + 2 * (lane >> 3) + 8 * (lane & 1)) + (n >> 3) * c, log2n);
                }
                floats += 32;
            }
        }
    }
    if (memory) {
        onefft_last *l = &plan->last[0];
        l->run = three ? kernels->la : kernels->la2;
        l->tw = memory + start;
        l->regions = 4;
        l->stride = three ? n / 4 : n / 2;
        l->count = count;
    }
    floats = (floats + 15) & ~(size_t)15;

    /* lb: butterfly j handles the output indices K + (N/4) q, K = element + 16 j. */
    start = floats;
    for (size_t j = 0; j < n >> 6; j++) {
        for (size_t l = 1; l < 4; l++) {
            float *table = memory ? memory + floats : NULL;
            for (size_t lane = 0; lane < 16; lane++) {
                onefft_table_root(table, 16, lane, l * (onefft_avx512_element[lane] + 16 * j), log2n);
            }
            floats += 32;
        }
    }
    if (memory) {
        onefft_last *l = &plan->last[1];
        l->run = kernels->lb[inverse];
        l->tw = memory + start;
        l->regions = 1;
        l->stride = 2 * n;
        l->count = n >> 6;
        plan->last_count = 2;
        plan->execute = onefft_execute_passes;
    }
    return floats * sizeof(float);
}

#endif /* ONEFFT_AVX512 */

/* The best instruction set that this build contains and the processor and operating system
 * support: 4 for AVX-512, 3 for AVX2 with FMA, 2 for SSE2 (the number of lane bits). */
static unsigned onefft_x86_select(void) {
#ifdef ONEFFT_AVX2
    __builtin_cpu_init();
    if (__builtin_cpu_supports("avx2") && __builtin_cpu_supports("fma")) {
#ifdef ONEFFT_AVX512
        if (__builtin_cpu_supports("avx512f")) return 4;
#endif
        return 3;
    }
#endif
    return 2;
}

static size_t onefft_x86_build(onefft_plan *plan, float *memory) {
    switch (onefft_x86_select()) {
#ifdef ONEFFT_AVX512
    case 4: return onefft_avx512_plan(plan, memory);
#endif
#ifdef ONEFFT_AVX2
    case 3: return onefft_x86_plan(plan, memory, &onefft_avx2_kernels);
#endif
    default: return onefft_x86_plan(plan, memory, &onefft_sse2_kernels);
    }
}

#endif /* ONEFFT_X86 */

/* ---- public interface ---------------------------------------------------------------------- */

static size_t onefft_build(onefft_plan *plan, float *memory) {
#if defined(ONEFFT_NEON)
    return onefft_neon_plan(plan, memory);
#elif defined(ONEFFT_X86)
    return onefft_x86_build(plan, memory);
#else
    return onefft_scalar_plan(plan, memory);
#endif
}

const char *onefft_implementation(void) {
#if defined(ONEFFT_NEON)
    return ONEFFT_NEON_FMA ? "neon (fused)" : "neon";
#elif defined(ONEFFT_X86)
    switch (onefft_x86_select()) {
    case 4: return "avx512";
    case 3: return "avx2";
    default: return "sse2";
    }
#else
    return "portable";
#endif
}

static void *onefft_aligned_alloc(size_t bytes) {
#ifdef _WIN32
    return _aligned_malloc(bytes, 64);
#else
    void *memory = NULL;
    return posix_memalign(&memory, 64, bytes) ? NULL : memory;
#endif
}

static void onefft_aligned_free(void *memory) {
#ifdef _WIN32
    _aligned_free(memory);
#else
    free(memory);
#endif
}

onefft_plan *onefft_plan_create(size_t n, int direction) {
    if (n == 0 || (n & (n - 1)) != 0) return NULL;
    if (direction != ONEFFT_FORWARD && direction != ONEFFT_INVERSE) return NULL;
    onefft_plan *plan = (onefft_plan *)calloc(1, sizeof(onefft_plan));
    if (!plan) return NULL;
    plan->n = n;
    while (((size_t)1 << plan->log2n) < n) plan->log2n++;
    plan->inverse = direction == ONEFFT_INVERSE;
    /* Tables address the data with 32 bit offsets, in units of complex values. */
    if (plan->log2n > ONEFFT_MAX_LOG2) {
        free(plan);
        return NULL;
    }
    size_t bytes = onefft_build(plan, NULL);
    plan->memory = onefft_aligned_alloc(bytes + 64);
    if (!plan->memory) {
        free(plan);
        return NULL;
    }
    onefft_build(plan, (float *)plan->memory);
    return plan;
}

void onefft_plan_destroy(onefft_plan *plan) {
    if (!plan) return;
    onefft_aligned_free(plan->memory);
    free(plan);
}

void onefft_plan_execute(const onefft_plan *plan, const float *in, float *out) { plan->execute(plan, in, out); }

size_t onefft_plan_size(const onefft_plan *plan) { return plan->n; }

/* ---- the plan cache behind onefft_forward() and onefft_inverse() ----------------------------
 *
 * One slot per direction and size. A slot is written once, with a compare-and-swap, and only
 * read afterwards, so transforms need no lock. If two threads create the plan for a slot at the
 * same time, one of them wins and the other frees its copy.
 */

static onefft_plan *onefft_cache[2][8 * sizeof(size_t)];

static const onefft_plan *onefft_cache_get(size_t n, int direction) {
    if (n == 0 || (n & (n - 1)) != 0) return NULL;
    unsigned log2n = (unsigned)__builtin_ctzll((unsigned long long)n);
    onefft_plan **slot = &onefft_cache[direction][log2n];
    onefft_plan *plan = __atomic_load_n(slot, __ATOMIC_ACQUIRE);
    if (plan) return plan;
    plan = onefft_plan_create(n, direction);
    if (!plan) return NULL;
    onefft_plan *expected = NULL;
    if (!__atomic_compare_exchange_n(slot, &expected, plan, 0, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE)) {
        onefft_plan_destroy(plan);
        plan = expected;
    }
    return plan;
}

int onefft_forward(const float *in, float *out, size_t n) {
    const onefft_plan *plan = onefft_cache_get(n, ONEFFT_FORWARD);
    if (!plan) return -1;
    plan->execute(plan, in, out);
    return 0;
}

int onefft_inverse(const float *in, float *out, size_t n) {
    const onefft_plan *plan = onefft_cache_get(n, ONEFFT_INVERSE);
    if (!plan) return -1;
    plan->execute(plan, in, out);
    return 0;
}

int onefft_cache_prepare(size_t n, int direction) {
    if (direction != ONEFFT_FORWARD && direction != ONEFFT_INVERSE) return -1;
    return onefft_cache_get(n, direction) ? 0 : -1;
}

void onefft_cache_clear(void) {
    for (int direction = 0; direction < 2; direction++) {
        for (size_t i = 0; i < 8 * sizeof(size_t); i++) {
            onefft_plan_destroy(__atomic_exchange_n(&onefft_cache[direction][i], NULL, __ATOMIC_ACQ_REL));
        }
    }
}
