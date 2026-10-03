#include "dsys.h"
#include "text.h"
#include "u.h"

#define COMP_CHARS 36U
#define COMP_WORDS 7U

static unsigned int
s6_len(const kword_t *s)
{
        return s == 0 ? 0U : (unsigned int)(s[0] & 0777777UL);
}

static int
s6_get(const kword_t *s, unsigned int pos)
{
        unsigned int sh;

        if (s == 0 || pos >= s6_len(s))
                return 0;
        sh = 30U - (pos % 6U) * 6U;
        return (int)(((s[1U + pos / 6U] >> sh) & 077UL) + 040U);
}

static int
prefix_text(const kword_t *prefix, const char *text)
{
        unsigned int i;
        unsigned int n;

        n = s6_len(prefix);
        for (i = 0U; i < n; ++i)
                if (text[i] == 0 || s6_get(prefix, i) != (unsigned char)text[i])
                        return 0;
        return 1;
}

static int
write_match(const char *text)
{
        kword_t rec[COMP_WORDS];
        unsigned int words;

        if (u_s6_pack(rec, COMP_WORDS, text) != 0)
                return 1;
        words = 1U + ((unsigned int)rec[0] + 5U) / 6U;
        return u_write_words_all(1, rec, words) != 0;
}

static int
complete_command(const kword_t *prefix)
{
        struct u_text_reader r;
        char line[COMP_CHARS + 1U];
        char match[COMP_CHARS + 1U];
        unsigned int i;
        int found;
        int rc;

        if (u_text_open(&r, "/SYSTEM/MANUAL/COMMANDS") != 0)
                return 1;
        found = 0;
        while ((rc = u_text_getline(&r, line, sizeof(line))) >= 0) {
                if (!prefix_text(prefix, line))
                        continue;
                if (found) {
                        u_text_close(&r);
                        return 1;
                }
                for (i = 0U; line[i] != 0; ++i)
                        match[i] = line[i];
                match[i] = 0;
                found = 1;
        }
        u_text_close(&r);
        if (rc == U_TEXT_ERROR || !found)
                return 1;
        return write_match(match);
}

static int
complete_path(const kword_t *word)
{
        struct vfs_dirent ent;
        kword_t dirrec[COMP_WORDS];
        kword_t namerec[COMP_WORDS];
        char dir[COMP_CHARS + 1U];
        char candidate[COMP_CHARS + 1U];
        char match[COMP_CHARS + 1U];
        unsigned int len;
        unsigned int slash;
        unsigned int i;
        unsigned int nlen;
        int found;
        int fd;
        int rc;

        len = s6_len(word);
        slash = len;
        while (slash != 0U && s6_get(word, slash - 1U) != '/')
                --slash;
        if (slash == 0U) {
                dir[0] = '.';
                dir[1] = 0;
        } else if (slash == 1U) {
                dir[0] = '/';
                dir[1] = 0;
        } else {
                for (i = 0U; i + 1U < slash; ++i)
                        dir[i] = (char)s6_get(word, i);
                dir[slash - 1U] = 0;
        }
        if (u_s6_pack(dirrec, COMP_WORDS, dir) != 0)
                return 1;
        fd = dsys_open(dirrec, SYS_O_RDONLY);
        if (fd < 0)
                return 1;
        found = 0;
        while ((rc = dsys_dirread(fd, &ent)) > 0) {
                if (u_s6_from_dirent(namerec, COMP_WORDS, &ent) != 0)
                        continue;
                nlen = s6_len(namerec);
                if (slash + nlen > COMP_CHARS)
                        continue;
                for (i = 0U; i < slash; ++i)
                        candidate[i] = (char)s6_get(word, i);
                for (i = 0U; i < nlen; ++i)
                        candidate[slash + i] = (char)s6_get(namerec, i);
                candidate[slash + nlen] = 0;
                if (!prefix_text(word, candidate))
                        continue;
                if (found) {
                        (void)dsys_close(fd);
                        return 1;
                }
                for (i = 0U; candidate[i] != 0; ++i)
                        match[i] = candidate[i];
                match[i] = 0;
                found = 1;
        }
        (void)dsys_close(fd);
        if (rc < 0 || !found)
                return 1;
        return write_match(match);
}

int
main(int argc, kword_t **argv)
{
        if (argc != 3 || argv == 0 || s6_len(argv[1]) != 1U)
                return 2;
        if (s6_get(argv[1], 0U) == 'C')
                return complete_command(argv[2]);
        if (s6_get(argv[1], 0U) == 'P')
                return complete_path(argv[2]);
        return 2;
}
