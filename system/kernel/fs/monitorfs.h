#ifndef DAIMON_MONITORFS_H
#define DAIMON_MONITORFS_H

#include "vfs.h"

#define MONITORFS_DEVICE_PROVIDER     2U
#define MONITORFS_PROCESS_PROVIDER    3U
#define MONITORFS_KIND_ROOT           1U
#define MONITORFS_KIND_DEVICE         2U
#define MONITORFS_KIND_DEVDIR        3U
#define MONITORFS_KIND_CTYDIR         MONITORFS_KIND_DEVDIR
#define MONITORFS_KIND_MEMBERS        5U
#define MONITORFS_KIND_SWAP_STATS     6U
#define MONITORFS_KIND_IOROOT         0U

/* D6SET SWAP is live allocation state, not lifetime I/O accounting; it
 * contains total, used, and free blocks.  DAIMOS 0.9/1.0 deliberately omits
 * per-device lifetime read/write/error counters from resident memory. */

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
#define MONITORFS_DEV_TTYDPY0         19U
#define MONITORFS_DEV_COUNT           20U

#define MONITORFS_PRESENT(id)         (1UL << (id))

extern kword_t mfsdev_names[MONITORFS_DEV_COUNT];
extern kword_t mfsdev_d6set_members;
int mfsproc_lookup(vnode_t dir, const struct vfs_name *name,
    vnode_t *nodep);
int mfsproc_readdir(vnode_t dir, unsigned int off, struct vfs_dirent *ent);
int mfsproc_stat(vnode_t node, struct vfs_stat *st);
int mfsdev_lookup(vnode_t dir, const struct vfs_name *name,
    vnode_t *nodep);
int mfsdev_readdir(vnode_t dir, unsigned int off,
    struct vfs_dirent *ent);
int mfsdev_stat(vnode_t node, struct vfs_stat *st);
int mfsdev_readchar(vnode_t node, kword_t off, unsigned int *chp);
int mfsproc_read_words(vnode_t node, kword_t off, kword_t *buf,
    unsigned int nwords);

/* Provider 3 uses three object kinds only.  Leaf identity is uniformly
 * encoded in the provider-private 18-bit key rather than vnode kind bits:
 * bits 0..7 hold the process/domain id, bits 8..10 hold the leaf selector,
 * and bit 17 selects the domain namespace.  This keeps canonical vnode kinds
 * <= 3 so struct file metadata packing remains unchanged. */
#define MONITORFS_PROCESS_KIND_ROOT   1U
#define MONITORFS_PROCESS_KIND_DIR    2U
#define MONITORFS_PROCESS_KIND_FILE   3U
#define MONITORFS_PROCESS_ID_MASK     0377U
#define MONITORFS_PROCESS_LEAF_SHIFT     8U
#define MONITORFS_PROCESS_LEAF_MASK      07U
#define MONITORFS_DOMAIN_TAG          0400000U
#define MONITORFS_PROCESS_ID(node) (VFS_INDEX(node) & MONITORFS_PROCESS_ID_MASK)
#define MONITORFS_PROCESS_LEAF(node) \
        ((VFS_INDEX(node) >> MONITORFS_PROCESS_LEAF_SHIFT) & \
        MONITORFS_PROCESS_LEAF_MASK)
#define MONITORFS_PROCESS_INDEX(slot, leaf) \
        (((slot) & MONITORFS_PROCESS_ID_MASK) | \
        (((leaf) & MONITORFS_PROCESS_LEAF_MASK) << \
        MONITORFS_PROCESS_LEAF_SHIFT))
#define MONITORFS_DOMAIN_ID(node) MONITORFS_PROCESS_ID(node)
#define MONITORFS_IS_DOMAIN(node) ((VFS_INDEX(node) & MONITORFS_DOMAIN_TAG) != 0U)

#define MONITORFS_PROCESS_LEAF_PPID         0U
#define MONITORFS_PROCESS_LEAF_STATE        1U
#define MONITORFS_PROCESS_LEAF_WORDS        2U
#define MONITORFS_PROCESS_LEAF_NAME         3U
#define MONITORFS_PROCESS_LEAF_CMDLINE      4U
#define MONITORFS_PROC_LEAF_ENV             5U

#define MONITORFS_DOMAIN_LEAF_PROCESSES   0U
#define MONITORFS_DOMAIN_LEAF_WORDS       1U
#define MONITORFS_DOMAIN_LEAF_SWAPPED     2U
#define MONITORFS_DOMAIN_LEAF_SWAPWORDS   3U
#define MONITORFS_DOMAIN_LEAF_STOPPED     4U
#define MONITORFS_DOMAIN_LEAF_PIDS        5U

#endif
