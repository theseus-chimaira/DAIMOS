#include "d6fs_provider.h"
#include "fs_mres.h"
#include "../proc/proc.h"

#ifdef __PDP10__
extern kword_t pclk_time36(void);
#define D6FS_NOW() pclk_time36()
#else
#define D6FS_NOW() 0UL
#endif

void d6fs_provider_set_extent(kword_t fcb[D6FS_FCB_WORDS],
    unsigned int index, kword_t start, kword_t blocks);
void d6fs_provider_clear_extent(kword_t fcb[D6FS_FCB_WORDS],
    unsigned int index);
int d6fs_provider_free_file_tail(const kword_t fcb[D6FS_FCB_RESERVED0],
    kword_t first_file_block);
int d6fs_provider_alloc_run(kword_t max_blocks,
    kword_t *startp, kword_t *blocksp);
int d6fs_provider_resize_fcb(vnode_t node,
    kword_t fcb[D6FS_FCB_WORDS], struct d6fs_fcb_info *fi,
    kword_t new_words);

int d6fs_provider_scan_slot(vnode_t dir,
    const struct vfs_name *name, unsigned int *slotp,
    struct d6fs_dirent_info *dip);

int d6fs_provider_fcb(vnode_t node, kword_t fcb[D6FS_FCB_WORDS],
    struct d6fs_fcb_info *info);
unsigned int d6fs_provider_vtype(unsigned int type);

int d6fs_provider_dirent(vnode_t dir, unsigned int slot,
    struct d6fs_dirent_info *di);

int d6fs_provider_readdir(vnode_t dir, unsigned int off,
    struct vfs_dirent *ent);
int d6fs_provider_parent_name(vnode_t node, vnode_t *parentp,
    struct vfs_name *namep);

