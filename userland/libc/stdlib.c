#include "u.h"

#define LIBC_USER_ADDR_MASK 0777777UL

/* Small word-aligned allocator for the contiguous DAIMOS process VM.
 * malloc sizes are C character units; heap growth is performed in PDP-10
 * words and the kernel rounds only the physical VM extent to APR granularity. */
struct heap_block {
        unsigned int words;
        struct heap_block *next;
};

static struct heap_block *heap_free;

static unsigned int
heap_header_words(void)
{
        return (sizeof(struct heap_block) + sizeof(kword_t) - 1U) /
            sizeof(kword_t);
}

static unsigned int
heap_data_words(unsigned int chars)
{
        return (chars + sizeof(kword_t) - 1U) / sizeof(kword_t);
}

int
abs(int value)
{
        return value < 0 ? -value : value;
}

int
atoi(char *text)
{
        int sign;
        int value;

        if (text == 0)
                return 0;
        while (*text == ' ' || *text == '\t' || *text == '\n' ||
            *text == '\r')
                ++text;
        sign = 1;
        if (*text == '-') {
                sign = -1;
                ++text;
        } else if (*text == '+') {
                ++text;
        }
        value = 0;
        while (*text >= '0' && *text <= '9') {
                value = value * 10 + (*text - '0');
                ++text;
        }
        return sign * value;
}

int
brk(void *address)
{
        kword_t word;

        word = (kword_t)(unsigned long)address;
        return dsys_brk(word) == word ? 0 : -1;
}

void *
sbrk(long increment)
{
        kword_t current;
        kword_t next;
        unsigned long chars;
        unsigned long words;

        current = dsys_brk(0UL);
        if (current == (kword_t)-1L)
                return (void *)-1L;
        if (increment >= 0) {
                chars = (unsigned long)increment;
                words = (chars + sizeof(kword_t) - 1UL) / sizeof(kword_t);
                if (words > LIBC_USER_ADDR_MASK - current)
                        return (void *)-1L;
                next = current + (kword_t)words;
        } else {
                chars = (unsigned long)(-increment);
                words = (chars + sizeof(kword_t) - 1UL) / sizeof(kword_t);
                if (words > current)
                        return (void *)-1L;
                next = current - (kword_t)words;
        }
        if (dsys_brk(next) != next)
                return (void *)-1L;
        return (void *)(unsigned long)current;
}

void *
malloc(unsigned int chars)
{
        struct heap_block **link;
        struct heap_block *block;
        struct heap_block *split;
        unsigned int header;
        unsigned int need;
        unsigned int remain;
        kword_t current;
        kword_t next;

        if (chars == 0U)
                chars = 1U;
        header = heap_header_words();
        need = header + heap_data_words(chars);
        if (need < header)
                return 0;

        link = &heap_free;
        while ((block = *link) != 0) {
                if (block->words >= need) {
                        remain = block->words - need;
                        if (remain > header) {
                                split = (struct heap_block *)
                                    ((kword_t *)block + need);
                                split->words = remain;
                                split->next = block->next;
                                *link = split;
                                block->words = need;
                        } else {
                                *link = block->next;
                        }
                        block->next = 0;
                        return (void *)((kword_t *)block + header);
                }
                link = &block->next;
        }

        current = dsys_brk(0UL);
        if (current == (kword_t)-1L ||
            (kword_t)need > LIBC_USER_ADDR_MASK - current)
                return 0;
        next = current + (kword_t)need;
        if (dsys_brk(next) != next)
                return 0;
        block = (struct heap_block *)(unsigned long)current;
        block->words = need;
        block->next = 0;
        return (void *)((kword_t *)block + header);
}

void
free(void *ptr)
{
        struct heap_block *block;
        struct heap_block *cur;
        struct heap_block *prev;
        unsigned int header;

        if (ptr == 0)
                return;
        header = heap_header_words();
        block = (struct heap_block *)((kword_t *)ptr - header);
        prev = 0;
        cur = heap_free;
        while (cur != 0 && (unsigned long)cur < (unsigned long)block) {
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
                heap_free = block;
        }
}

extern void *memset(void *, int, unsigned int);
extern void *memcpy(void *, void *, unsigned int);

void *
calloc(unsigned int count, unsigned int size)
{
        unsigned int chars;
        void *ptr;

        if (count != 0U && size > (~0U) / count)
                return 0;
        chars = count * size;
        ptr = malloc(chars);
        if (ptr != 0)
                (void)memset(ptr, 0, chars);
        return ptr;
}

void *
realloc(void *ptr, unsigned int chars)
{
        struct heap_block *block;
        void *newptr;
        unsigned int header;
        unsigned int oldchars;

        if (ptr == 0)
                return malloc(chars);
        if (chars == 0U) {
                free(ptr);
                return 0;
        }
        header = heap_header_words();
        block = (struct heap_block *)((kword_t *)ptr - header);
        oldchars = (block->words - header) * sizeof(kword_t);
        if (chars <= oldchars)
                return ptr;
        newptr = malloc(chars);
        if (newptr == 0)
                return 0;
        (void)memcpy(newptr, ptr, oldchars);
        free(ptr);
        return newptr;
}

static void
qsort_swap(unsigned char *a, unsigned char *b, unsigned int size)
{
        unsigned char c;

        while (size-- != 0U) {
                c = *a;
                *a++ = *b;
                *b++ = c;
        }
}

/* In-place heapsort avoids recursion and auxiliary heap consumption. */
void
qsort(void *base, unsigned int count, unsigned int size,
    int (*compare)(const void *, const void *))
{
        unsigned char *bytes;
        unsigned int start;
        unsigned int end;
        unsigned int root;
        unsigned int child;

        if (base == 0 || compare == 0 || count < 2U || size == 0U)
                return;
        bytes = (unsigned char *)base;
        start = count / 2U;
        end = count;
        for (;;) {
                if (start != 0U) {
                        --start;
                        root = start;
                } else {
                        if (--end == 0U)
                                return;
                        qsort_swap(bytes, bytes + end * size, size);
                        root = 0U;
                }
                for (;;) {
                        child = root * 2U + 1U;
                        if (child >= end)
                                break;
                        if (child + 1U < end &&
                            compare(bytes + child * size,
                                bytes + (child + 1U) * size) < 0)
                                ++child;
                        if (compare(bytes + root * size,
                            bytes + child * size) >= 0)
                                break;
                        qsort_swap(bytes + root * size,
                            bytes + child * size, size);
                        root = child;
                }
        }
}
