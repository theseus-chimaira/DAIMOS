/* D6 production-format host writer v059. Inactive pending full cutover. */
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WORD_MASK UINT64_C(0777777777777)
#define HALF_MASK UINT64_C(0777777)
#define DXR_F_COMPRESSED UINT64_C(0100000)
#define D6CODEC_WINDOW 128U
#define D6CODEC_MAX_LENGTH 127U
#define D6CODEC_GROUP_MAX 30U

static void die(const char *s)
{
    fprintf(stderr, "d6codec-v059: %s\n", s);
    exit(1);
}

static void die_path(const char *path)
{
    fprintf(stderr, "d6codec-v059: %s: %s\n", path, strerror(errno));
    exit(1);
}

static uint64_t get64le(const unsigned char b[8])
{
    uint64_t v = 0;
    unsigned i;
    for (i = 0; i < 8U; ++i)
        v |= (uint64_t)b[i] << (8U * i);
    return v;
}

static void put64le(unsigned char b[8], uint64_t v)
{
    unsigned i;
    for (i = 0; i < 8U; ++i)
        b[i] = (unsigned char)(v >> (8U * i));
}

static uint64_t *read_words(const char *path, size_t *np)
{
    FILE *f;
    long size = 0;
    size_t n, i;
    uint64_t *w;
    unsigned char b[8];

    f = fopen(path, "rb");
    if (f == NULL)
        die_path(path);
    if (fseek(f, 0L, SEEK_END) != 0 || (size = ftell(f)) < 0)
        die_path(path);
    if ((size % 8L) != 0L)
        die("input is not a whole number of 36-bit container words");
    rewind(f);
    n = (size_t)(size / 8L);
    w = n == 0U ? NULL : (uint64_t *)malloc(n * sizeof(*w));
    if (n != 0U && w == NULL)
        die("out of memory");
    for (i = 0; i < n; ++i) {
        if (fread(b, 1U, 8U, f) != 8U)
            die_path(path);
        w[i] = get64le(b);
        if ((w[i] & ~WORD_MASK) != 0U)
            die("input word exceeds 36 bits");
    }
    if (fclose(f) != 0)
        die_path(path);
    *np = n;
    return w;
}

static void write_words(const char *path, const uint64_t *w, size_t n)
{
    FILE *f;
    size_t i;
    unsigned char b[8];

    f = fopen(path, "wb");
    if (f == NULL)
        die_path(path);
    for (i = 0; i < n; ++i) {
        put64le(b, w[i]);
        if (fwrite(b, 1U, 8U, f) != 8U)
            die_path(path);
    }
    if (fclose(f) != 0)
        die_path(path);
}


