#include "text.h"

#define MOUNTALL_LINE_MAX 127U

static int
text_eq(const char *a, const char *b)
{
        unsigned int i;

        i = 0U;
        while (a[i] != 0 && b[i] != 0) {
                if (a[i] != b[i])
                        return 0;
                ++i;
        }
        return a[i] == b[i];
}

static int
parse_uint(const char *s, unsigned int *vp)
{
        unsigned int v;
        unsigned int i;

        if (s == 0 || s[0] == 0)
                return -1;
        v = 0U;
        for (i = 0U; s[i] != 0; ++i) {
                if (s[i] < '0' || s[i] > '9')
                        return -1;
                if (v > 077777U)
                        return -1;
                v = v * 10U + (unsigned int)(s[i] - '0');
        }
        *vp = v;
        return 0;
}

/*
 * FSTAB V1 contains policy, not device discovery:
 *
 *     MEMFS:/existing/mount/point:words[:PERSIST]
 *
 * Blank lines and comments beginning with '#' are ignored.  MEMFS is a
 * singleton provider, so at most one MEMFS entry can succeed.
 */
static int
mount_line(char *line, int *temp_memfs)
{
        char *target;
        char *words_text;
        char *flags_text;
        kword_t path[U_PATH_WORDS];
        unsigned int words;
        unsigned int flags;
        unsigned int i;

        if (line[0] == 0 || line[0] == '#')
                return 1;
        target = 0;
        words_text = 0;
        flags_text = 0;
        flags = 0U;
        for (i = 0U; line[i] != 0; ++i) {
                if (line[i] != ':')
                        continue;
                line[i] = 0;
                if (target == 0)
                        target = &line[i + 1U];
                else if (words_text == 0)
                        words_text = &line[i + 1U];
                else if (flags_text == 0)
                        flags_text = &line[i + 1U];
                else
                        return -1;
        }
        if (target == 0 || words_text == 0 || !text_eq(line, "MEMFS") ||
            parse_uint(words_text, &words) != 0 ||
            u_s6_pack(path, U_PATH_WORDS, target) != 0)
                return -1;
        if (flags_text != 0) {
                if (!text_eq(flags_text, "PERSIST"))
                        return -1;
                flags = 0002U;
        }
        if (dsys_memfs_mount(path, words, flags) != 0)
                return -1;
        if (text_eq(target, "/TEMP"))
                *temp_memfs = 1;
        return 0;
}

int
main(void)
{
        struct u_text_reader r;
        char line[MOUNTALL_LINE_MAX + 1U];
        int n;
        int failed;
        int temp_memfs;

        failed = 0;
        temp_memfs = 0;
        r.fd = -1;
        if (u_text_open(&r, "/CONFIG/FSTAB") != 0)
                return 2;
        for (;;) {
                n = u_text_getline(&r, line, sizeof(line));
                if (n == U_TEXT_EOF)
                        break;
                if (n < 0 || mount_line(line, &temp_memfs) < 0) {
                        failed = 1;
                        break;
                }
        }
        u_text_close(&r);
        if (failed)
                return 1;
        return temp_memfs ? 0 : 2;
}
