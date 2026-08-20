#include "kinit.h"
#include "bootinfo_hdd_v1.h"
#include "dboot_v1.h"
#include "dsk270.h"
#include "dct136.h"
#include "mres.h"
#include "mres_reloc.h"
#include "mres_driver.h"
#include "mres_io.h"
#include "mres_device.h"
#include "mres_vfs.h"
#include "mres_fs.h"
#include "mres_core.h"
#include "mres_cold.h"
#ifndef DAIMON_RUNTIME_MRES
#include "mres_layout.h"
#endif
#include "mem.h"
#include "machine.h"
#include "daimod_ids.h"
#include "errno.h"
#ifdef __PDP10__
#include "architecture/dec36/pdp10/io.h"
#endif

#ifdef DAIMON_KINIT_STAGE1_SCRATCH
#define kboot_source   KINIT_STAGE1_SCRATCH->ks_boot_source
#define kboot_fs_map   KINIT_STAGE1_SCRATCH->ks_fs_map
#define kboot_sector   KINIT_STAGE1_SCRATCH->ks_sector
#define kboot_bad_run  KINIT_STAGE1_SCRATCH->ks_bad_run
#else
static struct kinit_boot_source kboot_source;
static struct d6stripe_map kboot_fs_map;
static kword_t kboot_sector[DBOOT_SECTOR_WORDS];
static kword_t kboot_bad_run[DBOOT_MAX_BAD_RUNS];
#endif

#define KBOOT_HALF_MASK       0777777UL
#define KBOOT_UNIT_SHIFT      18U
#define KBOOT_MAX_DB1         DBOOT_MAX_BAD_RUNS

#ifdef __PDP10__
/* First words of the contiguous native MRES call-gate runs. */
extern kword_t kinit_gate_mres_io_kcc_map;
extern kword_t kinit_gate_mdrv_dtc_rewind;
extern kword_t kinit_gate_mdev_drv_regfam;
extern kword_t kinit_gate_mvfs_vfs_init;
#endif

static int
kboot_bind_io_gates(kword_t io_base)
{
#ifdef __PDP10__
        kword_t *gate;
        unsigned int i;
        int error;

        if (io_base > MRESR_ADDR18_MASK ||
            io_base + MRES_IO_VECTOR_FIRST + MRES_IO_VECTOR_WORDS - 1U >
            MRESR_ADDR18_MASK)
                return -EINVAL;
        gate = &kinit_gate_mres_io_kcc_map;
        for (i = 0U; i < MRES_IO_VECTOR_WORDS; i++) {
                error = mresr_patch_addr18(&gate[i],
                    io_base + MRES_IO_VECTOR_FIRST + i);
                if (error != 0)
                        return -EINVAL;
        }
#else
        (void)io_base;
#endif
        return 0;
}

static int
kboot_bind_device_gates(kword_t device_base)
{
#ifdef __PDP10__
        kword_t *gate;
        unsigned int i;
        int error;

        if (device_base > MRESR_ADDR18_MASK ||
            device_base + MRES_DEV_VECTOR_WORDS - 1U > MRESR_ADDR18_MASK)
                return -EINVAL;
        gate = &kinit_gate_mdev_drv_regfam;
        for (i = 0U; i < MRES_DEV_VECTOR_WORDS; i++) {
                error = mresr_patch_addr18(&gate[i], device_base + i);
                if (error != 0)
                        return -EINVAL;
        }
#else
        (void)device_base;
#endif
        return 0;
}

static int
kboot_bind_vfs_gates(kword_t vfs_base)
{
#ifdef __PDP10__
        kword_t *gate;
        unsigned int i;
        int error;

        if (vfs_base > MRESR_ADDR18_MASK ||
            vfs_base + MRES_VFS_VECTOR_WORDS - 1U > MRESR_ADDR18_MASK)
                return -EINVAL;
        gate = &kinit_gate_mvfs_vfs_init;
        for (i = 0U; i < MRES_VFS_VECTOR_WORDS; i++) {
                error = mresr_patch_addr18(&gate[i], vfs_base + i);
                if (error != 0)
                        return -EINVAL;
        }
#else
        (void)vfs_base;
#endif
        return 0;
}

static int
kboot_bind_vfs_fs_gates(kword_t vfs_base, kword_t fs_base)
{
#ifdef __PDP10__
        kword_t *gate;
        unsigned int i;
        int error;

        if (vfs_base > MRESR_ADDR18_MASK || fs_base > MRESR_ADDR18_MASK ||
            vfs_base + MRES_VFS_FS_GATE_FIRST + MRES_VFS_FS_GATE_WORDS - 1U >
            MRESR_ADDR18_MASK ||
            fs_base + MRES_FS_VECTOR_WORDS - 1U > MRESR_ADDR18_MASK ||
            MRES_VFS_FS_GATE_WORDS != MRES_FS_VECTOR_WORDS)
                return -EINVAL;
        gate = (kword_t *)(unsigned long)(vfs_base + MRES_VFS_FS_GATE_FIRST);
        for (i = 0U; i < MRES_FS_VECTOR_WORDS; i++) {
                error = mresr_patch_addr18(&gate[i], fs_base + i);
                if (error != 0)
                        return -EINVAL;
        }
#else
        (void)vfs_base;
        (void)fs_base;
#endif
        return 0;
}

static int
kboot_bind_driver_gates(kword_t driver_base)
{
#ifdef __PDP10__
        kword_t *gate;
        unsigned int i;
        int error;

        if (driver_base > MRESR_ADDR18_MASK ||
            driver_base + MRES_DRV_VECTOR_WORDS - 1U > MRESR_ADDR18_MASK)
                return -EINVAL;
        gate = &kinit_gate_mdrv_dtc_rewind;
        for (i = 0U; i < MRES_DRV_VECTOR_WORDS; i++) {
                error = mresr_patch_addr18(&gate[i], driver_base + i);
                if (error != 0)
                        return -EINVAL;
        }
#else
        (void)driver_base;
#endif
        return 0;
}

static kword_t *kboot_bootinfo;
#define kboot_mres_stream_sector kboot_source.ks_initfs_base

static int
kboot_is_paper(void)
{
        return kboot_bootinfo != 0 &&
            (kboot_bootinfo[BOOTINFO_WORD_FLAGS] &
            BOOTINFO_F_PAPER_SOURCE) != 0;
}

static kword_t
kboot_word_addr_add(kword_t base, kword_t words)
{
#ifdef __PDP10__
        return base + words;
#else
        return (kword_t)(((kword_t *)base) + words);
#endif
}

static void
kboot_zero_words(kword_t *base, unsigned int words)
{
#ifdef __PDP10__
        mach_words_zero(base, words);
#else
        unsigned int i;

        for (i = 0; i < words; i++)
                base[i] = 0;
#endif
}

static kword_t
kboot_add36(kword_t sum, kword_t word)
{
        kword_t old;

        old = sum & DBOOT_WORD_MASK;
        word &= DBOOT_WORD_MASK;
        sum = (old + word) & DBOOT_WORD_MASK;
        if (sum < old || sum < word)
                sum = (sum + 1UL) & DBOOT_WORD_MASK;
        return sum;
}

static int
kboot_sector_checksum_ok(kword_t *sector)
{
        unsigned int i;
        kword_t sum;
        kword_t word;

        sum = 0;
        for (i = 0; i < DBOOT_SECTOR_WORDS; i++) {
                word = i == DBOOTX_WORD_CHECKSUM ? 0 : sector[i];
                sum = kboot_add36(sum, word);
        }
        return ((~sum) & DBOOT_WORD_MASK) ==
            (sector[DBOOTX_WORD_CHECKSUM] & DBOOT_WORD_MASK);
}

static unsigned int
kboot_order_count(kword_t *bi)
{
        return (unsigned int)((bi[BOOTINFO_WORD_MEMBER_ORDER1] >> 28) & 037UL);
}

