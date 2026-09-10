#include "dtfs.h"
#include "fs_mres.h"
#include "syscall.h"

#ifndef DTFS_ENABLE_TENEX
#define DTFS_ENABLE_TENEX 0
#endif
#ifndef DTFS_ENABLE_ITS
#define DTFS_ENABLE_ITS 0
#endif

#define DTFS_BLOCK_WORDS      0200U
#define DTFS_BLOCKS           01102U
#define DTFS_LAST_BLOCK       01101U
#define DTFS_DIR_BLOCK        0144U       /* TENEX/native: decimal 100 */
#define DTFS_ITS_DIR_BLOCK    0100U       /* ITS: octal 100 */
#define DTFS_MAP_WORDS        0123U       /* decimal 83 */
#define DTFS_NAME_BASE        0123U
#define DTFS_TENEX_EXT_BASE   0151U       /* decimal 105 */
#define DTFS_MAGIC_WORD       0177U
#define DTFS_DATA_WORDS       0177U

#define DTFS_OWNER_FREE       000U
#define DTFS_OWNER_NATIVE_TAG 035U
#define DTFS_OWNER_RESERVED   036U
#define DTFS_OWNER_INVALID    037U
#define DTFS_NATIVE_MAGIC     0446446632021UL
#define DTFS_TENEX_MAX_FILE   026U
#define DTFS_ITS_FILE_SLOTS   027U       /* 23 decimal */
#define DTFS_ITS_NAME_WORDS   056U       /* 23 two-word names */
#define DTFS_ITS_MAP_ENTRIES  01076U     /* 82 words, seven 5-bit bytes each */
#define DTFS_ITS_END          037U
#define DTFS_ITS_DIR_OWNER    033U
#define DTFS_ITS_END_BLOCK    01067U
#define DTFS_ITS_MAP_FIRST    056U
#define DTFS_ITS_MAP_DIR      067U
#define DTFS_ITS_MAP_LAST     0177U
#define DTFS_ITS_MAP_RESERVED 0757367573674UL
#define DTFS_ITS_MAP_DIRWORD  0660000000000UL
#define DTFS_ITS_MAP_END      0777777777776UL
#define DTFS_TENEX_RESERVED   036U
#define DTFS_TENEX_INVALID    037U

#define DTFS_NEXT_SHIFT       18U
#define DTFS_FIRST_SHIFT      8U
#define DTFS_BLOCKNO_MASK     01777UL
#define DTFS_COUNT_MASK       0377UL
#define DTFS_NAME2_MASK       0777777777700UL
#define DTFS_TENEX_EXT_MASK   0777777000000UL

#define DTFS_MEDIA_UNIT_MASK  07U
#define DTFS_MEDIA_TENEX      010U
#define DTFS_MEDIA_ITS        020U

/* One directory and one transfer block are shared by every DTFS mount. */
kword_t dtfs_dir[DTFS_BLOCK_WORDS];
#define dtfs_block fs_block_workspace
unsigned int dtfs_cache_mount;
/* Unit number and the read-only foreign-media personality share one word. */
unsigned int dtfs_media[VFS_NMOUNT];

extern int dtfs_is_root(vnode_t node);
extern int dtfs_is_file(vnode_t node);

extern unsigned int dtfs_owner(unsigned int base, unsigned int index);
extern void dtfs_set_owner(unsigned int base, unsigned int index,
    unsigned int owner);

int dtfs_chain_walk(unsigned int unit, unsigned int slot,
    unsigned int off, kword_t *buf, unsigned int nwords,
    unsigned int mapoff, int writing);

extern int dtfs_native_valid(void);

extern int dtfs_its_valid(void);


