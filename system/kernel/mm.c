#include "mm.h"
#include "proc.h"
#include "module_runtime.h"
#include "proc_swap.h"

struct mm_extent mm_extents[MM_MAX_EXTENTS];
kword_t mm_core_words;
unsigned int mm_extent_count;
kword_t mm_compaction_count;
kword_t mm_words_moved;
kword_t mm_allocation_failures;
kword_t mm_loaded_module_words;

static kword_t
mm_span(kword_t base, kword_t words)
{
        return ((words & MM_HALF_MASK) << 18U) | (base & MM_HALF_MASK);
}

static kword_t
mm_meta(unsigned int type, unsigned int owner, unsigned int pins)
{
        return ((kword_t)(pins & MM_PIN_MASK) << MM_PIN_SHIFT) |
            ((kword_t)(type & MM_TYPE_MASK) << MM_TYPE_SHIFT) |
            ((kword_t)owner & MM_OWNER_MASK);
}

static void
mm_delete(unsigned int slot)
{
        while (++slot < mm_extent_count)
                mm_extents[slot - 1U] = mm_extents[slot];
        --mm_extent_count;
}

static void mm_coalesce(unsigned int slot);
static int mm_alloc_aligned_raw(kword_t words, kword_t alignment,
    unsigned int type, unsigned int owner, unsigned int preference,
    kword_t *basep);

static int
mm_insert(unsigned int slot, const struct mm_extent *extent)
{
        unsigned int i;

        if (mm_extent_count >= MM_MAX_EXTENTS)
                return MM_ERR_DESCRIPTORS;
        i = mm_extent_count++;
        while (i > slot) {
                mm_extents[i] = mm_extents[i - 1U];
                --i;
        }
        mm_extents[slot] = *extent;
        return MM_OK;
}

void
mm_boot_init(kword_t core_words)
{
        mm_core_words = core_words;
        mm_extent_count = 0U;
        mm_compaction_count = 0UL;
        mm_words_moved = 0UL;
        mm_allocation_failures = 0UL;
        mm_loaded_module_words = 0UL;
}

/* Add a physically free boot-time range.  KINIT deliberately adds only
 * ranges that do not contain its own live image or packaged MRES sources. */
int
mm_add_free(kword_t base, kword_t words)
{
        struct mm_extent extent;
        unsigned int slot;
        kword_t end;

        if (words == 0UL || base > MM_HALF_MASK || words > MM_HALF_MASK ||
            base >= mm_core_words || words > mm_core_words - base)
                return MM_ERR_INVAL;
        end = base + words;
        slot = 0U;
        while (slot < mm_extent_count &&
            MM_EXTENT_BASE(&mm_extents[slot]) < base)
                ++slot;
        if (slot != 0U) {
                struct mm_extent *left = &mm_extents[slot - 1U];
                if (MM_EXTENT_BASE(left) + MM_EXTENT_WORDS(left) > base)
                        return MM_ERR_INVAL;
        }
        if (slot < mm_extent_count &&
            end > MM_EXTENT_BASE(&mm_extents[slot]))
                return MM_ERR_INVAL;
        extent.span = mm_span(base, words);
        extent.meta = mm_meta(MM_TYPE_FREE, 0U, 0U);
        if (mm_insert(slot, &extent) != MM_OK)
                return MM_ERR_DESCRIPTORS;
        mm_coalesce(slot);
        return MM_OK;
}

kword_t
mm_total_free(void)
{
        unsigned int i;
        kword_t total;

        total = 0UL;
        for (i = 0U; i < mm_extent_count; ++i)
                if (MM_EXTENT_TYPE(&mm_extents[i]) == MM_TYPE_FREE)
                        total += MM_EXTENT_WORDS(&mm_extents[i]);
        return total;
}

kword_t
mm_largest_free(void)
{
        unsigned int i;
        kword_t largest;
        kword_t words;

        largest = 0UL;
        for (i = 0U; i < mm_extent_count; ++i) {
                if (MM_EXTENT_TYPE(&mm_extents[i]) != MM_TYPE_FREE)
                        continue;
                words = MM_EXTENT_WORDS(&mm_extents[i]);
                if (words > largest)
                        largest = words;
        }
        return largest;
}

