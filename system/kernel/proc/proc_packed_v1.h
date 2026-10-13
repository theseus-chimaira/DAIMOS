/*
 * @file proc_packed_v1.h
 * @brief Contract for future in-core packed process allocations.
 *
 * This header is intentionally declarative. No runtime packed-process state
 * exists until eviction, restoration, and rollback can be atomic.
 */
#ifndef DAIMOS_PROC_PACKED_V1_H
#define DAIMOS_PROC_PACKED_V1_H

#include "mm.h"

/* Packed extents belong to their process slot and need no APR alignment. */
#define PROC_PACKED_MM_TYPE MM_TYPE_PACKED_PROCESS

/*
 * Two-word inactive-process payload descriptor.  The separate lifecycle
 * discriminator is mandatory: neither a zero VM base nor a swap-record value
 * identifies the storage type safely.
 *
 * span: LH encoded word count, RH physical MM base.
 * layout: LH first retained writable word, RH decoded writable word count.
 *
 * A future sparse format may need more than one region; this initial format
 * describes only one contiguous writable suffix.  Its offsets are logical
 * process offsets, not physical addresses.
 */
struct proc_packed_v1 {
        kword_t span;
        kword_t layout;
};

#define PROC_PACKED_SPAN(base, words) \
        ((((kword_t)(words) & MM_HALF_MASK) << 18U) | \
         ((kword_t)(base) & MM_HALF_MASK))
#define PROC_PACKED_BASE(p) ((p)->span & MM_HALF_MASK)
#define PROC_PACKED_WORDS(p) (((p)->span >> 18U) & MM_HALF_MASK)
#define PROC_PACKED_OFFSET(p) (((p)->layout >> 18U) & MM_HALF_MASK)
#define PROC_PACKED_DECODED(p) ((p)->layout & MM_HALF_MASK)

/* Future lifecycle tag, stored OUTSIDE the overloaded swap record. */
#define PROC_STORAGE_RESIDENT 0U
#define PROC_STORAGE_DISK     1U
#define PROC_STORAGE_PACKED   2U

/*
 * A future nonresident packed process must retain:
 * - stable u-area containing saved registers and PURE executable backing;
 * - an MM_TYPE_PACKED_PROCESS extent owned by its process slot;
 * - a packed span (RH physical base, LH compressed word count).
 *
 * The existing proc_swap_records[slot].state is ambiguous across resident
 * and legacy disk-swapped processes. It MUST NOT hold a packed span until
 * a separately validated ownership discriminator has been introduced.
 * A packed process may not be scheduled or swapped by legacy disk routines.
 */
#endif
