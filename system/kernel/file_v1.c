#include "file_v1.h"
#include "devicefs_v1.h"
#include "procfs_v1.h"

struct memfs_v1 *file_v1_root;

#define FILE_V1_MEMFS_ROOT VFS_V1_NODE(MEMFS_V1_PROVIDER, MEMFS_V1_KIND_NODE, 0U)
#define FILE_V1_DEVICE_ROOT VFS_V1_NODE(DEVICEFS_V1_PROVIDER, DEVICEFS_V1_KIND_ROOT, 0U)
#define FILE_V1_PROC_ROOT VFS_V1_NODE(PROCFS_V1_PROVIDER, PROCFS_V1_KIND_ROOT, 0U)

vnode_v1_t file_v1_alias_node;
extern struct file_v1 file_v1_table[FILE_V1_NFILE];
extern vnode_v1_t file_v1_cwd[FILE_V1_OWNER_MAX];
extern kword_t file_v1_alias_cwd[(FILE_V1_OWNER_MAX + 35U) / 36U];

static int file_v1_alias_cwd_get(unsigned int owner);
static void file_v1_alias_cwd_set(unsigned int owner, int active);

extern int file_v1_component(const kword_t *path, unsigned int *posp,
    struct vfs_v1_name *name);

static int
file_v1_is_name(const struct vfs_v1_name *name, kword_t word,
    unsigned int chars)
{
        return vfs_v1_name_is6(name, word, chars);
}

extern int file_v1_lookup_child(vnode_v1_t dir,
    const struct vfs_v1_name *name, vnode_v1_t *nodep);

static int
file_v1_node_stat(vnode_v1_t node, struct vfs_v1_stat *st)
{
        switch (VFS_V1_PROVIDER(node)) {
        case MEMFS_V1_PROVIDER:
                return memfs_v1_stat(file_v1_root, node, st);
        case DEVICEFS_V1_PROVIDER:
                return devicefs_v1_stat(node, st);
        case PROCFS_V1_PROVIDER:
                return procfs_v1_stat(node, st);
        default:
                return -1;
        }
}


struct memfs_v1 *
file_v1_rootfs(void)
{
        return file_v1_root;
}


static int
file_v1_parent_node(vnode_v1_t node, vnode_v1_t *parentp)
{
        unsigned int kind;

        if (parentp == 0)
                return -1;
        if (VFS_V1_PROVIDER(node) == MEMFS_V1_PROVIDER) {
                if (node == FILE_V1_MEMFS_ROOT) {
                        *parentp = node;
                        return 0;
                }
                return memfs_v1_parent(file_v1_root, node, parentp, 0);
        }
        if (VFS_V1_PROVIDER(node) == DEVICEFS_V1_PROVIDER) {
                *parentp = FILE_V1_MEMFS_ROOT;
                return 0;
        }
        if (VFS_V1_PROVIDER(node) == PROCFS_V1_PROVIDER) {
                kind = VFS_V1_KIND(node);
                if (kind == PROCFS_V1_KIND_ROOT) {
                        *parentp = FILE_V1_MEMFS_ROOT;
                        return 0;
                }
                if (kind == PROCFS_V1_KIND_PROC) {
                        *parentp = FILE_V1_PROC_ROOT;
                        return 0;
                }
        }
        return -1;
}

static int
file_v1_name_dot(const struct vfs_v1_name *name)
{
        return vfs_v1_name_is6(name, VFS_V1_SIX6('.',' ',' ',' ',' ',' '), 1U);
}

static int
file_v1_name_dotdot(const struct vfs_v1_name *name)
{
        return vfs_v1_name_is6(name, VFS_V1_SIX6('.','.',' ',' ',' ',' '), 2U);
}

