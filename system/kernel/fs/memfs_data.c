/**
 * @file memfs_data.c
 * @brief Demand allocator for mutable MEMFS file contents.
 *
 * MEMFS owns at most four MM chunks and suballocates file extents inside them.
 * Free-list headers live in free storage, so an empty filesystem carries no
 * data-allocation metadata outside this small MRES state.  A file occupies one
 * contiguous extent; growth may relocate that file but never compacts other
 * files.  Completely free chunks are returned immediately to MM.
 */
#include "memfs.h"
#include "mm.h"

#define MEMFS_DATA_CHUNKS       4U
#define MEMFS_DATA_CHUNK_WORDS  02000UL
#define MEMFS_DATA_MM_OWNER     011U
#define MEMFS_HALF_MASK         0777777UL

struct memfs_data_chunk {
        kword_t base;
        kword_t words;
        kword_t free;
};

static struct memfs_data_chunk memfs_data_chunks[MEMFS_DATA_CHUNKS];
static kword_t memfs_data_limit;
static kword_t memfs_data_capacity;

static kword_t
memfs_alloc_words(kword_t words)
{
        if (words < 2UL)
                return 2UL;
        return (words + 1UL) & ~1UL;
}

void
memfs_data_init(kword_t limit)
{
        unsigned int i;

        for (i = 0U; i < MEMFS_DATA_CHUNKS; ++i) {
                memfs_data_chunks[i].base = 0UL;
                memfs_data_chunks[i].words = 0UL;
                memfs_data_chunks[i].free = 0UL;
        }
        memfs_data_limit = limit & ~1UL;
        memfs_data_capacity = 0UL;
}

static int
memfs_chunk_alloc(struct memfs_data_chunk *cp, kword_t need, kword_t *basep)
{
        kword_t cur;
        kword_t prev;

        prev = 0UL;
        for (cur = cp->free; cur != 0UL; cur = ((kword_t *)(unsigned long)cur)[1]) {
                kword_t *hp;
                kword_t size;

                hp = (kword_t *)(unsigned long)cur;
                size = hp[0];
                if (size >= need) {
                        if (size == need) {
                                if (prev == 0UL)
                                        cp->free = hp[1];
                                else
                                        ((kword_t *)(unsigned long)prev)[1] = hp[1];
                                *basep = cur;
                        } else {
                                hp[0] = size - need;
                                *basep = cur + size - need;
                        }
                        return 0;
                }
                prev = cur;
        }
        return -1;
}

static int
memfs_data_alloc(kword_t words, kword_t *basep)
{
        struct memfs_data_chunk *cp;
        kword_t base;
        kword_t chunk_words;
        kword_t need;
        kword_t remaining;
        unsigned int i;

        need = memfs_alloc_words(words);
        for (i = 0U; i < MEMFS_DATA_CHUNKS; ++i) {
                cp = &memfs_data_chunks[i];
                if (cp->base != 0UL && memfs_chunk_alloc(cp, need, basep) == 0)
                        return 0;
        }
        if (need > memfs_data_limit - memfs_data_capacity)
                return -1;
        for (i = 0U; i < MEMFS_DATA_CHUNKS; ++i) {
                if (memfs_data_chunks[i].base == 0UL)
                        break;
        }
        if (i == MEMFS_DATA_CHUNKS)
                return -1;
        remaining = memfs_data_limit - memfs_data_capacity;
        chunk_words = need > MEMFS_DATA_CHUNK_WORDS ? need : MEMFS_DATA_CHUNK_WORDS;
        if (chunk_words > remaining)
                chunk_words = remaining & ~1UL;
        if (chunk_words < need || mm_alloc(chunk_words, MM_TYPE_KERNEL_DYNAMIC,
            MEMFS_DATA_MM_OWNER, MM_ALLOC_LOW, &base) != MM_OK)
                return -1;
        cp = &memfs_data_chunks[i];
        cp->base = base;
        cp->words = chunk_words;
        cp->free = base;
        ((kword_t *)(unsigned long)base)[0] = chunk_words;
        ((kword_t *)(unsigned long)base)[1] = 0UL;
        memfs_data_capacity += chunk_words;
        return memfs_chunk_alloc(cp, need, basep);
}

