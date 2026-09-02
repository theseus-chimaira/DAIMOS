#include "exec.h"
#include "file.h"
#include "mach_user.h"

#define EXEC_HALF_MASK 0777777UL
#define EXEC_WORD_MASK 0777777777777UL
#define EXEC_DXR_MAGIC \
    ((VFS_SIX6('D','X','R',' ',' ',' ') >> 18) & EXEC_HALF_MASK)

int
exec_load_init(struct proc *p, unsigned int owner,
    const kword_t *path, kword_t base, kword_t limit)
{
        vnode_t node;
        struct vfs_stat st;
        kword_t hdr[EXEC_DXR_HDR_WORDS];
        kword_t *mem;
        kword_t target;
        kword_t process_words;
        kword_t relword;
        unsigned int entry;
        unsigned int image_words;
        unsigned int bss_words;
        unsigned int reloc_words;
        unsigned int i;
        unsigned int rel_index;

        if (p == 0 || path == 0 || base >= limit ||
            file_lookup_path(path, &node) != 0 ||
            vfs_stat(node, &st) != 0 || st.type != VFS_TYPE_REG ||
            st.size_words < EXEC_DXR_HDR_WORDS ||
            vfs_read_words(node, 0U, hdr, EXEC_DXR_HDR_WORDS) !=
            (int)EXEC_DXR_HDR_WORDS ||
            ((hdr[0] >> 18U) & EXEC_HALF_MASK) != EXEC_DXR_MAGIC)
                return -1;

        entry = (unsigned int)(hdr[0] & EXEC_HALF_MASK);
        image_words = (unsigned int)((hdr[1] >> 18U) & EXEC_HALF_MASK);
        bss_words = (unsigned int)(hdr[1] & EXEC_HALF_MASK);
        if (image_words == 0U || image_words > EXEC_DXR_MAX_IMAGE_WORDS ||
            bss_words > EXEC_DXR_MAX_BSS_WORDS || entry >= image_words)
                return -1;
        reloc_words = (image_words + 35U) / 36U;
        if ((kword_t)EXEC_DXR_HDR_WORDS + (kword_t)image_words +
            (kword_t)reloc_words > st.size_words)
                return -1;
        process_words = (kword_t)image_words + (kword_t)bss_words +
            (kword_t)EXEC_DXR_STACK_WORDS;
        if (process_words > EXEC_HALF_MASK ||
            base > limit || process_words > limit - base)
                return -1;

        mem = (kword_t *)(unsigned long)base;
        if (vfs_read_words(node, EXEC_DXR_HDR_WORDS, mem,
            image_words) != (int)image_words)
                return -1;

        relword = 0UL;
        rel_index = (unsigned int)-1;
        for (i = 0U; i < image_words; ++i) {
                unsigned int r;

                r = i / 36U;
                if (r != rel_index) {
                        if (vfs_read_words(node,
                            EXEC_DXR_HDR_WORDS + image_words + r,
                            &relword, 1U) != 1)
                                return -1;
                        rel_index = r;
                }
                if ((relword & ((kword_t)1UL << (35U - (i % 36U)))) != 0)
                        mem[i] = (mem[i] & ~(kword_t)EXEC_HALF_MASK) |
                            ((mem[i] + base) & EXEC_HALF_MASK);
        }
        for (i = 0U; i < bss_words + EXEC_DXR_STACK_WORDS; ++i)
                mem[image_words + i] = 0UL;

        target = EXEC_PDP10_JRST |
            ((kword_t)(unsigned long)mach_syscall_trampoline &
            EXEC_HALF_MASK);
        target &= EXEC_WORD_MASK;
        for (i = 0U; i < image_words; ++i)
                if ((mem[i] & EXEC_WORD_MASK) == EXEC_SYSCALL_MARKER)
                        mem[i] = target;

        p->meta = ((kword_t)owner & PROC_PID_MASK) |
            ((kword_t)PROC_SRUN << PROC_STATE_SHIFT) |
            (((kword_t)entry & PROC_HALF_MASK) << PROC_ENTRY_SHIFT);
        p->mem_layout = ((process_words & PROC_HALF_MASK) <<
            PROC_HALF_SHIFT) | (base & PROC_HALF_MASK);
        return 0;
}
