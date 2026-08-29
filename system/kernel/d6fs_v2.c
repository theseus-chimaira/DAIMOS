#include "d6fs_v2.h"

static kword_t
d6fs_v2_min_above(const struct d6fs_v2_diskset *set, kword_t floor)
{
        unsigned int i;
        kword_t next;

        next = 0UL;
        for (i = 0U; i < set->members; ++i) {
                if (set->blocks[i] > floor &&
                    (next == 0UL || set->blocks[i] < next))
                        next = set->blocks[i];
        }
        return next;
}

static unsigned int
d6fs_v2_width_above(const struct d6fs_v2_diskset *set, kword_t floor)
{
        unsigned int i;
        unsigned int width;

        width = 0U;
        for (i = 0U; i < set->members; ++i)
                if (set->blocks[i] > floor)
                        ++width;
        return width;
}

int
d6fs_v2_extent_encode(kword_t start, kword_t blocks, kword_t *runp,
    unsigned int *highp)
{
        kword_t count;

        if (runp == 0 || highp == 0 ||
            start > D6FS_V2_LOGICAL_BLOCK_MASK ||
            blocks == 0UL || blocks > D6FS_V2_EXTENT_MAX_BLOCKS)
                return -1;

        count = blocks - 1UL;
        *runp = ((start & D6FS_V2_LOGICAL_BLOCK_MASK) <<
            D6FS_V2_EXTENT_LOW_BITS) |
            (count & D6FS_V2_EXTENT_LOW_MASK);
        *highp = (unsigned int)((count >> D6FS_V2_EXTENT_LOW_BITS) &
            D6FS_V2_EXTENT_HIGH_MASK);
        return 0;
}

int
d6fs_v2_extent_decode(kword_t run, unsigned int high, kword_t *startp,
    kword_t *blocksp)
{
        kword_t count;

        if (startp == 0 || blocksp == 0 ||
            (high & ~((unsigned int)D6FS_V2_EXTENT_HIGH_MASK)) != 0U)
                return -1;

        *startp = (run >> D6FS_V2_EXTENT_LOW_BITS) &
            D6FS_V2_LOGICAL_BLOCK_MASK;
        count = ((kword_t)high << D6FS_V2_EXTENT_LOW_BITS) |
            (run & D6FS_V2_EXTENT_LOW_MASK);
        *blocksp = count + 1UL;
        return 0;
}

int
d6fs_v2_diskset_valid(const struct d6fs_v2_diskset *set)
{
        unsigned int i;

        if (set == 0 || set->members == 0U ||
            set->members > D6FS_V2_MAX_MEMBERS)
                return 0;
        for (i = 0U; i < set->members; ++i)
                if (set->blocks[i] == 0UL)
                        return 0;
        return 1;
}

kword_t
d6fs_v2_diskset_blocks(const struct d6fs_v2_diskset *set)
{
        unsigned int i;
        kword_t total;

        if (!d6fs_v2_diskset_valid(set))
                return 0UL;
        total = 0UL;
        for (i = 0U; i < set->members; ++i)
                total += set->blocks[i];
        return total;
}

int
d6fs_v2_map_block(const struct d6fs_v2_diskset *set, kword_t logical,
    struct d6fs_v2_phys *phys)
{
        unsigned int i;
        unsigned int width;
        unsigned int slot;
        kword_t floor;
        kword_t next;
        kword_t zone_blocks;
        kword_t rel;

        if (!d6fs_v2_diskset_valid(set) || phys == 0 ||
            logical >= d6fs_v2_diskset_blocks(set))
                return -1;

        floor = 0UL;
        for (;;) {
                next = d6fs_v2_min_above(set, floor);
                width = d6fs_v2_width_above(set, floor);
                if (next == 0UL || width == 0U)
                        return -1;
                zone_blocks = (next - floor) * (kword_t)width;
                if (logical < zone_blocks)
                        break;
                logical -= zone_blocks;
                floor = next;
        }

        slot = (unsigned int)(logical % (kword_t)width);
        rel = logical / (kword_t)width;
        for (i = 0U; i < set->members; ++i) {
                if (set->blocks[i] <= floor)
                        continue;
                if (slot == 0U) {
                        phys->member = i;
                        phys->block = floor + rel;
                        return 0;
                }
                --slot;
        }
        return -1;
}

unsigned int
d6fs_v2_extent_high_get(kword_t word, unsigned int extent)
{
        if (extent >= D6FS_V2_EXTENTS)
                return 0U;
        return (unsigned int)((word >> (extent * D6FS_V2_EXTENT_HIGH_BITS)) &
            D6FS_V2_EXTENT_HIGH_MASK);
}

int
d6fs_v2_extent_high_set(kword_t *wordp, unsigned int extent,
    unsigned int high)
{
        unsigned int shift;
        kword_t mask;

        if (wordp == 0 || extent >= D6FS_V2_EXTENTS ||
            (high & ~((unsigned int)D6FS_V2_EXTENT_HIGH_MASK)) != 0U)
                return -1;
        shift = extent * D6FS_V2_EXTENT_HIGH_BITS;
        mask = D6FS_V2_EXTENT_HIGH_MASK << shift;
        *wordp = (*wordp & ~mask) | ((kword_t)high << shift);
        return 0;
}

