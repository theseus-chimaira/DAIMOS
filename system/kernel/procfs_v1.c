#include "procfs_v1.h"
#include "kfmt_v1.h"
#include "proc_v1.h"

static const kword_t procfs_v1_files[] = {
        VFS_V1_SIX6('P','P','I','D',' ',' '),
        VFS_V1_SIX6('S','T','A','T','E',' '),
        VFS_V1_SIX6('W','O','R','D','S',' '),
        VFS_V1_SIX6('C','O','M','M',' ',' ')
};

#define PROCFS_V1_NFILES 4U

static unsigned int
procfs_v1_file_chars(unsigned int i)
{
        return (i == 0U || i == 3U) ? 4U : 5U;
}

extern int procfs_v1_is_root(vnode_v1_t node);
extern struct proc_v1 *procfs_v1_slot_live(unsigned int slot, kword_t *pidp);
extern int procfs_v1_is_proc(vnode_v1_t node, unsigned int *slotp);
extern int procfs_v1_is_file(vnode_v1_t node, unsigned int *slotp,
    unsigned int *fieldp);
extern int procfs_v1_find_pid(unsigned int pid, unsigned int *slotp);

int
procfs_v1_lookup(vnode_v1_t dir, const struct vfs_v1_name *name,
    vnode_v1_t *nodep)
{
        unsigned int slot;
        unsigned int pid;
        unsigned int i;

        if (name == 0 || nodep == 0)
                return -1;
        if (procfs_v1_is_root(dir)) {
                if (vfs_v1_name_get_pid(name, &pid) != 0 ||
                    procfs_v1_find_pid(pid, &slot) != 0)
                        return -1;
                *nodep = VFS_V1_NODE(PROCFS_V1_PROVIDER,
                    PROCFS_V1_KIND_PROC, slot);
                return 0;
        }
        if (!procfs_v1_is_proc(dir, &slot))
                return -1;
        for (i = 0U; i < PROCFS_V1_NFILES; ++i) {
                if (!vfs_v1_name_is6(name, procfs_v1_files[i],
                    procfs_v1_file_chars(i)))
                        continue;
                *nodep = VFS_V1_NODE(PROCFS_V1_PROVIDER,
                    PROCFS_V1_KIND_PPID + i, slot);
                return 0;
        }
        return -1;
}

int
procfs_v1_readdir(vnode_v1_t dir, unsigned int off,
    struct vfs_v1_dirent *ent)
{
        unsigned int slot;
        unsigned int visible;
        unsigned int proc_slot;
        kword_t pid;

        if (ent == 0)
                return -1;
        if (procfs_v1_is_root(dir)) {
                visible = 0U;
                for (slot = 0U; slot < PROC_V1_NPROC; ++slot) {
                        if (!procfs_v1_slot_live(slot, &pid))
                                continue;
                        if (visible++ != off)
                                continue;
                        if (vfs_v1_name_set_pid(&ent->name,
                            (unsigned int)pid) != 0)
                                return -1;
                        ent->type = VFS_V1_TYPE_DIR;
                        return 1;
                }
                return 0;
        }
        if (!procfs_v1_is_proc(dir, &proc_slot))
                return -1;
        (void)proc_slot;
        if (off >= PROCFS_V1_NFILES)
                return 0;
        if (vfs_v1_name_set6(&ent->name, procfs_v1_files[off],
            procfs_v1_file_chars(off)) != 0)
                return -1;
        ent->type = VFS_V1_TYPE_REG;
        return 1;
}

int
procfs_v1_stat(vnode_v1_t node, struct vfs_v1_stat *st)
{
        unsigned int slot;
        unsigned int field;

        if (st == 0)
                return -1;
        if (procfs_v1_is_root(node) || procfs_v1_is_proc(node, &slot)) {
                st->type = VFS_V1_TYPE_DIR;
                st->mode = 0555U;
        } else if (procfs_v1_is_file(node, &slot, &field)) {
                (void)field;
                st->type = VFS_V1_TYPE_REG;
                st->mode = 0444U;
        } else {
                return -1;
        }
        st->size_chars = 0;
        st->size_words = 0;
        return 0;
}

int
procfs_v1_readchar(vnode_v1_t node, kword_t off, unsigned int *chp)
{
        static const kword_t state_names[] = {
                VFS_V1_SIX6('F','R','E','E',' ',' '),
                VFS_V1_SIX6('I','D','L',' ',' ',' '),
                VFS_V1_SIX6('R','U','N',' ',' ',' '),
                VFS_V1_SIX6('S','L','E','E','P',' '),
                VFS_V1_SIX6('Z','O','M','B',' ',' ')
        };
        struct proc_v1 *p;
        kword_t value;
        unsigned int field;
        unsigned int slot;

        if (chp == 0 || !procfs_v1_is_file(node, &slot, &field) ||
            (p = proc_v1_get(slot)) == 0)
                return -1;
        if (field == PROCFS_V1_FIELD_COMM)
                return vfs_v1_sixbit_readchar(proc_v1_comm(p), 6U, off, chp);
        if (field == PROCFS_V1_FIELD_PPID)
                value = (kword_t)proc_v1_ppid(p);
        else if (field == PROCFS_V1_FIELD_STATE)
                value = (kword_t)PROC_V1_STATE(p);
        else
                value = PROC_V1_MEM_WORDS(p);
        if (field == PROCFS_V1_FIELD_STATE) {
                if (value >= (kword_t)(sizeof(state_names) / sizeof(state_names[0])))
                        return -1;
                return vfs_v1_sixbit_readchar(state_names[(unsigned int)value],
                    value == 3U ? 5U : (value == 1U || value == 2U ? 3U : 4U),
                    off, chp);
        }
        return kfmt_u36_decimal_readchar(value, off, chp);
}
