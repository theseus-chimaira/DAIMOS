#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CSIX_WORD_MASK       UINT64_C(0777777777777)
#define CSIX_S6_TYPE_SHIFT   30U
#define CSIX_S6_TYPE_MASK    UINT64_C(077)
#define CSIX_S6_TEXT         UINT64_C(1)
#define CSIX_S6_COUNT_MASK   UINT64_C(077777777)

struct linebuf {
    unsigned char *data;
    size_t len;
    size_t cap;
};

/* Native KCC writes five ASCII7 characters per 36-bit word.  This
 * input view feeds the existing C-SIX source encoder verbatim, so the
 * C-SIX escaping rules stay defined in exactly one place.
 */
struct csix_input {
    FILE *file;
    uint64_t word;
    unsigned int slot;
    int packed7;
    int padded;
};

static void
die(const char *msg)
{
    fprintf(stderr, "csix: %s\n", msg);
    exit(1);
}

static FILE *
open_input(const char *name)
{
    FILE *f;

    if (strcmp(name, "-") == 0)
        return stdin;
    f = fopen(name, "rb");
    if (f == NULL) {
        perror(name);
        exit(1);
    }
    return f;
}

static FILE *
open_output(const char *name)
{
    FILE *f;

    if (strcmp(name, "-") == 0)
        return stdout;
    f = fopen(name, "wb");
    if (f == NULL) {
        perror(name);
        exit(1);
    }
    return f;
}

static void
close_file(FILE *f, const char *name)
{
    if (f == stdin || f == stdout || f == stderr)
        return;
    if (fclose(f) != 0) {
        perror(name);
        exit(1);
    }
}

static void
write_word(FILE *out, uint64_t w)
{
    unsigned char b[8];
    unsigned int i;

    w &= CSIX_WORD_MASK;
    for (i = 0U; i < 8U; ++i) {
        b[i] = (unsigned char)(w & UINT64_C(0377));
        w >>= 8;
    }
    if (fwrite(b, 1U, sizeof(b), out) != sizeof(b))
        die("write error");
}

static int
read_word(FILE *in, uint64_t *wp)
{
    unsigned char b[8];
    size_t n;
    uint64_t w;
    unsigned int i;

    n = fread(b, 1U, sizeof(b), in);
    if (n == 0U) {
        if (ferror(in))
            die("read error");
        return 0;
    }
    if (n != sizeof(b))
        die("truncated 36-bit word container");
    if ((b[4] & 0360U) != 0U || b[5] != 0U || b[6] != 0U || b[7] != 0U)
        die("noncanonical 36-bit word container");
    w = UINT64_C(0);
    for (i = 0U; i < 5U; ++i)
        w |= (uint64_t)b[i] << (8U * i);
    *wp = w & CSIX_WORD_MASK;
    return 1;
}

static int
source_char(struct csix_input *input)
{
    unsigned int ch;

    if (!input->packed7)
        return getc(input->file);
    for (;;) {
        if (input->slot >= 5U) {
            if (!read_word(input->file, &input->word))
                return EOF;
            input->slot = 0U;
        }
        ch = (unsigned int)((input->word >>
            (29U - input->slot * 7U)) & UINT64_C(0177));
        ++input->slot;
        if (ch == 0U) {
            input->padded = 1;
            continue;
        }
        if (input->padded)
            die("nonzero data after packed ASCII7 padding");
        return (int)ch;
    }
}

static void
line_reserve(struct linebuf *line, size_t need)
{
    size_t cap;
    unsigned char *p;

    if (need <= line->cap)
        return;
    cap = line->cap != 0U ? line->cap : 128U;
    while (cap < need) {
        if (cap > ((size_t)-1) / 2U)
            die("source line too long");
        cap *= 2U;
    }
    p = (unsigned char *)realloc(line->data, cap);
    if (p == NULL)
        die("out of memory");
    line->data = p;
    line->cap = cap;
}

static void
line_put(struct linebuf *line, unsigned int c)
{
    line_reserve(line, line->len + 1U);
    line->data[line->len++] = (unsigned char)c;
}

static void
encode_char(struct linebuf *line, unsigned int c)
{
    if (c >= 'a' && c <= 'z') {
        line_put(line, c - 'a' + 'A');
        return;
    }
    if (c >= 'A' && c <= 'Z') {
        line_put(line, '@');
        line_put(line, c);
        return;
    }
    switch (c) {
    case '@': line_put(line, '@'); line_put(line, '@'); return;
    case '`': line_put(line, '@'); line_put(line, '\''); return;
    case '{': line_put(line, '@'); line_put(line, '<'); return;
    case '|': line_put(line, '@'); line_put(line, '!'); return;
    case '}': line_put(line, '@'); line_put(line, '>'); return;
    case '~': line_put(line, '@'); line_put(line, '-'); return;
    default:
        if (c >= 040U && c <= 0137U) {
            line_put(line, c);
            return;
        }
        die("character outside C-SIX source repertoire");
    }
}

