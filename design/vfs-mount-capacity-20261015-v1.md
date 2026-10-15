# VFS mount capacity: boot sizing and 17-slot upper bound (2026-10-15)

Status: design/audit only; implementation and native acceptance pending.

## Staged KINIT capacity calculator (v1)

`system/kernel/boot/vfs_mount_capacity_v1.c` now contains an unlinked,
side-effect-free calculator. The inputs are the controller-supported physical
unit counts (disk 0..4, drum 0..4, DECtape 0..8), the validated member mask
of exactly one selected root set, and the MEMFS availability flag. The result
counts that root once and all remaining supported physical units individually,
plus optional MEMFS. It rejects invalid masks and inconsistent root selection.
This is not yet connected to live KINIT, and is not a claim of native boot
acceptance. Its standalone tests live in daimos-testkit at
`tests/system/vfs-mount-capacity-v1`.

## Staged per-mount policy (v2)

`system/kernel/fs/vfs_mount_policy_v2.c/.h` implements a separate, unlinked
policy object with one 36-bit bitmap each for readonly, SWAP reservation and
LOGSTORE reservation. The selected capacity is validated in 1..17; all
accesses validate slot bounds. Reservations are exclusive across mounts;
release checks ownership before mutation, remount can toggle readonly and
unmount can clear all three bits for slot reuse. The policy uses three maps
rather than two densely packed words to avoid expensive cross-word bitfield
operations in the PDP-6 runtime. Host functional tests in daimos-testkit
`tests/system/vfs-mount-policy-v2` pass; PDP-6 GCC standalone source generation
passes. This does NOT replace the active `vfs_mount_ro` assembly alias or
establish native boot acceptance. Full assembler consumer migration remains
necessary before v2 can be linked and activated.

## Staged MM slab layout (v3)

`system/kernel/fs/vfs_mount_layout_v3.c/.h` now plans a single contiguous
MM-owned region sized to the boot-selected mount capacity. Six one-word-per-
mount arrays are planned: VFS target, VFS root, D6FS reader, shared DTFS/TSFS
media, TSFS file shape and TSFS extent locator. The exact offsets are derived
once from capacity; 4 mounts require 24 words, 17 mounts require 102 words.
No resident array has been converted to use the slab yet. This is a staged
planner, not an active allocation. It deliberately does not force separate
arrays for all providers and does not include the three-word policy bitmap.
The layout passes standalone host tests for capacities 1..17, and its C source
generates PDP-6 GCC assembly. Native KCORE sizes are not yet measured.

## Capacity policy

MONITORFS built-in entries consume no VFS mount slot. The root D6FS or TSFS
consumes one. MEMFS consumes one when mounted (singleton today). Independently
mountable D6FS, DTFS, and other supported volumes consume one each. Non-root
TSFS sets are not supported. A validated disk/drum BLOCKSET consumes one slot
regardless of member count. Until non-root disk/drum discovery is corrected,
count every unit outside known root BLOCKSET membership as potentially
mountable: do not infer a drum belongs to a set or is dedicated to swap.

Support up to 17 slots (the six-bit vnode mount-ID format has sufficient
headroom), but allocate only the boot-selected hardware-derived capacity.
Do not round capacity to a power of two. Three densely packed policy bits
per mount require two 36-bit words for either 16 or 17 mounts; measure the
additional provider table entries and code separately. Preserve mount slots
for removable media, not just currently mounted volumes. Do not equate
installed controller services with safely detected individual drives.

At boot, count each known root BLOCKSET once, each other disk/drum unit
that might be independently mountable once, available DECtape drive capacity
as potential DTFS mounts (except the units of the selected TSFS root), and
one optional MEMFS slot if enabled. Reduce this only for validated complete
set membership or verified reservations. Use conservative controller maximum
unit counts where individual physical drive presence cannot safely be probed.
The selected count must remain a bound on *future* mounts, not only current
media occupancy. Keep all discovery scratch transient in KINIT.

Current probe limitations: `storage/auxstore_boot.c` explicitly warns that
probing an unattached Type-270 trailing unit can render its controller
unusable; `modules/module_minit.c` identifies the disk/drum controllers, not
each individual unit. DRM AUXSTORE checks DRM0; TSFS boot scans DECtape
media membership but current tape presence is not lifetime mount capacity.

## Source constraints verified

- `fs/vfs.h`: VFS_NMOUNT=4, but mount IDs have six bits (1..63).
- `fs/vfs.s`: four entries in `vfs_mount_target` and `vfs_mount_root`;
  bounded loops in lookup, parent/name, mount, remount, readonly, unmount.
