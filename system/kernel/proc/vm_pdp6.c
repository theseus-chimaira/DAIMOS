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
        if (alloc_words > PROC_HALF_MASK ||
            mm_alloc_aligned(alloc_words, VM_PDP6_ALIGN_WORDS,
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
        unsigned int vector_words;
        unsigned int total_words;
        unsigned int string_off;
        unsigned int source_off;
        kword_t start;
        kword_t base;
        kword_t *dst;

        if (p == 0 || records == 0 || startup == 0 || !VM_SPACE_ACTIVE(p))
                return -1;
        argc = (unsigned int)((counts >> 18U) & PROC_HALF_MASK);
        envc = (unsigned int)(counts & PROC_HALF_MASK);
        vector_words = argc + (envc == 0U ? 0U : envc + 1U);
        total_words = vector_words;
        source_off = 0U;
        for (i = 0U; i < argc + envc; ++i) {
                nwords = 1U + ((unsigned int)records[source_off] + 5U) / 6U;
                if (total_words + nwords < total_words)
                        return -1;
                total_words += nwords;
                source_off += nwords;
        }
        if (total_words > (unsigned int)EXEC_DXR_STACK_WORDS)
                return -1;

        start = VM_SPACE_WORDS(p) - (kword_t)EXEC_DXR_STACK_WORDS;
        startup[0] = (kword_t)argc;
        startup[1] = argc == 0U ? 0UL : start;
        startup[2] = envc == 0U ? 0UL : start + (kword_t)argc;
        startup[3] = total_words == 0U ? start - 1UL :
            start + (kword_t)total_words - 1UL;
        if (total_words == 0U)
                return 0;

        base = VM_PDP6_BASE(p);
        dst = (kword_t *)(unsigned long)(base + start);
        string_off = vector_words;
        source_off = 0U;
        for (i = 0U; i < argc; ++i) {
                nwords = 1U + ((unsigned int)records[source_off] + 5U) / 6U;
                dst[i] = start + (kword_t)string_off;
                fs_copy_words(&records[source_off], &dst[string_off], nwords);
                source_off += nwords;
                string_off += nwords;
        }
        for (i = 0U; i < envc; ++i) {
                nwords = 1U + ((unsigned int)records[source_off] + 5U) / 6U;
                dst[argc + i] = start + (kword_t)string_off;
                fs_copy_words(&records[source_off], &dst[string_off], nwords);
                source_off += nwords;
                string_off += nwords;
        }
        if (envc != 0U)
                dst[argc + envc] = 0UL;
        return 0;
}

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
vm_extent_move(unsigned int owner, kword_t base, kword_t words)
{
        struct proc *p;
        kword_t new_base;
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
        if (PROC_TRANSITION(p) ||
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
#ifndef __PDP10__
        if (words == 0UL || mm_is_pinned(base)) {
                rc = MM_ERR_BUSY;
                goto out;
        }
#endif
        rc = mm_alloc_aligned_noreclaim(words, VM_PDP6_ALIGN_WORDS,
            MM_TYPE_PROCESS, owner, MM_ALLOC_HIGH, &new_base);
        if (rc != MM_OK)
                goto out;
        if ((long)new_base <= (long)base) {
                (void)mm_free(new_base, MM_TYPE_PROCESS, owner);
                rc = MM_ERR_FRAGMENTED;
                goto out;
        }

        src = (kword_t *)(unsigned long)base;
        dst = (kword_t *)(unsigned long)new_base;
        fs_copy_words(src, dst, (unsigned int)words);
        VM_PDP6_SET_BASE(p, new_base);
        rc = mm_free(base, MM_TYPE_PROCESS, owner);
        if (rc != MM_OK) {
                VM_PDP6_SET_BASE(p, base);
                (void)mm_free(new_base, MM_TYPE_PROCESS, owner);
                goto out;
        }
        rc = MM_OK;
out:
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
