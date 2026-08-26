#include "devicefs_v1.h"

static kword_t devicefs_v1_present;

static const struct devicefs_v1_desc devicefs_v1_devices[DEVICEFS_V1_DEV_COUNT] = {
        { VFS_V1_SIX6('C','T','Y','0',' ',' '), DEVICEFS_V1_META(4U, DEVICEFS_V1_CLASS_CHAR) },
        { VFS_V1_SIX6('C','L','K','0',' ',' '), DEVICEFS_V1_META(4U, DEVICEFS_V1_CLASS_CLOCK) },
        { VFS_V1_SIX6('P','T','R','0',' ',' '), DEVICEFS_V1_META(4U, DEVICEFS_V1_CLASS_CHAR) },
        { VFS_V1_SIX6('P','T','P','0',' ',' '), DEVICEFS_V1_META(4U, DEVICEFS_V1_CLASS_CHAR) },
        { VFS_V1_SIX6('C','R','0',' ',' ',' '), DEVICEFS_V1_META(3U, DEVICEFS_V1_CLASS_CHAR) },
        { VFS_V1_SIX6('C','P','0',' ',' ',' '), DEVICEFS_V1_META(3U, DEVICEFS_V1_CLASS_CHAR) },
        { VFS_V1_SIX6('D','C','S','0',' ',' '), DEVICEFS_V1_META(4U, DEVICEFS_V1_CLASS_CHAR) },
        { VFS_V1_SIX6('G','E','0',' ',' ',' '), DEVICEFS_V1_META(3U, DEVICEFS_V1_CLASS_CHAR) },
        { VFS_V1_SIX6('D','P','Y','0',' ',' '), DEVICEFS_V1_META(4U, DEVICEFS_V1_CLASS_DISPLAY) },
        { VFS_V1_SIX6('T','T','Y','0',' ',' '), DEVICEFS_V1_META(4U, DEVICEFS_V1_CLASS_CHAR) },
        { VFS_V1_SIX6('W','C','N','S','L','S'), DEVICEFS_V1_META(6U, DEVICEFS_V1_CLASS_DISPLAY) },
        { VFS_V1_SIX6('O','C','N','S','L','S'), DEVICEFS_V1_META(6U, DEVICEFS_V1_CLASS_DISPLAY) },
        { VFS_V1_SIX6('D','T','C','0',' ',' '), DEVICEFS_V1_META(4U, DEVICEFS_V1_CLASS_BLOCK) },
        { VFS_V1_SIX6('M','T','C','0',' ',' '), DEVICEFS_V1_META(4U, DEVICEFS_V1_CLASS_BLOCK) },
        { VFS_V1_SIX6('D','S','K','0',' ',' '), DEVICEFS_V1_META(4U, DEVICEFS_V1_CLASS_BLOCK) },
        { VFS_V1_SIX6('S','L','V','0',' ',' '), DEVICEFS_V1_META(4U, DEVICEFS_V1_CLASS_CONTROLLER) },
        { VFS_V1_SIX6('D','6','S','E','T','0'), DEVICEFS_V1_META(6U, DEVICEFS_V1_CLASS_MOUNTSRC) }
};

void
devicefs_v1_init(kword_t present_mask)
{
        devicefs_v1_present = present_mask &
            ((1UL << DEVICEFS_V1_DEV_COUNT) - 1UL);
}

int
devicefs_v1_set_present(unsigned int id, int present)
{
        if (id >= DEVICEFS_V1_DEV_COUNT)
                return -1;
        if (present)
                devicefs_v1_present |= DEVICEFS_V1_PRESENT(id);
        else
                devicefs_v1_present &= ~DEVICEFS_V1_PRESENT(id);
        return 0;
}

vnode_v1_t
devicefs_v1_root(void)
{
        return VFS_V1_NODE(DEVICEFS_V1_PROVIDER, DEVICEFS_V1_KIND_ROOT, 0U);
}

static int
devicefs_v1_is_root(vnode_v1_t node)
{
        return VFS_V1_PROVIDER(node) == DEVICEFS_V1_PROVIDER &&
            VFS_V1_KIND(node) == DEVICEFS_V1_KIND_ROOT;
}

