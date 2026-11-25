/**
 * @file proc_swap.h
 * @brief Compact process-swap backing format and runtime swap interface.
 *
 * Each process owns one resident swap-record word. While resident it encodes
 * executable backing and the PURE immutable-text boundary; while swapped it
 * encodes the BACKSTORE block span, with the original backing word retained
 * in the stable u-area. Complete sectors covered by PURE text may therefore
 * be reconstructed from the executable instead of consuming swap blocks.
 * This avoids a separate swap header and keeps per-process permanent RAM cost
 * to one word.
 */
#ifndef DAIMON_PROC_SWAP_H
#define DAIMON_PROC_SWAP_H

#include "proc.h"
#include "vfs.h"

#define PROC_SWAP_RECORD_WORDS 1U
/* Swap-in: resource pressure is recoverable; corrupt records and failed
 * I/O are not. Scheduler integration must explicitly handle PRESSURE. */
#define PROC_SWAP_IN_OK        0
#define PROC_SWAP_IN_PRESSURE   1
#define PROC_SWAP_IN_FATAL     -1
#define PROC_SWAP_TEXT_MASK       037777UL
#define PROC_SWAP_INDEX_MASK      0777777UL
#define PROC_SWAP_MOUNT_MASK      017UL
#define PROC_SWAP_MOUNT_SHIFT     18U
#define PROC_SWAP_TEXT_SHIFT      22U
#define PROC_SWAP_MOUNT_EXT_BIT   ((kword_t)1UL << 27U)
/* Original provider, not a provider inferred from a later mount root.
 * Zero is invalid for a PURE record. Bits 28..30 are outside UID9. */
#define PROC_SWAP_PROVIDER_SHIFT_V1924 28U
#define PROC_SWAP_PROVIDER_MASK_V1924 ((kword_t)7UL << 28U)
#define PROC_SWAP_ID_MASK_V1924 (PROC_SWAP_MOUNT_EXT_BIT | PROC_SWAP_PROVIDER_MASK_V1924)
#define PROC_SWAP_STAGE_ID_MASK_V1924 035UL
#define PROC_SWAP_STAGE_PROV_SHIFT_V1924 2U

/* Only while SIDL lacks its u-area, the otherwise unused scheduler RH bit 0
 * temporarily carries the fifth mount-slot bit until RUN publishes u-area.
 * It is cleared before SRUN uses RH for its runnable-queue link. */
#define PROC_SWAP_STAGE_MOUNT_EXT_BIT 1UL

/* Resident record (one 36-bit word): index:18 | mount slot low 4 | PURE text:14.
 * The original filesystem provider is saved in credential bits 28..30.
 * Reconstruct only when the published mount still names that provider.
 * Mount lifetime guards remain necessary for same-provider reuse.
 * Mount slot bit 4 (for public IDs 17..26) lives in an otherwise spare high
 * bit of the owning stable u-area credential word (offset 066, bit 27).
 * Together they preserve all 5 slot bits and the complete 18+14 other bits,
 * with no extra per-process word. While SIDL has no u-area, RUN stages the
 * extra bit in scheduler RH bit 0 and provider in bits 2..4, then publishes
 * them when the u-area exists. Bit 1 retains its rollback meaning.
 * EXEC rolls the bit back or forward with the record. Swapped PURE restore
 * must read the same owner's u-area; it cannot use the record alone. */

/** One-word resident/swapped process backing descriptor. */
struct proc_swap_record {
        /* Resident: packed executable backing + PURE immutable-text boundary.
         * Swapped: LH first BACKSTORE block, RH full-sector block count; the
         * resident backing record is retained in the stable u-area. */
        kword_t state;
};

extern struct proc_swap_record *proc_swap_records;
extern kword_t proc_swap_blocks_used;
/** Allocate and initialize one swap-record word per configured process slot. */
int proc_swap_boot_init(unsigned int slots);

/** Attach validated executable backing metadata to a resident process. */
int proc_swap_attach(int slot, vnode_t backing,
    kword_t text_words, unsigned int pure);
/** Detach swap backing only after validating the complete live disk run. */
/* A PACKED RAW process uses the existing mode map and a separate exact
 * PDP-6 MM-owner release, never the disk-run free path (v1609). */
int proc_swap_detach(int slot);
/** Write one resident process image to swap-tail storage and release its VM. */
int proc_swap_out(int slot);
/** Restore one swapped process image into a new aligned resident extent. */
int proc_swap_in(int slot);
/** Return nonzero while a staged, resident or swapped PURE process depends on this vnode. */
int proc_swap_backing_busy(vnode_t node);
/** Return nonzero while a staged, resident or swapped PURE process depends on this mount. */
int proc_swap_mount_busy(unsigned int mount);
int proc_swap_service_one(void);
/** Reclaim process VM until MM can satisfy a requested extent or no victim remains.
 * Returns MM_OK, -1 if no victim can be reclaimed, or an MM_ERR_INVAL/BUSY
 * refusal from compaction without swapping additional unrelated owners. */
int proc_swap_reclaim(kword_t words, kword_t alignment,
    unsigned int exclude_owner);

#endif
