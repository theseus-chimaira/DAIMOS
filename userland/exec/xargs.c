#include "u.h"

#define XARGS_STATUS_NOT_FOUND 127
#define XARGS_STATUS_ERROR     126

#define XARGS_INPUT_WORDS       8U
#define XARGS_RUN_WORDS       192U
#define XARGS_ARENA_WORDS     160U
#define XARGS_MAP_TOKEN_CHARS   24U

struct xargs_reader {
        int fd;
        kword_t words[XARGS_INPUT_WORDS];
        unsigned int pos;
        unsigned int used;
        kword_t chars_left;
        kword_t current_word;
        unsigned int slot;
        int record_end;
};

static kword_t xargs_path[U_PATH_WORDS];
static kword_t xargs_run[XARGS_RUN_WORDS];
static kword_t xargs_arena[XARGS_ARENA_WORDS];
static kword_t *xargs_vec[SYS_RUN_ARG_MAX];
static unsigned int xargs_arena_used;

static kword_t xargs_echo[U_ARG_WORDS];

static unsigned int
xargs_record_words(const kword_t *record)
{
        unsigned int chars;

        chars = (unsigned int)(record[0] & 0777777UL);
        return 1U + (chars + 5U) / 6U;
}

static int
xargs_s6_get(const kword_t *record, unsigned int pos)
{
        unsigned int shift;

        shift = 30U - (pos % 6U) * 6U;
        return (int)(((record[1U + pos / 6U] >> shift) & 077UL) + 040UL);
}

static int
xargs_s6_eq(const kword_t *record, const char *text)
{
        unsigned int len;
        unsigned int i;

        len = (unsigned int)(record[0] & 0777777UL);
        for (i = 0U; i < len && text[i] != 0; ++i)
                if (xargs_s6_get(record, i) != (unsigned char)text[i])
                        return 0;
        return i == len && text[i] == 0;
}

static int
xargs_s6_uint(const kword_t *record, unsigned int *valuep)
{
        unsigned int len;
        unsigned int i;
        unsigned int value;
        int ch;

        len = (unsigned int)(record[0] & 0777777UL);
        if (len == 0U || valuep == 0)
                return -1;
        value = 0U;
        for (i = 0U; i < len; ++i) {
                ch = xargs_s6_get(record, i);
                if (ch < '0' || ch > '9')
                        return -1;
                value = value * 10U + (unsigned int)(ch - '0');
        }
        *valuep = value;
        return 0;
}

static void
xargs_s6_clear(kword_t *record)
{
        unsigned int i;

        for (i = 0U; i < U_ARG_WORDS; ++i)
                record[i] = 0UL;
}

static int
xargs_s6_add(kword_t *record, int ch)
{
        unsigned int len;
        unsigned int word;
        unsigned int shift;

        len = (unsigned int)(record[0] & 0777777UL);
        if (len >= SYS_RUN_ARG_MAX_CHARS || ch < 040 || ch > 0137)
                return -1;
        word = 1U + len / 6U;
        shift = 30U - (len % 6U) * 6U;
        record[word] |= ((kword_t)(ch - 040) & 077UL) << shift;
        record[0] = (kword_t)(len + 1U);
        return 0;
}

static int
xargs_path_prefix(const char *prefix, const kword_t *command)
{
        unsigned int i;
        unsigned int len;

        xargs_s6_clear(xargs_path);
        for (i = 0U; prefix[i] != 0; ++i)
                if (xargs_s6_add(xargs_path, prefix[i]) != 0)
                        return -1;
        len = (unsigned int)(command[0] & 0777777UL);
        for (i = 0U; i < len; ++i)
                if (xargs_s6_add(xargs_path, xargs_s6_get(command, i)) != 0)
                        return -1;
        return 0;
}

static int
xargs_reader_word(struct xargs_reader *reader, kword_t *wordp)
{
        int got;

        if (reader->pos == reader->used) {
                got = dsys_read_words(reader->fd, reader->words,
                    XARGS_INPUT_WORDS);
                if (got <= 0)
                        return got;
                reader->pos = 0U;
                reader->used = (unsigned int)got;
        }
        *wordp = reader->words[reader->pos++];
        return 1;
}

