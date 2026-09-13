#ifndef DAIMON_PROC_H
#define DAIMON_PROC_H

#include "kcore.h"

/*
 * Process slots are a logical resource, not a promise that every process is
 * resident.  The table is allocated after physical-core discovery and is
 * sized 64/128/256 slots.  All interfaces use an 8-bit slot/PID field so the
 * architectural maximum is fixed at 256 even on smaller machines.
 */
#define PROC_MAX_SLOTS       256U
#define PROC_MIN_SLOTS        64U
#define PROC_WORDS             3U
#define PROC_NO_SLOT         0400U

#define PROC_FREE   0U
#define PROC_SIDL   1U
#define PROC_SRUN   2U
#define PROC_SLEEP  3U
#define PROC_ZOMB   4U
#define PROC_SSWAP  5U        /* reserved compatibility state; not steady-state */
#define PROC_STOP   6U

/* meta RH: pgrp:8, parent-slot:8, flags:2.  meta LH retains initial entry PC. */
#define PROC_PGRP_MASK       0377UL
#define PROC_PGRP_SHIFT          0U
#define PROC_PARENT_MASK     0377UL
#define PROC_PARENT_SHIFT       8U
#define PROC_FLAGS_MASK         03UL
#define PROC_FLAGS_SHIFT        16U
#define PROC_F_TRANSITION       01U
#define PROC_F_UAREA            02U
#define PROC_HALF_MASK      0777777UL
#define PROC_HALF_SHIFT         18U
#define PROC_ENTRY_SHIFT        18U

/*
 * sched LH: nice:6, recent-CPU:4, sleep-age:3, wait-class:2, state:3.
 * sched RH is the wait channel.  Nice is stored biased by 20.  The small
 * recent-CPU and sleep-age fields deliberately saturate; the scheduler is
 * intended to stay cheap enough for a rotating process-table scan.
 */
#define PROC_NICE_MIN       (-20)
#define PROC_NICE_MAX          19
#define PROC_NICE_BIAS         20U
#define PROC_NICE_MASK         077UL
#define PROC_NICE_SHIFT        18U
#define PROC_CPU_MASK          017UL
#define PROC_CPU_SHIFT         24U
#define PROC_SLEEP_MASK        07UL
#define PROC_SLEEP_SHIFT       28U
#define PROC_WAIT_MASK         03UL
#define PROC_WAIT_SHIFT        31U
#define PROC_STATE_MASK        07UL
#define PROC_STATE_SHIFT       33U
#define PROC_SCHED_RH_MASK  PROC_HALF_MASK
#define PROC_STATE_BITS \
        ((kword_t)PROC_STATE_MASK << PROC_STATE_SHIFT)
#define PROC_STATE_LO_BITS \
        ((kword_t)03UL << PROC_STATE_SHIFT)
#define PROC_SCHED_DEFAULT \
        ((kword_t)PROC_NICE_BIAS << PROC_NICE_SHIFT)

#define PROC_WAIT_NONE      0U
#define PROC_WAIT_EVENT     1U
#define PROC_WAIT_CHILD     2U
#define PROC_WAIT_INTR      3U

/*
 * Each active process allocates a stable executive u-area from
 * kernel-dynamic core.  It contains saved CPU/syscall context followed by that
 * process's private cwd/file table and kernel stack.  The u-area never moves
 * with the relocatable
 * user extent and remains resident while the process is swapped, so a sleeping
 * executive continuation cannot retain stale physical stack addresses.
 *
 * meta LH initially carries the executable entry PC.  proc_user_context_init()
 * consumes that value and repurposes the same half-word as the stable u-area
 * physical base; the three-word process descriptor therefore does not grow.
 */
#define PROC_UAREA_WORDS        0420UL
/*
 * One compact control word precedes cwd/file state.  Descriptors 0..15 are
 * ordinary two-word FILE records; low control bits 0..2 are now spare.
 * Session and domain IDs share this already-resident word; process-group ID
 * lives in meta RH so it survives after EXIT releases the u-area and group WAIT can
 * reap zombies.
 */
