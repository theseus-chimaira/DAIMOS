#ifndef DAIMON_PROC_V1_H
#define DAIMON_PROC_V1_H

#include "kcore.h"

#ifndef PROC_V1_NPROC
#define PROC_V1_NPROC 64U
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
#define PROC_V1_CRED_MASK      0777UL
#define PROC_V1_PARENT_SHIFT   8U
#define PROC_V1_STATE_SHIFT    15U
#define PROC_V1_UID_SHIFT      18U
#define PROC_V1_GID_SHIFT      27U
#define PROC_V1_HALF_MASK      0777777UL
#define PROC_V1_HALF_SHIFT     18U
#define PROC_V1_CTX_WORDS      3U

struct proc_v1 {
        kword_t meta;
        kword_t mem_layout;
        kword_t exec;
};

extern struct proc_v1 proc_v1_table[PROC_V1_NPROC];
extern struct proc_v1 *proc_v1_current;

#define PROC_V1_PID(p) ((unsigned int)((p)->meta & PROC_V1_PID_MASK))
#define PROC_V1_PARENT_SLOT(p) \
        ((unsigned int)(((p)->meta >> PROC_V1_PARENT_SHIFT) & PROC_V1_PARENT_MASK))
#define PROC_V1_STATE(p) \
        ((unsigned int)(((p)->meta >> PROC_V1_STATE_SHIFT) & PROC_V1_STATE_MASK))
#define PROC_V1_UID(p) \
        ((unsigned int)(((p)->meta >> PROC_V1_UID_SHIFT) & PROC_V1_CRED_MASK))
#define PROC_V1_GID(p) \
        ((unsigned int)(((p)->meta >> PROC_V1_GID_SHIFT) & PROC_V1_CRED_MASK))
#define PROC_V1_MEM_BASE(p) ((kword_t)((p)->mem_layout & PROC_V1_HALF_MASK))
#define PROC_V1_MEM_WORDS(p) \
        ((kword_t)(((p)->mem_layout >> PROC_V1_HALF_SHIFT) & PROC_V1_HALF_MASK))
#define PROC_V1_ENTRY(p) ((kword_t)((p)->exec & PROC_V1_HALF_MASK))
#define PROC_V1_STACK(p) \
        ((kword_t)(((p)->exec >> PROC_V1_HALF_SHIFT) & PROC_V1_HALF_MASK))

void proc_v1_init(void);
struct proc_v1 *proc_v1_alloc_init(void);
struct proc_v1 *proc_v1_alloc_child(struct proc_v1 *parent);
void proc_v1_reap(struct proc_v1 *p);
unsigned int proc_v1_slot(const struct proc_v1 *p);
unsigned int proc_v1_ppid(const struct proc_v1 *p);
void proc_v1_set_state(struct proc_v1 *p, unsigned int state);
void proc_v1_set_cred(struct proc_v1 *p, unsigned int uid, unsigned int gid);
void proc_v1_set_memory(struct proc_v1 *p, kword_t base, kword_t words);
void proc_v1_set_exec(struct proc_v1 *p, kword_t entry, kword_t stack);
int proc_v1_procfs_get(unsigned int slot, unsigned int field, kword_t *valuep);

#endif