static int
d6fs_v2_type_valid(unsigned int type)
{
        return type == D6FS_V2_TYPE_FREE || type == D6FS_V2_TYPE_REG ||
            type == D6FS_V2_TYPE_DIR || type == D6FS_V2_TYPE_SYMLINK;
}

int
d6fs_v2_fcb_encode(kword_t fcb[D6FS_V2_FCB_WORDS],
    const struct d6fs_v2_fcb_info *info)
{
        unsigned int i;

        if (fcb == 0 || info == 0 || !d6fs_v2_type_valid(info->type) ||
            info->flags > D6FS_V2_FLAG_MASK || info->mode > 07777U ||
            info->tail > 017U || info->extent_count > D6FS_V2_EXTENTS ||
            info->uid > D6FS_V2_FCB_MASK || info->gid > D6FS_V2_FCB_MASK ||
            info->parent_fcb > D6FS_V2_FCB_MASK)
                return -1;
        for (i = 0U; i < D6FS_V2_FCB_WORDS; ++i)
                fcb[i] = 0UL;
        fcb[D6FS_V2_FCB_META] = ((kword_t)info->type << 33) |
            ((kword_t)info->flags << 24) | ((kword_t)info->mode << 12) |
            ((kword_t)info->tail << 8) | ((kword_t)info->extent_count << 4);
        fcb[D6FS_V2_FCB_OWNER] = ((kword_t)info->uid << 18) |
            (kword_t)info->gid;
        fcb[D6FS_V2_FCB_SIZE] = info->size_words;
        fcb[D6FS_V2_FCB_MTIME] = info->mtime;
        fcb[D6FS_V2_FCB_PARENT] = (kword_t)info->parent_fcb << 18;
        return 0;
}

int
d6fs_v2_fcb_decode(const kword_t fcb[D6FS_V2_FCB_WORDS],
    struct d6fs_v2_fcb_info *info)
{
        if (fcb == 0 || info == 0)
                return -1;
        info->type = (unsigned int)((fcb[D6FS_V2_FCB_META] >> 33) & 07UL);
        info->flags = (unsigned int)((fcb[D6FS_V2_FCB_META] >> 24) & 0777UL);
        info->mode = (unsigned int)((fcb[D6FS_V2_FCB_META] >> 12) & 07777UL);
        info->tail = (unsigned int)((fcb[D6FS_V2_FCB_META] >> 8) & 017UL);
        info->extent_count =
            (unsigned int)((fcb[D6FS_V2_FCB_META] >> 4) & 017UL);
        info->uid =
            (unsigned int)((fcb[D6FS_V2_FCB_OWNER] >> 18) & D6FS_V2_FCB_MASK);
        info->gid = (unsigned int)(fcb[D6FS_V2_FCB_OWNER] & D6FS_V2_FCB_MASK);
        info->size_words = fcb[D6FS_V2_FCB_SIZE];
        info->mtime = fcb[D6FS_V2_FCB_MTIME];
        info->parent_fcb =
            (unsigned int)((fcb[D6FS_V2_FCB_PARENT] >> 18) & D6FS_V2_FCB_MASK);
        return 0;
}

int
d6fs_v2_fcb_valid(const kword_t fcb[D6FS_V2_FCB_WORDS],
    kword_t fs_blocks, unsigned int fcb_count)
{
        struct d6fs_v2_fcb_info info;
        unsigned int i;
        unsigned int high;
        kword_t start;
        kword_t blocks;
        kword_t capacity;

        if (fcb == 0 || fs_blocks == 0UL || fcb_count == 0U ||
            d6fs_v2_fcb_decode(fcb, &info) != 0 ||
            !d6fs_v2_type_valid(info.type) || info.extent_count > D6FS_V2_EXTENTS ||
            info.tail > 4U || (fcb[D6FS_V2_FCB_META] & 017UL) != 0UL ||
            (fcb[D6FS_V2_FCB_PARENT] & D6FS_V2_FCB_MASK) != 0UL ||
            fcb[D6FS_V2_FCB_RESERVED0] != 0UL ||
            fcb[D6FS_V2_FCB_RESERVED0 + 1U] != 0UL ||
            fcb[D6FS_V2_FCB_RESERVED0 + 2U] != 0UL)
                return 0;
        if (info.type == D6FS_V2_TYPE_FREE)
                return info.extent_count == 0U && info.size_words == 0UL;
        if (info.parent_fcb >= fcb_count)
                return 0;
        capacity = 0UL;
        for (i = 0U; i < D6FS_V2_EXTENTS; ++i) {
                if (i >= info.extent_count) {
                        if (fcb[D6FS_V2_FCB_EXTENT0 + i] != 0UL ||
                            d6fs_v2_extent_high_get(fcb[D6FS_V2_FCB_LENHIGH], i) != 0U)
                                return 0;
                        continue;
                }
                high = d6fs_v2_extent_high_get(fcb[D6FS_V2_FCB_LENHIGH], i);
                if (d6fs_v2_extent_decode(fcb[D6FS_V2_FCB_EXTENT0 + i], high,
                    &start, &blocks) != 0 || start >= fs_blocks ||
                    blocks > fs_blocks - start)
                        return 0;
                capacity += blocks * (kword_t)D6FS_V2_BLOCK_WORDS;
        }
        if (info.size_words > capacity)
                return 0;
        return 1;
}