extern int dtfs_tenex_valid(unsigned int unit, int deep);


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
int
dtfs_scan_slot(vnode_t node, const struct vfs_name *name,
    unsigned int *slotp)
{
        struct vfs_name media_name;
        unsigned int slot;
        unsigned int personality;
        int empty;
        int its;

        personality = dtfs_personality(node);
        if (personality == 0U)
                return dtfs_native_scan_slot(name, slotp);
        its = personality == DTFS_MEDIA_ITS;
        if (name != 0 && (!vfs_name_valid(name) ||
            name->chars > (its ? 13U : 10U)))
                return -1;
        for (slot = 0U; slot != (its ? DTFS_ITS_FILE_SLOTS :
            DTFS_FILE_SLOTS); ++slot) {
                if (its)
                        empty = dtfs_dir[slot * 2U] == 0UL &&
                            dtfs_dir[slot * 2U + 1U] == 0UL;
                else
                        empty = dtfs_dir[DTFS_NAME_BASE + slot] == 0UL;
                if (name == 0) {
                        if (!empty)
                                continue;
                        if (slotp != 0)
                                *slotp = slot;
                        return 0;
                }
                if (empty)
                        continue;
                dtfs_foreign_name(slot, &media_name, its);
                if (media_name.chars != name->chars ||
                    !fs_words_equal(media_name.words, name->words,
                    VFS_NAME_WORDS))
                        continue;
                if (slotp != 0)
                        *slotp = slot;
                return 0;
        }
        return -1;
}

#endif

extern void dtfs_set_name(unsigned int slot, const struct vfs_name *name);

static void
dtfs_clear_slot(unsigned int slot)
{
        unsigned int base;

        base = DTFS_NAME_BASE + slot * 2U;
        dtfs_dir[base] = 0;
        dtfs_dir[base + 1U] = 0;
        dtfs_dir[slot] &= ~1UL;
        dtfs_dir[22U + slot] &= ~1UL;
}

static void
dtfs_set_last_words(unsigned int slot, unsigned int words)
{
        unsigned int wi;

        wi = DTFS_NAME_BASE + slot * 2U + 1U;
        dtfs_dir[wi] = (dtfs_dir[wi] & ~077UL) | (words & 077U);
        dtfs_dir[22U + slot] = (dtfs_dir[22U + slot] & ~1UL) |
            ((words >> 6U) & 1U);
}

void
dtfs_set_exec(unsigned int slot, int executable)
{
        /* Both callers pass a C relational expression, hence exactly 0/1. */
        dtfs_dir[slot] = (dtfs_dir[slot] & ~1UL) | (kword_t)executable;
}

/* These are pure on-media field operations.  Keep them as expressions so the
 * PDP-10 compiler emits LDB/shift-OR sequences at the use site rather than
 * paying a subroutine call for each block header. */
#define DTFS_HDR_NEXT(h) \
    ((unsigned int)(((h) >> DTFS_NEXT_SHIFT) & DTFS_BLOCKNO_MASK))
#define DTFS_HEADER(next, first, count) \
    (((kword_t)(next) << DTFS_NEXT_SHIFT) | \
    ((kword_t)(first) << DTFS_FIRST_SHIFT) | (kword_t)(count))

#if DTFS_ENABLE_TENEX || DTFS_ENABLE_ITS
unsigned int
dtfs_block_info(vnode_t node, unsigned int slot, unsigned int *firstp)
{
        unsigned int block;
        unsigned int owner;
        unsigned int count;
        unsigned int mapoff;
        unsigned int media;
        unsigned int unit;

        owner = slot + 1U;
        count = 0U;
        if (firstp != 0)
                *firstp = 0U;
        media = dtfs_media[VFS_MOUNT_ID(node) - 1U];
        if ((media & DTFS_MEDIA_ITS) != 0U) {
                for (block = 0U; block != DTFS_ITS_MAP_ENTRIES; ++block)
                        if (dtfs_owner(DTFS_ITS_NAME_WORDS, block) == owner)
                                ++count;
                return count;
        }
        mapoff = (media & DTFS_MEDIA_TENEX) != 0U;
        unit = media & DTFS_MEDIA_UNIT_MASK;
        for (block = 1U; block != DTFS_LAST_BLOCK + 1U; ++block) {
                if (dtfs_owner(0U, block - mapoff) != owner)
                        continue;
                ++count;
                if (firstp != 0 && *firstp == 0U) {
                        if (dtfs_dtc_read(unit, block, dtfs_block) != 0)
                                return 0U;
                        if (((dtfs_block[0] >> DTFS_FIRST_SHIFT) &
                            DTFS_BLOCKNO_MASK) == block)
                                *firstp = block;
                }
        }
        return count;
}
#else
unsigned int
dtfs_block_info(vnode_t node, unsigned int slot, unsigned int *firstp)
{
        unsigned int block;
        unsigned int count;
        unsigned int owner;
        unsigned int unit;

        owner = slot + 1U;
        count = 0U;
        if (firstp != 0)
                *firstp = 0U;
        unit = dtfs_media[VFS_MOUNT_ID(node) - 1U] & DTFS_MEDIA_UNIT_MASK;
        for (block = 1U; block != DTFS_LAST_BLOCK + 1U; ++block) {
                if (dtfs_owner(0U, block) != owner)
                        continue;
                ++count;
                if (firstp != 0 && *firstp == 0U) {
                        if (dtfs_dtc_read(unit, block, dtfs_block) != 0)
                                return 0U;
                        if (((dtfs_block[0] >> DTFS_FIRST_SHIFT) &
                            DTFS_BLOCKNO_MASK) == block)
                                *firstp = block;
                }
        }
        return count;
}
#endif

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

        if (!dtfs_is_file(node) || dtfs_load(node) != 0)
                return -1;
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
        if (words > DTFS_LAST_BLOCK * DTFS_DATA_WORDS)
                return -1;
        new_blocks = words == 0U ? (int)mapoff :
            (int)((words + DTFS_DATA_WORDS - 1U) / DTFS_DATA_WORDS);
        last_words = new_blocks == 0U ? 0U :
            words - (new_blocks - 1U) * DTFS_DATA_WORDS;

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

