#include "d6fs_provider_v2.h"

#define D6FS_PROVIDER_V2_SUPER_DISABLED 2U

struct d6fs_provider_v2_mount {
        d6fs_v2_read_block_fn read_block;
        d6fs_v2_write_block_fn write_block;
        void *opaque;
        struct d6fs_v2_super_info super;
        kword_t alloc_cursor;
        kword_t super_block[2];
        unsigned int super_copy; /* 0/1 active copy, otherwise disabled */
};

static struct d6fs_provider_v2_mount d6fs_provider_v2_mounts[VFS_V1_NMOUNT];
static struct d6fs_v2_reader d6fs_provider_v2_reader;
static unsigned int d6fs_provider_v2_reader_mount;

kword_t *
d6fs_provider_v2_block_buffer(void)
{
        return d6fs_provider_v2_reader.cache;
}

void
d6fs_provider_v2_cache_invalidate(void)
{
        d6fs_provider_v2_reader.cache_block = D6FS_V2_CACHE_INVALID;
}

static void d6fs_provider_v2_lcache_flush(void);
static struct d6fs_provider_v2_mount *d6fs_provider_v2_mount_for(
    vnode_v1_t node);

static int
d6fs_provider_v2_load(vnode_v1_t node)
{
        unsigned int id;
        struct d6fs_provider_v2_mount *mp;

        id = VFS_V1_MOUNT_ID(node);
        if (id == 0U || id > VFS_V1_NMOUNT)
                return -1;
        mp = &d6fs_provider_v2_mounts[id - 1U];
        if (mp->read_block == 0)
                return -1;
        if (d6fs_provider_v2_reader_mount != id) {
                if (d6fs_v2_reader_init(&d6fs_provider_v2_reader,
                    mp->read_block, mp->opaque, &mp->super) != 0)
                        return -1;
                (void)d6fs_v2_reader_set_writer(&d6fs_provider_v2_reader,
                    mp->write_block);
                d6fs_provider_v2_reader_mount = id;
        }
        return 0;
}

static int
d6fs_provider_v2_fcb(vnode_v1_t node, kword_t fcb[D6FS_V2_FCB_WORDS],
    struct d6fs_v2_fcb_info *info)
{
        if (d6fs_provider_v2_load(node) != 0 ||
            d6fs_v2_reader_fcb(&d6fs_provider_v2_reader,
            VFS_V1_INDEX(node), fcb) != 0 ||
            d6fs_v2_fcb_decode(fcb, info) != 0)
                return -1;
        return 0;
}

static unsigned int
d6fs_provider_v2_vtype(unsigned int type)
{
        if (type == D6FS_V2_TYPE_DIR)
                return VFS_V1_TYPE_DIR;
        if (type == D6FS_V2_TYPE_SYMLINK)
                return VFS_V1_TYPE_SYMLINK;
        return VFS_V1_TYPE_REG;
}

static unsigned int
d6fs_provider_v2_name_chars(const kword_t words[4])
{
        unsigned int chars;
        unsigned int wi;
        unsigned int si;
        kword_t ch;

        chars = VFS_V1_NAME_MAX_CHARS;
        while (chars != 0U) {
                wi = (chars - 1U) / 6U;
                si = (chars - 1U) % 6U;
                ch = (words[wi] >> ((5U - si) * 6U)) & 077UL;
                if (ch != 0UL)
                        break;
                --chars;
        }
        return chars;
}

static int
d6fs_provider_v2_dirent(vnode_v1_t dir, unsigned int slot,
    struct d6fs_v2_dirent_info *di)
{
        kword_t fcb[D6FS_V2_FCB_WORDS];
        struct d6fs_v2_fcb_info fi;
        kword_t raw[D6FS_V2_DIRENT_WORDS];
        kword_t off;
        int rc;

        if (di == 0 || d6fs_provider_v2_fcb(dir, fcb, &fi) != 0 ||
            fi.type != D6FS_V2_TYPE_DIR)
                return -1;
        off = (kword_t)slot * D6FS_V2_DIRENT_WORDS;
        if (off >= fi.size_words)
                return 0;
        if (fi.size_words - off < D6FS_V2_DIRENT_WORDS)
                return -1;
        rc = d6fs_v2_reader_read_words(&d6fs_provider_v2_reader, fcb, off,
            raw, D6FS_V2_DIRENT_WORDS);
        if (rc != (int)D6FS_V2_DIRENT_WORDS ||
            !d6fs_v2_dirent_valid(raw,
            d6fs_provider_v2_reader.super.fcb_count) ||
            d6fs_v2_dirent_decode(raw, di) != 0)
                return -1;
        return 1;
}

int
d6fs_provider_v2_mount_rw(vnode_v1_t target,
    d6fs_v2_read_block_fn read_block, d6fs_v2_write_block_fn write_block,
    void *opaque, const struct d6fs_v2_super_info *super, unsigned int flags,
    vnode_v1_t *rootp)
{
        vnode_v1_t root;
        unsigned int id;
        struct d6fs_provider_v2_mount *mp;

        if (read_block == 0 || super == 0 || rootp == 0 ||
            super->root_fcb >= super->fcb_count ||
            ((flags & VFS_V1_MOUNT_RDONLY) == 0U && write_block == 0) ||
            vfs_v1_mount(target, D6FS_V2_PROVIDER, D6FS_V2_KIND_NODE,
            super->root_fcb, flags, &root) != 0)
                return -1;
        id = VFS_V1_MOUNT_ID(root);
        mp = &d6fs_provider_v2_mounts[id - 1U];
        mp->read_block = read_block;
        mp->write_block = write_block;
        mp->opaque = opaque;
        mp->super = *super;
        mp->alloc_cursor = super->summary_start + super->summary_blocks;
        mp->super_block[0] = 0UL;
        mp->super_block[1] = 0UL;
        mp->super_copy = D6FS_PROVIDER_V2_SUPER_DISABLED;
        if (mp->alloc_cursor >= super->total_blocks)
                mp->alloc_cursor = 0UL;
        d6fs_provider_v2_reader_mount = 0U;
        d6fs_provider_v2_lcache_flush();
        if (d6fs_provider_v2_load(root) != 0) {
                (void)vfs_v1_unmount(root);
                mp->read_block = 0;
                mp->write_block = 0;
                return -1;
        }
        *rootp = root;
        return 0;
}

