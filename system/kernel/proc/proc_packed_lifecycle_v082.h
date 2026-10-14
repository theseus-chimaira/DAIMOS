/* Inactive scheduler boundary for compressed physical process storage. */
#ifndef DAIMOS_PROC_PACKED_LIFECYCLE_V082_H
#define DAIMOS_PROC_PACKED_LIFECYCLE_V082_H
#include "proc_packed_inline_v079.h"
int proc_packed_try_evict_v082(unsigned int slot, kword_t retained_offset);
int proc_packed_try_restore_v082(unsigned int slot, kword_t backing,
    proc_packed_pure_reload_v31 reload_pure);
#endif
