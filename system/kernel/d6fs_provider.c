#include "d6fs_provider.h"

#define D6FS_PROVIDER_SUPER_DISABLED 2U

struct d6fs_provider_mount {
        kword_t alloc_cursor;
        kword_t super_block[2];
        unsigned int super_copy; /* 0/1 active copy, otherwise disabled */
};

static struct d6fs_provider_mount d6fs_provider_mount_state;
static struct d6fs_reader d6fs_provider_reader;
static unsigned int d6fs_provider_mount_id;

static int d6fs_provider_find_slot(vnode_t dir,
    const struct vfs_name *name, unsigned int *slotp,
    struct d6fs_dirent_info *dip);

static int
d6fs_provider_load(vnode_t node)
{
        unsigned int id;

        id = VFS_MOUNT_ID(node);
        if (id == 0U || id != d6fs_provider_mount_id ||
            d6fs_provider_reader.read_block == 0)
                return -1;
        return 0;
}

static int
d6fs_provider_fcb(vnode_t node, kword_t fcb[D6FS_FCB_WORDS],
    struct d6fs_fcb_info *info)
{
        if (d6fs_provider_load(node) != 0 ||
            d6fs_reader_fcb(&d6fs_provider_reader,
            VFS_INDEX(node), fcb, info) != 0)
                return -1;
        return 0;
}

static unsigned int
d6fs_provider_vtype(unsigned int type)
{
        if (type == D6FS_TYPE_DIR)
                return VFS_TYPE_DIR;
        if (type == D6FS_TYPE_SYMLINK)
                return VFS_TYPE_SYMLINK;
        return VFS_TYPE_REG;
}

static int
d6fs_provider_dirent(vnode_t dir, unsigned int slot,
    struct d6fs_dirent_info *di)
{
        kword_t fcb[D6FS_FCB_WORDS];
        struct d6fs_fcb_info fi;
        kword_t raw[D6FS_DIRENT_WORDS];
        kword_t off;
        int rc;

        if (di == 0 || d6fs_provider_fcb(dir, fcb, &fi) != 0 ||
            fi.type != D6FS_TYPE_DIR)
                return -1;
        off = (kword_t)slot * D6FS_DIRENT_WORDS;
        if (off >= fi.size_words)
                return 0;
        if (fi.size_words - off < D6FS_DIRENT_WORDS)
                return -1;
        rc = d6fs_reader_read_words(&d6fs_provider_reader, fcb, off,
            raw, D6FS_DIRENT_WORDS);
        if (rc != (int)D6FS_DIRENT_WORDS ||
            !d6fs_dirent_decode_valid(raw,
            d6fs_provider_reader.super.fcb_count, di))
                return -1;
        return 1;
}

int
d6fs_provider_mount_rw(vnode_t target,
    d6fs_read_block_fn read_block, d6fs_write_block_fn write_block,
    void *opaque, const struct d6fs_super_info *super, unsigned int flags,
    vnode_t *rootp)
{
        vnode_t root;
        unsigned int id;
        struct d6fs_provider_mount *mp;

        if (read_block == 0 || super == 0 || rootp == 0 ||
            super->root_fcb >= super->fcb_count ||
            ((flags & VFS_MOUNT_RDONLY) == 0U && write_block == 0) ||
            vfs_mount(target, D6FS_PROVIDER, D6FS_KIND_NODE,
            super->root_fcb, flags, &root) != 0)
                return -1;
        id = VFS_MOUNT_ID(root);
        if (d6fs_provider_reader.read_block != 0) {
                (void)vfs_unmount(root);
                return -1;
        }
        mp = &d6fs_provider_mount_state;
        mp->alloc_cursor = super->summary_start + super->summary_blocks;
        mp->super_block[0] = 0UL;
        mp->super_block[1] = 0UL;
        mp->super_copy = D6FS_PROVIDER_SUPER_DISABLED;
        if (mp->alloc_cursor >= super->total_blocks)
                mp->alloc_cursor = 0UL;
        d6fs_provider_mount_id = id;
        if (d6fs_reader_init(&d6fs_provider_reader,
            read_block, opaque, super) != 0) {
                (void)vfs_unmount(root);
                d6fs_provider_mount_id = 0U;
                return -1;
        }
        d6fs_provider_reader.write_block = write_block;
        *rootp = root;
        return 0;
}

static int
d6fs_provider_name_equal(const struct d6fs_dirent_info *di,
    const struct vfs_name *name)
{
        unsigned int i;

        for (i = 0U; i < VFS_NAME_WORDS; ++i)
                if (di->name[i] != name->words[i])
                        return 0;
        return 1;
}

