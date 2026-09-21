#include "d6fs.h"

static int
d6fs_optional_range_valid(kword_t start, kword_t blocks, kword_t total)
{
        if (blocks == 0UL)
                return start == 0UL;
        return start < total && blocks <= total - start;
}

static int
d6fs_ranges_overlap(kword_t astart, kword_t ablocks,
    kword_t bstart, kword_t bblocks)
{
        if (ablocks == 0UL || bblocks == 0UL)
                return 0;
        return astart < bstart + bblocks && bstart < astart + ablocks;
}

int
d6fs_super_decode(const kword_t sb[D6FS_SUPER_WORDS],
    struct d6fs_super_info *info)
{
        if (sb == 0 || info == 0)
                return -1;
        info->sequence = sb[D6FS_SB_SEQUENCE];
        info->state = (unsigned int)sb[D6FS_SB_STATE];
        info->fs_uuid[0] = sb[D6FS_SB_FS_UUID0];
        info->fs_uuid[1] = sb[D6FS_SB_FS_UUID1];
        info->total_blocks = sb[D6FS_SB_TOTAL_BLOCKS];
        info->root_fcb = (unsigned int)sb[D6FS_SB_ROOT_FCB];
        info->fcb_start = sb[D6FS_SB_FCB_START];
        info->fcb_count = (unsigned int)sb[D6FS_SB_FCB_COUNT];
        info->freemap_start = sb[D6FS_SB_FREEMAP_START];
        info->freemap_blocks = sb[D6FS_SB_FREEMAP_BLOCKS];
        return 0;
}

static int
d6fs_uuid_equal(const kword_t a[2], const kword_t b[2])
{
        return a[0] == b[0] && a[1] == b[1];
}

static int
d6fs_range_valid(kword_t start, kword_t blocks, kword_t total)
{
        return blocks != 0UL && start < total && blocks <= total - start;
}

int
d6fs_super_valid(const kword_t sb[D6FS_SUPER_WORDS],
    kword_t blockset_blocks)
{
        struct d6fs_super_info info;
        kword_t fcb_blocks;
        kword_t high;
        kword_t swap_start;
        kword_t swap_blocks;
        kword_t log_start;
        kword_t log_blocks;
        kword_t summary_start;
        kword_t summary_blocks;
        kword_t magic_version;

        if (sb == 0 || blockset_blocks == 0UL ||
            d6fs_super_decode(sb, &info) != 0)
                return 0;
        high = sb[D6FS_SB_RESERVATION_HIGH];
        swap_start = sb[D6FS_SB_SWAP_RESERVATION] >>
            D6FS_RESERVATION_START_SHIFT;
        swap_blocks = (((high >> D6FS_RES_SWAP_HI_SHIFT) &
            D6FS_RESERVATION_LEN_HIGH_MASK) << 12U) |
            (sb[D6FS_SB_SWAP_RESERVATION] & D6FS_RESERVATION_LEN_LOW_MASK);
        log_start = sb[D6FS_SB_LOG_RESERVATION] >>
            D6FS_RESERVATION_START_SHIFT;
        log_blocks = (((high >> D6FS_RES_LOG_HI_SHIFT) &
            D6FS_RESERVATION_LEN_HIGH_MASK) << 12U) |
            (sb[D6FS_SB_LOG_RESERVATION] & D6FS_RESERVATION_LEN_LOW_MASK);
        summary_start = sb[D6FS_SB_SUMMARY_START];
        summary_blocks = sb[D6FS_SB_SUMMARY_BLOCKS];
        magic_version = (D6FS_MAGIC & ~077UL) | D6FS_FORMAT_VERSION;
        if (sb[D6FS_SB_MAGIC_VERSION] != magic_version ||
            info.state > D6FS_STATE_DIRTY || info.total_blocks == 0UL ||
            info.total_blocks > blockset_blocks ||
            info.total_blocks > D6FS_LOGICAL_BLOCK_MASK + 1UL ||
            (high & D6FS_RESERVATION_RESERVED_MASK) != 0UL ||
            (swap_blocks == 0UL ? swap_start != 0UL :
            (swap_start != blockset_blocks ||
            swap_blocks > D6FS_LOGICAL_BLOCK_MASK + 1UL - swap_start)) ||
            info.fcb_count == 0U || info.fcb_count > D6FS_FCB_MASK ||
            info.root_fcb >= info.fcb_count ||
            !d6fs_range_valid(info.fcb_start,
            ((kword_t)info.fcb_count * D6FS_FCB_WORDS +
            D6FS_BLOCK_WORDS - 1UL) / D6FS_BLOCK_WORDS,
            info.total_blocks) ||
            !d6fs_range_valid(info.freemap_start, info.freemap_blocks,
            info.total_blocks) ||
            !d6fs_range_valid(summary_start, summary_blocks,
            info.total_blocks) ||
            !d6fs_optional_range_valid(log_start, log_blocks,
            info.total_blocks))
                return 0;
        fcb_blocks = ((kword_t)info.fcb_count * D6FS_FCB_WORDS +
            D6FS_BLOCK_WORDS - 1UL) / D6FS_BLOCK_WORDS;
        if (d6fs_ranges_overlap(log_start, log_blocks,
            info.fcb_start, fcb_blocks) ||
            d6fs_ranges_overlap(log_start, log_blocks,
            info.freemap_start, info.freemap_blocks) ||
            d6fs_ranges_overlap(log_start, log_blocks,
            summary_start, summary_blocks))
                return 0;
        return 1;
}

int
d6fs_super_select(const kword_t a[D6FS_SUPER_WORDS],
    const kword_t b[D6FS_SUPER_WORDS], kword_t blockset_blocks,
    struct d6fs_super_info *info, unsigned int *copyp)
{
        struct d6fs_super_info ai;
        struct d6fs_super_info bi;
        unsigned int i;
        int av;
        int bv;

        if (a == 0 || b == 0 || info == 0 || copyp == 0)
                return -1;
        av = d6fs_super_valid(a, blockset_blocks);
        bv = d6fs_super_valid(b, blockset_blocks);
        if (!av && !bv)
                return -1;
        if (av && d6fs_super_decode(a, &ai) != 0)
                return -1;
        if (bv && d6fs_super_decode(b, &bi) != 0)
                return -1;
        if (av && bv) {
                if (!d6fs_uuid_equal(ai.fs_uuid, bi.fs_uuid))
                        return -1;
                if (ai.sequence == bi.sequence) {
                        for (i = 0U; i < D6FS_SUPER_WORDS; ++i)
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
