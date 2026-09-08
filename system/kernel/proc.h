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

/* meta RH: pid:8, parent-slot:8, flags:2.  meta LH retains initial entry PC. */
#define PROC_PID_MASK        0377UL
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
#define PROC_SCHED_DEFAULT \
        ((kword_t)PROC_NICE_BIAS << PROC_NICE_SHIFT)

#define PROC_WAIT_NONE      0U
#define PROC_WAIT_EVENT     1U
#define PROC_WAIT_CHILD     2U
#define PROC_WAIT_TTY       3U

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
#define PROC_UAREA_WORDS        0435UL
#define PROC_KCTX_WORDS         0060UL
#define PROC_FILE_CWD_OFFSET     0045UL
#define PROC_FILE_TABLE_OFFSET   0046UL
#define PROC_USTACK_BASE         0115UL
#define PROC_KSTACK_WORDS \
        (PROC_UAREA_WORDS - PROC_USTACK_BASE)

struct proc {
        kword_t meta;
        kword_t mem_layout;     /* LH user/protected words, RH physical base. */
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
int proc_slot_release(unsigned int slot);
int proc_exit_finish(void);
kword_t proc_comm(const struct proc *p);
int proc_wait_event(volatile kword_t *eventp);
void proc_wakeup_event(volatile kword_t *eventp);
unsigned int proc_sched_tick_select(void);
unsigned int proc_sched_resched_select(void);
int proc_nice_current(int value);
int proc_nice_value(unsigned int slot);
int proc_swap_victim(unsigned int exclude_owner);
int proc_user_context_init(unsigned int slot, kword_t entry, kword_t stack,
    kword_t ac1, kword_t ac2, kword_t ac3);
void proc_sched_pi_tick(void);

#define PROC_PID(p) ((unsigned int)((p)->meta & PROC_PID_MASK))
#define PROC_PARENT_SLOT(p) \
        ((unsigned int)(((p)->meta >> PROC_PARENT_SHIFT) & PROC_PARENT_MASK))
#define PROC_FLAGS(p) \
        ((unsigned int)(((p)->meta >> PROC_FLAGS_SHIFT) & PROC_FLAGS_MASK))
#define PROC_TRANSITION(p) ((PROC_FLAGS(p) & PROC_F_TRANSITION) != 0U)
#define PROC_HAS_UAREA(p) ((PROC_FLAGS(p) & PROC_F_UAREA) != 0U)
#define PROC_MEM_BASE(p) ((kword_t)((p)->mem_layout & PROC_HALF_MASK))
#define PROC_MEM_WORDS(p) \
        ((kword_t)(((p)->mem_layout >> PROC_HALF_SHIFT) & PROC_HALF_MASK))
#define PROC_RESIDENT_WORDS(p) PROC_MEM_WORDS(p)
#define PROC_META_LH(p) \
        ((kword_t)(((p)->meta >> PROC_HALF_SHIFT) & PROC_HALF_MASK))
#define PROC_UAREA_BASE(p) PROC_META_LH(p)
#define PROC_ENTRY(p) PROC_META_LH(p)
#define PROC_SET_META_LH(p, v) \
        ((p)->meta = ((p)->meta & PROC_HALF_MASK) | \
        (((kword_t)(v) & PROC_HALF_MASK) << PROC_HALF_SHIFT))
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
#define PROC_WAIT_CHANNEL(p) ((kword_t)((p)->sched & PROC_SCHED_RH_MASK))

#define PROC_SET_STATE(p, s) \
        ((p)->sched = ((p)->sched & ~PROC_STATE_BITS) | \
        ((kword_t)((s) & PROC_STATE_MASK) << PROC_STATE_SHIFT))
#define PROC_SET_TRANSITION(p) \
        ((p)->meta |= ((kword_t)PROC_F_TRANSITION << PROC_FLAGS_SHIFT))
#define PROC_CLEAR_TRANSITION(p) \
        ((p)->meta &= ~((kword_t)PROC_F_TRANSITION << PROC_FLAGS_SHIFT))
#define PROC_SET_MEM_BASE(p, b) \
        ((p)->mem_layout = ((p)->mem_layout & \
        ((kword_t)PROC_HALF_MASK << PROC_HALF_SHIFT)) | \
        ((kword_t)(b) & PROC_HALF_MASK))

#endif
