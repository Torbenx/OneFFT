/* Adapter for OneFFT itself. */
#include "impl.h"
#include "onefft.h"

static void *one_create(size_t n) { return onefft_plan_create(n, ONEFFT_FORWARD); }
static void one_run(void *plan, const float *in, float *out) { onefft_plan_execute(plan, in, out); }
static void one_destroy(void *plan) { onefft_plan_destroy(plan); }

const bench_impl impl_onefft = {"onefft", one_create, one_run, one_destroy};
