/* Single guard for all VM teardown entry points; no storage heuristics.
 * Zero VM base is NOT sufficient evidence of a packed process.
 */
#include "proc_packed_teardown_v39.h"
#include "proc.h"

int
proc_packed_teardown_guard_v39(struct proc_packed_layout_v36 *layout,
    unsigned int slot)
{
        struct proc *p;
        int mode;

        if (!proc_table || !proc_storage_runtime_v29.words ||
            slot == 0U || slot >= proc_storage_runtime_v29.slots)
                return MM_ERR_INVAL;
        p = &proc_table[slot];
        if (!PROC_TRANSITION(p))
                return MM_ERR_BUSY;
        mode = proc_storage_get_v28(&proc_storage_runtime_v29, slot);
        if (mode == PROC_STORAGE_RESIDENT || mode == PROC_STORAGE_DISK)
                return MM_OK;
        if (mode != PROC_STORAGE_PACKED || !layout || !layout->words ||
            layout->slots != proc_storage_runtime_v29.slots)
                return MM_ERR_INVAL;
        return proc_packed_owner_discard_v37(layout, slot);
}
