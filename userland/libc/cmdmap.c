#include "cmdmap.h"
#include "text.h"

#define CMDMAP_LINE_CHARS 96U

static int
cmdmap_command_eq(const kword_t *command, const char *text, unsigned int n)
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
                if (ch != (unsigned char)text[i])
                        return 0;
        }
        return 1;
}

int
u_cmd_resolve(const kword_t *command, kword_t *path, unsigned int path_words)
{
        struct u_text_reader r;
        char line[CMDMAP_LINE_CHARS];
        char full[CMDMAP_LINE_CHARS];
        unsigned int split;
        unsigned int i;
        unsigned int p;
        int rc;

        if (command == 0 || path == 0 || path_words == 0U)
                return -1;
        if (u_text_open(&r, "/SYSTEM/EXEC/MAP") != 0)
                return -1;
        while ((rc = u_text_getline(&r, line, sizeof(line))) == 0) {
                for (split = 0U; line[split] != 0 && line[split] != ' ';
                    ++split)
                        ;
                if (line[split] != ' ' ||
                    !cmdmap_command_eq(command, line, split))
                        continue;
                while (line[split] == ' ')
                        ++split;
                p = 0U;
                for (i = 0U; "/SYSTEM/EXEC/"[i] != 0; ++i) {
                        if (p + 1U >= sizeof(full)) {
                                u_text_close(&r);
                                return -1;
                        }
                        full[p++] = "/SYSTEM/EXEC/"[i];
                }
                for (i = split; line[i] != 0; ++i) {
                        if (p + 1U >= sizeof(full)) {
                                u_text_close(&r);
                                return -1;
                        }
                        full[p++] = line[i];
                }
                full[p] = 0;
                u_text_close(&r);
                return u_s6_pack(path, path_words, full);
        }
        u_text_close(&r);
        return -1;
}
