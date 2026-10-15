/* Staged per-mount bitmap policy. No allocations, no side effects on failure. */
#include "vfs_mount_policy_v2.h"

#define VFS_POLICY_V2_ALL (VFS_POLICY_V2_RDONLY | VFS_POLICY_V2_SWAP | \
    VFS_POLICY_V2_LOGSTORE)

static unsigned int
vfs_policy_v2_valid(const struct vfs_mount_policy_v2 *p, unsigned int slot)
{
        return p != 0 && p->capacity != 0U &&
            p->capacity <= VFS_POLICY_V2_MAX_MOUNTS && slot < p->capacity;
}

int
vfs_policy_v2_init(struct vfs_mount_policy_v2 *p, unsigned int capacity)
{
        if (p == 0 || capacity == 0U ||
            capacity > VFS_POLICY_V2_MAX_MOUNTS)
                return -1;
        p->readonly = 0UL;
        p->swap = 0UL;
        p->logstore = 0UL;
        p->capacity = capacity;
        return 0;
}

int
vfs_policy_v2_get(const struct vfs_mount_policy_v2 *p,
    unsigned int slot, unsigned int *flagsp)
{
        kword_t bit;
        unsigned int flags;

        if (!vfs_policy_v2_valid(p, slot) || flagsp == 0)
                return -1;
        bit = 1UL << slot;
        flags = 0U;
        if ((p->readonly & bit) != 0UL)
                flags |= VFS_POLICY_V2_RDONLY;
        if ((p->swap & bit) != 0UL)
                flags |= VFS_POLICY_V2_SWAP;
        if ((p->logstore & bit) != 0UL)
                flags |= VFS_POLICY_V2_LOGSTORE;
        *flagsp = flags;
        return 0;
}

int
vfs_policy_v2_reserve(struct vfs_mount_policy_v2 *p,
    unsigned int slot, unsigned int flags)
{
        kword_t bit;

        if (!vfs_policy_v2_valid(p, slot) ||
            (flags & ~VFS_POLICY_V2_ALL) != 0U)
                return -1;
        bit = 1UL << slot;
        /* SWAP and LOGSTORE are exclusive across all mounts, including a
         * repeated reservation on the current mount. */
        if (((flags & VFS_POLICY_V2_SWAP) != 0U && p->swap != 0UL) ||
            ((flags & VFS_POLICY_V2_LOGSTORE) != 0U &&
            p->logstore != 0UL))
                return -1;
        if ((flags & VFS_POLICY_V2_RDONLY) != 0U)
                p->readonly |= bit;
        if ((flags & VFS_POLICY_V2_SWAP) != 0U)
                p->swap |= bit;
        if ((flags & VFS_POLICY_V2_LOGSTORE) != 0U)
                p->logstore |= bit;
        return 0;
}

int
vfs_policy_v2_release(struct vfs_mount_policy_v2 *p,
    unsigned int slot, unsigned int flags)
{
        kword_t bit;

        if (!vfs_policy_v2_valid(p, slot) ||
            (flags & ~(VFS_POLICY_V2_SWAP | VFS_POLICY_V2_LOGSTORE)) != 0U)
                return -1;
        bit = 1UL << slot;
        if (((flags & VFS_POLICY_V2_SWAP) != 0U &&
            (p->swap & bit) == 0UL) ||
            ((flags & VFS_POLICY_V2_LOGSTORE) != 0U &&
            (p->logstore & bit) == 0UL))
                return -1;
        if ((flags & VFS_POLICY_V2_SWAP) != 0U)
                p->swap &= ~bit;
        if ((flags & VFS_POLICY_V2_LOGSTORE) != 0U)
                p->logstore &= ~bit;
        return 0;
}

int
vfs_policy_v2_toggle_readonly(struct vfs_mount_policy_v2 *p,
    unsigned int slot)
{
        if (!vfs_policy_v2_valid(p, slot))
                return -1;
        p->readonly ^= 1UL << slot;
        return 0;
}

int
vfs_policy_v2_clear(struct vfs_mount_policy_v2 *p, unsigned int slot)
{
        kword_t bit;

        if (!vfs_policy_v2_valid(p, slot))
                return -1;
        bit = ~(1UL << slot);
        p->readonly &= bit;
        p->swap &= bit;
        p->logstore &= bit;
        return 0;
}
