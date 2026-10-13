/* Staged inline-codec packed-process entry points, not scheduler-linked. */
#ifndef DAIMOS_PROC_PACKED_INLINE_V079_H
#define DAIMOS_PROC_PACKED_INLINE_V079_H
#include "proc_packed_v1.h"
int proc_packed_inline_evict_v079(unsigned int slot, kword_t retained_offset);
int proc_packed_inline_restore_v079(unsigned int slot, kword_t backing,
    proc_packed_pure_reload_v31 reload_pure);
#endif
