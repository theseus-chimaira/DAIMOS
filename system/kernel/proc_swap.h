#ifndef DAIMON_PROC_SWAP_H
#define DAIMON_PROC_SWAP_H

#include "proc.h"
#include "vfs.h"

#define PROC_SWAP_RECORD_WORDS 3U
#define PROC_SWAP_TEXT_MASK  0377777UL
#define PROC_SWAP_PURE_BIT   (0400000UL << 18U)

struct proc_swap_record {
        vnode_t backing;
        kword_t image_span;          /* LH text words, RH initialized image. */
        kword_t disk_span;           /* LH first SWAP block, RH block count. */
};

extern struct proc_swap_record *proc_swap_records;
int proc_swap_boot_init(unsigned int slots);

void proc_swap_attach(unsigned int slot, vnode_t backing,
    kword_t image_words, kword_t text_words, unsigned int pure);
void proc_swap_detach(unsigned int slot);
int proc_swap_out(unsigned int slot);
int proc_swap_in(unsigned int slot);
int proc_swap_reclaim(kword_t words, kword_t alignment,
    unsigned int exclude_owner);

#endif
