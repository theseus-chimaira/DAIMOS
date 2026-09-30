/**
 * @file root_select.c
 * @brief Early root-controller discovery and boot handoff construction.
 *
 * Root selection runs as a MINIT, before kinit_boot() mounts the filesystem.
 * The low console-switch bits choose a controller class and an ordinal within
 * that class.  AUTO probes the supported classes in policy order.
 *
 * DSK and DRM roots contain D6FS blocksets.  Their member headers are scanned
 * here so KINIT can identify a complete root set and construct the compact
 * two-word physical-member handoff consumed by the later D6FS boot path.
 * DTC roots are delegated to the TSFS bootstrap selector.
 */

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

/*
 * root_info[] caches one discovered D6FS member per physical unit.
 *
 * Packed layout:
 *
 *   bit 35        valid
 *   bits 9..26    physical sector/block containing the member header
 *   bits 2..5     four-bit blockset membership mask
 *   bits 0..1     this member's logical index within the set
 *
 * Bits 6..8 and 27..34 are unused.
 *
 * Only entries up to the currently scanned physical unit are examined, which
 * lets root_scan_unit() initialize entries lazily as discovery advances.
 */
static kword_t root_info[ROOT_D6FS_UNITS];
static unsigned int root_class_selected = KINIT_ROOT_AUTO;

/**
 * @brief Scan one physical D6FS-capable unit for a root blockset member.
 *
 * Disk units may place the D6FS root marker within the first ROOT_SCAN_LIMIT
 * sectors.  Drum roots use block zero, so the DRM path performs a single
 * probe.  A valid member must identify a logical index represented in its own
 * membership mask.
 *
 * On success root_info[unit] receives the packed member description.  Failure
 * leaves that entry zero.
 *
 * @param root_class Physical controller class, DSK or DRM.
 * @param unit Physical unit number to inspect.
 */
static void
root_scan_unit(unsigned int root_class, unsigned int unit)
{
        kword_t *block;
        kword_t member;
        unsigned int index;
        unsigned int mask;
        unsigned int sector;

#if !KINIT_FULL
        (void)root_class;
#endif
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

/**
 * @brief Select one complete D6FS root set by discovery ordinal.
 *
 * Units are scanned incrementally in physical order.  A set becomes a
 * candidate only when the currently examined member is its lowest logical
 * member and every member named by its mask has already been discovered.
 * This prevents the same set from being counted more than once.
 *
 * The selected set is encoded into kinit_boot_handoff as four packed 18-bit
 * member descriptors, two per 36-bit word.  A descriptor stores the physical
 * unit in bits 16..17 and the member-header location in bits 0..15.  Unused
 * member slots remain all ones.
 *
 * @param root_class DSK or DRM controller class.
 * @param ordinal Zero-based complete-set ordinal within that class.
 * @return 0 when a complete set was selected, -1 when none matched.
 */
static int
root_select_d6fs(unsigned int root_class, unsigned int ordinal)
{
        kword_t handoff[2];
        kword_t info;
        unsigned int index;
        unsigned int mask;
        unsigned int present;
        unsigned int found;
        unsigned int unit;
        unsigned int physical;

        for (unit = 0U; unit < ROOT_D6FS_UNITS; ++unit) {
                root_scan_unit(root_class, unit);
                found = 0U;
                for (physical = 0U; physical <= unit; ++physical) {
                        info = root_info[physical];
                        if ((info & ROOT_INFO_VALID) == 0UL)
                                continue;
                        mask = (unsigned int)((info >> ROOT_INFO_MASK_SHIFT) &
                            017U);
                        index = (unsigned int)(info & 03U);
                        if ((mask & ((1U << index) - 1U)) != 0U)
                                continue;
                        handoff[0] = 0777777777777UL;
                        handoff[1] = 0777777777777UL;
                        present = 0U;
                        for (index = 0U; index <= unit; ++index) {
                                kword_t candidate;
                                kword_t half;
                                unsigned int cindex;

                                candidate = root_info[index];
                                if ((candidate & ROOT_INFO_VALID) == 0UL ||
                                    (unsigned int)((candidate >>
                                    ROOT_INFO_MASK_SHIFT) & 017U) != mask)
                                        continue;
                                cindex = (unsigned int)(candidate & 03U);
                                present |= 1U << cindex;
                                half = ((kword_t)index << 16U) |
                                    ((candidate >> ROOT_INFO_LOC_SHIFT) &
                                    0177777UL);
                                if ((cindex & 1U) == 0U)
                                        handoff[cindex / 2U] =
                                            (handoff[cindex / 2U] & 0777777UL) |
                                            (half << 18U);
                                else
                                        handoff[cindex / 2U] =
                                            (handoff[cindex / 2U] &
                                            0777777000000UL) | half;
                        }
                        if (present != mask)
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

/**
 * @brief Sample console root-selection switches and discover the boot root.
 *
 * This MINIT is the only point at which the console switch selector is read.
 * An explicit class searches only that class.  AUTO tries DSK, then DTC, then
 * DRM.  A successful selector stores both the chosen controller class and any
 * physical handoff needed by the filesystem-specific boot code.
 *
 * Failure is represented by leaving root_class_selected as KINIT_ROOT_AUTO;
 * kinit_boot() later converts that unresolved selection into the fatal ?RT
 * diagnostic after all MINIT processing has completed.
 */
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

/**
 * @brief Return the controller class selected by root_select_minit().
 *
 * @return KINIT_ROOT_DSK, KINIT_ROOT_DTC, or KINIT_ROOT_DRM after successful
 * selection; KINIT_ROOT_AUTO when selection has not succeeded.
 */
unsigned int
root_select_class(void)
{
        return root_class_selected;
}
