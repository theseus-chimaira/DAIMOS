#include "dsh_heap.h"

/* One shared heap owns persistent variable payloads and temporary argv.
 * Reclaimed blocks are sorted and coalesced, protecting active payloads
 * when a nested evaluation finishes.  Sizes are measured in PDP-10 words. */
struct dsh_heap_block {
        unsigned int words;
        struct dsh_heap_block *next;
};
static struct dsh_heap_block *dsh_heap_free_list;

kword_t *
dsh_heap_alloc(unsigned int words)
{
        struct dsh_heap_block **link;
        struct dsh_heap_block *block;
        struct dsh_heap_block *tail;
        unsigned int need;
        unsigned int remain;
        kword_t at;

        /* Every caller requests at least one payload word. */
        if (words > 0777777U - 2U)
                return 0;
        need = words + 2U;
        link = &dsh_heap_free_list;
        while ((block = *link) != 0) {
                if (block->words >= need) {
                        remain = block->words - need;
                        if (remain > 2U) {
                                tail = (struct dsh_heap_block *)
                                    ((kword_t *)block + need);
                                tail->words = remain;
                                tail->next = block->next;
                                *link = tail;
                                block->words = need;
                        } else {
                                *link = block->next;
                        }
                        return (kword_t *)block + 2U;
                }
                link = &block->next;
        }
        at = dsys_brk(0UL);
        if (at == (kword_t)-1L || (kword_t)need > 0777777UL - at ||
            dsys_brk(at + need) != at + need)
                return 0;
        block = (struct dsh_heap_block *)(unsigned long)at;
        block->words = need;
        block->next = 0;
        return (kword_t *)block + 2U;
}

void
dsh_heap_free(kword_t *payload)
{
        struct dsh_heap_block *block;
        struct dsh_heap_block *cur;
        struct dsh_heap_block *prev;

        if (payload == 0)
                return;
        block = (struct dsh_heap_block *)(payload - 2U);
        prev = 0;
        cur = dsh_heap_free_list;
        while (cur != 0 && cur < block) {
                prev = cur;
                cur = cur->next;
        }
        block->next = cur;
        if (cur != 0 && (kword_t *)block + block->words == (kword_t *)cur) {
                block->words += cur->words;
                block->next = cur->next;
        }
        if (prev != 0 && (kword_t *)prev + prev->words == (kword_t *)block) {
                prev->words += block->words;
                prev->next = block->next;
        } else if (prev != 0) {
                prev->next = block;
        } else {
                dsh_heap_free_list = block;
        }
}