static unsigned int
kboot_order_unit(kword_t *bi, unsigned int slot)
{
        kword_t word;
        unsigned int shift;

        if (slot < 9U) {
                word = bi[BOOTINFO_WORD_MEMBER_ORDER0];
                shift = slot * 4U;
        } else {
                word = bi[BOOTINFO_WORD_MEMBER_ORDER1];
                shift = (slot - 9U) * 4U;
        }
        return (unsigned int)((word >> shift) & 017UL);
}

static kword_t
kboot_locator(kword_t *bi, unsigned int slot)
{
        kword_t word;

        word = bi[BOOTINFO_WORD_DBOOT_LOC0 + (slot >> 1)] & DBOOT_WORD_MASK;
        if ((slot & 1U) != 0U)
                return BOOTINFO_DBOOT_LOC_ODD(word);
        return BOOTINFO_DBOOT_LOC_EVEN(word);
}

#ifdef __PDP10__
extern int dct136_boot_read_words(unsigned int, kword_t *, unsigned int);

static int
kboot_dsk_status(kword_t status)
{
        if ((status & DSK270_ST_WLE) != 0)
                return DSK270_E_WLOCK;
        if ((status & DSK270_ST_PER) != 0)
                return DSK270_E_PARITY;
        if ((status & DSK270_ST_ERRORS) != 0)
                return DSK270_E_STATUS;
        return DSK270_E_OK;
}

static int
kboot_dsk_wait(kword_t mask, unsigned int limit)
{
        unsigned int i;
        kword_t status;
        int error;

        for (i = 0; i < limit; i++) {
                status = pdp10_coni_dsk270();
                error = kboot_dsk_status(status);
                if (error != DSK270_E_OK)
                        return error;
                if ((status & mask) != 0)
                        return DSK270_E_OK;
                pdp10_io_wait(1U);
        }
        return DSK270_E_TIMEOUT;
}

/*
 * Discover attached DSK270 media independently of BOOTSET membership.
 * Stage1 identifies the boot members; KINIT adds standalone disks to the
 * permanent inventory used by diagnostics and /MOUNT.
 */
static unsigned int
kboot_dsk_present_mask(unsigned int required)
{
        unsigned int unit;
        unsigned int present;
        kword_t address;
        kword_t status;
        int error;

        present = required;
        for (unit = 0; unit < DSK270_UNIT_COUNT; unit++) {
                address = ((kword_t)unit & 03UL) << 16;
                pdp10_cono_dsk270(DSK270_CMD_SCL);
                pdp10_datao_dsk270(address);
                status = pdp10_coni_dsk270();
                if ((status & DSK270_ST_OPR) == 0)
                        present |= 1U << unit;
        }

        /* DATAO starts the controller even without a transfer command.  End
           that positioning cycle on a known boot member before KCORE takes
           ownership; otherwise the controller remains in its DFR/ADT loop. */
        unit = 0;
        while (unit < DSK270_UNIT_COUNT &&
            (required & (1U << unit)) == 0)
                unit++;
        if (unit < DSK270_UNIT_COUNT) {
                address = ((kword_t)unit & 03UL) << 16;
                pdp10_datao_dsk270(address);
                pdp10_cono_dsk270(DSK270_CMD_RD | DSK270_CMD_END);
                error = kboot_dsk_wait(DSK270_OF_IDS, DSK270_WAIT_SECTOR);
                pdp10_cono_dsk270(DSK270_CMD_SCL);
                if (error != DSK270_E_OK)
                        return required;
        }
        return present;
}

static int
kboot_read_sector(unsigned int unit, kword_t sector, kword_t *buf)
{
        kword_t cylinder;
        kword_t sector_in_cylinder;
        kword_t address;
        int error;

        if (unit >= DSK270_UNIT_COUNT || buf == 0 ||
            sector >= (kword_t)DSK270_SECTORS_PER_UNIT)
                return DSK270_E_ARG;
        cylinder = sector / (kword_t)DSK270_SECTORS_PER_CYL;
        sector_in_cylinder = sector % (kword_t)DSK270_SECTORS_PER_CYL;
        address = (((kword_t)unit & 03UL) << 16) |
            ((cylinder & 01777UL) << 6) | (sector_in_cylinder & 077UL);

        pdp10_cono_dct0((kword_t)DCT136_ST_MV);
        pdp10_cono_dsk270(DSK270_CMD_SCL);
        pdp10_datao_dsk270(address);
        error = kboot_dsk_wait(DSK270_OF_DFR, DSK270_WAIT_READY);
        if (error != DSK270_E_OK)
                goto out;
        pdp10_cono_dsk270(DSK270_CMD_RD);
        error = dct136_boot_read_words(0U, buf, DSK270_SECTOR_WORDS);
        pdp10_cono_dsk270(DSK270_CMD_END | DSK270_CMD_CLR);
        if (error != 0) {
                error = DSK270_E_DCT;
                goto out;
        }
        error = kboot_dsk_wait(DSK270_OF_IDS, DSK270_WAIT_SECTOR);
out:
        pdp10_cono_dct0((kword_t)0);
        if (error != DSK270_E_OK)
                pdp10_cono_dsk270(DSK270_CMD_END | DSK270_CMD_CLR);
        return error;
}
#else
#define kboot_read_sector dsk270_read_sector
static unsigned int
kboot_dsk_present_mask(unsigned int required)
{
        return required;
}
#endif

static int
kboot_bootinfo_ok(kword_t *bi)
{
        kword_t header;

        if (bi == 0)
                return 0;
        header = bi[BOOTINFO_WORD_HEADER] & DBOOT_WORD_MASK;
        if (DBOOT_HDR_MAGIC(header) != DBOOT_MAGIC_BI2 ||
            DBOOT_HDR_VERSION(header) != BOOTINFO_VERSION_V2)
                return 0;
        return (bi[BOOTINFO_WORD_TOTAL_WORDS] & DBOOT_WORD_MASK) ==
            BOOTINFO_TOTAL_WORDS;
}

static int
kboot_bad_contains(unsigned int count, kword_t sector)
{
        unsigned int i;
        kword_t word;
        kword_t start;
        kword_t end;

        for (i = 0; i < count; i++) {
                word = kboot_bad_run[i] & DBOOT_WORD_MASK;
                start = DBOOT_BAD_START(word);
                end = start + (kword_t)DBOOT_BAD_COUNTM1(word) + 1UL;
                if (end < start)
                        return 1;
                if (sector >= start && sector < end)
                        return 1;
        }
        return 0;
}

static int
kboot_bad_bounds(kword_t word, kword_t *startp, kword_t *endp)
{
        kword_t start;
        kword_t count;

        start = DBOOT_BAD_START(word);
        count = (kword_t)DBOOT_BAD_COUNTM1(word) + 1UL;
        if (start >= (kword_t)DSK270_SECTORS_PER_UNIT ||
            count > (kword_t)DSK270_SECTORS_PER_UNIT - start)
                return -EINVAL;
        *startp = start;
        *endp = start + count;
        return 0;
}

