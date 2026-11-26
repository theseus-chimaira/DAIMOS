/* K12 staged, synchronous prefix lease; no installed mode3 path. */
#ifndef PURE_BITMAP_PREFIX_V1978_H
#define PURE_BITMAP_PREFIX_V1978_H
#include "proc.h"
#include "proc_pure_bitmap_directory_v1977.h"

/* Both legacy kind bits deliberately form a new kind rejected by the
 * old RAW idle-recovery path. RH is zero until encoding succeeds, then
 * holds the exact encoded count. Never interpret this RH as a target.
 */
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
 * other clean refusal: original resident bytes unchanged, lease not
 * published. BUSY can mean an uncertain pin/owner; retain TRANSITION.
 * WRITE_FAULT: dirty incomplete candidate, no valid lease; quarantine.
 *
 * Cancel is only for a successful preparation in the SAME PI window.
 * It discards the candidate by committing normal RAW omission, NOT by
 * making the damaged resident image runnable. On uncertainty keep the
 * existing escrow/TRANSITION. No deferred pin-provenance claim here.
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
#endif
