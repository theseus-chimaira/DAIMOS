#include "proc.h"
#include "fs_mres.h"
#include "mm.h"
#include "vm.h"
#include "proc_swap.h"
#include "monitorfs.h"
#include "syscall.h"
#include "tty.h"

#define PROC_UAREA_MM_OWNER_BASE 01000U

/* Slot indexes are 1..PROC_MAX_SLOTS-1 and scheduler scores are small
 * positive values.  Signed locals avoid PDP-10 unsigned-compare glue while
 * preserving the externally stored unsigned slot/count representation. */

unsigned int proc_sched_age_phase;
extern struct file *file_table;

extern int proc_event_send(unsigned int target, unsigned int event, int group);
#ifdef __PDP10__
extern int native_sys_putchar_call(kword_t tty_char);
#endif
extern int proc_session_teardown(unsigned int leader_slot, kword_t leader_ctl);

/* KCC emits calls for tiny C helpers here.  Keep scheduler-common
 * expressions explicit so selection does not pay those calls. */
#define PROC_EFFECTIVE(p) \
        (PROC_NICE_ENCODED(p) + PROC_CPU_PENALTY(p))
#define PROC_UAREA_OWNER(slot) (PROC_UAREA_MM_OWNER_BASE + (slot))
#ifdef __PDP10__
#define PROC_SWAP_RECORD_PRESENT(slot) \
        (proc_swap_records[(slot)].state != 0UL)
#else
#define PROC_SWAP_RECORD_PRESENT(slot) \
        (proc_swap_records != 0 && proc_swap_records[(slot)].state != 0UL)
#endif

static int
proc_uarea_release(unsigned int slot, struct proc *p)
{
        kword_t base;

        if (!PROC_HAS_UAREA(p))
                return 0;
        base = PROC_UAREA_BASE(p);
        if (base == 0UL || mm_free(base, MM_TYPE_KERNEL_DYNAMIC,
            PROC_UAREA_OWNER(slot)) != MM_OK)
                return -1;
        p->meta &= ~((kword_t)PROC_F_UAREA << PROC_FLAGS_SHIFT);
        PROC_SET_META_LH(p, 0UL);
        return 0;
}

extern void proc_trim_high(void);

static void
proc_slot_zero(struct proc *p)
{
        p->meta = 0UL;
        VM_SPACE_RESET(p);
        p->sched = 0UL;
}

#ifndef __PDP10__
/* Target helpers are compact PDP-10 assembly.  Keep the same intrusive-list
 * semantics in host tests so queue invariants are exercised there too. */
void
proc_runq_add(unsigned int slot)
{
        struct proc *p;

        if (proc_table == 0 || slot == 0U || slot >= proc_slots)
                return;
        p = &proc_table[slot];
        if (PROC_STATE(p) != PROC_SRUN)
                return;
        p->sched = (p->sched & ~PROC_SCHED_RH_MASK) |
            (proc_runq_head & PROC_SCHED_RH_MASK);
        proc_runq_head = (kword_t)slot;
}

void
proc_runq_remove(unsigned int slot)
{
        unsigned int cur;
        unsigned int prev;
        struct proc *p;

        if (proc_table == 0 || slot == 0U || slot >= proc_slots)
                return;
        prev = 0U;
        cur = (unsigned int)(proc_runq_head & PROC_SCHED_RH_MASK);
        while (cur != 0U && cur != slot) {
                prev = cur;
                cur = PROC_RUNQ_NEXT(&proc_table[cur]);
        }
        if (cur == 0U)
                return;
        p = &proc_table[slot];
        cur = PROC_RUNQ_NEXT(p);
        if (prev == 0U)
                proc_runq_head = (kword_t)cur;
        else
                proc_table[prev].sched =
                    (proc_table[prev].sched & ~PROC_SCHED_RH_MASK) |
                    (kword_t)cur;
        p->sched &= ~PROC_SCHED_RH_MASK;
}
#endif


#define PROC_REPORT_BITS \
        ((kword_t)PROC_REPORT_MASK << PROC_REPORT_SHIFT)

