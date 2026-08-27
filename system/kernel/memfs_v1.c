#include "memfs_v1.h"

#define MEMFS_V1_HALF_MASK      0777777UL
#define MEMFS_V1_TYPE_SHIFT     15U
#define MEMFS_V1_TYPE_MASK      07UL
#define MEMFS_V1_MODE_SHIFT     3U
#define MEMFS_V1_MODE_MASK      07777UL
#define MEMFS_V1_FLAGS_MASK     07UL

#define NODE_PARENT(np) \
        ((unsigned int)(((np)->meta >> 18U) & MEMFS_V1_HALF_MASK))
#define NODE_TYPE(np) \
        ((unsigned int)(((np)->meta >> MEMFS_V1_TYPE_SHIFT) & MEMFS_V1_TYPE_MASK))
#define NODE_MODE(np) \
        ((unsigned int)(((np)->meta >> MEMFS_V1_MODE_SHIFT) & MEMFS_V1_MODE_MASK))
#define NODE_FLAGS(np) \
        ((unsigned int)((np)->meta & MEMFS_V1_FLAGS_MASK))
#define NODE_DATA_WORD(np) \
        ((unsigned int)(((np)->data >> 18U) & MEMFS_V1_HALF_MASK))
#define NODE_DATA_WORDS(np) \
        ((unsigned int)((np)->data & MEMFS_V1_HALF_MASK))

static void
node_set_meta(struct memfs_v1_node *np, unsigned int parent,
    unsigned int type, unsigned int mode, unsigned int flags)
{
        np->meta = ((kword_t)(parent & MEMFS_V1_HALF_MASK) << 18U) |
            ((kword_t)(type & MEMFS_V1_TYPE_MASK) << MEMFS_V1_TYPE_SHIFT) |
            ((kword_t)(mode & MEMFS_V1_MODE_MASK) << MEMFS_V1_MODE_SHIFT) |
            (kword_t)(flags & MEMFS_V1_FLAGS_MASK);
}

static void
node_set_parent(struct memfs_v1_node *np, unsigned int parent)
{
        np->meta = (np->meta & MEMFS_V1_HALF_MASK) |
            ((kword_t)(parent & MEMFS_V1_HALF_MASK) << 18U);
}

static void
node_set_data(struct memfs_v1_node *np, unsigned int word,
    unsigned int words)
{
        np->data = ((kword_t)(word & MEMFS_V1_HALF_MASK) << 18U) |
            (kword_t)(words & MEMFS_V1_HALF_MASK);
}

static int
memfs_v1_name_equal(const struct vfs_v1_name *a,
    const struct vfs_v1_name *b)
{
        unsigned int i;

        if (a == 0 || b == 0 || a->chars != b->chars)
                return 0;
        for (i = 0U; i < VFS_V1_NAME_WORDS; ++i) {
                if (a->words[i] != b->words[i])
                        return 0;
        }
        return 1;
}

static int
memfs_v1_name_valid(const struct vfs_v1_name *name)
{
        return name != 0 && name->chars != 0U &&
            name->chars <= VFS_V1_NAME_MAX_CHARS;
}

static int
memfs_v1_slot(const struct memfs_v1 *fs, vnode_v1_t node,
    unsigned int *slotp)
{
        unsigned int slot;

        if (fs == 0 || VFS_V1_PROVIDER(node) != MEMFS_V1_PROVIDER ||
            VFS_V1_KIND(node) != MEMFS_V1_KIND_NODE)
                return -1;
        slot = VFS_V1_INDEX(node);
        if (slot >= fs->node_count ||
            (NODE_FLAGS(&fs->nodes[slot]) & MEMFS_V1_F_USED) == 0U)
                return -1;
        if (slotp != 0)
                *slotp = slot;
        return 0;
}

static vnode_v1_t
memfs_v1_node_handle(unsigned int slot)
{
        return VFS_V1_NODE(MEMFS_V1_PROVIDER, MEMFS_V1_KIND_NODE, slot);
}

