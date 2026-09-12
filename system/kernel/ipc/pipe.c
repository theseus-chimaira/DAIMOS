#include "pipe.h"
#include "fs_mres.h"
#include "mm.h"
#include "proc.h"

#define PIPE_MM_OWNER        5U
#define PIPE_POS_MASK        0177UL
#define PIPE_COUNT_MASK      0377UL
#define PIPE_HEAD_SHIFT      0U
#define PIPE_TAIL_SHIFT      7U
#define PIPE_COUNT_SHIFT    14U
#define PIPE_REF_MASK      0777777UL
#define PIPE_READERS_SHIFT  18U

struct pipe {
        kword_t next_fifo;
        kword_t fifo_node;
        kword_t state;
        kword_t refs;
        volatile kword_t read_event;
        volatile kword_t write_event;
        kword_t data[PIPE_BUFFER_WORDS];
};

extern struct file *file_table;

kword_t pipe_fifo_head;

static unsigned int
pipe_head(const struct pipe *p)
{
        return (unsigned int)((p->state >> PIPE_HEAD_SHIFT) & PIPE_POS_MASK);
}

static unsigned int
pipe_tail(const struct pipe *p)
{
        return (unsigned int)((p->state >> PIPE_TAIL_SHIFT) & PIPE_POS_MASK);
}

static unsigned int
pipe_count(const struct pipe *p)
{
        return (unsigned int)((p->state >> PIPE_COUNT_SHIFT) & PIPE_COUNT_MASK);
}

static void
pipe_set_state(struct pipe *p, unsigned int head, unsigned int tail,
    unsigned int count)
{
        p->state = ((kword_t)head << PIPE_HEAD_SHIFT) |
            ((kword_t)tail << PIPE_TAIL_SHIFT) |
            ((kword_t)count << PIPE_COUNT_SHIFT);
}

static unsigned int
pipe_readers(const struct pipe *p)
{
        return (unsigned int)((p->refs >> PIPE_READERS_SHIFT) & PIPE_REF_MASK);
}

static unsigned int
pipe_writers(const struct pipe *p)
{
        return (unsigned int)(p->refs & PIPE_REF_MASK);
}

static void
pipe_set_refs(struct pipe *p, unsigned int readers, unsigned int writers)
{
        p->refs = ((kword_t)readers << PIPE_READERS_SHIFT) |
            (kword_t)writers;
}

static struct pipe *
pipe_from_node(vnode_t node)
{
        unsigned int base;

        if (VFS_PROVIDER(node) != PIPE_PROVIDER ||
            VFS_KIND(node) != PIPE_KIND_STREAM)
                return 0;
        base = VFS_INDEX(node);
        if (base == 0U)
                return 0;
        return (struct pipe *)(unsigned long)base;
}

static struct pipe *
pipe_alloc(vnode_t fifo_node)
{
        struct pipe *p;
        kword_t base;

        if (mm_alloc_aligned(PIPE_OBJECT_WORDS, 1UL,
            MM_TYPE_KERNEL_DYNAMIC, PIPE_MM_OWNER, MM_ALLOC_LOW,
            &base) != MM_OK)
                return 0;
        if ((base & ~VFS_INDEX_MASK) != 0UL || base == 0UL) {
                (void)mm_free(base, MM_TYPE_KERNEL_DYNAMIC, PIPE_MM_OWNER);
                return 0;
        }
        p = (struct pipe *)(unsigned long)base;
        fs_zero_words((kword_t *)p, PIPE_OBJECT_WORDS);
        p->fifo_node = fifo_node;
        return p;
}

static vnode_t
pipe_vnode(const struct pipe *p)
{
        return VFS_NODE_PACKED(PIPE_PROVIDER, PIPE_KIND_STREAM,
            (kword_t)(unsigned long)p);
}

static struct pipe *
pipe_fifo_find(vnode_t fifo_node)
{
        struct pipe *p;
        kword_t base;

        base = pipe_fifo_head;
        while (base != 0UL) {
                p = (struct pipe *)(unsigned long)base;
                if (p->fifo_node == fifo_node)
                        return p;
                base = p->next_fifo;
        }
        return 0;
}

static void
pipe_fifo_unlink(struct pipe *p)
{
        struct pipe *q;
        kword_t *link;
        kword_t base;

        link = &pipe_fifo_head;
        while ((base = *link) != 0UL) {
                q = (struct pipe *)(unsigned long)base;
                if (q == p) {
                        *link = q->next_fifo;
                        q->next_fifo = 0UL;
                        return;
                }
                link = &q->next_fifo;
        }
}

