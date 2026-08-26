#ifndef DAIMON_SYSCALL_V1_H
#define DAIMON_SYSCALL_V1_H

#include "file_v1.h"

#define SYS_V1_WRITE            1U
#define SYS_V1_EXIT             2U
#define SYS_V1_OPEN             3U
#define SYS_V1_READ             4U
#define SYS_V1_CLOSE            5U
#define SYS_V1_STAT             10U
#define SYS_V1_DIRREAD          14U
#define SYS_V1_MKDIR            18U
#define SYS_V1_UNLINK           20U
#define SYS_V1_TRUNCATE         22U
#define SYS_V1_READ_WORDS       31U
#define SYS_V1_WRITE_WORDS      32U

#define SYS_V1_O_RDONLY         000000U
#define SYS_V1_O_WRONLY         000001U
#define SYS_V1_O_RDWR           000002U
#define SYS_V1_O_APPEND         000004U
#define SYS_V1_O_CREAT          000010U
#define SYS_V1_O_TRUNC          000020U

struct sys_v1_stat {
        kword_t type;
        kword_t size_chars;
        kword_t size_words;
        kword_t mode;
};

struct sys_v1_dirent {
        kword_t chars;
        kword_t words[VFS_V1_NAME_WORDS];
        kword_t type;
};

int sys_v1_open(unsigned int owner, const kword_t *path, unsigned int flags);
int sys_v1_close(unsigned int owner, int fd);
int sys_v1_read_words(unsigned int owner, int fd, kword_t *buf,
    unsigned int nwords);
int sys_v1_write_words(unsigned int owner, int fd, const kword_t *buf,
    unsigned int nwords, kword_t size_chars);
int sys_v1_stat_path(const kword_t *path, struct sys_v1_stat *st);
int sys_v1_dirread(unsigned int owner, int fd, struct sys_v1_dirent *ent);
int sys_v1_mkdir(const kword_t *path, unsigned int mode);
int sys_v1_unlink(const kword_t *path);
int sys_v1_truncate(const kword_t *path, kword_t chars);

/* Called by mach_user_v1.s with the saved AC block. */
int exec_native_syscall_v1(kword_t *ac);

#endif
