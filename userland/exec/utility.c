#include "utility.h"
#include "text.h"

#define UTIL_LINE_CHARS 256U
#define UTIL_SORT_LINES 32U
#define UTIL_SORT_CHARS 96U
#define UTIL_WORD_BUF   32U
#define UTIL_RUN_WORDS 192U

#ifndef UTIL_GROUP
#define UTIL_GROUP 1
#endif

static unsigned int
util_s6_len(const kword_t *s)
{
        return s == 0 ? 0U : (unsigned int)(s[0] & 0777777UL);
}

static int
util_s6_ch(const kword_t *s, unsigned int pos)
{
        unsigned int sh;

        if (s == 0 || pos >= util_s6_len(s))
                return 0;
        sh = 30U - (pos % 6U) * 6U;
        return (int)(((s[1U + pos / 6U] >> sh) & 077UL) + 040UL);
}

static int
util_name_eq(const kword_t *s, const char *name)
{
        unsigned int n;
        unsigned int start;
        unsigned int i;

        n = util_s6_len(s);
        start = 0U;
        for (i = 0U; i < n; ++i)
                if (util_s6_ch(s, i) == '/')
                        start = i + 1U;
        for (i = 0U; name[i] != 0; ++i)
                if (start + i >= n || util_s6_ch(s, start + i) != name[i])
                        return 0;
        return start + i == n;
}

static int
util_s6_text(const kword_t *s, char *dst, unsigned int size)
{
        unsigned int n;
        unsigned int i;

        n = util_s6_len(s);
        if (size == 0U || n >= size)
                return -1;
        for (i = 0U; i < n; ++i)
                dst[i] = (char)util_s6_ch(s, i);
        dst[n] = 0;
        return 0;
}

static int
util_error(struct u_io *io, const char *name, const kword_t *arg)
{
        if (u_puts(io->err_fd, name) != 0 || u_puts(io->err_fd, ": ") != 0)
                return 1;
        if (arg != 0)
                (void)u_put_s6(io->err_fd, arg);
        (void)u_crlf(io->err_fd);
        return 1;
}

static int
util_open_text_arg(struct u_text_reader *r, const kword_t *arg)
{
        char path[U_PATH_WORDS * 6U];

        if (util_s6_text(arg, path, sizeof(path)) != 0)
                return -1;
        return u_text_open(r, path);
}

static int
util_put_line(int fd, const char *line)
{
        return u_puts(fd, line) != 0 || u_crlf(fd) != 0;
}

static int
util_write_s6rec_line(int fd, const char *line)
{
        kword_t record[44];
        unsigned int chars;
        unsigned int i;
        unsigned int words;

        for (i = 0U; i < 44U; ++i)
                record[i] = 0UL;
        chars = 0U;
        while (line[chars] != 0) {
                unsigned int sh;
                int ch;

                if (chars >= 258U)
                        return -1;
                ch = (unsigned char)line[chars];
                if (ch < 040 || ch > 0137)
                        return -1;
                sh = 30U - (chars % 6U) * 6U;
                record[1U + chars / 6U] |=
                    ((kword_t)((unsigned int)ch - 040U) & 077UL) << sh;
                ++chars;
        }
        record[0] = (1UL << 30) | (kword_t)chars;
        words = 1U + (chars + 5U) / 6U;
        return u_write_words_all(fd, record, words);
}

static int
util_parse_uint(const kword_t *arg, unsigned int *out)
{
        unsigned int n;
        unsigned int i;
        unsigned int v;
        int ch;

        n = util_s6_len(arg);
        if (n == 0U)
                return -1;
        v = 0U;
        for (i = 0U; i < n; ++i) {
                ch = util_s6_ch(arg, i);
                if (ch < '0' || ch > '9')
                        return -1;
                v = v * 10U + (unsigned int)(ch - '0');
        }
        *out = v;
        return 0;
}

#if UTIL_GROUP == 1
static int
util_cmd_check(int argc, kword_t **argv, struct u_io *io)
{
        struct vfs_stat st;

        (void)io;
        if (argc != 2)
                return 2;
        return dsys_stat(argv[1], &st) == 0 ? 0 : 1;
}

static int
util_cmd_basename(int argc, kword_t **argv, struct u_io *io)
{
        unsigned int n;
        unsigned int start;
        unsigned int i;

        if (argc != 2)
                return 2;
        n = util_s6_len(argv[1]);
        start = 0U;
        for (i = 0U; i < n; ++i)
                if (util_s6_ch(argv[1], i) == '/')
                        start = i + 1U;
        for (i = start; i < n; ++i)
                if (u_putc(io->out_fd, util_s6_ch(argv[1], i)) != 0)
                        return 1;
        return u_crlf(io->out_fd) != 0;
}

static int
util_cmd_dirname(int argc, kword_t **argv, struct u_io *io)
{
        unsigned int n;
        unsigned int slash;
        unsigned int i;

        if (argc != 2)
                return 2;
        n = util_s6_len(argv[1]);
        slash = 0U;
        for (i = 0U; i < n; ++i)
                if (util_s6_ch(argv[1], i) == '/')
                        slash = i;
        if (slash == 0U)
                return util_put_line(io->out_fd,
                    n != 0U && util_s6_ch(argv[1], 0U) == '/' ? "/" : ".");
        for (i = 0U; i < slash; ++i)
                if (u_putc(io->out_fd, util_s6_ch(argv[1], i)) != 0)
                        return 1;
        return u_crlf(io->out_fd) != 0;
}

static int
util_cmd_env(int argc, kword_t **argv, kword_t **envp, struct u_io *io)
{
        unsigned int i;

        (void)argv;
        if (argc != 1)
                return 2;
        if (envp == 0)
                return 0;
        for (i = 0U; envp[i] != 0; ++i)
                if (u_put_s6(io->out_fd, envp[i]) != 0 ||
                    u_crlf(io->out_fd) != 0)
                        return 1;
        return 0;
}
#endif

