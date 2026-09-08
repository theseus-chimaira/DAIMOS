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

static vnode_t vfs_mount_target[VFS_NMOUNT];
static vnode_t vfs_mount_root[VFS_NMOUNT];
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
        root = VFS_NODE(provider, VFS_MOUNT_KIND(id, kind), index);
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

int
vfs_lookup(vnode_t dir, const struct vfs_name *name,
    vnode_t *nodep)
{
        struct fs_mres_request req;
        vnode_t node;
        unsigned int provider;
        int rc;

        if (name == 0 || nodep == 0)
                return -1;
        if (dir == vfs_namespace_root) {
                if (vfs_name_is6(name,
                    VFS_SIX6('D','E','V','I','C','E'), 6U)) {
                        *nodep = VFS_DEVICE_ROOT;
                        return 0;
                }
                if (vfs_name_is6(name,
                    VFS_SIX6('P','R','O','C',' ',' '), 4U)) {
                        *nodep = VFS_PROC_ROOT;
                        return 0;
                }
        }
        provider = VFS_PROVIDER(dir);
        if (provider == DEVICEFS_PROVIDER)
                rc = devicefs_lookup(dir, name, &node);
        else if (provider == PROCFS_PROVIDER)
                rc = procfs_lookup(dir, name, &node);
        else {
                req.op = FS_MRES_OP_LOOKUP;
                req.a = dir;
                req.b = (kword_t)(unsigned long)name;
                req.c = (kword_t)(unsigned long)&node;
                rc = fs_provider_call(provider, &req);
        }
        if (rc != 0)
                return rc;
        node = VFS_INHERIT_MOUNT(dir, node);
        *nodep = vfs_follow_mount(node);
        return 0;
}

extern int vfs_readdir_raw(vnode_t dir, unsigned int off,
    struct vfs_dirent *ent);

int
vfs_readdir(vnode_t dir, unsigned int off, struct vfs_dirent *ent)
{
        unsigned int base;
        int rc;

        if (ent == 0)
                return -1;
        rc = vfs_readdir_raw(dir, off, ent);
        if (rc != 0 || dir != vfs_namespace_root)
                return rc;
        base = 0U;
        while (vfs_readdir_raw(dir, base, ent) > 0)
                ++base;
        if (off == base) {
                vfs_dirent_set6(ent,
                    VFS_SIX6('D','E','V','I','C','E'), 6U,
                    VFS_TYPE_DIR);
                return 1;
        }
        if (off == base + 1U) {
                vfs_dirent_set6(ent,
                    VFS_SIX6('P','R','O','C',' ',' '), 4U,
                    VFS_TYPE_DIR);
                return 1;
        }
        return 0;
}

static int
vfs_parent_raw(vnode_t node, vnode_t *parentp)
{
        struct fs_mres_request req;
        unsigned int provider;
        unsigned int kind;

        if (parentp == 0)
                return -1;
        provider = VFS_PROVIDER(node);
        if (provider == DEVICEFS_PROVIDER) {
                *parentp = vfs_namespace_root;
                return 0;
        }
        if (provider == PROCFS_PROVIDER) {
                kind = VFS_LOCAL_KIND(node);
                if (kind == PROCFS_KIND_ROOT) {
                        *parentp = vfs_namespace_root;
                        return 0;
                }
                if (kind == PROCFS_KIND_PROC) {
                        *parentp = VFS_PROC_ROOT;
                        return 0;
                }
                return -1;
        }
        {
                vnode_t parent;
                int rc;

                req.op = FS_MRES_OP_PARENT;
                req.a = node;
                req.b = (kword_t)(unsigned long)&parent;
                req.c = 0;
                rc = fs_provider_call(provider, &req);
                if (rc != 0)
                        return rc;
                *parentp = VFS_INHERIT_MOUNT(node, parent);
                return 0;
        }
}