int
d6fs_v2_super_encode(kword_t sb[D6FS_V2_SUPER_WORDS],
    const struct d6fs_v2_super_info *info)
{
        unsigned int i;

        if (sb == 0 || info == 0 || info->state > D6FS_V2_STATE_DIRTY ||
            info->total_blocks == 0UL ||
            info->total_blocks > D6FS_V2_LOGICAL_BLOCK_MASK + 1UL ||
            info->root_fcb > D6FS_V2_FCB_MASK ||
            info->fcb_count == 0U || info->fcb_count > D6FS_V2_FCB_MASK)
                return -1;
        for (i = 0U; i < D6FS_V2_SUPER_WORDS; ++i)
                sb[i] = 0UL;
        sb[D6FS_V2_SB_MAGIC_VERSION] =
            (D6FS_V2_MAGIC & ~077UL) | D6FS_V2_FORMAT_VERSION;
        sb[D6FS_V2_SB_SEQUENCE] = info->sequence;
        sb[D6FS_V2_SB_STATE] = (kword_t)info->state;
        sb[D6FS_V2_SB_FS_UUID0] = info->fs_uuid[0];
        sb[D6FS_V2_SB_FS_UUID1] = info->fs_uuid[1];
        sb[D6FS_V2_SB_DISKSET_UUID0] = info->diskset_uuid[0];
        sb[D6FS_V2_SB_DISKSET_UUID1] = info->diskset_uuid[1];
        sb[D6FS_V2_SB_TOTAL_BLOCKS] = info->total_blocks;
        sb[D6FS_V2_SB_ROOT_FCB] = (kword_t)info->root_fcb;
        sb[D6FS_V2_SB_FCB_START] = info->fcb_start;
        sb[D6FS_V2_SB_FCB_COUNT] = (kword_t)info->fcb_count;
        sb[D6FS_V2_SB_FREEMAP_START] = info->freemap_start;
        sb[D6FS_V2_SB_FREEMAP_BLOCKS] = info->freemap_blocks;
        sb[D6FS_V2_SB_SUMMARY_START] = info->summary_start;
        sb[D6FS_V2_SB_SUMMARY_BLOCKS] = info->summary_blocks;
        return 0;
}

int
d6fs_v2_super_decode(const kword_t sb[D6FS_V2_SUPER_WORDS],
    struct d6fs_v2_super_info *info)
{
        if (sb == 0 || info == 0)
                return -1;
        info->sequence = sb[D6FS_V2_SB_SEQUENCE];
        info->state = (unsigned int)sb[D6FS_V2_SB_STATE];
        info->fs_uuid[0] = sb[D6FS_V2_SB_FS_UUID0];
        info->fs_uuid[1] = sb[D6FS_V2_SB_FS_UUID1];
        info->diskset_uuid[0] = sb[D6FS_V2_SB_DISKSET_UUID0];
        info->diskset_uuid[1] = sb[D6FS_V2_SB_DISKSET_UUID1];
        info->total_blocks = sb[D6FS_V2_SB_TOTAL_BLOCKS];
        info->root_fcb = (unsigned int)sb[D6FS_V2_SB_ROOT_FCB];
        info->fcb_start = sb[D6FS_V2_SB_FCB_START];
        info->fcb_count = (unsigned int)sb[D6FS_V2_SB_FCB_COUNT];
        info->freemap_start = sb[D6FS_V2_SB_FREEMAP_START];
        info->freemap_blocks = sb[D6FS_V2_SB_FREEMAP_BLOCKS];
        info->summary_start = sb[D6FS_V2_SB_SUMMARY_START];
        info->summary_blocks = sb[D6FS_V2_SB_SUMMARY_BLOCKS];
        return 0;
}

static int
d6fs_v2_uuid_equal(const kword_t a[2], const kword_t b[2])
{
        return a[0] == b[0] && a[1] == b[1];
}

int
d6fs_v2_super_select(const kword_t a[D6FS_V2_SUPER_WORDS],
    const kword_t b[D6FS_V2_SUPER_WORDS], kword_t diskset_blocks,
    struct d6fs_v2_super_info *info, unsigned int *copyp)
{
        struct d6fs_v2_super_info ai;
        struct d6fs_v2_super_info bi;
        unsigned int i;
        int av;
        int bv;

