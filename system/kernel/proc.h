#ifndef DAIMON_PROC_H
#define DAIMON_PROC_H

#include "kcore.h"

#ifndef PROC_NPROC
#define PROC_NPROC 2U
#endif
#if PROC_NPROC > 128U
#error "PROC_NPROC must be <= 128"
#endif

#define PROC_FREE   0U
#define PROC_SIDL   1U
#define PROC_SRUN   2U
#define PROC_SLEEP  3U
#define PROC_ZOMB   4U
#define PROC_SSWAP  5U

#define PROC_PID_MASK       0377UL
#define PROC_PARENT_MASK    0177UL
#define PROC_STATE_MASK     07UL
#define PROC_PARENT_SHIFT   8U
#define PROC_STATE_SHIFT    15U
#define PROC_HALF_MASK      0777777UL
#define PROC_HALF_SHIFT     18U
#define PROC_ENTRY_SHIFT    18U
#define PROC_CTX_WORDS      3U
#define PROC_STATE_BITS \
        ((kword_t)PROC_STATE_MASK << PROC_STATE_SHIFT)

struct proc {
        kword_t meta;
        kword_t mem_layout;
};

extern struct proc proc_table[PROC_NPROC];
extern kword_t proc_wait_channel;

/*
 * V1 blocking uses an event word as its wait channel.  A zero event means
 * pending; the producer stores a nonzero completion value before wakeup.
 * The recheck after publishing the wait channel closes the completion race.
 * V1 has one schedulable user context, so one wait-channel word is sufficient.
 */
int proc_wait_event(volatile kword_t *eventp);
void proc_wakeup_event(volatile kword_t *eventp);

#define PROC_PID(p) ((unsigned int)((p)->meta & PROC_PID_MASK))
#define PROC_PARENT_SLOT(p) \
        ((unsigned int)(((p)->meta >> PROC_PARENT_SHIFT) & PROC_PARENT_MASK))
#define PROC_STATE(p) \
        ((unsigned int)(((p)->meta >> PROC_STATE_SHIFT) & PROC_STATE_MASK))
#define PROC_MEM_BASE(p) ((kword_t)((p)->mem_layout & PROC_HALF_MASK))
#define PROC_MEM_WORDS(p) \
        ((kword_t)(((p)->mem_layout >> PROC_HALF_SHIFT) & PROC_HALF_MASK))
#define PROC_ENTRY(p) \
        ((kword_t)(((p)->meta >> PROC_ENTRY_SHIFT) & PROC_HALF_MASK))
#define PROC_SET_STATE(p, s) \
        ((p)->meta = ((p)->meta & ~PROC_STATE_BITS) | \
        ((kword_t)((s) & PROC_STATE_MASK) << PROC_STATE_SHIFT))
#define PROC_SET_MEM_BASE(p, b) \
        ((p)->mem_layout = ((p)->mem_layout & \
        ((kword_t)PROC_HALF_MASK << PROC_HALF_SHIFT)) | \
        ((kword_t)(b) & PROC_HALF_MASK))


#endif
