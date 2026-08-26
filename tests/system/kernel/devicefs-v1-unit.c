#include <assert.h>
#include <stdio.h>
#include "devicefs_v1.h"

static void
set_name(struct vfs_v1_name *name, kword_t word, unsigned int chars)
{
        assert(vfs_v1_name_set6(name, word, chars) == 0);
}

int
main(void)
{
        vnode_v1_t root;
        vnode_v1_t node;
        struct vfs_v1_name name;
        struct vfs_v1_dirent ent;
        struct vfs_v1_stat st;
        unsigned int id;
        unsigned int ch;

        devicefs_v1_init(DEVICEFS_V1_PRESENT(DEVICEFS_V1_DEV_CTY0) |
            DEVICEFS_V1_PRESENT(DEVICEFS_V1_DEV_DSK0) |
            DEVICEFS_V1_PRESENT(DEVICEFS_V1_DEV_DTC0));
        root = devicefs_v1_root();
        assert(VFS_V1_PROVIDER(root) == DEVICEFS_V1_PROVIDER);
        assert(VFS_V1_KIND(root) == DEVICEFS_V1_KIND_ROOT);

        assert(devicefs_v1_readdir(root, 0U, &ent) == 1);
        assert(vfs_v1_name_is6(&ent.name, VFS_V1_SIX6('C','T','Y','0',' ',' '), 4U));
        assert(ent.type == VFS_V1_TYPE_DIR);
        set_name(&name, VFS_V1_SIX6('C','T','Y','0',' ',' '), 4U);
        assert(devicefs_v1_lookup(root, &name, &node) == 0);
        assert(VFS_V1_KIND(node) == DEVICEFS_V1_KIND_CTYDIR);
        assert(devicefs_v1_stat(node, &st) == 0 && st.type == VFS_V1_TYPE_DIR);
        assert(devicefs_v1_readdir(node, 0U, &ent) == 1);
        assert(vfs_v1_name_is6(&ent.name, VFS_V1_SIX6('I','O',' ',' ',' ',' '), 2U));
        assert(ent.type == VFS_V1_TYPE_CHAR);
        assert(devicefs_v1_lookup(node, &ent.name, &node) == 0);
        assert(devicefs_v1_device_id(node, &id) == 0 && id == DEVICEFS_V1_DEV_CTY0);
        set_name(&name, VFS_V1_SIX6('C','T','Y','0',' ',' '), 4U);
        assert(devicefs_v1_lookup(root, &name, &node) == 0);
        assert(devicefs_v1_readdir(node, 1U, &ent) == 1);
        assert(vfs_v1_name_is6(&ent.name, VFS_V1_SIX6('S','T','A','T','U','S'), 6U));
        assert(ent.type == VFS_V1_TYPE_REG);
        assert(devicefs_v1_lookup(node, &ent.name, &node) == 0);
        assert(devicefs_v1_stat(node, &st) == 0 && st.type == VFS_V1_TYPE_REG);
        assert(devicefs_v1_readchar(node, 0U, &ch) == 1 && ch == 'O');
        assert(devicefs_v1_readchar(node, 5U, &ch) == 1 && ch == 'E');
        assert(devicefs_v1_readchar(node, 6U, &ch) == 1 && ch == '\r');
        assert(devicefs_v1_readchar(node, 7U, &ch) == 1 && ch == '\n');
        assert(devicefs_v1_readchar(node, 8U, &ch) == 0);
        assert(devicefs_v1_readdir(root, 1U, &ent) == 1);
        assert(vfs_v1_name_is6(&ent.name, VFS_V1_SIX6('D','T','C','0',' ',' '), 4U));
        assert(ent.type == VFS_V1_TYPE_BLOCK);
        assert(devicefs_v1_readdir(root, 2U, &ent) == 1);
        assert(vfs_v1_name_is6(&ent.name, VFS_V1_SIX6('D','S','K','0',' ',' '), 4U));
        assert(ent.type == VFS_V1_TYPE_BLOCK);
        assert(devicefs_v1_readdir(root, 3U, &ent) == 0);

        set_name(&name, VFS_V1_SIX6('D','S','K','0',' ',' '), 4U);
        assert(devicefs_v1_lookup(root, &name, &node) == 0);
        assert(devicefs_v1_device_id(node, &id) == 0);
        assert(id == DEVICEFS_V1_DEV_DSK0);
        assert(devicefs_v1_device_class(node) == DEVICEFS_V1_CLASS_BLOCK);
        assert(devicefs_v1_stat(node, &st) == 0);
        assert(st.type == VFS_V1_TYPE_BLOCK);
        assert(st.mode == 0600U);

        set_name(&name, VFS_V1_SIX6('M','T','C','0',' ',' '), 4U);
        assert(devicefs_v1_lookup(root, &name, &node) != 0);

        assert(devicefs_v1_set_present(DEVICEFS_V1_DEV_D6SET0, 1) == 0);
        set_name(&name, VFS_V1_SIX6('D','6','S','E','T','0'), 6U);
        assert(devicefs_v1_lookup(root, &name, &node) == 0);
        assert(devicefs_v1_device_class(node) == DEVICEFS_V1_CLASS_MOUNTSRC);
        assert(devicefs_v1_stat(node, &st) == 0);
        assert(st.type == VFS_V1_TYPE_MOUNTSRC);
        assert(devicefs_v1_set_present(DEVICEFS_V1_DEV_D6SET0, 0) == 0);
        assert(devicefs_v1_lookup(root, &name, &node) != 0);

        puts("DEVICEFS v1 unit test PASS");
        return 0;
}
