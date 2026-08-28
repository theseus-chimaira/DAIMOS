#ifndef DAIMON_MEMFS_V1_H
#define DAIMON_MEMFS_V1_H

#include "vfs_v1.h"

#define MEMFS_V1_PROVIDER               4U
#define MEMFS_V1_KIND_NODE              1U

#define MEMFS_V1_F_USED                 0001U
#define MEMFS_V1_F_IMAGE                0002U
#define MEMFS_V1_F_WRITABLE             0004U

/*
 * A node is deliberately eight PDP-10 words.  The low 18 bits of meta hold
 * type/mode/flags, while the high half holds the parent slot.  data packs
 * the pool/image word offset in the high half and its length in the low.
 */
struct memfs_v1_node {
        struct vfs_v1_name name;
        kword_t meta;
        kword_t size_chars;
        kword_t data;
};

struct memfs_v1 {
        struct memfs_v1_node *nodes;
        unsigned int node_count;
        kword_t *pool;
        unsigned int pool_words;
        unsigned int used_words;
        int writable;
        const kword_t *image_data;
};

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
int memfs_v1_chmod(struct memfs_v1 *fs, vnode_v1_t node,
    unsigned int mode);
int memfs_v1_read_words(const struct memfs_v1 *fs, vnode_v1_t node,
    unsigned int off, kword_t *buf, unsigned int nwords);
int memfs_v1_write_words(struct memfs_v1 *fs, vnode_v1_t node,
    unsigned int off, const kword_t *buf, unsigned int nwords,
    kword_t size_chars);


#endif