static int
kboot_fs_member(unsigned int slot, unsigned int unit, kword_t dbx_sector, unsigned int bad_count)
{
        kword_t start;
        kword_t run_start;
        kword_t run_end;
        kword_t available;
        unsigned int first;
        unsigned int changed;
        unsigned int i;

        if (slot >= kboot_source.ks_member_count ||
            slot >= D6STRIPE_MAX_MEMBERS || unit >= DSK270_UNIT_COUNT ||
            dbx_sector >= (kword_t)DSK270_SECTORS_PER_UNIT - 1UL)
                return -EINVAL;

        /* DB0 and DB1 runs need not be ordered.  Advance until no run
           contains the candidate immediately following DBX. */
        start = dbx_sector + 1UL;
        do {
                changed = 0;
                for (i = 0; i < bad_count; i++) {
                        if (kboot_bad_bounds(kboot_bad_run[i], &run_start,
                            &run_end) != 0)
                                return -EINVAL;
                        if (run_start <= start && start < run_end) {
                                start = run_end;
                                changed = 1;
                        }
                }
        } while (changed != 0U);
        if (start >= (kword_t)DSK270_SECTORS_PER_UNIT)
                return -EINVAL;

        kboot_fs_map.ps_member_unit[slot] = unit;
        kboot_fs_map.ps_start[slot] = start;
#ifndef DAIMON_BOOT_NO_BAD_RUNS
        first = kboot_fs_map.ps_bad_total;
        kboot_fs_map.ps_bad_first[slot] = first;
        for (i = 0; i < bad_count; i++) {
                (void)kboot_bad_bounds(kboot_bad_run[i], &run_start,
                    &run_end);
                if (run_end <= start)
                        continue;
                if (run_start < start)
                        run_start = start;
                if (kboot_fs_map.ps_bad_total >= D6STRIPE_MAX_BAD_RUNS)
                        return -ENOSPC;
                kboot_fs_map.ps_bad_run[kboot_fs_map.ps_bad_total++] =
                    DAIMON_MAKE_PAIR(run_start, run_end - run_start);
        }
        kboot_fs_map.ps_bad_run_count[slot] =
            kboot_fs_map.ps_bad_total - first;
#else
        (void)bad_count;
        first = 0;
#endif
        /* The D6FS superblock supplies the actual logical block count.  This
           upper bound only protects raw region calls before it is mounted. */
        available = (kword_t)DSK270_SECTORS_PER_UNIT - start;
        if (kboot_source.ks_fs_region_sectors == 0 ||
            available < kboot_source.ks_fs_region_sectors)
                kboot_source.ks_fs_region_sectors = available;
        (void)first;
        return 0;
}

static int
kboot_read_dbx(unsigned int slot, unsigned int unit, kword_t locator, kword_t *runp)
{
        kword_t *bi;
        kword_t header;
        kword_t dbx_sector;
        kword_t next;
        kword_t member;
        kword_t run;
        unsigned int bad_count;
        unsigned int count;
        unsigned int i;
        unsigned int db1_count;
        int compact;
        int error;

        bi = kboot_bootinfo;
        bad_count = 0;
        db1_count = 0;
        error = kboot_read_sector(unit, locator, kboot_sector);
        if (error != DSK270_E_OK)
                return -EIO;
        header = kboot_sector[DBOOTX_WORD_HEADER] & DBOOT_WORD_MASK;
        compact = DBOOT_HDR_MAGIC(header) == DBOOT_MAGIC_DBC;
        if (compact) {
                dbx_sector = locator;
        } else {
                if (DBOOT_HDR_MAGIC(header) != DBOOT_MAGIC_DB0 ||
                    DBOOT_HDR_VERSION(header) != DBOOT_VERSION_V1 ||
                    (DBOOT_HDR_FLAGS(header) & DBOOT_DB0_F_TOO_MANY_BADS) != 0)
                        return -EINVAL;
                count = (unsigned int)DBOOT_HDR_COUNT(header);
                if (count > DBOOT_DB0_BAD_RUNS)
                        return -EINVAL;
                for (i = 0; i < count; i++)
                        kboot_bad_run[bad_count++] =
                            kboot_sector[1U + i] & DBOOT_WORD_MASK;
                next = 0;
                if ((DBOOT_HDR_FLAGS(header) & DBOOT_DB0_F_HAS_DB1) != 0)
                        next = kboot_sector[DBOOT_DB0_WORD_NEXT_DB1] &
                            DBOOT_HALF_MASK;
                dbx_sector = locator;
                while (next != 0) {
                        if (++db1_count > KBOOT_MAX_DB1 || next == dbx_sector)
                                return -EINVAL;
                        dbx_sector = next;
                        error = kboot_read_sector(unit, dbx_sector,
                            kboot_sector);
                        if (error != DSK270_E_OK)
                                return -EIO;
                        header = kboot_sector[DBOOTX_WORD_HEADER] &
                            DBOOT_WORD_MASK;
                        if (DBOOT_HDR_MAGIC(header) != DBOOT_MAGIC_DB1 ||
                            DBOOT_HDR_VERSION(header) != DBOOT_VERSION_V1 ||
                            DBOOT_HDR_FLAGS(header) != 0)
                                return -EINVAL;
                        count = (unsigned int)DBOOT_HDR_COUNT(header);
                        if (count > DBOOT_DB1_BAD_RUNS ||
                            bad_count + count > DBOOT_MAX_BAD_RUNS)
                                return -EINVAL;
                        for (i = 0; i < count; i++)
                                kboot_bad_run[bad_count++] =
                                    kboot_sector[DBOOT_DB1_WORD_BAD0 + i] &
                                    DBOOT_WORD_MASK;
                        next = kboot_sector[DBOOT_DB1_WORD_NEXT_DB1] &
                            DBOOT_HALF_MASK;
                }
                dbx_sector++;
                while (kboot_bad_contains(bad_count, dbx_sector)) {
                        if (dbx_sector == DBOOT_HALF_MASK)
                                return -EINVAL;
                        dbx_sector++;
                }
                error = kboot_read_sector(unit, dbx_sector, kboot_sector);
                if (error != DSK270_E_OK)
                        return -EIO;
                header = kboot_sector[DBOOTX_WORD_HEADER] & DBOOT_WORD_MASK;
        }

        if (DBOOT_HDR_VERSION(header) != DBOOTX_VERSION)
                return -EINVAL;
        if (compact) {
                if (DBOOT_HDR_MAGIC(header) != DBOOT_MAGIC_DBC)
                        return -EINVAL;
        } else if (DBOOT_HDR_MAGIC(header) != DBOOT_MAGIC_DBX) {
                return -EINVAL;
        }
        if ((DBOOT_HDR_FLAGS(header) & DBOOT_DBX_F_CHECKSUM) != 0 &&
            !kboot_sector_checksum_ok(kboot_sector))
                return -EINVAL;
        if ((kboot_sector[DBOOTX_WORD_GENERATION] & DBOOT_WORD_MASK) !=
            (bi[BOOTINFO_WORD_GENERATION] & DBOOT_WORD_MASK) ||
            (kboot_sector[DBOOTX_WORD_UUID0] & DBOOT_WORD_MASK) !=
            (bi[BOOTINFO_WORD_UUID0] & DBOOT_WORD_MASK) ||
            (kboot_sector[DBOOTX_WORD_UUID1] & DBOOT_WORD_MASK) !=
            (bi[BOOTINFO_WORD_UUID1] & DBOOT_WORD_MASK))
                return -EINVAL;

        member = kboot_sector[DBOOTX_WORD_MEMBER] & DBOOT_WORD_MASK;
        if (DBOOT_MEMBER_INDEX(member) != slot ||
            DBOOT_MEMBER_MASK(member) !=
            BOOTINFO_MEMBER_MASK(bi[BOOTINFO_WORD_MEMBER_SUMMARY]) ||
            DBOOT_MEMBER_DISK_COUNT(member) != kboot_source.ks_member_count)
                return -EINVAL;
        run = kboot_sector[DBOOTX_WORD_BOOT_RUN] & DBOOT_WORD_MASK;
        if (DBOOTX_BOOT_RUN_COUNT(run) == 0)
                return -EINVAL;
        *runp = run;
        return kboot_fs_member(slot, unit, dbx_sector, bad_count);
}

static void
kboot_divmod_members(kword_t value, kword_t *quotientp, kword_t *remainderp)
{
        kword_t divisor;
        kword_t quotient;

        divisor = (kword_t)kboot_source.ks_member_count;
        quotient = 0;
        while (value >= divisor) {
                value -= divisor;
                quotient += 1UL;
        }
        *quotientp = quotient;
        *remainderp = value;
}

