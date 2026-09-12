#include "proc.h"
#include "fs_mres.h"
#include "mm.h"
#include "proc_swap.h"
#include "procfs.h"
#include "syscall.h"

#define PROC_UAREA_MM_OWNER_BASE 01000U

/* Slot indexes are 1..PROC_MAX_SLOTS-1 and scheduler scores are small
 * positive values.  Signed locals avoid PDP-10 unsigned-compare glue while
 * preserving the externally stored unsigned slot/count representation. */

unsigned int proc_sched_age_phase;
extern struct file *file_table;

extern int proc_event_send(unsigned int target, unsigned int event, int group);

static inline void
proc_set_field(struct proc *p, kword_t mask, unsigned int shift,
    unsigned int value)
{
        p->sched = (p->sched & ~(mask << shift)) |
            (((kword_t)value & mask) << shift);
}

static inline void
proc_set_cpu(struct proc *p, unsigned int value)
{
        proc_set_field(p, PROC_CPU_MASK, PROC_CPU_SHIFT, value);
}

static inline void
proc_set_sleep_age(struct proc *p, unsigned int value)
{
        proc_set_field(p, PROC_SLEEP_MASK, PROC_SLEEP_SHIFT, value);
}

static unsigned int
proc_effective(const struct proc *p)
{
        return PROC_NICE_ENCODED(p) + PROC_CPU_PENALTY(p);
}

static inline unsigned int
proc_uarea_owner(unsigned int slot)
{
        return PROC_UAREA_MM_OWNER_BASE + slot;
}

static int
proc_uarea_release(unsigned int slot, struct proc *p)
{
        kword_t base;

        if (!PROC_HAS_UAREA(p))
                return 0;
        base = PROC_UAREA_BASE(p);
        if (base == 0UL || mm_free(base, MM_TYPE_KERNEL_DYNAMIC,
            proc_uarea_owner(slot)) != MM_OK)
                return -1;
        p->meta &= ~((kword_t)PROC_F_UAREA << PROC_FLAGS_SHIFT);
        PROC_SET_META_LH(p, 0UL);
        return 0;
}

static void
proc_trim_high(void)
{
        while (proc_high_slot > 1U &&
            PROC_IS_FREE(&proc_table[proc_high_slot - 1U]))
                --proc_high_slot;
}

static void
proc_slot_zero(struct proc *p)
{
        p->meta = 0UL;
        p->mem_layout = 0UL;
        p->sched = 0UL;
}


static inline void
proc_ctl_set(struct proc *p, kword_t mask, unsigned int shift,
    unsigned int value)
{
        kword_t ctl;

        ctl = PROC_CTL_WORD(p);
        ctl &= ~(mask << shift);
        ctl |= ((kword_t)value & mask) << shift;
        PROC_CTL_WORD(p) = ctl;
}

#define PROC_TTY_REC_SESSION_MASK 0377U
#define PROC_TTY_REC_PGRP_MASK    0377U
#define PROC_TTY_REC_PGRP_SHIFT      8U
#define PROC_TTY_REC_SESSION(r) \
        ((unsigned int)(r) & PROC_TTY_REC_SESSION_MASK)
#define PROC_TTY_REC_PGRP(r) \
        (((unsigned int)(r) >> PROC_TTY_REC_PGRP_SHIFT) & \
        PROC_TTY_REC_PGRP_MASK)
#define PROC_DOMAIN_BITS \
        ((kword_t)PROC_DOMAIN_MASK << PROC_DOMAIN_SHIFT)
#define PROC_SCOPE_BITS \
        (((kword_t)PROC_SESSION_MASK << PROC_SESSION_SHIFT) | PROC_DOMAIN_BITS)
#define PROC_STOP_BITS \
        ((kword_t)PROC_STOP_MASK << PROC_STOP_SHIFT)
#define PROC_STOP_JOB_BIT \
        ((kword_t)PROC_STOP_JOB << PROC_STOP_SHIFT)
#define PROC_WAIT_BITS \
        ((kword_t)PROC_WAIT_MASK << PROC_WAIT_SHIFT)

/* One compact record per logical TTY.  The extra ten words versus half-word
 * packing save substantially more KCORE instructions in every control path. */
kword_t proc_tty_records[PROC_TTY_COUNT];

extern int proc_tty_session_has(unsigned int session, unsigned int pgrp,
    unsigned int skip_slot);

