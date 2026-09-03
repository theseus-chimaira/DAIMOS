#include "diskset_boot.h"
#include "diskset_mres.h"
#include "diskset_layout.h"
#include "dsk270.h"
#include "kinit.h"
#include "module.h"

#define DISKSET_BOOT_HALF_MASK     0777777UL
#define DISKSET_BOOT_UNUSED_HALF   0777777UL
#define DISKSET_BOOT_UNIT_SHIFT    16U
#define DISKSET_BOOT_UNIT_MASK     03U
#define DISKSET_BOOT_LOCATOR_MASK  0177777UL

static int
diskset_boot_call(struct diskset_mres_request *req)
{
        unsigned int address;

        if (req == 0)
                return -1;
        address = module_service_get(MODULE_SERVICE_DISKSET);
        if (address == 0U)
                return -1;
        return (int)kinit_call18_1(address, (kword_t)(unsigned long)req);
}

static int
diskset_boot_layout_decode(const kword_t block[DISKSET_BLOCK_WORDS],
    struct diskset_layout *layout)
{
        kword_t range;

        if (block == 0 || layout == 0 ||
            block[DISKSET_LAYOUT_MAGIC_WORD] != DISKSET_LAYOUT_MAGIC_D6FSR2)
                return -1;
        range = block[DISKSET_LAYOUT_RANGE_WORD];
        layout->base = (range >> 18) & DISKSET_BOOT_HALF_MASK;
        layout->usable_blocks = range & DISKSET_BOOT_HALF_MASK;
        layout->super_a = block[DISKSET_LAYOUT_SUPER_A];
        layout->super_b = block[DISKSET_LAYOUT_SUPER_B];
        layout->swap_tail_blocks = block[DISKSET_LAYOUT_SWAP_TAIL];
        layout->bootstream_blocks = block[DISKSET_LAYOUT_BOOTSTREAM];
        layout->logstore_start = block[DISKSET_LAYOUT_LOGSTORE_START];
        layout->logstore_blocks = block[DISKSET_LAYOUT_LOGSTORE_BLOCKS];
        layout->badmap_start = block[DISKSET_LAYOUT_BADMAP_START];
        layout->badmap_blocks = block[DISKSET_LAYOUT_BADMAP_BLOCKS];
        if (layout->usable_blocks == 0UL ||
            layout->swap_tail_blocks >= DSK270_SECTORS_PER_UNIT ||
            layout->base >= DSK270_SECTORS_PER_UNIT ||
            layout->usable_blocks > DSK270_SECTORS_PER_UNIT - layout->base ||
            layout->swap_tail_blocks > DSK270_SECTORS_PER_UNIT -
            layout->base - layout->usable_blocks ||
            layout->super_a == layout->super_b ||
            layout->super_a > DISKSET_BOOT_HALF_MASK ||
            layout->super_b > DISKSET_BOOT_HALF_MASK)
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

static kword_t
diskset_boot_half(unsigned int index)
{
        kword_t word;

        word = kinit_boot_handoff[index / 2U];
        if ((index & 1U) == 0U)
                return (word >> 18) & DISKSET_BOOT_HALF_MASK;
        return word & DISKSET_BOOT_HALF_MASK;
}

static int
diskset_boot_configure(const struct diskset *config)
{
        struct diskset *runtime;
        kword_t *totalp;
        kword_t total;
        unsigned int i;

        if (config == 0 || diskset_state_addr == 0U ||
            diskset_total_addr == 0U || config->members == 0U ||
            config->members > DISKSET_MAX_MEMBERS)
                return -1;
        total = 0UL;
        for (i = 0U; i < config->members; ++i) {
                if (config->blocks[i] == 0UL ||
                    config->unit[i] >= DSK270_UNITS ||
                    config->base[i] >= DSK270_SECTORS_PER_UNIT ||
                    config->blocks[i] > DSK270_SECTORS_PER_UNIT -
                    config->base[i] ||
                    config->swap_tail_blocks > DSK270_SECTORS_PER_UNIT -
                    config->base[i] - config->blocks[i])
                        return -1;
                total += config->blocks[i];
        }
        if (config->logstore_start > total ||
            config->logstore_blocks > total - config->logstore_start)
                return -1;

        runtime = (struct diskset *)(unsigned long)diskset_state_addr;
        totalp = (kword_t *)(unsigned long)diskset_total_addr;
        runtime->members = 0U;
        *totalp = 0UL;
        for (i = 0U; i < config->members; ++i) {
                runtime->unit[i] = config->unit[i];
                runtime->base[i] = config->base[i];
                runtime->blocks[i] = config->blocks[i];
        }
        runtime->swap_tail_blocks = config->swap_tail_blocks;
        runtime->logstore_start = config->logstore_start;
        runtime->logstore_blocks = config->logstore_blocks;
        *totalp = total;
        runtime->members = config->members;
        return 0;
}

int
diskset_boot_discover(kword_t *super_ap, kword_t *super_bp)
{
        struct diskset config;
        struct diskset_layout layout;
        kword_t descriptor[DISKSET_BLOCK_WORDS];
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
        for (index = 0U; index < DISKSET_BOOT_MEMBERS; ++index) {
                half = diskset_boot_half(index);
                if (half == DISKSET_BOOT_UNUSED_HALF)
                        continue;
                unit = (unsigned int)((half >> DISKSET_BOOT_UNIT_SHIFT) &
                    DISKSET_BOOT_UNIT_MASK);
                locator = half & DISKSET_BOOT_LOCATOR_MASK;
                if (dsk270_read_sector(unit, locator, descriptor) != 0)
                        return -1;
                if (descriptor[DISKSET_LAYOUT_MAGIC_WORD] !=
                    DISKSET_LAYOUT_MAGIC_D6FSR2)
                        return members == 0U ? 1 : -1;
                if (diskset_boot_layout_decode(descriptor, &layout) != 0)
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
                if (members >= DISKSET_MAX_MEMBERS)
                        return -1;
                config.unit[members] = unit;
                config.base[members] = layout.base;
                config.blocks[members] = layout.usable_blocks;
                ++members;
        }
        if (members == 0U)
                return -1;
        config.members = members;
        config.swap_tail_blocks = first_swap_tail;
        config.logstore_start = first_logstore_start;
        config.logstore_blocks = first_logstore_blocks;
        rc = diskset_boot_configure(&config);
        if (rc != 0)
                return rc;
        *super_ap = first_super_a;
        *super_bp = first_super_b;
        return 0;
}

static int
diskset_boot_simple(unsigned int op, kword_t logical, const void *buffer)
{
        struct diskset_mres_request req;

        req.op = (kword_t)op;
        req.a = logical;
        req.b = (kword_t)(unsigned long)buffer;
        req.c = 0UL;
        return diskset_boot_call(&req);
}

kword_t
diskset_boot_blocks(void)
{
        int rc;

        rc = diskset_boot_simple(DISKSET_MRES_OP_BLOCKS, 0UL, 0);
        return rc < 0 ? 0UL : (kword_t)rc;
}

int
diskset_boot_writable(void)
{
        return diskset_boot_simple(DISKSET_MRES_OP_WRITABLE, 0UL, 0);
}

kword_t
diskset_boot_log_blocks(void)
{
        int rc;

        rc = diskset_boot_simple(DISKSET_MRES_OP_LOG_BLOCKS, 0UL, 0);
        return rc < 0 ? 0UL : (kword_t)rc;
}

int
diskset_boot_log_read(kword_t blockno,
    kword_t block[DISKSET_BLOCK_WORDS])
{
        return diskset_boot_simple(DISKSET_MRES_OP_LOG_READ, blockno, block);
}

int
diskset_boot_log_write(kword_t blockno,
    const kword_t block[DISKSET_BLOCK_WORDS])
{
        return diskset_boot_simple(DISKSET_MRES_OP_LOG_WRITE, blockno, block);
}
