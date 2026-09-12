#include "vfs.h"
#include "memfs.h"
#include "fs_mres.h"

#define VFS_MOUNT_BITS_MASK \
        ((kword_t)VFS_MOUNT_MASK << (VFS_KIND_SHIFT + VFS_MOUNT_SHIFT))
#define VFS_INHERIT_MOUNT(source, node) \
        (((node) & ~VFS_MOUNT_BITS_MASK) | ((source) & VFS_MOUNT_BITS_MASK))
#include "devicefs.h"
#include "procfs.h"
#include "dtfs.h"
#include "d6fs_provider.h"

vnode_t vfs_mount_target[VFS_NMOUNT];
vnode_t vfs_mount_root[VFS_NMOUNT];
extern kword_t vfs_mount_ro;

extern void file_unlock_mount(unsigned int mount_id);

#define VFS_DEVICE_ROOT \
    VFS_NODE(DEVICEFS_PROVIDER, DEVICEFS_KIND_ROOT, 0U)
#define VFS_PROC_ROOT \
    VFS_NODE(PROCFS_PROVIDER, PROCFS_KIND_ROOT, 0U)

extern vnode_t vfs_namespace_root;

static inline void
vfs_dirent_set6(struct vfs_dirent *ent, kword_t word,
    unsigned int chars, unsigned int type)
{
        ent->name.chars = chars;
        ent->name.words[0] = word;
        ent->name.words[1] = 0;
        ent->name.words[2] = 0;
        ent->name.words[3] = 0;
        ent->type = type;
}

static inline vnode_t
vfs_follow_mount(vnode_t node)
{
        unsigned int i;

        for (i = 0U; i < VFS_NMOUNT; ++i)
                if (vfs_mount_target[i] == node)
                        return vfs_mount_root[i];
        return node;
}

int
vfs_readonly(vnode_t node)
{
        unsigned int id;

        id = VFS_MOUNT_ID(node);
        if (id == 0U || id > VFS_NMOUNT ||
            vfs_mount_root[id - 1U] == VFS_NODE_NONE)
                return 0;
        return (vfs_mount_ro & ((kword_t)1UL << (id - 1U))) != 0;
}

int
vfs_mount(vnode_t target, unsigned int provider,
    unsigned int kind, unsigned int index, unsigned int flags,
    vnode_t *rootp)
{
        struct vfs_stat st;
        vnode_t root;
        unsigned int i;
        unsigned int id;

        if (rootp == 0 || provider == 0U ||
            provider > VFS_PROVIDER_MASK || kind > VFS_LOCAL_KIND_MASK ||
            index > VFS_INDEX_MASK || flags > VFS_MOUNT_RDONLY)
                return -1;
        if (target == VFS_NODE_NONE) {
                if (vfs_namespace_root != VFS_NODE_NONE)
                        return -1;
        } else if (vfs_stat(target, &st) != 0 ||
            st.type != VFS_TYPE_DIR)
                return -1;
        for (i = 0U; i < VFS_NMOUNT; ++i) {
                if (vfs_mount_root[i] == VFS_NODE_NONE)
                        break;
                if (vfs_mount_target[i] == target)
                        return -1;
        }
        if (i == VFS_NMOUNT)
                return -1;
        id = i + 1U;
        root = VFS_NODE_PACKED(provider,
            (id << VFS_MOUNT_SHIFT) | kind, index);
        vfs_mount_target[i] = target;
        vfs_mount_root[i] = root;
        if (flags == VFS_MOUNT_RDONLY)
                vfs_mount_ro |= (kword_t)1UL << i;
        else
                vfs_mount_ro &= ~((kword_t)1UL << i);
        *rootp = root;
        if (target == VFS_NODE_NONE)
                vfs_namespace_root = root;
        return 0;
}

int
vfs_unmount(vnode_t root)
{
        struct fs_mres_request req;
        unsigned int id;
        unsigned int i;

        id = VFS_MOUNT_ID(root);
        if (id == 0U || id > VFS_NMOUNT)
                return -1;
        i = id - 1U;
        if (vfs_mount_root[i] != root || vfs_sync(root) != 0)
                return -1;
        if (VFS_PROVIDER(root) == D6FS_PROVIDER) {
                req.op = FS_MRES_OP_PREPARE_UNMOUNT;
                req.a = root;
                if (fs_provider_call(D6FS_PROVIDER, &req) != 0)
                        return -1;
        }
        if (vfs_namespace_root == root)
                vfs_namespace_root = vfs_mount_target[i];
        file_unlock_mount(id);
        vfs_mount_target[i] = VFS_NODE_NONE;
        vfs_mount_root[i] = VFS_NODE_NONE;
        vfs_mount_ro &= ~((kword_t)1UL << i);
        return 0;
}
