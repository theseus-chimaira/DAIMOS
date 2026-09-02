#include "file.h"

extern struct file file_table[FILE_NFILE];
extern vnode_t file_cwd;

extern int file_component(const kword_t *path, unsigned int *posp,
    struct vfs_name *name);

static int
file_name_dot(const struct vfs_name *name)
{
        return vfs_name_is6(name, VFS_SIX6('.',' ',' ',' ',' ',' '), 1U);
}

static int
file_name_dotdot(const struct vfs_name *name)
{
        return vfs_name_is6(name, VFS_SIX6('.','.',' ',' ',' ',' '), 2U);
}

#define FILE_PATH_WORDS       18U
#define FILE_PATH_MAX_CHARS   ((FILE_PATH_WORDS - 1U) * 6U)
#define FILE_SYMLINK_MAX      8U

extern unsigned int file_path_char(const kword_t *path, unsigned int pos);
extern void file_path_setchar(kword_t *path, unsigned int pos, unsigned int ch);

static int
file_walk_path_at(const kword_t *path, int parent_only,
    vnode_t start_node, unsigned int depth,
    vnode_t *nodep, struct vfs_name *leaf)
{
        kword_t work[FILE_PATH_WORDS];
        kword_t combined[FILE_PATH_WORDS];
        unsigned int pos;
        unsigned int n;
        unsigned int words;
        unsigned int i;
        int rc;
        vnode_t node;
        vnode_t next;
        struct vfs_name name;

        if (path == 0 || nodep == 0 ||
            (parent_only && leaf == 0) || depth > FILE_SYMLINK_MAX)
                return -1;
        n = (unsigned int)path[0];
        if (n == 0U || n > FILE_PATH_MAX_CHARS)
                return -1;
        words = 1U + (n + 5U) / 6U;
        for (i = 0U; i < FILE_PATH_WORDS; ++i)
                work[i] = i < words ? path[i] : 0UL;

restart:
        n = (unsigned int)work[0];
        if (file_path_char(work, 0U) == (unsigned int)('/' - 040)) {
                node = vfs_root();
        } else {
                node = start_node;
                if (node == VFS_NODE_NONE)
                        node = vfs_root();
        }
        pos = 0U;
        for (;;) {
                rc = file_component(work, &pos, &name);
                if (rc < 0)
                        return -1;
                if (rc == 0) {
                        if (parent_only)
                                return -1;
                        *nodep = node;
                        return 0;
                }
                if (parent_only && pos >= n) {
                        if (file_name_dot(&name) || file_name_dotdot(&name))
                                return -1;
                        *nodep = node;
                        *leaf = name;
                        return 0;
                }
                if (file_name_dot(&name))
                        continue;
                if (file_name_dotdot(&name)) {
                        if (vfs_parent(node, &next) != 0)
                                return -1;
                        node = next;
                        continue;
                }
                if (vfs_lookup(node, &name, &next) != 0)
                        return -1;
                {
                        struct vfs_stat st;
                        if (vfs_stat(next, &st) != 0)
                                return -1;
                        if (st.type == VFS_TYPE_SYMLINK) {
                                unsigned int target_chars;
                                unsigned int target_words;
                                unsigned int out;

                                if (depth == FILE_SYMLINK_MAX ||
                                    st.size_chars == 0UL ||
                                    st.size_chars > FILE_PATH_MAX_CHARS)
                                        return -1;
                                target_chars = (unsigned int)st.size_chars;
                                target_words = (target_chars + 5U) / 6U;
                                for (i = 0U; i < FILE_PATH_WORDS; ++i)
                                        combined[i] = 0UL;
                                if (vfs_read_words(next, 0U, &combined[1],
                                    target_words) != (int)target_words)
                                        return -1;
                                out = target_chars;
                                if (pos < n) {
                                        if (file_path_char(combined,
                                            target_chars - 1U) !=
                                            (unsigned int)('/' - 040)) {
                                                if (out >= FILE_PATH_MAX_CHARS)
                                                        return -1;
                                                file_path_setchar(combined,
                                                    out++, '/' - 040);
                                        }
                                        if (n - pos > FILE_PATH_MAX_CHARS - out)
                                                return -1;
                                        for (i = pos; i < n; ++i)
                                                file_path_setchar(combined,
                                                    out++, file_path_char(work,
                                                    i));
                                }
                                combined[0] = out;
                                words = 1U + (out + 5U) / 6U;
                                for (i = 0U; i < FILE_PATH_WORDS; ++i)
                                        work[i] = i < words ? combined[i] : 0UL;
                                start_node = node;
                                ++depth;
                                goto restart;
                        }
                }
                node = next;
        }
}

