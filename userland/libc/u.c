#include "u.h"

#define U_TEXT_SINK_DATA_WORDS 43U
#define U_TEXT_SINK_MAX_CHARS  (U_TEXT_SINK_DATA_WORDS * 6U)
#define U_S6REC_TEXT_HEADER     (1UL << 30)

static int u_text_sink_fd = -1;
static unsigned int u_text_sink_chars;
static kword_t u_text_sink_record[U_TEXT_SINK_DATA_WORDS + 1U];

int
u_write_words_all(int fd, const kword_t *words, unsigned int nwords)
{
        unsigned int done;
        int rc;

        if (fd < 0 || (words == 0 && nwords != 0U))
                return -1;
        if (nwords == 0U)
                return 0;
        rc = dsys_write_words(fd, (kword_t *)words, nwords);
        if (rc == (int)nwords)
                return 0;
        if (rc <= 0 || (unsigned int)rc > nwords)
                return -1;
        done = (unsigned int)rc;
        while (done < nwords) {
                rc = dsys_write_words(fd, (kword_t *)&words[done],
                    nwords - done);
                if (rc <= 0 || (unsigned int)rc > nwords - done)
                        return -1;
                done += (unsigned int)rc;
        }
        return 0;
}

static int
u_text_sink_emit(int empty)
{
        unsigned int words;
        int rc;

        if (u_text_sink_fd < 0)
                return 1;
        if (u_text_sink_chars == 0U && !empty)
                return 0;
        u_text_sink_record[0] = U_S6REC_TEXT_HEADER |
            (kword_t)u_text_sink_chars;
        words = 1U + (u_text_sink_chars + 5U) / 6U;
        rc = u_write_words_all(u_text_sink_fd, u_text_sink_record, words);
        if (rc != 0)
                return 1;
        u_text_sink_chars = 0U;
        for (words = 1U; words <= U_TEXT_SINK_DATA_WORDS; ++words)
                u_text_sink_record[words] = 0UL;
        return 0;
}

int
u_text_sink_attach(int fd)
{
        unsigned int i;

        if (fd < 0 || u_text_sink_fd >= 0)
                return -1;
        u_text_sink_fd = fd;
        u_text_sink_chars = 0U;
        for (i = 0U; i <= U_TEXT_SINK_DATA_WORDS; ++i)
                u_text_sink_record[i] = 0UL;
        return 0;
}

int
u_text_sink_flush(void)
{
        return u_text_sink_emit(0);
}

int
u_text_sink_detach(void)
{
        int rc;

        if (u_text_sink_fd < 0)
                return 0;
        rc = u_text_sink_emit(0);
        u_text_sink_fd = -1;
        return rc == 0 ? 0 : -1;
}

int
u_putc(int fd, int ch)
{
        unsigned int slot;
        unsigned int shift;

        if (fd != u_text_sink_fd)
                return dsys_writechar(fd, ch) == 0 ? 0 : 1;
        if (ch == '\r')
                return 0;
        if (ch == '\n')
                return u_text_sink_emit(1);
        if (ch < 040 || ch > 0137 ||
            u_text_sink_chars >= U_TEXT_SINK_MAX_CHARS)
                return 1;
        slot = u_text_sink_chars++;
        shift = 30U - (slot % 6U) * 6U;
        u_text_sink_record[1U + slot / 6U] |=
            ((kword_t)((unsigned int)ch - 040U)) << shift;
        return 0;
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

int
u_put_s6(int fd, const kword_t *s)
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
                if (u_putc(fd, ch) != 0) return 1;
        }
        return 0;
}

int
u_put_uint(int fd, kword_t v)
{
        char b[16];
        unsigned int n;
        n = 0U;
        if (v == 0) return u_putc(fd, '0');
        while (v != 0 && n < sizeof(b)) { b[n++] = (char)('0' + (v % 10UL)); v /= 10UL; }
        while (n != 0U) if (u_putc(fd, b[--n]) != 0) return 1;
        return 0;
}

int
u_put_octal(int fd, kword_t v, unsigned int digits)
{
        unsigned int sh;
        if (digits == 0U || digits > 12U) return 1;
        while (digits != 0U) {
                sh = (digits - 1U) * 3U;
                if (u_putc(fd, '0' + (int)((v >> sh) & 07UL)) != 0) return 1;
                --digits;
        }
        return 0;
}

int
u_s6_eq(const kword_t *s, const char *text)
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
u_s6_pack(kword_t *dst, unsigned int words, const char *src)
{
        unsigned int i;
        unsigned int wi;
        unsigned int sh;
        unsigned int ch;
        if (dst == 0 || src == 0 || words < 2U) return -1;
        for (i = 0U; i < words; ++i) dst[i] = 0;
        for (i = 0U; src[i] != 0; ++i) {
                wi = 1U + i / 6U;
                if (wi >= words) return -1;
                sh = 30U - (i % 6U) * 6U;
                ch = (unsigned int)(unsigned char)src[i];
                if (ch >= (unsigned int)'a' && ch <= (unsigned int)'z')
                        ch -= (unsigned int)('a' - 'A');
                if (ch < 040U || ch > 0137U)
                        return -1;
                dst[wi] |= ((kword_t)(ch - 040U)) << sh;
        }
        dst[0] = i;
        return 0;
}

int
u_s6_from_dirent(kword_t *dst, unsigned int words,
    const struct vfs_dirent *ent)
{
        unsigned int i;
        if (dst == 0 || ent == 0 || words < VFS_NAME_WORDS + 1U) return -1;
        for (i = 0U; i < words; ++i) dst[i] = 0;
        dst[0] = ent->name.chars;
        for (i = 0U; i < VFS_NAME_WORDS; ++i) dst[i + 1U] = ent->name.words[i];
        return 0;
}
