#include "proc_v1.h"
#include "procfs_v1.h"

struct proc_v1 proc_v1_table[PROC_V1_NPROC];
struct proc_v1 *proc_v1_current;
unsigned int proc_v1_next_pid;

static void
proc_v1_zero(struct proc_v1 *p)
{
        p->meta = 0;
        p->mem_layout = 0;
}

unsigned int
proc_v1_slot(const struct proc_v1 *p)
{
        return (unsigned int)(p - proc_v1_table);
}

struct proc_v1 *
proc_v1_get(unsigned int slot)
{
        struct proc_v1 *p;

        if (slot >= PROC_V1_NPROC)
                return 0;
        p = &proc_v1_table[slot];
        return PROC_V1_STATE(p) == PROC_V1_FREE ? 0 : p;
}

void
proc_v1_set_state(struct proc_v1 *p, unsigned int state)
{
        p->meta = (p->meta & ~((kword_t)PROC_V1_STATE_MASK << PROC_V1_STATE_SHIFT)) |
            (((kword_t)state & PROC_V1_STATE_MASK) << PROC_V1_STATE_SHIFT);
}

void
proc_v1_set_memory(struct proc_v1 *p, kword_t base, kword_t words)
{
        p->mem_layout = (base & PROC_V1_HALF_MASK) |
            ((words & PROC_V1_HALF_MASK) << PROC_V1_HALF_SHIFT);
}

void
proc_v1_set_entry(struct proc_v1 *p, kword_t entry)
{
        p->meta = (p->meta & PROC_V1_HALF_MASK) |
            ((entry & PROC_V1_HALF_MASK) << PROC_V1_ENTRY_SHIFT);
}

unsigned int
proc_v1_ppid(const struct proc_v1 *p)
{
        struct proc_v1 *parent;
        unsigned int slot;

        slot = PROC_V1_PARENT_SLOT(p);
        if (slot == 0U || (parent = proc_v1_get(slot)) == 0)
                return 0U;
        return PROC_V1_PID(parent);
}

kword_t
proc_v1_comm(const struct proc_v1 *p)
{
        if (PROC_V1_PID(p) == 0U)
                return VFS_V1_SIX6('S','W','A','P','P','E');
        if (PROC_V1_PID(p) == 1U)
                return VFS_V1_SIX6('I','N','I','T',' ',' ');
        return VFS_V1_SIX6('U','S','E','R',' ',' ');
}

static int
proc_v1_pid_used(unsigned int pid)
{
        unsigned int i;

        for (i = 1U; i < PROC_V1_NPROC; ++i) {
                if (PROC_V1_STATE(&proc_v1_table[i]) != PROC_V1_FREE &&
                    PROC_V1_PID(&proc_v1_table[i]) == pid)
                        return 1;
        }
        return 0;
}

static unsigned int
proc_v1_new_pid(void)
{
        unsigned int candidate;
        unsigned int tries;

        tries = 254U;
        while (tries-- != 0U) {
                candidate = proc_v1_next_pid++;
                if (proc_v1_next_pid > 0377U)
                        proc_v1_next_pid = 2U;
                if (!proc_v1_pid_used(candidate))
                        return candidate;
        }
        return 0U;
}


struct proc_v1 *
proc_v1_alloc_child(struct proc_v1 *parent)
{
        unsigned int i;
        unsigned int pid;
        unsigned int parent_slot;
        struct proc_v1 *p;

        if (parent == 0)
                return 0;
        parent_slot = proc_v1_slot(parent);
        pid = proc_v1_new_pid();
        if (pid == 0U)
                return 0;
        for (i = 1U; i < PROC_V1_NPROC; ++i) {
                p = &proc_v1_table[i];
                if (PROC_V1_STATE(p) != PROC_V1_FREE)
                        continue;
                proc_v1_zero(p);
                p->meta = ((kword_t)pid & PROC_V1_PID_MASK) |
                    ((kword_t)parent_slot << PROC_V1_PARENT_SHIFT) |
                    ((kword_t)PROC_V1_SIDL << PROC_V1_STATE_SHIFT);
                return p;
        }
        return 0;
}

void
proc_v1_reap(struct proc_v1 *p)
{
        unsigned int slot;
        unsigned int i;

        if (p == 0)
                return;
        slot = proc_v1_slot(p);
        if (slot == 0U)
                return;
        for (i = 1U; i < PROC_V1_NPROC; ++i) {
                if (PROC_V1_STATE(&proc_v1_table[i]) != PROC_V1_FREE &&
                    PROC_V1_PARENT_SLOT(&proc_v1_table[i]) == slot)
                        proc_v1_table[i].meta &=
                            ~((kword_t)PROC_V1_PARENT_MASK << PROC_V1_PARENT_SHIFT);
        }
        proc_v1_zero(p);
        if (proc_v1_current == p)
                proc_v1_current = &proc_v1_table[0];
}
