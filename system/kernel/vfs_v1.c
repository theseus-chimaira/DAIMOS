#include "vfs_v1.h"
#include "memfs_v1.h"
#include "fs_mres.h"
#include "devicefs_v1.h"
#include "procfs_v1.h"
#include "dtfs_v1.h"
#include "d6fs_provider_v2.h"

extern vnode_v1_t file_v1_alias_node;


static int
vfs_v1_mres_call(unsigned int provider, struct fs_mres_request *req)
{
        return fs_provider_call(provider, req);
}

static vnode_v1_t vfs_v1_mount_target[VFS_V1_NMOUNT];
static vnode_v1_t vfs_v1_mount_root[VFS_V1_NMOUNT];
static kword_t vfs_v1_mount_ro;

struct vfs_v1_lock_entry {
        vnode_v1_t node;
        kword_t state;
};

#define VFS_V1_LOCK_EXBIT       ((kword_t)1UL << 35)
#define VFS_V1_LOCK_OWNER_MASK  0377777777777UL
#define VFS_V1_LOCK_OWNER_MAX   32U

static struct vfs_v1_lock_entry vfs_v1_locks[VFS_V1_NLOCK];

static void
vfs_v1_unlock_mount(unsigned int mount_id)
{
        unsigned int i;

        for (i = 0U; i < VFS_V1_NLOCK; ++i)
                if (vfs_v1_locks[i].node != VFS_V1_NODE_NONE &&
                    VFS_V1_MOUNT_ID(vfs_v1_locks[i].node) == mount_id) {
                        vfs_v1_locks[i].node = VFS_V1_NODE_NONE;
                        vfs_v1_locks[i].state = 0UL;
                }
}

int
vfs_v1_lock(vnode_v1_t node, unsigned int owner, unsigned int op)
{
        struct vfs_v1_stat st;
        struct vfs_v1_lock_entry *entry;
        struct vfs_v1_lock_entry *free_entry;
        kword_t bit;
        kword_t owners;
        unsigned int i;

        if (node == VFS_V1_NODE_NONE || owner == 0U || owner > VFS_V1_LOCK_OWNER_MAX ||
            (op != VFS_V1_LOCK_SHARED && op != VFS_V1_LOCK_EXCLUSIVE &&
            op != VFS_V1_LOCK_UNLOCK) || vfs_v1_stat(node, &st) != 0 ||
            st.type != VFS_V1_TYPE_REG)
                return -1;
        entry = 0;
        free_entry = 0;
        for (i = 0U; i < VFS_V1_NLOCK; ++i) {
                if (vfs_v1_locks[i].node == node) {
                        entry = &vfs_v1_locks[i];
                        break;
                }
                if (free_entry == 0 &&
                    vfs_v1_locks[i].node == VFS_V1_NODE_NONE)
                        free_entry = &vfs_v1_locks[i];
        }
        bit = (kword_t)1UL << owner;
        if (op == VFS_V1_LOCK_UNLOCK) {
                if (entry == 0)
                        return 0;
                entry->state &= ~bit;
                if ((entry->state & VFS_V1_LOCK_EXBIT) != 0UL)
                        entry->state &= ~VFS_V1_LOCK_EXBIT;
                if ((entry->state & VFS_V1_LOCK_OWNER_MASK) == 0UL) {
                        entry->node = VFS_V1_NODE_NONE;
                        entry->state = 0UL;
                }
                return 0;
        }
        if (entry == 0) {
                if (free_entry == 0)
                        return -1;
                entry = free_entry;
                entry->node = node;
                entry->state = 0UL;
        }
        owners = entry->state & VFS_V1_LOCK_OWNER_MASK;
        if (op == VFS_V1_LOCK_SHARED) {
                if ((entry->state & VFS_V1_LOCK_EXBIT) != 0UL &&
                    owners != bit)
                        return -1;
                entry->state = (owners | bit);
                return 0;
        }
        if ((owners & ~bit) != 0UL)
                return -1;
        entry->state = VFS_V1_LOCK_EXBIT | bit;
        return 0;
}

