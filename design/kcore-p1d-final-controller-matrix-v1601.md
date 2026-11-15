# P1-D v1601: final dynamic FSINFO and physical controller matrix

Date: 2026-11-15. Feature-acceptance conclusion for P1-D only;
P1-C dirty/write-through and loaded-simulator repeatability remain
open. DAIMOS VERSION 0.5; strict 32K/optimization gate HOLD.

## Production contract inspected

`system/kernel/boot/root_select.c` derives effective available
controllers from the original MINIT-published *paired READ and WRITE*
service handlers. D6FS disk/drum transports count only on a DSK/DRM
selected root; Type-551 transports count only on a DTC/TSFS root.
`system/kernel/boot/vfs_mount_capacity_v1.c` validates the selected
physical member mask, rejects missing/contradictory root inventories,
subtracts the members reserved for the root, and adds the one selected
root and optional MEMFS mount. KINIT reserves one MM-backed slab of six
mount arrays and publishes `vfs_mount_limit` last, only after all array
bases are initialized. No fixed four-slot fallback is permitted.

Original `native_sys_fsinfo` in
`system/kernel/proc/syscall_dispatch_pdp6.s` returns the published
capacity for AC2=-1 without touching userspace memory, 1 for an
occupied ordinary slot, 0 for a genuinely empty published slot, and
-1 for invalid slots or an unavailable provider. MOUNTS and DF in
`userland/exec/commands.c` obtain the capacity and scan all slots;
they cannot silently truncate to the historical four-slot constant
or interpret a published-but-failed mount as an empty slot.

## Accepted selected-root/service matrix

| Root and available services | Actual KINIT slots | Evidence |
|---|---:|---|
| Three-member DTC/TSFS (8 physical DTC positions) | 7 | v1532/v1533 full 256K PDP-6, genuine FSINFO, real nonroot DTFS |
| Two-member DTC/TSFS, DTC2 empty | 8 | v1591 full 256K PDP-6, real DTC3..6 mounts IDs 2..5 |
| Four-member DSK270/D6FS + DRM236 service | 6 | v1594 full 128K PDP-6, raw four-member DSK identity oracle |
| Four-member DRM236/D6FS + DSK270 service | 6 | v1593 full 128K PDP-6, four real DR units, FSINFO |
| Four-member DSK270/D6FS, upstream Type-167 DP/DR absent | 2 | v1597 full 128K PDP-6, four raw DSK members and XIP lifecycle |
| Four-member DRM236/D6FS, Type-136 DSK270 absent | 2 | **v1601** full 128K PDP-6, four raw DR members, linked MM/FSINFO and XIP |
| Selected root controller missing or lacks READ or WRITE service | invalid (0 capacity planner) | v22 original root selection/planner executed as native PDP-6 assembly, v1601 extended negative controls |
| Independent nonroot controller missing one READ/WRITE service | no phantom transport slots | v22 native PDP-6 full-profile service-pair acceptance |

The dynamic planner can describe up to **9** slots with the currently
compiled root/service classes when one physical unit is selected as
the root. The FSINFO representation and its source tests support
capacity up to **26**; that is not a claim that 26 transports are
physically installed or booted. A disk-only reduced profile has no
optional MEMFS/DRM transport slots; full native v22 tests also cover
this separate profile.

## Published provider failure versus empty slot

v1595 physically removed a mounted native DTC3 volume while the
original two-member TSFS root remained healthy. Its existing public
slot returned FSINFO **-1**, not empty 0, while three other separate
DTFS mounts, including highest public ID5, retained their identity.
A replacement with a different physical 72-bit media identity was
refused, the original reel revalidated, and all nonroot mounts retired.
The independent physical raw-media oracle rejected five corrupted
witnesses. v1596 additionally detached highest occupied DTC6, with
the unaffected mounted DTFS volumes and independently writable MEMFS
continuing correct original guest work; return and retirement PASS.
This needed a corrected simulator Type-551 drive state (sims v1598),
not a DAIMOS kernel workaround. This FSINFO/provider-identity policy
is now accepted for P1-D; it is not a general claim that owned-root
or BACKSTORE failures can recover.

## Final v1601 original physical root and negative controls

The reverse v1597 configuration is now authentic: the simulator
disables `DSK` before MINIT and leaves actual Type-167 `DP` and four
DRM units attached. Four independent 8192-sector DRM D6FSR2 root
members boot; preboot descriptor bytes equal their post-HALT RBN0,
with unique original member IDs and geometry
`000001+017377+000400` octal. The actual original KCORE linked
words include `vfs_mount_limit=023703` octal, whose post-HALT value
is **2**. Both separate MM-backed mount bases are nonzero, distinct,
and resident. Source-authentic `dsys_fsinfo` proves occupied root
provider 6/ID1 and MEMFS provider 4/ID2, invalid slots 2/3, and
normal XIP RUN/WAIT/EXEC completion with no simulator errors.

The independent oracle rejects **10** corrupt physical, service,
FSINFO, MM, and lifecycle witnesses. The preceding diagnostic run
before the strengthened preboot capture is explicitly excluded from
acceptance. Complete accepted evidence and the exact reproduction
command are in testkit
`tests/validation/daimos-fsinfo-drm-no-dsk-physical-v1601.md`.

Current-source native v1492 real FSINFO + MOUNTS/DF tests PASS,
including the broken comparison negative control and capacities
1/4/8/26. v22 original root-select/planner full and disk-only native
tests PASS, including the extra incomplete service-pair scenarios.
The source-linked host capacity limit and invalid-mask tests PASS.

## P1-D closure boundary

**P1-D dynamic FSINFO, alternate root/controller inventory, missing
hardware and published high-slot provider failure/return: ACCEPTED.**
No production source, KCORE RAM footprint, syscall ABI, MRES or
media format was changed by v1601. Existing old dated TODO/status
sections remain historical evidence, superseded by current status.

This does **not** close P1-C's dirty/write-through error isolation,
cross-provider cache/process owner timing or multiple loaded physical
guest repeatability. It does not authorize P1-E KA10 PURE/MM/APR
changes or lifting the strict 32K/optimization HOLD.
