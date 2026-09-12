#include "file.h"
#include "fs_mres.h"
#include "../ipc/pipe.h"

extern struct file *file_table;

static inline vnode_t
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
    vnode_t start_node, unsigned int *depthp,
    vnode_t *nodep, struct vfs_name *leaf)
{
        kword_t target[FILE_PATH_WORDS];
        unsigned int pos;
        unsigned int n;
        unsigned int target_chars;
        unsigned int target_words;
        int rc;
        vnode_t node;
        vnode_t next;
        struct vfs_name name;
        struct vfs_stat st;

        if (path == 0 || nodep == 0 ||
            (parent_only && leaf == 0))
                return -1;
        n = (unsigned int)path[0];
        if (n == 0U || n > FILE_PATH_MAX_CHARS)
                return -1;
        if (file_path_char(path, 0U) == (unsigned int)('/' - 040)) {
                node = vfs_namespace_root;
        } else {
                node = start_node;
                if (node == VFS_NODE_NONE)
                        node = vfs_namespace_root;
        }
        pos = 0U;
        for (;;) {
                rc = file_component(path, &pos, &name);
                if (rc < 0)
                        return -1;
                if (rc == 0) {
                        if (parent_only)
                                return -1;
                        *nodep = node;
                        return 0;
                }
                /* file_component() bounds both values by the path limit. */
                if (parent_only && (int)pos >= (int)n) {
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
                if (vfs_lookup(node, &name, &next) != 0 ||
                    vfs_stat(next, &st) != 0)
                        return -1;
                if (st.type == VFS_TYPE_SYMLINK) {
                        /* One counter spans target recursion and resumed suffixes. */
                        if (*depthp == FILE_SYMLINK_MAX || st.size_chars == 0UL ||
                            st.size_chars > FILE_PATH_MAX_CHARS)
                                return -1;
                        target_chars = (unsigned int)st.size_chars;
                        target_words = (target_chars + 5U) / 6U;
                        target[0] = target_chars;
                        if (vfs_read_words(next, 0U, &target[1], target_words) !=
                            (int)target_words ||
                            (++*depthp, file_walk_path_at(target, 0, node,
                            depthp, &next, 0)) != 0)
                                return -1;
                }
                node = next;
        }
}

static int
file_walk_path(const kword_t *path,
    int parent_only, vnode_t *nodep, struct vfs_name *leaf)
{
        vnode_t start;
        unsigned int depth;

        depth = 0U;
        start = file_cwd_get();
        if (start == VFS_NODE_NONE)
                start = vfs_namespace_root;
        return file_walk_path_at(path, parent_only, start, &depth,
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
        fp = &file_table[(unsigned int)fd];
        if (st.type == VFS_TYPE_FIFO) {
                node = pipe_fifo_open(node, fp->node_meta);
                if (node == VFS_NODE_NONE) {
                        fp->node_meta = 0UL;
                        fp->off_chars = 0UL;
                        return -1;
                }
                fp->node_meta = node |
                    (fp->node_meta & ~FILE_NODE_VNODE_MASK);
                fp->off_chars = 0UL;
                return fd;
        }
        if (st.type == VFS_TYPE_REG)
                fp->node_meta |= FILE_META_REGULAR;
        if ((flags & FILE_O_APPEND) != 0U)
                fp->off_chars = st.size_chars;
        return fd;
}



/* Close, dup, mount unlock, close-all and I/O wrappers are compact PDP-10 assembly. */