void
vfs_v1_unlock_owner(vnode_v1_t node, unsigned int owner)
{
        if (owner != 0U && owner <= VFS_V1_LOCK_OWNER_MAX)
                (void)vfs_v1_lock(node, owner, VFS_V1_LOCK_UNLOCK);
}

#define VFS_V1_MEMFS_ROOT \
    VFS_V1_NODE(MEMFS_V1_PROVIDER, MEMFS_V1_KIND_NODE, 0U)
#define VFS_V1_DEVICE_ROOT \
    VFS_V1_NODE(DEVICEFS_V1_PROVIDER, DEVICEFS_V1_KIND_ROOT, 0U)
#define VFS_V1_PROC_ROOT \
    VFS_V1_NODE(PROCFS_V1_PROVIDER, PROCFS_V1_KIND_ROOT, 0U)

vnode_v1_t vfs_v1_namespace_root = VFS_V1_NODE_NONE;

vnode_v1_t
vfs_v1_root(void)
{
        return vfs_v1_namespace_root;
}

int
vfs_v1_set_root(vnode_v1_t node)
{
        struct vfs_v1_stat st;

        if (node == VFS_V1_NODE_NONE || vfs_v1_stat(node, &st) != 0 ||
            st.type != VFS_V1_TYPE_DIR)
                return -1;
        vfs_v1_namespace_root = node;
        return 0;
}

static void
vfs_v1_dirent_set6(struct vfs_v1_dirent *ent, kword_t word,
    unsigned int chars, unsigned int type)
{
        ent->name.chars = chars;
        ent->name.words[0] = word;
        ent->name.words[1] = 0;
        ent->name.words[2] = 0;
        ent->name.words[3] = 0;
        ent->type = type;
}

vnode_v1_t
vfs_v1_follow_mount(vnode_v1_t node)
{
        unsigned int i;

        for (i = 0U; i < VFS_V1_NMOUNT; ++i)
                if (vfs_v1_mount_target[i] == node)
                        return vfs_v1_mount_root[i];
        return node;
}

int
vfs_v1_readonly(vnode_v1_t node)
{
        unsigned int id;

        id = VFS_V1_MOUNT_ID(node);
        if (id == 0U || id > VFS_V1_NMOUNT ||
            vfs_v1_mount_root[id - 1U] == VFS_V1_NODE_NONE)
                return 0;
        return (vfs_v1_mount_ro & ((kword_t)1UL << (id - 1U))) != 0;
}

int
vfs_v1_mount(vnode_v1_t target, unsigned int provider,
    unsigned int kind, unsigned int index, unsigned int flags,
    vnode_v1_t *rootp)
{
        struct vfs_v1_stat st;
        vnode_v1_t root;
        unsigned int i;
        unsigned int id;

        if (rootp == 0 || provider == 0U ||
            provider > VFS_V1_PROVIDER_MASK || kind > VFS_V1_LOCAL_KIND_MASK ||
            index > VFS_V1_INDEX_MASK || flags > VFS_V1_MOUNT_RDONLY)
                return -1;
        if (target == VFS_V1_NODE_NONE) {
                if (vfs_v1_namespace_root != VFS_V1_NODE_NONE)
                        return -1;
        } else if (vfs_v1_stat(target, &st) != 0 ||
            st.type != VFS_V1_TYPE_DIR)
                return -1;
        for (i = 0U; i < VFS_V1_NMOUNT; ++i) {
                if (vfs_v1_mount_root[i] == VFS_V1_NODE_NONE)
                        break;
                if (vfs_v1_mount_target[i] == target)
                        return -1;
        }
        if (i == VFS_V1_NMOUNT)
                return -1;
        id = i + 1U;
        root = VFS_V1_NODE(provider, VFS_V1_MOUNT_KIND(id, kind), index);
        vfs_v1_mount_target[i] = target;
        vfs_v1_mount_root[i] = root;
        if (flags == VFS_V1_MOUNT_RDONLY)
                vfs_v1_mount_ro |= (kword_t)1UL << i;
        else
                vfs_v1_mount_ro &= ~((kword_t)1UL << i);
        *rootp = root;
        if (target == VFS_V1_NODE_NONE)
                vfs_v1_namespace_root = root;
        return 0;
}

