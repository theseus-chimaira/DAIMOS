/* Inactive descriptor-layout selection. One shared owner/teardown engine
 * will consume this layout; code is not part of the 32K KCORE link.
 */
#ifndef DAIMOS_PROC_PACKED_LAYOUT_V36_H
#define DAIMOS_PROC_PACKED_LAYOUT_V36_H
#include "proc_packed_mm_registry_v35.h"

#define PROC_PACKED_LAYOUT_DENSE_V36 0U
#define PROC_PACKED_LAYOUT_SPARSE_V36 1U

struct proc_packed_layout_v36 {
        kword_t *words;
        unsigned int slots;
        unsigned int capacity;
        unsigned int format;
};

/* Select the layout requiring fewer data words, provided the requested
 * packed concurrency is supportable. Tie goes to O(1) dense lookup.
 * KINIT supplies an available descriptor RAM budget, not installed RAM.
 */
int proc_packed_layout_plan_v36(unsigned int slots,
    unsigned int simultaneous_packed, unsigned int budget_words,
    unsigned int *format, unsigned int *required_words);
int proc_packed_layout_init_v36(struct proc_packed_layout_v36 *layout,
    kword_t *storage, unsigned int slots, unsigned int capacity,
    unsigned int format);
/* Return a pointer to the mutable LAYOUT word, or null if not present.
 * For sparse insertion, slot tag is installed only after caller validates
 * the owning MM extent and publishes PACKED mode under TRANSITION.
 */
kword_t *proc_packed_layout_find_v36(
    struct proc_packed_layout_v36 *layout, unsigned int slot);
kword_t *proc_packed_layout_reserve_v36(
    struct proc_packed_layout_v36 *layout, unsigned int slot);
int proc_packed_layout_clear_v36(struct proc_packed_layout_v36 *layout,
    unsigned int slot);
#endif
