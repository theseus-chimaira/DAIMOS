#ifndef DAIMON_D6FS_V2_H
#define DAIMON_D6FS_V2_H

#include "kcore.h"

#define D6FS_V2_BLOCK_WORDS          0200U
#define D6FS_V2_MAX_MEMBERS          8U
#define D6FS_V2_EXTENTS              7U
#define D6FS_V2_LOGICAL_BLOCK_BITS   24U
#define D6FS_V2_EXTENT_LOW_BITS      12U
#define D6FS_V2_EXTENT_HIGH_BITS     5U
#define D6FS_V2_EXTENT_LENGTH_BITS   17U

#define D6FS_V2_LOGICAL_BLOCK_MASK   077777777UL
#define D6FS_V2_EXTENT_LOW_MASK      07777UL
#define D6FS_V2_EXTENT_HIGH_MASK     037UL
#define D6FS_V2_EXTENT_MAX_BLOCKS    0200000UL

struct d6fs_v2_diskset {
        unsigned int members;
        kword_t blocks[D6FS_V2_MAX_MEMBERS];
};

struct d6fs_v2_phys {
        unsigned int member;
        kword_t block;
};

/*
 * Encode/decode one inline extent.  The run word stores START24|LENLOW12;
 * the caller stores LENHIGH5 in the shared FCB length-high word.
 */
int d6fs_v2_extent_encode(kword_t start, kword_t blocks,
    kword_t *runp, unsigned int *highp);
int d6fs_v2_extent_decode(kword_t run, unsigned int high,
    kword_t *startp, kword_t *blocksp);

/*
 * Map the linear D6FS address space over unequal striped members.  blocks[]
 * contains the usable blocks on each member after physical swap tails have
 * already been removed by the diskset layer.  Members are numbered in their
 * configured stripe order.  Missing members are not representable here: a
 * diskset is either complete or unusable.
 */
int d6fs_v2_diskset_valid(const struct d6fs_v2_diskset *set);
kword_t d6fs_v2_diskset_blocks(const struct d6fs_v2_diskset *set);
int d6fs_v2_map_block(const struct d6fs_v2_diskset *set, kword_t logical,
    struct d6fs_v2_phys *phys);

#endif
