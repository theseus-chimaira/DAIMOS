#include "blockset_boot.h"

/* Compact installed root descriptor; the generic mapper keeps the public
 * seven-member descriptor layout from blockset.h. */
#define BLOCKSET_ROOT_DESC_WORDS 10U
#define BLOCKSET_ROOT_DESC_FLAGS 0U
#define BLOCKSET_ROOT_DESC_TOTAL 1U
#define BLOCKSET_ROOT_DESC_UNIT0 2U
#define BLOCKSET_ROOT_DESC_RANGE0 6U
#include "blockset_mres.h"
#include "blockset_layout.h"
#include "dsk270.h"
#include "kinit.h"
#include "module.h"
#include "monitorfs.h"
#include "fs_mres.h"

#define BLOCKSET_BOOT_HALF_MASK     0777777UL
#define BLOCKSET_BOOT_UNUSED_HALF   0777777UL
#define BLOCKSET_BOOT_UNIT_SHIFT    16U
#define BLOCKSET_BOOT_UNIT_MASK     03U
#define BLOCKSET_BOOT_LOCATOR_MASK  0177777UL

/* Reclaimable KINIT copy.  A singleton does not install BLOCKSET, so boot
 * D6FS/LOGSTORE I/O still needs its validated mapping here. */
static struct blockset blockset_boot_state;
static kword_t blockset_boot_total;
static kword_t blockset_boot_log_start;
static kword_t blockset_boot_log_count;

static kword_t
blockset_boot_half(unsigned int index)
{
        kword_t word;

        word = kinit_boot_handoff[index / 2U];
        if ((index & 1U) == 0U)
                return (word >> 18) & BLOCKSET_BOOT_HALF_MASK;
        return word & BLOCKSET_BOOT_HALF_MASK;
}

unsigned int
blockset_boot_member_count_hint(void)
{
        unsigned int index;
        unsigned int members;

        members = 0U;
        for (index = 0U; index < BLOCKSET_BOOT_MEMBERS; ++index)
                if (blockset_boot_half(index) != BLOCKSET_BOOT_UNUSED_HALF)
                        ++members;
        return members;
}

static int
blockset_boot_layout_decode(const kword_t block[BLOCKSET_BLOCK_WORDS],
    struct blockset_layout *layout)
{
        kword_t range;

        if (block == 0 || layout == 0 ||
            block[BLOCKSET_LAYOUT_MAGIC_WORD] != BLOCKSET_LAYOUT_MAGIC_D6FSR2)
                return -1;
        range = block[BLOCKSET_LAYOUT_RANGE_WORD];
        layout->base = (range >> 18) & BLOCKSET_BOOT_HALF_MASK;
        layout->usable_blocks = range & BLOCKSET_BOOT_HALF_MASK;
        layout->super_a = block[BLOCKSET_LAYOUT_SUPER_A];
        layout->super_b = block[BLOCKSET_LAYOUT_SUPER_B];
        layout->swap_tail_blocks = block[BLOCKSET_LAYOUT_SWAP_TAIL];
        layout->bootstream_blocks = block[BLOCKSET_LAYOUT_BOOTSTREAM];
        layout->logstore_start = block[BLOCKSET_LAYOUT_LOGSTORE_START];
        layout->logstore_blocks = block[BLOCKSET_LAYOUT_LOGSTORE_BLOCKS];
        layout->badmap_start = block[BLOCKSET_LAYOUT_BADMAP_START];
        layout->badmap_blocks = block[BLOCKSET_LAYOUT_BADMAP_BLOCKS];
        if (layout->usable_blocks == 0UL ||
            layout->swap_tail_blocks >= DSK270_SECTORS_PER_UNIT ||
            layout->base >= DSK270_SECTORS_PER_UNIT ||
            layout->usable_blocks > DSK270_SECTORS_PER_UNIT - layout->base ||
            layout->swap_tail_blocks > DSK270_SECTORS_PER_UNIT -
            layout->base - layout->usable_blocks ||
            layout->super_a == layout->super_b ||
            layout->super_a > BLOCKSET_BOOT_HALF_MASK ||
            layout->super_b > BLOCKSET_BOOT_HALF_MASK)
                return -1;
        if (layout->bootstream_blocks != 0UL || layout->logstore_start != 0UL ||
            layout->logstore_blocks != 0UL || layout->badmap_start != 0UL ||
            layout->badmap_blocks != 0UL) {
                if (layout->logstore_start != layout->bootstream_blocks ||
                    layout->badmap_start != layout->logstore_start +
                    layout->logstore_blocks ||
                    layout->super_a != layout->badmap_start +
                    layout->badmap_blocks ||
                    layout->super_b != layout->super_a + 1UL)
                        return -1;
        }
        return 0;
}

