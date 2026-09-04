#include "dtfs.h"
#include "fs_mres.h"

#define DTFS_BLOCK_WORDS      0200U
#define DTFS_BLOCKS           01102U
#define DTFS_LAST_BLOCK       01101U
#define DTFS_DIR_BLOCK        0144U       /* decimal 100 */
#define DTFS_MAP_WORDS        0123U       /* decimal 83 */
#define DTFS_NAME_BASE        0123U
#define DTFS_MAGIC_WORD       0177U
#define DTFS_DATA_WORDS       0177U

#define DTFS_OWNER_FREE       000U
#define DTFS_OWNER_NATIVE_TAG 035U
#define DTFS_OWNER_RESERVED   036U
#define DTFS_OWNER_INVALID    037U
#define DTFS_NATIVE_MAGIC     0446446632021UL

#define DTFS_NEXT_SHIFT       18U
#define DTFS_FIRST_SHIFT      8U
#define DTFS_BLOCKNO_MASK     01777UL
#define DTFS_COUNT_MASK       0377UL
#define DTFS_NAME2_MASK       0777777777700UL

unsigned int dtfs_dtc_read_addr;
unsigned int dtfs_dtc_write_addr;

/* One directory and one transfer block are shared by every DTFS mount. */
static kword_t dtfs_dir[DTFS_BLOCK_WORDS];
#define dtfs_block fs_block_workspace
static unsigned int dtfs_cache_mount;
static unsigned int dtfs_units[VFS_NMOUNT];

static int
dtfs_is_root(vnode_t node)
{
        return VFS_PROVIDER(node) == DTFS_PROVIDER &&
            VFS_LOCAL_KIND(node) == DTFS_KIND_ROOT &&
            VFS_MOUNT_ID(node) != 0U;
}

static int
dtfs_is_file(vnode_t node)
{
        return VFS_PROVIDER(node) == DTFS_PROVIDER &&
            VFS_LOCAL_KIND(node) == DTFS_KIND_FILE &&
            VFS_MOUNT_ID(node) != 0U &&
            VFS_INDEX(node) < DTFS_FILE_SLOTS;
}

static unsigned int
dtfs_owner(unsigned int block)
{
        unsigned int wi;
        unsigned int shift;

        wi = block / 7U;
        shift = 31U - (block % 7U) * 5U;
        return (unsigned int)((dtfs_dir[wi] >> shift) & 037UL);
}

static void
dtfs_set_owner(unsigned int block, unsigned int owner)
{
        unsigned int wi;
        unsigned int shift;
        kword_t mask;

        wi = block / 7U;
        shift = 31U - (block % 7U) * 5U;
        mask = (kword_t)037UL << shift;
        dtfs_dir[wi] = (dtfs_dir[wi] & ~mask) |
            ((kword_t)(owner & 037U) << shift);
}

static int
dtfs_dtc_read(unsigned int unit, unsigned int block, kword_t *buf)
{
        return dtfs_dtc_call(dtfs_dtc_read_addr, unit, block, buf);
}

static int
dtfs_dtc_write(unsigned int unit, unsigned int block, kword_t *buf)
{
        return dtfs_dtc_call(dtfs_dtc_write_addr, unit, block, buf);
}

static int
dtfs_native_valid(void)
{
        unsigned int block;

        if (dtfs_dir[DTFS_MAGIC_WORD] != DTFS_NATIVE_MAGIC)
                return 0;
        if (dtfs_owner(0U) != DTFS_OWNER_RESERVED ||
            dtfs_owner(DTFS_DIR_BLOCK) != DTFS_OWNER_RESERVED)
                return 0;
        for (block = DTFS_BLOCKS; block <= 01104U; ++block)
                if (dtfs_owner(block) != DTFS_OWNER_NATIVE_TAG)
                        return 0;
        return 1;
}

static int
dtfs_load(vnode_t node)
{
        unsigned int id;
        unsigned int unit;

        id = VFS_MOUNT_ID(node);
        if (id == 0U || id > VFS_NMOUNT)
                return -1;
        if (dtfs_cache_mount == id)
                return 0;
        unit = dtfs_units[id - 1U];
        if (dtfs_dtc_read(unit, DTFS_DIR_BLOCK, dtfs_dir) != 0 ||
            !dtfs_native_valid())
                return -1;
        dtfs_cache_mount = id;
        return 0;
}