void
proc_tty_release_session(unsigned int session, unsigned int leaving_slot)
{
        unsigned int i;

        if (session == 0U)
                return;
        if (proc_tty_session_has(session, 0U, leaving_slot))
                return;
        for (i = 0U; i < PROC_TTY_COUNT; ++i) {
                unsigned int record;

                record = (unsigned int)proc_tty_records[i];
                if (PROC_TTY_REC_SESSION(record) == session)
                        proc_tty_records[i] = 0UL;
        }
}


int
proc_slot_discard(unsigned int slot)
{
        struct proc *p;
        kword_t base;

        /* Process slots are bounded small positive table indices. */
        if (proc_table == 0 || (int)slot <= 0 ||
            (int)slot >= (int)proc_slots)
                return -1;
        p = &proc_table[slot];
        if (PROC_IS_FREE(p))
                return 0;
        base = PROC_MEM_BASE(p);
        if (base != 0UL && mm_free(base, MM_TYPE_PROCESS, slot) != MM_OK)
                return -1;
        proc_swap_detach(slot);
        proc_tty_release_session((unsigned int)(proc_scope_id(p) &
            PROC_ZOMB_SESSION_MASK), slot);
        if (proc_uarea_release(slot, p) != 0)
                return -1;
        proc_slot_zero(p);
        proc_trim_high();
        return 0;
}

static inline void
proc_queue_event(struct proc *p, unsigned int event)
{
        kword_t ctl;

        ctl = PROC_CTL_WORD(p);
        ctl |= (kword_t)SYS_EVENT_BIT(event) << PROC_EVENT_SHIFT;
        PROC_CTL_WORD(p) = ctl;
}

extern void proc_notify_parent(unsigned int parent);

static void
proc_child_report(struct proc *child, unsigned int report)
{
        proc_ctl_set(child, PROC_REPORT_MASK, PROC_REPORT_SHIFT, report);
        proc_notify_parent(PROC_PARENT_SLOT(child));
}

static void
proc_adopt_children(unsigned int old_parent)
{
        int i;
        unsigned int new_parent;

        new_parent = 0U;
        if (old_parent != 1U && proc_slots > 1U &&
            !PROC_IS_FREE_OR_ZOMB(&proc_table[1]))
                new_parent = 1U;
        for (i = 1; i < (int)proc_high_slot; ++i) {
                struct proc *child;
                unsigned int state;

                if ((unsigned int)i == old_parent)
                        continue;
                child = &proc_table[i];
                state = PROC_STATE(child);
                if (state == PROC_FREE ||
                    PROC_PARENT_SLOT(child) != old_parent)
                        continue;
                if (new_parent == 0U && state == PROC_ZOMB) {
                        proc_slot_zero(child);
                        continue;
                }
                PROC_SET_PARENT_SLOT(child, new_parent);
                if (new_parent != 0U && state == PROC_ZOMB)
                        proc_notify_parent(new_parent);
        }
        proc_trim_high();
}

extern int proc_has_live_user(void);

/* Release one process after its file table is closed.  The caller must never
 * run on the target u-area stack while this function executes. */
static int
proc_finish_slot(unsigned int slot, unsigned int status)
{
        struct proc *p;
        kword_t base;
        unsigned int parent;
        unsigned int pgrp;
        kword_t scope;
        kword_t ctl;

        p = &proc_table[slot];
        parent = PROC_PARENT_SLOT(p);
        pgrp = PROC_PGRP(p);
        ctl = PROC_CTL_WORD(p);
        scope = (ctl >> PROC_SESSION_SHIFT) & PROC_ZOMB_SCOPE_MASK;
        base = PROC_MEM_BASE(p);

        PROC_SET_TRANSITION(p);
        if (base != 0UL && mm_free(base, MM_TYPE_PROCESS, slot) != MM_OK) {
                PROC_CLEAR_TRANSITION(p);
                return -1;
        }
        proc_swap_detach(slot);
        p->mem_layout = 0UL;
        proc_tty_release_session((unsigned int)(scope &
            PROC_ZOMB_SESSION_MASK), slot);
        if (proc_uarea_release(slot, p) != 0)
                return -1;
        proc_adopt_children(slot);

        if (parent == 0U) {
                proc_slot_zero(p);
                proc_trim_high();
                return 0;
        }
        p->meta = ((kword_t)pgrp << PROC_PGRP_SHIFT) |
            ((kword_t)parent << PROC_PARENT_SHIFT) |
            (((kword_t)status & PROC_HALF_MASK) << PROC_HALF_SHIFT);
        p->sched = PROC_SCHED_DEFAULT | scope;
        PROC_SET_STATE(p, PROC_ZOMB);
        proc_notify_parent(parent);
        return 0;
}