static int
memfs_v1_find_child(const struct memfs_v1 *fs, unsigned int parent,
    const struct vfs_v1_name *name, unsigned int *slotp)
{
        const struct memfs_v1_node *np;
        unsigned int i;

        np = fs->nodes + 1;
        for (i = 1U; i < fs->node_count; ++i, ++np) {
                if ((NODE_FLAGS(np) & MEMFS_V1_F_USED) == 0U ||
                    NODE_PARENT(np) != parent)
                        continue;
                if (memfs_v1_name_equal(&np->name, name)) {
                        if (slotp != 0)
                                *slotp = i;
                        return 0;
                }
        }
        return -1;
}

static int
memfs_v1_free_slot(const struct memfs_v1 *fs, unsigned int *slotp)
{
        const struct memfs_v1_node *np;
        unsigned int i;

        np = fs->nodes + 1;
        for (i = 1U; i < fs->node_count; ++i, ++np) {
                if ((NODE_FLAGS(np) & MEMFS_V1_F_USED) == 0U) {
                        *slotp = i;
                        return 0;
                }
        }
        return -1;
}

static void
memfs_v1_clear_node(struct memfs_v1_node *np)
{
        unsigned int i;

        for (i = 0U; i < VFS_V1_NAME_WORDS; ++i)
                np->name.words[i] = 0;
        np->name.chars = 0U;
        np->meta = 0;
        np->size_chars = 0;
        np->data = 0;
}

vnode_v1_t
memfs_v1_root(const struct memfs_v1 *fs)
{
        if (fs == 0 || fs->nodes == 0 || fs->node_count == 0U)
                return VFS_V1_NODE_NONE;
        return memfs_v1_node_handle(0U);
}

int
memfs_v1_lookup(const struct memfs_v1 *fs, vnode_v1_t dir,
    const struct vfs_v1_name *name, vnode_v1_t *nodep)
{
        unsigned int parent;
        unsigned int slot;

        if (nodep == 0 || !memfs_v1_name_valid(name) ||
            memfs_v1_slot(fs, dir, &parent) != 0 ||
            NODE_TYPE(&fs->nodes[parent]) != VFS_V1_TYPE_DIR)
                return -1;
        if (memfs_v1_find_child(fs, parent, name, &slot) != 0)
                return -1;
        *nodep = memfs_v1_node_handle(slot);
        return 0;
}

int
memfs_v1_readdir(const struct memfs_v1 *fs, vnode_v1_t dir,
    unsigned int off, struct vfs_v1_dirent *ent)
{
        unsigned int parent;
        unsigned int i;
        unsigned int n;

        if (ent == 0 || memfs_v1_slot(fs, dir, &parent) != 0 ||
            NODE_TYPE(&fs->nodes[parent]) != VFS_V1_TYPE_DIR)
                return -1;
        {
                const struct memfs_v1_node *np;

                n = 0U;
                np = fs->nodes + 1;
                for (i = 1U; i < fs->node_count; ++i, ++np) {
                        if ((NODE_FLAGS(np) & MEMFS_V1_F_USED) == 0U ||
                            NODE_PARENT(np) != parent)
                                continue;
                        if (n++ != off)
                                continue;
                        ent->name = np->name;
                        ent->type = NODE_TYPE(np);
                        return 1;
                }
        }
        return 0;
}

int
memfs_v1_stat(const struct memfs_v1 *fs, vnode_v1_t node,
    struct vfs_v1_stat *st)
{
        unsigned int slot;
        const struct memfs_v1_node *np;

        if (st == 0 || memfs_v1_slot(fs, node, &slot) != 0)
                return -1;
        np = &fs->nodes[slot];
        st->type = NODE_TYPE(np);
        st->mode = NODE_MODE(np);
        st->size_chars = np->size_chars;
        st->size_words = NODE_DATA_WORDS(np);
        return 0;
}

int
memfs_v1_parent(const struct memfs_v1 *fs, vnode_v1_t node,
    vnode_v1_t *parentp, struct vfs_v1_name *namep)
{
        unsigned int slot;
        const struct memfs_v1_node *np;

        if (parentp == 0 || memfs_v1_slot(fs, node, &slot) != 0)
                return -1;
        np = &fs->nodes[slot];
        if (NODE_PARENT(np) >= fs->node_count ||
            (NODE_FLAGS(&fs->nodes[NODE_PARENT(np)]) & MEMFS_V1_F_USED) == 0U)
                return -1;
        *parentp = memfs_v1_node_handle(NODE_PARENT(np));
        if (namep != 0)
                *namep = np->name;
        return 0;
}

