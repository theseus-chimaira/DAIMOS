#include "tsfs_scan.h"

#define MASK36 0777777777777UL
#define HALF18 0777777UL
#define TSFS_MAGIC 0646346630000UL
#define TSDIR_MAGIC 0646344516200UL
#define TSFS_VERSION ((kword_t)1U << 18)

#define D_MAGIC 0U
#define D_VERSION 1U
#define D_FLAGS 2U
#define D_ID_HI 3U
#define D_ID_LO 4U
#define D_GENERATION 5U
#define D_MEMBER 6U
#define D_BLOCKS 7U
#define D_TDIR 8U
#define D_TDIR_BLOCKS 9U
#define D_TDIR_CHECKSUM 10U
#define D_CHECKSUM 11U

#define TD_MAGIC 0U
#define TD_VERSION 1U
#define TD_FLAGS 2U
#define TD_FILE_COUNT 3U
#define TD_EXTENT_COUNT 4U
#define TD_PATH_WORDS 5U
#define TD_CHECKSUM 6U
#define TD_FIRST_ENTRY 7U
#define TD_ENTRY_WORDS 5U
#define TD_MAX_ENTRIES ((TSFS_BLOCK_WORDS - TD_FIRST_ENTRY) / TD_ENTRY_WORDS)

#define TE_ID_FLAGS 0U
#define TE_MEMBER_BLOCK 1U
#define TE_BLOCKS_RECWORDS 2U
#define TE_RECORD_COUNT 3U
#define TE_CHECKSUM 4U
#define TABLE_FILE 1U
#define TABLE_EXTENT 2U
#define FILE_WORDS 8U
#define EXTENT_WORDS 4U

struct tsfs_member_info {
        kword_t id_hi;
        kword_t id_lo;
        kword_t generation;
        kword_t blocks;
        unsigned int members;
        unsigned int member;
        unsigned int tdir_member;
        kword_t tdir_block;
        kword_t tdir_blocks;
        kword_t tdir_checksum;
};

struct tsfs_table_info {
        unsigned int present;
        unsigned int member;
        unsigned int block;
        unsigned int blocks;
        unsigned int record_words;
        unsigned int records;
        kword_t checksum;
};

static kword_t
rol36(kword_t v)
{
        v &= MASK36;
        return ((v << 1) | (v >> 35)) & MASK36;
}

static kword_t
checksum_update(kword_t sum, const kword_t *p, unsigned int count,
    unsigned int skip)
{
        unsigned int i;

        for (i = 0U; i < count; ++i) {
                sum = rol36(sum);
                if (i != skip)
                        sum ^= p[i] & MASK36;
                sum = (sum + 1U) & MASK36;
        }
        return sum;
}

static kword_t
tsfs_checksum(const kword_t *p, unsigned int skip)
{
        return checksum_update(0, p, TSFS_BLOCK_WORDS, skip);
}

static int
tsfs_decode(const kword_t *p, struct tsfs_member_info *m)
{
        kword_t member;
        unsigned int i;

        if (p[D_MAGIC] != TSFS_MAGIC || p[D_VERSION] != TSFS_VERSION ||
            p[D_FLAGS] != 0 || p[D_CHECKSUM] != tsfs_checksum(p, D_CHECKSUM))
                return -1;
        for (i = TSFS_DESC_USED_WORDS; i < TSFS_BLOCK_WORDS; ++i)
                if (p[i] != 0)
                        return -1;
        member = p[D_MEMBER];
        m->members = (unsigned int)((member >> 18) & HALF18);
        m->member = (unsigned int)(member & HALF18);
        if (m->members == 0U || m->members > TSFS_MAX_MEMBERS ||
            m->member >= m->members || p[D_BLOCKS] != TSFS_BLOCK_COUNT)
                return -1;
        if ((p[D_TDIR] & HALF18) < 3U || p[D_TDIR_BLOCKS] != 1U)
                return -1;
        m->id_hi = p[D_ID_HI];
        m->id_lo = p[D_ID_LO];
        m->generation = p[D_GENERATION];
        m->blocks = p[D_BLOCKS];
        m->tdir_member = (unsigned int)(p[D_TDIR] >> 18);
        m->tdir_block = p[D_TDIR] & HALF18;
        m->tdir_blocks = p[D_TDIR_BLOCKS];
        m->tdir_checksum = p[D_TDIR_CHECKSUM];
        if (m->tdir_member >= m->members ||
            m->tdir_block + m->tdir_blocks > m->blocks)
                return -1;
        return 0;
}

