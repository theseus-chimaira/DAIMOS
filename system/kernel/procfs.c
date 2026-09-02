#include "procfs.h"
#include "kfmt.h"
#include "proc.h"

extern int procfs_is_file(vnode_t node, unsigned int *slotp,
    unsigned int *fieldp);

int
procfs_readchar(vnode_t node, kword_t off, unsigned int *chp)
{
        struct proc *p;
        kword_t value;
        unsigned int field;
        unsigned int slot;

        if (chp == 0 || !procfs_is_file(node, &slot, &field))
                return -1;
        p = &proc_table[slot];
        if (field == PROCFS_FIELD_COMM)
                return vfs_sixbit_readchar(proc_comm(p), 6U, off, chp);
        if (field == PROCFS_FIELD_STATE)
                return vfs_sixbit_readchar(
                    VFS_SIX6('R','U','N',' ',' ',' '), 3U, off, chp);
        value = field == PROCFS_FIELD_PPID ? 0 : PROC_MEM_WORDS(p);
        return kfmt_u36_decimal_readchar(value, off, chp);
}