int
d6fs_provider_lookup(vnode_t dir, const struct vfs_name *name,
    vnode_t *nodep)
{
        struct d6fs_dirent_info di;

        if (nodep == 0 || d6fs_provider_find_slot(dir, name, 0, &di) != 0)
                return -1;
        *nodep = VFS_NODE(D6FS_PROVIDER,
            VFS_MOUNT_KIND(VFS_MOUNT_ID(dir), D6FS_KIND_NODE), di.child_fcb);
        return 0;
}

int
d6fs_provider_readdir(vnode_t dir, unsigned int off,
    struct vfs_dirent *ent)
{
        struct d6fs_dirent_info di;
        unsigned int slot;
        unsigned int seen;
        unsigned int i;
        int rc;

        if (ent == 0)
                return -1;
        seen = 0U;
        for (slot = 0U;; ++slot) {
                rc = d6fs_provider_dirent(dir, slot, &di);
                if (rc == 0)
                        return 0;
                if (rc < 0)
                        return -1;
                if (di.child_fcb == 0U)
                        continue;
                if (seen++ != off)
                        continue;
                for (i = 0U; i < VFS_NAME_WORDS; ++i)
                        ent->name.words[i] = di.name[i];
                ent->name.chars = vfs_sixbit_name_chars(di.name, VFS_NAME_MAX_CHARS);
                ent->type = d6fs_provider_vtype(di.type);
                return 1;
        }
}

int
d6fs_provider_stat(vnode_t node, struct vfs_stat *st)
{
        struct d6fs_fcb_info fi;

        if (st == 0 || d6fs_provider_fcb(node, 0, &fi) != 0 ||
            fi.type == D6FS_TYPE_FREE)
                return -1;
        st->type = d6fs_provider_vtype(fi.type);
        st->mode = fi.mode;
        st->size_words = fi.size_words;
        if (fi.size_words == 0UL)
                st->size_chars = 0UL;
        else if (fi.type == D6FS_TYPE_SYMLINK)
                st->size_chars = (fi.size_words - 1UL) * 6UL +
                    (fi.tail != 0U ? fi.tail : 6U);
        else if (fi.tail != 0U)
                st->size_chars = (fi.size_words - 1UL) * 4UL + fi.tail;
        else
                st->size_chars = fi.size_words * 4UL;
        return 0;
}

int
d6fs_provider_parent(vnode_t node, vnode_t *parentp)
{
        struct d6fs_fcb_info fi;

        if (parentp == 0 || d6fs_provider_fcb(node, 0, &fi) != 0)
                return -1;
        if (VFS_INDEX(node) == d6fs_provider_reader.super.root_fcb) {
                *parentp = node;
                return 0;
        }
        if (fi.parent_fcb >= d6fs_provider_reader.super.fcb_count)
                return -1;
        *parentp = VFS_NODE(D6FS_PROVIDER,
            VFS_MOUNT_KIND(VFS_MOUNT_ID(node), D6FS_KIND_NODE),
            fi.parent_fcb);
        return 0;
}

int
d6fs_provider_parent_name(vnode_t node, vnode_t *parentp,
    struct vfs_name *namep)
{
        vnode_t parent;
        struct d6fs_dirent_info di;
        unsigned int slot;
        unsigned int i;
        int rc;

        if (parentp == 0 || namep == 0 ||
            d6fs_provider_parent(node, &parent) != 0 || parent == node)
                return -1;
        for (slot = 0U;; ++slot) {
                rc = d6fs_provider_dirent(parent, slot, &di);
                if (rc <= 0)
                        return -1;
                if (di.child_fcb != VFS_INDEX(node))
                        continue;
                namep->chars = vfs_sixbit_name_chars(di.name, VFS_NAME_MAX_CHARS);
                for (i = 0U; i < VFS_NAME_WORDS; ++i)
                        namep->words[i] = di.name[i];
                *parentp = parent;
                return 0;
        }
}

int
d6fs_provider_read_words(vnode_t node, unsigned int off,
    kword_t *buf, unsigned int nwords)
{
        kword_t fcb[D6FS_FCB_WORDS];
        struct d6fs_fcb_info fi;

        if (buf == 0 || d6fs_provider_fcb(node, fcb, &fi) != 0 ||
            (fi.type != D6FS_TYPE_REG && fi.type != D6FS_TYPE_SYMLINK))
                return -1;
        return d6fs_reader_read_words(&d6fs_provider_reader, fcb,
            off, buf, nwords);
}

int
d6fs_provider_sync(vnode_t node)
{
        return d6fs_provider_load(node);
}