static int
tsfs_same_member(const struct tsfs_member_info *a,
    const struct tsfs_member_info *b)
{
        return a->id_hi == b->id_hi && a->id_lo == b->id_lo &&
            a->generation == b->generation && a->blocks == b->blocks &&
            a->members == b->members && a->member == b->member &&
            a->tdir_member == b->tdir_member &&
            a->tdir_block == b->tdir_block &&
            a->tdir_blocks == b->tdir_blocks &&
            a->tdir_checksum == b->tdir_checksum;
}

static int
tsfs_read_member(unsigned int unit, struct tsfs_member_info *m)
{
        kword_t a[TSFS_BLOCK_WORDS];
        kword_t b[TSFS_BLOCK_WORDS];
        struct tsfs_member_info ma;
        struct tsfs_member_info mb;
        int va;
        int vb;

        va = dsys_dtc_read_block(unit, TSFS_DESC_PRIMARY_BLOCK, a) == 0 &&
            tsfs_decode(a, &ma) == 0;
        vb = dsys_dtc_read_block(unit, TSFS_DESC_BACKUP_BLOCK, b) == 0 &&
            tsfs_decode(b, &mb) == 0;
        if (!va && !vb)
                return -1;
        if (va && vb && !tsfs_same_member(&ma, &mb))
                return -1;
        *m = va ? ma : mb;
        return 0;
}

static int
tsfs_same_set(const struct tsfs_member_info *a,
    const struct tsfs_member_info *b)
{
        return a->id_hi == b->id_hi && a->id_lo == b->id_lo &&
            a->generation == b->generation && a->members == b->members &&
            a->tdir_member == b->tdir_member &&
            a->tdir_block == b->tdir_block &&
            a->tdir_blocks == b->tdir_blocks &&
            a->tdir_checksum == b->tdir_checksum;
}

int
tsfs_scan(unsigned int seed_unit, struct tsfs_scan_result *out)
{
        struct tsfs_member_info seed;
        struct tsfs_member_info m;
        unsigned int found;
        unsigned int unit;
        unsigned int i;

        if (out == 0 || seed_unit > 7U || tsfs_read_member(seed_unit, &seed) != 0)
                return -1;
        out->id_hi = seed.id_hi;
        out->id_lo = seed.id_lo;
        out->generation = seed.generation;
        out->members = seed.members;
        out->tdir_member = seed.tdir_member;
        out->tdir_block = seed.tdir_block;
        out->tdir_blocks = seed.tdir_blocks;
        out->tdir_checksum = seed.tdir_checksum;
        for (i = 0U; i < TSFS_MAX_MEMBERS; ++i) {
                out->unit[i] = 010U;
                out->blocks[i] = 0;
        }
        out->unit[seed.member] = seed_unit;
        out->blocks[seed.member] = seed.blocks;
        found = 1U;
        for (unit = 0U; unit < 8U && found < seed.members; ++unit) {
                if (unit == seed_unit)
                        continue;
                if (tsfs_read_member(unit, &m) != 0 || !tsfs_same_set(&seed, &m))
                        continue;
                if (out->unit[m.member] != 010U)
                        return -1;
                out->unit[m.member] = unit;
                out->blocks[m.member] = m.blocks;
                ++found;
        }
        return found == seed.members ? 0 : -1;
}

