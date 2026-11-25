/**
 * @file proc_pure_ram_v1381.c
 * @brief Zero-copy PURE prefix release and registry-free RAM restore.
 *
 * The MM extent table is authoritative. The existing two-bit storage
 * mode distinguishes resident from inactive RAM; a RAW mutable suffix
 * is recovered by exact owner/type/word-count matching. The PURE skip
 * boundary is derivable from the saved backing and original VM size.
 * No second registry, compression, disk allocator or temporary suffix
 * allocation is needed. MM rebases the existing pinned process extent to
 * its high-address writable suffix without copying a single data word.
 *
 * v1611: DROP is linked only in the explicit experimental swapless MRES.
 * Default KCORE and the accepted disk-swap profile remain unaffected.
 */
#include "proc_pure_ram_v1381.h"
#include "proc_swap.h"
#include "proc_storage_map_v28.h"
#include "vm_pdp6.h"
#include "exec.h"
#include "mm_internal.h"
#include "mm_pure_rebase_v1381.h"
#include "fs_mres.h"
#include "dtfs_media.h"
#include "dtfs.h"
#include "d6fs_provider.h"
#include "proc_pure_lifecycle_v1604.h"
#include "proc_pure_admit_v1608.h"
#include "vm_pdp6_pure_range_v1604.h"

#if !defined(DAIMOS_SWAPLESS_PURE_RAM_V1381) || \
    DAIMOS_SWAPLESS_PURE_RAM_V1381 != 1
#error proc_pure_ram_v1381_requires_swapless_build
#endif

#ifdef DAIMOS_TEST_HOST_WORD_ADDR
#define PRAM_PTR(x) ((kword_t *)(unsigned long)((x) * sizeof(kword_t)))
#else
#define PRAM_PTR(x) ((kword_t *)(unsigned long)(x))
#endif

/* Reuse the already linked assembler's exact two-bit mode operations. */
extern int proc_storage_expect_v30(unsigned int, unsigned int);
extern int proc_storage_publish_v30(unsigned int, unsigned int,
    unsigned int);
extern void proc_swap_storage_invariant_v30(void);
extern kword_t mach_pi_disable(void);
extern void mach_pi_restore(kword_t saved);

/* PI-excluded proof of a sole user extent. A zero expected base permits
 * discovery only before a transaction starts. The shared MM proof
 * checks every range before this scan. MODULE and KERNEL_DYNAMIC use
 * independent owner namespaces.
 */
kword_t
proc_pure_owner_v1918(unsigned int slot, unsigned int type,
    kword_t base, kword_t words, unsigned int pins)
{
        int i;
        kword_t found = 0UL;

        if (!words || mm_pure_table_v1922() != MM_OK)
                return 0UL;
        for (i = 0; i < mm_extent_count; ++i) {
                const struct mm_extent *e = &mm_extents[i];
                unsigned int kind = MM_EXTENT_TYPE(e);
                kword_t start = MM_EXTENT_BASE(e);
                kword_t size = MM_EXTENT_WORDS(e);

                if (MM_EXTENT_OWNER(e) != slot ||
                    kind == MM_TYPE_MODULE || kind == MM_TYPE_KERNEL_DYNAMIC)
                        continue;
                if (found || kind != type || size != words ||
                    MM_EXTENT_PINS(e) != pins || (base && base != start))
                        return 0UL;
                found = start;
        }
        return found;
}

/*
 * Caller already holds the process transition. The live executable
 * backing guard is supplied by proc_swap_attach and the resident/
 * nonresident guard scan. No new kernel word is needed.
 */
static int
pram_prepare(unsigned int slot, unsigned int action,
    struct proc **pp, kword_t *backingp, kword_t *wordsp,
    kword_t *skipp)
{
        struct proc *p;
        kword_t backing, words, skip;
        int rc;

        rc = pure_life_prepare_v1604(slot, action, &p, &backing,
            &words);
        if (rc != MM_OK)
                return rc;
        if (action != PURE_LIFE_TERMINAL_V1610) {
                rc = pure_admit_source_v1608(p, backing);
                if (rc != MM_OK)
                        return rc;
        }
        rc = pdp6_pure_skip_v1604(backing, words, &skip);
        if (rc != MM_OK)
                return rc;
        *pp = p;
        *backingp = backing;
        *wordsp = words;
        *skipp = skip;
        return MM_OK;
}

