#include "memfs.h"

#define MEMFS_HALF_MASK      0777777UL
#define MEMFS_TYPE_SHIFT     15U
#define MEMFS_TYPE_MASK      07UL
#define MEMFS_MODE_SHIFT     3U
#define MEMFS_MODE_MASK      07777UL
#define MEMFS_FLAGS_MASK     07UL

#define NODE_PARENT(np) \
        ((unsigned int)(((np)->meta >> 18U) & MEMFS_HALF_MASK))
#define NODE_TYPE(np) \
        ((unsigned int)(((np)->meta >> MEMFS_TYPE_SHIFT) & MEMFS_TYPE_MASK))
#define NODE_MODE(np) \
        ((unsigned int)(((np)->meta >> MEMFS_MODE_SHIFT) & MEMFS_MODE_MASK))
#define NODE_FLAGS(np) \
        ((unsigned int)((np)->meta & MEMFS_FLAGS_MASK))
#define NODE_DATA_WORD(np) \
        ((unsigned int)(((np)->data >> 18U) & MEMFS_HALF_MASK))
#define NODE_DATA_WORDS(np) \
        ((unsigned int)((np)->data & MEMFS_HALF_MASK))

extern vnode_t memfs_node_handle(unsigned int slot);
extern int memfs_name_valid(const struct vfs_name *name);
extern int memfs_slot(const struct memfs *fs, vnode_t node,
    unsigned int *slotp);
extern int memfs_find_child(const struct memfs *fs, unsigned int parent,
    const struct vfs_name *name, unsigned int *slotp);
extern int memfs_free_slot(const struct memfs *fs, unsigned int *slotp);
extern void memfs_clear_node(struct memfs_node *np);

int
memfs_lookup(const struct memfs *fs, vnode_t dir,
    const struct vfs_name *name, vnode_t *nodep)
{
        unsigned int parent;
        unsigned int slot;

        if (nodep == 0 || !memfs_name_valid(name) ||
            memfs_slot(fs, dir, &parent) != 0 ||
            NODE_TYPE(&fs->nodes[parent]) != VFS_TYPE_DIR)
                return -1;
        if (memfs_find_child(fs, parent, name, &slot) != 0)
                return -1;
        *nodep = memfs_node_handle(slot);
        return 0;
}

extern int memfs_readdir(const struct memfs *fs, vnode_t dir,
    unsigned int off, struct vfs_dirent *ent);

extern int memfs_stat(const struct memfs *fs, vnode_t node,
    struct vfs_stat *st);

extern int memfs_parent(const struct memfs *fs, vnode_t node,
    vnode_t *parentp, struct vfs_name *namep);

static int
memfs_new_node(struct memfs *fs, vnode_t dir,
    const struct vfs_name *name, unsigned int type, unsigned int mode,
    vnode_t *nodep)
{
        unsigned int parent;
        unsigned int slot;
        struct memfs_node *np;

        if (fs == 0 || !fs->writable || nodep == 0 ||
            !memfs_name_valid(name) || memfs_slot(fs, dir, &parent) != 0 ||
            NODE_TYPE(&fs->nodes[parent]) != VFS_TYPE_DIR ||
            (NODE_FLAGS(&fs->nodes[parent]) & MEMFS_F_WRITABLE) == 0U)
                return -1;
        if (memfs_find_child(fs, parent, name, 0) == 0 ||
            memfs_free_slot(fs, &slot) != 0)
                return -1;
        np = &fs->nodes[slot];
        memfs_clear_node(np);
        np->name = *name;
        np->meta = ((kword_t)(parent & MEMFS_HALF_MASK) << 18U) |
            ((kword_t)(type & MEMFS_TYPE_MASK) << MEMFS_TYPE_SHIFT) |
            ((kword_t)(mode & MEMFS_MODE_MASK) << MEMFS_MODE_SHIFT) |
            (kword_t)(MEMFS_F_USED | MEMFS_F_WRITABLE);
        np->data = (kword_t)(fs->used_words & MEMFS_HALF_MASK) << 18U;
        *nodep = memfs_node_handle(slot);
        return 0;
}

int
memfs_create(struct memfs *fs, vnode_t dir,
    const struct vfs_name *name, unsigned int mode, vnode_t *nodep)
{
        return memfs_new_node(fs, dir, name, VFS_TYPE_REG, mode,
            nodep);
}

int
memfs_mkdir(struct memfs *fs, vnode_t dir,
    const struct vfs_name *name, unsigned int mode, vnode_t *nodep)
{
        return memfs_new_node(fs, dir, name, VFS_TYPE_DIR, mode,
            nodep);
}

