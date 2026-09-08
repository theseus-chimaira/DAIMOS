#include "exec.h"
#include "mach_user.h"
#include "mm.h"
#include "proc.h"
#include "vfs.h"

void
kcore_boot_start(kword_t reclaim_base, kword_t reclaim_words)
{
        struct proc *p;
        int init_slot;
        unsigned int slot;
        kword_t entry;
        kword_t stack;
        kword_t first_entry;
        kword_t first_stack;
        static const kword_t init_path[] = {
                12UL,
                VFS_SIX6('/', 'S', 'Y', 'S', 'T', 'E'),
                VFS_SIX6('M', '/', 'I', 'N', 'I', 'T')
        };

        if (mm_add_free(reclaim_base, reclaim_words) != MM_OK ||
            proc_boot_init() != 0)
                return;
        mach_user_trap_init();
        proc_table[0].meta = 0UL;
        proc_table[0].mem_layout = 0UL;
        proc_table[0].sched = PROC_SCHED_DEFAULT;
        PROC_SET_STATE(&proc_table[0], PROC_SRUN);

        if (PROC_BOOT_USERS < 1 || PROC_BOOT_USERS >= PROC_MAX_SLOTS)
                return;
        first_entry = 0UL;
        first_stack = 0UL;
        for (slot = 1U; slot <= (unsigned int)PROC_BOOT_USERS; ++slot) {
                init_slot = proc_slot_claim(0U);
                if (init_slot != (int)slot)
                        return;
                p = &proc_table[slot];
                if (exec_load_init(p, slot, init_path) != 0)
                        return;
                PROC_SET_STATE(p, PROC_SRUN);
                entry = PROC_ENTRY(p);
                stack = PROC_MEM_WORDS(p) -
                    (kword_t)EXEC_DXR_STACK_WORDS - 1U;
                if (slot == 1U) {
                        first_entry = entry;
                        first_stack = stack;
                }
                if (proc_user_context_init(slot, entry, stack,
                    (kword_t)slot, 0UL, 0UL) != 0)
                        return;
        }

        p = &proc_table[1];
        proc_current_slot = 1UL;
        proc_sched_cursor = 1UL;
        mach_enter_user(PROC_MEM_BASE(p), first_entry, first_stack,
            1UL, 0UL, 0UL);
}
