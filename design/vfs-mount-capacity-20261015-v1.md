# VFS mount capacity: boot sizing and 16-slot upper bound (2026-10-15)

## Active D6FS reader migration and 32K measurement (v13)

KINIT transfers the initial MINIT reader pointer from the one-word fallback
`d6fs_reader_slots` to the slab reader region before publishing
`d6fs_reader_slots_base`. D6FS dispatch, additional mount and unmount now
use this base. Native PDP-6 context, multi-mount, remount, and poisoned
fallback tests pass. The diagnostic disk-root SIMH ROOT login and ECHO also
pass. This does not raise `vfs_mount_limit` above four: DTFS/TSFS metadata
and mount-policy conversion are pending.

Normal permanent-size gates pass: disk single last 036723 (556 free), disk
multi last 036764 (523 free), tape last 036071 (966 free), octal addresses.
GCC 32K lowmem disk/tape KINIT images build without shell. The KCORE ends
at 023540, **failing** its separate 020000 strict gate by 1888 decimal
words in both. KCORE+MRES is 12837/12705 decimal words (disk/tape); KINIT
ends at 072422/073327, with 1774/1321 words remaining after the 02000-octal
boot stack. This is not successful 32K native boot. The measurements are
reproducible using the daimos-testkit v13 footprint script.


Status (v12): **partially integrated**. VFS target/root tables are allocated
from MM by KINIT and used by active native VFS, FSINFO, and PURE backing;
provider metadata and mount policy still require conversion. The configured
mount limit remains four, with an eventual maximum of sixteen.

## Production MM-backed VFS target/root tables (v12)

After `module_run_minits()` has finished installing the permanent MRES
packages, but before `kinit_boot()` mounts the root, KINIT calls
`vfs_mount_slab_v4_prepare(VFS_NMOUNT, ...)`, zeroes the MM-owned allocation,
and publishes the `target` and `root` word addresses into the resident
`vfs_mount_target_base`/`vfs_mount_root_base` pointers. The tables occupy the
first two of six contiguous per-mount slab arrays. Runtime VFS lookup,
parent-boundary lookup, mount-slot discovery/publication/retirement, readonly,
unmount, FSINFO and the PURE swap backing unpacker use these pointer bases.
Other provider tables still reside in fixed four-entry arrays. KINIT does not
increase `vfs_mount_limit` above four. Original four-entry VFS BSS tables are
kept as bootstrap fallback until the whole mount conversion is complete.

With four configured mounts the MM slab is 24 words, of which eight are now
used for live target/root tables; the other sixteen remain reserved for the
pending provider migration. The current migration increases folded permanent
KCORE+MRES by 35 decimal words versus v11 because pointer addressing is more
expensive than absolute indexed assembly accesses. The dedicated disk-root
single permanent last is 036721 (558 decimal free), multi 036762 (525 free),
and tape-root last 036073 (964 free). This is a temporary RAM regression, not
a demonstrated net saving. After the whole cutover, remove the redundant
fixed arrays and review boot-patched addressing if hot-path overhead remains.

The first boot-image link also exposed two build-graph issues: the new KINIT
objects were missing from the explicit image link/object lists, and the MINIT
table declared `dtfs_minit` unconditionally even when tape boot was disabled.
Both are corrected. Native PDP-6 execution tests pass for VFS reservation,
slot reuse, PURE swap, and the relocated-target/root-table case in
`daimos-testkit/tests/validation/daimos-vfs-slab-live-v12.sh`.
The disk-root diagnostic SIMH boot reached ROOT login and ran an ECHO command
with the documented, test-only `DSH_IMAGE_LIMIT=037000` override. The normal
production DSH gate still rejects the 036021 image against 036000, so no
production image-size acceptance is claimed.

## Production runtime mount-count bound (v11)

Production `fs/vfs.s` exports `vfs_mount_limit`, initialized to four in its
resident DATA section. All active VFS slot bounds (lookup, parent, readonly,
storage release, mount insertion, shutdown and unmount) compare against that
word rather than hard-coding 4. `native_sys_fsinfo` uses the same bound;
`proc_swap_attach` uses it when validating an executable backing mount.
The fixed target/root/provider arrays remain four entries, so no code sets
`vfs_mount_limit` above four yet. It must be updated only after KINIT has
allocated and published all boot-sized tables and provider state.

Dedicated-root linked gates pass: disk single last 036656 (593 decimal free),
disk multi last 036717 (560 free), tape last 036030 (999 free). The extra
shared limit and its consumers cost three linked permanent words relative
to v10. Native PDP-6 VFS reservation/mutation and PURE swap tests pass.
`daimos-testkit/tests/validation/daimos-vfs-runtime-limit-v11.sh` also sets
the bound to two while the four-slot arrays remain allocated, verifies that
a third mount and slot-three storage reservation are rejected, and checks
that unmount/reuse still succeeds. Full native OS boot and >4 live mounts
remain pending.

## Native one-word PURE backing for 16 mounts (v10)