#if UTIL_GROUP == 2
static int
util_cmd_cmp(int argc, kword_t **argv, struct u_io *io)
{
        kword_t a[UTIL_WORD_BUF];
        kword_t b[UTIL_WORD_BUF];
        int fa;
        int fb;
        int na;
        int nb;
        unsigned int i;
        kword_t pos;

        if (argc != 3)
                return 2;
        fa = dsys_open(argv[1], SYS_O_RDONLY);
        if (fa < 0)
                return util_error(io, "CMP", argv[1]);
        fb = dsys_open(argv[2], SYS_O_RDONLY);
        if (fb < 0) {
                (void)dsys_close(fa);
                return util_error(io, "CMP", argv[2]);
        }
        pos = 0UL;
        for (;;) {
                na = dsys_read_words(fa, a, UTIL_WORD_BUF);
                nb = dsys_read_words(fb, b, UTIL_WORD_BUF);
                if (na < 0 || nb < 0) {
                        (void)dsys_close(fa);
                        (void)dsys_close(fb);
                        return 2;
                }
                for (i = 0U; i < (unsigned int)(na < nb ? na : nb); ++i) {
                        if (a[i] != b[i]) {
                                (void)u_puts(io->out_fd, "DIFFER WORD ");
                                (void)u_put_uint(io->out_fd, pos + i);
                                (void)u_crlf(io->out_fd);
                                (void)dsys_close(fa);
                                (void)dsys_close(fb);
                                return 1;
                        }
                }
                if (na != nb) {
                        (void)u_puts(io->out_fd, "DIFFER LENGTH");
                        (void)u_crlf(io->out_fd);
                        (void)dsys_close(fa);
                        (void)dsys_close(fb);
                        return 1;
                }
                if (na == 0)
                        break;
                pos += (kword_t)na;
        }
        (void)dsys_close(fa);
        (void)dsys_close(fb);
        return 0;
}

static int
util_cmd_head(int argc, kword_t **argv, struct u_io *io)
{
        struct u_text_reader r;
        char line[UTIL_LINE_CHARS];
        unsigned int limit;
        unsigned int count;
        int first;
        int a;
        int rc;

        limit = 10U;
        first = 1;
        if (argc > 1 && util_parse_uint(argv[1], &limit) == 0)
                first = 2;
        if (argc == first) {
                if (u_text_open_fd(&r, io->in_fd) != 0)
                        return 1;
                count = 0U;
                while (count < limit && (rc = u_text_getline(&r, line,
                    sizeof(line))) >= 0) {
                        if (util_put_line(io->out_fd, line) != 0) {
                                u_text_close(&r);
                                return 1;
                        }
                        ++count;
                }
                u_text_close(&r);
                return rc == U_TEXT_ERROR ? 1 : 0;
        }
        for (a = first; a < argc; ++a) {
                if (util_open_text_arg(&r, argv[a]) != 0)
                        return util_error(io, "HEAD", argv[a]);
                count = 0U;
                while (count < limit && (rc = u_text_getline(&r, line,
                    sizeof(line))) >= 0) {
                        if (util_put_line(io->out_fd, line) != 0) {
                                u_text_close(&r);
                                return 1;
                        }
                        ++count;
                }
                u_text_close(&r);
                if (rc == U_TEXT_ERROR)
                        return 1;
        }
        return 0;
}
#endif

static int
util_contains(const char *line, const char *pat)
{
        unsigned int i;
        unsigned int j;

        if (pat[0] == 0)
                return 1;
        for (i = 0U; line[i] != 0; ++i) {
                for (j = 0U; pat[j] != 0 && line[i + j] == pat[j]; ++j)
                        ;
                if (pat[j] == 0)
                        return 1;
        }
        return 0;
}

#if UTIL_GROUP == 2
static int
util_cmd_grep(int argc, kword_t **argv, struct u_io *io)
{
        struct u_text_reader r;
        char pattern[U_ARG_WORDS * 6U];
        char line[UTIL_LINE_CHARS];
        int rc;
        int found;

        if (argc != 3 || util_s6_text(argv[1], pattern, sizeof(pattern)) != 0)
                return 2;
        if (util_open_text_arg(&r, argv[2]) != 0)
                return util_error(io, "GREP", argv[2]);
        found = 0;
        while ((rc = u_text_getline(&r, line, sizeof(line))) >= 0) {
                if (util_contains(line, pattern)) {
                        found = 1;
                        if (util_put_line(io->out_fd, line) != 0) {
                                u_text_close(&r);
                                return 2;
                        }
                }
        }
        u_text_close(&r);
        if (rc == U_TEXT_ERROR)
                return 2;
        return found ? 0 : 1;
}

static int
util_cmd_wc(int argc, kword_t **argv, struct u_io *io)
{
        struct u_text_reader r;
        char line[UTIL_LINE_CHARS];
        kword_t lines;
        kword_t words;
        kword_t chars;
        unsigned int i;
        int inword;
        int rc;

        if (argc != 2)
                return 2;
        if (util_open_text_arg(&r, argv[1]) != 0)
                return util_error(io, "WC", argv[1]);
        lines = words = chars = 0UL;
        while ((rc = u_text_getline(&r, line, sizeof(line))) >= 0) {
                ++lines;
                inword = 0;
                for (i = 0U; line[i] != 0; ++i) {
                        ++chars;
                        if (line[i] == ' ' || line[i] == '\t')
                                inword = 0;
                        else if (!inword) {
                                ++words;
                                inword = 1;
                        }
                }
                ++chars;
        }
        u_text_close(&r);
        if (rc == U_TEXT_ERROR)
                return 1;
        if (u_put_uint(io->out_fd, lines) != 0 || u_putc(io->out_fd, ' ') != 0 ||
            u_put_uint(io->out_fd, words) != 0 || u_putc(io->out_fd, ' ') != 0 ||
            u_put_uint(io->out_fd, chars) != 0 || u_crlf(io->out_fd) != 0)
                return 1;
        return 0;
}

static int
util_cmd_tee(int argc, kword_t **argv, struct u_io *io)
{
        struct u_text_reader r;
        char line[UTIL_LINE_CHARS];
        int out;
        int rc;

        if (argc != 2)
                return 2;
        out = dsys_open(argv[1], SYS_O_WRONLY | SYS_O_CREAT | SYS_O_TRUNC);
        if (out < 0)
                return util_error(io, "TEE", argv[1]);
        if (u_text_open_fd(&r, io->in_fd) != 0) {
                (void)dsys_close(out);
                return 1;
        }
        while ((rc = u_text_getline(&r, line, sizeof(line))) >= 0) {
                if (util_put_line(io->out_fd, line) != 0 ||
                    util_write_s6rec_line(out, line) != 0) {
                        rc = U_TEXT_ERROR;
                        break;
                }
        }
        u_text_close(&r);
        (void)dsys_close(out);
        return rc == U_TEXT_ERROR ? 1 : 0;
}
#endif

