#ifndef PURE_FULL_ESCROW_V1989_H
#define PURE_FULL_ESCROW_V1989_H

#include "proc_pure_bitmap_restore_v1980.h"

/* Staged directory FULL only. While TRANSITION excludes the owner:
 * 067 LH: unchanged source sector, both kind bits, retry tag.
 *     RH: phase (3 bits), exact encoded length (15 bits).
 * 0407 LH: original APR-aligned BRK floor, plus version bit and
 *          target APR index (8 bits); one reserved bit must be zero.
 *      RH: original current BRK, completely unchanged.
 * The maximum PURE skip is 040000 words (14-bit text + origin).
 * Old FULL tags do not carry the BRK version bit. Terminal tags
 * and ordinary timer epochs remain distinct. All updates are PI
 * excluded. Clear the complete low floor field before resumption.
 */
#define PBR_LEN_V1989       077777UL
#define PBR_PHASE_SHIFT_V1989 15U
#define PBR_HOLD_V1989      1U
#define PBR_PAIR_V1989      2U
#define PBR_FREE_V1989      3U
#define PBR_SOURCE_V1989    4U
#define PBR_BRK_MASK_V1989  (01777UL << 18U)
#define PBR_BRK_TAG_V1989   01000UL
#define PBR_BRK_BAD_V1989   0400UL
#define PBR_TARGET_V1989    0377UL

#define PBR_TAG_V1989(source, encoded, phase) \
        PROC_PURE_BITMAP_FULL_V1980((source), \
            ((kword_t)(phase) << PBR_PHASE_SHIFT_V1989) | (encoded))
#define PBR_HEAP_V1989(brk) ((brk) & ~PBR_BRK_MASK_V1989)
#define PBR_HEAP_FIELD_V1989(brk) (((brk) >> 18U) & 01777UL)
#define PBR_TARGET_BASE_V1989(brk) \
        ((PBR_HEAP_FIELD_V1989(brk) & PBR_TARGET_V1989) << 10U)

/* Explicit slot-zero rollback only. Never publish a deferred FULL
 * destination, consume unknown pins or recover a HOLD transaction.
 * Not yet connected to the idle service or production package.
 */
int proc_pure_bitmap_recover_v1989(unsigned int slot);

#endif
