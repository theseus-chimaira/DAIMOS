#ifndef DAIMON_PROCFS_V1_H
#define DAIMON_PROCFS_V1_H

#include "vfs_v1.h"

#define PROCFS_V1_PROVIDER              3U
#define PROCFS_V1_KIND_ROOT             1U
#define PROCFS_V1_KIND_PROC             2U
#define PROCFS_V1_KIND_PPID             3U
#define PROCFS_V1_KIND_STATE            4U
#define PROCFS_V1_KIND_WORDS            5U
#define PROCFS_V1_KIND_COMM             6U

#define PROCFS_V1_FIELD_PID             1U
#define PROCFS_V1_FIELD_PPID            2U
#define PROCFS_V1_FIELD_STATE           3U
#define PROCFS_V1_FIELD_WORDS           4U
#define PROCFS_V1_FIELD_COMM            5U

/* Return zero for a live slot/field and nonzero for an unavailable slot. */
typedef int (*procfs_v1_get_fn)(unsigned int slot, unsigned int field,
    kword_t *valuep);

void procfs_v1_init(unsigned int slots, procfs_v1_get_fn getfn);
vnode_v1_t procfs_v1_root(void);
int procfs_v1_lookup(vnode_v1_t dir, const struct vfs_v1_name *name,
    vnode_v1_t *nodep);
int procfs_v1_readdir(vnode_v1_t dir, unsigned int off,
    struct vfs_v1_dirent *ent);
int procfs_v1_stat(vnode_v1_t node, struct vfs_v1_stat *st);
int procfs_v1_pid(unsigned int slot, kword_t *pidp);
int procfs_v1_readchar(vnode_v1_t node, kword_t off, unsigned int *chp);

#endif