int
d6fs_provider_v2_mount(vnode_v1_t target,
    d6fs_v2_read_block_fn read_block, void *opaque,
    const struct d6fs_v2_super_info *super, unsigned int flags,
    vnode_v1_t *rootp)
{
        return d6fs_provider_v2_mount_rw(target, read_block, 0, opaque,
            super, flags | VFS_V1_MOUNT_RDONLY, rootp);
}

#ifndef D6FS_PROVIDER_V2_LOOKUP_CACHE
#define D6FS_PROVIDER_V2_LOOKUP_CACHE 8U
#endif
struct d6fs_provider_v2_lookup_cache {
        vnode_v1_t dir;
        kword_t hash;
        unsigned int slot;
};
static struct d6fs_provider_v2_lookup_cache
    d6fs_provider_v2_lcache[D6FS_PROVIDER_V2_LOOKUP_CACHE];

static int
d6fs_provider_v2_name_equal(const struct d6fs_v2_dirent_info *di,
    const struct vfs_v1_name *name)
{
        unsigned int i;

        for (i = 0U; i < VFS_V1_NAME_WORDS; ++i)
                if (di->name[i] != name->words[i])
                        return 0;
        return 1;
}

int
d6fs_provider_v2_lookup(vnode_v1_t dir, const struct vfs_v1_name *name,
    vnode_v1_t *nodep)
{
        struct d6fs_v2_dirent_info di;
        kword_t hash;
        unsigned int slot;
        unsigned int i;
        int rc;

        if (name == 0 || nodep == 0 || name->chars == 0U ||
            name->chars > VFS_V1_NAME_MAX_CHARS)
                return -1;
        hash = d6fs_v2_name_hash24(name->words, name->chars);
        i = (unsigned int)hash & (D6FS_PROVIDER_V2_LOOKUP_CACHE - 1U);
        if (d6fs_provider_v2_lcache[i].dir == dir &&
            d6fs_provider_v2_lcache[i].hash == hash) {
                rc = d6fs_provider_v2_dirent(dir,
                    d6fs_provider_v2_lcache[i].slot, &di);
                if (rc > 0 && di.child_fcb != 0U && di.hash == hash &&
                    d6fs_provider_v2_name_equal(&di, name)) {
                        *nodep = VFS_V1_NODE(D6FS_V2_PROVIDER,
                            VFS_V1_MOUNT_KIND(VFS_V1_MOUNT_ID(dir),
                            D6FS_V2_KIND_NODE), di.child_fcb);
                        return 0;
                }
                d6fs_provider_v2_lcache[i].dir = VFS_V1_NODE_NONE;
        }
        for (slot = 0U;; ++slot) {
                rc = d6fs_provider_v2_dirent(dir, slot, &di);
                if (rc <= 0)
                        return -1;
                if (di.child_fcb == 0U)
                        continue;
                if (di.hash != hash) {
                        continue;
                }
                if (!d6fs_provider_v2_name_equal(&di, name))
                        continue;
                i = (unsigned int)hash &
                    (D6FS_PROVIDER_V2_LOOKUP_CACHE - 1U);
                d6fs_provider_v2_lcache[i].dir = dir;
                d6fs_provider_v2_lcache[i].hash = hash;
                d6fs_provider_v2_lcache[i].slot = slot;
                *nodep = VFS_V1_NODE(D6FS_V2_PROVIDER,
                    VFS_V1_MOUNT_KIND(VFS_V1_MOUNT_ID(dir),
                    D6FS_V2_KIND_NODE), di.child_fcb);
                return 0;
        }
}

int
d6fs_provider_v2_readdir(vnode_v1_t dir, unsigned int off,
    struct vfs_v1_dirent *ent)
{
        struct d6fs_v2_dirent_info di;
        unsigned int slot;
        unsigned int seen;
        unsigned int i;
        int rc;

        if (ent == 0)
                return -1;
        seen = 0U;
        for (slot = 0U;; ++slot) {
                rc = d6fs_provider_v2_dirent(dir, slot, &di);
                if (rc == 0)
                        return 0;
                if (rc < 0)
                        return -1;
                if (di.child_fcb == 0U)
                        continue;
                if (seen++ != off)
                        continue;
                for (i = 0U; i < VFS_V1_NAME_WORDS; ++i)
                        ent->name.words[i] = di.name[i];
                ent->name.chars = d6fs_provider_v2_name_chars(di.name);
                ent->type = d6fs_provider_v2_vtype(di.type);
                return 1;
        }
}

int
d6fs_provider_v2_stat(vnode_v1_t node, struct vfs_v1_stat *st)
{
        kword_t fcb[D6FS_V2_FCB_WORDS];
        struct d6fs_v2_fcb_info fi;

        if (st == 0 || d6fs_provider_v2_fcb(node, fcb, &fi) != 0 ||
            fi.type == D6FS_V2_TYPE_FREE)
                return -1;
        st->type = d6fs_provider_v2_vtype(fi.type);
        st->mode = fi.mode;
        st->size_words = fi.size_words;
        if (fi.size_words == 0UL)
                st->size_chars = 0UL;
        else if (fi.type == D6FS_V2_TYPE_SYMLINK)
                st->size_chars = (fi.size_words - 1UL) * 6UL +
                    (fi.tail != 0U ? fi.tail : 6U);
        else if (fi.tail != 0U)
                st->size_chars = (fi.size_words - 1UL) * 4UL + fi.tail;
        else
                st->size_chars = fi.size_words * 4UL;
        return 0;
}

int
d6fs_provider_v2_parent(vnode_v1_t node, vnode_v1_t *parentp)
{
        kword_t fcb[D6FS_V2_FCB_WORDS];
        struct d6fs_v2_fcb_info fi;

        if (parentp == 0 || d6fs_provider_v2_fcb(node, fcb, &fi) != 0)
                return -1;
        if (VFS_V1_INDEX(node) == d6fs_provider_v2_reader.super.root_fcb) {
                *parentp = node;
                return 0;
        }
        if (fi.parent_fcb >= d6fs_provider_v2_reader.super.fcb_count)
                return -1;
        *parentp = VFS_V1_NODE(D6FS_V2_PROVIDER,
            VFS_V1_MOUNT_KIND(VFS_V1_MOUNT_ID(node), D6FS_V2_KIND_NODE),
            fi.parent_fcb);
        return 0;
}

