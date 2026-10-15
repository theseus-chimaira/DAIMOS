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

Allow 17 slots if the complete post-fold resident plus dynamic cost of slot 17
is modest relative to 16. Preserve headroom for removable media, rather than
limiting capacity to currently mounted volumes. Do not equate installed
device services with harmless probing of unattached physical units.

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
   membership; provision capacity for potentially mountable physical units,
   reducing the count for validated known blocksets only. Separate this work
   from fixing non-root DRM BLOCKSET discovery; do not perform unsafe I/O.
5. Test four-slot baseline, 16 and 17 slots, root-only and mixed filesystems,
   exhaustion, mount-ID reuse, RO/remount and SWAP/LOGSTORE pinning; place
   tests in daimos-testkit. Compare full post-p10fold KCORE and permanent plus
   dynamic RAM; boot and test on PDP-6 SIMH before activation.

Block cache is explicitly out of scope. MM dynamic descriptor sizing and
MEMFS data tuning are separate tasks.