/*
 * Finish EXIT after assembly has moved execution to the permanent idle stack.
 * Heavy process resources are released immediately.  A child with a live
 * parent keeps only its three-word descriptor as a zombie until WAIT reaps it.
 */
int
proc_exit_finish(int status)
{
        unsigned int slot;

        slot = (unsigned int)proc_current_slot;
        proc_current_slot = 0UL;
        if (proc_finish_slot(slot, (unsigned int)status) != 0)
                return -1;
        return proc_has_live_user();
}

static inline int
proc_event_kill(unsigned int slot, unsigned int event)
{
        struct proc *p;
        struct file *saved;

        if (slot == (unsigned int)proc_current_slot) {
                file_close_all();
                proc_exit_current((int)(SYS_WAIT_EVENT_FLAG | event));
                return -1;
        }
        p = &proc_table[slot];
        saved = file_table;
        file_table = (struct file *)(unsigned long)
            (PROC_UAREA_BASE(p) + PROC_FILE_TABLE_OFFSET);
        file_close_all();
        file_table = saved;
        return proc_finish_slot(slot, SYS_WAIT_EVENT_FLAG | event);
}

int
proc_event_apply(unsigned int slot, unsigned int event)
{
        struct proc *p;
        kword_t ctl;
        unsigned int state;

        p = &proc_table[slot];
        state = PROC_STATE(p);

        /* Fatal events are reported through the zombie wait status.  Their
         * pending bit would live only in the u-area that proc_event_kill()
         * immediately releases, so do not create dead state. */
        if (event <= SYS_EVENT_HUP)
                return proc_event_kill(slot, event);
        proc_queue_event(p, event);
        if (event == SYS_EVENT_TSTP) {
                ctl = PROC_CTL_WORD(p);
                if ((ctl & PROC_STOP_JOB_BIT) != 0UL)
                        return 0;
                PROC_CTL_WORD(p) = ctl | PROC_STOP_JOB_BIT;
                PROC_SET_STATE(p, PROC_STOP);
                proc_child_report(p, PROC_REPORT_STOPPED);
                if (slot == (unsigned int)proc_current_slot)
                        proc_sched_resched_current();
                return 0;
        }
        if (event == SYS_EVENT_CONT) {
                ctl = PROC_CTL_WORD(p);
                if ((ctl & PROC_STOP_JOB_BIT) == 0UL)
                        return 0;
                ctl &= ~PROC_STOP_JOB_BIT;
                PROC_CTL_WORD(p) = ctl;
                if ((ctl & PROC_STOP_BITS) == 0UL && state == PROC_STOP) {
                        PROC_SET_STATE(p, PROC_WAIT_CLASS(p) == PROC_WAIT_NONE ?
                            PROC_SRUN : PROC_SLEEP);
                }
                proc_child_report(p, PROC_REPORT_CONTINUED);
                return 0;
        }
        /* proc_event_send admits only INT..ALRM; ALRM is the sole case left. */
        if (state == PROC_SLEEP && PROC_WAIT_CLASS(p) == PROC_WAIT_EVENT) {
                p->sched = (p->sched &
                    ~(PROC_WAIT_BITS | PROC_SCHED_RH_MASK | PROC_STATE_BITS)) |
                    ((kword_t)PROC_SRUN << PROC_STATE_SHIFT);
        }
        return 0;
}

/* The only installed input service is CTY, logical TTY 0. */
int
proc_tty_read_enter(void)
{
        struct proc *p;

        p = &proc_table[(unsigned int)proc_current_slot];
        for (;;) {
                unsigned int state;
                unsigned int record;
                unsigned int session;
                unsigned int pgrp;

                state = PROC_TTY_STATE(p);
                if (state == PROC_TTY_NO_TTY)
                        return 0;
                if (state != PROC_TTY_ATTACHED_BASE)
                        return -1;
                record = (unsigned int)proc_tty_records[0];
                session = PROC_SESSION(p);
                if (PROC_TTY_REC_SESSION(record) != session)
                        return -1;
                pgrp = PROC_PGRP(p);
                if (PROC_TTY_REC_PGRP(record) == pgrp)
                        return 0;
                if (proc_event_send(pgrp, SYS_EVENT_TSTP, 1) != 0)
                        return -1;
                /* A self TSTP requests an immediate PI6 reschedule.  This
                 * continuation resumes here after CONT and rechecks whether
                 * the group actually owns the terminal before consuming input. */
        }
}

