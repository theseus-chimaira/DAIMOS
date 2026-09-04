#ifndef DAIMON_DEVICEFS_H
#define DAIMON_DEVICEFS_H

#include "vfs.h"

#define DEVICEFS_PROVIDER            2U
#define DEVICEFS_KIND_ROOT           1U
#define DEVICEFS_KIND_DEVICE         2U
#define DEVICEFS_KIND_DEVDIR        3U
#define DEVICEFS_KIND_CTYDIR         DEVICEFS_KIND_DEVDIR
#define DEVICEFS_KIND_IN             4U
#define DEVICEFS_KIND_OUT            5U

#define DEVICEFS_DEV_CTY0            0U
#define DEVICEFS_DEV_CLK0            1U
#define DEVICEFS_DEV_PTR0            2U
#define DEVICEFS_DEV_PTP0            3U
#define DEVICEFS_DEV_CR0             4U
#define DEVICEFS_DEV_CP0             5U
#define DEVICEFS_DEV_DCS0            6U
#define DEVICEFS_DEV_GE0             7U
#define DEVICEFS_DEV_DPY0            8U
#define DEVICEFS_DEV_TTY0            9U
#define DEVICEFS_DEV_WCNSLS          10U
#define DEVICEFS_DEV_OCNSLS          11U
#define DEVICEFS_DEV_DTC0            12U
#define DEVICEFS_DEV_MTC0            13U
#define DEVICEFS_DEV_DSK0            14U
#define DEVICEFS_DEV_SLV0            15U
#define DEVICEFS_DEV_D6SET0          16U
#define DEVICEFS_DEV_COUNT           17U

#define DEVICEFS_PRESENT(id)         (1UL << (id))

#define DEVICEFS_IO_IN_COUNT         DEVICEFS_DEV_COUNT
#define DEVICEFS_IO_OUT_COUNT        DEVICEFS_DEV_COUNT


extern kword_t devicefs_present;
int devicefs_lookup(vnode_t dir, const struct vfs_name *name,
    vnode_t *nodep);
int devicefs_readdir(vnode_t dir, unsigned int off,
    struct vfs_dirent *ent);
int devicefs_stat(vnode_t node, struct vfs_stat *st);
int devicefs_readchar(vnode_t node, kword_t off, unsigned int *chp);

#endif