static void
memfs_data_free(kword_t base, kword_t words)
{
        struct memfs_data_chunk *cp;
        kword_t cur;
        kword_t next;
        kword_t prev;
        kword_t size;
        kword_t *hp;
        unsigned int i;

        size = memfs_alloc_words(words);
        for (i = 0U; i < MEMFS_DATA_CHUNKS; ++i) {
                cp = &memfs_data_chunks[i];
                if (cp->base != 0UL && base >= cp->base &&
                    base + size <= cp->base + cp->words)
                        break;
        }
        if (i == MEMFS_DATA_CHUNKS)
                return;
        prev = 0UL;
        cur = cp->free;
        while (cur != 0UL && cur < base) {
                prev = cur;
                cur = ((kword_t *)(unsigned long)cur)[1];
        }
        hp = (kword_t *)(unsigned long)base;
        hp[0] = size;
        hp[1] = cur;
        if (prev == 0UL)
                cp->free = base;
        else
                ((kword_t *)(unsigned long)prev)[1] = base;

        next = hp[1];
        if (next != 0UL && base + hp[0] == next) {
                hp[0] += ((kword_t *)(unsigned long)next)[0];
                hp[1] = ((kword_t *)(unsigned long)next)[1];
        }
        if (prev != 0UL) {
                kword_t *pp;

                pp = (kword_t *)(unsigned long)prev;
                if (prev + pp[0] == base) {
                        pp[0] += hp[0];
                        pp[1] = hp[1];
                        hp = pp;
                }
        }
        if (cp->free == cp->base && hp == (kword_t *)(unsigned long)cp->base &&
            hp[0] == cp->words && hp[1] == 0UL) {
                kword_t chunk_base;
                kword_t chunk_words;

                chunk_base = cp->base;
                chunk_words = cp->words;
                cp->base = 0UL;
                cp->words = 0UL;
                cp->free = 0UL;
                memfs_data_capacity -= chunk_words;
                (void)mm_free(chunk_base, MM_TYPE_KERNEL_DYNAMIC,
                    MEMFS_DATA_MM_OWNER);
        }
}

int
memfs_resize(struct memfs *fs, unsigned int slot, unsigned int words)
{
        struct memfs_node *np;
        kword_t base;
        kword_t oldbase;
        unsigned int copy;
        unsigned int oldwords;
        unsigned int i;

        if (fs == 0 || slot >= fs->node_count || words > MEMFS_HALF_MASK)
                return -1;
        np = &fs->nodes[slot];
        if ((np->meta & 06UL) != MEMFS_F_WRITABLE)
                return -1;
        oldwords = (unsigned int)(np->data & MEMFS_HALF_MASK);
        oldbase = (np->data >> 18U) & MEMFS_HALF_MASK;
        if (words == oldwords)
                return 0;
        if (words == 0U) {
                if (oldbase != 0UL)
                        memfs_data_free(oldbase, oldwords);
                np->data = 0UL;
                fs->used_words -= oldwords;
                return 0;
        }
        if (memfs_data_alloc((kword_t)words, &base) != 0)
                return -1;
        copy = oldwords < words ? oldwords : words;
        for (i = 0U; i < copy; ++i)
                ((kword_t *)(unsigned long)base)[i] =
                    ((kword_t *)(unsigned long)oldbase)[i];
        for (; i < words; ++i)
                ((kword_t *)(unsigned long)base)[i] = 0UL;
        if (oldbase != 0UL)
                memfs_data_free(oldbase, oldwords);
        np->data = ((base & MEMFS_HALF_MASK) << 18U) |
            ((kword_t)words & MEMFS_HALF_MASK);
        fs->used_words = fs->used_words - oldwords + words;
        return 0;
}

void
memfs_data_destroy(void)
{
        unsigned int i;

        for (i = 0U; i < MEMFS_DATA_CHUNKS; ++i) {
                if (memfs_data_chunks[i].base != 0UL)
                        (void)mm_free(memfs_data_chunks[i].base,
                            MM_TYPE_KERNEL_DYNAMIC, MEMFS_DATA_MM_OWNER);
        }
        memfs_data_init(0UL);
}