int
vfs_parent(vnode_t node, vnode_t *parentp)
{
        unsigned int id;

        if (parentp == 0)
                return -1;
        if (node == vfs_namespace_root) {
                *parentp = node;
                return 0;
        }

        id = VFS_MOUNT_ID(node);
        if (id != 0U && id <= VFS_NMOUNT &&
            vfs_mount_root[id - 1U] == node)
                return vfs_parent_raw(vfs_mount_target[id - 1U],
                    parentp);
        return vfs_parent_raw(node, parentp);
}

/*
 * Return the parent and namespace name of a directory while crossing a mount
 * root back through its mountpoint.
 */
int
vfs_parent_name(vnode_t node, vnode_t *parentp,
    struct vfs_name *namep)
{
        struct fs_mres_request req;
        unsigned int id;
        unsigned int provider;

        if (parentp == 0 || namep == 0 || node == vfs_namespace_root)
                return -1;
        id = VFS_MOUNT_ID(node);
        if (id != 0U && id <= VFS_NMOUNT &&
            vfs_mount_root[id - 1U] == node)
                node = vfs_mount_target[id - 1U];
        provider = VFS_PROVIDER(node);
        {
                vnode_t parent;
                int rc;

                req.op = FS_MRES_OP_PARENT_NAME;
                req.a = node;
                req.b = (kword_t)(unsigned long)&parent;
                req.c = (kword_t)(unsigned long)namep;
                rc = fs_provider_call(provider, &req);
                if (rc != 0)
                        return rc;
                *parentp = VFS_INHERIT_MOUNT(node, parent);
                return 0;
        }
}

int
vfs_create_op(unsigned int op, vnode_t dir, const struct vfs_name *name,
    unsigned int mode, vnode_t *nodep)
{
        struct fs_mres_request req;
        vnode_t node;
        int rc;

        if (nodep == 0)
                return -1;
        if (op == FS_MRES_OP_MKDIR && VFS_PROVIDER(dir) == DTFS_PROVIDER)
                return VFS_ERR_UNSUPPORTED;
        if (vfs_readonly(dir))
                return -1;
        req.op = op;
        req.a = dir;
        req.b = (kword_t)(unsigned long)name;
        req.c = (kword_t)mode;
        req.d = (kword_t)(unsigned long)&node;
        rc = fs_provider_call(VFS_PROVIDER(dir), &req);
        if (rc != 0)
                return rc;
        *nodep = VFS_INHERIT_MOUNT(dir, node);
        return 0;
}

int
vfs_symlink(vnode_t dir, const struct vfs_name *name,
    const kword_t *target, unsigned int target_chars, vnode_t *nodep)
{
        struct fs_mres_request req;
        vnode_t node;
        int rc;

        if (nodep == 0 || vfs_readonly(dir))
                return -1;
        req.op = FS_MRES_OP_SYMLINK;
        req.a = dir;
        req.b = (kword_t)(unsigned long)name;
        req.c = (kword_t)(unsigned long)target;
        req.d = (kword_t)target_chars;
        req.e = (kword_t)(unsigned long)&node;
        rc = fs_provider_call(VFS_PROVIDER(dir), &req);
        if (rc != 0)
                return rc;
        *nodep = VFS_INHERIT_MOUNT(dir, node);
        return 0;
}

int
vfs_rename(vnode_t olddir, const struct vfs_name *oldname,
    vnode_t newdir, const struct vfs_name *newname)
{
        struct fs_mres_request req;
        unsigned int provider;

        if (vfs_readonly(olddir) || vfs_readonly(newdir) ||
            VFS_PROVIDER(olddir) != VFS_PROVIDER(newdir) ||
            VFS_MOUNT_ID(olddir) != VFS_MOUNT_ID(newdir))
                return -1;
        provider = VFS_PROVIDER(olddir);
        req.op = FS_MRES_OP_RENAME;
        req.a = olddir;
        req.b = (kword_t)(unsigned long)oldname;
        req.c = newdir;
        req.d = (kword_t)(unsigned long)newname;
        return fs_provider_call(provider, &req);
}

extern int vfs_name_is6(const struct vfs_name *name, kword_t word,
    unsigned int chars);
extern int vfs_sixbit_readchar(kword_t word, unsigned int nchars, kword_t off,
    unsigned int *chp);
