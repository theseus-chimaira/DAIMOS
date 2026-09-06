#include "d6fs_provider.h"
#include "fs_mres.h"

struct d6fs_reader d6fs_provider_reader;

void d6fs_provider_set_extent(kword_t fcb[D6FS_FCB_WORDS],
    unsigned int index, kword_t start, kword_t blocks);
void d6fs_provider_clear_extent(kword_t fcb[D6FS_FCB_WORDS],
    unsigned int index);
kword_t d6fs_provider_blocks_for_words(kword_t words);
int d6fs_provider_free_file_tail(const kword_t fcb[D6FS_FCB_WORDS],
    kword_t first_file_block);
unsigned int d6fs_provider_tail(unsigned int type, kword_t words,
    kword_t size_chars);

static int d6fs_provider_scan_slot(vnode_t dir,
    const struct vfs_name *name, unsigned int *slotp,
    struct d6fs_dirent_info *dip);

static int
d6fs_provider_load(vnode_t node)
{
        unsigned int id;

        id = VFS_MOUNT_ID(node);
        if (id == 0U || id !=
            ((unsigned int)(unsigned long)d6fs_provider_reader.opaque &
            D6FS_PROVIDER_MOUNT_ID_MASK))
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

static int
d6fs_provider_name_equal(const struct d6fs_dirent_info *di,
    const struct vfs_name *name)
{
        return fs_words_equal(di->name, name->words, VFS_NAME_WORDS);
}

int
d6fs_provider_lookup(vnode_t dir, const struct vfs_name *name,
    vnode_t *nodep)
{
        struct d6fs_dirent_info di;

        if (nodep == 0 || d6fs_provider_scan_slot(dir, name, 0, &di) != 0)
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
                fs_copy_words(di.name, ent->name.words, VFS_NAME_WORDS);
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
                fs_copy_words(di.name, namep->words, VFS_NAME_WORDS);
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

int
d6fs_provider_prepare_unmount(vnode_t root)
{
        unsigned int copy;

        if (d6fs_provider_load(root) != 0)
                return -1;
        if ((((unsigned int)(unsigned long)d6fs_provider_reader.opaque &
            D6FS_PROVIDER_MOUNT_WRITABLE) != 0U)) {
                copy = (((unsigned int)(unsigned long)
                    d6fs_provider_reader.opaque &
                    D6FS_PROVIDER_MOUNT_COPY) != 0U) ? 0U : 1U;
                if (d6fs_reader_get_block(&d6fs_provider_reader,
                    D6FS_RUNTIME_SUPER_BLOCK(&d6fs_provider_reader, copy)) == 0)
                        return -1;
                fs_block_workspace[D6FS_SB_SEQUENCE] =
                    d6fs_provider_reader.super.sequence + 1UL;
                fs_block_workspace[D6FS_SB_STATE] = D6FS_STATE_CLEAN;
                if (d6fs_reader_write_block(&d6fs_provider_reader,
                    D6FS_RUNTIME_SUPER_BLOCK(&d6fs_provider_reader, copy),
                    fs_block_workspace) != 0)
                        return -1;
                ++d6fs_provider_reader.super.sequence;
        }
        d6fs_provider_reader.opaque = 0;
        D6FS_READER_CACHE_BLOCK(&d6fs_provider_reader) = D6FS_CACHE_INVALID;
        return 0;
}

static int
d6fs_provider_resize_fcb(vnode_t node,
    kword_t fcb[D6FS_FCB_WORDS], struct d6fs_fcb_info *fi,
    kword_t new_words, unsigned int new_tail)
{
        kword_t old_fcb[D6FS_FCB_WORDS];
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
        unsigned int extent_count;

        if (((unsigned int)(unsigned long)d6fs_provider_reader.opaque &
            D6FS_PROVIDER_MOUNT_WRITABLE) == 0U)
                return -1;
        fs_copy_words(fcb, old_fcb, D6FS_FCB_WORDS);
        extent_count = fi->extent_count;
        if (new_words == 0UL)
                new_tail = 0U;
        old_blocks = d6fs_provider_blocks_for_words(fi->size_words);
        new_blocks = d6fs_provider_blocks_for_words(new_words);

        if (new_blocks > old_blocks) {
                need = new_blocks - old_blocks;
                if (extent_count != 0U) {
                        i = extent_count - 1U;
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
                                        goto rollback;
                                if (d6fs_freemap_set(&d6fs_provider_reader,
                                    candidate, 1U) != 0)
                                        goto rollback;
                                ++last_blocks;
                                --need;
                                d6fs_provider_set_extent(fcb, i,
                                    last_start, last_blocks);
                        }
                }
                while (need != 0UL) {
                        if (extent_count >= D6FS_EXTENTS) {
                                goto rollback;
                        }
                        blocks = need;
                        if (blocks > D6FS_EXTENT_MAX_BLOCKS)
                                blocks = D6FS_EXTENT_MAX_BLOCKS;
                        if (d6fs_alloc_run(&d6fs_provider_reader,
                            d6fs_provider_reader.alloc_cursor, blocks, &start, &blocks) != 0) {
                                goto rollback;
                        }
                        for (candidate = 0UL; candidate < blocks; ++candidate) {
                                if (d6fs_reader_zero_block(
                                    &d6fs_provider_reader,
                                    start + candidate) == 0 &&
                                    d6fs_freemap_set(&d6fs_provider_reader,
                                    start + candidate, 1U) == 0)
                                        continue;
                                if (candidate != 0UL)
                                        (void)d6fs_free_run(&d6fs_provider_reader,
                                            start, candidate);
                                goto rollback;
                        }
                        i = extent_count;
                        d6fs_provider_set_extent(fcb, i, start, blocks);
                        ++extent_count;
                        need -= blocks;
                        d6fs_provider_reader.alloc_cursor = start + blocks;
                        if (d6fs_provider_reader.alloc_cursor >=
                            d6fs_provider_reader.super.total_blocks)
                                d6fs_provider_reader.alloc_cursor = 0UL;
                }
        } else if (new_blocks < old_blocks) {
                kword_t keep;
                kword_t extent_start;
                kword_t extent_blocks;

                keep = new_blocks;
                for (i = 0U; keep != 0UL; ++i) {
                        high = d6fs_extent_high_get(
                            old_fcb[D6FS_FCB_LENHIGH], i);
                        if (d6fs_extent_decode(
                            old_fcb[D6FS_FCB_EXTENT0 + i], high,
                            &extent_start, &extent_blocks) != 0)
                                return -1;
                        if (extent_blocks > keep)
                                extent_blocks = keep;
                        d6fs_provider_set_extent(fcb, i,
                            extent_start, extent_blocks);
                        keep -= extent_blocks;
                }
                extent_count = i;
                for (; i < D6FS_EXTENTS; ++i)
                        d6fs_provider_clear_extent(fcb, i);
        }

        /* Resize changes only size, tail, and extent count.  Preserve every
         * other on-disk metadata bit exactly as it was read. */
        fcb[D6FS_FCB_META] = (fcb[D6FS_FCB_META] & ~07760UL) |
            ((kword_t)new_tail << 8) |
            ((kword_t)extent_count << 4);
        fcb[D6FS_FCB_SIZE] = new_words;
        if (d6fs_reader_put_fcb(&d6fs_provider_reader,
            VFS_INDEX(node), fcb) != 0) {
                if (new_blocks > old_blocks)
                        goto rollback;
                return -1;
        }
        if (new_blocks < old_blocks &&
            d6fs_provider_free_file_tail(old_fcb, new_blocks) != 0)
                return -1;
        fi->extent_count = extent_count;
        fi->size_words = new_words;
        fi->tail = new_tail;
        return 0;

rollback:
        (void)d6fs_provider_free_file_tail(fcb, old_blocks);
        fs_copy_words(old_fcb, fcb, D6FS_FCB_WORDS);
        return -1;
}

static int
d6fs_provider_scan_slot(vnode_t dir, const struct vfs_name *name,
    unsigned int *slotp, struct d6fs_dirent_info *dip)
{
        struct d6fs_dirent_info di;
        kword_t hash;
        unsigned int slot;
        int rc;

        if (name != 0) {
                if (!vfs_name_valid(name))
                        return -1;
                hash = d6fs_name_hash24(name->words, name->chars);
        } else if (slotp == 0) {
                return -1;
        } else {
                hash = 0UL;
        }
        for (slot = 0U;; ++slot) {
                rc = d6fs_provider_dirent(dir, slot, &di);
                if (rc == 0) {
                        if (name != 0)
                                return 1;
                        *slotp = slot;
                        return 1;
                }
                if (rc < 0)
                        return -1;
                if (name == 0) {
                        if (di.child_fcb != 0U)
                                continue;
                        *slotp = slot;
                        return 0;
                }
                if (di.child_fcb == 0U || di.hash != hash ||
                    !d6fs_provider_name_equal(&di, name))
                        continue;
                if (slotp != 0)
                        *slotp = slot;
                if (dip != 0)
                        *dip = di;
                return 0;
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
        kword_t off;
        unsigned int tail;

        if (d6fs_provider_fcb(dir, fcb, &fi) != 0 ||
            fi.type != D6FS_TYPE_DIR ||
            (fi.flags & D6FS_FLAG_IMMUTABLE) != 0U)
                return -1;
        if (di != 0) {
                fs_copy_words(di->name, raw, VFS_NAME_WORDS);
                raw[4] = (di->hash << D6FS_DIRENT_HASH_SHIFT) |
                    ((kword_t)di->type << D6FS_DIRENT_TYPE_SHIFT) |
                    (kword_t)di->flags;
                raw[5] = (kword_t)di->child_fcb << 18;
        } else {
                fs_zero_words(raw, D6FS_DIRENT_WORDS);
        }
        off = (kword_t)slot * D6FS_DIRENT_WORDS;
        need = off + D6FS_DIRENT_WORDS;
        if (need > fi.size_words) {
                tail = 4U;
                if (d6fs_provider_resize_fcb(dir, fcb, &fi, need, tail) != 0)
                        return -1;
        }
        return d6fs_reader_write_words(&d6fs_provider_reader, fcb,
            off, raw,
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

        if (nodep == 0 || (type != D6FS_TYPE_REG && type != D6FS_TYPE_DIR &&
            type != D6FS_TYPE_SYMLINK) ||
            (type == D6FS_TYPE_SYMLINK && (payload == 0 || value == 0U)) ||
            d6fs_provider_scan_slot(dir, name, 0, 0) == 0 ||
            d6fs_provider_free_fcb(&index) != 0)
                return -1;
        if (d6fs_provider_scan_slot(dir, 0, &slot, 0) < 0)
                return -1;

        mode = type == D6FS_TYPE_SYMLINK ? 0777U : value;
        fs_zero_words(fcb, D6FS_FCB_WORDS);
        fcb[D6FS_FCB_META] = ((kword_t)type << 33) |
            ((kword_t)(mode & 07777U) << 12);
        fcb[D6FS_FCB_PARENT] = (kword_t)VFS_INDEX(dir) << 18;
        if (d6fs_reader_put_fcb(&d6fs_provider_reader, index, fcb) != 0)
                return -1;
        node = VFS_NODE(D6FS_PROVIDER,
            VFS_MOUNT_KIND(VFS_MOUNT_ID(dir), D6FS_KIND_NODE), index);

        if (type == D6FS_TYPE_SYMLINK) {
                fi.type = type;
                fi.extent_count = 0U;
                fi.size_words = 0UL;
                words = (value + 5U) / 6U;
                tail = value - (words - 1U) * 6U;
                if (d6fs_provider_resize_fcb(node, fcb, &fi, words, tail) != 0 ||
                    d6fs_reader_write_words(&d6fs_provider_reader, fcb, 0UL,
                    payload, words) != (int)words)
                        goto fail;
        }

        fs_copy_words(name->words, di.name, VFS_NAME_WORDS);
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
        fs_zero_words(fcb, D6FS_FCB_WORDS);
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

        if (d6fs_provider_scan_slot(dir, name, &slot, &di) != 0 ||
            d6fs_reader_fcb(&d6fs_provider_reader, di.child_fcb,
            fcb, &fi) != 0 ||
            (fi.flags & (D6FS_FLAG_NOUNLINK | D6FS_FLAG_IMMUTABLE)) != 0U)
                return -1;
        fs_copy_words(fcb, old_fcb, D6FS_FCB_WORDS);
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
        fs_zero_words(fcb, D6FS_FCB_WORDS);
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
        int empty_rc;

        if (VFS_MOUNT_ID(olddir) != VFS_MOUNT_ID(newdir) ||
            d6fs_provider_scan_slot(olddir, oldname, &oldslot, &di) != 0 ||
            d6fs_provider_scan_slot(newdir, newname, 0, 0) == 0 ||
            d6fs_reader_fcb(&d6fs_provider_reader, di.child_fcb,
            fcb, &fi) != 0 ||
            (fi.flags & (D6FS_FLAG_NOUNLINK | D6FS_FLAG_IMMUTABLE)) != 0U)
                return -1;
        old_di = di;
        old_parent = fi.parent_fcb;
        if (olddir == newdir) {
                newslot = oldslot;
        } else {
                empty_rc = d6fs_provider_scan_slot(newdir, 0, &newslot, 0);
                if (empty_rc < 0)
                        return -1;
        }
        fs_copy_words(newname->words, di.name, VFS_NAME_WORDS);
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
        fcb[D6FS_FCB_META] = (fcb[D6FS_FCB_META] & ~07777000UL) |
            ((kword_t)(mode & 07777U) << 12);
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
        }
        return d6fs_reader_write_words(&d6fs_provider_reader, fcb,
            off, buf, nwords);
}
