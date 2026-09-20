#ifndef DAIMON_BLOCKSET_H
#define DAIMON_BLOCKSET_H

#include "kcore.h"

#define BLOCKSET_MAX_MEMBERS 7U
#define BLOCKSET_BLOCK_WORDS 0200U

#define BLOCKSET_POLICY_INTERLEAVE 0U
#define BLOCKSET_POLICY_CONCAT     1U

/* Permanent compact descriptor.  Sixteen words cover seven members:
 *   0      LH: members | (policy << 3), RH: equal per-member tail blocks
 *   1      total logical blocks
 *   2..8   physical unit numbers
 *   9..15  member base,,block-count
 * Keeping units unpacked costs six words versus the densest representation
 * but saves more resident PDP-6 mapper code than those words cost. */
#define BLOCKSET_DESCRIPTOR_WORDS 16U
#define BLOCKSET_DESC_FLAGS       0U
#define BLOCKSET_DESC_TOTAL       1U
#define BLOCKSET_DESC_UNIT0       2U
#define BLOCKSET_DESC_RANGE0      9U
#define BLOCKSET_DESC_MEMBER_MASK 07U
#define BLOCKSET_DESC_POLICY_SHIFT 3U


/*
 * One homogeneous logical block set.  Physical scheduling remains in the
 * leaf driver.  INTERLEAVE is the current DSK/DRM policy; CONCAT exposes
 * member windows and is suitable for media such as DECtape where TSFS owns
 * higher-level extent placement.
 */
struct blockset {
        unsigned int members;
        unsigned int policy;
        unsigned int unit[BLOCKSET_MAX_MEMBERS];
        kword_t base[BLOCKSET_MAX_MEMBERS];
        kword_t blocks[BLOCKSET_MAX_MEMBERS];
        /* Legacy equal-sized physical tail.  It is intentionally untyped;
         * the PDP-6 swap backend is the only current consumer. */
        kword_t tail_blocks;
};

kword_t blockset_blocks(void);
int blockset_read_block(kword_t logical,
    kword_t block[BLOCKSET_BLOCK_WORDS]);
int blockset_write_block(kword_t logical,
    const kword_t block[BLOCKSET_BLOCK_WORDS]);
int blockset_writable(void);

kword_t blockset_tail_blocks(void);
int blockset_tail_read(kword_t logical, kword_t count, kword_t *block);
int blockset_tail_write(kword_t logical, kword_t count, const kword_t *block);

#endif
