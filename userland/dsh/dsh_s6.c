#include "dsh.h"

void
dsh_s6_clear(struct dsh_s6 *s)
{
        unsigned int i;

        s->len = 0U;
        for (i = 0U; i < DSH_S6_MAX_WORDS; ++i)
                s->words[i] = 0;
}

void
dsh_line_clear(struct dsh_line *line)
{
        unsigned int i;

        if (line == 0)
                return;
        line->len = 0U;
        for (i = 0U; i < DSH_LINE_MAX_WORDS; ++i)
                line->words[i] = 0;
}

int
dsh_s6_append(struct dsh_s6 *s, int ch)
{
        unsigned int wi;
        unsigned int sh;

        if (s == 0 || ch < 040 || ch > 0137 ||
            s->len >= DSH_S6_MAX_CHARS)
                return -1;
        wi = s->len / 6U;
        sh = 30U - (s->len % 6U) * 6U;
        s->words[wi] |= ((kword_t)(ch - 040) & 077UL) << sh;
        ++s->len;
        return 0;
}

int
dsh_s6_get(const struct dsh_s6 *s, unsigned int pos)
{
        unsigned int wi;
        unsigned int sh;

        if (s == 0 || pos >= s->len)
                return -1;
        wi = pos / 6U;
        sh = 30U - (pos % 6U) * 6U;
        return (int)(((s->words[wi] >> sh) & 077UL) + 040U);
}

int
dsh_s6_copy(struct dsh_s6 *dst, const struct dsh_s6 *src)
{
        unsigned int i;

        if (dst == 0 || src == 0)
                return -1;
        dst->len = src->len;
        for (i = 0U; i < DSH_S6_MAX_WORDS; ++i)
                dst->words[i] = src->words[i];
        return 0;
}

int
dsh_s6_eq_text(const struct dsh_s6 *s, const char *text)
{
        unsigned int i;
        int ch;
        int want;

        if (s == 0 || text == 0)
                return 0;
        for (i = 0U; i < s->len; ++i) {
                ch = (unsigned char)text[i];
                if (ch == 0)
                        return 0;
                want = ch;
                if (want >= 'a' && want <= 'z')
                        want -= 'a' - 'A';
                if (dsh_s6_get(s, i) != want)
                        return 0;
        }
        return text[s->len] == 0;
}

int
dsh_s6_eq_packed(const struct dsh_s6 *s, unsigned int len,
    kword_t word0, kword_t word1)
{
        if (s == 0 || s->len != len)
                return 0;
        if (s->words[0] != word0)
                return 0;
        if (len > DSH_S6_CHARS_PER_WORD && s->words[1] != word1)
                return 0;
        return 1;
}

int
dsh_s6_pack(const struct dsh_s6 *s, kword_t *dst, unsigned int words)
{
        unsigned int i;
        unsigned int used;

        if (s == 0 || dst == 0)
                return -1;
        used = (s->len + 5U) / 6U;
        if (words < used + 1U)
                return -1;
        for (i = 0U; i < words; ++i)
                dst[i] = 0;
        dst[0] = (kword_t)s->len;
        for (i = 0U; i < used; ++i)
                dst[i + 1U] = s->words[i];
        return 0;
}

int
dsh_s6_from_counted(struct dsh_s6 *dst, const kword_t *src)
{
        unsigned int i;
        unsigned int n;

        if (dst == 0 || src == 0)
                return -1;
        n = (unsigned int)(src[0] & 0777777UL);
        if (n > DSH_S6_MAX_CHARS)
                return -1;
        dsh_s6_clear(dst);
        dst->len = n;
        for (i = 0U; i < (n + 5U) / 6U; ++i)
                dst->words[i] = src[i + 1U];
        return 0;
}

int
dsh_s6_put(int fd, const struct dsh_s6 *s)
{
        kword_t packed[DSH_S6_MAX_WORDS + 1U];

        if (dsh_s6_pack(s, packed, DSH_S6_MAX_WORDS + 1U) != 0)
                return -1;
        return u_put_s6(fd, packed);
}

int
dsh_line_get(const struct dsh_line *line, unsigned int pos)
{
        unsigned int wi;
        unsigned int sh;

        if (line == 0 || pos >= line->len)
                return 0;
        wi = pos / DSH_S6_CHARS_PER_WORD;
        sh = 30U - (pos % DSH_S6_CHARS_PER_WORD) * 6U;
        return (int)(((line->words[wi] >> sh) & 077UL) + 040U);
}
