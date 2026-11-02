/**
 * @file proc.c
 * @brief Resident process, scheduler, session, event, and TTY policy.
 *
 * The three-word process table keeps only compact identity, VM, scheduling,
 * and zombie state.  Per-process executive state lives in a stable dynamically
 * allocated u-area so user VM extents may move or swap without invalidating a
 * sleeping kernel continuation.  Hand-written PDP-6 context switching, event
 * wakeup, process-control, and cooked-TTY hot paths live in proc_pdp6.s; this
 * file supplies the policy and resource-lifetime operations that are clearer
 * and smaller in C.
 */
#include "proc.h"
#include "fs_mres.h"
#include "mm.h"
#include "vm.h"
#include "proc_swap.h"
#include "monitorfs.h"
#include "syscall.h"
#include "tty.h"
#include "exec.h"

/* Slot indexes are 1..PROC_MAX_SLOTS-1 and scheduler scores are small
 * positive values.  Signed locals avoid PDP-10 unsigned-compare glue while
 * preserving the externally stored unsigned slot/count representation. */

unsigned int proc_sched_age_phase;
extern int native_sys_putchar_call(kword_t tty_char);
extern int proc_has_live_user(void);


/* One compact record per logical TTY.  The extra ten words versus half-word
 * packing save substantially more KCORE instructions in every control path. */
kword_t proc_tty_records[PROC_TTY_COUNT];

/* Canonical input storage is allocated only while a line is active.  Two
 * 18-bit managed-core bases share each pointer word, so all 21 terminals cost
 * only eleven fixed words rather than reserving a line array per terminal. */
#define PROC_TTY_LINE_PTR_WORDS ((PROC_TTY_COUNT + 1U) / 2U)
#define PROC_TTY_LINE_MM_OWNER_BASE 02000U
#define PROC_TTY_LINE_CHARS         120U
#define PROC_TTY_LINE_PACK          6U
#define PROC_TTY_LINE_DATA_WORDS \
        ((PROC_TTY_LINE_CHARS + PROC_TTY_LINE_PACK - 1U) / PROC_TTY_LINE_PACK)
#define PROC_TTY_LINE_WORDS         (1U + PROC_TTY_LINE_DATA_WORDS)
#define PROC_TTY_LINE_LEN_MASK      0377UL
#define PROC_TTY_LINE_DRAIN_SHIFT   8U
#define PROC_TTY_LINE_DRAIN_MASK    0377UL
#define PROC_TTY_LINE_READY         ((kword_t)1UL << 16U)
#define PROC_TTY_LINE_NL            ((kword_t)1UL << 17U)
#define PROC_TTY_LINE_EOF           ((kword_t)1UL << 18U)

kword_t proc_tty_line_bases[PROC_TTY_LINE_PTR_WORDS];

#define PROC_TTY_MODE(tty) \
        ((unsigned int)((proc_tty_records[(tty)] >> PROC_TTY_MODE_SHIFT) & \
        PROC_TTY_MODE_MASK))

extern kword_t proc_tty_line_base_get(unsigned int tty);
extern void proc_tty_line_base_set(unsigned int tty, kword_t base);

/** Free one TTY's dynamically allocated canonical-input line buffer. */
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

/**
 * Attempt to retire a canonical input buffer before changing TTY ownership.
 *
 * The physical MM extent can be pinned or its owner metadata can reject
 * mm_free(). A failed release keeps proc_tty_line_bases[] intact. In that
 * case a caller must NOT transfer the TTY to another session: the stale
 * SIXBIT input would become readable by an unrelated login.
 *
 * No new memory is allocated; a later detach/attach may retry safely.
 */
int
proc_tty_line_try_reset_v897(unsigned int tty)
{
        if (tty >= PROC_TTY_COUNT)
                return -1;
        proc_tty_line_reset(tty);
        return proc_tty_line_base_get(tty) == 0UL ? 0 : -1;
}

/** Change TTY input mode, discarding any partially cooked input line. */
int
proc_tty_mode_set(unsigned int tty, unsigned int mode)
{
        kword_t record;

        if (tty >= PROC_TTY_COUNT || (mode & ~PROC_TTY_MODE_MASK) != 0U)
                return -1;
        if (PROC_TTY_MODE(tty) == mode)
                return (int)mode;
        if (proc_tty_line_try_reset_v897(tty) != 0)
                return -1;
        record = proc_tty_records[tty];
        record &= ~(((kword_t)PROC_TTY_MODE_MASK << PROC_TTY_MODE_SHIFT) |
            PROC_TTY_CR_PENDING);
        record |= (kword_t)mode << PROC_TTY_MODE_SHIFT;
        proc_tty_records[tty] = record;
        return (int)mode;
}

/** Allocate and zero the compact canonical-input buffer for one TTY on demand. */
kword_t *
proc_tty_line_ensure(unsigned int tty)
{
        kword_t base;
        kword_t *line;

        base = proc_tty_line_base_get(tty);
        if (base == 0UL) {
                if (mm_alloc(PROC_TTY_LINE_WORDS, MM_TYPE_KERNEL_DYNAMIC,
                    PROC_TTY_LINE_MM_OWNER_BASE + tty, MM_ALLOC_LOW,
                    &base) != MM_OK)
                        return 0;
                line = (kword_t *)(unsigned long)base;
                fs_zero_words(line, PROC_TTY_LINE_WORDS);
                proc_tty_line_base_set(tty, base);
        }
        return (kword_t *)(unsigned long)base;
}

extern void proc_tty_line_put(kword_t *line, unsigned int pos,
    unsigned int ch);
extern unsigned int proc_tty_line_get(kword_t *line, unsigned int pos);

/** Emit one character through the resident TTY output service. */
void
proc_tty_echo(unsigned int tty, unsigned int ch)
{
        (void)native_sys_putchar_call(TTY_PACK(tty, ch));
}

/** Echo the conventional backspace-space-backspace erase sequence. */
void
proc_tty_echo_erase(unsigned int tty)
{
        proc_tty_echo(tty, 010U);
        proc_tty_echo(tty, 040U);
        proc_tty_echo(tty, 010U);
}

/** Finish EXIT from the permanent idle stack after switching away from
 * the dying process's private u-area; assembler owns finalization/retry. */
int
proc_exit_finish(int status)
{
        unsigned int slot;
        struct proc *p;

        slot = (unsigned int)proc_current_slot;
        proc_current_slot = 0UL;
        proc_current_ptr = 0;
        if (proc_finish_slot_owner_v932(slot, (unsigned int)status) != 0) {
                p = &proc_table[slot];
                if (PROC_TRANSITION(p) && PROC_HAS_UAREA(p) &&
                    (PROC_FINALIZE_WORD(p) & PROC_FINALIZE_MARK) != 0UL &&
                    (p->vm_state == ~(kword_t)0 ||
                     (PROC_FINALIZE_WORD(p) & PROC_FINALIZE_PREVM) != 0UL))
                        return 1; /* slot 0 owns the deferred finalizer */
                return -1;
        }
        return proc_has_live_user();
}