static uint64_t *read_text_words(const char *path, size_t *np)
{
    FILE *f;
    uint64_t *w = NULL;
    size_t n = 0U;
    size_t cap = 0U;
    char line[128];

    f = fopen(path, "r");
    if (f == NULL)
        die_path(path);
    while (fgets(line, sizeof(line), f) != NULL) {
        char *end;
        unsigned long long v;
        if (strchr(line, '\n') == NULL && !feof(f))
            die("text word line is too long");
        errno = 0;
        v = strtoull(line, &end, 8);
        if (errno != 0 || end == line)
            die("invalid octal text word");
        while (*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n')
            ++end;
        if (*end != '\0' || v > WORD_MASK)
            die("invalid octal text word");
        if (n == cap) {
            size_t newcap = cap == 0U ? 256U : cap * 2U;
            uint64_t *nw = (uint64_t *)realloc(w, newcap * sizeof(*nw));
            if (nw == NULL)
                die("out of memory");
            w = nw;
            cap = newcap;
        }
        w[n++] = (uint64_t)v;
    }
    if (ferror(f))
        die_path(path);
    if (fclose(f) != 0)
        die_path(path);
    *np = n;
    return w;
}

static void write_text_words(const char *path, const uint64_t *w, size_t n)
{
    FILE *f;
    size_t i;

    f = fopen(path, "w");
    if (f == NULL)
        die_path(path);
    for (i = 0U; i < n; ++i)
        if (fprintf(f, "%012llo\n", (unsigned long long)w[i]) < 0)
            die_path(path);
    if (fclose(f) != 0)
        die_path(path);
}

static uint64_t six3(char a, char b, char c)
{
    return ((uint64_t)(a - 040) << 12) |
           ((uint64_t)(b - 040) << 6) |
           (uint64_t)(c - 040);
}

/*
 * Production-candidate grouped D6 format encoder, host build v059.
 * Matches the native v053 policy: zero run first, then longest backward
 * word match in descending distance order; minimum word match length 2.
 * Pair 18-bit descriptors ahead of all literal words for each <=30-token
 * group. The output worst case is the input plus one control per 30 words.
 */
static uint64_t *compress_words(const uint64_t *in, size_t n, size_t *outn)
{
    size_t cap, op, pos;
    uint64_t *out;

    if (n > SIZE_MAX - 29U)
        die("input word count overflows group allocation");
    cap = n + (n + 29U) / 30U;
    if (cap > SIZE_MAX / sizeof(*out))
        die("compressed buffer allocation overflows");
    out = cap == 0U ? NULL : (uint64_t *)malloc(cap * sizeof(*out));
    if (cap != 0U && out == NULL)
        die("out of memory");
    op = 0U;
    pos = 0U;
    while (pos < n) {
        uint64_t desc[15];
        uint64_t literal[30];
        uint64_t flags = 0U;
        unsigned int tokens = 0U, matches = 0U, literals = 0U;
        size_t ctrlpos = op++;

        while (tokens < D6CODEC_GROUP_MAX && pos < n) {
            size_t limit = n - pos;
            size_t len = 0U, dist = 0U;
            unsigned int kind = 0U;

            if (limit > D6CODEC_MAX_LENGTH)
                limit = D6CODEC_MAX_LENGTH;
            if ((in[pos] & ~WORD_MASK) != 0U)
                die("source word exceeds 36 bits");
            if (in[pos] == 0U) {
                len = 1U;
                while (len < limit && in[pos + len] == 0U)
                    ++len;
                kind = 1U;
            } else {
                size_t d, maxdist = pos;
                if (maxdist > D6CODEC_WINDOW)
                    maxdist = D6CODEC_WINDOW;
                for (d = maxdist; d > 0U; --d) {
                    size_t candidate;
                    if (in[pos - d] != in[pos])
                        continue;
                    candidate = 1U;
                    while (candidate < limit &&
                        in[pos + candidate] == in[pos + candidate - d])
                        ++candidate;
                    if (candidate > len) {
                        len = candidate;
                        dist = d;
                        if (len == limit)
                            break;
                    }
                }
                if (len < 2U)
                    len = 0U;
            }
            if (len != 0U) {
                uint64_t word;
                flags |= UINT64_C(1) << (29U - tokens);
                word = ((uint64_t)kind << 16U) |
                    ((uint64_t)len << 9U) | (uint64_t)dist;
                if ((word & ~HALF_MASK) != 0U)
                    die("internal descriptor overflow");
                desc[matches++] = word;
                pos += len;
            } else {
                literal[literals++] = in[pos++];
            }
            ++tokens;
        }
        out[ctrlpos] = ((uint64_t)tokens << 30U) | flags;
        {
            unsigned int i;
            for (i = 0U; i < matches; i += 2U) {
                uint64_t packed = desc[i] << 18U;
                if (i + 1U < matches)
                    packed |= desc[i + 1U];
                out[op++] = packed;
            }
            for (i = 0U; i < literals; ++i)
                out[op++] = literal[i];
        }
        if (op > cap)
            die("internal compressed capacity overflow");
    }
    *outn = op;
    return out;
}

static void compress_raw(const char *inpath, const char *outpath, int text_mode)
{
    uint64_t *in, *out;
    size_t n, outn;
    in = text_mode ? read_text_words(inpath, &n) : read_words(inpath, &n);
    out = compress_words(in, n, &outn);
    if (text_mode)
        write_text_words(outpath, out, outn);
    else
        write_words(outpath, out, outn);
    free(out);
    free(in);
}

static void compress_framed(const char *inpath, const char *outpath)
{
    uint64_t *in, *payload, *out;
    size_t n, payload_n, i;

    in = read_words(inpath, &n);
    if (n > (size_t)WORD_MASK)
        die("framed input is too large");
    payload = compress_words(in, n, &payload_n);
    out = (uint64_t *)malloc((payload_n + 1U) * sizeof(*out));
    if (out == NULL)
        die("out of memory");
    out[0] = (uint64_t)n;
    for (i = 0U; i < payload_n; ++i)
        out[i + 1U] = payload[i];
    write_words(outpath, out, payload_n + 1U);
    free(out);
    free(payload);
    free(in);
}

static void compress_exec(const char *inpath, const char *outpath)
{
    uint64_t *in, *payload, *out;
    size_t n, payload_n, header = 0, reloc, image, relpos, outn, i;
    uint64_t dxr = six3('D', 'X', 'R');
    uint64_t tx2 = six3('T', 'X', '2');

    in = read_words(inpath, &n);
    if (n < 2U || ((in[0] >> 18) & HALF_MASK) != dxr)
        die("input is not a DXR executable");
    if ((in[1] & DXR_F_COMPRESSED) != 0U)
        die("input executable is already compressed");
    image = (size_t)((in[1] >> 18) & HALF_MASK);
    if (image == 0U)
        die("DXR executable has an empty image");
    reloc = (image + 35U) / 36U;
    if (n == 2U + image + reloc) {
        header = 2U;
    } else if (n == 3U + image + reloc) {
        header = 3U;
        if ((in[2] & HALF_MASK) != tx2 || ((in[2] >> 18) & HALF_MASK) > image)
            die("DXR2 executable has invalid text metadata");
    } else {
        die("DXR executable has inconsistent length");
    }
    relpos = header + image;
    payload = compress_words(in + header, image, &payload_n);
    outn = 3U + payload_n + reloc;
    out = (uint64_t *)malloc(outn * sizeof(*out));
    if (out == NULL)
        die("out of memory");
    out[0] = in[0];
    out[1] = in[1] | DXR_F_COMPRESSED;
    out[2] = header == 3U ? in[2] : tx2;
    for (i = 0U; i < payload_n; ++i)
        out[3U + i] = payload[i];
    for (i = 0U; i < reloc; ++i)
        out[3U + payload_n + i] = in[relpos + i];
    write_words(outpath, out, outn);
    free(out);
    free(payload);
    free(in);
}

static void usage(void)
{
    fprintf(stderr, "usage: d6codec-v059 [-t|-f] [-x|-X] input output\n");
    exit(1);
}

int main(int argc, char **argv)
{
    int exec_mode = 0;
    int text_mode = 0;
    int framed_mode = 0;
    int arg = 1;
    while (arg < argc && argv[arg][0] == '-') {
        if (strcmp(argv[arg], "-x") == 0 || strcmp(argv[arg], "-X") == 0)
            exec_mode = 1;
        else if (strcmp(argv[arg], "-t") == 0)
            text_mode = 1;
        else if (strcmp(argv[arg], "-f") == 0)
            framed_mode = 1;
        else
            usage();
        ++arg;
    }
    if (argc - arg != 2 || (exec_mode && (text_mode || framed_mode)) ||
        (text_mode && framed_mode))
        usage();
    if (exec_mode)
        compress_exec(argv[arg], argv[arg + 1]);
    else if (framed_mode)
        compress_framed(argv[arg], argv[arg + 1]);
    else
        compress_raw(argv[arg], argv[arg + 1], text_mode);
    return 0;
}