#define PROC_FDCTL_OFFSET        0045UL
#define PROC_FILE_CWD_OFFSET     0046UL
#define PROC_FILE_TABLE_OFFSET   0047UL
#define PROC_USTACK_BASE         0107UL
#define PROC_SESSION_MASK         0377UL
#define PROC_SESSION_SHIFT            3U
#define PROC_DOMAIN_MASK          0377UL
#define PROC_DOMAIN_SHIFT            11U
#define PROC_EVENT_MASK           0177UL
#define PROC_EVENT_SHIFT             19U
/* Event 7 uses one spare low control-word bit; events 0..6 stay packed. */
#define PROC_PIPE_EVENT_BIT            01UL
#define PROC_STOP_MASK              03UL
#define PROC_STOP_SHIFT              26U
#define PROC_STOP_JOB                01U
#define PROC_STOP_MM                 02U
#define PROC_REPORT_MASK             03UL
#define PROC_REPORT_SHIFT               28U
#define PROC_REPORT_NONE              0U
#define PROC_REPORT_STOPPED           1U
#define PROC_REPORT_CONTINUED         2U
/* Top six control-word bits encode NO_TTY, DETACHED, or tty-id+2. */
#define PROC_TTY_MASK                 077UL
#define PROC_TTY_SHIFT                   30U
#define PROC_TTY_NO_TTY                 0U
#define PROC_TTY_DETACHED               1U
#define PROC_TTY_ATTACHED_BASE          2U
#define PROC_TTY_COUNT                  21U
#define PROC_ZOMB_SESSION_MASK       0377UL
#define PROC_ZOMB_SESSION_SHIFT          0U
#define PROC_ZOMB_DOMAIN_MASK        0377UL
#define PROC_ZOMB_DOMAIN_SHIFT           8U
#define PROC_ZOMB_SCOPE_MASK \
        (PROC_ZOMB_SESSION_MASK | \
        (PROC_ZOMB_DOMAIN_MASK << PROC_ZOMB_DOMAIN_SHIFT))
#define PROC_KSTACK_WORDS \
        (PROC_UAREA_WORDS - PROC_USTACK_BASE)

struct proc {
        kword_t meta;
        kword_t vm_state;       /* LH user words, RH backend-private VM state. */
        kword_t sched;
};

extern struct proc *proc_table;
extern unsigned int proc_slots;
extern unsigned int proc_high_slot;
extern kword_t proc_current_slot;
extern kword_t proc_sched_cursor;
extern kword_t mach_kernel_stack_base;

int proc_boot_init(void);
unsigned int proc_slots_for_core(kword_t core_words);
int proc_slot_claim(unsigned int parent_slot);
int proc_exit_finish(int status);
int proc_event_apply(unsigned int slot, unsigned int event);
void proc_exit_current(int status);
int proc_slot_discard(unsigned int slot);
int proc_child_hierarchy(unsigned int child_slot, unsigned int mode,
    unsigned int requested_pgrp);
int proc_tty_read_enter(void);
int proc_tty_input(unsigned int ch);
void proc_sched_resched_current(void);
kword_t proc_scope_id(const struct proc *p);
int proc_wait_child(void);
kword_t proc_comm(const struct proc *p);
int proc_wait_event(volatile kword_t *eventp);
int proc_wait_event_intr(volatile kword_t *eventp);
void proc_wakeup_event(volatile kword_t *eventp);
unsigned int proc_sched_tick_select(void);
unsigned int proc_sched_resched_select(void);
int proc_nice_value(int slot);
int proc_nice_current(int value);
int proc_swap_victim(unsigned int exclude_owner);
int proc_user_context_init(unsigned int slot, kword_t entry, kword_t stack,
    kword_t ac1, kword_t ac2, kword_t ac3);
void proc_sched_pi_tick(void);

#define PROC_PGRP(p) \
        ((unsigned int)(((p)->meta >> PROC_PGRP_SHIFT) & PROC_PGRP_MASK))
#define PROC_PARENT_SLOT(p) \
        ((unsigned int)(((p)->meta >> PROC_PARENT_SHIFT) & PROC_PARENT_MASK))
#define PROC_FLAGS(p) \
        ((unsigned int)(((p)->meta >> PROC_FLAGS_SHIFT) & PROC_FLAGS_MASK))
#define PROC_TRANSITION(p) \
        (((p)->meta & ((kword_t)PROC_F_TRANSITION << PROC_FLAGS_SHIFT)) != 0UL)
#define PROC_HAS_UAREA(p) \
        (((p)->meta & ((kword_t)PROC_F_UAREA << PROC_FLAGS_SHIFT)) != 0UL)
#define PROC_META_LH(p) \
        ((kword_t)(((p)->meta >> PROC_HALF_SHIFT) & PROC_HALF_MASK))
#define PROC_UAREA_BASE(p) PROC_META_LH(p)
#define PROC_ENTRY(p) PROC_META_LH(p)
#define PROC_EXIT_STATUS(p) PROC_META_LH(p)
#define PROC_SET_META_LH(p, v) \
        ((p)->meta = ((p)->meta & PROC_HALF_MASK) | \
        (((kword_t)(v) & PROC_HALF_MASK) << PROC_HALF_SHIFT))