static int
dtfs_commit(vnode_t node)
{
        unsigned int id;

        id = VFS_MOUNT_ID(node);
        if (id == 0U || id > VFS_NMOUNT || dtfs_cache_mount != id)
                return -1;
        return dtfs_dtc_write(dtfs_units[id - 1U],
            DTFS_DIR_BLOCK, dtfs_dir);
}

static int
dtfs_name_valid(const struct vfs_name *name)
{
        return vfs_name_valid(name) &&
            name->chars <= DTFS_NAME_MAX_CHARS &&
            name->words[2] == 0 && name->words[3] == 0;
}

static int
dtfs_slot_name_eq(unsigned int slot, const struct vfs_name *name)
{
        unsigned int base;

        if (!dtfs_name_valid(name))
                return 0;
        base = DTFS_NAME_BASE + slot * 2U;
        return dtfs_dir[base] == name->words[0] &&
            (dtfs_dir[base + 1U] & DTFS_NAME2_MASK) ==
            (name->words[1] & DTFS_NAME2_MASK);
}

static int
dtfs_find(const struct vfs_name *name, unsigned int *slotp)
{
        unsigned int slot;

        for (slot = 0U; slot < DTFS_FILE_SLOTS; ++slot) {
                if (dtfs_dir[DTFS_NAME_BASE + slot * 2U] != 0 &&
                    dtfs_slot_name_eq(slot, name)) {
                        if (slotp != 0)
                                *slotp = slot;
                        return 0;
                }
        }
        return -1;
}

static int
dtfs_free_slot(unsigned int *slotp)
{
        unsigned int slot;

        for (slot = 0U; slot < DTFS_FILE_SLOTS; ++slot) {
                if (dtfs_dir[DTFS_NAME_BASE + slot * 2U] == 0) {
                        *slotp = slot;
                        return 0;
                }
        }
        return -1;
}

static void
dtfs_set_name(unsigned int slot, const struct vfs_name *name)
{
        unsigned int base;
        kword_t last;

        base = DTFS_NAME_BASE + slot * 2U;
        last = dtfs_dir[base + 1U] & 077UL;
        dtfs_dir[base] = name->words[0];
        dtfs_dir[base + 1U] = (name->words[1] & DTFS_NAME2_MASK) | last;
}

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

static unsigned int
dtfs_last_words(unsigned int slot)
{
        unsigned int low;
        unsigned int high;

        low = (unsigned int)(dtfs_dir[DTFS_NAME_BASE + slot * 2U + 1U] &
            077UL);
        high = (dtfs_dir[22U + slot] & 1UL) != 0 ? 0100U : 0U;
        return low | high;
}

static void
dtfs_set_last_words(unsigned int slot, unsigned int words)
{
        unsigned int wi;

        wi = DTFS_NAME_BASE + slot * 2U + 1U;
        dtfs_dir[wi] = (dtfs_dir[wi] & ~077UL) | (words & 077U);
        if ((words & 0100U) != 0)
                dtfs_dir[22U + slot] |= 1UL;
        else
                dtfs_dir[22U + slot] &= ~1UL;
}

static unsigned int
dtfs_exec(unsigned int slot)
{
        return (dtfs_dir[slot] & 1UL) != 0;
}

static void
dtfs_set_exec(unsigned int slot, int executable)
{
        if (executable)
                dtfs_dir[slot] |= 1UL;
        else
                dtfs_dir[slot] &= ~1UL;
}

static unsigned int
dtfs_hdr_next(kword_t h)
{
        return (unsigned int)((h >> DTFS_NEXT_SHIFT) & DTFS_BLOCKNO_MASK);
}

static unsigned int
dtfs_hdr_first(kword_t h)
{
        return (unsigned int)((h >> DTFS_FIRST_SHIFT) & DTFS_BLOCKNO_MASK);
}

static kword_t
dtfs_header(unsigned int next, unsigned int first, unsigned int count)
{
        return ((kword_t)(next & DTFS_BLOCKNO_MASK) << DTFS_NEXT_SHIFT) |
            ((kword_t)(first & DTFS_BLOCKNO_MASK) << DTFS_FIRST_SHIFT) |
            (kword_t)(count & DTFS_COUNT_MASK);
}