static int
devicefs_v1_is_present(unsigned int id)
{
        return id < DEVICEFS_V1_DEV_COUNT &&
            (devicefs_v1_present & DEVICEFS_V1_PRESENT(id)) != 0;
}

static int
devicefs_v1_is_device(vnode_v1_t node, unsigned int *idp)
{
        unsigned int id;

        if (VFS_V1_PROVIDER(node) != DEVICEFS_V1_PROVIDER ||
            VFS_V1_KIND(node) != DEVICEFS_V1_KIND_DEVICE)
                return 0;
        id = VFS_V1_INDEX(node);
        if (!devicefs_v1_is_present(id))
                return 0;
        if (idp != 0)
                *idp = id;
        return 1;
}

int
devicefs_v1_lookup(vnode_v1_t dir, const struct vfs_v1_name *name,
    vnode_v1_t *nodep)
{
        unsigned int i;
        const struct devicefs_v1_desc *dp;

        if (name == 0 || nodep == 0)
                return -1;
        if (VFS_V1_PROVIDER(dir) == DEVICEFS_V1_PROVIDER &&
            VFS_V1_KIND(dir) == DEVICEFS_V1_KIND_CTYDIR &&
            VFS_V1_INDEX(dir) == DEVICEFS_V1_DEV_CTY0 &&
            devicefs_v1_is_present(DEVICEFS_V1_DEV_CTY0)) {
                if (vfs_v1_name_is6(name,
                    VFS_V1_SIX6('I','O',' ',' ',' ',' '), 2U)) {
                        *nodep = VFS_V1_NODE(DEVICEFS_V1_PROVIDER,
                            DEVICEFS_V1_KIND_DEVICE, DEVICEFS_V1_DEV_CTY0);
                        return 0;
                }
                if (vfs_v1_name_is6(name,
                    VFS_V1_SIX6('S','T','A','T','U','S'), 6U)) {
                        *nodep = VFS_V1_NODE(DEVICEFS_V1_PROVIDER,
                            DEVICEFS_V1_KIND_STATUS, DEVICEFS_V1_DEV_CTY0);
                        return 0;
                }
                return -1;
        }
        if (!devicefs_v1_is_root(dir))
                return -1;
        for (i = 0U; i < DEVICEFS_V1_DEV_COUNT; ++i) {
                if (!devicefs_v1_is_present(i))
                        continue;
                dp = &devicefs_v1_devices[i];
                if (vfs_v1_name_is6(name, dp->name6,
                    DEVICEFS_V1_META_CHARS(dp->meta))) {
                        *nodep = VFS_V1_NODE(DEVICEFS_V1_PROVIDER,
                            i == DEVICEFS_V1_DEV_CTY0 ?
                            DEVICEFS_V1_KIND_CTYDIR : DEVICEFS_V1_KIND_DEVICE,
                            i);
                        return 0;
                }
        }
        return -1;
}

int
devicefs_v1_readdir(vnode_v1_t dir, unsigned int off,
    struct vfs_v1_dirent *ent)
{
        unsigned int i;
        unsigned int visible;
        const struct devicefs_v1_desc *dp;

