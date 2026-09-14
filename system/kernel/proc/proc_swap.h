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

struct proc_swap_record {
        /* Resident: packed executable backing + pure-text boundary.
         * Swapped: LH first SWAP block, RH full-sector block count; the
         * resident backing record is retained in the stable u-area. */
        kword_t state;
};

extern struct proc_swap_record *proc_swap_records;
extern kword_t proc_swap_blocks_used;
int proc_swap_boot_init(unsigned int slots);

int proc_swap_attach(int slot, vnode_t backing,
    kword_t text_words, unsigned int pure);
void proc_swap_detach(int slot);
int proc_swap_out(int slot);
int proc_swap_in(int slot);
int proc_swap_is_swapped(int slot);
int proc_swap_service_one(void);
int proc_swap_reclaim(kword_t words, kword_t alignment,
    unsigned int exclude_owner);

#endif
