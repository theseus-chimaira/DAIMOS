/**
 * @file vm_pdp6_swap.c
 * @brief Resident PDP-6 process swap-out, swap-in, and reclaim policy.
 *
 * Swapping uses the raw backing pool selected by the storage bridge. A resident
 * process's one-word record describes executable backing; on swap-out that
 * word is saved in the stable u-area and the record is reused for first-block
 * and block-count. Swap-in runs from slot-0 executive context so synchronous
 * storage I/O never suspends on a process stack whose user extent is absent.
 */
#include "proc_swap.h"
#include "vm_pdp6.h"
#include "bstore.h"
#include "d6fs_provider.h"
#include "dtfs.h"
#include "tsfs.h"
#include "memfs.h"
#include "exec.h"
#include "fs_mres.h"
#include "mm.h"
#include "storage.h"
#include "syscall.h"
#include "proc_storage_map_v28.h"

/* Boot-time map allocation lives next to the existing per-slot swap records. */
struct proc_storage_map_v28 proc_storage_runtime_v29;


struct proc_swap_record *proc_swap_records;
kword_t proc_swap_blocks_used;

/* The assembly swap transactions own mode publication under TRANSITION. */
extern int proc_storage_publish_v30(unsigned int slot,
    unsigned int old_state, unsigned int new_state);
extern void proc_swap_storage_invariant_v30(void);

/* Swap slot numbers and packed block spans are bounded positive quantities.
 * Use signed working values after unpacking so their comparisons stay compact
 * on the PDP-10; packed on-disk/in-memory fields remain unchanged. */

#ifndef PROC_SWAP_TRANSACTIONS_PDP6_ASM
/** Transfer a block-aligned process extent to or from BACKSTORE. */
static int
proc_swap_transfer_words(int write, kword_t first,
    kword_t *buf, kword_t words)
{
        kword_t blocks;

        if (words == 0UL ||
            (words % DSK_WORDS_PER_SECTOR) != 0UL)
                return -1;
        blocks = words / DSK_WORDS_PER_SECTOR;
        if (write)
                return backstore_write(first, blocks, buf);
        return backstore_read(first, blocks, buf);
}
#endif

/** Release only a nonresident disk span, then clear executable backing. */
void
proc_swap_detach(int slot)
{
        struct proc *p;
        kword_t state;
        int was_disk;

        p = &proc_table[slot];
        state = proc_swap_records[slot].state;
        was_disk = VM_PDP6_BASE(p) == 0UL && state != 0UL;
        if (was_disk) {
                backstore_free((state >> 18U) & MM_HALF_MASK,
                    state & MM_HALF_MASK);
                proc_swap_blocks_used -= state & MM_HALF_MASK;
                /* The discarded slot must not retain the DISK mode. */
                if (proc_storage_publish_v30((unsigned int)slot,
                    PROC_STORAGE_DISK, PROC_STORAGE_RESIDENT) != 0)
                        proc_swap_storage_invariant_v30();
        }
        proc_swap_records[slot].state = 0UL;
        if (PROC_HAS_UAREA(p)) {
                PROC_SWAP_BACKING_WORD(p) = 0UL;
                PROC_CRED_WORD(p) &= ~PROC_SWAP_MOUNT_EXT_BIT;
        } else if (PROC_STATE(p) == PROC_SIDL)
                p->sched &= ~PROC_SWAP_STAGE_MOUNT_EXT_BIT;
}

