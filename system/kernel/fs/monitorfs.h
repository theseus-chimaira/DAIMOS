#ifndef DAIMON_MONITORFS_H
#define DAIMON_MONITORFS_H

#include "vfs.h"

#define MONITORFS_DEVICE_PROVIDER     2U
#define MONITORFS_PROCESS_PROVIDER    3U
#define MONITORFS_KIND_ROOT           1U
#define MONITORFS_KIND_DEVICE         2U
#define MONITORFS_KIND_DEVDIR        3U
#define MONITORFS_KIND_CTYDIR         MONITORFS_KIND_DEVDIR
#define MONITORFS_KIND_STATS          4U
#define MONITORFS_KIND_MEMBERS        5U
#define MONITORFS_KIND_SWAP_STATS     6U
#define MONITORFS_KIND_LOG_STATS      7U
#define MONITORFS_KIND_IOROOT         0U

/* STATS files contain unlabeled 12-digit octal values, one per line.
 * Stream/special: reads, writes, errors.
 * Storage/aggregate: reads, writes, native units read, native units written,
 * errors.  Native units are DTC/D6SET/DRM blocks, MTC words, and DSK sectors.
 * D6SET LOG uses the five-line storage order.
 * D6SET SWAP contains three current-state lines: total, used, free blocks.
 */

#define MONITORFS_DEV_CTY0            0U
#define MONITORFS_DEV_CLK0            1U
#define MONITORFS_DEV_PTR0            2U
#define MONITORFS_DEV_PTP0            3U
#define MONITORFS_DEV_CR0             4U
#define MONITORFS_DEV_CP0             5U
#define MONITORFS_DEV_DCS0            6U
#define MONITORFS_DEV_GE0             7U
#define MONITORFS_DEV_DPY0            8U
#define MONITORFS_DEV_TTY0            9U
#define MONITORFS_DEV_WCNSLS          10U
#define MONITORFS_DEV_OCNSLS          11U
#define MONITORFS_DEV_DTC0            12U
#define MONITORFS_DEV_MTC0            13U
#define MONITORFS_DEV_DSK0            14U
#define MONITORFS_DEV_SLV0            15U
#define MONITORFS_DEV_D6SET0          16U
#define MONITORFS_DEV_DRM0            17U
#define MONITORFS_DEV_LPT0            18U
#define MONITORFS_DEV_COUNT           19U

#define MONITORFS_PRESENT(id)         (1UL << (id))

#define MONITORFS_IO_IN_COUNT         MONITORFS_DEV_COUNT
#define MONITORFS_IO_OUT_COUNT        MONITORFS_DEV_COUNT

extern kword_t mfsdev_names[MONITORFS_DEV_COUNT];
extern kword_t mfsdev_d6set_members;
int mfsdev_lookup(vnode_t dir, const struct vfs_name *name,
    vnode_t *nodep);
int mfsdev_readdir(vnode_t dir, unsigned int off,
    struct vfs_dirent *ent);
int mfsdev_stat(vnode_t node, struct vfs_stat *st);
int mfsdev_readchar(vnode_t node, kword_t off, unsigned int *chp);

/* Process and domain views use the compact provider-3 vnode encoding. */
#define MONITORFS_PROCESS_KIND_ROOT   1U
#define MONITORFS_PROCESS_KIND_PROC   2U
#define MONITORFS_PROCESS_KIND_PPID   3U
#define MONITORFS_PROCESS_KIND_STATE  4U
#define MONITORFS_PROCESS_KIND_WORDS  5U
#define MONITORFS_PROCESS_KIND_COMM   6U
#define MONITORFS_PROCESS_KIND_STATUS 7U
#define MONITORFS_DOMAIN_TAG          0400000U
#define MONITORFS_DOMAIN_ID(node) (VFS_INDEX(node) & ~MONITORFS_DOMAIN_TAG)
#define MONITORFS_IS_DOMAIN(node) ((VFS_INDEX(node) & MONITORFS_DOMAIN_TAG) != 0U)

#endif
