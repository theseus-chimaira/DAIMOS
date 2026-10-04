#include "dtfs.h"
#include "dtfs_media.h"
#include "fs_mres.h"
#include "syscall.h"

#ifndef DTFS_ENABLE_TENEX
#define DTFS_ENABLE_TENEX 0
#endif
#ifndef DTFS_ENABLE_ITS
#define DTFS_ENABLE_ITS 0
#endif

/* One directory and one transfer block are shared by every DTFS mount.
 * On the PDP-6 the directory cache is allocated from managed kernel memory
 * after MRES packing, so 128 cache words do not consume scarce permanent
 * low-core address space. */
kword_t *dtfs_dir;
#define dtfs_block fs_block_workspace
unsigned int dtfs_cache_mount;
/* Unit number and the read-only foreign-media personality share one word. */
/* Low half carries unit/personality; high half carries mount UID9,GID9. */
kword_t dtfs_media[VFS_NMOUNT];

extern int dtfs_is_root(vnode_t node);
extern int dtfs_is_file(vnode_t node);

extern unsigned int dtfs_owner(unsigned int base, unsigned int index);
extern void dtfs_set_owner(unsigned int base, unsigned int index,
    unsigned int owner);

int dtfs_chain_walk(unsigned int unit, unsigned int slot,
    unsigned int off, kword_t *buf, unsigned int nwords,
    unsigned int mapoff, int writing);

extern unsigned int dtfs_personality(vnode_t node);
#if !DTFS_ENABLE_TENEX && !DTFS_ENABLE_ITS
#define dtfs_personality(node) 0U
#endif

unsigned int
dtfs_unit(vnode_t node)
{
        return dtfs_media[VFS_MOUNT_ID(node) - 1U] & DTFS_MEDIA_UNIT_MASK;
}

extern int dtfs_load(vnode_t node);
extern void dtfs_patch_media(unsigned int mount, unsigned int media);

int
dtfs_commit(vnode_t node)
{
        unsigned int media;

        media = dtfs_media[VFS_MOUNT_ID(node) - 1U];
#if DTFS_ENABLE_ITS
        return dtfs_dtc_write(media & DTFS_MEDIA_UNIT_MASK,
            (media & DTFS_MEDIA_ITS) != 0U ? DTFS_ITS_DIR_BLOCK :
            DTFS_DIR_BLOCK, dtfs_dir);
#else
        return dtfs_dtc_write(media & DTFS_MEDIA_UNIT_MASK,
            DTFS_DIR_BLOCK, dtfs_dir);
#endif
}

extern int dtfs_native_scan_slot(const struct vfs_name *name,
    unsigned int *slotp);

/* Present TENEX/ITS NAME and EXT fields as one packed VFS NAME.EXT. */
extern void dtfs_foreign_name(unsigned int slot, struct vfs_name *name,
    int its);


extern int dtfs_foreign_set_name(unsigned int slot,
    const struct vfs_name *name, int its);


#if !DTFS_ENABLE_TENEX && !DTFS_ENABLE_ITS
#define dtfs_scan_slot(node, name, slotp) \
        dtfs_native_scan_slot((name), (slotp))
#else
extern int dtfs_scan_slot(vnode_t node, const struct vfs_name *name,
    unsigned int *slotp);

#endif

extern void dtfs_set_name(unsigned int slot, const struct vfs_name *name);

extern void dtfs_clear_slot(unsigned int slot);
extern void dtfs_set_last_words(unsigned int slot, unsigned int words);
extern void dtfs_set_exec(unsigned int slot, int executable);


/* These are pure on-media field operations.  Keep them as expressions so the
 * PDP-10 compiler emits LDB/shift-OR sequences at the use site rather than
 * paying a subroutine call for each block header. */
#define DTFS_HDR_NEXT(h) \
    ((unsigned int)(((h) >> DTFS_NEXT_SHIFT) & DTFS_BLOCKNO_MASK))
#define DTFS_HEADER(next, first, count) \
    (((kword_t)(next) << DTFS_NEXT_SHIFT) | \
    ((kword_t)(first) << DTFS_FIRST_SHIFT) | (kword_t)(count))

