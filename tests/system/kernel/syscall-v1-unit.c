#include <stdio.h>
#include <string.h>
#include "syscall_v1.h"
#include "devicefs_v1.h"
#include "procfs_v1.h"
#include "proc_v1.h"

#define NODES 16U
#define POOL 32U

static int cty_last = -1;
static unsigned int cty_writes;
kword_t kcore_cty_putchar_v1;
kword_t kcore_cty_getchar_v1;

static int
fake_putchar(int ch)
{
        cty_last = ch;
        ++cty_writes;
        return ch >= 0 ? 0 : -1;
}

static int
fake_getchar(void)
{
        return 'Q';
}

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

static struct proc_v1 *
setup_processes(void)
{
        unsigned int i;

        for (i = 0U; i < PROC_V1_NPROC; ++i) {
                proc_v1_table[i].meta = 0;
                proc_v1_table[i].mem_layout = 0;
        }
        proc_v1_table[0].meta = (kword_t)PROC_V1_SRUN << PROC_V1_STATE_SHIFT;
        proc_v1_table[1].meta = 1U | ((kword_t)PROC_V1_SIDL << PROC_V1_STATE_SHIFT);
        proc_v1_current = &proc_v1_table[0];
        proc_v1_next_pid = 2U;
        return &proc_v1_table[1];
}

static void
setup_ramfs(struct memfs_v1 *fs, struct memfs_v1_node *nodes,
    unsigned int node_count, kword_t *pool, unsigned int pool_words)
{
        unsigned int i;

        for (i = 0U; i < node_count; ++i)
                memset(&nodes[i], 0, sizeof(nodes[i]));
        fs->nodes = nodes;
        fs->node_count = node_count;
        fs->pool = pool;
        fs->pool_words = pool_words;
        fs->used_words = 0U;
        fs->writable = 1;
        fs->image_data = 0;
        nodes[0].meta = ((kword_t)VFS_V1_TYPE_DIR << 15U) |
            ((kword_t)0777U << 3U) | MEMFS_V1_F_USED | MEMFS_V1_F_WRITABLE;
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
        struct vfs_v1_stat st;
        struct sys_v1_meminfo mi;
        struct sys_v1_procinfo pi;
        struct proc_v1 *initp;
        int fd;

        setup_ramfs(&fs, nodes, NODES, pool, POOL);
        devicefs_v1_present = DEVICEFS_V1_PRESENT(DEVICEFS_V1_DEV_CTY0);
        initp = setup_processes();
        if (initp == 0)
                return 1;
        proc_v1_set_memory(initp, 01000UL, 0200UL);
        file_v1_root = &fs;
        file_v1_alias_node = VFS_V1_NODE_NONE;
        sys_v1_total_words = 0400000UL;
        sys_v1_resident_words = 0700UL;
        kcore_cty_putchar_v1 = (kword_t)(unsigned long)fake_putchar;
        kcore_cty_getchar_v1 = (kword_t)(unsigned long)fake_getchar;
        pack_path("/WORDS", path, 6U);
        in[0] = 012345670123UL;
        in[1] = 076543210765UL;
        fd = sys_v1_open(1U, path, SYS_V1_O_WRONLY | SYS_V1_O_CREAT |
            SYS_V1_O_TRUNC);
        if (fd < 0 || file_v1_write_words(1U, fd, in, 2U, 8U) != 2 ||
            file_v1_close(1U, fd) != 0)
                return 2;
        fd = sys_v1_open(1U, path, SYS_V1_O_RDONLY);
        if (fd < 0 || file_v1_read_words(1U, fd, out, 2U) != 2 ||
            out[0] != in[0] || out[1] != in[1] || file_v1_close(1U, fd) != 0)
                return 3;
        if (file_v1_stat_path_owner(1U, path, &st) != 0 || st.size_chars != 8U ||
            st.size_words != 2U || st.type != VFS_V1_TYPE_REG)
                return 4;
        if (sys_v1_meminfo(&mi) != 0 || mi.total_words != 0400000UL ||
            mi.resident_words != 0700UL || mi.process_words != 0200UL ||
            mi.process_slots_used != 2U)
                return 5;
        if (sys_v1_procinfo(1U, &pi) != 0 || pi.pid != 1U ||
            pi.words != 0200UL)
                return 6;
        pack_path("/DEVICE/CTY0/IO", path, 6U);
        fd = sys_v1_open(1U, path, SYS_V1_O_WRONLY | SYS_V1_O_CREAT |
            SYS_V1_O_TRUNC);
        if (fd < 0 || sys_v1_writechar(1U, fd, 'T') != 0 ||
            cty_writes != 1U || cty_last != 'T' ||
            file_v1_close(1U, fd) != 0)
                return 7;
        fd = sys_v1_open(1U, path, SYS_V1_O_RDONLY);
        if (fd < 0 || sys_v1_readchar(1U, fd) != 'Q' ||
            file_v1_close(1U, fd) != 0)
                return 8;
        puts("SYSCALL file v1 unit test PASS");
        return 0;
}
