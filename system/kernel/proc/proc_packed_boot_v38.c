/* Boot-only MM allocation for dense or sparse packed descriptor metadata.
 * Ownership belongs to this allocator, not to the process being packed.
 * KINIT code may be reclaimed; the published layout and extent persist.
 */
#include "proc_packed_boot_v38.h"
#include "fs_mres.h"
#include "proc.h"

#define PROC_PACKED_BOOT_OWNER_V38 5U

/* Persistent descriptor is owned by KCORE, not transient KINIT. */

int
proc_packed_boot_prepare_v38(unsigned int slots,
    unsigned int simultaneous_packed, unsigned int budget_words)
{
        kword_t base;
        kword_t *words;
        unsigned int format;
        unsigned int required;
        int rc;

        if (proc_packed_runtime_v38.words ||
            proc_packed_runtime_v38.slots ||
            proc_packed_runtime_v38.capacity || slots != proc_slots ||
            proc_storage_runtime_v29.slots != slots ||
            !proc_storage_runtime_v29.words)
                return MM_ERR_INVAL;
        rc = proc_packed_layout_plan_v36(slots, simultaneous_packed,
            budget_words, &format, &required);
        if (rc != MM_OK)
                return rc;
        rc = mm_alloc((kword_t)required, MM_TYPE_KERNEL_DYNAMIC,
            PROC_PACKED_BOOT_OWNER_V38, MM_ALLOC_LOW, &base);
        if (rc != MM_OK)
                return rc;
        words = (kword_t *)(unsigned long)base;
        fs_zero_words(words, required);
        rc = proc_packed_layout_init_v36(&proc_packed_runtime_v38,
            words, slots, simultaneous_packed, format);
        if (rc != MM_OK) {
                /* The runtime descriptor remains entirely unpublished. */
                (void)mm_free(base, MM_TYPE_KERNEL_DYNAMIC,
                    PROC_PACKED_BOOT_OWNER_V38);
                return rc;
        }
        return MM_OK;
}

int
proc_packed_boot_abort_v38(void)
{
        unsigned int i;
        unsigned int words;
        kword_t base;
        int rc;

        if (!proc_packed_runtime_v38.words)
                return MM_ERR_INVAL;
        words = proc_packed_runtime_v38.format ==
            PROC_PACKED_LAYOUT_DENSE_V36 ?
            proc_packed_runtime_v38.slots :
            2U * proc_packed_runtime_v38.capacity;
        for (i = 0U; i < words; ++i)
                if (proc_packed_runtime_v38.words[i])
                        return MM_ERR_BUSY;
        base = (kword_t)(unsigned long)proc_packed_runtime_v38.words;
        rc = mm_free(base, MM_TYPE_KERNEL_DYNAMIC,
            PROC_PACKED_BOOT_OWNER_V38);
        if (rc != MM_OK)
                return rc;
        proc_packed_runtime_v38.words = 0;
        proc_packed_runtime_v38.slots = 0U;
        proc_packed_runtime_v38.capacity = 0U;
        proc_packed_runtime_v38.format = 0U;
        return MM_OK;
}