static int
kboot_reduce_mres_runs(void)
{
        kword_t raw[KINIT_MAX_BOOT_MEMBERS];
        kword_t total;
        kword_t base;
        kword_t extra;
        kword_t first;
        kword_t carry;
        kword_t run;
        kword_t start;
        kword_t remaining;
        unsigned int i;
        unsigned int source_slot;
        unsigned int unit;

        total = 0;
        for (i = 0; i < kboot_source.ks_member_count; i++) {
                raw[i] = kboot_source.ks_mres_member[i];
                total += DBOOTX_BOOT_RUN_COUNT(raw[i]);
        }
        if (total == 0 || kboot_mres_stream_sector >= total)
                return -301;

        kboot_divmod_members(total, &base, &extra);
        for (i = 0; i < kboot_source.ks_member_count; i++) {
                kword_t actual;
                kword_t expected;

                expected = base;
                if ((kword_t)i < extra)
                        expected += 1UL;
                actual = DBOOTX_BOOT_RUN_COUNT(raw[i]);
                if (((actual ^ expected) & DBOOT_HALF_MASK) != 0)
                        return -302;
        }

        remaining = total - kboot_mres_stream_sector;
        kboot_divmod_members(kboot_mres_stream_sector, &base, &first);
        for (i = 0; i < kboot_source.ks_member_count; i++) {
                source_slot = (unsigned int)first + i;
                carry = 0;
                if (source_slot >= kboot_source.ks_member_count) {
                        source_slot -= kboot_source.ks_member_count;
                        carry = 1;
                }
                if ((kword_t)i >= remaining) {
                        kboot_source.ks_mres_member[i] = 0;
                        continue;
                }
                run = raw[source_slot];
                start = DBOOTX_BOOT_RUN_START(run);
                if (base + carry >= DBOOTX_BOOT_RUN_COUNT(run) ||
                    start > KBOOT_HALF_MASK - base - carry)
                        return -303;
                unit = kboot_order_unit(kboot_bootinfo, source_slot);
                kboot_source.ks_mres_member[i] =
                    (((kword_t)unit & KBOOT_HALF_MASK) << KBOOT_UNIT_SHIFT) |
                    (start + base + carry);
        }
        for (; i < KINIT_MAX_BOOT_MEMBERS; i++)
                kboot_source.ks_mres_member[i] = 0;
        kboot_source.ks_mres_stream_sectors = remaining;
        return 0;
}

int
kinit_import_bootinfo(kword_t *bootinfo_words)
{
        kword_t summary;
        kword_t stream_words;
        unsigned int count;
        unsigned int mask;
        unsigned int seen;
        unsigned int i;
        unsigned int unit;

        kboot_zero_words((kword_t *)&kboot_source,
            (unsigned int)(sizeof(kboot_source) / sizeof(kword_t)));
        kboot_zero_words((kword_t *)&kboot_fs_map,
            (unsigned int)(sizeof(kboot_fs_map) / sizeof(kword_t)));
        kboot_bootinfo = bootinfo_words;
        if (!kboot_bootinfo_ok(bootinfo_words))
                return -EINVAL;

        summary = bootinfo_words[BOOTINFO_WORD_MEMBER_SUMMARY] &
            DBOOT_WORD_MASK;
        count = (unsigned int)BOOTINFO_MEMBER_COUNT(summary);
        mask = (unsigned int)BOOTINFO_MEMBER_MASK(summary);
        if (kboot_is_paper()) {
                if (count != 0 || mask != 0)
                        return -EINVAL;
                stream_words = (kword_t)DAIMON_HEADER_WORDS_V1 +
                    (bootinfo_words[BOOTINFO_WORD_KCORE_FILE] &
                    DBOOT_HALF_MASK) +
                    (bootinfo_words[BOOTINFO_WORD_INIT_WORDS] &
                    DBOOT_HALF_MASK);
                if (stream_words > DBOOT_HALF_MASK)
                        return -EINVAL;
                kboot_source.ks_member_count = 0;
                kboot_source.ks_dsk_present_mask = 0;
                kboot_mres_stream_sector = 0;
                return 0;
        }
        if (count == 0 || count > KINIT_MAX_BOOT_MEMBERS ||
            kboot_order_count(bootinfo_words) != count)
                return -EINVAL;

        seen = 0;
        kboot_source.ks_member_count = count;
        for (i = 0; i < count; i++) {
                unit = kboot_order_unit(bootinfo_words, i);
                if (unit >= DSK270_UNIT_COUNT || (seen & (1U << unit)) != 0 ||
                    (mask & (1U << unit)) == 0)
                        return -EINVAL;
                seen |= 1U << unit;
        }
        if (seen != mask)
                return -EINVAL;
        kboot_source.ks_dsk_present_mask = seen;
        if (count > 1U)
                kboot_source.ks_diskset_member_map[0] = (kword_t)seen;

        stream_words = (kword_t)DAIMON_HEADER_WORDS_V1 +
            (bootinfo_words[BOOTINFO_WORD_KCORE_FILE] & DBOOT_HALF_MASK) +
            (bootinfo_words[BOOTINFO_WORD_INIT_WORDS] & DBOOT_HALF_MASK);
        if (stream_words > DBOOT_HALF_MASK)
                return -EINVAL;
        kboot_mres_stream_sector = (stream_words +
            (DBOOT_SECTOR_WORDS - 1U)) >> 7;
        return 0;
}

void
kinit_probe_disks(void)
{
        if (kboot_is_paper())
                return;
        kboot_source.ks_dsk_present_mask = kboot_dsk_present_mask(
            (unsigned int)kboot_source.ks_dsk_present_mask);
}

int
kinit_import_boot_badmaps(void)
{
        unsigned int i;
        unsigned int unit;
        kword_t locator;
        int error;

        if (kboot_bootinfo == 0)
                return -EINVAL;
        if (kboot_is_paper())
                return 0;
        if (kboot_source.ks_member_count == 0)
                return -EINVAL;
        kboot_fs_map.ps_member_count =
            (unsigned int)kboot_source.ks_member_count;
        for (i = 0; i < kboot_source.ks_member_count; i++) {
                unit = kboot_order_unit(kboot_bootinfo, i);
                locator = kboot_locator(kboot_bootinfo, i);
                error = kboot_read_dbx(i, unit, locator,
                    &kboot_source.ks_mres_member[i]);
                if (error != 0)
                        return error;
        }
        kboot_source.ks_fs_region_sectors *=
            kboot_source.ks_member_count;
        error = kboot_reduce_mres_runs();
        if (error != 0)
                return error;
        kboot_source.ks_fs_map_addr =
            (kword_t)(unsigned long)&kboot_fs_map;
        return 0;
}

static dboot_word
kboot_manifest_checksum(const kword_t *base, unsigned int words)
{
        unsigned int i;
        dboot_word sum;
        dboot_word half;

        sum = 0;
        for (i = 0; i < words; i++) {
                if (i == DMANIF_WORD_CHECKSUM)
                        continue;
                half = (base[i] >> 18) & DBOOT_HALF_MASK;
                sum = (sum + half) & DBOOT_HALF_MASK;
                if (sum == DBOOT_HALF_MASK)
                        sum = 0;
                half = base[i] & DBOOT_HALF_MASK;
                sum = (sum + half) & DBOOT_HALF_MASK;
                if (sum == DBOOT_HALF_MASK)
                        sum = 0;
        }
        return (~sum) & DBOOT_HALF_MASK;
}

struct kboot_manifest_entry {
        unsigned int kme_id;
        unsigned int kme_required;
        unsigned int kme_format;
        kword_t kme_minit_words;
        kword_t kme_stored_mres_words;
        kword_t kme_resident_mres_words;
};

