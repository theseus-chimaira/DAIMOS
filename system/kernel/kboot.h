#ifndef DAIMON_KBOOT_H
#define DAIMON_KBOOT_H

#include "kcore.h"
#include "memfs.h"

#define KBOOT_NODE_COUNT         64U
#define KBOOT_NODE_WORDS         (KBOOT_NODE_COUNT * 8U)
/* RAMFS0 is bootstrap scratch/storage, not a fixed physical reservation.
 * Keep it useful on 32K systems while preserving core for a user process. */
#define KBOOT_RAMFS0_MAX_WORDS   010000U
#define KBOOT_RAMFS0_MIN_WORDS   (KBOOT_NODE_WORDS + 01000U)
/* RAMFS0 is disposable scratch.  On a small machine it must yield enough
 * managed core for the first protected user image and some growth headroom. */
#define KBOOT_FIRST_USER_RESERVE_WORDS 014000U

int kfs_boot_rebind_root(void);

#endif
