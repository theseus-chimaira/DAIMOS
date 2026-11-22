/*
 * v1667 opt-in PURE-only terminal MRES entry. Never compiled into KCORE
 * or the disk-swap MRES. Mode2 continues through the exact original
 * PDP-6 RAW owner leaf; mode3 uses the bounded compressed owner
 * terminal introduced in v1661. No new process metadata is needed.
 *
 * The permanent vm_space_destroy still rejects mode3. This staged
 * integration MUST NOT be enabled in a runnable production kernel
 * until the terminal, RUN/EXIT/WAIT and scheduler gates are complete.
 */
#include "proc.h"
#include "proc_storage_map_v28.h"
#include "proc_pure_bitmap_terminal_v1661.h"
#include "mm.h"

#if !defined(DAIMOS_PBM_V1667) || DAIMOS_PBM_V1667 != 1
#error v1667_requires_explicit_test_only_pure_mres_build
#endif

extern int pdp6_pure_detach_v1609(int, struct proc *, kword_t);

int
proc_pure_term_v1667(int slot, struct proc *p, kword_t backing)
{
        unsigned int index;
        kword_t mode, shift;

        if (slot <= 0 || !p || !proc_table ||
            !proc_storage_runtime_v29.words ||
            (unsigned int)slot >= proc_storage_runtime_v29.slots ||
            p != &proc_table[slot])
                return -1;

        index = (unsigned int)slot / PROC_STMAP_SLOTS_WORD_V28;
        shift = (kword_t)((unsigned int)slot %
            PROC_STMAP_SLOTS_WORD_V28) * 2UL;
        mode = (proc_storage_runtime_v29.words[index] >> shift) & 3UL;
        if (mode == (kword_t)PROC_STORAGE_PACKED)
                return pdp6_pure_detach_v1609(slot, p, backing);
        if (mode == (kword_t)PROC_STORAGE_BITMAP_V1661)
                return proc_pure_bitmap_terminal_v1661((unsigned int)slot);
        return -1; /* DISK/resident are not PURE terminal owners */
}
