#include "proc.h"
#include "fs_mres.h"
#include "mm.h"

#define PROC_USER_FLAG_BITS ((kword_t)010000UL << 18U)
#define PROC_CTX_U_PC       020U
#define PROC_CTX_U_KSP      021U
#define PROC_CTX_STACK      PROC_USTACK_BASE
#define PROC_UAREA_MM_OWNER_BASE 01000U

int
proc_slot_claim(unsigned int parent_slot)
{
        unsigned int slot;
        struct proc *p;

        if (proc_table == 0 || parent_slot >= proc_slots)
                return -1;
        for (slot = 1U; slot < proc_slots; ++slot) {
                p = &proc_table[slot];
                if (PROC_STATE(p) != PROC_FREE)
                        continue;
                p->meta = ((kword_t)slot & PROC_PID_MASK) |
                    (((kword_t)parent_slot & PROC_PARENT_MASK) <<
                    PROC_PARENT_SHIFT);
                p->mem_layout = 0UL;
                p->sched = PROC_SCHED_DEFAULT;
                PROC_SET_STATE(p, PROC_SIDL);
                if (slot + 1U > proc_high_slot)
                        proc_high_slot = slot + 1U;
                return (int)slot;
        }
        return -1;
}

int
proc_user_context_init(unsigned int slot, kword_t entry, kword_t stack,
    kword_t ac1, kword_t ac2, kword_t ac3)
{
        struct proc *p;
        kword_t base;
        kword_t *ctx;

        if (proc_table == 0 || slot == 0U || slot >= proc_slots)
                return -1;
        p = &proc_table[slot];
        if (PROC_MEM_BASE(p) == 0UL || PROC_HAS_UAREA(p))
                return -1;
        if (mm_alloc(PROC_UAREA_WORDS, MM_TYPE_KERNEL_DYNAMIC,
            PROC_UAREA_MM_OWNER_BASE + slot, MM_ALLOC_LOW, &base) != MM_OK)
                return -1;
        ctx = (kword_t *)(unsigned long)base;
        fs_zero_words(ctx, (unsigned int)PROC_UAREA_WORDS);
        ctx[1] = ac1;
        ctx[2] = ac2;
        ctx[3] = ac3;
        ctx[017] = stack;
        ctx[PROC_CTX_U_PC] = PROC_USER_FLAG_BITS | (entry & PROC_HALF_MASK);
        ctx[PROC_CTX_U_KSP] = base + PROC_CTX_STACK;
        PROC_SET_META_LH(p, base);
        p->meta |= (kword_t)PROC_F_UAREA << PROC_FLAGS_SHIFT;
        return 0;
}
