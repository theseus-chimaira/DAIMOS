#ifndef DAIMON_DEVICEFS_V1_H
#define DAIMON_DEVICEFS_V1_H

#include "vfs_v1.h"

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

#define DEVICEFS_V1_CLASS_CHAR          1U
#define DEVICEFS_V1_CLASS_CLOCK         2U
#define DEVICEFS_V1_CLASS_DISPLAY       3U
#define DEVICEFS_V1_CLASS_BLOCK         4U
#define DEVICEFS_V1_CLASS_CONTROLLER    5U
#define DEVICEFS_V1_CLASS_MOUNTSRC      6U

#define DEVICEFS_V1_UNIT_NONE           0U
#define DEVICEFS_V1_UNIT_CHAR           1U
#define DEVICEFS_V1_UNIT_WORD           2U

struct devicefs_v1_desc {
        kword_t name6;
        kword_t meta;
};

#define DEVICEFS_V1_META(chars, class_id, unit_id) \
        (((kword_t)(chars) & 077UL) | (((kword_t)(class_id) & 077UL) << 6) | \
        (((kword_t)(unit_id) & 077UL) << 12))
#define DEVICEFS_V1_META_CHARS(meta) \
        ((unsigned int)((meta) & 077UL))
#define DEVICEFS_V1_META_CLASS(meta) \
        ((unsigned int)(((meta) >> 6) & 077UL))
#define DEVICEFS_V1_META_UNIT(meta) \
        ((unsigned int)(((meta) >> 12) & 077UL))

extern kword_t devicefs_v1_present;
int devicefs_v1_set_present(unsigned int id, int present);
vnode_v1_t devicefs_v1_root(void);
int devicefs_v1_lookup(vnode_v1_t dir, const struct vfs_v1_name *name,
    vnode_v1_t *nodep);
int devicefs_v1_readdir(vnode_v1_t dir, unsigned int off,
    struct vfs_v1_dirent *ent);
int devicefs_v1_stat(vnode_v1_t node, struct vfs_v1_stat *st);
int devicefs_v1_device_id(vnode_v1_t node, unsigned int *idp);
int devicefs_v1_readchar(vnode_v1_t node, kword_t off, unsigned int *chp);

#endif
