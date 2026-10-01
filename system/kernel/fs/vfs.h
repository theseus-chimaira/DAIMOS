#ifndef DAIMON_VFS_H
#define DAIMON_VFS_H

#include "kcore.h"
#include "storage.h"
#include <pdp10-sixbit.h>

/* Compact vnode handle: provider:6, kind/mount:12, index:18. */
typedef kword_t vnode_t;

#define VFS_NODE_NONE        0UL
#define VFS_PROVIDER_SHIFT   30U
#define VFS_KIND_SHIFT       18U
#define VFS_PROVIDER_MASK    077UL
#define VFS_KIND_MASK        07777UL
#define VFS_INDEX_MASK       0777777UL

/*
 * Mounted providers use the high six bits of kind as a VFS mount id.  Mount
 * id zero is the built-in namespace.  Existing provider vnode values are
 * therefore unchanged.
 */
#define VFS_LOCAL_KIND_MASK  077U
#define VFS_MOUNT_SHIFT      6U
#define VFS_MOUNT_MASK       077U
#define VFS_MOUNT_KIND(mount, kind) \
        ((((mount) & VFS_MOUNT_MASK) << VFS_MOUNT_SHIFT) | \
        ((kind) & VFS_LOCAL_KIND_MASK))

#define VFS_NODE(provider, kind, index) \
        ((((kword_t)(provider) & VFS_PROVIDER_MASK) << \
        VFS_PROVIDER_SHIFT) | \
        (((kword_t)(kind) & VFS_KIND_MASK) << VFS_KIND_SHIFT) | \
        ((kword_t)(index) & VFS_INDEX_MASK))
/* Use only when each component is already known to fit its field. */
#define VFS_NODE_PACKED(provider, kind, index) \
        (((kword_t)(provider) << VFS_PROVIDER_SHIFT) | \
        ((kword_t)(kind) << VFS_KIND_SHIFT) | (kword_t)(index))
/* vnode_t is one PDP-10 word, so shifting away the low 30 bits already
 * leaves exactly the six-bit provider field. */
#define VFS_PROVIDER(node) \
        ((unsigned int)((node) >> VFS_PROVIDER_SHIFT))
#define VFS_KIND(node) \
        ((unsigned int)(((node) >> VFS_KIND_SHIFT) & VFS_KIND_MASK))
#define VFS_LOCAL_KIND(node) \
        (VFS_KIND(node) & VFS_LOCAL_KIND_MASK)
#define VFS_MOUNT_ID(node) \
        ((VFS_KIND(node) >> VFS_MOUNT_SHIFT) & VFS_MOUNT_MASK)
#define VFS_INDEX(node) \
        ((unsigned int)((node) & VFS_INDEX_MASK))

#define VFS_TYPE_DIR         1U
#define VFS_TYPE_REG         2U
#define VFS_TYPE_CHAR        3U
#define VFS_TYPE_BLOCK       4U
#define VFS_TYPE_MOUNTSRC    5U
#define VFS_TYPE_SYMLINK     6U
#define VFS_TYPE_FIFO        7U

#define VFS_NAME_WORDS       4U
#define VFS_NAME_MAX_CHARS   (VFS_NAME_WORDS * 6U)

#define VFS_NMOUNT           4U
#define VFS_ID_MASK          0777U
#define VFS_MOUNT_RW         0U
#define VFS_MOUNT_RDONLY     1U
#define VFS_MOUNT_STORAGE_SWAP      020U
#define VFS_MOUNT_STORAGE_LOGSTORE  0400U
#define VFS_MOUNT_STORAGE_MASK \
        (VFS_MOUNT_STORAGE_SWAP | VFS_MOUNT_STORAGE_LOGSTORE)
#define VFS_ERR_UNSUPPORTED  (-2)
#define VFS_DEVICE_IO        (-3)
#define VFS_LOCK_SHARED      1U
#define VFS_LOCK_EXCLUSIVE   2U
#define VFS_LOCK_UNLOCK      3U

struct vfs_name {
        unsigned int chars;
        kword_t words[VFS_NAME_WORDS];
};

struct vfs_dirent {
        struct vfs_name name;
        unsigned int type;
};

struct vfs_stat {
        unsigned int type;
        unsigned int mode;
        kword_t reserved;
        kword_t size_words;
        unsigned int uid;
        unsigned int gid;
        kword_t mtime;
};

int vfs_name_valid(const struct vfs_name *name);
int vfs_name_words_equal(const kword_t *a, const kword_t *b,
    unsigned int count);
void vfs_name_from_words(const kword_t *src, struct vfs_name *dst);
int vfs_name_is6(const struct vfs_name *name, kword_t word,
    unsigned int chars);
unsigned int vfs_sixbit_name_chars(const kword_t *words,
    unsigned int maxchars);
unsigned int vfs_name_char(const struct vfs_name *name,
    unsigned int pos);
void vfs_name_setchar(struct vfs_name *name, unsigned int pos,
    unsigned int ch);

int vfs_lookup(vnode_t dir, const struct vfs_name *name,
    vnode_t *nodep);
int vfs_readdir(vnode_t dir, unsigned int off,
    struct vfs_dirent *ent);
int vfs_stat(vnode_t node, struct vfs_stat *st);
int vfs_parent(vnode_t node, vnode_t *parentp);
int vfs_parent_name(vnode_t node, vnode_t *parentp,
    struct vfs_name *namep);
int vfs_create(vnode_t dir, const struct vfs_name *name,
    unsigned int mode, vnode_t *nodep);
int vfs_mkfifo(vnode_t dir, const struct vfs_name *name,
    unsigned int mode, vnode_t *nodep);
int vfs_mkdir(vnode_t dir, const struct vfs_name *name,
    unsigned int mode, vnode_t *nodep);
int vfs_symlink(vnode_t dir, const struct vfs_name *name,
    const kword_t *target, vnode_t *nodep);
int vfs_unlink(vnode_t dir, const struct vfs_name *name);
int vfs_rename(vnode_t olddir, const struct vfs_name *oldname,
    vnode_t newdir, const struct vfs_name *newname);
int vfs_truncate(vnode_t node, unsigned int words);
int vfs_chmod(vnode_t node, unsigned int mode);
int vfs_chown(vnode_t node, unsigned int uid, unsigned int gid);
int vfs_utime(vnode_t node, kword_t mtime);
int vfs_read_words(vnode_t node, unsigned int off, kword_t *buf,
    unsigned int nwords);
int vfs_write_words(vnode_t node, unsigned int off,
    const kword_t *buf, unsigned int nwords);
int vfs_readchar(vnode_t node, kword_t off, unsigned int *chp);
int vfs_writechar(vnode_t node, kword_t off, unsigned int ch);
int vfs_sync(vnode_t node);

int vfs_storage_release(unsigned int mount, unsigned int flags);
int vfs_mount(vnode_t target, unsigned int provider,
    unsigned int kind, unsigned int index, unsigned int flags,
    vnode_t *rootp);
int vfs_mount_prevalidated(vnode_t target, unsigned int provider,
    unsigned int kind, unsigned int index, unsigned int flags,
    vnode_t *rootp);
int vfs_unmount(vnode_t root);
int vfs_remount(vnode_t node);
int vfs_readonly(vnode_t node);
extern vnode_t vfs_namespace_root;

/* Shared 128-word synchronous filesystem transfer workspace. */
extern kword_t fs_block_workspace[BSTORE_BLOCK_WORDS];

#endif