int
d6fs_provider_v2_parent_name(vnode_v1_t node, vnode_v1_t *parentp,
    struct vfs_v1_name *namep)
{
        vnode_v1_t parent;
        struct d6fs_v2_dirent_info di;
        unsigned int slot;
        unsigned int i;
        int rc;

        if (parentp == 0 || namep == 0 ||
            d6fs_provider_v2_parent(node, &parent) != 0 || parent == node)
                return -1;
        for (slot = 0U;; ++slot) {
                rc = d6fs_provider_v2_dirent(parent, slot, &di);
                if (rc <= 0)
                        return -1;
                if (di.child_fcb != VFS_V1_INDEX(node))
                        continue;
                namep->chars = d6fs_provider_v2_name_chars(di.name);
                for (i = 0U; i < VFS_V1_NAME_WORDS; ++i)
                        namep->words[i] = di.name[i];
                *parentp = parent;
                return 0;
        }
}

int
d6fs_provider_v2_read_words(vnode_v1_t node, unsigned int off,
    kword_t *buf, unsigned int nwords)
{
        kword_t fcb[D6FS_V2_FCB_WORDS];
        struct d6fs_v2_fcb_info fi;

        if (buf == 0 || d6fs_provider_v2_fcb(node, fcb, &fi) != 0 ||
            (fi.type != D6FS_V2_TYPE_REG && fi.type != D6FS_V2_TYPE_SYMLINK))
                return -1;
        return d6fs_v2_reader_read_words(&d6fs_provider_v2_reader, fcb,
            off, buf, nwords);
}

int
d6fs_provider_v2_sync(vnode_v1_t node)
{
        return d6fs_provider_v2_load(node);
}

static int
d6fs_provider_v2_write_super(struct d6fs_provider_v2_mount *mp,
    unsigned int copy, unsigned int state)
{
        const kword_t *cached;
        kword_t block[D6FS_V2_BLOCK_WORDS];
        kword_t raw[D6FS_V2_SUPER_WORDS];
        struct d6fs_v2_super_info next;
        unsigned int i;

        if (mp == 0 || copy > 1U || mp->write_block == 0 ||
            d6fs_provider_v2_reader_mount == 0U)
                return -1;
        next = mp->super;
        ++next.sequence;
        next.state = state;
        if (d6fs_v2_super_encode(raw, &next) != 0 ||
            d6fs_v2_reader_get_block(&d6fs_provider_v2_reader,
            mp->super_block[copy], &cached) != 0)
                return -1;
        for (i = 0U; i < D6FS_V2_BLOCK_WORDS; ++i)
                block[i] = cached[i];
        for (i = 0U; i < D6FS_V2_SUPER_WORDS; ++i)
                block[i] = raw[i];
        if (d6fs_v2_reader_write_block(&d6fs_provider_v2_reader,
            mp->super_block[copy], block) != 0)
                return -1;
        mp->super = next;
        d6fs_provider_v2_reader.super = next;
        mp->super_copy = copy;
        return 0;
}

int
d6fs_provider_v2_enable_state(vnode_v1_t root, kword_t super_a,
    kword_t super_b, unsigned int selected_copy)
{
        struct d6fs_provider_v2_mount *mp;

        if (selected_copy > 1U || super_a == super_b ||
            d6fs_provider_v2_load(root) != 0)
                return -1;
        mp = d6fs_provider_v2_mount_for(root);
        if (mp == 0 || mp->write_block == 0 ||
            mp->super.state != D6FS_V2_STATE_CLEAN)
                return -1;
        mp->super_block[0] = super_a;
        mp->super_block[1] = super_b;
        mp->super_copy = selected_copy;
        if (d6fs_provider_v2_write_super(mp, selected_copy ^ 1U,
            D6FS_V2_STATE_DIRTY) != 0) {
                mp->super_copy = D6FS_PROVIDER_V2_SUPER_DISABLED;
                return -1;
        }
        return 0;
}

int
d6fs_provider_v2_prepare_unmount(vnode_v1_t root)
{
        struct d6fs_provider_v2_mount *mp;
        unsigned int id;

        id = VFS_V1_MOUNT_ID(root);
        mp = d6fs_provider_v2_mount_for(root);
        if (mp == 0)
                return -1;
        if (mp->super_copy != D6FS_PROVIDER_V2_SUPER_DISABLED && mp->write_block != 0 &&
            d6fs_provider_v2_load(root) == 0 &&
            d6fs_provider_v2_write_super(mp, mp->super_copy ^ 1U,
            D6FS_V2_STATE_CLEAN) != 0)
                return -1;
        mp->read_block = 0;
        mp->write_block = 0;
        mp->super_copy = D6FS_PROVIDER_V2_SUPER_DISABLED;
        if (d6fs_provider_v2_reader_mount == id)
                d6fs_provider_v2_reader_mount = 0U;
        d6fs_provider_v2_lcache_flush();
        return 0;
}

static void
d6fs_provider_v2_lcache_flush(void)
{
        unsigned int i;

        for (i = 0U; i < D6FS_PROVIDER_V2_LOOKUP_CACHE; ++i)
                d6fs_provider_v2_lcache[i].dir = VFS_V1_NODE_NONE;
}

static struct d6fs_provider_v2_mount *
d6fs_provider_v2_mount_for(vnode_v1_t node)
{
        unsigned int id;

        id = VFS_V1_MOUNT_ID(node);
        if (id == 0U || id > VFS_V1_NMOUNT)
                return 0;
        return &d6fs_provider_v2_mounts[id - 1U];
}

static int
d6fs_provider_v2_writable(vnode_v1_t node)
{
        struct d6fs_provider_v2_mount *mp;

        mp = d6fs_provider_v2_mount_for(node);
        return mp != 0 && mp->read_block != 0 && mp->write_block != 0;
}