static int
dtfs_detect_unit(unsigned int unit, unsigned int type, int deep)
{
#if !DTFS_ENABLE_TENEX && !DTFS_ENABLE_ITS
        (void)deep;
        if (type != SYS_DTFS_TYPE_AUTO && type != SYS_DTFS_TYPE_NATIVE)
                return -1;
        if (dtfs_dtc_read(unit, DTFS_DIR_BLOCK, dtfs_dir) != 0 ||
            !dtfs_native_valid())
                return -1;
        return SYS_DTFS_TYPE_NATIVE;
#else
        /* TYPE is always masked by both private callers. */
#if DTFS_ENABLE_TENEX
        if (type != SYS_DTFS_TYPE_ITS) {
#else
        if (type != SYS_DTFS_TYPE_ITS) {
#endif
                if (dtfs_dtc_read(unit, DTFS_DIR_BLOCK, dtfs_dir) == 0) {
                        if (type != SYS_DTFS_TYPE_TENEX &&
                            dtfs_native_valid())
                                return SYS_DTFS_TYPE_NATIVE;
#if DTFS_ENABLE_TENEX
                        if (type != SYS_DTFS_TYPE_NATIVE &&
                            dtfs_tenex_valid(unit, deep))
                                return SYS_DTFS_TYPE_TENEX;
#endif
                }
                if (type != SYS_DTFS_TYPE_AUTO)
                        return -1;
        }
#if DTFS_ENABLE_ITS
        if (dtfs_dtc_read(unit, DTFS_ITS_DIR_BLOCK, dtfs_dir) == 0 &&
            dtfs_its_valid())
                return SYS_DTFS_TYPE_ITS;
#endif
        return -1;
#endif
}


int
dtfs_format_unit(unsigned int unit, unsigned int ctl)
{
        unsigned int op;
        unsigned int type;

        op = ctl & 07U;
        type = ctl & SYS_DTFS_TYPE_MASK;
        if (unit > 7U || op > SYS_DTFS_CTL_CHECK)
                return -1;
        if (op == SYS_DTFS_CTL_CHECK)
                return dtfs_detect_unit(unit, type, 1);
        if (type == SYS_DTFS_TYPE_NATIVE) {
                fs_zero_words(dtfs_dir, DTFS_BLOCK_WORDS);
                /* Fixed native map markers: entries 0, 100, and 578..580. */
                dtfs_dir[0] = (kword_t)DTFS_OWNER_RESERVED << 31U;
                dtfs_dir[14] = (kword_t)DTFS_OWNER_RESERVED << 21U;
                dtfs_dir[82] = ((kword_t)DTFS_OWNER_NATIVE_TAG << 11U) |
                    ((kword_t)DTFS_OWNER_NATIVE_TAG << 6U) |
                    ((kword_t)DTFS_OWNER_NATIVE_TAG << 1U);
                dtfs_dir[DTFS_MAGIC_WORD] = DTFS_NATIVE_MAGIC;
#if DTFS_ENABLE_TENEX
        } else if (type == SYS_DTFS_TYPE_TENEX) {
                fs_zero_words(dtfs_dir, DTFS_BLOCK_WORDS);
                dtfs_dir[0] = ((kword_t)DTFS_TENEX_RESERVED << 31U) |
                    ((kword_t)DTFS_TENEX_RESERVED << 26U);
                dtfs_dir[14] = (kword_t)DTFS_TENEX_RESERVED << 26U;
                dtfs_dir[82] = ((kword_t)DTFS_TENEX_INVALID << 16U) |
                    ((kword_t)DTFS_TENEX_INVALID << 11U) |
                    ((kword_t)DTFS_TENEX_INVALID << 6U) |
                    ((kword_t)DTFS_TENEX_INVALID << 1U);
#endif
#if DTFS_ENABLE_ITS
        } else if (type == SYS_DTFS_TYPE_ITS) {
                fs_zero_words(dtfs_dir, DTFS_BLOCK_WORDS);
                dtfs_dir[DTFS_ITS_MAP_FIRST] = DTFS_ITS_MAP_RESERVED;
                dtfs_dir[DTFS_ITS_MAP_DIR] = DTFS_ITS_MAP_DIRWORD;
                dtfs_dir[DTFS_ITS_MAP_LAST] = DTFS_ITS_MAP_END;
                dtfs_cache_mount = 0U;
                return dtfs_dtc_write(unit, DTFS_ITS_DIR_BLOCK,
                    dtfs_dir);
#endif
        } else {
                return -1;
        }
        dtfs_cache_mount = 0U;
        return dtfs_dtc_write(unit, DTFS_DIR_BLOCK, dtfs_dir);
}

int
dtfs_mount_unit(unsigned int unit, vnode_t target,
    unsigned int flags, vnode_t *rootp)
{
        vnode_t root;
        unsigned int format;
        unsigned int media;

        if (unit > 7U || rootp == 0 ||
            (flags & ~(VFS_MOUNT_RDONLY | SYS_DTFS_TYPE_MASK)) != 0U)
                return -1;
        format = dtfs_detect_unit(unit, flags & SYS_DTFS_TYPE_MASK, 0);
        if ((int)format < 0)
                return -1;
#if !DTFS_ENABLE_TENEX && !DTFS_ENABLE_ITS
        media = unit;
#else
        media = unit | (format - SYS_DTFS_TYPE_NATIVE);
#endif
        if (vfs_mount(target, DTFS_PROVIDER, DTFS_KIND_ROOT, 0U,
            flags & VFS_MOUNT_RDONLY, &root) != 0)
                return -1;
        format = VFS_MOUNT_ID(root);
        dtfs_patch_media(format, media);
        dtfs_cache_mount = format;
        *rootp = root;
        return 0;
}

extern int dtfs_lookup(vnode_t dir, const struct vfs_name *name,
    vnode_t *nodep);

extern int dtfs_readdir(vnode_t dir, unsigned int off,
    struct vfs_dirent *ent);

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
                st->size_chars = 0;
                st->size_words = 0;
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
        st->size_chars = (kword_t)words * 4U;
        return 0;
}