static int
blockset_boot_configure(const struct blockset *config)
{
        kword_t *runtime;
        kword_t total;
        unsigned int i;

        if (config == 0 || config->members == 0U ||
            config->members > BLOCKSET_MAX_MEMBERS ||
            config->policy != BLOCKSET_POLICY_INTERLEAVE)
                return -1;
        total = 0UL;
        for (i = 0U; i < config->members; ++i) {
                if (config->blocks[i] == 0UL ||
                    (i != 0U && config->policy == BLOCKSET_POLICY_INTERLEAVE &&
                    config->blocks[i] != config->blocks[0]) ||
                    config->unit[i] >= DSK270_UNITS ||
                    config->base[i] >= DSK270_SECTORS_PER_UNIT ||
                    config->blocks[i] > DSK270_SECTORS_PER_UNIT -
                    config->base[i] ||
                    config->tail_blocks > DSK270_SECTORS_PER_UNIT -
                    config->base[i] - config->blocks[i])
                        return -1;
                total += config->blocks[i];
        }

        blockset_boot_state.members = 0U;
        blockset_boot_state.policy = config->policy;
        blockset_boot_total = 0UL;
        for (i = 0U; i < config->members; ++i) {
                blockset_boot_state.unit[i] = config->unit[i];
                blockset_boot_state.base[i] = config->base[i];
                blockset_boot_state.blocks[i] = config->blocks[i];
        }
        blockset_boot_state.tail_blocks = config->tail_blocks;
        blockset_boot_total = total;
        blockset_boot_state.members = config->members;

        if (config->members == 1U) {
                blockset_direct_configure(config->unit[0], config->base[0],
                    config->blocks[0], config->tail_blocks);
                return 0;
        }
        blockset_direct_configure(0U, 0UL, 0UL,
            config->tail_blocks * (kword_t)config->members);
        if (blockset_state_addr == 0U ||
            module_service_get(MODULE_SERVICE_BLOCKSET) == 0U)
                return -1;
        runtime = (kword_t *)(unsigned long)blockset_state_addr;
        for (i = 0U; i < BLOCKSET_ROOT_DESC_WORDS; ++i)
                runtime[i] = 0UL;
        runtime[BLOCKSET_ROOT_DESC_TOTAL] = total;
        for (i = 0U; i < config->members; ++i) {
                runtime[BLOCKSET_ROOT_DESC_UNIT0 + i] =
                    (kword_t)config->unit[i];
                runtime[BLOCKSET_ROOT_DESC_RANGE0 + i] =
                    (config->base[i] << 18U) |
                    (config->base[i] + config->blocks[i]);
        }
        runtime[BLOCKSET_ROOT_DESC_FLAGS] =
            ((kword_t)(config->members |
            (config->policy << BLOCKSET_DESC_POLICY_SHIFT)) << 18U) |
            config->tail_blocks;
        return 0;
}