int
vfs_v1_unmount(vnode_v1_t root)
{
        struct fs_mres_request req;
        unsigned int id;
        unsigned int i;

        id = VFS_V1_MOUNT_ID(root);
        if (id == 0U || id > VFS_V1_NMOUNT)
                return -1;
        i = id - 1U;
        if (vfs_v1_mount_root[i] != root || vfs_v1_sync(root) != 0)
                return -1;
        if (VFS_V1_PROVIDER(root) == D6FS_V2_PROVIDER) {
                req.op = FS_MRES_OP_PREPARE_UNMOUNT;
                req.a = root;
                if (vfs_v1_mres_call(D6FS_V2_PROVIDER, &req) != 0)
                        return -1;
        }
        if (vfs_v1_namespace_root == root)
                vfs_v1_namespace_root = vfs_v1_mount_target[i];
        vfs_v1_unlock_mount(id);
        vfs_v1_mount_target[i] = VFS_V1_NODE_NONE;
        vfs_v1_mount_root[i] = VFS_V1_NODE_NONE;
        vfs_v1_mount_ro &= ~((kword_t)1UL << i);
        return 0;
}

int
vfs_v1_lookup(vnode_v1_t dir, const struct vfs_v1_name *name,
    vnode_v1_t *nodep)
{
        struct fs_mres_request req;
        vnode_v1_t node;
        unsigned int provider;
        int rc;

        if (name == 0 || nodep == 0)
                return -1;
        if (dir == vfs_v1_namespace_root) {
                if (vfs_v1_name_is6(name,
                    VFS_V1_SIX6('D','E','V','I','C','E'), 6U)) {
                        *nodep = VFS_V1_DEVICE_ROOT;
                        return 0;
                }
                if (vfs_v1_name_is6(name,
                    VFS_V1_SIX6('P','R','O','C',' ',' '), 4U)) {
                        *nodep = VFS_V1_PROC_ROOT;
                        return 0;
                }
                if (file_v1_alias_node != VFS_V1_NODE_NONE &&
                    vfs_v1_name_is6(name,
                    VFS_V1_SIX6('T','E','M','P',' ',' '), 4U)) {
                        *nodep = file_v1_alias_node;
                        return 0;
                }
        }
        provider = VFS_V1_PROVIDER(dir);
        switch (provider) {
        case MEMFS_V1_PROVIDER:
        case DTFS_V1_PROVIDER:
        case D6FS_V2_PROVIDER:
                req.op = FS_MRES_OP_LOOKUP;
                req.a = dir;
                req.b = (kword_t)(unsigned long)name;
                req.c = (kword_t)(unsigned long)&node;
                rc = vfs_v1_mres_call(provider, &req);
                break;
        case DEVICEFS_V1_PROVIDER:
                rc = devicefs_v1_lookup(dir, name, &node);
                break;
        case PROCFS_V1_PROVIDER:
                rc = procfs_v1_lookup(dir, name, &node);
                break;
        default:
                return -1;
        }
        if (rc != 0)
                return rc;
        *nodep = vfs_v1_follow_mount(node);
        return 0;
}

static int
vfs_v1_readdir_raw(vnode_v1_t dir, unsigned int off,
    struct vfs_v1_dirent *ent)
{
        struct fs_mres_request req;
        unsigned int provider;