#if DTFS_ENABLE_TENEX || DTFS_ENABLE_ITS
#else
#endif

extern unsigned int dtfs_block_info(vnode_t node, unsigned int slot,
    unsigned int *firstp);

extern unsigned int dtfs_size_words(vnode_t node, unsigned int slot);

extern int dtfs_find_free_block(unsigned int start, int tenex,
    unsigned int *blockp);

#if DTFS_ENABLE_ITS
static int
dtfs_its_resize(vnode_t node, unsigned int words, int grow_only)
{
        unsigned int slot;
        unsigned int owner;
        int old_blocks;
        int new_blocks;
        unsigned int block;
        unsigned int last;

        slot = VFS_INDEX(node);
        owner = slot + 1U;
        if (words > DTFS_ITS_END_BLOCK * DTFS_BLOCK_WORDS)
                return -1;
        new_blocks = (int)((words + DTFS_BLOCK_WORDS - 1U) / DTFS_BLOCK_WORDS);
        old_blocks = 0;
        last = 0U;
        for (block = 1U; block != DTFS_ITS_END_BLOCK + 1U; ++block)
                if (dtfs_owner(DTFS_ITS_NAME_WORDS, block - 1U) == owner) {
                        ++old_blocks;
                        last = block;
                        if (!grow_only && old_blocks > new_blocks)
                                dtfs_set_owner(DTFS_ITS_NAME_WORDS, block - 1U,
                                    DTFS_OWNER_FREE);
                }
        if (new_blocks <= old_blocks) {
                if (grow_only || new_blocks == old_blocks)
                        return 0;
                goto commit;
        }
        {
                unsigned int unit;

                unit = dtfs_unit(node);
                for (block = last + 1U; old_blocks != new_blocks; ++block) {
                        if (block == DTFS_ITS_END_BLOCK + 1U) {
                                dtfs_cache_mount = 0U;
                                return -1;
                        }
                        if (dtfs_owner(DTFS_ITS_NAME_WORDS, block - 1U) != DTFS_OWNER_FREE)
                                continue;
                        fs_zero_block_workspace();
                        if (dtfs_dtc_write(unit, block, dtfs_block) != 0) {
                                dtfs_cache_mount = 0U;
                                return -1;
                        }
                        dtfs_set_owner(DTFS_ITS_NAME_WORDS, block - 1U, owner);
                        ++old_blocks;
                }
        }
commit:
        if (dtfs_commit(node) != 0) {
                dtfs_cache_mount = 0U;
                return -1;
        }
        return 0;
}

#endif