#define PROC_SET_PGRP(p, v) \
        ((p)->meta = ((p)->meta & \
        ~((kword_t)PROC_PGRP_MASK << PROC_PGRP_SHIFT)) | \
        (((kword_t)(v) & PROC_PGRP_MASK) << PROC_PGRP_SHIFT))
#define PROC_SET_PARENT_SLOT(p, v) \
        ((p)->meta = ((p)->meta & \
        ~((kword_t)PROC_PARENT_MASK << PROC_PARENT_SHIFT)) | \
        (((kword_t)(v) & PROC_PARENT_MASK) << PROC_PARENT_SHIFT))
#define PROC_UAREA_WORD(p, off) \
        (((kword_t *)(unsigned long)PROC_UAREA_BASE(p))[(off)])
#define PROC_CTL_WORD(p) PROC_UAREA_WORD((p), PROC_FDCTL_OFFSET)
#define PROC_SESSION(p) \
        ((unsigned int)((PROC_CTL_WORD(p) >> PROC_SESSION_SHIFT) & \
        PROC_SESSION_MASK))
#define PROC_DOMAIN(p) \
        ((unsigned int)((PROC_CTL_WORD(p) >> PROC_DOMAIN_SHIFT) & \
        PROC_DOMAIN_MASK))
#define PROC_EVENTS(p) \
        ((unsigned int)(((PROC_CTL_WORD(p) >> PROC_EVENT_SHIFT) & \
        PROC_EVENT_MASK) | \
        ((PROC_CTL_WORD(p) & PROC_PIPE_EVENT_BIT) != 0UL ? 0200U : 0U)))
#define PROC_STOP_REASONS(p) \
        ((unsigned int)((PROC_CTL_WORD(p) >> PROC_STOP_SHIFT) & \
        PROC_STOP_MASK))
#define PROC_WAIT_REPORT(p) \
        ((unsigned int)((PROC_CTL_WORD(p) >> PROC_REPORT_SHIFT) & \
        PROC_REPORT_MASK))
#define PROC_TTY_STATE(p) \
        ((unsigned int)((PROC_CTL_WORD(p) >> PROC_TTY_SHIFT) & \
        PROC_TTY_MASK))
#define PROC_NICE_ENCODED(p) \
        ((unsigned int)(((p)->sched >> PROC_NICE_SHIFT) & PROC_NICE_MASK))
#define PROC_CPU_PENALTY(p) \
        ((unsigned int)(((p)->sched >> PROC_CPU_SHIFT) & PROC_CPU_MASK))
#define PROC_SLEEP_AGE(p) \
        ((unsigned int)(((p)->sched >> PROC_SLEEP_SHIFT) & PROC_SLEEP_MASK))
#define PROC_WAIT_CLASS(p) \
        ((unsigned int)(((p)->sched >> PROC_WAIT_SHIFT) & PROC_WAIT_MASK))
#define PROC_STATE(p) \
        ((unsigned int)(((p)->sched >> PROC_STATE_SHIFT) & PROC_STATE_MASK))
#define PROC_IS_FREE(p) \
        (((p)->sched & PROC_STATE_BITS) == 0UL)
#define PROC_IS_FREE_OR_ZOMB(p) \
        (((p)->sched & PROC_STATE_LO_BITS) == 0UL)
#define PROC_WAIT_CHANNEL(p) ((kword_t)((p)->sched & PROC_SCHED_RH_MASK))
#define PROC_ZOMB_SESSION(p) \
        ((unsigned int)(((p)->sched >> PROC_ZOMB_SESSION_SHIFT) & \
        PROC_ZOMB_SESSION_MASK))
#define PROC_ZOMB_DOMAIN(p) \
        ((unsigned int)(((p)->sched >> PROC_ZOMB_DOMAIN_SHIFT) & \
        PROC_ZOMB_DOMAIN_MASK))

#define PROC_SET_STATE(p, s) \
        ((p)->sched = ((p)->sched & ~PROC_STATE_BITS) | \
        ((kword_t)((s) & PROC_STATE_MASK) << PROC_STATE_SHIFT))
#define PROC_SET_TRANSITION(p) \
        ((p)->meta |= ((kword_t)PROC_F_TRANSITION << PROC_FLAGS_SHIFT))
#define PROC_CLEAR_TRANSITION(p) \
        ((p)->meta &= ~((kword_t)PROC_F_TRANSITION << PROC_FLAGS_SHIFT))

#endif