static void
xargs_reader_init(struct xargs_reader *reader, int fd)
{
        reader->fd = fd;
        reader->pos = 0U;
        reader->used = 0U;
        reader->chars_left = 0UL;
        reader->current_word = 0UL;
        reader->slot = 6U;
        reader->record_end = 0;
}

static int
xargs_reader_char(struct xargs_reader *reader)
{
        kword_t header;
        unsigned int shift;
        int rc;
        int ch;

        if (reader->record_end) {
                reader->record_end = 0;
                return '\n';
        }
        if (reader->chars_left == 0UL) {
                rc = xargs_reader_word(reader, &header);
                if (rc <= 0)
                        return rc == 0 ? -1 : -2;
                if (((header >> 30U) & 077UL) != 1UL)
                        return -2;
                reader->chars_left = header & 077777777UL;
                reader->slot = 6U;
                if (reader->chars_left == 0UL) {
                        reader->record_end = 1;
                        return xargs_reader_char(reader);
                }
        }
        if (reader->slot == 6U) {
                if (xargs_reader_word(reader, &reader->current_word) != 1)
                        return -2;
                reader->slot = 0U;
        }
        shift = 30U - reader->slot * 6U;
        ch = (int)(((reader->current_word >> shift) & 077UL) + 040UL);
        ++reader->slot;
        --reader->chars_left;
        if (reader->chars_left == 0UL)
                reader->record_end = 1;
        return ch;
}

static int
xargs_map_token(struct xargs_reader *reader, char *token,
    unsigned int size)
{
        unsigned int used;
        int ch;

        used = 0U;
        do {
                ch = xargs_reader_char(reader);
                if (ch < 0)
                        return ch;
        } while (ch == ' ' || ch == '\t' || ch == '\n');
        for (;;) {
                if (used + 1U >= size)
                        return -2;
                token[used++] = (char)ch;
                ch = xargs_reader_char(reader);
                if (ch < 0 || ch == ' ' || ch == '\t' || ch == '\n')
                        break;
        }
        token[used] = 0;
        return 1;
}

static int
xargs_command_text_eq(const kword_t *command, const char *text)
{
        unsigned int len;
        unsigned int start;
        unsigned int i;

        len = (unsigned int)(command[0] & 0777777UL);
        start = 0U;
        for (i = 0U; i < len; ++i)
                if (xargs_s6_get(command, i) == '/')
                        start = i + 1U;
        for (i = 0U; start + i < len && text[i] != 0; ++i)
                if (xargs_s6_get(command, start + i) !=
                    (unsigned char)text[i])
                        return 0;
        return start + i == len && text[i] == 0;
}

static int
xargs_map_resolve(const kword_t *command)
{
        struct xargs_reader reader;
        kword_t map_path[U_PATH_WORDS];
        char name[XARGS_MAP_TOKEN_CHARS + 1U];
        char target[XARGS_MAP_TOKEN_CHARS + 1U];
        int fd;
        int rc;

        xargs_s6_clear(map_path);
        if (xargs_path_prefix("/SYSTEM/EXEC/", xargs_echo) != 0)
                return -1;
        xargs_s6_clear(map_path);
        {
                static const char map_name[] = "/SYSTEM/EXEC/MAP";
                unsigned int i;
                for (i = 0U; map_name[i] != 0; ++i)
                        if (xargs_s6_add(map_path, map_name[i]) != 0)
                                return -1;
        }
        fd = dsys_open(map_path, SYS_O_RDONLY);
        if (fd < 0)
                return -1;
        xargs_reader_init(&reader, fd);
        for (;;) {
                rc = xargs_map_token(&reader, name, sizeof(name));
                if (rc < 0)
                        break;
                rc = xargs_map_token(&reader, target, sizeof(target));
                if (rc < 0) {
                        rc = -2;
                        break;
                }
                if (xargs_command_text_eq(command, name)) {
                        unsigned int i;
                        xargs_s6_clear(xargs_path);
                        for (i = 0U; "/SYSTEM/EXEC/"[i] != 0; ++i)
                                if (xargs_s6_add(xargs_path,
                                    "/SYSTEM/EXEC/"[i]) != 0) {
                                        rc = -2;
                                        break;
                                }
                        if (rc == -2)
                                break;
                        for (i = 0U; target[i] != 0; ++i)
                                if (xargs_s6_add(xargs_path, target[i]) != 0) {
                                        rc = -2;
                                        break;
                                }
                        if (rc != -2)
                                rc = 0;
                        break;
                }
        }
        (void)dsys_close(fd);
        return rc == 0 ? 0 : -1;
}