static int
kboot_manifest_entry_get(const kword_t *manifest, unsigned int version,
    unsigned int index, struct kboot_manifest_entry *ep)
{
        kword_t meta;
        kword_t sizes;
        unsigned int off;

        if (manifest == 0 || ep == 0)
                return -EINVAL;
        if (version == DMANIF_VERSION_V1) {
                meta = manifest[DMANIF_WORD_MODULE0 + index] &
                    DBOOT_WORD_MASK;
                ep->kme_id = (unsigned int)DMANIF_ENTRY_ID(meta);
                ep->kme_required = (unsigned int)DMANIF_ENTRY_REQUIRED(meta);
                ep->kme_format = DMANIF_FMT_RAW;
                ep->kme_minit_words = DMANIF_ENTRY_MINIT_WORDS(meta);
                ep->kme_stored_mres_words = DMANIF_ENTRY_MRES_WORDS(meta);
                ep->kme_resident_mres_words =
                    (ep->kme_id == DAIMOD_ID_MRESIO ||
                     ep->kme_id == DAIMOD_ID_MRESDRV) ? 0 :
                    ep->kme_stored_mres_words;
                return 0;
        }
        if (version != DMANIF_VERSION_V2)
                return -EINVAL;
        off = DMANIF_WORD_MODULE0 + DMANIF_V2_ENTRY_WORDS * index;
        meta = manifest[off] & DBOOT_WORD_MASK;
        sizes = manifest[off + 1U] & DBOOT_WORD_MASK;
        ep->kme_id = (unsigned int)DMANIF_V2_ENTRY_ID(meta);
        ep->kme_required = (unsigned int)DMANIF_V2_ENTRY_REQUIRED(meta);
        ep->kme_format = (unsigned int)DMANIF_V2_ENTRY_FORMAT(meta);
        ep->kme_minit_words = DMANIF_V2_ENTRY_MINIT_WORDS(meta);
        ep->kme_stored_mres_words = DMANIF_V2_STORED_MRES(sizes);
        ep->kme_resident_mres_words = DMANIF_V2_RESIDENT_MRES(sizes);
        return 0;
}

static int
kboot_manifest_header_ok(const kword_t *manifest,
    kword_t init_words,
    unsigned int *versionp,
    unsigned int *module_countp,
    unsigned int *manifest_wordsp,
    kword_t *minit_totalp,
    kword_t *mres_totalp)
{
        kword_t control;
        kword_t sizes;
        unsigned int module_count;
        unsigned int manifest_words;

        if (manifest == 0 || init_words < DMANIF_HEADER_WORDS_V1)
                return -EINVAL;
        if ((manifest[DMANIF_WORD_MAGIC] & DBOOT_WORD_MASK) != DMANIF_MAGIC)
                return -EINVAL;
        control = manifest[DMANIF_WORD_CONTROL] & DBOOT_WORD_MASK;
        module_count = DMANIF_CTL_MOD_COUNT(control);
        if (DMANIF_CTL_VERSION(control) == DMANIF_VERSION_V1 &&
            DMANIF_CTL_HDR_WORDS(control) == DMANIF_HEADER_WORDS_V1)
                manifest_words = DMANIF_HEADER_WORDS_V1 + module_count;
        else if (DMANIF_CTL_VERSION(control) == DMANIF_VERSION_V2 &&
            DMANIF_CTL_HDR_WORDS(control) == DMANIF_HEADER_WORDS_V2)
                manifest_words = DMANIF_HEADER_WORDS_V2 +
                    DMANIF_V2_ENTRY_WORDS * module_count;
        else
                return -EINVAL;
        if (module_count == 0U || init_words < (kword_t)manifest_words)
                return -EINVAL;
        if (kboot_manifest_checksum(manifest, manifest_words) !=
            DMANIF_CHECKSUM(manifest[DMANIF_WORD_CHECKSUM]))
                return -EINVAL;
        sizes = manifest[DMANIF_WORD_SIZES] & DBOOT_WORD_MASK;
        if (init_words < (kword_t)manifest_words +
            DMANIF_SIZES_MINIT(sizes) + DMANIF_SIZES_MRES(sizes))
                return -EINVAL;
        *versionp = (unsigned int)DMANIF_CTL_VERSION(control);
        *module_countp = module_count;
        *manifest_wordsp = manifest_words;
        *minit_totalp = DMANIF_SIZES_MINIT(sizes);
        *mres_totalp = DMANIF_SIZES_MRES(sizes);
        return 0;
}

static int
kboot_mrel_ok(const kword_t *source, kword_t stored_words,
    kword_t resident_words, unsigned int format, unsigned int min_image_words)
{
        struct mresr_desc desc;

        if (format != DMANIF_FMT_MREL1 && format != DMANIF_FMT_MREL2)
                return -EINVAL;
        if (mresr_decode(source, (unsigned int)stored_words, &desc) != 0 ||
            mresr_validate(source, (unsigned int)stored_words, &desc) != 0 ||
            desc.mr_stored_words != (unsigned int)stored_words ||
            (kword_t)(desc.mr_image_words + desc.mr_bss_words) != resident_words ||
            desc.mr_image_words < min_image_words)
                return -EINVAL;
        return 0;
}

