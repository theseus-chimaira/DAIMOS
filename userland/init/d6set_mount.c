/* D6SET1 transient mount scanner.  This code lives only in MOUNTALL. */
#include "u.h"
#include "d6fs.h"
#include "d6fs_provider.h"
#include "blockset_layout.h"
#include "drm236.h"
#include "fs_backing.h"

#define D6SET1_MEMBERS 4U
#define D6SET_HALF_MASK 0777777UL

static int
d6set1_layout(unsigned int unit, struct blockset_layout *lp)
{
        kword_t block[D6FS_BLOCK_WORDS];
        kword_t range;

        if (lp == 0 || unit >= D6SET1_MEMBERS ||
            dsys_drm_read_block(unit, 0U, block) != 0 ||
            block[BLOCKSET_LAYOUT_MAGIC_WORD] != BLOCKSET_LAYOUT_MAGIC_D6FSR2)
                return -1;
        range = block[BLOCKSET_LAYOUT_RANGE_WORD];
        lp->base = (range >> 18U) & D6SET_HALF_MASK;
        lp->usable_blocks = range & D6SET_HALF_MASK;
        lp->super_a = block[BLOCKSET_LAYOUT_SUPER_A];
        lp->super_b = block[BLOCKSET_LAYOUT_SUPER_B];
        lp->swap_tail_blocks = block[BLOCKSET_LAYOUT_SWAP_TAIL];
        lp->bootstream_blocks = block[BLOCKSET_LAYOUT_BOOTSTREAM];
        lp->logstore_start = block[BLOCKSET_LAYOUT_LOGSTORE_START];
        lp->logstore_blocks = block[BLOCKSET_LAYOUT_LOGSTORE_BLOCKS];
        lp->badmap_start = block[BLOCKSET_LAYOUT_BADMAP_START];
        lp->badmap_blocks = block[BLOCKSET_LAYOUT_BADMAP_BLOCKS];
        if (lp->usable_blocks == 0UL || lp->base >= DRM236_BLOCKS_PER_UNIT ||
            lp->usable_blocks > DRM236_BLOCKS_PER_UNIT - lp->base ||
            lp->swap_tail_blocks > DRM236_BLOCKS_PER_UNIT - lp->base -
            lp->usable_blocks || lp->super_a == lp->super_b)
                return -1;
        return 0;
}

static int
d6set1_read(const struct blockset_layout *lp, kword_t logical,
    kword_t block[D6FS_BLOCK_WORDS])
{
        unsigned int unit;
        kword_t rel;

        if (lp == 0 || block == 0 ||
            logical >= lp->usable_blocks * (kword_t)D6SET1_MEMBERS)
                return -1;
        unit = (unsigned int)(logical % D6SET1_MEMBERS);
        rel = logical / D6SET1_MEMBERS;
        return dsys_drm_read_block(unit, (unsigned int)(lp->base + rel), block);
}

static int
d6set1_same_layout(const struct blockset_layout *a,
    const struct blockset_layout *b)
{
        return a->base == b->base && a->usable_blocks == b->usable_blocks &&
            a->super_a == b->super_a && a->super_b == b->super_b &&
            a->swap_tail_blocks == b->swap_tail_blocks &&
            a->bootstream_blocks == b->bootstream_blocks &&
            a->logstore_start == b->logstore_start &&
            a->logstore_blocks == b->logstore_blocks &&
            a->badmap_start == b->badmap_start &&
            a->badmap_blocks == b->badmap_blocks;
}

int
d6set1_mount(kword_t *path, unsigned int flags)
{
        struct blockset_layout layout;
        struct blockset_layout other;
        struct d6fs_super_info super;
        kword_t a[D6FS_SUPER_WORDS];
        kword_t b[D6FS_SUPER_WORDS];
        kword_t handoff[D6FS_MOUNT_WORDS];
        kword_t block[D6FS_BLOCK_WORDS];
        kword_t total;
        kword_t summary_start;
        kword_t alloc_cursor;
        kword_t selector;
        unsigned int copy;
        unsigned int i;
        unsigned int unit;

        if (path == 0 || (flags != SYS_MOUNT_RW && flags != SYS_MOUNT_RDONLY) ||
            d6set1_layout(0U, &layout) != 0) {
                return -1;
        }
        for (unit = 1U; unit < D6SET1_MEMBERS; ++unit) {
                if (d6set1_layout(unit, &other) != 0 ||
                    !d6set1_same_layout(&layout, &other)) {
                        return -1;
                }
        }
        total = layout.usable_blocks * (kword_t)D6SET1_MEMBERS;
        if (layout.super_a >= total || layout.super_b >= total ||
            d6set1_read(&layout, layout.super_a, block) != 0) {
                return -1;
        }
        for (i = 0U; i < D6FS_SUPER_WORDS; ++i)
                a[i] = block[i];
        if (d6set1_read(&layout, layout.super_b, block) != 0) {
                return -1;
        }
        for (i = 0U; i < D6FS_SUPER_WORDS; ++i)
                b[i] = block[i];
        if (d6fs_super_select(a, b, total, &super, &copy) != 0) {
                return -1;
        }
        summary_start = (copy == 0U ? a : b)[D6FS_SB_SUMMARY_START];
        alloc_cursor = summary_start +
            (copy == 0U ? a : b)[D6FS_SB_SUMMARY_BLOCKS];
        if (alloc_cursor >= super.total_blocks)
                alloc_cursor = 0UL;
        for (i = 0U; i < D6FS_MOUNT_WORDS; ++i)
                handoff[i] = 0UL;
        handoff[0] = alloc_cursor;
        handoff[1] = (summary_start << D6FS_PROVIDER_SUMMARY_SHIFT) |
            (copy != 0U ? D6FS_PROVIDER_MOUNT_COPY : 0U);
        handoff[2] = super.sequence;
        handoff[3] = super.total_blocks;
        handoff[4] = super.root_fcb;
        handoff[5] = super.fcb_start;
        handoff[6] = super.fcb_count;
        handoff[7] = super.freemap_start;
        handoff[8] = super.freemap_blocks;
        handoff[9] = layout.super_a;
        handoff[10] = layout.super_b;
        handoff[D6FS_MOUNT_MARKER] = D6FS_MOUNT_MAGIC;
        selector = FS_BACKING_DIRECT_DRM_TAG | FS_BACKING_DIRECT_SET_TAG | 017UL;
        handoff[014] = (selector << 18U) | layout.base;
        handoff[D6FS_MOUNT_BACKING_BLOCKS] = super.total_blocks;
        i = (unsigned int)dsys_d6fs_mount(handoff, path, flags);
        return (int)i;
}