static void
pram_publish(unsigned int slot, unsigned int before, unsigned int after)
{
        if (proc_storage_publish_v30(slot, before, after) != 0)
                proc_swap_storage_invariant_v30();
}

/* Existing mount-owned state only. These pointers are published by KINIT
 * and retired by VFS; no provider dispatch or physical I/O here. */
extern kword_t *dtfs_media_base, *tsfs_file_shape_base;
extern kword_t *tsfs_extent_media_base, *d6fs_reader_slots_base;
extern int fs_mres_no_service(void);

/* Called with PI excluded, after ordinary backing admission. A known
 * unavailable source must not cost the only resident copy of its text.
 * A healthy publication is not proof of physical media continuity. */
static int
pram_drop_available_v1926(kword_t backing, const kword_t *u)
{
        unsigned int mount, provider;
        kword_t service, media;
        vnode_t root;

        mount = (unsigned int)((backing >> PROC_SWAP_MOUNT_SHIFT) &
            PROC_SWAP_MOUNT_MASK);
        if (u[PROC_CRED_OFFSET] & PROC_SWAP_MOUNT_EXT_BIT)
                mount |= 020U;
        provider = (unsigned int)((u[PROC_CRED_OFFSET] &
            PROC_SWAP_PROVIDER_MASK_V1924) >> PROC_SWAP_PROVIDER_SHIFT_V1924);
        /* Recheck the binding inside the same PI window as rebase. */
        if (!vfs_mount_root_base || mount >= vfs_mount_limit)
                return 0;
        root = vfs_mount_root_base[mount];
        if (VFS_PROVIDER(root) != provider || VFS_MOUNT_ID(root) != mount + 1U)
                return 0;
        switch (provider) {
        case MEMFS_PROVIDER:
                service = fs_memfs_service_jump;
                break;
        case DTFS_PROVIDER:
                if (!dtfs_media_base || !tsfs_file_shape_base ||
                    !tsfs_extent_media_base)
                        return 0;
                media = dtfs_media_base[mount];
                if (media & (DTFS_MEDIA_RD_SUSPECT_V1522 |
                    DTFS_MEDIA_WR_UNCERT_V1522 | DTFS_MEDIA_QUIESCING_V1562))
                        return 0;
                if ((media & DTFS_MEDIA_INTENT_ARMED_V1553) &&
                    (!(media & DTFS_MEDIA_FORMAT_V2_V1553) || vfs_readonly(root)))
                        return 0;
                /* Stamped identity is a pair, mandatory for DTVOL2. */
                if ((!tsfs_file_shape_base[mount] != !tsfs_extent_media_base[mount]) ||
                    ((media & DTFS_MEDIA_FORMAT_V2_V1553) &&
                        !tsfs_file_shape_base[mount]))
                        return 0;
                service = fs_dtfs_service_jump;
                break;
        case D6FS_PROVIDER:
                if (!d6fs_reader_slots_base || !d6fs_reader_slots_base[mount] ||
                    !dtfs_media_base || !tsfs_file_shape_base ||
                    !tsfs_extent_media_base || dtfs_media_base[mount] ||
                    !(tsfs_file_shape_base[mount] | tsfs_extent_media_base[mount]))
                        return 0;
                service = fs_d6fs_service_jump;
                break;
        case TSFS_PROVIDER:
                if (!dtfs_media_base || !tsfs_file_shape_base ||
                    !tsfs_extent_media_base || !dtfs_media_base[mount] ||
                    !tsfs_file_shape_base[mount] || !tsfs_extent_media_base[mount])
                        return 0;
                service = fs_tsfs_service_jump;
                break;
        default:
                return 0;
        }
        /* MINIT and module relocation publish a plain JRST, while an
         * absent service points at the resident refusing fallback. */
        return (service >> 18U) == 0254000UL &&
            (service & MM_HALF_MASK) &&
            (service & MM_HALF_MASK) != (kword_t)(unsigned long)fs_mres_no_service;
}

