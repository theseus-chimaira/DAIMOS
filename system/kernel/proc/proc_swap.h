/**
 * @file proc_swap.h
 * @brief Compact process-swap backing format and runtime swap interface.
 *
 * Each process owns one resident swap-record word. While resident it encodes
 * executable backing and the PURE text boundary; while swapped it encodes the
 * D6FS swap-tail block span, with the original backing word retained in the
 * stable u-area. This avoids a separate swap header and keeps per-process
 * permanent RAM cost to one word.
 */
#ifndef DAIMON_PROC_SWAP_H
#define DAIMON_PROC_SWAP_H

#include "proc.h"
#include "vfs.h"

#define PROC_SWAP_RECORD_WORDS 1U
#define PROC_SWAP_TEXT_MASK       037777UL
#define PROC_SWAP_INDEX_MASK      0777777UL
#define PROC_SWAP_MOUNT_MASK      03UL
#define PROC_SWAP_PROVIDER_MASK   03UL
#define PROC_SWAP_MOUNT_SHIFT     18U
#define PROC_SWAP_PROVIDER_SHIFT  20U
#define PROC_SWAP_TEXT_SHIFT      22U

/** One-word resident/swapped process backing descriptor. */
struct proc_swap_record {
        /* Resident: packed executable backing + pure-text boundary.
         * Swapped: LH first SWAP block, RH full-sector block count; the
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
void proc_swap_detach(int slot);
/** Write one resident process image to swap-tail storage and release its VM. */
int proc_swap_out(int slot);
/** Restore one swapped process image into a new aligned resident extent. */
int proc_swap_in(int slot);
int proc_swap_service_one(void);
/** Reclaim process VM until MM can satisfy a requested extent or no victim remains. */
int proc_swap_reclaim(kword_t words, kword_t alignment,
    unsigned int exclude_owner);

#endif
