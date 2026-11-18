/**
 * @file vm_pdp6_swap.c
 * @brief Resident PDP-6 process swap-out, swap-in, and reclaim policy.
 *
 * Swapping uses the raw backing pool selected by the storage bridge. A resident
 * process's one-word record describes executable backing; on swap-out that
 * word is saved in the stable u-area and the record is reused for first-block
 * and block-count. Swap-in runs from slot-0 executive context so synchronous
 * storage I/O never suspends on a process stack whose user extent is absent.
 * v1666: terminal backing release belongs to the single installed
 * memory-service MRES. The permanent kernel only retires common
 * swap-record/u-area metadata after that selected service succeeds.
 * No disk-range arithmetic or packed-MM owner scan is retained here.
 */
#include "proc_swap.h"
#include "vm_pdp6.h"
#include "bstore.h"
#include "d6fs_provider.h"
#include "dtfs.h"
#include "tsfs.h"
#include "memfs.h"
#include "exec.h"
#include "fs_mres.h"
#include "mm.h"
#include "storage.h"
#include "syscall.h"
#include "proc_storage_map_v28.h"

/* Boot-time map allocation lives next to the existing per-slot swap records. */
struct proc_storage_map_v28 proc_storage_runtime_v29;


struct proc_swap_record *proc_swap_records;
kword_t proc_swap_blocks_used;

/* The assembly swap transactions own mode publication under TRANSITION. */
extern int proc_storage_publish_v30(unsigned int slot,
    unsigned int old_state, unsigned int new_state);
extern int proc_storage_expect_v30(unsigned int slot, unsigned int mode);
extern void proc_swap_storage_invariant_v30(void);

/* One default-refusing KCORE trampoline, patched by KINIT to the
 * selected disk or RAW PURE MRES. AC1..3 use the existing C ABI.
 * A backend must verify its exact mode/owner and atomically publish
 * RESIDENT only after successfully releasing the physical backing.
 */
extern int proc_swap_term_v1666(int slot, struct proc *p,
    kword_t backing);

/** Release only validated nonresident disk backing, then clear ownership.
 *
 * Never erase a swap record when its disk allocation cannot be released.
 * The VM destroy caller must preserve the process u-area for retry.
 */
int
proc_swap_detach(int slot)
{
        struct proc *p;
        kword_t state;

        p = &proc_table[slot];
        state = proc_swap_records[slot].state;
        if (proc_storage_expect_v30((unsigned int)slot,
            PROC_STORAGE_RESIDENT) != 0 &&
            proc_swap_term_v1666(slot, p, state) != 0)
                return -1;
        proc_swap_records[slot].state = 0UL;
        if (PROC_HAS_UAREA(p)) {
                PROC_SWAP_BACKING_WORD(p) = 0UL;
                PROC_CRED_WORD(p) &= ~PROC_SWAP_MOUNT_EXT_BIT;
        }
        /* A staged EXEC may already have its u-area: either way clear
         * the temporary SIDL-only mount bit when backing is detached. */
        if (PROC_STATE(p) == PROC_SIDL)
                p->sched &= ~PROC_SWAP_STAGE_MOUNT_EXT_BIT;
        return 0;
}

/** Pin, write, free, and atomically publish one process as nonresident. */

/* Run one swap-in transaction from slot-0 executive context.  Selection is
 * deliberately derived from the existing scheduler fields, so no permanent
 * request queue or per-process swap scheduling state is needed. */
/** Service the scheduler's single pending slot-0 swap-in request. */

/** Allocate, restore, and republish one swapped process image. */

/** Swap victims until MM compaction can satisfy the requested free extent. */

/* Shared PURE backing construction, compiled in one KCC translation unit. */
#include "proc_pure_attach.c"