static unsigned int
dtfs_block_count(unsigned int slot)
{
        unsigned int block;
        unsigned int owner;
        unsigned int count;

        owner = slot + 1U;
        count = 0U;
        for (block = 1U; block <= DTFS_LAST_BLOCK; ++block)
                if (dtfs_owner(block) == owner)
                        ++count;
        return count;
}

static unsigned int
dtfs_size_words(unsigned int slot)
{
        unsigned int blocks;
        unsigned int last;

        blocks = dtfs_block_count(slot);
        if (blocks == 0U)
                return 0U;
        last = dtfs_last_words(slot);
        if (last == 0U || last > DTFS_DATA_WORDS)
                return 0U;
        return (blocks - 1U) * DTFS_DATA_WORDS + last;
}

static int
dtfs_first_block(unsigned int mount, unsigned int slot,
    unsigned int *blockp)
{
        unsigned int block;
        unsigned int owner;
        unsigned int unit;

        owner = slot + 1U;
        unit = dtfs_units[mount - 1U];
        for (block = 1U; block <= DTFS_LAST_BLOCK; ++block) {
                if (dtfs_owner(block) != owner)
                        continue;
                if (dtfs_dtc_read(unit, block, dtfs_block) != 0)
                        return -1;
                if (dtfs_hdr_first(dtfs_block[0]) == block) {
                        *blockp = block;
                        return 0;
                }
        }
        return -1;
}

static int
dtfs_find_free_block(unsigned int start, unsigned int *blockp)
{
        unsigned int n;
        unsigned int block;

        /* start==1 is the empty-file seed: allocate next to metadata. */
        if (start <= 1U || start > DTFS_LAST_BLOCK)
                start = DTFS_DIR_BLOCK + 1U;
        block = start;
        for (n = 0U; n < DTFS_LAST_BLOCK; ++n) {
                if (dtfs_owner(block) == DTFS_OWNER_FREE) {
                        *blockp = block;
                        return 0;
                }
                ++block;
                if (block > DTFS_LAST_BLOCK)
                        block = 1U;
        }
        return -1;
}

static int
dtfs_locate(vnode_t node, unsigned int file_block,
    unsigned int *physicalp)
{
        unsigned int mount;
        unsigned int slot;
        unsigned int block;
        unsigned int i;
        unsigned int unit;

        mount = VFS_MOUNT_ID(node);
        slot = VFS_INDEX(node);
        if (dtfs_first_block(mount, slot, &block) != 0)
                return -1;
        unit = dtfs_units[mount - 1U];
        for (i = 0U; i < file_block; ++i) {
                if (dtfs_dtc_read(unit, block, dtfs_block) != 0)
                        return -1;
                block = dtfs_hdr_next(dtfs_block[0]);
                if (block == 0U || block > DTFS_LAST_BLOCK)
                        return -1;
        }
        *physicalp = block;
        return 0;
}

