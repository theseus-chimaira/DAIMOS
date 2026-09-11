#include "proc.h"
#include "fs_mres.h"
#include "mm.h"
#include "proc_swap.h"
#include "procfs.h"
#include "syscall.h"

#define PROC_UAREA_MM_OWNER_BASE 01000U

unsigned int proc_sched_age_phase;
extern struct file *file_table;

static int proc_event_send(unsigned int target, unsigned int event, int group);

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

static inline int
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

static inline void
proc_trim_high(void)
{
        while (proc_high_slot > 1U &&
            PROC_STATE(&proc_table[proc_high_slot - 1U]) == PROC_FREE)
                --proc_high_slot;
}

static inline void
proc_slot_zero(struct proc *p)
{
        p->meta = 0UL;
        p->mem_layout = 0UL;
        p->sched = 0UL;
}

unsigned int
proc_session_id(const struct proc *p)
{
        if (PROC_STATE(p) == PROC_ZOMB)
                return PROC_ZOMB_SESSION(p);
        if (!PROC_HAS_UAREA(p))
                return 0U;
        return PROC_SESSION(p);
}

unsigned int
proc_domain_id(const struct proc *p)
{
        if (PROC_STATE(p) == PROC_ZOMB)
                return PROC_ZOMB_DOMAIN(p);
        if (!PROC_HAS_UAREA(p))
                return 0U;
        return PROC_DOMAIN(p);
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

/* One compact record per logical TTY.  The extra ten words versus half-word
 * packing save substantially more KCORE instructions in every control path. */
static kword_t proc_tty_records[PROC_TTY_COUNT];

static unsigned int
proc_tty_id(const struct proc *p)
{
        unsigned int state;

        if (p == 0 || !PROC_HAS_UAREA(p))
                return PROC_TTY_COUNT;
        state = PROC_TTY_STATE(p);
        if (state < PROC_TTY_ATTACHED_BASE)
                return PROC_TTY_COUNT;
        state -= PROC_TTY_ATTACHED_BASE;
        return state < PROC_TTY_COUNT ? state : PROC_TTY_COUNT;
}

static int
proc_tty_group_exists(unsigned int session, unsigned int pgrp)
{
        unsigned int i;

        if (pgrp == 0U)
                return 0;
        for (i = 1U; i < proc_high_slot; ++i) {
                const struct proc *member;

                member = &proc_table[i];
                if (PROC_STATE(member) == PROC_FREE ||
                    PROC_STATE(member) == PROC_ZOMB)
                        continue;
                if (PROC_PGRP(member) == pgrp &&
                    proc_session_id(member) == session)
                        return 1;
        }
        return 0;
}

static void
proc_tty_release_session(unsigned int session, unsigned int leaving_slot)
{
        unsigned int i;

        if (session == 0U)
                return;
        for (i = 1U; i < proc_high_slot; ++i) {
                const struct proc *member;

                if (i == leaving_slot)
                        continue;
                member = &proc_table[i];
                if (PROC_STATE(member) != PROC_FREE &&
                    PROC_STATE(member) != PROC_ZOMB &&
                    proc_session_id(member) == session)
                        return;
        }
        for (i = 0U; i < PROC_TTY_COUNT; ++i) {
                unsigned int record;

                record = (unsigned int)proc_tty_records[i];
                if (PROC_TTY_REC_SESSION(record) == session)
                        proc_tty_records[i] = 0UL;
        }
}

int
proc_child_hierarchy(unsigned int child_slot, unsigned int mode,
    unsigned int requested_pgrp)
{
        struct proc *child;
        struct proc *parent;
        unsigned int parent_slot;
        unsigned int pgrp;
        unsigned int session;
        unsigned int domain;
        unsigned int i;

        /* RUN calls this only after claiming a valid child slot and
         * installing its u-area; the parent is the live current process. */
        parent_slot = (unsigned int)proc_current_slot;
        parent = &proc_table[parent_slot];
        child = &proc_table[child_slot];
        session = PROC_SESSION(parent);
        domain = PROC_DOMAIN(parent);

        if (mode == SYS_RUN_PGRP_INHERIT) {
                if (requested_pgrp != 0U)
                        return -1;
                pgrp = PROC_PGRP(parent);
                if (pgrp == 0U)
                        return -1;
        } else if (mode == SYS_RUN_PGRP_NEW) {
                if (requested_pgrp != 0U)
                        return -1;
                pgrp = child_slot;
        } else if (mode == SYS_RUN_PGRP_JOIN) {
                if (requested_pgrp == 0U || requested_pgrp > PROC_PGRP_MASK)
                        return -1;
                pgrp = 0U;
                for (i = 1U; i < proc_high_slot; ++i) {
                        const struct proc *member;

                        member = &proc_table[i];
                        if (PROC_STATE(member) == PROC_FREE ||
                            PROC_PGRP(member) != requested_pgrp ||
                            proc_session_id(member) != session)
                                continue;
                        pgrp = requested_pgrp;
                        break;
                }
                if (pgrp == 0U)
                        return -1;
        } else {
                return -1;
        }

        PROC_SET_PGRP(child, pgrp);
        proc_ctl_set(child, PROC_SESSION_MASK, PROC_SESSION_SHIFT, session);
        proc_ctl_set(child, PROC_DOMAIN_MASK, PROC_DOMAIN_SHIFT, domain);
        proc_ctl_set(child, PROC_TTY_MASK, PROC_TTY_SHIFT,
            PROC_TTY_STATE(parent));
        return 0;
}

int
proc_control(unsigned int op, unsigned int arg)
{
        struct proc *p;
        unsigned int slot;
        unsigned int events;
        unsigned int session;
        unsigned int record;
        unsigned int tty_id;
        unsigned int pgrp;
        unsigned int i;

        /* PROCCTL is a user-mode syscall only.  The dispatcher can reach it
         * only with a current live process and its resident u-area installed. */
        slot = (unsigned int)proc_current_slot;
        p = &proc_table[slot];
        switch (op) {
        case SYS_PROCCTL_GETPGRP:
                return arg == 0U ? (int)PROC_PGRP(p) : -1;
        case SYS_PROCCTL_GETSESSION:
                return arg == 0U ? (int)PROC_SESSION(p) : -1;
        case SYS_PROCCTL_GETDOMAIN:
                return arg == 0U ? (int)PROC_DOMAIN(p) : -1;
        case SYS_PROCCTL_NEWSESSION:
                if (arg != 0U)
                        return -1;
                session = PROC_SESSION(p);
                proc_tty_release_session(session, slot);
                PROC_SET_PGRP(p, slot);
                proc_ctl_set(p, PROC_SESSION_MASK, PROC_SESSION_SHIFT, slot);
                proc_ctl_set(p, PROC_TTY_MASK, PROC_TTY_SHIFT,
                    PROC_TTY_NO_TTY);
                return (int)slot;
        case SYS_PROCCTL_NEWDOMAIN:
                if (arg != 0U || PROC_SESSION(p) != slot)
                        return -1;
                proc_ctl_set(p, PROC_DOMAIN_MASK, PROC_DOMAIN_SHIFT, slot);
                return (int)slot;
        case SYS_PROCCTL_GETEVENTS:
                if (arg != 0U)
                        return -1;
                events = PROC_EVENTS(p);
                proc_ctl_set(p, PROC_EVENT_MASK, PROC_EVENT_SHIFT, 0U);
                return (int)events;
        case SYS_PROCCTL_EVENT_PID:
        case SYS_PROCCTL_EVENT_PGRP:
                if ((arg & ~SYS_EVENT_ARG_MASK) != 0U)
                        return -1;
                return proc_event_send(arg & SYS_EVENT_TARGET_MASK,
                    (arg >> SYS_EVENT_CODE_SHIFT) & SYS_EVENT_CODE_MASK,
                    op == SYS_PROCCTL_EVENT_PGRP);
        case SYS_PROCCTL_GETTTY:
                return arg == 0U ? (int)PROC_TTY_STATE(p) : -1;
        case SYS_PROCCTL_TTY_ATTACH:
                if (arg >= PROC_TTY_COUNT || PROC_SESSION(p) != slot ||
                    PROC_TTY_STATE(p) >= PROC_TTY_ATTACHED_BASE)
                        return -1;
                record = (unsigned int)proc_tty_records[arg];
                session = PROC_TTY_REC_SESSION(record);
                if (session != 0U && session != slot)
                        return -1;
                if (session == 0U)
                        proc_tty_records[arg] = (kword_t)slot |
                            ((kword_t)PROC_PGRP(p) << PROC_TTY_REC_PGRP_SHIFT);
                proc_ctl_set(p, PROC_TTY_MASK, PROC_TTY_SHIFT,
                    PROC_TTY_ATTACHED_BASE + arg);
                return (int)arg;
        case SYS_PROCCTL_TTY_DETACH:
                if (arg != 0U || PROC_SESSION(p) != slot)
                        return -1;
                tty_id = proc_tty_id(p);
                if (tty_id >= PROC_TTY_COUNT)
                        return -1;
                record = (unsigned int)proc_tty_records[tty_id];
                if (PROC_TTY_REC_SESSION(record) != slot)
                        return -1;
                proc_tty_records[tty_id] = 0UL;
                for (i = 1U; i < proc_high_slot; ++i) {
                        struct proc *member;

                        member = &proc_table[i];
                        if (PROC_STATE(member) == PROC_FREE ||
                            PROC_STATE(member) == PROC_ZOMB ||
                            !PROC_HAS_UAREA(member) ||
                            proc_session_id(member) != slot ||
                            proc_tty_id(member) != tty_id)
                                continue;
                        proc_ctl_set(member, PROC_TTY_MASK, PROC_TTY_SHIFT,
                            PROC_TTY_DETACHED);
                }
                return 0;
        case SYS_PROCCTL_TTY_GETFG:
                if (arg != 0U)
                        return -1;
                tty_id = proc_tty_id(p);
                if (tty_id >= PROC_TTY_COUNT)
                        return -1;
                record = (unsigned int)proc_tty_records[tty_id];
                if (PROC_TTY_REC_SESSION(record) != PROC_SESSION(p))
                        return -1;
                return (int)PROC_TTY_REC_PGRP(record);
        case SYS_PROCCTL_TTY_SETFG:
                if (arg == 0U || arg > PROC_PGRP_MASK)
                        return -1;
                tty_id = proc_tty_id(p);
                if (tty_id >= PROC_TTY_COUNT)
                        return -1;
                record = (unsigned int)proc_tty_records[tty_id];
                session = PROC_SESSION(p);
                if (PROC_TTY_REC_SESSION(record) != session ||
                    !proc_tty_group_exists(session, arg))
                        return -1;
                pgrp = arg;
                proc_tty_records[tty_id] = (kword_t)session |
                    ((kword_t)pgrp << PROC_TTY_REC_PGRP_SHIFT);
                return (int)pgrp;
        default:
                return -1;
        }
}

int
proc_slot_discard(unsigned int slot)
{
        struct proc *p;
        kword_t base;

        if (proc_table == 0 || slot == 0U || slot >= proc_slots)
                return -1;
        p = &proc_table[slot];
        if (PROC_STATE(p) == PROC_FREE)
                return 0;
        base = PROC_MEM_BASE(p);
        if (base != 0UL && mm_free(base, MM_TYPE_PROCESS, slot) != MM_OK)
                return -1;
        proc_swap_detach(slot);
        proc_tty_release_session(proc_session_id(p), slot);
        if (proc_uarea_release(slot, p) != 0)
                return -1;
        proc_slot_zero(p);
        proc_trim_high();
        return 0;
}

static void
proc_wake_parent(unsigned int parent)
{
        struct proc *p;

        if (parent == 0U || proc_table == 0 || parent >= proc_slots)
                return;
        p = &proc_table[parent];
        if (PROC_STATE(p) != PROC_SLEEP ||
            PROC_WAIT_CLASS(p) != PROC_WAIT_CHILD)
                return;
        proc_set_field(p, PROC_WAIT_MASK, PROC_WAIT_SHIFT, PROC_WAIT_NONE);
        p->sched &= ~PROC_SCHED_RH_MASK;
        PROC_SET_STATE(p, PROC_SRUN);
}

static void
proc_queue_event(struct proc *p, unsigned int event)
{
        kword_t ctl;

        ctl = PROC_CTL_WORD(p);
        ctl |= (kword_t)SYS_EVENT_BIT(event) << PROC_EVENT_SHIFT;
        PROC_CTL_WORD(p) = ctl;
}

static void
proc_notify_parent(unsigned int parent)
{
        if (parent == 0U || proc_table == 0 || parent >= proc_slots ||
            PROC_STATE(&proc_table[parent]) == PROC_FREE ||
            PROC_STATE(&proc_table[parent]) == PROC_ZOMB)
                return;
        proc_queue_event(&proc_table[parent], SYS_EVENT_CHLD);
        proc_wake_parent(parent);
}

static void
proc_child_report(struct proc *child, unsigned int report)
{
        proc_ctl_set(child, PROC_REPORT_MASK, PROC_REPORT_SHIFT, report);
        proc_notify_parent(PROC_PARENT_SLOT(child));
}

static void
proc_adopt_children(unsigned int old_parent)
{
        unsigned int i;
        unsigned int new_parent;

        new_parent = 0U;
        if (old_parent != 1U && proc_slots > 1U &&
            PROC_STATE(&proc_table[1]) != PROC_FREE &&
            PROC_STATE(&proc_table[1]) != PROC_ZOMB)
                new_parent = 1U;
        for (i = 1U; i < proc_high_slot; ++i) {
                struct proc *child;

                if (i == old_parent)
                        continue;
                child = &proc_table[i];
                if (PROC_STATE(child) == PROC_FREE ||
                    PROC_PARENT_SLOT(child) != old_parent)
                        continue;
                if (new_parent == 0U && PROC_STATE(child) == PROC_ZOMB) {
                        proc_slot_zero(child);
                        continue;
                }
                PROC_SET_PARENT_SLOT(child, new_parent);
                if (new_parent != 0U && PROC_STATE(child) == PROC_ZOMB)
                        proc_notify_parent(new_parent);
        }
        proc_trim_high();
}

static int
proc_has_live_user(void)
{
        unsigned int i;

        for (i = 1U; i < proc_high_slot; ++i) {
                unsigned int state;

                state = PROC_STATE(&proc_table[i]);
                if (state != PROC_FREE && state != PROC_ZOMB)
                        return 1;
        }
        return 0;
}

/* Release one process after its file table is closed.  The caller must never
 * run on the target u-area stack while this function executes. */
static int
proc_finish_slot(unsigned int slot, unsigned int status)
{
        struct proc *p;
        kword_t base;
        unsigned int parent;
        unsigned int pgrp;
        unsigned int session;
        unsigned int domain;

        p = &proc_table[slot];
        parent = PROC_PARENT_SLOT(p);
        pgrp = PROC_PGRP(p);
        session = proc_session_id(p);
        domain = proc_domain_id(p);
        base = PROC_MEM_BASE(p);

        PROC_SET_TRANSITION(p);
        if (base != 0UL && mm_free(base, MM_TYPE_PROCESS, slot) != MM_OK) {
                PROC_CLEAR_TRANSITION(p);
                return -1;
        }
        proc_swap_detach(slot);
        p->mem_layout = 0UL;
        proc_tty_release_session(session, slot);
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
        p->sched = PROC_SCHED_DEFAULT |
            ((kword_t)session << PROC_ZOMB_SESSION_SHIFT) |
            ((kword_t)domain << PROC_ZOMB_DOMAIN_SHIFT);
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

static int
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
        if (!PROC_HAS_UAREA(p))
                return -1;
        saved = file_table;
        file_table = (struct file *)(unsigned long)
            (PROC_UAREA_BASE(p) + PROC_FILE_TABLE_OFFSET);
        file_close_all();
        file_table = saved;
        return proc_finish_slot(slot, SYS_WAIT_EVENT_FLAG | event);
}

static int
proc_event_apply(unsigned int slot, unsigned int event)
{
        struct proc *p;
        unsigned int reasons;
        unsigned int state;

        p = &proc_table[slot];
        state = PROC_STATE(p);
        proc_queue_event(p, event);

        if (event == SYS_EVENT_INT || event == SYS_EVENT_TERM ||
            event == SYS_EVENT_HUP)
                return proc_event_kill(slot, event);
        if (event == SYS_EVENT_TSTP) {
                reasons = PROC_STOP_REASONS(p);
                if ((reasons & PROC_STOP_JOB) != 0U)
                        return 0;
                proc_ctl_set(p, PROC_STOP_MASK, PROC_STOP_SHIFT,
                    reasons | PROC_STOP_JOB);
                PROC_SET_STATE(p, PROC_STOP);
                proc_child_report(p, PROC_REPORT_STOPPED);
                if (slot == (unsigned int)proc_current_slot)
                        proc_sched_resched_current();
                return 0;
        }
        if (event == SYS_EVENT_CONT) {
                reasons = PROC_STOP_REASONS(p);
                if ((reasons & PROC_STOP_JOB) == 0U)
                        return 0;
                reasons &= ~PROC_STOP_JOB;
                proc_ctl_set(p, PROC_STOP_MASK, PROC_STOP_SHIFT, reasons);
                if (reasons == 0U && PROC_STATE(p) == PROC_STOP) {
                        PROC_SET_STATE(p, PROC_WAIT_CLASS(p) == PROC_WAIT_NONE ?
                            PROC_SRUN : PROC_SLEEP);
                }
                proc_child_report(p, PROC_REPORT_CONTINUED);
                return 0;
        }
        if (event == SYS_EVENT_ALRM) {
                if (state == PROC_SLEEP &&
                    PROC_WAIT_CLASS(p) == PROC_WAIT_EVENT) {
                        proc_set_field(p, PROC_WAIT_MASK, PROC_WAIT_SHIFT,
                            PROC_WAIT_NONE);
                        p->sched &= ~PROC_SCHED_RH_MASK;
                        PROC_SET_STATE(p, PROC_SRUN);
                }
                return 0;
        }
        return -1;
}

static int
proc_event_send(unsigned int target, unsigned int event, int group)
{
        struct proc *caller;
        unsigned int caller_slot;
        unsigned int session;
        unsigned int domain;
        unsigned int i;
        unsigned int count;
        int self_deferred;

        caller_slot = (unsigned int)proc_current_slot;
        if (target == 0U || event >= SYS_EVENT_COUNT ||
            event == SYS_EVENT_CHLD)
                return -1;
        caller = &proc_table[caller_slot];
        session = PROC_SESSION(caller);
        domain = PROC_DOMAIN(caller);
        self_deferred = 0;
        count = 0U;

        if (!group) {
                struct proc *p;

                if (target >= proc_slots)
                        return -1;
                p = &proc_table[target];
                if (PROC_STATE(p) == PROC_FREE || PROC_STATE(p) == PROC_ZOMB ||
                    !PROC_HAS_UAREA(p) || PROC_DOMAIN(p) != domain)
                        return -1;
                return proc_event_apply(target, event);
        }

        for (i = 1U; i < proc_high_slot; ++i) {
                struct proc *p;

                p = &proc_table[i];
                if (PROC_STATE(p) == PROC_FREE || PROC_STATE(p) == PROC_ZOMB ||
                    !PROC_HAS_UAREA(p) || PROC_PGRP(p) != target ||
                    PROC_SESSION(p) != session || PROC_DOMAIN(p) != domain)
                        continue;
                ++count;
                if (i == caller_slot && (event == SYS_EVENT_INT ||
                    event == SYS_EVENT_TERM || event == SYS_EVENT_HUP ||
                    event == SYS_EVENT_TSTP)) {
                        self_deferred = 1;
                        continue;
                }
                if (proc_event_apply(i, event) != 0)
                        return -1;
        }
        if (count == 0U)
                return -1;
        if (self_deferred)
                return proc_event_apply(caller_slot, event);
        return 0;
}

int
proc_tty_read_enter(unsigned int tty_id)
{
        struct proc *p;

        if (tty_id >= PROC_TTY_COUNT)
                return -1;
        p = &proc_table[(unsigned int)proc_current_slot];
        for (;;) {
                unsigned int state;
                unsigned int record;
                unsigned int session;
                unsigned int pgrp;

                state = PROC_TTY_STATE(p);
                if (state == PROC_TTY_NO_TTY)
                        return 0;
                if (state == PROC_TTY_DETACHED ||
                    state != PROC_TTY_ATTACHED_BASE + tty_id)
                        return -1;
                record = (unsigned int)proc_tty_records[tty_id];
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
proc_tty_input(unsigned int tty_id, unsigned int ch)
{
        struct proc *p;
        unsigned int state;
        unsigned int record;
        unsigned int pgrp;

        if (tty_id >= PROC_TTY_COUNT)
                return -1;
        p = &proc_table[(unsigned int)proc_current_slot];
        state = PROC_TTY_STATE(p);
        if (state == PROC_TTY_NO_TTY)
                return (int)(ch & 0177U);
        if (state == PROC_TTY_DETACHED ||
            state != PROC_TTY_ATTACHED_BASE + tty_id)
                return -1;
        record = (unsigned int)proc_tty_records[tty_id];
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

        if ((flags & ~SYS_WAIT_NOHANG) != 0U ||
            (selector & ~(SYS_WAIT_PGRP_FLAG | SYS_WAIT_ID_MASK)) != 0U ||
            ((selector & SYS_WAIT_PGRP_FLAG) != 0U &&
            (selector & SYS_WAIT_ID_MASK) == 0U))
                return -1;
        parent = (unsigned int)proc_current_slot;

        for (;;) {
                unsigned int i;
                int have_child;

                have_child = 0;
                for (i = 1U; i < proc_high_slot; ++i) {
                        struct proc *child;

                        child = &proc_table[i];
                        if (PROC_STATE(child) == PROC_FREE ||
                            PROC_PARENT_SLOT(child) != parent)
                                continue;
                        if ((selector & SYS_WAIT_PGRP_FLAG) != 0U) {
                                if (PROC_PGRP(child) !=
                                    (selector & SYS_WAIT_ID_MASK))
                                        continue;
                        } else if (selector != 0U &&
                            (selector & SYS_WAIT_ID_MASK) != i) {
                                continue;
                        }
                        have_child = 1;
                        if (PROC_STATE(child) != PROC_ZOMB &&
                            PROC_HAS_UAREA(child)) {
                                unsigned int report;

                                report = PROC_WAIT_REPORT(child);
                                if (report == PROC_REPORT_STOPPED ||
                                    report == PROC_REPORT_CONTINUED) {
                                        proc_ctl_set(child, PROC_REPORT_MASK,
                                            PROC_REPORT_SHIFT,
                                            PROC_REPORT_NONE);
                                        if (statusp != 0)
                                                *statusp = SYS_WAIT_STATUS(
                                                    report == PROC_REPORT_STOPPED ?
                                                    SYS_WAIT_STOPPED :
                                                    SYS_WAIT_CONTINUED,
                                                    report == PROC_REPORT_STOPPED ?
                                                    SYS_EVENT_TSTP :
                                                    SYS_EVENT_CONT);
                                        return (int)i;
                                }
                        }
                        if (PROC_STATE(child) != PROC_ZOMB)
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
proc_nice_value(unsigned int slot)
{
        if (proc_table == 0 || slot == 0U || slot >= proc_slots ||
            PROC_STATE(&proc_table[slot]) == PROC_FREE)
                return 0;
        return (int)PROC_NICE_ENCODED(&proc_table[slot]) -
            (int)PROC_NICE_BIAS;
}

int
proc_nice_current(int value)
{
        unsigned int slot;
        struct proc *p;
        unsigned int encoded;

        slot = (unsigned int)proc_current_slot;
        if (proc_table == 0 || slot == 0U || slot >= proc_slots)
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
        unsigned int n;
        unsigned int limit;
        unsigned int best;
        unsigned int best_prio;

        if (proc_table == 0 || proc_high_slot <= 1U)
                return 0U;
        limit = proc_high_slot;
        best = 0U;
        best_prio = ~0U;
        for (n = 1U; n <= limit; ++n) {
                struct proc *p;
                unsigned int slot;
                unsigned int prio;

                slot = ((unsigned int)proc_sched_cursor + n) % limit;
                if (slot == 0U)
                        continue;
                p = &proc_table[slot];
                if (PROC_STATE(p) != PROC_SRUN || PROC_MEM_BASE(p) == 0UL ||
                    PROC_TRANSITION(p))
                        continue;
                prio = proc_effective(p);
                if (best == 0U || prio < best_prio) {
                        best = slot;
                        best_prio = prio;
                }
        }
        if (best != 0U)
                proc_sched_cursor = (kword_t)best;
        return best;
}

unsigned int
proc_sched_resched_select(void)
{
        return proc_select_runnable();
}

unsigned int
proc_sched_tick_select(void)
{
        unsigned int cur;
        unsigned int i;
        unsigned int limit;
        int age_tick;

        if (proc_table == 0 || proc_high_slot <= 1U)
                return 0U;
        cur = (unsigned int)proc_current_slot;
        limit = proc_high_slot;
        age_tick = 0;
        if (++proc_sched_age_phase >= 64U) {
                proc_sched_age_phase = 0U;
                age_tick = 1;
        }

        for (i = 1U; i < limit; ++i) {
                struct proc *p;
                unsigned int cpu;

                p = &proc_table[i];
                if (PROC_STATE(p) == PROC_FREE)
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
        unsigned int i;
        unsigned int best;
        unsigned int best_score;

        if (proc_table == 0)
                return -1;
        best = 0U;
        best_score = 0U;
        for (i = 1U; i < proc_high_slot; ++i) {
                const struct proc *p;
                unsigned int state;
                unsigned int score;
                int nice;

                if (i == exclude_owner || i == (unsigned int)proc_current_slot)
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
                        score = 04000U + PROC_SLEEP_AGE(p) * 0100U;
                        if (PROC_WAIT_CLASS(p) != PROC_WAIT_NONE)
                                score += 040U;
                } else {
                        score = 01000U + proc_effective(p);
                        if (nice > 0)
                                score += 02000U + (unsigned int)nice * 010U;
                }
                if (best == 0U || score > best_score) {
                        best = i;
                        best_score = score;
                }
        }
        return best == 0U ? -1 : (int)best;
}
