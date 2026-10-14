/* Staged inline-codec packed-process entry points, not scheduler-linked. */
#ifndef DAIMOS_PROC_PACKED_INLINE_V088_H
#define DAIMOS_PROC_PACKED_INLINE_V088_H
#include "proc_packed_v1.h"
int proc_packed_inline_evict_v088(unsigned int slot, kword_t retained_offset);
int proc_packed_inline_restore_v088(unsigned int slot, kword_t backing,
    proc_packed_pure_reload_v31 reload_pure);
#endif
