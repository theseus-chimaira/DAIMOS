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
 * A passed preflight alone NEVER authorizes process reclamation.
 */
#ifndef DAIMOS_PURE_LIFE_V1604_H
#define DAIMOS_PURE_LIFE_V1604_H

#include "proc.h"
#include "proc_swap.h"
#include "proc_storage_map_v28.h"
#include "vm.h"
#include "mm.h"

#ifdef DAIMOS_TEST_HOST_WORD_ADDR
#define PPL_PTR_V1604(x) ((kword_t *)(unsigned long)((x) * sizeof(kword_t)))
#else
#define PPL_PTR_V1604(x) ((kword_t *)(unsigned long)(x))
#endif

/* Static: no new exported API, heap state, or forced runtime dispatch. */
static int
pure_life_prepare_v1604(unsigned int slot, unsigned int terminal,
    struct proc **pp, kword_t *backingp, kword_t *wordsp)
{
        struct proc *p;
        kword_t *u;

        if (!slot || !proc_table || !proc_swap_records ||
            !proc_storage_runtime_v29.words ||
            slot >= proc_storage_runtime_v29.slots)
                return MM_ERR_INVAL;
        p = &proc_table[slot];
        if (!PROC_TRANSITION(p) || !PROC_HAS_UAREA(p) ||
            !PROC_UAREA_BASE(p) ||
            (terminal && PROC_STATE(p) == PROC_FREE) ||
            (!terminal && PROC_STATE(p) != PROC_SLEEP &&
             PROC_STATE(p) != PROC_STOP))
                return MM_ERR_INVAL;
        u = PPL_PTR_V1604(PROC_UAREA_BASE(p));
        if (!terminal && (u[PROC_FDCTL_OFFSET] &
            (PROC_USER_MAP_BIT | PROC_FATAL_CLOSE_BIT)))
                return MM_ERR_BUSY;
        *pp = p;
        *backingp = proc_swap_records[slot].state;
        *wordsp = VM_SPACE_WORDS(p);
        return MM_OK;
}

#endif
