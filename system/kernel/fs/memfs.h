#ifndef DAIMON_MEMFS_H
#define DAIMON_MEMFS_H

#include "vfs.h"

#define MEMFS_PROVIDER               4U
#define MEMFS_KIND_NODE              1U

#define MEMFS_F_USED                 0001U
#define MEMFS_F_WRITABLE             0004U
#define MEMFS_MOUNT_PERSIST          0002U

#define MEMFS_SNAPSHOT_MAGIC         055464663UL
#define MEMFS_SNAPSHOT_VERSION       2UL
#define MEMFS_SNAPSHOT_HEADER_WORDS  6U
#define MEMFS_NODE_WORDS             7U
#define MEMFS_NODE_COUNT             0100U
#define MEMFS_NODES_WORDS            0700U
#define MEMFS_BACKING_OFFSET         0700U
#define MEMFS_OWNER_OFFSET           01000U
#define MEMFS_OWNER_WORDS            0100U
#define MEMFS_MTIME_OFFSET           01100U
#define MEMFS_METADATA_WORDS         01200U


/*
 * A node is deliberately seven PDP-10 words.  The low 18 bits of meta hold
 * type/mode/flags, while the high half holds the parent slot.  File data packs
 * the resident physical word address in the high half and logical length in
 * the low half; nonresident mutable data is recovered through the per-node
 * backing descriptor table.
 */
struct memfs_node {
        struct vfs_name name;
        kword_t meta;
        kword_t data;
};

struct memfs {
        struct memfs_node *nodes;
        kword_t *pool;             /* reserved; mutable data is demand-backed */
        unsigned int pool_words;   /* configured mutable-data word ceiling */
        unsigned int used_words;
};

int memfs_snapshot_mount(struct memfs *fs, unsigned int flags);
int memfs_snapshot_shutdown(void);

int memfs_lookup(const struct memfs *fs, vnode_t dir,
    const struct vfs_name *name, vnode_t *nodep);
int memfs_readdir(const struct memfs *fs, vnode_t dir,
    unsigned int off, struct vfs_dirent *ent);
int memfs_stat(const struct memfs *fs, vnode_t node,
    struct vfs_stat *st);
int memfs_parent(const struct memfs *fs, vnode_t node,
    vnode_t *parentp, struct vfs_name *namep);
int memfs_create(struct memfs *fs, vnode_t dir,
    const struct vfs_name *name, unsigned int mode, vnode_t *nodep);
int memfs_mkfifo(struct memfs *fs, vnode_t dir,
    const struct vfs_name *name, unsigned int mode, vnode_t *nodep);
int memfs_mkdir(struct memfs *fs, vnode_t dir,
    const struct vfs_name *name, unsigned int mode, vnode_t *nodep);
int memfs_unlink(struct memfs *fs, vnode_t dir,
    const struct vfs_name *name);
int memfs_rename(struct memfs *fs, vnode_t olddir,
    const struct vfs_name *oldname, vnode_t newdir,
    const struct vfs_name *newname);
int memfs_truncate_words(struct memfs *fs, vnode_t node,
    unsigned int words);
int memfs_chmod(struct memfs *fs, vnode_t node,
    unsigned int mode);
int memfs_read_words(const struct memfs *fs, vnode_t node,
    unsigned int off, kword_t *buf, unsigned int nwords);
int memfs_write_words(struct memfs *fs, vnode_t node,
    unsigned int off, const kword_t *buf, unsigned int nwords);

void memfs_data_init(struct memfs *fs, kword_t limit);
void memfs_data_destroy(void);
int memfs_data_ensure(struct memfs *fs, unsigned int slot);
void memfs_data_dirty(unsigned int slot);
kword_t memfs_data_reclaim(kword_t wanted);


#endif
