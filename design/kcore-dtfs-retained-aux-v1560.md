# KCORE P0-B v1560: retain exact DTFS intent MM owner across VFS lifetime

Date: 2026-11-14. Tested production VFS/KCORE lifecycle feature.
P0-B remains **OPEN**; DTVOL2 remains READ-ONLY.

## Source sweep and lifetime defect

The already-tested v1552 pinned-MM physical WRITE_INTENT wrapper
accepts an `AC4` pointer to a one-word scratch `basep`. If `mm_pin`,
`mm_unpin` or `mm_free` refuses, the exact 0200-word MM owner is
retained in `*basep`, and the caller must refuse further DATA/DIR
writes. Supplying a transient stack word would lose that pointer
when the provider returns, and retiring the VFS slot would discard
the last owner reference. Neither outcome is acceptable for P0-B.

KINIT already provisions one non-movable, MM-backed mount slab with
six arrays of one word per possible VFS mount: target, root, reader,
media, shape and extent. The `reader` array is a provider-private
auxiliary owner: D6FS uses its entries only on D6FS mounts and frees
its exact MM reader before retiring such a slot. Each published slot
has exactly one provider. DTFS can therefore reuse the *same* word
on its own mount slots for the retained v1552 intent-scratch `basep`.
This does **not** allocate a seventh array or repurpose the 72-bit
DTFS ID (shape/extent) or the media word's 18-bit UID/GID owner.

## Production changes

`system/kernel/fs/fs_mres.s` now exports the alias
`vfs_mount_aux_base_v1560` at exactly the same address as
`d6fs_reader_slots_base`. Its existing boot publication and
MM allocation remain unchanged. The KCC full-profile KCORE map
verifies that both global names have one identical address.
Comments in the KINIT mount slab sources now document the two
provider-disjoint uses of this existing word.

`system/kernel/fs/vfs.s` adds the native
`vfs_dtfs_aux_empty_v1560(slot)` guard (AC1=zero-based slot):
invalid slot, absent MM table or nonzero retained base refuses,
without changing the allocation. No automatic recovery, clearing,
freeing or reassigning an uncertain MM owner is attempted.

The actual VFS lifecycle uses that guard at three independent
safety boundaries:

1. Before a new DTFS mount publishes the selected free VFS slot.
   Even if a prior bug left a retained owner in an otherwise
   unoccupied slot, publication cannot overwrite or hide it.
2. Immediately after the final owner recheck of DTFS UNMOUNT,
   **after all sleeping SYNC/PREPARE work** and before FILE mount
   unlock, namespace replacement, or slot retirement. D6FS's
   pre-existing reader lifecycle and initial root reader are not
   incorrectly blocked merely because their pointer is nonzero.
3. Administrative poisoned-volume replacement/discard: first
   reject an outstanding scratch extent **before physical RBN0
   and directory replacement reads**; check again after the
   serialized provider operation and final FILE/cwd/PIPE/swap
   owner recheck, before unlocking or retiring the original slot.

The provider lease still serializes DTFS I/O, and the native
PDP-6 executive is non-preemptible in the short final check-to-
retirement window. No newly sleeping code was introduced after
the last owner/aux guard. This protects the exact MM pointer
once the physical service is connected; it does not claim the
future final CLEAN/READBACK operation itself is owner-quiescent.

## Tests and size

Testkit `daimos-dtfs-mount-aux-v1560.sh` links the actual native
PDP-6 guard: **16 scenarios PASS, five unsafe mutants rejected**.
`daimos-dtfs-aux-lifecycle-v1560.sh` executes the real VFS mount-
publication and final unmount instruction slices: **14 native
scenarios PASS, five unsafe mutants rejected**. They verify
unmodified MM pointers and VFS state on refusal, valid DTFS
retirement after release, and D6FS/TSFS isolation.

The source-ordering test `daimos-dtfs-aux-retire-order-v1560.py`
checks mount/unmount/administrative discard and shared pointer
alias, rejecting **five unsafe source mutations**. Legacy native
v1514 physical media-loss/return unmount and v1523 multi-process
FILE/cwd/PIPE/swap busy regression suites remain PASS, including
their existing source mutants. The real KCC full-profile KCORE
and optional DTFS MRES assemble/fold/link PASS; map confirms
identical auxiliary and D6FS reader-base addresses. No additional
fixed KCORE word, per-mount RAM allocation, syscall ABI, on-media
layout or physical I/O is introduced. KCORE code grows for the
feature, which is not an optimization.

See `daimos-testkit/tests/validation/
daimos-dtfs-retained-owner-v1560.md` for reproducible commands.

## Remaining P0-B and next stage

**No v1552 MM service is called by production DTFS yet.** v1560
creates the *provider-retained basep location and lifecycle
barriers*, not an actual retained scratch allocation. The
v1551-v1554 physical DIRTY arm/verify still is not linked to
the live `dtfs_dtc_write` first-write path. No V2 DATA/DIR
mutation, writable mount or physical CLEAN unmount is enabled.

The next step is to pass `&mount_aux[slot]` into the existing
v1552 owned-MM and v1553 first-write gate under the exact DTFS
provider lease, authenticate the mounted original 72-bit ID,
and deny every bypass of the common data/metadata write veneer.
After that, VFS must exclude new FILE/cwd/PIPE/swap owners across
sleeping final CLEAN marker write/readback and retire only after
verified on-media CLEAN. Full Type-551 guest partial physical
writes, detached/reinserted old volume, distinct replacement and
cold-boot rejection remain mandatory; the hardware media-change
interval between DIRTY readback and a DATA write is unresolved.

P0-A complete; P0-B OPEN; P1-C separate. VERSION 0.5,
strict 32K and optimization HOLD.
