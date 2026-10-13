/* Slot-to-writable-layout indexing only. MM remains physical authority.
 * No MM calls, dynamic allocations, or process-state changes occur here.
 */
#include "proc_packed_layout_v36.h"
#include "proc.h"

int
proc_packed_layout_plan_v36(unsigned int slots,
    unsigned int simultaneous_packed, unsigned int budget_words,
    unsigned int *format, unsigned int *required_words)
{
        unsigned int sparse_words;
        if (!format || !required_words || slots < PROC_MIN_SLOTS ||
            slots > PROC_MAX_SLOTS || !simultaneous_packed ||
            simultaneous_packed >= slots)
                return MM_ERR_INVAL;
        sparse_words = 2U * simultaneous_packed;
        if (sparse_words < slots && sparse_words <= budget_words) {
                *format = PROC_PACKED_LAYOUT_SPARSE_V36;
                *required_words = sparse_words;
                return MM_OK;
        }
        if (slots <= budget_words) {
                *format = PROC_PACKED_LAYOUT_DENSE_V36;
                *required_words = slots;
                return MM_OK;
        }
        return MM_ERR_NOMEM;
}

int
proc_packed_layout_init_v36(struct proc_packed_layout_v36 *layout,
    kword_t *storage, unsigned int slots, unsigned int capacity,
    unsigned int format)
{
        unsigned int words;
        unsigned int i;
        if (!layout || !storage || layout->words || layout->slots ||
            layout->capacity || slots < PROC_MIN_SLOTS ||
            slots > PROC_MAX_SLOTS || !capacity || capacity >= slots ||
            format > PROC_PACKED_LAYOUT_SPARSE_V36)
                return MM_ERR_INVAL;
        words = format == PROC_PACKED_LAYOUT_DENSE_V36 ? slots :
            2U * capacity;
        for (i = 0U; i < words; ++i)
                if (storage[i])
                        return MM_ERR_INVAL;
        layout->words = storage;
        layout->slots = slots;
        layout->capacity = capacity;
        layout->format = format;
        return MM_OK;
}

kword_t *
proc_packed_layout_find_v36(struct proc_packed_layout_v36 *layout,
    unsigned int slot)
{
        unsigned int i;
        if (!layout || !layout->words || !slot || slot >= layout->slots)
                return 0;
        if (layout->format == PROC_PACKED_LAYOUT_DENSE_V36)
                return layout->words[slot] ? &layout->words[slot] : 0;
        for (i = 0U; i < layout->capacity; ++i)
                if (layout->words[2U * i] == slot)
                        return &layout->words[2U * i + 1U];
        return 0;
}

kword_t *
proc_packed_layout_reserve_v36(struct proc_packed_layout_v36 *layout,
    unsigned int slot)
{
        unsigned int i;
        if (!layout || !layout->words || !slot || slot >= layout->slots ||
            proc_packed_layout_find_v36(layout, slot))
                return 0;
        if (layout->format == PROC_PACKED_LAYOUT_DENSE_V36)
                return &layout->words[slot];
        for (i = 0U; i < layout->capacity; ++i)
                if (!layout->words[2U * i]) {
                        layout->words[2U * i] = slot;
                        return &layout->words[2U * i + 1U];
                }
        return 0;
}

int
proc_packed_layout_clear_v36(struct proc_packed_layout_v36 *layout,
    unsigned int slot)
{
        unsigned int i;
        kword_t *word = proc_packed_layout_find_v36(layout, slot);
        if (!word)
                return MM_ERR_INVAL;
        *word = 0UL;
        if (layout->format == PROC_PACKED_LAYOUT_SPARSE_V36)
                for (i = 0U; i < layout->capacity; ++i)
                        if (layout->words[2U * i] == slot) {
                                layout->words[2U * i] = 0UL;
                                break;
                        }
        return MM_OK;
}