        if (ent == 0)
                return -1;
        if (VFS_V1_PROVIDER(dir) == DEVICEFS_V1_PROVIDER &&
            VFS_V1_KIND(dir) == DEVICEFS_V1_KIND_CTYDIR &&
            VFS_V1_INDEX(dir) == DEVICEFS_V1_DEV_CTY0 &&
            devicefs_v1_is_present(DEVICEFS_V1_DEV_CTY0)) {
                if (off == 0U) {
                        vfs_v1_name_set6(&ent->name,
                            VFS_V1_SIX6('I','O',' ',' ',' ',' '), 2U);
                        ent->type = VFS_V1_TYPE_CHAR;
                        return 1;
                }
                if (off == 1U) {
                        vfs_v1_name_set6(&ent->name,
                            VFS_V1_SIX6('S','T','A','T','U','S'), 6U);
                        ent->type = VFS_V1_TYPE_REG;
                        return 1;
                }
                return 0;
        }
        if (!devicefs_v1_is_root(dir))
                return -1;
        visible = 0U;
        for (i = 0U; i < DEVICEFS_V1_DEV_COUNT; ++i) {
                if (!devicefs_v1_is_present(i))
                        continue;
                if (visible++ != off)
                        continue;
                dp = &devicefs_v1_devices[i];
                if (vfs_v1_name_set6(&ent->name, dp->name6,
                    DEVICEFS_V1_META_CHARS(dp->meta)) != 0)
                        return -1;
                if (i == DEVICEFS_V1_DEV_CTY0)
                        ent->type = VFS_V1_TYPE_DIR;
                else if (DEVICEFS_V1_META_CLASS(dp->meta) ==
                    DEVICEFS_V1_CLASS_BLOCK)
                        ent->type = VFS_V1_TYPE_BLOCK;
                else if (DEVICEFS_V1_META_CLASS(dp->meta) ==
                    DEVICEFS_V1_CLASS_MOUNTSRC)
                        ent->type = VFS_V1_TYPE_MOUNTSRC;
                else
                        ent->type = VFS_V1_TYPE_CHAR;
                return 1;
        }
        return 0;
}

int
devicefs_v1_stat(vnode_v1_t node, struct vfs_v1_stat *st)
{
        unsigned int id;
        unsigned int class_id;

        if (st == 0)
                return -1;
        if (devicefs_v1_is_root(node)) {
                st->type = VFS_V1_TYPE_DIR;
                st->mode = 0555U;
        } else if (VFS_V1_PROVIDER(node) == DEVICEFS_V1_PROVIDER &&
            VFS_V1_KIND(node) == DEVICEFS_V1_KIND_CTYDIR &&
            VFS_V1_INDEX(node) == DEVICEFS_V1_DEV_CTY0 &&
            devicefs_v1_is_present(DEVICEFS_V1_DEV_CTY0)) {
                st->type = VFS_V1_TYPE_DIR;
                st->mode = 0555U;
        } else if (VFS_V1_PROVIDER(node) == DEVICEFS_V1_PROVIDER &&
            VFS_V1_KIND(node) == DEVICEFS_V1_KIND_STATUS &&
            VFS_V1_INDEX(node) == DEVICEFS_V1_DEV_CTY0 &&
            devicefs_v1_is_present(DEVICEFS_V1_DEV_CTY0)) {
                st->type = VFS_V1_TYPE_REG;
                st->mode = 0444U;
        } else if (devicefs_v1_is_device(node, &id)) {
                class_id = DEVICEFS_V1_META_CLASS(devicefs_v1_devices[id].meta);
                if (class_id == DEVICEFS_V1_CLASS_BLOCK)
                        st->type = VFS_V1_TYPE_BLOCK;
                else if (class_id == DEVICEFS_V1_CLASS_MOUNTSRC)
                        st->type = VFS_V1_TYPE_MOUNTSRC;
                else
                        st->type = VFS_V1_TYPE_CHAR;
                st->mode = 0600U;
        } else {
                return -1;
        }
        st->size_chars = 0;
        st->size_words = 0;
        return 0;
}

int
devicefs_v1_device_id(vnode_v1_t node, unsigned int *idp)
{
        if (idp == 0 || !devicefs_v1_is_device(node, idp))
                return -1;
        return 0;
}

unsigned int
devicefs_v1_device_class(vnode_v1_t node)
{
        unsigned int id;

        if (!devicefs_v1_is_device(node, &id))
                return 0U;
        return DEVICEFS_V1_META_CLASS(devicefs_v1_devices[id].meta);
}


int
devicefs_v1_readchar(vnode_v1_t node, kword_t off, unsigned int *chp)
{
        if (VFS_V1_PROVIDER(node) != DEVICEFS_V1_PROVIDER ||
            VFS_V1_KIND(node) != DEVICEFS_V1_KIND_STATUS ||
            VFS_V1_INDEX(node) != DEVICEFS_V1_DEV_CTY0 ||
            !devicefs_v1_is_present(DEVICEFS_V1_DEV_CTY0))
                return -1;
        return vfs_v1_sixbit_readchar(
            VFS_V1_SIX6('O','N','L','I','N','E'), 6U, off, chp);
}
