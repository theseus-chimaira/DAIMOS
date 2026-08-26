#include <stdio.h>
#include <string.h>
#include "syscall_v1.h"
#include "ramfs_v1.h"
#include "devicefs_v1.h"
#include "procfs_v1.h"

#define NODES 16U
#define POOL 32U

static void
pack_path(const char *s, kword_t *out, unsigned int words)
{
        unsigned int i;
        unsigned int n;
        unsigned int wi;
        unsigned int shift;

        for (i = 0U; i < words; ++i)
                out[i] = 0;
        n = (unsigned int)strlen(s);
        out[0] = n;
        for (i = 0U; i < n; ++i) {
                wi = 1U + i / 6U;
                shift = 30U - (i % 6U) * 6U;
                out[wi] |= ((kword_t)(((unsigned int)s[i] - 040U) & 077U)) << shift;
        }
}

static int
proc_get(unsigned int slot, unsigned int field, kword_t *valuep)
{
        (void)field;
        if (slot != 1U)
                return -1;
        *valuep = 1U;
        return 0;
}

int
main(void)
{
        struct memfs_v1 fs;
        struct memfs_v1_node nodes[NODES];
        kword_t pool[POOL];
        kword_t path[6];
        kword_t in[2];
        kword_t out[2];
        struct sys_v1_stat st;
        int fd;

        if (ramfs_v1_init(&fs, nodes, NODES, pool, POOL) != 0)
                return 1;
        devicefs_v1_init(0);
        procfs_v1_init(2U, proc_get);
        file_v1_init(&fs);
        pack_path("/WORDS", path, 6U);
        in[0] = 012345670123UL;
        in[1] = 076543210765UL;
        fd = sys_v1_open(1U, path, SYS_V1_O_WRONLY | SYS_V1_O_CREAT |
            SYS_V1_O_TRUNC);
        if (fd < 0 || sys_v1_write_words(1U, fd, in, 2U, 8U) != 2 ||
            sys_v1_close(1U, fd) != 0)
                return 2;
        fd = sys_v1_open(1U, path, SYS_V1_O_RDONLY);
        if (fd < 0 || sys_v1_read_words(1U, fd, out, 2U) != 2 ||
            out[0] != in[0] || out[1] != in[1] || sys_v1_close(1U, fd) != 0)
                return 3;
        if (sys_v1_stat_path(path, &st) != 0 || st.size_chars != 8U ||
            st.size_words != 2U || st.type != VFS_V1_TYPE_REG)
                return 4;
        puts("SYSCALL file v1 unit test PASS");
        return 0;
}