extern int memfs_has_children(const struct memfs *fs, unsigned int slot);

extern void memfs_shift_after(struct memfs *fs, unsigned int start,
    int delta, unsigned int exclude);

extern int memfs_resize(struct memfs *fs, unsigned int slot,
    unsigned int words);

int
memfs_unlink(struct memfs *fs, vnode_t dir,
    const struct vfs_name *name)
{
        unsigned int parent;
        unsigned int slot;

        if (fs == 0 || !fs->writable || !memfs_name_valid(name) ||
            memfs_slot(fs, dir, &parent) != 0 ||
            NODE_TYPE(&fs->nodes[parent]) != VFS_TYPE_DIR ||
            (NODE_FLAGS(&fs->nodes[parent]) & MEMFS_F_WRITABLE) == 0U ||
            memfs_find_child(fs, parent, name, &slot) != 0 ||
            memfs_has_children(fs, slot))
                return -1;
        if (NODE_TYPE(&fs->nodes[slot]) == VFS_TYPE_REG &&
            memfs_resize(fs, slot, 0U) != 0)
                return -1;
        memfs_clear_node(&fs->nodes[slot]);
        return 0;
}


int
memfs_rename(struct memfs *fs, vnode_t olddir,
    const struct vfs_name *oldname, vnode_t newdir,
    const struct vfs_name *newname)
{
        unsigned int oldparent;
        unsigned int newparent;
        unsigned int slot;
        unsigned int p;

        if (fs == 0 || !fs->writable || !memfs_name_valid(oldname) ||
            !memfs_name_valid(newname) ||
            memfs_slot(fs, olddir, &oldparent) != 0 ||
            memfs_slot(fs, newdir, &newparent) != 0 ||
            NODE_TYPE(&fs->nodes[oldparent]) != VFS_TYPE_DIR ||
            NODE_TYPE(&fs->nodes[newparent]) != VFS_TYPE_DIR ||
            (NODE_FLAGS(&fs->nodes[oldparent]) & MEMFS_F_WRITABLE) == 0U ||
            (NODE_FLAGS(&fs->nodes[newparent]) & MEMFS_F_WRITABLE) == 0U ||
            memfs_find_child(fs, oldparent, oldname, &slot) != 0 ||
            memfs_find_child(fs, newparent, newname, 0) == 0)
                return -1;
        if (NODE_TYPE(&fs->nodes[slot]) == VFS_TYPE_DIR) {
                p = newparent;
                for (;;) {
                        if (p == slot)
                                return -1;
                        if (p == 0U)
                                break;
                        if (p >= fs->node_count ||
                            (NODE_FLAGS(&fs->nodes[p]) & MEMFS_F_USED) == 0U)
                                return -1;
                        p = NODE_PARENT(&fs->nodes[p]);
                }
        }
        fs->nodes[slot].meta =
            (fs->nodes[slot].meta & MEMFS_HALF_MASK) |
            ((kword_t)(newparent & MEMFS_HALF_MASK) << 18U);
        fs->nodes[slot].name = *newname;
        return 0;
}

int
memfs_chmod(struct memfs *fs, vnode_t node, unsigned int mode)
{
        unsigned int slot;

        if (fs == 0 || !fs->writable || memfs_slot(fs, node, &slot) != 0 ||
            (NODE_FLAGS(&fs->nodes[slot]) & MEMFS_F_WRITABLE) == 0U)
                return -1;
        fs->nodes[slot].meta =
            (fs->nodes[slot].meta &
            ~((kword_t)MEMFS_MODE_MASK << MEMFS_MODE_SHIFT)) |
            ((kword_t)(mode & MEMFS_MODE_MASK) << MEMFS_MODE_SHIFT);
        return 0;
}

int
memfs_truncate_words(struct memfs *fs, vnode_t node,
    unsigned int words, kword_t size_chars)
{
        unsigned int slot;

        if (memfs_slot(fs, node, &slot) != 0 ||
            NODE_TYPE(&fs->nodes[slot]) != VFS_TYPE_REG ||
            memfs_resize(fs, slot, words) != 0)
                return -1;
        fs->nodes[slot].size_chars = size_chars;
        return 0;
}

extern int memfs_read_words(const struct memfs *fs, vnode_t node,
    unsigned int off, kword_t *buf, unsigned int nwords);

extern int memfs_write_words(struct memfs *fs, vnode_t node,
    unsigned int off, const kword_t *buf, unsigned int nwords,
    kword_t size_chars);