        if (a == 0 || b == 0 || info == 0 || copyp == 0)
                return -1;
        av = d6fs_v2_super_valid(a, diskset_blocks);
        bv = d6fs_v2_super_valid(b, diskset_blocks);
        if (!av && !bv)
                return -1;
        if (av && d6fs_v2_super_decode(a, &ai) != 0)
                return -1;
        if (bv && d6fs_v2_super_decode(b, &bi) != 0)
                return -1;
        if (av && bv) {
                if (!d6fs_v2_uuid_equal(ai.fs_uuid, bi.fs_uuid) ||
                    !d6fs_v2_uuid_equal(ai.diskset_uuid, bi.diskset_uuid))
                        return -1;
                if (ai.sequence == bi.sequence) {
                        for (i = 0U; i < D6FS_V2_SUPER_WORDS; ++i)
                                if (a[i] != b[i])
                                        return -1;
                        *info = ai;
                        *copyp = 0U;
                        return 0;
                }
                if (bi.sequence > ai.sequence) {
                        *info = bi;
                        *copyp = 1U;
                } else {
                        *info = ai;
                        *copyp = 0U;
                }
                return 0;
        }
        if (av) {
                *info = ai;
                *copyp = 0U;
        } else {
                *info = bi;
                *copyp = 1U;
        }
        return 0;
}

static int
d6fs_v2_range_valid(kword_t start, kword_t blocks, kword_t total)
{
        return blocks != 0UL && start < total && blocks <= total - start;
}

int
d6fs_v2_super_valid(const kword_t sb[D6FS_V2_SUPER_WORDS],
    kword_t diskset_blocks)
{
        struct d6fs_v2_super_info info;
        kword_t magic_version;

        if (sb == 0 || diskset_blocks == 0UL ||
            d6fs_v2_super_decode(sb, &info) != 0)
                return 0;
        magic_version = (D6FS_V2_MAGIC & ~077UL) | D6FS_V2_FORMAT_VERSION;
        if (sb[D6FS_V2_SB_MAGIC_VERSION] != magic_version ||
            sb[D6FS_V2_SB_RESERVED] != 0UL ||
            info.state > D6FS_V2_STATE_DIRTY || info.total_blocks == 0UL ||
            info.total_blocks > diskset_blocks ||
            info.total_blocks > D6FS_V2_LOGICAL_BLOCK_MASK + 1UL ||
            info.fcb_count == 0U || info.fcb_count > D6FS_V2_FCB_MASK ||
            info.root_fcb >= info.fcb_count ||
            !d6fs_v2_range_valid(info.fcb_start,
            ((kword_t)info.fcb_count * D6FS_V2_FCB_WORDS +
            D6FS_V2_BLOCK_WORDS - 1UL) / D6FS_V2_BLOCK_WORDS,
            info.total_blocks) ||
            !d6fs_v2_range_valid(info.freemap_start, info.freemap_blocks,
            info.total_blocks) ||
            !d6fs_v2_range_valid(info.summary_start, info.summary_blocks,
            info.total_blocks))
                return 0;
        return 1;
}

int
d6fs_v2_dirent_decode(const kword_t ent[D6FS_V2_DIRENT_WORDS],
    struct d6fs_v2_dirent_info *info)
{
        if (ent == 0 || info == 0)
                return -1;
        info->name[0] = ent[0];
        info->name[1] = ent[1];
        info->name[2] = ent[2];
        info->name[3] = ent[3];
        info->hash = (ent[4] >> D6FS_V2_DIRENT_HASH_SHIFT) &
            D6FS_V2_HASH_MASK;
        info->type = (unsigned int)((ent[4] >> D6FS_V2_DIRENT_TYPE_SHIFT) &
            D6FS_V2_DIRENT_TYPE_MASK);
        info->flags = (unsigned int)(ent[4] & D6FS_V2_DIRENT_FLAGS_MASK);
        info->child_fcb = (unsigned int)((ent[5] >> 18) & D6FS_V2_FCB_MASK);
        return 0;
}

int
d6fs_v2_dirent_valid(const kword_t ent[D6FS_V2_DIRENT_WORDS],
    unsigned int fcb_count)
{
        struct d6fs_v2_dirent_info info;
        unsigned int empty;

        if (ent == 0 || fcb_count == 0U || d6fs_v2_dirent_decode(ent, &info) != 0 ||
            (ent[5] & D6FS_V2_FCB_MASK) != 0UL)
                return 0;
        empty = info.child_fcb == 0U;
        if (empty)
                return info.name[0] == 0UL && info.name[1] == 0UL &&
                    info.name[2] == 0UL && info.name[3] == 0UL &&
                    ent[4] == 0UL;
        if (info.child_fcb >= fcb_count || info.type == D6FS_V2_TYPE_FREE ||
            !d6fs_v2_type_valid(info.type))
                return 0;
        return info.name[0] != 0UL || info.name[1] != 0UL ||
            info.name[2] != 0UL || info.name[3] != 0UL;
}