static int
file_v1_walk_path_owner(unsigned int owner, const kword_t *path,
    int parent_only, vnode_v1_t *nodep, struct vfs_v1_name *leaf,
    int *aliasp)
{
        unsigned int pos;
        unsigned int n;
        int rc;
        int alias;
        vnode_v1_t node;
        vnode_v1_t next;
        struct vfs_v1_name name;

        if (file_v1_root == 0 || path == 0 || nodep == 0 ||
            owner >= FILE_V1_OWNER_MAX || (parent_only && leaf == 0))
                return -1;
        n = (unsigned int)path[0];
        if (n == 0U)
                return -1;
        if (((path[1] >> 30U) & 077UL) == (kword_t)('/' - 040)) {
                node = FILE_V1_MEMFS_ROOT;
                alias = 0;
        } else {
                node = file_v1_cwd[owner];
                if (node == VFS_V1_NODE_NONE)
                        node = FILE_V1_MEMFS_ROOT;
                alias = aliasp != 0 ? file_v1_alias_cwd_get(owner) : 0;
        }
        pos = 0U;
        for (;;) {
                rc = file_v1_component(path, &pos, &name);
                if (rc < 0)
                        return -1;
                if (rc == 0) {
                        if (parent_only)
                                return -1;
                        *nodep = node;
                        if (aliasp != 0)
                                *aliasp = alias;
                        return 0;
                }
                if (parent_only) {
                        if (pos >= n) {
                                if (file_v1_name_dot(&name) ||
                                    file_v1_name_dotdot(&name))
                                        return -1;
                                *nodep = node;
                                *leaf = name;
                                return 0;
                        }
                }
                if (file_v1_name_dot(&name))
                        continue;
                if (file_v1_name_dotdot(&name)) {
                        if (aliasp != 0 && alias && node == file_v1_alias_node) {
                                node = FILE_V1_MEMFS_ROOT;
                                alias = 0;
                        } else if (file_v1_parent_node(node, &next) != 0) {
                                return -1;
                        } else {
                                node = next;
                        }
                        continue;
                }
                if (file_v1_lookup_child(node, &name, &next) != 0)
                        return -1;
                if (aliasp != 0 &&
                    file_v1_alias_node != VFS_V1_NODE_NONE &&
                    node == FILE_V1_MEMFS_ROOT &&
                    next == file_v1_alias_node)
                        alias = 1;
                node = next;
        }
}

int
file_v1_lookup_path_owner(unsigned int owner, const kword_t *path,
    vnode_v1_t *nodep)
{
        return file_v1_walk_path_owner(owner, path, 0, nodep, 0, 0);
}

static int
file_v1_parent_path_owner(unsigned int owner, const kword_t *path,
    vnode_v1_t *dirp, struct vfs_v1_name *leaf)
{
        return file_v1_walk_path_owner(owner, path, 1, dirp, leaf, 0);
}

extern struct file_v1 *file_v1_find(unsigned int owner, int fd);
extern int file_v1_new_fd(unsigned int owner, vnode_v1_t node,
    unsigned int flags, int isdir);

int
file_v1_open(unsigned int owner, const kword_t *path, unsigned int flags)
{
        vnode_v1_t node;
        vnode_v1_t dir;
        struct vfs_v1_name leaf;
        struct vfs_v1_stat st;

        if (file_v1_lookup_path_owner(owner, path, &node) != 0) {
                if ((flags & FILE_V1_O_CREAT) == 0U ||
                    file_v1_parent_path_owner(owner, path, &dir, &leaf) != 0 ||
                    VFS_V1_PROVIDER(dir) != MEMFS_V1_PROVIDER ||
                    memfs_v1_create(file_v1_root, dir, &leaf, 0666U, &node) != 0)
                        return -1;
        }
        if (file_v1_node_stat(node, &st) != 0)
                return -1;
        if ((flags & FILE_V1_O_TRUNC) != 0U && st.type == VFS_V1_TYPE_REG) {
                if (VFS_V1_PROVIDER(node) != MEMFS_V1_PROVIDER ||
                    memfs_v1_truncate_words(file_v1_root, node, 0U, 0) != 0)
                        return -1;
        }
        return file_v1_new_fd(owner, node, flags, st.type == VFS_V1_TYPE_DIR);
}


int
file_v1_close(unsigned int owner, int fd)
{
        struct file_v1 *fp;

        fp = file_v1_find(owner, fd);
        if (fp == 0)
                return -1;
        fp->meta = 0U;
        fp->node = VFS_V1_NODE_NONE;
        fp->off_chars = 0;
        return 0;
}

