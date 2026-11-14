# KCORE P0-B v1553: authenticated DTFS first-write state gate

Date: 2026-11-14. Tested PDP-6 source, still deliberately unlinked
from regular mounted DTFS. P0-B remains **OPEN**.

## Source sweep: the actual integration boundary

DTFS data and directory mutations converge on `dtfs_dtc_write`.
Its public ABI contains only a physical DTC unit, block and buffer;
that is insufficient to identify a particular mounted volume and
its original 72-bit identity. The existing native mount path
performs a raw physical DTVOL1 identity read and stores both
halves in `tsfs_file_shape_base` / `tsfs_extent_media_base`
indexed by the published VFS mount slot. The mount's media word
is already in MM-backed `dtfs_media_base`, carrying physical unit,
personality, sticky READ_SUSPECT/WRITE_UNCERTAIN and UID/GID owner.

The central `fs_provider_reg_call` serializes DTFS, TSFS and D6FS
service requests across sleeps using one process-owned lease.
All future raw-RBN0, data-write, and final-clean state changes
must happen under that same provider lease. A private raw DTC
write does not by itself acquire it. In particular the existing
VFS `vfs_unmount` checks owners, calls sleeping provider SYNC,
then checks again: **there is no quiescent owner lock across a
sleeping CLEAN seal**. New FILE/cwd/PIPE/swap owners must not be
allowed to enter while that seal is in progress.

## Implemented now

`system/kernel/fs/dtfs_media.h` has two reserved right-half
runtime flags: `DTFS_MEDIA_FORMAT_V2_V1553` = octal `0100000`,
and `DTFS_MEDIA_INTENT_ARMED_V1553` = octal `0040000`.
They do not touch the existing READ_SUSPECT `0200000`,
WRITE_UNCERTAIN `0400000`, TENEX/ITS, physical unit, or 18-bit
UID/GID owner. Neither flag appears in an on-media block.

`system/kernel/fs/dtfs_intent_first_write_v1553.s` is a
PDP-6 assembly entry with AC1=zero-based VFS mount slot,
AC2=pointer to the provider's retained one-word MM scratch base.
Its caller holds the exclusive filesystem provider lease.
Before authorizing a first write it checks:

- slot in range, live published vnode root, exact DTFS provider
  number 5 and correct public slot+1 mount ID;
- native-personality DTVOL2 flag, with neither READ_SUSPECT nor
  WRITE_UNCERTAIN, and physical DTC unit from the published word;
- both saved nonzero 36-bit physical volume ID halves from the
  existing per-mount MM-backed shape/extent tables;
- no outstanding MM scratch owner handle from a previous refused
  unpin/free; reentry never replaces such an allocation.

If the per-mount ARMED bit is unset, the gate calls the tested
v1552 pinned-MM wrapper, which calls the tested v1551 raw RBN0
WRITE_INTENT write/readback. Only if that returns zero and the
published media word has not changed while I/O sleeps does it
set ARMED and authorize the first DATA/DIR write. Any failed
physical marker, MM release or identity/state recheck refuses
the request without marking it armed; an unresolved scratch
base is retained for exact owner-aware recovery.

The current gate also recognizes a previously ARMED mount with
no suspect bits/retained scratch. This *in-RAM* fast path is
not by itself evidence that the physical DTC medium was unchanged
since the first write; it must not be enabled in a production
provider without a media-change generation check or a fresh
readback validating the DIRTY marker and exact identity before
subsequent writes. Never treat a cached ARMED bit as on-media
durability evidence.

## Tests

`daimos-testkit/tests/validation/daimos-dtfs-first-write-v1553.sh`
links the **actual assembly gate** to a strict tested-v1552
physical/MM-service ABI oracle and runs it on a real PDP-6 CPU.
Eighteen scenarios PASS, covering successful first arm, already
armed, DTVOL1, suspect and uncertain media, wrong provider and
mount ID, retired mount, missing either identity half, retained
MM owner, physical failure and failed cleanup, altered mount
state during I/O, absent scratch pointer, invalid slot, foreign
personality and exact preservation of an 18-bit UID/GID owner.
Thirteen unsafe actual-source mutations fail the native oracle:
wrong version, suspect bypass, provider/mount mismatch, root
loss, wrong identity, abandoned retained scratch, skipped RBN0
arming, ignored arm failure, changed media, dropped armed-bit
publication and altered physical DTC unit. The test also checks
the shared C and assembly flag values for exact agreement.

An initial fixture had used `050102000000` to simulate a
different mount, but this changes vnode local kind. The correct
different public mount ID is `050201000000`. The fixture was
corrected, and the genuine mount-ID check subsequently PASSed.

## Not complete; next production features

This entry is **not linked to DTFS MRES or enabled by mount**.
Current mounted DTFS remains DTVOL1 and follows the original
write path; it does not yet guarantee persistent poisoning.
No fixed KCORE words, permanent RAM, ABI or default media-format
change was introduced by v1553.

Complete P0-B in this order:

1. Physically validate the same DTVOL2/DIRTY marker and both ID
   halves before **each additional write** on an already-armed
   volume, or prove a genuine Type-551 media-generation guard.
2. Add an MM-owned retained scratch-base slot and check it through
   module teardown, and publish FORMAT_V2 only after a complete
   physical DTVOL2/CLEAN mount; never promote legacy/dirty media.
3. Require every file and directory mutation (including resize,
   chain traversal, and directory commits) to pass the gate once
   on the authenticated mounted volume. No raw bypass or recursion.
4. Implement an exclusive VFS unmount quiescence phase which
   blocks new owners across sleeping final sync/clean operations;
   recheck owners and media identity after I/O. Only then commit
   and physically read back a CLEAN RBN0, and retire mount state.
5. Run real 256K full-guest Type-551 partial-write, physically
   detached/reinserted old tape, independent replacement and
   cold-boot acceptance with no false owner release.

P0-A remains complete, P0-B OPEN, P1-C separate. VERSION 0.5;
strict 32K/optimization HOLD.
