# VFS mount capacity: boot sizing and 17-slot upper bound (2026-10-15)

Status: design/audit only; implementation and native acceptance pending.

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
