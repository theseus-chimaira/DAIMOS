/**
 * @file proc_pure_ram_v1377.c
 * @brief Raw in-RAM PURE suffix lifetime for a swapless PDP-6.
 *
 * This staging code is intentionally NOT in the production kernel link.
 * In particular, VFS executable immutability and scheduler publication
 * must be integrated and tested before these transactions can be called.
 *
 * The PDP-6 has no split text/data mapping. A quiesced process saves its
 * user ACs in the stable u-area; complete PURE-only disk-swap sectors are
 * reconstructible from its validated immutable DXR. We preserve every word
 * outside that prefix unchanged in a separate, owner-checked RAM extent.
 *
 * The existing PACKED physical type/storage bit describes INACTIVE RAM
 * here, not necessarily compression. We reuse the MM-authoritative layout
 * registry and its two-bit state map without adding a disk record or an
 * extra per-slot descriptor. Storage is not published until after the
 * source process extent is released. All fallible operations before that
 * commit have explicit rollback. All successful transactions end with
 * the process either fully resident or distinctly inactive, never both.
 */
#include "proc_pure_ram_v1377.h"
#include "proc_packed_owner_v37.h"
#include "proc_swap.h"
#include "vm_pdp6.h"
#include "fs_mres.h"
#include "exec.h"
#include "mm_internal.h"

/* This entire object must be absent from any swap-enabled build. The
 * feature uses only existing per-process PURE metadata and managed RAM;
 * it must not pull BACKSTORE symbols into a minimal tape-root kernel.
 * Once production build profiles are implemented, only an explicitly
 * swapless target may define this gate.
 */
#if !defined(DAIMOS_SWAPLESS_PURE_RAM_V1377) || \
    DAIMOS_SWAPLESS_PURE_RAM_V1377 != 1
#error proc_pure_ram_v1377_requires_swapless_build
#endif

#ifdef DAIMOS_TEST_HOST_WORD_ADDR
#define PURE_RAM_PTR_V1377(x) \
        ((kword_t *)(unsigned long)((x) * sizeof(kword_t)))
#else
#define PURE_RAM_PTR_V1377(x) ((kword_t *)(unsigned long)(x))
#endif

#define PURE_RAM_SECTOR_WORDS_V1377 0200U

/* No full-sector prefix exists for small PURE text. Never elide a partial
 * sector: the first 020 logical words are the saved user AC area, while the
 * text/DATA boundary may share the last sector. This is the *same*
 * conservative prefix convention as proc_swap_skip_words.
 */
static kword_t
pure_ram_omit_v1377(kword_t backing, kword_t process_words)
{
        kword_t text = (backing >> PROC_SWAP_TEXT_SHIFT) &
            PROC_SWAP_TEXT_MASK;
        kword_t omit;
        if (!text || text > MM_HALF_MASK - EXEC_USER_ORIGIN)
                return 0UL;
        omit = (text + EXEC_USER_ORIGIN) &
            ~((kword_t)PURE_RAM_SECTOR_WORDS_V1377 - 1UL);
        if (!omit || omit >= process_words)
                return 0UL;
        return omit;
}

/* Validate owner/type/size and reject ambiguous duplicate MM extents.
 * The packed suffix is never a live APR mapping and is never movable by
 * the process compactor. MM descriptor pointers must not survive mutations.
 */
static int
pure_ram_owned_v1377(unsigned int slot, kword_t base, kword_t words,
    unsigned int type)
{
        int i;
        unsigned int found = 0U;
        if (!base || !words)
                return 0;
        for (i = 0; i < mm_extent_count; ++i) {
                const struct mm_extent *e = &mm_extents[i];
                if (MM_EXTENT_OWNER(e) != slot)
                        continue;
                if (MM_EXTENT_TYPE(e) == type) {
                        if (++found != 1U || MM_EXTENT_BASE(e) != base ||
                            MM_EXTENT_WORDS(e) != words ||
                            MM_EXTENT_PINNED(e))
                                return 0;
                } else if (MM_EXTENT_TYPE(e) ==
                    MM_TYPE_PACKED_PROCESS && type == MM_TYPE_PROCESS)
                        return 0;
        }
        return found == 1U;
}

/* Caller holds TRANSITION and executable source immutability exclusion. */
static int
pure_ram_common_v1377(unsigned int slot, struct proc **process,
    kword_t *backing, kword_t *words, kword_t *omit,
    unsigned int terminal_discard)
{
        struct proc *p;
        struct proc_packed_layout_v36 *r = &proc_packed_runtime_v38;
        kword_t *uarea;

        if (!slot || !proc_table || !proc_swap_records ||
            !proc_storage_runtime_v29.words || !r->words ||
            slot >= proc_storage_runtime_v29.slots ||
            slot >= r->slots || r->slots != proc_storage_runtime_v29.slots)
                return MM_ERR_INVAL;
        p = &proc_table[slot];
        if (!PROC_TRANSITION(p) || !PROC_HAS_UAREA(p) ||
            (terminal_discard && PROC_STATE(p) == PROC_FREE) ||
            (!terminal_discard && PROC_STATE(p) != PROC_SLEEP &&
                PROC_STATE(p) != PROC_STOP) ||
            !PROC_UAREA_BASE(p))
                return MM_ERR_INVAL;
        uarea = PURE_RAM_PTR_V1377(PROC_UAREA_BASE(p));
        if (!terminal_discard && (uarea[PROC_FDCTL_OFFSET] &
            (PROC_USER_MAP_BIT | PROC_FATAL_CLOSE_BIT)))
                return MM_ERR_BUSY;
        *words = VM_SPACE_WORDS(p);
        *backing = proc_swap_records[slot].state;
        *omit = pure_ram_omit_v1377(*backing, *words);
        if (!*words || !*omit)
                return MM_ERR_INVAL;
        *process = p;
        return MM_OK;
}

