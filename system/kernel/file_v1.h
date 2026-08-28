#ifndef DAIMON_FILE_V1_H
#define DAIMON_FILE_V1_H

#include "memfs_v1.h"

#define FILE_V1_O_READ          0001U
#define FILE_V1_O_WRITE         0002U
#define FILE_V1_O_APPEND        0004U
#define FILE_V1_O_CREAT         0010U
#define FILE_V1_O_TRUNC         0020U
#define FILE_V1_DEVICE_IO       VFS_V1_DEVICE_IO

#ifndef FILE_V1_NFILE
#define FILE_V1_NFILE           32U
#endif
#define FILE_V1_FD_FIRST        3U
#define FILE_V1_FD_MAX          15U

struct file_v1 {
        vnode_v1_t node;
        kword_t off_chars;
        kword_t meta;
};

#define FILE_V1_META_USED       0001U
#define FILE_V1_META_DIR        0002U
#define FILE_V1_META_FLAGS_SHIFT 2U
#define FILE_V1_META_FD_SHIFT   8U
#define FILE_V1_META_FLAGS(m)   (((m) >> FILE_V1_META_FLAGS_SHIFT) & 077U)
#define FILE_V1_META_FD(m)      (((m) >> FILE_V1_META_FD_SHIFT) & 077U)

/* DAIMOS 1.x has one live user process, so FILE state is global. */
extern struct memfs_v1 *file_v1_root;
extern vnode_v1_t file_v1_alias_node;
int file_v1_lookup_path(const kword_t *path, vnode_v1_t *nodep);
int file_v1_open(const kword_t *path, unsigned int flags);
int file_v1_close(int fd);
int file_v1_readchar(int fd);
int file_v1_writechar(int fd, unsigned int ch);
int file_v1_read_words(int fd, kword_t *buf, unsigned int nwords);
int file_v1_write_words(int fd, const kword_t *buf, unsigned int nwords,
    kword_t size_chars);
int file_v1_readdir(int fd, struct vfs_v1_dirent *ent);
int file_v1_stat_path(const kword_t *path, struct vfs_v1_stat *st);
int file_v1_mkdir(const kword_t *path, unsigned int mode);
int file_v1_unlink(const kword_t *path);
int file_v1_truncate(const kword_t *path, kword_t chars);
int file_v1_rename(const kword_t *oldpath, const kword_t *newpath);
int file_v1_chdir(const kword_t *path);
int file_v1_getcwd(kword_t *buf, unsigned int nwords);
unsigned int file_v1_used_slots(void);

#endif