        provider = VFS_V1_PROVIDER(dir);
        switch (provider) {
        case MEMFS_V1_PROVIDER:
        case DTFS_V1_PROVIDER:
        case D6FS_V2_PROVIDER:
                req.op = FS_MRES_OP_READDIR;
                req.a = dir;
                req.b = (kword_t)off;
                req.c = (kword_t)(unsigned long)ent;
                return vfs_v1_mres_call(provider, &req);
        case DEVICEFS_V1_PROVIDER:
                return devicefs_v1_readdir(dir, off, ent);
        case PROCFS_V1_PROVIDER:
                return procfs_v1_readdir(dir, off, ent);
        default:
                return -1;
        }
}

int
vfs_v1_readdir(vnode_v1_t dir, unsigned int off, struct vfs_v1_dirent *ent)
{
        unsigned int base;
        int rc;

        if (ent == 0)
                return -1;
        rc = vfs_v1_readdir_raw(dir, off, ent);
        if (rc != 0 || dir != vfs_v1_namespace_root)
                return rc;
        base = 0U;
        while (vfs_v1_readdir_raw(dir, base, ent) > 0)
                ++base;
        if (off == base) {
                vfs_v1_dirent_set6(ent,
                    VFS_V1_SIX6('D','E','V','I','C','E'), 6U,
                    VFS_V1_TYPE_DIR);
                return 1;
        }
        if (off == base + 1U) {
                vfs_v1_dirent_set6(ent,
                    VFS_V1_SIX6('P','R','O','C',' ',' '), 4U,
                    VFS_V1_TYPE_DIR);
                return 1;
        }
        if (off == base + 2U && file_v1_alias_node != VFS_V1_NODE_NONE) {
                vfs_v1_dirent_set6(ent,
                    VFS_V1_SIX6('T','E','M','P',' ',' '), 4U,
                    VFS_V1_TYPE_DIR);
                return 1;
        }
        return 0;
}

int
vfs_v1_stat(vnode_v1_t node, struct vfs_v1_stat *st)
{
        struct fs_mres_request req;
        unsigned int provider;

        provider = VFS_V1_PROVIDER(node);
        switch (provider) {
        case MEMFS_V1_PROVIDER:
        case DTFS_V1_PROVIDER:
        case D6FS_V2_PROVIDER:
                req.op = FS_MRES_OP_STAT;
                req.a = node;
                req.b = (kword_t)(unsigned long)st;
                return vfs_v1_mres_call(provider, &req);
        case DEVICEFS_V1_PROVIDER:
                return devicefs_v1_stat(node, st);
        case PROCFS_V1_PROVIDER:
                return procfs_v1_stat(node, st);
        default:
                return -1;
        }
}

static int
vfs_v1_parent_raw(vnode_v1_t node, vnode_v1_t *parentp)
{
        struct fs_mres_request req;
        unsigned int provider;
        unsigned int kind;

        if (parentp == 0)
                return -1;
        provider = VFS_V1_PROVIDER(node);
        switch (provider) {
        case MEMFS_V1_PROVIDER:
                if (node == VFS_V1_MEMFS_ROOT) {
                        *parentp = node;
                        return 0;
                }
                /* FALLTHROUGH */
        case DTFS_V1_PROVIDER:
        case D6FS_V2_PROVIDER:
                req.op = FS_MRES_OP_PARENT;
                req.a = node;
                req.b = (kword_t)(unsigned long)parentp;
                return vfs_v1_mres_call(provider, &req);
        case DEVICEFS_V1_PROVIDER:
                *parentp = vfs_v1_namespace_root;
                return 0;
        case PROCFS_V1_PROVIDER:
                kind = VFS_V1_LOCAL_KIND(node);
                if (kind == PROCFS_V1_KIND_ROOT) {
                        *parentp = vfs_v1_namespace_root;
                        return 0;
                }
                if (kind == PROCFS_V1_KIND_PROC) {
                        *parentp = VFS_V1_PROC_ROOT;
                        return 0;
                }
                return -1;
        default:
                return -1;
        }
}

