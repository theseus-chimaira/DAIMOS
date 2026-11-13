# KCORE v1527: physical DTFS return and shared BCACHE identity

Date: 2026-11-13. Scope is **KCORE feature correctness** only:
real mounted DTFS media failure/return and independent-volume
isolation, not optimization, MRES policy expansion, or a new
on-media format. DAIMOS VERSION remains 0.5; 32K gate HOLD.

## Root cause: raw identity read left a false BCACHE slot-0 tag

The v1526 real PDP-6 guest accepted mounted DTC3 and
cross-process FILE/cwd ownership, but the v1527 exchange
guest exposed a separate interaction of two real storage
services. It mounted stamped native DTC3 and independently
stamped native DTC4, opened a DTC3 directory FILE, detached
DTC3 at a controlled simulator stop, and exercised genuine
physical read failure. DTC3 was refused, DTC4 remained usable.
After attaching a different stamped DTC3 volume, its old
FSINFO/FILE were again refused, but FSINFO/STAT on untouched
DTC4 unexpectedly failed.

The failure was not guessed to be missing hardware. A live
SIMH memory snapshot showed the shared provider lease FREE
(`fs_provider_ready=777777777777` octal), MM capacity seven,
the original DTC3 still published with media
`000000200003` (READ_SUSPECT), and the independent DTC4
still published with media `000000000004` (unit 4,
**no** READ_SUSPECT/WRITE_UNCERTAIN bits). The real
Type-551 controller trace showed DTC3 commands, but no
physical DTC4 request after the replacement. The failure
therefore occurred before another DTC4 transfer.

Production `bcache_pdp6.s` uses `fs_block_workspace`
as its clean L1/slot-0 data. Its tag is
`bcache_workspace_tag`; a matching `bcache_fetch`
may return an immediate slot-0 hit without reloading
physical metadata. The v1522 `dtfs_identity_revalidate_v1522`
path correctly bypassed BCACHE to authenticate physical
RBN0, but read that RBN0 **directly into the same
fs_block_workspace without invalidating the slot-0 tag**.
After a rejected replacement it left a false TSFS file
record cache hit; TSFS parent/path reconstruction could
then treat the DTVOL1 identity record as a valid cached
TSFS metadata record. Both the DTC4 FSINFO path and
pathname STAT failed although the mount was healthy.

The production fix is precisely **one instruction** in
`system/kernel/fs/dtfs_runtime.s`: invoke
`bcache_workspace_invalidate` immediately before the
raw physical RBN0 read. That existing leaf only invalidates
the workspace tag, preserves AC1..AC3 and the independent
BCACHE slot 1, and does not need a second buffer or any
new persistent state. The earlier native mount-admission
path already invalidates both BCACHE identities before
its independent raw checks and is unchanged.

## Corrected real native source-linked regression

The older v1522 native mounted-ID fixture simulated
physical replies and MM table data, but the local
BCACHE BSS symbols used by the fixture had shadowed
the real assembler's local BCACHE state, so the fixture
had not exercised the production shared slot-0 tag.
The v1527 update exports the two existing local BCACHE
symbols **only inside the linked native test image**,
removes the fixture's duplicate variables, places
a synthetic TSFS clean block tag in the real slot 0,
then performs the production different-ID revalidation.
It asserts the tag is invalid and the old block cannot
be returned as a workspace hit. The test also verifies
the normal valid same-ID return and all existing v1522
failure cases.

Native PDP-6 result: baseline PASS; all original seven
unsafe source mutations rejected; the eighth
`stale-workspace-tag-v1527` mutation removes the
new invalidation and is rejected by an executed
stale-cache assertion. No fake BCACHE is used.

## Complete physical DTC3/DTC4 acceptance

The rebuilt full 256K KCC PDP-6 tape-root guest actually
reports `DTFS LOADED` and starts its disposable KCC
PID1 test from physical TSFS DTC0-2. The test creates
three distinct full 72-bit DTVOL1 identities:
original DTC3, independent DTC4, and replacement
DTC3. Host preflight verifies marker, both nonzero
36-bit halves, checksum and distinctness. The guest
uses real KCORE mount/FSINFO/FILE/unmount syscalls,
MM tables and Type-551/136 physical transfers.

The controlled SIMH console pauses and detaches
**only DTC3**, resumes the guest and observes
the mounted DTC3 old FILE and FSINFO refusing
a real physical read error. DTC4 remains accessible.
It attaches a valid but differently stamped DTC3;
the original mount's FSINFO and FILE remain refused.
Crucially, DTC4 FSINFO succeeds **before and after**
that different-volume probe and after the old FILE
refusal. It then detaches the replacement and
reattaches the **original** DTC3 image. KCORE
validates the full physical RBN0 and both saved
ID halves and safely restores access through the
original still-open FILE. After FILE close, both
DTFS mounts unmount cleanly with no FSINFO slots
left behind. The physical guest prints
`DTFSMEDIA V1527 RETURN_VERIFIED`, `PASS`,
and the host runner exits zero.

This closes P0-B physical **READ_SUSPECT**, different
volume fail-closed, matching identity recovery,
open-FILE preservation, and separate volume isolation.
This does **not** establish physical partial-write
`WRITE_UNCERTAIN` recovery, cross-reboot durable
cleanup, or P0-A root/BACKSTORE permanent-loss proof.
The previous v1526 cross-process physical owner test
is independently retained as a regression.

## Exact code and RAM accounting

The full KCC image rebuild PASSed. Its optional
DTFS/TSFS MRES changed from octal `003746`
text / `000002` BSS to **`003747` text /
`000002` BSS**, exactly **+1 optional
instruction word, +0 BSS**, no additional
resident data words and **no fixed KCORE change**.
Only the existing BCACHE workspace tag is used.
The strict 32K gate remains HOLD; this increment
does not attempt optimization.

Remaining feature work: P0-A authoritative
critical TSFS-root/BACKSTORE permanent-loss
classification and v1515 HALT, P0-B actual
failed/partial **physical write** with sticky
WRITE_UNCERTAIN and explicit safe administrator
recovery, P1-C mixed storage errors, P1-D
dynamic FSINFO capacity/inventory acceptance,
P1-E uncompressed swapless PURE/MM/APR lifecycle,
and final feature freeze.
