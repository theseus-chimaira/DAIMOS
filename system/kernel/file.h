#ifndef DAIMON_FILE_H
#define DAIMON_FILE_H

#include "vfs.h"

#define FILE_O_READ          0001U
#define FILE_O_WRITE         0002U
#define FILE_O_APPEND        0004U
#define FILE_O_CREAT         0010U
#define FILE_O_TRUNC         0020U
#define FILE_DEVICE_IO       VFS_DEVICE_IO

#ifndef FILE_NFILE
#define FILE_NFILE           13U
#endif
#define FILE_FD_FIRST        3U
#define FILE_FD_MAX          15U

struct file {
        vnode_t node;
        kword_t off_chars;
        kword_t meta;
};

#define FILE_META_DIR        0100U
#define FILE_META_DESC_SHIFT 7U
#define FILE_META_FLAGS(m)   ((m) & 077U)
#define FILE_META_DESC(m)    (((m) >> FILE_META_DESC_SHIFT) & 077U)

/* DAIMOS 1.x has one live user process, so FILE state is global. */
int file_lookup_path(const kword_t *path, vnode_t *nodep);
int file_open(const kword_t *path, unsigned int flags);
int file_close(int fd);
int file_dup(int fd);
int file_lock(int fd, unsigned int op);
void file_close_all(void);
int file_readchar(int fd);
int file_writechar(int fd, unsigned int ch);
int file_read_words(int fd, kword_t *buf, unsigned int nwords);
int file_write_words(int fd, const kword_t *buf, unsigned int nwords,
    kword_t size_chars);
int file_readdir(int fd, struct vfs_dirent *ent);
int file_stat_path(const kword_t *path, struct vfs_stat *st);
int file_mkdir(const kword_t *path, unsigned int mode);
int file_symlink(const kword_t *target, const kword_t *linkpath);
int file_unlink(const kword_t *path);
int file_truncate(const kword_t *path, kword_t chars);
int file_rename(const kword_t *oldpath, const kword_t *newpath);
int file_chdir(const kword_t *path);
int file_getcwd(kword_t *buf, unsigned int nwords);
unsigned int file_used_slots(void);

#endif
