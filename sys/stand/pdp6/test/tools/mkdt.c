/* mkdt.c - write DTC551 SIMH word image from octal 36-bit words. */

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
        fprintf(stderr, "usage: mkdt -t dtc -p words -o image\n");
        exit(2);
}

int main(int argc, char **argv)
{
        const char *type = NULL, *payload = NULL, *out_path = NULL;
        FILE *in, *out;
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
        if (type == NULL || strcmp(type, "dtc") != 0 ||
            payload == NULL || out_path == NULL)
                usage();
        in = fopen(payload, "r");
        if (in == NULL) {
                perror(payload);
                return 1;
        }
        out = fopen(out_path, "wb");
        if (out == NULL) {
                perror(out_path);
                return 1;
        }
        while (fgets(line, sizeof(line), in) != NULL) {
                unsigned long long w;
                char *p = line;
                while (*p == ' ' || *p == '\t')
                        p++;
                if (*p == '\0' || *p == '\n' || *p == '#')
                        continue;
                w = parse_word(p);
                if (w == ~0ULL) {
                        fprintf(stderr, "mkdt: bad word: %s", line);
                        return 1;
                }
                put32(out, (uint32_t)((w >> 18) & 0777777U));
                put32(out, (uint32_t)(w & 0777777U));
        }
        fclose(out);
        fclose(in);
        return 0;
}
