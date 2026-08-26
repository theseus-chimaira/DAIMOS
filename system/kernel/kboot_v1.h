#ifndef DAIMON_KBOOT_V1_H
#define DAIMON_KBOOT_V1_H

#include "kcore.h"
#include "memfs_v1.h"

#define KBOOT_V1_TOTAL_WORDS        01000000UL
#define KBOOT_V1_USER_BASE          0100000UL
#define KBOOT_V1_USER_LIMIT         0200000UL
#define KBOOT_V1_NODE_COUNT         64U
#define KBOOT_V1_RAMFS0_BASE        0600000UL
#define KBOOT_V1_RAMFS0_WORDS       0200000U

#if KBOOT_V1_RAMFS0_BASE + KBOOT_V1_RAMFS0_WORDS != KBOOT_V1_TOTAL_WORDS
#error "RAMFS0 must occupy the top 64K words"
#endif

extern struct memfs_v1 kboot_fs_v1;
extern struct memfs_v1_node kboot_nodes_v1[KBOOT_V1_NODE_COUNT];
extern vnode_v1_t kboot_ramfs0_dir_v1;
extern kword_t kboot_fs_ready_v1;
extern kword_t kcore_resident_end_v1;
extern kword_t kcore_cty_putchar_v1;
extern kword_t kcore_cty_getchar_v1;

void kcore_boot_v1(void);

#endif