static unsigned int
pipe_getchar(const struct pipe *p, unsigned int pos)
{
        unsigned int shift;

        shift = (3U - (pos & 3U)) * 9U;
        return (unsigned int)((p->data[pos >> 2U] >> shift) & 0777UL);
}

static void
pipe_putchar(struct pipe *p, unsigned int pos, unsigned int ch)
{
        unsigned int shift;
        kword_t mask;
        kword_t word;

        shift = (3U - (pos & 3U)) * 9U;
        mask = (kword_t)0777UL << shift;
        word = p->data[pos >> 2U] & ~mask;
        p->data[pos >> 2U] = word | (((kword_t)ch & 0777UL) << shift);
}

kword_t
pipe_create(void)
{
        struct pipe *p;
        unsigned int first;
        unsigned int second;
        unsigned int i;
        vnode_t node;

        if (file_table == 0)
                return ~0UL;
        first = FILE_NFILE;
        second = FILE_NFILE;
        for (i = 0U; i < FILE_NFILE; ++i) {
                if (file_table[i].node_meta != 0UL)
                        continue;
                if (first == FILE_NFILE)
                        first = i;
                else {
                        second = i;
                        break;
                }
        }
        if (second == FILE_NFILE)
                return ~0UL;
        p = pipe_alloc(VFS_NODE_NONE);
        if (p == 0)
                return ~0UL;
        pipe_set_refs(p, 1U, 1U);
        p->write_event = 1UL;
        node = pipe_vnode(p);
        file_table[first].node_meta = node | FILE_META_READ;
        file_table[first].off_chars = 0UL;
        file_table[second].node_meta = node | FILE_META_WRITE;
        file_table[second].off_chars = 0UL;
        return ((kword_t)first << 18U) | (kword_t)second;
}

vnode_t
pipe_fifo_open(vnode_t fifo_node, kword_t node_meta)
{
        struct pipe *p;
        unsigned int readers;
        unsigned int writers;
        int want_read;
        int want_write;

        want_read = (node_meta & FILE_META_READ) != 0UL;
        want_write = (node_meta & FILE_META_WRITE) != 0UL;
        if ((!want_read && !want_write) || fifo_node == VFS_NODE_NONE)
                return VFS_NODE_NONE;
        p = pipe_fifo_find(fifo_node);
        if (p == 0) {
                p = pipe_alloc(fifo_node);
                if (p == 0)
                        return VFS_NODE_NONE;
                p->next_fifo = pipe_fifo_head;
                pipe_fifo_head = (kword_t)(unsigned long)p;
        }
        readers = pipe_readers(p);
        writers = pipe_writers(p);
        if (want_read)
                ++readers;
        if (want_write)
                ++writers;
        pipe_set_refs(p, readers, writers);
        if (want_read) {
                p->write_event = 1UL;
                proc_wakeup_event(&p->write_event);
        }
        if (want_write) {
                p->read_event = 1UL;
                proc_wakeup_event(&p->read_event);
        }
        if (want_read && !want_write && pipe_writers(p) == 0U) {
                /* FIFO open is a rendezvous, not a persistent peer test.
                 * Once a writer has opened, this open must complete even if
                 * that writer closes again before this process is scheduled.
                 * read_event latches that peer transition across the sleep. */
                p->read_event = 0UL;
                if (pipe_writers(p) == 0U)
                        (void)proc_wait_event(&p->read_event);
        } else if (want_write && !want_read && pipe_readers(p) == 0U) {
                /* Symmetric rendezvous for a writer waiting for a reader. */
                p->write_event = 0UL;
                if (pipe_readers(p) == 0U)
                        (void)proc_wait_event(&p->write_event);
        }
        return pipe_vnode(p);
}

void
pipe_fifo_detach(vnode_t fifo_node)
{
        struct pipe *p;

        p = pipe_fifo_find(fifo_node);
        if (p == 0)
                return;
        pipe_fifo_unlink(p);
        p->fifo_node = VFS_NODE_NONE;
}

int
pipe_fifo_mount_busy(unsigned int mount_id)
{
        struct pipe *p;
        kword_t base;

        base = pipe_fifo_head;
        while (base != 0UL) {
                p = (struct pipe *)(unsigned long)base;
                if (VFS_MOUNT_ID(p->fifo_node) == mount_id)
                        return 1;
                base = p->next_fifo;
        }
        return 0;
}

