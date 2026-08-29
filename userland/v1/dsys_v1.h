#ifndef DAIMOS_USER_DSYS_V1_H
#define DAIMOS_USER_DSYS_V1_H

#include "syscall_v1.h"

extern int __syscall();

#define DSYS_V1_CALL0(n) __syscall((n))
#define DSYS_V1_CALL1(n,a) __syscall((n),(a))
#define DSYS_V1_CALL2(n,a,b) __syscall((n),(a),(b))
#define DSYS_V1_CALL3(n,a,b,c) __syscall((n),(a),(b),(c))
#define DSYS_V1_CALL4(n,a,b,c,d) __syscall((n),(a),(b),(c),(d))

static int dsys_v1_open(kword_t *p, unsigned int f) { return DSYS_V1_CALL2(SYS_V1_OPEN,p,f); }
static int dsys_v1_close(int fd) { return DSYS_V1_CALL1(SYS_V1_CLOSE,fd); }
static int dsys_v1_readchar(int fd) { return DSYS_V1_CALL1(SYS_V1_READCHAR,fd); }
static int dsys_v1_writechar(int fd, int ch) { return DSYS_V1_CALL2(SYS_V1_WRITECHAR,fd,ch); }
static int dsys_v1_read_words(int fd, kword_t *b, unsigned int n) { return DSYS_V1_CALL3(SYS_V1_READ_WORDS,fd,b,n); }
static int dsys_v1_write_words(int fd, kword_t *b, unsigned int n, kword_t c) { return DSYS_V1_CALL4(SYS_V1_WRITE_WORDS,fd,b,n,c); }
static int dsys_v1_stat(kword_t *p, struct vfs_v1_stat *s) { return DSYS_V1_CALL2(SYS_V1_STAT,p,s); }
static int dsys_v1_dirread(int fd, struct vfs_v1_dirent *e) { return DSYS_V1_CALL2(SYS_V1_DIRREAD,fd,e); }
static int dsys_v1_mkdir(kword_t *p) { return DSYS_V1_CALL2(SYS_V1_MKDIR,p,0777U); }
static int dsys_v1_unlink(kword_t *p) { return DSYS_V1_CALL1(SYS_V1_UNLINK,p); }
static int dsys_v1_rename(kword_t *a, kword_t *b) { return DSYS_V1_CALL2(SYS_V1_RENAME,a,b); }
static int dsys_v1_truncate(kword_t *p, kword_t n) { return DSYS_V1_CALL2(SYS_V1_TRUNCATE,p,n); }
static int dsys_v1_chmod(kword_t *p, unsigned int m) { return DSYS_V1_CALL2(SYS_V1_CHMOD,p,m); }
static int dsys_v1_dtfs_format(kword_t *p) { return DSYS_V1_CALL1(SYS_V1_DTFS_FORMAT,p); }
static int dsys_v1_dtfs_mount(kword_t *d, kword_t *p, unsigned int f) { return DSYS_V1_CALL3(SYS_V1_DTFS_MOUNT,d,p,f); }
static int dsys_v1_unmount(kword_t *p) { return DSYS_V1_CALL1(SYS_V1_UNMOUNT,p); }
static int dsys_v1_chdir(kword_t *p) { return DSYS_V1_CALL1(SYS_V1_CHDIR,p); }
static int dsys_v1_getcwd(kword_t *p, unsigned int n) { return DSYS_V1_CALL2(SYS_V1_GETCWD,p,n); }
static int dsys_v1_procinfo(unsigned int s, struct sys_v1_procinfo *p) { return DSYS_V1_CALL2(SYS_V1_PROCINFO,s,p); }
static int dsys_v1_meminfo(struct sys_v1_meminfo *p) { return DSYS_V1_CALL1(SYS_V1_MEMINFO,p); }
static int dsys_v1_exit(int rc) { return DSYS_V1_CALL1(SYS_V1_EXIT,rc); }
static int dsys_v1_halt(void) { return DSYS_V1_CALL0(SYS_V1_HALT); }

#endif
