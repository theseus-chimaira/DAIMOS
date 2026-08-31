#ifndef DAIMON_PROC_V1_H
#define DAIMON_PROC_V1_H

#include "kcore.h"

#ifndef PROC_V1_NPROC
#define PROC_V1_NPROC 2U
#endif
#if PROC_V1_NPROC > 128U
#error "PROC_V1_NPROC must be <= 128"
#endif

#define PROC_V1_FREE   0U
#define PROC_V1_SIDL   1U
#define PROC_V1_SRUN   2U
#define PROC_V1_SLEEP  3U
#define PROC_V1_ZOMB   4U

#define PROC_V1_PID_MASK       0377UL
#define PROC_V1_PARENT_MASK    0177UL
#define PROC_V1_STATE_MASK     07UL
#define PROC_V1_PARENT_SHIFT   8U
#define PROC_V1_STATE_SHIFT    15U
#define PROC_V1_HALF_MASK      0777777UL
#define PROC_V1_HALF_SHIFT     18U
#define PROC_V1_ENTRY_SHIFT    18U
#define PROC_V1_CTX_WORDS      3U

struct proc_v1 {
        kword_t meta;
        kword_t mem_layout;
};

extern struct proc_v1 proc_v1_table[PROC_V1_NPROC];
extern kword_t proc_v1_wait_channel;

/*
 * V1 blocking uses an event word as its wait channel.  A zero event means
 * pending; the producer stores a nonzero completion value before wakeup.
 * The recheck after publishing the wait channel closes the completion race.
 * V1 has one schedulable user context, so one wait-channel word is sufficient.
 */
int proc_v1_wait_event(volatile kword_t *eventp);
void proc_v1_wakeup_event(volatile kword_t *eventp);

#define PROC_V1_PID(p) ((unsigned int)((p)->meta & PROC_V1_PID_MASK))
#define PROC_V1_PARENT_SLOT(p) \
        ((unsigned int)(((p)->meta >> PROC_V1_PARENT_SHIFT) & PROC_V1_PARENT_MASK))
#define PROC_V1_STATE(p) \
        ((unsigned int)(((p)->meta >> PROC_V1_STATE_SHIFT) & PROC_V1_STATE_MASK))
#define PROC_V1_MEM_BASE(p) ((kword_t)((p)->mem_layout & PROC_V1_HALF_MASK))
#define PROC_V1_MEM_WORDS(p) \
        ((kword_t)(((p)->mem_layout >> PROC_V1_HALF_SHIFT) & PROC_V1_HALF_MASK))
#define PROC_V1_ENTRY(p) \
        ((kword_t)(((p)->meta >> PROC_V1_ENTRY_SHIFT) & PROC_V1_HALF_MASK))


#endif
