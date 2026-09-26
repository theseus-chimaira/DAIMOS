#if KINIT_FULL
#include "tsfs_boot.h"
#include "kinit.h"
#include "module.h"
#include "fs_mres.h"
#include "tsfs.h"

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
#define D_USED_WORDS 12U
#define DESC_PRIMARY 1U
#define DESC_BACKUP 2U
#define MAX_MEMBERS 7U

#define TD_MAGIC 0U
#define TD_VERSION 1U
#define TD_FLAGS 2U
#define TD_FILE_COUNT 3U
#define TD_EXTENT_COUNT 4U
#define TD_PATH_WORDS 5U
#define TD_CHECKSUM 6U
#define TD_FIRST_ENTRY 7U
#define TD_ENTRY_WORDS 5U
#define TD_MAX_ENTRIES ((TSFS_BLOCKS_PER_MEMBER - TD_FIRST_ENTRY) / TD_ENTRY_WORDS)

#define TE_ID_FLAGS 0U
#define TE_MEMBER_BLOCK 1U
#define TE_BLOCKS_RECWORDS 2U
#define TE_RECORD_COUNT 3U
#define TE_CHECKSUM 4U
#define TABLE_FILE 1U
#define TABLE_EXTENT 2U
#define EXTENT_WORDS 4U

struct member_info {
        kword_t id_hi;
        kword_t id_lo;
        kword_t generation;
        unsigned int members;
        unsigned int member;
        unsigned int tdir_member;
        unsigned int tdir_block;
        kword_t tdir_checksum;
};

struct scan_info {
        struct member_info seed;
        unsigned int unit[MAX_MEMBERS];
};

struct table_info {
        unsigned int present;
        unsigned int member;
        unsigned int block;
        unsigned int blocks;
        unsigned int record_words;
        unsigned int records;
        kword_t checksum;
};

/* Reclaimable KINIT scratch/state. */
static kword_t tsfs_block[0200];
static kword_t tsfs_handoff[3];
static int tsfs_selected;

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
        return checksum_update(0UL, p, 0200U, skip);
}

static int
decode_member(const kword_t *p, struct member_info *m)
{
        kword_t member;
        unsigned int i;

        if (p[D_MAGIC] != TSFS_MAGIC || p[D_VERSION] != TSFS_VERSION ||
            p[D_FLAGS] != 0UL || p[D_CHECKSUM] != tsfs_checksum(p, D_CHECKSUM))
                return -1;
        for (i = D_USED_WORDS; i < 0200U; ++i)
                if (p[i] != 0UL)
                        return -1;
        member = p[D_MEMBER];
        m->members = (unsigned int)((member >> 18) & HALF18);
        m->member = (unsigned int)(member & HALF18);
        if (m->members == 0U || m->members > MAX_MEMBERS ||
            m->member >= m->members || p[D_BLOCKS] != TSFS_BLOCKS_PER_MEMBER)
                return -1;
        m->id_hi = p[D_ID_HI];
        m->id_lo = p[D_ID_LO];
        m->generation = p[D_GENERATION];
        m->tdir_member = (unsigned int)(p[D_TDIR] >> 18);
        m->tdir_block = (unsigned int)(p[D_TDIR] & HALF18);
        m->tdir_checksum = p[D_TDIR_CHECKSUM];
        if (m->tdir_member >= m->members || m->tdir_block < 3U ||
            p[D_TDIR_BLOCKS] != 1UL || m->tdir_block >= TSFS_BLOCKS_PER_MEMBER)
                return -1;
        return 0;
}

static int
same_member(const struct member_info *a, const struct member_info *b)
{
        return a->id_hi == b->id_hi && a->id_lo == b->id_lo &&
            a->generation == b->generation && a->members == b->members &&
            a->member == b->member && a->tdir_member == b->tdir_member &&
            a->tdir_block == b->tdir_block &&
            a->tdir_checksum == b->tdir_checksum;
}