static int
d6fs_provider_v2_set_extent(kword_t fcb[D6FS_V2_FCB_WORDS],
    unsigned int index, kword_t start, kword_t blocks)
{
        kword_t run;
        unsigned int high;

        if (index >= D6FS_V2_EXTENTS ||
            d6fs_v2_extent_encode(start, blocks, &run, &high) != 0)
                return -1;
        fcb[D6FS_V2_FCB_EXTENT0 + index] = run;
        return d6fs_v2_extent_high_set(&fcb[D6FS_V2_FCB_LENHIGH], index,
            high);
}

static int
d6fs_provider_v2_clear_extent(kword_t fcb[D6FS_V2_FCB_WORDS],
    unsigned int index)
{
        if (index >= D6FS_V2_EXTENTS)
                return -1;
        fcb[D6FS_V2_FCB_EXTENT0 + index] = 0UL;
        return d6fs_v2_extent_high_set(&fcb[D6FS_V2_FCB_LENHIGH], index, 0U);
}

static kword_t
d6fs_provider_v2_blocks_for_words(kword_t words)
{
        if (words == 0UL)
                return 0UL;
        return (words + (kword_t)D6FS_V2_BLOCK_WORDS - 1UL) /
            (kword_t)D6FS_V2_BLOCK_WORDS;
}

static int
d6fs_provider_v2_free_file_tail(const kword_t fcb[D6FS_V2_FCB_WORDS],
    kword_t first_file_block)
{
        kword_t logical;
        kword_t file_block;

        file_block = first_file_block;
        while (d6fs_v2_file_block(fcb, file_block, &logical) == 0) {
                if (d6fs_v2_freemap_set(&d6fs_provider_v2_reader,
                    logical, 0U) != 0)
                        return -1;
                ++file_block;
        }
        return 0;
}

