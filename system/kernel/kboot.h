#ifndef DAIMON_KBOOT_H
#define DAIMON_KBOOT_H

#include "kcore.h"
#include "memfs.h"

#define KBOOT_TOTAL_WORDS        01000000UL
#define KBOOT_USER_BASE          0100000UL
#define KBOOT_USER_LIMIT         0200000UL
#define KBOOT_NODE_COUNT         64U
#define KBOOT_RAMFS0_BASE        0600000UL
#define KBOOT_RAMFS0_WORDS       0200000U
#define KBOOT_NODE_WORDS          (KBOOT_NODE_COUNT * 8U)

#if KBOOT_RAMFS0_BASE + KBOOT_RAMFS0_WORDS != KBOOT_TOTAL_WORDS
#error "RAMFS0 must occupy the top 64K words"
#endif

#endif