static int
memfs_v1_new_node(struct memfs_v1 *fs, vnode_v1_t dir,
    const struct vfs_v1_name *name, unsigned int type, unsigned int mode,
    vnode_v1_t *nodep)
{
        unsigned int parent;
        unsigned int slot;
        struct memfs_v1_node *np;

        if (fs == 0 || !fs->writable || nodep == 0 ||
            !memfs_v1_name_valid(name) || memfs_v1_slot(fs, dir, &parent) != 0 ||
            NODE_TYPE(&fs->nodes[parent]) != VFS_V1_TYPE_DIR ||
            (NODE_FLAGS(&fs->nodes[parent]) & MEMFS_V1_F_WRITABLE) == 0U)
                return -1;
        if (memfs_v1_find_child(fs, parent, name, 0) == 0 ||
            memfs_v1_free_slot(fs, &slot) != 0)
                return -1;
        np = &fs->nodes[slot];
        memfs_v1_clear_node(np);
        np->name = *name;
        node_set_meta(np, parent, type, mode,
            MEMFS_V1_F_USED | MEMFS_V1_F_WRITABLE);
        node_set_data(np, fs->used_words, 0U);
        *nodep = memfs_v1_node_handle(slot);
        return 0;
}

int
memfs_v1_create(struct memfs_v1 *fs, vnode_v1_t dir,
    const struct vfs_v1_name *name, unsigned int mode, vnode_v1_t *nodep)
{
        return memfs_v1_new_node(fs, dir, name, VFS_V1_TYPE_REG, mode,
            nodep);
}

int
memfs_v1_mkdir(struct memfs_v1 *fs, vnode_v1_t dir,
    const struct vfs_v1_name *name, unsigned int mode, vnode_v1_t *nodep)
{
        return memfs_v1_new_node(fs, dir, name, VFS_V1_TYPE_DIR, mode,
            nodep);
}

static int
memfs_v1_has_children(const struct memfs_v1 *fs, unsigned int slot)
{
        const struct memfs_v1_node *np;
        unsigned int i;

        np = fs->nodes + 1;
        for (i = 1U; i < fs->node_count; ++i, ++np) {
                if ((NODE_FLAGS(np) & MEMFS_V1_F_USED) != 0U &&
                    NODE_PARENT(np) == slot)
                        return 1;
        }
        return 0;
}

extern void memfs_v1_shift_after(struct memfs_v1 *fs, unsigned int start,
    int delta, unsigned int exclude);

extern int memfs_v1_resize(struct memfs_v1 *fs, unsigned int slot,
    unsigned int words);

int
memfs_v1_unlink(struct memfs_v1 *fs, vnode_v1_t dir,
    const struct vfs_v1_name *name)
{
        unsigned int parent;
        unsigned int slot;

        if (fs == 0 || !fs->writable || !memfs_v1_name_valid(name) ||
            memfs_v1_slot(fs, dir, &parent) != 0 ||
            NODE_TYPE(&fs->nodes[parent]) != VFS_V1_TYPE_DIR ||
            (NODE_FLAGS(&fs->nodes[parent]) & MEMFS_V1_F_WRITABLE) == 0U ||
            memfs_v1_find_child(fs, parent, name, &slot) != 0 ||
            memfs_v1_has_children(fs, slot))
                return -1;
        if (NODE_TYPE(&fs->nodes[slot]) == VFS_V1_TYPE_REG &&
            memfs_v1_resize(fs, slot, 0U) != 0)
                return -1;
        memfs_v1_clear_node(&fs->nodes[slot]);
        return 0;
}


