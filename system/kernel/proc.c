#include "proc.h"
#include "procfs.h"

extern struct proc proc_table[PROC_NPROC];
kword_t
proc_comm(const struct proc *p)
{
        return PROC_PID(p) == 0U ?
            VFS_SIX6('S','W','A','P','P','E') :
            VFS_SIX6('I','N','I','T',' ',' ');
}
