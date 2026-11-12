/* Shim for the proof-of-concept in reference/: the original Android logging header is not part of
 * this repository. Only what reference/fft.h and reference/aarch64 need is provided. */
#ifndef ONEFFT_REFERENCE_SHIM_LOG_H
#define ONEFFT_REFERENCE_SHIM_LOG_H
#include <stdlib.h>
#include <string.h>
#define LOGD(TAG, ...) ((void)0)
#endif
