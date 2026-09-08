#include "exec.h"
#include "mach_user.h"
#include "mm.h"
#include "proc.h"
#include "vfs.h"

void
kcore_boot_start(kword_t reclaim_base, kword_t reclaim_words)
{
        struct proc *p;
        static const kword_t init_path[] = {
                12UL,
                VFS_SIX6('/', 'S', 'Y', 'S', 'T', 'E'),
                VFS_SIX6('M', '/', 'I', 'N', 'I', 'T')
        };

        if (mm_add_free(reclaim_base, reclaim_words) != MM_OK)
                return;
        mach_user_trap_init();
        proc_table[0].meta = (kword_t)PROC_SRUN << PROC_STATE_SHIFT;
        p = &proc_table[1];
        if (exec_load_init(p, 1U, init_path) != 0)
                return;
        PROC_SET_STATE(p, PROC_SRUN);
        mach_enter_user(PROC_MEM_BASE(p), PROC_ENTRY(p),
            PROC_MEM_WORDS(p) - (kword_t)EXEC_DXR_STACK_WORDS - 1U,
            0UL, 0UL, 0UL);
}
