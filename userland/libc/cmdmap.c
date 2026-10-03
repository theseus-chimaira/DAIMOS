#include "cmdmap.h"
#include "text.h"

static int
cmdmap_s6_char(const kword_t *s, unsigned int pos)
{
        unsigned int sh;

        sh = 30U - (pos % 6U) * 6U;
        return (int)(((s[1U + pos / 6U] >> sh) & 077UL) + 040UL);
}

static void
cmdmap_s6_set(kword_t *s, unsigned int pos, int ch)
{
        unsigned int wi;
        unsigned int sh;
        kword_t mask;

        wi = 1U + pos / 6U;
        sh = 30U - (pos % 6U) * 6U;
        mask = (kword_t)077UL << sh;
        s[wi] = (s[wi] & ~mask) |
            (((kword_t)(ch - 040) & 077UL) << sh);
}

static int
cmdmap_command_eq(const kword_t *command, const kword_t *line,
    unsigned int n)
{
        unsigned int len;
        unsigned int start;
        unsigned int i;
        unsigned int shift;
        int ch;

        if (command == 0)
                return 0;
        len = (unsigned int)(command[0] & 0777777UL);
        start = 0U;
        for (i = 0U; i < len; ++i) {
                shift = 30U - (i % 6U) * 6U;
                ch = (int)(((command[1U + i / 6U] >> shift) & 077UL) + 040UL);
                if (ch == '/')
                        start = i + 1U;
        }
        if (len - start != n)
                return 0;
        for (i = 0U; i < n; ++i) {
                unsigned int pos;

                pos = start + i;
                shift = 30U - (pos % 6U) * 6U;
                ch = (int)(((command[1U + pos / 6U] >> shift) & 077UL) + 040UL);
                if (ch != cmdmap_s6_char(line, i))
                        return 0;
        }
        return 1;
}

static int
cmdmap_make_target(kword_t *line, unsigned int words, unsigned int start)
{
        static const char prefix[] = "/SYSTEM/LIBEXEC/";
        unsigned int line_len;
        unsigned int prefix_len;
        unsigned int target_len;
        unsigned int total;
        unsigned int i;

        line_len = (unsigned int)line[0];
        prefix_len = (unsigned int)(sizeof(prefix) - 1U);
        if (start > line_len)
                return -1;
        target_len = line_len - start;
        total = prefix_len + target_len;
        if (total > (words - 1U) * 6U)
                return -1;
        i = target_len;
        while (i != 0U) {
                --i;
                cmdmap_s6_set(line, prefix_len + i,
                    cmdmap_s6_char(line, start + i));
        }
        for (i = 0U; i < prefix_len; ++i)
                cmdmap_s6_set(line, i, (unsigned char)prefix[i]);
        line[0] = (kword_t)total;
        return 0;
}

int
u_cmd_resolve(const kword_t *command, kword_t *path, unsigned int path_words)
{
        struct u_text_reader r;
        unsigned int len;
        unsigned int split;
        int rc;

        if (command == 0 || path == 0 || path_words == 0U)
                return -1;
        if (u_text_open(&r, "/SYSTEM/LIBEXEC/MAP") != 0)
                return -1;
        while ((rc = u_text_gets6(&r, path, path_words)) >= 0) {
                len = (unsigned int)rc;
                for (split = 0U; split < len &&
                    cmdmap_s6_char(path, split) != ' '; ++split)
                        ;
                if (split == len || !cmdmap_command_eq(command, path, split))
                        continue;
                while (split < len && cmdmap_s6_char(path, split) == ' ')
                        ++split;
                u_text_close(&r);
                return cmdmap_make_target(path, path_words, split);
        }
        u_text_close(&r);
        return -1;
}