static int
d6fs_provider_write_super(struct d6fs_provider_mount *mp,
    unsigned int copy, unsigned int state)
{
        if (mp == 0 || copy > 1U || d6fs_provider_reader.write_block == 0 ||
            d6fs_provider_mount_id == 0U)
                return -1;
        if (d6fs_reader_get_block(&d6fs_provider_reader,
            mp->super_block[copy]) == 0)
                return -1;
        fs_block_workspace[D6FS_SB_SEQUENCE] =
            d6fs_provider_reader.super.sequence + 1UL;
        fs_block_workspace[D6FS_SB_STATE] = (kword_t)state;
        if (d6fs_reader_write_block(&d6fs_provider_reader,
            mp->super_block[copy], fs_block_workspace) != 0)
                return -1;
        ++d6fs_provider_reader.super.sequence;
        d6fs_provider_reader.super.state = state;
        mp->super_copy = copy;
        return 0;
}

int
d6fs_provider_enable_state(vnode_t root, kword_t super_a,
    kword_t super_b, unsigned int selected_copy)
{
        struct d6fs_provider_mount *mp;

        if (selected_copy > 1U || super_a == super_b ||
            d6fs_provider_load(root) != 0)
                return -1;
        mp = &d6fs_provider_mount_state;
        if (d6fs_provider_reader.write_block == 0 ||
            d6fs_provider_reader.super.state != D6FS_STATE_CLEAN)
                return -1;
        mp->super_block[0] = super_a;
        mp->super_block[1] = super_b;
        mp->super_copy = selected_copy;
        if (d6fs_provider_write_super(mp, selected_copy ^ 1U,
            D6FS_STATE_DIRTY) != 0) {
                mp->super_copy = D6FS_PROVIDER_SUPER_DISABLED;
                return -1;
        }
        return 0;
}

int
d6fs_provider_prepare_unmount(vnode_t root)
{
        struct d6fs_provider_mount *mp;
        if (d6fs_provider_load(root) != 0)
                return -1;
        mp = &d6fs_provider_mount_state;
        if (mp->super_copy != D6FS_PROVIDER_SUPER_DISABLED &&
            d6fs_provider_reader.write_block != 0 &&
            d6fs_provider_write_super(mp, mp->super_copy ^ 1U,
            D6FS_STATE_CLEAN) != 0)
                return -1;
        mp->super_copy = D6FS_PROVIDER_SUPER_DISABLED;
        d6fs_provider_mount_id = 0U;
        d6fs_provider_reader.read_block = 0;
        d6fs_provider_reader.write_block = 0;
        d6fs_provider_reader.cache_block = D6FS_CACHE_INVALID;
        return 0;
}

static int
d6fs_provider_writable(vnode_t node)
{
        return d6fs_provider_load(node) == 0 &&
            d6fs_provider_reader.write_block != 0;
}

static void
d6fs_provider_init_fcb(kword_t fcb[D6FS_FCB_WORDS],
    struct d6fs_fcb_info *fi, unsigned int type, unsigned int mode,
    unsigned int parent)
{
        unsigned int i;

        for (i = 0U; i < D6FS_FCB_WORDS; ++i)
                fcb[i] = 0UL;
        fcb[D6FS_FCB_META] = ((kword_t)type << 33) |
            ((kword_t)(mode & 07777U) << 12);
        fcb[D6FS_FCB_PARENT] = (kword_t)parent << 18;
        fi->type = type;
        fi->flags = 0U;
        fi->mode = mode & 07777U;
        fi->tail = 0U;
        fi->extent_count = 0U;
        fi->uid = 0U;
        fi->gid = 0U;
        fi->size_words = 0UL;
        fi->mtime = 0UL;
        fi->parent_fcb = parent;
}

static int
d6fs_provider_set_extent(kword_t fcb[D6FS_FCB_WORDS],
    unsigned int index, kword_t start, kword_t blocks)
{
        kword_t length;
        kword_t mask;
        unsigned int shift;

        if (index >= D6FS_EXTENTS || start > D6FS_LOGICAL_BLOCK_MASK ||
            blocks == 0UL || blocks > D6FS_EXTENT_MAX_BLOCKS)
                return -1;
        length = blocks - 1UL;
        fcb[D6FS_FCB_EXTENT0 + index] = (start << D6FS_EXTENT_LOW_BITS) |
            (length & D6FS_EXTENT_LOW_MASK);
        shift = index * D6FS_EXTENT_HIGH_BITS;
        mask = (kword_t)D6FS_EXTENT_HIGH_MASK << shift;
        fcb[D6FS_FCB_LENHIGH] = (fcb[D6FS_FCB_LENHIGH] & ~mask) |
            (((length >> D6FS_EXTENT_LOW_BITS) & D6FS_EXTENT_HIGH_MASK) <<
            shift);
        return 0;
}