int
vfs_v1_parent(vnode_v1_t node, vnode_v1_t *parentp)
{
        unsigned int id;

        if (parentp == 0)
                return -1;
        if (node == vfs_v1_namespace_root) {
                *parentp = node;
                return 0;
        }

        id = VFS_V1_MOUNT_ID(node);
        if (id != 0U && id <= VFS_V1_NMOUNT &&
            vfs_v1_mount_root[id - 1U] == node)
                return vfs_v1_parent_raw(vfs_v1_mount_target[id - 1U],
                    parentp);
        return vfs_v1_parent_raw(node, parentp);
}

/*
 * Return the parent and namespace name of a directory while crossing a mount
 * root back through its mountpoint.  The current writable mountpoints live in
 * MEMFS, so their stored name is authoritative for getcwd().
 */
int
vfs_v1_parent_name(vnode_v1_t node, vnode_v1_t *parentp,
    struct vfs_v1_name *namep)
{
        struct fs_mres_request req;
        unsigned int id;
        unsigned int provider;

        if (parentp == 0 || namep == 0 || node == vfs_v1_namespace_root)
                return -1;
        id = VFS_V1_MOUNT_ID(node);
        if (id != 0U && id <= VFS_V1_NMOUNT &&
            vfs_v1_mount_root[id - 1U] == node)
                node = vfs_v1_mount_target[id - 1U];
        provider = VFS_V1_PROVIDER(node);
        if (provider != MEMFS_V1_PROVIDER && provider != D6FS_V2_PROVIDER)
                return -1;
        req.op = FS_MRES_OP_PARENT_NAME;
        req.a = node;
        req.b = (kword_t)(unsigned long)parentp;
        req.c = (kword_t)(unsigned long)namep;
        return vfs_v1_mres_call(provider, &req);
}

int
vfs_v1_create(vnode_v1_t dir, const struct vfs_v1_name *name,
    unsigned int mode, vnode_v1_t *nodep)
{
        struct fs_mres_request req;
        unsigned int provider;

        if (vfs_v1_readonly(dir))
                return -1;
        provider = VFS_V1_PROVIDER(dir);
        if (provider != MEMFS_V1_PROVIDER && provider != DTFS_V1_PROVIDER &&
            provider != D6FS_V2_PROVIDER)
                return -1;
        req.op = FS_MRES_OP_CREATE;
        req.a = dir;
        req.b = (kword_t)(unsigned long)name;
        req.c = (kword_t)mode;
        req.d = (kword_t)(unsigned long)nodep;
        return vfs_v1_mres_call(provider, &req);
}

int
vfs_v1_mkdir(vnode_v1_t dir, const struct vfs_v1_name *name,
    unsigned int mode, vnode_v1_t *nodep)
{
        struct fs_mres_request req;
        unsigned int provider;

        if (vfs_v1_readonly(dir))
                return -1;
        provider = VFS_V1_PROVIDER(dir);
        if (provider != MEMFS_V1_PROVIDER && provider != D6FS_V2_PROVIDER)
                return -1;
        req.op = FS_MRES_OP_MKDIR;
        req.a = dir;
        req.b = (kword_t)(unsigned long)name;
        req.c = (kword_t)mode;
        req.d = (kword_t)(unsigned long)nodep;
        return vfs_v1_mres_call(provider, &req);
}

int
vfs_v1_symlink(vnode_v1_t dir, const struct vfs_v1_name *name,
    const kword_t *target, unsigned int target_chars, vnode_v1_t *nodep)
{
        struct fs_mres_request req;

        if (vfs_v1_readonly(dir) || VFS_V1_PROVIDER(dir) != D6FS_V2_PROVIDER)
                return -1;
        req.op = FS_MRES_OP_SYMLINK;
        req.a = dir;
        req.b = (kword_t)(unsigned long)name;
        req.c = (kword_t)(unsigned long)target;
        req.d = (kword_t)target_chars;
        req.e = (kword_t)(unsigned long)nodep;
        return vfs_v1_mres_call(D6FS_V2_PROVIDER, &req);
}

