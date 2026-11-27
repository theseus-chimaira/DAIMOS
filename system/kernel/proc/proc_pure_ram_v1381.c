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
                    !tsfs_file_shape_base[mount] || !tsfs_extent_media_base[mount] ||
                    (tsfs_file_shape_base[mount] & TSFS_MEDIA_SUSPECT_V1928))
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

#ifdef DAIMOS_PBM_PREFIX_V1978
#if DAIMOS_PBM_PREFIX_V1978 == 1
#include "proc_pure_bitmap_prefix_v1978.h"

#ifdef DAIMOS_PBM_DIRTY_DROP_V1993
/* Shared synchronous/deferred cancellation identity. As with FULL,
 * stable process/u-area and agreeing backing records are the trust
 * boundary, not allocation generations or authentication. */
static struct proc *
pbd_process_v1993(unsigned int slot)
{
        struct proc *p;
        kword_t area;
        int i;

        if (!slot || slot == proc_current_slot || !proc_table ||
            !proc_swap_records || !proc_storage_runtime_v29.words ||
            slot >= proc_slots || slot >= proc_high_slot ||
            slot >= proc_storage_runtime_v29.slots)
                return 0;
        p = &proc_table[slot];
        area = PROC_UAREA_BASE(p);
        if (!PROC_TRANSITION(p) || !PROC_HAS_UAREA(p) || !area ||
            (PROC_STATE(p) != PROC_SLEEP && PROC_STATE(p) != PROC_STOP &&
             PROC_STATE(p) != PROC_SRUN) || mm_pure_table_v1922() != MM_OK)
                return 0;
        for (i = 0; i < mm_extent_count; ++i) {
                const struct mm_extent *e = &mm_extents[i];
                if (MM_EXTENT_BASE(e) == area)
                        return MM_EXTENT_TYPE(e) == MM_TYPE_KERNEL_DYNAMIC &&
                            MM_EXTENT_OWNER(e) == 01000U + slot &&
                            MM_EXTENT_WORDS(e) == PROC_UAREA_WORDS &&
                            !MM_EXTENT_PINS(e) ? p : 0;
        }
        return 0;
}

#endif

/* Validate the process extent AND the stable u-area before dereferencing
 * either. The common sole-user-owner check proves the entire MM table.
 * The u-area is a separate exact KERNEL_DYNAMIC owner (01000+slot).
 * All checks run under the caller's PI exclusion. */