static int
d6fs_provider_v2_resize_fcb(vnode_v1_t node,
    kword_t fcb[D6FS_V2_FCB_WORDS], struct d6fs_v2_fcb_info *fi,
    kword_t new_words, unsigned int new_tail)
{
        struct d6fs_provider_v2_mount *mp;
        kword_t old_fcb[D6FS_V2_FCB_WORDS];
        struct d6fs_v2_fcb_info work;
        kword_t old_blocks;
        kword_t new_blocks;
        kword_t have_blocks;
        kword_t need;
        kword_t start;
        kword_t blocks;
        kword_t last_start;
        kword_t last_blocks;
        kword_t candidate;
        unsigned int allocated;
        unsigned int high;
        unsigned int i;

        if (fcb == 0 || fi == 0 || !d6fs_provider_v2_writable(node) ||
            new_tail > (fi->type == D6FS_V2_TYPE_SYMLINK ? 6U : 4U) ||
            (new_words == 0UL && new_tail != 0U))
                return -1;
        mp = d6fs_provider_v2_mount_for(node);
        if (mp == 0)
                return -1;
        for (i = 0U; i < D6FS_V2_FCB_WORDS; ++i)
                old_fcb[i] = fcb[i];
        work = *fi;
        old_blocks = d6fs_provider_v2_blocks_for_words(fi->size_words);
        new_blocks = d6fs_provider_v2_blocks_for_words(new_words);
        have_blocks = old_blocks;

        if (new_blocks > old_blocks) {
                need = new_blocks - old_blocks;
                if (work.extent_count != 0U) {
                        i = work.extent_count - 1U;
                        high = d6fs_v2_extent_high_get(
                            fcb[D6FS_V2_FCB_LENHIGH], i);
                        if (d6fs_v2_extent_decode(
                            fcb[D6FS_V2_FCB_EXTENT0 + i], high,
                            &last_start, &last_blocks) != 0)
                                return -1;
                        while (need != 0UL &&
                            last_blocks < D6FS_V2_EXTENT_MAX_BLOCKS) {
                                candidate = last_start + last_blocks;
                                if (candidate >= d6fs_provider_v2_reader.super.total_blocks ||
                                    d6fs_v2_freemap_test(&d6fs_provider_v2_reader,
                                    candidate, &allocated) != 0 || allocated)
                                        break;
                                if (d6fs_v2_freemap_set(&d6fs_provider_v2_reader,
                                    candidate, 1U) != 0 ||
                                    d6fs_v2_reader_zero_block(
                                    &d6fs_provider_v2_reader, candidate) != 0) {
                                        (void)d6fs_v2_freemap_set(
                                            &d6fs_provider_v2_reader,
                                            candidate, 0U);
                                        (void)d6fs_provider_v2_free_file_tail(
                                            fcb, old_blocks);
                                        return -1;
                                }
                                ++last_blocks;
                                ++have_blocks;
                                --need;
                        }
                        if (d6fs_provider_v2_set_extent(fcb, i, last_start,
                            last_blocks) != 0)
                                return -1;
                }
                while (need != 0UL) {
                        if (work.extent_count >= D6FS_V2_EXTENTS) {
                                (void)d6fs_provider_v2_free_file_tail(fcb,
                                    old_blocks);
                                for (i = 0U; i < D6FS_V2_FCB_WORDS; ++i)
                                        fcb[i] = old_fcb[i];
                                return -1;
                        }
                        blocks = need;
                        if (blocks > D6FS_V2_EXTENT_MAX_BLOCKS)
                                blocks = D6FS_V2_EXTENT_MAX_BLOCKS;
                        if (d6fs_v2_alloc_run(&d6fs_provider_v2_reader,
                            mp->alloc_cursor, blocks, &start, &blocks) != 0) {
                                (void)d6fs_provider_v2_free_file_tail(fcb,
                                    old_blocks);
                                for (i = 0U; i < D6FS_V2_FCB_WORDS; ++i)
                                        fcb[i] = old_fcb[i];
                                return -1;
                        }
                        for (candidate = 0UL; candidate < blocks; ++candidate)
                                if (d6fs_v2_reader_zero_block(
                                    &d6fs_provider_v2_reader,
                                    start + candidate) != 0) {
                                        (void)d6fs_v2_free_run(
                                            &d6fs_provider_v2_reader, start,
                                            blocks);
                                        (void)d6fs_provider_v2_free_file_tail(
                                            fcb, old_blocks);
                                        for (i = 0U;
                                            i < D6FS_V2_FCB_WORDS; ++i)
                                                fcb[i] = old_fcb[i];
                                        return -1;
                                }
                        i = work.extent_count;
                        if (d6fs_provider_v2_set_extent(fcb, i, start,
                            blocks) != 0)
                                return -1;
                        ++work.extent_count;
                        have_blocks += blocks;
                        need -= blocks;
                        mp->alloc_cursor = start + blocks;
                        if (mp->alloc_cursor >=
                            d6fs_provider_v2_reader.super.total_blocks)
                                mp->alloc_cursor = 0UL;
                }
        } else if (new_blocks < old_blocks) {
                kword_t keep;
                kword_t extent_start;
                kword_t extent_blocks;
                kword_t consumed;
                unsigned int new_count;

                keep = new_blocks;
                consumed = 0UL;
                new_count = 0U;
                for (i = 0U; i < fi->extent_count; ++i) {
                        high = d6fs_v2_extent_high_get(
                            old_fcb[D6FS_V2_FCB_LENHIGH], i);
                        if (d6fs_v2_extent_decode(
                            old_fcb[D6FS_V2_FCB_EXTENT0 + i], high,
                            &extent_start, &extent_blocks) != 0)
                                return -1;
                        if (consumed >= keep) {
                                (void)d6fs_provider_v2_clear_extent(fcb, i);
                                continue;
                        }
                        if (extent_blocks > keep - consumed)
                                extent_blocks = keep - consumed;
                        if (extent_blocks != 0UL) {
                                if (d6fs_provider_v2_set_extent(fcb, i,
                                    extent_start, extent_blocks) != 0)
                                        return -1;
                                ++new_count;
                        } else {
                                (void)d6fs_provider_v2_clear_extent(fcb, i);
                        }
                        consumed += extent_blocks;
                }
                for (i = new_count; i < D6FS_V2_EXTENTS; ++i)
                        (void)d6fs_provider_v2_clear_extent(fcb, i);
                work.extent_count = new_count;
                if (d6fs_v2_fcb_encode(fcb, &work) != 0)
                        return -1;
                /* Reapply retained extents after metadata encode cleared them. */
                consumed = 0UL;
                new_count = 0U;
                for (i = 0U; i < fi->extent_count && consumed < keep; ++i) {
                        high = d6fs_v2_extent_high_get(
                            old_fcb[D6FS_V2_FCB_LENHIGH], i);
                        if (d6fs_v2_extent_decode(
                            old_fcb[D6FS_V2_FCB_EXTENT0 + i], high,
                            &extent_start, &extent_blocks) != 0)
                                return -1;
                        if (extent_blocks > keep - consumed)
                                extent_blocks = keep - consumed;
                        if (extent_blocks != 0UL) {
                                if (d6fs_provider_v2_set_extent(fcb,
                                    new_count, extent_start, extent_blocks) != 0)
                                        return -1;
                                consumed += extent_blocks;
                                ++new_count;
                        }
                }
        }

        work.size_words = new_words;
        work.tail = new_words == 0UL ? 0U : new_tail;
        work.extent_count = new_blocks == 0UL ? 0U : work.extent_count;
        if (new_blocks > old_blocks)
                work.extent_count = 0U;
        if (new_blocks > old_blocks) {
                for (i = 0U; i < D6FS_V2_EXTENTS; ++i)
                        if (fcb[D6FS_V2_FCB_EXTENT0 + i] != 0UL)
                                ++work.extent_count;
        }
        /* Encode metadata without destroying the already-built extent words. */
        fcb[D6FS_V2_FCB_META] = ((kword_t)work.type << 33) |
            ((kword_t)work.flags << 24) | ((kword_t)work.mode << 12) |
            ((kword_t)work.tail << 8) | ((kword_t)work.extent_count << 4);
        fcb[D6FS_V2_FCB_OWNER] = ((kword_t)work.uid << 18) |
            (kword_t)work.gid;
        fcb[D6FS_V2_FCB_SIZE] = work.size_words;
        fcb[D6FS_V2_FCB_MTIME] = work.mtime;
        fcb[D6FS_V2_FCB_PARENT] = (kword_t)work.parent_fcb << 18;
        if (d6fs_v2_reader_put_fcb(&d6fs_provider_v2_reader,
            VFS_V1_INDEX(node), fcb) != 0) {
                if (new_blocks > old_blocks) {
                        (void)d6fs_provider_v2_free_file_tail(fcb, old_blocks);
                        for (i = 0U; i < D6FS_V2_FCB_WORDS; ++i)
                                fcb[i] = old_fcb[i];
                }
                return -1;
        }
        if (new_blocks < old_blocks &&
            d6fs_provider_v2_free_file_tail(old_fcb, new_blocks) != 0)
                return -1;
        *fi = work;
        (void)have_blocks;
        return 0;
}

static int
d6fs_provider_v2_find_slot(vnode_v1_t dir, const struct vfs_v1_name *name,
    unsigned int *slotp, struct d6fs_v2_dirent_info *dip)
{
        struct d6fs_v2_dirent_info di;
        kword_t hash;
        unsigned int slot;
        int rc;

        if (name == 0 || name->chars == 0U ||
            name->chars > VFS_V1_NAME_MAX_CHARS)
                return -1;
        hash = d6fs_v2_name_hash24(name->words, name->chars);
        for (slot = 0U;; ++slot) {
                rc = d6fs_provider_v2_dirent(dir, slot, &di);
                if (rc == 0)
                        return 1;
                if (rc < 0)
                        return -1;
                if (di.child_fcb == 0U || di.hash != hash)
                        continue;
                if (!d6fs_provider_v2_name_equal(&di, name))
                        continue;
                if (slotp != 0)
                        *slotp = slot;
                if (dip != 0)
                        *dip = di;
                return 0;
        }
}

static int
d6fs_provider_v2_empty_slot(vnode_v1_t dir, unsigned int *slotp)
{
        struct d6fs_v2_dirent_info di;
        unsigned int slot;
        int rc;

        if (slotp == 0)
                return -1;
        for (slot = 0U;; ++slot) {
                rc = d6fs_provider_v2_dirent(dir, slot, &di);
                if (rc == 0) {
                        *slotp = slot;
                        return 1;
                }
                if (rc < 0)
                        return -1;
                if (di.child_fcb == 0U) {
                        *slotp = slot;
                        return 0;
                }
        }
}

