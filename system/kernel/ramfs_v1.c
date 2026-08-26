#include "ramfs_v1.h"

int
ramfs_v1_init(struct memfs_v1 *fs, struct memfs_v1_node *nodes,
    unsigned int node_count, kword_t *pool, unsigned int pool_words)
{
        return memfs_v1_init(fs, nodes, node_count, pool, pool_words, 1);
}