- `drivers/device_state.s`: `vfs_mount_ro` is a one-word alias of an unused
  MonitorFS input counter slot. It contains four per-slot groups: readonly
  bit 0, SWAP bit 4, LOGSTORE bit 8 (shifted by mount index).
- `fs/vfs.s`: mount flags, storage reservation tests, and unmount clearing
  rely on the one-word packing. A simple 4->17 limit change is invalid.
- `fs/d6fs_runtime.s`: remount flips `vfs_mount_ro` with XOR.
- `proc/syscall_dispatch_pdp6.s`: FSINFO accepts only slots 0..3 and reads
  the readonly bit directly from `vfs_mount_ro`.
- `fs/fs_mres.s`: four `d6fs_reader_slots` pointers, indexed by mount slot.
- `fs/dtfs.c`: `dtfs_media[VFS_NMOUNT]` per-mount state.
- `fs/tsfs_runtime.s`: two four-word mount metadata arrays.
- `proc/proc_pure_attach.c`: compares mount ID with `VFS_NMOUNT`.

## Additional runtime audit (2026-10-15)

The current fixed symbols are direct operands in native assembly, not merely
arrays referenced via pointers. Simply replacing `.block 4` with an MM-backed
pointer would silently break `vfs_mount_root(index)` and related addressing.
The migration needs an explicit base register or a bounded helper, with exact
before/after instruction accounting. In particular, `tsfs_file_record` takes
the address of `dtfs_media` as an immediate, whereas other providers directly
index that symbol. A runtime-sized slab should group mount-target, mount-root,
reader-pointer and per-provider metadata only after all access conventions are
converted together; ensure unmount reuse clears every per-mount field.

One 36-bit policy word holds 36 independently indexed RO bits; storing three
independent 17-bit RO/SWAP/LOGSTORE maps needs three words. A fully packed
three-bit-per-mount encoding needs two words for 17 mounts, but may cost more
instructions in the hot path. Measure both implementations, rather than
assuming the minimum policy bits minimize total resident RAM.

The current `native_sys_fsinfo` hard-codes the maximum zero-based slot 3.
The D6FS remount path directly XORs the packed RO policy word. Both need
replacement before any mount count above four is safe. The existing
`vfs_mount_ro` alias within `mfsdev_io_in` must not be enlarged in place:
the following device counters occupy adjacent words.

Controller detection alone cannot establish disk/drum per-unit presence:
the Type-270 source warns that a read on an unattached trailing unit can
render the controller unusable. The interim capacity calculation must use
controller-supported unit counts, subtracting only *validated known* root
BLOCKSET member overlap. TSFS root members reduce separately mountable
DECtape capacity; later removable DTFS volumes need reserved slots.

## Implementation sequence

1. Define and test a per-slot mount-policy representation that can encode
   RO, SWAP and LOGSTORE independently for at least 17 mount slots. Avoid
   introducing branches into ordinary filesystem I/O; compare compact
   representation with a per-mount word. Preserve storage ownership rules,
   atomic publication, and cleanup on mount/remount/unmount/failure.
2. Convert all direct readers and writers of `vfs_mount_ro`, including
   FSINFO and D6FS remount. Retain the MonitorFS counter-word reuse only if
   still correct; account for any extra BSS in the KCORE total.
3. Convert all four-slot VFS loops and provider per-mount arrays together.
   Reject out-of-range mount IDs before indexing them. Ensure root handling,
   parent, lookup, readonly policy and mount teardown retain semantics.
4. Add boot selection: KINIT knows core size, root class and selected root
   membership; compute an actual mount-slot capacity from the installed
   hardware's safe-to-establish limits, conservatively counting unconfirmed
   disk/drum units and preserving future DTFS mountability. Reduce the count
   for validated known blocksets only. Separate this from correcting non-root
   DRM BLOCKSET discovery; do not perform unsafe I/O.
5. Test four-slot baseline, 16 and 17 slots, root-only and mixed filesystems,
   exhaustion, mount-ID reuse, RO/remount and SWAP/LOGSTORE pinning; place
   tests in daimos-testkit. Compare full post-p10fold KCORE and permanent plus
   dynamic RAM; boot and test on PDP-6 SIMH before activation.

Block cache is explicitly out of scope. MM dynamic descriptor sizing and
MEMFS data tuning are separate tasks.
