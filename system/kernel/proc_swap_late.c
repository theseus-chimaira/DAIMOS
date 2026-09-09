#include "proc_swap.h"
#include "mm.h"

void
proc_swap_attach(unsigned int slot, vnode_t backing, kword_t image_words,
    kword_t text_words, unsigned int pure)
{
        struct proc_swap_record *r;

        if (proc_swap_records == 0 || slot >= proc_slots)
                return;
        r = &proc_swap_records[slot];
        r->backing = backing;
        r->image_span = ((text_words & PROC_SWAP_TEXT_MASK) << 18U) |
            (image_words & MM_HALF_MASK);
        if (pure != 0U)
                r->image_span |= PROC_SWAP_PURE_BIT;
        r->disk_span = 0UL;
}