#if UTIL_GROUP == 1
struct util_expr {
        const char *s;
        unsigned int pos;
        int error;
};

static int util_expr_cmp(struct util_expr *st);

static void
util_expr_skip(struct util_expr *st)
{
        while (st->s[st->pos] == ' ' || st->s[st->pos] == '\t')
                ++st->pos;
}

static int
util_expr_number(struct util_expr *st)
{
        int neg;
        int value;

        util_expr_skip(st);
        neg = 0;
        if (st->s[st->pos] == '-') {
                neg = 1;
                ++st->pos;
        }
        util_expr_skip(st);
        if (st->s[st->pos] < '0' || st->s[st->pos] > '9') {
                st->error = 1;
                return 0;
        }
        value = 0;
        while (st->s[st->pos] >= '0' && st->s[st->pos] <= '9') {
                value = value * 10 + st->s[st->pos] - '0';
                ++st->pos;
        }
        return neg ? -value : value;
}

static int
util_expr_primary(struct util_expr *st)
{
        int value;

        util_expr_skip(st);
        if (st->s[st->pos] != '(')
                return util_expr_number(st);
        ++st->pos;
        value = util_expr_cmp(st);
        util_expr_skip(st);
        if (st->s[st->pos] != ')') {
                st->error = 1;
                return 0;
        }
        ++st->pos;
        return value;
}

static int
util_expr_mul(struct util_expr *st)
{
        int value;

        value = util_expr_primary(st);
        for (;;) {
                int op;
                int rhs;

                util_expr_skip(st);
                op = st->s[st->pos];
                if (op != '*' && op != '/' && op != '%')
                        return value;
                ++st->pos;
                rhs = util_expr_primary(st);
                if (st->error)
                        return 0;
                if (op == '*')
                        value *= rhs;
                else if (rhs == 0) {
                        st->error = 1;
                        return 0;
                } else if (op == '/')
                        value /= rhs;
                else
                        value %= rhs;
        }
}

static int
util_expr_add(struct util_expr *st)
{
        int value;

        value = util_expr_mul(st);
        for (;;) {
                int op;
                int rhs;

                util_expr_skip(st);
                op = st->s[st->pos];
                if (op != '+' && op != '-')
                        return value;
                ++st->pos;
                rhs = util_expr_mul(st);
                if (st->error)
                        return 0;
                if (op == '+')
                        value += rhs;
                else
                        value -= rhs;
        }
}

static int
util_expr_word(struct util_expr *st, const char *word)
{
        unsigned int p;
        unsigned int i;

        util_expr_skip(st);
        p = st->pos;
        for (i = 0U; word[i] != 0; ++i)
                if (st->s[p + i] != word[i])
                        return 0;
        if (st->s[p + i] != 0 && st->s[p + i] != ' ' &&
            st->s[p + i] != '\t' && st->s[p + i] != ')')
                return 0;
        st->pos = p + i;
        return 1;
}

static int
util_expr_cmp(struct util_expr *st)
{
        int value;

        value = util_expr_add(st);
        for (;;) {
                int rhs;

                if (util_expr_word(st, "EQ")) {
                        rhs = util_expr_add(st);
                        value = value == rhs;
                } else if (util_expr_word(st, "NE")) {
                        rhs = util_expr_add(st);
                        value = value != rhs;
                } else if (util_expr_word(st, "LT")) {
                        rhs = util_expr_add(st);
                        value = value < rhs;
                } else if (util_expr_word(st, "LE")) {
                        rhs = util_expr_add(st);
                        value = value <= rhs;
                } else if (util_expr_word(st, "GT")) {
                        rhs = util_expr_add(st);
                        value = value > rhs;
                } else if (util_expr_word(st, "GE")) {
                        rhs = util_expr_add(st);
                        value = value >= rhs;
                } else
                        return value;
                if (st->error)
                        return 0;
        }
}

static int
util_join_args(int argc, kword_t **argv, int first, char *buf,
    unsigned int size)
{
        unsigned int used;
        int a;

        used = 0U;
        for (a = first; a < argc; ++a) {
                unsigned int i;
                unsigned int n;

                if (a != first) {
                        if (used + 1U >= size)
                                return -1;
                        buf[used++] = ' ';
                }
                n = util_s6_len(argv[a]);
                for (i = 0U; i < n; ++i) {
                        if (used + 1U >= size)
                                return -1;
                        buf[used++] = (char)util_s6_ch(argv[a], i);
                }
        }
        if (used >= size)
                return -1;
        buf[used] = 0;
        return 0;
}

static int
util_eval_expr_text(const char *text, int *out)
{
        struct util_expr st;

        st.s = text;
        st.pos = 0U;
        st.error = 0;
        *out = util_expr_cmp(&st);
        util_expr_skip(&st);
        return st.error || st.s[st.pos] != 0 ? -1 : 0;
}

static int
util_put_int(int fd, int value)
{
        if (value < 0) {
                if (u_putc(fd, '-') != 0)
                        return 1;
                return u_put_uint(fd, (kword_t)(-value));
        }
        return u_put_uint(fd, (kword_t)value);
}

static int
util_cmd_expr(int argc, kword_t **argv, struct u_io *io)
{
        char expr[UTIL_LINE_CHARS];
        int value;

        if (argc < 2 || util_join_args(argc, argv, 1, expr,
            sizeof(expr)) != 0 || util_eval_expr_text(expr, &value) != 0)
                return 2;
        if (util_put_int(io->out_fd, value) != 0 || u_crlf(io->out_fd) != 0)
                return 2;
        return value == 0 ? 1 : 0;
}

