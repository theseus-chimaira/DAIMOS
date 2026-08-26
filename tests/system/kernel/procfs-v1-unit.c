#include <assert.h>
#include <stdio.h>
#include "procfs_v1.h"

struct fake_proc {
        int live;
        kword_t pid;
        kword_t ppid;
        kword_t state;
        kword_t words;
        kword_t comm;
};

static struct fake_proc procs[4] = {
        { 1, 1UL, 0UL, 2UL, 64UL, VFS_V1_SIX6('I','N','I','T',' ',' ') },
        { 0, 0UL, 0UL, 0UL, 0UL, 0UL },
        { 1, 42UL, 1UL, 3UL, 17UL, VFS_V1_SIX6('D','S','H',' ',' ',' ') },
        { 0, 0UL, 0UL, 0UL, 0UL, 0UL }
};

static int
fake_get(unsigned int slot, unsigned int field, kword_t *valuep)
{
        struct fake_proc *pp;

        if (slot >= 4U || valuep == 0 || !procs[slot].live)
                return -1;
        pp = &procs[slot];
        switch (field) {
        case PROCFS_V1_FIELD_PID: *valuep = pp->pid; break;
        case PROCFS_V1_FIELD_PPID: *valuep = pp->ppid; break;
        case PROCFS_V1_FIELD_STATE: *valuep = pp->state; break;
        case PROCFS_V1_FIELD_WORDS: *valuep = pp->words; break;
        case PROCFS_V1_FIELD_COMM: *valuep = pp->comm; break;
        default: return -1;
        }
        return 0;
}

int
main(void)
{
        vnode_v1_t root;
        vnode_v1_t proc;
        vnode_v1_t file;
        struct vfs_v1_name name;
        struct vfs_v1_dirent ent;
        struct vfs_v1_stat st;
        unsigned int value;
        kword_t word;

        procfs_v1_init(4U, fake_get);
        root = procfs_v1_root();
        assert(VFS_V1_PROVIDER(root) == PROCFS_V1_PROVIDER);

        assert(procfs_v1_readdir(root, 0U, &ent) == 1);
        assert(vfs_v1_name_get_uint(&ent.name, &value) == 0 && value == 1U);
        assert(ent.type == VFS_V1_TYPE_DIR);
        assert(procfs_v1_readdir(root, 1U, &ent) == 1);
        assert(vfs_v1_name_get_uint(&ent.name, &value) == 0 && value == 42U);
        assert(procfs_v1_readdir(root, 2U, &ent) == 0);

        assert(vfs_v1_name_set_uint(&name, 42U) == 0);
        assert(procfs_v1_lookup(root, &name, &proc) == 0);
        assert(VFS_V1_KIND(proc) == PROCFS_V1_KIND_PROC);
        assert(VFS_V1_INDEX(proc) == 2U);
        assert(procfs_v1_stat(proc, &st) == 0 && st.type == VFS_V1_TYPE_DIR);

        assert(vfs_v1_name_set6(&name,
            VFS_V1_SIX6('W','O','R','D','S',' '), 5U) == 0);
        assert(procfs_v1_lookup(proc, &name, &file) == 0);
        assert(procfs_v1_value(file, &word) == 0 && word == 17UL);
        assert(procfs_v1_stat(file, &st) == 0 && st.type == VFS_V1_TYPE_REG);

        assert(procfs_v1_readdir(proc, 4U, &ent) == 1);
        assert(vfs_v1_name_is6(&ent.name,
            VFS_V1_SIX6('C','O','M','M',' ',' '), 4U));
        assert(procfs_v1_lookup(proc, &ent.name, &file) == 0);
        assert(procfs_v1_value(file, &word) == 0);
        assert(word == VFS_V1_SIX6('D','S','H',' ',' ',' '));

        procs[2].live = 0;
        assert(procfs_v1_stat(proc, &st) != 0);
        assert(procfs_v1_lookup(root, &name, &file) != 0);

        puts("PROCFS v1 unit test PASS");
        return 0;
}
