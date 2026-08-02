/* mktap.c - write a SIMH magnetic-tape record from octal 36-bit words. */

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WORD_MASK 0777777777777ULL

static void put32(FILE *f, uint32_t v)
{
        unsigned char b[4];
        b[0] = (unsigned char)(v & 0xff);
        b[1] = (unsigned char)((v >> 8) & 0xff);
        b[2] = (unsigned char)((v >> 16) & 0xff);
        b[3] = (unsigned char)((v >> 24) & 0xff);
        fwrite(b, 1, 4, f);
}

static unsigned long long parse_word(const char *s)
{
        char *end;
        unsigned long long v;
        errno = 0;
        v = strtoull(s, &end, 8);
        if (errno != 0 || end == s || (v & ~WORD_MASK))
                return ~0ULL;
        return v;
}

static void usage(void)
{
        fprintf(stderr, "usage: mktap -t mtc -p words -o tape\n");
        exit(2);
}

int main(int argc, char **argv)
{
        const char *type = NULL, *payload = NULL, *out_path = NULL;
        FILE *in, *out;
        unsigned char *data = NULL;
        size_t used = 0, cap = 0;
        char line[256];
        int i;

        for (i = 1; i < argc; i++) {
                if (strcmp(argv[i], "-t") == 0 && i + 1 < argc)
                        type = argv[++i];
                else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc)
                        payload = argv[++i];
                else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc)
                        out_path = argv[++i];
                else
                        usage();
        }
        if (type == NULL || strcmp(type, "mtc") != 0 ||
            payload == NULL || out_path == NULL)
                usage();
        in = fopen(payload, "r");
        if (in == NULL) {
                perror(payload);
                return 1;
        }
        while (fgets(line, sizeof(line), in) != NULL) {
                unsigned long long w;
                unsigned char b[5];
                char *p = line;
                while (*p == ' ' || *p == '\t')
                        p++;
                if (*p == '\0' || *p == '\n' || *p == '#')
                        continue;
                w = parse_word(p);
                if (w == ~0ULL) {
                        fprintf(stderr, "mktap: bad word: %s", line);
                        return 1;
                }
                if (used + 5 > cap) {
                        cap = cap ? cap * 2 : 1024;
                        data = realloc(data, cap);
                        if (data == NULL) {
                                perror("mktap: realloc");
                                return 1;
                        }
                }
                b[0] = (unsigned char)((w >> 28) & 0xff);
                b[1] = (unsigned char)((w >> 20) & 0xff);
                b[2] = (unsigned char)((w >> 12) & 0xff);
                b[3] = (unsigned char)((w >> 4) & 0xff);
                b[4] = (unsigned char)(w & 0x0f);
                memcpy(data + used, b, 5);
                used += 5;
        }
        fclose(in);

        out = fopen(out_path, "wb");
        if (out == NULL) {
                perror(out_path);
                return 1;
        }
        put32(out, (uint32_t)used);
        fwrite(data, 1, used, out);
        if (used & 1)
                fputc(0, out);
        put32(out, (uint32_t)used);
        put32(out, 0);
        fclose(out);
        free(data);
        return 0;
}