static struct proc *
pbmp_owner_v1978(unsigned int slot, unsigned int pins)
{
#ifdef DAIMOS_PBM_DIRTY_DROP_V1993
        struct proc *p = pbd_process_v1993(slot);
        kword_t base, words;

        if (!p)
                return 0;
        base = VM_PDP6_BASE(p);
        words = VM_SPACE_WORDS(p);
        if (!base || ((base | words) & (VM_PDP6_ALIGN_WORDS - 1UL)) ||
            (PROC_STATE(p) != PROC_SLEEP && PROC_STATE(p) != PROC_STOP) ||
            proc_storage_expect_v30(slot, PROC_STORAGE_RESIDENT) != 0 ||
            !proc_pure_owner_v1918(slot, MM_TYPE_PROCESS, base, words, pins))
                return 0;
        return p;
#else
        struct proc *p;
        kword_t base, words, uarea;
        int i;

        /* This is an ownership proof, shared by ordinary RAW fallback.
         * The codec's real-time exclusion belongs to lease admission. */
        if (!slot || slot == proc_current_slot ||
            !proc_table || !proc_swap_records || slot >= proc_slots ||
            slot >= proc_high_slot || slot >= proc_storage_runtime_v29.slots)
                return 0;
        p = &proc_table[slot];
        base = VM_PDP6_BASE(p);
        words = VM_SPACE_WORDS(p);
        uarea = PROC_UAREA_BASE(p);
        if (!PROC_TRANSITION(p) || !PROC_HAS_UAREA(p) || !uarea ||
            !base || ((base | words) & (VM_PDP6_ALIGN_WORDS - 1UL)) ||
            (PROC_STATE(p) != PROC_SLEEP && PROC_STATE(p) != PROC_STOP) ||
            proc_storage_expect_v30(slot, PROC_STORAGE_RESIDENT) != 0 ||
            !proc_pure_owner_v1918(slot, MM_TYPE_PROCESS, base, words, pins))
                return 0;
        for (i = 0; i < mm_extent_count; ++i) {
                const struct mm_extent *e = &mm_extents[i];
                if (MM_EXTENT_BASE(e) == uarea)
                        return MM_EXTENT_TYPE(e) == MM_TYPE_KERNEL_DYNAMIC &&
                            MM_EXTENT_OWNER(e) == 01000U + slot &&
                            MM_EXTENT_WORDS(e) == PROC_UAREA_WORDS &&
                            !MM_EXTENT_PINS(e) ? p : 0;
        }
        return 0;
#endif
}
#ifdef DAIMOS_PBM_DIRTY_DROP_V1993
static int
pbd_same_v1993(unsigned int slot, const struct pure_bitmap_lease_v1978 *id,
    kword_t tag, unsigned int mode)
{
        struct proc *p = pbd_process_v1993(slot);
        kword_t *u;

        if (!p || p->vm_state != id->vm_state ||
            PROC_UAREA_BASE(p) != id->uarea ||
            proc_swap_records[slot].state != id->backing ||
            proc_storage_expect_v30(slot, mode))
                return 0;
        u = PRAM_PTR(id->uarea);
        return u[PROC_SWAP_BACKING_OFFSET] == id->backing &&
            (u[PROC_CRED_OFFSET] & PROC_SWAP_EXEC_MASK_V1976) == id->identity &&
            u[PROC_FINALIZE_OFFSET] == tag &&
            !(u[PROC_FDCTL_OFFSET] & (PROC_USER_MAP_BIT | PROC_FATAL_CLOSE_BIT));
}

extern int proc_pure_recover_arm_v1902(kword_t);

/* PI excluded throughout. Revoke retry authority before any mutation.
 * A rebase changes only the descriptor; the disjoint, untouched RAW
 * suffix remains authoritative. Never restore dirty text or acquire
 * another pin. Re-arm only on an exact unchanged ownership witness.
 */
static int
pbd_rollback_v1993(unsigned int slot, struct pure_bitmap_lease_v1978 *id,
    kword_t suffix, kword_t skip, unsigned int phase, unsigned int arm)
{
        struct proc *p = &proc_table[slot];
        kword_t *u = PRAM_PTR(id->uarea);
        kword_t words = VM_SPACE_WORDS(p), base = suffix - skip;
        kword_t hold = PBD_TAG_V1993(suffix, 0U, words, id->identity);
        unsigned int mode = phase == PBD_REBASE_V1993 ?
            PROC_STORAGE_RESIDENT : PROC_STORAGE_PACKED;

        if (!pbd_same_v1993(slot, id, hold, mode))
                return MM_ERR_BUSY;
        if (phase == PBD_REBASE_V1993) {
                if (VM_PDP6_BASE(p) != base ||
                    !proc_pure_owner_v1918(slot, MM_TYPE_PROCESS, base, words, 1U))
                        return MM_ERR_BUSY;
                (void)mm_pure_rebase_pinned_v1381(base, slot, skip, words);
                if (!pbd_same_v1993(slot, id, hold, mode))
                        return MM_ERR_BUSY;
                if (!proc_pure_owner_v1918(slot, MM_TYPE_PACKED_PROCESS,
                    suffix, words - skip, 1U)) {
                        if (proc_pure_owner_v1918(slot, MM_TYPE_PROCESS,
                            base, words, 1U))
                                goto retry;
                        return MM_ERR_BUSY;
                }
                VM_PDP6_SET_BASE(p, 0UL);
                pram_publish(slot, PROC_STORAGE_RESIDENT, PROC_STORAGE_PACKED);
                id->vm_state = p->vm_state;
                mode = PROC_STORAGE_PACKED;
                phase = PBD_UNPIN_V1993;
        }
        if (phase != PBD_UNPIN_V1993 || VM_PDP6_BASE(p) ||
            !pbd_same_v1993(slot, id, hold, mode) ||
            !proc_pure_owner_v1918(slot, MM_TYPE_PACKED_PROCESS,
                suffix, words - skip, 1U))
                return MM_ERR_BUSY;
        (void)mm_unpin(suffix);
        if (!pbd_same_v1993(slot, id, hold, mode))
                return MM_ERR_BUSY;
        if (!proc_pure_owner_v1918(slot, MM_TYPE_PACKED_PROCESS,
            suffix, words - skip, 0U)) {
                if (proc_pure_owner_v1918(slot, MM_TYPE_PACKED_PROCESS,
                    suffix, words - skip, 1U))
                        goto retry;
                return MM_ERR_BUSY;
        }
        u[PROC_FINALIZE_OFFSET] = 0UL;
        PROC_CLEAR_TRANSITION(p);
        return MM_OK;
retry:
        if (arm)
                (void)proc_pure_recover_arm_v1902(id->uarea);
        u[PROC_FINALIZE_OFFSET] = PBD_TAG_V1993(suffix, phase, words, id->identity);
        return MM_ERR_BUSY;
}

