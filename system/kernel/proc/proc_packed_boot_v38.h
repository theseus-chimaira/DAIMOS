/* Optional, transient KINIT allocation for the future packed descriptor map.
 * Do not invoke until packed process teardown is integrated with VM destroy.
 */
#ifndef DAIMOS_PROC_PACKED_BOOT_V38_H
#define DAIMOS_PROC_PACKED_BOOT_V38_H
#include "proc_packed_layout_v36.h"

/* Runtime descriptor; zero-initialized and resident once KINIT publishes it. */
extern struct proc_packed_layout_v36 proc_packed_runtime_v38;

/* The caller chooses expected packed concurrency and a descriptor RAM budget
 * after required KINIT allocations. No compressor or scheduler dependency.
 * The implementation allocates ONE kernel-dynamic extent, not per process.
 */
int proc_packed_boot_prepare_v38(unsigned int slots,
    unsigned int simultaneous_packed, unsigned int budget_words);

/* Only valid before the map is used: undo a prepared but unactivated table.
 * The allocated entries must all remain empty; failure retains ownership.
 */
int proc_packed_boot_abort_v38(void);
#endif
