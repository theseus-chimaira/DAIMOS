#include "sched_v1.h"
#include "exec_v1.h"

int
sched_v1_run_once(sched_v1_enter_fn enterfn)
{
        struct proc_v1 *p;

        p = proc_v1_current;
        if (p == 0 || PROC_V1_STATE(p) != PROC_V1_SRUN ||
            PROC_V1_MEM_BASE(p) == 0 || enterfn == 0)
                return SCHED_V1_NO_RUNNABLE;
        enterfn(PROC_V1_MEM_BASE(p), PROC_V1_ENTRY(p),
            PROC_V1_MEM_WORDS(p) - (kword_t)EXEC_V1_DXR_STACK_WORDS - 1U,
            0, 0, 0);
        return SCHED_V1_ENTERED;
}

void
sched_v1_yield(void)
{
        unsigned int start;
        unsigned int i;
        unsigned int slot;

        start = proc_v1_slot(proc_v1_current);
        for (i = 1U; i <= PROC_V1_NPROC; ++i) {
                slot = (start + i) % PROC_V1_NPROC;
                if (PROC_V1_STATE(&proc_v1_table[slot]) == PROC_V1_SRUN) {
                        proc_v1_current = &proc_v1_table[slot];
                        return;
                }
        }
}
