#include "file.h"
#include "fs_mres.h"

extern struct file *file_table;

static vnode_t
file_cwd_get(void)
{
        if (file_table == 0)
                return VFS_NODE_NONE;
        return *((vnode_t *)file_table - 1);
}

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
        fs_copy_words(path, work, words);

restart:
        n = (unsigned int)work[0];
        if (file_path_char(work, 0U) == (unsigned int)('/' - 040)) {
                node = vfs_namespace_root;
        } else {
                node = start_node;
                if (node == VFS_NODE_NONE)
                        node = vfs_namespace_root;
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
                                fs_copy_words(combined, work, words);
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

        start = file_cwd_get();
        if (start == VFS_NODE_NONE)
                start = vfs_namespace_root;
        return file_walk_path_at(path, parent_only, start, 0U,
            nodep, leaf);
}

int
file_lookup_path(const kword_t *path, vnode_t *nodep)
{
        return file_walk_path(path, 0, nodep, 0);
}

int
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
        if (fd < 0)
                return fd;
        fp = &file_table[(unsigned int)fd - FILE_FD_FIRST];
        fp->meta |= (kword_t)((unsigned int)fd + 1U) <<
            FILE_META_DESC_SHIFT;
        if (st.type == VFS_TYPE_REG)
                fp->meta |= FILE_META_REGULAR;
        if ((flags & FILE_O_APPEND) != 0U)
                fp->off_chars = st.size_chars;
        return fd;
}


int
file_close(int fd)
{
        struct file *fp;
        fp = file_find(fd);
        if (fp == 0 || vfs_sync(fp->node) != 0)
                return -1;
        fp->node = VFS_NODE_NONE;
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
        dst = &file_table[(unsigned int)newfd - FILE_FD_FIRST];
        dst->off_chars = src->off_chars;
        dst->meta |= ((kword_t)FILE_META_DESC(src->meta) <<
            FILE_META_DESC_SHIFT) |
            (src->meta & (FILE_META_LOCK_MASK | FILE_META_REGULAR));
        return newfd;
}



/* Mount unlock, close-all and I/O wrappers are compact PDP-10 assembly. */