static int
same_set(const struct member_info *a, const struct member_info *b)
{
        return a->id_hi == b->id_hi && a->id_lo == b->id_lo &&
            a->generation == b->generation && a->members == b->members &&
            a->tdir_member == b->tdir_member &&
            a->tdir_block == b->tdir_block &&
            a->tdir_checksum == b->tdir_checksum;
}

static int
read_block(unsigned int unit, unsigned int block)
{
        unsigned int service;

        service = module_service_get(MODULE_SERVICE_DTC_READ_BLOCK);
        if (service == 0U)
                return -1;
        return (int)kinit_call_storage_io(service, unit, (kword_t)block,
            tsfs_block);
}

static int
read_member(unsigned int unit, struct member_info *m)
{
        struct member_info a;
        struct member_info b;
        int va;
        int vb;

        va = read_block(unit, DESC_PRIMARY) == 0 &&
            decode_member(tsfs_block, &a) == 0;
        vb = read_block(unit, DESC_BACKUP) == 0 &&
            decode_member(tsfs_block, &b) == 0;
        if (!va && !vb)
                return -1;
        if (va && vb && !same_member(&a, &b))
                return -1;
        *m = va ? a : b;
        return 0;
}

static int
scan_set(unsigned int seed_unit, struct scan_info *scan)
{
        struct member_info m;
        unsigned int found;
        unsigned int i;
        unsigned int unit;

        if (read_member(seed_unit, &scan->seed) != 0)
                return -1;
        for (i = 0U; i < MAX_MEMBERS; ++i)
                scan->unit[i] = 010U;
        scan->unit[scan->seed.member] = seed_unit;
        found = 1U;
        for (unit = 0U; unit < 8U && found < scan->seed.members; ++unit) {
                if (unit == seed_unit)
                        continue;
                if (read_member(unit, &m) != 0 || !same_set(&scan->seed, &m))
                        continue;
                if (scan->unit[m.member] != 010U)
                        return -1;
                scan->unit[m.member] = unit;
                ++found;
        }
        return found == scan->seed.members ? 0 : -1;
}

static int
decode_table(const kword_t *e, unsigned int id, unsigned int members,
    struct table_info *t)
{
        kword_t mb;
        kword_t shape;

        if (((e[TE_ID_FLAGS] >> 18) & HALF18) != id ||
            (e[TE_ID_FLAGS] & HALF18) != 0UL)
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
            t->block + t->blocks > TSFS_BLOCKS_PER_MEMBER)
                return -1;
        return 0;
}

static int
table_checksum_ok(const struct scan_info *scan, const struct table_info *t)
{
        kword_t sum;
        unsigned int i;
        unsigned int unit;

        if (t->member >= scan->seed.members)
                return 0;
        unit = scan->unit[t->member];
        if (unit > 7U)
                return 0;
        sum = 0UL;
        for (i = 0U; i < t->blocks; ++i) {
                if (read_block(unit, t->block + i) != 0)
                        return 0;
                sum = checksum_update(sum, tsfs_block, 0200U, 0200U);
        }
        return sum == t->checksum;
}

static int
scan_metadata(const struct scan_info *scan, struct table_info *file,
    struct table_info *extent)
{
        const kword_t *e;
        unsigned int i;
        unsigned int file_seen;
        unsigned int extent_seen;
        unsigned int file_count;
        unsigned int extent_count;
        unsigned int unit;

