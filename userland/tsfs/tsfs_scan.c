#include "tsfs_scan.h"

#define MASK36 0777777777777UL
#define HALF18 0777777UL
#define TSFS_MAGIC 0646346630000UL
#define TSDIR_MAGIC 0646344516200UL
#define TSFS_VERSION (((kword_t)1U << 18) | 1U)

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
