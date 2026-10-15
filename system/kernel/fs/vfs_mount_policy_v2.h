/* Staged indexed mount policy; not installed in active KCORE yet. */
#ifndef DAIMON_VFS_MOUNT_POLICY_V2_H
#define DAIMON_VFS_MOUNT_POLICY_V2_H

#include "kcore.h"

#define VFS_POLICY_V2_MAX_MOUNTS 17U
#define VFS_POLICY_V2_RDONLY 1U
#define VFS_POLICY_V2_SWAP 2U
#define VFS_POLICY_V2_LOGSTORE 4U

/* Three 36-bit maps, one bit per slot and property. */
struct vfs_mount_policy_v2 {
        kword_t readonly;
        kword_t swap;
        kword_t logstore;
        unsigned int capacity;
};

int vfs_policy_v2_init(struct vfs_mount_policy_v2 *p,
    unsigned int capacity);
int vfs_policy_v2_get(const struct vfs_mount_policy_v2 *p,
    unsigned int slot, unsigned int *flagsp);
int vfs_policy_v2_reserve(struct vfs_mount_policy_v2 *p,
    unsigned int slot, unsigned int flags);
int vfs_policy_v2_release(struct vfs_mount_policy_v2 *p,
    unsigned int slot, unsigned int flags);
int vfs_policy_v2_toggle_readonly(struct vfs_mount_policy_v2 *p,
    unsigned int slot);
int vfs_policy_v2_clear(struct vfs_mount_policy_v2 *p,
    unsigned int slot);

#endif