/* Called only by the original, still PI-excluded lease holder.
 * No deferred transaction may adopt a pin by inspecting its count.
 * Save the agreeing backing identity before the first rebase attempt. */
static int
pbd_start_v1993(unsigned int slot, const struct pure_bitmap_lease_v1978 *lease,
    unsigned int partial)
{
        struct pure_bitmap_lease_v1978 id = *lease;
        struct proc *p = pbmp_owner_v1978(slot, 1U);
        kword_t *u, skip, suffix, encoded;

        if (!p || p->vm_state != id.vm_state ||
            PROC_UAREA_BASE(p) != id.uarea ||
            proc_swap_records[slot].state != id.backing ||
            !id.backing || !(id.identity & PROC_SWAP_MUTABLE_V1976) ||
            VM_SPACE_WORDS(p) > MM_HALF_MASK ||
            pdp6_pure_skip_v1604(id.backing, VM_SPACE_WORDS(p), &skip) != MM_OK)
                return MM_ERR_BUSY;
        u = PRAM_PTR(id.uarea);
        suffix = VM_PDP6_BASE(p) + skip;
        encoded = u[PROC_FINALIZE_OFFSET] & MM_HALF_MASK;
        if (u[PROC_SWAP_BACKING_OFFSET] ||
            (u[PROC_CRED_OFFSET] & PROC_SWAP_EXEC_MASK_V1976) != id.identity ||
            (u[PROC_FDCTL_OFFSET] & (PROC_USER_MAP_BIT | PROC_FATAL_CLOSE_BIT)) ||
            u[PROC_FINALIZE_OFFSET] != (PROC_PURE_BITMAP_ESCROW_V1978(suffix) | encoded) ||
            (partial ? encoded != 0UL :
             !encoded || encoded > skip || encoded >= VM_SPACE_WORDS(p) - skip))
                return MM_ERR_BUSY;
        u[PROC_SWAP_BACKING_OFFSET] = id.backing;
        u[PROC_FINALIZE_OFFSET] = PBD_TAG_V1993(suffix, 0U,
            VM_SPACE_WORDS(p), id.identity);
        return pbd_rollback_v1993(slot, &id, suffix, skip, PBD_REBASE_V1993, 1U);
}

/* No loader, allocation, codec, provider access, or new pin on retry.
 * A wake may have made the held process SRUN; slot zero still owns
 * execution exclusion. Never route a recognized dirty HOLD to RAW. */
