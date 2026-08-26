#include "procfs_v1.h"

struct procfs_v1_desc {
        kword_t name6;
        kword_t meta;
};

#define PROCFS_V1_META(chars, field) \
        (((kword_t)(chars) & 077UL) | (((kword_t)(field) & 077UL) << 6))
#define PROCFS_V1_META_CHARS(meta) \
        ((unsigned int)((meta) & 077UL))
#define PROCFS_V1_META_FIELD(meta) \
        ((unsigned int)(((meta) >> 6) & 077UL))

static unsigned int procfs_v1_slots;
static procfs_v1_get_fn procfs_v1_get;

static const struct procfs_v1_desc procfs_v1_files[] = {
        { VFS_V1_SIX6('P','I','D',' ',' ',' '), PROCFS_V1_META(3U, PROCFS_V1_FIELD_PID) },
        { VFS_V1_SIX6('P','P','I','D',' ',' '), PROCFS_V1_META(4U, PROCFS_V1_FIELD_PPID) },
        { VFS_V1_SIX6('S','T','A','T','E',' '), PROCFS_V1_META(5U, PROCFS_V1_FIELD_STATE) },
        { VFS_V1_SIX6('W','O','R','D','S',' '), PROCFS_V1_META(5U, PROCFS_V1_FIELD_WORDS) },
        { VFS_V1_SIX6('C','O','M','M',' ',' '), PROCFS_V1_META(4U, PROCFS_V1_FIELD_COMM) }
};

#define PROCFS_V1_NFILES \
        ((unsigned int)(sizeof(procfs_v1_files) / sizeof(procfs_v1_files[0])))

void
procfs_v1_init(unsigned int slots, procfs_v1_get_fn getfn)
{
        procfs_v1_slots = slots;
        procfs_v1_get = getfn;
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

        if (procfs_v1_get == 0 || slot >= procfs_v1_slots ||
            procfs_v1_get(slot, PROCFS_V1_FIELD_PID, &pid) != 0)
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
        switch (kind) {
        case PROCFS_V1_KIND_PID: field = PROCFS_V1_FIELD_PID; break;
        case PROCFS_V1_KIND_PPID: field = PROCFS_V1_FIELD_PPID; break;
        case PROCFS_V1_KIND_STATE: field = PROCFS_V1_FIELD_STATE; break;
        case PROCFS_V1_KIND_WORDS: field = PROCFS_V1_FIELD_WORDS; break;
        case PROCFS_V1_KIND_COMM: field = PROCFS_V1_FIELD_COMM; break;
        default: return 0;
        }
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
        for (slot = 0U; slot < procfs_v1_slots; ++slot) {
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
        const struct procfs_v1_desc *dp;

        if (name == 0 || nodep == 0)
                return -1;
        if (procfs_v1_is_root(dir)) {
                if (vfs_v1_name_get_uint(name, &pid) != 0 ||
                    procfs_v1_find_pid(pid, &slot) != 0)
                        return -1;
                *nodep = VFS_V1_NODE(PROCFS_V1_PROVIDER,
                    PROCFS_V1_KIND_PROC, slot);
                return 0;
        }
        if (!procfs_v1_is_proc(dir, &slot))
                return -1;
        for (i = 0U; i < PROCFS_V1_NFILES; ++i) {
                dp = &procfs_v1_files[i];
                if (!vfs_v1_name_is6(name, dp->name6,
                    PROCFS_V1_META_CHARS(dp->meta)))
                        continue;
                *nodep = VFS_V1_NODE(PROCFS_V1_PROVIDER,
                    PROCFS_V1_KIND_PID + i, slot);
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
        const struct procfs_v1_desc *dp;

        if (ent == 0)
                return -1;
        if (procfs_v1_is_root(dir)) {
                visible = 0U;
                for (slot = 0U; slot < procfs_v1_slots; ++slot) {
                        if (!procfs_v1_slot_live(slot, &pid))
                                continue;
                        if (visible++ != off)
                                continue;
                        if (vfs_v1_name_set_uint(&ent->name,
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
        dp = &procfs_v1_files[off];
        if (vfs_v1_name_set6(&ent->name, dp->name6,
            PROCFS_V1_META_CHARS(dp->meta)) != 0)
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
procfs_v1_value(vnode_v1_t node, kword_t *valuep)
{
        unsigned int slot;
        unsigned int field;

        if (valuep == 0 || procfs_v1_get == 0 ||
            !procfs_v1_is_file(node, &slot, &field))
                return -1;
        return procfs_v1_get(slot, field, valuep);
}
