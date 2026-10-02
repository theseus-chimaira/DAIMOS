#ifndef DAIMOS_USER_TSFS_SCAN_H
#define DAIMOS_USER_TSFS_SCAN_H

#include "u.h"

#define TSFS_BLOCK_WORDS 0200U
#define TSFS_BLOCK_COUNT 01102U
#define TSFS_DESC_PRIMARY_BLOCK 1U
#define TSFS_DESC_BACKUP_BLOCK 2U
#define TSFS_DESC_USED_WORDS 12U
#define TSFS_MAX_MEMBERS 8U

struct tsfs_scan_result {
        kword_t id_hi;
        kword_t id_lo;
        kword_t generation;
        unsigned int members;
        unsigned int unit[TSFS_MAX_MEMBERS];
        kword_t blocks[TSFS_MAX_MEMBERS];
        unsigned int tdir_member;
        kword_t tdir_block;
        kword_t tdir_blocks;
        kword_t tdir_checksum;
};

int tsfs_scan(unsigned int seed_unit, struct tsfs_scan_result *out);
int tsfs_build_mount_handoff(const struct tsfs_scan_result *scan,
    kword_t out[SYS_TSFS_MOUNT_WORDS]);

#endif
