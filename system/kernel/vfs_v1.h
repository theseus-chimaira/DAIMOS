#ifndef DAIMON_VFS_V1_H
#define DAIMON_VFS_V1_H

#include "kcore.h"

/* Compact vnode handle: provider:6, kind/mount:12, index:18. */
typedef kword_t vnode_v1_t;

#define VFS_V1_NODE_NONE        0UL
#define VFS_V1_PROVIDER_SHIFT   30U
#define VFS_V1_KIND_SHIFT       18U
#define VFS_V1_PROVIDER_MASK    077UL
#define VFS_V1_KIND_MASK        07777UL
#define VFS_V1_INDEX_MASK       0777777UL

/*
 * Mounted providers use the high six bits of kind as a VFS mount id.  Mount
 * id zero is the built-in namespace.  Existing provider vnode values are
 * therefore unchanged.
 */
#define VFS_V1_LOCAL_KIND_MASK  077U
#define VFS_V1_MOUNT_SHIFT      6U
#define VFS_V1_MOUNT_MASK       077U
#define VFS_V1_MOUNT_KIND(mount, kind) \
        ((((mount) & VFS_V1_MOUNT_MASK) << VFS_V1_MOUNT_SHIFT) | \
        ((kind) & VFS_V1_LOCAL_KIND_MASK))

#define VFS_V1_NODE(provider, kind, index) \
        ((((kword_t)(provider) & VFS_V1_PROVIDER_MASK) << \
        VFS_V1_PROVIDER_SHIFT) | \
        (((kword_t)(kind) & VFS_V1_KIND_MASK) << VFS_V1_KIND_SHIFT) | \
        ((kword_t)(index) & VFS_V1_INDEX_MASK))
#define VFS_V1_PROVIDER(node) \
        ((unsigned int)(((node) >> VFS_V1_PROVIDER_SHIFT) & \
        VFS_V1_PROVIDER_MASK))
#define VFS_V1_KIND(node) \
        ((unsigned int)(((node) >> VFS_V1_KIND_SHIFT) & VFS_V1_KIND_MASK))
#define VFS_V1_LOCAL_KIND(node) \
        (VFS_V1_KIND(node) & VFS_V1_LOCAL_KIND_MASK)
#define VFS_V1_MOUNT_ID(node) \
        ((VFS_V1_KIND(node) >> VFS_V1_MOUNT_SHIFT) & VFS_V1_MOUNT_MASK)
#define VFS_V1_INDEX(node) \
        ((unsigned int)((node) & VFS_V1_INDEX_MASK))

#define VFS_V1_TYPE_DIR         1U
#define VFS_V1_TYPE_REG         2U
#define VFS_V1_TYPE_CHAR        3U
#define VFS_V1_TYPE_BLOCK       4U
#define VFS_V1_TYPE_MOUNTSRC    5U
#define VFS_V1_TYPE_SYMLINK     6U

#define VFS_V1_NAME_WORDS       4U
#define VFS_V1_NAME_MAX_CHARS   (VFS_V1_NAME_WORDS * 6U)

#define VFS_V1_NMOUNT           4U
#define VFS_V1_MOUNT_RW         0U
#define VFS_V1_MOUNT_RDONLY     1U
#define VFS_V1_DEVICE_IO        (-3)
#define VFS_V1_LOCK_SHARED      1U
#define VFS_V1_LOCK_EXCLUSIVE   2U
#define VFS_V1_LOCK_UNLOCK      3U
#define VFS_V1_NLOCK            8U

/* Compile-time PDP-10 SIXBIT packing, also usable by host tests. */
#define VFS_V1_SIXCHAR(ch)      ((kword_t)(((unsigned int)(ch) - 040U) & 077U))
#define VFS_V1_SIX6(a,b,c,d,e,f) \
        ((VFS_V1_SIXCHAR(a) << 30) | (VFS_V1_SIXCHAR(b) << 24) | \
        (VFS_V1_SIXCHAR(c) << 18) | (VFS_V1_SIXCHAR(d) << 12) | \
        (VFS_V1_SIXCHAR(e) << 6) | VFS_V1_SIXCHAR(f))

struct vfs_v1_name {
        unsigned int chars;
        kword_t words[VFS_V1_NAME_WORDS];
};

struct vfs_v1_dirent {
        struct vfs_v1_name name;
        unsigned int type;
};

struct vfs_v1_stat {
        unsigned int type;
        unsigned int mode;
        kword_t size_chars;
        kword_t size_words;
};

int vfs_v1_name_set6(struct vfs_v1_name *name, kword_t word,
    unsigned int chars);
int vfs_v1_name_is6(const struct vfs_v1_name *name, kword_t word,
    unsigned int chars);
int vfs_v1_sixbit_readchar(kword_t word, unsigned int nchars, kword_t off,
    unsigned int *chp);

int vfs_v1_lookup(vnode_v1_t dir, const struct vfs_v1_name *name,
    vnode_v1_t *nodep);
int vfs_v1_readdir(vnode_v1_t dir, unsigned int off,
    struct vfs_v1_dirent *ent);
int vfs_v1_stat(vnode_v1_t node, struct vfs_v1_stat *st);
int vfs_v1_parent(vnode_v1_t node, vnode_v1_t *parentp);
int vfs_v1_parent_name(vnode_v1_t node, vnode_v1_t *parentp,
    struct vfs_v1_name *namep);
int vfs_v1_create(vnode_v1_t dir, const struct vfs_v1_name *name,
    unsigned int mode, vnode_v1_t *nodep);
int vfs_v1_mkdir(vnode_v1_t dir, const struct vfs_v1_name *name,
    unsigned int mode, vnode_v1_t *nodep);
int vfs_v1_symlink(vnode_v1_t dir, const struct vfs_v1_name *name,
    const kword_t *target, unsigned int target_chars, vnode_v1_t *nodep);
int vfs_v1_unlink(vnode_v1_t dir, const struct vfs_v1_name *name);
int vfs_v1_rename(vnode_v1_t olddir, const struct vfs_v1_name *oldname,
    vnode_v1_t newdir, const struct vfs_v1_name *newname);
int vfs_v1_truncate(vnode_v1_t node, unsigned int words, kword_t size_chars);
int vfs_v1_chmod(vnode_v1_t node, unsigned int mode);
int vfs_v1_read_words(vnode_v1_t node, unsigned int off, kword_t *buf,
    unsigned int nwords);
int vfs_v1_write_words(vnode_v1_t node, unsigned int off,
    const kword_t *buf, unsigned int nwords, kword_t size_chars);
int vfs_v1_readchar(vnode_v1_t node, kword_t off, unsigned int *chp);
int vfs_v1_writechar(vnode_v1_t node, kword_t off, unsigned int ch);
int vfs_v1_sync(vnode_v1_t node);
int vfs_v1_lock(vnode_v1_t node, unsigned int owner, unsigned int op);
void vfs_v1_unlock_owner(vnode_v1_t node, unsigned int owner);

vnode_v1_t vfs_v1_follow_mount(vnode_v1_t node);
int vfs_v1_mount(vnode_v1_t target, unsigned int provider,
    unsigned int kind, unsigned int index, unsigned int flags,
    vnode_v1_t *rootp);
int vfs_v1_unmount(vnode_v1_t root);
int vfs_v1_readonly(vnode_v1_t node);
vnode_v1_t vfs_v1_root(void);
int vfs_v1_set_root(vnode_v1_t node);

#endif
