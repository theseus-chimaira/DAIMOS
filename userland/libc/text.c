#include "text.h"

int
u_text_open(struct u_text_reader *r, const char *path)
{
        kword_t packed[U_PATH_WORDS];
        struct vfs_stat st;
        int fd;

        if (r == 0 || path == 0)
                return -1;
        if (u_s6_pack(packed, U_PATH_WORDS, path) != 0)
                return -1;
        if (dsys_stat(packed, &st) != 0 || st.type != VFS_TYPE_REG)
                return -1;
        fd = dsys_open(packed, SYS_O_RDONLY);
        if (fd < 0)
                return -1;
        r->fd = fd;
        r->chars_left = st.size_chars;
        r->word = 0UL;
        r->pos = 4U;
        return 0;
}

static int
u_text_getc(struct u_text_reader *r)
{
        unsigned int shift;
        int rc;
        int ch;

        if (r->chars_left == 0UL)
                return U_TEXT_EOF;
        if (r->pos >= 4U) {
                rc = dsys_read_words(r->fd, &r->word, 1U);
                if (rc != 1)
                        return U_TEXT_ERROR;
                r->pos = 0U;
        }
        shift = 27U - r->pos * 9U;
        ch = (int)((r->word >> shift) & 0777UL);
        ++r->pos;
        --r->chars_left;
        return ch;
}

int
u_text_getline(struct u_text_reader *r, char *buf, unsigned int size)
{
        unsigned int n;
        int ch;

        if (r == 0 || buf == 0 || size < 2U)
                return U_TEXT_ERROR;
        n = 0U;
        for (;;) {
                ch = u_text_getc(r);
                if (ch == U_TEXT_EOF) {
                        if (n == 0U)
                                return U_TEXT_EOF;
                        buf[n] = 0;
                        return (int)n;
                }
                if (ch < 0)
                        return U_TEXT_ERROR;
                if (ch == '\n') {
                        buf[n] = 0;
                        return (int)n;
                }
                if (ch == '\r')
                        continue;
                if (ch > 0377 || n + 1U >= size)
                        return U_TEXT_ERROR;
                buf[n++] = (char)ch;
        }
}

void
u_text_close(struct u_text_reader *r)
{
        if (r != 0 && r->fd >= 0) {
                (void)dsys_close(r->fd);
                r->fd = -1;
        }
}