int
proc_pure_ram_drop_v1381(unsigned int slot)
{
        struct proc *p;
        kword_t backing, words, skip, source, suffix;
        kword_t pi_state;
        kword_t *u;
        int rc;

        rc = pram_prepare(slot, PURE_LIFE_DROP_V1610,
            &p, &backing, &words, &skip);
        if (rc != MM_OK)
                return rc;
        if (proc_storage_expect_v30(slot, PROC_STORAGE_RESIDENT) != 0)
                return MM_ERR_INVAL;
        source = VM_PDP6_BASE(p);
        u = PRAM_PTR(PROC_UAREA_BASE(p));
        if (!source || u[PROC_SWAP_BACKING_OFFSET] ||
            (u[PROC_FINALIZE_OFFSET] &
             (PROC_FINALIZE_MARK | PROC_SWAP_RETRY_TAG_V1507)) ||
            (source & (VM_PDP6_ALIGN_WORDS - 1UL)) ||
            (words & (VM_PDP6_ALIGN_WORDS - 1UL)))
                return MM_ERR_INVAL;
        pi_state = mach_pi_disable();
        if (!pram_drop_available_v1926(backing, u)) {
                rc = MM_ERR_INVAL;
                goto out;
        }
        if (!proc_pure_owner_v1918(slot, MM_TYPE_PROCESS, source,
            words, 0U)) {
                rc = MM_ERR_INVAL;
                goto out;
        }
        suffix = source + skip;
        /* Save the original suffix identity before any mutation. A
         * failed pin/rebase must not authorize an unrelated RAW owner
         * discovered later by the service. */
        u[PROC_FINALIZE_OFFSET] = PROC_PURE_INPLACE_ESCROW_V1919(suffix,
            PROC_PURE_DROP_TAG_V1919);
        rc = mm_pin(source);
        if (rc != MM_OK)
                goto resident_done;
        if (!proc_pure_owner_v1918(slot, MM_TYPE_PROCESS, source,
            words, 1U))
                goto uncertain;

        /* Both shapes keep the same high end. Reconcile a reported
         * failure using the original physical addresses, never a
         * newly discovered descriptor. PI remains off through
         * rebase, mode publication and checked pin retirement. */
        rc = mm_pure_rebase_pinned_v1381(source, slot, skip, words);
        if (proc_pure_owner_v1918(slot, MM_TYPE_PACKED_PROCESS, suffix,
            words - skip, 1U)) {
                u[PROC_SWAP_BACKING_OFFSET] = backing;
                VM_PDP6_SET_BASE(p, 0UL);
                pram_publish(slot, PROC_STORAGE_RESIDENT,
                    PROC_STORAGE_PACKED);
                rc = mm_unpin(suffix);
                if (!proc_pure_owner_v1918(slot, MM_TYPE_PACKED_PROCESS,
                    suffix, words - skip, 0U))
                        goto uncertain;
                u[PROC_FINALIZE_OFFSET] = 0UL;
                if (rc == MM_OK)
                        PROC_CLEAR_TRANSITION(p);
                goto out;
        }
        if (rc == MM_OK ||
            !proc_pure_owner_v1918(slot, MM_TYPE_PROCESS, source,
                words, 1U))
                goto uncertain;
        if (mm_unpin(source) != MM_OK)
                rc = MM_ERR_BUSY;
resident_done:
        if (!proc_pure_owner_v1918(slot, MM_TYPE_PROCESS, source,
            words, 0U))
                goto uncertain;
        u[PROC_FINALIZE_OFFSET] = 0UL;
        goto out;
uncertain:
        rc = MM_ERR_BUSY;
out:
        mach_pi_restore(pi_state);
        return rc;
}

/* v1611: the resident swapless MRES only needs the zero-copy DROP.
 * The historical unbounded restore and staged terminal cleanup are
 * retained for native regression, but neither is linked into that MRES.
 * Production FULL restoration uses the capacity-bounded v1391 path;
 * production terminal cleanup is already handled by v1609 KCORE. */
