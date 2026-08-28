#include "proc_v1.h"
#include "procfs_v1.h"

extern struct proc_v1 proc_v1_table[PROC_V1_NPROC];
kword_t
proc_v1_comm(const struct proc_v1 *p)
{
        return PROC_V1_PID(p) == 0U ?
            VFS_V1_SIX6('S','W','A','P','P','E') :
            VFS_V1_SIX6('I','N','I','T',' ',' ');
}