static int
mm_use_free(unsigned int slot, kword_t base, kword_t words,
    unsigned int type, unsigned int owner, kword_t *basep)
{
        struct mm_extent used;
        struct mm_extent tail;
        struct mm_extent *freep;
        kword_t free_base;
        kword_t free_end;
        kword_t before;
        kword_t after;
        unsigned int needed;

        freep = &mm_extents[slot];
        free_base = MM_EXTENT_BASE(freep);
        free_end = free_base + MM_EXTENT_WORDS(freep);
        if (base < free_base || words > free_end - base)
                return MM_ERR_INVAL;
        before = base - free_base;
        after = free_end - (base + words);
        needed = (before != 0UL ? 1U : 0U) + (after != 0UL ? 1U : 0U);
        if (mm_extent_count + needed > MM_MAX_EXTENTS)
                return MM_ERR_DESCRIPTORS;

        used.span = mm_span(base, words);
        used.meta = mm_meta(type, owner, 0U);
        if (before == 0UL) {
                *freep = used;
                if (after != 0UL) {
                        tail.span = mm_span(base + words, after);
                        tail.meta = mm_meta(MM_TYPE_FREE, 0U, 0U);
                        if (mm_insert(slot + 1U, &tail) != MM_OK)
                                return MM_ERR_DESCRIPTORS;
                }
        } else {
                freep->span = mm_span(free_base, before);
                if (mm_insert(slot + 1U, &used) != MM_OK)
                        return MM_ERR_DESCRIPTORS;
                if (after != 0UL) {
                        tail.span = mm_span(base + words, after);
                        tail.meta = mm_meta(MM_TYPE_FREE, 0U, 0U);
                        if (mm_insert(slot + 2U, &tail) != MM_OK)
                                return MM_ERR_DESCRIPTORS;
                }
        }
        *basep = base;
        return MM_OK;
}

static int
mm_has_aligned_fit(kword_t words, kword_t alignment)
{
        struct mm_extent *e;
        kword_t base;
        kword_t end;
        unsigned int i;

        for (i = 0U; i < mm_extent_count; ++i) {
                e = &mm_extents[i];
                if (MM_EXTENT_TYPE(e) != MM_TYPE_FREE)
                        continue;
                base = (MM_EXTENT_BASE(e) + alignment - 1UL) &
                    ~(alignment - 1UL);
                end = MM_EXTENT_BASE(e) + MM_EXTENT_WORDS(e);
                if (base <= end && words <= end - base)
                        return 1;
        }
        return 0;
}

static int
mm_alloc_aligned_raw(kword_t words, kword_t alignment, unsigned int type,
    unsigned int owner, unsigned int preference, kword_t *basep)
{
        struct mm_extent *freep;
        kword_t free_base;
        kword_t free_words;
        kword_t free_end;
        kword_t base;
        unsigned int i;
        int step;

        if (basep == 0 || words == 0UL || words > MM_HALF_MASK ||
            alignment == 0UL || alignment > MM_HALF_MASK ||
            (alignment & (alignment - 1UL)) != 0UL ||
            type == MM_TYPE_FREE || type > MM_TYPE_KERNEL_DYNAMIC ||
            preference > MM_ALLOC_HIGH)
                return MM_ERR_INVAL;

        if (preference == MM_ALLOC_LOW) {
                i = 0U;
                step = 1;
        } else {
                i = mm_extent_count;
                step = -1;
        }
        while ((step > 0 && i < mm_extent_count) || (step < 0 && i != 0U)) {
                if (step < 0)
                        --i;
                freep = &mm_extents[i];
                if (MM_EXTENT_TYPE(freep) == MM_TYPE_FREE) {
                        free_base = MM_EXTENT_BASE(freep);
                        free_words = MM_EXTENT_WORDS(freep);
                        free_end = free_base + free_words;
                        if (free_words >= words) {
                                if (preference == MM_ALLOC_LOW) {
                                        base = (free_base + alignment - 1UL) &
                                            ~(alignment - 1UL);
                                        if (base >= free_base && base <= free_end &&
                                            words <= free_end - base)
                                                return mm_use_free(i, base, words,
                                                    type, owner, basep);
                                } else {
                                        base = (free_end - words) &
                                            ~(alignment - 1UL);
                                        if (base >= free_base)
                                                return mm_use_free(i, base, words,
                                                    type, owner, basep);
                                }
                        }
                }
                if (step > 0)
                        ++i;
        }
        return mm_total_free() >= words ? MM_ERR_FRAGMENTED : MM_ERR_NOMEM;
}

