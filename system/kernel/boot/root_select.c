#include "root_select.h"
#include "kinit.h"
#include "d6fs_boot.h"
#include "dsk270.h"
#if KINIT_FULL
#include "drm236.h"
#endif
#include "blockset_layout.h"
#include "tsfs_boot.h"

#define ROOT_D6FS_UNITS      4U
#define ROOT_SCAN_LIMIT      0200U
#define ROOT_INFO_VALID      0400000000000UL
#define ROOT_INFO_LOC_SHIFT  9U
#define ROOT_INFO_MASK_SHIFT 2U

static kword_t root_info[ROOT_D6FS_UNITS];
static unsigned int root_class_selected = KINIT_ROOT_AUTO;

static void
root_scan_unit(unsigned int root_class, unsigned int unit)
{
        kword_t *block;
        kword_t member;
        unsigned int index;
        unsigned int mask;
        unsigned int sector;

        block = d6fs_boot_block_buffer();
        root_info[unit] = 0UL;
#if KINIT_FULL
        for (sector = 0U; sector < (root_class == KINIT_ROOT_DRM ? 1U :
            ROOT_SCAN_LIMIT); ++sector) {
                if ((root_class == KINIT_ROOT_DRM ?
            drm236_read_block(unit, (kword_t)sector, block) :
            dsk270_read_sector(unit, (kword_t)sector, block)) != 0)
                        return;
#else
        for (sector = 0U; sector < ROOT_SCAN_LIMIT; ++sector) {
                if (dsk270_read_sector(unit, (kword_t)sector, block) != 0)
                        return;
#endif
                if (block[BLOCKSET_LAYOUT_MAGIC_WORD] !=
                    BLOCKSET_LAYOUT_MAGIC_D6FSR2)
                        continue;
                member = block[5];
                mask = (unsigned int)((member >> 20U) & 017U);
                index = (unsigned int)((member >> 16U) & 017U);
                if (index >= ROOT_D6FS_UNITS ||
                    (mask & (1U << index)) == 0U)
                        continue;
                root_info[unit] = ROOT_INFO_VALID |
                    ((kword_t)sector << ROOT_INFO_LOC_SHIFT) |
                    ((kword_t)mask << ROOT_INFO_MASK_SHIFT) |
                    (kword_t)index;
                return;
        }
}

static int
root_select_d6fs(unsigned int root_class, unsigned int ordinal)
{
        kword_t handoff[2];
        kword_t info;
        unsigned int index;
        unsigned int mask;
        unsigned int member;
        unsigned int scan_unit;
        unsigned int unit;
        unsigned int physical;
        unsigned int found;

        for (unit = 0U; unit < ROOT_D6FS_UNITS; ++unit)
                root_info[unit] = 0UL;
        for (scan_unit = 0U; scan_unit < ROOT_D6FS_UNITS; ++scan_unit) {
                root_scan_unit(root_class, scan_unit);
                found = 0U;
                for (unit = 0U; unit <= scan_unit; ++unit) {
                        info = root_info[unit];
                        if ((info & ROOT_INFO_VALID) == 0UL)
                                continue;
                        mask = (unsigned int)((info >> ROOT_INFO_MASK_SHIFT) & 017U);
                        index = (unsigned int)(info & 03U);
                        if ((mask & ((1U << index) - 1U)) != 0U)
                                continue;
                        handoff[0] = 0777777777777UL;
                        handoff[1] = 0777777777777UL;
                        for (member = 0U; member < ROOT_D6FS_UNITS; ++member) {
                                if ((mask & (1U << member)) == 0U)
                                        continue;
                                for (physical = 0U; physical <= scan_unit;
                                    ++physical) {
                                        kword_t candidate;
                                        unsigned int cmask;
                                        unsigned int cindex;

                                        candidate = root_info[physical];
                                        if ((candidate & ROOT_INFO_VALID) == 0UL)
                                                continue;
                                        cmask = (unsigned int)((candidate >>
                                            ROOT_INFO_MASK_SHIFT) & 017U);
                                        cindex = (unsigned int)(candidate & 03U);
                                        if (cmask == mask && cindex == member)
                                                break;
                                }
                                if (physical > scan_unit)
                                        break;
                                {
                                        kword_t half;
                                        half = ((kword_t)physical << 16U) |
                                            ((root_info[physical] >> ROOT_INFO_LOC_SHIFT) &
                                            0177777UL);
                                        if ((member & 1U) == 0U)
                                                handoff[member / 2U] =
                                                    (handoff[member / 2U] & 0777777UL) |
                                                    (half << 18U);
                                        else
                                                handoff[member / 2U] =
                                                    (handoff[member / 2U] &
                                                    0777777000000UL) | half;
                                }
                        }
                        if (member != ROOT_D6FS_UNITS &&
                            (mask & (1U << member)) != 0U)
                                continue;
                        if (found++ != ordinal)
                                continue;
                        kinit_boot_handoff[0] = handoff[0];
                        kinit_boot_handoff[1] = handoff[1];
                        return 0;
                }
        }
        return -1;
}

void
root_select_minit(void)
{
        unsigned int root_class;
        unsigned int ordinal;
        kword_t selector;

        kinit_boot_handoff[0] = 0777777777777UL;
        kinit_boot_handoff[1] = 0777777777777UL;
        selector = kinit_read_switches() & KINIT_ROOT_SELECT_MASK;
        root_class = (unsigned int)(selector & KINIT_ROOT_CLASS_MASK);
        ordinal = (unsigned int)((selector & KINIT_ROOT_ORD_MASK) >>
            KINIT_ROOT_ORD_SHIFT);
        root_class_selected = KINIT_ROOT_AUTO;
        if (root_class == KINIT_ROOT_AUTO || root_class == KINIT_ROOT_DSK) {
                if (root_select_d6fs(KINIT_ROOT_DSK, ordinal) == 0) {
                        root_class_selected = KINIT_ROOT_DSK;
                        return;
                }
                if (root_class == KINIT_ROOT_DSK)
                        return;
        }
#if KINIT_FULL
        if (root_class == KINIT_ROOT_AUTO || root_class == KINIT_ROOT_DTC) {
                if (tsfs_boot_select(ordinal) == 0) {
                        root_class_selected = KINIT_ROOT_DTC;
                        return;
                }
                if (root_class == KINIT_ROOT_DTC)
                        return;
        }
        if (root_class == KINIT_ROOT_AUTO || root_class == KINIT_ROOT_DRM) {
                if (root_select_d6fs(KINIT_ROOT_DRM, ordinal) == 0) {
                        root_class_selected = KINIT_ROOT_DRM;
                        return;
                }
        }
#endif
}

unsigned int
root_select_class(void)
{
        return root_class_selected;
}
