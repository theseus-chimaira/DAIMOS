#ifndef DAIMOS_USER_U_V1_H
#define DAIMOS_USER_U_V1_H

#include "dsys_v1.h"

#define U_V1_PATH_WORDS 18U
#define U_V1_MAX_ARGS 16U
#define U_V1_ARG_WORDS 18U

struct u_v1_io {
        int in_fd;
        int out_fd;
        int err_fd;
};

int u_v1_putc(int fd, int ch);
int u_v1_puts(int fd, const char *s);
int u_v1_put_s6(int fd, const kword_t *s);
int u_v1_put_uint(int fd, kword_t v);
int u_v1_put_octal(int fd, kword_t v, unsigned int digits);
int u_v1_crlf(int fd);
int u_v1_s6_eq(const kword_t *s, const char *text);
int u_v1_s6_pack(kword_t *dst, unsigned int words, const char *src);
int u_v1_s6_from_dirent(kword_t *dst, unsigned int words,
    const struct sys_v1_dirent *ent);

#endif
