/* Exact-size codec-backed eviction. Caller must hold TRANSITION. */
#ifndef DAIMOS_PROC_PACKED_EVICT_V077_H
#define DAIMOS_PROC_PACKED_EVICT_V077_H
#include "proc_packed_v1.h"
typedef int (*proc_packed_sizer_v077)(const kword_t *source,
    unsigned int words);
int proc_packed_evict_codec_v077(unsigned int slot,
    kword_t retained_offset, proc_packed_sizer_v077 sizer,
    proc_packed_encoder_v32 encoder);
#endif