/** Pin, write, free, and atomically publish one process as nonresident. */
#ifndef PROC_SWAP_TRANSACTIONS_PDP6_ASM
int
proc_swap_out(int slot)
{
        struct proc_swap_record *r;
        struct proc *p;
        kword_t base;
        kword_t words;
        kword_t blocks;
        kword_t first;
        kword_t resident_state;
        kword_t *mem;
        unsigned int state;

        p = &proc_table[slot];
        state = PROC_STATE(p);
        if ((state != PROC_SRUN && state != PROC_SLEEP && state != PROC_STOP) ||
            PROC_TRANSITION(p) || !PROC_HAS_UAREA(p) ||
            PROC_USER_MAPPING_HELD(p))
                return -1;
        r = &proc_swap_records[slot];
        resident_state = r->state;
        if (resident_state == 0UL)
                return -1;
        base = VM_PDP6_BASE(p);
        words = VM_SPACE_WORDS(p);
        if (base == 0UL || words == 0UL || mm_is_pinned(base) ||
            (words % DSK_WORDS_PER_SECTOR) != 0UL)
                return -1;

        PROC_SET_TRANSITION(p);
        if (mm_pin(base) != MM_OK) {
                PROC_CLEAR_TRANSITION(p);
                return -1;
        }
        mem = (kword_t *)(unsigned long)base;
        blocks = words / DSK_WORDS_PER_SECTOR;
        first = backstore_blocks;
        if (blocks == 0UL || backstore_alloc(blocks, 0UL, &first) != 0)
                goto fail_unpin;
        if (proc_swap_transfer_words(1, first, mem, words) != 0)
                goto fail_unpin;

        /* Keep the resident executable-backing record in the already
         * resident u-area while the one-word swap record is reused for the
         * disk span.  No disk header or large executive-stack bounce buffer
         * is required. */
        PROC_SWAP_BACKING_WORD(p) = resident_state;
        if (mm_unpin(base) != MM_OK)
                goto fail_record;
        if (mm_free(base, MM_TYPE_PROCESS, slot) != MM_OK)
                goto fail_record;
        VM_PDP6_SET_BASE(p, 0UL);
        r->state = ((first & MM_HALF_MASK) << 18U) |
            (blocks & MM_HALF_MASK);
        proc_swap_blocks_used += blocks;
        if (proc_storage_publish_v30((unsigned int)slot,
            PROC_STORAGE_RESIDENT, PROC_STORAGE_DISK) != 0)
                proc_swap_storage_invariant_v30();
        PROC_CLEAR_TRANSITION(p);
        return 0;

fail_unpin:
        (void)mm_unpin(base);
fail_record:
        if (first < backstore_blocks)
                backstore_free(first, blocks);
        PROC_SWAP_BACKING_WORD(p) = 0UL;
        PROC_CLEAR_TRANSITION(p);
        return -1;
}
#endif

/* Run one swap-in transaction from slot-0 executive context.  Selection is
 * deliberately derived from the existing scheduler fields, so no permanent
 * request queue or per-process swap scheduling state is needed. */
/** Service the scheduler's single pending slot-0 swap-in request. */
#ifndef PROC_SWAP_SERVICE_PDP6_ASM
int
proc_swap_service_one(void)
{
        struct proc *p;
        int slot;

        if ((proc_sched_cursor & PROC_SCHED_SWAP_REQUEST) == 0UL)
                return 0;
        slot = (int)(proc_sched_cursor & PROC_PGRP_MASK);
        if (slot <= 0 || slot >= (int)proc_high_slot) {
                proc_sched_cursor = (kword_t)slot;
                return 0;
        }
        p = &proc_table[slot];
        if (PROC_STATE(p) != PROC_SRUN || PROC_IS_FREE(p) ||
            VM_PDP6_BASE(p) != 0UL || proc_swap_records[slot].state == 0UL ||
            PROC_TRANSITION(p)) {
                proc_sched_cursor = (kword_t)slot;
                return 0;
        }

        /* Keep PROC_SCHED_SWAP_REQUEST set for the complete transaction.
         * PI6 treats that bit as slot-0 service busy and must not switch away
         * from the permanent idle stack while synchronous swap I/O is active. */
        if (proc_swap_in(slot) == 0) {
                proc_sched_cursor = (kword_t)slot;
                return slot;
        }
        proc_sched_cursor = (kword_t)slot;

        /* proc_swap_in already performs normal MM reclaim.  A failure after
         * that point cannot be left as an SRUN process that wins scheduling
         * forever; terminate it with the normal fatal-event cleanup path. */
        (void)proc_event_apply((unsigned int)slot, SYS_EVENT_TERM);
        return -1;
}
#endif

