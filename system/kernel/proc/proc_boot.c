#include "proc.h"
#include "fs_mres.h"
#include "mm.h"
#include "proc_swap.h"

#define PROC_TABLE_MM_OWNER 3U

extern unsigned int proc_sched_age_phase;

unsigned int
proc_slots_for_core(kword_t core_words)
{
        if (core_words <= 0100000UL)
                return 24U;
        if (core_words <= 0200000UL)
                return 40U;
        if (core_words <= 0300000UL)
                return 64U;
        if (core_words <= 0400000UL)
                return 88U;
        if (core_words <= 0600000UL)
                return 128U;
        if (core_words <= 01000000UL)
                return 192U;
        return PROC_MAX_SLOTS;
}

int
proc_boot_init(void)
{
        kword_t base;
        kword_t words;
        kword_t *wp;
        unsigned int slots;

        if (proc_table != 0 || proc_slots != 0U)
                return -1;
        slots = proc_slots_for_core(mm_core_words);
        words = (kword_t)slots * (kword_t)PROC_WORDS;
        if (mm_alloc(words, MM_TYPE_KERNEL_DYNAMIC, PROC_TABLE_MM_OWNER,
            MM_ALLOC_LOW, &base) != MM_OK)
                return -1;
        wp = (kword_t *)(unsigned long)base;
        fs_zero_words(wp, (unsigned int)words);
        proc_table = (struct proc *)(unsigned long)base;
        proc_slots = slots;
        proc_high_slot = 1U;
        proc_sched_age_phase = 0U;
        proc_runq_head = 0UL;
        proc_rt_owner = 0UL;
        if (proc_swap_boot_init(slots) != 0) {
                proc_table = 0;
                proc_slots = 0U;
                proc_high_slot = 0U;
                (void)mm_free(base, MM_TYPE_KERNEL_DYNAMIC,
                    PROC_TABLE_MM_OWNER);
                return -1;
        }
        return 0;
}