/* Test registry capacity before any MM allocation. The actual reservation
 * happens after the suffix has been copied, while lifecycle exclusion
 * prevents any intervening slot publication.
 */
static int
pure_ram_registry_room_v1377(unsigned int slot)
{
        struct proc_packed_layout_v36 *r = &proc_packed_runtime_v38;
        unsigned int i;
        if (proc_packed_layout_find_v36(r, slot))
                return 0;
        if (r->format == PROC_PACKED_LAYOUT_DENSE_V36)
                return r->words[slot] == 0UL;
        if (r->format != PROC_PACKED_LAYOUT_SPARSE_V36)
                return 0;
        for (i = 0U; i < r->capacity; ++i)
                if (!r->words[2U * i] && !r->words[2U * i + 1U])
                        return 1;
        return 0;
}

int
proc_pure_ram_drop_v1377(unsigned int slot)
{
        struct proc *p;
        struct proc_packed_layout_v36 *r = &proc_packed_runtime_v38;
        kword_t backing, words, omit, retained, base, target, *entry;
        kword_t *uarea;
        int source_pin_live = 0;
        int source_pin_uncertain = 0;
        int rc;

        rc = pure_ram_common_v1377(slot, &p, &backing, &words, &omit, 0U);
        if (rc != MM_OK)
                return rc;
        base = VM_PDP6_BASE(p);
        retained = words - omit;
        uarea = PURE_RAM_PTR_V1377(PROC_UAREA_BASE(p));
        if (!base || uarea[PROC_SWAP_BACKING_OFFSET] ||
            proc_storage_get_v28(&proc_storage_runtime_v29, slot) !=
                PROC_STORAGE_RESIDENT ||
            !pure_ram_registry_room_v1377(slot) ||
            !pure_ram_owned_v1377(slot, base, words, MM_TYPE_PROCESS))
                return MM_ERR_INVAL;

        rc = mm_pin(base);
        if (rc != MM_OK)
                return rc;
        source_pin_live = 1;
        rc = mm_alloc_aligned_noreclaim(retained, 1UL,
            MM_TYPE_PACKED_PROCESS, slot, MM_ALLOC_HIGH, &target);
        if (rc != MM_OK)
                goto undo_source_pin;
        rc = mm_pin(target);
        if (rc != MM_OK)
                goto undo_target;
        fs_copy_words(PURE_RAM_PTR_V1377(base + omit),
            PURE_RAM_PTR_V1377(target), (unsigned int)retained);
        entry = proc_packed_layout_reserve_v36(r, slot);
        if (!entry) {
                rc = MM_ERR_DESCRIPTORS;
                goto undo_target_pin;
        }
        *entry = (omit << 18U) | retained;

        rc = mm_unpin(base);
        if (rc != MM_OK) {
                /* Unknown pin status on a failed operation: never retry an
                 * unpin blindly, and keep TRANSITION for owner recovery.
                 */
                source_pin_uncertain = 1;
                goto undo_layout;
        }
        source_pin_live = 0;
        rc = mm_free(base, MM_TYPE_PROCESS, slot);
        if (rc != MM_OK)
                goto undo_layout;

        /* The MM free is the only fallible commit operation. Mode/layout
         * and backing words are published under the held TRANSITION.
         * A dropped PURE process is now nonresident and guards its vnode.
         */
        uarea[PROC_SWAP_BACKING_OFFSET] = backing;
        VM_PDP6_SET_BASE(p, 0UL);
        proc_storage_runtime_v29.words[
            slot / PROC_STORAGE_MAP_SLOTS_PER_WORD_V28] |=
            (kword_t)PROC_STORAGE_PACKED <<
            ((slot % PROC_STORAGE_MAP_SLOTS_PER_WORD_V28) * 2U);
        rc = mm_unpin(target);
        if (rc != MM_OK)
                return MM_ERR_BUSY; /* committed, leave TRANSITION held */
        PROC_CLEAR_TRANSITION(p);
        return MM_OK;

undo_layout:
        (void)proc_packed_layout_clear_v36(r, slot);
undo_target_pin:
        (void)mm_unpin(target);
undo_target:
        (void)mm_free(target, MM_TYPE_PACKED_PROCESS, slot);
undo_source_pin:
        /* Do not unpin twice if source unpin already succeeded. A transient
         * unpin refusal remains a held pin and lifecycle exclusion stays
         * with the caller until the MM error is diagnosed.
         */
        if (source_pin_live && !source_pin_uncertain)
                (void)mm_unpin(base);
        return rc;
}