static int
dtfs_resize(vnode_t node, unsigned int words)
{
        unsigned int mount;
        unsigned int slot;
        unsigned int old_blocks;
        unsigned int new_blocks;
        unsigned int first;
        unsigned int prev;
        unsigned int block;
        unsigned int next;
        unsigned int i;
        unsigned int unit;
        unsigned int owner;
        unsigned int count;

        if (!dtfs_is_file(node) || dtfs_load(node) != 0)
                return -1;
        mount = VFS_MOUNT_ID(node);
        slot = VFS_INDEX(node);
        unit = dtfs_units[mount - 1U];
        owner = slot + 1U;
        old_blocks = dtfs_block_count(slot);
        new_blocks = words == 0U ? 0U :
            (words + DTFS_DATA_WORDS - 1U) / DTFS_DATA_WORDS;
        if (new_blocks > DTFS_LAST_BLOCK)
                return -1;

        if (old_blocks == 0U) {
                first = 0U;
                prev = 0U;
        } else {
                if (dtfs_first_block(mount, slot, &first) != 0)
                        return -1;
                prev = first;
                for (i = 1U; i < old_blocks; ++i) {
                        if (dtfs_dtc_read(unit, prev, dtfs_block) != 0)
                                return -1;
                        next = dtfs_hdr_next(dtfs_block[0]);
                        if (next == 0U || next > DTFS_LAST_BLOCK)
                                return -1;
                        prev = next;
                }
        }

        while (old_blocks < new_blocks) {
                if (dtfs_find_free_block(prev + 1U, &block) != 0)
                        return -1;
                if (first == 0U)
                        first = block;
                fs_zero_block_workspace();
                count = (old_blocks + 1U == new_blocks) ?
                    words - old_blocks * DTFS_DATA_WORDS :
                    DTFS_DATA_WORDS;
                dtfs_block[0] = dtfs_header(0U, first, count);
                if (dtfs_dtc_write(unit, block, dtfs_block) != 0)
                        return -1;
                dtfs_set_owner(block, owner);
                if (prev != 0U) {
                        if (dtfs_dtc_read(unit, prev, dtfs_block) != 0)
                                return -1;
                        dtfs_block[0] = dtfs_header(block, first,
                            DTFS_DATA_WORDS);
                        if (dtfs_dtc_write(unit, prev, dtfs_block) != 0)
                                return -1;
                }
                prev = block;
                ++old_blocks;
        }

        if (new_blocks < old_blocks) {
                if (new_blocks == 0U) {
                        block = first;
                } else {
                        block = first;
                        for (i = 1U; i < new_blocks; ++i) {
                                if (dtfs_dtc_read(unit, block,
                                    dtfs_block) != 0)
                                        return -1;
                                block = dtfs_hdr_next(dtfs_block[0]);
                        }
                        if (dtfs_dtc_read(unit, block, dtfs_block) != 0)
                                return -1;
                        next = dtfs_hdr_next(dtfs_block[0]);
                        count = words - (new_blocks - 1U) * DTFS_DATA_WORDS;
                        dtfs_block[0] = dtfs_header(0U, first, count);
                        if (dtfs_dtc_write(unit, block, dtfs_block) != 0)
                                return -1;
                        block = next;
                }
                while (block != 0U) {
                        if (block > DTFS_LAST_BLOCK ||
                            dtfs_dtc_read(unit, block, dtfs_block) != 0)
                                return -1;
                        next = dtfs_hdr_next(dtfs_block[0]);
                        dtfs_set_owner(block, DTFS_OWNER_FREE);
                        block = next;
                }
        } else if (new_blocks != 0U) {
                if (dtfs_locate(node, new_blocks - 1U, &block) != 0 ||
                    dtfs_dtc_read(unit, block, dtfs_block) != 0)
                        return -1;
                count = words - (new_blocks - 1U) * DTFS_DATA_WORDS;
                dtfs_block[0] = dtfs_header(0U, first, count);
                if (dtfs_dtc_write(unit, block, dtfs_block) != 0)
                        return -1;
        }
        dtfs_set_last_words(slot, new_blocks == 0U ? 0U :
            words - (new_blocks - 1U) * DTFS_DATA_WORDS);
        return dtfs_commit(node);
}

int
dtfs_format_unit(unsigned int unit)
{
        unsigned int i;

        if (unit > 7U || dtfs_dtc_write_addr == 0U)
                return -1;
        for (i = 0U; i < DTFS_BLOCK_WORDS; ++i)
                dtfs_dir[i] = 0;
        dtfs_set_owner(0U, DTFS_OWNER_RESERVED);
        dtfs_set_owner(DTFS_DIR_BLOCK, DTFS_OWNER_RESERVED);
        for (i = DTFS_BLOCKS; i <= 01104U; ++i)
                dtfs_set_owner(i, DTFS_OWNER_NATIVE_TAG);
        dtfs_dir[DTFS_MAGIC_WORD] = DTFS_NATIVE_MAGIC;
        dtfs_cache_mount = 0U;
        return dtfs_dtc_write(unit, DTFS_DIR_BLOCK, dtfs_dir);
}

int
dtfs_mount_unit(unsigned int unit, vnode_t target,
    unsigned int flags, vnode_t *rootp)
{
        vnode_t root;
        unsigned int id;

        if (unit > 7U || rootp == 0 || dtfs_dtc_read_addr == 0U ||
            dtfs_dtc_read(unit, DTFS_DIR_BLOCK, dtfs_dir) != 0 ||
            !dtfs_native_valid() ||
            vfs_mount(target, DTFS_PROVIDER, DTFS_KIND_ROOT, 0U,
            flags, &root) != 0)
                return -1;
        id = VFS_MOUNT_ID(root);
        dtfs_units[id - 1U] = unit;
        dtfs_cache_mount = id;
        *rootp = root;
        return 0;
}