int
kinit_load_payload(kword_t init_base, kword_t init_words, kword_t kcore_base, kword_t kcore_words)
{
        const kword_t *manifest;
        const kword_t *minit_cursor;
        const kword_t *mres_cursor;
        const kword_t *mres_core_source;
        const kword_t *mres_d6fs_source;
        const kword_t *mres_io_source;
        const kword_t *mres_device_source;
        const kword_t *mres_vfs_source;
        const kword_t *mres_fs_source;
        const kword_t *mres_driver_source;
        const kword_t *core_bind_source;
        const kword_t *initfs_source;
        struct kboot_manifest_entry module;
        kword_t minit_total;
        kword_t mres_total;
        kword_t kinit_words;
        kword_t resident_end;
        kword_t initfs_words;
        kword_t entry_mres_words;
        kword_t seen_mres_words;
        kword_t seen_resident_words;
        kword_t core_stored_words;
        kword_t core_resident_words;
        kword_t d6fs_stored_words;
        kword_t d6fs_resident_words;
        kword_t core_bind_words;
        kword_t io_stored_words;
        kword_t io_resident_words;
        kword_t device_stored_words;
        kword_t device_resident_words;
        kword_t vfs_stored_words;
        kword_t vfs_resident_words;
        kword_t fs_stored_words;
        kword_t fs_resident_words;
        kword_t driver_stored_words;
        kword_t driver_resident_words;
        unsigned int core_format;
        unsigned int d6fs_format;
        unsigned int io_format;
        unsigned int device_format;
        unsigned int vfs_format;
        unsigned int fs_format;
        unsigned int driver_format;
        unsigned int manifest_version;
        unsigned int module_count;
        unsigned int manifest_words;
        unsigned int id;
        unsigned int i;

        manifest = (const kword_t *)init_base;
        if (kboot_manifest_header_ok(manifest, init_words, &manifest_version,
            &module_count, &manifest_words, &minit_total, &mres_total) != 0)
                return -EINVAL;

        kinit_words = init_words - (kword_t)manifest_words - minit_total -
            mres_total;
        minit_cursor = (const kword_t *)kboot_word_addr_add(init_base,
            (kword_t)manifest_words + kinit_words);
        mres_cursor = minit_cursor + minit_total;
        if (kboot_is_paper()) {
                kword_t cold_base;
                kword_t cold_words;

                cold_base = kboot_bootinfo[BOOTINFO_WORD_MODULE_START] &
                    DBOOT_HALF_MASK;
                cold_words = kboot_bootinfo[BOOTINFO_WORD_MODULE_WORDS] &
                    DBOOT_HALF_MASK;
                if (cold_base == 0 || cold_words == 0 ||
                    (cold_words & (DBOOT_SECTOR_WORDS - 1U)) != 0 ||
                    cold_base > DBOOT_HALF_MASK - cold_words)
                        return -EINVAL;
                kboot_source.ks_mres_member[0] = cold_base;
                kboot_source.ks_mres_stream_sectors = cold_words >> 7;

        }
        mres_core_source = 0;
        mres_d6fs_source = 0;
        mres_io_source = 0;
        mres_device_source = 0;
        mres_vfs_source = 0;
        mres_fs_source = 0;
        mres_driver_source = 0;
        core_bind_source = 0;
        initfs_source = 0;
        initfs_words = 0;
        seen_mres_words = 0;
        seen_resident_words = 0;
        core_stored_words = 0;
        core_resident_words = 0;
        d6fs_stored_words = 0;
        d6fs_resident_words = 0;
        core_bind_words = 0;
        io_stored_words = 0;
        io_resident_words = 0;
        device_stored_words = 0;
        device_resident_words = 0;
        vfs_stored_words = 0;
        vfs_resident_words = 0;
        fs_stored_words = 0;
        fs_resident_words = 0;
        driver_stored_words = 0;
        driver_resident_words = 0;
        core_format = DMANIF_FMT_RAW;
        d6fs_format = DMANIF_FMT_RAW;
        io_format = DMANIF_FMT_RAW;
        device_format = DMANIF_FMT_RAW;
        vfs_format = DMANIF_FMT_RAW;
        fs_format = DMANIF_FMT_RAW;
        driver_format = DMANIF_FMT_RAW;

        /*
         * Diagnostic-only MINIT records may precede or follow the three
         * base-profile resident payloads.  Scan the manifest once so placement is
         * independent of module order and duplicate resident providers are
         * rejected.
         */
        for (i = 0U; i < module_count; i++) {
                if (kboot_manifest_entry_get(manifest, manifest_version, i,
                    &module) != 0)
                        return -EINVAL;
                id = module.kme_id;
                entry_mres_words = module.kme_stored_mres_words;
                if (seen_mres_words > mres_total ||
                    entry_mres_words > mres_total - seen_mres_words ||
                    seen_resident_words >
                    DMANIF_RESIDENT_MRES(manifest[DMANIF_WORD_CHECKSUM]) ||
                    module.kme_resident_mres_words >
                    DMANIF_RESIDENT_MRES(manifest[DMANIF_WORD_CHECKSUM]) -
                    seen_resident_words)
                        return -EINVAL;
                if (id == DAIMOD_ID_MRESCORE) {
                        struct mresb_desc bind_desc;

                        if (mres_core_source != 0 || core_bind_source != 0 ||
                            module.kme_required == 0 ||
                            module.kme_format != DMANIF_FMT_MREL2 ||
                            module.kme_minit_words == 0 ||
                            kboot_mrel_ok(mres_cursor, entry_mres_words,
                            module.kme_resident_mres_words,
                            module.kme_format, MRES_CORE_VECTOR_WORDS) != 0 ||
                            mresb_decode(minit_cursor,
                            (unsigned int)module.kme_minit_words,
                            &bind_desc) != 0 ||
                            bind_desc.mb_stored_words !=
                            (unsigned int)module.kme_minit_words ||
                            bind_desc.mb_provider_id != DAIMOD_ID_MRESCORE ||
                            bind_desc.mb_kcore_words != (unsigned int)kcore_words)
                                return -EINVAL;
                        mres_core_source = mres_cursor;
                        core_bind_source = minit_cursor;
                        core_stored_words = entry_mres_words;
                        core_resident_words = module.kme_resident_mres_words;
                        core_bind_words = module.kme_minit_words;
                        core_format = module.kme_format;
                } else if (id == DAIMOD_ID_MRESD6FS) {
                        if (mres_d6fs_source != 0 || module.kme_required == 0 ||
                            module.kme_format != DMANIF_FMT_MREL2 ||
                            kboot_mrel_ok(mres_cursor, entry_mres_words,
                            module.kme_resident_mres_words,
                            module.kme_format, 1U) != 0)
                                return -EINVAL;
                        mres_d6fs_source = mres_cursor;
                        d6fs_stored_words = entry_mres_words;
                        d6fs_resident_words = module.kme_resident_mres_words;
                        d6fs_format = module.kme_format;
                } else if (id == DAIMOD_ID_MRESIO) {
                        struct mresr_desc desc;

                        if (mres_io_source != 0 || module.kme_required == 0)
                                return -EINVAL;
                        if (module.kme_format == DMANIF_FMT_RAW) {
#ifdef DAIMON_RUNTIME_MRES
                                return -EINVAL;
#else
                                if (entry_mres_words !=
                                    (kword_t)MRES_IO_STORED_WORDS ||
                                    module.kme_resident_mres_words != 0)
                                        return -EINVAL;
#endif
                        } else if (module.kme_format == DMANIF_FMT_MREL1 ||
                            module.kme_format == DMANIF_FMT_MREL2) {
                                if (mresr_decode(mres_cursor,
                                    (unsigned int)entry_mres_words, &desc) != 0 ||
                                    mresr_validate(mres_cursor,
                                    (unsigned int)entry_mres_words, &desc) != 0 ||
                                    desc.mr_stored_words !=
                                    (unsigned int)entry_mres_words ||
                                    (kword_t)(desc.mr_image_words +
                                    desc.mr_bss_words) !=
                                    module.kme_resident_mres_words ||
                                    desc.mr_image_words <
                                    MRES_IO_VECTOR_FIRST + MRES_IO_VECTOR_WORDS)
                                        return -EINVAL;
                        } else {
                                return -EINVAL;
                        }
                        mres_io_source = mres_cursor;
                        io_format = module.kme_format;
                        io_stored_words = entry_mres_words;
                        io_resident_words = module.kme_resident_mres_words;
                } else if (id == DAIMOD_ID_MRESDEV) {
                        struct mresr_desc desc;

                        if (mres_device_source != 0 || module.kme_required == 0 ||
                            (module.kme_format != DMANIF_FMT_MREL1 &&
                            module.kme_format != DMANIF_FMT_MREL2))
                                return -EINVAL;
                        if (mresr_decode(mres_cursor,
                            (unsigned int)entry_mres_words, &desc) != 0 ||
                            mresr_validate(mres_cursor,
                            (unsigned int)entry_mres_words, &desc) != 0 ||
                            desc.mr_stored_words !=
                            (unsigned int)entry_mres_words ||
                            (kword_t)(desc.mr_image_words +
                            desc.mr_bss_words) !=
                            module.kme_resident_mres_words ||
                            desc.mr_image_words < MRES_DEV_VECTOR_WORDS)
                                return -EINVAL;
                        mres_device_source = mres_cursor;
                        device_format = module.kme_format;
                        device_stored_words = entry_mres_words;
                        device_resident_words =
                            module.kme_resident_mres_words;
                } else if (id == DAIMOD_ID_MRESVFS) {
                        struct mresr_desc desc;

                        if (mres_vfs_source != 0 || module.kme_required == 0 ||
                            (module.kme_format != DMANIF_FMT_MREL1 &&
                            module.kme_format != DMANIF_FMT_MREL2))
                                return -EINVAL;
                        if (mresr_decode(mres_cursor,
                            (unsigned int)entry_mres_words, &desc) != 0 ||
                            mresr_validate(mres_cursor,
                            (unsigned int)entry_mres_words, &desc) != 0 ||
                            desc.mr_stored_words !=
                            (unsigned int)entry_mres_words ||
                            (kword_t)(desc.mr_image_words +
                            desc.mr_bss_words) !=
                            module.kme_resident_mres_words ||
                            desc.mr_image_words < MRES_VFS_VECTOR_WORDS)
                                return -EINVAL;
                        mres_vfs_source = mres_cursor;
                        vfs_format = module.kme_format;
                        vfs_stored_words = entry_mres_words;
                        vfs_resident_words = module.kme_resident_mres_words;
                } else if (id == DAIMOD_ID_MRESFS) {
                        struct mresr_desc desc;

                        if (mres_fs_source != 0 || module.kme_required == 0 ||
                            (module.kme_format != DMANIF_FMT_MREL1 &&
                            module.kme_format != DMANIF_FMT_MREL2))
                                return -EINVAL;
                        if (mresr_decode(mres_cursor,
                            (unsigned int)entry_mres_words, &desc) != 0 ||
                            mresr_validate(mres_cursor,
                            (unsigned int)entry_mres_words, &desc) != 0 ||
                            desc.mr_stored_words !=
                            (unsigned int)entry_mres_words ||
                            (kword_t)(desc.mr_image_words +
                            desc.mr_bss_words) !=
                            module.kme_resident_mres_words ||
                            desc.mr_image_words < MRES_FS_VECTOR_WORDS)
                                return -EINVAL;
                        mres_fs_source = mres_cursor;
                        fs_format = module.kme_format;
                        fs_stored_words = entry_mres_words;
                        fs_resident_words = module.kme_resident_mres_words;
                } else if (id == DAIMOD_ID_MRESDRV) {
                        struct mresr_desc desc;

                        if (mres_driver_source != 0 || module.kme_required == 0)
                                return -EINVAL;
                        if (module.kme_format == DMANIF_FMT_RAW) {
#ifdef DAIMON_RUNTIME_MRES
                                return -EINVAL;
#else
                                if (entry_mres_words !=
                                    (kword_t)MRES_DRV_STORED_WORDS ||
                                    module.kme_resident_mres_words != 0)
                                        return -EINVAL;
#endif
                        } else if (module.kme_format == DMANIF_FMT_MREL1 ||
                            module.kme_format == DMANIF_FMT_MREL2) {
                                if (mresr_decode(mres_cursor,
                                    (unsigned int)entry_mres_words, &desc) != 0 ||
                                    mresr_validate(mres_cursor,
                                    (unsigned int)entry_mres_words, &desc) != 0 ||
                                    desc.mr_stored_words !=
                                    (unsigned int)entry_mres_words ||
                                    (kword_t)(desc.mr_image_words +
                                    desc.mr_bss_words) !=
                                    module.kme_resident_mres_words)
                                        return -EINVAL;
                        } else {
                                return -EINVAL;
                        }
                        mres_driver_source = mres_cursor;
                        driver_format = module.kme_format;
                        driver_stored_words = entry_mres_words;
                        driver_resident_words =
                            module.kme_resident_mres_words;
                } else if (id == DAIMOD_ID_INITFS0) {
                        if (initfs_source != 0 || module.kme_required == 0 ||
                            module.kme_format != DMANIF_FMT_RAW ||
                            entry_mres_words == 0 ||
                            module.kme_resident_mres_words != entry_mres_words)
                                return -EINVAL;
                        initfs_source = mres_cursor;
                        initfs_words = entry_mres_words;
                } else if (module.kme_format != DMANIF_FMT_RAW ||
                    entry_mres_words != 0 || module.kme_resident_mres_words != 0) {
                        return -EINVAL;
                }
                minit_cursor += module.kme_minit_words;
                mres_cursor += entry_mres_words;
                seen_mres_words += entry_mres_words;
                seen_resident_words += module.kme_resident_mres_words;
        }
        if (seen_mres_words != mres_total || mres_io_source == 0 ||
            mres_driver_source == 0)
                return -EINVAL;
        if (DMANIF_RESIDENT_MRES(manifest[DMANIF_WORD_CHECKSUM]) !=
            seen_resident_words)
                return -EINVAL;

#ifndef __PDP10__
        (void)core_stored_words;
        (void)core_resident_words;
        (void)d6fs_stored_words;
        (void)d6fs_resident_words;
        (void)core_bind_words;
        (void)core_format;
        (void)d6fs_format;
        (void)io_stored_words;
        (void)io_resident_words;
        (void)io_format;
        (void)device_stored_words;
        (void)device_resident_words;
        (void)device_format;
        (void)vfs_stored_words;
        (void)vfs_resident_words;
        (void)vfs_format;
        (void)fs_stored_words;
        (void)fs_resident_words;
        (void)fs_format;
        (void)driver_stored_words;
        (void)driver_resident_words;
        (void)driver_format;
#endif
        resident_end = kboot_word_addr_add(kcore_base, kcore_words);
#ifdef __PDP10__
        if (resident_end < (kword_t)KCORE_STACK_END)
                resident_end = (kword_t)KCORE_STACK_END;
        if (mres_core_source != 0 && mres_d6fs_source != 0 &&
            mres_fs_source != 0 && mres_vfs_source != 0 &&
            mres_device_source != 0 && mres_io_source != 0 &&
            mres_driver_source != 0 && core_bind_source != 0 &&
            core_format == DMANIF_FMT_MREL2 &&
            d6fs_format == DMANIF_FMT_MREL2 &&
            fs_format == DMANIF_FMT_MREL2 &&
            vfs_format == DMANIF_FMT_MREL2 &&
            device_format == DMANIF_FMT_MREL2 &&
            io_format == DMANIF_FMT_MREL1 &&
            (driver_format == DMANIF_FMT_MREL1 ||
            driver_format == DMANIF_FMT_MREL2)) {
                const kword_t *payload[7];
                kword_t stored[7];
                kword_t extent[7];
                unsigned int ids[7];
                struct mresr_input inputs[7];
                struct mresr_provider providers[7];
                kword_t bases[7];
                kword_t packed_end;
                kword_t memory_limit;
                kword_t packed_start;
                unsigned int n;

                payload[0] = mres_core_source;
                payload[1] = mres_d6fs_source;
                payload[2] = mres_fs_source;
                payload[3] = mres_vfs_source;
                payload[4] = mres_device_source;
                payload[5] = mres_io_source;
                payload[6] = mres_driver_source;
                stored[0] = core_stored_words;
                stored[1] = d6fs_stored_words;
                stored[2] = fs_stored_words;
                stored[3] = vfs_stored_words;
                stored[4] = device_stored_words;
                stored[5] = io_stored_words;
                stored[6] = driver_stored_words;
                extent[0] = core_resident_words;
                extent[1] = d6fs_resident_words;
                extent[2] = fs_resident_words;
                extent[3] = vfs_resident_words;
                extent[4] = device_resident_words;
                extent[5] = io_resident_words;
                extent[6] = driver_resident_words;
                ids[0] = DAIMOD_ID_MRESCORE;
                ids[1] = DAIMOD_ID_MRESD6FS;
                ids[2] = DAIMOD_ID_MRESFS;
                ids[3] = DAIMOD_ID_MRESVFS;
                ids[4] = DAIMOD_ID_MRESDEV;
                ids[5] = DAIMOD_ID_MRESIO;
                ids[6] = DAIMOD_ID_MRESDRV;
                for (n = 0U; n < 7U; n++) {
                        inputs[n].mr_payload = payload[n];
                        inputs[n].mr_payload_words = (unsigned int)stored[n];
                }
                memory_limit = (kword_t)machine_probe_memory_kwords() <<
                    DAIMON_KWORD_SHIFT;
#ifdef DAIMON_RUNTIME_MRES
                if (kboot_bootinfo != 0 &&
                    (kboot_bootinfo[BOOTINFO_WORD_FLAGS] &
                    BOOTINFO_F_PAPER_SOURCE) != 0) {
                        kword_t source_base;

                        source_base = kboot_bootinfo[
                            BOOTINFO_WORD_MODULE_START] & DBOOT_HALF_MASK;
                        if (source_base == 0 || source_base > memory_limit)
                                return -EINVAL;
                        memory_limit = source_base;
                }
#endif
                if (resident_end > memory_limit ||
                    initfs_words > memory_limit - resident_end)
                        return -ENOMEM;
                packed_start = resident_end + initfs_words;
                if (mresr_preflight(inputs, 7U, packed_start, memory_limit,
                    bases, &packed_end) != 0)
                        return -ENOMEM;
                /* Preserve INITFS before provider destinations can overlap
                   the disk-resident source stream. */
                if (initfs_source != 0) {
                        mach_words_copy((kword_t *)(unsigned long)resident_end,
                            initfs_source, (unsigned int)initfs_words);
                        kboot_source.ks_initfs_base = resident_end;
                        kboot_source.ks_initfs_words = initfs_words;
                } else {
                        kboot_source.ks_initfs_base = 0;
                        kboot_source.ks_initfs_words = 0;
                }
                for (n = 0U; n < 7U; n++) {
                        if (mresr_load(payload[n], (unsigned int)stored[n],
                            bases[n], (kword_t *)(unsigned long)bases[n],
                            (unsigned int)extent[n]) != 0)
                                return -EINVAL;
                        providers[n].mp_id = ids[n];
                        providers[n].mp_base = bases[n];
                        providers[n].mp_resident_words =
                            (unsigned int)extent[n];
                }
                for (n = 0U; n < 7U; n++) {
                        if (mresr_bind(payload[n], (unsigned int)stored[n],
                            (kword_t *)(unsigned long)bases[n],
                            (unsigned int)extent[n], providers, 7U) != 0)
                                return -EINVAL;
                }
                if (mresb_apply(core_bind_source,
                    (unsigned int)core_bind_words,
                    (kword_t *)(unsigned long)kcore_base,
                    (unsigned int)kcore_words, DAIMOD_ID_MRESCORE,
                    bases[0], (unsigned int)extent[0]) != 0 ||
                    kboot_bind_vfs_fs_gates(bases[3], bases[2]) != 0 ||
                    kboot_bind_vfs_gates(bases[3]) != 0 ||
                    kboot_bind_device_gates(bases[4]) != 0 ||
                    kboot_bind_io_gates(bases[5]) != 0 ||
                    kboot_bind_driver_gates(bases[6]) != 0)
                        return -EINVAL;
                resident_end = packed_end;
                mres_io_stage(0, 0, bases[5]);
                mres_driver_stage(0, 0, bases[6]);
#ifdef DAIMON_RUNTIME_MRES
        } else {
                return -EINVAL;
        }
#else
        } else if (io_format == DMANIF_FMT_MREL1 &&
            driver_format == DMANIF_FMT_MREL1 &&
            (mres_device_source == 0 || device_format == DMANIF_FMT_MREL1) &&
            (mres_vfs_source == 0 || vfs_format == DMANIF_FMT_MREL1) &&
            (mres_fs_source == 0 || fs_format == DMANIF_FMT_MREL1)) {
                struct mresr_input inputs[5];
                kword_t bases[5];
                unsigned int input_count;
                unsigned int io_index;
                unsigned int vfs_index;
                unsigned int fs_index;
                unsigned int driver_index;
                kword_t packed_end;
                kword_t memory_limit;
                kword_t packed_start;

                memory_limit = (kword_t)machine_probe_memory_kwords() <<
                    DAIMON_KWORD_SHIFT;
                if (resident_end > memory_limit ||
                    initfs_words > memory_limit - resident_end)
                        return -ENOMEM;
                packed_start = resident_end + initfs_words;
                input_count = 0U;
                if (mres_device_source != 0) {
                        inputs[input_count].mr_payload = mres_device_source;
                        inputs[input_count].mr_payload_words =
                            (unsigned int)device_stored_words;
                        input_count++;
                }
                vfs_index = input_count;
                if (mres_vfs_source != 0) {
                        inputs[input_count].mr_payload = mres_vfs_source;
                        inputs[input_count].mr_payload_words =
                            (unsigned int)vfs_stored_words;
                        input_count++;
                }
                fs_index = input_count;
                if (mres_fs_source != 0) {
                        inputs[input_count].mr_payload = mres_fs_source;
                        inputs[input_count].mr_payload_words =
                            (unsigned int)fs_stored_words;
                        input_count++;
                }
                io_index = input_count;
                inputs[input_count].mr_payload = mres_io_source;
                inputs[input_count].mr_payload_words =
                    (unsigned int)io_stored_words;
                input_count++;
                driver_index = input_count;
                inputs[input_count].mr_payload = mres_driver_source;
                inputs[input_count].mr_payload_words =
                    (unsigned int)driver_stored_words;
                input_count++;
                if (mresr_preflight(inputs, input_count, packed_start,
                    memory_limit, bases, &packed_end) != 0)
                        return -ENOMEM;
                if (mres_device_source != 0) {
                        kword_t next_base;

                        next_base = mres_vfs_source != 0 ?
                            bases[vfs_index] : bases[io_index];
                        if (next_base - bases[0] != device_resident_words)
                                return -ENOMEM;
                }
                if (mres_vfs_source != 0) {
                        kword_t next_base;

                        next_base = mres_fs_source != 0 ?
                            bases[fs_index] : bases[io_index];
                        if (next_base - bases[vfs_index] != vfs_resident_words)
                                return -ENOMEM;
                }
                if (mres_fs_source != 0 &&
                    bases[io_index] - bases[fs_index] != fs_resident_words)
                        return -ENOMEM;
                if (bases[driver_index] - bases[io_index] !=
                    io_resident_words ||
                    packed_end - bases[driver_index] != driver_resident_words)
                        return -ENOMEM;

                /* Publish nothing until every final extent has passed. */
                if (initfs_source != 0) {
                        mach_words_copy((kword_t *)(unsigned long)resident_end,
                            initfs_source, (unsigned int)initfs_words);
                        kboot_source.ks_initfs_base = resident_end;
                        kboot_source.ks_initfs_words = initfs_words;
                } else {
                        kboot_source.ks_initfs_base = 0;
                        kboot_source.ks_initfs_words = 0;
                }
                if (mres_device_source != 0 &&
                    (mresr_load(mres_device_source,
                    inputs[0].mr_payload_words, bases[0],
                    (kword_t *)(unsigned long)bases[0],
                    (unsigned int)device_resident_words) != 0 ||
                    kboot_bind_device_gates(bases[0]) != 0))
                        return -EINVAL;
                if (mres_vfs_source != 0 &&
                    (mresr_load(mres_vfs_source,
                    inputs[vfs_index].mr_payload_words, bases[vfs_index],
                    (kword_t *)(unsigned long)bases[vfs_index],
                    (unsigned int)vfs_resident_words) != 0 ||
                    kboot_bind_vfs_gates(bases[vfs_index]) != 0))
                        return -EINVAL;
                if (mres_fs_source != 0 &&
                    (mresr_load(mres_fs_source,
                    inputs[fs_index].mr_payload_words, bases[fs_index],
                    (kword_t *)(unsigned long)bases[fs_index],
                    (unsigned int)fs_resident_words) != 0 ||
                    mres_vfs_source == 0 ||
                    kboot_bind_vfs_fs_gates(bases[vfs_index],
                    bases[fs_index]) != 0))
                        return -EINVAL;
                if (mresr_load(mres_io_source,
                    inputs[io_index].mr_payload_words, bases[io_index],
                    (kword_t *)(unsigned long)bases[io_index],
                    (unsigned int)io_resident_words) != 0 ||
                    mresr_load(mres_driver_source,
                    inputs[driver_index].mr_payload_words, bases[driver_index],
                    (kword_t *)(unsigned long)bases[driver_index],
                    (unsigned int)driver_resident_words) != 0 ||
                    kboot_bind_io_gates(bases[io_index]) != 0 ||
                    kboot_bind_driver_gates(bases[driver_index]) != 0)
                        return -EINVAL;
                resident_end = packed_end;
                mres_io_stage(0, 0, bases[io_index]);
                mres_driver_stage(0, 0, bases[driver_index]);
        } else