static int
xargs_append_run(unsigned int *usedp, const kword_t *record)
{
        unsigned int words;
        unsigned int i;

        words = xargs_record_words(record);
        if (*usedp + words > XARGS_RUN_WORDS)
                return -1;
        for (i = 0U; i < words; ++i)
                xargs_run[(*usedp)++] = record[i];
        return 0;
}

static int
xargs_run_path(const kword_t *path, unsigned int argc, kword_t **argv,
    kword_t **envp)
{
        struct sys_run_v2 *run;
        kword_t status;
        unsigned int used;
        unsigned int envc;
        unsigned int i;
        int pid;

        used = SYS_RUN_V2_FIXED_WORDS;
        if (xargs_append_run(&used, path) != 0)
                return XARGS_STATUS_ERROR;
        for (i = 0U; i < argc; ++i)
                if (xargs_append_run(&used, argv[i]) != 0)
                        return XARGS_STATUS_ERROR;
        envc = 0U;
        if (envp != 0) {
                while (envp[envc] != 0 && envc < SYS_RUN_ENV_MAX) {
                        if (xargs_append_run(&used, envp[envc]) != 0)
                                return XARGS_STATUS_ERROR;
                        ++envc;
                }
                if (envc == SYS_RUN_ENV_MAX && envp[envc] != 0)
                        return XARGS_STATUS_ERROR;
        }
        if (used + 3U > XARGS_RUN_WORDS)
                return XARGS_STATUS_ERROR;
        xargs_run[used++] = SYS_RUN_FD_MAP(0U, 0U);
        xargs_run[used++] = SYS_RUN_FD_MAP(1U, 1U);
        xargs_run[used++] = SYS_RUN_FD_MAP(2U, 2U);
        run = (struct sys_run_v2 *)xargs_run;
        run->version_words = SYS_RUN_HEADER(SYS_RUN_VERSION_2, used);
        run->flags = SYS_RUN_PGRP_INHERIT;
        run->pgrp = 0UL;
        run->fdmap_count = 3UL;
        run->argc = argc;
        run->envc = envc;
        pid = dsys_run(run);
        if (pid < 0)
                return XARGS_STATUS_NOT_FOUND;
        if (dsys_wait((unsigned int)pid, &status, 0U) != pid ||
            SYS_WAIT_STATUS_KIND(status) != SYS_WAIT_EXITED)
                return XARGS_STATUS_ERROR;
        return (int)SYS_WAIT_STATUS_VALUE(status);
}

static int
xargs_run_search(unsigned int argc, kword_t **argv, kword_t **envp)
{
        struct vfs_stat st;
        unsigned int len;
        unsigned int i;

        if (argc == 0U)
                return 0;
        len = (unsigned int)(argv[0][0] & 0777777UL);
        if (len != 0U && xargs_s6_get(argv[0], 0U) == '/') {
                for (i = 0U; i < U_PATH_WORDS; ++i)
                        xargs_path[i] = argv[0][i];
                if (dsys_stat(xargs_path, &st) == 0)
                        return xargs_run_path(xargs_path, argc, argv, envp);
                if (xargs_map_resolve(argv[0]) == 0)
                        return xargs_run_path(xargs_path, argc, argv, envp);
                return XARGS_STATUS_NOT_FOUND;
        }
        if (xargs_path_prefix("/SYSTEM/EXEC/", argv[0]) == 0 &&
            dsys_stat(xargs_path, &st) == 0)
                return xargs_run_path(xargs_path, argc, argv, envp);
        if (xargs_path_prefix("/OPTION/BASE/EXEC/", argv[0]) == 0 &&
            dsys_stat(xargs_path, &st) == 0)
                return xargs_run_path(xargs_path, argc, argv, envp);
        if (xargs_map_resolve(argv[0]) == 0)
                return xargs_run_path(xargs_path, argc, argv, envp);
        return XARGS_STATUS_NOT_FOUND;
}

