#include "file_v1.h"

struct memfs_v1 *file_v1_root;

#define FILE_V1_MEMFS_ROOT VFS_V1_NODE(MEMFS_V1_PROVIDER, MEMFS_V1_KIND_NODE, 0U)

vnode_v1_t file_v1_alias_node;
extern struct file_v1 file_v1_table[FILE_V1_NFILE];
extern vnode_v1_t file_v1_cwd;
extern kword_t file_v1_alias_cwd;

extern int file_v1_component(const kword_t *path, unsigned int *posp,
    struct vfs_v1_name *name);

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

#define FILE_V1_PATH_WORDS       18U
#define FILE_V1_PATH_MAX_CHARS   ((FILE_V1_PATH_WORDS - 1U) * 6U)
#define FILE_V1_SYMLINK_MAX      8U

static unsigned int
file_v1_path_char(const kword_t *path, unsigned int pos)
{
        unsigned int shift;

        shift = 30U - 6U * (pos % 6U);
        return (unsigned int)((path[1U + pos / 6U] >> shift) & 077UL);
}

static void
file_v1_path_setchar(kword_t *path, unsigned int pos, unsigned int ch)
{
        unsigned int shift;
        kword_t mask;

        shift = 30U - 6U * (pos % 6U);
        mask = (kword_t)077UL << shift;
        path[1U + pos / 6U] = (path[1U + pos / 6U] & ~mask) |
            (((kword_t)ch & 077UL) << shift);
}

static int
file_v1_walk_path_at(const kword_t *path, int parent_only,
    vnode_v1_t start_node, int start_alias, unsigned int depth,
    vnode_v1_t *nodep, struct vfs_v1_name *leaf, int *aliasp)
{
        kword_t work[FILE_V1_PATH_WORDS];
        kword_t combined[FILE_V1_PATH_WORDS];
        unsigned int pos;
        unsigned int n;
        unsigned int words;
        unsigned int i;
        int rc;
        int alias;
        vnode_v1_t node;
        vnode_v1_t next;
        struct vfs_v1_name name;

        if (path == 0 || nodep == 0 ||
            (parent_only && leaf == 0) || depth > FILE_V1_SYMLINK_MAX)
                return -1;
        n = (unsigned int)path[0];
        if (n == 0U || n > FILE_V1_PATH_MAX_CHARS)
                return -1;
        words = 1U + (n + 5U) / 6U;
        for (i = 0U; i < FILE_V1_PATH_WORDS; ++i)
                work[i] = i < words ? path[i] : 0UL;

restart:
        n = (unsigned int)work[0];
        if (file_v1_path_char(work, 0U) == (unsigned int)('/' - 040)) {
                node = vfs_v1_root();
                alias = 0;
        } else {
                node = start_node;
                if (node == VFS_V1_NODE_NONE)
                        node = vfs_v1_root();
                alias = start_alias;
        }
        pos = 0U;
        for (;;) {
                rc = file_v1_component(work, &pos, &name);
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
                if (parent_only && pos >= n) {
                        if (file_v1_name_dot(&name) || file_v1_name_dotdot(&name))
                                return -1;
                        *nodep = node;
                        *leaf = name;
                        if (aliasp != 0)
                                *aliasp = alias;
                        return 0;
                }
                if (file_v1_name_dot(&name))
                        continue;
                if (file_v1_name_dotdot(&name)) {
                        if (alias && node == file_v1_alias_node) {
                                node = FILE_V1_MEMFS_ROOT;
                                alias = 0;
                        } else if (vfs_v1_parent(node, &next) != 0) {
                                return -1;
                        } else {
                                node = next;
                        }
                        continue;
                }
                if (vfs_v1_lookup(node, &name, &next) != 0)
                        return -1;
                {
                        struct vfs_v1_stat st;
                        if (vfs_v1_stat(next, &st) != 0)
                                return -1;
                        if (st.type == VFS_V1_TYPE_SYMLINK) {
                                unsigned int target_chars;
                                unsigned int target_words;
                                unsigned int out;

                                if (depth == FILE_V1_SYMLINK_MAX ||
                                    st.size_chars == 0UL ||
                                    st.size_chars > FILE_V1_PATH_MAX_CHARS)
                                        return -1;
                                target_chars = (unsigned int)st.size_chars;
                                target_words = (target_chars + 5U) / 6U;
                                for (i = 0U; i < FILE_V1_PATH_WORDS; ++i)
                                        combined[i] = 0UL;
                                if (vfs_v1_read_words(next, 0U, &combined[1],
                                    target_words) != (int)target_words)
                                        return -1;
                                out = target_chars;
                                if (pos < n) {
                                        if (file_v1_path_char(combined,
                                            target_chars - 1U) !=
                                            (unsigned int)('/' - 040)) {
                                                if (out >= FILE_V1_PATH_MAX_CHARS)
                                                        return -1;
                                                file_v1_path_setchar(combined,
                                                    out++, '/' - 040);
                                        }
                                        if (n - pos > FILE_V1_PATH_MAX_CHARS - out)
                                                return -1;
                                        for (i = pos; i < n; ++i)
                                                file_v1_path_setchar(combined,
                                                    out++, file_v1_path_char(work,
                                                    i));
                                }
                                combined[0] = out;
                                words = 1U + (out + 5U) / 6U;
                                for (i = 0U; i < FILE_V1_PATH_WORDS; ++i)
                                        work[i] = i < words ? combined[i] : 0UL;
                                start_node = node;
                                start_alias = alias;
                                ++depth;
                                goto restart;
                        }
                }
                if (aliasp != 0 && file_v1_alias_node != VFS_V1_NODE_NONE &&
                    node == FILE_V1_MEMFS_ROOT && next == file_v1_alias_node)
                        alias = 1;
                node = next;
        }
}

