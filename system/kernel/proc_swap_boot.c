#include "proc_swap.h"
#include "fs_mres.h"
#include "mm.h"

#define PROC_SWAP_MM_OWNER 4U

int
proc_swap_boot_init(unsigned int slots)
{
        kword_t base;
        kword_t words;
        kword_t *wp;

        if (proc_swap_records != 0 || slots == 0U || slots > PROC_MAX_SLOTS)
                return -1;
        words = (kword_t)slots * (kword_t)PROC_SWAP_RECORD_WORDS;
        if (mm_alloc(words, MM_TYPE_KERNEL_DYNAMIC, PROC_SWAP_MM_OWNER,
            MM_ALLOC_LOW, &base) != MM_OK)
                return -1;
        wp = (kword_t *)(unsigned long)base;
        fs_zero_words(wp, (unsigned int)words);
        proc_swap_records = (struct proc_swap_record *)(unsigned long)base;
        return 0;
}
