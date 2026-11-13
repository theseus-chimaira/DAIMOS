# KCORE v1536: persistent MEMFS snapshot and process BACKSTORE ownership

Date: 2026-11-13. P1-C feature correctness. This is NOT optimization,
new storage routing, or a production kernel/MRES change.

## Implemented source contract

`proc_swap_boot_init` in `system/kernel/proc/proc_swap_boot.c`
reads the last BACKSTORE block as the possible persisted MEMFS
snapshot header, invalidating the shared BCACHE workspace first.
For supported v1/v2 marker/version and a bounded nonempty range
ending at the final block, it marks that entire range in the
authoritative 36-bit-per-word BACKSTORE bitmap *before* swap use.
`backstore_alloc`, `backstore_verify_allocated_v795` and
`backstore_free` in `system/kernel/storage/backstore.s` use
the same bitmap and exact ownership accounting. A MEMFS snapshot
HALT writes its commit marker last, after data and metadata.

## v1536 source-linked KINIT reservation acceptance

The testkit compiles the complete production `proc_swap_boot.c`
using host-only physical MM allocator and BACKSTORE read stubs.
No reservation body is reimplemented. Nine live cases PASS:
valid committed v1 and v2, unsupported version, invalid magic,
empty span, start beyond capacity, span exceeding capacity,
span not ending at commit block, and failed header read.
For a 145-block BACKSTORE containing a 38-block snapshot in
107..144, the test requires exact bitmap bits, correct used-block
count, zero swap usage, one MM table/slot-map publication, one
header read and one BCACHE invalidation. Invalid headers must
never fabricate a reserved run; repeated initialization refuses.

Four compiled source mutants are executed and rejected:
discard v2 acceptance, reverse commit-last range condition,
omit BCACHE invalidation, and erase bitmap bits.

## v1536 native PDP-6 process allocation and ownership

The testkit directly extracts original production assembly
`backstore_alloc`, `backstore_verify_allocated_v795` and
`backstore_free` and executes them on the PDP-6 through
`p10run`. The same 145-block bitmap contains snapshot 107..144
and an independently held process block 0. The native test
refuses a 107-block allocation and an unauthorized free
crossing the snapshot start, allocates 100 blocks at block 1,
then another 6 at block 101, refuses further allocations,
releases both temporary owners, and rejects duplicate free.
The snapshot remains allocated throughout; the original
process block remains owned. This also crosses 36-bit bitmap
word boundaries.

Three executed unsafe instruction mutants are rejected:
invert bitmap occupancy, omit allocation accounting, and
bypass complete-run ownership preflight. The original
test missed the final mutation because it only exercised
valid frees; adding invalid/duplicate releases fixed that
oracle weakness. The production implementation did not change.

## Physical regression and limits

An independently rebuilt, current-source 128K full KCC
PDP-6 v1477 two-boot snapshot test PASSes. Boot 1 commits
safe MEMFS XIP and data at HALT. Cold boot 2 with new RAM
and the same AUXSTORE physical images restores exact file
contents/executable, runs XIP and HALTs. AUXSTORE partition
descriptor geometry remains unchanged after each boot.

**Accepted:** KINIT persisted snapshot reservation and
native BACKSTORE process-owner allocation/free isolation,
plus current-source snapshot cold-reboot preservation.

**Not accepted yet:** a single real PDP-6 with active
swapped process restoration at the instant a persistent
MEMFS snapshot commits, simultaneous DSK/DRM/DTC failure,
or LOGSTORE queue fault/return. Separate source-linked
tests are not proof of concurrent PI/MM/VFS ownership
rollback. P0-A authoritative permanent critical loss,
P0-B durable old-volume admission, other P1-C/D/E and
final feature freeze remain open.

No new permanent RAM, KCORE/MRES instructions or on-media
format. VERSION 0.5; strict 32K/optimization HOLD.
