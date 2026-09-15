#include "exec.h"
#include "fs_mres.h"
#include "file.h"
#include "vm.h"
#include "proc_swap.h"

#define EXEC_HALF_MASK 0777777UL
#define EXEC_WORD_MASK 0777777777777UL
#define EXEC_DXR_MAGIC \
    ((VFS_SIX6('D','X','R',' ',' ',' ') >> 18) & EXEC_HALF_MASK)

int
exec_load_process(struct proc *p, unsigned int owner,
    const kword_t *path)
{
        vnode_t node;
        struct vfs_stat st;
        kword_t hdr[EXEC_DXR_EXT_HDR_WORDS];
        int process_words;
        int entry;
        int image_words;
        int text_words;
        int bss_words;
        int header_words;
        int dxr_flags;
        int reloc_words;

        /* DXR header fields are at most 18 bits.  Signed locals avoid KCC
         * sign-bit normalization around ordinary bounded comparisons. */
        if (p == 0 || path == 0 ||
            file_lookup_path(path, &node) != 0 ||
            vfs_stat(node, &st) != 0 || st.type != VFS_TYPE_REG ||
            st.size_words < EXEC_DXR_BASE_HDR_WORDS ||
            vfs_read_words(node, 0U, hdr, EXEC_DXR_BASE_HDR_WORDS) !=
            (int)EXEC_DXR_BASE_HDR_WORDS ||
            ((hdr[0] >> 18U) & EXEC_HALF_MASK) != EXEC_DXR_MAGIC)
                return -1;

        entry = (int)(hdr[0] & EXEC_HALF_MASK);
        image_words = (int)((hdr[1] >> 18U) & EXEC_HALF_MASK);
        bss_words = (int)(hdr[1] & EXEC_DXR_BSS_MASK);
        dxr_flags = (int)(hdr[1] &
            (EXEC_DXR_F_PURE | EXEC_DXR_F_IMPURE));
        if (image_words == 0 || image_words > (int)EXEC_DXR_MAX_IMAGE_WORDS ||
            bss_words > (int)EXEC_DXR_MAX_BSS_WORDS ||
            entry >= image_words ||
            dxr_flags == (EXEC_DXR_F_PURE | EXEC_DXR_F_IMPURE))
                return -1;
        reloc_words = (image_words + 35) / 36;
        if (st.size_words == (kword_t)EXEC_DXR_BASE_HDR_WORDS +
            (kword_t)image_words + (kword_t)reloc_words) {
                header_words = EXEC_DXR_BASE_HDR_WORDS;
                text_words = 0;
        } else if (st.size_words == (kword_t)EXEC_DXR_EXT_HDR_WORDS +
            (kword_t)image_words + (kword_t)reloc_words) {
                if (vfs_read_words(node, 2U, &hdr[2], 1U) != 1 ||
                    (unsigned int)(hdr[2] & EXEC_HALF_MASK) !=
                    EXEC_DXR_TEXT_TAG)
                        return -1;
                text_words = (int)((hdr[2] >> 18U) & EXEC_HALF_MASK);
                if (text_words > image_words)
                        return -1;
                header_words = EXEC_DXR_EXT_HDR_WORDS;
        } else {
                return -1;
        }
        process_words = (int)EXEC_USER_ORIGIN + image_words + bss_words +
            (int)EXEC_DXR_STACK_WORDS;
        if (process_words > (int)EXEC_HALF_MASK)
                return -1;
        if (vm_space_create(p, owner, (kword_t)process_words) != 0)
                return -1;
        if (vm_space_load_file(p, node, (kword_t)header_words,
            (kword_t)EXEC_USER_ORIGIN, (unsigned int)image_words) != 0)
                goto fail;

        p->meta = (p->meta &
            ((kword_t)PROC_PARENT_MASK << PROC_PARENT_SHIFT)) |
            (((kword_t)(entry + EXEC_USER_ORIGIN) & PROC_HALF_MASK) <<
            PROC_ENTRY_SHIFT);
        p->sched = PROC_SCHED_DEFAULT;
        PROC_SET_STATE(p, PROC_SIDL);
        if (proc_swap_attach(owner, node, (kword_t)text_words,
            (unsigned int)dxr_flags == EXEC_DXR_F_PURE &&
            header_words == EXEC_DXR_EXT_HDR_WORDS) != 0)
                goto fail;
        return 0;

fail:
        if (vm_space_destroy(p, owner) == 0)
                PROC_SET_META_LH(p, 0UL);
        return -1;
}
