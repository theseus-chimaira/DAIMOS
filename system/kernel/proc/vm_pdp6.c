#include "vm_pdp6.h"
#include "fs_mres.h"
#include "mm.h"
#include "mm_internal.h"
#include "proc_swap.h"

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

        if (proc_table == 0 || owner >= proc_slots ||
            owner == (unsigned int)proc_current_slot)
                return MM_ERR_BUSY;
        p = &proc_table[owner];
        if (PROC_IS_FREE_OR_ZOMB(p) || PROC_TRANSITION(p) ||
            VM_PDP6_BASE(p) != base || VM_SPACE_WORDS(p) != words ||
            (PROC_HAS_UAREA(p) && PROC_USER_MAPPING_HELD(p)))
                return MM_ERR_BUSY;
        old_state = PROC_STATE(p);
        if (PROC_HAS_UAREA(p)) {
                kword_t ctl;

                ctl = PROC_CTL_WORD(p);
                ctl |= (kword_t)PROC_STOP_MM << PROC_STOP_SHIFT;
                PROC_CTL_WORD(p) = ctl;
                PROC_SET_STATE(p, PROC_STOP);
        } else if (old_state == PROC_SRUN) {
                return MM_ERR_BUSY;
        }

        PROC_SET_TRANSITION(p);
        if (words == 0UL || mm_is_pinned(base)) {
                rc = MM_ERR_BUSY;
                goto out;
        }
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
                    PROC_STOP_SHIFT)) == 0UL)
                        PROC_SET_STATE(p, old_state);
        }
        return rc;
}
