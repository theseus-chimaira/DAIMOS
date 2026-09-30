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
        if (root_class == KINIT_ROOT_AUTO || root_class == KINIT_ROOT_DSK) {
                if (d6fs_boot_select(KINIT_ROOT_DSK, ordinal) == 0) {
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