static int
util_cmd_bc(int argc, kword_t **argv, struct u_io *io)
{
        char expr[UTIL_LINE_CHARS];
        struct u_text_reader r;
        int value;
        int rc;

        if (argc > 1) {
                if (util_join_args(argc, argv, 1, expr, sizeof(expr)) != 0 ||
                    util_eval_expr_text(expr, &value) != 0)
                        return 2;
                return util_put_int(io->out_fd, value) != 0 ||
                    u_crlf(io->out_fd) != 0;
        }
        if (u_text_open_fd(&r, io->in_fd) != 0)
                return 1;
        while ((rc = u_text_getline(&r, expr, sizeof(expr))) >= 0) {
                if (util_eval_expr_text(expr, &value) != 0)
                        break;
                if (util_put_int(io->out_fd, value) != 0 ||
                    u_crlf(io->out_fd) != 0) {
                        u_text_close(&r);
                        return 1;
                }
        }
        u_text_close(&r);
        if (rc == U_TEXT_EOF)
                return 0;
        return 2;
}

static int
util_s6_same(const kword_t *a, const kword_t *b)
{
        unsigned int n;
        unsigned int i;

        n = util_s6_len(a);
        if (n != util_s6_len(b))
                return 0;
        for (i = 0U; i < n; ++i)
                if (util_s6_ch(a, i) != util_s6_ch(b, i))
                        return 0;
        return 1;
}

static int
util_test_unary(const kword_t *op, const kword_t *arg)
{
        struct vfs_stat st;

        if (u_s6_eq(op, "-Z")) return util_s6_len(arg) == 0U ? 0 : 1;
        if (u_s6_eq(op, "-N")) return util_s6_len(arg) != 0U ? 0 : 1;
        if (u_s6_eq(op, "-NZ")) {
                if (util_s6_len(arg) == 0U) return 1;
                if (util_s6_len(arg) == 1U && util_s6_ch(arg, 0U) == '0') return 1;
                return 0;
        }
        if (dsys_stat((kword_t *)arg, &st) != 0)
                return (u_s6_eq(op, "-E") || u_s6_eq(op, "-R") ||
                    u_s6_eq(op, "-W") || u_s6_eq(op, "-X") ||
                    u_s6_eq(op, "-D") || u_s6_eq(op, "-F")) ? 1 : 2;
        if (u_s6_eq(op, "-E") || u_s6_eq(op, "-R") ||
            u_s6_eq(op, "-W") || u_s6_eq(op, "-X")) return 0;
        if (u_s6_eq(op, "-D")) return st.type == VFS_TYPE_DIR ? 0 : 1;
        if (u_s6_eq(op, "-F")) return st.type == VFS_TYPE_REG ? 0 : 1;
        return 2;
}

static int
util_test_binary(const kword_t *left, const kword_t *op, const kword_t *right)
{
        unsigned int l;
        unsigned int r;

        if (u_s6_eq(op, "=") || u_s6_eq(op, "EQ"))
                return util_s6_same(left, right) ? 0 : 1;
        if (u_s6_eq(op, "!=") || u_s6_eq(op, "NE"))
                return util_s6_same(left, right) ? 1 : 0;
        if (util_parse_uint(left, &l) != 0 || util_parse_uint(right, &r) != 0)
                return 2;
        if (u_s6_eq(op, "-EQ")) return l == r ? 0 : 1;
        if (u_s6_eq(op, "-NE")) return l != r ? 0 : 1;
        if (u_s6_eq(op, "-GT")) return l > r ? 0 : 1;
        if (u_s6_eq(op, "-GE")) return l >= r ? 0 : 1;
        if (u_s6_eq(op, "-LT")) return l < r ? 0 : 1;
        if (u_s6_eq(op, "-LE")) return l <= r ? 0 : 1;
        return 2;
}

static int
util_cmd_test(int argc, kword_t **argv, int bracket)
{
        if (bracket) {
                if (argc < 2 || !u_s6_eq(argv[argc - 1], "]"))
                        return 2;
                --argc;
        }
        if (argc == 1) return 1;
        if (argc == 2) return util_s6_len(argv[1]) != 0U ? 0 : 1;
        if (argc == 3) return util_test_unary(argv[1], argv[2]);
        if (argc == 4) return util_test_binary(argv[1], argv[2], argv[3]);
        return 2;
}
#endif

#if UTIL_GROUP == 2
static int
util_cmd_tail(int argc, kword_t **argv, struct u_io *io)
{
        char lines[31][UTIL_SORT_CHARS];
        struct u_text_reader r;
        unsigned int limit;
        unsigned int used;
        unsigned int total;
        unsigned int i;
        int first;
        int rc;

        limit = 10U;
        first = 1;
        if (argc > 1 && util_parse_uint(argv[1], &limit) == 0) {
                if (limit > 31U) limit = 31U;
                first = 2;
        }
        if (argc - first > 1)
                return 2;
        if (argc == first) {
                if (u_text_open_fd(&r, io->in_fd) != 0)
                        return 1;
        } else if (util_open_text_arg(&r, argv[first]) != 0)
                return util_error(io, "TAIL", argv[first]);
        used = 0U;
        total = 0U;
        while ((rc = u_text_getline(&r, lines[total % 31U],
            UTIL_SORT_CHARS)) >= 0) {
                ++total;
                if (used < limit) ++used;
        }
        u_text_close(&r);
        if (rc == U_TEXT_ERROR)
                return 1;
        for (i = 0U; i < used; ++i) {
                unsigned int index = (total - used + i) % 31U;
                if (util_put_line(io->out_fd, lines[index]) != 0)
                        return 1;
        }
        return 0;
}

static int
util_cmd_cut(int argc, kword_t **argv, struct u_io *io)
{
        struct u_text_reader r;
        char line[UTIL_LINE_CHARS];
        unsigned int first;
        unsigned int last;
        unsigned int i;
        int filearg;
        int rc;

        if (argc < 2 || util_parse_uint(argv[1], &first) != 0 || first == 0U)
                return 2;
        last = first;
        filearg = 2;
        if (argc > 2 && util_parse_uint(argv[2], &last) == 0)
                filearg = 3;
        if (last < first || argc - filearg > 1)
                return 2;
        if (argc == filearg) {
                if (u_text_open_fd(&r, io->in_fd) != 0)
                        return 1;
        } else if (util_open_text_arg(&r, argv[filearg]) != 0)
                return util_error(io, "CUT", argv[filearg]);
        while ((rc = u_text_getline(&r, line, sizeof(line))) >= 0) {
                for (i = first - 1U; i < last && line[i] != 0; ++i)
                        if (u_putc(io->out_fd, line[i]) != 0) {
                                u_text_close(&r);
                                return 1;
                        }
                if (u_crlf(io->out_fd) != 0) {
                        u_text_close(&r);
                        return 1;
                }
        }
        u_text_close(&r);
        return rc == U_TEXT_ERROR ? 1 : 0;
}

