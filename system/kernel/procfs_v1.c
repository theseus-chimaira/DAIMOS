#include "procfs_v1.h"
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

vnode_v1_t
procfs_v1_root(void)
{
        return VFS_V1_NODE(PROCFS_V1_PROVIDER, PROCFS_V1_KIND_ROOT, 0U);
}

static int
procfs_v1_is_root(vnode_v1_t node)
{
        return VFS_V1_PROVIDER(node) == PROCFS_V1_PROVIDER &&
            VFS_V1_KIND(node) == PROCFS_V1_KIND_ROOT;
}

static int
procfs_v1_slot_live(unsigned int slot, kword_t *pidp)
{
        kword_t pid;

        if (slot >= PROC_V1_NPROC ||
            proc_v1_procfs_get(slot, PROCFS_V1_FIELD_PID, &pid) != 0)
                return 0;
        if (pidp != 0)
                *pidp = pid;
        return 1;
}

static int
procfs_v1_is_proc(vnode_v1_t node, unsigned int *slotp)
{
        unsigned int slot;

        if (VFS_V1_PROVIDER(node) != PROCFS_V1_PROVIDER ||
            VFS_V1_KIND(node) != PROCFS_V1_KIND_PROC)
                return 0;
        slot = VFS_V1_INDEX(node);
        if (!procfs_v1_slot_live(slot, 0))
                return 0;
        if (slotp != 0)
                *slotp = slot;
        return 1;
}

static int
procfs_v1_is_file(vnode_v1_t node, unsigned int *slotp,
    unsigned int *fieldp)
{
        unsigned int kind;
        unsigned int slot;
        unsigned int field;

        if (VFS_V1_PROVIDER(node) != PROCFS_V1_PROVIDER)
                return 0;
        kind = VFS_V1_KIND(node);
        if (kind < PROCFS_V1_KIND_PPID || kind > PROCFS_V1_KIND_COMM)
                return 0;
        field = kind - 1U;
        slot = VFS_V1_INDEX(node);
        if (!procfs_v1_slot_live(slot, 0))
                return 0;
        if (slotp != 0)
                *slotp = slot;
        if (fieldp != 0)
                *fieldp = field;
        return 1;
}

static int
procfs_v1_find_pid(unsigned int pid, unsigned int *slotp)
{
        unsigned int slot;
        kword_t value;

        if (slotp == 0)
                return -1;
        for (slot = 0U; slot < PROC_V1_NPROC; ++slot) {
                if (!procfs_v1_slot_live(slot, &value))
                        continue;
                if (value == (kword_t)pid) {
                        *slotp = slot;
                        return 0;
                }
        }
        return -1;
}

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
procfs_v1_pid(unsigned int slot, kword_t *pidp)
{
        if (slot >= PROC_V1_NPROC)
                return -1;
        return proc_v1_procfs_get(slot, PROCFS_V1_FIELD_PID, pidp);
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
        kword_t value;
        unsigned int field;
        unsigned int slot;

        if (chp == 0 || !procfs_v1_is_file(node, &slot, &field) ||
            proc_v1_procfs_get(slot, field, &value) != 0)
                return -1;
        if (field == PROCFS_V1_FIELD_COMM)
                return vfs_v1_sixbit_readchar(value, 6U, off, chp);
        if (field == PROCFS_V1_FIELD_STATE) {
                if (value >= (kword_t)(sizeof(state_names) / sizeof(state_names[0])))
                        return -1;
                return vfs_v1_sixbit_readchar(state_names[(unsigned int)value],
                    value == 3U ? 5U : (value == 1U || value == 2U ? 3U : 4U),
                    off, chp);
        }
        return vfs_v1_decimal_readchar(value, off, chp);
}
