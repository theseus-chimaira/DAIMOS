#ifndef DAIMOS_USER_DSYS_H
#define DAIMOS_USER_DSYS_H

#include "syscall.h"

/* Shared assembler veneers.  The C ABI supplies real arguments in AC1..AC4;
 * each veneer issues one direct monitor UUO instead of constructing a generic
 * five-argument __syscall call. */
int dsys_open(kword_t *p, unsigned int f);
int dsys_close(int fd);
int dsys_readchar(int fd);
int dsys_writechar(int fd, int ch);
int dsys_write_chars(int fd, const char *b, unsigned int nchars);
int dsys_write_nonets(int fd, const kword_t *b, unsigned int nchars);
int dsys_read_words(int fd, kword_t *b, unsigned int n);
int dsys_write_words(int fd, kword_t *b, unsigned int n, kword_t c);
int dsys_stat(kword_t *p, struct vfs_stat *s);
int dsys_dirread(int fd, struct vfs_dirent *e);
int dsys_mkdir(kword_t *p);
int dsys_unlink(kword_t *p);
int dsys_rename(kword_t *a, kword_t *b);
int dsys_truncate(kword_t *p, kword_t n);
int dsys_chmod(kword_t *p, unsigned int m);
int dsys_dtfs_format(kword_t *p, unsigned int t);
int dsys_dtfs_check(kword_t *p, unsigned int t);
int dsys_dtfs_mount(kword_t *d, kword_t *p, unsigned int f);
int dsys_unmount(kword_t *p);
int dsys_flock(int fd, unsigned int op);
int dsys_dup(int fd);
int dsys_symlink(kword_t *t, kword_t *p);
int dsys_nice(int n);
int dsys_chdir(kword_t *p);
int dsys_getcwd(kword_t *p, unsigned int n);
int dsys_procinfo(unsigned int s, struct sys_procinfo *p);
int dsys_meminfo(struct sys_meminfo *p);
int dsys_exit(int rc);
int dsys_halt(void);

#endif