int
proc_pure_dirty_recover_v1993(unsigned int slot)
{
        struct pure_bitmap_lease_v1978 id;
        struct proc *p;
        kword_t saved = mach_pi_disable(), *u, tag, suffix, skip;
        unsigned int phase;
        int rc = MM_ERR_BUSY;

        if (proc_current_slot || !(p = pbd_process_v1993(slot)))
                goto out;
        id.uarea = PROC_UAREA_BASE(p);
        u = PRAM_PTR(id.uarea);
        tag = u[PROC_FINALIZE_OFFSET];
        if ((tag & (PROC_FINALIZE_MARK | PROC_PURE_KIND_MASK_V1919 |
            PROC_PURE_TERM_TAG_V1920 | PBD_MARK_V1993)) !=
            (PROC_PURE_KIND_MASK_V1919 | PBD_MARK_V1993)) {
                rc = PBD_NOT_MINE_V1993;
                goto out;
        }
        phase = (unsigned int)(tag & 3UL);
        suffix = PROC_PURE_FULL_SOURCE_V1917(tag);
        id.vm_state = p->vm_state;
        id.backing = proc_swap_records[slot].state;
        id.identity = u[PROC_CRED_OFFSET] & PROC_SWAP_EXEC_MASK_V1976;
        if ((phase != PBD_REBASE_V1993 && phase != PBD_UNPIN_V1993) ||
            tag != PBD_TAG_V1993(suffix, phase,
                ((tag >> 7U) & 0377UL) << 10U,
                ((tag >> 2U) & 037UL) << 27U))
                goto out;
        /* Stable u-area and exact tag: revoke authority while keeping
         * the ORIGINAL geometry and executable credential. A later
         * metadata repair cannot reactivate a failed witness. */
        u[PROC_FINALIZE_OFFSET] = tag & ~(PROC_SWAP_RETRY_TAG_V1507 | 3UL);
        if (tag != PBD_TAG_V1993(suffix, phase, VM_SPACE_WORDS(p), id.identity) ||
            !id.backing || !(id.identity & PROC_SWAP_MUTABLE_V1976) ||
            pdp6_pure_skip_v1604(id.backing, VM_SPACE_WORDS(p), &skip) != MM_OK ||
            suffix <= skip || ((suffix - skip) & (VM_PDP6_ALIGN_WORDS - 1UL)) ||
            (VM_SPACE_WORDS(p) & (VM_PDP6_ALIGN_WORDS - 1UL)) ||
            VM_SPACE_WORDS(p) - skip > MM_HALF_MASK + 1UL - suffix)
                goto out;
        rc = pbd_rollback_v1993(slot, &id, suffix, skip, phase, 0U);
out:
        mach_pi_restore(saved);
        return rc;
}
#endif

int
proc_pure_bitmap_prepare_v1978(unsigned int slot,
    struct pure_bitmap_lease_v1978 *lease)
{
        struct proc *p;
        kword_t backing, words, skip, base, suffix, encoded;
        kword_t *u;
        int rc;