int
proc_tty_input(unsigned int ch)
{
        struct proc *p;
        unsigned int state;
        unsigned int record;
        unsigned int pgrp;

        p = &proc_table[(unsigned int)proc_current_slot];
        state = PROC_TTY_STATE(p);
        if (state == PROC_TTY_NO_TTY)
                return (int)(ch & 0177U);
        if (state != PROC_TTY_ATTACHED_BASE)
                return -1;
        record = (unsigned int)proc_tty_records[0];
        if (PROC_TTY_REC_SESSION(record) != PROC_SESSION(p))
                return -1;
        pgrp = PROC_TTY_REC_PGRP(record);
        if (pgrp == 0U || pgrp != PROC_PGRP(p))
                return -1;
        ch &= 0177U;
        if (ch == 003U) {
                (void)proc_event_send(pgrp, SYS_EVENT_INT, 1);
                return -2;
        }
        if (ch == 032U) {
                if (proc_event_send(pgrp, SYS_EVENT_TSTP, 1) != 0)
                        return -1;
                return -2;
        }
        return (int)ch;
}

int
proc_wait_status(unsigned int selector, kword_t *statusp, unsigned int flags)
{
        unsigned int parent;
        unsigned int id;
        int group;

        id = selector & SYS_WAIT_ID_MASK;
        group = (selector & SYS_WAIT_PGRP_FLAG) != 0U;
        if ((flags & ~SYS_WAIT_NOHANG) != 0U ||
            (selector & ~(SYS_WAIT_PGRP_FLAG | SYS_WAIT_ID_MASK)) != 0U ||
            (group && id == 0U))
                return -1;
        parent = (unsigned int)proc_current_slot;

        for (;;) {
                int i;
                int have_child;

                have_child = 0;
                for (i = 1; i < (int)proc_high_slot; ++i) {
                        struct proc *child;
                        unsigned int state;

                        child = &proc_table[i];
                        state = PROC_STATE(child);
                        if (state == PROC_FREE ||
                            PROC_PARENT_SLOT(child) != parent)
                                continue;
                        if (group) {
                                if (PROC_PGRP(child) != id)
                                        continue;
                        } else if (id != 0U && id != (unsigned int)i) {
                                continue;
                        }
                        have_child = 1;
                        if (state != PROC_ZOMB && PROC_HAS_UAREA(child)) {
                                unsigned int report;

                                report = PROC_WAIT_REPORT(child);
                                if (report == PROC_REPORT_STOPPED ||
                                    report == PROC_REPORT_CONTINUED) {
                                        proc_ctl_set(child, PROC_REPORT_MASK,
                                            PROC_REPORT_SHIFT,
                                            PROC_REPORT_NONE);
                                        if (statusp != 0)
                                                *statusp = SYS_WAIT_STATUS(
                                                    report + 1U, report + 2U);
                                        return (int)i;
                                }
                        }
                        if (state != PROC_ZOMB)
                                continue;
                        if (statusp != 0)
                                *statusp = SYS_WAIT_STATUS(SYS_WAIT_EXITED,
                                    PROC_EXIT_STATUS(child));
                        proc_slot_zero(child);
                        proc_trim_high();
                        return (int)i;
                }
                if (!have_child)
                        return -1;
                if ((flags & SYS_WAIT_NOHANG) != 0U)
                        return 0;
                if (proc_wait_child() != 0)
                        return -1;
        }
}

kword_t
proc_comm(const struct proc *p)
{
        unsigned int slot;

        if (p == 0 || proc_table == 0)
                return VFS_SIX6('U','S','E','R',' ',' ');
        slot = (unsigned int)(p - proc_table);
        if (slot == 0U)
                return VFS_SIX6('S','W','A','P','P','E');
        if (slot == 1U)
                return VFS_SIX6('I','N','I','T',' ',' ');
        return VFS_SIX6('U','S','E','R',' ',' ');
}

int
proc_nice_value(int slot)
{
        if (proc_table == 0 || slot <= 0 || slot >= (int)proc_slots ||
            PROC_IS_FREE(&proc_table[slot]))
                return 0;
        return (int)PROC_NICE_ENCODED(&proc_table[slot]) -
            (int)PROC_NICE_BIAS;
}

