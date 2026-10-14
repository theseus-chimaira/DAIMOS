/* Inactive scheduler boundary for compressed physical process storage. */
#ifndef DAIMOS_PROC_PACKED_LIFECYCLE_V088_H
#define DAIMOS_PROC_PACKED_LIFECYCLE_V088_H
#include "proc_packed_inline_v088.h"
int proc_packed_try_evict_v088(unsigned int slot, kword_t retained_offset);
int proc_packed_try_restore_v088(unsigned int slot, kword_t backing,
    proc_packed_pure_reload_v31 reload_pure);
#endif
