#ifndef DAIMON_MEMFS_V1_H
#define DAIMON_MEMFS_V1_H

#include "vfs_v1.h"

#define MEMFS_V1_PROVIDER               4U
#define MEMFS_V1_KIND_NODE              1U

#define MEMFS_V1_F_USED                 0001U
#define MEMFS_V1_F_IMAGE                0002U
#define MEMFS_V1_F_WRITABLE             0004U

struct memfs_v1_node {
        struct vfs_v1_name name;
        unsigned int parent;
        unsigned int type;
        unsigned int mode;
        unsigned int flags;
        kword_t size_chars;
        unsigned int data_word;
        unsigned int data_words;
        const kword_t *image_data;
};

struct memfs_v1 {
        struct memfs_v1_node *nodes;
        unsigned int node_count;
        kword_t *pool;
        unsigned int pool_words;
        unsigned int used_words;
        int writable;
};

int memfs_v1_init(struct memfs_v1 *fs, struct memfs_v1_node *nodes,
    unsigned int node_count, kword_t *pool, unsigned int pool_words,
    int writable);
int memfs_v1_enable_write(struct memfs_v1 *fs, kword_t *pool,
    unsigned int pool_words);
int memfs_v1_attach_pool(struct memfs_v1 *fs, kword_t *pool,
    unsigned int pool_words);
vnode_v1_t memfs_v1_root(const struct memfs_v1 *fs);
int memfs_v1_lookup(const struct memfs_v1 *fs, vnode_v1_t dir,
    const struct vfs_v1_name *name, vnode_v1_t *nodep);
int memfs_v1_readdir(const struct memfs_v1 *fs, vnode_v1_t dir,
    unsigned int off, struct vfs_v1_dirent *ent);
int memfs_v1_stat(const struct memfs_v1 *fs, vnode_v1_t node,
    struct vfs_v1_stat *st);
int memfs_v1_parent(const struct memfs_v1 *fs, vnode_v1_t node,
    vnode_v1_t *parentp, struct vfs_v1_name *namep);
int memfs_v1_create(struct memfs_v1 *fs, vnode_v1_t dir,
    const struct vfs_v1_name *name, unsigned int mode, vnode_v1_t *nodep);
int memfs_v1_mkdir(struct memfs_v1 *fs, vnode_v1_t dir,
    const struct vfs_v1_name *name, unsigned int mode, vnode_v1_t *nodep);
int memfs_v1_unlink(struct memfs_v1 *fs, vnode_v1_t dir,
    const struct vfs_v1_name *name);
int memfs_v1_rename(struct memfs_v1 *fs, vnode_v1_t olddir,
    const struct vfs_v1_name *oldname, vnode_v1_t newdir,
    const struct vfs_v1_name *newname);
int memfs_v1_truncate_words(struct memfs_v1 *fs, vnode_v1_t node,
    unsigned int words, kword_t size_chars);
int memfs_v1_read_words(const struct memfs_v1 *fs, vnode_v1_t node,
    unsigned int off, kword_t *buf, unsigned int nwords);
int memfs_v1_write_words(struct memfs_v1 *fs, vnode_v1_t node,
    unsigned int off, const kword_t *buf, unsigned int nwords,
    kword_t size_chars);

/* Used by INITFS/bootstrap code to install image-backed namespace nodes. */
int memfs_v1_import_dir(struct memfs_v1 *fs, vnode_v1_t dir,
    const struct vfs_v1_name *name, unsigned int mode, int writable,
    vnode_v1_t *nodep);
int memfs_v1_import_node(struct memfs_v1 *fs, unsigned int slot,
    unsigned int parent, const struct vfs_v1_name *name, unsigned int type,
    unsigned int mode, const kword_t *data, unsigned int data_words,
    kword_t size_chars);

#endif