static unsigned int
file_v1_word_char(kword_t word, unsigned int i)
{
        return (unsigned int)((word >> (27U - 9U * i)) & 0777UL);
}

static kword_t
file_v1_set_word_char(kword_t word, unsigned int i, unsigned int c)
{
        unsigned int shift;
        kword_t mask;

        shift = 27U - 9U * i;
        mask = (kword_t)0777UL << shift;
        return (word & ~mask) | (((kword_t)c & 0777UL) << shift);
}

int
file_v1_readchar(unsigned int owner, int fd)
{
        struct file_v1 *fp;
        struct vfs_v1_stat st;
        unsigned int ch;
        unsigned int pos;
        unsigned int wi;
        unsigned int bi;
        kword_t word;
        int rc;

        fp = file_v1_find(owner, fd);
        if (fp == 0 || (fp->meta & FILE_V1_META_DIR) != 0U ||
            (FILE_V1_META_FLAGS(fp->meta) & FILE_V1_O_READ) == 0U)
                return -1;
        switch (VFS_V1_PROVIDER(fp->node)) {
        case MEMFS_V1_PROVIDER:
                if (memfs_v1_stat(file_v1_root, fp->node, &st) != 0)
                        return -1;
                if (fp->off_chars >= st.size_chars)
                        return -2;
                pos = (unsigned int)fp->off_chars;
                wi = pos / 4U;
                bi = pos % 4U;
                if (memfs_v1_read_words(file_v1_root, fp->node, wi, &word,
                    1U) != 1)
                        return -1;
                ch = file_v1_word_char(word, bi);
                break;
        case PROCFS_V1_PROVIDER:
                rc = procfs_v1_readchar(fp->node, fp->off_chars, &ch);
                if (rc <= 0)
                        return rc == 0 ? -2 : -1;
                break;
        case DEVICEFS_V1_PROVIDER:
                rc = devicefs_v1_readchar(fp->node, fp->off_chars, &ch);
                if (rc == FILE_V1_DEVICE_IO)
                        return rc;
                if (rc <= 0)
                        return rc == 0 ? -2 : -1;
                break;
        default:
                return -1;
        }
        ++fp->off_chars;
        return (int)ch;
}

int
file_v1_writechar(unsigned int owner, int fd, unsigned int ch)
{
        struct file_v1 *fp;
        struct vfs_v1_stat st;
        unsigned int pos;
        unsigned int wi;
        unsigned int bi;
        unsigned int need_words;
        kword_t word;
        kword_t end_chars;

        fp = file_v1_find(owner, fd);
        if (fp == 0 || (fp->meta & FILE_V1_META_DIR) != 0U ||
            (FILE_V1_META_FLAGS(fp->meta) & FILE_V1_O_WRITE) == 0U)
                return -1;
        if (fp->node == VFS_V1_NODE(DEVICEFS_V1_PROVIDER,
            DEVICEFS_V1_KIND_DEVICE, DEVICEFS_V1_DEV_CTY0))
                return FILE_V1_DEVICE_IO;
        if (VFS_V1_PROVIDER(fp->node) != MEMFS_V1_PROVIDER ||
            memfs_v1_stat(file_v1_root, fp->node, &st) != 0)
                return -1;
        if ((FILE_V1_META_FLAGS(fp->meta) & FILE_V1_O_APPEND) != 0U)
                fp->off_chars = st.size_chars;
        pos = (unsigned int)fp->off_chars;
        end_chars = fp->off_chars + 1U;
        need_words = (unsigned int)((end_chars + 3U) / 4U);
        if (need_words > (unsigned int)st.size_words &&
            memfs_v1_truncate_words(file_v1_root, fp->node, need_words,
                st.size_chars) != 0)
                return -1;
        wi = pos / 4U;
        bi = pos % 4U;
        word = 0;
        (void)memfs_v1_read_words(file_v1_root, fp->node, wi, &word, 1U);
        word = file_v1_set_word_char(word, bi, ch);
        if (memfs_v1_write_words(file_v1_root, fp->node, wi, &word, 1U,
            end_chars) != 1)
                return -1;
        fp->off_chars = end_chars;
        return 0;
}
int
file_v1_read_words(unsigned int owner, int fd, kword_t *buf, unsigned int nwords)
{
        struct file_v1 *fp;
        int rc;
        unsigned int off;

        fp = file_v1_find(owner, fd);
        if (fp == 0 || buf == 0 || (fp->meta & FILE_V1_META_DIR) != 0U ||
            (FILE_V1_META_FLAGS(fp->meta) & FILE_V1_O_READ) == 0U ||
            VFS_V1_PROVIDER(fp->node) != MEMFS_V1_PROVIDER)
                return -1;
        off = (unsigned int)(fp->off_chars / 4U);
        rc = memfs_v1_read_words(file_v1_root, fp->node, off, buf, nwords);
        if (rc > 0)
                fp->off_chars += (kword_t)(unsigned int)rc * 4U;
        return rc;
}