static int
util_strcmp(const char *a, const char *b)
{
        unsigned int i;
        for (i = 0U; a[i] != 0 && a[i] == b[i]; ++i)
                ;
        return (unsigned char)a[i] - (unsigned char)b[i];
}

static void
util_strcpy(char *dst, const char *src, unsigned int size)
{
        unsigned int i;
        for (i = 0U; i + 1U < size && src[i] != 0; ++i)
                dst[i] = src[i];
        dst[i] = 0;
}

static int
util_cmd_sort(int argc, kword_t **argv, struct u_io *io)
{
        char lines[UTIL_SORT_LINES][UTIL_SORT_CHARS];
        char tmp[UTIL_SORT_CHARS];
        struct u_text_reader r;
        unsigned int used;
        unsigned int i;
        unsigned int j;
        int rc;

        if (argc > 2)
                return 2;
        if (argc == 1) {
                if (u_text_open_fd(&r, io->in_fd) != 0) return 1;
        } else if (util_open_text_arg(&r, argv[1]) != 0)
                return util_error(io, "SORT", argv[1]);
        used = 0U;
        while (used < UTIL_SORT_LINES &&
            (rc = u_text_getline(&r, lines[used], UTIL_SORT_CHARS)) >= 0)
                ++used;
        if (used == UTIL_SORT_LINES && rc >= 0) {
                u_text_close(&r);
                return util_error(io, "SORT: TOO MANY RECORDS", 0);
        }
        u_text_close(&r);
        if (rc == U_TEXT_ERROR) return 1;
        for (i = 1U; i < used; ++i)
                for (j = i; j != 0U && util_strcmp(lines[j - 1U], lines[j]) > 0;
                    --j) {
                        util_strcpy(tmp, lines[j - 1U], sizeof(tmp));
                        util_strcpy(lines[j - 1U], lines[j], UTIL_SORT_CHARS);
                        util_strcpy(lines[j], tmp, UTIL_SORT_CHARS);
                }
        for (i = 0U; i < used; ++i)
                if (util_put_line(io->out_fd, lines[i]) != 0) return 1;
        return 0;
}

static int
util_cmd_dd(int argc, kword_t **argv, struct u_io *io)
{
        kword_t buf[UTIL_WORD_BUF];
        kword_t *inpath;
        kword_t *outpath;
        unsigned int count;
        unsigned int copied;
        int in;
        int out;
        int i;
        int n;

        inpath = 0;
        outpath = 0;
        count = 0U;
        for (i = 1; i < argc; ++i) {
                if (u_s6_eq(argv[i], "IF") && i + 1 < argc) inpath = argv[++i];
                else if (u_s6_eq(argv[i], "OF") && i + 1 < argc) outpath = argv[++i];
                else if (u_s6_eq(argv[i], "COUNT") && i + 1 < argc) {
                        if (util_parse_uint(argv[++i], &count) != 0) return 2;
                } else return 2;
        }
        in = inpath == 0 ? io->in_fd : dsys_open(inpath, SYS_O_RDONLY);
        if (in < 0) return util_error(io, "DD", inpath);
        out = outpath == 0 ? io->out_fd :
            dsys_open(outpath, SYS_O_WRONLY | SYS_O_CREAT | SYS_O_TRUNC);
        if (out < 0) {
                if (inpath != 0) (void)dsys_close(in);
                return util_error(io, "DD", outpath);
        }
        copied = 0U;
        while (count == 0U || copied < count) {
                unsigned int want = UTIL_WORD_BUF;
                if (count != 0U && count - copied < want) want = count - copied;
                n = dsys_read_words(in, buf, want);
                if (n < 0) { copied = 0U; break; }
                if (n == 0) break;
                if (u_write_words_all(out, buf, (unsigned int)n) != 0) {
                        copied = 0U; break;
                }
                copied += (unsigned int)n;
        }
        if (inpath != 0) (void)dsys_close(in);
        if (outpath != 0) (void)dsys_close(out);
        return copied == 0U && count != 0U ? 1 : 0;
}
#endif

#if UTIL_GROUP == 3
static int
util_path_from_parts(kword_t *dst, unsigned int words, const char *prefix,
    const kword_t *tail)
{
        char text[U_PATH_WORDS * 6U];
        unsigned int used;
        unsigned int i;

        used = 0U;
        for (i = 0U; prefix[i] != 0; ++i) {
                if (used + 1U >= sizeof(text)) return -1;
                text[used++] = prefix[i];
        }
        for (i = 0U; i < util_s6_len(tail); ++i) {
                if (used + 1U >= sizeof(text)) return -1;
                text[used++] = (char)util_s6_ch(tail, i);
        }
        text[used] = 0;
        return u_s6_pack(dst, words, text);
}

static int
util_render_sixmd_line(int fd, const char *src)
{
        unsigned int i;
        int heading;

        if (src[0] == '%' && src[1] == 'S')
                return 0;
        i = 0U;
        heading = 0;
        while (src[i] == '#') {
                ++heading;
                ++i;
        }
        if (heading != 0 && src[i] == ' ') {
                ++i;
                if (src[i] == '@' && src[i + 1U] == '?') i += 2U;
        }
        while (src[i] != 0) {
                int ch = (unsigned char)src[i++];
                if (ch == '@') {
                        ch = (unsigned char)src[i++];
                        if (ch >= 'A' && ch <= 'Z') ch += 'a' - 'A';
                        else if (ch != '@') return 1;
                } else if (ch == '\\' && src[i] != 0)
                        ch = (unsigned char)src[i++];
                else if (ch == '*')
                        continue;
                if (u_putc(fd, ch) != 0) return 1;
        }
        if (heading != 0 && u_crlf(fd) != 0) return 1;
        return u_crlf(fd) != 0;
}