        /* A live RT lease precludes the long PI-excluded codec passes,
         * not the existing zero-copy RAW reclaim transaction. */
        if (proc_rt_owner || !lease || !(p = pbmp_owner_v1978(slot, 0U)))
                return MM_ERR_INVAL;
        rc = pram_prepare(slot, PURE_LIFE_DROP_V1610, &p, &backing,
            &words, &skip);
        if (rc != MM_OK)
                return rc;
        base = VM_PDP6_BASE(p);
        u = PRAM_PTR(PROC_UAREA_BASE(p));
        if (u[PROC_FINALIZE_OFFSET] || u[PROC_SWAP_BACKING_OFFSET] ||
            !(u[PROC_CRED_OFFSET] & PROC_SWAP_MUTABLE_V1976) ||
            !pram_drop_available_v1926(backing, u))
                return MM_ERR_INVAL;
        suffix = base + skip;
        /* Until a write is possible, the original text is intact. The
         * old DROP kind is safe for clean pin uncertainty only. */
        u[PROC_FINALIZE_OFFSET] = PROC_PURE_INPLACE_ESCROW_V1919(suffix,
            PROC_PURE_DROP_TAG_V1919);
        rc = mm_pin(base);
        if (!proc_pure_owner_v1918(slot, MM_TYPE_PROCESS, base, words, 1U)) {
                if (rc != MM_OK && proc_pure_owner_v1918(slot,
                    MM_TYPE_PROCESS, base, words, 0U)) {
                        u[PROC_FINALIZE_OFFSET] = 0UL;
                        return rc;
                }
                return MM_ERR_BUSY;
        }
        if (rc != MM_OK)
                goto clean;
        /* Both regions are inside one exact pinned descriptor; the
         * capacity stops at the original mutable boundary. No separate
         * allocation, VFS scratch or released prefix is used. Mark dirty
         * BEFORE the codec may perform its first store. */
        u[PROC_FINALIZE_OFFSET] = PROC_PURE_BITMAP_ESCROW_V1978(suffix);
#ifdef DAIMOS_PBM_DIRTY_DROP_V1993
        /* Preserve the original identity even when the bounded write
         * pass fails before publishing an encoded count. All encoder
         * stores are in [base, suffix); RAW is disjoint. A general
         * memory fault outside that contract is not recoverable here. */
        lease->vm_state = p->vm_state;
        lease->backing = backing;
        lease->uarea = PROC_UAREA_BASE(p);
        lease->identity = u[PROC_CRED_OFFSET] & PROC_SWAP_EXEC_MASK_V1976;
#endif
        rc = pbmd_encode_v1977(suffix, words - skip, base, skip,
            PBMD_FORWARD_V1977, &encoded);
        if (rc == PBMD_WRITE_FAULT_V1977)
                return rc;             /* no valid candidate; quarantine */
        if (rc != MM_OK) {
                /* The codec contract guarantees ordinary refusal made
                 * no store. Only now may this be a clean DROP again. */
                u[PROC_FINALIZE_OFFSET] = PROC_PURE_INPLACE_ESCROW_V1919(suffix,
                    PROC_PURE_DROP_TAG_V1919);
                goto clean;
        }
        u[PROC_FINALIZE_OFFSET] |= encoded;
#ifndef DAIMOS_PBM_DIRTY_DROP_V1993
        lease->vm_state = p->vm_state;
        lease->backing = backing;
        lease->uarea = PROC_UAREA_BASE(p);
        lease->identity = u[PROC_CRED_OFFSET] & PROC_SWAP_EXEC_MASK_V1976;
#endif
        return MM_OK;
clean:
        /* Zero->one was witnessed inside this PI window, so this pin
         * is ours even if the successful pin reply was lost. */
        (void)mm_unpin(base);
        if (!proc_pure_owner_v1918(slot, MM_TYPE_PROCESS, base, words, 0U))
                return MM_ERR_BUSY;
        u[PROC_FINALIZE_OFFSET] = 0UL;
        return rc;
}

static struct proc *
pbmp_lease_v1979(unsigned int slot,
    const struct pure_bitmap_lease_v1978 *lease, kword_t *skipp)
{
        struct proc *p;
        kword_t backing, words, skip, base, suffix, escrow, encoded;
        kword_t *u;

        /* No new pin or discovered owner is accepted for cancellation.
         * The caller must still own the original synchronous lease. */
        if (proc_rt_owner || !lease || !(p = pbmp_owner_v1978(slot, 1U)) ||
            p->vm_state != lease->vm_state ||
            PROC_UAREA_BASE(p) != lease->uarea ||
            proc_swap_records[slot].state != lease->backing)
                return 0;
        backing = lease->backing;
        words = VM_SPACE_WORDS(p);
        base = VM_PDP6_BASE(p);
        u = PRAM_PTR(lease->uarea);
        if (u[PROC_SWAP_BACKING_OFFSET] ||
            (u[PROC_CRED_OFFSET] & PROC_SWAP_EXEC_MASK_V1976) != lease->identity ||
            pdp6_pure_skip_v1604(backing, words, &skip) != MM_OK)
                return 0;
        suffix = base + skip;
        escrow = u[PROC_FINALIZE_OFFSET];
        encoded = escrow & MM_HALF_MASK;
        if (!encoded || encoded > skip || encoded >= words - skip ||
            escrow != (PROC_PURE_BITMAP_ESCROW_V1978(suffix) | encoded))
                return 0;
        *skipp = skip;
        return p;
}

