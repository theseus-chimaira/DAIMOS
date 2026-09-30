#include "kinit.h"
#include "d6fs_boot.h"
#include "root_select.h"
#include "tsfs_boot.h"

static void
root_name(struct vfs_name *name, kword_t word, unsigned int chars)
{
        unsigned int i;

        name->chars = chars;
        name->words[0] = word;
        for (i = 1U; i < VFS_NAME_WORDS; ++i)
                name->words[i] = 0UL;
}

static int
root_has_init(void)
{
        struct vfs_name name;
        struct vfs_stat st;
        vnode_t system;
        vnode_t init;

        root_name(&name, VFS_SIX6('S', 'Y', 'S', 'T', 'E', 'M'), 6U);
        if (vfs_lookup(vfs_namespace_root, &name, &system) != 0)
                return 0;
        root_name(&name, VFS_SIX6('I', 'N', 'I', 'T', ' ', ' '), 4U);
        if (vfs_lookup(system, &name, &init) != 0 ||
            vfs_stat(init, &st) != 0 || st.type != VFS_TYPE_REG)
                return 0;
        return 1;
}

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
