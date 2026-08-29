#include "vfs_v1.h"
#include "memfs_v1.h"
#include "devicefs_v1.h"
#include "procfs_v1.h"
#include "dtfs_v1.h"

extern struct memfs_v1 *file_v1_root;
extern vnode_v1_t file_v1_alias_node;

static vnode_v1_t vfs_v1_mount_target[VFS_V1_NMOUNT];
static vnode_v1_t vfs_v1_mount_root[VFS_V1_NMOUNT];
static kword_t vfs_v1_mount_ro;

#define VFS_V1_MEMFS_ROOT \
    VFS_V1_NODE(MEMFS_V1_PROVIDER, MEMFS_V1_KIND_NODE, 0U)
#define VFS_V1_DEVICE_ROOT \
    VFS_V1_NODE(DEVICEFS_V1_PROVIDER, DEVICEFS_V1_KIND_ROOT, 0U)
#define VFS_V1_PROC_ROOT \
    VFS_V1_NODE(PROCFS_V1_PROVIDER, PROCFS_V1_KIND_ROOT, 0U)

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

        if (rootp == 0 || target == VFS_V1_NODE_NONE || provider == 0U ||
            provider > VFS_V1_PROVIDER_MASK || kind > VFS_V1_LOCAL_KIND_MASK ||
            index > VFS_V1_INDEX_MASK || flags > VFS_V1_MOUNT_RDONLY ||
            vfs_v1_stat(target, &st) != 0 || st.type != VFS_V1_TYPE_DIR)
                return -1;
        for (i = 0U; i < VFS_V1_NMOUNT; ++i) {
                if (vfs_v1_mount_target[i] == target)
                        return -1;
                if (vfs_v1_mount_root[i] == VFS_V1_NODE_NONE)
                        break;
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
        return 0;
}

int
vfs_v1_unmount(vnode_v1_t root)
{
        unsigned int id;
        unsigned int i;

        id = VFS_V1_MOUNT_ID(root);
        if (id == 0U || id > VFS_V1_NMOUNT)
                return -1;
        i = id - 1U;
        if (vfs_v1_mount_root[i] != root || vfs_v1_sync(root) != 0)
                return -1;
        vfs_v1_mount_target[i] = VFS_V1_NODE_NONE;
        vfs_v1_mount_root[i] = VFS_V1_NODE_NONE;
        vfs_v1_mount_ro &= ~((kword_t)1UL << i);
        return 0;
}

