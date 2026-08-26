#include "memfs_v1.h"

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
            (fs->nodes[slot].flags & MEMFS_V1_F_USED) == 0U)
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
        unsigned int i;

        for (i = 1U; i < fs->node_count; ++i) {
                if ((fs->nodes[i].flags & MEMFS_V1_F_USED) == 0U ||
                    fs->nodes[i].parent != parent)
                        continue;
                if (memfs_v1_name_equal(&fs->nodes[i].name, name)) {
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
        unsigned int i;

        for (i = 1U; i < fs->node_count; ++i) {
                if ((fs->nodes[i].flags & MEMFS_V1_F_USED) == 0U) {
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
        np->parent = 0U;
        np->type = 0U;
        np->mode = 0U;
        np->flags = 0U;
        np->size_chars = 0;
        np->data_word = 0U;
        np->data_words = 0U;
        np->image_data = 0;
}

int
memfs_v1_init(struct memfs_v1 *fs, struct memfs_v1_node *nodes,
    unsigned int node_count, kword_t *pool, unsigned int pool_words,
    int writable)
{
        unsigned int i;

        if (fs == 0 || nodes == 0 || node_count == 0U)
                return -1;
        if (pool_words != 0U && pool == 0)
                return -1;
        fs->nodes = nodes;
        fs->node_count = node_count;
        fs->pool = pool;
        fs->pool_words = pool_words;
        fs->used_words = 0U;
        fs->writable = writable != 0;
        for (i = 0U; i < node_count; ++i)
                memfs_v1_clear_node(&nodes[i]);
        nodes[0].flags = MEMFS_V1_F_USED;
        if (fs->writable)
                nodes[0].flags |= MEMFS_V1_F_WRITABLE;
        nodes[0].type = VFS_V1_TYPE_DIR;
        nodes[0].mode = fs->writable ? 0777U : 0555U;
        nodes[0].parent = 0U;
        return 0;
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
            fs->nodes[parent].type != VFS_V1_TYPE_DIR)
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
            fs->nodes[parent].type != VFS_V1_TYPE_DIR)
                return -1;
        n = 0U;
        for (i = 1U; i < fs->node_count; ++i) {
                if ((fs->nodes[i].flags & MEMFS_V1_F_USED) == 0U ||
                    fs->nodes[i].parent != parent)
                        continue;
                if (n++ != off)
                        continue;
                ent->name = fs->nodes[i].name;
                ent->type = fs->nodes[i].type;
                return 1;
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
        st->type = np->type;
        st->mode = np->mode;
        st->size_chars = np->size_chars;
        st->size_words = np->data_words;
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
        if (np->parent >= fs->node_count ||
            (fs->nodes[np->parent].flags & MEMFS_V1_F_USED) == 0U)
                return -1;
        *parentp = memfs_v1_node_handle(np->parent);
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
            fs->nodes[parent].type != VFS_V1_TYPE_DIR)
                return -1;
        if (memfs_v1_find_child(fs, parent, name, 0) == 0 ||
            memfs_v1_free_slot(fs, &slot) != 0)
                return -1;
        np = &fs->nodes[slot];
        memfs_v1_clear_node(np);
        np->name = *name;
        np->parent = parent;
        np->type = type;
        np->mode = mode;
        np->flags = MEMFS_V1_F_USED | MEMFS_V1_F_WRITABLE;
        np->data_word = fs->used_words;
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
        unsigned int i;

        for (i = 1U; i < fs->node_count; ++i) {
                if ((fs->nodes[i].flags & MEMFS_V1_F_USED) != 0U &&
                    fs->nodes[i].parent == slot)
                        return 1;
        }
        return 0;
}

static void
memfs_v1_shift_after(struct memfs_v1 *fs, unsigned int start,
    int delta, unsigned int exclude)
{
        unsigned int i;

        for (i = 1U; i < fs->node_count; ++i) {
                if (i == exclude ||
                    (fs->nodes[i].flags & MEMFS_V1_F_USED) == 0U ||
                    (fs->nodes[i].flags & MEMFS_V1_F_IMAGE) != 0U ||
                    fs->nodes[i].data_word < start)
                        continue;
                if (delta > 0)
                        fs->nodes[i].data_word += (unsigned int)delta;
                else
                        fs->nodes[i].data_word -= (unsigned int)(-delta);
        }
}

static int
memfs_v1_resize(struct memfs_v1 *fs, unsigned int slot,
    unsigned int words)
{
        struct memfs_v1_node *np;
        unsigned int old;
        unsigned int pos;
        unsigned int i;
        unsigned int delta;

        np = &fs->nodes[slot];
        if ((np->flags & MEMFS_V1_F_IMAGE) != 0U ||
            (np->flags & MEMFS_V1_F_WRITABLE) == 0U)
                return -1;
        old = np->data_words;
        if (words == old)
                return 0;
        pos = np->data_word + old;
        if (words > old) {
                delta = words - old;
                if (delta > fs->pool_words - fs->used_words)
                        return -1;
                i = fs->used_words;
                while (i > pos) {
                        --i;
                        fs->pool[i + delta] = fs->pool[i];
                }
                for (i = pos; i < pos + delta; ++i)
                        fs->pool[i] = 0;
                memfs_v1_shift_after(fs, pos, (int)delta, slot);
                fs->used_words += delta;
        } else {
                delta = old - words;
                pos = np->data_word + words;
                for (i = pos; i + delta < fs->used_words; ++i)
                        fs->pool[i] = fs->pool[i + delta];
                memfs_v1_shift_after(fs, np->data_word + old, -(int)delta, slot);
                fs->used_words -= delta;
        }
        np->data_words = words;
        return 0;
}

int
memfs_v1_unlink(struct memfs_v1 *fs, vnode_v1_t dir,
    const struct vfs_v1_name *name)
{
        unsigned int parent;
        unsigned int slot;

        if (fs == 0 || !fs->writable || !memfs_v1_name_valid(name) ||
            memfs_v1_slot(fs, dir, &parent) != 0 ||
            fs->nodes[parent].type != VFS_V1_TYPE_DIR ||
            memfs_v1_find_child(fs, parent, name, &slot) != 0 ||
            memfs_v1_has_children(fs, slot))
                return -1;
        if (fs->nodes[slot].type == VFS_V1_TYPE_REG &&
            memfs_v1_resize(fs, slot, 0U) != 0)
                return -1;
        memfs_v1_clear_node(&fs->nodes[slot]);
        return 0;
}

int
memfs_v1_truncate_words(struct memfs_v1 *fs, vnode_v1_t node,
    unsigned int words, kword_t size_chars)
{
        unsigned int slot;

        if (memfs_v1_slot(fs, node, &slot) != 0 ||
            fs->nodes[slot].type != VFS_V1_TYPE_REG ||
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
        if (np->type != VFS_V1_TYPE_REG)
                return -1;
        if (off >= np->data_words)
                return 0;
        n = np->data_words - off;
        if (n > nwords)
                n = nwords;
        if ((np->flags & MEMFS_V1_F_IMAGE) != 0U)
                src = np->image_data;
        else
                src = fs->pool + np->data_word;
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
        if (np->type != VFS_V1_TYPE_REG ||
            (np->flags & MEMFS_V1_F_WRITABLE) == 0U ||
            nwords > (~0U) - off)
                return -1;
        need = off + nwords;
        if (need > np->data_words && memfs_v1_resize(fs, slot, need) != 0)
                return -1;
        np = &fs->nodes[slot];
        for (i = 0U; i < nwords; ++i)
                fs->pool[np->data_word + off + i] = buf[i];
        if (size_chars > np->size_chars)
                np->size_chars = size_chars;
        return (int)nwords;
}

int
memfs_v1_import_node(struct memfs_v1 *fs, unsigned int slot,
    unsigned int parent, const struct vfs_v1_name *name, unsigned int type,
    unsigned int mode, const kword_t *data, unsigned int data_words,
    kword_t size_chars)
{
        struct memfs_v1_node *np;

        if (fs == 0 || slot == 0U || slot >= fs->node_count ||
            parent >= fs->node_count || !memfs_v1_name_valid(name) ||
            (type != VFS_V1_TYPE_DIR && type != VFS_V1_TYPE_REG) ||
            (fs->nodes[slot].flags & MEMFS_V1_F_USED) != 0U ||
            (fs->nodes[parent].flags & MEMFS_V1_F_USED) == 0U ||
            (type == VFS_V1_TYPE_REG && data_words != 0U && data == 0))
                return -1;
        np = &fs->nodes[slot];
        memfs_v1_clear_node(np);
        np->name = *name;
        np->parent = parent;
        np->type = type;
        np->mode = mode;
        np->flags = MEMFS_V1_F_USED | MEMFS_V1_F_IMAGE;
        np->data_words = data_words;
        np->size_chars = size_chars;
        np->image_data = data;
        return 0;
}