static int
d6fs_provider_clear_extent(kword_t fcb[D6FS_FCB_WORDS],
    unsigned int index)
{
        unsigned int shift;

        if (index >= D6FS_EXTENTS)
                return -1;
        fcb[D6FS_FCB_EXTENT0 + index] = 0UL;
        shift = index * D6FS_EXTENT_HIGH_BITS;
        fcb[D6FS_FCB_LENHIGH] &=
            ~((kword_t)D6FS_EXTENT_HIGH_MASK << shift);
        return 0;
}

static kword_t
d6fs_provider_blocks_for_words(kword_t words)
{
        if (words == 0UL)
                return 0UL;
        return (words + (kword_t)D6FS_BLOCK_WORDS - 1UL) /
            (kword_t)D6FS_BLOCK_WORDS;
}

static int
d6fs_provider_free_file_tail(const kword_t fcb[D6FS_FCB_WORDS],
    kword_t first_file_block)
{
        kword_t logical;
        kword_t file_block;

        file_block = first_file_block;
        while ((logical = d6fs_file_block(fcb, file_block)) !=
            D6FS_CACHE_INVALID) {
                if (d6fs_freemap_set(&d6fs_provider_reader,
                    logical, 0U) != 0)
                        return -1;
                ++file_block;
        }
        return 0;
}

static int
d6fs_provider_rollback_growth(kword_t fcb[D6FS_FCB_WORDS],
    const kword_t old_fcb[D6FS_FCB_WORDS], kword_t old_blocks)
{
        unsigned int i;

        (void)d6fs_provider_free_file_tail(fcb, old_blocks);
        for (i = 0U; i < D6FS_FCB_WORDS; ++i)
                fcb[i] = old_fcb[i];
        return -1;
}