#ifndef DAIMOS_PURE_DROP_ONLY_V1611
int
proc_pure_ram_restore_v1381(unsigned int slot,
    proc_pure_ram_reload_v1381 reload)
{
        struct proc *p;
        kword_t backing, words, skip, raw, source, target;
        kword_t *u;
        int rc;
        int source_pinned = 0;
        int target_pinned = 0;
        int source_uncertain = 0;

        if (!reload)
                return MM_ERR_INVAL;
        rc = pram_prepare(slot, PURE_LIFE_RESTORE_V1610,
            &p, &backing, &words, &skip);
        if (rc != MM_OK)
                return rc;
        if (proc_storage_expect_v30(slot, PROC_STORAGE_PACKED) != 0)
                return MM_ERR_INVAL;
        u = PRAM_PTR(PROC_UAREA_BASE(p));
        raw = words - skip;
        if (VM_PDP6_BASE(p) ||
            u[PROC_SWAP_BACKING_OFFSET] != backing ||
            !(source = proc_pure_owner_v1918(slot, MM_TYPE_PACKED_PROCESS,
                0UL, raw, 0U)))
                return MM_ERR_INVAL;
        rc = mm_pin(source);
        if (rc != MM_OK)
                return rc;
        source_pinned = 1;
        rc = mm_alloc_aligned_noreclaim(words, VM_PDP6_ALIGN_WORDS,
            MM_TYPE_PROCESS, slot, MM_ALLOC_HIGH, &target);
        if (rc != MM_OK)
                goto fail_source;
        rc = mm_pin(target);
        if (rc != MM_OK)
                goto fail_target;
        target_pinned = 1;
        fs_zero_words(PRAM_PTR(target), (unsigned int)skip);
        if (reload(backing, target, PROC_UAREA_BASE(p))) {
                rc = MM_ERR_INVAL;
                goto fail_target;
        }
        /* Decode/reload may have initialized writable words. The raw
         * inactive suffix is always authoritative and overlays them.
         */
        fs_copy_words(PRAM_PTR(source), PRAM_PTR(target + skip),
            (unsigned int)raw);
        rc = mm_unpin(source);
        if (rc != MM_OK) {
                source_uncertain = 1;
                goto fail_target;
        }
        source_pinned = 0;
        rc = mm_free(source, MM_TYPE_PACKED_PROCESS, slot);
        if (rc != MM_OK)
                goto fail_target;

        VM_PDP6_SET_BASE(p, target);
        u[PROC_SWAP_BACKING_OFFSET] = 0UL;
        pram_publish(slot, PROC_STORAGE_PACKED, PROC_STORAGE_RESIDENT);
        rc = mm_unpin(target);
        if (rc != MM_OK)
                return MM_ERR_BUSY;
        PROC_CLEAR_TRANSITION(p);
        return MM_OK;

fail_target:
        if (target_pinned)
                (void)mm_unpin(target);
        (void)mm_free(target, MM_TYPE_PROCESS, slot);
fail_source:
        if (source_pinned && !source_uncertain)
                (void)mm_unpin(source);
        return rc;
}

int
proc_pure_ram_discard_v1381(unsigned int slot)
{
        struct proc *p;
        kword_t backing, words, skip, source;
        kword_t *u;
        int rc;

        rc = pram_prepare(slot, PURE_LIFE_TERMINAL_V1610,
            &p, &backing, &words, &skip);
        if (rc != MM_OK)
                return rc;
        u = PRAM_PTR(PROC_UAREA_BASE(p));
        if (VM_PDP6_BASE(p) ||
            u[PROC_SWAP_BACKING_OFFSET] != backing ||
            proc_storage_expect_v30(slot, PROC_STORAGE_PACKED) != 0 ||
            !(source = proc_pure_owner_v1918(slot, MM_TYPE_PACKED_PROCESS,
                0UL, words - skip, 0U)))
                return MM_ERR_INVAL;
        rc = mm_free(source, MM_TYPE_PACKED_PROCESS, slot);
        if (rc != MM_OK)
                return rc;
        pram_publish(slot, PROC_STORAGE_PACKED, PROC_STORAGE_RESIDENT);
        proc_swap_records[slot].state = 0UL;
        u[PROC_SWAP_BACKING_OFFSET] = 0UL;
        u[PROC_CRED_OFFSET] &= ~PROC_SWAP_ID_MASK_V1924;
        /* Teardown owns TRANSITION until the complete slot is retired. */
        return MM_OK;
}
#endif /* !DAIMOS_PURE_DROP_ONLY_V1611 */