int
vfs_v1_unlink(vnode_v1_t dir, const struct vfs_v1_name *name)
{
        struct fs_mres_request req;
        unsigned int provider;

        if (vfs_v1_readonly(dir))
                return -1;
        provider = VFS_V1_PROVIDER(dir);
        if (provider != MEMFS_V1_PROVIDER && provider != DTFS_V1_PROVIDER &&
            provider != D6FS_V2_PROVIDER)
                return -1;
        req.op = FS_MRES_OP_UNLINK;
        req.a = dir;
        req.b = (kword_t)(unsigned long)name;
        return vfs_v1_mres_call(provider, &req);
}

int
vfs_v1_rename(vnode_v1_t olddir, const struct vfs_v1_name *oldname,
    vnode_v1_t newdir, const struct vfs_v1_name *newname)
{
        struct fs_mres_request req;
        unsigned int provider;

        if (vfs_v1_readonly(olddir) || vfs_v1_readonly(newdir) ||
            VFS_V1_PROVIDER(olddir) != VFS_V1_PROVIDER(newdir) ||
            VFS_V1_MOUNT_ID(olddir) != VFS_V1_MOUNT_ID(newdir))
                return -1;
        provider = VFS_V1_PROVIDER(olddir);
        if (provider != MEMFS_V1_PROVIDER && provider != DTFS_V1_PROVIDER &&
            provider != D6FS_V2_PROVIDER)
                return -1;
        req.op = FS_MRES_OP_RENAME;
        req.a = olddir;
        req.b = (kword_t)(unsigned long)oldname;
        req.c = newdir;
        req.d = (kword_t)(unsigned long)newname;
        return vfs_v1_mres_call(provider, &req);
}

int
vfs_v1_truncate(vnode_v1_t node, unsigned int words, kword_t size_chars)
{
        struct fs_mres_request req;
        unsigned int provider;

        if (vfs_v1_readonly(node))
                return -1;
        provider = VFS_V1_PROVIDER(node);
        if (provider != MEMFS_V1_PROVIDER && provider != DTFS_V1_PROVIDER &&
            provider != D6FS_V2_PROVIDER)
                return -1;
        req.op = FS_MRES_OP_TRUNCATE;
        req.a = node;
        req.b = (kword_t)words;
        req.c = size_chars;
        return vfs_v1_mres_call(provider, &req);
}

int
vfs_v1_chmod(vnode_v1_t node, unsigned int mode)
{
        struct fs_mres_request req;
        unsigned int provider;

        if (vfs_v1_readonly(node))
                return -1;
        provider = VFS_V1_PROVIDER(node);
        if (provider != MEMFS_V1_PROVIDER && provider != DTFS_V1_PROVIDER &&
            provider != D6FS_V2_PROVIDER)
                return -1;
        req.op = FS_MRES_OP_CHMOD;
        req.a = node;
        req.b = (kword_t)mode;
        return vfs_v1_mres_call(provider, &req);
}

int
vfs_v1_read_words(vnode_v1_t node, unsigned int off, kword_t *buf,
    unsigned int nwords)
{
        struct fs_mres_request req;
        unsigned int provider;

        provider = VFS_V1_PROVIDER(node);
        if (provider != MEMFS_V1_PROVIDER && provider != DTFS_V1_PROVIDER &&
            provider != D6FS_V2_PROVIDER)
                return -1;
        req.op = FS_MRES_OP_READ_WORDS;
        req.a = node;
        req.b = (kword_t)off;
        req.c = (kword_t)(unsigned long)buf;
        req.d = (kword_t)nwords;
        return vfs_v1_mres_call(provider, &req);
}