The production swap backing format contained a previously missed two-bit
mount-slot field (`PROC_SWAP_MOUNT_MASK=03`). Merely widening VFS tables
would cause swapped PURE executables from mounts 5..16 to be reconstructed
from the wrong volume. The corrected active record retains exactly one
36-bit word per process: 18 bits of vnode index, 4 bits of zero-based mount
slot, and 14 bits of PURE-text length. Two former provider bits now belong
to the mount field. A mount has only one provider, and `proc_swap_mount_busy`
pins its presence while nonresident PURE text depends on it. The native
`proc_swap_unpack_backing` therefore reads provider identity from that
slot's mounted root, returning an unusable/empty backing if it is absent.
`proc_swap_backing_busy` compares mount/index within that uniquely owned
slot; its per-process format needs no additional RAM. Mount attach remains
limited to the active four-slot VFS until the common runtime migration.

Native PDP-6 full PURE swap tests pass after updating their fixture to link
the production PURE backing/storage-state assembly and initialize the
storage-mode map. A separate versioned PDP-6 test validates 16th-slot
provider reconstruction, maximum 14-bit text length and missing-root
rejection (`daimos-testkit/tests/target/daimos-swap-mount16-pdp6-v10`).
The linked permanent-size gates pass for both dedicated profiles. Relative
to v9, permanent last addresses decrease by 4 decimal words: disk single
036653 (596 decimal free), disk multi 036714 (563 free), tape 036025
(1002 free). The active VFS remains limited to four mounts.

## Linked size baseline (2026-10-15)

Following the KCC-safe staged packed-function symbol fix, the complete
`permanent-size` link succeeds but the combined auto-root profile exceeds
the 040000 permanent-address gate: KCORE 023633, KCORE+MRES 042223,
permanent last 042302. With `ROOT=disk`, the single-root last address is
036643 (604 decimal words free) and the multi-member last address is
036704 (571 free). With `ROOT=tape`, the last address is 036015
(1010 decimal words free). Both dedicated profiles pass. These are linked
profile measurements, not native PDP-6 boot acceptance. Future dynamic VFS
integration should be measured against these distinct build configurations;
do not mistake the combined auto-root overrun for a universal kernel failure.

## Staged KINIT capacity calculator (v1)

`system/kernel/boot/vfs_mount_capacity_v1.c` now contains an unlinked,
side-effect-free calculator. The inputs are the controller-supported physical
unit counts (disk 0..4, drum 0..4, DECtape 0..8), the validated member mask
of exactly one selected root set, and the MEMFS availability flag. The result
counts that root once and all remaining supported physical units individually,
plus optional MEMFS, capped at 16 simultaneous mounts. It rejects invalid masks and inconsistent root selection.
This is not yet connected to live KINIT, and is not a claim of native boot
acceptance. Its standalone tests live in daimos-testkit at
`tests/system/vfs-mount-capacity-v1`.

## Staged per-mount policy (v2)

`system/kernel/fs/vfs_mount_policy_v2.c/.h` implements a separate, unlinked
policy object with one 36-bit bitmap each for readonly, SWAP reservation and
LOGSTORE reservation. The selected capacity is validated in 1..16; all
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
once from capacity; 4 mounts require 24 words, 16 mounts require 96 words.
No resident array has been converted to use the slab yet. This is a staged
planner, not an active allocation. It deliberately does not force separate
arrays for all providers and does not include the three-word policy bitmap.
The layout passes standalone host tests for capacities 1..16, and its C source
generates PDP-6 GCC assembly. Native KCORE sizes are not yet measured.

## Staged KINIT slab provisioning (v4)

`system/kernel/boot/vfs_mount_slab_v4.c/.h` uses the v3 layout to acquire a
single owner-tagged `MM_TYPE_KERNEL_DYNAMIC` extent, zero exactly its selected
word count, and publish six absolute bases. Failed validation and MM allocation
leave its output untouched. This is deliberately unlinked: permanent native
VFS and provider array accesses must be converted together before KINIT can
publish these addresses. Standalone host tests (including MM failure and
canary checking) live in `daimos-testkit/tests/system/vfs-mount-slab-v4`;
PDP-6 GCC source assembly generation passes, but no linked size or boot test
has been performed. The owner tag 020 octal is reserved for this staged slab.

## Staged native mount lookup (v5)

## Staged parent-boundary conversion (v6)

`system/kernel/fs/vfs_parent_mount_v6.s` extracts the mount-root-to-target
resolution shared by the existing `vfs_parent` and `vfs_parent_name` paths.
It decodes the existing six-bit vnode mount ID, enforces the selected 1..16
capacity, verifies that the vnode equals the currently published root in
that slot, and then substitutes the corresponding target vnode. It leaves
all other vnodes unchanged. Both table addresses are explicit arguments;
there are no four-slot fixed-symbol references. Standalone DAS assembly
passes; native execution and integration into live parent paths are pending.
This is intentionally separate from the unrelated filesystem parent dispatch.

## Production parent-boundary consolidation (v7)