static int
table_decode(const kword_t *e, unsigned int expected_id,
    unsigned int members, struct tsfs_table_info *t)
{
        kword_t idflags;
        kword_t mb;
        kword_t shape;

        idflags = e[TE_ID_FLAGS];
        if (((idflags >> 18) & HALF18) != expected_id ||
            (idflags & HALF18) != 0)
                return -1;
        mb = e[TE_MEMBER_BLOCK];
        shape = e[TE_BLOCKS_RECWORDS];
        t->present = 1U;
        t->member = (unsigned int)((mb >> 18) & HALF18);
        t->block = (unsigned int)(mb & HALF18);
        t->blocks = (unsigned int)((shape >> 18) & HALF18);
        t->record_words = (unsigned int)(shape & HALF18);
        t->records = (unsigned int)e[TE_RECORD_COUNT];
        t->checksum = e[TE_CHECKSUM];
        if (t->member >= members || t->blocks == 0U || t->block < 3U ||
            t->block + t->blocks > TSFS_BLOCK_COUNT)
                return -1;
        return 0;
}

static int
table_checksum_ok(const struct tsfs_scan_result *scan,
    const struct tsfs_table_info *t)
{
        kword_t block[TSFS_BLOCK_WORDS];
        kword_t sum;
        unsigned int unit;
        unsigned int i;

        if (t->member >= scan->members)
                return 0;
        unit = scan->unit[t->member];
        if (unit > 7U)
                return 0;
        sum = 0;
        for (i = 0U; i < t->blocks; ++i) {
                if (dsys_dtc_read_block(unit, t->block + i, block) != 0)
                        return 0;
                sum = checksum_update(sum, block, TSFS_BLOCK_WORDS,
                    TSFS_BLOCK_WORDS);
        }
        return sum == t->checksum;
}

static int
file_table_structure_ok(const struct tsfs_scan_result *scan,
    const struct tsfs_table_info *t)
{
        kword_t block[TSFS_BLOCK_WORDS];
        unsigned int unit;
        unsigned int per_block;
        unsigned int index;
        unsigned int current_block;

        if (t->record_words != FILE_WORDS || t->records == 0U ||
            (kword_t)t->records * FILE_WORDS >
            (kword_t)t->blocks * TSFS_BLOCK_WORDS)
                return 0;
        unit = scan->unit[t->member];
        per_block = TSFS_BLOCK_WORDS / FILE_WORDS;
        current_block = 0777777U;
        for (index = 0U; index < t->records; ++index) {
                const kword_t *r;
                unsigned int b;
                unsigned int slot;
                unsigned int parent;
                unsigned int flags;
                unsigned int first;
                unsigned int count;

                b = index / per_block;
                slot = index % per_block;
                if (b != current_block) {
                        if (dsys_dtc_read_block(unit, t->block + b, block) != 0)
                                return 0;
                        current_block = b;
                }
                r = block + slot * FILE_WORDS;
                parent = (unsigned int)((r[0] >> 18) & HALF18);
                flags = (unsigned int)(r[0] & HALF18);
                if (flags != 1U && flags != 2U)
                        return 0;
                if (index == 0U) {
                        if (parent != 0U || flags != 1U)
                                return 0;
                } else if (parent >= index) {
                        return 0;
                }
                if (flags == 1U) {
                        if (r[5] != 0 || r[6] != 0)
                                return 0;
                        first = (unsigned int)((r[7] >> 18) & HALF18);
                        count = (unsigned int)(r[7] & HALF18);
                        if ((count == 0U && first != 0U) ||
                            (count != 0U && (first <= index ||
                            first + count > t->records)))
                                return 0;
                } else if (r[5] != 0 || r[6] != 0 || r[7] != 0) {
                        return 0;
                }
        }
        return 1;
}

static int
scan_metadata(const struct tsfs_scan_result *scan,
    struct tsfs_table_info *file, struct tsfs_table_info *extent)
{
        kword_t td[TSFS_BLOCK_WORDS];
        unsigned int unit;
        unsigned int i;
        unsigned int file_seen;
        unsigned int extent_seen;