static int
file_walk_path(const kword_t *path,
    int parent_only, vnode_t *nodep, struct vfs_name *leaf)
{
        vnode_t start;

        start = file_cwd;
        if (start == VFS_NODE_NONE)
                start = vfs_root();
        return file_walk_path_at(path, parent_only, start, 0U,
            nodep, leaf);
}

int
file_lookup_path(const kword_t *path, vnode_t *nodep)
{
        return file_walk_path(path, 0, nodep, 0);
}

static int
file_parent_path(const kword_t *path, vnode_t *dirp,
    struct vfs_name *leaf)
{
        return file_walk_path(path, 1, dirp, leaf);
}

extern struct file *file_find(int fd);
extern int file_new_fd(vnode_t node, unsigned int flags, int isdir);

int
file_open(const kword_t *path, unsigned int flags)
{
        vnode_t node;
        vnode_t dir;
        struct vfs_name leaf;
        struct vfs_stat st;
        struct file *fp;
        int fd;

        if (file_lookup_path(path, &node) != 0) {
                if ((flags & FILE_O_CREAT) == 0U ||
                    file_parent_path(path, &dir, &leaf) != 0 ||
                    vfs_create(dir, &leaf, 0666U, &node) != 0)
                        return -1;
        }
        if (vfs_stat(node, &st) != 0)
                return -1;
        if ((flags & FILE_O_TRUNC) != 0U && st.type == VFS_TYPE_REG) {
                if (vfs_truncate(node, 0U, 0) != 0)
                        return -1;
                st.size_chars = 0;
        }
        fd = file_new_fd(node, flags, st.type == VFS_TYPE_DIR);
        if (fd >= 0) {
                fp = file_find(fd);
                if (fp != 0) {
                        fp->meta |= (kword_t)((unsigned int)fd + 1U) <<
                            FILE_META_DESC_SHIFT;
                        if ((flags & FILE_O_APPEND) != 0U)
                                fp->off_chars = st.size_chars;
                }
        }
        return fd;
}


int
file_close(int fd)
{
        struct file *fp;
        vnode_t node;
        unsigned int desc;
        unsigned int i;
        int still_open;

        fp = file_find(fd);
        if (fp == 0 || vfs_sync(fp->node) != 0)
                return -1;
        node = fp->node;
        desc = FILE_META_DESC(fp->meta);
        fp->meta = 0U;
        fp->node = VFS_NODE_NONE;
        fp->off_chars = 0;
        still_open = 0;
        if (desc != 0U)
                for (i = 0U; i < FILE_NFILE; ++i)
                        if ((file_table[i].meta & FILE_META_USED) != 0U &&
                            FILE_META_DESC(file_table[i].meta) == desc) {
                                still_open = 1;
                                break;
                        }
        if (!still_open)
                vfs_unlock_owner(node, desc);
        return 0;
}

int
file_dup(int fd)
{
        struct file *src;
        struct file *dst;
        unsigned int flags;
        int newfd;

        src = file_find(fd);
        if (src == 0)
                return -1;
        flags = FILE_META_FLAGS(src->meta);
        newfd = file_new_fd(src->node, flags,
            (src->meta & FILE_META_DIR) != 0U);
        if (newfd < 0)
                return -1;
        dst = file_find(newfd);
        if (dst == 0)
                return -1;
        dst->off_chars = src->off_chars;
        dst->meta |= (kword_t)FILE_META_DESC(src->meta) <<
            FILE_META_DESC_SHIFT;
        return newfd;
}

int
file_lock(int fd, unsigned int op)
{
        struct file *fp;
        unsigned int desc;

        fp = file_find(fd);
        if (fp == 0 || (fp->meta & FILE_META_DIR) != 0U)
                return -1;
        desc = FILE_META_DESC(fp->meta);
        if (desc == 0U)
                return -1;
        return vfs_lock(fp->node, desc, op);
}

void
file_close_all(void)
{
        unsigned int i;

        for (i = 0U; i < FILE_NFILE; ++i)
                if ((file_table[i].meta & FILE_META_USED) != 0U)
                        (void)file_close((int)FILE_META_FD(
                            file_table[i].meta));
}


int
file_readchar(int fd)
{
        struct file *fp;
        unsigned int ch;
        int rc;

        fp = file_find(fd);
        if (fp == 0 || (fp->meta & FILE_META_DIR) != 0U ||
            (FILE_META_FLAGS(fp->meta) & FILE_O_READ) == 0U)
                return -1;
        rc = vfs_readchar(fp->node, fp->off_chars, &ch);
        if (rc == VFS_DEVICE_IO)
                return rc;
        if (rc <= 0)
                return rc == 0 ? -2 : -1;
        ++fp->off_chars;
        return (int)ch;
}


