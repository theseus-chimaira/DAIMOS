#include "proc_v1.h"
#include "procfs_v1.h"

struct proc_v1 proc_v1_table[PROC_V1_NPROC];
struct proc_v1 *proc_v1_current;
static unsigned int proc_v1_next_pid = 2U;

static void
proc_v1_zero(struct proc_v1 *p)
{
        p->meta = 0;
        p->mem_layout = 0;
        p->exec = 0;
}

unsigned int
proc_v1_slot(const struct proc_v1 *p)
{
        if (p == 0 || p < proc_v1_table || p >= proc_v1_table + PROC_V1_NPROC)
                return PROC_V1_NPROC;
        return (unsigned int)(p - proc_v1_table);
}

void
proc_v1_set_state(struct proc_v1 *p, unsigned int state)
{
        if (p == 0)
                return;
        p->meta = (p->meta & ~((kword_t)PROC_V1_STATE_MASK << PROC_V1_STATE_SHIFT)) |
            (((kword_t)state & PROC_V1_STATE_MASK) << PROC_V1_STATE_SHIFT);
}

void
proc_v1_set_cred(struct proc_v1 *p, unsigned int uid, unsigned int gid)
{
        if (p == 0)
                return;
        p->meta = (p->meta & ((((kword_t)1U << PROC_V1_UID_SHIFT) - 1U))) |
            (((kword_t)uid & PROC_V1_CRED_MASK) << PROC_V1_UID_SHIFT) |
            (((kword_t)gid & PROC_V1_CRED_MASK) << PROC_V1_GID_SHIFT);
}

void
proc_v1_set_memory(struct proc_v1 *p, kword_t base, kword_t words)
{
        if (p == 0)
                return;
        p->mem_layout = (base & PROC_V1_HALF_MASK) |
            ((words & PROC_V1_HALF_MASK) << PROC_V1_HALF_SHIFT);
}

void
proc_v1_set_exec(struct proc_v1 *p, kword_t entry, kword_t stack)
{
        if (p == 0)
                return;
        p->exec = (entry & PROC_V1_HALF_MASK) |
            ((stack & PROC_V1_HALF_MASK) << PROC_V1_HALF_SHIFT);
}

unsigned int
proc_v1_ppid(const struct proc_v1 *p)
{
        unsigned int slot;

        if (p == 0)
                return 0U;
        slot = PROC_V1_PARENT_SLOT(p);
        if (slot == 0U || slot >= PROC_V1_NPROC ||
            PROC_V1_STATE(&proc_v1_table[slot]) == PROC_V1_FREE)
                return 0U;
        return PROC_V1_PID(&proc_v1_table[slot]);
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

void
proc_v1_init(void)
{
        unsigned int i;

        for (i = 0U; i < PROC_V1_NPROC; ++i)
                proc_v1_zero(&proc_v1_table[i]);
        proc_v1_set_state(&proc_v1_table[0], PROC_V1_SRUN);
        proc_v1_current = &proc_v1_table[0];
        proc_v1_next_pid = 2U;
}

struct proc_v1 *
proc_v1_alloc_init(void)
{
        struct proc_v1 *p;

        p = &proc_v1_table[1];
        proc_v1_zero(p);
        p->meta = 1U | ((kword_t)PROC_V1_SIDL << PROC_V1_STATE_SHIFT);
        return p;
}

struct proc_v1 *
proc_v1_alloc_child(struct proc_v1 *parent)
{
        unsigned int i;
        unsigned int pid;
        unsigned int parent_slot;
        struct proc_v1 *p;

        parent_slot = proc_v1_slot(parent);
        if (parent_slot >= PROC_V1_NPROC)
                return 0;
        pid = proc_v1_new_pid();
        if (pid == 0U)
                return 0;
        for (i = 1U; i < PROC_V1_NPROC; ++i) {
                p = &proc_v1_table[i];
                if (PROC_V1_STATE(p) != PROC_V1_FREE)
                        continue;
                proc_v1_zero(p);
                p->meta = (parent->meta & ~((kword_t)PROC_V1_HALF_MASK)) |
                    ((kword_t)pid & PROC_V1_PID_MASK) |
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

        slot = proc_v1_slot(p);
        if (slot == 0U || slot >= PROC_V1_NPROC)
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

int
proc_v1_procfs_get(unsigned int slot, unsigned int field, kword_t *valuep)
{
        struct proc_v1 *p;

        if (slot >= PROC_V1_NPROC || valuep == 0)
                return -1;
        p = &proc_v1_table[slot];
        if (PROC_V1_STATE(p) == PROC_V1_FREE)
                return -1;
        switch (field) {
        case PROCFS_V1_FIELD_PID:
                *valuep = (kword_t)PROC_V1_PID(p);
                break;
        case PROCFS_V1_FIELD_PPID:
                *valuep = (kword_t)proc_v1_ppid(p);
                break;
        case PROCFS_V1_FIELD_STATE:
                *valuep = (kword_t)PROC_V1_STATE(p);
                break;
        case PROCFS_V1_FIELD_WORDS:
                *valuep = PROC_V1_MEM_WORDS(p);
                break;
        case PROCFS_V1_FIELD_COMM:
                if (PROC_V1_PID(p) == 0U)
                        *valuep = VFS_V1_SIX6('S','W','A','P','P','E');
                else if (PROC_V1_PID(p) == 1U)
                        *valuep = VFS_V1_SIX6('I','N','I','T',' ',' ');
                else
                        *valuep = VFS_V1_SIX6('U','S','E','R',' ',' ');
                break;
        default:
                return -1;
        }
        return 0;
}
