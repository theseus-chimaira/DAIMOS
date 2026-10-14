/* Opt-in PDP-6 slot-0 restore dispatcher. Storage mode, never VM base or
 * swap-record contents alone, selects the restoration implementation.
 */
#include "proc_swap.h"
#include "proc_storage_map_v28.h"
#include "proc_packed_lifecycle_v088.h"

extern int proc_swap_reload_pure(kword_t backing, kword_t base);

int
proc_swap_restore_dispatch_v088(int slot)
{
    int mode;

    if (slot <= 0 || !proc_table || !proc_swap_records ||
        !proc_storage_runtime_v29.words ||
        (unsigned int)slot >= proc_storage_runtime_v29.slots)
        return -1;
    mode = proc_storage_get_v28(&proc_storage_runtime_v29,
        (unsigned int)slot);
    if (mode == (int)PROC_STORAGE_DISK)
        return proc_swap_in(slot);
    if (mode == (int)PROC_STORAGE_PACKED) {
        kword_t backing = proc_swap_records[slot].state;
        if (!backing)
            return -1;
        return proc_packed_try_restore_v088((unsigned int)slot, backing,
            proc_swap_reload_pure);
    }
    return -1;
}
