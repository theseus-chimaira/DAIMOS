#ifndef DAIMON_PROCFS_H
#define DAIMON_PROCFS_H

#include "vfs.h"

#define PROCFS_PROVIDER              3U
#define PROCFS_KIND_ROOT             1U
#define PROCFS_KIND_PROC             2U
#define PROCFS_KIND_PPID             3U
#define PROCFS_KIND_STATE            4U
#define PROCFS_KIND_WORDS            5U
#define PROCFS_KIND_COMM             6U
#define PROCFS_KIND_STATUS           7U

#define PROCFS_FIELD_PID             1U
#define PROCFS_FIELD_PPID            2U
#define PROCFS_FIELD_STATE           3U
#define PROCFS_FIELD_WORDS           4U
#define PROCFS_FIELD_COMM            5U

int procfs_lookup(vnode_t dir, const struct vfs_name *name,
    vnode_t *nodep);
int procfs_readdir(vnode_t dir, unsigned int off,
    struct vfs_dirent *ent);
int procfs_stat(vnode_t node, struct vfs_stat *st);
int procfs_readchar(vnode_t node, kword_t off, unsigned int *chp);

#endif