static int
d6fs_provider_resize_fcb(vnode_t node,
    kword_t fcb[D6FS_FCB_WORDS], struct d6fs_fcb_info *fi,
    kword_t new_words, unsigned int new_tail)
{
        struct d6fs_provider_mount *mp;
        kword_t old_fcb[D6FS_FCB_WORDS];
        struct d6fs_fcb_info work;
        kword_t old_blocks;
        kword_t new_blocks;
        kword_t need;
        kword_t start;
        kword_t blocks;
        kword_t last_start;
        kword_t last_blocks;
        kword_t candidate;
        int allocated;
        unsigned int high;
        unsigned int i;

        if (fcb == 0 || fi == 0 || !d6fs_provider_writable(node) ||
            new_tail > (fi->type == D6FS_TYPE_SYMLINK ? 6U : 4U) ||
            (new_words == 0UL && new_tail != 0U))
                return -1;
        mp = &d6fs_provider_mount_state;
        for (i = 0U; i < D6FS_FCB_WORDS; ++i)
                old_fcb[i] = fcb[i];
        work = *fi;
        old_blocks = d6fs_provider_blocks_for_words(fi->size_words);
        new_blocks = d6fs_provider_blocks_for_words(new_words);

        if (new_blocks > old_blocks) {
                need = new_blocks - old_blocks;
                if (work.extent_count != 0U) {
                        i = work.extent_count - 1U;
                        high = d6fs_extent_high_get(
                            fcb[D6FS_FCB_LENHIGH], i);
                        if (d6fs_extent_decode(
                            fcb[D6FS_FCB_EXTENT0 + i], high,
                            &last_start, &last_blocks) != 0)
                                return -1;
                        while (need != 0UL &&
                            last_blocks < D6FS_EXTENT_MAX_BLOCKS) {
                                candidate = last_start + last_blocks;
                                if (candidate >= d6fs_provider_reader.super.total_blocks)
                                        break;
                                allocated = d6fs_freemap_state(
                                    &d6fs_provider_reader, candidate);
                                if (allocated != 0)
                                        break;
                                if (d6fs_reader_zero_block(
                                    &d6fs_provider_reader, candidate) != 0)
                                        return d6fs_provider_rollback_growth(
                                            fcb, old_fcb, old_blocks);
                                if (d6fs_freemap_set(&d6fs_provider_reader,
                                    candidate, 1U) != 0)
                                        return d6fs_provider_rollback_growth(
                                            fcb, old_fcb, old_blocks);
                                ++last_blocks;
                                --need;
                                if (d6fs_provider_set_extent(fcb, i,
                                    last_start, last_blocks) != 0) {
                                        (void)d6fs_freemap_set(
                                            &d6fs_provider_reader,
                                            candidate, 0U);
                                        return d6fs_provider_rollback_growth(
                                            fcb, old_fcb, old_blocks);
                                }
                        }
                }
                while (need != 0UL) {
                        if (work.extent_count >= D6FS_EXTENTS) {
                                return d6fs_provider_rollback_growth(
                                    fcb, old_fcb, old_blocks);
                        }
                        blocks = need;
                        if (blocks > D6FS_EXTENT_MAX_BLOCKS)
                                blocks = D6FS_EXTENT_MAX_BLOCKS;
                        if (d6fs_alloc_run(&d6fs_provider_reader,
                            mp->alloc_cursor, blocks, &start, &blocks) != 0) {
                                return d6fs_provider_rollback_growth(
                                    fcb, old_fcb, old_blocks);
                        }
                        for (candidate = 0UL; candidate < blocks; ++candidate)
                                if (d6fs_reader_zero_block(
                                    &d6fs_provider_reader,
                                    start + candidate) != 0)
                                        return d6fs_provider_rollback_growth(
                                            fcb, old_fcb, old_blocks);
                        for (candidate = 0UL; candidate < blocks; ++candidate)
                                if (d6fs_freemap_set(&d6fs_provider_reader,
                                    start + candidate, 1U) != 0) {
                                        if (candidate != 0UL)
                                                (void)d6fs_free_run(
                                                    &d6fs_provider_reader,
                                                    start, candidate);
                                        return d6fs_provider_rollback_growth(
                                            fcb, old_fcb, old_blocks);
                                }
                        i = work.extent_count;
                        if (d6fs_provider_set_extent(fcb, i, start,
                            blocks) != 0) {
                                (void)d6fs_free_run(&d6fs_provider_reader,
                                    start, blocks);
                                return d6fs_provider_rollback_growth(
                                    fcb, old_fcb, old_blocks);
                        }
                        ++work.extent_count;
                        need -= blocks;
                        mp->alloc_cursor = start + blocks;
                        if (mp->alloc_cursor >=
                            d6fs_provider_reader.super.total_blocks)
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
                        high = d6fs_extent_high_get(
                            old_fcb[D6FS_FCB_LENHIGH], i);
                        if (d6fs_extent_decode(
                            old_fcb[D6FS_FCB_EXTENT0 + i], high,
                            &extent_start, &extent_blocks) != 0)
                                return -1;
                        if (consumed >= keep) {
                                (void)d6fs_provider_clear_extent(fcb, i);
                                continue;
                        }
                        if (extent_blocks > keep - consumed)
                                extent_blocks = keep - consumed;
                        if (extent_blocks != 0UL) {
                                if (d6fs_provider_set_extent(fcb, i,
                                    extent_start, extent_blocks) != 0)
                                        return -1;
                                ++new_count;
                        } else {
                                (void)d6fs_provider_clear_extent(fcb, i);
                        }
                        consumed += extent_blocks;
                }
                for (i = new_count; i < D6FS_EXTENTS; ++i)
                        (void)d6fs_provider_clear_extent(fcb, i);
                work.extent_count = new_count;
        }

        work.size_words = new_words;
        work.tail = new_words == 0UL ? 0U : new_tail;
        /* Encode metadata without destroying the already-built extent words. */
        fcb[D6FS_FCB_META] = ((kword_t)work.type << 33) |
            ((kword_t)work.flags << 24) | ((kword_t)work.mode << 12) |
            ((kword_t)work.tail << 8) | ((kword_t)work.extent_count << 4);
        fcb[D6FS_FCB_OWNER] = ((kword_t)work.uid << 18) |
            (kword_t)work.gid;
        fcb[D6FS_FCB_SIZE] = work.size_words;
        fcb[D6FS_FCB_MTIME] = work.mtime;
        fcb[D6FS_FCB_PARENT] = (kword_t)work.parent_fcb << 18;
        if (d6fs_reader_put_fcb(&d6fs_provider_reader,
            VFS_INDEX(node), fcb) != 0) {
                if (new_blocks > old_blocks)
                        return d6fs_provider_rollback_growth(
                            fcb, old_fcb, old_blocks);
                return -1;
        }
        if (new_blocks < old_blocks &&
            d6fs_provider_free_file_tail(old_fcb, new_blocks) != 0)
                return -1;
        *fi = work;
        return 0;
}

static int
d6fs_provider_find_slot(vnode_t dir, const struct vfs_name *name,
    unsigned int *slotp, struct d6fs_dirent_info *dip)
{
        struct d6fs_dirent_info di;
        kword_t hash;
        unsigned int slot;
        int rc;

        if (!vfs_name_valid(name))
                return -1;
        hash = d6fs_name_hash24(name->words, name->chars);
        for (slot = 0U;; ++slot) {
                rc = d6fs_provider_dirent(dir, slot, &di);
                if (rc == 0)
                        return 1;
                if (rc < 0)
                        return -1;
                if (di.child_fcb == 0U || di.hash != hash)
                        continue;
                if (!d6fs_provider_name_equal(&di, name))
                        continue;
                if (slotp != 0)
                        *slotp = slot;
                if (dip != 0)
                        *dip = di;
                return 0;
        }
}

