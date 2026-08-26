#include "u_v1.h"

int
u_v1_putc(int fd, int ch)
{
        return dsys_v1_writechar(fd, ch) == 0 ? 0 : 1;
}

int
u_v1_puts(int fd, const char *s)
{
        unsigned int i;
        if (s == 0) return 1;
        for (i = 0U; s[i] != 0; ++i)
                if (u_v1_putc(fd, (unsigned char)s[i]) != 0) return 1;
        return 0;
}

int
u_v1_crlf(int fd)
{
        return u_v1_putc(fd, '\r') != 0 || u_v1_putc(fd, '\n') != 0;
}

int
u_v1_put_s6(int fd, const kword_t *s)
{
        unsigned int i;
        unsigned int n;
        unsigned int wi;
        unsigned int sh;
        int ch;
        if (s == 0) return 1;
        n = (unsigned int)(s[0] & 0777777UL);
        for (i = 0U; i < n; ++i) {
                wi = 1U + i / 6U;
                sh = 30U - (i % 6U) * 6U;
                ch = (int)(((s[wi] >> sh) & 077UL) + 040UL);
                if (u_v1_putc(fd, ch) != 0) return 1;
        }
        return 0;
}

int
u_v1_put_uint(int fd, kword_t v)
{
        char b[16];
        unsigned int n;
        n = 0U;
        if (v == 0) return u_v1_putc(fd, '0');
        while (v != 0 && n < sizeof(b)) { b[n++] = (char)('0' + (v % 10UL)); v /= 10UL; }
        while (n != 0U) if (u_v1_putc(fd, b[--n]) != 0) return 1;
        return 0;
}

int
u_v1_put_octal(int fd, kword_t v, unsigned int digits)
{
        unsigned int sh;
        if (digits == 0U || digits > 12U) return 1;
        while (digits != 0U) {
                sh = (digits - 1U) * 3U;
                if (u_v1_putc(fd, '0' + (int)((v >> sh) & 07UL)) != 0) return 1;
                --digits;
        }
        return 0;
}

int
u_v1_s6_eq(const kword_t *s, const char *text)
{
        unsigned int i;
        unsigned int n;
        unsigned int wi;
        unsigned int sh;
        if (s == 0 || text == 0) return 0;
        n = (unsigned int)(s[0] & 0777777UL);
        for (i = 0U; text[i] != 0; ++i) {
                if (i >= n) return 0;
                wi = 1U + i / 6U;
                sh = 30U - (i % 6U) * 6U;
                if ((unsigned int)text[i] != (unsigned int)(((s[wi] >> sh) & 077UL) + 040UL)) return 0;
        }
        return i == n;
}

int
u_v1_s6_pack(kword_t *dst, unsigned int words, const char *src)
{
        unsigned int i;
        unsigned int wi;
        unsigned int sh;
        if (dst == 0 || src == 0 || words < 2U) return -1;
        for (i = 0U; i < words; ++i) dst[i] = 0;
        for (i = 0U; src[i] != 0; ++i) {
                wi = 1U + i / 6U;
                if (wi >= words) return -1;
                sh = 30U - (i % 6U) * 6U;
                dst[wi] |= ((kword_t)(((unsigned int)src[i] - 040U) & 077U)) << sh;
        }
        dst[0] = i;
        return 0;
}

int
u_v1_s6_from_dirent(kword_t *dst, unsigned int words,
    const struct vfs_v1_dirent *ent)
{
        unsigned int i;
        if (dst == 0 || ent == 0 || words < VFS_V1_NAME_WORDS + 1U) return -1;
        for (i = 0U; i < words; ++i) dst[i] = 0;
        dst[0] = ent->name.chars;
        for (i = 0U; i < VFS_V1_NAME_WORDS; ++i) dst[i + 1U] = ent->name.words[i];
        return 0;
}
