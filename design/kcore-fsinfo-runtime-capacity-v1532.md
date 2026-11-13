# KCORE v1532: FSINFO dynamic mount capacity, real PDP-6

Date: 2026-11-13. P1-D feature-completion acceptance. No optimization,
MRES redesign, or additional permanent mount-slot table.

## Runtime contract

KINIT already inventories installed controllers, validates the selected
root and allocates the six MM-backed mount arrays. It publishes the
resulting `vfs_mount_limit` only when the slab is ready. The old
`VFS_NMOUNT=4` constant was still used by the MOUNTS and DF command
consumers despite the published runtime count.

The previously uncommitted v1492 contract is accepted:

* FSINFO (UUO 077 extension 051), AC2=-1, returns the positive
  KINIT-published slot count without looking at AC3 or mapping user
  memory. A zero/unpublished count returns -1.
* Ordinary AC2=zero-based slot remains 1 for occupied, 0 for empty,
  and -1 for an invalid slot or provider error. In particular, an
  empty slot cannot terminate enumeration.
* MOUNTS and DF query the count once and iterate all of it, preserving
  provider errors rather than silently truncating at four or stopping
  at the first empty entry.

The production implementation lives in
`system/kernel/proc/syscall_dispatch_pdp6.s`, `syscall.h`,
`fs/vfs.h` and the necessary user-facing MOUNTS/DF loops in
`userland/exec/commands.c`. No unrelated userland functionality
was added. No fixed four-slot fallback is permitted.

## Native v1492 verification

`daimos-fsinfo-mount-limit-v1492.sh` links and executes the exact
production FSINFO assembly on PDP-6 and rejects a mutated capacity
comparison. It verifies pointer-free capacity, empty last slot,
first out-of-range, zero/unpublished count and the 26-slot boundary.
It also extracts and compiles the actual MOUNTS and DF C functions,
with host-only FSINFO fixtures for 1, 4, 8, 17 and 26 slots, gaps,
errors and unavailable capacity. All assertions PASS.

## Physical full guest acceptance

A fresh 256K KCC PDP-6 ROOT=tape BOOT=ptr image was built from the
current FSINFO sources, rather than using a pre-v1492 KCORE. The
real PID1 `daimos-fsinfo-capacity-physical-v1532.c` test boots
from three TSFS root tapes on DTC0-DTC2. KINIT's actual inventory
of one selected three-member root, five spare DTC transports and
one MEMFS capability publishes **seven** MM mount slots.

The full physical guest:

1. Queries the count with a null userspace record: exactly 7.
2. Reads the real TSFS root in slot 0 (provider 7).
3. Mounts physically stamped native DTC3 at `/MOUNT/DT0`,
   verifies provider 5 and public ID 2 in slot 1.
4. Scans every published slot; slots 2 through 6 are empty
   (zero, not errors), with exactly two occupied slots.
5. Verifies first and next out-of-range slots return negative.
6. Normally unmounts DTC3; slot 1 becomes empty, the count
   remains 7, and root FSINFO still succeeds.

The full guest reports `FSINFOCAP V1532 CAPACITY7`,
`SCANNED7`, `RETIRED`, `PASS`, and the host exits zero.
All filesystem I/O, MM mount publication and user/syscall
transitions are real. No mock FSINFO provider is involved.

This verifies high-slot enumeration but **not yet a physical
mount occupying an ID above four**, nor a 26-volume full guest:
the current actual DTC-root inventory has seven slots. More
physical mounts and cross-inventory acceptance are distinct
remaining P1-D checks.

The fixed KCORE and userland source changes carry no new
permanent data word. The exact newly linked byte/word delta
versus v1531 must be reported from matching full-image builds;
it is not inferred from source line counts. VERSION 0.5 and
strict 32K/optimization HOLD remain unchanged.