#ifndef __PDP10__
int
d6fs_provider_resize_fcb(vnode_t node,
    kword_t fcb[D6FS_FCB_WORDS], struct d6fs_fcb_info *fi,
    kword_t new_words)
{
        kword_t old_fcb[D6FS_FCB_RESERVED0];
        long old_blocks;
        long new_blocks;
        long need;
        kword_t start;
        kword_t blocks;
        kword_t last_start;
        kword_t last_blocks;
        long candidate;
        int allocated;
        unsigned int high;
        int i;
        int extent_count;

        if (((unsigned int)d6fs_active_reader->opaque &
            D6FS_PROVIDER_MOUNT_WRITABLE) == 0U)
                return -1;
        fs_copy_words(fcb, old_fcb, D6FS_FCB_RESERVED0);
        extent_count = (int)fi->extent_count;
        old_blocks = (long)((fi->size_words + 0177UL) >> 7);
        {
                long n;

                n = (long)((new_words + 0177UL) >> 7);
                if (n > (long)d6fs_active_reader->super.total_blocks)
                        return -1;
                new_blocks = n;
        }

        if (new_blocks > old_blocks) {
                need = new_blocks - old_blocks;
                if (extent_count != 0U) {
                        i = extent_count - 1;
                        high = d6fs_extent_high_get(
                            fcb[D6FS_FCB_LENHIGH], i);
                        if (d6fs_extent_decode(
                            fcb[D6FS_FCB_EXTENT0 + i], high,
                            &last_start, &last_blocks) != 0)
                                return -1;
                        while (need != 0UL &&
                            last_blocks < D6FS_EXTENT_MAX_BLOCKS) {
                                candidate = last_start + last_blocks;
                                if (candidate >= (long)d6fs_active_reader->super.total_blocks)
                                        break;
                                allocated = d6fs_freemap_state(
                                    d6fs_active_reader, candidate);
                                if (allocated != 0)
                                        break;
                                if (d6fs_reader_zero_block(
                                    d6fs_active_reader, candidate) != 0)
                                        goto rollback;
                                if (d6fs_freemap_set(d6fs_active_reader,
                                    candidate, 1U) != 0)
                                        goto rollback;
                                ++last_blocks;
                                --need;
                                d6fs_provider_set_extent(fcb, i,
                                    last_start, last_blocks);
                        }
                }
                while (need != 0UL) {
                        if (extent_count >= (int)D6FS_EXTENTS) {
                                goto rollback;
                        }
                        blocks = need;
                        if (blocks > D6FS_EXTENT_MAX_BLOCKS)
                                blocks = D6FS_EXTENT_MAX_BLOCKS;
                        if (d6fs_provider_alloc_run(blocks,
                            &start, &blocks) != 0) {
                                goto rollback;
                        }
                        for (candidate = 0; candidate < (long)blocks; ++candidate) {
                                if (d6fs_reader_zero_block(
                                    d6fs_active_reader,
                                    start + candidate) == 0 &&
                                    d6fs_freemap_set(d6fs_active_reader,
                                    start + candidate, 1U) == 0)
                                        continue;
                                if (candidate != 0)
                                        (void)d6fs_free_run(d6fs_active_reader,
                                            start, candidate);
                                goto rollback;
                        }
                        i = extent_count;
                        d6fs_provider_set_extent(fcb, i, start, blocks);
                        ++extent_count;
                        need -= blocks;
                        d6fs_active_reader->alloc_cursor = start + blocks;
                        if ((long)d6fs_active_reader->alloc_cursor >=
                            (long)d6fs_active_reader->super.total_blocks)
                                d6fs_active_reader->alloc_cursor = 0UL;
                }
        } else if (new_blocks < old_blocks) {
                kword_t keep;
                kword_t extent_start;
                kword_t extent_blocks;

                keep = new_blocks;
                for (i = 0; keep != 0UL; ++i) {
                        high = d6fs_extent_high_get(
                            old_fcb[D6FS_FCB_LENHIGH], i);
                        if (d6fs_extent_decode(
                            old_fcb[D6FS_FCB_EXTENT0 + i], high,
                            &extent_start, &extent_blocks) != 0)
                                return -1;
                        if ((long)extent_blocks > (long)keep)
                                extent_blocks = (kword_t)keep;
                        d6fs_provider_set_extent(fcb, i,
                            extent_start, extent_blocks);
                        keep -= extent_blocks;
                }
                extent_count = i;
                for (; i < (int)D6FS_EXTENTS; ++i)
                        d6fs_provider_clear_extent(fcb, i);
        }

        /* Resize changes only size and extent count.  The former tail bits
         * are reserved zero in the SIXBIT-only format. */
        fcb[D6FS_FCB_META] = (fcb[D6FS_FCB_META] & ~07760UL) |
            ((kword_t)extent_count << 4);
        fcb[D6FS_FCB_SIZE] = new_words;
        if (d6fs_reader_put_fcb(d6fs_active_reader,
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
        fi->tail = 0U;
        return 0;

rollback:
        /* Make newly added extents visible to the file-block walker before
         * releasing the grown tail.  The normal metadata publication has not
         * happened yet on this failure path. */
        fcb[D6FS_FCB_META] = (fcb[D6FS_FCB_META] & ~0360UL) |
            ((kword_t)extent_count << 4);
        (void)d6fs_provider_free_file_tail(fcb, old_blocks);
        fs_copy_words(old_fcb, fcb, D6FS_FCB_RESERVED0);
        return -1;
}

#endif

int
d6fs_provider_scan_slot(vnode_t dir, const struct vfs_name *name,
    unsigned int *slotp, struct d6fs_dirent_info *dip)
{
        struct d6fs_dirent_info di;
        kword_t hash;
        unsigned int slot;
        unsigned int empty_slot;
        int have_empty;
        int rc;

        if (name != 0) {
                if (!vfs_name_valid(name))
                        return -1;
                hash = d6fs_name_hash24(name->words, name->chars);
        }
        have_empty = 0;
        for (slot = 0U;; ++slot) {
                rc = d6fs_provider_dirent(dir, slot, &di);
                if (rc == 0) {
                        if (name != 0) {
                                *slotp = have_empty ? empty_slot : slot;
                                return 1;
                        }
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
                if (di.child_fcb == 0U) {
                        if (dip == 0 && !have_empty) {
                                empty_slot = slot;
                                have_empty = 1;
                        }
                        continue;
                }
                if (di.hash != hash ||
                    !vfs_name_words_equal(di.name, name->words, VFS_NAME_WORDS))
                        continue;
                *slotp = slot;
                if (dip != 0)
                        *dip = di;
                return 0;
        }
}

#ifndef __PDP10__
static int
d6fs_provider_free_fcb(unsigned int *indexp)
{
        kword_t fcb[D6FS_FCB_WORDS];
        struct d6fs_fcb_info fi;
        unsigned int i;

        for (i = 1U; i < d6fs_active_reader->super.fcb_count; ++i) {
                if (d6fs_reader_fcb(d6fs_active_reader, i, fcb, &fi) != 0)
                        continue;
                if (fi.type == D6FS_TYPE_FREE) {
                        *indexp = i;
                        return 0;
                }
        }
        return -1;
}
#else
extern int d6fs_provider_free_fcb(unsigned int *indexp);
#endif

#ifndef __PDP10__
static int
d6fs_provider_write_dirent(vnode_t dir, unsigned int slot,
    const struct d6fs_dirent_info *di)
{
        kword_t fcb[D6FS_FCB_WORDS];
        struct d6fs_fcb_info fi;
        kword_t raw[D6FS_DIRENT_WORDS];
        kword_t need;
        kword_t off;

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
        if (need > fi.size_words &&
            d6fs_provider_resize_fcb(dir, fcb, &fi, need) != 0)
                return -1;
        return d6fs_reader_write_words(d6fs_active_reader, fcb,
            off, raw,
            D6FS_DIRENT_WORDS) == (int)D6FS_DIRENT_WORDS ? 0 : -1;
}
#else
extern int d6fs_provider_write_dirent(vnode_t dir, unsigned int slot,
    const struct d6fs_dirent_info *di);
#endif

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

#ifndef __PDP10__
        if (nodep == 0 || (type != D6FS_TYPE_REG && type != D6FS_TYPE_DIR &&
            type != D6FS_TYPE_SYMLINK && type != D6FS_TYPE_FIFO) ||
            (type == D6FS_TYPE_SYMLINK && (payload == 0 || payload[0] == 0UL)))
                return -1;
#endif
        if (d6fs_provider_scan_slot(dir, name, &slot, 0) == 0 ||
            d6fs_provider_free_fcb(&index) != 0)
                return -1;

        mode = type == D6FS_TYPE_SYMLINK ? 0777U : value;
        fs_zero_words(fcb, D6FS_FCB_WORDS);
        fcb[D6FS_FCB_META] = ((kword_t)type << 33) |
            ((kword_t)(mode & 07777U) << 12);
        fcb[D6FS_FCB_MTIME] = D6FS_NOW();
        if (proc_table != 0 && proc_current_slot != 0UL &&
            PROC_HAS_UAREA(&proc_table[(unsigned int)proc_current_slot])) {
                struct proc *p;

                p = &proc_table[(unsigned int)proc_current_slot];
                fcb[D6FS_FCB_OWNER] = ((kword_t)PROC_UID(p) << 18U) |
                    (kword_t)PROC_GID(p);
        }
        fcb[D6FS_FCB_PARENT] = (kword_t)VFS_INDEX(dir) << 18;
        if (d6fs_reader_put_fcb(d6fs_active_reader, index, fcb) != 0)
                return -1;
        node = VFS_NODE_PACKED(D6FS_PROVIDER, D6FS_KIND_NODE, index);

        if (type == D6FS_TYPE_SYMLINK) {
                fi.type = type;
                fi.extent_count = 0U;
                fi.size_words = 0UL;
                words = 1U + ((unsigned int)payload[0] + 5U) / 6U;
                if (d6fs_provider_resize_fcb(node, fcb, &fi, words) != 0)
                        goto fail_fcb;
                if (d6fs_reader_write_words(d6fs_active_reader, fcb, 0UL,
                    payload, words) != (int)words)
                        goto fail_symlink;
        }

        fs_copy_words(name->words, di.name, VFS_NAME_WORDS);
        di.hash = d6fs_name_hash24(name->words, name->chars);
        di.type = type;
        di.flags = 0U;
        di.child_fcb = index;
        if (d6fs_provider_write_dirent(dir, slot, &di) != 0) {
                if (type == D6FS_TYPE_SYMLINK)
                        goto fail_symlink;
                goto fail_fcb;
        }
        *nodep = node;
        return 0;

fail_symlink:
        (void)d6fs_provider_resize_fcb(node, fcb, &fi, 0UL);
fail_fcb:
        fs_zero_words(fcb, D6FS_FCB_WORDS);
        (void)d6fs_reader_put_fcb(d6fs_active_reader, index, fcb);
        return -1;
}

int
d6fs_provider_unlink(vnode_t dir, const struct vfs_name *name)
{
        struct d6fs_dirent_info di;
        kword_t fcb[D6FS_FCB_WORDS];
        kword_t old_fcb[D6FS_FCB_RESERVED0];
        struct d6fs_fcb_info fi;
        unsigned int slot;

        if (d6fs_provider_scan_slot(dir, name, &slot, &di) != 0 ||
            d6fs_reader_fcb(d6fs_active_reader, di.child_fcb,
            fcb, &fi) != 0 ||
            (fi.flags & (D6FS_FLAG_NOUNLINK | D6FS_FLAG_IMMUTABLE)) != 0U)
                return -1;
        fs_copy_words(fcb, old_fcb, D6FS_FCB_RESERVED0);
        if (fi.type == D6FS_TYPE_DIR && fi.size_words != 0UL) {
                struct d6fs_dirent_info child;
                unsigned int s;
                int rc;

                for (s = 0U;; ++s) {
                        rc = d6fs_provider_dirent(VFS_NODE_PACKED(
                            D6FS_PROVIDER, D6FS_KIND_NODE, di.child_fcb),
                            s, &child);
                        if (rc == 0)
                                break;
                        if (rc < 0 || child.child_fcb != 0U)
                                return -1;
                }
        }
        if (d6fs_provider_write_dirent(dir, slot, 0) != 0)
                return -1;
        fs_zero_words(fcb, D6FS_FCB_WORDS);
        if (d6fs_reader_put_fcb(d6fs_active_reader,
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
            d6fs_provider_scan_slot(newdir, newname, &newslot, 0) == 0 ||
            d6fs_reader_fcb(d6fs_active_reader, di.child_fcb,
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
                fcb[D6FS_FCB_PARENT] = (kword_t)VFS_INDEX(newdir) << 18;
                if (d6fs_reader_put_fcb(d6fs_active_reader,
                    di.child_fcb, fcb) != 0) {
                        (void)d6fs_provider_write_dirent(olddir, oldslot,
                            &old_di);
                        return -1;
                }
                if (d6fs_provider_write_dirent(newdir, newslot, &di) != 0) {
                        fcb[D6FS_FCB_PARENT] = (kword_t)old_parent << 18;
                        (void)d6fs_reader_put_fcb(d6fs_active_reader,
                            di.child_fcb, fcb);
                        (void)d6fs_provider_write_dirent(olddir, oldslot,
                            &old_di);
                        return -1;
                }
        }
        return 0;
}