static int
d6fs_provider_empty_slot(vnode_t dir, unsigned int *slotp)
{
        struct d6fs_dirent_info di;
        unsigned int slot;
        int rc;

        if (slotp == 0)
                return -1;
        for (slot = 0U;; ++slot) {
                rc = d6fs_provider_dirent(dir, slot, &di);
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
d6fs_provider_free_fcb(unsigned int *indexp)
{
        kword_t fcb[D6FS_FCB_WORDS];
        struct d6fs_fcb_info fi;
        unsigned int i;

        if (indexp == 0)
                return -1;
        for (i = 1U; i < d6fs_provider_reader.super.fcb_count; ++i) {
                if (d6fs_reader_fcb(&d6fs_provider_reader, i, fcb, &fi) != 0)
                        continue;
                if (fi.type == D6FS_TYPE_FREE) {
                        *indexp = i;
                        return 0;
                }
        }
        return -1;
}

static int
d6fs_provider_write_dirent(vnode_t dir, unsigned int slot,
    const struct d6fs_dirent_info *di)
{
        kword_t fcb[D6FS_FCB_WORDS];
        struct d6fs_fcb_info fi;
        kword_t raw[D6FS_DIRENT_WORDS];
        kword_t need;
        unsigned int i;
        unsigned int tail;

        if (d6fs_provider_fcb(dir, fcb, &fi) != 0 ||
            fi.type != D6FS_TYPE_DIR ||
            (fi.flags & D6FS_FLAG_IMMUTABLE) != 0U)
                return -1;
        if (di != 0) {
                for (i = 0U; i < 4U; ++i)
                        raw[i] = di->name[i];
                raw[4] = (di->hash << D6FS_DIRENT_HASH_SHIFT) |
                    ((kword_t)di->type << D6FS_DIRENT_TYPE_SHIFT) |
                    (kword_t)di->flags;
                raw[5] = (kword_t)di->child_fcb << 18;
        } else {
                for (i = 0U; i < D6FS_DIRENT_WORDS; ++i)
                        raw[i] = 0UL;
        }
        need = ((kword_t)slot + 1UL) * D6FS_DIRENT_WORDS;
        if (need > fi.size_words) {
                tail = 4U;
                if (d6fs_provider_resize_fcb(dir, fcb, &fi, need, tail) != 0)
                        return -1;
        }
        return d6fs_reader_write_words(&d6fs_provider_reader, fcb,
            (kword_t)slot * D6FS_DIRENT_WORDS, raw,
            D6FS_DIRENT_WORDS) == (int)D6FS_DIRENT_WORDS ? 0 : -1;
}

int
d6fs_provider_create_object(vnode_t dir, const struct vfs_name *name,
    const kword_t *payload, unsigned int value, unsigned int type,
    vnode_t *nodep)
{
        kword_t fcb[D6FS_FCB_WORDS];
        struct d6fs_fcb_info fi;
        struct d6fs_dirent_info di;
        vnode_t node;
        unsigned int index;
        unsigned int slot;
        unsigned int mode;
        unsigned int words;
        unsigned int tail;
        unsigned int i;

        if (nodep == 0 || !d6fs_provider_writable(dir) ||
            (type != D6FS_TYPE_REG && type != D6FS_TYPE_DIR &&
            type != D6FS_TYPE_SYMLINK) ||
            (type == D6FS_TYPE_SYMLINK && (payload == 0 || value == 0U)) ||
            d6fs_provider_find_slot(dir, name, 0, 0) == 0 ||
            d6fs_provider_free_fcb(&index) != 0)
                return -1;
        if (d6fs_provider_empty_slot(dir, &slot) < 0)
                return -1;

        mode = type == D6FS_TYPE_SYMLINK ? 0777U : value;
        d6fs_provider_init_fcb(fcb, &fi, type, mode, VFS_INDEX(dir));
        if (d6fs_reader_put_fcb(&d6fs_provider_reader, index, fcb) != 0)
                return -1;
        node = VFS_NODE(D6FS_PROVIDER,
            VFS_MOUNT_KIND(VFS_MOUNT_ID(dir), D6FS_KIND_NODE), index);

        if (type == D6FS_TYPE_SYMLINK) {
                words = (value + 5U) / 6U;
                tail = value - (words - 1U) * 6U;
                if (d6fs_provider_resize_fcb(node, fcb, &fi, words, tail) != 0 ||
                    d6fs_provider_fcb(node, fcb, &fi) != 0 ||
                    d6fs_reader_write_words(&d6fs_provider_reader, fcb, 0UL,
                    payload, words) != (int)words)
                        goto fail;
        }

        for (i = 0U; i < 4U; ++i)
                di.name[i] = name->words[i];
        di.hash = d6fs_name_hash24(name->words, name->chars);
        di.type = type;
        di.flags = 0U;
        di.child_fcb = index;
        if (d6fs_provider_write_dirent(dir, slot, &di) != 0)
                goto fail;
        *nodep = node;
        return 0;

fail:
        if (type == D6FS_TYPE_SYMLINK &&
            d6fs_provider_fcb(node, fcb, &fi) == 0)
                (void)d6fs_provider_resize_fcb(node, fcb, &fi, 0UL, 0U);
        for (i = 0U; i < D6FS_FCB_WORDS; ++i)
                fcb[i] = 0UL;
        (void)d6fs_reader_put_fcb(&d6fs_provider_reader, index, fcb);
        return -1;
}

int
d6fs_provider_unlink(vnode_t dir, const struct vfs_name *name)
{
        struct d6fs_dirent_info di;
        kword_t fcb[D6FS_FCB_WORDS];
        kword_t old_fcb[D6FS_FCB_WORDS];
        struct d6fs_fcb_info fi;
        unsigned int slot;
        unsigned int i;

        if (!d6fs_provider_writable(dir) ||
            d6fs_provider_find_slot(dir, name, &slot, &di) != 0 ||
            d6fs_reader_fcb(&d6fs_provider_reader, di.child_fcb,
            fcb, &fi) != 0 ||
            (fi.flags & (D6FS_FLAG_NOUNLINK | D6FS_FLAG_IMMUTABLE)) != 0U)
                return -1;
        for (i = 0U; i < D6FS_FCB_WORDS; ++i)
                old_fcb[i] = fcb[i];
        if (fi.type == D6FS_TYPE_DIR && fi.size_words != 0UL) {
                struct d6fs_dirent_info child;
                unsigned int s;
                int rc;

                for (s = 0U;; ++s) {
                        rc = d6fs_provider_dirent(VFS_NODE(
                            D6FS_PROVIDER,
                            VFS_MOUNT_KIND(VFS_MOUNT_ID(dir),
                            D6FS_KIND_NODE), di.child_fcb), s, &child);
                        if (rc == 0)
                                break;
                        if (rc < 0 || child.child_fcb != 0U)
                                return -1;
                }
        }
        if (d6fs_provider_write_dirent(dir, slot, 0) != 0)
                return -1;
        for (i = 0U; i < D6FS_FCB_WORDS; ++i)
                fcb[i] = 0UL;
        if (d6fs_reader_put_fcb(&d6fs_provider_reader,
            di.child_fcb, fcb) != 0)
                return -1;
        if (d6fs_provider_free_file_tail(old_fcb, 0UL) != 0) {
                /* Any leaked blocks are fsck-recoverable. */
        }
        return 0;
}

int
d6fs_provider_rename(vnode_t olddir,
    const struct vfs_name *oldname, vnode_t newdir,
    const struct vfs_name *newname)
{
        struct d6fs_dirent_info di;
        struct d6fs_dirent_info old_di;
        kword_t fcb[D6FS_FCB_WORDS];
        struct d6fs_fcb_info fi;
        unsigned int old_parent;
        unsigned int oldslot;
        unsigned int newslot;
        unsigned int i;
        int empty_rc;

        if (!d6fs_provider_writable(olddir) ||
            VFS_MOUNT_ID(olddir) != VFS_MOUNT_ID(newdir) ||
            d6fs_provider_find_slot(olddir, oldname, &oldslot, &di) != 0 ||
            d6fs_provider_find_slot(newdir, newname, 0, 0) == 0 ||
            d6fs_reader_fcb(&d6fs_provider_reader, di.child_fcb,
            fcb, &fi) != 0 ||
            (fi.flags & (D6FS_FLAG_NOUNLINK | D6FS_FLAG_IMMUTABLE)) != 0U)
                return -1;
        old_di = di;
        old_parent = fi.parent_fcb;
        if (olddir == newdir) {
                newslot = oldslot;
        } else {
                empty_rc = d6fs_provider_empty_slot(newdir, &newslot);
                if (empty_rc < 0)
                        return -1;
        }
        for (i = 0U; i < 4U; ++i)
                di.name[i] = newname->words[i];
        di.hash = d6fs_name_hash24(newname->words, newname->chars);
        if (olddir == newdir) {
                if (d6fs_provider_write_dirent(newdir, newslot, &di) != 0)
                        return -1;
        } else {
                /*
                 * Remove the old namespace reference first.  A crash from
                 * this point until publication in the new directory can
                 * leave an orphaned FCB, which fsck can repair, but never a
                 * transient second hard-link-like reference.
                 */
                if (d6fs_provider_write_dirent(olddir, oldslot, 0) != 0)
                        return -1;
                fi.parent_fcb = VFS_INDEX(newdir);
                fcb[D6FS_FCB_PARENT] = (kword_t)fi.parent_fcb << 18;
                if (d6fs_reader_put_fcb(&d6fs_provider_reader,
                    di.child_fcb, fcb) != 0) {
                        (void)d6fs_provider_write_dirent(olddir, oldslot,
                            &old_di);
                        return -1;
                }
                if (d6fs_provider_write_dirent(newdir, newslot, &di) != 0) {
                        fi.parent_fcb = old_parent;
                        fcb[D6FS_FCB_PARENT] = (kword_t)old_parent << 18;
                        (void)d6fs_reader_put_fcb(&d6fs_provider_reader,
                            di.child_fcb, fcb);
                        (void)d6fs_provider_write_dirent(olddir, oldslot,
                            &old_di);
                        return -1;
                }
        }
        return 0;
}

static unsigned int
d6fs_provider_tail(unsigned int type, kword_t words, kword_t size_chars)
{
        kword_t chars_per_word;
        kword_t base;

        if (words == 0UL)
                return 0U;
        chars_per_word = type == D6FS_TYPE_SYMLINK ? 6UL : 4UL;
        base = (words - 1UL) * chars_per_word;
        if (size_chars <= base || size_chars > base + chars_per_word)
                return (unsigned int)chars_per_word;
        return (unsigned int)(size_chars - base);
}

int
d6fs_provider_truncate(vnode_t node, unsigned int words,
    kword_t size_chars)
{
        kword_t fcb[D6FS_FCB_WORDS];
        struct d6fs_fcb_info fi;
        unsigned int tail;

        if (d6fs_provider_fcb(node, fcb, &fi) != 0 ||
            (fi.type != D6FS_TYPE_REG && fi.type != D6FS_TYPE_SYMLINK) ||
            (fi.flags & (D6FS_FLAG_APPEND | D6FS_FLAG_IMMUTABLE)) != 0U)
                return -1;
        tail = d6fs_provider_tail(fi.type, (kword_t)words, size_chars);
        return d6fs_provider_resize_fcb(node, fcb, &fi,
            (kword_t)words, tail);
}

int
d6fs_provider_chmod(vnode_t node, unsigned int mode)
{
        kword_t fcb[D6FS_FCB_WORDS];
        struct d6fs_fcb_info fi;

        if (d6fs_provider_fcb(node, fcb, &fi) != 0 ||
            fi.type == D6FS_TYPE_FREE ||
            (fi.flags & D6FS_FLAG_IMMUTABLE) != 0U)
                return -1;
        fi.mode = mode & 07777U;
        fcb[D6FS_FCB_META] = ((kword_t)fi.type << 33) |
            ((kword_t)fi.flags << 24) | ((kword_t)fi.mode << 12) |
            ((kword_t)fi.tail << 8) | ((kword_t)fi.extent_count << 4);
        return d6fs_reader_put_fcb(&d6fs_provider_reader,
            VFS_INDEX(node), fcb);
}

int
d6fs_provider_write_words(vnode_t node, unsigned int off,
    const kword_t *buf, unsigned int nwords, kword_t size_chars)
{
        kword_t fcb[D6FS_FCB_WORDS];
        struct d6fs_fcb_info fi;
        kword_t need;
        unsigned int tail;

        if (buf == 0 || d6fs_provider_fcb(node, fcb, &fi) != 0 ||
            (fi.type != D6FS_TYPE_REG && fi.type != D6FS_TYPE_SYMLINK) ||
            (fi.flags & D6FS_FLAG_IMMUTABLE) != 0U ||
            ((fi.flags & D6FS_FLAG_APPEND) != 0U &&
            (kword_t)off != fi.size_words))
                return -1;
        need = (kword_t)off + (kword_t)nwords;
        if (need > fi.size_words) {
                tail = d6fs_provider_tail(fi.type, need, size_chars);
                if (d6fs_provider_resize_fcb(node, fcb, &fi, need,
                    tail) != 0)
                        return -1;
                if (d6fs_provider_fcb(node, fcb, &fi) != 0)
                        return -1;
        }
        return d6fs_reader_write_words(&d6fs_provider_reader, fcb,
            off, buf, nwords);
}