        file->present = 0U;
        file->member = file->block = file->blocks = 0U;
        file->record_words = file->records = 0U;
        file->checksum = 0;
        extent->present = 0U;
        extent->member = extent->block = extent->blocks = 0U;
        extent->record_words = extent->records = 0U;
        extent->checksum = 0;
        if (scan->tdir_blocks != 1U || scan->tdir_member >= scan->members)
                return -1;
        unit = scan->unit[scan->tdir_member];
        if (unit > 7U || dsys_dtc_read_block(unit,
            (unsigned int)scan->tdir_block, td) != 0 ||
            td[TD_MAGIC] != TSDIR_MAGIC || td[TD_VERSION] != TSFS_VERSION ||
            td[TD_FLAGS] != 0 || td[TD_PATH_WORDS] != 0 ||
            td[TD_CHECKSUM] != scan->tdir_checksum ||
            tsfs_checksum(td, TD_CHECKSUM) != scan->tdir_checksum)
                return -1;
        file_seen = 0U;
        extent_seen = 0U;
        for (i = 0U; i < TD_MAX_ENTRIES; ++i) {
                const kword_t *e;
                unsigned int id;

                e = td + TD_FIRST_ENTRY + i * TD_ENTRY_WORDS;
                id = (unsigned int)((e[TE_ID_FLAGS] >> 18) & HALF18);
                if (id == 0U)
                        break;
                if (id == TABLE_FILE) {
                        if (file_seen || table_decode(e, TABLE_FILE,
                            scan->members, file) != 0)
                                return -1;
                        file_seen = 1U;
                } else if (id == TABLE_EXTENT) {
                        if (extent_seen || table_decode(e, TABLE_EXTENT,
                            scan->members, extent) != 0)
                                return -1;
                        extent_seen = 1U;
                } else {
                        return -1;
                }
        }
        if (td[TD_FILE_COUNT] == 0) {
                if (file_seen || td[TD_EXTENT_COUNT] != 0 || extent_seen)
                        return -1;
                return 0;
        }
        if (!file_seen || file->record_words != FILE_WORDS ||
            file->records != (unsigned int)td[TD_FILE_COUNT] ||
            !table_checksum_ok(scan, file) ||
            !file_table_structure_ok(scan, file))
                return -1;
        /* Extent-bearing files are deliberately the next milestone. */
        if (td[TD_EXTENT_COUNT] != 0 || extent_seen)
                return -1;
        return 0;
}

int
tsfs_build_mount_handoff(const struct tsfs_scan_result *scan,
    kword_t out[SYS_TSFS_MOUNT_WORDS])
{
        struct tsfs_table_info file;
        struct tsfs_table_info extent;
        unsigned int i;
        kword_t total;

        if (scan == 0 || out == 0 || scan->members == 0U ||
            scan->members > TSFS_MAX_MEMBERS ||
            scan->tdir_member >= scan->members ||
            scan_metadata(scan, &file, &extent) != 0)
                return -1;
        for (i = 0U; i < SYS_TSFS_MOUNT_WORDS; ++i)
                out[i] = 0;
        out[0] = (kword_t)(scan->members | (1U << 3)) << 18;
        total = 0;
        for (i = 0U; i < scan->members; ++i) {
                if (scan->unit[i] > 7U || scan->blocks[i] != TSFS_BLOCK_COUNT)
                        return -1;
                out[2U + i] = scan->unit[i];
                out[9U + i] = scan->blocks[i];
                total += scan->blocks[i];
        }
        out[1] = total;
        if (file.present) {
                out[SYS_TSFS_FILE_LOC] =
                    ((kword_t)scan->unit[file.member] << 18) | file.block;
                out[SYS_TSFS_FILE_SHAPE] =
                    ((kword_t)file.blocks << 18) | file.records;
        }
        if (extent.present) {
                out[SYS_TSFS_EXTENT_LOC] =
                    ((kword_t)scan->unit[extent.member] << 18) | extent.block;
                out[SYS_TSFS_EXTENT_SHAPE] =
                    ((kword_t)extent.blocks << 18) | extent.records;
        }
        return 0;
}
