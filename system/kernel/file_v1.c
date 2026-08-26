#include "file_v1.h"
#include "devicefs_v1.h"
#include "procfs_v1.h"

static struct memfs_v1 *file_v1_root;
static struct vfs_v1_name file_v1_alias_name;
static vnode_v1_t file_v1_alias_node;
static int file_v1_alias_active;
static struct file_v1 file_v1_table[FILE_V1_NFILE];
static vnode_v1_t file_v1_cwd[FILE_V1_OWNER_MAX];
static kword_t file_v1_alias_cwd[(FILE_V1_OWNER_MAX + 35U) / 36U];

static int file_v1_chdir_lookup(unsigned int owner, const kword_t *path,
    vnode_v1_t *nodep, int *aliasp);
static int file_v1_alias_cwd_get(unsigned int owner);
static void file_v1_alias_cwd_set(unsigned int owner, int active);

static unsigned int
file_v1_path_chars(const kword_t *path)
{
        if (path == 0)
                return 0U;
        return (unsigned int)path[0];
}

static unsigned int
file_v1_path_char(const kword_t *path, unsigned int pos)
{
        unsigned int wi;
        unsigned int bi;
        unsigned int shift;

        wi = 1U + pos / 6U;
        bi = pos % 6U;
        shift = 30U - bi * 6U;
        return (unsigned int)(((path[wi] >> shift) & 077UL) + 040U);
}

static int
file_v1_component(const kword_t *path, unsigned int *posp,
    struct vfs_v1_name *name)
{
        unsigned int n;
        unsigned int pos;
        unsigned int c;
        unsigned int i;
        unsigned int wi;
        unsigned int shift;

        if (path == 0 || posp == 0 || name == 0)
                return -1;
        n = file_v1_path_chars(path);
        pos = *posp;
        while (pos < n && file_v1_path_char(path, pos) == '/')
                ++pos;
        if (pos >= n) {
                *posp = pos;
                return 0;
        }
        name->chars = 0U;
        for (i = 0U; i < VFS_V1_NAME_WORDS; ++i)
                name->words[i] = 0;
        while (pos < n && file_v1_path_char(path, pos) != '/') {
                if (name->chars >= VFS_V1_NAME_MAX_CHARS)
                        return -1;
                c = file_v1_path_char(path, pos++);
                wi = name->chars / 6U;
                shift = 30U - (name->chars % 6U) * 6U;
                name->words[wi] |= ((kword_t)((c - 040U) & 077U)) << shift;
                ++name->chars;
        }
        *posp = pos;
        return 1;
}

static int
file_v1_is_name(const struct vfs_v1_name *name, kword_t word,
    unsigned int chars)
{
        return vfs_v1_name_is6(name, word, chars);
}

static int
file_v1_lookup_child(vnode_v1_t dir, const struct vfs_v1_name *name,
    vnode_v1_t *nodep)
{
        switch (VFS_V1_PROVIDER(dir)) {
        case MEMFS_V1_PROVIDER:
                if (dir == memfs_v1_root(file_v1_root)) {
                        if (file_v1_is_name(name,
                            VFS_V1_SIX6('D','E','V','I','C','E'), 6U)) {
                                *nodep = devicefs_v1_root();
                                return 0;
                        }
                        if (file_v1_is_name(name,
                            VFS_V1_SIX6('P','R','O','C',' ',' '), 4U)) {
                                *nodep = procfs_v1_root();
                                return 0;
                        }
                        if (file_v1_alias_active &&
                            name->chars == file_v1_alias_name.chars) {
                                unsigned int i;
                                for (i = 0U; i < VFS_V1_NAME_WORDS; ++i) {
                                        if (name->words[i] !=
                                            file_v1_alias_name.words[i])
                                                break;
                                }
                                if (i == VFS_V1_NAME_WORDS) {
                                        *nodep = file_v1_alias_node;
                                        return 0;
                                }
                        }
                }
                return memfs_v1_lookup(file_v1_root, dir, name, nodep);
        case DEVICEFS_V1_PROVIDER:
                return devicefs_v1_lookup(dir, name, nodep);
        case PROCFS_V1_PROVIDER:
                return procfs_v1_lookup(dir, name, nodep);
        default:
                return -1;
        }
}

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