int
d6fs_v2_reader_init(struct d6fs_v2_reader *reader,
    d6fs_v2_read_block_fn read_block, void *opaque,
    const struct d6fs_v2_super_info *super)
{
        if (reader == 0 || read_block == 0 || super == 0 ||
            super->total_blocks == 0UL || super->fcb_count == 0U)
                return -1;
        reader->read_block = read_block;
        reader->write_block = 0;
        reader->opaque = opaque;
        reader->super = *super;
        reader->cache_block = 0UL;
        reader->cache_valid = 0U;
        reader->physical_reads = 0UL;
        reader->physical_writes = 0UL;
        reader->cache_hits = 0UL;
        reader->cache_misses = 0UL;
        return 0;
}

static int
d6fs_v2_reader_block(struct d6fs_v2_reader *reader, kword_t logical,
    const kword_t **blockp)
{
        if (reader == 0 || blockp == 0 || logical >= reader->super.total_blocks)
                return -1;
        if (!reader->cache_valid || reader->cache_block != logical) {
                ++reader->cache_misses;
                if (reader->read_block(reader->opaque, logical,
                    reader->cache) != 0)
                        return -1;
                ++reader->physical_reads;
                reader->cache_block = logical;
                reader->cache_valid = 1U;
        } else {
                ++reader->cache_hits;
        }
        *blockp = reader->cache;
        return 0;
}

int
d6fs_v2_reader_get_block(struct d6fs_v2_reader *reader, kword_t logical,
    const kword_t **blockp)
{
        return d6fs_v2_reader_block(reader, logical, blockp);
}

int
d6fs_v2_reader_fcb(struct d6fs_v2_reader *reader, unsigned int fcb_index,
    kword_t fcb[D6FS_V2_FCB_WORDS])
{
        kword_t word_index;
        kword_t block_no;
        unsigned int in_block;
        unsigned int i;
        const kword_t *block;

        if (reader == 0 || fcb == 0 || fcb_index >= reader->super.fcb_count)
                return -1;
        word_index = (kword_t)fcb_index * D6FS_V2_FCB_WORDS;
        block_no = reader->super.fcb_start + word_index / D6FS_V2_BLOCK_WORDS;
        in_block = (unsigned int)(word_index % D6FS_V2_BLOCK_WORDS);
        if (in_block + D6FS_V2_FCB_WORDS > D6FS_V2_BLOCK_WORDS ||
            d6fs_v2_reader_block(reader, block_no, &block) != 0)
                return -1;
        for (i = 0U; i < D6FS_V2_FCB_WORDS; ++i)
                fcb[i] = block[in_block + i];
        return d6fs_v2_fcb_valid(fcb, reader->super.total_blocks,
            reader->super.fcb_count) ? 0 : -1;
}

int
d6fs_v2_file_block(const kword_t fcb[D6FS_V2_FCB_WORDS],
    kword_t file_block, kword_t *logical_block)
{
        struct d6fs_v2_fcb_info info;
        unsigned int i;
        unsigned int high;
        kword_t start;
        kword_t blocks;

        if (fcb == 0 || logical_block == 0 ||
            d6fs_v2_fcb_decode(fcb, &info) != 0)
                return -1;
        for (i = 0U; i < info.extent_count; ++i) {
                high = d6fs_v2_extent_high_get(fcb[D6FS_V2_FCB_LENHIGH], i);
                if (d6fs_v2_extent_decode(fcb[D6FS_V2_FCB_EXTENT0 + i], high,
                    &start, &blocks) != 0)
                        return -1;
                if (file_block < blocks) {
                        *logical_block = start + file_block;
                        return 0;
                }
                file_block -= blocks;
        }
        return -1;
}

int
d6fs_v2_reader_read_words(struct d6fs_v2_reader *reader,
    const kword_t fcb[D6FS_V2_FCB_WORDS], kword_t off, kword_t *buf,
    unsigned int nwords)
{
        struct d6fs_v2_fcb_info info;
        kword_t pos;
        kword_t file_block;
        kword_t logical;
        const kword_t *block;
        unsigned int in_block;
        unsigned int take;
        unsigned int done;
        unsigned int i;

        if (reader == 0 || fcb == 0 || buf == 0 ||
            d6fs_v2_fcb_decode(fcb, &info) != 0)
                return -1;
        if (off >= info.size_words)
                return 0;
        if ((kword_t)nwords > info.size_words - off)
                nwords = (unsigned int)(info.size_words - off);
        done = 0U;
        while (done < nwords) {
                pos = off + done;
                file_block = pos / D6FS_V2_BLOCK_WORDS;
                in_block = (unsigned int)(pos % D6FS_V2_BLOCK_WORDS);
                if (d6fs_v2_file_block(fcb, file_block, &logical) != 0 ||
                    d6fs_v2_reader_block(reader, logical, &block) != 0)
                        return -1;
                take = D6FS_V2_BLOCK_WORDS - in_block;
                if (take > nwords - done)
                        take = nwords - done;
                for (i = 0U; i < take; ++i)
                        buf[done + i] = block[in_block + i];
                done += take;
        }
        return (int)done;
}

