#include "exec.h"
#include "file.h"
#include "mach_user.h"
#include "mm.h"
#include "proc_swap.h"

#define EXEC_HALF_MASK 0777777UL
#define EXEC_WORD_MASK 0777777777777UL
#define EXEC_DXR_MAGIC \
    ((VFS_SIX6('D','X','R',' ',' ',' ') >> 18) & EXEC_HALF_MASK)

int
exec_load_init(struct proc *p, unsigned int owner,
    const kword_t *path)
{
        vnode_t node;
        struct vfs_stat st;
        kword_t hdr[EXEC_DXR_EXT_HDR_WORDS];
        kword_t *mem;
        kword_t process_words;
        kword_t alloc_words;
        kword_t base;
        unsigned int entry;
        unsigned int image_words;
        unsigned int text_words;
        unsigned int bss_words;
        unsigned int header_words;
        unsigned int dxr_flags;
        unsigned int reloc_words;
        unsigned int i;

        if (p == 0 || path == 0 ||
            file_lookup_path(path, &node) != 0 ||
            vfs_stat(node, &st) != 0 || st.type != VFS_TYPE_REG ||
            st.size_words < EXEC_DXR_BASE_HDR_WORDS ||
            vfs_read_words(node, 0U, hdr, EXEC_DXR_BASE_HDR_WORDS) !=
            (int)EXEC_DXR_BASE_HDR_WORDS ||
            ((hdr[0] >> 18U) & EXEC_HALF_MASK) != EXEC_DXR_MAGIC)
                return -1;

        entry = (unsigned int)(hdr[0] & EXEC_HALF_MASK);
        image_words = (unsigned int)((hdr[1] >> 18U) & EXEC_HALF_MASK);
        bss_words = (unsigned int)(hdr[1] & EXEC_DXR_BSS_MASK);
        dxr_flags = (unsigned int)(hdr[1] &
            (EXEC_DXR_F_PURE | EXEC_DXR_F_IMPURE));
        if (image_words == 0U || image_words > EXEC_DXR_MAX_IMAGE_WORDS ||
            bss_words > EXEC_DXR_MAX_BSS_WORDS ||
            (kword_t)entry >= (kword_t)image_words ||
            dxr_flags == (EXEC_DXR_F_PURE | EXEC_DXR_F_IMPURE))
                return -1;
        reloc_words = (image_words + 35U) / 36U;
        if (st.size_words == (kword_t)EXEC_DXR_BASE_HDR_WORDS +
            (kword_t)image_words + (kword_t)reloc_words) {
                header_words = EXEC_DXR_BASE_HDR_WORDS;
                text_words = 0U;
        } else if (st.size_words == (kword_t)EXEC_DXR_EXT_HDR_WORDS +
            (kword_t)image_words + (kword_t)reloc_words) {
                if (vfs_read_words(node, 2U, &hdr[2], 1U) != 1 ||
                    (unsigned int)(hdr[2] & EXEC_HALF_MASK) !=
                    EXEC_DXR_TEXT_TAG)
                        return -1;
                text_words = (unsigned int)((hdr[2] >> 18U) & EXEC_HALF_MASK);
                if (text_words > image_words)
                        return -1;
                header_words = EXEC_DXR_EXT_HDR_WORDS;
        } else {
                return -1;
        }
        process_words = (kword_t)EXEC_USER_ORIGIN +
            (kword_t)image_words + (kword_t)bss_words +
            (kword_t)EXEC_DXR_STACK_WORDS;
        if (process_words > EXEC_HALF_MASK)
                return -1;
        alloc_words = (process_words + EXEC_PDP6_ALIGN_WORDS - 1U) &
            ~((kword_t)EXEC_PDP6_ALIGN_WORDS - 1UL);
        if (alloc_words > EXEC_HALF_MASK ||
            mm_alloc_aligned(alloc_words, EXEC_PDP6_ALIGN_WORDS,
            MM_TYPE_PROCESS, owner, MM_ALLOC_HIGH, &base) != MM_OK)
                return -1;

        mem = (kword_t *)(unsigned long)base;
        for (i = 0U; (kword_t)i < alloc_words; ++i)
                mem[i] = 0UL;
        if (vfs_read_words(node, header_words,
            mem + EXEC_USER_ORIGIN, image_words) != (int)image_words)
                goto fail;



        p->meta = ((kword_t)owner & PROC_PID_MASK) |
            ((kword_t)PROC_SIDL << PROC_STATE_SHIFT) |
            (((kword_t)(entry + EXEC_USER_ORIGIN) & PROC_HALF_MASK) <<
            PROC_ENTRY_SHIFT);
        p->mem_layout = ((alloc_words & PROC_HALF_MASK) <<
            PROC_HALF_SHIFT) | (base & PROC_HALF_MASK);
        proc_swap_attach(owner, node, (kword_t)image_words,
            (kword_t)text_words, header_words,
            (dxr_flags == EXEC_DXR_F_PURE &&
            header_words == EXEC_DXR_EXT_HDR_WORDS) ? PROC_SWAP_EXEC_PURE : 0U);
        return 0;

fail:
        (void)mm_free(base, MM_TYPE_PROCESS, owner);
        return -1;
}
