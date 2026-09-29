#include "vm_pdp6.h"
#include "fs_mres.h"
#include "mm.h"
#include "mm_internal.h"
#include "proc_swap.h"
#include "exec.h"

int
vm_space_create(struct proc *p, unsigned int owner, kword_t words)
{
        kword_t alloc_words;
        kword_t base;
        kword_t *mem;

        alloc_words = (words + VM_PDP6_ALIGN_WORDS - 1UL) &
            ~(VM_PDP6_ALIGN_WORDS - 1UL);
#ifndef __PDP10__
        if (alloc_words > PROC_HALF_MASK)
                return -1;
#endif
        if (mm_alloc_aligned(alloc_words, VM_PDP6_ALIGN_WORDS,
            MM_TYPE_PROCESS, owner, MM_ALLOC_HIGH, &base) != MM_OK)
                return -1;
        mem = (kword_t *)(unsigned long)base;
        fs_zero_words(mem, (unsigned int)alloc_words);
        VM_PDP6_SET_SPACE(p, alloc_words, base);
        return 0;
}

int
vm_space_load_file(struct proc *p, vnode_t node, kword_t file_offset,
    kword_t user_offset, unsigned int words)
{
        kword_t base;

        base = VM_PDP6_BASE(p);
        return vfs_read_words(node, file_offset,
            (kword_t *)(unsigned long)(base + user_offset), words) ==
            (int)words ? 0 : -1;
}

int
vm_space_startup(struct proc *p, const kword_t *records,
    kword_t counts, kword_t *startup)
{
        unsigned int argc;
        unsigned int envc;
        unsigned int i;
        unsigned int nwords;
        unsigned int string_off;
        unsigned int source_off;
        kword_t start;
        kword_t base;
        kword_t *dst;

#ifndef __PDP10__
        if (p == 0 || records == 0 || startup == 0 || !VM_SPACE_ACTIVE(p))
                return -1;
#endif
        argc = (unsigned int)((counts >> 18U) & PROC_HALF_MASK);
        envc = (unsigned int)(counts & PROC_HALF_MASK);
        start = VM_SPACE_WORDS(p) - (kword_t)EXEC_DXR_STACK_WORDS;
        startup[0] = (kword_t)argc;
        /* Keep one process-image metadata word immediately before argv.
         * MonitorFS can reconstruct COMM/CMDLINE/ENVIRONMENT from the live
         * image without adding per-process resident kernel state. */
        startup[1] = argc == 0U ? 0UL : start + 1UL;
        startup[2] = envc == 0U ? 0UL : start + 1UL + (kword_t)argc;

        base = VM_PDP6_BASE(p);
        dst = (kword_t *)(unsigned long)(base + start);
        dst[0] = counts;
        string_off = 1U + argc + envc + (envc != 0U);
        source_off = 0U;
        for (i = 0U; i < argc + envc; ++i) {
                nwords = 1U + ((unsigned int)records[source_off] + 5U) / 6U;
                dst[1U + i] = start + (kword_t)string_off;
                fs_copy_words(&records[source_off], &dst[string_off], nwords);
                source_off += nwords;
                string_off += nwords;
        }
        if (envc != 0U)
                dst[1U + argc + envc] = 0UL;
        startup[3] = start + (kword_t)string_off - 1UL;
        return 0;
}

#ifndef __PDP10__
int
vm_space_inspect_word(const struct proc *p, kword_t offset, kword_t *wordp)
{
        kword_t base;

        if (p == 0 || wordp == 0 || offset >= VM_SPACE_WORDS(p))
                return -1;
        base = VM_PDP6_BASE(p);
        if (base == 0UL)
                return -1;
        *wordp = ((const kword_t *)(unsigned long)base)[offset];
        return 0;
}
#endif

int
vm_space_destroy(struct proc *p, unsigned int owner)
{
        kword_t base;

        base = VM_PDP6_BASE(p);
        if (base != 0UL &&
            mm_free(base, MM_TYPE_PROCESS, owner) != MM_OK)
                return -1;
        proc_swap_detach((int)owner);
        VM_SPACE_RESET(p);
        return 0;
}

int
vm_space_can_swap(const struct proc *p)
{
        kword_t base;

        base = VM_PDP6_BASE(p);
        return base != 0UL && !mm_is_pinned(base);
}

int
vm_extent_move(unsigned int owner, kword_t base, kword_t words,
    kword_t new_base)
{
        struct proc *p;
        kword_t *src;
        kword_t *dst;
        unsigned int old_state;
        int rc;

#ifndef __PDP10__
        if (proc_table == 0 || owner >= proc_slots)
                return MM_ERR_BUSY;
#endif
        if (owner == (unsigned int)proc_current_slot)
                return MM_ERR_BUSY;
        p = &proc_table[owner];
#ifndef __PDP10__
        if (PROC_IS_FREE_OR_ZOMB(p) || VM_PDP6_BASE(p) != base ||
            VM_SPACE_WORDS(p) != words)
                return MM_ERR_BUSY;
#endif
        if (new_base == base || (new_base & (VM_PDP6_ALIGN_WORDS - 1UL)) != 0UL ||
            PROC_TRANSITION(p) ||
            (PROC_HAS_UAREA(p) && PROC_USER_MAPPING_HELD(p)))
                return MM_ERR_BUSY;
        old_state = PROC_STATE(p);
        if (PROC_HAS_UAREA(p)) {
                kword_t ctl;

                ctl = PROC_CTL_WORD(p);
                ctl |= (kword_t)PROC_STOP_MM << PROC_STOP_SHIFT;
                PROC_CTL_WORD(p) = ctl;
                proc_runq_remove(owner);
                PROC_SET_STATE(p, PROC_STOP);
        } else if (old_state == PROC_SRUN) {
                return MM_ERR_BUSY;
        }

        PROC_SET_TRANSITION(p);
        rc = MM_OK;
#ifndef __PDP10__
        if (words == 0UL || mm_is_pinned(base))
                rc = MM_ERR_BUSY;
#endif
        if (rc == MM_OK) {
                src = (kword_t *)(unsigned long)base;
                dst = (kword_t *)(unsigned long)new_base;
                fs_move_words(src, dst, (unsigned int)words);
                VM_PDP6_SET_BASE(p, new_base);
        }
        PROC_CLEAR_TRANSITION(p);
        if (PROC_HAS_UAREA(p)) {
                kword_t ctl;

                ctl = PROC_CTL_WORD(p);
                ctl &= ~((kword_t)PROC_STOP_MM << PROC_STOP_SHIFT);
                PROC_CTL_WORD(p) = ctl;
                if ((ctl & ((kword_t)PROC_STOP_MASK <<
                    PROC_STOP_SHIFT)) == 0UL) {
                        PROC_SET_STATE(p, old_state);
                        if (old_state == PROC_SRUN)
                                proc_runq_add(owner);
                }
        }
        return rc;
}