kword_t
d6fs_v2_name_hash24(const kword_t words[4], unsigned int chars)
{
        kword_t h;
        kword_t word;
        unsigned int i;
        unsigned int c;

        if (words == 0 || chars == 0U || chars > 24U)
                return 0UL;
        h = 0UL;
        word = 0UL;
        for (i = 0U; i < chars; ++i) {
                if ((i % 6U) == 0U)
                        word = words[i / 6U];
                c = (unsigned int)(((word >> 30) & 077UL) + 040UL);
                word <<= 6;
                h = ((h << 5) ^ (h >> 19) ^ (c & 0377U)) &
                    D6FS_V2_HASH_MASK;
        }
        return h;
}

int
d6fs_v2_dirent_encode(kword_t ent[D6FS_V2_DIRENT_WORDS],
    const struct d6fs_v2_dirent_info *info)
{
        unsigned int i;

        if (ent == 0 || info == 0 || info->hash > D6FS_V2_HASH_MASK ||
            info->type > D6FS_V2_DIRENT_TYPE_MASK ||
            info->flags > D6FS_V2_DIRENT_FLAGS_MASK ||
            info->child_fcb > D6FS_V2_FCB_MASK)
                return -1;
        for (i = 0U; i < 4U; ++i)
                ent[i] = info->name[i];
        ent[4] = (info->hash << D6FS_V2_DIRENT_HASH_SHIFT) |
            ((kword_t)info->type << D6FS_V2_DIRENT_TYPE_SHIFT) |
            (kword_t)info->flags;
        ent[5] = (kword_t)info->child_fcb << 18;
        return 0;
}

int
d6fs_v2_reader_set_writer(struct d6fs_v2_reader *reader,
    d6fs_v2_write_block_fn write_block)
{
        if (reader == 0)
                return -1;
        reader->write_block = write_block;
        return 0;
}

int
d6fs_v2_reader_write_block(struct d6fs_v2_reader *reader, kword_t logical,
    const kword_t block[D6FS_V2_BLOCK_WORDS])
{
        unsigned int i;

        if (reader == 0 || block == 0 || reader->write_block == 0 ||
            logical >= reader->super.total_blocks ||
            reader->write_block(reader->opaque, logical, block) != 0)
                return -1;
        ++reader->physical_writes;
        reader->cache_block = logical;
        reader->cache_valid = 1U;
        for (i = 0U; i < D6FS_V2_BLOCK_WORDS; ++i)
                reader->cache[i] = block[i];
        return 0;
}

int
d6fs_v2_reader_zero_block(struct d6fs_v2_reader *reader, kword_t logical)
{
        unsigned int i;

        if (reader == 0 || reader->write_block == 0 ||
            logical >= reader->super.total_blocks)
                return -1;
        for (i = 0U; i < D6FS_V2_BLOCK_WORDS; ++i)
                reader->cache[i] = 0UL;
        if (reader->write_block(reader->opaque, logical, reader->cache) != 0) {
                reader->cache_valid = 0U;
                return -1;
        }
        ++reader->physical_writes;
        reader->cache_block = logical;
        reader->cache_valid = 1U;
        return 0;
}

int
d6fs_v2_reader_write_words(struct d6fs_v2_reader *reader,
    const kword_t fcb[D6FS_V2_FCB_WORDS], kword_t off,
    const kword_t *buf, unsigned int nwords)
{
        kword_t logical;
        kword_t pos;
        kword_t file_block;
        unsigned int in_block;
        unsigned int done;
        unsigned int take;
        unsigned int i;
        const kword_t *cached;
        kword_t block[D6FS_V2_BLOCK_WORDS];

        if (reader == 0 || fcb == 0 || buf == 0 || reader->write_block == 0)
                return -1;
        done = 0U;
        while (done < nwords) {
                pos = off + (kword_t)done;
                file_block = pos / D6FS_V2_BLOCK_WORDS;
                in_block = (unsigned int)(pos % D6FS_V2_BLOCK_WORDS);
                if (d6fs_v2_file_block(fcb, file_block, &logical) != 0 ||
                    d6fs_v2_reader_block(reader, logical, &cached) != 0)
                        return -1;
                for (i = 0U; i < D6FS_V2_BLOCK_WORDS; ++i)
                        block[i] = cached[i];
                take = D6FS_V2_BLOCK_WORDS - in_block;
                if (take > nwords - done)
                        take = nwords - done;
                for (i = 0U; i < take; ++i)
                        block[in_block + i] = buf[done + i];
                if (d6fs_v2_reader_write_block(reader, logical, block) != 0)
                        return -1;
                done += take;
        }
        return (int)done;
}