static void
emit_record(FILE *out, const struct linebuf *line)
{
    uint64_t h;
    size_t off;

    if (line->len > (size_t)CSIX_S6_COUNT_MASK)
        die("C-SIX record too long");
    h = (CSIX_S6_TEXT << CSIX_S6_TYPE_SHIFT) | (uint64_t)line->len;
    write_word(out, h);
    for (off = 0U; off < line->len; off += 6U) {
        uint64_t w;
        unsigned int i;

        w = UINT64_C(0);
        for (i = 0U; i < 6U; ++i) {
            unsigned int v;

            v = 0U;
            if (off + i < line->len) {
                unsigned int c;

                c = line->data[off + i];
                if (c < 040U || c > 0137U)
                    die("internal C-SIX encoding error");
                v = c - 040U;
            }
            w = (w << 6) | (uint64_t)(v & 077U);
        }
        write_word(out, w);
    }
}

static void
encode(FILE *in, FILE *out, int packed7)
{
    struct linebuf line;
    struct csix_input source;
    unsigned int column;
    int c;
    int saw_any;
    int at_bol;

    memset(&line, 0, sizeof(line));
    memset(&source, 0, sizeof(source));
    source.file = in;
    source.packed7 = packed7;
    source.slot = 5U;
    column = 0U;
    saw_any = 0;
    at_bol = 1;
    while ((c = source_char(&source)) != EOF) {
        saw_any = 1;
        if (c == '\r') {
            int next;

            next = source_char(&source);
            if (next != '\n')
                die("bare carriage return in source");
            c = '\n';
        }
        if (c == '\n') {
            emit_record(out, &line);
            line.len = 0U;
            column = 0U;
            at_bol = 1;
            continue;
        }
        if (c == '\t') {
            unsigned int n;

            n = 8U - (column & 7U);
            while (n-- != 0U) {
                encode_char(&line, ' ');
                column++;
            }
            at_bol = 0;
            continue;
        }
        encode_char(&line, (unsigned int)(unsigned char)c);
        column++;
        at_bol = 0;
    }
    if (ferror(in))
        die("read error");
    if (saw_any && !at_bol)
        die("source file does not end with newline");
    free(line.data);
}

static int
decode_escape(unsigned int e)
{
    if (e >= 'A' && e <= 'Z')
        return (int)e;
    switch (e) {
    case '@': return '@';
    case '\'': return '`';
    case '<': return '{';
    case '!': return '|';
    case '>': return '}';
    case '-': return '~';
    default: return -1;
    }
}

static void
decode(FILE *in, FILE *out)
{
    uint64_t h;

    while (read_word(in, &h)) {
        uint64_t len;
        uint64_t words;
        uint64_t wi;
        uint64_t left;
        int escaped;

        if (((h >> CSIX_S6_TYPE_SHIFT) & CSIX_S6_TYPE_MASK) != CSIX_S6_TEXT)
            die("non-text S6REC record");
        len = h & CSIX_S6_COUNT_MASK;
        if (h != ((CSIX_S6_TEXT << CSIX_S6_TYPE_SHIFT) | len))
            die("noncanonical S6REC header");
        words = (len + UINT64_C(5)) / UINT64_C(6);
        left = len;
        escaped = 0;
        for (wi = UINT64_C(0); wi < words; ++wi) {
            uint64_t w;
            unsigned int i;

            if (!read_word(in, &w))
                die("truncated S6REC payload");
            for (i = 0U; i < 6U; ++i) {
                unsigned int v;
                unsigned int c;

                v = (unsigned int)((w >> (30U - 6U * i)) & UINT64_C(077));
                if (left == 0U) {
                    if (v != 0U)
                        die("nonzero S6REC padding");
                    continue;
                }
                left--;
                c = v + 040U;
                if (escaped) {
                    int d;

                    d = decode_escape(c);
                    if (d < 0)
                        die("unassigned C-SIX escape");
                    if (putc(d, out) == EOF)
                        die("write error");
                    escaped = 0;
                } else if (c == '@') {
                    escaped = 1;
                } else {
                    if (c >= 'A' && c <= 'Z')
                        c += 'a' - 'A';
                    if (putc((int)c, out) == EOF)
                        die("write error");
                }
            }
        }
        if (escaped)
            die("dangling C-SIX escape");
        if (putc('\n', out) == EOF)
            die("write error");
    }
}

static void
usage(void)
{
    fprintf(stderr, "usage: csix -e input output\n");
    fprintf(stderr, "       csix -e -p input.packed7 output.s6rec\n");
    fprintf(stderr, "       csix -d input output\n");
    exit(2);
}

int
main(int argc, char **argv)
{
    FILE *in;
    FILE *out;
    int encode_mode;
    int packed7;
    int start;

    if (argc < 4 || argc > 5 ||
        (strcmp(argv[1], "-e") != 0 && strcmp(argv[1], "-d") != 0))
        usage();
    encode_mode = strcmp(argv[1], "-e") == 0;
    packed7 = argc == 5;
    if (packed7 && (!encode_mode || strcmp(argv[2], "-p") != 0))
        usage();
    start = packed7 ? 3 : 2;
    in = open_input(argv[start]);
    out = open_output(argv[start + 1]);
    if (encode_mode)
        encode(in, out, packed7);
    else
        decode(in, out);
    if (fflush(out) != 0)
        die("write error");
    close_file(in, argv[start]);
    close_file(out, argv[start + 1]);
    return 0;
}