int
dtfs_lookup(vnode_t dir, const struct vfs_name *name,
    vnode_t *nodep)
{
        unsigned int slot;

        if (!dtfs_is_root(dir) || nodep == 0 || dtfs_load(dir) != 0 ||
            dtfs_find(name, &slot) != 0)
                return -1;
        *nodep = VFS_NODE(DTFS_PROVIDER,
            VFS_MOUNT_KIND(VFS_MOUNT_ID(dir), DTFS_KIND_FILE), slot);
        return 0;
}

int
dtfs_readdir(vnode_t dir, unsigned int off,
    struct vfs_dirent *ent)
{
        unsigned int slot;
        unsigned int seen;
        unsigned int base;

        if (!dtfs_is_root(dir) || ent == 0 || dtfs_load(dir) != 0)
                return -1;
        seen = 0U;
        for (slot = 0U; slot < DTFS_FILE_SLOTS; ++slot) {
                base = DTFS_NAME_BASE + slot * 2U;
                if (dtfs_dir[base] == 0)
                        continue;
                if (seen++ != off)
                        continue;
                ent->name.words[0] = dtfs_dir[base];
                ent->name.words[1] = dtfs_dir[base + 1U] &
                    DTFS_NAME2_MASK;
                ent->name.words[2] = 0;
                ent->name.words[3] = 0;
                ent->name.chars = vfs_sixbit_name_chars(ent->name.words,
                    DTFS_NAME_MAX_CHARS);
                ent->type = VFS_TYPE_REG;
                return 1;
        }
        return 0;
}

int
dtfs_stat(vnode_t node, struct vfs_stat *st)
{
        unsigned int slot;
        unsigned int words;

        if (st == 0 || dtfs_load(node) != 0)
                return -1;
        if (dtfs_is_root(node)) {
                st->type = VFS_TYPE_DIR;
                st->mode = 0777U;
                st->size_chars = 0;
                st->size_words = 0;
                return 0;
        }
        if (!dtfs_is_file(node))
                return -1;
        slot = VFS_INDEX(node);
        if (dtfs_dir[DTFS_NAME_BASE + slot * 2U] == 0)
                return -1;
        words = dtfs_size_words(slot);
        st->type = VFS_TYPE_REG;
        st->mode = 0666U | (dtfs_exec(slot) ? 0111U : 0U);
        st->size_words = words;
        st->size_chars = (kword_t)words * 4U;
        return 0;
}

int
dtfs_parent(vnode_t node, vnode_t *parentp)
{
        if (parentp == 0 || !dtfs_is_file(node))
                return -1;
        *parentp = VFS_NODE(DTFS_PROVIDER,
            VFS_MOUNT_KIND(VFS_MOUNT_ID(node), DTFS_KIND_ROOT), 0U);
        return 0;
}

int
dtfs_create(vnode_t dir, const struct vfs_name *name,
    unsigned int mode, vnode_t *nodep)
{
        unsigned int slot;

        if (!dtfs_is_root(dir) || nodep == 0 || !dtfs_name_valid(name) ||
            dtfs_load(dir) != 0 || dtfs_find(name, 0) == 0 ||
            dtfs_free_slot(&slot) != 0)
                return -1;
        dtfs_set_name(slot, name);
        dtfs_set_last_words(slot, 0U);
        dtfs_set_exec(slot, (mode & 0111U) != 0U);
        if (dtfs_commit(dir) != 0) {
                dtfs_clear_slot(slot);
                return -1;
        }
        *nodep = VFS_NODE(DTFS_PROVIDER,
            VFS_MOUNT_KIND(VFS_MOUNT_ID(dir), DTFS_KIND_FILE), slot);
        return 0;
}