static int
file_v1_walk_path(const kword_t *path,
    int parent_only, vnode_v1_t *nodep, struct vfs_v1_name *leaf,
    int *aliasp)
{
        vnode_v1_t start;
        int alias;

        start = file_v1_cwd;
        if (start == VFS_V1_NODE_NONE)
                start = vfs_v1_root();
        alias = aliasp != 0 && file_v1_alias_cwd != 0;
        return file_v1_walk_path_at(path, parent_only, start, alias, 0U,
            nodep, leaf, aliasp);
}

int
file_v1_lookup_path(const kword_t *path, vnode_v1_t *nodep)
{
        return file_v1_walk_path(path, 0, nodep, 0, 0);
}

static int
file_v1_parent_path(const kword_t *path, vnode_v1_t *dirp,
    struct vfs_v1_name *leaf)
{
        return file_v1_walk_path(path, 1, dirp, leaf, 0);
}

extern struct file_v1 *file_v1_find(int fd);
extern int file_v1_new_fd(vnode_v1_t node, unsigned int flags, int isdir);

int
file_v1_open(const kword_t *path, unsigned int flags)
{
        vnode_v1_t node;
        vnode_v1_t dir;
        struct vfs_v1_name leaf;
        struct vfs_v1_stat st;
        struct file_v1 *fp;
        int fd;

        if (file_v1_lookup_path(path, &node) != 0) {
                if ((flags & FILE_V1_O_CREAT) == 0U ||
                    file_v1_parent_path(path, &dir, &leaf) != 0 ||
                    vfs_v1_create(dir, &leaf, 0666U, &node) != 0)
                        return -1;
        }
        if (vfs_v1_stat(node, &st) != 0)
                return -1;
        if ((flags & FILE_V1_O_TRUNC) != 0U && st.type == VFS_V1_TYPE_REG) {
                if (vfs_v1_truncate(node, 0U, 0) != 0)
                        return -1;
                st.size_chars = 0;
        }
        fd = file_v1_new_fd(node, flags, st.type == VFS_V1_TYPE_DIR);
        if (fd >= 0) {
                fp = file_v1_find(fd);
                if (fp != 0) {
                        fp->meta |= (kword_t)((unsigned int)fd + 1U) <<
                            FILE_V1_META_DESC_SHIFT;
                        if ((flags & FILE_V1_O_APPEND) != 0U)
                                fp->off_chars = st.size_chars;
                }
        }
        return fd;
}