static int
util_render_manual_path(const char *path, const kword_t *topic, struct u_io *io)
{
        struct u_text_reader r;
        char line[UTIL_LINE_CHARS];
        char name[U_ARG_WORDS * 6U];
        unsigned int i;
        unsigned int start;
        unsigned int len;
        int found;
        int rc;

        if (util_s6_text(topic, name, sizeof(name)) != 0)
                return 1;
        if (name[0] == '[' && name[1] == 0) {
                name[0] = 'B'; name[1] = 'R'; name[2] = 'A'; name[3] = 'C';
                name[4] = 'K'; name[5] = 'E'; name[6] = 'T'; name[7] = 0;
        }
        if (u_text_open(&r, path) != 0)
                return -1;
        found = 0;
        while ((rc = u_text_getline(&r, line, sizeof(line))) >= 0) {
                if (found && line[0] == '%' && line[1] == 'S')
                        break;
                if (!found) {
                        if (line[0] != '#' || line[1] != ' ' ||
                            line[2] != '@' || line[3] != '?')
                                continue;
                        start = 4U;
                        for (len = 0U; line[start + len] != 0 &&
                            line[start + len] != ' ' &&
                            line[start + len] != '-'; ++len)
                                ;
                        for (i = 0U; name[i] != 0 && i < len; ++i)
                                if (name[i] != line[start + i])
                                        break;
                        if (i != len || name[i] != 0)
                                continue;
                        found = 1;
                }
                if (util_render_sixmd_line(io->out_fd, line) != 0) {
                        u_text_close(&r);
                        return 1;
                }
        }
        u_text_close(&r);
        if (rc == U_TEXT_ERROR)
                return 2;
        return found ? 0 : 1;
}

static int
util_render_help_path(const char *path, const kword_t *topic, struct u_io *io)
{
        struct u_text_reader r;
        char line[UTIL_LINE_CHARS];
        char name[U_ARG_WORDS * 6U];
        unsigned int i;
        unsigned int start;
        unsigned int len;
        int found;
        int selected;
        int rc;

        if (util_s6_text(topic, name, sizeof(name)) != 0)
                return 1;
        if (name[0] == '[' && name[1] == 0) {
                name[0] = 'B'; name[1] = 'R'; name[2] = 'A'; name[3] = 'C';
                name[4] = 'K'; name[5] = 'E'; name[6] = 'T'; name[7] = 0;
        }
        if (u_text_open(&r, path) != 0)
                return -1;
        found = 0;
        selected = 0;
        while ((rc = u_text_getline(&r, line, sizeof(line))) >= 0) {
                if (found && line[0] == '%' && line[1] == 'S')
                        break;
                if (!found) {
                        if (line[0] != '#' || line[1] != ' ' ||
                            line[2] != '@' || line[3] != '?')
                                continue;
                        start = 4U;
                        for (len = 0U; line[start + len] != 0 &&
                            line[start + len] != ' ' &&
                            line[start + len] != '-'; ++len)
                                ;
                        for (i = 0U; name[i] != 0 && i < len; ++i)
                                if (name[i] != line[start + i])
                                        break;
                        if (i != len || name[i] != 0)
                                continue;
                        found = 1;
                        if (util_render_sixmd_line(io->out_fd, line) != 0) {
                                u_text_close(&r);
                                return 1;
                        }
                        continue;
                }
                if (line[0] == '#' && line[1] == '#' && line[2] == ' ') {
                        selected = line[3] == '@' && line[4] == '?' &&
                            ((line[5] == 'O' && line[6] == 'P' &&
                            line[7] == 'T' && line[8] == 'I' &&
                            line[9] == 'O' && line[10] == 'N' &&
                            line[11] == 'S' && line[12] == 0) ||
                            (line[5] == 'O' && line[6] == 'P' &&
                            line[7] == 'E' && line[8] == 'R' &&
                            line[9] == 'A' && line[10] == 'N' &&
                            line[11] == 'D' && line[12] == 'S' &&
                            line[13] == 0));
                }
                if (line[0] == 'U' && line[1] == 'S' && line[2] == 'A' &&
                    line[3] == 'G' && line[4] == 'E' && line[5] == ' ' &&
                    line[6] == 'I' && line[7] == 'S' && line[8] == ' ') {
                        if (util_render_sixmd_line(io->out_fd, line) != 0) {
                                u_text_close(&r);
                                return 1;
                        }
                        continue;
                }
                if (selected && util_render_sixmd_line(io->out_fd, line) != 0) {
                        u_text_close(&r);
                        return 1;
                }
        }
        u_text_close(&r);
        if (rc == U_TEXT_ERROR)
                return 2;
        return found ? 0 : 1;
}

static int
util_not_available(struct u_io *io, const char *what)
{
        return u_puts(io->err_fd, what) != 0 ||
            u_puts(io->err_fd, " NOT AVAILABLE") != 0 ||
            u_crlf(io->err_fd) != 0;
}

static int
util_render_manual(const kword_t *topic, struct u_io *io)
{
        int rc;

        rc = util_render_manual_path("/SYSTEM/MANUAL/PAGES", topic, io);
        if (rc == 0)
                return 0;
        if (rc == 2) {
                (void)util_not_available(io, "MANUAL");
                return 1;
        }
        rc = util_render_manual_path("/OPTION/BASE/MANUAL/ASMUTILS/PAGES",
                topic, io);
        if (rc == 0)
                return 0;
        if (rc == 2) {
                (void)util_not_available(io, "MANUAL");
                return 1;
        }
        (void)util_not_available(io, "MANUAL");
        return 1;
}

static int
util_render_help(const kword_t *topic, struct u_io *io)
{
        int rc;

        rc = util_render_help_path("/SYSTEM/MANUAL/PAGES", topic, io);
        if (rc == 0)
                return 0;
        if (rc == 2) {
                (void)util_not_available(io, "HELP");
                return 1;
        }
        rc = util_render_help_path("/OPTION/BASE/MANUAL/ASMUTILS/PAGES",
                topic, io);
        if (rc == 0)
                return 0;
        if (rc == 2) {
                (void)util_not_available(io, "HELP");
                return 1;
        }
        (void)util_not_available(io, "HELP");
        return 1;
}

static int
util_cmd_man(int argc, kword_t **argv, struct u_io *io)
{
        if (argc != 2)
                return 2;
        return util_render_manual(argv[1], io);
}

