/**
 * @file proc_pure_lifecycle_v1604.h
 * @brief Common preflight for an inactive PURE process RAM transaction.
 *
 * P1-E staging only; this header does not add a production KCORE object.
 * This check deliberately knows nothing about PDP-6 APR alignment, the
 * physical relocation base, the PURE prefix geometry, or the number of
 * machine-mapped segments. A later KA10 backend may use the same process
 * ownership preflight with a different retained writable region.
 *
 * The caller already owns PROC_TRANSITION and must separately guarantee
 * immutable executable backing, correct storage mode, no active executive
 * continuation, and exact physical MM ownership before publication.
 * v1610: a runnable nonresident target may be restored on slot 0; a
 * runnable process may NOT be dropped or restored on its own stack.
 * A passed preflight alone NEVER authorizes process reclamation.
 */
#ifndef DAIMOS_PURE_LIFE_V1604_H
#define DAIMOS_PURE_LIFE_V1604_H

#include "proc.h"
#include "proc_swap.h"
#include "proc_storage_map_v28.h"
#include "vm.h"
#include "mm.h"

/*
 * v1610: distinguish drop from slot-0 wakeup without another process bit
 * or another per-CPU state word. The terminal contract is unchanged.
 * RUN is admissible ONLY for RESTORE, and only while slot 0 is active.
 * The restore caller must still verify PACKED mode and exact MM owner.
 */
#define PURE_LIFE_DROP_V1610     0U
#define PURE_LIFE_TERMINAL_V1610 1U
#define PURE_LIFE_RESTORE_V1610  2U

#ifdef DAIMOS_TEST_HOST_WORD_ADDR
#define PPL_PTR_V1604(x) ((kword_t *)(unsigned long)((x) * sizeof(kword_t)))
#else
#define PPL_PTR_V1604(x) ((kword_t *)(unsigned long)(x))
#endif

/* Static: no new exported API, heap state, or forced runtime dispatch. */
static int
pure_life_prepare_v1604(unsigned int slot, unsigned int action,
    struct proc **pp, kword_t *backingp, kword_t *wordsp)
{
        struct proc *p;
        kword_t *u;

        if (!slot || action > PURE_LIFE_RESTORE_V1610 ||
            !proc_table || !proc_swap_records ||
            !proc_storage_runtime_v29.words ||
            slot >= proc_storage_runtime_v29.slots)
                return MM_ERR_INVAL;
        p = &proc_table[slot];
        if (!PROC_TRANSITION(p) || !PROC_HAS_UAREA(p) ||
            !PROC_UAREA_BASE(p) ||
            (action == PURE_LIFE_TERMINAL_V1610 &&
             PROC_STATE(p) == PROC_FREE) ||
            (action != PURE_LIFE_TERMINAL_V1610 &&
             PROC_STATE(p) != PROC_SLEEP &&
             PROC_STATE(p) != PROC_STOP &&
             !(action == PURE_LIFE_RESTORE_V1610 &&
               PROC_STATE(p) == PROC_SRUN &&
               proc_current_slot == 0UL)))
                return MM_ERR_INVAL;
        u = PPL_PTR_V1604(PROC_UAREA_BASE(p));
        if (action != PURE_LIFE_TERMINAL_V1610 &&
            (u[PROC_FDCTL_OFFSET] &
            (PROC_USER_MAP_BIT | PROC_FATAL_CLOSE_BIT)))
                return MM_ERR_BUSY;
        *pp = p;
        *backingp = proc_swap_records[slot].state;
        *wordsp = VM_SPACE_WORDS(p);
        return MM_OK;
}

#endif