void
file_v1_init(struct memfs_v1 *rootfs)
{
        unsigned int i;

        file_v1_root = rootfs;
        file_v1_alias_active = 0;
        file_v1_alias_node = VFS_V1_NODE_NONE;
        file_v1_alias_name.chars = 0U;
        for (i = 0U; i < VFS_V1_NAME_WORDS; ++i)
                file_v1_alias_name.words[i] = 0;
        for (i = 0U; i < FILE_V1_OWNER_MAX; ++i)
                file_v1_cwd[i] = memfs_v1_root(rootfs);
        for (i = 0U; i < (FILE_V1_OWNER_MAX + 35U) / 36U; ++i)
                file_v1_alias_cwd[i] = 0;
        for (i = 0U; i < FILE_V1_NFILE; ++i) {
                file_v1_table[i].node = VFS_V1_NODE_NONE;
                file_v1_table[i].off_chars = 0;
                file_v1_table[i].owner = 0U;
                file_v1_table[i].meta = 0U;
        }
}

struct memfs_v1 *
file_v1_rootfs(void)
{
        return file_v1_root;
}

int
file_v1_alias_root(const struct vfs_v1_name *name, vnode_v1_t node)
{
        struct vfs_v1_stat st;

        if (file_v1_root == 0 || file_v1_alias_active || name == 0 ||
            name->chars == 0U || file_v1_node_stat(node, &st) != 0 ||
            st.type != VFS_V1_TYPE_DIR)
                return -1;
        file_v1_alias_name = *name;
        file_v1_alias_node = node;
        file_v1_alias_active = 1;
        return 0;
}