int
proc_pure_ram_restore_v1377(unsigned int slot,
    proc_pure_ram_reload_v1377 reload)
{
        struct proc *p;
        struct proc_packed_v1 saved;
        struct proc_packed_layout_v36 *r = &proc_packed_runtime_v38;
        kword_t backing, words, omit, retained, source, target, *uarea;
        int source_pin_live = 0;
        int source_pin_uncertain = 0;
        int rc;

        if (!reload)
                return MM_ERR_INVAL;
        rc = pure_ram_common_v1377(slot, &p, &backing, &words, &omit, 0U);
        if (rc != MM_OK)
                return rc;
        uarea = PURE_RAM_PTR_V1377(PROC_UAREA_BASE(p));
        retained = words - omit;
        if (VM_PDP6_BASE(p) || !backing ||
            uarea[PROC_SWAP_BACKING_OFFSET] != backing ||
            proc_packed_owner_lookup_v37(r, slot, &saved) != MM_OK ||
            PROC_PACKED_OFFSET(&saved) != omit ||
            PROC_PACKED_DECODED(&saved) != retained ||
            PROC_PACKED_WORDS(&saved) != retained)
                return MM_ERR_INVAL;

        source = PROC_PACKED_BASE(&saved);
        rc = mm_pin(source);
        if (rc != MM_OK)
                return rc;
        source_pin_live = 1;
        rc = mm_alloc_aligned_noreclaim(words, VM_PDP6_ALIGN_WORDS,
            MM_TYPE_PROCESS, slot, MM_ALLOC_HIGH, &target);
        if (rc != MM_OK)
                goto undo_source_pin;
        rc = mm_pin(target);
        if (rc != MM_OK)
                goto undo_target;
        fs_zero_words(PURE_RAM_PTR_V1377(target), (unsigned int)omit);
        if (reload(backing, target, PROC_UAREA_BASE(p)) != 0) {
                rc = MM_ERR_INVAL;
                goto undo_target_pin;
        }
        /* A compressed executable may decode the whole initial image.
         * Overlaying EVERY retained mutable word after reload is mandatory.
         */
        fs_copy_words(PURE_RAM_PTR_V1377(source),
            PURE_RAM_PTR_V1377(target + omit), (unsigned int)retained);

        rc = mm_unpin(source);
        if (rc != MM_OK) {
                source_pin_uncertain = 1;
                goto undo_target_pin;
        }
        source_pin_live = 0;
        rc = mm_free(source, MM_TYPE_PACKED_PROCESS, slot);
        if (rc != MM_OK)
                goto undo_target_pin;

        /* Release is the commit: no further fallible owner mutation.
         * A validated exact slot layout cannot disappear while locked.
         */
        (void)proc_packed_layout_clear_v36(r, slot);
        proc_storage_runtime_v29.words[
            slot / PROC_STORAGE_MAP_SLOTS_PER_WORD_V28] &=
            ~((kword_t)3UL <<
            ((slot % PROC_STORAGE_MAP_SLOTS_PER_WORD_V28) * 2U));
        VM_PDP6_SET_BASE(p, target);
        uarea[PROC_SWAP_BACKING_OFFSET] = 0UL;
        rc = mm_unpin(target);
        if (rc != MM_OK)
                return MM_ERR_BUSY; /* resident but still locked/pinned */
        PROC_CLEAR_TRANSITION(p);
        return MM_OK;

undo_target_pin:
        (void)mm_unpin(target);
undo_target:
        (void)mm_free(target, MM_TYPE_PROCESS, slot);
undo_source_pin:
        /* Every failed allocation, pin and restore must release the
         * source pin once. In particular, an MM_ERR_NOMEM before the
         * destination exists must never strand a pinned inactive image.
         */
        if (source_pin_live && !source_pin_uncertain)
                (void)mm_unpin(source);
        return rc;
}

int
proc_pure_ram_discard_v1377(unsigned int slot)
{
        struct proc *p;
        kword_t backing, words, omit, *uarea;
        int rc;

        rc = pure_ram_common_v1377(slot, &p, &backing, &words, &omit, 1U);
        if (rc != MM_OK)
                return rc;
        uarea = PURE_RAM_PTR_V1377(PROC_UAREA_BASE(p));
        if (VM_PDP6_BASE(p) ||
            uarea[PROC_SWAP_BACKING_OFFSET] != backing)
                return MM_ERR_INVAL;
        rc = proc_packed_owner_discard_v37(&proc_packed_runtime_v38,
            slot);
        if (rc != MM_OK)
                return rc;
        proc_swap_records[slot].state = 0UL;
        uarea[PROC_SWAP_BACKING_OFFSET] = 0UL;
        uarea[PROC_CRED_OFFSET] &= ~PROC_SWAP_MOUNT_EXT_BIT;
        /* Caller still holds TRANSITION for the rest of process teardown. */
        return MM_OK;
}