static struct proc *
mm_process_owner(unsigned int owner, kword_t base)
{
        struct proc *p;

        if (owner >= PROC_NPROC)
                return 0;
        p = &proc_table[owner];
        if (PROC_STATE(p) == PROC_FREE || PROC_MEM_BASE(p) != base)
                return 0;
        return p;
}

int
mm_move_process(struct proc *p, unsigned int owner)
{
        struct mm_extent *extent;
        kword_t old_base;
        kword_t new_base;
        kword_t words;
        kword_t *src;
        kword_t *dst;
        unsigned int i;
        int rc;

        if (p == 0 || owner >= PROC_NPROC || p != &proc_table[owner] ||
            PROC_STATE(p) == PROC_FREE || PROC_STATE(p) == PROC_SRUN)
                return MM_ERR_BUSY;
        old_base = PROC_MEM_BASE(p);
        words = PROC_MEM_WORDS(p);
        extent = 0;
        for (i = 0U; i < mm_extent_count; ++i) {
                if (MM_EXTENT_BASE(&mm_extents[i]) == old_base) {
                        extent = &mm_extents[i];
                        break;
                }
        }
        if (extent == 0 || MM_EXTENT_TYPE(extent) != MM_TYPE_PROCESS ||
            MM_EXTENT_OWNER(extent) != owner ||
            MM_EXTENT_WORDS(extent) != words)
                return MM_ERR_INVAL;
        if (MM_EXTENT_PINS(extent) != 0U)
                return MM_ERR_BUSY;

        rc = mm_alloc_aligned_raw(words, 02000UL, MM_TYPE_PROCESS, owner,
            MM_ALLOC_HIGH, &new_base);
        if (rc != MM_OK)
                return rc;
        if (new_base <= old_base) {
                (void)mm_free(new_base, MM_TYPE_PROCESS, owner);
                return MM_ERR_FRAGMENTED;
        }

        src = (kword_t *)(unsigned long)old_base;
        dst = (kword_t *)(unsigned long)new_base;
        for (i = 0U; (kword_t)i < words; ++i)
                dst[i] = src[i];

        PROC_SET_MEM_BASE(p, new_base);
        mm_words_moved += words;
        rc = mm_free(old_base, MM_TYPE_PROCESS, owner);
        if (rc != MM_OK) {
                PROC_SET_MEM_BASE(p, old_base);
                (void)mm_free(new_base, MM_TYPE_PROCESS, owner);
                return rc;
        }
        return MM_OK;
}