extern int dtfs_parent(vnode_t node, vnode_t *parentp);


int
dtfs_create(vnode_t dir, const struct vfs_name *name,
    unsigned int mode, vnode_t *nodep)
{
        unsigned int slot;
        unsigned int personality;

        if (!dtfs_is_root(dir) || nodep == 0 || dtfs_load(dir) != 0 ||
            dtfs_scan_slot(dir, name, 0) == 0)
                return -1;
        personality = dtfs_personality(dir);
        if (dtfs_scan_slot(dir, 0, &slot) != 0)
                return -1;
        if (personality == DTFS_MEDIA_TENEX) {
                unsigned int block;

                if (dtfs_find_free_block(1U, 1, &block) != 0 ||
                    dtfs_foreign_set_name(slot, name, 0) != 0)
                        return -1;
                fs_zero_block_workspace();
                dtfs_block[0] = DTFS_HEADER(0U, block, 0U);
                if (dtfs_dtc_write(dtfs_unit(dir), block, dtfs_block) != 0) {
                        dtfs_dir[DTFS_NAME_BASE + slot] = 0UL;
                        dtfs_dir[DTFS_TENEX_EXT_BASE + slot] = 0UL;
                        return -1;
                }
                dtfs_set_owner(0U, block - 1U, slot + 1U);
                if (dtfs_commit(dir) != 0) {
                        dtfs_set_owner(0U, block - 1U, DTFS_OWNER_FREE);
                        dtfs_dir[DTFS_NAME_BASE + slot] = 0UL;
                        dtfs_dir[DTFS_TENEX_EXT_BASE + slot] = 0UL;
                        return -1;
                }
        } else if (personality == DTFS_MEDIA_ITS) {
                if (dtfs_foreign_set_name(slot, name, 1) != 0)
                        return -1;
                if (dtfs_commit(dir) != 0) {
                        dtfs_dir[slot * 2U] = 0UL;
                        dtfs_dir[slot * 2U + 1U] = 0UL;
                        return -1;
                }
        } else {
                dtfs_set_name(slot, name);
                dtfs_set_last_words(slot, 0U);
                dtfs_set_exec(slot, (mode & 0111U) != 0U);
                if (dtfs_commit(dir) != 0) {
                        dtfs_clear_slot(slot);
                        return -1;
                }
        }
        *nodep = VFS_NODE_PACKED(DTFS_PROVIDER,
            (VFS_MOUNT_ID(dir) << VFS_MOUNT_SHIFT) | DTFS_KIND_FILE, slot);
        return 0;
}

