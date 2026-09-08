#ifndef DAIMOS_USER_DSYS_H
#define DAIMOS_USER_DSYS_H

#include "syscall.h"

extern int __syscall();

#define DSYS_CALL0(n) __syscall((n))
#define DSYS_CALL1(n,a) __syscall((n),(a))
#define DSYS_CALL2(n,a,b) __syscall((n),(a),(b))
#define DSYS_CALL3(n,a,b,c) __syscall((n),(a),(b),(c))
#define DSYS_CALL4(n,a,b,c,d) __syscall((n),(a),(b),(c),(d))

static int dsys_open(kword_t *p, unsigned int f) { return DSYS_CALL2(SYS_OPEN,p,f); }
static int dsys_close(int fd) { return DSYS_CALL1(SYS_CLOSE,fd); }
static int dsys_readchar(int fd) { return DSYS_CALL1(SYS_READCHAR,fd); }
static int dsys_writechar(int fd, int ch) { return DSYS_CALL2(SYS_WRITECHAR,fd,ch); }
static int dsys_read_words(int fd, kword_t *b, unsigned int n) { return DSYS_CALL3(SYS_READ_WORDS,fd,b,n); }
static int dsys_write_words(int fd, kword_t *b, unsigned int n, kword_t c) { return DSYS_CALL4(SYS_WRITE_WORDS,fd,b,n,c); }
static int dsys_stat(kword_t *p, struct vfs_stat *s) { return DSYS_CALL2(SYS_STAT,p,s); }
static int dsys_dirread(int fd, struct vfs_dirent *e) { return DSYS_CALL2(SYS_DIRREAD,fd,e); }
static int dsys_mkdir(kword_t *p) { return DSYS_CALL2(SYS_MKDIR,p,0777U); }
static int dsys_unlink(kword_t *p) { return DSYS_CALL1(SYS_UNLINK,p); }
static int dsys_rename(kword_t *a, kword_t *b) { return DSYS_CALL2(SYS_RENAME,a,b); }
static int dsys_truncate(kword_t *p, kword_t n) { return DSYS_CALL2(SYS_TRUNCATE,p,n); }
static int dsys_chmod(kword_t *p, unsigned int m) { return DSYS_CALL2(SYS_CHMOD,p,m); }
static int dsys_dtfs_format(kword_t *p, unsigned int t) { return DSYS_CALL2(SYS_DTFS_FORMAT,p,SYS_DTFS_CTL_FORMAT|t); }
static int dsys_dtfs_check(kword_t *p, unsigned int t) { return DSYS_CALL2(SYS_DTFS_FORMAT,p,SYS_DTFS_CTL_CHECK|t); }
static int dsys_dtfs_mount(kword_t *d, kword_t *p, unsigned int f) { return DSYS_CALL3(SYS_DTFS_MOUNT,d,p,f); }
static int dsys_unmount(kword_t *p) { return DSYS_CALL1(SYS_UNMOUNT,p); }
static int dsys_flock(int fd, unsigned int op) { return DSYS_CALL2(SYS_FLOCK,fd,op); }
static int dsys_dup(int fd) { return DSYS_CALL1(SYS_DUP,fd); }
static int dsys_symlink(kword_t *t, kword_t *p) { return DSYS_CALL2(SYS_SYMLINK,t,p); }
static int dsys_nice(int n) { return DSYS_CALL1(SYS_NICE,n); }
static int dsys_chdir(kword_t *p) { return DSYS_CALL1(SYS_CHDIR,p); }
static int dsys_getcwd(kword_t *p, unsigned int n) { return DSYS_CALL2(SYS_GETCWD,p,n); }
static int dsys_procinfo(unsigned int s, struct sys_procinfo *p) { return DSYS_CALL2(SYS_PROCINFO,s,p); }
static int dsys_meminfo(struct sys_meminfo *p) { return DSYS_CALL1(SYS_MEMINFO,p); }
static int dsys_exit(int rc) { return DSYS_CALL1(SYS_EXIT,rc); }
static int dsys_halt(void) { return DSYS_CALL0(SYS_HALT); }

#endif
