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

/*
 * Staging-only allocation API. The caller owns DESCRIPTOR and must keep it
 * unpublished until the entire eviction transaction succeeds. These calls
 * do not alter process residency or legacy swap records.
 */
int proc_packed_reserve_v1(unsigned int slot, kword_t encoded_words,
    kword_t logical_offset, kword_t decoded_words,
    struct proc_packed_v1 *descriptor);
int proc_packed_release_v1(unsigned int slot,
    struct proc_packed_v1 *descriptor);

/*
 * Prepare an aligned, PRIVATE restored process image. The caller must reload
 * the omitted PURE prefix before publishing the result or freeing the packed
 * source. The prepared destination remains PINNED until the caller has
 * reloaded PURE and committed its VM base, or invokes abort_restore.
 * The eventual commit path must release that pin after publishing the base.
 * No residency or scheduler state is modified by this API.
 */
typedef int (*proc_packed_decoder_v26)(const kword_t *src,
    unsigned int encoded_words, kword_t *dst, unsigned int decoded_words);
int proc_packed_prepare_restore_v26(unsigned int slot,
    const struct proc_packed_v1 *descriptor, kword_t process_words,
    proc_packed_decoder_v26 decoder, kword_t *restored_base);
int proc_packed_abort_restore_v26(unsigned int slot, kword_t restored_base);

/* Complete restore is staged outside the live scheduler until tested.
 * PURE reloader receives executable backing and a pinned new process base.
 * It must reconstruct all omitted words, including the compressed DXR case.
 * The writable decoder is invoked AFTER this reload, so PURE reconstruction
 * cannot overwrite saved writable state.
 */
typedef int (*proc_packed_pure_reload_v31)(kword_t backing, kword_t base);
/* Caller must supply a LOSSLESS encoder and its true upper bound. A result
 * no smaller than the retained suffix is rejected: identity is not packing.
 * The destination descriptor must remain private throughout the transaction.
 */
typedef int (*proc_packed_encoder_v32)(const kword_t *source,
    unsigned int words, kword_t *destination, unsigned int capacity);
int proc_packed_evict_transaction_v32(unsigned int slot,
    struct proc_packed_v1 *descriptor, kword_t retained_offset,
    unsigned int encoded_capacity, proc_packed_encoder_v32 encoder);
/* Inactive registry-aware eviction. Unlike v32 it must publish to the
 * boot-selected MM-authoritative registry before clearing TRANSITION.
 * On an unrecoverable post-commit error it leaves TRANSITION held. */
int proc_packed_evict_transaction_v45(unsigned int slot,
    kword_t retained_offset, unsigned int encoded_capacity,
    proc_packed_encoder_v32 encoder);
int proc_packed_restore_transaction_v31(unsigned int slot,
    struct proc_packed_v1 *descriptor, kword_t backing,
    proc_packed_decoder_v26 decoder,
    proc_packed_pure_reload_v31 reload_pure);

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
