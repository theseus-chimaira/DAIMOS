/**
 * @file proc_pure_ram_v1377.h
 * @brief Staged swapless PURE-only inactive-process transactions.
 *
 * NOT linked or enabled by the production scheduler. The caller must own
 * lifecycle exclusion, PROC_F_TRANSITION and the executable's mutation and
 * unmount exclusion through the complete transition. Only sleeping or
 * stopped processes with saved user registers may be considered.
 * Compilation is restricted to DAIMOS_SWAPLESS_PURE_RAM_V1377=1.
 */
#ifndef DAIMOS_PROC_PURE_RAM_V1377_H
#define DAIMOS_PROC_PURE_RAM_V1377_H

#include "proc_packed_boot_v38.h"

/* Rebuild canonical PURE text from executable backing. This is the ABI of
 * proc_swap_reload_pure: backing, new APR base and owning stable u-area.
 * Return zero only after complete validated text reconstruction. */
typedef int (*proc_pure_ram_reload_v1377)(kword_t, kword_t, kword_t);

/* Drop reconstructible PURE sectors; retain the mutable suffix verbatim in
 * an owner-checked, unaligned MM extent. The no-compression representation
 * reuses the inactive PACKED mode and MM type but NEVER uses the codec.
 * Return MM_OK on commit, typed MM error without publication otherwise.
 */
int proc_pure_ram_drop_v1377(unsigned int slot);

/* Reconstruct one complete APR-aligned process extent. The inactive extent
 * and executable backing remain authoritative until the restore commits.
 * A nonzero result MUST NOT make the process runnable.
 */
int proc_pure_ram_restore_v1377(unsigned int slot,
    proc_pure_ram_reload_v1377 reload);

/* Release a validated inactive extent on EXIT/EXEC/failed RUN teardown.
 * Caller holds TRANSITION and exclusion; backing release follows MM free.
 */
int proc_pure_ram_discard_v1377(unsigned int slot);

#endif