static int
d6fs_provider_v2_free_fcb(unsigned int *indexp)
{
        kword_t fcb[D6FS_V2_FCB_WORDS];
        struct d6fs_v2_fcb_info fi;
        unsigned int i;

        if (indexp == 0)
                return -1;
        for (i = 1U; i < d6fs_provider_v2_reader.super.fcb_count; ++i) {
                if (d6fs_v2_reader_fcb(&d6fs_provider_v2_reader, i, fcb) != 0 ||
                    d6fs_v2_fcb_decode(fcb, &fi) != 0)
                        continue;
                if (fi.type == D6FS_V2_TYPE_FREE) {
                        *indexp = i;
                        return 0;
                }
        }
        return -1;
}

static int
d6fs_provider_v2_write_dirent(vnode_v1_t dir, unsigned int slot,
    const struct d6fs_v2_dirent_info *di)
{
        kword_t fcb[D6FS_V2_FCB_WORDS];
        struct d6fs_v2_fcb_info fi;
        kword_t raw[D6FS_V2_DIRENT_WORDS];
        kword_t need;
        unsigned int tail;

        if (di == 0 || d6fs_provider_v2_fcb(dir, fcb, &fi) != 0 ||
            fi.type != D6FS_V2_TYPE_DIR ||
            (fi.flags & D6FS_V2_FLAG_IMMUTABLE) != 0U ||
            d6fs_v2_dirent_encode(raw, di) != 0)
                return -1;
        need = ((kword_t)slot + 1UL) * D6FS_V2_DIRENT_WORDS;
        if (need > fi.size_words) {
                tail = 4U;
                if (d6fs_provider_v2_resize_fcb(dir, fcb, &fi, need, tail) != 0)
                        return -1;
        }
        return d6fs_v2_reader_write_words(&d6fs_provider_v2_reader, fcb,
            (kword_t)slot * D6FS_V2_DIRENT_WORDS, raw,
            D6FS_V2_DIRENT_WORDS) == (int)D6FS_V2_DIRENT_WORDS ? 0 : -1;
}

static int
d6fs_provider_v2_create_type(vnode_v1_t dir,
    const struct vfs_v1_name *name, unsigned int mode, unsigned int type,
    vnode_v1_t *nodep)
{
        kword_t fcb[D6FS_V2_FCB_WORDS];
        struct d6fs_v2_fcb_info fi;
        struct d6fs_v2_dirent_info di;
        unsigned int index;
        unsigned int slot;
        unsigned int append_slot;
        unsigned int i;

        if (nodep == 0 || !d6fs_provider_v2_writable(dir) ||
            (type != D6FS_V2_TYPE_REG && type != D6FS_V2_TYPE_DIR &&
            type != D6FS_V2_TYPE_SYMLINK) ||
            d6fs_provider_v2_find_slot(dir, name, 0, 0) == 0 ||
            d6fs_provider_v2_free_fcb(&index) != 0)
                return -1;
        append_slot = (unsigned int)d6fs_provider_v2_empty_slot(dir, &slot);
        if ((int)append_slot < 0)
                return -1;
        for (i = 0U; i < D6FS_V2_FCB_WORDS; ++i)
                fcb[i] = 0UL;
        fi.type = type;
        fi.flags = 0U;
        fi.mode = mode & 07777U;
        fi.tail = 0U;
        fi.extent_count = 0U;
        fi.uid = 0U;
        fi.gid = 0U;
        fi.size_words = 0UL;
        fi.mtime = 0UL;
        fi.parent_fcb = VFS_V1_INDEX(dir);
        if (d6fs_v2_fcb_encode(fcb, &fi) != 0 ||
            d6fs_v2_reader_put_fcb(&d6fs_provider_v2_reader, index, fcb) != 0)
                return -1;
        for (i = 0U; i < 4U; ++i)
                di.name[i] = name->words[i];
        di.hash = d6fs_v2_name_hash24(name->words, name->chars);
        di.type = type;
        di.flags = 0U;
        di.child_fcb = index;
        if (d6fs_provider_v2_write_dirent(dir, slot, &di) != 0) {
                for (i = 0U; i < D6FS_V2_FCB_WORDS; ++i)
                        fcb[i] = 0UL;
                (void)d6fs_v2_reader_put_fcb(&d6fs_provider_v2_reader,
                    index, fcb);
                return -1;
        }
        d6fs_provider_v2_lcache_flush();
        *nodep = VFS_V1_NODE(D6FS_V2_PROVIDER,
            VFS_V1_MOUNT_KIND(VFS_V1_MOUNT_ID(dir), D6FS_V2_KIND_NODE), index);
        (void)append_slot;
        return 0;
}

int
d6fs_provider_v2_create(vnode_v1_t dir, const struct vfs_v1_name *name,
    unsigned int mode, vnode_v1_t *nodep)
{
        return d6fs_provider_v2_create_type(dir, name, mode,
            D6FS_V2_TYPE_REG, nodep);
}

int
d6fs_provider_v2_mkdir(vnode_v1_t dir, const struct vfs_v1_name *name,
    unsigned int mode, vnode_v1_t *nodep)
{
        return d6fs_provider_v2_create_type(dir, name, mode,
            D6FS_V2_TYPE_DIR, nodep);
}