The active `fs/vfs.s` now shares one `vfs_parent_resolve_slot` leaf between
`vfs_parent` and `vfs_parent_name`, instead of duplicating fixed-array loads
and comparisons. This limits the eventual dynamic-address cutover to one
internal parent-resolution site. It preserves the existing four-mount bound
and direct `vfs_mount_root`/`vfs_mount_target` addressing; it is NOT the
16-mount activation. The modified production source assembles successfully
as a relocatable kernel object with `das -C`; linked KCORE and native boot
regressions remain pending.

## Production lookup isolation (v8)

The active `fs/vfs.s` now calls `vfs_lookup_resolve_mount` for mounted-target
substitution. The same four-entry loop is retained in a dedicated leaf; the
future slab-base substitution will not have to disturb provider dispatch or
the VFS return/pop path. Relocatable DAS assembly passes, as do complete
linked `permanent-size` checks for dedicated disk and tape root profiles.
Both linked profiles grow by two words against the v7 baseline: disk-root
single last 036645 (602 decimal words free), multi last 036706 (569 free),
tape-root last 036017 (1008 free). Native boot remains untested; no dynamic
table addressing is active yet.

## Production mount mutation isolation (v9)

`fs/vfs.s` now routes mount insertion-slot discovery through
`vfs_mount_find_slot`, successful target/root/policy publication through
`vfs_mount_publish_slot`, and successful unmount slot retirement through
`vfs_mount_retire_slot`. All three remain native resident assembly and retain
the exact existing four-slot table layout and policy representation. The
find-slot helper returns its status in AC0, leaving AC1 and the packed mount
arguments available to the caller. Publication takes its flags in AC0 so
its extra PDP-6 return address cannot change the KINIT/proc stack offsets.

The dedicated disk-root linked size gate passes: single-root permanent last
036657 (592 decimal free), multi-root last 036720 (559 free). The dedicated
tape-root gate passes: permanent last 036031 (998 free). Relative to v8, the
extra three calls and cleanup paths grow the folded linked permanent size by
10 decimal words. The native `p10run --machine pdp6` tests for existing VFS
storage reservations and STAT stack frames pass. A new test in daimos-testkit,
`tests/validation/daimos-vfs-mount-mutation-v9.sh`, also executes production
mount/unmount and verifies all policy bits, root retirement and slot reuse.
This validates the current four-slot behavior, not dynamic addressing or full
PDP-6 operating-system boot acceptance.

`system/kernel/fs/vfs_mount_access_v5.s` is the first standalone native
consumer of the proposed slab. It takes explicit absolute target/root bases,
the KINIT-selected capacity and a target vnode, searches only in-bounds slots,
and returns an occupied matching root (or zero). It skips unpublished entries
even if the target matches, preserving the empty-slot distinction. Its DAS
standalone assembly succeeds. It is intentionally not linked into active
KCORE, and no native execution test is claimed. Before cutover, add a native
testkit execution fixture and migrate remaining parent, lookup, mount,
unmount and FSINFO paths as a coordinated change. Direct fixed-symbol access
cannot safely coexist with slab-based publication.

The same staged assembly now exports checked READ/WRITE/CLEAR table access
primitives for slots 0..capacity-1. READ publishes the full 36-bit datum
separately from success/failure status, and invalid access does not mutate
the table. DAS object generation is checked in daimos-testkit at
`tests/system/vfs-mount-access-v5`; native runtime execution remains pending.

## Capacity policy

MONITORFS built-in entries consume no VFS mount slot. The root D6FS or TSFS
consumes one. MEMFS consumes one when mounted (singleton today). Independently
mountable D6FS, DTFS, and other supported volumes consume one each. Non-root
TSFS sets are not supported. A validated disk/drum BLOCKSET consumes one slot
regardless of member count. Until non-root disk/drum discovery is corrected,
count every unit outside known root BLOCKSET membership as potentially
mountable: do not infer a drum belongs to a set or is dedicated to swap.

Support up to 16 slots (the six-bit vnode mount-ID format has sufficient
headroom), but allocate only the boot-selected hardware-derived capacity.
Do not round capacity to a power of two. Three densely packed policy bits
per mount require two 36-bit words at 16 mounts; measure the
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
  rely on the one-word packing. A simple 4->16 limit change is invalid.
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
independent 16-bit RO/SWAP/LOGSTORE maps needs three words. A fully packed
three-bit-per-mount encoding needs two words for 16 mounts, but may cost more
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
   RO, SWAP and LOGSTORE independently for up to 16 mount slots. Avoid
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
5. Test four-slot baseline and 16 slots, root-only and mixed filesystems,
   exhaustion, mount-ID reuse, RO/remount and SWAP/LOGSTORE pinning; place
   tests in daimos-testkit. Compare full post-p10fold KCORE and permanent plus
   dynamic RAM; boot and test on PDP-6 SIMH before activation.

Block cache is explicitly out of scope. MM dynamic descriptor sizing and
MEMFS data tuning are separate tasks.