int
file_v1_write_words(unsigned int owner, int fd, const kword_t *buf,
    unsigned int nwords, kword_t size_chars)
{
        struct file_v1 *fp;
        struct vfs_v1_stat st;
        unsigned int off;
        int rc;

        fp = file_v1_find(owner, fd);
        if (fp == 0 || buf == 0 || (fp->meta & FILE_V1_META_DIR) != 0U ||
            (FILE_V1_META_FLAGS(fp->meta) & FILE_V1_O_WRITE) == 0U ||
            VFS_V1_PROVIDER(fp->node) != MEMFS_V1_PROVIDER ||
            memfs_v1_stat(file_v1_root, fp->node, &st) != 0)
                return -1;
        if ((FILE_V1_META_FLAGS(fp->meta) & FILE_V1_O_APPEND) != 0U)
                fp->off_chars = st.size_chars;
        off = (unsigned int)(fp->off_chars / 4U);
        rc = memfs_v1_write_words(file_v1_root, fp->node, off, buf, nwords,
            size_chars);
        if (rc > 0)
                fp->off_chars += (kword_t)(unsigned int)rc * 4U;
        return rc;
}

extern int file_v1_readdir(unsigned int owner, int fd,
    struct vfs_v1_dirent *ent);

int
file_v1_stat_path_owner(unsigned int owner, const kword_t *path,
    struct vfs_v1_stat *st)
{
        vnode_v1_t node;

        if (file_v1_lookup_path_owner(owner, path, &node) != 0)
                return -1;
        return file_v1_node_stat(node, st);
}

int
file_v1_mkdir_owner(unsigned int owner, const kword_t *path,
    unsigned int mode)
{
        vnode_v1_t dir;
        vnode_v1_t node;
        struct vfs_v1_name leaf;

        if (file_v1_parent_path_owner(owner, path, &dir, &leaf) != 0 ||
            VFS_V1_PROVIDER(dir) != MEMFS_V1_PROVIDER)
                return -1;
        return memfs_v1_mkdir(file_v1_root, dir, &leaf, mode, &node);
}

int
file_v1_unlink_owner(unsigned int owner, const kword_t *path)
{
        vnode_v1_t dir;
        struct vfs_v1_name leaf;

        if (file_v1_parent_path_owner(owner, path, &dir, &leaf) != 0 ||
            VFS_V1_PROVIDER(dir) != MEMFS_V1_PROVIDER)
                return -1;
        return memfs_v1_unlink(file_v1_root, dir, &leaf);
}

int
file_v1_truncate_owner(unsigned int owner, const kword_t *path,
    kword_t chars)
{
        vnode_v1_t node;
        unsigned int words;

        if (file_v1_lookup_path_owner(owner, path, &node) != 0 ||
            VFS_V1_PROVIDER(node) != MEMFS_V1_PROVIDER)
                return -1;
        words = (unsigned int)((chars + 3U) / 4U);
        return memfs_v1_truncate_words(file_v1_root, node, words, chars);
}