int
proc_pure_bitmap_cancel_v1978(unsigned int slot,
    const struct pure_bitmap_lease_v1978 *lease)
{
#ifdef DAIMOS_PBM_DIRTY_DROP_V1993
        if (!lease)
                return MM_ERR_BUSY;
        return pbd_start_v1993(slot, lease, 0U);
#else
        kword_t skip, base, words, suffix, backing, *u;
        struct proc *p = pbmp_lease_v1979(slot, lease, &skip);
        if (!p)
                return MM_ERR_BUSY;
        base = VM_PDP6_BASE(p);
        words = VM_SPACE_WORDS(p);
        suffix = base + skip;
        backing = lease->backing;
        u = PRAM_PTR(lease->uarea);
        /* Prefix is dirty. Even on reported failure, the only ordinary
         * cancellation is a proven RAW rebase; NEVER unpin the full
         * resident owner or clear TRANSITION after a refused rebase. */
        (void)mm_pure_rebase_pinned_v1381(base, slot, skip, words);
        if (!proc_pure_owner_v1918(slot, MM_TYPE_PACKED_PROCESS, suffix,
            words - skip, 1U))
                return MM_ERR_BUSY;
        u[PROC_SWAP_BACKING_OFFSET] = backing;
        VM_PDP6_SET_BASE(p, 0UL);
        pram_publish(slot, PROC_STORAGE_RESIDENT, PROC_STORAGE_PACKED);
        (void)mm_unpin(suffix);
        if (!proc_pure_owner_v1918(slot, MM_TYPE_PACKED_PROCESS, suffix,
            words - skip, 0U))
                return MM_ERR_BUSY;
        u[PROC_FINALIZE_OFFSET] = 0UL;
        PROC_CLEAR_TRANSITION(p);
        return MM_OK;
#endif
}
#ifdef DAIMOS_PBM_COMMIT_V1979
#include "mm_pure_bitmap_commit_v1979.h"
#include "proc_pure_bitmap_terminal_v1661.h"

/* The only new live-mode store is guarded by the original synchronous
 * lease and an exact candidate compare. Never call old mode3 restore
 * with this directory representation. */
