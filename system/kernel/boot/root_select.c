/**
 * @file root_select.c
 * @brief Early root-controller discovery and boot handoff construction.
 *
 * Root selection runs as a MINIT, before kinit_boot() mounts the filesystem.
 * The low console-switch bits choose a controller class and an ordinal within
 * that class.  AUTO probes the supported classes in policy order.
 *
 * This file owns only controller-class policy.  Filesystem-specific discovery
 * is delegated to d6fs_boot_select() for DSK/DRM and tsfs_boot_select() for
 * DTC, keeping physical/media interpretation in the corresponding boot layer.
 */

#include "root_select.h"
#include "kinit.h"
#include "d6fs_boot.h"
#include "tsfs_boot.h"
#include "vfs_mount_capacity_v1.h"
#include "dsk270.h"
#include "module.h"
#if KINIT_FULL
#include "drm236.h"
#endif

static unsigned int root_class_selected = KINIT_ROOT_AUTO;

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
#if D6FS_FS_ENABLED
        if (root_class == KINIT_ROOT_AUTO || root_class == KINIT_ROOT_DSK) {
                if (d6fs_boot_select(KINIT_ROOT_DSK, ordinal) == 0) {
                        root_class_selected = KINIT_ROOT_DSK;
                        return;
                }
                if (root_class == KINIT_ROOT_DSK)
                        return;
        }
#endif
#if KINIT_TAPE_BOOT
        if (root_class == KINIT_ROOT_AUTO || root_class == KINIT_ROOT_DTC) {
                if (tsfs_boot_select(ordinal) == 0) {
                        root_class_selected = KINIT_ROOT_DTC;
                        return;
                }
                if (root_class == KINIT_ROOT_DTC)
                        return;
        }
#endif
#if D6FS_FS_ENABLED && KINIT_FULL
        if (root_class == KINIT_ROOT_AUTO || root_class == KINIT_ROOT_DRM) {
                if (d6fs_boot_select(KINIT_ROOT_DRM, ordinal) == 0) {
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

/* Root discovery already checked all D6FS member descriptors. Each 18-bit
 * handoff half carries the selected *physical* unit in bits 16..17 or is
 * 0777777 when absent. Do not use the logical member mask as a unit mask:
 * members may have been assembled from nonconsecutive physical drives. */
static unsigned int
root_select_d6fs_units_v22(void)
{
        unsigned int mask;
        unsigned int i;

        mask = 0U;
        for (i = 0U; i < 4U; ++i) {
                kword_t half;
                unsigned int unit;

                half = (kinit_boot_handoff[i / 2U] >>
                    ((i & 1U) != 0U ? 0U : 18U)) & 0777777UL;
                if (half == 0777777UL)
                        continue;
                unit = (unsigned int)((half >> 16U) & 03U);
                if ((mask & (1U << unit)) != 0U)
                        return 0U;      /* duplicate selected physical unit */
                mask |= 1U << unit;
        }
        return mask;
}

unsigned int
root_select_mount_capacity_v22(void)
{
        struct vfs_mount_inventory_v1 inv;

        /* MINIT publishes device services only after successful controller
         * probes. Count supported positions (even when media is empty) on
         * detected transports; absent controllers must cost no slab RAM. */
        /* D6FS MRES is not installed on a TSFS root, and DTFS/TSFS MRES is
         * only installed on a DTC root. Hardware without its filesystem
         * provider cannot currently produce another VFS mount. Excluding
         * it saves six dynamic words per otherwise-wasted mount slot. */
        inv.disk_units = D6FS_FS_ENABLED &&
            root_class_selected != KINIT_ROOT_DTC &&
            module_service_get(MODULE_SERVICE_DSK_READ_SECTOR) != 0U &&
            module_service_get(MODULE_SERVICE_DSK_WRITE_SECTOR) != 0U ?
            DSK270_UNITS : 0U;
#if KINIT_FULL
        inv.drum_units = D6FS_FS_ENABLED &&
            root_class_selected != KINIT_ROOT_DTC &&
            module_service_get(MODULE_SERVICE_DRM_READ_BLOCK) != 0U &&
            module_service_get(MODULE_SERVICE_DRM_WRITE_BLOCK) != 0U ?
            DRM236_UNITS : 0U;
        inv.memfs_enabled = 1U;          /* MEMFS MINIT is in full builds */
#else
        inv.drum_units = 0U;
        inv.memfs_enabled = 0U;
#endif
#if KINIT_TAPE_BOOT
        inv.tape_units = root_class_selected == KINIT_ROOT_DTC &&
            module_service_get(MODULE_SERVICE_DTC_READ_BLOCK) != 0U &&
            module_service_get(MODULE_SERVICE_DTC_WRITE_BLOCK) != 0U ?
            8U : 0U;                    /* installed DTC, 8 transport slots */
#else
        inv.tape_units = 0U;
#endif
        inv.root_disk_members = 0U;
        inv.root_drum_members = 0U;
        inv.root_tape_members = 0U;

        if (root_class_selected == KINIT_ROOT_DSK)
                inv.root_disk_members = root_select_d6fs_units_v22();
#if D6FS_FS_ENABLED && KINIT_FULL
        else if (root_class_selected == KINIT_ROOT_DRM)
                inv.root_drum_members = root_select_d6fs_units_v22();
#endif
#if KINIT_TAPE_BOOT
        else if (root_class_selected == KINIT_ROOT_DTC)
                inv.root_tape_members = tsfs_boot_selected_unit_mask_v22();
#endif
        /* The planner validates one selected root, capacity and every unit
         * mask before MM allocation. Zero is a hard boot configuration error,
         * never a request to silently allocate four fallback slots. */
        return vfs_mount_capacity_v1(&inv);
}
