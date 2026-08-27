#ifndef DAIMON_FILE_V1_H
#define DAIMON_FILE_V1_H

#include "memfs_v1.h"

#define FILE_V1_O_READ          0001U
#define FILE_V1_O_WRITE         0002U
#define FILE_V1_O_APPEND        0004U
#define FILE_V1_O_CREAT         0010U
#define FILE_V1_O_TRUNC         0020U
#define FILE_V1_DEVICE_IO       (-3)

#ifndef FILE_V1_NFILE
#define FILE_V1_NFILE           32U
#endif
#define FILE_V1_FD_FIRST        3U
#define FILE_V1_FD_MAX          15U
#ifndef FILE_V1_OWNER_MAX
#define FILE_V1_OWNER_MAX       64U
#endif

struct file_v1 {
        vnode_v1_t node;
        kword_t off_chars;
        kword_t meta;
};

#define FILE_V1_META_USED       0001U
#define FILE_V1_META_DIR        0002U
#define FILE_V1_META_FLAGS_SHIFT 2U
#define FILE_V1_META_FD_SHIFT   8U
#define FILE_V1_META_OWNER_SHIFT 14U
#define FILE_V1_META_FLAGS(m)   (((m) >> FILE_V1_META_FLAGS_SHIFT) & 077U)
#define FILE_V1_META_FD(m)      (((m) >> FILE_V1_META_FD_SHIFT) & 077U)
#define FILE_V1_META_OWNER(m)   (((m) >> FILE_V1_META_OWNER_SHIFT) & 077U)

#ifdef __PDP10__
extern struct memfs_v1 *file_v1_root;
extern vnode_v1_t file_v1_alias_node;
#else
void file_v1_init(struct memfs_v1 *rootfs);
int file_v1_alias_root(const struct vfs_v1_name *name, vnode_v1_t node);
#endif
struct memfs_v1 *file_v1_rootfs(void);
int file_v1_lookup_path_owner(unsigned int owner, const kword_t *path,
    vnode_v1_t *nodep);
int file_v1_open(unsigned int owner, const kword_t *path, unsigned int flags);
int file_v1_close(unsigned int owner, int fd);
int file_v1_readchar(unsigned int owner, int fd);
int file_v1_writechar(unsigned int owner, int fd, unsigned int ch);
int file_v1_read_words(unsigned int owner, int fd, kword_t *buf, unsigned int nwords);
int file_v1_write_words(unsigned int owner, int fd, const kword_t *buf,
    unsigned int nwords, kword_t size_chars);
int file_v1_readdir(unsigned int owner, int fd, struct vfs_v1_dirent *ent);
int file_v1_stat_path_owner(unsigned int owner, const kword_t *path,
    struct vfs_v1_stat *st);
int file_v1_mkdir_owner(unsigned int owner, const kword_t *path,
    unsigned int mode);
int file_v1_unlink_owner(unsigned int owner, const kword_t *path);
int file_v1_truncate_owner(unsigned int owner, const kword_t *path,
    kword_t chars);
int file_v1_rename(unsigned int owner, const kword_t *oldpath,
    const kword_t *newpath);
int file_v1_chdir(unsigned int owner, const kword_t *path);
int file_v1_getcwd(unsigned int owner, kword_t *buf, unsigned int nwords);
unsigned int file_v1_used_slots(void);

#endif
