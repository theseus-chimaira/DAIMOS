#ifndef DAIMON_TSFS_H
#define DAIMON_TSFS_H

#include "vfs.h"
#include "blockset.h"

#define TSFS_PROVIDER              7U
#define TSFS_KIND_ROOT             1U
#define TSFS_KIND_NODE             2U
#define TSFS_BLOCKS_PER_MEMBER     01102U

/* The transient mount helper validates metadata and leaves only compact table
 * locators.  This keeps the handoff/state at the existing 20 words. */
#define TSFS_MOUNT_META_WORDS      4U
#define TSFS_MOUNT_WORDS           (BLOCKSET_DESCRIPTOR_WORDS + TSFS_MOUNT_META_WORDS)
#define TSFS_MOUNT_FILE_LOC        BLOCKSET_DESCRIPTOR_WORDS
#define TSFS_MOUNT_FILE_SHAPE      (BLOCKSET_DESCRIPTOR_WORDS + 1U)
#define TSFS_MOUNT_EXTENT_LOC      (BLOCKSET_DESCRIPTOR_WORDS + 2U)
#define TSFS_MOUNT_EXTENT_SHAPE    (BLOCKSET_DESCRIPTOR_WORDS + 3U)

#define TSFS_FILE_WORDS            8U
#define TSFS_FILE_PARENT_FLAGS     0U
#define TSFS_FILE_NAME0            1U
#define TSFS_FILE_NAME_WORDS       4U
#define TSFS_FILE_SIZE_WORDS       5U
#define TSFS_FILE_EXTENT_RANGE     6U
#define TSFS_FILE_AUX              7U
#define TSFS_FILE_FLAG_DIR         1U
#define TSFS_FILE_FLAG_REG         2U
#define TSFS_FILE_FLAG_MASK        3U
#define TSFS_FILE_MODE_SHIFT       6U
#define TSFS_FILE_MODE_MASK        07777U
#define TSFS_FILE_FLAG_RESERVED    074U
#define TSFS_FILE_OWNER_SHIFT      9U

/* Regular-file payloads use fixed restart extents.  Metadata is never
 * compressed.  A payload extent is always explicitly STORED or D6LZ. */
#define TSFS_EXTENT_FLAG_STORED    0U
#define TSFS_EXTENT_FLAG_D6LZ      1U
#define TSFS_EXTENT_FLAG_MASK      1U
#define TSFS_RESTART_WORDS         0400U

int tsfs_mount_set(const kword_t *handoff, vnode_t target,
    unsigned int flags, vnode_t *rootp);
int tsfs_lookup(vnode_t dir, const struct vfs_name *name,
    vnode_t *nodep);
int tsfs_readdir(vnode_t dir, unsigned int off, struct vfs_dirent *ent);
int tsfs_stat(vnode_t node, struct vfs_stat *st);
int tsfs_parent(vnode_t node, vnode_t *parentp);
int tsfs_parent_name(vnode_t node, vnode_t *parentp,
    struct vfs_name *namep);

#endif