int
pipe_readchar(vnode_t node)
{
        struct pipe *p;
        unsigned int head;
        unsigned int tail;
        unsigned int count;
        unsigned int ch;

        p = pipe_from_node(node);
        if (p == 0)
                return -1;
        for (;;) {
                count = pipe_count(p);
                if (count != 0U) {
                        head = pipe_head(p);
                        tail = pipe_tail(p);
                        ch = pipe_getchar(p, head);
                        head = (head + 1U) & PIPE_POS_MASK;
                        --count;
                        pipe_set_state(p, head, tail, count);
                        p->write_event = 1UL;
                        proc_wakeup_event(&p->write_event);
                        return (int)ch;
                }
                if (pipe_writers(p) == 0U)
                        return -2;
                p->read_event = 0UL;
                if (pipe_count(p) != 0U || pipe_writers(p) == 0U)
                        continue;
                (void)proc_wait_event(&p->read_event);
        }
}

static int
pipe_wait_space(struct pipe *p, unsigned int need)
{
        unsigned int count;

        for (;;) {
                if (pipe_readers(p) == 0U)
                        return -1;
                count = pipe_count(p);
                if (PIPE_BUFFER_CHARS - count >= need)
                        return 0;
                p->write_event = 0UL;
                if (pipe_readers(p) == 0U ||
                    PIPE_BUFFER_CHARS - pipe_count(p) >= need)
                        continue;
                (void)proc_wait_event(&p->write_event);
        }
}

int
pipe_writechar(vnode_t node, unsigned int ch, unsigned int reserve)
{
        struct pipe *p;
        unsigned int head;
        unsigned int tail;
        unsigned int count;

        p = pipe_from_node(node);
        if (p == 0 || reserve == 0U || reserve > PIPE_BUF ||
            pipe_wait_space(p, reserve) != 0)
                return -1;
        count = pipe_count(p);
        head = pipe_head(p);
        tail = pipe_tail(p);
        pipe_putchar(p, tail, ch);
        tail = (tail + 1U) & PIPE_POS_MASK;
        ++count;
        pipe_set_state(p, head, tail, count);
        p->read_event = 1UL;
        proc_wakeup_event(&p->read_event);
        return 0;
}

void
pipe_add_ref(kword_t node_meta)
{
        struct pipe *p;
        vnode_t node;
        unsigned int readers;
        unsigned int writers;

        node = FILE_NODE(node_meta);
        p = pipe_from_node(node);
        if (p == 0)
                return;
        readers = pipe_readers(p);
        writers = pipe_writers(p);
        if ((node_meta & FILE_META_READ) != 0UL)
                ++readers;
        if ((node_meta & FILE_META_WRITE) != 0UL)
                ++writers;
        pipe_set_refs(p, readers, writers);
}

void
pipe_add_refs(struct file *table)
{
        unsigned int i;

        if (table == 0)
                return;
        for (i = 0U; i < FILE_NFILE; ++i) {
                if (table[i].node_meta != 0UL)
                        pipe_add_ref(table[i].node_meta);
        }
}

int
pipe_close_ref(vnode_t node, kword_t node_meta)
{
        struct pipe *p;
        unsigned int readers;
        unsigned int writers;

        p = pipe_from_node(node);
        if (p == 0)
                return -1;
        readers = pipe_readers(p);
        writers = pipe_writers(p);
        if ((node_meta & FILE_META_READ) != 0UL) {
                if (readers == 0U)
                        return -1;
                --readers;
        }
        if ((node_meta & FILE_META_WRITE) != 0UL) {
                if (writers == 0U)
                        return -1;
                --writers;
        }
        pipe_set_refs(p, readers, writers);
        if (readers == 0U) {
                p->write_event = 1UL;
                proc_wakeup_event(&p->write_event);
        }
        if (writers == 0U) {
                p->read_event = 1UL;
                proc_wakeup_event(&p->read_event);
        }
        if (readers == 0U && writers == 0U) {
                if (p->fifo_node != VFS_NODE_NONE)
                        pipe_fifo_unlink(p);
                if (mm_free((kword_t)(unsigned long)p,
                    MM_TYPE_KERNEL_DYNAMIC, PIPE_MM_OWNER) != MM_OK)
                        return -1;
        }
        return 0;
}