static int
util_manual_topics_path(const char *path, struct u_io *io)
{
        struct u_text_reader r;
        char line[UTIL_LINE_CHARS];
        int rc;

        if (u_text_open(&r, path) != 0)
                return -1;
        while ((rc = u_text_getline(&r, line, sizeof(line))) >= 0)
                if (u_puts(io->out_fd, "  ") != 0 ||
                    u_puts(io->out_fd, line) != 0 ||
                    u_crlf(io->out_fd) != 0) {
                        u_text_close(&r);
                        return 1;
                }
        u_text_close(&r);
        return rc == U_TEXT_ERROR ? 1 : 0;
}

static int
util_manual_topics(struct u_io *io)
{
        int rc;

        if (u_puts(io->out_fd, "MANUAL TOPICS") != 0 || u_crlf(io->out_fd) != 0)
                return 1;
        rc = util_manual_topics_path("/SYSTEM/MANUAL/INDEX", io);
        if (rc != 0) {
                (void)util_not_available(io, "HELP");
                return 1;
        }
        rc = util_manual_topics_path("/OPTION/BASE/MANUAL/ASMUTILS/INDEX", io);
        return rc > 0 ? rc : 0;
}

static int
util_cmd_help(int argc, kword_t **argv, struct u_io *io)
{
        if (argc == 1)
                return util_manual_topics(io);
        if (argc == 2)
                return util_render_help(argv[1], io);
        return 2;
}

static int
util_apropos_path(const char *path, const char *needle, struct u_io *io,
        int *foundp)
{
        struct u_text_reader r;
        char line[UTIL_LINE_CHARS];
        int rc;

        if (u_text_open(&r, path) != 0)
                return -1;
        while ((rc = u_text_getline(&r, line, sizeof(line))) >= 0)
                if (util_contains(line, needle)) {
                        *foundp = 1;
                        if (util_put_line(io->out_fd, line) != 0) {
                                u_text_close(&r);
                                return 2;
                        }
                }
        u_text_close(&r);
        return rc == U_TEXT_ERROR ? 2 : 0;
}

static int
util_cmd_apropos(int argc, kword_t **argv, struct u_io *io)
{
        char needle[U_ARG_WORDS * 6U];
        int rc;
        int found;

        if (argc != 2 || util_s6_text(argv[1], needle, sizeof(needle)) != 0)
                return 2;
        found = 0;
        rc = util_apropos_path("/SYSTEM/MANUAL/INDEX", needle, io, &found);
        if (rc != 0) {
                (void)util_not_available(io, "APROPOS");
                return 1;
        }
        rc = util_apropos_path("/OPTION/BASE/MANUAL/ASMUTILS/INDEX", needle,
                io, &found);
        if (rc > 0)
                return rc;
        return found ? 0 : 1;
}
#endif

#if UTIL_GROUP == 2
static int
util_cmd_strings(int argc, kword_t **argv, struct u_io *io)
{
        struct u_text_reader r;
        char line[UTIL_LINE_CHARS];
        int a;
        int rc;

        if (argc < 2)
                return 2;
        for (a = 1; a < argc; ++a) {
                if (util_open_text_arg(&r, argv[a]) != 0)
                        return util_error(io, "STRINGS", argv[a]);
                while ((rc = u_text_getline(&r, line, sizeof(line))) >= 0) {
                        unsigned int i;
                        unsigned int start;

                        start = 0U;
                        for (i = 0U;; ++i) {
                                int printable = line[i] >= 040 && line[i] <= 0176;
                                if (printable) continue;
                                if (i - start >= 4U) {
                                        unsigned int j;
                                        for (j = start; j < i; ++j)
                                                if (u_putc(io->out_fd, line[j]) != 0) {
                                                        u_text_close(&r);
                                                        return 1;
                                                }
                                        if (u_crlf(io->out_fd) != 0) {
                                                u_text_close(&r);
                                                return 1;
                                        }
                                }
                                if (line[i] == 0) break;
                                start = i + 1U;
                        }
                }
                u_text_close(&r);
                if (rc == U_TEXT_ERROR) return 1;
        }
        return 0;
}
#endif

#if UTIL_GROUP == 3
static int
util_append_name(kword_t *dst, unsigned int words, const kword_t *base,
    const struct vfs_dirent *ent)
{
        char text[U_PATH_WORDS * 6U];
        unsigned int used;
        unsigned int i;
        kword_t name[U_PATH_WORDS];

        if (util_s6_text(base, text, sizeof(text)) != 0 ||
            u_s6_from_dirent(name, U_PATH_WORDS, ent) != 0)
                return -1;
        used = 0U;
        while (text[used] != 0) ++used;
        if (used == 0U || text[used - 1U] != '/') {
                if (used + 1U >= sizeof(text)) return -1;
                text[used++] = '/';
        }
        for (i = 0U; i < util_s6_len(name); ++i) {
                if (used + 1U >= sizeof(text)) return -1;
                text[used++] = (char)util_s6_ch(name, i);
        }
        text[used] = 0;
        return u_s6_pack(dst, words, text);
}

static int
util_walk(const kword_t *path, unsigned int depth, int tree, struct u_io *io)
{
        struct vfs_stat st;
        struct vfs_dirent ent;
        kword_t child[U_PATH_WORDS];
        int fd;
        int rc;
        unsigned int i;

        if (dsys_stat((kword_t *)path, &st) != 0)
                return 1;
        if (tree)
                for (i = 0U; i < depth; ++i)
                        if (u_puts(io->out_fd, "  ") != 0) return 1;
        if (u_put_s6(io->out_fd, path) != 0 || u_crlf(io->out_fd) != 0)
                return 1;
        if (st.type != VFS_TYPE_DIR || depth >= 4U)
                return 0;
        fd = dsys_open((kword_t *)path, SYS_O_RDONLY);
        if (fd < 0) return 1;
        while ((rc = dsys_dirread(fd, &ent)) > 0) {
                kword_t name[U_PATH_WORDS];
                if (u_s6_from_dirent(name, U_PATH_WORDS, &ent) != 0) continue;
                if (u_s6_eq(name, ".") || u_s6_eq(name, "..")) continue;
                if (util_append_name(child, U_PATH_WORDS, path, &ent) != 0 ||
                    util_walk(child, depth + 1U, tree, io) != 0) {
                        (void)dsys_close(fd);
                        return 1;
                }
        }
        (void)dsys_close(fd);
        return rc < 0 ? 1 : 0;
}

