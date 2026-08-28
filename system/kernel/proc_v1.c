#include "proc_v1.h"
#include "procfs_v1.h"

extern struct proc_v1 proc_v1_table[PROC_V1_NPROC];
struct proc_v1 *proc_v1_current;

unsigned int
proc_v1_ppid(const struct proc_v1 *p)
{
        struct proc_v1 *parent;
        unsigned int slot;

        slot = PROC_V1_PARENT_SLOT(p);
        if (slot == 0U || (parent = proc_v1_get(slot)) == 0)
                return 0U;
        return PROC_V1_PID(parent);
}

kword_t
proc_v1_comm(const struct proc_v1 *p)
{
        if (PROC_V1_PID(p) == 0U)
                return VFS_V1_SIX6('S','W','A','P','P','E');
        if (PROC_V1_PID(p) == 1U)
                return VFS_V1_SIX6('I','N','I','T',' ',' ');
        return VFS_V1_SIX6('U','S','E','R',' ',' ');
}
