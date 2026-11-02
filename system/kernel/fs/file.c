#include "file.h"
#include "fs_mres.h"
#include "../ipc/pipe.h"
#include "../proc/proc.h"

extern struct file *file_table;

/* UNIX owner/group/other selection.  There are intentionally no supplementary
 * groups in 0.9.  Bootstrap/kernel context and UID 0 are privileged. */
extern int file_access_stat(const struct vfs_stat *st, unsigned int need);

extern int file_component(const kword_t *path, unsigned int *posp,
    struct vfs_name *name);

static int
file_name_dot(const struct vfs_name *name)
{
        return vfs_name_is6(name, PDP10_SIX6('.',' ',' ',' ',' ',' '), 1U);
}

static int
file_name_dotdot(const struct vfs_name *name)
{
        return vfs_name_is6(name, PDP10_SIX6('.','.',' ',' ',' ',' '), 2U);
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
                if (file_check_access(node, 01U) != 0)
                        return -1;
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
                        /* Symlinks store one counted SIXBIT path record. */
                        if (*depthp == FILE_SYMLINK_MAX ||
                            st.size_words < 2UL ||
                            st.size_words > FILE_PATH_WORDS)
                                return -1;
                        target_words = (unsigned int)st.size_words;
                        if (vfs_read_words(next, 0U, target, target_words) !=
                            (int)target_words)
                                return -1;
                        target_chars = (unsigned int)target[0];
                        if (target_chars == 0U ||
                            target_chars > FILE_PATH_MAX_CHARS ||
                            target_words != 1U + (target_chars + 5U) / 6U ||
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
        start = vfs_namespace_root;
        if (file_table != 0 && *((vnode_t *)file_table - 1) != VFS_NODE_NONE)
                start = *((vnode_t *)file_table - 1);
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


/* Close, dup, mount unlock, close-all and I/O wrappers are compact PDP-10 assembly. */