int
vfs_v1_lookup(vnode_v1_t dir, const struct vfs_v1_name *name,
    vnode_v1_t *nodep)
{
        vnode_v1_t node;
        int rc;

        if (name == 0 || nodep == 0)
                return -1;
        if (dir == VFS_V1_MEMFS_ROOT) {
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
        switch (VFS_V1_PROVIDER(dir)) {
        case MEMFS_V1_PROVIDER:
                rc = memfs_v1_lookup(file_v1_root, dir, name, &node);
                break;
        case DEVICEFS_V1_PROVIDER:
                rc = devicefs_v1_lookup(dir, name, &node);
                break;
        case PROCFS_V1_PROVIDER:
                rc = procfs_v1_lookup(dir, name, &node);
                break;
        case DTFS_V1_PROVIDER:
                rc = dtfs_v1_lookup(dir, name, &node);
                break;
        default:
                return -1;
        }
        if (rc != 0)
                return rc;
        *nodep = vfs_v1_follow_mount(node);
        return 0;
}

int
vfs_v1_readdir(vnode_v1_t dir, unsigned int off, struct vfs_v1_dirent *ent)
{
        unsigned int base;
        int rc;

        if (ent == 0)
                return -1;
        switch (VFS_V1_PROVIDER(dir)) {
        case MEMFS_V1_PROVIDER:
                rc = memfs_v1_readdir(file_v1_root, dir, off, ent);
                if (rc != 0 || dir != VFS_V1_MEMFS_ROOT)
                        return rc;
                base = 0U;
                while (memfs_v1_readdir(file_v1_root, dir, base, ent) > 0)
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
                if (off == base + 2U &&
                    file_v1_alias_node != VFS_V1_NODE_NONE) {
                        vfs_v1_dirent_set6(ent,
                            VFS_V1_SIX6('T','E','M','P',' ',' '), 4U,
                            VFS_V1_TYPE_DIR);
                        return 1;
                }
                return 0;
        case DEVICEFS_V1_PROVIDER:
                return devicefs_v1_readdir(dir, off, ent);
        case PROCFS_V1_PROVIDER:
                return procfs_v1_readdir(dir, off, ent);
        case DTFS_V1_PROVIDER:
                return dtfs_v1_readdir(dir, off, ent);
        default:
                return -1;
        }
}

int
vfs_v1_stat(vnode_v1_t node, struct vfs_v1_stat *st)
{
        switch (VFS_V1_PROVIDER(node)) {
        case MEMFS_V1_PROVIDER:
                return memfs_v1_stat(file_v1_root, node, st);
        case DEVICEFS_V1_PROVIDER:
                return devicefs_v1_stat(node, st);
        case PROCFS_V1_PROVIDER:
                return procfs_v1_stat(node, st);
        case DTFS_V1_PROVIDER:
                return dtfs_v1_stat(node, st);
        default:
                return -1;
        }
}

static int
vfs_v1_parent_raw(vnode_v1_t node, vnode_v1_t *parentp)
{
        unsigned int kind;

        if (parentp == 0)
                return -1;
        switch (VFS_V1_PROVIDER(node)) {
        case MEMFS_V1_PROVIDER:
                if (node == VFS_V1_MEMFS_ROOT) {
                        *parentp = node;
                        return 0;
                }
                return memfs_v1_parent(file_v1_root, node, parentp, 0);
        case DEVICEFS_V1_PROVIDER:
                *parentp = VFS_V1_MEMFS_ROOT;
                return 0;
        case PROCFS_V1_PROVIDER:
                kind = VFS_V1_LOCAL_KIND(node);
                if (kind == PROCFS_V1_KIND_ROOT) {
                        *parentp = VFS_V1_MEMFS_ROOT;
                        return 0;
                }
                if (kind == PROCFS_V1_KIND_PROC) {
                        *parentp = VFS_V1_PROC_ROOT;
                        return 0;
                }
                return -1;
        case DTFS_V1_PROVIDER:
                return dtfs_v1_parent(node, parentp);
        default:
                return -1;
        }
}

int
vfs_v1_parent(vnode_v1_t node, vnode_v1_t *parentp)
{
        unsigned int id;

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
        unsigned int id;

        if (parentp == 0 || namep == 0)
                return -1;
        id = VFS_V1_MOUNT_ID(node);
        if (id != 0U && id <= VFS_V1_NMOUNT &&
            vfs_v1_mount_root[id - 1U] == node)
                node = vfs_v1_mount_target[id - 1U];
        if (VFS_V1_PROVIDER(node) != MEMFS_V1_PROVIDER)
                return -1;
        return memfs_v1_parent(file_v1_root, node, parentp, namep);
}

int
vfs_v1_create(vnode_v1_t dir, const struct vfs_v1_name *name,
    unsigned int mode, vnode_v1_t *nodep)
{
        if (vfs_v1_readonly(dir))
                return -1;
        switch (VFS_V1_PROVIDER(dir)) {
        case MEMFS_V1_PROVIDER:
                return memfs_v1_create(file_v1_root, dir, name, mode, nodep);
        case DTFS_V1_PROVIDER:
                return dtfs_v1_create(dir, name, mode, nodep);
        default:
                return -1;
        }
}

int
vfs_v1_mkdir(vnode_v1_t dir, const struct vfs_v1_name *name,
    unsigned int mode, vnode_v1_t *nodep)
{
        if (vfs_v1_readonly(dir))
                return -1;
        if (VFS_V1_PROVIDER(dir) == MEMFS_V1_PROVIDER)
                return memfs_v1_mkdir(file_v1_root, dir, name, mode, nodep);
        return -1;
}

int
vfs_v1_unlink(vnode_v1_t dir, const struct vfs_v1_name *name)
{
        if (vfs_v1_readonly(dir))
                return -1;
        switch (VFS_V1_PROVIDER(dir)) {
        case MEMFS_V1_PROVIDER:
                return memfs_v1_unlink(file_v1_root, dir, name);
        case DTFS_V1_PROVIDER:
                return dtfs_v1_unlink(dir, name);
        default:
                return -1;
        }
}

int
vfs_v1_rename(vnode_v1_t olddir, const struct vfs_v1_name *oldname,
    vnode_v1_t newdir, const struct vfs_v1_name *newname)
{
        if (vfs_v1_readonly(olddir) || vfs_v1_readonly(newdir) ||
            VFS_V1_PROVIDER(olddir) != VFS_V1_PROVIDER(newdir) ||
            VFS_V1_MOUNT_ID(olddir) != VFS_V1_MOUNT_ID(newdir))
                return -1;
        switch (VFS_V1_PROVIDER(olddir)) {
        case MEMFS_V1_PROVIDER:
                return memfs_v1_rename(file_v1_root, olddir, oldname, newdir,
                    newname);
        case DTFS_V1_PROVIDER:
                return dtfs_v1_rename(olddir, oldname, newdir, newname);
        default:
                return -1;
        }
}

int
vfs_v1_truncate(vnode_v1_t node, unsigned int words, kword_t size_chars)
{
        if (vfs_v1_readonly(node))
                return -1;
        switch (VFS_V1_PROVIDER(node)) {
        case MEMFS_V1_PROVIDER:
                return memfs_v1_truncate_words(file_v1_root, node, words,
                    size_chars);
        case DTFS_V1_PROVIDER:
                return dtfs_v1_truncate(node, words, size_chars);
        default:
                return -1;
        }
}

int
vfs_v1_chmod(vnode_v1_t node, unsigned int mode)
{
        if (vfs_v1_readonly(node))
                return -1;
        switch (VFS_V1_PROVIDER(node)) {
        case MEMFS_V1_PROVIDER:
                return memfs_v1_chmod(file_v1_root, node, mode);
        case DTFS_V1_PROVIDER:
                return dtfs_v1_chmod(node, mode);
        default:
                return -1;
        }
}

int
vfs_v1_read_words(vnode_v1_t node, unsigned int off, kword_t *buf,
    unsigned int nwords)
{
        switch (VFS_V1_PROVIDER(node)) {
        case MEMFS_V1_PROVIDER:
                return memfs_v1_read_words(file_v1_root, node, off, buf,
                    nwords);
        case DTFS_V1_PROVIDER:
                return dtfs_v1_read_words(node, off, buf, nwords);
        default:
                return -1;
        }
}

int
vfs_v1_write_words(vnode_v1_t node, unsigned int off,
    const kword_t *buf, unsigned int nwords, kword_t size_chars)
{
        if (vfs_v1_readonly(node))
                return -1;
        switch (VFS_V1_PROVIDER(node)) {
        case MEMFS_V1_PROVIDER:
                return memfs_v1_write_words(file_v1_root, node, off, buf,
                    nwords, size_chars);
        case DTFS_V1_PROVIDER:
                return dtfs_v1_write_words(node, off, buf, nwords, size_chars);
        default:
                return -1;
        }
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
        if (VFS_V1_PROVIDER(node) == DTFS_V1_PROVIDER)
                return dtfs_v1_sync(node);
        return 0;
}

extern int vfs_v1_name_set6(struct vfs_v1_name *name, kword_t word,
    unsigned int chars);
extern int vfs_v1_name_is6(const struct vfs_v1_name *name, kword_t word,
    unsigned int chars);
extern int vfs_v1_sixbit_readchar(kword_t word, unsigned int nchars, kword_t off,
    unsigned int *chp);
