#include "text.h"

#define SED_LINE_CHARS 256U
#define SED_PATTERN_CHARS 64U

static unsigned int
sed_arg_text(const kword_t *arg, char *dst, unsigned int size)
{
        unsigned int n;
        unsigned int i;
        unsigned int sh;

        n = (unsigned int)(arg[0] & 0777777UL);
        if (n + 1U > size) return 0U;
        for (i = 0U; i < n; ++i) {
                sh = 30U - (i % 6U) * 6U;
                dst[i] = (char)(((arg[1U + i / 6U] >> sh) & 077UL) + 040UL);
        }
        dst[n] = 0;
        return n;
}

static int
sed_find(const char *line, const char *pat)
{
        unsigned int i;
        unsigned int j;

        for (i = 0U; line[i] != 0; ++i) {
                for (j = 0U; pat[j] != 0 && line[i + j] == pat[j]; ++j)
                        ;
                if (pat[j] == 0) return (int)i;
        }
        return -1;
}

static int
sed_stream(struct u_text_reader *r, const char *old, const char *newtext,
    int print_only)
{
        char line[SED_LINE_CHARS];
        unsigned int oldn;
        unsigned int i;
        int pos;
        int rc;

        for (oldn = 0U; old[oldn] != 0; ++oldn) ;
        while ((rc = u_text_getline(r, line, sizeof(line))) >= 0) {
                pos = sed_find(line, old);
                if (pos < 0) {
                        if (!print_only &&
                            (u_puts(1, line) != 0 || u_crlf(1) != 0)) return 1;
                        continue;
                }
                for (i = 0U; i < (unsigned int)pos; ++i)
                        if (u_putc(1, line[i]) != 0) return 1;
                if (u_puts(1, newtext) != 0) return 1;
                if (u_puts(1, line + (unsigned int)pos + oldn) != 0 ||
                    u_crlf(1) != 0) return 1;
        }
        return rc == U_TEXT_EOF ? 0 : 1;
}

int
main(int argc, kword_t **argv, kword_t **envp)
{
        struct u_text_reader r;
        char expr[SED_PATTERN_CHARS * 2U + 8U];
        char old[SED_PATTERN_CHARS];
        char newtext[SED_PATTERN_CHARS];
        char path[U_PATH_WORDS * 6U];
        unsigned int n;
        unsigned int i;
        unsigned int o;
        unsigned int q;
        int print_only;
        int argi;
        int rc;
        int status;

        (void)envp;
        argi = 1;
        print_only = 0;
        if (argi < argc && u_s6_eq(argv[argi], "-N")) {
                print_only = 1; ++argi;
        }
        if (argi >= argc) return 2;
        n = sed_arg_text(argv[argi++], expr, sizeof(expr));
        if (n < 4U || expr[0] != 'S') return 2;
        o = 0U;
        for (i = 2U; i < n && expr[i] != expr[1]; ++i)
                if (o + 1U < sizeof(old)) old[o++] = expr[i]; else return 2;
        if (i >= n || o == 0U) return 2;
        old[o] = 0; ++i;
        q = 0U;
        while (i < n && expr[i] != expr[1]) {
                if (q + 1U >= sizeof(newtext)) return 2;
                newtext[q++] = expr[i++];
        }
        newtext[q] = 0;
        if (i < n) ++i;
        if (i < n && expr[i] == 'P') print_only = 1;
        status = 0;
        if (argi == argc) {
                if (u_text_open_fd(&r, 0) != 0) return 1;
                status = sed_stream(&r, old, newtext, print_only);
                u_text_close(&r);
                return status;
        }
        while (argi < argc) {
                if (sed_arg_text(argv[argi++], path, sizeof(path)) == 0U ||
                    u_text_open(&r, path) != 0) { status = 1; continue; }
                rc = sed_stream(&r, old, newtext, print_only);
                u_text_close(&r);
                if (rc != 0) status = 1;
        }
        return status;
}