int
vfs_v1_write_words(vnode_v1_t node, unsigned int off,
    const kword_t *buf, unsigned int nwords, kword_t size_chars)
{
        struct fs_mres_request req;
        unsigned int provider;

        if (vfs_v1_readonly(node))
                return -1;
        provider = VFS_V1_PROVIDER(node);
        if (provider != MEMFS_V1_PROVIDER && provider != DTFS_V1_PROVIDER &&
            provider != D6FS_V2_PROVIDER)
                return -1;
        req.op = FS_MRES_OP_WRITE_WORDS;
        req.a = node;
        req.b = (kword_t)off;
        req.c = (kword_t)(unsigned long)buf;
        req.d = (kword_t)nwords;
        req.e = size_chars;
        return vfs_v1_mres_call(provider, &req);
}

int
vfs_v1_readchar(vnode_v1_t node, kword_t off, unsigned int *chp)
{
        struct vfs_v1_stat st;
        unsigned int wi;
        unsigned int bi;
        kword_t word;
        int rc;

        if (chp == 0)
                return -1;
        switch (VFS_V1_PROVIDER(node)) {
        case PROCFS_V1_PROVIDER:
                return procfs_v1_readchar(node, off, chp);
        case DEVICEFS_V1_PROVIDER:
                return devicefs_v1_readchar(node, off, chp);
        default:
                break;
        }
        if (vfs_v1_stat(node, &st) != 0 || st.type != VFS_V1_TYPE_REG)
                return -1;
        if (off >= st.size_chars)
                return 0;
        wi = (unsigned int)(off / 4U);
        bi = (unsigned int)(off % 4U);
        rc = vfs_v1_read_words(node, wi, &word, 1U);
        if (rc != 1)
                return -1;
        *chp = (unsigned int)((word >> (27U - 9U * bi)) & 0777UL);
        return 1;
}

int
vfs_v1_writechar(vnode_v1_t node, kword_t off, unsigned int ch)
{
        struct vfs_v1_stat st;
        unsigned int wi;
        unsigned int bi;
        unsigned int need_words;
        unsigned int shift;
        kword_t word;
        kword_t mask;
        kword_t end_chars;

        if (VFS_V1_PROVIDER(node) == DEVICEFS_V1_PROVIDER &&
            VFS_V1_LOCAL_KIND(node) == DEVICEFS_V1_KIND_DEVICE &&
            VFS_V1_INDEX(node) == DEVICEFS_V1_DEV_CTY0)
                return VFS_V1_DEVICE_IO;
        if (vfs_v1_stat(node, &st) != 0 || st.type != VFS_V1_TYPE_REG)
                return -1;
        end_chars = off + 1U;
        need_words = (unsigned int)((end_chars + 3U) / 4U);
        if (need_words > (unsigned int)st.size_words &&
            vfs_v1_truncate(node, need_words, st.size_chars) != 0)
                return -1;
        wi = (unsigned int)(off / 4U);
        bi = (unsigned int)(off % 4U);
        word = 0;
        (void)vfs_v1_read_words(node, wi, &word, 1U);
        shift = 27U - 9U * bi;
        mask = (kword_t)0777UL << shift;
        word = (word & ~mask) | (((kword_t)ch & 0777UL) << shift);
        if (vfs_v1_write_words(node, wi, &word, 1U, end_chars) != 1)
                return -1;
        return 0;
}

int
vfs_v1_sync(vnode_v1_t node)
{
        struct fs_mres_request req;
        unsigned int provider;

        provider = VFS_V1_PROVIDER(node);
        if (provider != DTFS_V1_PROVIDER && provider != D6FS_V2_PROVIDER)
                return 0;
        req.op = FS_MRES_OP_SYNC;
        req.a = node;
        return vfs_v1_mres_call(provider, &req);
}

extern int vfs_v1_name_set6(struct vfs_v1_name *name, kword_t word,
    unsigned int chars);
extern int vfs_v1_name_is6(const struct vfs_v1_name *name, kword_t word,
    unsigned int chars);
extern int vfs_v1_sixbit_readchar(kword_t word, unsigned int nchars, kword_t off,
    unsigned int *chp);