/** Allocate, restore, and republish one swapped process image. */
#ifndef PROC_SWAP_TRANSACTIONS_PDP6_ASM
int
proc_swap_in(int slot)
{
        struct proc_swap_record *r;
        struct proc *p;
        kword_t base;
        kword_t words;
        kword_t resident_state;
        kword_t first;
        kword_t blocks;
        kword_t *mem;

        p = &proc_table[slot];
        r = &proc_swap_records[slot];
        if (VM_PDP6_BASE(p) != 0UL || PROC_IS_FREE(p) ||
            r->state == 0UL || PROC_TRANSITION(p) || !PROC_HAS_UAREA(p))
                return -1;
        words = VM_SPACE_WORDS(p);
        resident_state = PROC_SWAP_BACKING_WORD(p);
        if (words == 0UL || resident_state == 0UL ||
            (words % DSK_WORDS_PER_SECTOR) != 0UL)
                return -1;

        first = (r->state >> 18U) & MM_HALF_MASK;
        blocks = r->state & MM_HALF_MASK;
        if (blocks == 0UL || blocks != words / DSK_WORDS_PER_SECTOR)
                return -1;

        PROC_SET_TRANSITION(p);
        {
                int rc;

                rc = mm_alloc_aligned(words, VM_PDP6_ALIGN_WORDS,
                    MM_TYPE_PROCESS, slot, MM_ALLOC_HIGH, &base);
                if (rc != MM_OK) {
                        PROC_CLEAR_TRANSITION(p);
                        return rc >= MM_ERR_DESCRIPTORS &&
                            rc <= MM_ERR_NOMEM ? PROC_SWAP_IN_PRESSURE :
                            PROC_SWAP_IN_FATAL;
                }
        }
        if (mm_pin(base) != MM_OK) {
                (void)mm_free(base, MM_TYPE_PROCESS, slot);
                PROC_CLEAR_TRANSITION(p);
                return -1;
        }
        mem = (kword_t *)(unsigned long)base;
        if (proc_swap_transfer_words(0, first, mem, words) != 0)
                goto fail;

        if (mm_unpin(base) != MM_OK)
                goto fail_free;
        VM_PDP6_SET_BASE(p, base);
        proc_swap_blocks_used -= blocks;
        backstore_free(first, blocks);
        r->state = resident_state;
        PROC_SWAP_BACKING_WORD(p) = 0UL;
        if (proc_storage_publish_v30((unsigned int)slot,
            PROC_STORAGE_DISK, PROC_STORAGE_RESIDENT) != 0)
                proc_swap_storage_invariant_v30();
        PROC_CLEAR_TRANSITION(p);
        return 0;

fail:
        (void)mm_unpin(base);
fail_free:
        (void)mm_free(base, MM_TYPE_PROCESS, slot);
        PROC_CLEAR_TRANSITION(p);
        return -1;
}
#endif

/** Swap victims until MM compaction can satisfy the requested free extent. */
#ifndef PROC_SWAP_RECLAIM_PDP6_ASM
int
proc_swap_reclaim(kword_t words, kword_t alignment,
    unsigned int exclude_owner)
{
        int victim;

        for (;;) {
                victim = proc_swap_victim(exclude_owner);
                if (victim < 0)
                        break;
                if (proc_swap_out(victim) != 0)
                        break;
                if (mm_compact(words, alignment) == MM_OK)
                        return 0;
        }
        return -1;
}
#endif

/* Shared PURE backing construction, compiled in one KCC translation unit. */
#include "proc_pure_attach.c"