static int
util_cmd_tree_find(int argc, kword_t **argv, struct u_io *io, int tree)
{
        kword_t dot[U_PATH_WORDS];
        int a;
        int rc;

        if (argc == 1) {
                if (u_s6_pack(dot, U_PATH_WORDS, ".") != 0) return 1;
                return util_walk(dot, 0U, tree, io);
        }
        rc = 0;
        for (a = 1; a < argc; ++a)
                if (util_walk(argv[a], 0U, tree, io) != 0)
                        rc = 1;
        return rc;
}
#endif

#if UTIL_GROUP == 1
static int
util_leap(unsigned int year)
{
        return year % 4U == 0U && (year % 100U != 0U || year % 400U == 0U);
}

static unsigned int
util_month_days(unsigned int month, unsigned int year)
{
        static const unsigned int days[12] = {
                31U, 28U, 31U, 30U, 31U, 30U, 31U, 31U, 30U, 31U, 30U, 31U
        };
        if (month == 2U && util_leap(year)) return 29U;
        return days[month - 1U];
}

static unsigned int
util_weekday(unsigned int year, unsigned int month, unsigned int day)
{
        unsigned int y;
        unsigned int m;
        unsigned int k;
        unsigned int j;
        unsigned int h;

        y = year;
        m = month;
        if (m < 3U) {
                m += 12U;
                --y;
        }
        k = y % 100U;
        j = y / 100U;
        h = (day + (13U * (m + 1U)) / 5U + k + k / 4U +
            j / 4U + 5U * j) % 7U;
        return (h + 6U) % 7U;
}

static int
util_cmd_cal(int argc, kword_t **argv, struct u_io *io)
{
        unsigned int month;
        unsigned int year;
        unsigned int day;
        unsigned int col;

        month = 1U;
        year = 1970U;
        if (argc == 2) {
                if (util_parse_uint(argv[1], &year) != 0) return 2;
        } else if (argc == 3) {
                if (util_parse_uint(argv[1], &month) != 0 ||
                    util_parse_uint(argv[2], &year) != 0) return 2;
        } else if (argc != 1) return 2;
        if (month < 1U || month > 12U || year < 1U) return 2;
        if (u_put_uint(io->out_fd, month) != 0 || u_putc(io->out_fd, ' ') != 0 ||
            u_put_uint(io->out_fd, year) != 0 || u_crlf(io->out_fd) != 0 ||
            util_put_line(io->out_fd, "SU MO TU WE TH FR SA") != 0) return 1;
        col = util_weekday(year, month, 1U);
        for (day = 0U; day < col; ++day)
                if (u_puts(io->out_fd, "   ") != 0) return 1;
        for (day = 1U; day <= util_month_days(month, year); ++day) {
                if (day < 10U && u_putc(io->out_fd, ' ') != 0) return 1;
                if (u_put_uint(io->out_fd, day) != 0) return 1;
                ++col;
                if (col == 7U) {
                        col = 0U;
                        if (u_crlf(io->out_fd) != 0) return 1;
                } else if (u_putc(io->out_fd, ' ') != 0) return 1;
        }
        return col == 0U ? 0 : u_crlf(io->out_fd) != 0;
}
#endif


int
utility_dispatch(int argc, kword_t **argv, kword_t **envp, struct u_io *io)
{
        if (argc <= 0 || argv == 0 || io == 0)
                return 1;
#if UTIL_GROUP == 1
        if (util_name_eq(argv[0], "TRUE")) return 0;
        if (util_name_eq(argv[0], "FALSE")) return 1;
        if (util_name_eq(argv[0], "CHECK")) return util_cmd_check(argc, argv, io);
        if (util_name_eq(argv[0], "BASENAME")) return util_cmd_basename(argc, argv, io);
        if (util_name_eq(argv[0], "DIRNAME")) return util_cmd_dirname(argc, argv, io);
        if (util_name_eq(argv[0], "ENV")) return util_cmd_env(argc, argv, envp, io);
        if (util_name_eq(argv[0], "EXPR")) return util_cmd_expr(argc, argv, io);
        if (util_name_eq(argv[0], "BC")) return util_cmd_bc(argc, argv, io);
        if (util_name_eq(argv[0], "TEST")) return util_cmd_test(argc, argv, 0);
        if (util_name_eq(argv[0], "[")) return util_cmd_test(argc, argv, 1);
        if (util_name_eq(argv[0], "BRACKET")) return util_cmd_test(argc, argv, 1);
#endif
#if UTIL_GROUP == 2
        if (util_name_eq(argv[0], "CMP")) return util_cmd_cmp(argc, argv, io);
        if (util_name_eq(argv[0], "HEAD")) return util_cmd_head(argc, argv, io);
        if (util_name_eq(argv[0], "GREP")) return util_cmd_grep(argc, argv, io);
        if (util_name_eq(argv[0], "WC")) return util_cmd_wc(argc, argv, io);
        if (util_name_eq(argv[0], "TEE")) return util_cmd_tee(argc, argv, io);
        if (util_name_eq(argv[0], "TAIL")) return util_cmd_tail(argc, argv, io);
        if (util_name_eq(argv[0], "CUT")) return util_cmd_cut(argc, argv, io);
        if (util_name_eq(argv[0], "SORT")) return util_cmd_sort(argc, argv, io);
        if (util_name_eq(argv[0], "DD")) return util_cmd_dd(argc, argv, io);
        if (util_name_eq(argv[0], "STRINGS")) return util_cmd_strings(argc, argv, io);
#endif
#if UTIL_GROUP == 3
        if (util_name_eq(argv[0], "MAN")) return util_cmd_man(argc, argv, io);
        if (util_name_eq(argv[0], "HELP")) return util_cmd_help(argc, argv, io);
        if (util_name_eq(argv[0], "APROPOS")) return util_cmd_apropos(argc, argv, io);
        if (util_name_eq(argv[0], "TREE")) return util_cmd_tree_find(argc, argv, io, 1);
        if (util_name_eq(argv[0], "FIND")) return util_cmd_tree_find(argc, argv, io, 0);
#endif
#if UTIL_GROUP == 1
        if (util_name_eq(argv[0], "CAL")) return util_cmd_cal(argc, argv, io);
#endif
        return util_error(io, "UTILITY: UNKNOWN", argv[0]);
}