int
d6fs_provider_v2_symlink(vnode_v1_t dir, const struct vfs_v1_name *name,
    const kword_t *target, unsigned int target_chars, vnode_v1_t *nodep)
{
        kword_t fcb[D6FS_V2_FCB_WORDS];
        struct d6fs_v2_fcb_info fi;
        struct d6fs_v2_dirent_info di;
        vnode_v1_t node;
        unsigned int index;
        unsigned int slot;
        unsigned int words;
        unsigned int tail;
        unsigned int i;
        int append_slot;

        if (target == 0 || target_chars == 0U || nodep == 0 ||
            !d6fs_provider_v2_writable(dir) ||
            d6fs_provider_v2_find_slot(dir, name, 0, 0) == 0 ||
            d6fs_provider_v2_free_fcb(&index) != 0)
                return -1;
        append_slot = d6fs_provider_v2_empty_slot(dir, &slot);
        if (append_slot < 0)
                return -1;
        words = (target_chars + 5U) / 6U;
        tail = target_chars - (words - 1U) * 6U;
        for (i = 0U; i < D6FS_V2_FCB_WORDS; ++i)
                fcb[i] = 0UL;
        fi.type = D6FS_V2_TYPE_SYMLINK;
        fi.flags = 0U;
        fi.mode = 0777U;
        fi.tail = 0U;
        fi.extent_count = 0U;
        fi.uid = 0U;
        fi.gid = 0U;
        fi.size_words = 0UL;
        fi.mtime = 0UL;
        fi.parent_fcb = VFS_V1_INDEX(dir);
        if (d6fs_v2_fcb_encode(fcb, &fi) != 0 ||
            d6fs_v2_reader_put_fcb(&d6fs_provider_v2_reader, index, fcb) != 0)
                return -1;
        node = VFS_V1_NODE(D6FS_V2_PROVIDER,
            VFS_V1_MOUNT_KIND(VFS_V1_MOUNT_ID(dir), D6FS_V2_KIND_NODE), index);
        if (d6fs_provider_v2_resize_fcb(node, fcb, &fi, words, tail) != 0 ||
            d6fs_provider_v2_fcb(node, fcb, &fi) != 0 ||
            d6fs_v2_reader_write_words(&d6fs_provider_v2_reader, fcb, 0UL,
            target, words) != (int)words)
                goto fail;
        for (i = 0U; i < 4U; ++i)
                di.name[i] = name->words[i];
        di.hash = d6fs_v2_name_hash24(name->words, name->chars);
        di.type = D6FS_V2_TYPE_SYMLINK;
        di.flags = 0U;
        di.child_fcb = index;
        if (d6fs_provider_v2_write_dirent(dir, slot, &di) != 0)
                goto fail;
        d6fs_provider_v2_lcache_flush();
        *nodep = node;
        (void)append_slot;
        return 0;

fail:
        if (d6fs_provider_v2_fcb(node, fcb, &fi) == 0)
                (void)d6fs_provider_v2_resize_fcb(node, fcb, &fi, 0UL, 0U);
        for (i = 0U; i < D6FS_V2_FCB_WORDS; ++i)
                fcb[i] = 0UL;
        (void)d6fs_v2_reader_put_fcb(&d6fs_provider_v2_reader, index, fcb);
        return -1;
}

int
d6fs_provider_v2_unlink(vnode_v1_t dir, const struct vfs_v1_name *name)
{
        struct d6fs_v2_dirent_info di;
        struct d6fs_v2_dirent_info zero_di;
        kword_t fcb[D6FS_V2_FCB_WORDS];
        kword_t old_fcb[D6FS_V2_FCB_WORDS];
        struct d6fs_v2_fcb_info fi;
        unsigned int slot;
        unsigned int i;

        if (!d6fs_provider_v2_writable(dir) ||
            d6fs_provider_v2_find_slot(dir, name, &slot, &di) != 0 ||
            d6fs_v2_reader_fcb(&d6fs_provider_v2_reader, di.child_fcb,
            fcb) != 0 || d6fs_v2_fcb_decode(fcb, &fi) != 0 ||
            (fi.flags & (D6FS_V2_FLAG_NOUNLINK | D6FS_V2_FLAG_IMMUTABLE)) != 0U)
                return -1;
        for (i = 0U; i < D6FS_V2_FCB_WORDS; ++i)
                old_fcb[i] = fcb[i];
        if (fi.type == D6FS_V2_TYPE_DIR && fi.size_words != 0UL) {
                struct d6fs_v2_dirent_info child;
                unsigned int s;
                int rc;

                for (s = 0U;; ++s) {
                        rc = d6fs_provider_v2_dirent(VFS_V1_NODE(
                            D6FS_V2_PROVIDER,
                            VFS_V1_MOUNT_KIND(VFS_V1_MOUNT_ID(dir),
                            D6FS_V2_KIND_NODE), di.child_fcb), s, &child);
                        if (rc == 0)
                                break;
                        if (rc < 0 || child.child_fcb != 0U)
                                return -1;
                }
        }
        for (i = 0U; i < 4U; ++i)
                zero_di.name[i] = 0UL;
        zero_di.hash = 0UL;
        zero_di.type = 0U;
        zero_di.flags = 0U;
        zero_di.child_fcb = 0U;
        if (d6fs_provider_v2_write_dirent(dir, slot, &zero_di) != 0)
                return -1;
        for (i = 0U; i < D6FS_V2_FCB_WORDS; ++i)
                fcb[i] = 0UL;
        if (d6fs_v2_reader_put_fcb(&d6fs_provider_v2_reader,
            di.child_fcb, fcb) != 0)
                return -1;
        if (d6fs_provider_v2_free_file_tail(old_fcb, 0UL) != 0) {
                /* Any leaked blocks are fsck-recoverable. */
        }
        d6fs_provider_v2_lcache_flush();
        return 0;
}