int
dtfs_unlink(vnode_t dir, const struct vfs_name *name)
{
        unsigned int slot;
        unsigned int block;
        unsigned int next;
        unsigned int blocks;
        unsigned int i;
        unsigned int personality;
        vnode_t node;

        if (!dtfs_is_root(dir) || dtfs_load(dir) != 0 ||
            dtfs_scan_slot(dir, name, &slot) != 0)
                return -1;
        personality = dtfs_personality(dir);
        node = VFS_NODE_PACKED(DTFS_PROVIDER,
            (VFS_MOUNT_ID(dir) << VFS_MOUNT_SHIFT) | DTFS_KIND_FILE, slot);
        if (personality == DTFS_MEDIA_TENEX) {
                blocks = dtfs_block_info(node, slot, &block);
                if (blocks == 0U || block == 0U)
                        return -1;
                next = block;
                for (i = 0U; i != blocks; ++i) {
                        block = next;
                        if (block == 0U || block > DTFS_LAST_BLOCK ||
                            dtfs_owner(0U, block - 1U) != slot + 1U ||
                            dtfs_dtc_read(dtfs_unit(dir), block,
                            dtfs_block) != 0) {
                                dtfs_cache_mount = 0U;
                                return -1;
                        }
                        next = DTFS_HDR_NEXT(dtfs_block[0]);
                        dtfs_set_owner(0U, block - 1U, DTFS_OWNER_FREE);
                }
                if (next != 0U) {
                        dtfs_cache_mount = 0U;
                        return -1;
                }
                dtfs_dir[DTFS_NAME_BASE + slot] = 0UL;
                dtfs_dir[DTFS_TENEX_EXT_BASE + slot] = 0UL;
                if (dtfs_commit(dir) != 0) {
                        dtfs_cache_mount = 0U;
                        return -1;
                }
                return 0;
        }
        if (dtfs_resize(node, 0U) != 0)
                return -1;
        if (personality == DTFS_MEDIA_ITS) {
                dtfs_dir[slot * 2U] = 0UL;
                dtfs_dir[slot * 2U + 1U] = 0UL;
        } else {
                dtfs_clear_slot(slot);
        }
        return dtfs_commit(dir);
}

int
dtfs_rename(vnode_t olddir, const struct vfs_name *oldname,
    vnode_t newdir, const struct vfs_name *newname)
{
        unsigned int slot;
        unsigned int personality;

        if (!dtfs_is_root(olddir) || !dtfs_is_root(newdir) ||
            VFS_MOUNT_ID(olddir) != VFS_MOUNT_ID(newdir) ||
            dtfs_load(olddir) != 0 ||
            dtfs_scan_slot(olddir, oldname, &slot) != 0 ||
            dtfs_scan_slot(olddir, newname, 0) == 0)
                return -1;
        personality = dtfs_personality(olddir);
        if (personality != 0U) {
                if (dtfs_foreign_set_name(slot, newname,
                    personality == DTFS_MEDIA_ITS) != 0)
                        return -1;
        } else {
                dtfs_set_name(slot, newname);
        }
        return dtfs_commit(olddir);
}