#if !DTFS_ENABLE_TENEX && !DTFS_ENABLE_ITS
static int
dtfs_resize(vnode_t node, unsigned int words)
{
        unsigned int slot;
        int old_blocks;
        int new_blocks;
        unsigned int first;
        unsigned int prev;
        unsigned int block;
        unsigned int next;
        unsigned int i;
        unsigned int unit;
        unsigned int owner;
        unsigned int last_words;
        unsigned int personality;
        unsigned int mapoff;
        int word_count;

        if (!dtfs_is_file(node) || dtfs_load(node) != 0)
                return -1;
        if (words > DTFS_LAST_BLOCK * DTFS_DATA_WORDS)
                return -1;
        word_count = (int)words;
        personality = dtfs_personality(node);
#if DTFS_ENABLE_ITS
        if (personality == DTFS_MEDIA_ITS)
                return dtfs_its_resize(node, words, 0);
#endif
        mapoff = personality == DTFS_MEDIA_TENEX ? 1U : 0U;
        slot = VFS_INDEX(node);
        unit = dtfs_unit(node);
        owner = slot + 1U;
        old_blocks = (int)dtfs_block_info(node, slot, &first);
        new_blocks = word_count == 0 ? (int)mapoff :
            (word_count + (int)DTFS_DATA_WORDS - 1) /
            (int)DTFS_DATA_WORDS;
        last_words = new_blocks == 0 ? 0U :
            (unsigned int)(word_count -
            (new_blocks - 1) * (int)DTFS_DATA_WORDS);

        if (old_blocks == 0U) {
                first = 0U;
                prev = 0U;
        } else {
                if (first == 0U)
                        return -1;
                prev = first;
                for (i = 1U; i != old_blocks; ++i) {
                        if (dtfs_dtc_read(unit, prev, dtfs_block) != 0)
                                return -1;
                        next = DTFS_HDR_NEXT(dtfs_block[0]);
                        if (next == 0U || next > DTFS_LAST_BLOCK)
                                return -1;
                        prev = next;
                }
        }

        if (old_blocks < new_blocks) {
                do {
                        if (dtfs_find_free_block(prev + 1U, mapoff, &block) != 0)
                                return -1;
                        if (first == 0U)
                                first = block;
                        fs_zero_block_workspace();
                        dtfs_block[0] = DTFS_HEADER(0U, first,
                            old_blocks + 1U == new_blocks ? last_words :
                            DTFS_DATA_WORDS);
                        if (dtfs_dtc_write(unit, block, dtfs_block) != 0)
                                return -1;
                        dtfs_set_owner(0U, block - mapoff, owner);
                        if (prev != 0U) {
                                if (dtfs_dtc_read(unit, prev,
                                    dtfs_block) != 0)
                                        return -1;
                                dtfs_block[0] = DTFS_HEADER(block, first,
                                    DTFS_DATA_WORDS);
                                if (dtfs_dtc_write(unit, prev,
                                    dtfs_block) != 0)
                                        return -1;
                        }
                        prev = block;
                        ++old_blocks;
                } while (old_blocks != new_blocks);
        } else if (new_blocks < old_blocks) {
                if (new_blocks == 0U) {
                        block = first;
                } else {
                        block = first;
                        for (i = 1U; i != new_blocks; ++i) {
                                if (dtfs_dtc_read(unit, block,
                                    dtfs_block) != 0)
                                        return -1;
                                block = DTFS_HDR_NEXT(dtfs_block[0]);
                        }
                        if (dtfs_dtc_read(unit, block, dtfs_block) != 0)
                                return -1;
                        next = DTFS_HDR_NEXT(dtfs_block[0]);
                        dtfs_block[0] = DTFS_HEADER(0U, first, last_words);
                        if (dtfs_dtc_write(unit, block, dtfs_block) != 0)
                                return -1;
                        block = next;
                }
                while (block != 0U) {
                        if (block > DTFS_LAST_BLOCK ||
                            dtfs_dtc_read(unit, block, dtfs_block) != 0)
                                return -1;
                        next = DTFS_HDR_NEXT(dtfs_block[0]);
                        dtfs_set_owner(0U, block - mapoff, DTFS_OWNER_FREE);
                        block = next;
                }
        } else if (new_blocks != 0U) {
                /* PREV already names the last block from the initial walk. */
                if (dtfs_dtc_read(unit, prev, dtfs_block) != 0)
                        return -1;
                dtfs_block[0] = DTFS_HEADER(0U, first, last_words);
                if (dtfs_dtc_write(unit, prev, dtfs_block) != 0)
                        return -1;
        }
        if (mapoff == 0U)
                dtfs_set_last_words(slot, last_words);
        return dtfs_commit(node);
}
#else
extern int dtfs_resize(vnode_t node, unsigned int words);
#endif

/* dtfs_mount_unit is implemented compactly in dtfs_runtime.s. */

extern int dtfs_lookup(vnode_t dir, const struct vfs_name *name,
    vnode_t *nodep);

extern int dtfs_readdir(vnode_t dir, unsigned int off,
    struct vfs_dirent *ent);

extern void dtfs_stat_owner(vnode_t node, struct vfs_stat *st);

