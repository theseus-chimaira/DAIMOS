#ifndef DAIMON_D6FS_PROVIDER_H
#define DAIMON_D6FS_PROVIDER_H

#include "d6fs.h"
#include "vfs.h"

#define D6FS_PROVIDER        6U
#define D6FS_KIND_NODE       1U
#define D6FS_PROVIDER_MOUNT_ID_MASK   077U
#define D6FS_PROVIDER_MOUNT_WRITABLE  0100U
#define D6FS_PROVIDER_MOUNT_COPY      0200U

/* The filesystem UUID is needed only while KINIT selects the superblock.
 * After that, reuse those two resident reader words for the physical dual-
 * superblock locations needed by the writable unmount commit. */
#define D6FS_RUNTIME_SUPER_BLOCK(reader, copy) ((reader)->super.fs_uuid[(copy)])

/* KINIT-only relocated D6FS runtime destinations and callback addresses. */
extern unsigned int d6fs_block_read_addr;
extern unsigned int d6fs_block_write_addr;
extern unsigned int d6fs_provider_reader_addr;
extern unsigned int d6fs_direct_map_addr;

int d6fs_provider_lookup(vnode_t dir, const struct vfs_name *name,
    vnode_t *nodep);
int d6fs_provider_readdir(vnode_t dir, unsigned int off,
    struct vfs_dirent *ent);
int d6fs_provider_stat(vnode_t node, struct vfs_stat *st);
int d6fs_provider_parent(vnode_t node, vnode_t *parentp);
int d6fs_provider_parent_name(vnode_t node, vnode_t *parentp,
    struct vfs_name *namep);
int d6fs_provider_read_words(vnode_t node, unsigned int off,
    kword_t *buf, unsigned int nwords);
int d6fs_provider_sync(vnode_t node);
int d6fs_provider_prepare_unmount(vnode_t root);
int d6fs_provider_create_object(vnode_t dir, const struct vfs_name *name,
    const kword_t *payload, unsigned int value, unsigned int type,
    vnode_t *nodep);
int d6fs_provider_unlink(vnode_t dir, const struct vfs_name *name);
int d6fs_provider_rename(vnode_t olddir,
    const struct vfs_name *oldname, vnode_t newdir,
    const struct vfs_name *newname);
int d6fs_provider_truncate(vnode_t node, unsigned int words,
    kword_t size_chars);
int d6fs_provider_chmod(vnode_t node, unsigned int mode);
int d6fs_provider_write_words(vnode_t node, unsigned int off,
    const kword_t *buf, unsigned int nwords, kword_t size_chars);

#endif