int
d6fs_v2_reader_put_fcb(struct d6fs_v2_reader *reader,
    unsigned int fcb_index, const kword_t fcb[D6FS_V2_FCB_WORDS])
{
        kword_t word_index;
        kword_t block_no;
        unsigned int in_block;
        unsigned int i;
        const kword_t *cached;
        kword_t block[D6FS_V2_BLOCK_WORDS];

        if (reader == 0 || fcb == 0 || reader->write_block == 0 ||
            fcb_index >= reader->super.fcb_count)
                return -1;
        word_index = (kword_t)fcb_index * D6FS_V2_FCB_WORDS;
        block_no = reader->super.fcb_start + word_index / D6FS_V2_BLOCK_WORDS;
        in_block = (unsigned int)(word_index % D6FS_V2_BLOCK_WORDS);
        if (in_block + D6FS_V2_FCB_WORDS > D6FS_V2_BLOCK_WORDS ||
            d6fs_v2_reader_block(reader, block_no, &cached) != 0)
                return -1;
        for (i = 0U; i < D6FS_V2_BLOCK_WORDS; ++i)
                block[i] = cached[i];
        for (i = 0U; i < D6FS_V2_FCB_WORDS; ++i)
                block[in_block + i] = fcb[i];
        return d6fs_v2_reader_write_block(reader, block_no, block);
}

static int
d6fs_v2_bitmap_position(kword_t logical, kword_t *map_blockp,
    unsigned int *wordp, unsigned int *bitp)
{
        kword_t in;

        if (map_blockp == 0 || wordp == 0 || bitp == 0)
                return -1;
        *map_blockp = logical / (kword_t)D6FS_V2_BITS_PER_MAP_BLOCK;
        in = logical % (kword_t)D6FS_V2_BITS_PER_MAP_BLOCK;
        *wordp = (unsigned int)(in / D6FS_V2_BITS_PER_WORD);
        *bitp = (unsigned int)(in % D6FS_V2_BITS_PER_WORD);
        return 0;
}

static kword_t
d6fs_v2_bit_mask(unsigned int bit)
{
        return 1UL << (35U - bit);
}

int
d6fs_v2_freemap_test(struct d6fs_v2_reader *reader, kword_t logical,
    unsigned int *allocatedp)
{
        kword_t mbi;
        unsigned int wi;
        unsigned int bi;
        const kword_t *block;

        if (reader == 0 || allocatedp == 0 ||
            logical >= reader->super.total_blocks ||
            d6fs_v2_bitmap_position(logical, &mbi, &wi, &bi) != 0 ||
            mbi >= reader->super.freemap_blocks ||
            d6fs_v2_reader_block(reader, reader->super.freemap_start + mbi,
            &block) != 0)
                return -1;
        *allocatedp = (block[wi] & d6fs_v2_bit_mask(bi)) != 0UL;
        return 0;
}

static int
d6fs_v2_summary_test(struct d6fs_v2_reader *reader, kword_t map_index,
    unsigned int *has_freep)
{
        kword_t sbi;
        kword_t in;
        unsigned int wi;
        unsigned int bi;
        const kword_t *block;

        if (reader == 0 || has_freep == 0)
                return -1;
        sbi = map_index / (kword_t)D6FS_V2_BITS_PER_MAP_BLOCK;
        in = map_index % (kword_t)D6FS_V2_BITS_PER_MAP_BLOCK;
        wi = (unsigned int)(in / D6FS_V2_BITS_PER_WORD);
        bi = (unsigned int)(in % D6FS_V2_BITS_PER_WORD);
        if (sbi >= reader->super.summary_blocks ||
            d6fs_v2_reader_block(reader, reader->super.summary_start + sbi,
            &block) != 0)
                return -1;
        *has_freep = (block[wi] & d6fs_v2_bit_mask(bi)) != 0UL;
        return 0;
}

static int
d6fs_v2_summary_set(struct d6fs_v2_reader *reader, kword_t map_index,
    unsigned int has_free)
{
        kword_t sbi;
        kword_t in;
        unsigned int wi;
        unsigned int bi;
        unsigned int i;
        const kword_t *cached;
        kword_t block[D6FS_V2_BLOCK_WORDS];
        kword_t bits_per_block;

        bits_per_block = D6FS_V2_BITS_PER_MAP_BLOCK;
        sbi = map_index / bits_per_block;
        in = map_index % bits_per_block;
        wi = (unsigned int)(in / D6FS_V2_BITS_PER_WORD);
        bi = (unsigned int)(in % D6FS_V2_BITS_PER_WORD);
        if (sbi >= reader->super.summary_blocks ||
            d6fs_v2_reader_block(reader, reader->super.summary_start + sbi,
            &cached) != 0)
                return -1;
        for (i = 0U; i < D6FS_V2_BLOCK_WORDS; ++i)
                block[i] = cached[i];
        if (has_free)
                block[wi] |= d6fs_v2_bit_mask(bi);
        else
                block[wi] &= ~d6fs_v2_bit_mask(bi);
        return d6fs_v2_reader_write_block(reader,
            reader->super.summary_start + sbi, block);
}

