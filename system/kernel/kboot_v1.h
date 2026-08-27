#ifndef DAIMON_KBOOT_V1_H
#define DAIMON_KBOOT_V1_H

#include "kcore.h"
#include "memfs_v1.h"
#include "file_v1.h"
#include "proc_v1.h"
#include "devicefs_v1.h"

#define KBOOT_V1_TOTAL_WORDS        01000000UL
#define KBOOT_V1_USER_BASE          0100000UL
#define KBOOT_V1_USER_LIMIT         0200000UL
#define KBOOT_V1_NODE_COUNT         64U
#define KBOOT_V1_RAMFS0_BASE        0600000UL
#define KBOOT_V1_RAMFS0_WORDS       0200000U
#define KBOOT_V1_NODE_WORDS          (KBOOT_V1_NODE_COUNT * 8U)
#define KBOOT_V1_FILE_TABLE_WORDS    (FILE_V1_NFILE * 3U)
#define KBOOT_V1_FILE_CWD_WORDS      FILE_V1_OWNER_MAX
#define KBOOT_V1_FILE_ALIAS_WORDS    ((FILE_V1_OWNER_MAX + 35U) / 36U)
#define KBOOT_V1_FILE_STATE_WORDS    (KBOOT_V1_FILE_TABLE_WORDS + \
    KBOOT_V1_FILE_CWD_WORDS + KBOOT_V1_FILE_ALIAS_WORDS)
#define KBOOT_V1_PROC_TABLE_WORDS    (PROC_V1_NPROC * 2U)
#define KBOOT_V1_DEVICE_STATE_WORDS  (1U + DEVICEFS_V1_IO_IN_COUNT + \
    DEVICEFS_V1_IO_OUT_COUNT)
#define KBOOT_V1_RUNTIME_STATE_WORDS (KBOOT_V1_FILE_STATE_WORDS + \
    KBOOT_V1_PROC_TABLE_WORDS + KBOOT_V1_DEVICE_STATE_WORDS)

#if KBOOT_V1_RAMFS0_BASE + KBOOT_V1_RAMFS0_WORDS != KBOOT_V1_TOTAL_WORDS
#error "RAMFS0 must occupy the top 64K words"
#endif

extern struct memfs_v1 kboot_fs_v1;
extern kword_t kboot_fs_ready_v2;
extern kword_t kcore_resident_end_v1;
extern kword_t kcore_cty_putchar_v1;
extern kword_t kcore_cty_getchar_v1;

void kcore_boot_v1(void);

#endif
