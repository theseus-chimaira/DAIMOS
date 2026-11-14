# KCORE P0-B v1557: unambiguous mounted DTVOL2 physical-unit owner

Date: 2026-11-14. **Actual optional DTFS MRES feature, native tested.**
P0-B durable writable DTFS remains OPEN.

## Required invariant

The ordinary `dtfs_dtc_write` entry receives UNIT/BLOCK/BUFFER,
not the original published VFS vnode or mount identity. A physical
Type-551 unit can historically be mounted into more than one DTFS
slot, including old V1 volumes; this behavior does not prevent
ordinary legacy writes, but is incompatible with a future
per-mount V2 DIRTY-before-data barrier. If V2 and V1 share a
unit, the V1 mount could issue a physical data/directory write
without the V2 mount's exact durable identity or armed state.

v1557 adds a source-linked actual PDP-6 physical-unit ownership
check before any new **DTVOL2** mount is published, and the
matching reverse check before admitting a new V1 mount.
`dtfs_v2_mount_exclusive_v1557.s` is called from production
`dtfs_mount_unit` *after* native directory and reserved RBN0
physical validation and *before* `vfs_mount_prevalidated`.
It uses the existing serialized filesystem-provider lease
across the entire operation; no other provider request can
publish a competing mount during the check-to-publish window.

The native helper scans the actual VFS root and MM-backed
mounted-media slab. Only published DTFS roots with the same
physical DTC selector are relevant:

- A new verified V2/CLEAN read-only mount refuses **any**
  existing DTFS mount of the same physical unit, V1 or V2.
- A new V1 or all-zero legacy mount refuses an existing V2
  mount of the same physical unit, even if the new mount
  itself is writable.
- Existing V1/V1 multiple-mount behavior remains unchanged.
  Other units, unrelated providers and retired mount slots
  with stale media tags do not cause false conflicts.
- Missing VFS/MM table pointers or an invalid unit refuse
  before publication. There is no modified owner on refusal.

This rule makes a future physical-unit-to-mount association
unique whenever the mounted volume is V2. It does not rely on
probing userspace input or a cached directory; the on-media
admission itself was already checked by v1555. It does not
create any new public API or fixed KCORE data word, perform
any extra Type-551 I/O, or allocate MM memory. The helper
is assembled into the existing optional DTFS MRES source
by `system/kernel/Makefile`. `system/boot/pdp6/Makefile`
tracks its file for correct boot/MRES rebuilds.

## Acceptance

The actual helper passes **26 PDP-6 native cases and rejects
eight unsafe source mutations** in
`daimos-dtfs-v2-mount-exclusive-v1557.sh`. Physical-unit
conflicts are tested in both directions and at later VFS slots;
V1/V1 compatibility, foreign-provider exclusion, stale retired
slots and invalid/null tables are explicitly covered. The
oracle checks all published root/media words remain unchanged.

The expanded real-host-image `daimos-dtfs-v2-ro-admission-v1555.sh`
executes the modified production `dtfs_mount_unit` with genuine
`mkdtfs` DTVOL2 and DTVOL1 RBN0/0144 bytes. **24 native cases
PASS and 11 unsafe source mutations are rejected**, including
bypassing either direction of the new mounted-unit check.
Every rejected mount leaves the VFS publisher uncalled.

Historical v1519 direct native mount and v1520 72-bit identity,
legacy/MKFS suites PASS. The real full-profile **KCC DTFS MRES
assembly/fold/link PASS** with the newly concatenated production
helper. All source-linked tests remain exclusively in testkit.
Test specification:
`daimos-testkit/tests/validation/daimos-dtfs-v2-unit-ownership-v1557.md`.

## Remaining P0-B gates

The only enabled DTVOL2 mount remains verified CLEAN and
READ-ONLY. The existing production central write veneer continues
to reject ALL mounted V2 data/directory writes, including internal
callers; the first-write and subsequent-armed physical DIRTY
services of v1551-v1554 remain outside normal provider writing.

The next substantive features are provider-retained exact MM
scratch ownership, binding the unique V2 mount and 72-bit ID
to a mandatory physical WRITE_INTENT gate for every DATA/DIR
and chain/resize write, and exclusive final CLEAN/unmount
which excludes any newly opened owners throughout sleeping
physical seal/write/readback. Hardware media replacement races
between RBN0 verification and data transfer, full Type-551
guest partial-write, reinsertion, replacement and cold boot
must be verified separately. **No writable V2 cutover yet.**

P0-A complete; P0-B OPEN; P1-C separate. VERSION 0.5;
strict 32K/optimization HOLD.