int
blockset_boot_discover(kword_t *super_ap, kword_t *super_bp)
{
        struct blockset config;
        struct blockset_layout layout;
        kword_t descriptor[BLOCKSET_BLOCK_WORDS];
        unsigned int index;
        unsigned int members;
        unsigned int unit;
        kword_t first_super_a;
        kword_t first_super_b;
        kword_t first_swap_tail;
        kword_t first_bootstream;
        kword_t first_logstore_start;
        kword_t first_logstore_blocks;
        kword_t first_badmap_start;
        kword_t first_badmap_blocks;
        kword_t half;
        kword_t locator;
        int rc;

        if (super_ap == 0 || super_bp == 0)
                return -1;
        members = 0U;
        first_super_a = 0UL;
        first_super_b = 0UL;
        first_swap_tail = 0UL;
        first_bootstream = 0UL;
        first_logstore_start = 0UL;
        first_logstore_blocks = 0UL;
        first_badmap_start = 0UL;
        first_badmap_blocks = 0UL;
        for (index = 0U; index < BLOCKSET_BOOT_MEMBERS; ++index) {
                half = blockset_boot_half(index);
                if (half == BLOCKSET_BOOT_UNUSED_HALF)
                        continue;
                unit = (unsigned int)((half >> BLOCKSET_BOOT_UNIT_SHIFT) &
                    BLOCKSET_BOOT_UNIT_MASK);
                locator = half & BLOCKSET_BOOT_LOCATOR_MASK;
                if (dsk270_read_sector(unit, locator, descriptor) != 0)
                        return -1;
                if (descriptor[BLOCKSET_LAYOUT_MAGIC_WORD] !=
                    BLOCKSET_LAYOUT_MAGIC_D6FSR2)
                        return members == 0U ? 1 : -1;
                if (blockset_boot_layout_decode(descriptor, &layout) != 0)
                        return -1;
                if (members == 0U) {
                        first_super_a = layout.super_a;
                        first_super_b = layout.super_b;
                        first_swap_tail = layout.swap_tail_blocks;
                        first_bootstream = layout.bootstream_blocks;
                        first_logstore_start = layout.logstore_start;
                        first_logstore_blocks = layout.logstore_blocks;
                        first_badmap_start = layout.badmap_start;
                        first_badmap_blocks = layout.badmap_blocks;
                } else if (layout.super_a != first_super_a ||
                    layout.super_b != first_super_b ||
                    layout.swap_tail_blocks != first_swap_tail ||
                    layout.bootstream_blocks != first_bootstream ||
                    layout.logstore_start != first_logstore_start ||
                    layout.logstore_blocks != first_logstore_blocks ||
                    layout.badmap_start != first_badmap_start ||
                    layout.badmap_blocks != first_badmap_blocks) {
                        return -1;
                }
                if (members >= BLOCKSET_MAX_MEMBERS)
                        return -1;
                config.unit[members] = unit;
                config.base[members] = layout.base;
                config.blocks[members] = layout.usable_blocks;
                ++members;
        }
        if (members == 0U)
                return -1;
        config.members = members;
        config.policy = BLOCKSET_POLICY_INTERLEAVE;
        config.tail_blocks = first_swap_tail;
        blockset_boot_log_start = first_logstore_start;
        blockset_boot_log_count = first_logstore_blocks;
        rc = blockset_boot_configure(&config);
        if (rc != 0)
                return rc;
        {
                kword_t packed;

                packed = (kword_t)members;
                for (index = 0U; index < members; ++index)
                        packed |= ((kword_t)config.unit[index] & 07UL) <<
                            (3U + 3U * index);
                mfsdev_d6set_members = packed;
        }
        *super_ap = first_super_a;
        *super_bp = first_super_b;
        return 0;
}

int
blockset_boot_read(kword_t blockno, kword_t block[BLOCKSET_BLOCK_WORDS])
{
        if (block == 0 || blockno >= blockset_boot_total)
                return -1;
        if (blockset_boot_state.members == 1U)
                return dsk270_read_sector(blockset_boot_state.unit[0],
                    blockset_boot_state.base[0] + blockno, block);
        if (blockset_read_addr == 0U)
                return -1;
        return (int)kinit_call_blockset_io(blockset_read_addr, blockno, block);
}

kword_t
blockset_boot_blocks(void)
{
        return blockset_boot_total;
}

int
blockset_boot_writable(void)
{
        if (blockset_boot_state.members == 0U)
                return 0;
        if (blockset_boot_state.members == 1U)
                return module_service_get(MODULE_SERVICE_DSK_WRITE_SECTOR) != 0U;
        return blockset_write_addr != 0U;
}

int
blockset_boot_write(kword_t blockno,
    const kword_t block[BLOCKSET_BLOCK_WORDS])
{
        if (block == 0 || blockno >= blockset_boot_total)
                return -1;
        if (blockset_boot_state.members == 1U)
                return dsk270_write_sector(blockset_boot_state.unit[0],
                    blockset_boot_state.base[0] + blockno, block);
        if (blockset_write_addr == 0U)
                return -1;
        return (int)kinit_call_blockset_io(blockset_write_addr, blockno,
            (void *)block);
}

int
blockset_boot_direct(unsigned int *unitp, kword_t *basep,
    kword_t *blocksp, kword_t *tailp)
{
        if (blockset_boot_state.members != 1U || unitp == 0 || basep == 0 ||
            blocksp == 0 || tailp == 0)
                return 0;
        *unitp = blockset_boot_state.unit[0];
        *basep = blockset_boot_state.base[0];
        *blocksp = blockset_boot_state.blocks[0];
        *tailp = blockset_boot_state.tail_blocks;
        return 1;
}

kword_t
blockset_boot_log_blocks(void)
{
        return blockset_boot_log_count;
}

int
blockset_boot_log_read(kword_t blockno,
    kword_t block[BLOCKSET_BLOCK_WORDS])
{
        if (blockno >= blockset_boot_log_count)
                return -1;
        return blockset_boot_read(blockset_boot_log_start + blockno, block);
}

int
blockset_boot_log_write(kword_t blockno,
    const kword_t block[BLOCKSET_BLOCK_WORDS])
{
        if (blockno >= blockset_boot_log_count)
                return -1;
        return blockset_boot_write(blockset_boot_log_start + blockno, block);
}
