#ifndef DAIMOS_USER_U_H
#define DAIMOS_USER_U_H

#include "dsys.h"

#define U_PATH_WORDS 18U
#define U_MAX_ARGS 16U
#define U_ARG_WORDS 18U

struct u_io {
        int in_fd;
        int out_fd;
        int err_fd;
};

int u_putc(int fd, int ch);
int u_puts(int fd, const char *s);
int u_put_s6(int fd, const kword_t *s);
int u_put_uint(int fd, kword_t v);
int u_put_octal(int fd, kword_t v, unsigned int digits);
int u_crlf(int fd);
int u_text_sink_attach(int fd);
int u_text_sink_flush(void);
int u_text_sink_detach(void);
int u_s6_eq(const kword_t *s, const char *text);
int u_s6_pack(kword_t *dst, unsigned int words, const char *src);
int u_s6_from_dirent(kword_t *dst, unsigned int words,
    const struct vfs_dirent *ent);

#endif