int
d6fs_provider_v2_rename(vnode_v1_t olddir,
    const struct vfs_v1_name *oldname, vnode_v1_t newdir,
    const struct vfs_v1_name *newname)
{
        struct d6fs_v2_dirent_info di;
        struct d6fs_v2_dirent_info old_di;
        struct d6fs_v2_dirent_info zero_di;
        kword_t fcb[D6FS_V2_FCB_WORDS];
        struct d6fs_v2_fcb_info fi;
        unsigned int old_parent;
        unsigned int oldslot;
        unsigned int newslot;
        unsigned int i;
        int empty_rc;

        if (!d6fs_provider_v2_writable(olddir) ||
            VFS_V1_MOUNT_ID(olddir) != VFS_V1_MOUNT_ID(newdir) ||
            d6fs_provider_v2_find_slot(olddir, oldname, &oldslot, &di) != 0 ||
            d6fs_provider_v2_find_slot(newdir, newname, 0, 0) == 0 ||
            d6fs_v2_reader_fcb(&d6fs_provider_v2_reader, di.child_fcb,
            fcb) != 0 || d6fs_v2_fcb_decode(fcb, &fi) != 0 ||
            (fi.flags & (D6FS_V2_FLAG_NOUNLINK | D6FS_V2_FLAG_IMMUTABLE)) != 0U)
                return -1;
        old_di = di;
        old_parent = fi.parent_fcb;
        if (olddir == newdir) {
                newslot = oldslot;
        } else {
                empty_rc = d6fs_provider_v2_empty_slot(newdir, &newslot);
                if (empty_rc < 0)
                        return -1;
        }
        for (i = 0U; i < 4U; ++i)
                di.name[i] = newname->words[i];
        di.hash = d6fs_v2_name_hash24(newname->words, newname->chars);
        if (olddir == newdir) {
                if (d6fs_provider_v2_write_dirent(newdir, newslot, &di) != 0)
                        return -1;
        } else {
                for (i = 0U; i < 4U; ++i)
                        zero_di.name[i] = 0UL;
                zero_di.hash = 0UL;
                zero_di.type = 0U;
                zero_di.flags = 0U;
                zero_di.child_fcb = 0U;
                /*
                 * Remove the old namespace reference first.  A crash from
                 * this point until publication in the new directory can
                 * leave an orphaned FCB, which fsck can repair, but never a
                 * transient second hard-link-like reference.
                 */
                if (d6fs_provider_v2_write_dirent(olddir, oldslot,
                    &zero_di) != 0)
                        return -1;
                fi.parent_fcb = VFS_V1_INDEX(newdir);
                fcb[D6FS_V2_FCB_PARENT] = (kword_t)fi.parent_fcb << 18;
                if (d6fs_v2_reader_put_fcb(&d6fs_provider_v2_reader,
                    di.child_fcb, fcb) != 0) {
                        (void)d6fs_provider_v2_write_dirent(olddir, oldslot,
                            &old_di);
                        return -1;
                }
                if (d6fs_provider_v2_write_dirent(newdir, newslot, &di) != 0) {
                        fi.parent_fcb = old_parent;
                        fcb[D6FS_V2_FCB_PARENT] = (kword_t)old_parent << 18;
                        (void)d6fs_v2_reader_put_fcb(&d6fs_provider_v2_reader,
                            di.child_fcb, fcb);
                        (void)d6fs_provider_v2_write_dirent(olddir, oldslot,
                            &old_di);
                        return -1;
                }
        }
        d6fs_provider_v2_lcache_flush();
        return 0;
}

int
d6fs_provider_v2_truncate(vnode_v1_t node, unsigned int words,
    kword_t size_chars)
{
        kword_t fcb[D6FS_V2_FCB_WORDS];
        struct d6fs_v2_fcb_info fi;
        unsigned int tail;

        if (d6fs_provider_v2_fcb(node, fcb, &fi) != 0 ||
            (fi.type != D6FS_V2_TYPE_REG && fi.type != D6FS_V2_TYPE_SYMLINK) ||
            (fi.flags & (D6FS_V2_FLAG_APPEND | D6FS_V2_FLAG_IMMUTABLE)) != 0U)
                return -1;
        if (words == 0U)
                tail = 0U;
        else {
                kword_t chars_per_word = fi.type == D6FS_V2_TYPE_SYMLINK ?
                    6UL : 4UL;
                kword_t base = ((kword_t)words - 1UL) * chars_per_word;
                if (size_chars <= base || size_chars > base + chars_per_word)
                        tail = (unsigned int)chars_per_word;
                else
                        tail = (unsigned int)(size_chars - base);
        }
        return d6fs_provider_v2_resize_fcb(node, fcb, &fi,
            (kword_t)words, tail);
}

int
d6fs_provider_v2_chmod(vnode_v1_t node, unsigned int mode)
{
        kword_t fcb[D6FS_V2_FCB_WORDS];
        struct d6fs_v2_fcb_info fi;

        if (d6fs_provider_v2_fcb(node, fcb, &fi) != 0 ||
            fi.type == D6FS_V2_TYPE_FREE ||
            (fi.flags & D6FS_V2_FLAG_IMMUTABLE) != 0U)
                return -1;
        fi.mode = mode & 07777U;
        fcb[D6FS_V2_FCB_META] = ((kword_t)fi.type << 33) |
            ((kword_t)fi.flags << 24) | ((kword_t)fi.mode << 12) |
            ((kword_t)fi.tail << 8) | ((kword_t)fi.extent_count << 4);
        return d6fs_v2_reader_put_fcb(&d6fs_provider_v2_reader,
            VFS_V1_INDEX(node), fcb);
}

int
d6fs_provider_v2_write_words(vnode_v1_t node, unsigned int off,
    const kword_t *buf, unsigned int nwords, kword_t size_chars)
{
        kword_t fcb[D6FS_V2_FCB_WORDS];
        struct d6fs_v2_fcb_info fi;
        kword_t need;
        unsigned int tail;

        if (buf == 0 || d6fs_provider_v2_fcb(node, fcb, &fi) != 0 ||
            (fi.type != D6FS_V2_TYPE_REG && fi.type != D6FS_V2_TYPE_SYMLINK) ||
            (fi.flags & D6FS_V2_FLAG_IMMUTABLE) != 0U ||
            ((fi.flags & D6FS_V2_FLAG_APPEND) != 0U &&
            (kword_t)off != fi.size_words))
                return -1;
        need = (kword_t)off + (kword_t)nwords;
        if (need > fi.size_words) {
                if (need == 0UL)
                        tail = 0U;
                else {
                        kword_t chars_per_word =
                            fi.type == D6FS_V2_TYPE_SYMLINK ? 6UL : 4UL;
                        kword_t base = (need - 1UL) * chars_per_word;
                        if (size_chars <= base ||
                            size_chars > base + chars_per_word)
                                tail = (unsigned int)chars_per_word;
                        else
                                tail = (unsigned int)(size_chars - base);
                }
                if (d6fs_provider_v2_resize_fcb(node, fcb, &fi, need,
                    tail) != 0)
                        return -1;
                if (d6fs_provider_v2_fcb(node, fcb, &fi) != 0)
                        return -1;
        }
        return d6fs_v2_reader_write_words(&d6fs_provider_v2_reader, fcb,
            off, buf, nwords);
}
