#include <stdio.h>
#include <string.h>

#include "initfs_v1.h"
#include "ramfs_v1.h"

#define CHECK(x) do { if (!(x)) { \
        fprintf(stderr, "memfs-v1-unit:%d: %s\n", __LINE__, #x); \
        return 1; \
} } while (0)

static int
name6(struct vfs_v1_name *name, kword_t word, unsigned int chars)
{
        return vfs_v1_name_set6(name, word, chars);
}

static unsigned int
pack_nonets(kword_t *out, const char *text)
{
        unsigned int n;
        unsigned int i;
        unsigned int wi;
        unsigned int shift;

        n = (unsigned int)strlen(text) + 1U;
        for (i = 0U; i < (n + 3U) / 4U; ++i)
                out[i] = 0;
        for (i = 0U; i < n; ++i) {
                unsigned int c;

                c = i + 1U == n ? 0U : (unsigned int)(unsigned char)text[i];
                wi = i / 4U;
                shift = (3U - (i & 3U)) * 9U;
                out[wi] |= ((kword_t)c & 0777UL) << shift;
        }
        return (n + 3U) / 4U;
}

static int
test_ramfs(void)
{
        struct memfs_v1 fs;
        struct memfs_v1_node nodes[12];
        kword_t pool[32];
        struct vfs_v1_name a;
        struct vfs_v1_name b;
        struct vfs_v1_name dir;
        vnode_v1_t root;
        vnode_v1_t av;
        vnode_v1_t bv;
        vnode_v1_t dv;
        kword_t aw[4];
        kword_t bw[2];
        kword_t got[4];
        struct vfs_v1_stat st;

        CHECK(ramfs_v1_init(&fs, nodes, 12U, pool, 32U) == 0);
        root = memfs_v1_root(&fs);
        CHECK(root != VFS_V1_NODE_NONE);
        CHECK(name6(&a, VFS_V1_SIX6('A',' ',' ',' ',' ',' '), 1U) == 0);
        CHECK(name6(&b, VFS_V1_SIX6('B',' ',' ',' ',' ',' '), 1U) == 0);
        CHECK(name6(&dir, VFS_V1_SIX6('T','M','P',' ',' ',' '), 3U) == 0);
        CHECK(memfs_v1_create(&fs, root, &a, 0666U, &av) == 0);
        CHECK(memfs_v1_create(&fs, root, &b, 0666U, &bv) == 0);
        aw[0] = 0111UL;
        aw[1] = 0222UL;
        bw[0] = 0777001UL;
        bw[1] = 0777002UL;
        CHECK(memfs_v1_write_words(&fs, av, 0U, aw, 2U, 8U) == 2);
        CHECK(memfs_v1_write_words(&fs, bv, 0U, bw, 2U, 8U) == 2);

        /* Growing A compacts B upward; B's contents must survive. */
        aw[2] = 0333UL;
        aw[3] = 0444UL;
        CHECK(memfs_v1_write_words(&fs, av, 2U, aw + 2, 2U, 16U) == 2);
        CHECK(memfs_v1_read_words(&fs, bv, 0U, got, 2U) == 2);
        CHECK(got[0] == bw[0] && got[1] == bw[1]);

        /* Shrinking A compacts B downward and updates metadata. */
        CHECK(memfs_v1_truncate_words(&fs, av, 1U, 4U) == 0);
        CHECK(memfs_v1_read_words(&fs, bv, 0U, got, 2U) == 2);
        CHECK(got[0] == bw[0] && got[1] == bw[1]);
        CHECK(memfs_v1_stat(&fs, av, &st) == 0);
        CHECK(st.size_words == 1U && st.size_chars == 4U);

        CHECK(memfs_v1_mkdir(&fs, root, &dir, 0777U, &dv) == 0);
        CHECK(memfs_v1_stat(&fs, dv, &st) == 0 && st.type == VFS_V1_TYPE_DIR);
        CHECK(memfs_v1_unlink(&fs, root, &b) == 0);
        CHECK(memfs_v1_lookup(&fs, root, &b, &bv) != 0);
        return 0;
}

static int
test_initfs(void)
{
        kword_t image[32];
        kword_t strings[4];
        unsigned int sw;
        unsigned int i;
        unsigned int base;
        struct memfs_v1 fs;
        struct memfs_v1_node nodes[6];
        struct vfs_v1_name name;
        vnode_v1_t root;
        vnode_v1_t system;
        vnode_v1_t hello;
        kword_t got[2];
        struct vfs_v1_stat st;

        for (i = 0U; i < 32U; ++i)
                image[i] = 0;
        sw = pack_nonets(strings, "SYSTEM");
        CHECK(sw == 2U);
        sw += pack_nonets(strings + sw, "HELLO");
        CHECK(sw == 4U);

        image[0] = INITFS_V1_MAGIC;
        image[1] = INITFS_V1_VERSION;
        image[2] = 2U;
        image[3] = INITFS_V1_ENT_WORDS;
        image[4] = sw;
        image[5] = 2U;

        base = INITFS_V1_HDR_WORDS;
        image[base + 0U] = 0U;
        image[base + 1U] = INITFS_V1_DIR;
        image[base + 2U] = 0555U;
        image[base + 6U] = 0U;

        base += INITFS_V1_ENT_WORDS;
        image[base + 0U] = 8U;
        image[base + 1U] = INITFS_V1_REG;
        image[base + 2U] = 0444U;
        image[base + 3U] = 0U;
        image[base + 4U] = 2U;
        image[base + 5U] = 8U;
        image[base + 6U] = 1U;

        base = INITFS_V1_HDR_WORDS + 2U * INITFS_V1_ENT_WORDS;
        for (i = 0U; i < sw; ++i)
                image[base + i] = strings[i];
        image[base + sw + 0U] = 012345670123UL;
        image[base + sw + 1U] = 076543210765UL;

        CHECK(initfs_v1_mount(&fs, nodes, 6U, image, base + sw + 2U) == 0);
        root = memfs_v1_root(&fs);
        CHECK(name6(&name, VFS_V1_SIX6('S','Y','S','T','E','M'), 6U) == 0);
        CHECK(memfs_v1_lookup(&fs, root, &name, &system) == 0);
        CHECK(name6(&name, VFS_V1_SIX6('H','E','L','L','O',' '), 5U) == 0);
        CHECK(memfs_v1_lookup(&fs, system, &name, &hello) == 0);
        CHECK(memfs_v1_read_words(&fs, hello, 0U, got, 2U) == 2);
        CHECK(got[0] == 012345670123UL && got[1] == 076543210765UL);
        CHECK(memfs_v1_stat(&fs, hello, &st) == 0);
        CHECK(st.type == VFS_V1_TYPE_REG && st.mode == 0444U &&
            st.size_words == 2U && st.size_chars == 8U);
        CHECK(memfs_v1_write_words(&fs, hello, 0U, got, 1U, 4U) < 0);
        return 0;
}

int
main(void)
{
        if (test_ramfs() != 0 || test_initfs() != 0)
                return 1;
        puts("MEMFS/INITFS/RAMFS v1 unit test PASS");
        return 0;
}