int
file_v1_close(int fd)
{
        struct file_v1 *fp;
        vnode_v1_t node;
        unsigned int desc;
        unsigned int i;
        int still_open;

        fp = file_v1_find(fd);
        if (fp == 0 || vfs_v1_sync(fp->node) != 0)
                return -1;
        node = fp->node;
        desc = FILE_V1_META_DESC(fp->meta);
        fp->meta = 0U;
        fp->node = VFS_V1_NODE_NONE;
        fp->off_chars = 0;
        still_open = 0;
        if (desc != 0U)
                for (i = 0U; i < FILE_V1_NFILE; ++i)
                        if ((file_v1_table[i].meta & FILE_V1_META_USED) != 0U &&
                            FILE_V1_META_DESC(file_v1_table[i].meta) == desc) {
                                still_open = 1;
                                break;
                        }
        if (!still_open)
                vfs_v1_unlock_owner(node, desc);
        return 0;
}

int
file_v1_dup(int fd)
{
        struct file_v1 *src;
        struct file_v1 *dst;
        unsigned int flags;
        int newfd;

        src = file_v1_find(fd);
        if (src == 0)
                return -1;
        flags = FILE_V1_META_FLAGS(src->meta);
        newfd = file_v1_new_fd(src->node, flags,
            (src->meta & FILE_V1_META_DIR) != 0U);
        if (newfd < 0)
                return -1;
        dst = file_v1_find(newfd);
        if (dst == 0)
                return -1;
        dst->off_chars = src->off_chars;
        dst->meta |= (kword_t)FILE_V1_META_DESC(src->meta) <<
            FILE_V1_META_DESC_SHIFT;
        return newfd;
}

int
file_v1_lock(int fd, unsigned int op)
{
        struct file_v1 *fp;
        unsigned int desc;

        fp = file_v1_find(fd);
        if (fp == 0 || (fp->meta & FILE_V1_META_DIR) != 0U)
                return -1;
        desc = FILE_V1_META_DESC(fp->meta);
        if (desc == 0U)
                return -1;
        return vfs_v1_lock(fp->node, desc, op);
}

void
file_v1_close_all(void)
{
        unsigned int i;

        for (i = 0U; i < FILE_V1_NFILE; ++i)
                if ((file_v1_table[i].meta & FILE_V1_META_USED) != 0U)
                        (void)file_v1_close((int)FILE_V1_META_FD(
                            file_v1_table[i].meta));
}


int
file_v1_readchar(int fd)
{
        struct file_v1 *fp;
        unsigned int ch;
        int rc;

        fp = file_v1_find(fd);
        if (fp == 0 || (fp->meta & FILE_V1_META_DIR) != 0U ||
            (FILE_V1_META_FLAGS(fp->meta) & FILE_V1_O_READ) == 0U)
                return -1;
        rc = vfs_v1_readchar(fp->node, fp->off_chars, &ch);
        if (rc == VFS_V1_DEVICE_IO)
                return rc;
        if (rc <= 0)
                return rc == 0 ? -2 : -1;
        ++fp->off_chars;
        return (int)ch;
}


int
file_v1_writechar(int fd, unsigned int ch)
{
        struct file_v1 *fp;
        int rc;

        fp = file_v1_find(fd);
        if (fp == 0 || (fp->meta & FILE_V1_META_DIR) != 0U ||
            (FILE_V1_META_FLAGS(fp->meta) & FILE_V1_O_WRITE) == 0U)
                return -1;
        rc = vfs_v1_writechar(fp->node, fp->off_chars, ch);
        if (rc != 0)
                return rc;
        ++fp->off_chars;
        return 0;
}

int
file_v1_read_words(int fd, kword_t *buf, unsigned int nwords)
{
        struct file_v1 *fp;
        int rc;
        unsigned int off;

        fp = file_v1_find(fd);
        if (fp == 0 || buf == 0 || (fp->meta & FILE_V1_META_DIR) != 0U ||
            (FILE_V1_META_FLAGS(fp->meta) & FILE_V1_O_READ) == 0U)
                return -1;
        off = (unsigned int)(fp->off_chars / 4U);
        rc = vfs_v1_read_words(fp->node, off, buf, nwords);
        if (rc > 0)
                fp->off_chars += (kword_t)(unsigned int)rc * 4U;
        return rc;
}


