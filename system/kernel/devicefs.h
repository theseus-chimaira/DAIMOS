#ifndef DAIMON_DEVICEFS_V1_H
#define DAIMON_DEVICEFS_V1_H

#include "vfs.h"

#define DEVICEFS_V1_PROVIDER            2U
#define DEVICEFS_V1_KIND_ROOT           1U
#define DEVICEFS_V1_KIND_DEVICE         2U
#define DEVICEFS_V1_KIND_CTYDIR         3U
#define DEVICEFS_V1_KIND_IN             4U
#define DEVICEFS_V1_KIND_OUT            5U

#define DEVICEFS_V1_DEV_CTY0            0U
#define DEVICEFS_V1_DEV_CLK0            1U
#define DEVICEFS_V1_DEV_PTR0            2U
#define DEVICEFS_V1_DEV_PTP0            3U
#define DEVICEFS_V1_DEV_CR0             4U
#define DEVICEFS_V1_DEV_CP0             5U
#define DEVICEFS_V1_DEV_DCS0            6U
#define DEVICEFS_V1_DEV_GE0             7U
#define DEVICEFS_V1_DEV_DPY0            8U
#define DEVICEFS_V1_DEV_TTY0            9U
#define DEVICEFS_V1_DEV_WCNSLS          10U
#define DEVICEFS_V1_DEV_OCNSLS          11U
#define DEVICEFS_V1_DEV_DTC0            12U
#define DEVICEFS_V1_DEV_MTC0            13U
#define DEVICEFS_V1_DEV_DSK0            14U
#define DEVICEFS_V1_DEV_SLV0            15U
#define DEVICEFS_V1_DEV_D6SET0          16U
#define DEVICEFS_V1_DEV_COUNT           17U

#define DEVICEFS_V1_PRESENT(id)         (1UL << (id))

#define DEVICEFS_V1_IO_IN_CTY0          0U
#define DEVICEFS_V1_IO_IN_PTR0          1U
#define DEVICEFS_V1_IO_IN_CR0           2U
#define DEVICEFS_V1_IO_IN_DCS0          3U
#define DEVICEFS_V1_IO_IN_GE0           4U
#define DEVICEFS_V1_IO_IN_WCNSLS        5U
#define DEVICEFS_V1_IO_IN_OCNSLS        6U
#define DEVICEFS_V1_IO_IN_DTC0          7U
#define DEVICEFS_V1_IO_IN_MTC0          8U
#define DEVICEFS_V1_IO_IN_DSK0          9U
#define DEVICEFS_V1_IO_IN_COUNT         10U

#define DEVICEFS_V1_IO_OUT_CTY0         0U
#define DEVICEFS_V1_IO_OUT_PTP0         1U
#define DEVICEFS_V1_IO_OUT_CP0          2U
#define DEVICEFS_V1_IO_OUT_DCS0         3U
#define DEVICEFS_V1_IO_OUT_GE0          4U
#define DEVICEFS_V1_IO_OUT_DPY0         5U
#define DEVICEFS_V1_IO_OUT_WCNSLS       6U
#define DEVICEFS_V1_IO_OUT_DTC0         7U
#define DEVICEFS_V1_IO_OUT_MTC0         8U
#define DEVICEFS_V1_IO_OUT_DSK0         9U
#define DEVICEFS_V1_IO_OUT_COUNT        10U

extern kword_t devicefs_present;
int devicefs_lookup(vnode_t dir, const struct vfs_name *name,
    vnode_t *nodep);
int devicefs_readdir(vnode_t dir, unsigned int off,
    struct vfs_dirent *ent);
int devicefs_stat(vnode_t node, struct vfs_stat *st);
int devicefs_readchar(vnode_t node, kword_t off, unsigned int *chp);

#endif