int
mm_move_module(unsigned int owner)
{
        struct mm_extent *extent;
        kword_t old_base;
        kword_t new_base;
        kword_t words;
        unsigned int i;
        int rc;

        if (module_moves_enabled == 0U || owner == 0U ||
            owner > MODULE_RUNTIME_MAX ||
            MODULE_RUNTIME_IMAGE_WORDS(&module_runtime_descs[owner]) == 0UL)
                return MM_ERR_BUSY;
        old_base = MODULE_RUNTIME_BASE(&module_runtime_descs[owner]);
        words = MODULE_RUNTIME_EXTENT_WORDS(&module_runtime_descs[owner]);
        if (old_base == 0UL || words == 0UL)
                return MM_ERR_INVAL;
        extent = 0;
        for (i = 0U; i < mm_extent_count; ++i) {
                if (MM_EXTENT_BASE(&mm_extents[i]) == old_base) {
                        extent = &mm_extents[i];
                        break;
                }
        }
        if (extent == 0 || MM_EXTENT_TYPE(extent) != MM_TYPE_MODULE ||
            MM_EXTENT_OWNER(extent) != owner ||
            MM_EXTENT_WORDS(extent) != words)
                return MM_ERR_INVAL;
        if (MM_EXTENT_PINS(extent) != 0U)
                return MM_ERR_BUSY;

        rc = mm_alloc_aligned_raw(words, 1UL, MM_TYPE_MODULE, owner,
            MM_ALLOC_LOW, &new_base);
        if (rc != MM_OK)
                return rc;
        if (new_base >= old_base) {
                (void)mm_free(new_base, MM_TYPE_MODULE, owner);
                return MM_ERR_FRAGMENTED;
        }
        if (module_runtime_move(owner, new_base) != 0) {
                (void)mm_free(new_base, MM_TYPE_MODULE, owner);
                return MM_ERR_INVAL;
        }
        mm_words_moved += words;
        rc = mm_free(old_base, MM_TYPE_MODULE, owner);
        if (rc != MM_OK)
                return rc;
        return MM_OK;
}

int
mm_compact(kword_t words, kword_t alignment)
{
        struct mm_extent *extent;
        struct proc *p;
        kword_t base;
        unsigned int owner;
        unsigned int i;

        if (words == 0UL || alignment == 0UL ||
            (alignment & (alignment - 1UL)) != 0UL)
                return MM_ERR_INVAL;
        if (mm_total_free() < words)
                return MM_ERR_NOMEM;
        if (mm_has_aligned_fit(words, alignment))
                return MM_OK;
        ++mm_compaction_count;

        i = 0U;
        while (i < mm_extent_count) {
                extent = &mm_extents[i];
                if (MM_EXTENT_TYPE(extent) != MM_TYPE_MODULE ||
                    MM_EXTENT_PINS(extent) != 0U) {
                        ++i;
                        continue;
                }
                owner = MM_EXTENT_OWNER(extent);
                if (mm_move_module(owner) != MM_OK) {
                        ++i;
                        continue;
                }
                if (mm_has_aligned_fit(words, alignment))
                        return MM_OK;
                i = 0U;
        }

        i = 0U;
        while (i < mm_extent_count) {
                extent = &mm_extents[i];
                if (MM_EXTENT_TYPE(extent) != MM_TYPE_PROCESS ||
                    MM_EXTENT_PINS(extent) != 0U) {
                        ++i;
                        continue;
                }
                base = MM_EXTENT_BASE(extent);
                owner = MM_EXTENT_OWNER(extent);
                p = mm_process_owner(owner, base);
                if (p == 0 || mm_move_process(p, owner) != MM_OK) {
                        ++i;
                        continue;
                }
                if (mm_has_aligned_fit(words, alignment))
                        return MM_OK;
                i = 0U;
        }
        return MM_ERR_FRAGMENTED;
}

int
mm_alloc_aligned(kword_t words, kword_t alignment, unsigned int type,
    unsigned int owner, unsigned int preference, kword_t *basep)
{
        int rc;

        rc = mm_alloc_aligned_raw(words, alignment, type, owner,
            preference, basep);
        if (rc == MM_ERR_FRAGMENTED) {
                rc = mm_compact(words, alignment);
                if (rc == MM_OK)
                        rc = mm_alloc_aligned_raw(words, alignment, type, owner,
                            preference, basep);
        }
        if (rc == MM_OK)
                return MM_OK;
        if (rc != MM_ERR_NOMEM) {
                ++mm_allocation_failures;
                return rc;
        }
        if (proc_swap_reclaim(words, alignment,
            type == MM_TYPE_PROCESS ? owner : PROC_NPROC) != 0) {
                ++mm_allocation_failures;
                return MM_ERR_NOMEM;
        }
        rc = mm_alloc_aligned_raw(words, alignment, type, owner,
            preference, basep);
        if (rc == MM_ERR_FRAGMENTED) {
                rc = mm_compact(words, alignment);
                if (rc == MM_OK)
                        rc = mm_alloc_aligned_raw(words, alignment, type, owner,
                            preference, basep);
        }
        if (rc != MM_OK)
                ++mm_allocation_failures;
        return rc;
}

