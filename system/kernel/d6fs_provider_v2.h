#ifndef DAIMON_D6FS_PROVIDER_V2_H
#define DAIMON_D6FS_PROVIDER_V2_H

#include "d6fs_v2.h"
#include "vfs_v1.h"

#define D6FS_V2_PROVIDER        6U
#define D6FS_V2_KIND_NODE       1U

int d6fs_provider_v2_mount(vnode_v1_t target,
    d6fs_v2_read_block_fn read_block, void *opaque,
    const struct d6fs_v2_super_info *super, unsigned int flags,
    vnode_v1_t *rootp);
/* Shared block buffer used for mount probing and the active reader cache. */
kword_t *d6fs_provider_v2_block_buffer(void);
int d6fs_provider_v2_mount_rw(vnode_v1_t target,
    d6fs_v2_read_block_fn read_block, d6fs_v2_write_block_fn write_block,
    void *opaque, const struct d6fs_v2_super_info *super, unsigned int flags,
    vnode_v1_t *rootp);
int d6fs_provider_v2_lookup(vnode_v1_t dir, const struct vfs_v1_name *name,
    vnode_v1_t *nodep);
int d6fs_provider_v2_readdir(vnode_v1_t dir, unsigned int off,
    struct vfs_v1_dirent *ent);
int d6fs_provider_v2_stat(vnode_v1_t node, struct vfs_v1_stat *st);
int d6fs_provider_v2_parent(vnode_v1_t node, vnode_v1_t *parentp);
int d6fs_provider_v2_parent_name(vnode_v1_t node, vnode_v1_t *parentp,
    struct vfs_v1_name *namep);
int d6fs_provider_v2_read_words(vnode_v1_t node, unsigned int off,
    kword_t *buf, unsigned int nwords);
int d6fs_provider_v2_sync(vnode_v1_t node);
int d6fs_provider_v2_enable_state(vnode_v1_t root, kword_t super_a,
    kword_t super_b, unsigned int selected_copy);
int d6fs_provider_v2_prepare_unmount(vnode_v1_t root);
int d6fs_provider_v2_create(vnode_v1_t dir, const struct vfs_v1_name *name,
    unsigned int mode, vnode_v1_t *nodep);
int d6fs_provider_v2_mkdir(vnode_v1_t dir, const struct vfs_v1_name *name,
    unsigned int mode, vnode_v1_t *nodep);
int d6fs_provider_v2_unlink(vnode_v1_t dir, const struct vfs_v1_name *name);
int d6fs_provider_v2_rename(vnode_v1_t olddir,
    const struct vfs_v1_name *oldname, vnode_v1_t newdir,
    const struct vfs_v1_name *newname);
int d6fs_provider_v2_truncate(vnode_v1_t node, unsigned int words,
    kword_t size_chars);
int d6fs_provider_v2_chmod(vnode_v1_t node, unsigned int mode);
int d6fs_provider_v2_write_words(vnode_v1_t node, unsigned int off,
    const kword_t *buf, unsigned int nwords, kword_t size_chars);

#endif