static int
xargs_flush(unsigned int *argcp, unsigned int base, int saw_input,
    kword_t **envp)
{
        int rc;

        if (*argcp == base && !saw_input)
                return 0;
        rc = xargs_run_search(*argcp, xargs_vec, envp);
        *argcp = base;
        xargs_arena_used = 0U;
        return rc;
}

static kword_t *
xargs_new_token(void)
{
        kword_t *record;

        if (XARGS_ARENA_WORDS - xargs_arena_used < U_ARG_WORDS)
                return 0;
        record = &xargs_arena[xargs_arena_used];
        xargs_s6_clear(record);
        xargs_arena_used += U_ARG_WORDS;
        return record;
}

int
main(int argc, kword_t **argv, kword_t **envp)
{
        struct xargs_reader reader;
        kword_t *token;
        unsigned int base;
        unsigned int used;
        unsigned int limit;
        unsigned int nflag;
        int argi;
        int ch;
        int in_word;
        int saw_input;
        int rc;

        xargs_s6_clear(xargs_echo);
        if (xargs_s6_add(xargs_echo, 'E') != 0 ||
            xargs_s6_add(xargs_echo, 'C') != 0 ||
            xargs_s6_add(xargs_echo, 'H') != 0 ||
            xargs_s6_add(xargs_echo, 'O') != 0)
                return XARGS_STATUS_ERROR;
        nflag = SYS_RUN_ARG_MAX;
        argi = 1;
        if (argc > 2 && xargs_s6_eq(argv[1], "-N")) {
                if (xargs_s6_uint(argv[2], &nflag) != 0 || nflag == 0U)
                        return 2;
                argi = 3;
        }
        base = 0U;
        if (argi >= argc) {
                xargs_vec[base++] = xargs_echo;
        } else {
                while (argi < argc && base < SYS_RUN_ARG_MAX)
                        xargs_vec[base++] = argv[argi++];
                if (argi != argc)
                        return 2;
        }
        if (nflag > SYS_RUN_ARG_MAX)
                nflag = SYS_RUN_ARG_MAX;
        limit = nflag <= base ? base + 1U : nflag;
        if (limit > SYS_RUN_ARG_MAX)
                limit = SYS_RUN_ARG_MAX;
        used = base;
        xargs_arena_used = 0U;
        token = 0;
        in_word = 0;
        saw_input = 0;
        xargs_reader_init(&reader, 0);
        for (;;) {
                ch = xargs_reader_char(&reader);
                if (ch == -2)
                        return 1;
                if (ch < 0)
                        break;
                if (ch == ' ' || ch == '\t' || ch == '\n') {
                        if (in_word)
                                in_word = 0;
                        continue;
                }
                if (ch < 040 || ch > 0137)
                        return 1;
                if (!in_word) {
                        if (used == limit ||
                            XARGS_ARENA_WORDS - xargs_arena_used <
                            U_ARG_WORDS) {
                                rc = xargs_flush(&used, base, saw_input,
                                    envp);
                                if (rc != 0)
                                        return rc;
                                saw_input = 0;
                        }
                        token = xargs_new_token();
                        if (token == 0 || used >= SYS_RUN_ARG_MAX)
                                return XARGS_STATUS_ERROR;
                        xargs_vec[used++] = token;
                        in_word = 1;
                        saw_input = 1;
                }
                if (xargs_s6_add(token, ch) != 0)
                        return 1;
        }
        return xargs_flush(&used, base, saw_input, envp);
}