int
proc_nice_current(int value)
{
        int slot;
        struct proc *p;
        unsigned int encoded;

        slot = (int)proc_current_slot;
        if (proc_table == 0 || slot == 0 || slot >= (int)proc_slots)
                return -1;
        if (value < PROC_NICE_MIN)
                value = PROC_NICE_MIN;
        if (value > PROC_NICE_MAX)
                value = PROC_NICE_MAX;
        p = &proc_table[slot];
        encoded = (unsigned int)(value + (int)PROC_NICE_BIAS);
        proc_set_field(p, PROC_NICE_MASK, PROC_NICE_SHIFT, encoded);
        return value;
}

static unsigned int
proc_select_runnable(void)
{
        int n;
        int limit;
        int best;
        int best_prio;

        if (proc_table == 0 || (int)proc_high_slot <= 1)
                return 0U;
        limit = (int)proc_high_slot;
        best = 0;
        best_prio = 0;
        for (n = 1; n <= limit; ++n) {
                struct proc *p;
                int slot;
                int prio;

                slot = ((int)proc_sched_cursor + n) % limit;
                if (slot == 0)
                        continue;
                p = &proc_table[slot];
                if (PROC_STATE(p) != PROC_SRUN || PROC_MEM_BASE(p) == 0UL ||
                    PROC_TRANSITION(p))
                        continue;
                prio = proc_effective(p);
                if (best == 0 || prio < best_prio) {
                        best = slot;
                        best_prio = prio;
                }
        }
        if (best != 0)
                proc_sched_cursor = (kword_t)best;
        return (unsigned int)best;
}

unsigned int
proc_sched_resched_select(void)
{
        return proc_select_runnable();
}

unsigned int
proc_sched_tick_select(void)
{
        int cur;
        int i;
        int limit;
        int age_tick;

        if (proc_table == 0 || (int)proc_high_slot <= 1)
                return 0U;
        cur = (int)proc_current_slot;
        limit = (int)proc_high_slot;
        age_tick = 0;
        if (++proc_sched_age_phase >= 64U) {
                proc_sched_age_phase = 0U;
                age_tick = 1;
        }

        for (i = 1; i < limit; ++i) {
                struct proc *p;
                unsigned int cpu;

                p = &proc_table[i];
                if (PROC_IS_FREE(p))
                        continue;
                cpu = PROC_CPU_PENALTY(p);
                if (cpu != 0U)
                        --cpu;
                if (i == cur && PROC_STATE(p) == PROC_SRUN &&
                    !PROC_TRANSITION(p)) {
                        cpu += 2U;
                        if (cpu > (unsigned int)PROC_CPU_MASK)
                                cpu = (unsigned int)PROC_CPU_MASK;
                }
                proc_set_cpu(p, cpu);
                if (PROC_STATE(p) == PROC_SLEEP) {
                        unsigned int age;

                        age = PROC_SLEEP_AGE(p);
                        if (age_tick && age < (unsigned int)PROC_SLEEP_MASK)
                                ++age;
                        proc_set_sleep_age(p, age);
                } else if (PROC_SLEEP_AGE(p) != 0U) {
                        proc_set_sleep_age(p, 0U);
                }
        }
        return proc_select_runnable();
}

int
proc_swap_victim(unsigned int exclude_owner)
{
        int i;
        int best;
        int best_score;

        if (proc_table == 0)
                return -1;
        best = 0;
        best_score = 0;
        for (i = 1; i < (int)proc_high_slot; ++i) {
                const struct proc *p;
                unsigned int state;
                int score;
                int nice;

                if ((unsigned int)i == exclude_owner || i == (int)proc_current_slot)
                        continue;
                p = &proc_table[i];
                state = PROC_STATE(p);
                if ((state != PROC_SLEEP && state != PROC_STOP &&
                    state != PROC_SRUN) || PROC_TRANSITION(p))
                        continue;
                if (PROC_MEM_BASE(p) == 0UL || mm_is_pinned(PROC_MEM_BASE(p)))
                        continue;

                nice = proc_nice_value(i);
                if (state == PROC_SLEEP || state == PROC_STOP) {
                        score = 04000 + (int)PROC_SLEEP_AGE(p) * 0100;
                        if (PROC_WAIT_CLASS(p) != PROC_WAIT_NONE)
                                score += 040;
                } else {
                        score = 01000 + (int)proc_effective(p);
                        if (nice > 0)
                                score += 02000 + nice * 010;
                }
                if (best == 0 || score > best_score) {
                        best = i;
                        best_score = score;
                }
        }
        return best == 0 ? -1 : best;
}