static int
file_v1_parent_node(vnode_v1_t node, vnode_v1_t *parentp)
{
        unsigned int kind;

        if (parentp == 0)
                return -1;
        if (VFS_V1_PROVIDER(node) == MEMFS_V1_PROVIDER) {
                if (node == memfs_v1_root(file_v1_root)) {
                        *parentp = node;
                        return 0;
                }
                return memfs_v1_parent(file_v1_root, node, parentp, 0);
        }
        if (VFS_V1_PROVIDER(node) == DEVICEFS_V1_PROVIDER) {
                *parentp = memfs_v1_root(file_v1_root);
                return 0;
        }
        if (VFS_V1_PROVIDER(node) == PROCFS_V1_PROVIDER) {
                kind = VFS_V1_KIND(node);
                if (kind == PROCFS_V1_KIND_ROOT) {
                        *parentp = memfs_v1_root(file_v1_root);
                        return 0;
                }
                if (kind == PROCFS_V1_KIND_PROC) {
                        *parentp = procfs_v1_root();
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

int
file_v1_lookup_path_owner(unsigned int owner, const kword_t *path,
    vnode_v1_t *nodep)
{
        unsigned int pos;
        unsigned int n;
        int rc;
        vnode_v1_t node;
        vnode_v1_t next;
        struct vfs_v1_name name;

        if (file_v1_root == 0 || path == 0 || nodep == 0 ||
            owner >= FILE_V1_OWNER_MAX)
                return -1;
        n = file_v1_path_chars(path);
        if (n == 0U)
                return -1;
        if (file_v1_path_char(path, 0U) == '/')
                node = memfs_v1_root(file_v1_root);
        else
                node = file_v1_cwd[owner];
        pos = 0U;
        while (pos < n) {
                rc = file_v1_component(path, &pos, &name);
                if (rc < 0)
                        return -1;
                if (rc == 0)
                        break;
                if (file_v1_name_dot(&name))
                        continue;
                if (file_v1_name_dotdot(&name)) {
                        if (file_v1_parent_node(node, &next) != 0)
                                return -1;
                        node = next;
                        continue;
                }
                if (file_v1_lookup_child(node, &name, &next) != 0)
                        return -1;
                node = next;
        }
        *nodep = node;
        return 0;
}

int
file_v1_lookup_path(const kword_t *path, vnode_v1_t *nodep)
{
        return file_v1_lookup_path_owner(0U, path, nodep);
}

static int
file_v1_parent_path_owner(unsigned int owner, const kword_t *path,
    vnode_v1_t *dirp, struct vfs_v1_name *leaf)
{
        unsigned int pos;
        unsigned int n;
        int rc;
        vnode_v1_t node;
        vnode_v1_t next;
        struct vfs_v1_name name;

        if (file_v1_root == 0 || path == 0 || dirp == 0 || leaf == 0 ||
            owner >= FILE_V1_OWNER_MAX)
                return -1;
        n = file_v1_path_chars(path);
        if (n == 0U)
                return -1;
        if (file_v1_path_char(path, 0U) == '/')
                node = memfs_v1_root(file_v1_root);
        else
                node = file_v1_cwd[owner];
        pos = 0U;
        for (;;) {
                rc = file_v1_component(path, &pos, &name);
                if (rc <= 0)
                        return -1;
                while (pos < n && file_v1_path_char(path, pos) == '/')
                        ++pos;
                if (pos >= n) {
                        if (file_v1_name_dot(&name) || file_v1_name_dotdot(&name))
                                return -1;
                        *dirp = node;
                        *leaf = name;
                        return 0;
                }
                if (file_v1_name_dot(&name))
                        continue;
                if (file_v1_name_dotdot(&name)) {
                        if (file_v1_parent_node(node, &next) != 0)
                                return -1;
                        node = next;
                        continue;
                }
                if (file_v1_lookup_child(node, &name, &next) != 0)
                        return -1;
                node = next;
        }
}

static struct file_v1 *
file_v1_find(unsigned int owner, int fd)
{
        unsigned int i;

        if (fd < (int)FILE_V1_FD_FIRST || fd > (int)FILE_V1_FD_MAX)
                return 0;
        for (i = 0U; i < FILE_V1_NFILE; ++i) {
                if ((file_v1_table[i].meta & FILE_V1_META_USED) != 0U &&
                    file_v1_table[i].owner == owner &&
                    FILE_V1_META_FD(file_v1_table[i].meta) == (unsigned int)fd)
                        return &file_v1_table[i];
        }
        return 0;
}

static int
file_v1_new_fd(unsigned int owner, vnode_v1_t node, unsigned int flags,
    int isdir)
{
        unsigned int fd;
        unsigned int i;

        for (fd = FILE_V1_FD_FIRST; fd <= FILE_V1_FD_MAX; ++fd) {
                if (file_v1_find(owner, (int)fd) != 0)
                        continue;
                for (i = 0U; i < FILE_V1_NFILE; ++i) {
                        if ((file_v1_table[i].meta & FILE_V1_META_USED) != 0U)
                                continue;
                        file_v1_table[i].node = node;
                        file_v1_table[i].off_chars = 0;
                        file_v1_table[i].owner = owner;
                        file_v1_table[i].meta = FILE_V1_META_USED |
                            (isdir ? FILE_V1_META_DIR : 0U) |
                            ((flags & 077U) << FILE_V1_META_FLAGS_SHIFT) |
                            (fd << FILE_V1_META_FD_SHIFT);
                        return (int)fd;
                }
                return -1;
        }
        return -1;
}

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
        if ((flags & FILE_V1_O_TRUNC) != 0U) {
                if (VFS_V1_PROVIDER(node) != MEMFS_V1_PROVIDER ||
                    st.type != VFS_V1_TYPE_REG ||
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
        fp->owner = 0U;
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
file_v1_read(unsigned int owner, int fd, char *buf, unsigned int nchars)
{
        struct file_v1 *fp;
        struct vfs_v1_stat st;
        unsigned int n;
        unsigned int i;
        unsigned int pos;
        unsigned int wi;
        unsigned int bi;
        kword_t word;

        fp = file_v1_find(owner, fd);
        if (fp == 0 || buf == 0 || (fp->meta & FILE_V1_META_DIR) != 0U ||
            (FILE_V1_META_FLAGS(fp->meta) & FILE_V1_O_READ) == 0U ||
            VFS_V1_PROVIDER(fp->node) != MEMFS_V1_PROVIDER ||
            memfs_v1_stat(file_v1_root, fp->node, &st) != 0)
                return -1;
        if (fp->off_chars >= st.size_chars)
                return 0;
        n = (unsigned int)(st.size_chars - fp->off_chars);
        if (n > nchars)
                n = nchars;
        for (i = 0U; i < n; ++i) {
                pos = (unsigned int)fp->off_chars + i;
                wi = pos / 4U;
                bi = pos % 4U;
                if (memfs_v1_read_words(file_v1_root, fp->node, wi, &word, 1U) != 1)
                        return -1;
                buf[i] = (char)file_v1_word_char(word, bi);
        }
        fp->off_chars += n;
        return (int)n;
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
file_v1_write(unsigned int owner, int fd, const char *buf,
    unsigned int nchars)
{
        struct file_v1 *fp;
        struct vfs_v1_stat st;
        unsigned int i;
        unsigned int pos;
        unsigned int wi;
        unsigned int bi;
        unsigned int need_words;
        kword_t word;
        kword_t end_chars;

        fp = file_v1_find(owner, fd);
        if (fp == 0 || buf == 0 || (fp->meta & FILE_V1_META_DIR) != 0U ||
            (FILE_V1_META_FLAGS(fp->meta) & FILE_V1_O_WRITE) == 0U ||
            VFS_V1_PROVIDER(fp->node) != MEMFS_V1_PROVIDER ||
            memfs_v1_stat(file_v1_root, fp->node, &st) != 0)
                return -1;
        if ((FILE_V1_META_FLAGS(fp->meta) & FILE_V1_O_APPEND) != 0U)
                fp->off_chars = st.size_chars;
        end_chars = fp->off_chars + nchars;
        need_words = (unsigned int)((end_chars + 3U) / 4U);
        if (need_words > (unsigned int)st.size_words &&
            memfs_v1_truncate_words(file_v1_root, fp->node, need_words,
                st.size_chars) != 0)
                return -1;
        for (i = 0U; i < nchars; ++i) {
                pos = (unsigned int)fp->off_chars + i;
                wi = pos / 4U;
                bi = pos % 4U;
                word = 0;
                (void)memfs_v1_read_words(file_v1_root, fp->node, wi, &word, 1U);
                word = file_v1_set_word_char(word, bi,
                    (unsigned int)(unsigned char)buf[i]);
                if (memfs_v1_write_words(file_v1_root, fp->node, wi, &word, 1U,
                    end_chars) != 1)
                        return -1;
        }
        fp->off_chars = end_chars;
        return (int)nchars;
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

int
file_v1_readdir(unsigned int owner, int fd, struct vfs_v1_dirent *ent)
{
        struct file_v1 *fp;
        int rc;

        fp = file_v1_find(owner, fd);
        if (fp == 0 || ent == 0 || (fp->meta & FILE_V1_META_DIR) == 0U)
                return -1;
        switch (VFS_V1_PROVIDER(fp->node)) {
        case MEMFS_V1_PROVIDER:
                rc = memfs_v1_readdir(file_v1_root, fp->node,
                    (unsigned int)fp->off_chars, ent);
                if (fp->node == memfs_v1_root(file_v1_root) && rc == 0) {
                        unsigned int base;
                        base = 0U;
                        while (memfs_v1_readdir(file_v1_root, fp->node, base, ent) > 0)
                                ++base;
                        if ((unsigned int)fp->off_chars == base) {
                                vfs_v1_name_set6(&ent->name,
                                    VFS_V1_SIX6('D','E','V','I','C','E'), 6U);
                                ent->type = VFS_V1_TYPE_DIR;
                                rc = 1;
                        } else if ((unsigned int)fp->off_chars == base + 1U) {
                                vfs_v1_name_set6(&ent->name,
                                    VFS_V1_SIX6('P','R','O','C',' ',' '), 4U);
                                ent->type = VFS_V1_TYPE_DIR;
                                rc = 1;
                        } else if (file_v1_alias_active &&
                            (unsigned int)fp->off_chars == base + 2U) {
                                ent->name = file_v1_alias_name;
                                ent->type = VFS_V1_TYPE_DIR;
                                rc = 1;
                        }
                }
                break;
        case DEVICEFS_V1_PROVIDER:
                rc = devicefs_v1_readdir(fp->node, (unsigned int)fp->off_chars,
                    ent);
                break;
        case PROCFS_V1_PROVIDER:
                rc = procfs_v1_readdir(fp->node, (unsigned int)fp->off_chars,
                    ent);
                break;
        default:
                return -1;
        }
        if (rc > 0)
                ++fp->off_chars;
        return rc;
}

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
file_v1_stat_path(const kword_t *path, struct vfs_v1_stat *st)
{
        return file_v1_stat_path_owner(0U, path, st);
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
file_v1_mkdir(const kword_t *path, unsigned int mode)
{
        return file_v1_mkdir_owner(0U, path, mode);
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
file_v1_unlink(const kword_t *path)
{
        return file_v1_unlink_owner(0U, path);
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
file_v1_truncate(const kword_t *path, kword_t chars)
{
        return file_v1_truncate_owner(0U, path, chars);
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
            file_v1_chdir_lookup(owner, path, &node, &alias) != 0 ||
            file_v1_node_stat(node, &st) != 0 || st.type != VFS_V1_TYPE_DIR)
                return -1;
        file_v1_cwd[owner] = node;
        file_v1_alias_cwd_set(owner, alias);
        return 0;
}

static int
file_v1_path_put_char(kword_t *buf, unsigned int nwords,
    unsigned int pos, unsigned int ch)
{
        unsigned int wi;
        unsigned int shift;

        wi = 1U + pos / 6U;
        if (wi >= nwords)
                return -1;
        shift = 30U - (pos % 6U) * 6U;
        buf[wi] |= ((kword_t)((ch - 040U) & 077U)) << shift;
        return 0;
}

static int
file_v1_getcwd_pseudo(vnode_v1_t node, kword_t *buf, unsigned int nwords)
{
        kword_t pid;
        kword_t word;
        unsigned int i;
        unsigned int chars;

        for (i = 0U; i < nwords; ++i)
                buf[i] = 0;

        if (VFS_V1_PROVIDER(node) == DEVICEFS_V1_PROVIDER) {
                if (node == devicefs_v1_root() && nwords >= 3U) {
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
        if (node == procfs_v1_root()) {
                buf[0] = 5U;
                buf[1] = VFS_V1_SIX6('/','P','R','O','C',' ');
                return 0;
        }
        if (VFS_V1_KIND(node) != PROCFS_V1_KIND_PROC ||
            procfs_v1_value(VFS_V1_NODE(PROCFS_V1_PROVIDER,
            PROCFS_V1_KIND_PID, VFS_V1_INDEX(node)), &pid) != 0 || pid > 0377U ||
            nwords < 3U)
                return -1;
        if (pid >= 100U)
                chars = 3U;
        else if (pid >= 10U)
                chars = 2U;
        else
                chars = 1U;
        word = ((kword_t)(020U + (unsigned int)(pid / 100U)) << 30);
        if (chars < 3U)
                word = 0;
        if (chars >= 2U)
                word |= ((kword_t)(020U + (unsigned int)((pid / 10U) % 10U)) <<
                    (chars == 3U ? 24U : 30U));
        word |= ((kword_t)(020U + (unsigned int)(pid % 10U)) <<
            (chars == 3U ? 18U : chars == 2U ? 24U : 30U));
        buf[0] = 6U + chars;
        buf[1] = VFS_V1_SIX6('/','P','R','O','C','/');
        buf[2] = word;
        return 0;
}

int
file_v1_getcwd(unsigned int owner, kword_t *buf, unsigned int nwords)
{
        struct vfs_v1_name parts[16];
        vnode_v1_t node;
        vnode_v1_t parent;
        unsigned int depth;
        unsigned int i;
        unsigned int j;
        unsigned int pos;

        if (file_v1_root == 0 || owner >= FILE_V1_OWNER_MAX || buf == 0 ||
            nwords < 2U)
                return -1;
        node = file_v1_cwd[owner];
        if (VFS_V1_PROVIDER(node) != MEMFS_V1_PROVIDER)
                return file_v1_getcwd_pseudo(node, buf, nwords);
        for (i = 0U; i < nwords; ++i)
                buf[i] = 0;
        depth = 0U;
        while (node != memfs_v1_root(file_v1_root)) {
                if (depth >= 16U ||
                    memfs_v1_parent(file_v1_root, node, &parent,
                    &parts[depth]) != 0)
                        return -1;
                node = parent;
                ++depth;
        }
        pos = 0U;
        if (file_v1_path_put_char(buf, nwords, pos++, '/') != 0)
                return -1;
        for (i = depth; i != 0U; --i) {
                if (pos != 1U && file_v1_path_put_char(buf, nwords, pos++, '/') != 0)
                        return -1;
                for (j = 0U; j < parts[i - 1U].chars; ++j) {
                        unsigned int wi;
                        unsigned int shift;
                        unsigned int ch;
                        wi = j / 6U;
                        shift = 30U - (j % 6U) * 6U;
                        ch = (unsigned int)(((parts[i - 1U].words[wi] >> shift) & 077UL) + 040U);
                        if (file_v1_path_put_char(buf, nwords, pos++, ch) != 0)
                                return -1;
                }
        }
        buf[0] = (kword_t)pos;
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

static int
file_v1_chdir_lookup(unsigned int owner, const kword_t *path,
    vnode_v1_t *nodep, int *aliasp)
{
        struct vfs_v1_name name;
        vnode_v1_t next;
        vnode_v1_t node;
        unsigned int n;
        unsigned int pos;
        int alias;
        int rc;

        if (file_v1_root == 0 || path == 0 || nodep == 0 || aliasp == 0 ||
            owner >= FILE_V1_OWNER_MAX)
                return -1;
        n = file_v1_path_chars(path);
        if (n == 0U)
                return -1;
        if (file_v1_path_char(path, 0U) == '/') {
                node = memfs_v1_root(file_v1_root);
                alias = 0;
        } else {
                node = file_v1_cwd[owner];
                alias = file_v1_alias_cwd_get(owner);
        }
        pos = 0U;
        while (pos < n) {
                rc = file_v1_component(path, &pos, &name);
                if (rc < 0)
                        return -1;
                if (rc == 0)
                        break;
                if (file_v1_name_dot(&name))
                        continue;
                if (file_v1_name_dotdot(&name)) {
                        if (alias && node == file_v1_alias_node) {
                                node = memfs_v1_root(file_v1_root);
                                alias = 0;
                        } else {
                                if (file_v1_parent_node(node, &next) != 0)
                                        return -1;
                                node = next;
                        }
                        continue;
                }
                if (file_v1_lookup_child(node, &name, &next) != 0)
                        return -1;
                if (file_v1_alias_active &&
                    node == memfs_v1_root(file_v1_root) &&
                    next == file_v1_alias_node)
                        alias = 1;
                node = next;
        }
        *nodep = node;
        *aliasp = alias;
        return 0;
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
