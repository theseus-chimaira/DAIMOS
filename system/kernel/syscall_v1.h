#ifndef DAIMON_SYSCALL_V1_H
#define DAIMON_SYSCALL_V1_H

#include "file_v1.h"

#define SYS_V1_WRITE            1U
#define SYS_V1_EXIT             2U
#define SYS_V1_OPEN             3U
#define SYS_V1_READ             4U
#define SYS_V1_CLOSE            5U
#define SYS_V1_PUTCHAR          6U
#define SYS_V1_GETCHAR          7U
#define SYS_V1_CHDIR            8U
#define SYS_V1_GETCWD           9U
#define SYS_V1_STAT             10U
#define SYS_V1_DIRREAD          14U
#define SYS_V1_MKDIR            18U
#define SYS_V1_UNLINK           20U
#define SYS_V1_RENAME           21U
#define SYS_V1_TRUNCATE         22U
#define SYS_V1_READ_WORDS       31U
#define SYS_V1_WRITE_WORDS      32U
#define SYS_V1_PROCINFO         33U
#define SYS_V1_MEMINFO          34U
#define SYS_V1_READCHAR         35U
#define SYS_V1_WRITECHAR        36U
#define SYS_V1_HALT             37U

#define SYS_V1_O_RDONLY         000000U
#define SYS_V1_O_WRONLY         000001U
#define SYS_V1_O_RDWR           000002U
#define SYS_V1_O_APPEND         000004U
#define SYS_V1_O_CREAT          000010U
#define SYS_V1_O_TRUNC          000020U
#define SYS_V1_PROC_SLOTS       64U

struct sys_v1_procinfo {
        kword_t pid;
        kword_t ppid;
        kword_t state;
        kword_t words;
        kword_t comm;
};

struct sys_v1_meminfo {
        kword_t total_words;
        kword_t resident_words;
        kword_t process_words;
        kword_t ramfs_used_words;
        kword_t ramfs_capacity_words;
        kword_t process_slots_used;
        kword_t process_slots_total;
        kword_t file_slots_used;
        kword_t file_slots_total;
};

int sys_v1_open(unsigned int owner, const kword_t *path, unsigned int flags);
int sys_v1_close(unsigned int owner, int fd);
int sys_v1_putchar(int ch);
int sys_v1_getchar(void);
int sys_v1_readchar(unsigned int owner, int fd);
int sys_v1_writechar(unsigned int owner, int fd, int ch);
int sys_v1_chdir(unsigned int owner, const kword_t *path);
int sys_v1_getcwd(unsigned int owner, kword_t *buf, unsigned int nwords);
int sys_v1_read_words(unsigned int owner, int fd, kword_t *buf,
    unsigned int nwords);
int sys_v1_write_words(unsigned int owner, int fd, const kword_t *buf,
    unsigned int nwords, kword_t size_chars);
int sys_v1_stat_path(unsigned int owner, const kword_t *path,
    struct vfs_v1_stat *st);
int sys_v1_dirread(unsigned int owner, int fd, struct vfs_v1_dirent *ent);
int sys_v1_mkdir(unsigned int owner, const kword_t *path, unsigned int mode);
int sys_v1_unlink(unsigned int owner, const kword_t *path);
int sys_v1_rename(unsigned int owner, const kword_t *oldpath,
    const kword_t *newpath);
int sys_v1_truncate(unsigned int owner, const kword_t *path, kword_t chars);
int sys_v1_procinfo(unsigned int slot, struct sys_v1_procinfo *info);
int sys_v1_meminfo(struct sys_v1_meminfo *info);
int sys_v1_halt(void);
void sys_v1_set_memory_bounds(kword_t total_words, kword_t resident_words);

/* Called by mach_user_v1.s with the saved AC block. */
int exec_native_syscall_v1(kword_t *ac);

#endif