int
proc_pure_bitmap_commit_v1979(unsigned int slot,
    const struct pure_bitmap_lease_v1978 *lease)
{
        kword_t skip, base, words, encoded, shift, *modep, *u;
        struct proc *p = pbmp_lease_v1979(slot, lease, &skip);
        int rc;
#ifdef DAIMOS_PBM_PLACE_V1985
        kword_t target, gap;
#endif
        if (!p)
                return MM_ERR_BUSY;
        base = VM_PDP6_BASE(p);
        words = VM_SPACE_WORDS(p);
        u = PRAM_PTR(lease->uarea);
        encoded = u[PROC_FINALIZE_OFFSET] & MM_HALF_MASK;
        if (pbmd_verify_v1979(base + skip, words - skip, base, encoded) != MM_OK)
                goto cancel;
        shift = (slot % PROC_STMAP_SLOTS_WORD_V28) * 2U;
        modep = &proc_storage_runtime_v29.words[slot / PROC_STMAP_SLOTS_WORD_V28];
        /* Original mode was proved by pbmp_lease. No callback/sleep,
         * no fallible operation between exact MM commit and publication.
         * A lost reply is reconciled against the exact selected base. */
#ifdef DAIMOS_PBM_PLACE_V1985
        /* Preserve the low placement policy. A higher gap is useful
         * only if the bitmap fits in its leading, non-APR-usable
         * fragment; do not consume a whole free APR to move upwards.
         * The original full APR stays owned throughout. */
        target = base;
        if (mm_find_fit(encoded, PDP6_PURE_SECTOR_V1604, MM_ALLOC_LOW, &gap) &&
            gap && (gap < base || ((gap & 01777UL) &&
            encoded <= 02000UL - (gap & 01777UL))))
                target = gap;
        if (target != base) {
                u[PROC_FINALIZE_OFFSET] = PROC_PURE_BITMAP_PLACE_V1985(target, encoded);
                rc = mm_pure_bitmap_place_v1985(base, slot, words,
                    (skip << 18U) | encoded, target);
        } else
#endif
        rc = mm_pure_bitmap_commit_v1979(base, slot, words,
            (skip << 18U) | encoded);
#ifdef DAIMOS_PBM_PLACE_V1985
        if (proc_pure_owner_v1918(slot, MM_TYPE_PACKED_PROCESS, target, encoded, 0U)) {
#else
        if (proc_pure_owner_v1918(slot, MM_TYPE_PACKED_PROCESS, base, encoded, 0U)) {
#endif
                u[PROC_SWAP_BACKING_OFFSET] = lease->backing;
                VM_PDP6_SET_BASE(p, 0UL);
                *modep |= (kword_t)PROC_STORAGE_BITMAP_V1661 << shift;
                u[PROC_FINALIZE_OFFSET] = 0UL;
                PROC_CLEAR_TRANSITION(p);
                return MM_OK;
        }
        /* Refused commit may cancel only while the original full pinned
         * owner is still exact. Success with no witness is uncertain. */
        if (rc == MM_OK)
                return MM_ERR_BUSY;
#ifdef DAIMOS_PBM_PLACE_V1985
        if (target != base) {
                /* Only exact original full ownership authorizes
                 * cancellation; never adopt a new source base. */
                if (!proc_pure_owner_v1918(slot, MM_TYPE_PROCESS, base, words, 1U))
                        return MM_ERR_BUSY;
                u[PROC_FINALIZE_OFFSET] = PROC_PURE_BITMAP_ESCROW_V1978(base + skip) | encoded;
        }
#endif
cancel:
        if (proc_pure_bitmap_cancel_v1978(slot, lease) == MM_OK)
                return PBMP_RAW_V1979;
        return MM_ERR_BUSY;
}

/* Staged reclaim entry. A clean refusal is distinguishable by the
 * original unpinned resident owner AND zero escrow, not rc alone.
 * Hold PI across preparation, comparison and publication. */
int
proc_pure_bitmap_drop_v1979(unsigned int slot)
{
        struct pure_bitmap_lease_v1978 lease;
        struct proc *p;
        kword_t saved = mach_pi_disable();
        int rc = proc_pure_bitmap_prepare_v1978(slot, &lease);
        if (rc == MM_OK) {
                rc = proc_pure_bitmap_commit_v1979(slot, &lease);
                if (rc == PBMP_RAW_V1979)
                        rc = MM_OK;
#ifdef DAIMOS_PBM_DIRTY_DROP_V1993
        } else if (rc == PBMD_WRITE_FAULT_V1977) {
                rc = pbd_start_v1993(slot, &lease, 1U);
#endif
        } else if (rc == PBM_NO_GAIN_V1975 || rc == MM_ERR_NOMEM ||
            rc == MM_ERR_INVAL) {
                p = pbmp_owner_v1978(slot, 0U);
                if (p && !PRAM_PTR(PROC_UAREA_BASE(p))[PROC_FINALIZE_OFFSET])
                        rc = proc_pure_ram_drop_v1381(slot);
        }
        mach_pi_restore(saved);
        return rc;
}
#endif

#endif
#endif

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
        u[PROC_CRED_OFFSET] &= ~PROC_SWAP_EXEC_MASK_V1976;
        /* Teardown owns TRANSITION until the complete slot is retired. */
        return MM_OK;
}
#endif /* !DAIMOS_PURE_DROP_ONLY_V1611 */