int
file_v1_rename(unsigned int owner, const kword_t *oldpath,
    const kword_t *newpath)
{
        vnode_v1_t olddir;
        vnode_v1_t newdir;
        struct vfs_v1_name oldname;
        struct vfs_v1_name newname;

        if (file_v1_parent_path_owner(owner, oldpath, &olddir, &oldname) != 0 ||
            file_v1_parent_path_owner(owner, newpath, &newdir, &newname) != 0 ||
            VFS_V1_PROVIDER(olddir) != MEMFS_V1_PROVIDER ||
            VFS_V1_PROVIDER(newdir) != MEMFS_V1_PROVIDER)
                return -1;
        return memfs_v1_rename(file_v1_root, olddir, &oldname, newdir,
            &newname);
}

int
file_v1_chdir(unsigned int owner, const kword_t *path)
{
        vnode_v1_t node;
        struct vfs_v1_stat st;
        int alias;

        if (owner >= FILE_V1_OWNER_MAX ||
            file_v1_walk_path_owner(owner, path, 0, &node, 0, &alias) != 0 ||
            file_v1_node_stat(node, &st) != 0 || st.type != VFS_V1_TYPE_DIR)
                return -1;
        file_v1_cwd[owner] = node;
        file_v1_alias_cwd_set(owner, alias);
        return 0;
}

int
file_v1_getcwd_pseudo(vnode_v1_t node, kword_t *buf, unsigned int nwords)
{
        struct vfs_v1_name name;
        kword_t pid;
        unsigned int i;

        for (i = 0U; i < nwords; ++i)
                buf[i] = 0;
        if (VFS_V1_PROVIDER(node) == DEVICEFS_V1_PROVIDER) {
                if (node == FILE_V1_DEVICE_ROOT && nwords >= 3U) {
                        buf[0] = 7U;
                        buf[1] = VFS_V1_SIX6('/','D','E','V','I','C');
                        buf[2] = VFS_V1_SIX6('E',' ',' ',' ',' ',' ');
                        return 0;
                }
                if (VFS_V1_KIND(node) == DEVICEFS_V1_KIND_CTYDIR &&
                    VFS_V1_INDEX(node) == DEVICEFS_V1_DEV_CTY0 && nwords >= 4U) {
                        buf[0] = 12U;
                        buf[1] = VFS_V1_SIX6('/','D','E','V','I','C');
                        buf[2] = VFS_V1_SIX6('E','/','C','T','Y','0');
                        buf[3] = 0;
                        return 0;
                }
                return -1;
        }
        if (VFS_V1_PROVIDER(node) != PROCFS_V1_PROVIDER || nwords < 2U)
                return -1;
        if (node == FILE_V1_PROC_ROOT) {
                buf[0] = 5U;
                buf[1] = VFS_V1_SIX6('/','P','R','O','C',' ');
                return 0;
        }
        if (VFS_V1_KIND(node) != PROCFS_V1_KIND_PROC || nwords < 3U ||
            procfs_v1_pid(VFS_V1_INDEX(node), &pid) != 0 || pid > 0377U ||
            vfs_v1_name_set_pid(&name, (unsigned int)pid) != 0)
                return -1;
        buf[0] = 6U + name.chars;
        buf[1] = VFS_V1_SIX6('/','P','R','O','C','/');
        buf[2] = name.words[0];
        return 0;
}

static int
file_v1_alias_cwd_get(unsigned int owner)
{
        return (file_v1_alias_cwd[owner / 36U] &
            ((kword_t)1U << (owner % 36U))) != 0U;
}

static void
file_v1_alias_cwd_set(unsigned int owner, int active)
{
        kword_t mask;

        mask = (kword_t)1U << (owner % 36U);
        if (active)
                file_v1_alias_cwd[owner / 36U] |= mask;
        else
                file_v1_alias_cwd[owner / 36U] &= ~mask;
}

unsigned int
file_v1_used_slots(void)
{
        unsigned int i;
        unsigned int n;

        n = 0U;
        for (i = 0U; i < FILE_V1_NFILE; ++i) {
                if ((file_v1_table[i].meta & FILE_V1_META_USED) != 0U)
                        ++n;
        }
        return n;
}
