/* Read /CONFIG/PASSWD S6REC fields; do not authenticate here.
 * An eventual hash verifier belongs in LOGIN, not in this reader. */
#include "passwd_reader.h"
#include "text.h"
#define LOGIN_LINE_MAX 127U
static int
reader_text_eq(const char *a, const char *b)
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
text_copy(char *dst, unsigned int size, const char *src)
{
        unsigned int i;

        if (size == 0U)
                return -1;
        for (i = 0U; src[i] != 0; ++i) {
                if (i + 1U >= size)
                        return -1;
                dst[i] = src[i];
        }
        dst[i] = 0;
        return 0;
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
        if (v > 0777U)
                return -1;
        *vp = v;
        return 0;
}

/* Return 0 for a valid matching account and -1 otherwise. */
static int
login_find_account(const char *name, struct login_account *account)
{
        struct u_text_reader r;
        char line[LOGIN_LINE_MAX + 1U];
        char *f[6];
        int n;
        int result;

        r.fd = -1;
        if (u_text_open(&r, "/CONFIG/PASSWD") != 0)
                return -1;
        result = -1;
        for (;;) {
                n = u_text_getline(&r, line, sizeof(line));
                if (n == U_TEXT_EOF)
                        break;
                if (n < 0) {
                        result = -1;
                        break;
                }
                n = u_text_fields(line, f, 6U);
                if (n == 0)
                        continue;
                if (n != 6 || !reader_text_eq(f[0], name))
                        continue;
                if (parse_uint(f[2], &account->uid) != 0 ||
                    parse_uint(f[3], &account->gid) != 0 ||
                    text_copy(account->password, sizeof(account->password),
                    f[1]) != 0 ||
                    u_s6_pack(account->home, U_PATH_WORDS, f[4]) != 0 ||
                    u_s6_pack(account->shell, U_PATH_WORDS, f[5]) != 0) {
                        result = -1;
                        break;
                }
                result = 0;
                break;
        }
        u_text_close(&r);
        return result;
}