int
mm_alloc(kword_t words, unsigned int type, unsigned int owner,
    unsigned int preference, kword_t *basep)
{
        return mm_alloc_aligned(words, 1UL, type, owner, preference, basep);
}

static void
mm_coalesce(unsigned int slot)
{
        struct mm_extent *left;
        struct mm_extent *right;
        kword_t base;
        kword_t words;

        if (slot != 0U && MM_EXTENT_TYPE(&mm_extents[slot - 1U]) == MM_TYPE_FREE) {
                left = &mm_extents[slot - 1U];
                right = &mm_extents[slot];
                if (MM_EXTENT_BASE(left) + MM_EXTENT_WORDS(left) ==
                    MM_EXTENT_BASE(right)) {
                        words = MM_EXTENT_WORDS(left) + MM_EXTENT_WORDS(right);
                        left->span = mm_span(MM_EXTENT_BASE(left), words);
                        mm_delete(slot);
                        --slot;
                }
        }
        if (slot + 1U < mm_extent_count &&
            MM_EXTENT_TYPE(&mm_extents[slot + 1U]) == MM_TYPE_FREE) {
                left = &mm_extents[slot];
                right = &mm_extents[slot + 1U];
                base = MM_EXTENT_BASE(left);
                if (base + MM_EXTENT_WORDS(left) == MM_EXTENT_BASE(right)) {
                        words = MM_EXTENT_WORDS(left) + MM_EXTENT_WORDS(right);
                        left->span = mm_span(base, words);
                        mm_delete(slot + 1U);
                }
        }
}

int
mm_free(kword_t base, unsigned int type, unsigned int owner)
{
        unsigned int i;
        struct mm_extent *extent;

        for (i = 0U; i < mm_extent_count; ++i) {
                extent = &mm_extents[i];
                if (MM_EXTENT_BASE(extent) != base)
                        continue;
                if (MM_EXTENT_TYPE(extent) != type ||
                    MM_EXTENT_OWNER(extent) != owner)
                        return MM_ERR_INVAL;
                if (MM_EXTENT_PINS(extent) != 0U)
                        return MM_ERR_BUSY;
                extent->meta = mm_meta(MM_TYPE_FREE, 0U, 0U);
                mm_coalesce(i);
                return MM_OK;
        }
        return MM_ERR_INVAL;
}

int
mm_pin(kword_t base)
{
        unsigned int i;
        unsigned int pins;
        struct mm_extent *extent;

        for (i = 0U; i < mm_extent_count; ++i) {
                extent = &mm_extents[i];
                if (MM_EXTENT_BASE(extent) != base)
                        continue;
                if (MM_EXTENT_TYPE(extent) == MM_TYPE_FREE)
                        return MM_ERR_INVAL;
                pins = MM_EXTENT_PINS(extent);
                if (pins == MM_PIN_MASK)
                        return MM_ERR_BUSY;
                extent->meta += (kword_t)1UL << MM_PIN_SHIFT;
                return MM_OK;
        }
        return MM_ERR_INVAL;
}

int
mm_unpin(kword_t base)
{
        unsigned int i;
        struct mm_extent *extent;

        for (i = 0U; i < mm_extent_count; ++i) {
                extent = &mm_extents[i];
                if (MM_EXTENT_BASE(extent) != base)
                        continue;
                if (MM_EXTENT_PINS(extent) == 0U)
                        return MM_ERR_INVAL;
                extent->meta -= (kword_t)1UL << MM_PIN_SHIFT;
                return MM_OK;
        }
        return MM_ERR_INVAL;
}