static void
proc_report_set(struct proc *p, unsigned int report)
{
        kword_t ctl;

        ctl = PROC_CTL_WORD(p);
        ctl &= ~PROC_REPORT_BITS;
        ctl |= ((kword_t)report & PROC_REPORT_MASK) << PROC_REPORT_SHIFT;
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

/* Canonical input storage is allocated only while a line is active.  Two
 * 18-bit managed-core bases share each pointer word, so all 21 terminals cost
 * only eleven fixed words rather than reserving a line array per terminal. */
#define PROC_TTY_LINE_PTR_WORDS ((PROC_TTY_COUNT + 1U) / 2U)
#define PROC_TTY_LINE_MM_OWNER_BASE 02000U
#define PROC_TTY_LINE_CHARS         120U
#define PROC_TTY_LINE_PACK          5U
#define PROC_TTY_LINE_DATA_WORDS \
        ((PROC_TTY_LINE_CHARS + PROC_TTY_LINE_PACK - 1U) / PROC_TTY_LINE_PACK)
#define PROC_TTY_LINE_WORDS         (1U + PROC_TTY_LINE_DATA_WORDS)
#define PROC_TTY_LINE_LEN_MASK      0377UL
#define PROC_TTY_LINE_DRAIN_SHIFT   8U
#define PROC_TTY_LINE_DRAIN_MASK    0377UL
#define PROC_TTY_LINE_READY         ((kword_t)1UL << 16U)
#define PROC_TTY_LINE_NL            ((kword_t)1UL << 17U)
#define PROC_TTY_LINE_EOF           ((kword_t)1UL << 18U)

#ifdef __PDP10__
kword_t proc_tty_line_bases[PROC_TTY_LINE_PTR_WORDS];
#else
static kword_t proc_tty_line_bases[PROC_TTY_LINE_PTR_WORDS];
#endif

static unsigned int
proc_tty_mode(unsigned int tty)
{
        return (unsigned int)((proc_tty_records[tty] >> PROC_TTY_MODE_SHIFT) &
            PROC_TTY_MODE_MASK);
}

#ifdef __PDP10__
extern kword_t proc_tty_line_base_get(unsigned int tty);
extern void proc_tty_line_base_set(unsigned int tty, kword_t base);
#else
static kword_t
proc_tty_line_base_get(unsigned int tty)
{
        kword_t word;

        word = proc_tty_line_bases[tty >> 1U];
        if ((tty & 1U) != 0U)
                return (word >> 18U) & PROC_HALF_MASK;
        return word & PROC_HALF_MASK;
}

static void
proc_tty_line_base_set(unsigned int tty, kword_t base)
{
        kword_t *word;

        word = &proc_tty_line_bases[tty >> 1U];
        base &= PROC_HALF_MASK;
        if ((tty & 1U) != 0U)
                *word = (*word & PROC_HALF_MASK) | (base << 18U);
        else
                *word = (*word & (PROC_HALF_MASK << 18U)) | base;
}

#endif

void
proc_tty_line_reset(unsigned int tty)
{
        kword_t base;

        if (tty >= PROC_TTY_COUNT)
                return;
        base = proc_tty_line_base_get(tty);
        if (base == 0UL)
                return;
        if (mm_free(base, MM_TYPE_KERNEL_DYNAMIC,
            PROC_TTY_LINE_MM_OWNER_BASE + tty) == MM_OK)
                proc_tty_line_base_set(tty, 0UL);
}

int
proc_tty_mode_set(unsigned int tty, unsigned int mode)
{
        kword_t record;

        if (tty >= PROC_TTY_COUNT || (mode & ~PROC_TTY_MODE_MASK) != 0U)
                return -1;
        if (proc_tty_mode(tty) == mode)
                return (int)mode;
        proc_tty_line_reset(tty);
        if (proc_tty_line_base_get(tty) != 0UL)
                return -1;
        record = proc_tty_records[tty];
        record &= ~(((kword_t)PROC_TTY_MODE_MASK << PROC_TTY_MODE_SHIFT) |
            PROC_TTY_CR_PENDING);
        record |= (kword_t)mode << PROC_TTY_MODE_SHIFT;
        proc_tty_records[tty] = record;
        return (int)mode;
}

#ifdef __PDP10__
kword_t *
#else
static kword_t *
#endif
proc_tty_line_ensure(unsigned int tty)
{
        kword_t base;
        kword_t *line;
        unsigned int i;

        base = proc_tty_line_base_get(tty);
        if (base == 0UL) {
                if (mm_alloc(PROC_TTY_LINE_WORDS, MM_TYPE_KERNEL_DYNAMIC,
                    PROC_TTY_LINE_MM_OWNER_BASE + tty, MM_ALLOC_LOW,
                    &base) != MM_OK)
                        return 0;
                line = (kword_t *)(unsigned long)base;
                for (i = 0U; i < PROC_TTY_LINE_WORDS; ++i)
                        line[i] = 0UL;
                proc_tty_line_base_set(tty, base);
        }
        return (kword_t *)(unsigned long)base;
}

#ifdef __PDP10__
extern void proc_tty_line_put(kword_t *line, unsigned int pos,
    unsigned int ch);
extern unsigned int proc_tty_line_get(kword_t *line, unsigned int pos);
#else
static void
proc_tty_line_put(kword_t *line, unsigned int pos, unsigned int ch)
{
        unsigned int wi;
        unsigned int shift;
        kword_t mask;

        wi = 1U + pos / PROC_TTY_LINE_PACK;
        shift = (PROC_TTY_LINE_PACK - 1U -
            (pos % PROC_TTY_LINE_PACK)) * 7U;
        mask = (kword_t)0177UL << shift;
        line[wi] = (line[wi] & ~mask) | (((kword_t)ch & 0177UL) << shift);
}

static unsigned int
proc_tty_line_get(kword_t *line, unsigned int pos)
{
        unsigned int wi;
        unsigned int shift;

        wi = 1U + pos / PROC_TTY_LINE_PACK;
        shift = (PROC_TTY_LINE_PACK - 1U -
            (pos % PROC_TTY_LINE_PACK)) * 7U;
        return (unsigned int)((line[wi] >> shift) & 0177UL);
}

#endif

#ifdef __PDP10__
void
#else
static void
#endif
proc_tty_echo(unsigned int tty, unsigned int ch)
{
#ifdef __PDP10__
        (void)native_sys_putchar_call(TTY_PACK(tty, ch));
#else
        (void)tty_putchar(TTY_PACK(tty, ch));
#endif
}

#ifdef __PDP10__
void
#else
static void
#endif
proc_tty_echo_erase(unsigned int tty)
{
        proc_tty_echo(tty, 010U);
        proc_tty_echo(tty, 040U);
        proc_tty_echo(tty, 010U);
}

/* Return one already-cooked byte, EOF, or INPUT_REPEAT when hardware input is
 * still required.  The first canonical read allocates its compact line block
 * here, before the caller enters a blocking hardware-input path. */
#ifndef __PDP10__
int
proc_tty_line_take(unsigned int tty)
{
        kword_t *line;
        kword_t header;
        unsigned int len;
        unsigned int drain;
        unsigned int ch;

        if (tty >= PROC_TTY_COUNT)
                return -1;
        if ((proc_tty_mode(tty) & PROC_TTY_MODE_CANONICAL) == 0U)
                return PROC_TTY_INPUT_REPEAT;
        line = proc_tty_line_ensure(tty);
        if (line == 0)
                return -1;
        header = line[0];
        if ((header & PROC_TTY_LINE_READY) == 0UL)
                return PROC_TTY_INPUT_REPEAT;
        len = (unsigned int)(header & PROC_TTY_LINE_LEN_MASK);
        drain = (unsigned int)((header >> PROC_TTY_LINE_DRAIN_SHIFT) &
            PROC_TTY_LINE_DRAIN_MASK);
        if (drain < len) {
                ch = proc_tty_line_get(line, drain++);
                line[0] = (header & ~((kword_t)PROC_TTY_LINE_DRAIN_MASK <<
                    PROC_TTY_LINE_DRAIN_SHIFT)) |
                    ((kword_t)drain << PROC_TTY_LINE_DRAIN_SHIFT);
                if (drain == len && (header & PROC_TTY_LINE_NL) == 0UL)
                        proc_tty_line_reset(tty);
                return (int)ch;
        }
        if ((header & PROC_TTY_LINE_NL) != 0UL) {
                proc_tty_line_reset(tty);
                return 012;
        }
        if ((header & PROC_TTY_LINE_EOF) != 0UL) {
                proc_tty_line_reset(tty);
                return PROC_TTY_INPUT_EOF;
        }
        proc_tty_line_reset(tty);
        return PROC_TTY_INPUT_EOF;
}

#endif

/* Process one byte after controlling-TTY and foreground-pgrp validation. */
#ifndef __PDP10__
int
proc_tty_canon_input(unsigned int tty, unsigned int ch)
{
        kword_t *line;
        kword_t header;
        unsigned int len;
        unsigned int mode;

        if (tty >= PROC_TTY_COUNT)
                return -1;
        mode = proc_tty_mode(tty);
        if ((mode & PROC_TTY_MODE_CANONICAL) == 0U)
                return (int)(ch & 0177U);
        ch &= 0177U;
        if ((proc_tty_records[tty] & PROC_TTY_CR_PENDING) != 0UL) {
                proc_tty_records[tty] &= ~PROC_TTY_CR_PENDING;
                if (ch == 012U)
                        return PROC_TTY_INPUT_REPEAT;
        }
        line = proc_tty_line_ensure(tty);
        if (line == 0)
                return -1;
        header = line[0];
        if ((header & PROC_TTY_LINE_READY) != 0UL)
                return proc_tty_line_take(tty);
        len = (unsigned int)(header & PROC_TTY_LINE_LEN_MASK);
        if (ch == 015U || ch == 012U) {
                if (ch == 015U)
                        proc_tty_records[tty] |= PROC_TTY_CR_PENDING;
                line[0] = header | PROC_TTY_LINE_READY | PROC_TTY_LINE_NL;
                if ((mode & PROC_TTY_MODE_ECHO) != 0U) {
                        proc_tty_echo(tty, 015U);
                        proc_tty_echo(tty, 012U);
                }
                return proc_tty_line_take(tty);
        }
        if (ch == 010U || ch == 0177U) {
                if (len != 0U) {
                        --len;
                        line[0] = (header & ~PROC_TTY_LINE_LEN_MASK) |
                            (kword_t)len;
                        if ((mode & PROC_TTY_MODE_ECHO) != 0U)
                                proc_tty_echo_erase(tty);
                }
                return PROC_TTY_INPUT_REPEAT;
        }
        if (ch == 025U) {
                if ((mode & PROC_TTY_MODE_ECHO) != 0U) {
                        while (len != 0U) {
                                --len;
                                proc_tty_echo_erase(tty);
                        }
                }
                line[0] = header & ~PROC_TTY_LINE_LEN_MASK;
                return PROC_TTY_INPUT_REPEAT;
        }
        if (ch == 004U) {
                if (len == 0U)
                        line[0] = header | PROC_TTY_LINE_READY |
                            PROC_TTY_LINE_EOF;
                else
                        line[0] = header | PROC_TTY_LINE_READY;
                return proc_tty_line_take(tty);
        }
        if (len >= PROC_TTY_LINE_CHARS) {
                if ((mode & PROC_TTY_MODE_ECHO) != 0U)
                        proc_tty_echo(tty, 007U);
                return PROC_TTY_INPUT_REPEAT;
        }
        proc_tty_line_put(line, len, ch);
        line[0] = (header & ~PROC_TTY_LINE_LEN_MASK) | (kword_t)(len + 1U);
        if ((mode & PROC_TTY_MODE_ECHO) != 0U && ch >= 040U && ch <= 0176U)
                proc_tty_echo(tty, ch);
        return PROC_TTY_INPUT_REPEAT;
}

#endif

extern int proc_tty_session_has(unsigned int session, unsigned int pgrp,
    unsigned int skip_slot);

void
proc_tty_release_session(unsigned int session, unsigned int leaving_slot)
{
        int i;

        if (session == 0U)
                return;
        if (proc_tty_session_has(session, 0U, leaving_slot))
                return;
        for (i = 0; i < (int)PROC_TTY_COUNT; ++i) {
                unsigned int record;

                record = (unsigned int)proc_tty_records[i];
                if (PROC_TTY_REC_SESSION(record) == session) {
                        proc_tty_line_reset((unsigned int)i);
                        proc_tty_records[i] = 0UL;
                }
        }
}


int
proc_slot_discard(unsigned int slot)
{
        struct proc *p;

#ifndef __PDP10__
        /* Target caller has already claimed this concrete slot. */
        if (proc_table == 0 || (int)slot <= 0 ||
            (int)slot >= (int)proc_slots)
                return -1;
#endif
        p = &proc_table[slot];
        if ((unsigned int)proc_rt_owner == slot)
                proc_rt_owner = 0UL;
        if (PROC_IS_FREE(p))
                return 0;
        if (vm_space_destroy(p, slot) != 0)
                return -1;
        proc_tty_release_session((unsigned int)(proc_scope_id(p) &
            PROC_ZOMB_SESSION_MASK), slot);
        if (proc_uarea_release(slot, p) != 0)
                return -1;
        proc_runq_remove(slot);
        proc_slot_zero(p);
        proc_trim_high();
        return 0;
}

extern void proc_notify_parent(unsigned int parent);

static void
proc_child_report(struct proc *child, unsigned int report)
{
        proc_report_set(child, report);
        proc_notify_parent(PROC_PARENT_SLOT(child));
}

static void
proc_adopt_children(unsigned int old_parent)
{
        int i;
        unsigned int new_parent;

        new_parent = 0U;
#ifdef __PDP10__
        if (old_parent != 1U && !PROC_IS_FREE_OR_ZOMB(&proc_table[1]))
                new_parent = 1U;
#else
        if (old_parent != 1U && proc_slots > 1U &&
            !PROC_IS_FREE_OR_ZOMB(&proc_table[1]))
                new_parent = 1U;
#endif
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
        unsigned int parent;
        unsigned int pgrp;
        kword_t scope;
        kword_t ctl;

        p = &proc_table[slot];
        if ((unsigned int)proc_rt_owner == slot)
                proc_rt_owner = 0UL;
        parent = PROC_PARENT_SLOT(p);
        pgrp = PROC_PGRP(p);
        ctl = PROC_CTL_WORD(p);
        scope = (ctl >> PROC_SESSION_SHIFT) & PROC_ZOMB_SCOPE_MASK;
        if (proc_session_teardown(slot, ctl) != 0)
                return -1;

        PROC_SET_TRANSITION(p);
        if (vm_space_destroy(p, slot) != 0) {
                PROC_CLEAR_TRANSITION(p);
                return -1;
        }
        proc_tty_release_session((unsigned int)(scope &
            PROC_ZOMB_SESSION_MASK), slot);
        if (proc_uarea_release(slot, p) != 0)
                return -1;
        proc_runq_remove(slot);
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

int
proc_event_apply(unsigned int slot, unsigned int event)
{
        struct proc *p;
        kword_t ctl;
        unsigned int state;

        p = &proc_table[slot];
        state = PROC_STATE(p);

        /* Fatal events are reported through the zombie wait status.  Their
         * pending bit would live only in the u-area released below, so do not
         * create dead state. */
        if (event <= SYS_EVENT_HUP || event == SYS_EVENT_PIPE) {
                struct file *saved;

                if (slot == (unsigned int)proc_current_slot) {
                        file_close_all();
                        proc_exit_current((int)(SYS_WAIT_EVENT_FLAG | event));
                        return -1;
                }
                saved = file_table;
                file_table = (struct file *)(unsigned long)
                    (PROC_UAREA_BASE(p) + PROC_FILE_TABLE_OFFSET);
                file_close_all();
                file_table = saved;
                return proc_finish_slot(slot, SYS_WAIT_EVENT_FLAG | event);
        }
        ctl = PROC_CTL_WORD(p);
        if (event == SYS_EVENT_PIPE)
                ctl |= PROC_PIPE_EVENT_BIT;
        else
                ctl |= (kword_t)SYS_EVENT_BIT(event) << PROC_EVENT_SHIFT;
        PROC_CTL_WORD(p) = ctl;
        if (event == SYS_EVENT_TSTP) {
                if ((ctl & PROC_STOP_JOB_BIT) != 0UL)
                        return 0;
                PROC_CTL_WORD(p) = ctl | PROC_STOP_JOB_BIT;
                proc_runq_remove(slot);
                PROC_SET_STATE(p, PROC_STOP);
                proc_child_report(p, PROC_REPORT_STOPPED);
                if (slot == (unsigned int)proc_current_slot)
                        proc_sched_resched_current();
                return 0;
        }
        if (event == SYS_EVENT_CONT) {
                if ((ctl & PROC_STOP_JOB_BIT) == 0UL)
                        return 0;
                ctl &= ~PROC_STOP_JOB_BIT;
                PROC_CTL_WORD(p) = ctl;
                if ((ctl & PROC_STOP_BITS) == 0UL && state == PROC_STOP) {
                        if (PROC_WAIT_CLASS(p) == PROC_WAIT_NONE) {
                                p->sched &= ~PROC_CPU_SLEEP_BITS;
                                PROC_SET_STATE(p, PROC_SRUN);
                                proc_runq_add(slot);
                        } else {
                                PROC_SET_STATE(p, PROC_SLEEP);
                        }
                }
                proc_child_report(p, PROC_REPORT_CONTINUED);
                return 0;
        }
        /* ALRM interrupts only explicit user waits.  Internal event waits
         * remain noninterruptible.  A stopped continuation stays stopped;
         * CONT later resumes it at the interrupted syscall return. */
        if (event == SYS_EVENT_ALRM &&
            (PROC_WAIT_CLASS(p) == PROC_WAIT_CHILD ||
            PROC_WAIT_CLASS(p) == PROC_WAIT_INTR)) {
                p->sched &= ~(PROC_WAIT_BITS | PROC_SCHED_RH_MASK |
                    ((kword_t)01UL << PROC_STATE_SHIFT) |
                    PROC_CPU_SLEEP_BITS);
                if (state == PROC_SLEEP)
                        proc_runq_add(slot);
        }
        return 0;
}

#ifndef __PDP10__
/* Return the logical terminal which supplies the current process input.
 * A process with no controlling terminal retains the historical CTY fallback
 * used during early userspace bootstrap.  DETACHED is deliberately an error. */
int
proc_tty_read_enter(void)
{
        struct proc *p;

        p = &proc_table[(unsigned int)proc_current_slot];
        for (;;) {
                unsigned int state;
                unsigned int tty;
                kword_t record;
                unsigned int session;
                unsigned int pgrp;

                state = PROC_TTY_STATE(p);
                if (state == PROC_TTY_NO_TTY)
                        return 0;
                if (state < PROC_TTY_ATTACHED_BASE ||
                    state >= PROC_TTY_ATTACHED_BASE + PROC_TTY_COUNT)
                        return -1;
                tty = state - PROC_TTY_ATTACHED_BASE;
                record = proc_tty_records[tty];
                session = PROC_SESSION(p);
                if (PROC_TTY_REC_SESSION(record) != session)
                        return -1;
                pgrp = PROC_PGRP(p);
                if (PROC_TTY_REC_PGRP(record) == pgrp)
                        return (int)tty;
                if (proc_event_send(pgrp, SYS_EVENT_TSTP, 1) != 0)
                        return -1;
                /* A self TSTP requests an immediate PI6 reschedule.  This
                 * continuation resumes here after CONT and rechecks whether
                 * the group actually owns the terminal before consuming input. */
        }
}

int
proc_tty_input(unsigned int tty, unsigned int ch)
{
        struct proc *p;
        unsigned int state;
        kword_t record;
        unsigned int pgrp;

        if (tty >= PROC_TTY_COUNT)
                return -1;
        p = &proc_table[(unsigned int)proc_current_slot];
        state = PROC_TTY_STATE(p);
        if (state == PROC_TTY_NO_TTY) {
                if (tty != 0U)
                        return -1;
                return (int)(ch & 0177U);
        }
        if (state != PROC_TTY_ATTACHED_BASE + tty)
                return -1;
        record = proc_tty_records[tty];
        if (PROC_TTY_REC_SESSION(record) != PROC_SESSION(p))
                return -1;
        pgrp = PROC_TTY_REC_PGRP(record);
        if (pgrp == 0U || pgrp != PROC_PGRP(p))
                return -1;
        ch &= 0177U;
        if ((proc_tty_mode(tty) & PROC_TTY_MODE_SIGNALS) != 0U) {
                if (ch == 003U) {
                        (void)proc_event_send(pgrp, SYS_EVENT_INT, 1);
                        return PROC_TTY_INPUT_REPEAT;
                }
                if (ch == 032U) {
                        if (proc_event_send(pgrp, SYS_EVENT_TSTP, 1) != 0)
                                return -1;
                        return PROC_TTY_INPUT_REPEAT;
                }
        }
        if ((proc_tty_mode(tty) & PROC_TTY_MODE_CANONICAL) != 0U)
                return proc_tty_canon_input(tty, ch);
        return (int)ch;
}

/* Bind terminal output to the process controlling TTY.  Output is not gated
 * by the foreground pgrp (DAIMOS has no TOSTOP mode); it is only required to
 * belong to the controlling session.  NO_TTY keeps the bootstrap CTY path. */
int
proc_tty_output(unsigned int ch)
{
        struct proc *p;
        unsigned int state;
        unsigned int tty;
        kword_t record;

        p = &proc_table[(unsigned int)proc_current_slot];
        state = PROC_TTY_STATE(p);
        if (state == PROC_TTY_NO_TTY)
                return (int)TTY_PACK(TTY_ID_CTY, ch);
        if (state < PROC_TTY_ATTACHED_BASE ||
            state >= PROC_TTY_ATTACHED_BASE + PROC_TTY_COUNT)
                return -1;
        tty = state - PROC_TTY_ATTACHED_BASE;
        record = proc_tty_records[tty];
        if (PROC_TTY_REC_SESSION(record) != PROC_SESSION(p))
                return -1;
        return (int)TTY_PACK(tty, ch);
}

/* DCS and GE each have one hardware scanner, but several logical terminals.
 * Their MRES readers defer an input byte for another line into otherwise
 * unused high bits of that terminal's existing session record.  Encoding
 * byte+1 leaves zero as the empty marker and costs no additional per-TTY RAM. */
int
proc_tty_pending_take(unsigned int tty)
{
        kword_t record;
        unsigned int encoded;

        if (tty >= PROC_TTY_COUNT)
                return -1;
        record = proc_tty_records[tty];
        encoded = (unsigned int)((record >> PROC_TTY_PENDING_SHIFT) &
            PROC_TTY_PENDING_MASK);
        if (encoded == 0U)
                return -1;
        record &= ~((kword_t)PROC_TTY_PENDING_MASK <<
            PROC_TTY_PENDING_SHIFT);
        proc_tty_records[tty] = record;
        return (int)(encoded - 1U);
}

int
proc_tty_pending_store(unsigned int tty, unsigned int ch)
{
        kword_t record;
        kword_t field;

        if (tty >= PROC_TTY_COUNT || ch > TTY_DATA_MASK)
                return -1;
        record = proc_tty_records[tty];
        field = (record >> PROC_TTY_PENDING_SHIFT) & PROC_TTY_PENDING_MASK;
        if (field != 0UL)
                return -1;
        record |= ((kword_t)(ch + 1U) & PROC_TTY_PENDING_MASK) <<
            PROC_TTY_PENDING_SHIFT;
        proc_tty_records[tty] = record;
        return 0;
}
#endif

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
                                        proc_report_set(child, PROC_REPORT_NONE);
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

#ifndef __PDP10__
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
#endif

#ifndef __PDP10__
int
proc_nice_value(int slot)
{
        if (proc_table == 0 || slot <= 0 || slot >= (int)proc_slots ||
            PROC_IS_FREE(&proc_table[slot]))
                return 0;
        return (int)PROC_NICE_ENCODED(&proc_table[slot]) -
            (int)PROC_NICE_BIAS;
}
#endif

int
proc_nice_current(int value)
{
        int slot;
        struct proc *p;
        unsigned int encoded;

        slot = (int)proc_current_slot;
#ifndef __PDP10__
        if (proc_table == 0 || slot == 0 || slot >= (int)proc_slots)
                return -1;
#endif
        if (value < PROC_NICE_MIN)
                value = PROC_NICE_MIN;
        if (value > PROC_NICE_MAX)
                value = PROC_NICE_MAX;
        p = &proc_table[slot];
        encoded = (unsigned int)(value + (int)PROC_NICE_BIAS);
        p->sched = (p->sched &
            ~((kword_t)PROC_NICE_MASK << PROC_NICE_SHIFT)) |
            ((kword_t)encoded << PROC_NICE_SHIFT);
        return value;
}


static unsigned int
proc_select_runnable(int elapsed_ticks)
{
        int best;
        int best_prio;
        int best_rank;
        int cur;
        int slot;

        if (proc_table == 0 || (int)proc_high_slot <= 1)
                return 0U;
        best = 0;
        best_prio = 0;
        best_rank = 0;
        cur = (int)(proc_sched_cursor & PROC_PGRP_MASK);
        /* Sleep age is used only by swap victim selection.  It advances on
         * the original 64-clock boundary, so the descriptor table is touched
         * once per 64 elapsed ticks rather than once per scheduling quantum. */
        if (elapsed_ticks != 0) {
                proc_sched_age_phase += (unsigned int)elapsed_ticks;
                if (proc_sched_age_phase >= 64U) {
                        struct proc *p;

                        proc_sched_age_phase -= 64U;
                        p = &proc_table[1];
                        for (slot = 1; slot < (int)proc_high_slot;
                            ++slot, ++p) {
                                if (PROC_STATE(p) == PROC_SLEEP &&
                                    PROC_SLEEP_AGE(p) < PROC_SLEEP_MASK)
                                        p->sched +=
                                            (kword_t)1UL << PROC_SLEEP_SHIFT;
                        }
                }
        }

        if (proc_rt_owner != 0UL) {
                struct proc *p;

                p = &proc_table[(unsigned int)proc_rt_owner];
                if (PROC_STATE(p) == PROC_SRUN && !PROC_TRANSITION(p)) {
                        proc_sched_cursor = proc_rt_owner;
                        if (!VM_SPACE_ACTIVE(p)) {
                                proc_sched_cursor |= PROC_SCHED_SWAP_REQUEST;
                                return 0U;
                        }
                        return (unsigned int)proc_rt_owner;
                }
        }

        /* Recent CPU is relevant only while a process competes to run.
         * Account exactly the elapsed clock ticks while walking the run queue:
         * the current job accumulates penalty, competitors decay.  Blocking
         * wakeups clear dynamic CPU/sleep accounting before requeueing. */
        slot = (int)(proc_runq_head & PROC_SCHED_RH_MASK);
        while (slot != 0) {
                struct proc *p;
                int prio;
                int rank;

                p = &proc_table[slot];
                if (elapsed_ticks != 0) {
                        int cpu;

                        cpu = (int)PROC_CPU_PENALTY(p);
                        if (slot == (int)proc_current_slot &&
                            !PROC_TRANSITION(p)) {
                                if (cpu == 0)
                                        ++cpu;
                                cpu += elapsed_ticks;
                                if (cpu > (int)PROC_CPU_MASK)
                                        cpu = (int)PROC_CPU_MASK;
                        } else if (cpu > elapsed_ticks) {
                                cpu -= elapsed_ticks;
                        } else {
                                cpu = 0;
                        }
                        p->sched = (p->sched &
                            ~((kword_t)PROC_CPU_MASK << PROC_CPU_SHIFT)) |
                            ((kword_t)(unsigned int)cpu << PROC_CPU_SHIFT);
                }
                if (!PROC_TRANSITION(p) &&
                    (VM_SPACE_ACTIVE(p) || PROC_SWAP_RECORD_PRESENT(slot))) {
                        prio = (int)PROC_EFFECTIVE(p);
                        /* Slots fit in eight bits.  Mod-256 distance preserves
                         * cyclic slot order; distance zero is the current slot
                         * and therefore sorts after every other candidate. */
                        rank = (slot - cur) & (int)PROC_PGRP_MASK;
                        if (rank == 0)
                                rank = (int)PROC_PGRP_MASK + 1;
                        if (best == 0 || prio < best_prio ||
                            (prio == best_prio && rank < best_rank)) {
                                best = slot;
                                best_prio = prio;
                                best_rank = rank;
                        }
                }
                slot = (int)PROC_RUNQ_NEXT(p);
        }
        if (best != 0) {
                proc_sched_cursor = (kword_t)best;
                if (!VM_SPACE_ACTIVE(&proc_table[best])) {
                        proc_sched_cursor |= PROC_SCHED_SWAP_REQUEST;
                        return 0U;
                }
        }
        return (unsigned int)best;
}

unsigned int
proc_sched_resched_select(void)
{
        int elapsed_ticks;

        elapsed_ticks = (int)proc_sched_deferred_ticks;
        proc_sched_deferred_ticks = 0UL;
        return proc_select_runnable(elapsed_ticks);
}

unsigned int
proc_sched_tick_select(void)
{
        int elapsed_ticks;

        elapsed_ticks = (int)proc_sched_deferred_ticks;
        proc_sched_deferred_ticks = 0UL;
        if (elapsed_ticks == 0)
                elapsed_ticks = 1;
        return proc_select_runnable(elapsed_ticks);
}

int
proc_swap_victim(unsigned int exclude_owner)
{
        struct proc *p;
        int exclude;
        int i;
        int best;
        int best_score;

#ifndef __PDP10__
        if (proc_table == 0)
                return -1;
#endif
        exclude = (int)exclude_owner;
        best = 0;
        best_score = 0;
        p = &proc_table[1];
        for (i = 1; i < (int)proc_high_slot; ++i, ++p) {
                unsigned int state;
                int score;
                int nice;

                if (i == exclude || i == (int)proc_current_slot)
                        continue;
                state = PROC_STATE(p);
                if ((state != PROC_SLEEP && state != PROC_STOP &&
                    state != PROC_SRUN) || PROC_TRANSITION(p) ||
                    (PROC_HAS_UAREA(p) && PROC_USER_MAPPING_HELD(p)))
                        continue;
                if (!vm_space_can_swap(p))
                        continue;

                nice = (int)PROC_NICE_ENCODED(p) - (int)PROC_NICE_BIAS;
                if (state == PROC_SLEEP || state == PROC_STOP) {
                        score = 04000 + (int)PROC_SLEEP_AGE(p) * 0100;
                        if (PROC_WAIT_CLASS(p) != PROC_WAIT_NONE)
                                score += 040;
                } else {
                        score = 01000 + (int)PROC_EFFECTIVE(p);
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
