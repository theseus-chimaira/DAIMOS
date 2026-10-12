/* Console-only character writer for programs with no S6REC sink. */
#include "u.h"

int
u_putc(int fd, int ch)
{
        return dsys_writechar(fd, ch) == 0 ? 0 : 1;
}

int
u_puts(int fd, const char *s)
{
        unsigned int i;
        if (s == 0) return 1;
        for (i = 0U; s[i] != 0; ++i)
                if (u_putc(fd, (unsigned char)s[i]) != 0) return 1;
        return 0;
}

int
u_crlf(int fd)
{
        return u_putc(fd, '\r') != 0 || u_putc(fd, '\n') != 0;
}
