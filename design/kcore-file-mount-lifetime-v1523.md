# KCORE v1523: stable FILE/cwd ownership before VFS mount retirement

Date: 2026-11-12. P0-B KCORE feature-completion increment, following
the v1522 DTFS physical-error identity gate. VERSION remains 0.5.
This is source-linked native PDP-6 acceptance, not (by itself)
physical hot-swap or P0-A permanent root/BACKSTORE loss detection.

## Source bug found

Before v1523, `vfs_unmount` checked PIPE and process executable
BACKSTORE owners, then synchronized a mounted filesystem and retired
its VFS mount ID. It called `file_unlock_mount`, which only clears
FLOCK bits in the **current process's** descriptor table. It does
not invalidate a FILE vnode and does not check another process's
descriptors or current working directory. Those vnodes contain a
six-bit mount ID and can later resolve into a *different volume*
when the freed slot is recycled. The same risk exists with a
swapped process: the u-area, including cwd and FILE records, stays
resident even when the user VM has no physical address.

The original inline owner checks contained a separate bug:
`pipe_fifo_mount_busy` uses/clobbers AC2, but the following
`proc_swap_mount_busy` derived its public mount ID from AC2
as though it still held the zero-based VFS slot. This could
query the wrong swap owner and miss a real dependency.

## Implemented v1523 fixed KCORE contract

`file_mount_busy_v1523(public_mount_id)` scans the **existing**
three-word `proc_table`, 0 through `proc_high_slot - 1`,
without building an independent registry or allocating memory.
Only entries with the authoritative `PROC_F_UAREA` metadata bit
and nonzero stable physical u-area pointer are scanned. In
each such u-area, it checks the cwd vnode at octal `025`
and all sixteen two-word FILE entries starting at `026`.
It extracts the full six-bit mount ID from the existing canonical
vnode (not FILE metadata or a guessed truncated index), returning
busy for any reference. It does not inspect or depend on the
resident user VM pointer and therefore covers swapped owners.
Unused entries and slots with no owned u-area are ignored.
AC10..AC13 are preserved; no process, MM or filesystem state is
modified by the scan.

`vfs_unmount` now uses one shared
`vfs_unmount_owner_check_v1523` for PIPE, process-swap and
FILE/cwd references. The helper saves the *public mount ID*
across all three calls, fixing AC2 clobber without another
stored owner. It runs both **before** potentially sleeping
`vfs_sync` / PREPARE_UNMOUNT and **again afterward**, just
before namespace and slot retirement. The second check is
essential: another process may open a FILE or change cwd while
the provider sleeps; a stale result from the first check
cannot justify retirement. Following the precommit check the
executive performs no potentially sleeping provider call;
the existing namespace/lock cleanup and slot retirement are
unchanged. Any busy result preserves the original mount,
FILE/cwd/PIPE/backing ownership and returns the usual refusal.
No additional persistent table, reference counter, bitmap or
process descriptor field was introduced.

This guard also protects an otherwise healthy mounted DTFS
volume (not only one flagged by v1522). A failed physical write
still retains WRITE_UNCERTAIN and never becomes safe merely
because an unmount or revalidation was attempted. Root and
BACKSTORE permanent loss are unchanged, with no new HALT
classification or controller-status heuristics.

## Native PDP-6 acceptance

New `daimos-testkit/tests/validation/daimos-file-mount-busy-v1523.sh`
source-links **the real** `file_mount_busy_v1523` and
`vfs_unmount` assembly. The fixture uses actual three-word
process descriptors and three stable octal `0410` u-areas,
with occupied FILE records in their production offsets.
PIPE, process-swap and physical provider SYNC are the only
controlled boundaries; their ABI intentionally clobbers AC2.
The scenarios cover:

1. An ordinary current-process open FILE on the secondary mount
   prevents SYNC, unlock and retirement.
2. A different process with no resident VM, owning FD15
   (the last of sixteen descriptors), also prevents unmount.
3. Remote cwd ownership blocks retirement after all FILEs close.
4. Invalid or unowned stale u-area contents do not block it.
5. With no live references, physical SYNC failure still refuses
   unmount and retains all mount/namespace state.
6. An independently opened remote FILE *during a sleeping
   SYNC* is caught by the precommit recheck: the mount survives.
7. After the remote FILE closes and the physical medium has
   returned, the secondary mount is retired exactly once, with
   the independent root and namespace unchanged.

Seven executed unsafe source mutations are rejected: remove
the FILE/cwd check, ignore remote processes, ignore cwd,
ignore FD15, break public-ID preservation before swap check,
break it before FILE check, or bypass the precommit check.
The original v1514 nonroot VFS failed-SYNC/return/ordering test
and v1522 physical error/volume identity native test also PASS.
The v1514 fixture supplies a zero-return mount-reference stub
because its historical scenario has no live process references;
only v1523 establishes the real all-process FILE/cwd scan.

## Production build and controlled size

A full isolated 256K KCC tape-root `image` built successfully
from the updated source: fixed KCORE, optional DTFS/TSFS MRES,
KINIT, compiled tools and media generation. This is a build/
link result, **not** a substitute for a booted guest test.

For exact RAM accounting, the v1522 `vfs.s` and
`file_runtime.s` were reassembled from committed
`ce33a2c0` and substituted into an old KCORE relink.
Both old and v1523 links use identical KCC outputs,
the same other objects and independently computed fold plans.

| Symbol (octal address) | v1522 control | v1523 | Change |
| --- | ---: | ---: | ---: |
| `__kcore_low_init_end` | `023110` | `023176` | +54 decimal words |
| `__kcore_low_end` | `023525` | `023613` | +54 decimal words |
| `mm_extents` | `023525` | `023613` | +54 decimal words |

**+54 fixed resident instruction words, zero new data words.**
No MRES code or dynamic mount-slab geometry was changed.
The independently deferred strict 32K target remains on HOLD.
Optimization/folding work is outside this feature stage.

## Remaining P0 gates

- P0-B: full physical error/return/hot-swap acceptance with
  genuine open FILE descriptors, dirty/partial writes, repeated
  media exchange, checked identity, multi-process progress and
  administrative recovery for sticky WRITE_UNCERTAIN.
- P0-A: authoritative permanent root/BACKSTORE and exact TSFS
  selected-member loss classification, followed by qualified
  v1515 HALT dispatch. A generic Type-551 select error is not
  hardware-proof of permanent loss.

The existing full tape-root boot fixture now offers a separate
v1523 cwd-unmount mode. It uses a fully initialized physical
DTC3 tape rather than a zero-byte SIMH image and intentionally
does not rerun target MKFS formatting as a prerequisite to the
unmount lifecycle test. That guest test must be reported with
its own actual result; native tests above do not imply it PASSED.
