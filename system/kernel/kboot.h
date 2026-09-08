#ifndef DAIMON_KBOOT_H
#define DAIMON_KBOOT_H

#include "kcore.h"
#include "memfs.h"

#define KBOOT_NODE_COUNT         64U
#define KBOOT_NODE_WORDS         (KBOOT_NODE_COUNT * 8U)
/* RAMFS0 is optional scratch storage.  Keep it disabled on machines below
 * 96K words; low-memory systems need the core more than a tiny RAM disk.
 * KINIT still uses a node-only MEMFS instance transiently while mounting the
 * real root, then releases it before entering KCORE. */
#define KBOOT_RAMFS0_MIN_CORE_WORDS 0300000UL
#define KBOOT_RAMFS0_MAX_WORDS      010000U
#define KBOOT_RAMFS0_MIN_WORDS      (KBOOT_NODE_WORDS + 01000U)
#define KBOOT_FIRST_USER_RESERVE_WORDS 014000U

int kfs_boot_rebind_root(void);

#endif
