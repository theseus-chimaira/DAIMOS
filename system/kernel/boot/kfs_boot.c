#include "kinit.h"
#include "kboot.h"
#include "mm.h"
#include "module.h"
#include "fs_mres.h"

#define TYPE_SHIFT      15U
#define MODE_SHIFT      3U

static void
boot_name6(struct vfs_name *name, kword_t word, unsigned int chars)
{
        unsigned int i;

        name->chars = chars;
        name->words[0] = word;
        for (i = 1U; i < VFS_NAME_WORDS; ++i)
                name->words[i] = 0;
}

/*
 * Mount the optional runtime RAMFS after D6FS is already the namespace root.
 * Disk boot no longer constructs a temporary MEMFS root: D6FS can mount
 * directly on VFS_NODE_NONE, and the disk image already contains the mount
 * point directories needed below.
 */
int
kfs_boot_mount_ramfs(kword_t future_free_words)
{
        struct memfs config;
        struct fs_mres_request req;
        struct vfs_name name;
        struct memfs_node *nodes;
        vnode_t mount_dir;
        vnode_t ramfs_target;
        vnode_t temp_target;
        vnode_t ramfs_mount;
        vnode_t temp_mount;
        unsigned int memfs_service;
        unsigned int i;
        kword_t ramfs_base;
        kword_t ramfs_words;
        kword_t spare_words;
        kword_t *wp;

        if (mm_core_words < KBOOT_RAMFS0_MIN_CORE_WORDS)
                return 0;

        memfs_service = module_service_get(MODULE_SERVICE_MEMFS);
        if (memfs_service == 0U)
                return -1;

        /* Resolve every D6FS mount point before allocating MEMFS storage. */
        boot_name6(&name, VFS_SIX6('M','O','U','N','T',' '), 5U);
        if (vfs_lookup(vfs_namespace_root, &name, &mount_dir) != 0)
                return -1;
        boot_name6(&name, VFS_SIX6('R','A','M','F','S','0'), 6U);
        if (vfs_lookup(mount_dir, &name, &ramfs_target) != 0)
                return -1;
        boot_name6(&name, VFS_SIX6('T','E','M','P',' ',' '), 4U);
        if (vfs_lookup(vfs_namespace_root, &name, &temp_target) != 0)
                return -1;

        ramfs_words = mm_largest_free();
        if (ramfs_words > KBOOT_RAMFS0_MAX_WORDS)
                ramfs_words = KBOOT_RAMFS0_MAX_WORDS;
        spare_words = mm_total_free();
        if (future_free_words > MM_HALF_MASK - spare_words)
                spare_words = MM_HALF_MASK;
        else
                spare_words += future_free_words;
        if (spare_words <= KBOOT_FIRST_USER_RESERVE_WORDS)
                return -1;
        spare_words -= KBOOT_FIRST_USER_RESERVE_WORDS;
        if (ramfs_words > spare_words)
                ramfs_words = spare_words;
        if (ramfs_words < KBOOT_RAMFS0_MIN_WORDS)
                return -1;
        if (mm_alloc(ramfs_words, MM_TYPE_KERNEL_DYNAMIC, 1U,
            MM_ALLOC_LOW, &ramfs_base) != MM_OK)
                return -1;

        /* MEMFS has one resident instance.  Build only its final root node;
         * there is no bootstrap namespace to copy or later tear down. */
        wp = (kword_t *)(unsigned long)ramfs_base;
        for (i = 0U; i < KBOOT_NODE_WORDS; ++i)
                wp[i] = 0;
        nodes = (struct memfs_node *)(unsigned long)ramfs_base;
        nodes[0].meta = ((kword_t)VFS_TYPE_DIR << TYPE_SHIFT) |
            ((kword_t)0777U << MODE_SHIFT) |
            MEMFS_F_USED | MEMFS_F_WRITABLE;

        config.nodes = nodes;
        config.node_count = KBOOT_NODE_COUNT;
        config.pool = (kword_t *)(unsigned long)(ramfs_base +
            KBOOT_NODE_WORDS);
        config.pool_words = (unsigned int)(ramfs_words - KBOOT_NODE_WORDS);
        config.used_words = 0U;
        config.image_data = 0;
        req.op = FS_MRES_OP_MEMFS_INIT;
        req.a = (kword_t)(unsigned long)&config;
        ramfs_mount = VFS_NODE_NONE;
        if ((int)kinit_call18_1(memfs_service,
            (kword_t)(unsigned long)&req) != 0)
                goto fail;

        if (vfs_mount(ramfs_target, MEMFS_PROVIDER, MEMFS_KIND_NODE,
            0U, VFS_MOUNT_RW, &ramfs_mount) != 0)
                goto fail;
        if (vfs_mount(temp_target, MEMFS_PROVIDER, MEMFS_KIND_NODE,
            0U, VFS_MOUNT_RW, &temp_mount) != 0)
                goto fail;
        return 0;

fail:
        if (ramfs_mount != VFS_NODE_NONE && vfs_unmount(ramfs_mount) != 0)
                return -1;
        config.nodes = 0;
        config.node_count = 0U;
        config.pool = 0;
        config.pool_words = 0U;
        config.used_words = 0U;
        config.image_data = 0;
        req.op = FS_MRES_OP_MEMFS_INIT;
        req.a = (kword_t)(unsigned long)&config;
        if ((int)kinit_call18_1(memfs_service,
            (kword_t)(unsigned long)&req) != 0)
                return -1;
        if (mm_free(ramfs_base, MM_TYPE_KERNEL_DYNAMIC, 1U) != MM_OK)
                return -1;
        return -1;
}