int
dtfs_unlink(vnode_t dir, const struct vfs_name *name)
{
        unsigned int slot;
        vnode_t node;

        if (!dtfs_is_root(dir) || dtfs_load(dir) != 0 ||
            dtfs_find(name, &slot) != 0)
                return -1;
        node = VFS_NODE(DTFS_PROVIDER,
            VFS_MOUNT_KIND(VFS_MOUNT_ID(dir), DTFS_KIND_FILE), slot);
        if (dtfs_resize(node, 0U) != 0)
                return -1;
        dtfs_clear_slot(slot);
        return dtfs_commit(dir);
}

int
dtfs_rename(vnode_t olddir, const struct vfs_name *oldname,
    vnode_t newdir, const struct vfs_name *newname)
{
        unsigned int slot;

        if (!dtfs_is_root(olddir) || !dtfs_is_root(newdir) ||
            VFS_MOUNT_ID(olddir) != VFS_MOUNT_ID(newdir) ||
            !dtfs_name_valid(newname) || dtfs_load(olddir) != 0 ||
            dtfs_find(oldname, &slot) != 0 || dtfs_find(newname, 0) == 0)
                return -1;
        dtfs_set_name(slot, newname);
        return dtfs_commit(olddir);
}

int
dtfs_truncate(vnode_t node, unsigned int words, kword_t size_chars)
{
        (void)size_chars;
        return dtfs_resize(node, words);
}

int
dtfs_chmod(vnode_t node, unsigned int mode)
{
        if (!dtfs_is_file(node) || dtfs_load(node) != 0)
                return -1;
        dtfs_set_exec(VFS_INDEX(node), (mode & 0111U) != 0U);
        return dtfs_commit(node);
}

int
dtfs_read_words(vnode_t node, unsigned int off, kword_t *buf,
    unsigned int nwords)
{
        unsigned int size;
        unsigned int done;
        unsigned int file_block;
        unsigned int in_block;
        unsigned int block;
        unsigned int take;
        unsigned int unit;

        if (!dtfs_is_file(node) || buf == 0 || dtfs_load(node) != 0)
                return -1;
        size = dtfs_size_words(VFS_INDEX(node));
        if (off >= size)
                return 0;
        if (nwords > size - off)
                nwords = size - off;
        unit = dtfs_units[VFS_MOUNT_ID(node) - 1U];
        done = 0U;
        while (done < nwords) {
                file_block = (off + done) / DTFS_DATA_WORDS;
                in_block = (off + done) % DTFS_DATA_WORDS;
                if (dtfs_locate(node, file_block, &block) != 0 ||
                    dtfs_dtc_read(unit, block, dtfs_block) != 0)
                        return -1;
                take = DTFS_DATA_WORDS - in_block;
                if (take > nwords - done)
                        take = nwords - done;
                fs_copy_words(&dtfs_block[1U + in_block],
                    &buf[done], take);
                done += take;
        }
        return (int)done;
}

int
dtfs_write_words(vnode_t node, unsigned int off,
    const kword_t *buf, unsigned int nwords, kword_t size_chars)
{
        unsigned int old_size;
        unsigned int need;
        unsigned int done;
        unsigned int file_block;
        unsigned int in_block;
        unsigned int block;
        unsigned int take;
        unsigned int unit;

        (void)size_chars;
        if (!dtfs_is_file(node) || buf == 0 || dtfs_load(node) != 0)
                return -1;
        old_size = dtfs_size_words(VFS_INDEX(node));
        need = off + nwords;
        if (need > old_size && dtfs_resize(node, need) != 0)
                return -1;
        unit = dtfs_units[VFS_MOUNT_ID(node) - 1U];
        done = 0U;
        while (done < nwords) {
                file_block = (off + done) / DTFS_DATA_WORDS;
                in_block = (off + done) % DTFS_DATA_WORDS;
                if (dtfs_locate(node, file_block, &block) != 0 ||
                    dtfs_dtc_read(unit, block, dtfs_block) != 0)
                        return -1;
                take = DTFS_DATA_WORDS - in_block;
                if (take > nwords - done)
                        take = nwords - done;
                fs_copy_words(&buf[done],
                    &dtfs_block[1U + in_block], take);
                if (dtfs_dtc_write(unit, block, dtfs_block) != 0)
                        return -1;
                done += take;
        }
        return (int)done;
}

int
dtfs_sync(vnode_t node)
{
        if (!dtfs_is_root(node) && !dtfs_is_file(node))
                return -1;
        return 0;
}
