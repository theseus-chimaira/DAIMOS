#include <assert.h>
#include <stdio.h>
#include "procfs_v1.h"
#include "proc_v1.h"


static struct proc_v1 *
setup_processes(void)
{
        unsigned int i;

        for (i = 0U; i < PROC_V1_NPROC; ++i) {
                proc_v1_table[i].meta = 0;
                proc_v1_table[i].mem_layout = 0;
        }
        proc_v1_table[0].meta = (kword_t)PROC_V1_SRUN << PROC_V1_STATE_SHIFT;
        proc_v1_table[1].meta = 1U | ((kword_t)PROC_V1_SIDL << PROC_V1_STATE_SHIFT);
        proc_v1_current = &proc_v1_table[0];
        proc_v1_next_pid = 2U;
        return &proc_v1_table[1];
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
        struct proc_v1 *initp;
        struct proc_v1 *child;
        unsigned int value;
        unsigned int ch;

        initp = setup_processes();
        proc_v1_set_state(initp, PROC_V1_SRUN);
        child = proc_v1_alloc_child(initp);
        assert(child != 0);
        proc_v1_set_memory(child, 01000UL, 17UL);
        proc_v1_set_state(child, PROC_V1_SLEEP);

        root = procfs_v1_root();
        assert(VFS_V1_PROVIDER(root) == PROCFS_V1_PROVIDER);

        assert(procfs_v1_readdir(root, 0U, &ent) == 1);
        assert(vfs_v1_name_get_pid(&ent.name, &value) == 0 && value == 0U);
        assert(ent.type == VFS_V1_TYPE_DIR);
        assert(procfs_v1_readdir(root, 1U, &ent) == 1);
        assert(vfs_v1_name_get_pid(&ent.name, &value) == 0 && value == 1U);
        assert(procfs_v1_readdir(root, 2U, &ent) == 1);
        assert(vfs_v1_name_get_pid(&ent.name, &value) == 0 && value == 2U);
        assert(procfs_v1_readdir(root, 3U, &ent) == 0);

        assert(vfs_v1_name_set_pid(&name, 2U) == 0);
        assert(procfs_v1_lookup(root, &name, &proc) == 0);
        assert(VFS_V1_KIND(proc) == PROCFS_V1_KIND_PROC);
        assert(VFS_V1_INDEX(proc) == 2U);
        assert(procfs_v1_stat(proc, &st) == 0 && st.type == VFS_V1_TYPE_DIR);

        assert(vfs_v1_name_set6(&name,
            VFS_V1_SIX6('W','O','R','D','S',' '), 5U) == 0);
        assert(procfs_v1_lookup(proc, &name, &file) == 0);
        assert(procfs_v1_stat(file, &st) == 0 && st.type == VFS_V1_TYPE_REG);
        assert(procfs_v1_readchar(file, 0U, &ch) == 1 && ch == '1');
        assert(procfs_v1_readchar(file, 1U, &ch) == 1 && ch == '7');
        assert(procfs_v1_readchar(file, 2U, &ch) == 1 && ch == '\r');
        assert(procfs_v1_readchar(file, 3U, &ch) == 1 && ch == '\n');
        assert(procfs_v1_readchar(file, 4U, &ch) == 0);

        assert(vfs_v1_name_set6(&name,
            VFS_V1_SIX6('S','T','A','T','E',' '), 5U) == 0);
        assert(procfs_v1_lookup(proc, &name, &file) == 0);
        assert(procfs_v1_readchar(file, 0U, &ch) == 1 && ch == 'S');
        assert(procfs_v1_readchar(file, 4U, &ch) == 1 && ch == 'P');
        assert(procfs_v1_readchar(file, 5U, &ch) == 1 && ch == '\r');

        assert(procfs_v1_readdir(proc, 3U, &ent) == 1);
        assert(vfs_v1_name_is6(&ent.name,
            VFS_V1_SIX6('C','O','M','M',' ',' '), 4U));
        assert(procfs_v1_lookup(proc, &ent.name, &file) == 0);

        assert(vfs_v1_name_set6(&name,
            VFS_V1_SIX6('P','I','D',' ',' ',' '), 3U) == 0);
        assert(procfs_v1_lookup(proc, &name, &file) != 0);
        assert(procfs_v1_readdir(proc, 4U, &ent) == 0);

        proc_v1_reap(child);
        assert(procfs_v1_stat(proc, &st) != 0);
        assert(procfs_v1_lookup(root, &name, &file) != 0);

        puts("PROCFS v1 unit test PASS");
        return 0;
}
