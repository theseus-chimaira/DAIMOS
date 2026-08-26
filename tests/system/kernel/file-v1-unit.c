#include <stdio.h>
#include <string.h>
#include "file_v1.h"
#include "ramfs_v1.h"
#include "devicefs_v1.h"
#include "procfs_v1.h"

#define NODES 24U
#define POOL 64U

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
name_eq(const struct vfs_v1_name *name, const char *s)
{
        kword_t p[6];
        struct vfs_v1_name want;
        unsigned int i;
        unsigned int n;

        pack_path(s, p, 6U);
        n = (unsigned int)p[0];
        want.chars = n;
        for (i = 0U; i < VFS_V1_NAME_WORDS; ++i)
                want.words[i] = p[1U + i];
        if (name->chars != want.chars)
                return 0;
        for (i = 0U; i < VFS_V1_NAME_WORDS; ++i) {
                if (name->words[i] != want.words[i])
                        return 0;
        }
        return 1;
}

static int
proc_get(unsigned int slot, unsigned int field, kword_t *valuep)
{
        if (slot != 1U)
                return -1;
        *valuep = field == PROCFS_V1_FIELD_PID ? 1UL : 0UL;
        return 0;
}

int
main(void)
{
        struct memfs_v1 fs;
        struct memfs_v1_node nodes[NODES];
        kword_t pool[POOL];
        kword_t p_file[6];
        kword_t p_dir[6];
        kword_t p_nested[6];
        kword_t p_root[2];
        kword_t p_device[4];
        kword_t p_proc[4];
        kword_t p_rel[3];
        kword_t p_rel2[3];
        kword_t cwd[6];
        kword_t want_cwd[6];
        struct vfs_v1_stat st;
        struct vfs_v1_dirent ent;
        char buf[16];
        int fd;
        int rc;
        int saw_device;
        int saw_proc;

        if (ramfs_v1_init(&fs, nodes, NODES, pool, POOL) != 0)
                return 1;
        devicefs_v1_init(DEVICEFS_V1_PRESENT(DEVICEFS_V1_DEV_CTY0));
        procfs_v1_init(4U, proc_get);
        file_v1_init(&fs);

        pack_path("/HELLO", p_file, 6U);
        fd = file_v1_open(1U, p_file, FILE_V1_O_WRITE | FILE_V1_O_CREAT |
            FILE_V1_O_TRUNC);
        if (fd < 0 || file_v1_write(1U, fd, "ABCDEF", 6U) != 6 ||
            file_v1_close(1U, fd) != 0)
                return 2;
        fd = file_v1_open(1U, p_file, FILE_V1_O_READ);
        if (fd < 0)
                return 3;
        memset(buf, 0, sizeof(buf));
        if (file_v1_read(1U, fd, buf, 6U) != 6 || memcmp(buf, "ABCDEF", 6U) != 0)
                return 4;
        if (file_v1_close(1U, fd) != 0)
                return 5;

        if (file_v1_truncate(p_file, 3U) != 0 ||
            file_v1_stat_path(p_file, &st) != 0 || st.size_chars != 3U)
                return 6;

        pack_path("/TMP", p_dir, 6U);
        pack_path("/TMP/X", p_nested, 6U);
        if (file_v1_mkdir(p_dir, 0777U) != 0)
                return 7;
        fd = file_v1_open(1U, p_nested, FILE_V1_O_WRITE | FILE_V1_O_CREAT);
        if (fd < 0 || file_v1_write(1U, fd, "Z", 1U) != 1 ||
            file_v1_close(1U, fd) != 0)
                return 8;

        pack_path("/DEVICE", p_device, 4U);
        if (file_v1_stat_path(p_device, &st) != 0 || st.type != VFS_V1_TYPE_DIR)
                return 9;
        pack_path("/PROC", p_proc, 4U);
        if (file_v1_stat_path(p_proc, &st) != 0 || st.type != VFS_V1_TYPE_DIR)
                return 10;

        pack_path("/", p_root, 2U);
        fd = file_v1_open(1U, p_root, FILE_V1_O_READ);
        if (fd < 0)
                return 11;
        saw_device = 0;
        saw_proc = 0;
        while ((rc = file_v1_readdir(1U, fd, &ent)) > 0) {
                if (name_eq(&ent.name, "DEVICE"))
                        saw_device = 1;
                if (name_eq(&ent.name, "PROC"))
                        saw_proc = 1;
        }
        if (rc < 0 || !saw_device || !saw_proc)
                return 12;
        if (file_v1_close(1U, fd) != 0)
                return 13;

        pack_path("R", p_rel, 3U);
        pack_path("S", p_rel2, 3U);
        if (file_v1_chdir(1U, p_dir) != 0 ||
            file_v1_getcwd(1U, cwd, 6U) != 0)
                return 14;
        pack_path("/TMP", want_cwd, 6U);
        if (memcmp(cwd, want_cwd, sizeof(cwd)) != 0)
                return 15;
        fd = file_v1_open(1U, p_rel, FILE_V1_O_WRITE | FILE_V1_O_CREAT);
        if (fd < 0 || file_v1_close(1U, fd) != 0 ||
            file_v1_rename(1U, p_rel, p_rel2) != 0 ||
            file_v1_stat_path_owner(1U, p_rel2, &st) != 0)
                return 16;
        if (file_v1_unlink_owner(1U, p_rel2) != 0 ||
            file_v1_chdir(1U, p_root) != 0)
                return 17;
        if (file_v1_unlink(p_nested) != 0 || file_v1_unlink(p_file) != 0)
                return 18;

        puts("FILE/VFS v1 unit test PASS");
        return 0;
}