        file->present = 0U;
        extent->present = 0U;
        unit = scan->unit[scan->seed.tdir_member];
        if (unit > 7U || read_block(unit, scan->seed.tdir_block) != 0 || tsfs_block[TD_MAGIC] != TSDIR_MAGIC ||
            tsfs_block[TD_VERSION] != TSFS_VERSION ||
            tsfs_block[TD_FLAGS] != 0UL || tsfs_block[TD_PATH_WORDS] != 0UL ||
            tsfs_block[TD_CHECKSUM] != scan->seed.tdir_checksum ||
            tsfs_checksum(tsfs_block, TD_CHECKSUM) != scan->seed.tdir_checksum)
                return -1;
        file_count = (unsigned int)tsfs_block[TD_FILE_COUNT];
        extent_count = (unsigned int)tsfs_block[TD_EXTENT_COUNT];
        file_seen = 0U;
        extent_seen = 0U;
        for (i = 0U; i < (0200U - TD_FIRST_ENTRY) / TD_ENTRY_WORDS; ++i) {
                unsigned int id;

                e = tsfs_block + TD_FIRST_ENTRY + i * TD_ENTRY_WORDS;
                id = (unsigned int)((e[TE_ID_FLAGS] >> 18) & HALF18);
                if (id == 0U)
                        break;
                if (id == TABLE_FILE) {
                        if (file_seen || decode_table(e, TABLE_FILE,
                            scan->seed.members, file) != 0)
                                return -1;
                        file_seen = 1U;
                } else if (id == TABLE_EXTENT) {
                        if (extent_seen || decode_table(e, TABLE_EXTENT,
                            scan->seed.members, extent) != 0)
                                return -1;
                        extent_seen = 1U;
                } else {
                        return -1;
                }
        }
        if (!file_seen || file->record_words != TSFS_FILE_WORDS ||
            file->records != file_count || !table_checksum_ok(scan, file))
                return -1;
        if (extent_count == 0U) {
                if (extent_seen)
                        return -1;
        } else if (!extent_seen || extent->record_words != EXTENT_WORDS ||
            extent->records != extent_count ||
            !table_checksum_ok(scan, extent)) {
                return -1;
        }
        return 0;
}

static int
build_handoff(const struct scan_info *scan)
{
        struct table_info file;
        struct table_info extent;
        kword_t member_map;
        kword_t hi;
        unsigned int i;

        if (scan_metadata(scan, &file, &extent) != 0 || !file.present ||
            file.records >= 0100000U || scan->unit[file.member] > 7U)
                return -1;
        hi = ((kword_t)scan->unit[file.member] << 15) | file.records;
        tsfs_handoff[0] = (hi << 18) | file.block;
        member_map = 0UL;
        for (i = 0U; i < scan->seed.members; ++i) {
                if (scan->unit[i] > 7U)
                        return -1;
                member_map |= (kword_t)scan->unit[i] << (3U * i);
        }
        tsfs_handoff[1] = member_map;
        tsfs_handoff[2] = 0UL;
        if (extent.present) {
                if (extent.records >= 0100000U || scan->unit[extent.member] > 7U)
                        return -1;
                hi = ((kword_t)scan->unit[extent.member] << 15) | extent.records;
                tsfs_handoff[2] = (hi << 18) | extent.block;
        }
        return 0;
}

int
tsfs_boot_select(unsigned int ordinal)
{
        struct member_info seed;
        struct scan_info scan;
        unsigned int found;
        unsigned int unit;

        tsfs_selected = 0;
        found = 0U;
        for (unit = 0U; unit < 8U; ++unit) {
                if (read_member(unit, &seed) != 0 || seed.member != 0U)
                        continue;
                if (scan_set(unit, &scan) != 0 || build_handoff(&scan) != 0)
                        continue;
                if (found++ != ordinal)
                        continue;
                tsfs_selected = 1;
                return 0;
        }
        return -1;
}

int
tsfs_boot_mount_root(void)
{
        struct fs_mres_request req;
        unsigned int service;
        vnode_t root;

        if (!tsfs_selected)
                return -1;
        service = (unsigned int)(fs_tsfs_service_jump & 0777777UL);
        if (service == 0U)
                return -1;
        root = VFS_NODE_NONE;
        req.op = FS_MRES_OP_MOUNT_UNIT;
        req.a = (kword_t)(unsigned long)tsfs_handoff;
        req.b = VFS_NODE_NONE;
        req.c = VFS_MOUNT_RDONLY;
        req.d = (kword_t)(unsigned long)&root;
        req.e = 0UL;
        if ((int)kinit_call_fs_request(service, &req) != 0 ||
            root == VFS_NODE_NONE)
                return -1;
        return 0;
}

#else
typedef int tsfs_lowmem_unit;
#endif