int
file_v1_write_words(int fd, const kword_t *buf, unsigned int nwords,
    kword_t size_chars)
{
        struct file_v1 *fp;
        unsigned int off;
        int rc;

        fp = file_v1_find(fd);
        if (fp == 0 || buf == 0 || (fp->meta & FILE_V1_META_DIR) != 0U ||
            (FILE_V1_META_FLAGS(fp->meta) & FILE_V1_O_WRITE) == 0U)
                return -1;
        off = (unsigned int)(fp->off_chars / 4U);
        rc = vfs_v1_write_words(fp->node, off, buf, nwords, size_chars);
        if (rc > 0)
                fp->off_chars += (kword_t)(unsigned int)rc * 4U;
        return rc;
}

int
file_v1_readdir(int fd, struct vfs_v1_dirent *ent)
{
        struct file_v1 *fp;
        int rc;

        fp = file_v1_find(fd);
        if (fp == 0 || ent == 0 || (fp->meta & FILE_V1_META_DIR) == 0U)
                return -1;
        rc = vfs_v1_readdir(fp->node, (unsigned int)fp->off_chars, ent);
        if (rc > 0)
                ++fp->off_chars;
        return rc;
}


int
file_v1_stat_path(const kword_t *path, struct vfs_v1_stat *st)
{
        vnode_v1_t node;

        if (file_v1_lookup_path(path, &node) != 0)
                return -1;
        return vfs_v1_stat(node, st);
}

int
file_v1_mkdir(const kword_t *path, unsigned int mode)
{
        vnode_v1_t dir;
        vnode_v1_t node;
        struct vfs_v1_name leaf;

        if (file_v1_parent_path(path, &dir, &leaf) != 0)
                return -1;
        return vfs_v1_mkdir(dir, &leaf, mode, &node);
}


int
file_v1_symlink(const kword_t *target, const kword_t *linkpath)
{
        vnode_v1_t dir;
        vnode_v1_t node;
        struct vfs_v1_name leaf;
        unsigned int chars;

        if (target == 0 || linkpath == 0)
                return -1;
        chars = (unsigned int)target[0];
        if (chars == 0U || chars > FILE_V1_PATH_MAX_CHARS ||
            file_v1_parent_path(linkpath, &dir, &leaf) != 0)
                return -1;
        return vfs_v1_symlink(dir, &leaf, target + 1, chars, &node);
}

int
file_v1_unlink(const kword_t *path)
{
        vnode_v1_t dir;
        struct vfs_v1_name leaf;

        if (file_v1_parent_path(path, &dir, &leaf) != 0)
                return -1;
        return vfs_v1_unlink(dir, &leaf);
}


int
file_v1_truncate(const kword_t *path, kword_t chars)
{
        vnode_v1_t node;
        unsigned int words;

        if (file_v1_lookup_path(path, &node) != 0)
                return -1;
        words = (unsigned int)((chars + 3U) / 4U);
        return vfs_v1_truncate(node, words, chars);
}


int
file_v1_rename(const kword_t *oldpath, const kword_t *newpath)
{
        vnode_v1_t olddir;
        vnode_v1_t newdir;
        struct vfs_v1_name oldname;
        struct vfs_v1_name newname;

        if (file_v1_parent_path(oldpath, &olddir, &oldname) != 0 ||
            file_v1_parent_path(newpath, &newdir, &newname) != 0)
                return -1;
        return vfs_v1_rename(olddir, &oldname, newdir, &newname);
}


int
file_v1_chdir(const kword_t *path)
{
        vnode_v1_t node;
        struct vfs_v1_stat st;
        int alias;

        if (file_v1_walk_path(path, 0, &node, 0, &alias) != 0 ||
            vfs_v1_stat(node, &st) != 0 || st.type != VFS_V1_TYPE_DIR)
                return -1;
        file_v1_cwd = node;
        file_v1_alias_cwd = alias != 0;
        return 0;
}
