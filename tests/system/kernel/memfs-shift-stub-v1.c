#include "memfs_v1.h"

void
memfs_v1_shift_after(struct memfs_v1 *fs, unsigned int start,
    int delta, unsigned int exclude)
{
        unsigned int i;
        unsigned int off;
        struct memfs_v1_node *np;

        for (i = 1U; i < fs->node_count; ++i) {
                np = &fs->nodes[i];
                if (i == exclude || (np->meta & MEMFS_V1_F_USED) == 0 ||
                    (np->meta & MEMFS_V1_F_IMAGE) != 0)
                        continue;
                off = (unsigned int)((np->data >> 18U) & 0777777UL);
                if (off < start)
                        continue;
                off = (unsigned int)((int)off + delta);
                np->data = (np->data & 0777777UL) | ((kword_t)off << 18U);
        }
}