static int
d6fs_v2_map_block_has_free(struct d6fs_v2_reader *reader, kword_t mbi,
    unsigned int *has_freep)
{
        const kword_t *block;
        kword_t first;
        kword_t limit;
        kword_t logical;
        unsigned int wi;
        unsigned int bi;

        if (reader == 0 || has_freep == 0 ||
            mbi >= reader->super.freemap_blocks ||
            d6fs_v2_reader_block(reader, reader->super.freemap_start + mbi,
            &block) != 0)
                return -1;
        first = mbi * (kword_t)D6FS_V2_BITS_PER_MAP_BLOCK;
        limit = first + (kword_t)D6FS_V2_BITS_PER_MAP_BLOCK;
        if (limit > reader->super.total_blocks)
                limit = reader->super.total_blocks;
        *has_freep = 0U;
        for (logical = first; logical < limit; ++logical) {
                kword_t in = logical - first;
                wi = (unsigned int)(in / D6FS_V2_BITS_PER_WORD);
                bi = (unsigned int)(in % D6FS_V2_BITS_PER_WORD);
                if ((block[wi] & d6fs_v2_bit_mask(bi)) == 0UL) {
                        *has_freep = 1U;
                        break;
                }
        }
        return 0;
}

int
d6fs_v2_freemap_set(struct d6fs_v2_reader *reader, kword_t logical,
    unsigned int allocated)
{
        kword_t mbi;
        unsigned int wi;
        unsigned int bi;
        unsigned int i;
        unsigned int has_free;
        const kword_t *cached;
        kword_t block[D6FS_V2_BLOCK_WORDS];

        if (reader == 0 || reader->write_block == 0 ||
            logical >= reader->super.total_blocks ||
            d6fs_v2_bitmap_position(logical, &mbi, &wi, &bi) != 0 ||
            mbi >= reader->super.freemap_blocks ||
            d6fs_v2_reader_block(reader, reader->super.freemap_start + mbi,
            &cached) != 0)
                return -1;
        for (i = 0U; i < D6FS_V2_BLOCK_WORDS; ++i)
                block[i] = cached[i];
        if (allocated)
                block[wi] |= d6fs_v2_bit_mask(bi);
        else
                block[wi] &= ~d6fs_v2_bit_mask(bi);
        if (d6fs_v2_reader_write_block(reader,
            reader->super.freemap_start + mbi, block) != 0)
                return -1;
        if (!allocated)
                has_free = 1U;
        else if (d6fs_v2_map_block_has_free(reader, mbi, &has_free) != 0)
                return -1;
        return d6fs_v2_summary_set(reader, mbi, has_free);
}

int
d6fs_v2_alloc_run(struct d6fs_v2_reader *reader, kword_t cursor,
    kword_t max_blocks, kword_t *startp, kword_t *blocksp)
{
        kword_t logical;
        kword_t start;
        kword_t count;
        kword_t scanned;
        unsigned int allocated;

        if (reader == 0 || startp == 0 || blocksp == 0 ||
            reader->write_block == 0 || reader->super.total_blocks == 0UL ||
            max_blocks == 0UL)
                return -1;
        cursor %= reader->super.total_blocks;
        start = 0UL;
        count = 0UL;
        for (scanned = 0UL; scanned < reader->super.total_blocks; ++scanned) {
                unsigned int has_free;
                kword_t mbi;
                kword_t skip;

                logical = cursor + scanned;
                if (logical >= reader->super.total_blocks)
                        logical -= reader->super.total_blocks;
                if (count == 0UL &&
                    logical % (kword_t)D6FS_V2_BITS_PER_MAP_BLOCK == 0UL) {
                        mbi = logical / (kword_t)D6FS_V2_BITS_PER_MAP_BLOCK;
                        if (mbi < reader->super.freemap_blocks &&
                            d6fs_v2_summary_test(reader, mbi, &has_free) == 0 &&
                            !has_free) {
                                skip = (kword_t)D6FS_V2_BITS_PER_MAP_BLOCK;
                                if (skip > reader->super.total_blocks - scanned)
                                        skip = reader->super.total_blocks - scanned;
                                if (skip != 0UL)
                                        scanned += skip - 1UL;
                                continue;
                        }
                }
                if (d6fs_v2_freemap_test(reader, logical, &allocated) != 0)
                        return -1;
                if (!allocated) {
                        if (count == 0UL)
                                start = logical;
                        else if (logical != start + count)
                                count = 0UL;
                        if (count == 0UL)
                                start = logical;
                        ++count;
                        if (count >= max_blocks)
                                break;
                } else if (count != 0UL) {
                        break;
                }
        }
        if (count == 0UL)
                return -1;
        for (logical = 0UL; logical < count; ++logical)
                if (d6fs_v2_freemap_set(reader, start + logical, 1U) != 0) {
                        while (logical != 0UL) {
                                --logical;
                                (void)d6fs_v2_freemap_set(reader,
                                    start + logical, 0U);
                        }
                        return -1;
                }
        *startp = start;
        *blocksp = count;
        return 0;
}

int
d6fs_v2_free_run(struct d6fs_v2_reader *reader, kword_t start,
    kword_t blocks)
{
        kword_t i;

        if (reader == 0 || blocks == 0UL || start >= reader->super.total_blocks ||
            blocks > reader->super.total_blocks - start)
                return -1;
        for (i = 0UL; i < blocks; ++i)
                if (d6fs_v2_freemap_set(reader, start + i, 0U) != 0)
                        return -1;
        return 0;
}
