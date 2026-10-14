/* Staged VM destruction preflight under lifecycle exclusion.
 * Return 1 after PACKED owner retirement (caller must reset VM),
 * 0 for legacy RESIDENT/DISK cleanup, -1 if destruction must stop.
 * Never call the disk-span detacher for PACKED process backing.
 */
#include "proc_packed_teardown_v39.h"
#include "proc_packed_boot_v38.h"
#include "proc_swap.h"
#include "vm_pdp6.h"

int
proc_packed_destroy_v084(unsigned int slot)
{
    struct proc *p;
    int mode;
    if (!proc_table || !proc_swap_records ||
        !proc_storage_runtime_v29.words || slot == 0U ||
        slot >= proc_storage_runtime_v29.slots)
        return -1;
    p = &proc_table[slot];
    if (!PROC_TRANSITION(p))
        return -1;
    mode = proc_storage_get_v28(&proc_storage_runtime_v29, slot);
    if (mode == (int)PROC_STORAGE_RESIDENT ||
        mode == (int)PROC_STORAGE_DISK)
        return 0;
    if (mode != (int)PROC_STORAGE_PACKED || VM_PDP6_BASE(p) ||
        proc_packed_teardown_guard_v39(&proc_packed_runtime_v38,
            slot) != MM_OK)
        return -1;
    /* Guard retired registry and owning MM extent. Retained swap record
     * contains original executable backing, NOT a disk block span. */
    proc_swap_records[slot].state = 0UL;
    return 1;
}