int
dtfs_truncate(vnode_t node, unsigned int words, kword_t size_chars)
{
        (void)size_chars;
        return dtfs_resize(node, words);
}

extern int dtfs_chmod(vnode_t node, unsigned int mode);

#if !DTFS_ENABLE_TENEX && !DTFS_ENABLE_ITS
int
dtfs_chain_walk(unsigned int unit, unsigned int slot, unsigned int off,
    kword_t *buf, unsigned int nwords, unsigned int mapoff, int writing)
{
        unsigned int blocks;
        unsigned int first;
        unsigned int block;
        unsigned int count;
        unsigned int take;
        unsigned int done;
        unsigned int seen;
        unsigned int words;

        (void)mapoff;
        blocks = 0U;
        first = 0U;
        for (block = 1U; block != DTFS_LAST_BLOCK + 1U; ++block) {
                if (dtfs_owner(0U, block) != slot + 1U)
                        continue;
                ++blocks;
                if (first == 0U &&
                    dtfs_dtc_read(unit, block, dtfs_block) == 0 &&
                    ((dtfs_block[0] >> DTFS_FIRST_SHIFT) &
                    DTFS_BLOCKNO_MASK) == block)
                        first = block;
        }
        if (first == 0U)
                return blocks == 0U ? 0 : -1;
        block = first;
        done = 0U;
        words = 0U;
        seen = 0U;
        for (;;) {
                if (block == 0U || block > DTFS_LAST_BLOCK ||
                    dtfs_owner(0U, block) != slot + 1U ||
                    dtfs_dtc_read(unit, block, dtfs_block) != 0 ||
                    ((dtfs_block[0] >> DTFS_FIRST_SHIFT) &
                    DTFS_BLOCKNO_MASK) != first)
                        return -1;
                count = (unsigned int)(dtfs_block[0] & DTFS_COUNT_MASK);
                if (count > DTFS_DATA_WORDS)
                        return -1;
                words += count;
                if (buf != 0) {
                        if (off < count) {
                                take = count - off;
                                if (take > nwords - done)
                                        take = nwords - done;
                                if (writing) {
                                        fs_copy_words(&buf[done],
                                            &dtfs_block[1U + off], take);
                                        if (dtfs_dtc_write(unit, block,
                                            dtfs_block) != 0)
                                                return -1;
                                } else {
                                        fs_copy_words(&dtfs_block[1U + off],
                                            &buf[done], take);
                                }
                                done += take;
                                off = 0U;
                                if (done == nwords)
                                        return (int)done;
                        } else {
                                off -= count;
                        }
                }
                block = DTFS_HDR_NEXT(dtfs_block[0]);
                ++seen;
                if (block == 0U) {
                        if (seen != blocks)
                                return -1;
                        return buf != 0 ? (int)done : (int)words;
                }
                if (count == 0U || seen == blocks)
                        return -1;
        }
}
#else
int
dtfs_chain_walk(unsigned int unit, unsigned int slot, unsigned int off,
    kword_t *buf, unsigned int nwords, unsigned int mapoff, int writing)
{
        unsigned int blocks;
        unsigned int first;
        unsigned int block;
        unsigned int count;
        unsigned int take;
        unsigned int done;
        unsigned int seen;
        unsigned int words;
        unsigned int data_base;
        unsigned int owner;
        unsigned int last_block;
        int its;

        owner = slot + 1U;
        its = mapoff == DTFS_ITS_NAME_WORDS;
        if (its) {
                done = 0U;
                block = 1U;
                last_block = writing ? DTFS_ITS_END_BLOCK + 1U :
                    DTFS_ITS_MAP_ENTRIES + 1U;
                goto its_scan;
        }

        blocks = 0U;
        first = 0U;
        for (block = 1U; block != DTFS_LAST_BLOCK + 1U; ++block) {
                if (dtfs_owner(0U, block - mapoff) != owner)
                        continue;
                ++blocks;
                if (first == 0U &&
                    dtfs_dtc_read(unit, block, dtfs_block) == 0 &&
                    ((dtfs_block[0] >> DTFS_FIRST_SHIFT) &
                    DTFS_BLOCKNO_MASK) == block)
                        first = block;
        }
        if (first == 0U)
                return mapoff == 0U && blocks == 0U ? 0 : -1;
        block = first;
        done = 0U;
        words = 0U;
        seen = 0U;
chain_scan:
        if (block == 0U || block > DTFS_LAST_BLOCK ||
            dtfs_owner(0U, block - mapoff) != owner ||
            dtfs_dtc_read(unit, block, dtfs_block) != 0 ||
            ((dtfs_block[0] >> DTFS_FIRST_SHIFT) &
            DTFS_BLOCKNO_MASK) != first)
                return -1;
        count = (unsigned int)(dtfs_block[0] & DTFS_COUNT_MASK);
        if (count > DTFS_DATA_WORDS)
                return -1;
        words += count;
        if (buf == 0)
                goto chain_next;
        data_base = 1U;
        goto transfer_block;

its_scan:
        if (block == last_block)
                return writing ? -1 : (int)done;
        count = dtfs_owner(DTFS_ITS_NAME_WORDS, block - 1U);
        if (!writing && count == DTFS_ITS_END)
                return (int)done;
        if (count != owner) {
                ++block;
                goto its_scan;
        }
        count = DTFS_BLOCK_WORDS;
        if (off >= count) {
                off -= count;
                ++block;
                goto its_scan;
        }
        if (dtfs_dtc_read(unit, block, dtfs_block) != 0)
                return -1;
        data_base = 0U;

transfer_block:
        if (off < count) {
                take = count - off;
                if (take > nwords - done)
                        take = nwords - done;
                if (writing) {
                        fs_copy_words(&buf[done],
                            &dtfs_block[data_base + off], take);
                        if (dtfs_dtc_write(unit, block, dtfs_block) != 0)
                                return -1;
                } else {
                        fs_copy_words(&dtfs_block[data_base + off],
                            &buf[done], take);
                }
                done += take;
                off = 0U;
                if (done == nwords)
                        return (int)done;
        } else {
                off -= count;
        }
        if (its) {
                ++block;
                goto its_scan;
        }

chain_next:
        block = DTFS_HDR_NEXT(dtfs_block[0]);
        ++seen;
        if (block == 0U) {
                if (seen != blocks)
                        return -1;
                return buf != 0 ? (int)done : (int)words;
        }
        if (count == 0U || seen == blocks)
                return -1;
        goto chain_scan;
}

