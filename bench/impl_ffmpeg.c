/* Adapter for FFmpeg's libavutil/tx (AV_TX_FLOAT_FFT, out-of-place). */
#include "impl.h"
#include <libavutil/mem.h>
#include <libavutil/tx.h>

typedef struct {
    AVTXContext *ctx;
    av_tx_fn fn;
} ffmpeg_plan;

static void *ffmpeg_create(size_t n) {
    ffmpeg_plan *p = av_mallocz(sizeof(*p));
    float scale = 1.0f;
    if (n < 2 || av_tx_init(&p->ctx, &p->fn, AV_TX_FLOAT_FFT, 0, (int)n, &scale, 0) < 0) {
        av_free(p);
        return NULL;
    }
    return p;
}

static void ffmpeg_run(void *plan, const float *in, float *out) {
    ffmpeg_plan *p = plan;
    p->fn(p->ctx, out, (void *)in, sizeof(AVComplexFloat));
}

static void ffmpeg_destroy(void *plan) {
    ffmpeg_plan *p = plan;
    av_tx_uninit(&p->ctx);
    av_free(p);
}

const bench_impl impl_ffmpeg = {"ffmpeg", ffmpeg_create, ffmpeg_run, ffmpeg_destroy};