#endif /* DAIMON_RUNTIME_MRES */
#endif /* __PDP10__ */
#ifndef DAIMON_RUNTIME_MRES
        {
                /*
                 * Legacy RAW provider path.  Preserve INITFS before fixed
                 * MRES copies because the trailer may overlap destinations.
                 */
                if (initfs_source != 0) {
                        mach_words_copy((kword_t *)(unsigned long)resident_end,
                            initfs_source, (unsigned int)initfs_words);
                        kboot_source.ks_initfs_base = resident_end;
                        kboot_source.ks_initfs_words = initfs_words;
                        resident_end = kboot_word_addr_add(resident_end,
                            initfs_words);
                } else {
                        kboot_source.ks_initfs_base = 0;
                        kboot_source.ks_initfs_words = 0;
                }
                if (mres_device_source != 0 || mres_vfs_source != 0 ||
                    mres_fs_source != 0 ||
                    io_format != DMANIF_FMT_RAW ||
                    driver_format != DMANIF_FMT_RAW)
                        return -EINVAL;
                mres_io_stage((kword_t)mres_io_source,
                    (kword_t)MRES_IO_STORED_WORDS, (kword_t)MRES_IO_BASE);
                mres_driver_stage((kword_t)mres_driver_source,
                    (kword_t)MRES_DRV_STORED_WORDS, (kword_t)MRES_DRV_BASE);
                if (kboot_bind_io_gates((kword_t)MRES_IO_BASE) != 0 ||
                    kboot_bind_driver_gates((kword_t)MRES_DRV_BASE) != 0)
                        return -EINVAL;
        }
#endif /* !DAIMON_RUNTIME_MRES */
        mem_set_resident_end(resident_end);
        return 0;
}

struct kinit_boot_source *
kinit_boot_source(void)
{
        return &kboot_source;
}
