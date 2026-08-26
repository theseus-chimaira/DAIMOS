#include <assert.h>
#include <stdio.h>
#include "proc_v1.h"
#include "procfs_v1.h"
#include "sched_v1.h"

static kword_t entered_base;
static kword_t entered_entry;
static kword_t entered_stack;

static void
fake_enter(kword_t base, kword_t entry, kword_t stack, kword_t ac1, kword_t ac2, kword_t ac3)
{
        assert(ac1 == 0 && ac2 == 0 && ac3 == 0);
        entered_base = base;
        entered_entry = entry;
        entered_stack = stack;
}

int
main(void)
{
        struct proc_v1 *init;
        struct proc_v1 *child;
        struct proc_v1 *child2;
        struct vfs_v1_dirent ent;
        kword_t value;
        unsigned int pid;

        proc_v1_init();
        assert(proc_v1_current == &proc_v1_table[0]);
        assert(PROC_V1_STATE(proc_v1_current) == PROC_V1_SRUN);

        init = proc_v1_alloc_init();
        assert(init == &proc_v1_table[1]);
        assert(PROC_V1_PID(init) == 1U);
        assert(PROC_V1_STATE(init) == PROC_V1_SIDL);
        proc_v1_set_cred(init, 7U, 11U);
        assert(PROC_V1_UID(init) == 7U);
        assert(PROC_V1_GID(init) == 11U);

        child = proc_v1_alloc_child(init);
        child2 = proc_v1_alloc_child(init);
        assert(child != 0 && child2 != 0);
        assert(PROC_V1_PID(child) == 2U);
        assert(PROC_V1_PID(child2) == 3U);
        assert(proc_v1_ppid(child) == 1U);
        assert(PROC_V1_UID(child) == 7U);
        assert(PROC_V1_GID(child) == 11U);

        proc_v1_set_memory(child, 01000UL, 0200UL);
        proc_v1_set_exec(child, 01020UL, 01177UL);
        proc_v1_set_state(child, PROC_V1_SRUN);
        proc_v1_current = child;
        assert(sched_v1_run_once(fake_enter) == SCHED_V1_ENTERED);
        assert(entered_base == 01000UL);
        assert(entered_entry == 01020UL);
        assert(entered_stack == 01177UL);

        proc_v1_set_state(child2, PROC_V1_SRUN);
        sched_v1_yield();
        assert(proc_v1_current == child2);

        assert(procfs_v1_readdir(procfs_v1_root(), 0U, &ent) == 1);
        assert(vfs_v1_name_get_pid(&ent.name, &pid) == 0);
        assert(pid == 0U);
        assert(proc_v1_procfs_get(proc_v1_slot(child), PROCFS_V1_FIELD_WORDS,
            &value) == 0 && value == 0200UL);

        proc_v1_reap(child);
        assert(PROC_V1_STATE(child) == PROC_V1_FREE);
        assert(proc_v1_ppid(child2) == 1U);
        proc_v1_reap(init);
        assert(proc_v1_ppid(child2) == 0U);

        puts("PROC/SCHED v1 unit test PASS");
        return 0;
}