int
file_writechar(int fd, unsigned int ch)
{
        struct file *fp;
        int rc;

        fp = file_find(fd);
        if (fp == 0 || (fp->meta & FILE_META_DIR) != 0U ||
            (FILE_META_FLAGS(fp->meta) & FILE_O_WRITE) == 0U)
                return -1;
        rc = vfs_writechar(fp->node, fp->off_chars, ch);
        if (rc != 0)
                return rc;
        ++fp->off_chars;
        return 0;
}

int
file_read_words(int fd, kword_t *buf, unsigned int nwords)
{
        struct file *fp;
        int rc;
        unsigned int off;

        fp = file_find(fd);
        if (fp == 0 || buf == 0 || (fp->meta & FILE_META_DIR) != 0U ||
            (FILE_META_FLAGS(fp->meta) & FILE_O_READ) == 0U)
                return -1;
        off = (unsigned int)(fp->off_chars / 4U);
        rc = vfs_read_words(fp->node, off, buf, nwords);
        if (rc > 0)
                fp->off_chars += (kword_t)(unsigned int)rc * 4U;
        return rc;
}


int
file_write_words(int fd, const kword_t *buf, unsigned int nwords,
    kword_t size_chars)
{
        struct file *fp;
        unsigned int off;
        int rc;

        fp = file_find(fd);
        if (fp == 0 || buf == 0 || (fp->meta & FILE_META_DIR) != 0U ||
            (FILE_META_FLAGS(fp->meta) & FILE_O_WRITE) == 0U)
                return -1;
        off = (unsigned int)(fp->off_chars / 4U);
        rc = vfs_write_words(fp->node, off, buf, nwords, size_chars);
        if (rc > 0)
                fp->off_chars += (kword_t)(unsigned int)rc * 4U;
        return rc;
}

int
file_readdir(int fd, struct vfs_dirent *ent)
{
        struct file *fp;
        int rc;

        fp = file_find(fd);
        if (fp == 0 || ent == 0 || (fp->meta & FILE_META_DIR) == 0U)
                return -1;
        rc = vfs_readdir(fp->node, (unsigned int)fp->off_chars, ent);
        if (rc > 0)
                ++fp->off_chars;
        return rc;
}


int
file_stat_path(const kword_t *path, struct vfs_stat *st)
{
        vnode_t node;

        if (file_lookup_path(path, &node) != 0)
                return -1;
        return vfs_stat(node, st);
}

int
file_mkdir(const kword_t *path, unsigned int mode)
{
        vnode_t dir;
        vnode_t node;
        struct vfs_name leaf;

        if (file_parent_path(path, &dir, &leaf) != 0)
                return -1;
        return vfs_mkdir(dir, &leaf, mode, &node);
}


int
file_symlink(const kword_t *target, const kword_t *linkpath)
{
        vnode_t dir;
        vnode_t node;
        struct vfs_name leaf;
        unsigned int chars;

        if (target == 0 || linkpath == 0)
                return -1;
        chars = (unsigned int)target[0];
        if (chars == 0U || chars > FILE_PATH_MAX_CHARS ||
            file_parent_path(linkpath, &dir, &leaf) != 0)
                return -1;
        return vfs_symlink(dir, &leaf, target + 1, chars, &node);
}

int
file_unlink(const kword_t *path)
{
        vnode_t dir;
        struct vfs_name leaf;

        if (file_parent_path(path, &dir, &leaf) != 0)
                return -1;
        return vfs_unlink(dir, &leaf);
}


int
file_truncate(const kword_t *path, kword_t chars)
{
        vnode_t node;
        unsigned int words;

        if (file_lookup_path(path, &node) != 0)
                return -1;
        words = (unsigned int)((chars + 3U) / 4U);
        return vfs_truncate(node, words, chars);
}


int
file_rename(const kword_t *oldpath, const kword_t *newpath)
{
        vnode_t olddir;
        vnode_t newdir;
        struct vfs_name oldname;
        struct vfs_name newname;

        if (file_parent_path(oldpath, &olddir, &oldname) != 0 ||
            file_parent_path(newpath, &newdir, &newname) != 0)
                return -1;
        return vfs_rename(olddir, &oldname, newdir, &newname);
}


int
file_chdir(const kword_t *path)
{
        vnode_t node;
        struct vfs_stat st;

        if (file_walk_path(path, 0, &node, 0) != 0 ||
            vfs_stat(node, &st) != 0 || st.type != VFS_TYPE_DIR)
                return -1;
        file_cwd = node;
        return 0;
}
