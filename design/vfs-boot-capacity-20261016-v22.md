# VFS v22 — capacity from boot-validated controller and root inventory

## Lifecycle and safety

KINIT obtains a selected root during `root_select_minit`, before the end of
MINIT processing. D6FS root selection encodes up to four *physical* disk or
drum unit locators in two handoff words. A missing half is `0777777`; each
occupied half carries its physical unit in bits 16..17. The v22 selector
decodes the physical units, deduplicates them, and rejects an empty root mask.
Logical blockset member indices must **not** be mistaken for physical units:
a blockset can be constructed from noncontiguous device units.

The TSFS selector independently validates the selected DECtape set before
retaining an eight-bit mask of selected physical units. The mask is transient
KINIT state, not a new permanent table. It is zero for unresolved selection.

`root_select_mount_capacity_v22()` reads the selected root and installed storage
service addresses in the transient MINIT directory. This distinguishes a
controller that passed its hardware probe from one merely compiled into the
image. It counts all supported unit *positions* on an installed controller,
including an empty DECtape transport; removable media occupancy must not
change the namespace table capacity. It also excludes devices whose mount
filesystem MRES is intentionally absent in this root profile: D6FS is not
installed with a TSFS root, while DTFS/TSFS is not installed with a D6FS
root. Full-profile MEMFS uses one slot; reduced profiles omit that slot.

The existing pure `vfs_mount_capacity_v1()` checks counts, root member masks,
exactly one root class, and the 26-mount upper bound. For the current MRES
matrix, typical capacities are:

| Boot profile and detected services | Root members | Slots | Slab words |
|---|---:|---:|---:|
| Full, DSK+DRM with D6FS, MEMFS | 1 | 9 | 54 |
| Full, DSK+DRM with D6FS, MEMFS | 4 | 6 | 36 |
| Full, DSK alone with D6FS, MEMFS | 1 | 5 | 30 |
| Full, DTC with TSFS, MEMFS | 1 | 9 | 54 |
| Full, DTC with TSFS, MEMFS | 8 | 2 | 12 |
| Reduced DSK profile, no MEMFS | 1 | 4 | 24 |

Each selected multi-member root occupies **one** mount slot, not one per
member. Other physically available and currently mountable units remain in
the budget. Only one 6-array MM allocation is made; its size is six times
the computed slot count. This does not increase permanent or per-process RAM.

KINIT calls the planner after all MINITs but **before** boot root mounting,
allocates and clears the complete MM slab, migrates the boot D6FS reader,
publishes all six indexed base pointers, then writes `vfs_mount_limit`
**last**. The kernel cannot observe a larger limit with old or uninitialized
tables. Bad selections, unavailable root services or allocation errors are
fatal, not silently downgraded to a four-slot fallback.

The underlying vnode, FILE descriptor, readonly/storage-owner policy and
PURE swap formats permit up to **26 slots**; the current installed-provider
configuration needs at most nine. A future mixed-provider root profile may
select a greater number once its MRES installation policy is implemented and
validated. The capacity planner is independently tested through 26, but the
present hardware/profile matrix does not advertise phantom mounts.

## Verification

Native PDP-6 tests compile the production `root_select.c` and capacity
planner for both full and reduced boot profiles and substitute only the
physical discovery sources. They validate noncontiguous disk members,
four-member sets, drum and DECtape roots, one/eight-tape selections, device
controllers omitted after failed MINIT probes, and unresolved roots. Other
tests exercise the MM slab allocator and VFS's mount-26 policy and provider
dispatch. The full disk-root diagnostic PDP-6 SIMH boot reaches ROOT login
and ECHO using the existing **test-only** DSH size-gate override.

32K footprint checks are static memory-layout measurements requiring no
shell and are **not** successful 32K boots. The historical strict KCORE
`020000` gate still fails. The permanent disk/tape size checks remain in
force. Hardware root/mount tests beyond the attached media and full 26-slot
simultaneous live mounting are not claimed.

The v22 measured GCC 32K KCORE end is `024142` octal, 2146 decimal words
above the strict gate. Disk/tape KINIT image ends are `073277` / `074277`,
leaving 1345 / 833 decimal words after the reserved boot stacks. Permanent
single-root last words remain disk `037355`, tape `036537`; disk multi-root
last word `037416` (octal). New inventory/planning code is reclaimed with
KINIT; resident KCORE remains unchanged.
