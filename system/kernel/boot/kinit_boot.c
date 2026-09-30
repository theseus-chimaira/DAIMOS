/**
 * @file kinit_boot.c
 * @brief Mount and validate the root filesystem selected during MINIT.
 *
 * Root discovery is deliberately separate from root mounting.  The
 * root_select MINIT samples the console switches, discovers the requested
 * physical root, and records its boot handoff while MINIT services are still
 * available.  This file consumes that completed selection later in KINIT,
 * mounts the corresponding filesystem, and performs the final bootability
 * check.
 *
 * DSK and DRM roots contain D6FS.  DTC roots contain TSFS.  A root is accepted
 * for normal boot only when /SYSTEM/INIT exists and is a regular file.
 */

#include "kinit.h"
#include "d6fs_boot.h"
#include "root_select.h"
#include "tsfs_boot.h"

/**
 * @brief Construct one SIXBIT VFS path component.
 *
 * VFS names carry their logical character count separately from their packed
 * storage.  This matters for names such as INIT: the SIXBIT word is padded to
 * six characters, while chars remains four so the padding is not part of the
 * pathname component.
 *
 * @param name Destination VFS name structure.
 * @param word First packed six-character SIXBIT word.
 * @param chars Logical number of characters in the component.
 */
static void
root_name(struct vfs_name *name, kword_t word, unsigned int chars)
{
        unsigned int i;

        name->chars = chars;
        name->words[0] = word;
        for (i = 1U; i < VFS_NAME_WORDS; ++i)
                name->words[i] = 0UL;
}

/**
 * @brief Verify that the mounted namespace contains a usable system init.
 *
 * The root filesystem is considered bootable only if SYSTEM can be looked up
 * directly below the namespace root and SYSTEM/INIT exists as a regular file.
 * Merely finding an object named INIT is insufficient: directories or other
 * vnode types must not satisfy the boot check.
 *
 * @return Nonzero when /SYSTEM/INIT is a regular file, zero otherwise.
 */
static int
root_has_init(void)
{
        struct vfs_name name;
        struct vfs_stat st;
        vnode_t system;
        vnode_t init;

        root_name(&name, PDP10_SIX6('S', 'Y', 'S', 'T', 'E', 'M'), 6U);
        if (vfs_lookup(vfs_namespace_root, &name, &system) != 0)
                return 0;
        root_name(&name, PDP10_SIX6('I', 'N', 'I', 'T', ' ', ' '), 4U);
        if (vfs_lookup(system, &name, &init) != 0 ||
            vfs_stat(init, &st) != 0 || st.type != VFS_TYPE_REG)
                return 0;
        return 1;
}

/**
 * @brief Mount the previously selected root filesystem and validate it.
 *
 * root_select_minit() has already sampled the console switches and selected
 * the physical root before this function runs.  This routine therefore does
 * no discovery: it dispatches the selected controller class to the matching
 * filesystem boot path.  DSK and DRM use D6FS; DTC uses TSFS in full builds.
 *
 * Failure to mount, an unsupported/unresolved root class, or a root lacking a
 * regular /SYSTEM/INIT is fatal and emits the compact ?RT early-boot code.
 */
void
kinit_boot(void)
{
        unsigned int root_class;
        int rc;

        root_class = root_select_class();
#if KINIT_FULL
        if (root_class == KINIT_ROOT_DSK || root_class == KINIT_ROOT_DRM)
#else
        if (root_class == KINIT_ROOT_DSK)
#endif
                rc = d6fs_boot_mount_root(0U);
#if KINIT_FULL
        else if (root_class == KINIT_ROOT_DTC)
                rc = tsfs_boot_mount_root();
#endif
        else
                rc = -1;
        if (rc != 0 || !root_has_init())
                kinit_error18(KINIT_ERR_RT);
}
