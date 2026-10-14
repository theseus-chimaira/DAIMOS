/* Staged lifecycle gate, not linked into active PDP-6 scheduler.
 * Storage state is authoritative; the disk swap record is NOT a packed
 * descriptor. The caller must provide slot-0/lifecycle exclusion.
 */
#include "proc_packed_lifecycle_v082.h"
#include "proc_storage_map_v28.h"
#include "proc_swap.h"
#include "vm_pdp6.h"

int
proc_packed_try_evict_v082(unsigned int slot, kword_t offset)
{
    struct proc *p;
    unsigned int state;
    int rc;
    if (!proc_table || !proc_swap_records ||
        !proc_storage_runtime_v29.words || !slot ||
        slot >= proc_storage_runtime_v29.slots || slot >= proc_slots ||
        slot == (unsigned int)proc_current_slot)
        return -1;
    p = &proc_table[slot];
    state = PROC_STATE(p);
    if ((state != PROC_SLEEP && state != PROC_STOP && state != PROC_SRUN) ||
        PROC_TRANSITION(p) || !PROC_HAS_UAREA(p) ||
        PROC_USER_MAPPING_HELD(p) || !VM_PDP6_BASE(p) ||
        !proc_swap_records[slot].state ||
        proc_storage_get_v28(&proc_storage_runtime_v29, slot) !=
            PROC_STORAGE_RESIDENT)
        return -1;
    PROC_SET_TRANSITION(p);
    rc = proc_packed_inline_evict_v079(slot, offset);
    if (rc != 0) PROC_CLEAR_TRANSITION(p);
    return rc;
}

int
proc_packed_try_restore_v082(unsigned int slot, kword_t backing,
    proc_packed_pure_reload_v31 reload)
{
    struct proc *p;
    int rc;
    if (!reload || !backing || !proc_table ||
        !proc_storage_runtime_v29.words || !slot ||
        slot >= proc_storage_runtime_v29.slots || slot >= proc_slots)
        return -1;
    p = &proc_table[slot];
    if (PROC_STATE(p) != PROC_SRUN || PROC_IS_FREE(p) ||
        PROC_TRANSITION(p) || !PROC_HAS_UAREA(p) ||
        VM_PDP6_BASE(p) ||
        proc_storage_get_v28(&proc_storage_runtime_v29, slot) !=
            PROC_STORAGE_PACKED)
        return -1;
    PROC_SET_TRANSITION(p);
    rc = proc_packed_inline_restore_v079(slot, backing, reload);
    /* A postcommit failure may retain TRANSITION intentionally. */
    if (rc != 0 && VM_PDP6_BASE(p) == 0UL)
        PROC_CLEAR_TRANSITION(p);
    return rc;
}
