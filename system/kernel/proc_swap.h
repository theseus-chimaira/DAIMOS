#ifndef DAIMON_PROC_SWAP_H
#define DAIMON_PROC_SWAP_H

#include "proc.h"
#include "vfs.h"

#define PROC_SWAP_RECORD_WORDS 3U

int proc_swap_boot_init(unsigned int slots);

void proc_swap_attach(unsigned int slot, vnode_t backing,
    kword_t image_words, kword_t text_words, unsigned int pure);
void proc_swap_detach(unsigned int slot);
int proc_swap_out(unsigned int slot);
int proc_swap_in(unsigned int slot);
int proc_swap_reclaim(kword_t words, kword_t alignment,
    unsigned int exclude_owner);
kword_t proc_swap_used_blocks(void);
kword_t proc_swap_free_blocks(void);
extern kword_t proc_swap_words_read;
extern kword_t proc_swap_words_written;

#endif