#endif

static int
dtfs_transfer_words(vnode_t node, unsigned int off, kword_t *buf,
    unsigned int nwords, int writing)
{
        unsigned int size;
        unsigned int need;
        unsigned int personality;
        unsigned int slot;

        if (!dtfs_is_file(node) || buf == 0 || dtfs_load(node) != 0)
                return -1;
        personality = dtfs_personality(node);
        slot = VFS_INDEX(node);
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
                return dtfs_chain_walk(dtfs_unit(node), slot, off, buf,
                    nwords, DTFS_ITS_NAME_WORDS, writing);
        }
#endif
        if (!writing) {
                if (personality == DTFS_MEDIA_TENEX &&
                    (slot >= DTFS_FILE_SLOTS ||
                    dtfs_dir[DTFS_NAME_BASE + slot] == 0UL))
                        return -1;
                if (nwords == 0U)
                        return 0;
                return dtfs_chain_walk(dtfs_unit(node), slot, off, buf,
                    nwords, personality == DTFS_MEDIA_TENEX, 0);
        }
        size = dtfs_size_words(node, slot);
        need = off + nwords;
        if (need > size && dtfs_resize(node, need) != 0)
                return -1;
        return dtfs_chain_walk(dtfs_unit(node), slot, off, buf,
            nwords, personality == DTFS_MEDIA_TENEX, 1);
}

int
dtfs_read_words(vnode_t node, unsigned int off, kword_t *buf,
    unsigned int nwords)
{
        return dtfs_transfer_words(node, off, buf, nwords, 0);
}

int
dtfs_write_words(vnode_t node, unsigned int off,
    const kword_t *buf, unsigned int nwords, kword_t size_chars)
{
        (void)size_chars;
        return dtfs_transfer_words(node, off, (kword_t *)buf,
            nwords, 1);
}

extern int dtfs_sync(vnode_t node);
