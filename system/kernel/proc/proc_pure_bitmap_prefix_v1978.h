/* K12 synchronous prefix lease; experimental directory profile only. */
#ifndef PURE_BITMAP_PREFIX_V1978_H
#define PURE_BITMAP_PREFIX_V1978_H
#include "proc.h"
#include "proc_pure_bitmap_directory_v1977.h"

/* Both legacy kind bits deliberately form a new kind rejected by the
 * old RAW idle-recovery path. RH is zero until encoding succeeds, then
 * holds the exact encoded count. Never interpret this RH as a target.
 */
/* v1985 placed DROP: bit31 distinguishes the proposed bitmap target
 * from a dirty prefix's original suffix. Mode0 and original VM base
 * persist until commit publication; RH remains the encoded count.
 * Neither legacy RAW recovery nor mode3 terminal may consume it. */
#define PROC_PURE_BITMAP_PLACE_V1985(target, count) \
        (PROC_PURE_INPLACE_ESCROW_V1919((target), PROC_PURE_KIND_MASK_V1919) | \
        PROC_PURE_TERM_TAG_V1920 | (count))

#define PROC_PURE_BITMAP_PREP_V1978 PROC_PURE_KIND_MASK_V1919
#define PROC_PURE_BITMAP_ESCROW_V1978(suffix) \
        PROC_PURE_INPLACE_ESCROW_V1919((suffix), PROC_PURE_BITMAP_PREP_V1978)

/* Caller-stack identity, not a persistent owner registry. The original
 * full extent and its backing must remain unchanged across the lease. */
struct pure_bitmap_lease_v1978 {
        kword_t vm_state;
        kword_t backing;
        kword_t uarea;
        kword_t identity;
};

/* Caller holds TRANSITION, global PI exclusion and the same synchronous
 * executive continuation throughout prepare and commit/cancel. No I/O,
 * allocation, sleep or callback between them. The lease object is private
 * kernel stack storage. No pin may be acquired by another actor.
 *
 * OK: dirty prefix, original once-pinned PROCESS, exact encoded count in
 * word 067 RH; only commit/cancel may follow. Ordinary no-gain/space or
 * other clean refusal: original resident bytes unchanged, no usable
 * lease. BUSY can mean an uncertain pin/owner; retain TRANSITION.
 * WRITE_FAULT: dirty incomplete candidate; quarantine except for the
 * v1993 staged DROP wrapper's original disjoint-write identity proof.
 *
 * Cancel is only for a successful preparation in the SAME PI window.
 * It discards the candidate by committing normal RAW omission, NOT by
 * making the damaged resident image runnable. On uncertainty keep the
 * existing escrow/TRANSITION. v1993 optionally retains an authorized
 * original-pin rollback phase across a refused rebase or unpin.
 */
int proc_pure_bitmap_prepare_v1978(unsigned int slot,
    struct pure_bitmap_lease_v1978 *lease);
int proc_pure_bitmap_cancel_v1978(unsigned int slot,
    const struct pure_bitmap_lease_v1978 *lease);
/* Staged K13 commit: zero = bitmap, positive = exact RAW cancellation.
 * BUSY leaves uncertain ownership quarantined under TRANSITION. */
#define PBMP_RAW_V1979 3
int proc_pure_bitmap_commit_v1979(unsigned int slot,
    const struct pure_bitmap_lease_v1978 *lease);
/* Synchronous staged reclaim entry. Success means exact RAW or bitmap
 * published and TRANSITION cleared; uncertainty must remain held. */
int proc_pure_bitmap_drop_v1979(unsigned int slot);
#ifdef DAIMOS_PBM_DIRTY_DROP_V1993
/* Dirty DROP rollback only. RH bits17..16 cannot occur in a valid encoded
 * count (bounded by the PURE prefix). The original suffix sector is
 * retained, not an address discovered from a later MM descriptor.
 * HOLD has no retry bit. Neither old RAW nor mode3 FULL may consume it.
 * The saved backing word is reserved under TRANSITION even before
 * publishing mode2. No new permanent words. */
#define PBD_MARK_V1993 0600000UL
#define PBD_REBASE_V1993 1U
#define PBD_UNPIN_V1993 2U
/* RH: marker17..16, reserved15, original APR words/02000 in
 * 14..7, executable credential31..27 in 6..2, phase1..0.
 * Neither a valid FULL phase (1..4) nor a candidate count has
 * both marker bits set, even if the storage map is inconsistent. */
#define PBD_TAG_V1993(source, phase, words, cred) \
    ((PROC_PURE_BITMAP_ESCROW_V1978(source) & ~PROC_SWAP_RETRY_TAG_V1507) | \
    PBD_MARK_V1993 | (((words) >> 10U) << 7U) | (((cred) >> 27U) << 2U) | \
    (phase) | ((phase) ? PROC_SWAP_RETRY_TAG_V1507 : 0UL))
#define PBD_NOT_MINE_V1993 1
/* Slot-zero only. Positive NOT_MINE allows the ordinary RAW handler;
 * all other results belong to the directory DROP transaction. */
int proc_pure_dirty_recover_v1993(unsigned int slot);
#endif
#endif