int
dtfs_stat(vnode_t node, struct vfs_stat *st)
{
        unsigned int slot;
        unsigned int words;
        unsigned int personality;

        if (st == 0 || dtfs_load(node) != 0)
                return -1;
        personality = dtfs_personality(node);
        if (dtfs_is_root(node)) {
                st->type = VFS_TYPE_DIR;
                st->mode = (personality != 0U && vfs_readonly(node)) ?
                    0555U : 0777U;
                st->reserved = 0;
                st->size_words = 0;
                dtfs_stat_owner(node, st);
                return 0;
        }
        if (!dtfs_is_file(node))
                return -1;
        slot = VFS_INDEX(node);
        if (personality == DTFS_MEDIA_ITS) {
                if (slot >= DTFS_ITS_FILE_SLOTS ||
                    (dtfs_dir[slot * 2U] == 0UL &&
                    dtfs_dir[slot * 2U + 1U] == 0UL))
                        return -1;
                words = dtfs_block_info(node, slot, 0) * DTFS_BLOCK_WORDS;
        } else {
                if (slot >= DTFS_FILE_SLOTS || dtfs_dir[DTFS_NAME_BASE +
                    (personality == DTFS_MEDIA_TENEX ? slot : slot * 2U)] == 0)
                        return -1;
                words = dtfs_size_words(node, slot);
        }
        st->type = VFS_TYPE_REG;
        st->mode = (personality != 0U && vfs_readonly(node)) ? 0444U :
            (personality != 0U ? 0666U : 0666U |
            ((dtfs_dir[slot] & 1UL) != 0 ? 0111U : 0U));
        st->size_words = words;
        st->reserved = 0;
        dtfs_stat_owner(node, st);
        return 0;
}

extern int dtfs_parent(vnode_t node, vnode_t *parentp);


/* dtfs_create is implemented compactly in dtfs_runtime.s. */

/* dtfs_unlink is implemented compactly in dtfs_runtime.s. */

/* dtfs_rename is implemented compactly in dtfs_runtime.s. */

int
dtfs_truncate(vnode_t node, unsigned int words)
{
        return dtfs_resize(node, words);
}

extern int dtfs_chmod(vnode_t node, unsigned int mode);


static int
dtfs_transfer_words(vnode_t node, unsigned int off, kword_t *buf,
    unsigned int nwords, int writing)
{
        unsigned int size;
        unsigned int need;
        unsigned int personality;
        unsigned int slot;
        unsigned int mapoff;

        if (!dtfs_is_file(node) || buf == 0 || dtfs_load(node) != 0)
                return -1;
        personality = dtfs_personality(node);
        slot = VFS_INDEX(node);
        mapoff = personality == DTFS_MEDIA_TENEX ? 1U : 0U;
#if DTFS_ENABLE_ITS
        if (personality == DTFS_MEDIA_ITS) {
                if (slot >= DTFS_ITS_FILE_SLOTS ||
                    (dtfs_dir[slot * 2U] == 0UL &&
                    dtfs_dir[slot * 2U + 1U] == 0UL))
                        return -1;
                if (nwords == 0U)
                        return 0;
                if (writing && dtfs_its_resize(node, off + nwords, 1) != 0)
                        return -1;
                mapoff = DTFS_ITS_NAME_WORDS;
                goto transfer;
        }
#endif
        if (!writing) {
                if (personality == DTFS_MEDIA_TENEX &&
                    (slot >= DTFS_FILE_SLOTS ||
                    dtfs_dir[DTFS_NAME_BASE + slot] == 0UL))
                        return -1;
                if (nwords == 0U)
                        return 0;
        } else {
                size = dtfs_size_words(node, slot);
                need = off + nwords;
                /* Valid DTFS sizes are positive; a negative need is larger. */
                if (((long)need < 0L || (long)need > (long)size) &&
                    dtfs_resize(node, need) != 0)
                        return -1;
        }
#if DTFS_ENABLE_ITS
transfer:
#endif
        return dtfs_chain_walk(dtfs_unit(node), slot, off, buf,
            nwords, mapoff, writing);
}

int
dtfs_read_words(vnode_t node, unsigned int off, kword_t *buf,
    unsigned int nwords)
{
        return dtfs_transfer_words(node, off, buf, nwords, 0);
}

int
dtfs_write_words(vnode_t node, unsigned int off,
    const kword_t *buf, unsigned int nwords)
{
        return dtfs_transfer_words(node, off, (kword_t *)buf,
            nwords, 1);
}

extern int dtfs_sync(vnode_t node);
