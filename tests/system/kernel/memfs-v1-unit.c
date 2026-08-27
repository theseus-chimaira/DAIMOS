#include <stdio.h>
#include <string.h>

#include "memfs_v1.h"

#define CHECK(x) do { if (!(x)) { \
        fprintf(stderr, "memfs-v1-unit:%d: %s\n", __LINE__, #x); \
        return 1; \
} } while (0)

static int
name6(struct vfs_v1_name *name, kword_t word, unsigned int chars)
{
        return vfs_v1_name_set6(name, word, chars);
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

        setup_ramfs(&fs, nodes, 12U, pool, 32U);
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


int
main(void)
{
        if (test_ramfs() != 0)
                return 1;
        puts("MEMFS steady-state v1 unit test PASS");
        return 0;
}