int
memfs_v1_rename(struct memfs_v1 *fs, vnode_v1_t olddir,
    const struct vfs_v1_name *oldname, vnode_v1_t newdir,
    const struct vfs_v1_name *newname)
{
        unsigned int oldparent;
        unsigned int newparent;
        unsigned int slot;
        unsigned int p;

        if (fs == 0 || !fs->writable || !memfs_v1_name_valid(oldname) ||
            !memfs_v1_name_valid(newname) ||
            memfs_v1_slot(fs, olddir, &oldparent) != 0 ||
            memfs_v1_slot(fs, newdir, &newparent) != 0 ||
            NODE_TYPE(&fs->nodes[oldparent]) != VFS_V1_TYPE_DIR ||
            NODE_TYPE(&fs->nodes[newparent]) != VFS_V1_TYPE_DIR ||
            (NODE_FLAGS(&fs->nodes[oldparent]) & MEMFS_V1_F_WRITABLE) == 0U ||
            (NODE_FLAGS(&fs->nodes[newparent]) & MEMFS_V1_F_WRITABLE) == 0U ||
            memfs_v1_find_child(fs, oldparent, oldname, &slot) != 0 ||
            memfs_v1_find_child(fs, newparent, newname, 0) == 0)
                return -1;
        if (NODE_TYPE(&fs->nodes[slot]) == VFS_V1_TYPE_DIR) {
                p = newparent;
                for (;;) {
                        if (p == slot)
                                return -1;
                        if (p == 0U)
                                break;
                        if (p >= fs->node_count ||
                            (NODE_FLAGS(&fs->nodes[p]) & MEMFS_V1_F_USED) == 0U)
                                return -1;
                        p = NODE_PARENT(&fs->nodes[p]);
                }
        }
        node_set_parent(&fs->nodes[slot], newparent);
        fs->nodes[slot].name = *newname;
        return 0;
}

int
memfs_v1_truncate_words(struct memfs_v1 *fs, vnode_v1_t node,
    unsigned int words, kword_t size_chars)
{
        unsigned int slot;

        if (memfs_v1_slot(fs, node, &slot) != 0 ||
            NODE_TYPE(&fs->nodes[slot]) != VFS_V1_TYPE_REG ||
            memfs_v1_resize(fs, slot, words) != 0)
                return -1;
        fs->nodes[slot].size_chars = size_chars;
        return 0;
}

int
memfs_v1_read_words(const struct memfs_v1 *fs, vnode_v1_t node,
    unsigned int off, kword_t *buf, unsigned int nwords)
{
        unsigned int slot;
        unsigned int n;
        unsigned int i;
        const struct memfs_v1_node *np;
        const kword_t *src;

        if (buf == 0 || memfs_v1_slot(fs, node, &slot) != 0)
                return -1;
        np = &fs->nodes[slot];
        if (NODE_TYPE(np) != VFS_V1_TYPE_REG)
                return -1;
        if (off >= NODE_DATA_WORDS(np))
                return 0;
        n = NODE_DATA_WORDS(np) - off;
        if (n > nwords)
                n = nwords;
        if ((NODE_FLAGS(np) & MEMFS_V1_F_IMAGE) != 0U)
                src = fs->image_data + NODE_DATA_WORD(np);
        else
                src = fs->pool + NODE_DATA_WORD(np);
        for (i = 0U; i < n; ++i)
                buf[i] = src[off + i];
        return (int)n;
}

int
memfs_v1_write_words(struct memfs_v1 *fs, vnode_v1_t node,
    unsigned int off, const kword_t *buf, unsigned int nwords,
    kword_t size_chars)
{
        unsigned int slot;
        unsigned int need;
        unsigned int i;
        struct memfs_v1_node *np;

        if (buf == 0 || memfs_v1_slot(fs, node, &slot) != 0)
                return -1;
        np = &fs->nodes[slot];
        if (NODE_TYPE(np) != VFS_V1_TYPE_REG ||
            (NODE_FLAGS(np) & MEMFS_V1_F_WRITABLE) == 0U ||
            nwords > (~0U) - off)
                return -1;
        need = off + nwords;
        if (need > NODE_DATA_WORDS(np) && memfs_v1_resize(fs, slot, need) != 0)
                return -1;
        np = &fs->nodes[slot];
        {
                kword_t *dst;

                dst = fs->pool + NODE_DATA_WORD(np) + off;
                for (i = 0U; i < nwords; ++i)
                        dst[i] = buf[i];
        }
        if (size_chars > np->size_chars)
                np->size_chars = size_chars;
        return (int)nwords;
}
