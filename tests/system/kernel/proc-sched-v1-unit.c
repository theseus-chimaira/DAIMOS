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
        struct proc_v1 *init;
        struct proc_v1 *child;
        struct proc_v1 *child2;
        struct vfs_v1_dirent ent;
        kword_t value;
        unsigned int pid;

        init = setup_processes();
        assert(proc_v1_current == &proc_v1_table[0]);
        assert(PROC_V1_STATE(proc_v1_current) == PROC_V1_SRUN);

        assert(init == &proc_v1_table[1]);
        assert(PROC_V1_PID(init) == 1U);
        assert(PROC_V1_STATE(init) == PROC_V1_SIDL);

        child = proc_v1_alloc_child(init);
        child2 = proc_v1_alloc_child(init);
        assert(child != 0 && child2 != 0);
        assert(PROC_V1_PID(child) == 2U);
        assert(PROC_V1_PID(child2) == 3U);
        assert(proc_v1_ppid(child) == 1U);

        proc_v1_set_memory(child, 01000UL, 03200UL);
        proc_v1_set_entry(child, 01020UL);
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
        assert(proc_v1_get(proc_v1_slot(child)) == child);
        value = PROC_V1_MEM_WORDS(child);
        assert(value == 03200UL);

        proc_v1_reap(child);
        assert(PROC_V1_STATE(child) == PROC_V1_FREE);
        assert(proc_v1_ppid(child2) == 1U);
        proc_v1_reap(init);
        assert(proc_v1_ppid(child2) == 0U);

        puts("PROC/SCHED v1 unit test PASS");
        return 0;
}
