## 2026-11-13 v1550: native DTVOL2 validator and opt-in formatter

The v1549 P0-B on-media protocol now has concrete source:
`system/kernel/fs/dtfs_intent_v1550.h` holds its six checked
record constants, and `dtfs_intent_validate_v1550.s` contains
the native PDP-6 parser returning CLEAN=0, WRITE_INTENT=1 and
INVALID=-1 after full 36-bit checks and zero padding.
`mkdtfs` keeps its normal DTVOL1 default; only explicit
`--durable-v1550` creates experimental DTVOL2/CLEAN media.

Native p10run 13 actual-image record cases and seven source
mutation controls PASS. Legacy v1520 host and native mount
regressions remain PASS. The parser is **not installed in
production DTFS mount/write paths**, and new-format media is
not supported for mounted operations yet. P0-B OPEN; next
physical write-ahead arming and checked clean unmount. No fixed
KCORE words/RAM or ABI changes. See
`design/kcore-dtfs-intent-source-v1550.md`. VERSION 0.5,
strict 32K/optimization HOLD.

## 2026-11-13 v1549: DTFS durable write-intent design and offline models

P0-B source sweep confirms that DTVOL1 in native reserved physical
RBN0 contains only a 72-bit identity, not a durable WRITE_UNCERTAIN
record. v1531's owner-safe explicit discard and changed-media
replacement cannot protect an old partly damaged stamped DECtape
after the volatile mount table disappears on cold boot. A poison
write *after* a physical partial-write error is not reliable.

Proposed DTVOL2 uses the same reserved RBN0 and checked CLEAN vs
WRITE_INTENT seal, with physical write-and-readback of intent BEFORE
the first data/directory write, then owner-checked clean unmount.
Old DTVOL1 and zero-ID media cannot be silently promoted because
their previous write history is unknowable. 16 offline invariants,
256 simulated partial-seal transitions and five real host `mkdtfs`
image byte tests PASS. **No KCORE runtime/formatter changes yet**:
P0-B remains OPEN; models are not physical PDP-6 acceptance.
See `design/kcore-dtfs-durable-intent-v1549.md`.
VERSION 0.5, strict 32K/optimization HOLD.

## 2026-11-13 v1548: KCORE critical-device P0-A feature accepted

The remaining shared ROOT-first/owned-swap physical test now PASSes.
Full 256K KCC PDP-6 on DRM236 DR0..DR3 starts actual BACKSTORE
owners, then attempts a real D6FS root data read after a cache miss.
The test-only selected descriptor PI2 error goes through the real
driver completion and v1538 root gate to v1515 **ROOT HALT PC
`003743`**, not swap PC `003747`. Physical witnesses: one root
miss arm, one Type-236 PI2 error, five clean pre-fault samples,
and octal `0320` owned BACKSTORE blocks **both at root arm and
after HALT**; mapped controller and D6FS selected-root words
also match. Thirteen unsafe corrupt-source/evidence cases rejected.

**P0-A COMPLETE** for the explicitly accepted critical-service
fail-stop policy and physical/native matrix v1538..v1548. It is
not proof of every naturally possible hardware failure: selected
PI2 error injection differs from physical media removal. No new
production instructions, resident RAM, syscall ABI or storage
format. Next P0-B DTFS durable poisoned-media reinsertion; P1-C
mixed snapshot/swap remains open. See
`design/kcore-p0a-failstop-acceptance-v1548.md`. VERSION 0.5,
strict 32K/optimization HOLD.

## 2026-11-13 v1547: shared physical DRM root and owned BACKSTORE

Full 256K KCC PDP-6 boots `ROOT=drum BOOT=drm`, with selected
D6FS root and mapped BACKSTORE sharing DR0..DR3. A genuine
Type-236 PI2 completion on a live owned swap WRITE is redirected
to the driver's original error path and HALTs at the **BACKSTORE**
PC `003747` (root is `003743`). Physical memory: swap enabled 1,
blocks used `0320`, device map `400000000000`, volume data
`075774`, selected D6FS root `060101000000`; one PI2 error and
four clean pre-fault samples. Nine corrupted-evidence mutants
rejected. No code/RAM change.

P0-A remains open only for shared physical **ROOT-first** loss
with live owned BACKSTORE; v1546 native proves both orders but
cannot replace that guest. See
`design/kcore-shared-drm-root-backstore-v1547.md`, VERSION 0.5,
strict 32K/optimization HOLD.

## 2026-11-13 v1546: shared ROOT/BACKSTORE native failure order

Source-linked PDP-6 D6FS reader, DSK270/DRM236 direct adapter,
BACKSTORE DEVICE/VOLUME gateway and both v1515 HALT leaves on
the same unit PASS 12 cases and five unsafe source mutants.
Root-first vs swap-first failure selects the correct distinct
HALT instruction; disabled-but-owned swap remains fatal and
unowned inactive probes remain nonfatal. Backing owner count
is retained and a second operation never runs after a fatal
first I/O. No KCORE executable or RAM changes.

Remaining P0-A: actual combined full-guest ROOT/BACKSTORE
DEVICE/VOLUME physical failure, shared mapper and live ownership.
See `design/kcore-shared-root-backstore-v1546.md`. VERSION 0.5,
strict 32K/optimization HOLD.

## 2026-11-13 v1545: physical mounted D6FS secondary loss (P0-A)

Full-guest D6FS secondary error isolation now PASSes on both
physical controller classes. With selected root on DRM236,
independently mounted DSK270 D6FS fails its first post-DETACH
file-data READ nonfatally; a separate ROOT file-data read succeeds.
The inverse selected DSK270 root and mounted DRM236 secondary
likewise PASSes after physical DR0..DR3 detach. Each full guest
uses 256K KCC PDP-6, real `dsys_d6fs_mount` public mount ID 2,
host-validated DBOOT/superblock handoff and actual D6FS media.

These guests uncovered a real KINIT adapter ABI bug. MINIT was
patching D6FS secondary DSK direct calls to raw MRES services
which expect `(encoded_address, buffer)` rather than the public
`(unit, sector, buffer)` arguments. Exports 7/8/11/12 now call
the existing fixed `dsk270_read_sector`/`dsk270_write_sector`
encoding bridge, like the already-correct DRM bindings. The
native source guard rejects four wrong KINIT bindings, and the
actual PDP-6 direct DSK encoder test rejects an unsafe raw-MRES
call. Six corrupted physical handoffs are rejected. There is no
additional resident instruction, RAM/data, syscall ABI or media
layout change; only reclaimed KINIT instructions changed.

**P0-A still OPEN** for combined shared-device ROOT/BACKSTORE
DEVICE/VOLUME failure ordering and exact critical owner reason.
See `design/kcore-secondary-d6fs-physical-v1545.md`. VERSION 0.5,
32K/optimization HOLD.

## 2026-11-13 v1544: native secondary D6FS DSK/DRM source isolation

The actual PDP-6 D6FS read/write callers, full physical
DSK270/DRM236 direct adapter and selected-root classifier now
pass 24 integrated native cases/six unsafe source mutations.
Physical kind, unit/block mapping, direct and three-member sets,
secondary error returns and root-only logical I/O counts are
verified. The older v1513/v1515/direct-adapter native regressions
and full KCC fixed-KCORE/D6FS-MRES link PASS. Only outdated
v1513/v1515 policy comments were corrected in production source;
zero executable instruction/data/ABI changes.

Full-guest secondary *mounted* D6FS physical detach remains
OPEN: it needs a genuinely validated transient mount handoff;
the existing syscall has no production userland handoff scanner.
Likewise mixed shared ROOT/BACKSTORE DEVICE/VOLUME physical tests
remain. See `design/kcore-secondary-d6fs-direct-isolation-v1544.md`.
VERSION 0.5, size/optimization HOLD.

## 2026-11-13 v1543: actual D6FS DRM236 root-loss HALT accepted

Full 256K PDP-6 KCC `ROOT=drum BOOT=drm` with four authentic
Type-236 D6FS root members boots successfully. After a checked
PID1 root checkpoint and physical detach of DR0..DR3, a required
root `/SYSTEM/INIT` data read HALTs at exact v1515 root PC
`003743` octal (BACKSTORE would be `003747`). No DAIMOS code,
RAM or syscall changes were required. The matching Type-270
disk-root physical test v1540 also PASSes.

P0-A still needs secondary DSK270/DRM236 real failed-I/O negative
controls and shared root/BACKSTORE DEVICE/VOLUME fault coverage.
See `design/kcore-drm-root-outage-v1543.md`, VERSION 0.5,
optimization/strict 32K HOLD.

## 2026-11-13 v1542: physical Type-236 BACKSTORE HALT accepted (P0-A)

Full 256K PDP-6 KCC disk-root F07 guest PASSes a selected real
Type-236 PI2 error completion on the first enabled, already-owned
BACKSTORE WRITE. The fixed v1541 completion gate HALTs at octal
PC 003747, exactly v1515's distinct BACKSTORE reason-2 HALT
(linked fatal entry 003740; root HALT is 003743). Physical
post-HALT native counters: PI2 error 1, BACKSTORE enabled 1,
owned blocks 0320 octal. Four sampled clean pre-error intervals,
nine corrupt-evidence verifier mutations rejected. The old v1511/
v1512 post-negative scheduler retry is intentionally not allowed.
Actual controller error completion, not simulated direct -1.
See design/kcore-backstore-physical-failstop-v1542.md.

P0-A remains partly open for alternate DRM root, negative
secondary DSK/DRM and shared DEVICE/VOLUME root/BACKSTORE fault
profiles. No KCORE code/RAM/ABI change in v1542. KCORE feature
completion only, VERSION 0.5, strict 32K/optimization HOLD.

## 2026-11-13 v1541: BACKSTORE failed service is a terminal event (P0-A)

The BACKSTORE policy now matches the already accepted v1538 ROOT
policy: after an actual required transfer's bounded physical-driver
recovery returns failure, the machine HALTs. `fs_mres.s`'s sole
`backing_region_io` completion routes valid negative direct DSK270/
DRM236 DEVICE and BLOCKSET VOLUME transfers to fixed v1515 with
AC1=2, reaching the distinct BACKSTORE-specific HALT. A successful
transfer, zero count or invalid request is never promoted to HALT.
An inactive **unowned** region still returns ordinary failure to
optional KINIT snapshot probing. Disabling new swap allocations
does not absolve already allocated DISK owners: a failed transfer
with nonzero `backstore_blocks_used` remains fatal.

Native source-linked PDP-6 tests PASS 17 positive/negative cases
and six unsafe mutations; the full KCC tape-root fixed KCORE and
optional FS MRES link PASS. No new permanent DATA/BSS or ABI.
This deliberately supersedes the old v1511/v1512 policy of returning
an exhausted BACKSTORE I/O failure to the scheduler for later retry;
pre-error driver recovery remains. No process backing owner can be
freed or reconstructed on this fatal path.

Physical positive Type-236 PI2 error/HALT acceptance is a separate
full-guest gate; see `design/kcore-backstore-service-failstop-v1541.md`.
Current accepted root physical positives include TSFS three-member
v1538, single-member v1539 and D6FS disk v1540 (testkit). P0-A
remains OPEN for BACKSTORE positive physical fault and DRM-root/
secondary-DSK/DRM mixed hardware profiles. VERSION 0.5;
strict 32K and optimization HOLD.

## 2026-11-13 v1539: physically lost single TSFS root member (P0-A)

The same full 256K KCC PDP-6 image now PASSes a root-outage
variant that detaches only physical **DTC0** of its selected
three-member TSFS root while DTC1 and DTC2 stay attached. After
the guest attempts a real uncached `/SYSTEM/INIT` root-file
data read, it HALTs at the exact linked root-specific v1515
PC `003743` (octal), rather than returning an error or halting
as a user process at PC zero. This is a real Type-551/Type-136
SIMH media-detach acceptance, not a permanent-loss status bit.

The all-members-detached root HALT and independent nonroot DTC3
fault/return under healthy DTC4/MEMFS were accepted in v1538.
Remaining P0-A: D6FS disk/drum root physical outage and the
distinct BACKSTORE critical-unavailability policy and acceptance.
No production KCORE/MRES code or resident state changes in v1539;
VERSION 0.5, strict 32K and optimization HOLD. Testkit:
`tests/validation/daimos-root-member-outage-v1539.md`.

## 2026-11-13 v1538: selected-root I/O fail-stop after bounded error (P0-A)

The P0-A root policy is intentionally simplified: an actual negative
root-provider backing read/write after its normal bounded controller
recovery is a fatal inability to complete essential root I/O. No
positive proof of physically permanent device absence is required.
Secondary filesystem errors retain normal nonfatal mount/FILE/cache
handling. BACKSTORE remains a separate critical failure case and
its previously accepted recoverable PI/timeout rollback is unchanged.

Fixed `kcore_root_fs_io_failure_v1538` compares the failed provider
(D6FS 6 or TSFS 7) and public mount ID against the selected
`vfs_namespace_root`, stopping at v1515's root-specific HALT only
on an exact match. Actual D6FS read and write-through failures and
TSFS FILE/EXTENT metadata, STORED and BLOCK data reads are routed.
Logical invalid arguments, missing records, EOF, MM/decoder failure,
unpublished root and secondary mounts must not invoke fatal stop.
No new permanent storage state; the fixed gate is ten instructions.

Source-linked real PDP-6 native D6FS (11 cases, five unsafe mutants)
and TSFS (12 cases, four unsafe mutants) PASS, as do existing
v1513/v1515/v1516/v1517/v1537 and TSFS slab/multiextent controls.
Full KCC tape-root and disk-root fixed KCORE plus optional provider
MRES links PASS. A full tape-root image also builds successfully.

The current-source full 256K KCC physical TSFS-root guest PASSes
real DTC0/DTC1/DTC2 detachment followed by a root `/SYSTEM/INIT`
data read: the PDP-6 stops at exact linked root HALT PC `003743`.
A separate v1534 physical guest PASSes genuine failed DTC3 WRITE
without stopping the TSFS root, DTC4 native DTFS or writable MEMFS.

**P0-A remains partly OPEN:** physical disk/drum D6FS-root outage,
single-member TSFS-root loss, and the independent
BACKSTORE-unrecoverability policy/physical acceptance. See
`design/kcore-root-io-failstop-v1538.md`. No general optimization;
VERSION 0.5, strict 32K HOLD.

## 2026-11-13 v1537: guarded TSFS root-member terminal dispatch (partial P0-A)

Optional TSFS provider now has the five-instruction
`tsfs_confirmed_root_member_loss_v1537` entry. For an externally
**authoritatively confirmed permanent** loss event, it checks the
exact active physical TSFS root-set member mask through v1516;
only a real member reaches fixed v1515's root HALT. Unrelated DTC
units, a non-TSFS root and invalid root geometry return without
HALT. Actual PDP-6 native eight-case source-linked tests and four
unsafe source mutations PASS; v1515/v1516/v1517 regressions PASS.
Full KCC fixed KCORE and optional DTFS/TSFS MRES links PASS.
Controlled identical-object MRES relink: +5 TEXT words, zero BSS;
fixed KCORE unchanged.

**P0-A remains OPEN:** no original Type-551/Type-136, DSK270 or
DRM236 driver yet provides authoritative permanent physical-loss
evidence. The new trusted entry deliberately has no production
caller. Generic PI, illegal select and watchdog errors remain
recoverable and never imply confirmed loss. DSK/DRM/BACKSTORE
critical owner routing and physical loss/negative acceptance are
outstanding. See `design/kcore-critical-loss-dispatch-v1537.md`.
VERSION 0.5; strict 32K and optimization HOLD.

## 2026-11-13 v1536: snapshot/swap BACKSTORE separation accepted (partial P1-C)

The current-source 128K full KCC PDP-6 MEMFS PERSIST two-boot
v1477 guest PASSes exact data/XIP cold restore, both orderly
HALTs and unchanged AUXSTORE descriptor geometry.

The *actual* KINIT `proc_swap_boot.c` reservation code is
compiled with a host-only backing-header/MM adapter: nine
v1/v2/invalid/offline reservation cases PASS, exact all-bit
bitmap and accounting checks PASS, and four deliberately
unsafe production-source mutants are rejected. Separate
native PDP-6 tests directly execute the actual KCORE
`backstore_alloc`, `backstore_verify_allocated_v795`
and `backstore_free` instructions: a snapshot owns blocks
107..144, an old process owns block 0, and process-owned
allocations of 100 plus 6 blocks start at 1 and 101,
never steal the snapshot. Invalid overlapping free,
duplicate free, overlarge allocation and three unsafe
assembly mutants are rejected. No production code/RAM
changes. See `design/kcore-snapshot-swap-reservation-v1536.md`.

This closes the static reservation/native ownership portion
of P1-C, **not** one concurrent physical guest with actual
swapped-process restoration during MEMFS snapshot commit
and combined DTC/DSK/DRM I/O failure. That remains OPEN.
VERSION 0.5; strict 32K/optimization HOLD.

## 2026-11-13 v1535: physical media gate CTY delivery repaired

A newer v1527 DTC3 replacement/return physical rerun had stalled
after a READY_* checkpoint without reporting an FSINFO error.
The standalone test transport delivered `CONT` and `A\r`
back-to-back on SIMH's monitor stdin; the console keyboard
poll is a different input consumer. A v1535 guest witness
confirmed a missing acknowledgement rather than a negative
DTFS provider return. A trial with premature SIMH queued
input also hung; `SHOW SEND` had drained while PID1
was still blocked in READCHAR.

The physical testkit now queues a simulated CTY keystroke
with `SEND AFTER=200000,DELAY=20000,"A\r"` **before**
`CONT`. Both delays are based on the actual simulator CTY
10000-instruction/character hardware model, allowing the
READY_* CR/LF to finish before attempting the guest read.
No host sleeps, extra production kernel logic, retries,
different media identity policy or relaxed checks.

Full 256K PDP-6 v1527 original/different/returned physical
media gate now PASSes on repeated independent runs, with
the original held FILE and DTC4 isolation intact. v1528
physical WRITE_UNCERTAIN, v1531 owner-checked administrative
replacement, and v1534 mixed-provider failure all PASS with
the same revised test transport. The v1529 actual partially
written Type-551 block also PASSes with the test-only r5
simulator: exactly one requested new 36-bit data word changed,
the other remained old; sticky refusal after return confirmed.
Only testkit code changed; KCORE and MRES words unchanged.

Details: `design/kcore-physical-console-ack-v1535.md`.
VERSION 0.5; strict 32K/optimization HOLD.

## 2026-11-13 v1534: mixed TSFS/DTFS/MEMFS physical WRITE fault PASS

Full 256K KCC PDP-6 KCORE/loaded MRES guest **PASS** with
simultaneous root TSFS, stamped writable DTC3 and DTC4
native DTFS, and independently writable MEMFS.
PID1 verifies exact two-word DTC4 and three-word MEMFS
data before failure. It warms DTC3's directory **last**
to ensure physically DETACHing DTC3 causes a real native
metadata WRITE failure (not merely a preceding read error).

While poisoned DTC3 refuses FSINFO/CLOSE/UNMOUNT,
real TSFS root, DTC4 native file data, MEMFS RAM data
and dynamic FSINFO count continue to pass. Physically
reattaching the same DTC3 cannot clear WRITE_UNCERTAIN:
FSINFO/STAT/CLOSE still refuse. A subsequent healthy
DTC4 CHMOD performs a real successful metadata WRITE.
Both healthy secondary mounts then retire normally;
DTC3's original FILE/slot remain quarantined. Guest
`MIXEDIO V1534 PASS` and host runner PASS.

The first attempted mixed test suffered a *read* failure
because its DTC4 precheck displaced shared directory
cache; the final ordered experiment proves the desired
failed WRITE. A stale native v1522 source extractor was
updated for the v1531 replacement function and its
original eight unsafe mutations again reject on PDP-6.
No production KCORE/MRES or permanent RAM changes.

**P1-C partial physical provider-isolation gate CLOSED**;
mixed DSK/DRM/DTC mapper/backing, LOGSTORE snapshot
plus swap concurrency and rollback remain OPEN.
See `design/kcore-mixed-storage-physical-v1534.md`.
VERSION 0.5; strict 32K/optimization HOLD.

## 2026-11-13 v1533: physical mount ID 5 and stable holes PASS

Real 256K PDP-6 KCORE/PID1 mounts four *distinct* native
72-bit-stamped physical DTFS volumes on DTC3-DTC6, obtaining
public mount IDs 2, 3, 4, **5** in the KINIT seven-slot slab.
FSINFO verifies every provider and pathname; two upper slots
remain empty. Unmounting the middle ID 3 does not renumber
or damage high ID 5. The other three mounts retire normally
and all nonroot slots become empty. Physical guest PASS;
v1532 FSINFO physical regression PASS on the same harness.
No KCORE/MRES production code or RAM change.

P1-D actual high slot accepted; physical 26-volume maximum
is not implied. See `design/kcore-fsinfo-high-physical-v1533.md`.
VERSION 0.5; 32K/optimization HOLD.

## 2026-11-13 v1532: runtime FSINFO capacity physical PASS

Previously uncommitted v1492 FSINFO capacity syscall and MOUNTS/DF
consumers now pass native PDP-6 tests: AC2=-1 queries the live
KINIT-published `vfs_mount_limit` without mapping a user pointer;
ordinary entries distinguish occupied, empty and invalid/provider
error. Exact user command loops cover 1/4/8/17/26 slots and
provider failures. The broken-capacity source mutation is rejected.

Fresh 256K KCC PDP-6 TSFS-root physical guest PASS: KINIT publishes
**7** slots from its actual root/DTC/MEMFS inventory; PID1 checks
the TSFS root, mounts real DTC3 native DTFS, scans slots 0..6
including empty holes 2..6, rejects out-of-range indices,
then unmounts DTC3 and rechecks the stable capacity/root.
`FSINFOCAP V1532 CAPACITY7`, `SCANNED7`, `RETIRED`, `PASS`.
The physical test does not claim 26 actual mounted volumes or
occupation beyond slot 4; those remain P1-D gates.

Details: `design/kcore-fsinfo-runtime-capacity-v1532.md`.
VERSION 0.5, strict 32K/optimization HOLD.

## 2026-11-13 v1531: root-only poisoned-DTFS replacement PASS

Full rebuilt 256K PDP-6 KCORE/DTFS/TSFS + real
Type-551 DTC3 physical guest **PASS**:
root-only UUO 077 extension 054 permits explicit
DROP_FD for one WRITE_UNCERTAIN native DTFS FILE
without claiming SYNC success. An ordinary CLOSE
still refuses; an outstanding cwd or any remote
FILE/PIPE/swap reference prevents slot retirement.
After a real write failure, return of the original
same stamped 72-bit-ID DTC3 is **rejected**, even
after all owners are released. Only a physically
verified *different* stamped DTC3 volume with
valid native directory/RBN0 can authorize
REPLACE_SLOT. The old slot retires without
syncing damaged data, the new medium mounts
fresh, old filename is absent, and TSFS root
remains available. Guest PASS.

Native source-linked PDP-6 test PASS checks
actual stable-uarea remote/swapped FD15 and
cwd references, PIPE/swap exclusion, failed
physical verification and remote-open race
during sleeping provider lease. Four executed
unsafe mutations are rejected. The initial
native oracle false pass was corrected by
seeding its failure witness before first
assertion. v1526-v1529 physical regressions
PASS on the new KCORE, and v1514/v1523
native legacy source windows were corrected
to exclude the unrelated v1531 implementation.

Full DXR versus accepted v1527: fixed KCORE
`023116` -> `023261` (+99 decimal text/image
words), BSS `000415` unchanged. Optional
DTFS/TSFS MRES `003747` -> `004057`
(+72 words), BSS `000002` unchanged.
No additional persistent data allocation.
The feature is deliberately destructive
*replacement*, not in-place filesystem repair
or forced cleanup of another process.
**LIMITATION:** v1531 does not persist a
quarantine blacklist: deliberate reinsertion
of the old tape (or reboot with its volatile
poison lost) can permit a later normal
MOUNT if its format/ID survived the failed
write. Operationally quarantine that tape
offline; durable cross-reboot refusal is
a separately OPEN P0-B integrity gate.
Detail: `design/kcore-dtfs-admin-replace-v1531.md`.
VERSION 0.5; strict 32K and optimization HOLD.

## 2026-11-13 v1529: actual partially written physical DTC3 block PASS

Full isolated 256K PDP-6 DTC-root guest, unchanged
production v1527 KCORE + installed DTFS/TSFS MRES:
new FILE allocated and seeded with two words on
native DTC3 RBN0145. A versioned, compile-time
test-only Type-551 fault interrupts the **second
changed word** of a real replacement transfer.
Host DETACH flushes the actual medium and verifies
the complete physical 128-word block: exactly
one requested word changed to its new value;
the other remains old, header and unrelated
words unaffected. No mocked provider or device
callback. Guest reports WRITE_FAILED,
STICKY_CONFIRMED and PASS.

Even on return of the **same damaged** 72-bit-ID
volume, FSINFO/STAT/CLOSE/UNMOUNT remain refused
and original FILE ownership retained. Independent
TSFS root continues serving. Simulator was built
with full normal platform flags plus the opt-in
v1529 diagnostic; prior bad-CC experimental
builds were discarded. v1528 one-word guest
control, independent RBN0145 physical read/write,
native empty-file resize and v1527 different/same
volume return regression PASSed.

P0-B actual partial-write quarantine is now
physically accepted; **explicit checked
administrative repair/discard and owner retirement
remain OPEN**. Production RAM and code delta
zero; MRES text octal 003747, BSS 000002.
Details: `design/kcore-dtfs-partial-write-physical-v1529.md`.
VERSION 0.5, strict 32K HOLD, no optimization.

## 2026-11-13 v1530: independent physical-test media accepted

The v1529 actual mixed-old/new physical DTFS block
test exposed an unrelated fixture error: two test
jobs sharing one prebuilt KCC image could overwrite
the same DTC3 tape and SIMH do-file. The corrected
runner now keeps **all** DTC0-DTC3 writable images,
the optional DTC4 and replacement volumes, and the
headless SIMH command file private to each run.
No shared prebuilt boot image or DTC tape is modified.

The physical v1529 partial-write/WRITE_UNCERTAIN,
v1528 failed-write quarantine, v1527 cross-volume
72-bit-ID return/DTC4 isolation and v1526 remote
FILE/cwd ownership tests all PASS again using the
isolated fixture. This is test-harness correctness,
not a KCORE/MRES or on-media-format modification.
P0-B checked administrative recovery remains OPEN;
VERSION 0.5, zero RAM growth and strict 32K HOLD.
Details: `design/kcore-physical-test-media-isolation-v1530.md`.

## 2026-11-13 v1528: physical failed-write quarantine accepted

Real PDP-6 256K DTC-root KCORE/PID1 with unchanged
v1527 DTFS/TSFS MRES **PASS**: mount stamped DTC3
read/write, create and write a file, commit real
metadata, DETACH DTC3 at a synchronized SIMH stop,
and attempt a further metadata write, which fails.
After the original identically stamped DTC3 returns,
FSINFO and STAT still fail. FILE CLOSE fails twice
without losing ownership; ordinary UNMOUNT refuses.
The guest reports `DTFSWRITE V1528 WRITE_FAILED`,
`WRITE_STICKY`, and `PASS`. An independent repeat
with Type-551 CONO trace also PASSes and confirms
real DTC3 write commands.

The existing production `WRITE_UNCERTAIN` state
correctly persists across matching 72-bit volume
return: automatic revalidation cannot certify
metadata/data following a failed physical write.
No production source change, RAM or MRES growth.
This is a genuine *failed write* but not yet
a physically interrupted **partial-block** write;
that and checked administrative cleanup remain
P0-B OPEN. File close's refusal and retaining the
original mount are deliberate fail-closed behavior.
Detail: `design/kcore-dtfs-write-uncertain-physical-v1528.md`.
VERSION 0.5; strict 32K HOLD, no optimization.

## 2026-11-13 v1527: physical DTC3 replacement/return and DTC4 isolation PASS

**Actual 256K PDP-6 DTC-root KCORE** with the
installed optional DTFS/TSFS MRES now PASSes
physical native read-failure/volume-return tests:
mount distinct stamped DTC3 and DTC4 volumes;
retain a DTC3 FILE; physically DETACH only DTC3;
refuse stale DTC3 FSINFO and FILE after real
read failure while DTC4 stays available;
ATTACH a different stamped DTC3 image and
reject its full 72-bit identity without losing
DTC4; DETACH replacement and ATTACH the
original DTC3 image; verify exact RBN0 ID,
restore the original still-open FILE, and
unmount both cleanly. Real guest reports
`RETURN_VERIFIED` and `PASS`; rerun of the
v1526 cross-process physical gate also PASSes.

**Assembler/BCACHE correctness bug fixed:**
`dtfs_identity_revalidate_v1522` read physical
RBN0 directly into `fs_block_workspace`
without first invalidating BCACHE slot 0's
workspace tag. That caused false TSFS metadata
cache hits after a rejected DTC3 replacement,
making unrelated DTC4 FSINFO/STAT fail even
though the MM mount and provider lease were
healthy. Add exactly one `PUSHJ` to the
existing `bcache_workspace_invalidate` leaf
before the raw RBN0 read. Keep the independent
BCACHE slot 1; no global flush or workaround.

The v1522 native fixture's duplicate BCACHE
variables were corrected so the actual cache
BSS/tag is now tested. The new executable
stale-tag source mutation is rejected,
alongside the old seven v1522 mutations.
Full KCC image PASS; optional MRES text
octal **003746 -> 003747** (+1 word),
BSS unchanged **000002**; no fixed KCORE
instruction or new persistent RAM words.
VERSION remains 0.5; 32K HOLD, no optimization.
Details: `design/kcore-dtfs-physical-return-v1527.md`.

**OPEN:** P0-A authoritative permanent
TSFS root/BACKSTORE loss evidence with
qualified HALT; remaining P0-B actual
partial physical write / WRITE_UNCERTAIN
and checked administrative recovery;
P1-C mixed-provider faults, P1-D dynamic
FSINFO capacity, P1-E swapless PURE/MM/APR
and final feature freeze.

## 2026-11-13 v1526: physical multi-process DTFS owner acceptance

Full real 256K PDP-6 DTC-root KCORE + installed
DTFS/TSFS MRES guest **PASS**. PID1 mounts
stamped DTC3 at `/MOUNT/DT0`, then RUNs an
independent child with two actual kernel pipes.
The child alone holds cwd on DTFS: PID1's UNMOUNT
refuses, FSINFO keeps provider 5. The child
leaves cwd but retains a real open FILE on DTFS:
PID1's UNMOUNT again refuses. After the child
closes FILE but remains alive, PID1 verifies
its live non-zombie PROCINFO, successfully
unmounts and verifies FSINFO slot empty.
Child then exits and is reaped through WAIT.
Two real guest runs PASS, including the stricter
PID/PROCINFO verification; no mock VFS/driver.

**P0-B cross-process cwd/FILE ownership is now
physically accepted.** Source needed no
additional KCORE or MRES change, zero extra
resident RAM. Open P0-B: physical DTC3 read
error, same/different 72-bit ID return, sticky
write uncertainty and administrative recovery.
P0-A permanent critical loss and P1 work remain.
Detailed test evidence:
`design/kcore-dtfs-cross-process-v1526.md`
and daimos-testkit v1526 report.

## 2026-11-13 v1525: actual physical mounted DTFS cwd/FILE acceptance

**P0-B real 256K PDP-6 guest PASS**, using a DTC-selected
TSFS root with the optional DTFS MRES genuinely LOADED.
The real target PID1 process mounted a separately attached,
host-stamped native DTC3 (both 36-bit volume-ID halves and
checksum validated), then saw FSINFO provider 5
at `/MOUNT/DT0`, checked STAT, entered that cwd and
observed a refused busy-cwd UNMOUNT. After opening a real
directory FILE on that mount and leaving cwd, UNMOUNT still
refused because the FILE remained open. Closing the FILE
permitted final UNMOUNT, and FSINFO correctly showed the
mount slot empty. This is actual kernel, MRES, VFS,
PDP-6 and Type-551/Type-136 physical I/O, not a mock
or a STAT of an existing TSFS mountpoint. The disposable
test root launches its compact actual KCC test executable
as PID1 to avoid multiple long DTC0 root EXEC seeks.

Two *real production correctness bugs* found and repaired:
(1) native directory owner for RBN0144 lives at C dir[14],
which is assembly octal `+016`, not old `+014`;
(2) TSFS `tsfs_parent` and `tsfs_parent_name` must test
negative helper return only: a successful parent vnode
is **nonzero**. The latter bug had caused both FSINFO
path reconstruction and GETCWD to fail across the
mounted DTFS / TSFS boundary. Native source-linked
v1519 mount-admission fixture now uses the correct index
and rejects the original wrong-index mutant; new v1525
TSFS parent source-linked test rejects both old `JUMPN`
mutations. Existing v1522/v1523 boundaries remain.

The old installed `mkdtfs` had been stale and emitted
legacy zero identity. Current source rebuilt as
`mkdtfs-v1525` from versioned `local.tar.gz`, installed
in `PDP10_PREFIX`, now produces the full 72-bit stamp.
The physical acceptance test verifies the entire
stamped ID and C dir[14] owner *before* booting.
Rebuilt full KCC image and optional DTFS/TSFS MRES
PASS. Controlled optional MRES size unchanged:
octal **003746 text, 000002 BSS**. **Zero new resident
instructions and persistent data words.** VERSION 0.5;
no optimization or strict 32K acceptance (HOLD).

**Still OPEN:** P0-A authoritative permanent critical
root/BACKSTORE/selected TSFS-member loss and qualified
v1515 HALT; remaining P0-B real cross-process FILE/cwd
ownership, read error/same-volume 72-bit reattach,
different-volume rejection, sticky dirty/partial-write
WRITE_UNCERTAIN administrative recovery; P1-C mixed
storage faults, P1-D remaining dynamic FSINFO inventory
validation, P1-E uncompressed swapless PURE/MM/APR
integration and feature freeze. Complete evidence:
`design/kcore-dtfs-physical-mount-v1525.md`.

## 2026-11-12 v1524: Type-551 physical RBN0 now works; P0-B guest still open

The v1523 tape-root guest was able to boot and enter DSH, but
formatting/checking the secondary DTC3 volume did not finish.
We isolated a concrete simulator defect below KCORE:
the newly selected stopped Type-551 transport kept `DTB_REQ`
through its first forward block mark, so `FNC_SRCH` failed
to send the first RBN0 to the prearmed Type-136 channel.
Original SIMH physically failed one-block DTC3 RBN0 writes;
ordinary RBN1 and RBN0144 passed. `sims` v1524 activates
the pending search *before* publishing that first mark.
The exact same production tape driver now physically writes,
reads and compares all three blocks. The previous simulator
still fails the actual native RBN0 negative-control test.
Native stream, reverse, select, watchdog, v1522 identity
and v1523 FILE/cwd regressions also PASS.

No KCORE code, on-media DTFS stamp, permanent RAM word or
instruction was modified to accommodate this simulator defect.
Corrected `pdp6-v1524` is installed under `PDP10_PREFIX`
from a versioned `local.tar.gz`, retaining `pdp6-v1470`
for comparison. Full tape-root DSH `MKFS.DTFS` still showed
heavy DTC0 root-media traffic, without yet reaching DTC3.
An isolated 256K KCC disk-selected auto-root guest now
**physically formats DTC3 and reports FSCK.DTFS NATIVE OK**.
But its optional DTFS/TSFS MRES is deliberately *not installed*
by KINIT when the root is DSK; its MOUNT.DTFS is refused.
The old v578 guest script falsely treated the pre-existing
D6FS mountpoint as a successful DTFS mount. Its checks now
reject MOUNT/UNMOUNT command errors and require actual
FSINFO mount-table publication and subsequent retirement.
A freshly regenerated disk-root guest then PASSED the
explicit negative-policy oracle: MKFS/FSCK OK, expected
MOUNT.DTFS refusal and an inventory with D6FS/MEMFS
but no DTFS.
No true booted DTFS mount/FILE/dirty-recovery acceptance yet.

**KCORE features still OPEN:** P0-A authoritative permanent
root/BACKSTORE/selected TSFS member loss and v1515 HALT;
P0-B physically mounted non-root DTFS full open-FILE,
replaced-media and dirty/partial-write lifetime/recovery;
P1-C genuinely untested mixed device/LOGSTORE/MEMFS/swap
fault interactions; P1-D integrate and physically test
dynamic FSINFO capacity/MOUNTS/DF; P1-E integrate uncompressed
swapless PURE drop and APR/MM restore; final matched feature
acceptance. Mutable-only RAM compression remains strictly
conditional on a measured *net* peak-RAM reduction. No
optimization work yet; strict 32K stays HOLD. Full evidence:
`design/kcore-dtc-first-mark-v1524.md`.

## 2026-11-12 v1523: mounted FILE/cwd lifetime and unmount refusal

P0-B feature correctness: `vfs_unmount` must not retire a public
mount ID while any live process retains a FILE vnode or current
working directory on that mount. The new fixed KCORE
`file_mount_busy_v1523` scans actual stable u-areas for all
`proc_high_slot` processes (including swapped processes), using
the existing three-word process descriptor and all 16 FILE slots.
No new owner table, bitmap, reference count, per-process field or
dynamic allocation is introduced. The helper preserves the public
mount ID across PIPE, SWAP and FILE checks, correcting the old
AC2-clobber dependency, and performs a second owner check after
potentially sleeping FS SYNC/PREPARE, before committing retirement.
Failures preserve old mount and FILE/cwd ownership.

Source-linked native PDP-6 `daimos-file-mount-busy-v1523.sh`
PASS with seven rejected unsafe mutations: local/remote FILE,
swapped process FD15, remote cwd, proper owner ID, failed SYNC,
concurrent open during SYNC, and final clean retirement. Prior
v1514 error/return unmount test PASS. An isolated 256K KCC
tape-root image assembles and links; controlled identical-link
fixed KCORE growth: **+54 instruction words, zero additional
data words**. Documentation:
`design/kcore-file-mount-lifetime-v1523.md`.

**Not yet a full physical P0-B acceptance.** The additional
booted tape-root fixture reaches DSH but a real DTC3 formatting
or validation operation has stalled without a successful
secondary mount. This is not evidence of open-FILE physical
recovery; the source-linked test only establishes correct
process/mount ownership. P0-A reliable permanent media-loss
classification is still OPEN. Strict 32K and optimization
remain excluded.

## 2026-11-12 v1522: non-root DTFS mounted-ID fail-closed return (native)

The first P0-B increment is installed: a negative raw DTFS read
marks exactly the still-mounted DTFS roots sharing its actual DTC
unit with READ_SUSPECT in a spare bit of the existing MM media word.
Negative physical writes mark WRITE_UNCERTAIN, a distinct *sticky*
condition, because partially written data is not repaired merely
by verifying the same medium. The existing shared BCACHE and
directory invalidations remain. On subsequent `dtfs_load`, before
using old directory/FILE/vnode state or a cached hit, READ_SUSPECT
forces a physical reserved RBN0 identity read, full native v1520
record validation and comparison of **both** retained 36-bit ID
halves. Only an exact stamped same-volume match clears the read
bit and allows reloading the directory. A changed/invalid/legacy
identity, I/O failure, or WRITE_UNCERTAIN refuses reuse without
retiring VFS/FILE ownership. DTFS FSINFO and SYNC share this
checked path. Other media units and providers remain unaffected.

The real source-linked PDP-6 + BCACHE v1522 suite PASSES,
including **seven executed unsafe mutants** and actual raw-DTC
AC6 clobber. v1490/v1518/v1519/v1520 DTFS, BCACHE and v1516
TSFS root-member regressions PASS. A clean isolated 256K KCC
tape-root KCORE/DTFS+TSFS MRES link PASSES. Controlled same-map,
same-companion fold/relink measures **+86 optional MRES TEXT
words, +0 static/data/fixed KCORE or MM mount words**. The
separate strict single-root residency gate still fails and
32K remains HOLD. This is native, not a full guest test.

P0-B is **still open** for full booted open-FILE/dirty-state
fault/return/replacement and administrative cleanup. Silent
exchange without observed error remains undetectable, and
WRITE_UNCERTAIN cannot be auto-cleared. P0-A authoritative
Type-551/root/BACKSTORE permanent-loss detection and actual
v1515 fatal caller also remain OPEN: ordinary PI/status/error
or watchdog is never a root-loss signal. Technical report:
`design/kcore-dtfs-mounted-identity-v1522.md`. Testkit:
`tests/validation/daimos-dtfs-mounted-id-v1522.md`.

## 2026-11-12 v1521: current KCORE feature-only checklist (source audited)

**Authoritative current list:** `design/kcore-feature-source-audit-v1521.md`;
the historical dated sections below are evidence at their own revision,
not an additive list of unfinished features. Committed baseline v1520
(`65ed0b5b`), with separate uncommitted v1492 FSINFO changes. Fresh
KCC kernel source-to-ASM build and v1492 source-linked native FSINFO /
actual command-body tests PASS; **no new full physical guest claim**.

1. **P0 critical-device loss:** prove trustworthy permanent boot-root or
   BACKSTORE absence, qualify selected TSFS root-member loss, and route
   confirmed loss to the implemented v1515 no-return HALT leaf. A DTC
   select error, PI error or watchdog alone is not a reliable loss signal.
2. **P0 mounted non-root media:** compare the saved v1520 DTFS 72-bit
   identity after observed failure/return *before* cached or open-handle
   reuse; safely handle replacement, zero-ID legacy media, dirty writes,
   mount ownership and independent root operation. Existing mount-time
   format validation is not a replacement-recovery protocol.
3. **P1 mixed storage:** close only genuinely unaccepted DEVICE/VOLUME,
   BCACHE, physical provider and LOGSTORE/MEMFS/swap concurrency fault
   combinations; preserve existing allocators, shared cache and v1484/
   v1485/v1490/v1511-v1514 accepted branches.
4. **P1 FSINFO:** integrate/commit locally modified v1492 capacity
   query and MOUNTS/DF scans; native 1/4/8/26-slot source tests PASS,
   but a linked physical multi-mount guest and failure/holes are OPEN.
5. **P1 swapless PURE:** integrate the uncompressed inactive-owner
   drop/MM rebase and complete APR restore into real scheduler/VM
   life cycles with fault-atomic ownership; isolated v1381-v1397
   experiments are not production-linked.
6. **Conditional:** only after uncompressed PURE acceptance, decide
   on optional mutable-only RAM compression using *net peak RAM*.
   "No compression" is valid; compressed disk-swap is cancelled.
7. **Feature freeze:** run matched affected production guest/native
   matrix and reconcile docs. No size-only KCORE optimization yet.

**Already accepted:** grouped block codec, restricted MEMFS XIP and
snapshots, defined-profile EXEC/RUN/OOM, normal backed swap, repaired
BACKSTORE TERM/finalize, bounded failure retry and independent resident/
swapped progress, physical DRM PI errors and missing completions, native
DTFS mount/identity groundwork, 48K non-XIP boot and dynamic KINIT
mount slab. Do not reopen these via older unchecked boxes.
Strict 32K work is **on HOLD**; MRES unload/reload, separate D6FS
packer, unified TSFS and general userland features are out of scope.
DAIMOS VERSION remains 0.5.

## 2026-11-12 v1520: native DTFS volume identity stamp and mount-admission PASS

Native DTFS uses reserved physical block 0 for an optional `DTVOL1`
record: two random 36-bit halves (72-bit ID), complement checksum and
zero padding; existing native directory/block layout stays unchanged.
Official host `mkdtfs` draws nine bytes from `/dev/urandom` and
fails if unavailable. Userspace DTFSPROBE and the actual PDP-6
kernel DTFS mount now physically validate the record; legacy tapes
with an all-zero block remain accepted but have **no verified ID**.
The two identity halves are retained in the existing TSFS-only
shape/extent MM slab words of a DTFS mount, requiring **zero new
resident data words**. Target `MKFS.DTFS` clears any old block-0
stamp before rewriting a native directory and, lacking trusted
randomness, intentionally produces legacy rather than fake IDs.
Host/target C source-linked tests and native PDP-6 source-linked
mount/ID tests PASS, including five executed negative mutations.
Older DTFS/BCACHE/TSFS regressions and isolated KCC/GCC tape-root
links PASS; controlled +46 optional MRES instruction words, +0
permanent data and fixed KCORE. The independent strict 32K KCORE
size gate still FAILS (`023241` octal >= `020000`). This is
**identity storage and mount validation, NOT online replacement
recovery**: mounted open FILEs and cached hits still require a
checked post-error identity transition. See
`design/kcore-dtfs-volume-id-v1520.md`.

## 2026-11-12 v1519: physical native DTFS mount-admission verification PASS

`dtfs_mount_unit` previously trusted the short-lived userspace
format probe: a direct mount syscall could publish a native DTFS
mount without validating its current physical tape. The optional
DTFS provider now invalidates shared clean cache tags/directory
identity and performs a **raw physical** read of native directory
block 0144 before VFS mount publication. It refuses failed I/O,
wrong native magic, missing reserved owners for blocks 0/0144,
or incorrect three terminal native allocation tags. Native
PDP-6 source-linked mount admission plus six executed unsafe
mutants PASS. Existing DTFS/BCACHE regression tests PASS.
Isolated GCC lowmem 32K and KCC full 256K tape-root KINIT/MRES
links PASS. Controlled folded size: **+27 optional tape MRES
instructions, +0 data, +0 fixed KCORE words**. The independent
strict 32K fixed KCORE size gate still FAILS `023241` octal
against `<020000`. Format validation cannot distinguish two
separately formatted native tapes: persistent volume identity,
checked replacement/open FILE handling and actual tape-removal
signaling remain OPEN. Details:
`design/kcore-dtfs-native-mount-validation-v1519.md`.

## 2026-11-12 v1518: failed DTFS I/O invalidates shared cached identities

Corrected optional DTFS DTC read/write veneers: after a *negative
physical read*, clear the shared directory's cached mount ID and
invalidate both clean BCACHE tags, preserving the original I/O error.
Writes already invalidated clean tags before physical transfer; now
they also clear the directory identity if the physical write fails.
Successful reads/writes keep their existing return and directory
semantics. Native PDP-6 source-linked DTFS directory loader, actual
BCACHE and DTC wrappers PASS; four executed unsafe source mutations
rejected, plus existing DTFS/BCACHE/TSFS regressions PASS. Isolated
KCC 256K and GCC 32K tape-root KINIT/MRES builds LINK. Controlled
cost **+6 optional MRES instructions, +0 data, +0 fixed KCORE**.
The existing strict 32K fixed-KCORE size gate still FAILS at
`023241` octal >= `020000`. This fix prevents stale reuse **after
an observed I/O error**; it does not authenticate a different tape
or detect a silently disconnected device. See
`design/kcore-dtfs-io-error-cache-v1518.md`.

## 2026-11-12 v1517: simulated Type-551 detach is not an authoritative media-loss event

Executed PDP-6 SIMH legal Type-551 SEARCH/SELECT + CONI 0214:
unattached DTC0=`000004`, attached DTC0=`001000`, detached
DTC0=`000004`, and unrelated unattached DTC7=`000004` octal.
Four actual HALT PCs and memory-stored controller-status words
verified by testkit `3536d98`. SIMH emits `DTB_ILL` for selecting
unattached units, but Type-551 status combines illegal/select
conditions, and TU55/TU56 historical documentation does not
validate a universal physical tape-removed indication. In
particular TU56 notes analog SELECT ECHO is not used with 550/551/552.
Do not automatically HALT on `DTB_ILL`, generic I/O error or timeout.
v1515 critical HALT and v1516 exact root-member ownership are
ready; reliable hardware event and physical guest acceptance remain
OPEN. No production RAM/code changes. See
`design/kcore-dtc-select-status-v1517.md`.

## 2026-11-12 v1516: exact selected TSFS root physical-member mask

TSFS formerly stored eight 3-bit physical DECtape selectors, with
unused slots zero-filled, making real physical DTC0 indistinguishable
from trailing padding. KINIT now packs the verified 8-unit root
membership mask into previously unused bits 24..31 of the **same**
36-bit per-mount shape word. Existing selectors, volume format and
RAM layout are unchanged. The optional source-linked
`tsfs_root_owns_dtc_unit_v1516` checks namespace-root provider,
published mount bounds and physical unit membership; invalid root
metadata returns -1 rather than a false noncritical verdict.
KINIT host 1/2/8 member tests PASS (including unit zero), native
PDP-6 predicate and four unsafe mutants PASS, TSFS stored-extent
and cross-member tests PASS; isolated 32K GCC and full 256K KCC
tape-root KINIT/MRES builds LINK. Controlled additional optional
MRES code ~25-27 words, zero data, fixed KCORE unchanged.
Strict 32K fixed KCORE size gate remains FAILED at `023241` octal
against `<020000`; not an accepted 32K deployment. No automatic
device-loss report or critical HALT caller in v1516. See
`design/kcore-tsfs-root-member-v1516.md`.

## 2026-11-12 v1515 DECtape addendum: selected TSFS root members are critical

Confirmed removal of **any physical DECtape belonging to the mounted
TSFS root set** is a fatal root-device loss, even if the other members
remain readable. `tsfs_boot_select()` validates the physical set and
`tsfs_boot_unit_mask_v22()` supplies its selected-member mask during
boot; the TSFS runtime retains its logical-to-physical member map.
Do not treat every tape on the shared DTC controller as critical.
An auxiliary non-root DTFS tape is nonfatal on removal: fail I/O,
retain mount/FILE ownership, and revalidate tape identity/cache before
recovery. Type-136 timeouts alone cannot confirm tape removal.
The v1515 fatal leaf is ready but loss classification and calling it
from DTC remain OPEN; no new KCORE code or RAM in this addendum.

## 2026-11-12 v1515: fixed KCORE critical-device fatal stop

For non-removable media, permanent medium loss **is device loss**;
the category is distinguished from individual bad blocks and transient
controller errors. On *confirmed* boot-root or BACKSTORE device loss,
KCORE now provides a no-return `kcore_critical_device_lost_v1515`
entry, disabling PI and HALTing at separate linked root/swap code
addresses; invalid reasons have a third HALT. Six native PDP-6
source-linked tests PASS, including three executed negative mutations
and exact simulator HALT-PC comparisons. A controlled KCC KCORE relink
measures **+11 fixed instruction words, +0 permanent data words**.
Reliable physical device-loss classification and its caller routing
remain OPEN; existing generic negative I/O / PI6 watchdog results
must never automatically invoke this leaf. See
`design/kcore-critical-device-stop-v1515.md`.

## 2026-11-12 v1514: source-linked non-root unmount retry PASS

Native PDP-6 v1514 links actual `vfs_unmount` and mount retirement:
missing secondary D6FS media makes sync fail and preserves the
mount root/target, RO policy and FILE owner without a fake
unmount. When that **same** medium returns, sync and provider
preparation succeed, FILE is unlocked and only its mount slot
is retired; the root namespace is unchanged. Three actual
VFS ordering mutants rejected. Testkit `34fab36`. Full media
replacement revalidation, real BCACHE invalidation and dirty
open-handle recovery remain OPEN. No production RAM/code change.
See `design/kcore-nonroot-unmount-return-v1514.md`.

## 2026-11-12 v1513: root/BACKSTORE media loss fatal; non-root recoverable

Confirmed loss of root filesystem or BACKSTORE is **crashworthy**:
no reconstruction, quarantine, forced foreign process teardown,
or fabricated backing release. Mounted non-root media loss is
recoverable in principle; I/O must fail, the mount and other
filesystems remain owned, and returning/replaced media needs
checked revalidation plus BCACHE invalidation. The physical
drivers currently return the same negative status for a transient
PI error and PI6 watchdog timeout, so this status alone must
**not** trigger fatal HALT. Reliable permanent-loss detection,
minimal fatal dispatch and full physical non-root recovery
remain OPEN. Native source-linked v1513 PASS: two secondary
D6FS read failures publish no clean cache entry, root reader
remains operational, same secondary returns and reads succeed,
then repeated disconnect/return also works; three real source
mutants rejected. Testkit `8ab7648`, no runtime code/RAM change.
See `design/kcore-media-loss-policy-v1513.md`.

## 2026-11-12 v1512: physical Type-236 lost completion / PI6 watchdog PASS

Disposable booted 256K PDP-6 v1512 suppressed exactly two real
selected Type-236 `DR_DONE` completion notifications, leaving the
actual private I/O descriptors pending for production's original
300-tick PI6 watchdog. Sixteen fault-on samples recorded two
real PI6 timeout completions and 30 subsequent checked-free
refusals: 32 exact failed slot-0 restores. Direct PI2-error
count remained zero. Failed DISK slot `010` retained original
`02350`/`0330` backing, u-area, bitmap accounting, retry tag
and pending TERM with no false READY_ONCE, fatal close or ZOMB.
Independent swapped PID `011` restored and executed 562 genuine
SLEEP calls; resident PID 5 progressed by 992. Strict physical
v1512, source and negative-mutation tests PASS; no production
code, new process state, resident RAM or ABI growth. Real lost
media/DEVICE reconstruction remains OPEN. Detailed evidence:
`design/kcore-backstore-watchdog-physical-v1512.md`.

## 2026-11-12 v1511: physical Type-236 BACKSTORE read-error rollback PASS

The booted 256K KCC PDP-6 v1511 faulted two genuine selected
DRM PI2 completions through the original Type-167/236 error and
event-wakeup path. Real slot-0 failed returns distinguish 2
controller read errors from 25 checked BACKSTORE free refusals.
Thirteen physical fault-on samples retained exact bad DISK
slot `010` run `02350`/`0330`, u-area, allocation, pending TERM
and retry epoch with no false RAM/READY_ONCE or zombie.
An independent swapped PID `011` restored and executed 397 real
SLEEP calls, while resident PID 5 progressed by 823. Strict
v1511 physical verifier PASS, five source and eleven evidence
negative controls rejected. This is injected PI2 DRM error-path
acceptance, **not** actual lost media or terminal reclamation.
Production KCORE unchanged; no RAM/process-state growth. See
`design/kcore-backstore-device-error-physical-v1511.md`.

## 2026-11-12 v1510: physical healthy swapped contender progress PASS

Full 256K KCC PDP-6 v1510 source-linked physical guest observed
original failed DISK slot `010` (BACKSTORE `02350`/`0330` octal)
across 13 post-fault snapshots, 1..24 refused releases and stable
bitmap/u-area/TERM/retry ownership. An independently swapped
slot `011` was `SLEEP/DISK` before a **real checked swap-in**
commit at sample 32, then `SLEEP/RESIDENT` and executing 331 real
user SLEEP syscalls by sample 37. The independent resident PID 5
also progressed (42..770), while no owner repair/finalization or
false READY_ONCE occurred. Strict physical v1510 verifier PASS;
eight source and eleven false-evidence mutations rejected. No
production KCORE source/ABI/RAM change from v1507. The guest
intentionally stopped after diagnostic sampling, not HALT.
All persistent-refusal scheduler fairness gates are now accepted;
genuine DEVICE/media loss and terminal owner recovery remain OPEN.
Evidence and SHA256: `design/kcore-backstore-swapped-progress-physical-v1510.md`.

## 2026-11-12 v1509: independent resident progress under permanent BACKSTORE refusal

Booted 256K PDP-6 v1509 source-linked physical samples 24..29 retain
the exact failed DISK slot `010` extent `02350`/`0330`, original
u-area, retry marker and pending TERM with 3..14 refused frees.
A distinct real PID 5 resident spinner entered `SLEEP` 98..456
times (358 new real syscalls) across 7,904 PI6 ticks; its process
descriptor stayed live at every fault-on sample. Strict physical
v1509 verifier PASS; four guest-source and nine progress/ownership
mutations rejected. No KCORE source, process state or RAM growth.
Only **resident** bystander progress is accepted: healthy swapped
contender, genuine DEVICE/media loss and unrecoverable owner
reconstruction/reclaim remain OPEN. See
`design/kcore-backstore-bystander-physical-v1509.md`.

## 2026-11-12 v1508: physical sustained BACKSTORE refusal (scoped PASS)

Disposable real 256K KCC PDP-6 captured three post-fault snapshots of
one genuine selected DISK slot `010`, extent `02350`/`0330`.
Checked-release refusals advanced 1/4/7 while PI6 clock progressed;
the original DISK record, stable u-area, retry epoch and pending TERM
survived. No owner TERM consumption, zombie, false RESIDENT or repair.
Testkit's strict physical v1508 verifier PASS; ten unsafe witness
mutations rejected. Diagnostic sampling stopped intentionally, NOT a
normal HALT or terminal reclaim. Physical bystander fairness, genuine
DEVICE loss and recovery/reclamation still OPEN. No new KCORE code,
resident RAM or process state beyond accepted v1507.
See `design/kcore-backstore-persistent-physical-v1508.md`.

## 2026-11-12 v1507: owner-specific BACKSTORE retry (native accepted)

KCORE uses the existing stable u-area word 067 for a transient
nonresident RUN retry tag (LH bit 040000) and 17-bit PI6 clock epoch.
Real slot-0 swap-service failures retain the exact DISK backing,
u-area and pending TERM; normal and realtime selection postpone only
that owner for 0100 octal ticks, allowing healthy swapped processes
to run. Two independent failed owners cannot alternate without clock
progress. Expired retry tags, checked successful swap-in and checked
exchange remove the tag; genuine FINALIZE ownership is never
overwritten. The original global allocator-pressure delay is
unchanged. **No new process state or resident data word.**

Testkit v1507 real-source scheduler/clock/bitmap native tests PASS,
including three executable negative mutations, two independent
failures, direct repair and clock wrap. v1493/v1505/v1316-v1317
native regressions PASS (eight combined negative mutations);
v788 real exchange publication and v854 full sector/PURE exchange
also PASS with retry-tag retirement.
Isolated 256K KCC PDP-6 image build PASS. **Physical persistent
BACKSTORE/DEVICE failure and irreparable-resource release remain
OPEN**; do not confuse throttled retry with terminal reclamation.
See `design/kcore-backstore-retry-scheduler-v1507.md`.

## 2026-11-11 v1506: repaired physical BACKSTORE -> owning TERM accepted

Testkit `2bf10ff` v1504 completes the real 256K PDP-6 corrupt-bitmap
guest with one fault, three matching checked frees and one precise
repair. Both scheduler TERM requests coalesced with the already-pending
TERM on nonresident DISK slot `010`. After repair, the same owner
entered `proc_deferred_term_v970` once and consumed the pending bit,
ran its private fatal close once with `0400001`, finished once and
published one zombie. Strict fault-specific verifier PASSes with
20 negative owner mutations plus v1503's 15 prior checks. F07 exact
18/18 and independent physical MM/owner checks PASS; zero final
backing. Original normal v1336 still correctly rejects injected
swap-in failures. **Repaired corruption full physical path ACCEPTED;
permanently irreparable BACKSTORE handling remains OPEN.** No runtime
KCORE/RAM change. See
`design/kcore-backstore-owner-return-physical-v1506.md`.

## 2026-11-11 v1505: repeated checked-BACKSTORE rollback preserves owner

Testkit `b0e5075` native PDP-6 runs six successive source-linked
swap-in/slot-0/bitmap-free failures on the same corrupt DISK owner,
then explicitly repairs its bit and restores once. All six refused
attempts preserve the same DISK run, bitmap/accounting, unchanged VM,
released MM extent/pin and no READY_ONCE; later valid release commits
one VM with zero backing. Existing v1493 two unsafe mutations PASS.
Success AC1 is `slot|READY_ONCE` (`01002` for slot 2), while failure
requests TERM but may only coalesce a pending event. Corrected the
production service's misleading API comment, no functional source or
RAM change. These requests are forced by the fixture, not evidence
of real retry cadence or bounded permanent-corruption recovery.
That and physical v1504 owner-return acceptance remain OPEN. See
`design/kcore-backstore-repeated-refusal-v1505.md`.

## 2026-11-11 v1503: physical pre-existing TERM coalescing accepted

Testkit `8987b4a` verifies the completed 256K PDP-6 v1499 guest's
real map-backed event snapshots. Both scheduler TERM requests found
the same nonresident DISK slot `010`, no WAIT class, saved executive
PC and **an already-pending TERM**. Both returned zero, coalesced,
and left VM, FD-control, scheduler and FINALIZE unchanged. One real
BACKSTORE bit fault, three matching frees, one repair, final zero
backing and F07 18/18 child milestones were also observed. The
strict scoped verifier rejects 15 mutations; independent physical
MM/process/owner verifiers PASS. Invalid v1499 SKIPN branch counters
are explicitly excluded; v1502 corrected the test-only polarity.
Physical *coalescing* PASS, but actual owner-return TERM consumption,
permanent corrupt backing and full fault-specific physical finalizer
are still OPEN. No KCORE source/RAM change. See
`design/kcore-backstore-coalesced-physical-v1503.md` and current
pdp10-doc `STATUS/KCORE-FEATURE-STATE-V1503.MD`.

## 2026-11-11 v1502: feature-status/documentation synchronization

Current authoritative KCORE feature checklist: pdp10-doc
`STATUS/KCORE-FEATURE-STATE-V1502.MD`. Native tests v1498, v1500 and
newly committed v1501 (`daimos-testkit 7f415a8`) re-PASS with their
executable negative controls. The native real-source v1500 foreign
TERM -> PREVM idle finalizer test uses a controlled VM/BACKSTORE
backend. v1501 adds real foreign TERM deferral and owning-return
pending-bit consumption after *controlled* VM restoration, with a
stub private fatal-close observer. Both lifecycle seams are covered
at native level, not as one physical failure transaction.

The initial v1499 disposable 256K physical guest captured two real
pre/post TERM-owner snapshots, but its optional `SKIPN`-guarded event
counters were inverted and cannot qualify the TERM branch. Testkit
v1502 (`6c3de2c`) corrects this diagnostic guard to `SKIPE` and its
native positive/negative test PASSes. A corrected physical guest
and accepted branch/owner classification remain **OPEN**.
The v1495 corrupt-bitmap full guest still cannot establish actual
TERM branch, idle PREVM owner and permanently failed backing policy.
The original v1336 normal no-failed-swap-in gate stays strict. No
production KCORE code, RAM, ABI or acceptance policy is changed by
this documentation step. Current uncommitted v1492 FSINFO dynamic
mount-capacity work is not counted as an installed interface.

Other open KCORE tracks: remaining mixed BACKSTORE/BCACHE/LOGSTORE
device/media failure cases; physical DTFS tape-root integration;
swapless PURE-text scheduler/MM/full APR lifecycle; mount enumeration
ABI acceptance; and only then optional net-RAM-positive mutable
compression. 32K is on HOLD, MRES code unload/reload stays POST-1.0,
and KCORE size optimization remains last.

## 2026-11-11 v1500: native real event through idle PREVM finalizer

Testkit `5674a7e` source-links unchanged production
`proc_event_apply`, `proc_finish_slot` and
`proc_finish_service_v881` in the same PDP-6 instruction run.
For an immediately finalizable DISK owner, a controlled VM refusal
retains FINALIZE+PREVM, exit status and disk ownership. Idle retry
commits VM once, tolerates two u-area MM refusals and publishes the
zombie without repeat HUP/FILE/VM effects. A missing PREVM-marker
mutation is rejected. Native gate PASS; physical `vm_space_destroy`
and BACKSTORE bitmap are controlled. v1499 full-guest branch capture
and permanent-corruption acceptance remain OPEN. No KCORE code/RAM
change. See `design/kcore-backstore-event-finalizer-v1500.md`.

## 2026-11-11 v1498: native nonresident TERM policy acceptance

Testkit `f9ab499` executes unchanged production `proc_event_apply`
on a PDP-6 with a nonresident DISK owner. WAIT_CHILD and a saved
executive PC defer TERM and coalesce pending bits; a user-mode saved
PC with no WAIT/pending event enters foreign finalization. Controlled
VM refusal preserves DISK ownership, unlike a successful commit.
The executable unsafe-saved-PC-bypass mutation is rejected. Physical
v1495 did not identify which deferral/failure predicate applied;
v1499 branch/owner instrumentation is diagnostic only until accepted.
Full physical corrupt-backing feature gate remains OPEN. No KCORE
code, resident RAM or ABI change. See
`design/kcore-backstore-term-policy-v1498.md`.

## 2026-11-11 v1497: physical corrupt-BACKSTORE lifecycle is not accepted

Fresh disposable 256K KCC PDP-6 F07 v1495 diagnostic reached HALT,
with one real bitmap fault, three matching checked frees, a repaired
bitmap and final 0/0 process-swap/BACKSTORE blocks. Genuine F07 child
counts are 18/18; testkit v1496 fixes a SIMH banner `+` miscount.
However two failed swap-ins each called and returned from real TERM
delivery without immediate TRANSITION/PREVM, and the third matching
free had no PREVM marker. `proc_event_apply` permits safe deferred
fatal delivery for owned executive continuations; the exact predicate
was not captured. The injected test's idle-finalizer assumption is
unproven. v1336 normal profile forbids fatal swap-in by design.
Independent physical MM/owner/slot/stack verifiers PASS; the specific
BACKSTORE fault acceptance remains OPEN. No production code/RAM
changed. See `design/kcore-backstore-physical-f07-diagnostic-v1497.md`.

## 2026-11-11 v1494: retained PREVM finalizer on VM refusal

Testkit v1494 source-links the production `proc_finish_slot` and
`proc_finish_service_v881` and injects a controlled first VM destroy
refusal followed by success and two u-area MM release refusals.
FINALIZE+PREVM, TRANSITION, exit status and the original VM owner
survive the first refusal; later idle retries complete without a
repeated sleeping HUP or VM release. The premature-VM-commit mutation
is rejected. Earlier v1081/v1086 finalizer tests also PASS.
`vm_space_destroy` is a controlled backend in v1494; the genuine
BACKSTORE release appears in independent native v1493/v1405 tests.
The combined full 256K physical corruption/TERM/finalizer sequence
remains OPEN. No production KCORE changes. See
`design/kcore-backstore-prevm-finalizer-v1494.md`.

## 2026-11-11 v1493: source-linked checked BACKSTORE scheduler refusal

Testkit v1493 runs the production `proc_swap_in` and
`proc_swap_mres_service_v481` with real bitmap ownership validation.
On a deliberately incomplete bitmap the slot-0 service requests exactly
one TERM rather than READY_ONCE, exchange or cooldown; failed release
retains the original DISK record/bitmap/count and rolls back new MM.
Two negative executable mutations (unchecked free, false READY_ONCE)
are rejected. Existing v1405 rollback-and-retry native test also PASSes.
No DAIMOS production code or resident RAM changes. This is a native
source-linked service gate, not a real full-guest TERM/finalizer or
physical-device acceptance. The corrupt-bitmap full 256K guest remains
OPEN. See `design/kcore-backstore-scheduler-refusal-v1493.md`.

## 2026-11-11 v1490: BCACHE/DTC write correctness, feature phase

Production `dtfs_dtc_write` formerly called `bcache_reclaim` before
physical I/O, clobbering the DTC unit/block/buffer AC1..AC3 ABI.
Moreover an MM free refusal retained valid cache tags even across a
caller-requested media invalidation. v1490 separates nonallocating
`bcache_invalidate_all` from slab reclamation; DTC write arguments
survive, both tags are always invalidated, and an MM-refused slab
remains recorded for retry. Native real-PDP-6-source testkit v1490 and
both executable negative controls PASS. Fresh KCC KCORE and DTFS MRES
links PASS; this is correctness work, not size optimization. Real
physical tape-root acceptance remains a separate gate. See
`design/kcore-bcache-write-reclaim-v1490.md`.

## 2026-11-11 v1488: reconciled KCORE feature and RAM audit

Production D6 grouped codec v1464 and strictly restricted MEMFS XIP
v1469-v1485 are ACCEPTED and must not be reimplemented based on older
open v512/v1029/v1286 checklist prose. Physical 48K non-XIP v1486
boot and five DATE/MEMSTAT probes PASS. 32K is deferred; neither
32K nor 48K is an XIP profile, because MEMFS requires at least 96K.

The v1487 source-preserving shared-DXR EXEC/PURE pilot passes its
isolated native DXR positive/error regression, but costs +180 TEXT
words (current EXEC 218, candidate 163, shared engine 235).
Its original v1469 strict MEMFS XIP entry is copied unchanged, but
this is NOT a fresh XIP guest-lifecycle acceptance for the candidate.
Do not link this RAM-negative candidate as an optimization.
Staged swapless PURE MM transaction and restore modules v1381/v1384/
v1391/v1395/v1397 remain out of production until scheduler, backing
source immutability, complete APR reconstitution, and real RAM-pressure
acceptance. The v1451 30-word bitmap RAM codec remains isolated.

Remaining KCORE feature work: truly outstanding shared backing/
BCACHE/LOGSTORE failure and mixed-service gates, swapless PURE
scheduler integration, then net-positive optional in-RAM compression.
Online D6FS packer and unified TSFS remain separate 0.9 tasks;
unified TSFS is scheduled after KCORE final optimization. Size-only
optimization is LAST. Current fixed KCORE, MRES, transient KINIT and
48K guest RAM figures refer to distinct profiles: never subtract
unmatched figures to claim word savings.

Dynamic MRES module unload/reload is NOT being deleted: keep it as
POST-1.0, with no implementation or testing before 0.9 release.
This documentation sweep makes no production source change.

## 2026-11-11 v1487: KCORE feature scope excludes runtime MRES unloading

Dynamic MRES module unload/reload is a POST-1.0 feature, not a
0.9/1.0 requirement. Preserve it in the future roadmap but do not
spend development or testing effort on it until 0.9 ships.
Current boot-installed MRES modules remain permanent; filesystem
unmount and snapshot recovery do not imply MRES code unloading.

## 2026-11-11 v1485: reclaim uncommitted MEMFS on corrupt snapshot

The real 128K two-cold-boot v1485 test first publishes an ordinary
root-owned strict MEMFS XIP snapshot. Host-side corruption then flips
one bit in the committed namespace metadata without changing the
checksum header or AUXSTORE partition descriptor. Boot 2 correctly
refuses the persistent mount, but before this fix the unpublished
memfs_mres_fs singleton still pointed at its 01200-word namespace MM
allocation. MEMINFO falsely reported MEMFS capacity and a same-boot
volatile mount was rejected despite no published VFS mount.

system/kernel/fs/memfs_runtime.s now routes snapshot-admission refusal
to the existing mount-failure cleanup. It releases the actual namespace
extent first, clears exactly the three-word singleton only after
successful MM free, then returns failure; if MM refuses, ownership is
retained instead of losing the reference. No provider, format,
allocator policy, permanent data or per-process state changes.
Matched full-profile KCC fixed KCORE endpoint remains 023434 octal;
MEMFS MRES code is one word larger, BSS unchanged.

The isolated physical 128K guest PASSes: persistent boot 1 creates and
executes strict XIP; boot 2 refuses deliberately corrupted snapshot
metadata; a new volatile MEMFS mount then succeeds in the same boot,
the exact new XIP image is readable/runnable, and the guest HALTs with
XIP-SNAPSHOT-CORRUPT-RECOVERED and STATUS:0. A clean-media post-fix
128K/192K/256K two-cold-boot matrix separately PASSes exact persistent
payload/XIP restoration and unchanged AUXSTORE geometry on both boots.
Source-linked v1094/v1471 ownership and native MEMFS normal/v1403/v1468
fatal regressions all PASS. No sub-96K MEMFS/XIP profiles are tested.

## 2026-11-11 v1484: LOGSTORE direct-DRM ABI and persistent MEMFS snapshots

Persistent MEMFS could publish a structurally valid snapshot on the
auxiliary Type 236 drum but could fail to restore it after a cold boot.
Disk-level inspection located the committed snapshot at physical drum
block 015767 (v2 header, metadata length, and exact 36-bit checksum
were valid). However, DR0 block zero's AUXSTORE descriptor was replaced
by words from KCORE `vfs_write_words`, preventing AUXSTORE discovery on
the next boot. Restoring *only* the first seven descriptor words of an
already-corrupted diagnostic image let the existing snapshot restore;
the repair was diagnostic evidence, never a production workaround.

An isolated v1483 SIMH worktree recorded two actual Type 167/236 DR0
block-zero WRITEs during INIT/LOGIN, using KCORE text addresses 005372
and 005373 as DMA source. Those numbers were supposed to be LOGSTORE
physical drum blocks 05372/05373. Source tracing established an ABI
mismatch: `logstore_runtime.s` passes unit, block, buffer in AC1..AC3;
`logstore_minit` patched its direct read/write slots to raw DRM MRES
services expecting hardware group-address, buffer in AC1..AC2. Hence
the 05372/05373 block argument was taken as a core-buffer address and
unit zero was mistaken for drum block zero. v1484 patches the existing
public KCORE DRM/DSK read/write bridges instead, after validating that
the corresponding raw device services are installed. Only reclaimable
KINIT selection code changes; no KCORE or process words are added.

The persistence path also needed previously uncommitted v1477/v1480
corrections: three inverted snapshot backing-size/reserve comparisons;
BCACHE workspace invalidation before direct snapshot reads/writes; and
boot-time recognition and reservation of committed v2 snapshot ranges
in addition to v1, so swap cannot claim retained snapshot blocks.
These are included in the same source review and native guest matrix.
The MEMFS MRES image gains two code words from the new cache-invalidate
calls (snapshot MRES BSS unchanged); fixed KCORE endpoint remains
octal 023434 in matched 128K KCC images.

The full-profile physical 128K, 192K and 256K two-cold-boot guests PASS:
first boot creates/executes strict root-owned MEMFS XIP and snapshot
payload, HALT commits; second boot reads exact saved data and executes
restored XIP. AUXSTORE partition geometry is inspected and compared
after *each* boot, rejecting the former DR0 block-zero clobber.
The isolated 128K v1478 test refuses one snapshot payload write on
each cold boot, verifies `dsys_halt` returns failure without committing
an incomplete image, then performs a successful HALT retry. Its next
boot restores the same strict XIP/data payload; both boots PASS with
the AUXSTORE descriptor intact. Native v706 LOGSTORE direct read/write,
v1 commit-last append, and MEMFS normal/v1403/v1468 fatal cases PASS.
All 32K/48K tests remain non-XIP by the explicit MEMFS 96K minimum.

## 2026-11-11 v1479: MEMFS XIP requires at least 96K physical words

Raw XIP is exclusively a MEMFS-backed process image. MEMFS is disabled
on hosts with less than 96K physical words, so 32K and 48K are NOT XIP
test targets. No active XIP guest test case used either configuration:
the full-profile acceptance matrix already covers only 128K, 192K and
256K. The v1469 direct guest driver now refuses physical sizes below
96K before it builds anything. Keep the separate low-memory 32K/48K
non-XIP boot, APR, swapless and PURE release tests; they are unrelated
to the MEMFS-dependent XIP feature. Do not add 32K/48K XIP tests again.

## 2026-11-11 v1476: correct three-word MEMFS state clear on unmount

Full-profile XIP v1474 filesystem tests unmount and remount /TEMP through
the public root syscall. Each 128K/192K/256K KCC PDP-6 baseline guest
PASSed three cycles, checking duplicate-mount refusal, old executable
invalidation, zero MEMFS usage/capacity while unmounted, exact resource
recovery, certified XIP recreation, fresh RUN/WAIT and v1469 RUN/EXEC.

An isolated v1475 fault refused the first MEMFS data-destroy preflight
without changing filesystem ownership. The original mount and executable
survived, but a subsequent remount cycle failed with the original code.
This revealed an actual two-word MRES memory overwrite: memfs_mres_fs
has exactly three BSS words, but successful unmount called fs_zero_words
with a five-word count. The extra zeros erased adjacent MRES memory,
including the test-only one-shot refusal counter, and provoked a second
spurious refusal after remount.

Production memfs_runtime.s now clears exactly three words, consistent
with its mount-failure cleanup. The physical 128K v1475 guest now PASSes
refusal/retry, exact accounting, three remount cycles and original
RUN/EXEC. A v1476 MRES map canary checks the first word following the
three-word record. Only one existing instruction immediate changes
from 5 to 3: no new KCORE words, MRES words, BSS or process state.

Native v1094/v1471 XIP release and v1403/v1468 MEMFS tests PASS.
All three post-fix physical 128K/192K/256K normal remount guests PASS
three cycles and the complete original v1469 RUN/EXEC regression,
ending with XIP-REMOUNT-PASS, XIP-RUN-PASS and STATUS:0.
MEMFS orderly HALT uses snapshot publication; it does not
dynamically unload the MRES. Persistent snapshot/reboot and runtime
module unload/reload remain separate open work.

## 2026-11-11 v1473: privileged MEMFS XIP process-slot pressure accepted

New `daimos-testkit/tests/validation/daimos-xip-pressure-matrix-v1473.sh`
runs a full-profile KCC DSK/D6FS physical PDP-6 at 128K, 192K and
256K. Each guest creates one certified root-owned no-write MEMFS XIP
image and performs three queued RUN/WAIT pressure rounds, allocating
distinct child slots and 0410-word u-areas up to the process-table
admission limit (maximum 56 children per batch). The guest checks the
expected occupied slot count, waits every child with zero EXIT status,
then verifies baseline process slots, PROCESS MM words and MEMFS logical
data words, and post-release CHMOD and exact image readback. The original
v1469 XIP negative/positive RUN and EXEC replacement checks then pass.
All three guests report XIP-PRESSURE-PASS, XIP-RUN-PASS and STATUS:0.

A test-design defect was found and corrected: queued zero-write XIP
children finish in two instructions and can become zombies before the
parent returns. An occupied process slot does NOT establish an active
XIP pin; therefore no unmount refusal is inferred from queued slots.
Instead the existing v1472 injected u-area failure test now calls
`dsys_unmount(/TEMP)` while a pending unpublished RUN has a proven
MEMFS pin (CHMOD is refused). The full 128K v1472 guest PASSes this
new check, then confirms deferred unpin/reap, exact MM counters (one
allocation refusal, two injected unpin refusals), and new XIP RUN/WAIT.

The v1473 pressure and proven-pinned unmount tests require NO DAIMOS
KCORE or MEMFS source changes. All added code lives in daimos-testkit.
Still open: explicit MEMFS unmount/re-mount and provider shutdown/reload
lifetime tests, and the separate 32K/48K non-XIP swapless gates.

## 2026-11-11 v1472: XIP RUN u-area OOM and deferred retry matrix accepted

After v1471 fixed XIP VM teardown, v1472 closes the outstanding fault
boundary between successful `exec_load_process` strict MEMFS admission
and the subsequent RUN child u-area allocation. The daimos-testkit's
`daimos-xip-uarea-retry-v1472.sh` copies the exact production kernel to
an isolated build tree and inserts test-only failure hooks at the real
`proc_run_block` allocator gateway and the resident MEMFS XIP-unpin leaf.
The first hook verifies that operation 031 owns the newly staged pinned
physical base before returning one simulated u-area allocation refusal.
The second refuses two unpins without changing either the provider's
reference count or the physical MM pin. Production kernel sources and
the uninstrumented image ABI are unchanged.

Independent full-profile KCC disk-root PDP-6 guests at **128K, 192K and
256K** all PASS. Each reaches ROOT LOGIN/DSH with the correct KINIT
memory banner and MEMFS loaded; the first XIP RUN fails as expected.
The unpublished child is observed as SYS_TEARDOWN_RUN_VM (stage 3),
its pinned file refuses CHMOD and the child is not published as a
WAITable zombie. Slot zero retries with the original shared exponential
backoff and eventually unpins the image, discards the slot, and returns
process-slot/VM accounting to baseline. CHMOD then succeeds, file bytes
remain unchanged, and subsequent XIP RUN/WAIT and EXEC replacement pass
with `STATUS:0`. The boot simulator's post-HALT examination confirms
**exactly one allocator refusal and exactly two unpin refusals** in
each KCORE image. The testkit provides a three-size matrix runner.

This is *fault-test-only* instrumentation and adds **zero words** to
production KCORE/MRES, no new permanent ownership fields and no new
compression algorithm. The initial native guest tests were corrected
to query stage 3 instead of stage 2, to treat nonblocking WAIT=0 as
nonzombie, and to respect slot-zero's capped retry backoff rather than
mistaking a long injected refusal sequence for a kernel deadlock.
Remaining open: heavier concurrent memory/process-slot pressure,
provider/mount loss and cross-mount lifetime fault testing, and the
separate swapless PURE/bitmap reclaim integration.

## 2026-11-11 v1471: XIP detach refusal retains VM pin for retry

Production `vm_space_destroy` had a post-release failure boundary: it
released the MEMFS XIP pin before calling `proc_swap_detach`. An injected
backing-record refusal returned -1 while retaining a VM descriptor that
pointed at an already unpinned MEMFS extent. The new daimos-testkit
`daimos-xip-detach-retry-v1471` native PDP-6 witness reproduced the
failure against v1470 at phase octal 011, then passed after the fix.

For a pinned XIP base only, `vm_space_destroy` now uses the existing
read-only MEMFS query 031, detaches the **resident** zero-text backing
record before releasing operation 026's MEMFS pin, and publishes VM_NONE
only when both operations succeed. If detach refuses, both VM and pin
are intact. If MM unpin refuses, the VM and pin remain owned; XIP's
resident backing record may already be empty, and detach is idempotent
on owner retry. Other private-process VM teardown retains its original
MM-free-then-detach order. The ordinary branch uses the stacked physical
base after the query because MRES calls may clobber scratch AC3.

Native production-leaf tests PASS: backing-detach refusal and retry,
MM-unpin refusal and retry, normal process-MM base validation and two
unsafe release/base mutations rejected. Previous v1094, v1108 and
v1109 fault/retry suites PASS with test-only fixtures updated for
operations 030/031 and the new teardown order. Actual full-profile
128K, 192K and 256K XIP guest boot/LOGIN/DSH, six RUN/WAIT and EXEC
PASS after correcting the saved-base regression found by the first
128K guest run.

Matched full KCC fixed KCORE endpoint grows from octal 023411 to
023434 (+19 words); the VM object text grows 607 -> 626, BSS 1 -> 1.
This is required functional code and no size optimization. Independent
192K/256K v1471 guest retests PASS, completing the three-size matrix.
Unpublished RUN u-area allocation failure and heavy concurrent RAM
pressure remain separate OPEN release gates.

## 2026-11-11 v1470: privileged raw-MEMFS XIP accepted at 128K/192K/256K

The v1469 **unchanged production kernel** now PASSes an independent,
full-profile (`BOOT_PROFILE=full COMPILER=kcc`) PDP-6 disk-root XIP guest
at physical core sizes 128K, 192K and 256K. The v1470 testkit uses
`BOOT_CORE_WORDS=0400000/0600000/01000000` and
`SIMH_CPU_KWORDS=128/192/256` respectively, builds separate images,
and verifies the actual KINIT `MEM` banner before accepting a result.
Each guest loaded MEMFS and D6FS, reached INIT V1, ROOT LOGIN and DSH,
executed three serial and three queued certified XIP RUN/WAIT children,
rejected malformed content, non-root execution and nonzero argv,
verified EXEC argv rollback and completed privileged XIP EXEC with
shell STATUS:0. Source and XIP reference cleanup checks stayed green.

The first 192K run was a **false positive**: Open SIMH rejected the
`set cpu 192k` request as an unknown CPU modifier and booted 256K
instead. The `sims` fork mistakenly labeled its 12 * 16K memory option
`196K`. This was corrected to `192K`, compiled with `make pdp6`, tested
with direct `set cpu` / `show cpu` for all three sizes, installed to
`$PDP10_PREFIX/bin/pdp6` via a versioned v1470 executable, and pushed
to sims commit `16dc791`. The actual 192K guest was **retested** with
the corrected simulator, reported `MEM 192 K`, and PASSed. The v1470
matrix now preflights every simulator CPU modifier to avoid this error.

The attempted lowmem-MEMFS experiment was stopped and its source edits
reverted: MEMFS is absent by design from the 48K lowmem configuration.
There are **no DAIMOS kernel feature/size changes** in v1470 and no
KCORE size optimization. Still OPEN are controlled MM/u-area failure
injection, post-publication teardown retries, heavy memory pressure,
and 32K/48K acceptance of the *other* non-XIP features.

## 2026-11-11 v1469: privileged MEMFS raw-XIP RUN/EXEC full guest PASS

Actual production KCORE now recognizes the XIP magic in `exec_load_process`
and, for an invoking root process and MEMFS vnode only, calls the v1468
strict pinned-image admission. The certified extent is published directly
as the staged process VM; `RUN` creates its independent u-area and `EXEC`
uses the existing transactional replacement/rollback. Unlike the ordinary
DXR path, XIP never copies code, relocates it or writes startup words into
the pinned MEMFS extent. `vm_space_startup` requires zero argc/envc and
publishes only kernel-resident startup results; mutating BRK requests are
refused. MEMFS's read-only operation 031 identifies the pinned extent using
existing reference metadata, with no new persistent per-process words.
Swap victim selection and direct swap-out already exclude MM-pinned bases.

The fresh 256K KCC DSK/D6FS guest boots ROOT/DSH and PASSes three serial
and three queued raw-XIP RUN/WAIT lifetimes, an intentionally malformed
nonzero image-word refusal, an unprivileged UID1 RUN refusal, argv refusal,
failed EXEC argv rollback and successful root EXEC replacement with final
status zero. The restricted program is only MOVEI AC2,immediate followed
by unindexed UUO 040 EXIT; DAIMOS takes EXIT status from UUO effective
address, not AC2. MM pins and metadata mutation are checked across runs.
The prior v216 SIMH/AAP user-mode release, native MEMFS normal/fatal,
v1394 host/native policy, v200 EXEC rollback and v20 PURE-bit tests PASS.

Exact GCC lowmem fixed KCORE: 9701 -> 9765 words (+64 functional code),
endpoint 023125 octal. Matched v1468/v1469 KCC MEMFS MRES link against
the same KCORE map: 1849 -> 1851 resident words (+2), BSS 13 unchanged.
These are feature-footprint measurements, NOT a size-only optimization or
strict 32K placement acceptance. Still open: 32K/48K and full pressure,
fault-injected u-area/VM rollback and wider device/mount lifecycle tests.
This accepts only the intentionally tiny no-write XIP instruction profile,
not arbitrary code executing from shared MEMFS memory.

## 2026-11-11 v1468: strict write-free raw-XIP provider admission

Movable MEMFS now provides private FS MRES operation 030 (24 decimal)
for a certified, root-owned, nonwritable raw XIP program. The strict
operation reuses v1467 file ownership, format verification and MM pinning,
then checks entry 020, APR extent geometry and **every** program word.
Only MOVEI AC2,immediate followed by UUO 040 EXIT is permitted; all other
nonheader words must be zero. The candidate is pinned throughout inspection.
Caller base/entry outputs are published only on success, and a refusal
releases its provisional pin; MM cleanup refusal is a fatal ownership
invariant. The format-only operation 027 remains for hardware diagnostics.

Native PDP-6 normal, failed-instruction, nonzero-trailing-data and fatal
MM-unpin tests PASS, plus source-before negative witness 0243. Existing
SIMH/AAP user XIP and v1394 C-policy tests PASS. Matched full MEMFS MRES
relocation against the same KCORE map: 1783 -> 1854 resident words (+71,
BSS unchanged at 13). This is a **provider feature**, not RUN/EXEC process
launch; KCORE 9701-word v1466 fixed footprint is not updated or optimized.
Still open: root-authorized loader selection, no-write startup, BRK/swap
exclusion, atomic VM publication and scheduled RAM-pressure acceptance.

## 2026-11-11 v1467: MEMFS raw-XIP provenance gate, feature work only

Production MEMFS `memfs_xip_admit` now verifies the existing UID9 owner
metadata is zero before pinning a candidate raw-XIP file. A regular,
nonwritable executable with valid XIP/PURE header is no longer sufficient
when owned by a non-root account. Rejection leaves both output pointers
and physical MM pin/refcount unchanged. This enforces one source-authenticity
prerequisite without adding per-process state or changing normal DXR EXEC.

The source-linked native PDP-6 MEMFS normal/fatal tests PASS; v216 real
user-mode XIP UUO and VM teardown PASS on SIMH and AAP; v202 AAP and v1394
host/native restricted-instruction policy PASS. A pre-v1467 production-source
negative control fails the new owner test at octal `0236`, as required.
Full privileged RUN/EXEC admission, instruction-policy linkage, startup
no-write handling, BRK/swap exclusions, and integrated pressure/rollback
acceptance remain OPEN. No size optimization was attempted.
An exact matched movable MEMFS MRES link against the existing KCORE map
measures 1774 -> 1781 resident words (+7 initialized, BSS still 13).
No fixed-KCORE object was changed; this is footprint accounting only.

## 2026-11-10 v1464: block-only low core, +14 fixed words

Removed D6LZ36 KCORE/Stage-1 code and replaced it with the versioned
v1458 fixed block decoder (53 TEXT words, zero DATA/BSS) and 58-word
VFS/direct-buffer frontend. There is no second decoder or compatibility
fallback. All five Stage-1 images contain the exact KCORE 53-word image,
verified by native object comparison. EXEC/PURE/TSFS call the new API.
Native PDP-6 decoder tests and a fresh 256K KCC DSK/D6FS boot through
ROOT LOGIN, DSH and external ECHO PASS.

Against the matched previous lowmem GCC 32K build, fixed KCORE is
**9687 -> 9701 words (+14)**; movable SWAP MRES remains **1762**, and
complete KCORE+MRES inventory **16367 -> 16381**. The strict 16K
single-root permanent last is **16428 decimal** and still requires
**45 words** to fit. No new codec static data or BSS; minimizing the
merged fixed code and preserving correct storage ownership is preferred
over a second decoder. The full isolated 256K TSFS-root LOGIN/DSH/ECHO
and return to prompt also PASS using the new block-only format. The
480-second command bound accommodates slow DECtape seeks (sampled PC in
`dtc_search_wait`) while still requiring the full external EXEC return.
The separate post-cutover 256K swap/OOM acceptance remains open until
a new strict full-guest run finishes.

## 2026-11-10 v1462: merged branches; fixed footprint unchanged

Merged native-toolchain and v1420-v1461 block-codec experiment histories
without activating their unaccepted alternate production decoder.
Release Stage-1, KCORE, filesystem, swap and host tools keep the existing
D6LZ36 decoder while versioned block-codec sources and tests remain
available for isolated research. This avoids silently combining a codec
format change with the already accepted v1405 BACKSTORE release behavior.

Fresh GCC disk-root lowmem measurements: **9687 fixed KCORE words**,
**1762 SWAP MRES words**, **16367 KCORE+MRES words**, with a strict 32K
single-root permanent-placement deficit of **31 words**. The normal
native BACKSTORE/swap tests PASS; 256K v1406 F07 full-guest acceptance
remains historical evidence for the unchanged production code paths,
not a new full-guest run of every integrated optional feature.

## 2026-11-10 v1406: full 256K owner/OOM acceptance; no code growth

The committed v1405 SWAP-MRES checked-free implementation now passed the
fresh strict v1349 physical 256K PDP-6 KCC DSK/D6FS guest to HALT/EXIT,
including original F07/held-provider and 18-child postflight, real
MM_ERR_NOMEM and cooldown, 34 successful swap-ins, zero leaked disk blocks,
and all four independent dynamic MM extents reclaimed. The six owner-evidence
negative mutants also fail validation. This is a genuine new full-guest
acceptance rather than the older v1351 result or a partial boot preflight.

No v1406 production code changes. Fixed KCORE remains **9687 words**;
movable SWAP MRES **1762 words**, KCORE+MRES inventory **16367**,
strict single-root permanent placement **31 words over**. Full 32K
admission and other configurations still require separate validation.
Raw XIP RUN/EXEC, swapless PURE scheduler and unusual BACKSTORE bitmap
refusal through `SYS_EVENT_TERM` remain open; D6 codec stays deferred.

## 2026-11-10 v1405: SWAP MRES release safety, +5 movable words

The production SWAP-MRES `proc_swap_in` path now checks the real BACKSTORE
free **before** publishing the restored RAM image or changing the process
swap accounting. An incomplete bitmap forces the existing MM allocation
rollback, retaining the original DISK record for retry. Exchange/prepare/
cancel/swap-out/PURE scratch cleanup routes unexpected release refusals to
the existing fatal storage invariant. New 36-bit PDP-6 v1405 red/green
and mutation tests PASS; selected existing exchange and PURE tests PASS.

The 32K GCC lowmem disk-root build reports fixed KCORE unchanged at
**9687 words**; SWAP MRES increases **1757 -> 1762** (+5 code words),
KCORE+MRES inventory **16362 -> 16367**, permanent last address
**16409 -> 16414**. The strict single-root placement still FAILS by
**31 words**, versus 26 at v1404. No new static RAM words are allocated.
The full physical 256K F07 postflight is separate from these native checks.

## 2026-11-10 v1401-v1404: MEMFS provider safety +19 resident words

MEMFS's private sector allocator now refuses an already-freed sector and
does not clear its final chunk descriptor until MM confirms free success.
The provider checks MM release results before discarding the last resident
file owner. A failed eviction backing WRITE followed by failed cleanup
BACKSTORE free now triggers the existing fatal ownership invariant instead
of leaving newly allocated backing blocks orphaned. Native PDP-6 red/green
tests v1401-v1403 and non-writing XIP SIMH/AAP v216 PASS.

After fixing the boot Makefile's missing MEMFS data/snapshot assembler
dependencies, a fresh direct MRES build measures **1771 resident words**
(1758 initialized, 13 BSS), up from 1752 at v1400, with NO fixed KCORE
code changes. Build dependency correctness was required to avoid misleading
stale linker measurements. The refreshed lowmem profile still has **9687
fixed KCORE words** and a strict single-root 26-word overage; this is not
an accepted 32K placement. Legacy v202 AAP and v209 APR tests also PASS.
Full physical pressure/guest lifecycle remains
an open item; grouped D6 codec is deferred.

## 2026-11-10 v1399/v1400: MEMFS failure-atomic backing owner

Production MEMFS now checks `backstore_free` results before losing its last
disk owner, overwriting user words, releasing resident file extents, or
publishing resized vnode data. Tentative relocation extents are discarded
on refused release, not committed. The low-core spill path avoids double-
freeing the already-evicted original chunk. Before unmount, the provider
verifies every existing vnode BACKSTORE owner; PREPARE_UNMOUNT returns -1
with mounted state unchanged if preflight fails. Unexpected errors after
partial teardown route to KCORE's existing fatal ownership invariant.

The test first failed against v1398 in the actual PDP-6 provider, then
passes after the production fixes, including first/later invalid backing,
write, resize, truncate and PREPARE_UNMOUNT refusal. v216 SIMH/AAP user
XIP and VM teardown also PASS. MEMFS MRES **1719 -> 1752 resident words**
(+33 code, unchanged 13 BSS); fixed KCORE size is unchanged by these MOVABLE
changes. Partial MM release and eviction-write cleanup failures and full
swap pressure still need separate acceptance; the grouped D6 codec remains
deferred.

## 2026-11-10 v1398: checked BACKSTORE lifecycle saves 7 fixed words

BACKSTORE free now returns 0 success / -1 refusal and verifies complete
ownership before changing bitmap/counters. A disable-for-new-allocation flag
cannot prevent existing disk owners from verifying or releasing their runs.
Resident swap detach uses that one checked release rather than an enable-
gated preverification followed by a void free; failed releases keep the
process's disk record and storage accounting intact. Native PDP-6 and host
release checks PASS. This uses no new persistent data or extra MRES table.

Controlled full GCC 32K lowmem disk-root A/B: committed v1397 (`170c7f02`)
**KCORE 9694 words** versus v1398 **9687**; **7 fixed words saved**.
Both have SWAP MRES=1757 and softclock MRES=292. Total KCORE+MRES inventory
16369 -> 16362, permanent last address 16416 -> 16409. Single-root strict
16K placement overage improves 33 -> 26 words but is NOT yet within limit.
Earlier comparison to a stale build was invalid for attribution.

Unfinished: MEMFS's backing-drop consumer still lacks failure-atomic
propagation, remaining swap MRES free callers need acceptance under rejected
release, and full physical pressure / XIP / PURE scheduling tests remain open.
Grouped D6 codec is deferred until those feature tracks are finished.

## 2026-11-10 v1396/v1397: XIP provider +7, no fixed-core growth

The movable MEMFS provider now freezes mode, owner and timestamp metadata
for pinned raw-XIP nodes (CHMOD/CHOWN/UTIME through one dispatch function).
Exact source-linked MEMFS native and SIMH/AAP user lifecycle tests PASS.
The MEMFS MRES resident package grows **1712 -> 1719 words** (+7 TEXT),
no new DATA/BSS; fixed KCORE remains **9870** GCC lowmem words. This does
not imply that the restricted v1394 policy is wired into RUN/EXEC.

The AUXSTORE v1 on-disk reader keeps legacy zero-length pool compatibility.
New `mkauxstore` outputs zero,zero for absent partitions. Formatter and
discovery/region tests PASS; no permanent KCORE growth.

The still-unlinked PURE DROP transaction now refuses failed rebase cleanup
unpins and keeps TRANSITION held (MM_ERR_BUSY). Native failure-injection
tests PASS; the KCC standalone body grows **518 -> 521 TEXT** (+3), zero
DATA/BSS. Actual production PURE scheduler activation and net 32K/48K RAM
measurement remain open. D6 codec discussion remains deferred.

## 2026-11-10 v1393-v1395: prioritized feature work, D6 codec deferred

The current implementation order is AUXSTORE/BACKSTORE, restricted raw
MEMFS XIP and swapless PURE RAM. The proposed grouped D6 codec is deferred
until those features are finished; production D6LZ36 remains unchanged.

BACKSTORE's existing verifier is reused on free without its new-allocation
enable gate, preserving cleanup of existing owners after STORAGECTL disable.
Partial, duplicate and out-of-range frees cannot mutate the bitmap or count.
Native v1393/v777/v795 and AUXSTORE v657/gateway v815/publication v665
tests PASS. BACKSTORE remains a SWAP MRES implementation rather than new
fixed KCORE code. Complete physical swap/OOM retesting is still open.

The staged root/trusted raw XIP instruction policy v1394 passes host and
native PDP-6 tests at 80 TEXT / 0 DATA / 0 BSS. It intentionally accepts
only a stackless MOVEI AC2,immediate plus EXIT; no normal EXEC/RUN linkage
or scheduled user-lifecycle acceptance is claimed. The existing APR gives
no write protection to arbitrary shared XIP instructions.

The unlinked PURE FULL transaction now verifies all MM release failures
and retains TRANSITION on incomplete rollback. v1395 four-case native fault
tests and v1390 common FULL regression PASS. The standalone C transaction
grows 359 -> 368 target TEXT words, zero static DATA/BSS, for explicit
rollback safety. No new codec dependency or production scheduler change.
Linked size and real total-RAM acceptance must still precede cutover.

Rebuilt GCC lowmem 32K disk-root linked report: fixed KCORE **9870 words**,
SWAP MRES **1757 words**, complete package inventory **16651 words** and
PERMANENT_LAST **16698 decimal**. All are unchanged from the previous linked
checkpoint. The standard report still fails the strict single-root 16K
permanent-address limit by 315 words; no boot/DSH acceptance is inferred.

## 2026-11-10 v1391/v1392: bounded FULL image restoration, staged only

The unlinked v1391 FULL restore transaction passes the exact pinned
MM APR extent size to the single shared DXR backing reader. Before
writing, the assembler rejects invalid APR size, physical alignment
or 18-bit extent and any initialized image plus the 020-octal user
origin larger than the allocation. This includes compressed input:
FULL decompression starts only after the capacity gate succeeds.
The previous unbounded staged three-argument FULL entry was removed.
Actual production disk/drum swapping is unchanged.

Native plain/compressed success, exact-fit, oversized, malformed,
failed read/decoder, mount-18, suffix integrity and rollback PASS
with the corrected installed KCC. The discovered
`swap_slots[1]` symbolic-address compiler defect was corrected in
KCC's optimizer instead of changing DAIMOS array access syntax.

Cost gate: the common DXR module grows from 212 to **235 TEXT**
words (161 common core plus 74 bounded backing adapter), zero
DATA/BSS; the separate staged v1391 C FULL transaction is 359
TEXT, zero DATA/BSS. A hypothetical merged EXEC/FULL pair is
119+161+74=354 versus 183+50=233, hence **+121 pre-fold
words** without the new C transaction. No KCORE or actual
physical-memory savings are claimed until folded linking and
full 32K/48K guest pressure tests are accepted.

## 2026-11-10 v1389/v1390: shared DXR code folded within its source

Unlinked `exec_image_common_v1386.s` now uses BLT register frames:
225 -> **212 PDP-6 TEXT words** including the FULL/PREFIX
backing adapter, still zero DATA/BSS. The common parser/payload
section is 161 TEXT words. The FULL backing entry reuses precisely
the parser and decoder used by staged EXEC and PURE_PREFIX: no
additional DXR header implementation. Native v1390 PASSes
plain/compressed FULL restoration, high mount 18 and rollback
with the actual staged v1381 restoration transaction.

The v1387/v1390 test provider also required a real read-count fix:
on a two-word header request it improperly wrote three words into
the caller. After enforcing the exact requested count, the native
mount-18 tests PASS even with broader test-only format selection.
This was fixture corruption, not evidence of a KCC or production
loader defect.

The historical production `proc_swap_reload_pure` leaf is 50 TEXT
words; the new shared FULL/PREFIX adapter is 51. The generated
alternate EXEC caller remains 119 versus 183 words for real EXEC.
The **combined hypothetical** linked code before p10fold is
119 + 161 + 51 = 331 versus 183 + 50 = 233,
or **+98 words**. This excludes other new swapless transaction
code, required allocator/scheduler integration and any existing
shared code folded away. Thus no claim of overall RAM improvement
is justified without measuring the real complete kernel and
per-process released words at 32K/48K. The v1390 testkit size gate
pins these isolated metrics; production Makefiles stay unchanged.

Before any live FULL callback cutover, pass the actual destination
extent capacity to the loader and reject image sizes exceeding it;
the current three-argument legacy reload callback cannot directly
prove that bound. Compressed PURE_PREFIX is still unsupported.

## 2026-11-10 v1386-v1388: one shared staged DXR engine, cost-gated

`exec_image_common_v1386.s` stages a common four-word DXR parser
and FULL/PURE_PREFIX image dispatcher, replacing duplicated loader
logic in a native PDP-6 test without adding per-process state.
FULL supports existing DXR1/2 plain images and DXR2 compressed FULL
decoding. PURE_PREFIX is strict about the immutable DXR2 text
boundary and rejects compressed prefix requests before any user
memory write. `exec_image_prefix_backing_v1386` uses the canonical
existing mount/provider backing decoder and the same parser/reader.
The independent v1385 DXR parser remains testkit-only historical
evidence.

Native testkit v1386 (shared parser/payload), v1387 (real PURE drop,
MM reverse rebase and restoration via the shared loader), and v1388
(actual EXEC caller with the shared loader in a disposable alternate
assembly source) PASS. v1388 covers DXR1/2, compressed FULL, RT-only
admission, errors before VM creation, VM shortage, read/decode
failure and attachment rollback. No mandatory system EXEC source,
production KCORE Makefile, raw-swap loader or resident C object was
modified.

Isolated size **before p10fold**: original `exec_load.s` 183 words,
alternate EXEC caller 119, common parser/payload 169, combined 288
or **+105 words**. The common adapter for TRUE swapless PURE also
raises the standalone common object from 169 to 225. Do not count
these as a production image delta or quietly accept their RAM cost.
The current target is to consolidate and shrink that code before
live 32K/48K acceptance. Native tests do not yet prove actual
filesystem concurrency, hardware PI or booted EXEC/RUN behavior.

## 2026-11-10 v1384/v1385: in-place RESTORE, no second production loader

The unlinked MM `mm_pure_expand_v1384.s` and transaction
`proc_pure_restore_v1384.c` can reclaim an entirely free original
PURE prefix *without allocating or copying another process image*.
Owner/type/size/pin/physical arena gates precede the descriptor
claim. The full address range remains owned by an inactive
PACKED_PROCESS extent during all filesystem I/O. On failure, the MM
descriptor rolls back to the original raw suffix; on success, the
complete PROCESS extent, physical VM base and resident storage mode
are published atomically under PI exclusion.

The testkit-only `pure-ram-prefix-v1385/exec_prefix_fixture_v1385.s`
validates native plain-DXR2 prefix reading for both low and high mount
IDs. Production must **merge EXEC full-image
loading and PURE_PREFIX restoration into one executable-loading
engine**. Only the MM reserve/commit and process lifecycle orchestration
remain independent. The existing compressed DXR decoder is not yet
bounded at the PURE boundary; compressed images select the safe
full-image fallback instead of overwriting the retained suffix.

Host and native PDP-6 tests PASS the MM preflight, original prefix
claim, rollback on short reads, unchanged writable words, blocked
prefix, mount 18 and plain DXR header checks. Isolated text: 323
transaction + 130 MM + 78 test-only plain loader = 531 words,
zero DATA/BSS, **not** projected production resident growth.
Production remains at three fixed C translation units, unchanged.

## 2026-11-10 v1383: ONE WORD of sizes, not an extra permanent record

Potential swapless RESTORE geometry fits one 36-bit word:
`LH = uncompressed PURE text words`,
`RH = current mutable suffix words`. Compute the truly released
prefix as complete 0200-octal-word sectors of
`text_words + EXEC_USER_ORIGIN`; then `VM_WORDS = prefix + suffix`.
The current DXR2 TX2 text count, existing one-word PURE backing
and the process `vm_state` LH already supply this information, so
there is no justification for allocating a second permanent size
word today. Beware DXR2's 18-bit text field versus the currently
enforced 14-bit in-core PURE backing count.

If the low prefix gap remains free, RESTORE can potentially reverse
the in-place MM rebase and refill only the missing prefix, leaving
all mutable words undisturbed. Existing plain DXR reads would need
to stop at the exact released prefix. Existing compressed DXR
reload **cannot** be used unchanged: it writes the entire initialized
image and the old D6LZ core may reject a match crossing the smaller
text output limit. Require a bounded-prefix decoder or independently
framed text segment at the future atomic grouped-codec cutover,
and validate in native PDP-6 tests. If the prefix gap is occupied,
retain the full-extent safe restore/fallback and handle pressure.
No source/Makefile behavior changed by this v1383 design decision.

## 2026-11-10 v1381/v1382: zero-copy drop by MM descriptor rebase

The staged `proc_pure_ram_v1381.c` now retains inactive writable
process RAM **without any second physical allocation or copy**.
The standalone `mm_pure_rebase_v1381.s` checks an exactly-once-pinned
owner-tagged APR extent and changes its original descriptor to start
at the existing physical suffix address, removing PURE prefix words
from MM ownership without moving data. Under a single short PI-off
critical section, C publishes saved backing, nonresident VM and the
already existing two-bit PACKED mode. Restore and terminal discard
retain the validated registry-free lifecycle from v1379.

Host and actual PDP-6 KCC/assembly MM rebase tests PASS, including
rollback, precise type/owner/pin/18-bit range checks and exact suffix
restoration; v1382 adds real resident PURE busy scanners through
zero-copy DROP, restore and final discard. A corrected test allocator
now rejects overlapping extents. KCC text: 518 transaction + 70
MM primitive = **588 words**, zero data/BSS (v1379: 567 C words),
not a production linked-size delta. For a 1024-word sample process,
old DROP required 768 extra words transiently; v1381 requires zero.

Neither module is linked into production, so the three resident C
translation units, raw swap and normal boot are unchanged. Hardware
PI/full MM concurrency, no-partner restore memory pressure, EXEC and
teardown integration, and actual 32K/48K tape-root guest tests are
still open. See `../pdp10-doc/KERNEL/SWAPLESS-MEMORY.MD`.

## 2026-11-09 v1379/v1380: registry-free PURE-only staging

`proc_pure_ram_v1379.[ch]` supersedes the standalone v1377
prototype for the **raw swapless** inactive-process representation.
No dense/sparse registry is required: the existing owner-tagged
MM_TYPE_PACKED_PROCESS extent supplies the suffix physical base and
length, while `vm_state` and PURE backing supply the logical span.
Existing two-bit modes are published using resident assembler
functions. Host and native PDP-6 tests PASS roundtrips, MM fault
rollback and terminal teardown without linking the old C registry.

The v1380 native regression combines v1379 with real resident PURE
write/unmount busy scanners and verifies public mount 18 across
all MM release windows. No new resident VFS guard is needed on those
tested paths. The v1379 transaction object is **567 PDP-6 KCC words**
with zero DATA/BSS, versus 672 for v1377. It also drops 704 words
of prototype C map/layout/owner objects from the isolated test link,
reusing existing native KCORE mode operations. These are NOT linked
image savings, and no production build was changed.

An in-place MM conversion/copy could avoid the current second-suffix
peak RAM allocation, but requires a correct atomic allocator primitive:
the existing `mm_process_shrink_pinned` only permits 02000-octal
granularity for live PROCESS extents. This is unresolved. VFS
serialization, KINIT, scheduler, EXIT/WAIT, 32K/48K tape-root runtime
and real RAM high-water acceptance remain open. v1377 is retained
as historical regression evidence.

## 2026-11-09 v1377: PURE-only inactive RAM lifecycle staged

`proc_pure_ram_v1377.[ch]` stages a no-codec, no-disk, owner-checked
inactive process representation for an explicitly swapless PDP-6
build. PURE-sector omission reuses the conservative existing swap
prefix boundary and assumes user ACs are saved in the stable u-area.
Drop retains every mutable suffix word verbatim; restore reconstructs
text before overlaying those saved bytes; discard frees terminal
process RAM while clearing the mount-extension credential bit. These
are scoped to a caller already holding lifecycle TRANSITION and
immutable source protection, and reject compilation without the
swapless-only profile macro. The original raw process swap is intact.

Host tests PASS with dense/sparse descriptor layouts, repeated image
roundtrips and injected failure cases. The KCC-generated transaction
executes successfully on PDP-6 SIMH against controlled MM, provider
and scheduler stubs, including retry after allocation exhaustion.
No production scheduler code, KINIT link, file/mount locking or
process-switch path has changed. This is not a complete swapless
booted acceptance. Standalone KCC transaction size is 672 words
(data/BSS zero); three existing staged support modules total 704
additional pre-fold words in the isolated native test. See
`../pdp10-doc/KERNEL/SWAPLESS-MEMORY.MD` for the remaining gates.

## 2026-11-09 v1375: P0-D compressed swap retired; swapless reclamation planned

Production disk/drum process swapping remains RAW. The old opt-in
compressed disk-swap record requirement (decision 5/P0-D) is withdrawn.
This does not refer to or alter the separate accepted OOM P0-D test
matrix.
Swapless-only PURE text dropping and opt-in compression of **mutable
process RAM** are the replacement work. The existing swap path already
omits reconstructible PURE sectors from disk writes; this does not
constitute swapless RAM reclamation. The PDP-6 single contiguous APR
process mapping requires inactive-state storage and full reconstruction
before rescheduling. Both new features are currently NOT implemented;
no KCORE size or executable behavior changed in this revision.

Current KCORE C-source inventory from the actual PDP-6 Makefiles:
`proc/proc.c`, `fs/file.c`, `proc/vm_pdp6_swap.c` = **3 linked C
translation units**. `drivers/wall_clock.c` is compiled but its
`wall_clock.dobj` is removed from the fixed KCORE object list in favor
of the selected clock MRES/gateway; `proc_pure_attach.c` is textually
included by `vm_pdp6_swap.c`, not an independent translation unit.
Most other kernel C sources are KINIT or MRES code or inactive prototypes.
Canonical design: `../pdp10-doc/KERNEL/SWAPLESS-MEMORY.MD`.

v1376 implements a separate unlinked, lossless, allocation-free
36-bit-word RAM codec and gives it accepted host and KCC/PDP-6 native
unit tests in `daimos-testkit`. Its standalone object has 449 KCC
text words, no static data/BSS, so no overall savings are established.
PURE-only inactive-process retention and full APR restoration remain
unimplemented and must precede any production compression mode.

The older KCORE.MD v219 packed-process task list and historical
compressed-swap references below are superseded for current scope.
Keep prior test evidence but do not execute old compressed-disk-swap
TODOs. The active queue is the top of TODO.MD.

## 2026-11-08 v1351: complete physical 256K F07 owner lifetime PASS

The fresh v1349 256K PDP-6 guest passed the original strict 18-child
RUN/ACK/TERM/WAIT, physical requester OOM, held FS provider,
independent spinner witness, HALT/EXIT and zero-leak F07 checks.
One genuine swap-in MM_ERR_NOMEM/no-partner cooldown, 34 disk
swap-ins, zero exchanges and zero final swap/backstore blocks were
observed. The new phase-correct MM owner test confirms four actual
8K test-owner extents with resident spinner pin pre-pressure,
then exact independent typed owner/pin release after cooldown,
final prepared=4/freed=4/count=0/error=0; all six negative owner
controls and two mandatory-field schema controls are rejected.
Testkit `f156746`, docs
`pdp10-doc/KERNEL/SWAP-FULLGUEST-OWNER-LIFECYCLE-V1351.MD`.

This definitively closes the booted physical no-partner swap-in
liveness gate **for the tested 256K PDP-6 guest configuration**.
No production KCORE, PI6, MM, FS, swap, MRES source, or permanent
resident RAM changed. The old speculative fallback stays excluded.
Other feature-freeze and memory optimization work remains OPEN;
native grouped D6CODEC staging baseline v1350 is 38/38 PASS but
does not activate its production replacement for D6LZ36.

## 2026-11-08 v1348: full physical no-partner PDP-6 suite accepted

The original strict 256K KCC PDP-6 test, including all 18 real
RUN/ACK and TERM/WAIT child lifecycles, held-provider conflict,
genuine requester-only BRK OOM, independent spinner >=5 SLEEP
progress, HALT and SIMH EXIT, now PASSes. The v1348 disposable
fixture waits for **actual** spinner progress instead of lowering
the held-provider witness threshold (observed count 8).

One real production MM_ERR_NOMEM/no-eligible-partner cooldown was
followed by 34 successful disk swap-ins, no exchanges and final
zero outstanding swap/backstore blocks. Independent v1349 offline
verification of this same completed guest confirms four genuine
8K-word test-owner MM extents before release, resident process pin,
one post-cooldown independent typed owner release, exact four frees
and no retained owner; six corrupted evidence controls are rejected.
No production KCORE/FS/MM/PI6/SWAP/MRES source was modified and
no RAM grew. The old +16-word fallback is not justified. Full
256K booted no-partner acceptance is CLOSED for this exact guest.
See `pdp10-doc/KERNEL/SWAP-FULLGUEST-COMPLETE-V1348.MD` and its
validated v1348 evidence. Other KCORE functionality and resident
size optimization remain OPEN.

## 2026-11-08 v1347: actual 256K physical no-partner guest completed

The unbounded 256K PDP-6 guest now completes authentic 18-child
RUN/ACK and TERM/WAIT lifecycles, requester/witness KWP, HALT and
SIMH EXIT after real no-partner swap-in MM pressure. The corrected
source-linked v1303 postflight checker verifies one MM_ERR_NOMEM,
one cooldown, 34 successful swap-ins, no exchanges and zero final
swap/backstore allocated blocks. Optional v1342+ read-only sampler
address symbols do not replace or extend original mandatory v1303
after-HALT counters; testkit `a7454e4` rejects absent/duplicate
mandatory fields. Retained evidence SHA256:
`dcab588ef04e13280b8417ce017e7e47a012ea89cb99fe4a1246eb3cc4d65045`.

One original v1150 suite witness remains unaccepted: the completed
guest recorded four genuine independent spinner sleeps after the
provider lease/predispatch, against the original threshold of five.
The versioned v1348 disposable guest requires one more *real*
scheduled spinner sleep before INIT sends its TERM event, using
only a read-only test selector. Full-suite acceptance is still OPEN.
Production KCORE/MM/FS/PI6/DRM/SWAP/MRES remains unchanged.

## 2026-11-08 v1346: final swap-out reclaim refusal accounting

The production `proc_swap_out_fail` shared return label rejects
ineligible reclaim candidates before a backing transaction as
well as later failed operations. Consequently its raw v1209 count
cannot by itself establish a fatal swap-out. Testkit `f12880d`
corrects the v1303 final physical no-partner verifier without
changing kernel code. Strict full-guest HALT, 18 child TERM/WAIT,
typed allocator failure and cooldown, balanced restores, zero
fatal swap-in and **zero outstanding backing** remain mandatory.
Seven unsafe evidence mutations are rejected. Original v1303
and source-linked v1345 tests PASS. The full booted 256K guest
cleanup/liveness acceptance remains OPEN. Documentation:
`pdp10-doc/KERNEL/SWAP-RECLAIM-REFUSAL-VERIFIER-V1346.MD`.
No KCORE/MRES/FS/DRM/PI6/SWAP source or resident size changed.

## 2026-11-08 v1345: booted physical no-partner and DRM event boundary

The v1344 bounded booted 256K PDP-6 guest proved a genuine
no-eligible-partner MM allocation failure followed by release of
four owner-tagged 8K extents and five additional ordinary swap-ins
(13 to 18). The independent test-only guarded provider release
also returned its original holder after three actual busy attempts.
The production slot-0 polling PC was source-mapped to the DRM236
completion event: in nine samples, AC1 equaled the active request
descriptor plus two words. The DRM native watchdog regression PASS.
Testkit `453e269` verified actual linked counters and rejected six
unsafe evidence mutations; docs `pdp10-doc/KERNEL/SWAP-SLOT0-DRM-AND-INDEPENDENT-RELEASE-V1345.MD`.

The diagnostic intentionally stopped at 18 read-only samples:
all-child TERM/WAIT, normal HALT and complete MM/backing cleanup
remain UNVERIFIED. No production MM, PI6, KCORE, swap, provider or
device source changed and no permanent KCORE/MRES RAM grew.

## 2026-11-07 v1340: genuine independent MM release and swap recovery

The booted 256K F07 guest v1338 entered the authentic no-partner
allocation-pressure cooldown (one MM_ERR_NOMEM, no exchange/fatal
swap-in). The pinned slot-5 user independently read cooldown=1,
freed four original owner-075 8K MM extents through production
`mm_free`, and unpinned its own VM through production `mm_unpin`.
All source-linked owner/pin counters reconcile. Normal swap-in
success rose from 13 to 18 afterward, proving immediate resumed
physical swap activity after independent release.

The same full guest stalled subsequently after INIT `^`; no final
HALT, provider/OOM completion, 18 TERM/WAIT or zero-backed cleanup
has passed, so overall liveness remains OPEN. The testkit v1340
sampler fixes loss of guest output at Ctrl-E/SIMH stop boundary
(selftests PASS) and adds read-only `proc_timer_next` for diagnosing
the late wait. No production KCORE or MRES code or RAM change.
See `pdp10-doc/KERNEL/SWAP-INDEPENDENT-OWNER-RELEASE-V1340.MD`.

## 2026-11-07 v1336: real guest swap-in shortage/no-partner branch

The previously native-only no-partner pressure path has now been
observed under an actual 256K PDP-6 DSK/D6FS boot. A disposable
guest with four early genuine 8K MM owner extents recorded 93
real typed swap-in allocation failures and exactly 93 cooldown
entries, no exchanges, 54 ordinary swap-outs and 37 swap-ins.
Testkit `c642d28` verifies the source-linked read-only SIMH sample
and rejects eight mutated counter states. These measurements
establish reachability of the production branch, not completion
of full-system recovery: INIT was blocked waiting for child RUN/ACK
while owning the memory required to proceed. The test was stopped
without release, HALT or final leak checks. The independent resident
user releaser (testkit `745690a`) has passed disposable preflight
but still requires complete booted acceptance. No KCORE/MRES
source or resident footprint was modified. Documentation in
`pdp10-doc/KERNEL/SWAP-NO-PARTNER-LIVE-PRESSURE-V1336.MD`.

## 2026-11-07 v1330: no-partner full-guest pin strategy (zero growth)

The v1323 booted 256K 24K-process F07 guest successfully swapped
51 images out and 35 in but failed to allocate even a new 1K
APR-aligned, typed MM test-owner extent after 18 child admissions.
Its explicit `!t` failure and zero real swap-in MM pressure/cooldown
show a test-setup limitation, not a production kernel defect.
Existing resident image pinning through unchanged production MM
(`mm_pin`/`mm_unpin`, testkit `1a0693e`) is the v1329 alternative.
It avoids any synthetic owner allocation or fabricated MM return,
but was still awaiting full-guest final classification at checkpoint.
No KCORE/MRES source or RAM growth, nor historical fallback, applies.

## 2026-11-07 v1325: physical owner pressure sizing; no code growth

The booted v1322 256K PDP-6 guest completed 18 real child admissions
and substantial ordinary swapping but the separate 8K test-owned MM
allocation failed (`!t`). The strict postmortem proves zero real
swap-in allocation pressure and cooldown events, so there is no
kernel liveness defect to fix from this evidence. A smaller 1K
APR-aligned, owner-tagged v1323 full-guest test has passed build
preflight but remains unaccepted until complete physical results.
Testkit v1324 also fixed early guest HALT misclassification by the
read-only PTY sampler without touching the DAIMOS kernel.
No resident KCORE/MRES size change or rejected swap fallback added.
Independent owner admission postmortem testkit `e6e0d26` and
corresponding pdp10-doc `2f990d6` agree on this negative result.

## 2026-11-07 v1322: no-partner full-guest physical owner test

The native production MM/swap/PI6/gateway transaction tests remain
PASS, but a booted 256K genuine no-partner swap-in has not yet been
demonstrated. Testkit v1321 tried to combine the 24K process F07
with six actual 8K-word MM_TYPE_KERNEL_DYNAMIC test-owner extents.
The initial run was intentionally interrupted during admission
(15 swap-outs and 8 swap-ins, zero MM failures), before the extra
allocations. The earlier completed v1308 snapshot measured only
15840 managed words free at OOM; a six-times-8K reservation is
therefore unlikely to fit. Testkit v1322 instead uses one 8K-word
real owner extent with exact typed owner release and same physical
swap-in no-partner acceptance requirements. Its disposable guest
preflight PASS, but a completed full-system result remains OPEN.
No production KCORE, MM, swap or MRES source was modified and
no permanent RAM footprint increase is introduced by this test.

## 2026-11-07 v1320: native swap-in via production DEVICE gateway

The native PDP-6 swap-in transaction now executes the unchanged
`fs_mres.s` backing DEVICE gateway and its eight-sector dispatch
loop, with only the terminal DSK270 hardware read stubbed. The
test verifies physical sector translation and 0200-word buffer
progression, correct RESIDENT payload/owner and bitmap cleanup.
Both first- and third-sector I/O failure preserve the DISK image,
undo process MM allocation and report one TERM. A buffer-advance
mutation is rejected by executable PDP-6 assertions. Testkit
`d65899c`, docs `pdp10-doc/KERNEL/SWAP-DEVICE-GATEWAY-NATIVE-V1320.MD`.

This does not change production KCORE or MRES RAM. Booted 256K
full-system no-partner liveness remains OPEN. No swap fallback
has been added; no kernel size optimization was attempted.

## 2026-11-07 v1318: full native production swap-in transaction

There is no production KCORE/MRES source, footprint, ABI or runtime
allocation-policy change. Testkit `7ffd840e` now executes actual
native PDP-6 `proc_swap_in` after real MM NOMEM and three rejected
test exchange candidates; production PI6 countdown, scheduler
swap request, MM allocation, pin/unpin, resident VM and packed storage
mode publication, and backstore bitmap release all PASS. The physical
disk read provider remains a native test stub, but its two copied
physical endpoint words are checked. A failing read invokes the
production MM unpin/free rollback, preserving backing ownership and
terminating the failed restore request exactly once. Three unsafe
production-code mutants are detected at runtime. See
`pdp10-doc/KERNEL/SWAP-IN-TRANSACTION-NATIVE-V1318.MD` and the
versioned v1318 test evidence (SHA256 recorded in PROJECT-STATE.MD).
This does **not** close the booted 256K full-system no-partner gate;
no historical v1208/v1211 fallback was merged or resized.

## 2026-11-07 v1315: PDP-6 PI6 cooldown and scheduler gate

Production kernel source and resident size remain unchanged.
Testkit `bfb629ae` extends the native real-MM no-partner test
to execute actual PI6 clock timekeeping and the scheduler's
swapped-process gate. 63 consecutive tick calls leave a 1-tick
cooldown and allow the independent runnable path; the 64th tick
expires backoff and permits a real swap request. An actual `mm_free`
of the independent owner's physical extent allows retry, actual
MM allocation and one-shot scheduler dispatch. Six executable
negative mutants fail as intended. The fixture excludes hardware
PI6 dispatch and substitutes disk swap-in and partner admission,
so booted full-system recovery remains OPEN, without any proposed
RAM growth or fallback. Full detail:
`pdp10-doc/KERNEL/SWAP-NATIVE-REAL-MM-NO-PARTNER-V1314.MD`.

## 2026-11-07 v1314: swap-in actual MM shortage native acceptance

No production kernel memory or swap algorithm changes in v1314.
The native PDP-6 testkit `d40a1158` now exercises the unchanged
production physical MM allocator plus slot-0 swap service under
a real occupied 8K-word arena, instead of fabricating an MM error.
The 1K-word pending process extent obtains MM_ERR_NOMEM after the
bounded reclaim attempts; all three exchange candidates are
rejected, the disk-backed process retains its record and cooldown
0100, and no TERM occurs. A real MM free of the occupying extent
allows allocation, correct process ownership and one-shot dispatch.
Four executable negative mutations are rejected, and adjacent
native MM/swap tests pass. Test stubs still model backstore transfer
and exchange admission, so this does not close **full-system**
no-partner forward-progress, PI6 retry or disk rollback acceptance.
See `pdp10-doc/KERNEL/SWAP-NATIVE-REAL-MM-NO-PARTNER-V1314.MD`.
The v1208/v1211 fallback and KCORE size optimization stay deferred.

## 2026-11-07 v1312: full live-swap guest trace and replay

On the same production-source kernel, the fully traced 24K-BSS
256K PDP-6 guest completed 15 live read-only PC/scheduler/swap
snapshots, 18 additional RUN/TERM/WAIT cycles, requester-only OOM,
provider/witness milestones and HALT. The final counters recorded
51 successful swap-outs and swap-ins, zero real swap-in allocator
pressure, zero exchange and zero cooldown. The original runner
could not parse interactive SIMH post-HALT `sim> ` prefixes. That
simulator-formatting issue is corrected by v1309 testkit; v1311
replays the unchanged original v1075 postflight checks against a
separate prompt-normalized copy, all provider/protected/MM/slot/
swap/media validations PASS. A corrupted provider counter is
rejected. This is evidence replay, not a direct rerun claim.

See `pdp10-doc/KERNEL/SWAP-SIMH-LIVE-TRACE-V1309.MD` and
`$TMPDIR/daimos-swap-no-partner-evidence-v1312-r1.tar.gz`
(SHA256 `e5224db0e4266bea68c7e953c22f49c6074443ea930a5b360665725ab8a1b669`).
The remaining full-system no-partner allocation-pressure path was
not entered; its liveness gate remains OPEN. No production KCORE,
MRES or swap algorithm change and no RAM footprint optimization.

## 2026-11-07 v1309: read-only SIMH swap progress observation

The kernel remains unchanged since production OOM activation.
Testkit v1306 now observes the exact linked PDP-6 KCORE PC,
scheduler timer/cursor, swap-in/out service counts, actual MM
allocation failures, exchange/cooldown state and backing ownership
at bounded host-selected intervals using SIMH Ctrl-E/EXAMINE/GO.
Its separate raw and guest logs preserve the original F07 post-HALT
acceptance contract. The operator procedure and acceptance limits
are in pdp10-doc `KERNEL/SWAP-SIMH-LIVE-TRACE-V1309.MD`.

The original full 24-process 16K-BSS physical F07 guest PASSes
with 41 swap-ins and no pressure/cooldown. An exploratory 24K-BSS
guest counted 49 swap-outs and 33 swap-ins, but was interrupted
before its complete release/cleanup checks; no full acceptance.
The concurrently running v1308 long diagnostic is not yet accepted
at this checkpoint. Full-system no-partner liveness remains open,
with no evidence warranting v1208/v1211 fallback RAM or code growth.

## 2026-11-07 v1302: swap-in pressure and no-partner validation

Functional acceptance of production requester-only OOM remains
unchanged. The subsequent swap-in pressure audit used the clean
`283b54dc` kernel and changed no production code. A fresh 256K
PDP-6 16-child scheduler-pressure guest PASSes; current native
incoming, rollback and service-wiring tests PASS. New daimos-testkit
v1300 source/mutation guards and v1301 real native no-partner service
test PASS, including preserving disk ownership and deferring retries
without terminating a viable process. The obsolete C check in
v756 is repaired to examine the current native assembly contract.

The still-open question is full-system forward progress during real
swap-in pressure with no exchange partner; none of these results
establish that scenario. Historical v1208/v1211 fallback experiments
remain rejected pending a demonstrable present-day failure and exact
rollback acceptance. No KCORE feature size optimization is authorized.

## 2026-11-07 v1298: requester-only BRK OOM F12/F13 accepted

The 256K KCC PDP-6 DSK/D6FS PCLK60 headless CTY writable-process
release profile now has production requester-only physical BRK OOM ON.
The accepted F11 62/64 gates are joined by completed F12 activation
and F13 post-enable signoff: 64/64 active OOM release gates CLOSED.
All 14 F03 booted policy cases, all 18 P0-D native stages, all 17
P0-D physical guests individually, and both fresh 64-slot F07
pressure profiles PASS. The interrupted P0-D monolithic attempt
is not represented as an uninterrupted run.

Recovered original post-activation v1285 Android and independent
Linux x86-64 E13 archives both use candidate `78259106`, whose sole
change from `e6993a76` is the production OOM switch. The bundled
assembly is byte-identical to the released source. Both policies
on both hosts PASS; the same-policy linked KCORE maps match exactly,
and independent PDP-6 simulator hashes differ. All four full-media
builds pass strict F10 diagnostics with no unexplained errors.
The v1298 evidence package is
`$TMPDIR/daimos-f13-release-evidence-v1298-r1.tar.gz`, SHA256
`5dcb4d75ea6ea2291e5ec01f54874ce8d704713c1608fff9dcd64ab8aab83221`.
Release evidence is scoped to the selected 256K profile; restricted
raw MEMFS XIP, strict 32K operation, other machine/media targets
and the separate 16K permanent residency optimization are not
claimed. DTFS tests remain excluded during the planned rewrite.

## 2026-11-07 v1296: F12 activation and exact-source local acceptance

The requester-only physical BRK OOM production switch is now 1 in
the uncommitted kernel working tree. Fresh source/dispatch/cause
checks, both 64-slot 256K release-media policy guests, all 18 native
P0-D release stages, all 17 individually completed physical P0-D
guests and all 14 F03 two-policy full-system cases PASS. The
interrupted P0D_FULL matrix is not called an uninterrupted PASS.
The v1190 F10 diagnostics checker accepts both current 64-slot
release-media logs, with no unexplained diagnostics.

Local SHA256-verified evidence is archived at
`$TMPDIR/daimos-f12-production-oom-evidence-v1296-r1.tar.gz`.
F12 remains OPEN for independent post-activation second-host/media
qualification and final release review; F13 remains OPEN for
post-enable signoff and commit/push. The older v1286 default-OFF
report below is a dated source snapshot, not the current switch.
DTFS tests remain excluded. No KCORE optimization was performed.

## 2026-11-07 v1279: Linux local-relocation toolchain defect, not KCORE growth

The v1257 stable-uarea scheduler locality guard did not cause the
host-specific v1112 failed-RUN HALT. The Linux-installed October 13
`p10fold` predates the verified v1141 cross-object LOCAL relocation
identity fix and incorrectly folds distinct object-local code.
After installing corrected `daimos-tools` v1277 and testing native
v1141/v1257 safety, the Linux guest passes and its KCORE endpoint
`023267` octal matches Android exactly; no resident KCORE or SWAP
MRES source edit was necessary. See
`pdp10-doc/TOOLCHAIN/P10FOLD-LOCAL-RELOCATION-SAFETY-V1141.MD`.
OOM F11/F12/F13 release signoff remains separate from this tool bug
and the still-unmet permanent 16K KCORE/MRES RAM target.

## 2026-11-07 v1274: OOM F07 maximum-process/descriptor gate CLOSED

The first release-profile 256K KCC/DSK/D6FS kernel passed one
simultaneous real physical-OOM snapshot with all 64 process slots in
use (slot 0 kernel, 63 live users), 55 resident APR extents and eight
swapped users, 127/136 MM extents, 239576/242085 managed words,
31642 exact APR rounding words and a 128-word held provider workspace.
Private/idle stack high-water was 180/206 and 63/256 words. All 57
extra children were admitted and individually reaped, provider/FILE/MM
rollback completed and the expected guest HALT was reached within
900 seconds; 13 unsafe negative snapshot mutants are rejected.
The verified archive SHA256 is
`0fb22920bfd9f66c155ef3bb61f8db0c0f146f80f89bbc3e6e1b3406d9599e1b`.
This closes F07 only for the S07/S08 scope, not E13/F11/F12/F13;
requester OOM remains production-disabled. This 256K dynamic pressure
measurement is NOT evidence that permanent KCORE+MRES fits 16K.

## 2026-11-07 v1269: clean production-source F07 24-process regression PASS

The 256K KCC PDP-6 DSK/D6FS test runs successfully without the
experimental v1208/v1211 swap changes: 18 additional RUN children,
real disk swapping, protected physical BRK OOM, held foreign FILE
provider, full TERM/WAIT/reap and final resource accounting all PASS
on committed DAIMOS `9a841363`. The complete simulated workload
reaches `<++++++++++++++++++^@%&*?{}|~~~~~~~~~~~~~~~~~~KWP` and HALTs
before the original 900-second cutoff. v1150/v1159/v1203/v1205/v1207
verifiers and all evidence checks PASS. Clean swap counts: 41 normal
out/in transfers, no allocation-pressure service or exchange calls.

The v1208/v1211 experiment was also tested successfully, with ten
real fallback invocations, eight exchanges and 33 normal out/in
transfers, but increases the identically instrumented SWAP MRES by
16 linked words and does not reduce KCORE itself. An independent
no-equal-partner pressure case remains necessary before deciding
whether to accept its added permanent residency; the clean F07 PASS
proves only that this specific workload does not need it.

Evidence: `$TMPDIR/daimos-f07-clean-v1269-acceptance-r1.tar.gz`,
SHA256 `b0883d79d0a7baacb007690f62be6234a458c927afabf08ad1aac2d57510dde2`.
The 64-slot/combined-high-descriptor F07 gate is still OPEN, as are
E13/F11; production requester OOM remains disabled. See pdp10-doc
`KERNEL/OOM-F07-24PROCESS-SWAP-CLEANUP-V1264.MD` for the earlier
experiment and the completed clean comparison.

## 2026-11-05 v1189: static 16K residency is separate from OOM

F05 is no longer an OOM enablement gate. The previously measured
16K permanent KCORE/MM-table/MRES budget shortfall remains a
KCORE/MRES size-optimization problem and is not waived as a design
target. F07 still requires dynamic peak allocation/descriptor and
APR/provider/stack use evidence for the selected 256K OOM profile.
No production code, footprint or OOM switch changed by this scope
correction.

## 2026-11-05 v1163: second simulator target and strict host compiler repair

Two full 256K KCC DSK/D6FS E13 requester-OOM guests PASS on Linux
x86_64 Open SIMH `a6b511e5`, independently of the earlier Android
aarch64 Open SIMH `edd19b17` tests. All four guest archives pass the
testkit v1163 cross-platform process/FS-owner, real BRK physical OOM,
WAIT/MM recovery and exact KCORE low-end comparison (off `023410`,
test-on `023426` octal). They are disposable instrumented images,
not a measured production KCORE+resident-MRES budget. The two hosts
used DAIMOS revisions `27acb950` and `90dfc8d0`; final exact-revision
E13 is therefore still open, as are other required simulator targets.

Linux had stale installed AUXSTORE tooling. Canonical host formatter
now declares POSIX 2008 APIs for strict C99 builds (DAIMOS `90dfc8d0`),
and host v636/v638 safety/DBOOT regressions PASS on both platforms.
Production kernel code/data, ABI and OOM default OFF are unchanged;
KCC diagnostics and F05/F06/F10 signoff remain outstanding. 51/65.

## 2026-11-05 v1161: fatal release graph and P0-D native preflight

Frozen v1160 candidate DAIMOS `fe0f3c0e` PASSes the 18-stage native
P0-D release matrix and v1083/v1087 direct/transitive allocation-free
fatal-close analysis. PIPE/DPY/TTY/VM/u-area/swap/XIP/VM_NONE tests
and their negative ownership mutants PASS; no kernel change needed.
The full booted `P0D_FULL=1` matrix was NOT run, so A04/F02 stay open
alongside E13 and remaining F gates. Checklist 51/65; OOM OFF.

## 2026-11-05 v1160: S07/S08 frozen 256K DSK/D6FS candidate

Two fresh full 256K KCC PDP-6 E13 guests PASS physical requester BRK
OOM under a real DSK270/D6FS root and foreign provider lease, both
production-OOM-disabled and disposable-enabled. Original in-flight
owner resumes on its private stack and releases the lease. Both
guests show 160 independent spinner advances, zero errors and exact
WAIT/FILE/MM cleanup; only enabled policy selects requester slot 3.
Complete SIMH/counter/build/boot/map/provenance evidence archived at
`$TMPDIR/daimos-e13-release-media-v1160.qel3Zm.tar.gz`.

S07/S08 define this exact 256K disk-root, CTY headless, 60 Hz PCLK,
writable-RUN/EXEC OOM acceptance profile. Strict 32K DSH, alternate
boot/root media, raw-XIP process admission and GUI/multiTTY acceptance
are explicitly deferred, not removed from DAIMOS. v1123 source guard
PASS. 51/65 checks complete; E13 other simulator/media coverage and
final A04/F02-F11 acceptance remain. No permanent code/data/ABI
growth and production requester-OOM is still OFF.

## 2026-11-05 v1159: E12 protected-process OOM regression PASS

Two full PDP-6 256K physical BRK-OOM guests PASS enabled and
disabled requester-only policies. The v1159 testkit reads actual
process scheduler states at the OOM decision and verifies live
INIT, blocked witness, gateway waiter, unrelated spinner and
in-flight filesystem owner. The original provider holds `7,,0`
through the fault and later releases on its private continuation.
Only the faulting requester slot 3 reaches the fatal branch when
enabled (one selection); disabled records zero selections and BRK
-1. Both runs record 160 spinner advances and zero integrity errors,
with exact existing FILE/WAIT/MM cleanup PASS. This covers protection
of nonrequesters, not INIT as a faulting requester. E12 closed,
49/65 checks complete. No resident production code/data/ABI growth;
requester-OOM remains OFF until all activation gates pass.

## 2026-11-05 v1158: E11 separate RUN parent and real BRK OOM PASS

Both enabled and disabled full 256K PDP-6 guests PASS. A live
independent RUN parent forces genuine MM descriptor exhaustion,
observes real -3 child admission failure and exact rollback, and
waits while another child encounters actual physical BRK OOM.
The enabled requester alone is terminated; the disabled requester
gets -1. The RUN parent survives, exits and is reaped with exact
process MM/slot accounting. Testkit v1158 uses only a disposable
kernel copy. E11 closed, 48/65 mandatory checks complete; E12/E13,
release acceptance and RAM budgets remain open. Production OOM OFF.

## 2026-11-05 v1157: E10 APR/EXEC/RUN provider-contention PASS

Independent PDP-6 full guest rebuild PASSes the testkit v1157r11
scenario with a genuine held foreign filesystem-provider lease,
asynchronous APR fatal during post-commit EXEC FILE failure,
unpublished child RUN rollback and a physical u-area MM_BUSY.
FD3 survives staging until real foreign-lock refusal; the original
provider owner resumes and unlocks, and final WAIT/FILE/VM ownership
is exact. Recorded 6 VFS calls, 5 refusals, 7 FILE passes, 6 sleeps,
one physical pin/busy/unpin and zero errors. E10 closed: 47/65
mandatory checks complete, 18 open (16 pre-enable plus F12/F13).
E11-E13 and final RAM/size gates remain. Production OOM OFF and
no resident production code, data or ABI changes.

## 2026-11-05 v1156: OOM shortage classification E09

The frozen full-system v1156 error-class matrix PASSes real physical
BRK OOM (-1), fragmentation (-2), descriptor exhaustion (-3),
invalid/wrong-owner (-4) and MM_BUSY (-5), plus authentic full
backing-store bitmap refusal and recovery. Both production-disabled
and disposable requester-OOM-enabled guests preserve the requester
across backing exhaustion and accept subsequent ordinary BRK.
The production allocator and error codes are unchanged; no permanent
RAM/ABI change. E09 is closed, E10-E13 and final F gates remain open.

## 2026-11-05 v1155: E08 process ownership under concurrent sessions

The disposable v1155 full PDP-6 guest now checks two independent
sessions plus two concurrently live two-member groups across
STOP, CONT, fatal group events and real parent CHLD/WAIT, followed
by 64 numeric PID reuse lifetimes with stale WAIT/TERM rejection.
The foreign sessions remain active until individually terminated
and reaped. Final process and FILE accounting matches the baseline.
Two full-system v1155 guests and the original v979 control PASS.
E08 is closed without any production KCORE/MRES, persistent RAM,
ABI or OOM policy change. E09-E13 and final F gates remain open.

## 2026-11-05 v1154: E07 PI6 progress across physical release retries

The v1153 isolated full PDP-6 guest proves 55 actual PI6 clock ticks
and three independent user sleeps after a physical PIPE MM_BUSY and
before final FILE/PIPE/DPY/VM/u-area publication with a held foreign
provider. The v1154 guest independently proves 17 actual PI6 ticks
and eight independent user sleeps between the first refused and the
second successful genuine DISK bitmap verifier calls; all original
v1122 PIPE/DISK ownership and WAIT/TTY checks PASS. v1148 covers
canonical TTY and VM/u-area under an actual HUP continuation (32
real PI6 ticks and 48 bystander sleeps). E07 is accepted; the
remaining E08-E13/F and resident RAM proof are mandatory. No
production KCORE/MRES code, state, or ABI changed; OOM stays OFF.

## 2026-11-05 v1151r1: 32 repeated true physical-OOM cycles PASS

The opt-in enabled and production-disabled 256K full PDP-6 testkit
v1151 guests each complete 33 actual physical BRK shortage events,
32 subsequent successful RUN/BRK recoveries, 256 PID lifetimes,
512 stale wait/event rejections and at least 1024 qualified PI6
ticks. Exact process, FILE, swap and non-cache MM descriptor counts
remain stable. The sole intentional 0201-word, owner-6 clean BCACHE
MM slab may become resident after a FILE read, but a subsequent real
physical-OOM reclaim must free it; this is verified through a
test-only read-only probe of the actual BCACHE base. Raw descriptor
count alone cannot distinguish that bounded cache from a resource
leak. E05/E06 accepted, E07-E13 and F acceptance remain open.
No resident production KCORE/MRES/ABI growth or OOM-policy change.

## 2026-11-05 v1150: actual physical OOM under FS-provider contention

The new testkit `test-daimos-oom-provider-held-v1150` and its
disabled-control target each PASS a fresh, disposable 256K PDP-6
full-system guest. Real BRK exhaustion reports cause -1 while a
foreign filesystem provider remains leased to slot 6 (`7,,0`) and
slot 4 sleeps on the pre-dispatch wait event. The requester has a
live ordinary FILE requiring close. The opt-in OOM guest retires
only the requester, retaining the FILE and private owner across
seven foreign-provider busy attempts; the disabled guest instead
returns BRK -1 and waits during normal EXIT. The original holder
alone later resumes the provider's unlock path. Each guest records
160 independent spinner advances, surviving witness, exact WAIT
and stable MM/slot accounting, with zero integrity errors.

B06 is closed; 41/65 mandatory checks are complete and 22
pre-enable blockers remain before F12/F13. No changes to production
KCORE instructions, MRES code, permanent data, ABI or default-OFF
requester-OOM policy were made. E/F endurance and RAM acceptance
remain open.

## 2026-11-05 v1149: B07 unbounded owner retry safety

Current code retains a private fatal FILE/HUP owner across all
failed close intervals. Delay doubling stops at 01000 ticks but
the retry jumps back without a forced termination threshold. A
foreign filesystem-provider lock belongs to its original in-flight
caller and returns BUSY to an unrelated fatal close; the slot-0
finalizer retains failed owner state. Testkit v1149 rejects eight
mutations that violate these constraints, and native v885/v1102
owner/reaper retry tests PASS. B07 is closed as a safety contract,
not as a promise of eventual completion. B06/E/F remain open,
production requester OOM stays disabled; KCORE size is unchanged.

## 2026-11-05 v1148: live PI6 across deferred HUP and owner cleanup

The v1148 integrated disposable 256K PDP-6 guest PASSes twice with
one original-session child, a separate-session runnable bystander,
and a physically pinned controlling TTY. The leader's failed first
EXIT leaves the session attached while a remote HUP is deferred to
the child's private continuation; its one owner-return HUP and
terminal WAIT are checked. The leader's second EXIT then encounters
the actual VM/u-area MM_BUSY refusals and preserves exact ownership
through slot-0 retries and final one-time completion. PI6 advances
32 real ticks after the physical busy phase and the independent user
sleeps 48 times in each rebuild. The v1098 retry-clock mutation
checks PASS. This closes C04, but NOT B06/B07 or E/F stress/size.
No production KCORE/MRES/RAM/ABI modifications; OOM remains OFF.

## 2026-11-05 v1146r3: exact-once D16 matrix completed

Nine frozen-candidate PDP-6/native regressions PASS across all chosen
physical release retries: FILE, PIPE, DPY, TTY, process VM, u-area,
swap bitmap and backing record, failed unpublished RUN and APR/EXEC,
and provider-lock ownership. Explicit owner counts and publication
order are correct, with no unexpected MM owner/base alias or premature
WAIT. The default-OFF versus isolated requester-OOM-ON variants both
PASS 24 successive process lifetimes, PID reuse, stale WAIT and TERM
rejection, and exact MM/slot recovery. D16 is accepted for the
present writable-process scope; scheduled XIP remains excluded.

Checklist: 38/65 mandatory complete, 25 pre-enable blockers still
open, and F12/F13 cannot run yet. C04, genuine sustained E/F faults
and RAM/signoff are separate. All source and image revisions are fixed
in testkit v1146 logs. The production OOM flag remains OFF; no
resident KCORE/MRES RAM or ABI change occurred in this stage.

## 2026-11-05 v1145-v1146: verified D16 components, not yet D16 signoff

Testkit's repaired v1087 full release graph/native suite PASSes on
current production assembly, including the real -5 DPY return and
fatal-owner HUP/FILE retry. A fresh 256K v1122 guest PASSes a
separate physical PIPE busy/release plus second-member disk backing
bitmap refusal/restore with normal terminal WAIT/TTY, one swap detach
and zero integrity errors. A revision-locked v1146r1 sequential
full D16 matrix is staged but not executed end-to-end. D16/C04/E/F
and dynamic physical RAM measurement remain open, requester OOM OFF.
No kernel/MRES instructions, data, ABI or permanent RAM changed.

## 2026-11-04 v1144: D14 physical PIPE/DPY/VM/u-area combination

The new testkit v1144 full 256K PDP-6 guest combines the original
v1118 held foreign FS provider, sleeping waiter, FILE retry, HUP
refusal, process VM and u-area failures with a live Type-340 DPY
raw list. The final variant replaces the older test-only synthetic
PIPE/u-area -5 returns with actual MM pins. PIPE, DPY, VM and u-area
each independently show one real pin, one MM_BUSY, one unpin and
one physical free, with four distinct allocated bases. VM detach
occurs once. The test verifies FILE/PIPE/DPY resources were released
before VM teardown, and VM/u-area release was complete before the
slot-5 ZOMB could become parent-WAIT-visible. Pending WAIT(NO-HANG)
returns zero, final WAIT/CHLD is exact, 48 stale WAIT/PID/event
requests are rejected, the provider lease stays held and an unrelated
spinner advances 37 times. All fault-hook errors are zero. **D14
PASS** in this isolated guest; D16/C04/E/F remain open.

The independent original v1118 mixed-fault full guest was also
rebuilt after the v1144 testkit changes and PASSed its legacy
PIPE/VM/FILE/HUP/foreign-provider, WAIT/CHLD, 48 stale-request and
bystander assertions. The aggregate v1087 transitive-release suite
PASSes after native test fixture ABI maintenance; its in-flight
provider caveat remains unchanged.

All new allocations and witnesses are disposable-test-only. No
production KCORE code or resident data, MRES, ABI or OOM switch has
changed. Do not use this isolated 256K result as strict 32K or
sustained runtime-RAM acceptance.

## 2026-11-04 v1143: D15 excludes unscheduled XIP process launch

The current scope verifier v1123 PASSes production RUN/EXEC writable
startup and all six unsafe mutations. Fresh source-linked PDP-6
regressions v1108 (MEMFS MRES XIP/MM ownership/refusal) and v1109
(VM destroy through XIP provider, retry and VM_NONE) both PASS,
including rejection of their unsafe mutations. XIP provider release
support is real, but an XIP process is not admitted by ordinary
production EXEC/RUN. Accordingly, D15 is accepted for the current
requester-only BRK OOM scope with scheduled XIP processes excluded;
the separately required trusted XIP process-launch work X03-X08 is
NOT implemented or waived for DAIMOS release. The OOM gate remains
OFF; no kernel code, KCORE/MRES RAM or ABI changed.

## 2026-11-04 v1142: D13 physical unpublished RUN u-area rollback

The disposable 256K PDP-6 nested APR/EXEC/failed-RUN test now
executes a genuine physical MM_BUSY (-5) on the unpublished child
u-area. The test pins the child's actual kernel-dynamic MM extent
immediately before its first attempted free; the resident rollback
retains the slot, and an explicitly verified unpin permits the
existing deferred finalizer to release the same extent later.
Two fresh guest builds each confirm two real target MM-free attempts,
one pin, one BUSY refusal, one unpin and zero fault-hook errors,
together with the APR fault,
EXEC VFS FILE refusals/retry, parent RUN return and final WAIT.
This fills the previously missing physical-u-area D13 subcase.

The production allocator, rollback path and scheduler are unmodified;
the six diagnostic words are present only in the disposable test
kernel. No new permanent RAM or ABI, no production OOM activation.
D14-D16, E/F and the total RAM high-water audit remain open.

## 2026-11-04 v1141: compiler/linker fold correction repairs nested RUN

The real defect behind the nonreturning D13 nested RUN was an unsafe
cross-object local TEXT relocation comparison in the installed host
`p10fold` and `dlink`. The old plan incorrectly merged the two-word
`proc_discard_ok` (`SETZ 1; JRST proc_discard_return`) with an unrelated
two-word block from `syscall_dispatch_pdp6` because both local JRST
addends had the same numeric offset in different objects. The target
addresses after linking were not equivalent. This is a toolchain bug,
not an OOM teardown or `mm_free` bug.

Both source tool trees now require same-object identity for local
section references outside the matched TEXT span, and DLINK independently
rejects forged invalid plans. Testkit v1141 passes against both new
tool trees; the old P10FOLD and DLINK each fail the negative control.
Corrected binaries were installed in PDP10_PREFIX from `local.tar.gz`.
The fresh uninstrumented 256K nested APR/EXEC/failed-RUN guest now
PASSes (`late_run=1`, `finish_zombie=1`, `error=0`, parent return and
WAIT); the historical v890 EXEC/FILE/WAIT guest and native v1129
sentinel regression also PASS. The offending fold is absent from
the new link plan (28 instead of 29 folds in this guest profile).
The independently rebuilt v197 RUN rollback control also PASSes
16 post-VM fd-map failures with three live children, and a second
fresh, uninstrumented nested D13 boot reproduces the corrected PASS.
For this same full-guest profile, `__kcore_low_end` moved from
`023411` to `023413` octal, an explicit two-word resident TEXT
increase required to remove the incorrect two-word fold. No data
words were introduced; total runtime RAM high-water was not measured.

No production kernel source, resident table, ABI or OOM switch was
modified. The combined physical MM_BUSY unpublished RUN acceptance,
broader D/E/F checklist and RAM measurement remain outstanding;
production OOM stays OFF.

## 2026-11-04 v1140: breakpoint-sensitive D13 completion

An additional fresh 256K full-system trace placed a breakpoint at the
instruction immediately following `proc_uarea_release`'s `mm_free`
call and at the discard return labels. Unlike the earlier v1138
stage trace, the new debugger-interrupted guest reached MM return,
`proc_discard_ok`, discard return and `proc_run_return`, and later
published EXEC ZOMB=1. The tested production instructions were not
changed. This is evidence of timing/debugger sensitivity, **not**
proof that the failing uninstrumented D13 case is fixed. The test
still reports diagnostic-only; do not count a passing instrumented
run as release or OOM-activation acceptance. Investigate the
timing-dependent parent/child scheduling or MM ownership boundary
using stable before/after records and an uninstrumented control.

## 2026-11-04 v1139: child u-area rollback enters real MM free

A fresh v1138-derived 256K diagnostic with an additional simulator
breakpoint at linked `mm_free` (002045 octal) PASSed. The late RUN
failure reached child `proc_discard_retry_uarea_v892`, entered
`proc_uarea_release`, and entered `mm_free`. MM calls also occur during
other process lifecycles in this combined guest, so counts without a
before-UUO slot/stack correlation must not be treated as retries of
the failed child. An attempted direct reuse of an earlier writable
guest image instead stopped at `?RT`, and is explicitly discarded as
evidence. D13 remains open pending release-return/stack verification.

## 2026-11-04 v1138: unchanged-kernel RUN discard stage trace

The testkit's new v1138 SIMH breakpoint-only diagnostic ran against a
fresh disposable 256K PDP-6 guest. It reached the late
`proc_run_claimed_bad`, `proc_slot_discard`, new/live descriptor branch,
TTY/VM rollback, `proc_discard_vm_done`, `proc_discard_retry_uarea_v892`,
and `proc_uarea_release`. No `proc_discard_ok`, discard return or late
RUN epilogue was observed before the invalid-UUO trap. Some later
u-area-release breakpoints are post-trap cleanup for other slots, not
proof of a retry loop in the failed child. The test is diagnostic-only;
it does not accept D13 or enable requester OOM termination.

The next source-level check is the MM-free return and the descriptor/
parent-stack state immediately before and after child u-area release.
Retain separate PC samples before the UUO and post-trap cleanup, and
check the actual release return before changing production code.

## 2026-11-04 v1139: u-area MM and slot-trim boundary

The extended disposable 256K D13 guest again reproduced the invalid
UUO without modifying linked production instructions. Simulator-only
breakpoints show the late RUN's `proc_uarea_release` reaches `mm_free`,
then `proc_runq_remove` and `proc_trim_high`. The trace does not hit
`proc_discard_return` or the late `proc_run_return` before termination.
Other MM/u-area releases occur elsewhere in the test, so neither a
double-free nor an MM refusal is established. The remaining investigation
is the return/interrupt boundary after slot trimming. OOM OFF; D13 OPEN.

## 2026-11-04 v1138: unchanged-kernel nested RUN discard trace

The disposable 256K PDP-6 diagnostic ran one-shot SIMH execution
breakpoints at post-fold addresses derived from the current build's
assembler labels and fold map. Production kernel instructions were
unchanged. The late RUN entered `proc_run_claimed_bad`,
`proc_slot_discard`, the new-discard and live-slot branches,
TTY/VM release, `proc_discard_vm_done`, and
`proc_discard_retry_uarea_v892`. It reached `proc_uarea_release`
but did not hit discard completion or the corresponding RUN return
before the original invalid UUO. Earlier RUN returns and unrelated
u-area releases were also sampled. Do not infer an MM error code or
allocation failure from the incomplete trace. D13 remains OPEN;
requester OOM stays disabled and resident code/data remain unchanged.

## 2026-11-04 v1134-v1137: diagnostic-only nested RUN return isolation

Fresh disposable full-system PDP-6 tests verify unreachable-text
placement does not fix the D13 combined APR/EXEC/RUN invalid-UUO.
Simulator-only snapshots show the parent enters the late claimed-child
rollback with an intact saved monitor return, stack pointers and
slot 1; it never reaches that same RUN's epilogue before the fault.
Existing earlier successful RUN returns account for the two v1133
epilogue counters. The failure investigation now targets the nested
unpublished-child discard. No production code/data, ABI, MRES or
OOM-policy changes; full D13 acceptance and related D/E/F gates open.

## 2026-11-04 v1130-v1133: D13 fault isolation without resident changes

The exact v890 full guest and v1129 native double-VM_NONE-destroy
regression PASS against the production VM fix. Sixteen physical
late-RUN fdmap rollback cases under both inherited (v197) and NEW
(v1130) process groups PASS, with stable MM and recovered RUN/WAIT.
The combined D13 guest still fails before the late R user marker;
v1131 full rollback tracing changes its result to a clean `<EARH`
and exact terminal ZOMB, while v1132 BSS-only and v1133 two-counter
RUN-entry/return diagnostics still reproduce `<EA` plus the same
simulator UUO. All added counters are disposable kernel-copy test
instructions. Do not solve this with resident NOPs or padding;
trace the real syscall mapped-return/parent continuation first.
No production code/data/MRES change from v1130-v1133, OOM OFF.

## 2026-11-04 v1129: fatal EXEC VM_NONE double-destroy rejection fixed

The v890 post-commit EXEC fault revealed a genuine VM teardown bug.
`vm_space_destroy` was decoding the full-word `VM_SPACE_NONE` (-1)
as a nonzero RH physical base `0777777`, entering MEMFS-XIP probing
and an invalid MM free instead of allowing finalization. Owner-side
FILE closure actually completed, but PREVM retried forever and WAIT
could never observe the original child. A disposable stage probe
verified one `vm_space_destroy` failure with returned -1, zero TTY
failures, and no VM/u-area/zombie commit on the failing child.

The corrected production leaf recognizes exactly the full VM_NONE
sentinel *after* validating the process's storage mode. It bypasses
only the already-completed physical extent free, still calls normal
backing detach, and retains the standard publication/error checks.
This adds two executable instructions to the leaf and no new
persistent data or owner tables. The v1129 source-linked native
PDP-6 test PASSes two idempotent calls and rejects both unsafe
mutations. v1109 real MEMFS-XIP/VM and v880 swap retry also PASS.
The **original, unmodified** v890 256K guest, which previously
timed out at both 65 and 180 seconds, now PASSes with real VFS
FILE retry, final EXIT and parent WAIT. Normal production OOM
remains disabled. The expanded v1126 nested APR/EXEC/RUN acceptance
still fails independently before the late RUN marker (`<EA` plus
simulator UUO); D13, D14-D16 and P0-E/F remain OPEN.

## 2026-11-04 v1125: physical MM -4 with foreign provider lease

The full PDP-6 disposable guest combines an actual wrong-owner MM
descriptor with a foreign process holding the genuine serialized FS
lease. Seven physical `mm_free` refusals retain the original process
u-area and pending WAIT; a distinct slot-4 provider process retains
its private kernel stack/lease `5,,0` during the refusal and through
HALT, while the scheduler advances the unrelated slot-3 spinner.
No forced close, forced unlock or premature slot reuse occurs. The
test-only kernel-copy assembly adds no production KCORE or MRES
words; default OOM stays OFF. D12 closed, D13-D16 and P0-E/F open.

## 2026-11-04 v1124: actual physical TTY/VM/u-area teardown chain

The full 256K PDP-6 v1124 guest PASSes three distinct genuine MM
pin-induced `MM_BUSY` (-5) releases during one normal last-session
EXIT. The first user EXIT rejects its owner-side HUP because the
real TTY canonical input extent is pinned, preserving the original
TTY/session and private process continuation. A second real EXIT
unpins that exact extent and completes HUP. The genuine process VM
then fails MM release while pinned, retains PREVM/VM ownership, and
is later unpinned by the actual clock-driven slot-0 reaper. The next
release finds a physically pinned u-area, retains VM_NONE/FINALIZE,
and the following reaper retry unpins/releases it exactly once.

The guest witnesses exactly 1 physical TTY/MM busy, 1 process VM
MM busy and 1 u-area MM busy; each has exactly 1 pin and unpin.
VM retained/destroyed=1, u-area retained=1, final process release=1
and zero test errors. No invented allocator refusal, replacement
owner or hand-invoked finalizer. The old standalone v969 TTY-only
fixture was updated to arm at owner-side HUP and independently
rebooted: PASS with 1 pin, 1 retained session and 1 unpin/retry.

D11 is now checked: **33 of 65 mandatory OOM items complete**, 32
unfinished, including 30 pre-enable blockers and two post-signoff
actions. D12-D16, sustained P0-E and P0-F remain OPEN. The OOM
production flag, KCORE/MRES code, resident memory and ABI are
unchanged. The `P0D_FULL` optional matrix now includes v1124.

## 2026-11-04 v1123: OOM activation gates made individually testable

The new top-of-DAIMOS-TODO v1123 checklist specifies 65 mandatory
tasks for requester-only BRK OOM activation (32 checked, 33 OPEN;
31 pre-enable blockers and two post-signoff actions)
and eight separately optional MEMFS raw-XIP RUN/EXEC admission tasks
(two checked, six OPEN). It retains v1087 allocation-free transitive
reclaim and v1086 PI6 non-sleeping slot-0 proofs, along with v1092
provider lifetime safety through fail-closed quarantine. A forever-
stalled provider is not promised eventual cleanup or reclamation.

The remaining immediate release path is physical TTY+HUP/VM/u-area,
mixed APR/EXEC/RUN rollback under held provider faults, sustained
independent process progress and exact MM/WAIT/PID/bitmap accounting
under repeated true exhaustion. Final enabled/disabled test matrices
and real 16K executive/MRES residency measurements precede changing
`PROC_OOM_BRK_ENABLED_V1000,0` in production.

Raw MEMFS XIP admission is deliberately outside current normal
RUN/EXEC: both paths use `vm_space_startup`, which writes startup
metadata even with zero args/environment. The v1123 source guard
confirms that no normal loader calls op 027; six negative mutations
are rejected. The existing native provider/MRES XIP unpin/VM retry
regressions remain applicable. Any new opt-in XIP process launcher
must meet its separate trust/no-writing/swap/BRK and scheduled
teardown requirements and requalify OOM before being enabled.

The full source-linked v1096 P0-D matrix PASSes on current code;
the v1123 checklist inventory audit structurally PASSes but its
readiness check fails closed on 31 open pre-activation IDs. All production
KCore/MRES memory, ABI and disabled OOM behavior remain unchanged.
The v981/v999 policy and BRK-cause checks, v1113 real MM -2/-3
regression, v1092 provider quarantine with 14 negative mutants,
and v1086 PI6 non-sleeping closure audit independently rerun PASS.

## 2026-11-04 v1122: physical PIPE refusal plus real DISK bitmap failure

A disposable 256K PDP-6 full guest now exercises *both* actual
release errors in the same session-HUP teardown, with distinct
process owners. First-member slot 3 owns an ordinary private PIPE;
the first HUP reaches its genuine final FILE/PIPE MM release and
gets physical MM_BUSY (-5) from a test-only MM pin. The first HUP
returns -1 without discarding ownership. On the next HUP, the
original first member is revisited once, unpins/frees that exact
PIPE and commits. The independently owned second member (slot 4)
then performs a real SWAP-MRES-backed DISK swap-out. Its actual
checked bitmap verifier refuses because the test has temporarily
cleared one allocated backing bit. The next slot-0 PREVM retry
repairs the bit and releases the original DISK extent/record once.

Full-system counters PASS: PIPE calls=2, one pin, one MM_BUSY, one
unpin and one true free, zero owner errors; first member HUP
repeats=1; one real swap-out; verifier calls=2, one actual bitmap
fault and one exact repair; one backing free, zero swap errors,
zero final DISK record/blocks. The pinned first-member u-area,
intermediate nonwaitable leader, three leader HUP attempts, final
leader/member WAIT statuses and TTY handoff all PASS. This is the
genuine DISK-bitmap overlap absent in the v1121 same-member DPY
swap-out/restore path. No fake MM/bitmap return code is used.

The test-only source generator composes the existing v1104 and
v1107 kernel hooks and keeps the single-provider modes unchanged.
No production KCore/MRES code, RAM, process ABI or OOM activation
changed. P0-D now has physical PIPE+DISK bitmap cross-owner
evidence, but booted MEMFS XIP, prolonged provider contention,
full P0-E liveness and P0-F activation remain OPEN.
The original v1104 PIPE-only and v1107 physical bitmap-only full
guest regressions were independently rebuilt after the combined
test-generator update and PASS their original exact assertions.

## 2026-11-04 v1121: physical DPY release across DISK swap restore

The disposable 256K PDP-6 v1121 guest combines a real Type-340
display-list MM_BUSY (-5) on final FILE close with an earlier actual
DISK swap-out of the *same* session member. The first mixed guest
reached a clean HALT, but its inherited swap-only assertion falsely
required zero second-member HUP retries and two bitmap-verifier
calls. The trace proved an additional second HUP was necessary while
the private DPY FILE close remained incomplete, and the DISK bitmap
verifier was never reached. The test was not accepted as a bitmap
fault on that evidence.

The corrected guest instruments the actual SWAP MRES `proc_swap_in`
entry and successful return using disposable KCORE counters. One
physical swap-out, one successful swap-in, one backing free, and a
cleared terminal DISK record/block count PASS. The DPY extent records
two frees with exactly one physical pin-induced MM_BUSY, one unpin,
one true free, zero ownership errors, 118 actual PI6 refreshes, and
90 refreshes while the old display list remains held after refusal.
Session HUP partial-commit, the first member's MM pin/unpin, the
second member's later HUP, final WAIT and TTY ownership PASS.

This is physical SWAP/DPY release integration, NOT a simultaneous
DPY MM_BUSY and DISK bitmap-detach refusal. A private FILE-close
continuation requires the swapped process to become resident before
VM teardown, consuming the DISK record. The separate v1107 actual
bitmap-verifier refusal remains its own regression. Live MEMFS XIP,
concurrent DISK bitmap errors with a distinct active owner and full
P0-D/E/F acceptance are still OPEN. No production code, persistent
KCore/MRES RAM, ABI or OOM switch changed.
The original v1107 swap-only bitmap fault fixture was independently
rebooted after v1121 and PASSes: two verifier calls, one forced bitmap
failure, one exact restoration/free and no terminal DISK leak.

## 2026-11-04 v1120: real numeric PID reuse following physical OOM

Two independent booted 256K PDP-6 guests PASS with genuine BRK
physical-memory exhaustion, one with ordinary production-default
OOM disabled and one with requester OOM enabled only in a private
test image. After the exhausted requester completes (returns -1 when
disabled, is fatally retired when enabled), the unrelated blocked
witness runs and both children are reaped. Parent process slots and
MM words return to the original baseline.

The same parent then RUNs and reaps 24 new children with alternating
distinct exit statuses 062/063 octal. All 23 consecutive child pairs
reuse exactly the same numeric PID, demonstrating real slot recycling
and proper WAIT identity across lifetimes. Between children, stale
WAIT and PID-directed TERM return -1; process slots and process words
remain exactly at the post-OOM baseline after every reap. A PID is an
8-bit process slot, not a persistent generation number: an old PID
may validly identify a different child after reuse, so no stale event
is sent while that number is assigned to the new process.

The unmodified v1109 native XIP/VM release/repair tests and v1087
fatal allocation-free teardown audit also PASS. This closes the
sequential numeric reuse subgate under both OOM policies, but not
concurrent multi-provider fault/liveness acceptance. No production
KCore, MRES, persistent storage, syscall ABI or default policy changed.
P0-D/E/F remain OPEN; production requester OOM remains disabled.

## 2026-11-04 v1119: physical PIPE and Type-340 DPY in one session HUP

The full 256K PDP-6 v1119 disposable guest gives the same second
session member both a real PIPE and a published raw Type-340 display
list. Physical MM free faults for the distinct PIPE owner 5 and DPY
owner 014 are independently armed at the same HUP delivery and
composed at MM entry/busy/delete. Each exact extent records one pin,
one MM_BUSY (-5), one unpin and one true deletion on retry, with no
ownership errors. DPY PI6 refresh runs 14 times, eight during the
held-list interval after the first physical refusal.

The first member stays pinned while the session HUP is incomplete;
the leader/member WAIT and TTY handoff PASS after retry without
duplicate HUP. Two fresh guest boots PASS, including the final C89
test-source rerun. Combined-mode injection required a test-only
anchor-composition fix; no production kernel/MRES/ABI was touched.
This closes a physical PIPE+DPY session-release cross-product, not
the outstanding simultaneous XIP/VM/swap bitmap and PID-reuse cases.
P0-D/E/F remain OPEN and requester OOM stays disabled.

## 2026-11-04 v1118: simultaneous physical PIPE and VM release refusal

A disposable full 256K PDP-6 guest now composes the actual PIPE
MM_TYPE_KERNEL_DYNAMIC and process-VM MM_TYPE_PROCESS free hooks in
one `mm_free` entry, preserving both production owner/type checks.
One PIPE and one VM MM_BUSY (-5) occur during the same fatal process
teardown. VM pin/unpin and one true MM delete, swap detach, PIPE
owner retention and exact second close, two HUP-stage calls/one
injected refusal, FILE close retries and the final u-area retry PASS.
One EXIT/CHLD/finalization, intact foreign FS lease, 37 bystander
cycles and 48 rejected stale WAIT/PID events after reap all PASS.

The first composition attempt exposed an incompatible test-only
injection anchor, repaired in daimos-testkit v1118 before building.
No production kernel/MM code, KCore/MRES allocation, or ABI changed.
This is a completed *physical PIPE plus physical VM* P0-D subset;
HUP refusal is synthetic, and concurrent DPY/XIP/swap bitmap faults,
actual numeric PID reuse and full P0-E/F acceptance remain OPEN.
Production requester OOM remains disabled.

## 2026-11-04 v1117: booted fatal teardown plus stale PID/WAIT checks

The extended v1103 disposable PDP-6 guest executes real PIPE MM_BUSY,
FILE/HUP-stage/VM/u-area refusal and retry while keeping an FS lease
and unrelated runnable bystander. After the dying child is reaped,
48 repeated stale WAIT and PID-directed TERM requests return -1,
with process-slot and MM-word counts never increasing above the
post-reap baseline. The original finalization, provider, and
bystander counters also PASS. No production code, data, MRES or
default OOM change. Numeric PID reuse and the larger P0-D/E fault
matrix remain open; v1117 is an integrated liveness subset only.

## 2026-11-04 v1116: direct booted RUN -3 MM cause verification

v1116 uses private booted PDP-6 kernel copies to observe real
MM allocator return codes at the process-VM and child u-area RUN
stages, while actual MM descriptor slots are exhausted. A valid
child RUN returns -1 with exactly one reached allocator reporting
-3; an unexecuted allocation has a distinct test-only -7 sentinel.
Both OOM-disabled and private OOM-enabled guests PASS the child-slot,
memory-owner, WAIT and independent witness checks. Only the later
true physical-OOM requester is retired by the enabled private policy.
Native v1111/v1113 MM classification and error-propagation controls
PASS independently.
No production kernel text, BSS, MRES slots or ABI changes. P0-D/E
mixed faults and P0-F policy activation remain outstanding; the
production requester OOM switch is still disabled.

## 2026-11-04 v1115: booted fragmented physical core and BRK -2

The full 256K PDP-6 guest constructs actual fragmented MM extents
using the production allocator and exact frees. A test-only export of
`mm_find_fit` confirms sufficient aggregate words for a 64K image,
with no fitting contiguous extent. The real VM BRK returns -1 with
exact internal cause -2; the disabled and disposable enabled-policy
boots both PASS without killing the fragmented requester, and the
later genuine physical-OOM requester behaves correctly. Every tracked
test extent is freed; parent MM accounting and witness survive.
An earlier 40K request succeeded via in-place delta growth, which
the larger target now excludes. The real BRK retry reclaimed one
unrelated descriptor, so no-leak accounting permits a decreased
global descriptor count, never an increase. No production KCore/MRES
memory change. Direct RUN failure cause and P0-D/E gates remain open.

## 2026-11-04 v1114: booted MM descriptor saturation / RUN rollback

An isolated 256K PDP-6 kernel test extension acquires actual one-word
MM_TYPE_KERNEL_DYNAMIC extents until `mm_extent_count` equals
`mm_extent_capacity`, then checks that valid child RUN fails without
killing its parent or leaving extra process words/slots. Each tracked
physical extent is freed through the normal typed MM release path.
OOM-disabled and disposable enabled-policy boots both PASS, including
v1110 BRK negatives, the physical-OOM requester and independent
witness. The RUN internal failure code is not captured; native v1113
already verifies exact -2/-3 VM/BRK propagation. No production text,
BSS or MRES changes. Booted BRK fragmentation and complete P0-F
pressure-classification acceptance remain open.

## 2026-11-04 v1113: VM and BRK preserve actual -2/-3 MM causes

The native PDP-6 regression now tests the entire production failure
chain from `mm_find_fit` and `mm_alloc_aligned` through
`vm_space_brk_current` to the opt-in fatal BRK dispatcher. Real
fragmented free gaps produce -2; a full MM descriptor table with a
fitting gap produces -3. Both return ordinary BRK failure while
preserving VM, logical break and descriptor ownership. Three unsafe
cause-losing source mutations fail. FS/swap/relocation and fatal
observers are stubs; full guest pressure and WAIT/PID checks remain
open. There is zero production text/data change and OOM remains off.

## 2026-11-04 v1112: booted negative failed-RUN case

The full 256K v1075 guest has an opt-in v1112 missing-executable RUN
case: the parent receives -1 without retaining a child slot or
changing process-MM accounting. It then passes the v1110 invalid
BRK cases, genuine physical-OOM requester, and blocked bystander.
Both disabled-policy and disposable enabled-policy PDP-6 boots PASS;
only genuine physical exhaustion kills the requester when enabled.
No production KCore/MRES/ABI changes. Booted fragmentation, MM
descriptor exhaustion and memory-pressure failed RUN remain open.

## 2026-11-04 v1111: actual MM allocator error classes

Source-linked native PDP-6 `mm_alloc_aligned_noreclaim` distinguishes
physical shortage (-1), fragmented free words (-2), and no free extent
descriptor (-3), with unchanged live extent ownership on failures.
Expanding only descriptor capacity permits the formerly refused small
allocation and preserves sorted extent order. Two unsafe error-code
collapses are rejected. The separate native v1000 BRK-dispatch gate
also PASSes. There is no change to production kernel memory or OOM
policy. Booted BRK propagation, fragmented/descriptor negative tests,
and P0-F completion remain outstanding.

## 2026-11-04 v1110: booted BRK classification with OOM disabled/enabled

The opt-in v1110 guest mode adds invalid-floor BRK `1` and address-
limit BRK `0777777` checks to the physical exhaustion v1075 guest.
Both fail with `-1` while preserving the original break and normal
child WAIT/slot/MM accounting. Booted PDP-6 disabled-policy and
disposable enabled-policy configurations PASS, including the separate
physical exhaustion requester and runnable witness. Only true physical
OOM terminates the requester in the enabled test. Production OOM stays
disabled, and production KCore/MRES occupancy is unchanged. P0-F is
still OPEN for fragmentation, descriptor exhaustion, failed RUN and
the final full-matrix activation decision.

## 2026-11-04 v1109: full native VM/MEMFS-XIP/MM teardown leaf

v1109 runs real `vm_space_destroy`, MEMFS direct-provider XIP
release and physical MM pin adjustment together on the native PDP-6.
An owned XIP whose pin state is invalid refuses unpin with -4; VM
keeps its original mapping and does not call process-type MM free or
the backing detach witness. After repinning the same MM extent, the
unchanged VM destroy releases XIP once, calls detach once, and commits
VM_SPACE_NONE. The foreign provider lease is untouched, with no wait.
Two unsafe branch mutations are rejected. Storage-mode, swap-detach
and forbidden process-type MM calls are fixture oracles, not full
production swap or real executable admission. The native P0-D matrix
PASSes; the optional full guest matrix was not rerun. This adds no resident
production code or memory. The booted MEMFS-XIP lifecycle and WAIT/PID
ownership combinations remain unverified; P0-D/E/F remain open.

## 2026-11-04 v1108: checked XIP MM pin-state retry

Source-linked v1108 executes the real MEMFS XIP release gateway and
MM pin adjustment. A live MEMFS XIP reference with a physically
unpinned MM extent causes `mm_unpin` to fail -4; the release returns
-4 and retains the ref. `mm_pin` repairs the exact extent, after which
XIP release succeeds and an additional release is harmless. Foreign
provider lease and slot-0 no-wait constraints remain intact. The
original v1086 and v1094 XIP tests and the expanded v1096 native
P0-D matrix PASS. There is no KCore/MRES growth.
This is a native leaf test, not yet the booted MEMFS-XIP lifecycle
cross-product required by P0-D.

## 2026-11-04 v1107: finalizer scheduler-RH state discrimination

`proc_finish_service_v881` must decode `PROC_SIDL` before testing the
SIDL-only `02` discard-VM marker in the scheduler RH. A non-SIDL
process uses the same RH as an intrusive run-queue successor, so
successor slot 2 is not evidence of pending SIDL discard. The prior
ordering could skip a live PREVM+FINALIZE owner entirely. The corrected
order retains ordinary FINALIZE retry and SIDL discard separately.
Source-linked v1107 positive/unsafe-mutant test PASS. A real 256K PDP-6
guest also PASSes: one real SWAP MRES DISK swap-out of 16 blocks,
an injected first backing-bitmap verifier refusal, one successful
verification and exact physical backing free on slot-0 retry, cleared
final DISK record and unchanged allocation baseline. The already
finalized session member is not HUPed twice; WAIT/TTY handoff PASS.
All v907/v1104/v1105/v1106 booted compatibility guests and the
native P0-D matrix PASS. The production full KCC image builds;
`__kcore_low_end` remains `023171` octal after the changed branch
sequence. No additional KCore BSS, MRES, process fields or persistent
data; requester OOM disabled. P0-D/E/F remain OPEN.

## 2026-11-04 v1106: actual process-VM MM refusal and finalizer retry

The v1106 booted 256K guest preserves v997's held FS-provider lease,
fatal FILE close retries, u-area MM_BUSY and continuously runnable
bystander, but replaces the first synthetic VM error with a physical
pin on the exact dying process's resident VM extent. Production VM
destroy takes its real XIP-provider classification path and calls
`mm_free` with MM_TYPE_PROCESS / owner 5, yielding MM_BUSY (-5).
The original VM descriptor and PREVM retry owner survive unchanged.
On the next slot-0 retry, unpin enables the same MM extent's one actual
deletion, followed by one production swap-detach stage and VM_NONE
commit. That swap record is empty; disk-swap failure is not covered.

Booted PASS: 2 VM and process-MM calls, one physical pin/unpin,
one -5 and one successful MM delete, one swap detach, 2 u-area calls
with one real MM_BUSY, 3 FILE calls/2 refusals, 36 bystander cycles,
zero provider errors. Original booted v997, combined booted v1103,
and native release matrix all PASS. Hooks are test-only;
KCore remains `023171` octal, no resident allocations or MRES growth,
production OOM disabled. P0-D/E/F remain open.

## 2026-11-04 v1105: physical Type-340 HUP/DPY-MM fault stage

The v1105 disposable booted PDP-6 guest extends the real two-member
session HUP test with a live Type-340 raw display list in the second
member. Its remote HUP drives FILE DPY CLOSE through actual DPY STOP
and the production MM allocator. The isolated fault hook physically
pins the exact 32-word DPY extent once; MM_BUSY (-5) must retain the
FILE descriptor, raw-list base/IOWD and PI7 refresh ownership.
The existing first member simultaneously retains a pinned u-area and
a committed VM_NONE+FINALIZE stage. Parent WAIT_NOHANG must not reap
the leader during partial HUP. Eight PI6 refreshes occur while the
DPY extent remains pinned. The second HUP unpins and frees the exact
original DPY extent once; first-member u-area and TTY release then
complete without repeated HUP or ownership errors.

Results: two DPY frees attempted, one MM_BUSY, one successful free,
one physical DPY pin/unpin, 14 raw refreshes total, zero errors.
Separate booted v1104 PIPE/HUP and original v907 HUP guests and the
full v1096 native release-stage matrix all PASS.
No production code, KCORE resident data, MRES or OOM enable changes;
`__kcore_low_end` remains `023171` octal. P0-D/E/F remain OPEN.

## 2026-11-04 v1104: booted physical member HUP and PIPE final-close fault

The v1104 daimos-testkit guest augments the real v907 two-member
session/TTY HUP scenario with anonymous PIPEs held by the members.
It pins the second member's exact PIPE MM extent on its HUP-triggered
last descriptor close, so production `mm_free` returns MM_BUSY (-5).
The FILE reference, PIPE writer count and owner are retained for the
second HUP. The first member independently owns a pinned u-area and
a committed VM_NONE+FINALIZE state: the second HUP must skip it.
The second PIPE is unpinned and freed once, then the TTY session
commits and the first member's u-area is unpinned and reaped.

Guest assertions: PIPE free attempts 2, pins 1/unpins 1, busy 1,
success 1, no duplicate HUP, no premature parent WAIT, intact HUP
status/TTY handoff and zero owner errors. Original v907 guest and
native P0-D matrix both PASS. The production kernel is
untouched; `__kcore_low_end` remains `023171` octal with no new BSS,
MRES or process fields. P0-D/E/F remain OPEN; OOM is still disabled.

## 2026-11-04 v1103: no production growth for mixed-stage fault test

The new testkit v1103 disposable 256K PDP-6 guest combines real
PIPE-owned MM_BUSY retry with two test-only FILE refusals, one HUP
stage refusal, a VM retry and the actual u-area MM_BUSY retry, all
for one fatal child. Both PIPE MM attempts preserve the exact last
reference count until free commits. The held foreign FS gateway
lease, WAIT/CHLD, one finalization and a runnable bystander remain
correct. The HUP-stage rejection is injected, not a physical member
HUP (independently tested by booted v907).
The original v997 booted baseline and the full native P0-D matrix
were independently rerun and PASS after the testkit extension.

No production kernel file, storage mode, process field, owner word
or MRES was changed; baseline `__kcore_low_end` remains `023171`
octal from v1102. P0-D/E/F and the production-disabled OOM decision
remain open pending more booted cross-stage coverage.

## 2026-11-04 v1101-v1102: correct capped fatal reaper backoff

A booted PDP-6 wrong-owner MM fault found that clearing the shared
`proc_finish_last_tick_v881` pending bit *before* slot-0 cleanup made
the v1098 prepublished-EXIT test classify every failed MM release as
the first. Permanent -4 or -5 MM errors thus retried at 020 octal
ticks forever instead of doubling to the 01000-octal cap.

v1102 leaves pending set through `proc_finish_service_v881`, recording
any failed owner in an unused bit of saved AC11. After scanning all
eligible slots, the pending bit is cleared only if none refused.
The old completion/last-user return value is restored before return.
The existing 36-bit retry-clock word continues to store pending,
delay and epoch. No new RAM, process, MRES or owner fields.

Source-linked PDP-6 v1102 proves the 020/040/0100 progression and
retirement, rejecting four unsafe variants; v881/v1098/native P0-D
matrix PASS. Booted 256K v1101 misowned-uarea and v909 pinned-uarea
quarantine, WAIT refusal and runnable bystander both PASS (seven
failed frees, six reaper invocations each). Full KCC image PASS:
`__kcore_low_end` `023162` -> `023171` octal, seven added code words
and zero permanent data words. OOM stays OFF; P0-D/E/F remain OPEN.

## 2026-11-04 v1100: typed DPY rollback without persistent state

The Type-340 `dpy_write_words` old-list replacement failure now
preserves the allocator's exact error (-5 busy/pinned or -4 invalid
MM type/owner). The existing allocation-result stack word carries
the old error across the separate `mm_free` of the unpublished new
list, avoiding a new data or owner word. Failed replacement still
keeps the original display list, IOWD and FILE descriptor; a second
failed free retains its original orphan-owner path. Native PDP-6
source-linked v874 and v1100 cases PASS, including two unsafe
mutation rejections. The v1100 full KCC image PASSes and retains the
same `__kcore_low_end` `023162` octal (no new KCore or BSS words).
P0-D/E/F remain open and production OOM is OFF.

## 2026-11-04 v1099: typed final PIPE/FIFO MM refusal

The last-reference PIPE/FIFO close now preserves the actual MM_FREE
return (-5 pinned, -4 invalid type/owner) rather than collapsing both
to -1. It leaves the FD, original counts and FIFO list ownership
untouched on failure; head reinsertion uses AC3 to preserve the
returned status in AC1. Normal successful closes remain unchanged.
This exposes the actual allocator result to FILE teardown diagnostics
without new tables or memory allocations.

Native source-linked PDP-6 v1099 checks both failure classes,
subsequent exact release, named FIFO link restoration and four unsafe
mutation controls. The v1096 component matrix and v1087 allocation-
free release audit plus eleven unsafe mutants PASS. Full 256K KCC
image builds, KCORE end `023162` octal unchanged from v1098, no new
static data. P0-D/E/F remain open; production OOM termination OFF.

## 2026-11-04 v1098: arm first PREVM retry after EXIT stack handoff

The v1086 slot-0 safety handoff publishes FINALIZE+PREVM before
discarding the private executive stack. That marker is NOT evidence
that `proc_finish_last_tick_v881` has a valid delay. On a first
`vm_space_destroy` refusal, the former `proc_finish_mark_pending_v892`
mistook the prepublished marker for a prior retry and set the global
pending flag with a zero interval; the idle reaper correctly never
entered that corrupted zero-delay state, stranding the process.

v1098 initializes a new 020-octal-tick first retry from the current
APR epoch if a marked finalizer has no global pending owner; existing
pending owners retain their backoff. The slot-0 reaper still rejects
a genuinely malformed zero delay, and no FILE/MM owner, PI6 mask,
private kernel stack, process record or permanent timer word changes.
Real PDP-6 v1098 native tests and three unsafe clock mutations PASS.
The previously failing v997 256K guest now finishes its two-stage
VM then u-area retries with correct CHLD, held provider lease and
independent bystander progress (36 cycles), without a forced release.
The v1077 256K requester-only physical-OOM regression also PASSes
with two D6FS SYNC refusals, one MM_BUSY and surviving bystander.
The production KCC full-profile build gives `__kcore_low_end` octal
`023156` -> `023162` (+4 resident code words), with no new BSS,
MRES exports, process fields, or durable timer metadata.

## 2026-11-04 v1096-v1097: u-area MM error typing, P0-D matrix

`proc_uarea_release` now propagates the real `mm_free` result on a
failed final u-area deletion: -5 (pinned/busy) versus -4 (wrong type,
wrong owner, nonexistent extent). A live-u-area flag with zero base
also reports invalid -4. The process table, stable u-area ownership,
MM allocation and finalizer retry stage remain unchanged on refusal;
release status zero means exactly one actual MM free or true absence.
The existing finalizer checks nonzero before publishing FREE/ZOMB,
retaining its original retry owner. No MM or new backing allocation is
introduced.

PDP-6 source-linked v1097 physical-MM tests and three unsafe mutation
controls PASS; the v1096 PIPE/DPY/HUP/VM/XIP/TTY/u-area component
matrix and the allocation-free fatal path audits PASS. Full production
KCC build succeeds: KCORE low end `023154` -> `023156` octal (+2
resident words), no new permanent state. The full booted v997/v1077
guest checks initially uncovered the v1098 PREVM clock defect; both
PASS after that correction. P0-D stays OPEN for untested mixed
resource faults, and production OOM termination remains disabled.

## 2026-11-03 v1093-v1094: P0-D typed release and XIP safety

The checked canonical TTY buffer reset now returns the real MM_FREE
error (-5 pinned, -4 invalid type/owner/missing allocation), while
the unchecked historical void reset delegates to the same release
leaf. Neither a busy nor an invalid free drops the line base or
changes its session; successful release commits once. Native PDP-6
physical MM tests and the full v898 pinned-session guest PASS.

MEMFS XIP release no longer conflates its *owned* unpin failure with
the -1 absent-MRES trampoline. Owned failure returns -4; VM destroy
does not attempt MM_TYPE_PROCESS free, detach backing or publish
VM_NONE until owner-side retry succeeds. Non-XIP status 0 and absent
provider -1 retain the legitimate process-MM fallback. Native v1094
tests exercise these states, backing refusal and four unsafe variants;
v1086 slot-0 PI6, v880 detach, v1081 PREVM and v1087 owner graph PASS.

Matched full KCC image end `023161` -> `023154` octal: five fewer
resident words, no additional data. P0-D remains OPEN for cross-stage
idempotence and permanent corrupt-owner diagnostics. Production OOM
termination remains disabled.
The 256K v1077 injected D6FS/MM physical-OOM guest also PASSes with
the expected two SYNC refusals, one MM_BUSY and surviving witness.

## 2026-11-03 v1092: fail-closed provider ownership (P0-B CLOSED)

P0-B adopts a safety contract, not an unsafe generic recovery scheme.
If an in-flight DTFS/D6FS/TSFS provider never returns, its encoded
lease, caller's private kernel continuation, FILE/MM ownership and
PID are quarantined in place. Only that provider's real return may
unlock the shared workspace. A pre-dispatch waiter may abort on a
pending fatal event (v1090/v1091); an unrelated normal waiter can
remain blocked. There is no automatic unlock, forced wakeup or PID
recycling. Controlled restart or positively proved provider-specific
quiescence is the only supported response to a truly unreturned call.

For a returned but permanently refusing provider CLOSE, the existing
fatal owner continues its capped 01000-octal-tick private retry,
retaining FD and GETTEARDOWN diagnostic status until the provider
accepts closure. This caps retry CPU cost, not time to completion.
Native v1092 source contract rejects 14 unsafe mutants. Full-system
v891 permanent rejection, v953 repaired-provider revival, and v1091
held-provider/foreign-waiter containment PASS. No production source,
KCORE, MRES or process table growth at v1092: KCORE `023161` octal.
Booted v914 permanent invalid -4 refusal also PASSes (eleven attempts,
FD/ERR owner held, no premature exit), complementing v891's -5 case.

This closes the P0-B *resource lifetime safety* contract. It does not
establish bounded reclamation of unresponsive resources, nor authorize
automatic requester OOM termination: P0-D/E/F are still open and
the production OOM gate is disabled. Reproducible provider bugs still
require fixes in their owning modules, not forced generic cancellation.

## 2026-11-03 v1090-v1091: safe fatal wake of FS lease waiters

The fatal-event path now distinguishes an owner already executing in
the serialized DTFS/D6FS/TSFS provider from an independent process
waiting *before* gateway dispatch. The waiter receives the normal
pending fatal bit and an exact-address/exact-wait-class targeted wake;
its resumed request returns BUSY on its original private stack without
acquiring the held lease. At user return the ordinary fatal path
closes owned descriptors and reports the correct WAIT status.

Other wait channels, WAIT_CHILD, WAIT_INTR, STOP and live provider
owners are not interrupted. Already established fatal-close owners
may still enter a free provider despite old pending fatal bits.
No new state word, process slot, lock, MM allocation or forced release.
Native v1090 plus four negative controls and full-system v1091 PASS:
one original provider lease retained, the foreign waiter exits after
TERM, an independent spinner completes 73 cycles, zero errors and
correct parent WAIT; old v1087 transitive no-allocation checks PASS.
Matched full-image KCC KCORE end `023127` -> `023161` octal (+26 words),
no persistent data growth. P0-B remains open for genuinely nonreturning
executing providers and permanent refusal; production OOM stays OFF.
The updated 256K v1077 physical-OOM requester/D6FS/MM guest PASSes
with the expected two SYNC and one MM_BUSY refusals and correct
independent witness recovery.

## 2026-11-03 v1088-v1089: P0-B in-flight ownership and tape progress

The production filesystem register gateway's lock cannot be revoked
behind an executing provider: it protects shared block workspace and
private kernel continuations. Native v1088 enters the *actual* gateway
and tests nested same-owner refusal, foreign fatal-close refusal and
owner-only unlock on return, with two unsafe negative variants rejected.

Source audit exposed a concrete missed-completion hole in the optional
Type-136 tape driver. Its DTC search and MTC control operations had
clock-driven bounds, but their runtime data-transfer wait did not.
v1089 adds an APR-tick tape MRES watchdog and uses the existing DTC
search retry word after search to count data-transfer ticks (300 for
DTC; 5400 for MTC). When expiry occurs, it marks the tape I/O failed,
stops the controller and wakes the *original* request owner. Normal
owner-side return, not timer-side lock theft, releases the provider.
No additional BSS or process metadata is needed.

Native timeout/negative-mutation, DTC stream/reverse I/O and MTC
record/control regressions PASS. Clean full KCC 256K build PASS;
full-media SIMH reports DTC and MTC OK and reaches INIT/LOGIN before
the intentionally bounded boot check ends. KCC KCORE end is `023127`
octal, one instruction above v1087 `023126`; no new permanent state.
The tape module itself measures 375 -> 405 resident words (+30), with
15 BSS words unchanged, for +31 combined resident words after KCORE.
The updated full 256K v1077 physical-OOM requester/D6FS/MM fault guest
also PASSes with two D6FS and one MM_BUSY refusals; the production
OOM-kill build flag is not changed.

P0-B stays OPEN for arbitrary software-stalled provider calls and
permanent refusal; P0-D/E/F remain open. The production OOM gate is
still disabled. These driver timeouts require a healthy APR clock.

## 2026-11-03 v1081: retain final TTY ownership before VM destruction

The last non-leader member of a session can encounter MM_BUSY while
retiring the session's dynamically allocated canonical input line.
The prior proc_tty_release_session silently kept its TTY owner but the
process finalizer proceeded to destroy VM and free the u-area, losing the
only natural owner of the retry. Leader HUP teardown already refused a
failed line free; this final-member case was separate.

The TTY helper now returns 0 or -1 using the existing checked line
reset. proc_finish_slot calls it before vm_space_destroy, and on
refusal reuses the existing PREVM+FINALIZE owner marker and idle-stack
retry, including the same failure path as a VM free refusal.
Successful TTY release, VM destruction and post-VM u-area release
remain distinct one-way stages. No new static owner table or BSS word.
The serialized FILE provider lease and device watchdogs remain intact.

The native v1081 finalizer test proves refusal leaves VM untouched and
the PREVM owner live, followed by a single committed VM teardown and
u-area MM retries. Source-linked real-MM TTY v967/v968 and existing
v881 deferred u-area regression pass. The complete 256K v1077
physical-OOM provider/MM guest also passes. Production requester-OOM
termination remains off while other transitive cleanup dependencies
are not proven.

The ordinary FILE close paths are narrower than the general provider
ABI: current D6FS SYNC is an immediate zero result, DTFS SYNC only
validates a vnode, and TSFS uses a no-op SYNC. Thus no new generic
provider abort mechanism is warranted by those particular closes.
Keep the shared DTFS/D6FS/TSFS serialization and foreign-owner fatal
nonwait guard: a different in-flight provider operation may still
hold the lease, and forcing unlock would break resource ownership.

Matched full-image baseline/current KCC configurations give
`__kcore_low_end` `023110` -> `023112` octal: **two additional
resident instruction words**, no persistent data words. This is a
configuration-matched comparison, distinct from the earlier v1080
full-profile `023160` map and from instrumented test images.

## 2026-11-03 v1080: restore DRM236 full-FIFO owner wait

Source audit of the v1079 watchdog addition found a newly introduced
runtime failure at `drm236_runtime_queue_full`: masking the active
descriptor's watchdog LH with `HRRZ` accidentally removed the original
`SKIPN` conditional branch, leaving an unconditional `JRST retry`.
When one active and one pending DRM descriptor both existed, a third
requester hot-spun inside the kernel forever instead of calling
`proc_wait_event` on the active descriptor. This bypassed the
normal scheduler and starved the live completion owner.

v1080 uses `HRRZ 1,drm236_active_request` followed by `JUMPE 1,retry`.
The third requester waits on the live active descriptor's event at
offset two, returns after its completion, and then claims or queues
normally. The descriptor pointer is taken only from the RH of the
packed watchdog word. **No new permanent RAM or MRES words**: `JUMPE`
replaces the existing `JRST` in place.

The native source-linked PDP-6 v1080 test creates three overlapping
request descriptors and checks the precise two waits: first on the
prior active owner, then on the newly claimed request. It verifies the
fresh 300-tick deadline, returned I/O status, cleared ownership and
single resumption. PASS. The v1079 driver fails this test as a
negative control. Existing v1079 watchdog and Type-167/236 real-SIMH
online I/O tests PASS. The full 256K requester-OOM guest also PASSes
with unchanged D6FS and MM refusal counts.

The independent DSK slot-0 swap path and the DRM slot-0
`proc_wait_boot` path still depend on a completing event; this patch
does not yet establish a bounded slot-0 hardware failure.

## 2026-11-03 v1079: bound in-flight DRM236 runtime I/O

Auditing active providers exposed a real unbounded device wait: DRM236
runtime I/O kept 3-word descriptors on private executive stacks and
slept for PI2 Type-236 DONE, but unlike DSK270 only its *early boot*
polling path had a completion timeout. If drum hardware never
interrupted, the request and its caller's stack could remain live
indefinitely, preventing the serialized filesystem lease from
returning through the normal owner-release path.

The DRM MRES now exports `drm236_watchdog_tick_v1079`, reached on every
qualified APR/CLK 60-Hz tick after the existing DSK watchdog. KINIT
patches both direct call sites; no runtime table or extra BSS is used.
The existing active request word is repacked as
`remaining_ticks:18,,descriptor_ptr:18` and starts at octal `0454`
(300 ticks, five seconds). PI2 DONE or timeout share the *same*
completion and FIFO promotion routine. On timeout the Type-167/236
controllers are cleared, the live descriptor's event becomes `-1`,
its owner is awakened and the single queued request, if any, starts
with its own complete 300-tick deadline. Descriptor pointers are
always unwrapped from RH before dereference. No foreign lease is
forcibly released; the device call returns a real error so its caller
can unwind and release its own resources.

PDP-6 native source-linked v1079 clock+DRM tests PASS for two consecutive
timeouts, exact 300-tick deadlines, single pending FIFO promotion,
error accounting, and no double completion. A deliberately disabled
watchdog fails the same test. The restored Type-167/236 online read,
write, and nested queue test PASSes (its obsolete `drm236_read_jump`
test symbol was corrected to the current `drm236_read_call`).
The full 256K v1077 physical OOM/FILE/MM guest also PASSes with the new
driver/clock integration. The independent `ROOT=drum` Stage1 SIMH
build/boot PASSes with DSK0 unattached, reaching DRM236 MRES, D6FS,
INIT and LOGIN. Full KCC link adds one KCORE word (`023157` to
`023160`, octal). DRM MRES residency grows from 124 to 141 words
(+17). Total additional resident RAM: **18 words**, all code and
relocation metadata, with **zero new permanent state words**.

This closes the **runtime DRM hardware-stall** completion case when
qualified APR clock ticks continue. It is not general asynchronous
provider cancellation: a provider looping in other code, permanent
close refusal, a clock-disabled slot-0 device request or an allocation
inside fatal teardown still needs separate correctness work. Automatic
OOM requester termination stays DISABLED.

## 2026-11-03 v1078: fatal-owner FILE close does not wait on a foreign provider

The serialized DTFS/D6FS/TSFS MRES gateway formerly put **all** foreign
lease contenders to sleep in `proc_wait_event`, including a process
already retiring through `proc_fatal_close_current_v879`. If a
different provider retained the shared lease indefinitely, this dying
owner could never return from `file_close_all` to its bounded-backoff
private retry; it would stay stuck inside the gateway wait.

The gateway now checks the **existing** `PROC_FATAL_CLOSE_BIT` in
`file_table[-2]` only after a held foreign lease is established.
Fatal owners receive `-5` before entering the provider, with their
request ACs restored; the FILE descriptor remains owned and the
existing private fatal retry sleeps and reattempts closure. Ordinary
foreign waiters still sleep/revalidate, recursive owners still refuse,
free providers still dispatch and only the real provider return
unlocks/wakes waiters. The offset is loaded using AC1, not AC0
(which cannot index PDP-6 memory); AC1 is already saved in the
existing seven-AC gateway frame. No new persistent data or ABI.

Source-extracted native PDP-6 testkit v1078 PASSes fatal fast refusal,
no unexpected provider execution/wait, normal foreign wait/recovery,
free-provider fatal dispatch and recursion. Original v1077 gateway
fails the new native negative control. Existing native provider
gateway/indirection/lock and rename tests v326/v327/v866/v915/v916/
v917/v428/v681/v466 PASS; provider ownership/static v954/v986/v987
checks PASS. The comparable full-profile KCC KCORE linked end rises
from `023151` to `023157`: **six additional code words, zero
new permanent data words**.
The full 256K v1077 requester-only physical-OOM guest with two real
D6FS SYNC refusals and one u-area MM_BUSY also PASSes on the revised
gateway (FILE 3/2, MM 2/1; `<KWP`; bystander and accounting preserved).

This fixes a *waiting-to-acquire* deadlock of the fatal owner, not
a provider already executing an unbounded operation. It cannot cancel
an in-flight provider call or guarantee reclamation when a provider
permanently refuses to return. Production requester OOM termination
remains **DISABLED**.

## 2026-11-03 v1077: physical-OOM exit across real D6FS and MM refusals

The isolated full-system 256K PDP-6 testkit v1077 guest enables the
existing OOM BRK branch *only in its disposable kernel copy*. The
requesting child opens a real D6FS `/SYSTEM/INIT` vnode before triggering
genuine physical OOM. Test-only hooks in the actual D6FS SYNC provider
refuse two close attempts; the third succeeds. The ordinary FILE/VFS
boundary records exactly three D6FS closes and two failures for that
child. A separate hook refuses its first MM_TYPE_KERNEL_DYNAMIC
u-area `mm_free` with MM_BUSY (-5). The permanent idle-stack finalizer
retries and succeeds on the second call, without a second VM teardown.
The guest reports `<KWP`, no error markers, correct parent WAIT status,
unrelated PIPE-blocked witness survival and recovered process-memory
accounting. Native counter witnesses: close calls/refusals = 3/2,
u-area free calls/refusals = 2/1. **PASS**.

This is a finite-refusal success and ownership regression, not proof
that a permanently unresponsive provider can be canceled or that
every callback on the fatal path is transitively allocation-free.
Production OOM activation stays OFF; no production source, KCORE code
or persistent RAM was changed for v1077.

## 2026-11-03 v1075: booted physical-OOM requester and witness acceptance

A new testkit full-system PDP-6 256K fixture tests the already-existing
BRK OOM dispatch under a disposable, test-only build with
`PROC_OOM_BRK_ENABLED_V1000=1`. The requesting child grows its logical
break to valid 0776000 octal, which cannot be physically satisfied
after allocation/reclaim. Its real fatal-owner FILE/HUP/VM/u-area path
exits with status 1; the unrelated child remains blocked on a PIPE,
later wakes after close, and exits normally. The parent confirms that
process-count and process-word accounting return to baseline and then
HALTs. Guest output: `<KWP`, no error marker. This is a complete
256K physical-OOM *success-path* test, not a mocked syscall.

The separate full-guest negative control uses the identical request and
unchanged production flag 0: BRK returns -1 to the child, which emits
`D` and exits normally. The parent verifies its non-OOM exit status,
resumes the independent witness and confirms memory recovery. The guest
emits `<DNWP`, not an intentional assertion-failure marker.
No runtime KCORE changes or additional resident words are introduced.
The owner-private provider-failure retry, unresponsive provider and
allocation-free bounded teardown gates remain P0; production OOM killing
must remain OFF.

## 2026-11-03 v1074: BRK double-MM rollback cannot silently orphan a new extent

Source audit of `proc/vm_pdp6.s` found a real failure path during
nonadjacent BRK growth: if `mm_free` refused to release the old mapping,
the kernel attempted to free the provisional new extent but ignored a
second MM failure and returned to user mode. The provisional allocation
was never published in the process descriptor; losing its base would
orphan an MM_TYPE_PROCESS extent and make later recovery impossible.

The rollback now preserves the *exact* old MM refusal (-4 for invalid
ownership/type, -5 for a pinned extent) in an existing stack local,
checks the new-extent `mm_free`, and enters the existing resident
`proc_swap_storage_invariant_v30` HALT on an impossible double refusal.
The successful rollback still preserves old VM state/APR/BRK and returns
the original failure; there is no new allocation or permanent data.
The new `daimos-testkit` PDP-6 source-extracted test verifies -4/-5
clean rollback and second-free refusal, with two failing negative
controls (original v1073 source and removal of the new guard).
GCC KCORE and full-profile KCC image build PASS. The KCC image boots
through INIT to LOGIN, and the full native `daimos-brk-v1` guest
regression passes BRK/SBRK growth, data preservation and shrink.

Automatic requester OOM termination remains DISABLED. This fix seals
one MM rollback invariant; it does not solve permanently refusing
FILE/MM providers or prove allocation-free, bounded fatal teardown.

## 2026-11-02 v970: deferred fatal-event precedence

PDP-6 foreign fatal delivery now records INT, TERM, HUP and PIPE on a
protected executive stack instead of refusing non-TERM events. All four
coalesce in existing control-word bits; owner-side return chooses HUP,
then INT, then TERM, then PIPE, marks private fatal cleanup ownership,
and enters the existing allocation-free retry loop. Native source-linked
v970 deferred-event and HUP retry fixtures pass. No new process fields
or static allocations. Booted mixed-event, self/group-send and PID reuse
acceptance remain open; size optimization stays deferred.

## 2026-11-01 v925: protect saved executive context after wait wakeup

The v924 wait-class guard missed the interval after a child's CHLD
wakeup clears WAIT_CHILD but before the noncurrent parent's saved
executive stack resumes. `proc_restore_kernel_tail` restores and clears
the saved CTX_PC only when the owner runs; while noncurrent, its stable
u-area CTX_PC still carries the PI return mode. An executive return
lacks LH 010000 (the user-mode return bit). The OOM selector, remote
fatal path, and direct foreign finalizer now refuse such a context,
in addition to existing wait-class/lease and fatal-FILE protections.
The current owner may still unwind its own stack. Twelve assembly
instructions, no new KCORE data words, per-process state, or ABI.

Native PDP-6 v925 pre-fix negative controls reproduced unsafe OOM
selection (exit 010), remote fatal (032), and direct finish (034).
The repaired source passes these three tests, existing WAIT_CHILD and
INTR checks, and historical FILE, RT, session/finalizer and rollback
native regressions. Extracted-source fixtures now explicitly initialize
saved user PI returns; the v921 static audit asserts all three CTX_PC
checks as well as the wait-class masks.

Booted 256K PDP-6 v924 provider/WAIT_CHILD compatibility on v925 PASS:
three fatal refusals (one real WAIT_CHILD), stable owner lease 4,,0,
73 bystander cycles, zero errors, controlled HALT. This is booted
nonregression rather than direct handoff injection. Cooperative pending
TERM delivery, genuinely stalled provider recovery and allocation-free
OOM termination remain P0. OOM killing stays OFF.

## 2026-11-01 v924: protect all private wait-class continuations

The v918/v920 remote teardown and OOM guards incorrectly tested only
PROC_WAIT_EVENT_LH (020000). A child-WAIT owner instead retains its
private executive stack with PROC_WAIT_CHILD_LH (040000); the old OOM
selector could select it and remote TERM/direct finish could free its
live stack. All three guards now use PROC_WAIT_LH_MASK (060000): EVENT,
CHILD and interruptible waits. These are only operand changes to the
same three instructions: no additional KCORE instructions, permanent
RAM words, per-process state or syscall ABI changes.

Native PDP-6 v924 negative baselines all exposed the defect (OOM exit
010, remote fatal exit 032, direct finish exit 033). Post-fix tests
cover CHILD and INTR, with existing EVENT and teardown regressions
passing. The v921 static OOM inventory now verifies all wait encodings
and all three full-mask guards. Historical extracted-assembly tests
were updated for the shared constant and missing finalizer fixtures.

Full-system 256K PDP-6 v924 PASS: real child-WAIT parent and living
child alongside the held serialized FS owner, foreign gateway waiter,
and runnable bystander. Three actual fatal guard refusals, exactly one
WAIT_CHILD-class refusal, lease 4,,0 stable at all three samples,
72 independent bystander sleep cycles, zero errors and controlled HALT.

Safe refusal is not automatic deferred TERM delivery: a sender must
retry once the private wait completes. A genuinely nonreturning FS
provider still retains its lease and resources; no forced unlock or
automatic OOM kill is permitted. Establish cooperative pending fatal
behavior, bounded cancellation, and allocation-free OOM cleanup before
relaxing these safety guards. Automatic OOM killing stays OFF.

## 2026-11-01 v923: repeated FILE SYNC refusal and partial close retry

A new native PDP-6 v923 test retries remote fatal FILE cleanup across
two consecutive SYNC refusals. The failing target descriptor remains
owned, a successful independent descriptor stays closed and is never
replayed, global `file_table` remains associated with its original
caller, and final successful retry closes the sole survivor before
one fatal-slot completion. Native v923, v867, v865 and v885 PASS.

The test extends the existing explicit-target close fixture and proves
this particular partial-close retry sequence without new production
KCORE instructions, data words or ABI changes. Infinite provider
refusal, an operation that never returns, and allocation-free OOM
termination remain unresolved P0. Automatic OOM killing stays OFF.

## 2026-11-01 v922: repeated partial session HUP retry proof

The v905 session-HUP retry path already recognizes an irreversibly
committed member only through TRANSITION+VM_NONE+FINALIZE. Native PDP-6
v922 repeatedly fails the second member's HUP after the first has
committed VM destruction but retained its pending u-area allocation.
Both failed attempts preserve TTY ownership and the first member's
finalization marker without redelivering its HUP. The eventual success
only delivers HUP to the previously failing member before TTY release.
A test-only pre-v905 negative control fails as required (exit 023).

Native v922, v905, v875 and v918 tests and the v921 source audit PASS.
No production KCORE, RAM, or ABI changes; this expands test coverage
rather than claiming comprehensive OOM cleanup. A permanently stalled
provider or failed FILE SYNC still prevents bounded teardown; automatic
OOM termination remains disabled.

## 2026-11-01 v921: OOM teardown safety-path inventory

The resident OOM victim selector remains read-only; automatic OOM
termination is disabled. A new executable testkit static audit checks
the current PDP-6 fatal/finish teardown edges and private-wait guards.
It finds FILE close -> VFS_SYNC (potentially sleeping or failing),
process finish -> session HUP (partial delivery possible), and VM
release -> MEMFS XIP provider, MM free and backing detach. The u-area
is freed through the resident MM table. Individual MM deletion uses no
allocation but does not establish transitive allocation-freedom or
bounded completion for all provider and session callbacks.

v921 testkit audit PASS for current call-chain inventory and existing
private-stack protection; it explicitly reports the termination chain
NOT CERTIFIED. v917-v920 native and booted acceptance remains valid.
No production KCORE/RAM/ABI changes in this stage. Safe reclamation of
nonreturning providers, failure-atomic multi-stage teardown and full
allocation-free OOM kill proof remain open P0. OOM killing stays OFF.

## 2026-11-01 v920: protect direct noncurrent process finalization

The direct `proc_finish_slot` entry can destroy a target VM without
passing through the v918 fatal-event gate. It now refuses a foreign
target with `PROC_WAIT_EVENT_LH` set before transition marking or
resource cleanup. A self-exiting process remains allowed to unwind its
own stack normally. This adds nine assembly lines with no new BSS,
per-process state, syscall or module ABI changes.

Post-v920 booted PDP-6 compatibility acceptance: the full 256K v919
remote-fatal provider test PASSes against v920. One suspended lease
owner, one foreign waiter, exactly two refused fatal deliveries,
stable lease 4,,0, 72 runnable bystander sleep cycles, zero errors
and controlled HALT. This confirms no scheduler/provider/fatal regression
from the direct finalizer guard; it does not inject a direct
proc_finish_slot request from another guest process.

Native PDP-6 v920 verifies that a direct attempt does not release VM or
u-area while the foreign private executive continuation is live, and
that normal fatal teardown remains functional after the event wait
clears. Native v883, v918 OOM/fatal, and v868 regressions pass. Full
booted v919 provider/fatal compatibility is being checked separately.
Reclaiming a genuinely unresponsive provider remains open; automatic
OOM killing stays disabled.

## 2026-11-01 v919: booted remote fatal protection for provider waits

A full 256K PDP-6 guest now verifies the v918 private kernel-event
stack lifetime guard with real TERM delivery. One process owns a live
serialized filesystem gateway lease while suspended in proc_wait_event,
and a second process sleeps on the occupied provider lock. INIT sends
TERM to both. The fatal-event path refuses both before remote FILE
close, VM destruction, or private-stack release. Disposable test-only
instrumentation records exactly two actual guard rejections.

The filesystem lease remains encoded 4,,0 at acquisition, contention,
and final controlled HALT. The independent third job runs 72 sleep
cycles, and the guest reports zero errors. The existing v917 scheduler
regression and v918 native safeguards remain accepted. v919 adds no
production KCORE instructions, resident RAM, ABI, or per-process state.
Recovery or eventual reclamation of genuinely nonreturning provider
resources remains an open P0 gate, as does allocation-free OOM killing.
Automatic OOM victim termination stays OFF.

## 2026-11-01 v918: preserve private kernel-event continuations

A process in PROC_WAIT_EVENT_LH sleep may still own an active private
executive stack, including a blocked filesystem-provider or MM call.
Non-current fatal delivery now refuses to close such a victim's FILE
references or destroy its VM; OOM selection omits the same protected
waiter. Both changes use existing resident scheduler state. Twelve
assembly lines added, no BSS, process-table, syscall or module ABI
changes. This is safety containment, not forced recovery.

Post-v918 full-system compatibility: the existing isolated 256K PDP-6
v917 held-provider/waiter/bystander boot regression PASSed against v918
KCORE. One held owner, one foreign waiter, stable lease 4,,0, 72
unrelated bystander sleep cycles, zero errors and normal controlled
HALT. This is a provider/scheduler non-regression, not an injected
full-system remote fatal delivery against that suspended waiter.

Native PDP-6 v918 separately verifies refusal without close/VM mutation,
normal fatal lifecycle after the event state clears, and exclusion from
OOM candidate selection. Existing v866, v883, v879, v868, v867, v865
and v885 native regressions PASS. Full booted v918 fatal delivery under
a live private-event wait and allocation-free OOM termination remain
unverified. Automatic OOM killing stays OFF; TSFS v1.2 is deferred.

## 2026-11-01 v917: fix owner-encoded provider event busy retry

The real PDP-6 scheduler checks the entire `proc_wait_event` word.
The filesystem gateway formerly passed `fs_provider_ready` as that
word, but a held lease stores owner_slot+1 in its left half. The
nonzero value made a foreign waiter retry without sleeping. A full
256K booted guest exposed 8193 repeated lock attempts and stalled
independent work. The gateway now uses one separate resident event
word, zero while held and signaled when the owner unlocks. The lock
word continues to identify its original owner. Slot 0 returns -5
rather than entering the bootstrap wait loop on a foreign lease.

Native v917 validates the zero event and slot-zero refusal, with v915,
v916, v866, v326 and v327 provider tests passing. Full booted v917
shows one holder and one waiter blocked, owner encoding 4,,0 stable
at acquisition, contention and controlled HALT, 72 bystander sleep
cycles and zero errors. The first post-fix guest timeout came from a
fault-injection hook accidentally using the old lock word again; the
corrected hook and genuine scheduler path passed. No forced unlock,
per-process state or syscall ABI change. One resident event word was
added. A nonreturning owner's resources remain owned; automatic OOM
victim termination is not yet enabled.

## 2026-11-01 v916: foreign FS provider lock cannot be stolen

Native PDP-6 testkit v916 exercises the production fs_provider_reg_call
through four repeated waiter attempts while a different process retains
the existing FS lease word. It checks that none of those calls can
enter the provider, corrupt its owner, or release its shared workspace.
The fifth simulated wait observes normal unlock, then dispatches with
all seven request registers and stack restored. The v915 recursive
self-owner rejection and v866 owner encoding regressions also PASS.
No production code, kernel BSS or ABI change. The test uses a native
simulated wait/retry oracle, not an actual permanently stalled physical
provider or full-system scheduler fault. A genuinely nonreturning owner
still cannot be freed or terminated safely; that full-system progress
and OOM cleanup acceptance remains P0. OOM killing stays OFF.

## 2026-11-01 v915: provider lock detects same-owner recursion

DTFS/D6FS/TSFS share a serialized KCORE gateway with one existing
`fs_provider_ready` word encoding owner_slot+1 in LH while held.
Previously a second gateway call on the *same owner's* kernel stack
entered `proc_wait_event` awaiting its own lease and could never make
progress. The lock-wait path decodes the existing owner-slot field
and compares it with `proc_current_slot`; self-ownership returns -5
without sleeping, restores all saved request registers and the stack,
and preserves the original lease. Other processes still wait/resume.
MEMFS remains direct. Nine resident KCORE instructions are added, but
no new BSS, per-process metadata, heap or provider ABI changes.

Native PDP-6 v915 verifies recursive -5, unchanged lease ownership,
zero owner sleeps and subsequent cross-process wait/release. The v866,
v326 and v327 lock/dispatch/AC-preservation regressions also PASS.
Final integrated v915 rerun: booted 256K PDP-6 with the compacted
nine-instruction gateway PASSes the full v914 APR-fault/permanent FILE
provider refusal test: 11 rejected provider calls, 11 preserved FILE
ownership checks, 11 private sleeps, 512-tick cap, zero errors and a
runnable bystander through controlled guest HALT. This checks system
regression, not a booted nested-lock injection.

Post-v915 full 256K PDP-6 v914 guest regression PASSes: real APR
fault, 11 permanent invalid FILE-provider refusals, 11 private sleeps,
512-tick capped retry, FD retained, independent process runnable and
normal controlled HALT (zero errors). This is not a test of a separate
process permanently holding the filesystem provider lease.

This establishes local self-deadlock avoidance, not recovery when a
separate process holds a provider lock indefinitely. Full booted
provider-lock failure/PI6 and allocation-free OOM fatal cleanup remain
P0; OOM killing remains disabled and TSFS v1.2 work stays deferred.

## 2026-11-01 v914: persistent invalid FILE-provider errors remain owned

The v891 full-system 256K PDP-6 APR-fault continuation already proved
safe retention under permanent VFS provider error -5. The v914 testkit
extends that exact real hardware-fault and bystander workload to a
permanently invalid provider result -4, without changing production
code. A copied kernel returns -4 only from the victim's FD 0 vfs_sync.
The first build deliberately caught a stale v891 guest assertion
(expect class 5); the corrected v914 guest expects class 4 and PASSes.
Measured: one real APR fault, 11 provider calls/rejections, 11 FILE
cleanup passes, 11 owner-stack sleeps, 11 retained-FD checks, zero
premature closes, witness clears, victim exits or test errors. The
retry cap reaches 01000 octal (512 decimal) ticks. GETTEARDOWN reports
FILE stage and the actual negative provider error, while an independent
user remains runnable. Controlled guest HALT does not reap the owner.
Native v888 MM_BUSY/INVAL classification, v909 fatal stage/finalizer
and v885 fatal FILE retry/backoff regressions PASS.

The test proves containment of a permanently invalid FILE provider,
not recovery of corrupt or missing MM extents or bounded reclamation.
Automatic OOM termination remains disabled. Fatal provider-lock and
allocation-free victim retirement acceptance are still P0. No new
kernel instructions, RAM, syscall ABI or module dependency.

## 2026-11-01 v913: booted Type-340 double-MM refusal accepted

Full-system 256K PDP-6 with a live raw Type-340 program exercises the
v912 double-MM-release path, not only an isolated MM stub. A test-only
copied kernel pins the active old list (physical base 19760 decimal)
and the new unpublished list (base 19792 decimal) at their actual MM
free boundaries. Both free attempts return MM_BUSY. A subsequent
WRITE_WORDS and final CLOSE each return MM_BUSY before allocating or
unpublishing anything further. Runtime ownership witnesses confirm
`dpy_list_base == 19760` and `dpy_list_orphan_v912 == 19792` after
all four refusals. FILE/process slot accounting remains unchanged.
There are two physical pins, zero unpins, four rejected frees, zero
errors, 18 live PI6 refreshes, 18 PI7 BLKO completions and 12 refreshes
after first MM_BUSY. The guest prints <D and HALTs normally. Native
v912/v874/v904 regressions pass. Fault hooks and ownership witnesses
live only in the v913 testkit's disposable kernel copy; production
image, kernel ABI and permanent RAM are unchanged since v912.
This proves safe retained ownership under simultaneous permanent
refusal, not reclamation of the two still-pinned MM extents. Permanent
fatal FILE teardown, provider lock failures, and allocation-free OOM
victim retirement remain P0. OOM killing remains OFF; TSFS v1.2 and
final KCORE shrinking remain deferred.

## 2026-11-01 v912: DPY double-MM-free refusal retains both owners

Type-340 replacement previously executed HALT if freeing the old active
raw list failed and freeing the unpublished new list also failed. The
normal MM refusal contract makes this possible when both extents are
physically pinned. The driver now retains that unpublished extent in
one DPY-MRES word, `dpy_list_orphan_v912`; the original active list
remains published. The next DPY WRITE_WORDS or STOP first retries the
orphan release, without allocating further memory or changing PI7.
If MM still refuses, it propagates the error without losing ownership;
if MM succeeds, it clears the orphan before the requested operation.
The original list's clock refresh is rearmed after double failure.

PDP-6 native v912 proves double refusal, no HALT, one tracked orphan,
no second allocation under continued refusal, successful orphan drain,
subsequent replacement/STOP and unchanged original rollback policy.
Existing v871/v874/v904 and PI7 policy regressions PASS. One optional
DPY-MRES BSS word is added; no KCORE BSS or syscall ABI change.
v912 post-commit full-image regression: the existing v910 256K PDP-6
physical-MM refusal guest was rebuilt against the v912 DPY-MRES and
passed again (base 19743 decimal, 3 BUSY, 1 pin, 0 unpins, 18 PI6
refreshes, 18 PI7 completions, 12 post-error refreshes, 0 errors,
normal HALT). This is a regression against the changed driver, not a
full-system injected double-free test.

Permanent refusal can retain both allocations; this is safe ownership,
not eventual reclamation. Full booted PI7/double-fault acceptance, fatal
FILE teardown, provider locks, automatic OOM and KCORE size gates remain
open. OOM termination stays disabled; TSFS v1.2 remains deferred.

## 2026-11-01 v911: SYS_MEMINFO counts all 16 FILE slots

Full-system 256K PDP-6 exposed two independent resident SYS_MEMINFO
errors. FILE capacity and the counted scan used octal 015 (13) instead
of FILE_NFILE=16 (octal 020). More seriously, the routine wrote the
FILE count into result word 7 *before* an in-place filesystem MRES
request initialized result words 2..8, destroying the count. Fix both
limits and move the existing FILE scan to after the optional MEMFS
request, leaving syscall ABI and resident storage unchanged. No new
instructions or per-process state.

A C89 testkit guest duplicates CTY descriptors until all 16 FILE slots
are occupied, checks MEMINFO after each DUP, exercises the final FD 15,
then closes every duplicate and checks baseline counts. Corrected
full-system guest marker <F and normal HALT PASS. Two independent
negative guests PASS: the copied 13-slot kernel reports !1 and the
copied pre-fix FILE-count ordering reports !4. Native v708/v874/v904
PASS. Re-running v910 with production v911 PASSes permanent DPY MM_BUSY
(3 refusals, 1 pin, 0 unpins), 18 PI6/PI7 frames, 12 after refusal,
stable *now genuinely populated* FILE/process counts and normal HALT.
The former v910 counter comparison alone could not establish positive
FILE occupancy because the old routine zeroed both samples.
Permanent provider release, rollback double-fault, OOM and KCORE size
gates remain open; OOM killing stays disabled, TSFS v1.2 deferred.

## 2026-11-01 v910: persistent Type-340 MM refusal retains ownership

Full-system 256K PDP-6 (`DPY OK`) now proves the raw-list final CLOSE
can encounter *permanent* physical MM_BUSY without destroying its owner.
A test-only copy of the kernel pins one real MM owner-014 dynamic-list
extent and never unpins it. Three consecutive CLOSE attempts each return
MM_BUSY (-5); the FILE count and process count remain unchanged from
the post-OPEN sample. PI6 clocks keep replaying the same published list:
18 starts, 18 completed PI7 frames and 12 refreshes after the first
refusal. Exact physical target 19732 decimal, three frees, one pin,
zero unpins, three busy results, zero errors; normal test HALT. Native
v871/v874/v904 and PI7 policy regressions PASS. All v910 instrumentation
lives in daimos-testkit, with no production kernel, size or ABI changes.
The still-pinned allocation cannot be reclaimed; replacement double
failure, locked providers and allocation-free OOM termination remain P0.

## 2026-11-01 v909: permanent MM refusal cannot spin the idle reaper

A booted 256K PDP-6 fault-injected physical pin on an APR-fault victim's
u-area found that `proc_idle_loop` retried MM release every timer tick,
even though `proc_sched_tick_select` already enforced shared capped
backoff. The original guest made 1,281 failed frees and 1,280 reaper
passes under the same one permanent pin. The corrected idle loop
subtracts the saved timer epoch modulo the existing clock mask and
requires the existing retry interval to elapse before every attempt.
The fixed guest makes 7 failed frees and 6 reaper passes in the same
02400-octal-tick window, with no additional permanent state. A second
lifecycle correction clears the private fatal-FILE flag only after VM
teardown has committed and the u-area finalizer has been recorded;
TRANSITION+VM_NONE+FINALIZE retains ownership. GETTEARDOWN now reports
MM, 512-tick backoff and one real pin, and WAIT(NO HANG) cannot reap
the victim. A separate child remains runnable. Native v909 pre-fix
negative control and deferred-owner/FILE-witness regressions cover the
transition. This is safe retention, not a permanently failing MM
provider's recovery or approval to enable OOM killing.

## 2026-11-01 v908: live DPY MM_BUSY retains an active raw list

Full 256K PDP-6 with a headless but present Type-340 (`DPY OK`) accepts
normal raw-list WRITE_WORDS, actual PI6 clock replay and PI7 BLKO
completion. A disposable copied-kernel MM hook pins the exact owner-014
physical extent on final CLOSE, causing real MM_BUSY (-5). The FILE
entry and published raw list survive and *six* further raw frames
refresh before unpin. The next CLOSE unpins/frees the original extent;
another DPY OPEN/WRITE/CLOSE succeeds, process/FILE accounting stays
stable and the guest HALTs normally. Testkit v908 counters: base 19730
decimal, 3 tracked MM frees, 1 pin, 1 unpin, 1 busy, 18 raw refreshes,
18 PI7 raw completions, 6 post-error/pre-unpin refreshes, 0 errors.
Existing native v709/v871/v874/v904 and PI7 policy tests PASS. No
production change, ABI adjustment, new permanent data, or size sweep.
Permanent FILE/MM refusal, fatal-process cleanup and OOM termination
remain unaccepted.

## 2026-11-01 v907: physical MM session-HUP retry accepted

A booted 256K PDP-6 now accepts the v905 partial-session-HUP recovery
under a real `mm_pin`/`mm_unpin` of the first victim's physical u-area.
The first child's HUP commits but MM_BUSY retains its stable owner;
the second child's first HUP is rejected by a disposable test hook.
A second leader HUP through the actual PROCCTL syscall skips the
TRANSITION+VM_NONE+FINALIZE child, delivers only the outstanding HUP,
then permits MM unpin, TTY detach, parent WAIT and TTY reattachment.
Guest counters: leader 2, first child 3, u-area base 20020 decimal,
one physical pin/unpin, one rejected HUP, one later delivered HUP,
zero repeats, one pending-owner verification, zero errors and normal
HALT. Fault injection is confined to daimos-testkit's copied kernel.
No production kernel source or permanent-state changes in v907.

## 2026-11-01 v906: stale TSFS-specific KCORE backlog entries retired

Historical V13/V15/V22 cross-provider mount and TSFS metadata/format
items are no longer active KCORE feature or size gates. Post-KCORE
TSFS v1.2 work retains its separate pre-0.9 milestone; only current
TSFS boot compatibility remains a regression contract for shared
KCORE changes. Generic VFS and D6FS acceptance stays in KCORE.
The ordinary full-system session-leader teardown test on v905 PASSed;
HUP partial-provider fault injection remains outstanding.

## 2026-11-01 v905: prevent duplicate committed session HUP

During a session leader retry, a prior child HUP may have successfully
terminated the child but been unable to release its u-area MM extent.
If a later member fails HUP, reissuing HUP to that transitional child
would be rejected by `proc_event_apply` and permanently block TTY
teardown. The session scanner now skips only an actual completed
TRANSITION+VM_NONE+FINALIZE owner, not an arbitrary transition.
Native PDP-6 v905 positive/negative control verifies retry, member
call counts, retained TTY, final commit and marker/VM guards; v875
retains its original acceptance. Permanent-provider and complete
full-system partial-fault testing remain open.

## 2026-11-01 v904: DPY STOP publishes no stale PI7 ownership

P0 DPY STOP now issues `CONO 0130,000100` (INIT/PIA disabled)
before clearing the published `dpy_list_base`. Previously an
intervening PI7 completion could observe an absent raw list while
hardware still owned the span. Reordering existing instructions does
not add code or permanent state. Native PDP-6 actual-MM v904 verifies
pinned and wrong-owner CLOSE rejection, retained FILE/DPY identity,
correct-owner retry/free and absent-list close. The isolated old-order
contract is rejected; v709/v871/v874 regressions PASS. A live PI7
full-system replay fault remains unverified.

## 2026-11-01 v903: TSFS deferred until after KCORE

The KCORE feature and size backlog excludes TSFS v1.2 sharded-metadata
development. Complete and test KCORE (including its deferred size pass)
first; then implement and accept TSFS v1.2 as separate pre-0.9 work.
Preserve existing TSFS boot behavior during KCORE regression testing.

## 2026-11-01 v902: existing full-system PIPE workload passes

After v901 restored native PIPE character I/O, the original
`daimos-pipe-anon-v1.c` workload was re-run unmodified in a booted
256K PDP-6 with headless simulator console settings. It verifies
PIPE creation, READCHAR/WRITECHAR, EOF, EPIPE, duplicated references,
`RUN` inheritance and blocked-peer wakeup. All phases and normal
HALT PASS. The versioned v902 harness is in daimos-testkit.
There are no production code or permanent-data changes, and no
KCORE size optimization was attempted. Remaining FILE/MM/DPY and
safe OOM termination feature gates are tracked in TODO.MD.

## 2026-11-01 v901: PIPE character I/O uses existing word FIFO

The v899 exploratory full-system test initially tried `READCHAR`
on a retained PIPE after MM_BUSY. Its result was -1, although
`READ_WORDS` subsequently proved the descriptor and ring buffer
were valid. Source audit identified an independent feature gap:
`vfs_readchar` and `vfs_writechar` only accepted MonitorFS device
streams, despite the older PIPE tests exercising character I/O.
The VFS now routes PIPE stream vnodes (`070001`) to existing
`pipe_read_words` and `pipe_write_words` with a one-word request.
READCHAR receives the full word into the caller's buffer and uses
the ordinary positive/zero/error result; WRITECHAR uses one
stack-local source word and converts a successful count of one to
its required zero return. The underlying PIPE wait, EOF, event,
reference-count and pin rules are unchanged. No new static data.

The native PDP-6 test exercises character queue transfer, EOF,
broken reader, and preserved MFSDEV dispatch; old v900 VFS fails
its negative control as expected. A booted 256K PDP-6 guest
accepts READCHAR/WRITECHAR, mixed char/word API interchange,
EOF after the writer closes, stable FILE-slot accounting, CTY
success and HALT. The earlier v899/v900 physical MM retry tests
remain valid as independent lifetime coverage. No size optimization
was attempted; the permanent provider and OOM-kill gates remain open.

## 2026-11-01 v900: named FIFO final-close linked-list rollback

The last-reference PIPE release validated in v899 has an additional
named-FIFO linked-list ownership gate. A full PDP-6 guest runs with
a disposable test-kernel hook that pins the exact named FIFO pipe
object just before the original `mm_free`. The actual MM allocator
returns MM_ERR_BUSY. Unlike an anonymous PIPE, the FIFO was already
unlinked from its active list before attempting release; the original
v870 code restores the same object to the list after MM refusal.
The test opens the same pathname again and confirms that a word
written through the original retained descriptor is read through
its new descriptor. A later final-close retry unpins and frees the
physical extent; new OPEN/CLOSE and normal HALT also succeed.
SIMH full-system PASS: one pin, one busy, one unpin, zero errors;
tracked object base 19687 decimal, three last-ref calls including
possible base reuse. All hooks are confined to the copied kernel;
production KCORE and ABI remain unchanged. No size work was run.
Permanent provider refusal and allocation-free automatic OOM
termination remain independent open gates.

## 2026-11-01 v899: real PIPE final close retry in full PDP-6 guest

The v870 allocation-free PIPE last-reference path now passes a
booted 256K PDP-6 real-MM fault-injection acceptance. A copied
kernel pins the exact anonymous PIPE physical object just before
its regular last-reference `mm_free`, which returns the genuine
MM_ERR_BUSY -5. The failed CLOSE leaves the read FILE descriptor
and original PIPE reader count intact; a normal word-read still
reports EOF because the writer closed earlier. The next CLOSE
unpins that extent, then the original MM release succeeds and
clears the descriptor. Repeated close is refused, and a fresh
independent PIPE opens/closes normally with stable FILE counts.
Guest SIMH counters: tracked physical base 19687 decimal, three
last-close calls including the new PIPE's possible base reuse,
1 pin, 1 unpin, 1 MM_BUSY, zero errors, normal HALT. The separate
native v870 final-PIPE test also passes. All hooks are test-only;
there is no production change, resident state, or size optimization.
This does not establish safe forced cleanup of permanently refusing
FILE/MM providers or authorize automatic OOM killing.

## 2026-11-01 v898: full-system real pinned TTY detach rejection

The v897 ownership guard now passes a second, stronger acceptance test.
A disposable 256K PDP-6 image builds the production KCORE with a
single test-only `proc.c` instrumentation hook. On the first TTY
`DETACH` it allocates a real canonical line extent, pins that exact
base through the actual PDP-6 MM allocator, and allows the regular
`mm_free` to reject release. The userland syscall observes -1 and the
controlling session remains attached. Before the second DETACH, the
test unpins the same base; normal MM cleanup succeeds. A subsequent
session ATTACH and an independently running child complete normally.
Full-system SIMH PASS: helper calls 4, pins 1, MM_BUSY 1, unpins 1,
errors 0, valid MM base 20005 (decimal), user success and HALT. No
production source or permanent data changed in v898. No KCORE size
optimization was attempted. Persistent provider and OOM kill gates
remain unresolved.

## 2026-11-01 v897: TTY canonical-buffer owner gate, no size sweep

With feature correctness taking priority over KCORE size reduction, the
process teardown audit found that the native `TTY_DETACH` transaction
ignored `mm_free` failure in `proc_tty_line_reset` and immediately
cleared the controlling session. A later `TTY_ATTACH` could claim the
free TTY while its previous owner's canonical SIXBIT input remained
pinned and readable. A new checked, allocation-free helper in `proc.c`
only reports success once the exact MM base has been removed. Native
`TTY_DETACH` refuses to change session state on rejection; the
previously unowned `TTY_ATTACH` path retries any retained buffer and
refuses a new session until release succeeds. Existing route bits are
preserved across the KCC helper call. TTY mode changes share the
checked helper. The policy is safe for both transient MM_BUSY and
persistent owner/type/extent inconsistencies; no forced free occurs.

Native PDP-6 process-control tests PASS and a negative control against
pre-v897 process assembly FAILS as expected. Another native test links
the **actual extracted C reset/checked-reset functions** against the
real PDP-6 `mm_free`/`mm_unpin`: pinned/wrong owner/missing extent all
retain the base, and the unpinned correct owner releases it exactly
once. The booted production PDP-6 `daimos-tty-session-state-v897.sh` also
PASSes the full attach/detach/reattach and independent child session
workload with headless console devices.
Automatic OOM termination remains disabled until remaining FILE,
TTY, MM and session lifetime gates are proved; no size sweep was run.

## 2026-11-01 v896: close RUN startup MM ownership window

Source audit found that the PDP-6 `RUN` path allocated and zeroed a
kernel-dynamic u-area, but only published its base and ownership flag in
the SIDL process descriptor *after* `vm_space_startup` returned.
`proc_run_startup_failed` already branches to `proc_slot_discard`,
which can only release a u-area represented in the descriptor. Thus a
startup rejection could leave an MM extent allocated with no owner in
the process table. The current PDP-6 startup helper normally returns
zero; fault injection is required to exercise this otherwise latent
rollback hole. The v896 change moves the existing three publication
instructions before the startup call; it adds no descriptor field,
static allocation or normal-path code and does not publish an SRUN child.
Full-system 256K PDP-6 acceptance injects one rejected startup at the
RUN-specific call site (the production helper normally returns zero),
then probes the exact u-area physical base with the real MM allocator:
one discard, zero leaked extents, successful subsequent RUN and WAIT,
normal HALT. The isolated v895-ordering negative control performs the
same guest sequence but detects **one** stranded MM extent despite
apparently normal process-slot and VM accounting. Native v885/v892/v894
cleanup regressions PASS. The full-system v197 16-case RUN FD-map
rollback also PASSes with three living children after correcting the
test harness's headless console setup. Identical lowmem disk-root GCC package size:
KCORE 9,426 words, KCORE+MRES 15,912, permanent endpoint 15,959,
all unchanged from v895, with 1,234 words still above the 8,192 goal.
This is not evidence that permanently broken FILE/MM providers can be
forcibly reclaimed or that automatic OOM termination is safe.

## 2026-11-01 v895: full-system pre-VM RUN rollback MM_BUSY accepted

The v894 native pre-VM rollback fix now passes a **booted 256K PDP-6**
test using the actual MM allocator. A user RUN loads `/SYSTEM/INIT`
and fails its fourth FD mapping. Only the copied test kernel pins the
unpublished child's real process VM immediately before the normal
`vm_space_destroy` call. Two genuine `mm_free` MM_ERR_BUSY (-5)
results retain the VM, SIDL+TRANSITION, scheduler RH retry bit 02 and
the read-only `GETTEARDOWN` stage 3 RUN_VM status. The existing
slot-zero reaper executes twice, unpins the real VM on the third
overall destructor attempt and finishes the rollback without creating
a child or parent WAIT report.

Native full-system `daimos-run-prevm-uarea-v895.sh` SIMH PASS:
target PID 3, one physical pin, three destructor attempts, two
actual MM_ERR_BUSY responses, one unpin, two slot-zero retry calls,
zero test errors. An independently scheduled user child runs and
exits normally. CTY `<RFBP` and normal HALT PASS. All fault
injection lives in a disposable copied test kernel; there is **no
new production source, binary, static RAM, ABI or size change**.
The last v894 GCC lowmem disk-root KCORE remains `022322` octal
(9,426 words), +1,234 above 8,192.

This completes the v894 pre-VM RUN MM_BUSY native-to-full-system
acceptance. Permanently rejecting or corrupted providers, wider
OOM exhaustion/victim/rollback coverage and the final resident size
reduction remain open; automatic OOM killing is still disabled.

## 2026-11-01 v894: retry pre-VM RUN cleanup without a u-area

`proc_slot_discard` previously cleared TRANSITION and returned when
`vm_space_destroy` rejected an unpublished RUN child's VM release.
The child remained SIDL with a live VM (and possibly a stable u-area),
but the v881-v893 reaper only recognized **post-VM** cleanup. The
slot could stay allocated indefinitely even if MM later unpinned it.

The resident PDP-6 path now retains TRANSITION and sets unused SIDL
scheduler RH bit 1 (`PROC_SIDL_DISCARD_VM_PENDING=02`). Slot-zero
maintenance recognizes only TRANSITION+SIDL+this bit, retries actual
VM destruction, and clears the bit *only when VM teardown succeeds*.
If u-area release subsequently fails, the existing v892
FINALIZE|DISCARD marker and reaper perform the independent second
stage. This handles children with and without an allocated u-area,
preserving backing/MM ownership and avoiding a premature FREE,
ZOMB, HUP, or WAIT event. The same one-word capped retry clock is
reused, with no new process descriptors or static RAM.

Read-only `GETTEARDOWN` now reports stage 3 (RUN_VM) plus the shared
retry interval for that SIDL owner, including before a u-area exists.
Existing stages 0/1/2 and their query ABI are unchanged. Native PDP-6
v894 fault injection PASSes two rejected VM releases per child,
successful VM release and an additional u-area retry, correct FREE
without parent event, and no duplicate VM/TTY cleanup. Native stage-3
diagnostics and v881/v883/v885/v886/v887/v892 regressions PASS.
Pre-v894 v893 source fails the v894 rollback negative control.

Paired GCC lowmem disk-root packaged builds: KCORE 9,381 -> 9,426
words (+45 code), KCORE+MRES 15,867 -> 15,912, permanent endpoint
15,914 -> 15,959. The full-profile permanent limit is a separate
issue. A booted guest with **actual** VM MM_BUSY and recovery remains
the next acceptance gate; automatic OOM killing remains disabled.

## 2026-11-01 v893: real MM-busy RUN rollback in a full PDP-6 guest

The v892 deferred unpublished-RUN cleanup is now accepted in a booted
256K PDP-6 full-system test with the **real MM allocator**. The isolated
test kernel pins the first rejected RUN child's existing u-area only
after successful production `vm_space_destroy`, then lets normal
`proc_uarea_release` call real `mm_free`. Two genuine MM_ERR_BUSY (-5)
results preserve the TRANSITION/VM_NONE/FINALIZE|DISCARD owner. The
slot-zero reaper retries twice; a test-only unpin before the third
real `mm_free` lets cleanup publish FREE once. An independent sleeping/
runnable user runs during this recovery. The parent never receives a
spurious WAIT event for the unpublished child.

Native full-system `daimos-run-discard-uarea-v893.sh` SIMH PASS:
target PID 3, 1 pin, 3 frees, 2 MM_ERR_BUSY, 1 unpin, 2 reaper calls,
0 test errors; guest CTY `<RFBP` and normal HALT. Fault injection
and counters are confined to the copied test kernel. **No DAIMOS
production code, ABI, KCORE size or static data changed in v893.**
The last same-profile v892 lowmem KCORE remains 9,381 words.

This closes the specific v892 post-VM RUN MM-retry acceptance gate,
not irreversible pre-VM rollback failures, indefinitely broken
providers or complete allocation-free OOM termination. Automatic OOM
killing remains disabled pending those independent correctness gates.

## 2026-11-01 v892: retryable post-VM RUN rollback

Fixed a resource-ownership hole in resident `proc_slot_discard`. An
unpublished RUN child whose VM destruction succeeded but whose u-area
MM free was rejected previously remained TRANSITION/VM_NONE without a
deferred-finalizer tag. The slot-0 service could not find it again.

The existing u-area word 067 now distinguishes **discard** from EXIT:
LH `0600000` means FINALIZE + DISCARD, with no new resident table or
allocation. A failed release leaves the uncommitted child owned and
arms the existing shared reaper/backoff word. Subsequent retries skip
VM destruction and TTY release. Successful release clears the slot
as FREE without a zombie, parent WAIT notification, or HUP. The old
EXIT marker, retry backoff and fatal FILE continuation are unchanged.

Native PDP-6 v892 fault-injection PASS: two failed releases followed
by successful rollback, one VM teardown and TTY release, exact-once
FREE, no EXIT/HUP/WAIT publication. The same test FAILS against
unmodified v891 at the missing marker. v881/v883/v885 regression
tests PASS. The existing full-system PDP-6 RUN v197 regression also
PASSes 16 post-VM fdmap rollback cases with three live children; it
does not inject an MM-free failure. A same-configuration GCC build links:
KCORE `022207` (9,351 words) at v891 versus `022245` (9,381) at
v892, +30 resident code words; KCORE+MRES 15,837 -> 15,867 and
permanent endpoint 15,884 -> 15,914 words. No new static allocation,
syscall, per-process descriptor or module. Full-system fault-injected
RUN cleanup and long-term provider recovery remain separate gates;
automatic OOM killing remains disabled.

## 2026-11-01 v891: permanent FILE-refusal containment accepted

The disposable PDP-6 testkit `daimos-fatal-file-persistent-v891.sh`
now exercises a real user APR fault with a VFS-sync provider that
continues returning -5 for FD 0. This differs from v889/v890, whose
providers eventually accepted the third close. The original owner
retains its private stack, FILE descriptor and u-area word-067 error
witness throughout the failing interval; `GETTEARDOWN(pid|0400)`
continues to report FILE stage 1, FD 0 and actual error class 5.

Native full-system PDP-6 SIMH PASS after 02400 octal timer ticks:
11 provider rejections, 11 FILE cleanup passes, 11 owner-stack
sleeps, retry delay capped at 01000 octal (512 decimal) ticks,
11 owned-FD checks, no forced close, witness clear or victim EXIT,
an independently running user process and zero test errors.
The test-only HALT stops the isolated guest after INIT checks the
read-only diagnostic; it does not imply that the victim was reaped.

**No DAIMOS source/binary or resident data changed.** Last accepted
KCORE size remains 022124 octal (9,300 words; 1,108 above the
8,192-word target). This is bounded CPU overhead and safe ownership
retention, **not** eventual recovery from a permanently broken
provider. Provider-specific repair, malformed MM owner diagnosis,
full exhaustion gates and automatic OOM termination remain open.

## 2026-11-01 v890: full-system destructive EXEC failure recovery

No production KCORE code or data changes. The disposable PDP-6
guest `daimos-fatal-exec-integration-v890.sh` uses an actual
user-mode `dsys_exec` with a valid versioned launch record.
The kernel's first staged `exec_load_process` is made to report
`EXEC_LOAD_NOMEM` only for the selected child, entering the
production low-memory EXEC fallback. The production
`vm_space_destroy` really destroys the old image; a test-only
observation requires its descriptor to be `VM_SPACE_NONE`.
Only **after** that irreversible commit does the isolated
replacement `exec_load_process` return -1. The normal
`EXEC_REPLACE_FATAL` return and `native_sys_exec` route
enter `proc_fatal_close_current_v879` on the original
process-owned executive stack.

The isolated VFS-sync hook returns -5 for the selected child's
FD 0 on its first two closes; the third goes through the real
VFS path. SIMH guest PASS: one staged-memory failure, one actual
VM destruction, one post-commit replacement-load rejection,
three FILE passes, three VFS calls (two -5 failures), two private
sleep intervals at 1 and 2 ticks, two retained-descriptor
observations, FILE stage 1/ERR 5 from `GETTEARDOWN`, exactly
one descriptor/witness clear and one terminal exit. The parent
receives the original fatal EXEC exit status 1 through `WAIT`.
CTY `<EH`, final HALT, and zero test errors PASS.

The v878/v879 native fatal EXEC, v885 FILE backoff, v888 real
provider error/MM classification, v887 detail and v881 deferred
MM tests also PASS. No production binary changed: KCORE remains
`022124` octal = 9,300 words, 1,108 above the 8,192-word
resident target, with zero new permanent data.

Transient APR-fatal and post-commit EXEC-fatal FILE rejection
recovery are now both full-system accepted. Indefinitely
rejecting providers still prevent guaranteed reclamation;
no forced close/unpin or automatic OOM killing is enabled.

## 2026-11-01 v889: full guest APR fatal FILE rejection acceptance

No permanent kernel code or data changes. The disposable PDP-6
guest test `daimos-fatal-file-integration-v889.sh` confirms that a
real user-mode APR protection/address fault enters the production
PI6 fault trampoline and `proc_fatal_close_current_v879` on its
process-owned kernel stack. Only the copied test kernel intercepts
the affected process's VFS-sync entry for FD 0, returning -5 on
its first two calls and allowing the third to run normally.

The guest records exactly one actual APR fault, three VFS-sync
calls, two rejections, three fatal FILE cleanup passes, two
owner-stack sleeps at 1 and 2 timer ticks, retained FD 0 after
each rejected close, and a valid `GETTEARDOWN` FILE stage with
error class 5 (actual returned -5). Successful provider service
clears FD 0 and the u-area word-067 error witness; the process
exits once and its parent `WAIT` receives the expected fault
status. Guest CTY output `<FH` followed by normal HALT, zero
test error counters: PASS.

The native v888 FILE/provider-error and MM result tests, v885
backoff, v878/v879 fatal EXEC, and v887 owner-detail fixtures
also PASS. No production source changes and therefore KCORE
remains `022124` octal = 9,300 resident words (+1,108 above
the 8,192-word goal). This accepts APR-fatal transient FILE
recovery; a true post-commit EXEC failure under a booted guest
and permanent provider refusal remain separate acceptance work.
Automatic OOM victim killing remains disabled.

## 2026-11-01 v888: preserve the rejecting FILE provider's return

The v887 FILE detail identified surviving descriptors and their
provider class but not the actual failed close result. On a private
fatal FILE-close continuation, the existing bulk-close scanner
now retains the **first rejected descriptor and its actual signed
provider return** in previously unused u-area word 067.
LH encodes valid flag 040 plus FD 0..017, RH the provider's
18-bit return. This word is reset after all closes succeed and is
later reused, as before, for the independent post-VM MM finalizer
marker/status. Nonfatal bulk closes leave word 067 untouched.
No static RAM, process-table field, heap or provider table is added.

The existing `GETTEARDOWN(pid|0400)` query now places a compact
actual FILE-close error class in result bits 15..17: 0 if not
recorded, 1..5 for returned -1..-5, and 6 for any other nonzero
provider result. The first rejected FD remains the first live FD
after the bulk pass. `ps` exposes the class in a numeric ERR column
alongside TD/RETRY/DETAIL. For deferred MM, v887's exact-extent
pin/type/owner probe already distinguishes a busy extent from
invalid metadata; this is a snapshot, not a historical errno.

Native PDP-6 `daimos-file-provider-error-v888.sh` executes the
production bulk-close and diagnostic routines with externally
mocked rejecting providers. It verifies first-return capture,
multiple failures, preserved descriptors, error replacement on
retry, cleanup success resetting word 067, and a nonfatal close
never writing the marker: PASS. Pre-v888 negative control FAILs.
Earlier native provider/backoff, v887/v886 diagnostics and fatal
event regressions PASS. Actual production `mm_free` native
validation also PASSes pinned-busy versus missing/type/owner-invalid
distinctions, preserved ownership and successful release after
unpin. The ordinary full-system PDP-6 session-leader HUP/TTY
detach/reattach regression PASSes with the modified FILE runtime;
it does not inject fatal-private-stack provider rejection.
KCORE lowmem link PASS at `022124` octal
(9,300 words), +26 resident words over v887 and +1,108 over 8192,
with no additional permanent data. Automatic OOM killing remains
disabled; no timeout-based forced release is authorized.

## 2026-11-01 v887: inspect actual FILE and MM teardown owners

Existing `procctl(GETTEARDOWN, pid)` remains unchanged. Set bit 8
(octal 0400) in the PID argument to request a read-only detail
snapshot; bits 9..17 are rejected. No UUO number, process-table
slot, resident diagnostic buffer, or persistent provider counter
is introduced.

For a fatal FILE owner, the result encodes stage 1, the number of
still-open entries among the existing 16 descriptors, the first
remaining FD (020 means none), and its provider class (VFS, PIPE
or DPY raw-list). The original FILE table and provider's close
policy remain untouched. A descriptor remaining after the
bulk-close pass is a **current outstanding reference**, not proof
of a permanent provider error.

For a post-VM deferred MM owner, the new read-only helper reuses
the allocator's existing exact-base lookup. It checks the
u-area's MM_TYPE_KERNEL_DYNAMIC and owner 01000+PID before
returning the actual pin count (0..255). Distinct sentinels 0400,
0401, 0402 mean missing extent, wrong type, wrong owner, each
packed above stage 2. This distinguishes an actual MM_BUSY pin
from invalid allocation state; it never forcibly unpins or frees.

The `ps` program now adds a `DETAIL` numeric column to its existing
`TD` and `RETRY` fields. It prints the packed detail result rather
than allocating buffers or formatting raw provider addresses.

Native PDP-6 detail validation PASS: ordinary/invalid states,
open FILE counts, PIPE/DPY classification, release-to-next FD,
exact MM pin counts, zero pins, missing extent, type mismatch and
owner mismatch. v886 compatibility and dispatch regressions PASS.
The pre-v887 source FAILs the detail negative control as expected.
The full-system PDP-6 guest PASSes the user-mode detailed UUO,
one real u-area pin, two MM_ERR_BUSY failures, successful unpin
and cleanup, runnable companion and normal HUP/TTY completion.
KCORE lowmem link PASS at `022072` octal (9,274 words), +72
over v886, no new permanent RAM. The higher code size remains
above the 020000-octal-word target; no OOM victim killing enabled.

## 2026-10-31 v886: readonly teardown diagnostics

The existing UUO-077 extension dispatcher forwards unused
SYS_PROCCTL operation 041 to a new
`GETTEARDOWN(pid)` without adding a syscall entry or permanent
process table. It returns -1 for invalid/FREE slots, 0 for ordinary
and fully committed zombie processes, 1 for a fatal FILE owner
holding its private kernel stack, or `(ticks << 2) | 2` for an
irreversibly terminated process whose u-area remains quarantined
after VM destruction. The MM retry ticks are read from the
already resident shared reaper-clock word (16..512 ticks); they
are not individual per-process retry counters. The FILE-specific
private-stack delay is deliberately not exposed. No kernel address,
FILE pointer or MM extent address is returned.

Operation 020 was not free (it is `pipe_create`); choosing 041
preserves the preexisting pipe and other extension ABIs and adds
only four packed readonly dispatch-table words.
The `ps` user utility now reports numeric `TD` stage and `RETRY`
columns, offering a practical way to identify stuck termination
without unsafe forced release. Normal PID queries continue to
return zero. The native PDP-6 handler fixture exercises invalid,
FREE, ordinary, FILE, MM, stale marker, no-VM and zombie cases:
PASS. The old reserved-op dispatch FAILs the negative control.
The full-system PDP-6 test PASSes user-mode normal/invalid PID
queries and actual pending MM state while a separate process
spins; it records two real MM_ERR_BUSY releases, later recovery,
and normal HUP/TTY completion. PIPE creation stays functional.
The PS program compiles with the PDP-6 compiler. KCORE
lowmem link PASS at `021762` octal (9,202 words), +37 over v885,
no new permanent data. Persistent resource refusal still has no
safe automatic forced reclaim; OOM killing stays disabled.

## 2026-10-31 v885: capped fatal FILE and deferred MM retry overhead

An unavailable FILE provider previously forced the fatal APR/EXEC
owner to sleep exactly one tick and retry closure again, indefinitely.
v885 keeps a one-word retry interval on the already allocated private
kernel stack. Consecutive provider failures sleep 1, 2, 4, 8, ...
through 01000 octal ticks (512 = about 8.5 seconds at 60 Hz);
later failures stay capped. The existing FILE handle owner, fatal
control bit, saved exit status, and private continuation remain
intact until close succeeds. No forced release or status loss occurs.

For post-VM MM u-area release, the preexisting reaper clock word
now carries pending in its highest bit, the capped retry interval
in remaining LH bits, and its prior last-service clock in RH.
An initial pending owner requests a 020-octal-tick (16-tick)
retry; each unsuccessful service doubles the shared interval
through 01000 octal ticks. The next new owner resets the interval
so it does not inherit a permanently pinned victim's delay.
Once no pending owner remains, the entire LH is cleared.
No permanent counter, table, or extra u-area word is allocated.

Native PDP-6 capped FILE and MM backoff tests PASS (14 rejected
FILE closes and later success; several MM rounds through the cap,
new-owner reset and successful final clear). Earlier fatal close,
deferred MM, non-current HUP, scheduler and swap tests PASS.
Pre-v885 FILE close negative control FAILs as expected.
The full-system continuously runnable companion/real MM_BUSY
transient recovery also PASSes. KCORE exclusive lowmem end
`021715` octal = 9,165 resident words, +30 over v884, 973
above 020000. A permanently refusing provider is **not** forcibly
freed: reclamation remains unbounded and OOM killing remains off.

## 2026-10-31 v884: bounded slot-0 maintenance turn under CPU load

The v881 idle-only deferred u-area finalizer could starve behind a
continuously runnable user. v884 uses the high bit of the existing
`proc_finish_last_tick_v881` word as the pending flag, with the low
17 bits retaining the last service epoch. A failed u-area free arms
the flag, and the service pass clears it before retrying, so only
remaining failures re-arm it. No new permanent data or per-process
queue is allocated. On a qualified PI6 quantum, pending cleanup
at least 020 octal ticks old receives one slot-0 maintenance turn.
A runnable RT owner and pending disk swap take precedence; a
sleeping/stopped RT lease does not starve cleanup. After the pass,
slot 0 immediately requests normal scheduling instead of occupying
a further user quantum. The no-pending idle path skips MM scanning.

Native PDP-6 scheduler, 17-bit timer wrap, RT/swap, dormant RT,
v879 VM-less fatal and v881/v883 finalizer/event regressions PASS.
Pre-v884 scheduler FAILs the new negative-control test as expected.
Full-system MM pin/unpin/HUP with an independent nonstop spinner
PASSes (2 reaper passes with a nonempty run queue, 3 frees, 2 actual
MM_ERR_BUSY, 1 pin/unpin, no errors). KCORE link PASS at `021657`
octal (9,135 resident words), +39 over v883 and +943 above 020000.
Permanent resource refusal and hard RT owner starvation remain open;
automatic OOM victim killing is still disabled.

## 2026-10-31 v883: committed non-current fatal delivery

Non-current fatal events previously returned an ambiguous -1 after
`proc_finish_slot` had already destroyed the target VM but failed
to release a pinned u-area. A session leader therefore treated a
successfully delivered HUP as failed, preventing synchronous session
teardown although slot 0 owned the deferred finalization.
`proc_event_apply` now returns success for precisely the verified
TRANSITION + VM_SPACE_NONE + u-area word 067 pending marker state.
Failure before irreversible teardown, including FILE close rejection
and VM destroy failure, still returns an error. Duplicate events
remain rejected while the process is quarantined. The C fallback
mirrors this contract. Native PDP-6 acceptance and pre-v883 negative
control PASS/expected FAIL (including injected VM precommit failure),
along with earlier FILE/EXIT/finalizer regressions. KCORE exclusive
end `021610` octal (9,096 words), +17
over v882 and 904 above 020000; no permanent data change.
The full-system disposable-kernel non-current HUP injection also
PASSes: target slot 2, 3 final u-area frees, 2 genuine MM_ERR_BUSY,
1 MM pin and unpin, no hook error, normal session HUP/TTY HALT.
The unchanged current-EXIT v882 variant also PASSes.

## 2026-10-31 v882: full-system genuine MM_BUSY retry acceptance

A testkit-only full-system PDP-6 image pins a real stable u-area on
its first process-owned EXIT, verifies two genuine `mm_free`
`MM_ERR_BUSY` responses, then unpins on the third release attempt.
Guest counters read through SIMH after normal HALT report:
target slot 1 in the manual run and slot 2 in the clean run,
3 calls, 1 pin, 2 busy, 1 unpin, 0 errors.
The same session HUP/TTY scenario completes. Fault instrumentation
is generated only inside a disposable test source tree; production
KCORE is unchanged, still `021567` octal (9,079 words).
Non-current fatal finalizers return a transient failure to their
caller and need a separate acceptance contract. Permanent MM errors
and progress under sustained runnable load remain open.

## 2026-10-31 v881: defer failed post-VM u-area release from slot 0

When MM refuses to release an exiting process's stable u-area after
VM destruction, its finalizer now retains TRANSITION, VM_SPACE_NONE,
the u-area owner and exit status instead of trapping the machine in
the slot-0 HALT path. Pending status is encoded in formerly unused
u-area word 067 (LH marker, RH status). Session, domain and event
control bits remain unchanged. The idle kernel retries eligible
finalizers once per clock tick with scheduler PI6 inhibited, without
allocating any per-slot queue. A successful retry removes the runq
entry, adopts children and publishes FREE/ZOMB exactly once.
Native PDP-6 retry, untagged-transition exclusion and the compiled
production C EXIT return behavior PASS. Old close, HUP and VM tests
remain PASS. Lowmem KCORE links at `021567` octal (9,079 words):
+100 over v880, 887 over the strict 020000 limit. One resident
clock word, no new process descriptor/u-area words. Persistent MM
errors and the full-system injected-failure / OOM tests remain open.
Ordinary full-system session leader HUP and TTY regression PASSes.

## 2026-10-31 v880: transactional disk-swap detach and VM ownership

`proc_swap_detach` now returns an error when its nonresident backing
run is incomplete or invalid. It checks the entire allocation through
the existing SWAP MRES bitmap verifier before freeing blocks and
clearing its swap record. `vm_space_destroy` propagates a failed
detach and does not publish VM_SPACE_NONE, retaining a retryable
process owner. A one-word resident jump entry is patched by MINIT to
SWAP export 6; no new persistent bitmap/table or duplicate verifier.
Native PDP-6 VM error/rollback and real C detach fault injection PASS;
pre-v880 VM code fails the negative control as expected. Lowmem KCORE
link PASS at `021423` octal (8,979 words), +16 over v879.
Persistent MM/u-area release failure after VM destruction remains open.

## 2026-10-31 v879: process-owned fatal cleanup retry

APR-user faults and post-commit EXEC failures now keep an owned kernel
continuation for retrying failed FILE closes. The existing per-u-area
control bit 2 marks the continuation; event delivery, OOM selection
and swap-victim selection exclude it. Failed closes yield a clock tick
on the private stack instead of HALTing immediately on the slot-0
stack. A tagged VM-less EXEC-fatal continuation is schedulable and
restores in executive mode without loading an invalid APR map.
Native PDP-6 owner-stack, VM-less selection/restore, swap-out guard,
EXEC/FILE/EXIT, HUP and OOM selector tests PASS. Untagged
VM_SPACE_NONE (-1) is now rejected in native/C scheduling, including
RT and ready-once selection. The pre-v879 swap-out guard FAILs the
negative-control test, as expected. Lowmem KCORE link PASS at
`021403` octal (8,963 words), +108 words over v878 and 771 above the
strict 8,192-word target; no additional persistent data words.
Full-system failed-provider/PI6 acceptance, bounded total cleanup,
post-VM destruction release failures and OOM termination remain open.

## 2026-10-31 v878: post-commit EXEC must close FILE descriptors

`native_sys_exec` now closes the stable process FILE table before
nonreturning `proc_exit_current` after `EXEC_REPLACE_FATAL`. This
prevents the v869 finalizer guard from rejecting otherwise closable
descriptors solely because the fatal EXEC branch skipped cleanup.
Native v878 test including pre-v878 negative control PASS/expected
FAIL; earlier EXIT, FILE and finalizer tests PASS. Lowmem KCORE link
PASS at exclusive endpoint `021227` octal (+1 word). No new static
data. Persistently failed closes and APR fatal cleanup still need
process-owned deferral before OOM termination can be enabled.

## 2026-10-31 v877: full-system session teardown acceptance

Post-v876 `daimos-session-teardown-v1.sh` PASSes in a headless
CTY/DPY guest with unrelated graphical color-scope devices disabled.
It covers leader exit, HUP to running/stopped members and TTY reattach.
The earlier unisolated run had crashed in SDL before INIT. This is
ordinary session-lifecycle acceptance, not injected failing cleanup
or automatic OOM termination. No code or size change in v877;
resident KCORE endpoint remains `021226` octal.

## 2026-10-31 v876: exclude and preserve RT victim through HUP errors

The native finalizer now marks TRANSITION before sending session HUP,
rejects pre-existing TRANSITION, and leaves RT ownership intact until
successful VM destruction. HUP and VM-free failures clear TRANSITION
while preserving the live process's RT ownership. Native PDP-6 v876
fault-injection PASS and pre-v876 negative control FAIL as expected.
Lowmem KCORE link PASS: exclusive end `021226` octal (+4 code words).
No new persistent words. APR/EXEC nonreturning cleanup and OOM kill
remain unresolved.

## 2026-10-31 v875: session HUP cannot detach TTY prematurely

Session-leader teardown now leaves the existing controlling-TTY record
intact until every member's fatal HUP returns success. A failed FILE
close in one member can no longer silently detach the still-live
session before its leader's release reports failure. Native PDP-6
failed-HUP, successful-retry and nonleader tests PASS; the pre-v875
negative control FAILs as expected. KCORE link PASS, exclusive lowmem
endpoint `021222` octal (+4 words versus v874). Nonreturning APR/EXEC
exit and complete OOM termination remain unverified.

## 2026-10-31 v874: transactional Type 340 replacement

The optional DPY MRES no longer publishes a replacement display list
before the old extent is freed. Failed old-list release rolls back the
unpublished new allocation, retains the original list and IOWD, and
returns an error. Native PDP-6 replacement/rollback fault injection,
existing DPY close and interrupt tests, and DPY MRES link PASS. The
lowmem KCORE endpoint remains `021216` octal; no extra resident KCORE
words or persistent state were introduced. Full-system replay after a
forced MM failure and automatic OOM termination remain unverified.

## 2026-10-31 v873: named-FIFO full-system test baseline

The unchanged KCC named-FIFO full-system test times out after D6FS
startup on both pre-v870 v869 and current v871. This is not a PASS;
the stall remains undiagnosed. Native v870 MM-free fault injection
provides a positive repair check and a failing pre-v870 control.
No new KCORE code or size change in v873.

## 2026-10-31 v871: DPY CLOSE propagates failed MM release

FILE no longer masks a Type 340 raw-list release failure with the
device VFS_SYNC no-op. The display MRES preserves its list base and
IOWD when `mm_free` fails, permitting retry and refresh restoration.
Absent DPY CLOSE succeeds without an owned list. Native v871 and
updated v709 gateway tests PASS; the DPY assembler and lowmem KCORE
link PASS at `021216` octal, one word below v870. DPY replacement
and broader OOM termination remain unfinished.

## 2026-10-31 v870: retry-safe final PIPE/FIFO close

The last `pipe_close_ref` no longer changes reader/writer counts before
the final `mm_free` succeeds. Failed release leaves the descriptor and
original reference counts for retry; named FIFOs are relinked to the
existing list without allocation. Native PDP-6 v870 final-release fault
injection PASS; lowmem KCORE link PASS at `021217` octal, six words above
v869. OOM killing is still disabled pending a complete teardown audit.

## 2026-10-31 v868-v869: current EXIT and final FILE teardown guard

Ordinary PDP-6 EXIT and current fatal-event handling now honor failed
`file_close_all` results, leaving the requesting process and any failed
descriptor intact. `proc_finish_slot` additionally validates the stable
u-area and all 16 FILE descriptors before any resource release. Native
PDP-6 testkit v868 and v869 PASS; lowmem KCORE link PASS at `021211`
octal, 18 words larger than the v867 `021167` baseline. These changes
prevent silent descriptor loss but do not activate OOM termination.
Nonreturning APR and EXEC fatal exits, provider waits and full OOM
resource-progress acceptance are still unresolved.

## 2026-10-31 v867: failed FILE close blocks fatal-other teardown

Native KCORE `file_close_all_at_v865` now reports unsuccessful descriptor
closure. The fatal-other event path does not free its target u-area when
one or more FDs remain open due to failed VFS sync. Native PDP-6 v867
fault injection PASS; v865 ordinary explicit-table close PASS. Lowmem
KCORE linked without additional permanent data. This protects against
orphaning target resources but is not an enabled OOM kill path. Provider
blocking, current EXIT and recursive teardown still need OOM-safe proof.

## 2026-10-29 v670: root mount failure staged

Native KINIT trace confirms BLOCKSET discovery, block reads and D6FS
superblock selection are reached before `?RT`; neither VFS mount nor
VFS stat is reached. Testkit v670 diagnostic PASSes as a failure-stage
classification only. Full-system swap acceptance remains BLOCKED.

## 2026-10-29 v669: full-system swap pressure blocked

The v298 test assumes swap transactions are in `kcore.map`; current
swap transactions reside in loadable SWAP MRES. Testkit v669 corrects
the test's symbol assumptions, but a no-breakpoint native run halts
at `?RT`/`065206` before workload completion. Full-system swap-out
recovery remains UNVERIFIED; no kernel size/production change.

`?RT` originates from `kinit_boot()` root mount/INIT validation,
not from the swap transaction code.

## 2026-10-29 v668: native swap MM-free rollback

PDP-6 native execution verifies successful unpin followed by failed
MM free retains resident ownership and original swap record, clears
TRANSITION, and frees provisional BACKING. No KCORE optimization.

## 2026-10-29 v667: swap-out MM unpin failure invariant

The PDP-6 swap-out path now rejects either failed unpin through the
existing storage/MM invariant halt. Returning to the scheduler with
uncertain pin ownership is no longer permitted. This is a correctness
fix, not a KCORE size optimization. Source and isolated native PDP-6
unpin fault tests PASS; fresh 32K boot reaches `proc_boot_init`.

## 2026-10-29 v666: late swap MM failure ordering audit

Source control-flow regression verifies MM unpin/free failure branches
before DISK publication and provisional BACKING cleanup. No production
code modified; native late-MM fault injection remains open.

## 2026-10-29 v665: BACKING partial-write fault acceptance

Native PDP-6 test confirms a failure on block 2 of a 3-block BACKING
write stops further I/O and returns an error. Swap/MEMFS write-failure
publication ordering checks PASS. This is not a claim of physical
multi-block atomicity; full-system fault recovery is still open.

## 2026-10-29 v664: native missing-VOLUME-mapper test

PDP-6 native instruction test executes actual `fs_mres.s` BACKING I/O
with an unavailable mapper: mapped VOLUME fails without device I/O;
direct DEVICE mode succeeds. Full-system live faults remain open.

## 2026-10-29 v663: native multi-member VOLUME acceptance

The unchanged D6FS HDD native multi-member suite passed with host-native
relay helpers, covering two/four members, file I/O and invalid-member
rejection. Testkit v663 now regenerates the relay helpers to avoid
reusing wrong-architecture binaries. Native mapper-loss injection is
still open; no KCORE optimization performed.

## 2026-10-29 v662: BACKING runtime vocabulary

MonitorFS SWAP stats now query `backing_region_blocks`, matching
process swap initialization. The physical I/O gateway is renamed
internally without altering callable aliases, media formats or
resident state. No optimization sweep.

## 2026-10-29 v661: direct LOG REGION interface

Reclaimable LOG boot I/O now uses bounded AUXSTORE REGION operations
without redundant LOG-specific wrappers. LOG record/recovery ownership
and its permanent MRES ABI are unchanged. Not a KCORE size sweep.

## 2026-10-29 v660: DEVICE/VOLUME/REGION state naming

The three preexisting resident backing-service state words now have
clear DEVICE/VOLUME/REGION names; old external symbols remain aliases
at their original addresses. Swap boot calls `backing_region_blocks`.
No additional resident state, abstraction, or optimization.

## 2026-10-29 v659: VOLUME fail-closed I/O semantics

Multi-member VOLUME mapping no longer falls back to direct physical
sector arithmetic if the mapper is unavailable. Direct DEVICE I/O is
unchanged. Fresh native 32K GCC disk-root boot and source regressions
PASS; native mapper-loss injection pending. No optimization sweep.

## 2026-10-29 v658: DEVICE/VOLUME first structural split

Direct physical ranges and multi-member root tails now have explicit
configuration entries, preserving original BLOCKSET ABI and one
DSK/DRM I/O implementation. No size optimization. See
`design/device-volume-separation-v658.md`.

## 2026-10-29 v657: shared backing feature interface

First BSTORE feature increment: bounded raw AUXSTORE region read/write
API for three partitions, used by LOGSTORE boot I/O. Host regression
PASS, earlier AUXSTORE selection/overlap PASS. No KCORE size
optimization performed. Further production BACKSTORE/BCACHE/recovery
integration remains open.

## 2026-10-29 v656: host mm-core fixture repaired

daimos-testkit `make mm-core` now PASS after restoring test-only VM
host lifecycle coverage and fixing runtime descriptor and process-slot
expectations. No production KCORE code or RAM changes. See
`design/mm-core-host-fixture-v656.md`.

## 2026-10-29 v655: physical root media loss and KINIT build-coherence fix

Native 96K GCC full-profile boot with DSK0 detached after Stage1
reaches fatal HALT; validated stack watermark `0172` octal (122 words)
inside `0400` reserve (256 words). Fixes KINIT reserve/watermark
Makefile-mode invalidation: generated C and assembler helpers must
agree. A `0577` measurement from a previously reused mixed-mode build
was spurious. Native loss and 1/0/1 mode-switch tests PASS. No new
permanent KCORE memory. See `design/kinit-controller-media-loss-v655.md`.

## 2026-10-29 v653: invalid /SYSTEM/INIT rejection accepted

Native full-profile PDP-6 SIMH rejects missing INIT directory entry
and nonregular INIT FCB with `?RT`/HALT after D6FS initializes.
Disposable disk copies; original media CLEAN; testkit PASS for both.
No production code or RAM change.

## 2026-10-29 v652: KINIT reserve reduced to 256 words

Default KINIT bootstrap reserve `0600` -> `0400` octal, based on
previously measured worst-case failure watermark `0172` octal (122
words). Fresh 32K GCC lowmem instrumented image ends `077260` and
boots to `proc_boot_init`; image plus reserve ends at `077660`, below
`0100000`. Invalid-root HALT uses `0164` octal. No permanent KCORE
growth. See `design/kinit-stack-reserve-256-v652.md`.

## 2026-10-29 v651: root BLOCKSET read-error stack bound

Native 96K GCC full-profile KINIT: runtime-only BLOCKSET read-service
failure during root mount reaches `?RT`/HALT. Watermark `0172` octal
(122 words) below `0600` reserve. Testkit PASS; production unchanged.

## 2026-10-29 v650: native MM allocation-failure stack bound

96K GCC full-profile KINIT: runtime-only SIMH fault at linked idle-stack
`mm_alloc` request forces a genuine allocation failure. Fatal HALT
watermark `0172` octal (122 words), reserve `0600` octal. Testkit PASS;
source and persistent media unchanged. I/O and missing-INIT pending.

## 2026-10-29 v649: D6FS mount-validation fatal-path stack

Native 96K GCC full-profile instrumented disk-root boot with corrupted
copies of both D6FS superblocks reaches `?RT`/HALT and records stack
`0172` octal (122 words) against `0600` reserve. Testkit PASS. This
is a distinct mount-validation path; allocation/I/O failures remain.

## 2026-10-29 v648: explicit disk-selector fatal stack bound

Native 48K tape-root PDP-6 SIMH `SW=1` failure HALT records
`0164` octal (116 words) versus `0600` reserved. Testkit PASS.
This shares early selector-failure coverage; D6FS mount and allocator
failure-path measurements are still open. No production RAM impact.

## 2026-10-28 v647: full-profile native DSH acceptance

GCC/KCC at 96K/256K: native ROOT login, DSH, ECHO, external TRUE,
STATUS:0, HALT and CLEAN D6FS shutdown PASS in all four configurations.
An earlier ?RT was caused by reusing test media left DIRTY at the
pre-userspace breakpoint, not by a filesystem implementation failure.
See `design/kinit-dsh-full-profile-v647.md`.

## 2026-10-28 v646: 256K GCC full-profile KINIT acceptance

Native 256K GCC full-profile kernel initialization PASS, KINIT end
`107421`, stack reserve `0600`, D6FS/BSET/MEMFS loaded, process-init
breakpoint at `063067`. No production code change.

## 2026-10-28 v645: KCC 256K full-profile initialization

Fresh KCC full-profile 256K native SIMH image PASS: image end
`113573` octal, stack reserve `0600` intact, correct `MEM 256 K`
and D6FS/BSET/MEMFS initialization before `proc_boot_init`.
See `design/kinit-full256-kcc-v645.md`.

## 2026-10-28 v644: KCC 96K full-profile KINIT

Native KCC full-profile GCC-comparison gate PASS at 96K: linked KINIT
ends at octal `113573`, fits the unchanged `0600` reserve, and boots
to `proc_boot_init` after D6FS/BSET/MEMFS device initialization.
Userspace and 256K acceptance remain separate.

## 2026-10-28 v643: GCC 96K full-profile KINIT

Native 96K GCC disk-root full-profile KINIT reached `proc_boot_init`
after initializing DSK270, DRM236, BSET, D6FS, and MEMFS. The linked
`107421` image end plus `0600` reserve fits. No production changes.

## 2026-10-28 v642: lowmem KINIT installed-core matrix

The native 48K/96K/256K installed-core early-boot matrix passes
with the same instrumented lowmem tape-root image. All reach the
`proc_boot_init` breakpoint with correct MEM banners and device probes.
The full-profile and userland gates remain outstanding.

## 2026-10-28 v641: 32K KINIT-only boot test

Native PDP-6 SIMH 32K kernel-only test PASS. The test uses a
map-derived breakpoint at `proc_boot_init` after hardware/TSFS boot,
before userland. DTC, DSK270 and DTFS markers pass; octal `0600`
KINIT reserve is retained. No production kernel mode added. See
`design/kinit-kernel-only-v641.md`.

## 2026-10-28 v640: KINIT fatal-HALT watermark verification

For instrumented KINIT builds, both ordinary and short-code fatal
HALT paths record the existing stack high-water word. Native 48K
invalid-root boot HALT measured `0164` octal (116 decimal) stack words
versus a `0600` octal reserve. Production KCORE and KINIT are unchanged
when watermarking is off. See `design/kinit-failure-watermark-v640.md`.

## 2026-10-28 v639: KINIT watermark bounds

Corrected the optional watermark fill/scan inclusive endpoint in
transient KINIT assembly. The 32K `077200` image plus `0600` reserve
has exclusive endpoint `0100000`, outside installed 32K core.
48K watermark-enabled native boot PASS; full 32K failure-path stack
usage remains unmeasured. See `design/kinit-watermark-bound-v639.md`.

## 2026-10-28 v638: KINIT transient-only 32K image reduction

PDP-6 native IDIVI decimal memory formatting, the blank SIXBIT count
loop, and complementary memory probes replace larger GCC-generated
transient KINIT routines. Link end `077236` -> `077200`: 30 decimal
words recovered, with the original `0600`-octal stack reserve intact.
Native 32K TSFS+AUXSTORE boot and `MEM 32 K` diagnostics PASS; the
linked image sits exactly at the boundary, so stack watermark and
other-profile tests remain outstanding. Permanent KCORE is unchanged.
The generated DRM DBOOT descriptor is converted to AUXST1 only after
strict signature/layout checks and explicit build-time confirmation.
See `design/kinit-lowmem-footprint-v638.md` for full evidence.

## 2026-10-27 v512: pre-final-sweep functional gate

Consolidated the unresolved functional prerequisites in TODO.MD. Before the
final KCORE size pass, close UTC clock/DATE/SETTIME, explicit swap formatting,
combined device/BLOCKSET/provider inventory, MonitorFS swapped-process
metadata correctness and scheduler/compiler regression gates. Decide whether
the production D6 codec cutover, TSFS v1.2, remainder of BSTORE migration,
privileged raw MEMFS XIP and optional compressed swap are in release scope;
if yes, implement and test them before freezing the permanent image. They
are not all automatically required for version 0.9. Keep the accepted v508
lowmem size numbers as the baseline, and do not resurrect obsolete mount
capacity v13-v21 substeps or failed MEMFS auto-loading proposals. No kernel
implementation or size saving is claimed by this inventory.

## 2026-10-27 v513: fresh current GCC size results (no code change)

Re-ran `permanent-size` on current sources, not the older v219/XIP-linked
10,844-word snapshot. All three builds link KCORE as `020544` octal,
8548 decimal words. 32K GCC lowmem TSFS/tape reports KCORE+MRES
`032227` = 13463; endpoint `032306` = 13510, 2873 words free below
octal `040000`. 32K GCC lowmem D6FS/disk reports KCORE+MRES
`032343` = 13539; endpoint `032422` = 13586, 2797 free. Disk multi
endpoint `032463` = 13619 (2764 free).

The 256K GCC **full/auto** package reports KCORE+MRES `043537` = 18271,
single-root LAST `043616` = 18318 (1934 over the 16K permanent limit)
and multi-root LAST `043657` = 18351 (1967 over). `permanent-size`
**fails** for this profile. This is a package/address-space budget, not
a measured peak of concurrently resident optional MRES/MM allocations.
No production kernel changes and no attributed word savings in v513.
`TODO.MD` now has the consolidated outstanding XIP path and deferred
512-decimal-word granularity investigation, explicitly separating PDP-6
hardware APR alignment from software allocation policy.

## 2026-10-27 v511: KCORE worklist reconciliation

P-1 documentation cleanup checked the newest v507-v510 changes before
continuing size optimization. AUXST1 raw DSK/DRM safety and native swap
acceptance are complete; an additional D6FS superblock reader for raw
AUXSTORE would incorrectly reject valid unformatted partitions. Remaining
combined-provider/BLOCKSET coverage is distinct. Next live size audit is
the mandatory `proc_swap` resident object and its optional swap service
entry points; first establish actual call-graph/lifetime requirements and
measure the complete linked image before changing any code. The strict
32K full DSH command gate is deferred to a separate DSH optimization run:
boot-only kernel tests remain at 32K, while DSH functional tests use
48K/96K/256K. No kernel change or claimed saving in this checkpoint.

## 2026-10-27 v510: root-only D6FS validation and next KCORE audit

Verified call sites: `d6fs_boot_mount_root()` invokes
`blockset_boot_discover()`, reads both superblocks and calls
`d6fs_super_select()` before mounting. `kinit_boot()` routes DSK/DRM
root classes to that path, while TSFS root uses the separate tape path.
The D6FS runtime MRES mount gateway is prevalidated, not a general
unvalidated non-root partition discovery interface. Accordingly there
is no existing non-root D6FS consumer for an additional automatic
KINIT superblock check. Preserve the raw AUXST1 contract and avoid
inventing a loader or redundant read-only parser solely for discovery.

Commenced the next deferred KCORE audit: the `proc_swap` fixed object
is a concatenation of compiled `vm_pdp6_swap.c` (with several transaction
functions disabled in favor of assembly), PURE backing assembly and
storage-runtime assembly. The actual physical swap service is packaged
separately as the optional 412-word `swapv481` MRES. Resident symbols
such as `proc_swap_detach` also clear executable backing and mount
provenance, so removing the entire fixed object for tape-only builds
would be unsafe without a precise call-graph and full-link measurement.
No kernel change or RAM saving is claimed in this audit checkpoint.

## 2026-10-27 v509: AUXST1 D6FS superblock compatibility finding

Inspected `auxstore_boot.c`, `d6fs_super_boot.c`,
`d6fs_boot_mount_root()`, and host `mkauxstore`. The v507 AUXST1
validator already rejects out-of-device, overlapping, and inconsistent
partition offsets. Crucially, `mkauxstore` records D6FS partition and
relative superblock *locations* but does not format their contents.
Consequently, a valid TSFS+AUXST1 swap volume may contain an unformatted
D6FS partition. Requiring `d6fs_super_select()` to succeed during
AUXSTORE discovery would break the existing v494/v508 native swap cases.

For D6FS **root** mounts, KINIT already reads both superblocks via
`blockset_boot_read()` and uses `d6fs_super_select()` to validate
generation, UUID, CLEAN state, and reservation structure before mounting.
For AUXSTORE **raw** backing, the validated block-zero AUXST1 descriptor
is authoritative; no D6FS filesystem is mounted or required. Its
partition-superblock locations must not be mistaken for a requirement
that D6FS is formatted. No additional permanent metadata, I/O or code
has been introduced. The new testkit v509 regression verifies that a
legal AUXST1 descriptor is accepted with only its descriptor-block read.

Future work, if separately required: validation of an explicitly
*selected* formatted non-root D6FS/BLOCKSET set before it is published
as D6FS-backed storage. Do not introduce such verification into plain
AUXSTORE detection, and do not weaken existing D6FS-root checks.

## 2026-10-27 v508: positive TSFS+DRM0 AUXST1 swap acceptance

A new daimos-testkit native regression provisions a private 020000-block
DRM0 AUXST1 image and boots 32K TSFS with the v506 optional DRM MRES.
The workload successfully swaps out a sleeping pipe reader, wakes it,
restores the swapped process via slot zero, and obtains WAIT status.
A first attempt failed during host image provisioning because its
CACHESTORE tail did not exactly fill the medium; the corrected fixture
passes with a longer 180-second runtime limit for drum operations.
The earlier positive 32K DSK0 AUXST1 test remains passing.

Fresh GCC lowmem full-image sizes: 8548 KCORE words, 13463 packaged
KCORE+MRES for tape and 13539 for disk. Default KINIT stack remains
0600 octal (384 decimal) words. Existing native high-water readings
129/136/201 words are observations, not a maximum over every failure
path. No memory-growth optimization or runtime kernel change in v508.
Next chat starts KINIT-only D6FS superblock/BLOCKSET read-only validation;
new SIZEOPT source-object audits remain queued afterward.

## 2026-10-27 v507: AUXST1 D6FS partition overlap rejection

The 384-word KINIT stack is retained. Code inspection showed that
`auxstore_try` previously checked the AUXST1 BACKSTORE/LOGSTORE/CACHESTORE
ranges against each other, but did not check the descriptor's optional
D6FS partition. KINIT now rejects a D6FS partition extending beyond
the physical device or overlapping any of those three ranges. It also
rejects duplicated/out-of-bounds superblock offsets and inconsistent
empty-partition fields. An absent D6FS partition remains valid.

This check reuses the existing physical descriptor block and involves
no additional I/O or permanent state. Full GCC lowmem tape link passed:
KCORE **8548**, packaged KCORE+MRES **13463**, unchanged. Native 32K
TSFS+AUXST1 swap-out/wake/swap-in/WAIT regression passed. Existing
AUXSTORE choice test and new v507 invalid-descriptor host test passed
in daimos-testkit. This is descriptor validation, **not yet** validation
of D6FS on-disk superblocks or a full BLOCKSET member inventory.

Based on native stack watermarks 129/136/201 words and direct inspection
of KINIT fatal paths, ordinary early rejection paths typically stop
through diagnostic output and `kinit_halt`; they do not do complex
unwinding. A 20–50-word incremental exceptional-path allowance is a
**rough estimate only**, not a measured bound. The 384-word reserve
still leaves 183 words above the largest observed 201-word successful
full-profile boot; no further reduction is indicated.

## 2026-10-27 v506: default KINIT stack 384 words and lowmem DRM

Per explicit policy decision, changed the default KINIT bootstrap stack
reserve from octal `02000` (1024) to `0600` (384) words in the PDP-6
boot Makefile and the KINIT header. Prior instrumented successful
boot high-water results were 129 words (32K TSFS), 136 (32K TSFS+DRM)
and 201 (256K full device configuration). The latter leaves **183
words** under a 384-word reservation. These measurements do not prove
all exceptional paths; the change remains subject to native regression.

Enabled the existing 124-word DRM MRES and `drm236_minit` in lowmem
independently of `KINIT_FULL` (which continues to gate MEMFS and other
full-profile services). The original probe, relocation, PI service and
storage patch ABI are unchanged. The complete GCC lowmem TSFS image
build passed: KINIT end octal `076144`, permissible end with 384-word
stack `077200`. Live 32K SIMH boot reported `DTC OK`, `DSK270 OK`,
`DRM236 OK`, `DTFS LOADED` and `INIT V1`. Full linked KCORE remains
8548 words; packaged tape KCORE+MRES **13463** (increase 124 words,
resident only when drum is present). Dedicated testkit regression:
`tests/validation/daimos-kinit-stack384-drm-v506.sh`.

Outstanding: positive TSFS+DRM AUXST1 swap/wake test, full-profile
boot with the smaller stack reserve, error-path stack measurement and
KINIT-only D6FS reservation/BLOCKSET discovery. Do not extrapolate
successful boots to a proven global worst-case high-water limit.

## 2026-10-27 v505: full-feature 256K KINIT stack watermark

Built a fresh `BOOT_PROFILE=full ROOT=auto` GCC PDP-6 256K image in
an isolated build directory with `KINIT_STACK_WATERMARK=1` and the
unchanged `02000` (1024-word) default stack reservation. Ran headless
SIMH with its full enabled device configuration; a 65-second native
boot reached `INIT V1` after `DRM236 OK`, `BSET LOADED`, `LOGSTR LOADED`,
`MEMFS LOADED` and `D6FS LOADED`. Reading the live KCORE watermark
symbol at `020474` yielded octal `0311` = **201 decimal words**.

For comparison, earlier 32K lowmem TSFS/DSK used 129 words and the
experimental 32K TSFS/DRM variant 136 words. A hypothetical 256-word
reserve would leave only **55 words** at this observed maximum;
this is NOT clearly sufficient safety margin for untested exceptional
paths, failures, or other root/provider configurations. Thus the user
condition ('clearly below 256') is not met, and the standard remains
**1024 words**. A future policy change to 512 requires complete testkit
coverage; full boot success does not prove every failure path safe.

## 2026-10-27 v504: native KINIT watermark, 512-word stack, TSFS and DRM

Fresh separate GCC PDP-6 32K lowmem TSFS builds with
`KINIT_STACK_RESERVE_WORDS=01000` and `KINIT_STACK_WATERMARK=1` passed
the complete image builder and booted in headless SIMH to `INIT V1`.
Read the instrumented KCORE `kinit_stack_highwater` symbol (`020474`)
through SIMH console `ex` using a PTY, not a redirected pipe:

- Stock lowmem TSFS/DSK boot: octal `0201` = **129 decimal words**
  used, leaving **383** of the 512-word reservation.
- Experimental optional DRM MINIT/MRES added to lowmem: SIMH reported
  `DRM236 OK`, reached INIT, and highwater was octal `0210` = **136
  decimal words**, leaving **376** words.
- Instrumented DRM image end octal `076215`; the 512-word stack-safe
  bound is octal `077000`, so the generated image passes. The default
  octal `02000` (1024-word) KINIT reserve has **not** been changed.

Both measurements cover these successful native boot paths, not worst
case failure handling, every root provider, simultaneous disks/drums,
BLOCKSET metadata or severe resource exhaustion. Consequently **do not
yet adopt 512 words as the default** or claim universal safety. The
temporary DRM code was reverted; only the pre-existing unrelated DSH
working-tree change remains. Follow-up: test the 512-word instrumented
reserve across the remaining boot/error matrix and add reproducible
native watermark regression in daimos-testkit before accepting the
stack policy or optional lowmem DRM driver.

## 2026-10-27 v503: exploratory 512-word KINIT stack boot

Built the accepted GCC PDP-6 `BOOT_PROFILE=lowmem ROOT=tape` image with
`KINIT_STACK_RESERVE_WORDS=01000` (512 decimal words), **without** changing
the default `02000` (1024 decimal) reserve. The complete native image
build passed. A headless SIMH PDP-6 32K boot reported `DTC OK`,
`DSK270 OK`, `DTFS LOADED` and reached `INIT V1`; a 16-second controlled
timeout ended the session (124), not a kernel HALT. This is one boot-path
success, **not** a measured high-water result or approval to shrink the
default stack. Retest with KINIT stack watermark and stress the complete
DSK/DRM/BLOCKSET/provider matrix before accepting a smaller reserve.

## 2026-10-27 v502: optional DRM lowmem experiment, boot-space blocker

Tested separating `drm236_minit` and its package from `KINIT_FULL` so
the normal 32K TSFS-root KINIT could install Type-236 DRM if detected
before the conditional SWAP MINIT. This preserved the existing driver
ABI and probe sequence. Full GCC PDP-6 permanent-size link succeeded:
KCORE **8548** unchanged; DRM MRES **124** resident words; tape
KCORE+packaged-MRES **13339 -> 13463** words. Actual installed RAM on a
DRM-equipped simulator was **not tested**.

`make image BOOT_PROFILE=lowmem ROOT=tape` then failed the existing
KINIT bootstrap stack check. Experimental `__kinit_image_end` was
octal `076144`, **100 words beyond** the highest stack-safe end
`076000` for the 32K profile's octal `02000` stack reserve. This is
separate from the final permanent-size limit and must not be solved by
reducing the reserved stack. The experiment was reverted and the
baseline remeasured: KCORE 8548, tape packaged total 13339.

Before enabling lowmem DRM, perform a source-level KINIT/MINIT size
reduction of at least 100 linked transient words **plus safety margin**,
without growing permanent RAM or changing hardware/FS semantics. Then
repeat 32K native image + stack gate, tape-only boot, positive drum AUXST1
swap transaction, disk AUXST1 regression and full-image accounting.

## 2026-10-27 v501: automatic backed-lowmem MEMFS cancelled

Decision: cancel automatic MEMFS activation on detection of backingstore,
regardless of RAM size. The v499 prototype had already been reverted;
v500's external-package loading idea is not an active implementation task.
Current lowmem FSTAB policy remains unchanged and MEMFS stays excluded.
Explicitly loadable modules may be reconsidered only after a dedicated
KINIT size-optimization pass. Do not add special automatic MEMFS loading.
This does not cancel existing SWAP/AUXSTORE work or the deferred KCORE
SIZEOPT object audits.

## 2026-10-27 v500: measured KINIT headroom and optional MEMFS delivery

Read `mkimage.sh`, `kinit.c`/`mres_install`, the native image map and the
accepted package configuration. For the accepted 32K TSFS lowmem build,
`__kinit_image_end = 075637` (31647 decimal); the preserved octal
`02000`-word (1024-word) stack requires the image end to be no higher
than `076000` (31744). Thus just **97 words** of transient image headroom
remain. The existing MEMFS resident payload alone is **1710 words**, not
counting its MRES header, exports and relocation map. It cannot be
included in the initial image under this layout.

`run_minits()` already releases MRES source ranges as MINIT progresses,
but the loader must first place the **whole KINIT image** below the
stack-reserve boundary. Earlier reclamation or conditional installation
cannot address the pre-execution overlap. It would be incorrect to
decrease the configured stack reserve to conceal this constraint.

For lowmem systems with backing, optional MEMFS therefore requires an
**external MRES source** (for example, a TSFS-root-resident module file)
read only after filesystem root selection and validation. The loader
must retain package ABI/relocation checks, reserve contiguous permanent
MRES placement before any unrelated low allocations, and prevent using
the package as a mutable scratch buffer while installing it. This is a
design prerequisite, NOT a completed loader. Test a mounted-root package
failure, no-backing tape absence, a valid-backed tape mount with correct
VFS capacity, and existing swap/wake/WAIT before accepting the change.
No kernel source change is accepted in v500.

## 2026-10-27 v499: conditional lowmem MEMFS image experiment rejected

Investigated conditional MEMFS installation for 32K TSFS-root systems with
validated AUXST1 backing. The existing MEMFS resident package is **1710
words**. A prototype separated its MINIT from `KINIT_FULL`, checked the
SWAP MINIT's published `backstore_blocks`, added the conditional VFS mount
slot and let MOUNTALL attempt `/TEMP` only when FSTAB lacked explicit MEMFS.
The lowmem full link kept KCORE **8548** but packaged tape KCORE+MRES grew
from **13339 to 15049** and disk from **13415 to 15125**. These are
*packaged* upper bounds, not verified native resident values.

**Native test failed before boot:** `mkimage: KINIT image leaves less than
the configured stack reserve`. The current lowmem image includes MEMFS
package source and relocation metadata in transient KINIT even if its
MINIT would later skip installation. Thus a conditional runtime guard
does not solve the boot-image occupancy problem. Reducing the KINIT
stack reserve would mask the failure, not fix it. The prototype was
fully reverted, restoring the accepted tape size **13339**. No kernel
changes were accepted and the unrelated DSH change was preserved.

Next: design an optional MEMFS package delivery or reclaimed source
arrangement that **does not consume the lowmem KINIT stack headroom**;
account for package/relocation source, installed 1710-word MRES, VFS
mount slab increment and MEMFS's minimum 640-word metadata allocation.
Then validate unbacked tape, backed TSFS/AUXST1 mount and swap-wait,
and explicit FSTAB error semantics on native 32K. The policy goal remains
MEMFS when backing exists, independently of installed RAM size.

## 2026-10-27 v496: native TSFS AUXST1 swap path verified

Root cause of v494/v495 test failure: TSFS packaging named its INIT
using the fixed `INIT_DXR` Makefile variable, whereas the native test
and disk-root packaging require the externally selectable
`SYSTEM_INIT_DXR`. TSFS now uses `SYSTEM_INIT_DXR` consistently in its
dependency and manifest. The unchanged swap-wait process workload then
**PASS**ed native 32K TSFS root with disposable DSK0 AUXST1: sleeping
reader swap-out, wake, slot-0 restoration, and WAIT result. No swap
engine workaround or KCORE change was needed.

INIT/MOUNTALL no longer warn merely because a filesystem was not
requested in FSTAB: absent MEMFS means no MEMFS warning, while failed
requested mounts still return failure. Full-profile builds configured
below 64K now select the no-MEMFS FSTAB, including 32K and 63K; 64K
and 256K retain the normal MEMFS FSTAB. The fstab.lowmem comments remain
SIXBIT-safe. Measured complete GCC PDP-6 lowmem links: KCORE **8548**,
packaged tape **13339**, disk **13415** words, all unchanged.

## 2026-10-27 v495: narrowed TSFS swap-wait diagnostic to INIT selection

Repeated the v494 32K TSFS-root/AUXST1 swap-wait fixture with preserved
build media and a manifest assertion. The generated `tsfs-root.manifest`
correctly names the custom swap-wait `init.dxr`, and `dtc1.tap` is an exact
copy of the generated TSFS root member. Nevertheless the native simulator
prints the **stock** `INIT V1` banner, which is absent from the custom
workload, and the test times out without swap markers. Do not attribute
this failure to swap transfer/restoration until the executable actually
launched at boot is identified. The testkit diagnostic now reports
unexpected stock INIT explicitly and can preserve artifacts with
`KEEP_V494_FILES=1`. No kernel or KCORE change is justified yet.

## 2026-10-27 v494: TSFS AUXST1 native swap transaction blocker

Adapted the existing native swap-wait workload to a lowmem TSFS root
with a separately provisioned, disposable AUXST1 DSK0 backing image.
The kernel detects DSK270 and reaches INIT V1, but the existing
45-second SIMH execution window expires without the expected swap,
wake and WAIT markers (simulator timeout status 124). The native
swap transaction therefore remains **unverified**; do not claim
successful swapping or accept kernel changes on this evidence.

The diagnostic lives exclusively in daimos-testkit as an opt-in v494
target. Next inspect TSFS executable lookup/INIT startup versus actual
swap service dispatch, extending the deadline only after establishing
which stage consumes the execution window. No KCORE changes or
permanent-memory increase were made.

## 2026-10-27 v493: positive AUXST1 tape-root boot fixture

Native 32K TSFS-root boot with a disposable DSK0 AUXST1 descriptor
(`BACKSTORE 07001+01000`) reached INIT V1 with DSK270 detected. The
descriptor was provisioned explicitly in a temporary simulator image;
no physical device or root image was overwritten. A reusable native
testkit v493 fixture generates this medium and exercises the boot.
This is **boot reachability**, not yet an end-to-end swap-out/in proof.
The image reports `INIT: MOUNTALL FAILED`, seen in earlier control boots;
do not treat that as evidence of swap correctness or failure without
separate transaction coverage. No KCORE or package layout changes.

## 2026-10-27 v492: AUXSTORE selects a backing-capable device

Reclaimable KINIT AUXST1 discovery now prefers DRM0 when its validated
descriptor contains a nonzero swap reservation, but checks DSK0 when
the drum descriptor has no swap capacity. If the disk offers a swap
reservation, KINIT selects that disk; otherwise it preserves a valid
log-only drum descriptor. Discovery remains read-only, with neither
D6FS runtime nor any KCORE changes. The Type-270 safe-probe limit of
unit zero is unchanged.

Full linked GCC lowmem memory remains KCORE **8548**, tape package
**13339**, disk package **13415**. Native 32K tape-root without AUXST1
reaches INIT V1. The independent host testkit v492 test executes the
actual KINIT discovery function against deterministic synthetic device
descriptors, covering drum-only, disk-only, both with swap, log-only
drum with disk swap, and no backing device. A *positive native* TSFS
AUXST1 swap-in/out transaction is not yet demonstrated; lowmem DRM and
D6FS-reservation/BLOCKSET recognition remain open.

## 2026-10-27 v491: conditional TSFS SWAP installation via AUXST1

KINIT's SWAP MINIT now probes already-installed DSK/DRM device services
using the reclaimable `auxstore_boot_discover` AUXST1 descriptor reader
when D6FS is excluded. Only a validated nonzero BACKSTORE reservation
causes `auxstore_post_minits` to publish the direct backing mapping and
the pinned SWAP/BACKSTORE MRES to install. Unprovisioned TSFS boots leave
both services absent; no media formatting or writes occur during probe.
No D6FS filesystem provider is loaded and KCORE is untouched.

Full GCC lowmem links remain KCORE **8548**, packaged tape total **13339**
and disk total **13415**. Native 32K tape without AUXST1 reaches INIT V1.
The dynamic installed MRES saving for that case is the 412-word SWAP
package, as in v490. This stage provides AUXST1 discovery for an already
supported DSK driver, but does **not** yet include a lowmem DRM driver,
D6FS reservation reader, non-root BLOCKSET recognition or positive native
AUXST1 swap transaction fixture. Those remain required before claiming
full adaptive tape+drum/disk support.

## 2026-10-27 v490: lowmem TSFS without pinned SWAP MRES

For the currently supported lowmem TSFS-root build (`KINIT_FULL=0`,
`D6FS_FS_ENABLED=0`), KINIT no longer installs the pinned SWAP/BACKSTORE
package. Its resident KCORE failure entries remain unchanged; v489's
zero-capacity process boot initializes executable backing and the packed
state map without allocator exports. This is a deliberately narrow
device-free configuration, **not** a solution for future lowmem
TSFS+DSK/DRM AUXSTORE: enable runtime, validated backing discovery before
extending this path to such devices.

The tape image still *packages* 412 SWAP words, so the static complete
link remains 8548 KCORE / 13339 KCORE+MRES and PERMANENT_LAST 13386.
But `mres_install` advances the actual permanent MRES boundary only for
installed packages. Skipping SWAP therefore eliminates **412 resident
words** on this tape-only boot, implying an installed KCORE+MRES endpoint
of 12927 words rather than 13339; this is derived from exact package
size and loader behavior, not independently measured with runtime
MEMINFO yet. Native 32K tape-root boot reached INIT V1; disk-root
static link unchanged at 8548 / 13415. No KCORE change.

## 2026-10-27 v489: device-free process boot no longer calls BACKSTORE MRES

The reclaimable `proc_swap_boot_init` now handles a validated zero-block
backing pool without calling `backstore_bitmap_words` or `backstore_init`.
It allocates the required per-process executable backing records and packed
storage-mode map, publishes an empty authoritative BACKSTORE state, and
retains the existing nonzero-capacity initialization path unchanged.
This is a prerequisite for omitting the swap/BACKSTORE MRES in KINIT on
systems with no validated physical backing source; it does not yet omit
that package. No permanent KCORE changes were made.

Full GCC PDP-6 lowmem links remain tape 8548 KCORE / 13339 combined and
disk 8548 KCORE / 13415 combined. Native 32K tape boot reached INIT V1;
native disk swap-wait passed swap-out, wake, restoration and WAIT.

## 2026-10-27 v488: storage-first SWAP MINIT ordering

Moved the pinned SWAP MRES package immediately after the DSK/DRM driver
packages in both the physical package-source order and MINIT table.
This is a necessary boot-order prerequisite for KINIT to decide whether
to install swap after probing the physical storage devices, rather than
unconditionally installing SWAP before the probes. KCORE is unchanged.

Measured GCC PDP-6 lowmem full links: disk-root KCORE 8548,
KCORE+MRES 13415, tape-root KCORE 8548, KCORE+MRES 13339, all
unchanged versus v484. Both native 32K disk-root and tape-root boots
reached INIT V1; testkit eight-member TSFS selection PASS. This change
**does not yet disable the SWAP MRES** on tape-only boots: the shared
BACKSTORE allocator and `proc_swap_boot_init` still require published
exports. The next stage must separate always-needed metadata helpers
from optional transfer/reclaim services and have KINIT select only after
validated device-space discovery. D6FS runtime is never needed to read
its reservation metadata during TSFS-root KINIT.

## 2026-10-27 v485: process storage-mode MRES experiment reverted

Investigated dynamic per-slot process backing records and the packed
two-bit storage-state map. Resident backing descriptors are necessary for
EXEC/PURE recovery, VFS mount protection and MonitorFS reporting; the
storage-mode map distinguishes RESIDENT, DISK and PACKED lifecycles and is
checked by both native swap and packed-process teardown. At eight process
slots it uses just one word. Eliminating it would require a new encoding
and coordinated lifecycle changes, rather than a small independent RAM cut.

As a bounded KCORE experiment, moved `proc_storage_expect_v30` and
`proc_storage_publish_v30` assembly into the already pinned swap MRES,
leaving two resident patchable entry points. GCC PDP-6 lowmem full link:
KCORE **8548 -> 8508** (-40), SWAP MRES **412 -> 454** (+42),
KCORE+MRES **13415 -> 13417** (+2). The extra permanent two words violate
the no-growth rule. Reverted the implementation before runtime testing and
rebuilt the accepted v484 baseline (**8548 / 412 / 13415**). No record,
map, ABI or storage-state changes were retained. A future retry needs at
least two verified linked-word savings to offset the entry cost.

## 2026-10-27 v484: accepted shared BACKSTORE allocator relocation

The allocator implementation (originally 108 pre-link TEXT words) now lives
in the pinned `swapv481` MRES shared by process swap and MEMFS. The four
authoritative state words remain in KCORE, together with stable one-word
`backstore_bitmap_words`, `backstore_init`, `backstore_alloc`, and
`backstore_free` entry points. SWAP MINIT patches all four entry points
from additional package exports, preserving the public caller ABI. There
is **no second MRES installation** or duplicate bitmap. Bitmap mark/free
instructions were shortened using `IORM` and `ANDM`, respectively.

Root cause of the v482/v483 failed boots: `kcore_end.dobj` was no longer
the last fixed KCORE link object. `__kcore_low_end` was placed at octal
`020620`, exactly on `backstore_bitmap`; KINIT allocated the MM extent
registry starting there and overwrote BACKSTORE state. `proc_swap_boot_init`
then received an invalid bitmap-size result and failed allocation. The
v484 build explicitly moves `kcore_end.dobj` last, placing the four backing
state words at `020620`-`020623` and the exclusive KCORE/MM boundary at
`020624`. The regression in daimos-testkit checks this invariant.

Fully linked GCC PDP-6 lowmem results (decimal 36-bit words):

| Root | KCORE v481 -> v484 | SWAP/shared MRES v481 -> v484 | Combined |
|---|---:|---:|---:|
| disk | 8652 -> **8548** | 308 -> **412** | **13415**, unchanged |
| tape | 8652 -> **8548** | 308 -> **412** | **13339**, unchanged |

The accepted KCORE reduction is **104 words**, not the original 108-word
allocator count: four patchable resident entry words are required.
Standalone PDP-6 BACKSTORE test PASS; 32K disk-root native SWAP WAIT and
SWAP STRESS both PASS, including wake, restoration, stopped jobs, and WAIT.
The normal 32K disk-root control and changed kernel both reach INIT; the
normal image reports `INIT: MOUNTALL FAILED`, independently of this change.
The strict 32K DSH acceptance work remains deferred. Swap boot metadata
remains unchanged and requires further analysis before any modification.

## 2026-10-27 v482: shared BACKSTORE MRES relocation experiment rejected

Moved only the 108-word allocator TEXT into a dedicated boot MRES while
keeping the four public entry points and authoritative four-word backing
state in KCORE. The GCC PDP-6 lowmem disk-root full link was size-neutral:
KCORE 8652 -> 8544, BSTORE MRES +108, combined permanent 13415 -> 13415.
The tape-root link likewise held combined residency at 13339. This is a
valid static separation, **not an accepted kernel change**.

The native 32K disk-root swap-wait regression halted after D6FS setup
(`kinit_halt`, PC 060407 in the experiment image), before INIT ran. The
first four-word entry assembly included a non-patchable POPJ for FREE;
that was corrected to JRST and boot failure persisted. The experiment
was reverted, including the build configuration and MINIT/service
publication edits. Rebuilt baseline: KCORE **8652**, combined **13415**.
The native failure's exact origin remains unproven; a targeted bootstrap
allocation/service-publication trace is necessary before reconsideration.
Do not claim a 108-word accepted saving or assume the MRES design is
invalid merely because this initial integration failed.

The swap-specific boot state remains unchanged: `proc_swap_boot_init`
allocates `slots + ceil(backstore_blocks/36) + ceil(slots/18)` dynamic
words in a single extent. Even if backing capacity is zero, the one-word
per-slot executable backing record is required for EXEC, VFS unmount
protection, MonitorFS process reporting, and process lifecycle. The
two-bit-per-slot storage map is consumed by swap transitions and packed
process-state machinery. The zero-capacity allocation offers at most a
small conditional optimization; do not eliminate the map or change
transaction representation without native coverage for both absent and
present backing storage. No boot-state source changes were made.

## 2026-10-27 v481: process swap transfer/reclaim moved into pinned MRES

Accepted focused KCORE relocation, **without increasing combined permanent
residency**. `proc_swap_pdp6.s` is now a separate `swapv481` MRES package;
`proc_swap.s` retains PURE backing helpers, executable metadata, and the
two-bit per-process storage-map operations in KCORE. Resident entry words
`proc_swap_reclaim` and `proc_swap_service_one` are patched by SWAP MINIT
to call the package's `proc_swap_mres_reclaim_v481` and
`proc_swap_mres_service_v481` exports. Prior to publication both entries
return -1. The immutable-transaction invariant HALT remains in KCORE.
The package is placed directly after MonitorFS in *both* MRES source and
MINIT order; putting it last caused a native 32K MRES installation failure,
because reclaimable package-source lifetime is boot-order sensitive.

Fully linked GCC PDP-6 lowmem measurements, decimal 36-bit words:

| Root | KCORE old -> new | MRES change | KCORE+MRES old -> new |
|---|---:|---:|---:|
| disk | 8960 -> **8652** | +308 | 13415 -> **13415** |
| tape | 8960 -> **8652** | +308 | 13339 -> **13339** |

Disk-root native SIMH PASS: `test-daimos-swap-wait` (sleep/restore/WAIT),
`test-daimos-swap-stress` (full-pipe and stopped-job swap/resume). PDP-6
standalone PURE reconstruction and verification PASS. The 32K DSH strict
acceptance task is separate and not addressed by these tests.

No change to the swap record layout, process state transitions, backing
allocator, MEMFS, or storage ABI. The private `proc_swap_out/in` bodies are
now in the swap MRES; the resident KCORE symbols retained for MM and idle
are the reclaim/service entries. Keep package source order and embedded
export order checked in future updates.

Follow-on **analysis only**: `backstore.s` (108 text + 4 BSS prelink) is
shared by process swap *and MEMFS*. Moving it into swap MRES would make
MEMFS depend on swap and is not justified; investigate a common resident
service while preserving the single authoritative bitmap. Boot still
allocates one swap record per process, the bitmap of `ceil(blocks/36)`
words, and a packed storage-state map even with zero backing capacity.
Resident executable-backing records and storage-mode validation remain
necessary independently of physical swap availability. Runtime block
capacity and peak dynamic allocations need separate measurement before
any boot metadata reduction. No backstore/boot-state source was changed.

## 2026-10-27 Proposal B v480: ten final searches; optimization closed

Ten additional small-saving source searches covering FILE OPEN,
CLOSE, SEEK, descriptor allocation, removal, SYMLINK, VFS attribute
mutations, WRITE/TRUNCATE, MOUNT and UNMOUNT found no credible safe
>2-word candidate. No production code change. Native SYMLINK v448,
VFS RENAME v455 and FILE RENAME v457 PASS. Full-system SYMLINK and
forced in-RENAME PI6 rescheduling remain unverified. Close Proposal B
optimization at KCORE 8960, KCORE+MRES 15507, PERMANENT_LAST 15554.
Strict 32K P0 is still open. See
`design/proposal-b-final-ten-searches-20261027-v480.md`.

## 2026-10-27 Proposal B v479: RENAME work closed

Accepted VFS RENAME improvements include namespace identity checking,
mandatory AC7 provider restoration and the v472 three-word save frame.
Native v455/v461/v466 and namespace v450 tests PASS on current source;
v471 full-system multiprocess physical RENAME passed previously.
Retain existing FILE/VFS/provider boundaries until a cross-layer
replacement demonstrates real RAM savings and safe blocking ownership.
Current linked lowmem KCORE 8960, KCORE+MRES 15507, permanent last
15554. Strict 32K P0 remains the active priority; see v479 closure.

## 2026-10-27 v472 DSH 32K physical budget (no code change)

The 036000-octal DSH DXR **image** cap is distinct from the full 32K
**physical** KCORE+MRES+process-memory budget. v440 KCC image 035757
has 17 decimal words of image-gate headroom. A historical stripped
kernel plus aligned DSH VM estimate missed 32K by **210 words before
other allocations**; it is not today's measured target. The old
20,480-word DSH allocation combined with current PERMANENT_LAST 15556
would miss by 3268 words, but mixes build profiles and dates. Obtain
matched current DSH DXR/BSS and live 32K allocation peaks before
setting an accepted size-cut objective. Strict acceptance stays
deferred; `design/dsh-32k-size-cut-budget-20261027-v472.md`.

## 2026-10-27 P0 v474: strict 32K acceptance resumed

Fresh GCC lowmem full link (default): **KCORE 8960** (`021400`),
KCORE+MRES 15507, PERMANENT_LAST 15554. Strict endpoint `<020000`
requires **769** saved KCORE words. Explicit single disk/tape links:
**9008** (`021460`), requiring **817** words saved. Their KINIT
footprints leave 764/209 words after reserved stack. Auto-root 32K
boot failed pre-simulator for stack room; single-disk boot reached
LOGIN, then rejected ROOT. P0 **NOT ACCEPTED**; P2 paused. See
`design/p0-strict-32k-gates-20261027-v474.md`.

## 2026-10-27 Proposal B v472: VFS RENAME save-frame reduction

Preserve AC4 directly across readonly checks (the helper uses AC1–3).
RENAME save frame shrinks from four to three words; full linked KCORE
**8960**, down 2 words from v471. Native provider integration and
source-level AC4 contract tests PASS. See v472 design note.

## 2026-10-27 Proposal B v466 RENAME provider dispatch correction

Restore AC7 provider field after RENAME namespace XOR; previously
valid RENAME operations failed before provider dispatch. Native
mapped FILE/VFS and real gateway lock/wait fixture PASS. Correct
linked baseline **KCORE 8962** (+1 necessary repair); no data growth.
See v466 design note.

## 2026-10-26 Proposal B v460 RENAME cross-layer ownership gate

Audited syscall two-pointer VM lease, FILE dual parent/name frame,
VFS readonly/namespace policy and serialized provider transaction.
A replacement must delete an existing syscall/FILE boundary together,
not add a trampoline; integrated mapping and blocking-I/O tests are
required before cutover. No production change or new RAM saving.
Last fully linked KCORE **8961**, KCORE+MRES **15508**,
PERMANENT_LAST **15555**. See v460 design note.

## 2026-10-26 Proposal B v459 size-neutral RENAME XOR experiment

Reversing RENAME namespace XOR operands left full linked lowmem KCORE
at **8961**. The experiment was reverted. No production change or
RAM saving is accepted. See v459 design note.

## 2026-10-26 Proposal B v458 rejected FILE RENAME cleanup split

Duplicating the FILE RENAME cleanup tail increased full linked lowmem
KCORE **8961 -> 8963**. Reverted the experiment; the accepted baseline
remains **8961** with no production changes. See v458 design note.

## 2026-10-26 Proposal B v457 native FILE RENAME lifecycle fixture

New daimos-testkit PDP-6 fixture executes production FILE RENAME
assembly with controlled parent, access and provider callbacks.
Success, both lookup failures, both permission failures and provider
error PASS. Native VFS and DTFS RENAME regressions also PASS.
No production edit: linked KCORE **8961**, KCORE+MRES **15508**,
PERMANENT_LAST **15555** remain the accepted v452 values.
See v457 design note.

## 2026-10-26 Proposal B v456 RENAME readonly and provider failures

Native PDP-6 VFS RENAME test covers readonly source/destination,
writable dispatch and negative provider-result propagation. VFS and
DTFS RENAME regressions PASS. No production changes or RAM savings;
fully linked v452 baseline KCORE **8961**, KCORE+MRES **15508**,
PERMANENT_LAST **15555**. See v456 design note.

## 2026-10-26 Proposal B v455 native VFS RENAME dispatch

New native PDP-6 testkit executable exercises production VFS RENAME
dispatch, same-namespace success and cross-provider/cross-mount
rejection. PASS. No production code or permanent RAM changed; linked
baseline remains KCORE **8961**, KCORE+MRES **15508**,
PERMANENT_LAST **15555**. See v455 design note.

## 2026-10-26 Proposal B v454 RENAME ownership guards

Expanded testkit namespace comparisons and added an explicit FILE
RENAME two-parent/permission/frame guard. Static checks, native PDP-6
FILE and pathname-walk, and DTFS RENAME provider tests PASS. No
production edit. Last fully linked KCORE **8961**, KCORE+MRES
**15508**, PERMANENT_LAST **15555**. See v454 design note.

## 2026-10-26 Proposal B v453 native DTFS RENAME regression

Restored the standalone native DTFS RENAME fixture by providing the
production-equivalent `kconst_1_1` stack constant in daimos-testkit.
Native ITS/TENEX, destination-exists and cross-mount cases PASS.
No DAIMOS production change; last fully linked KCORE **8961**,
KCORE+MRES **15508**, PERMANENT_LAST **15555** (v452).
See v453 design note.

## 2026-10-26 Proposal B v452 masked-XOR RENAME identity

Use one XOR and masked nonzero test for provider/mount mismatch.
Full linked GCC PDP-6 lowmem KCORE **8961** (-2), KCORE+MRES
**15508**, PERMANENT_LAST **15555**. Testkit namespace, native
PDP-6 FILE, pathname-walk and provider-lock tests PASS. No new
persistent allocation or ABI change. See v452 design note.

## 2026-10-26 Proposal B v451 RENAME namespace regression closure

Strengthened testkit coverage to detect collisions between distinct
provider/mount identities across all 4096 combinations. Focused
PDP-6 FILE, pathname-walk, provider-lock and identity checks PASS.
No production change or added permanent RAM. Last fully linked
baseline remains v450 KCORE **8963**, KCORE+MRES **15510**,
PERMANENT_LAST **15557**. See v451 design note.

## 2026-10-26 Proposal B v450 RENAME namespace identity

One upper-12-bit provider+mount comparison replaces independent
namespace field decodes. Linked KCORE **8963** (−3); all 4096
provider/mount combinations and focused provider/FILE/PIPE tests PASS.
See v450 design note.

## 2026-10-26 Proposal B v449 TRUNCATE/WRITE safety guard

TRUNCATE and WRITE_WORDS reuse one resident readonly/swap-protection
and provider-dispatch continuation. Linked KCORE **8966** (−6), with
two extra transient stack words for TRUNCATE and no permanent data.
Focused native provider/FILE/PIPE and frame tests PASS. See v449 note.

## 2026-10-26 Proposal B v447 CREATE/SYMLINK E argument

Removed the redundant branch around the SYMLINK-only provider E=0
assignment. Full linked KCORE **8972** (-1); DTFS CREATE, FILE/PIPE,
UMASK, provider lock and updated SYMLINK ABI guard PASS. Native
end-to-end SYMLINK runtime test remains open. See v447 design note.

## 2026-10-26 Proposal B v446 SYMLINK/CREATE policy consolidation

Replaced the separate SYMLINK read-only check and vnode publication
sequence with the existing CREATE policy via exact argument mapping.
Full linked **KCORE 8973** (8 saved), no permanent RAM growth. Native
provider, FILE/PIPE, DTFS CREATE and SYMLINK ABI guard PASS. Dedicated
SYMLINK end-to-end test pending. See v446 design note.

## 2026-10-26 Proposal B v445 CREATE/SYMLINK shared dispatch

Removed duplicated provider call from VFS CREATE/SYMLINK; both now
share call and vnode publication. Fully linked **KCORE 8981** (-1).
FILE/PIPE, provider lock and shared-dispatch guards PASS. See v445 note.

## 2026-10-26 Proposal B v443 consolidated VFS mutations

UNLINK/CHMOD and CHOWN/UTIME now share the readonly-check and physical
provider dispatch continuation. Full linked lowmem **KCORE 8982**
(five words saved); no permanent RAM allocation. Focused provider,
FILE/PIPE and frame-contract tests PASS. See v443 design note.

## 2026-10-26 Proposal B v442 rejected direct-op experiment

FILE/VFS MKDIR/MKFIFO opcode handoff with shared VFS entry compiled,
but full linked KCORE increased **8987 -> 8990** (+3). Both source
files reverted. Baseline KCORE **8987** unchanged. Future consolidation
must delete a full layer rather than insert another dispatch adapter.
See v442 design note.

## 2026-10-26 Proposal B v441 OPEN directory projection

Directory descriptor argument now derives directly from the VFS stat
type, saving one linked KCORE word: **8987**. FILE/PIPE direct-mount,
UMASK, OPEN lifetime and type-equivalence guards PASS. KCC DSH remains
within the original size gate. See v441 design note.

## 2026-10-26 v439 FIFO regression unblocked

GCC full-profile test image avoids the unintended KCC DSH overflow;
PDP-6 FIFO integration PASS verifies v438. No KCORE or DSH source
change, no KCC DSH size reduction. KCORE **8988**. See v439 note.

## 2026-10-26 Proposal B v438 FIFO OPEN completion

FILE OPEN now shares the descriptor result publication between FIFO
and regular-file success paths. Fully linked KCORE **8988**, one word
below v437; no RAM expansion. OPEN lifetime source guard PASS; see
the v438 design note for verification scope.

## 2026-10-26 Proposal B v437 OPEN ownership boundary

Source audit and testkit guard PASS: OPEN's 14-word scratch, retained
node, post-create stat, FIFO rollback and caller VM mapping lease
constrain FILE/syscall consolidation. No production edits, no saving;
accepted linked **KCORE 8989**. See v437 design note.

## 2026-10-26 Proposal B v436 OPEN normalization

Accepted one KCORE word saved by normalizing OPEN flags directly in
the argument register. Full linked KCORE **8989**, no new RAM
allocation. Exhaustive 18-bit flag equivalence and EXEC ABI tests PASS.
The unified FILE/VFS/syscall architecture remains unfinished. See
`design/open-flag-register-abi-20261026-v436.md`.

## 2026-10-26 Proposal B v435 unified pathname ABI design

Identified OPEN's syscall -> FILE -> VFS/provider ownership chain,
including the separate held user mapping and sleeping I/O workspace.
Drafted a single pathname-request contract to replace (not add to)
existing syscall/FILE branches, first targeting OPEN success and
failure publication. No production edit or measured saving; KCORE
**8990**. See v435 design note.

## 2026-10-26 Proposal B v434: larger resident layer census

The linked PDP-6 map shows contiguous spans of 823 words around VFS,
1050 around FILE and 787 from `exec_native_syscall` to the later
syscall/device bridge, versus only 30 for provider serialization.
These spans are not exact exclusive subsystem sizes or guaranteed
savings. Prioritize a cross-layer FILE/VFS/syscall ABI redesign over
further provider bridge tuning. No production change; KCORE **8990**.
See `design/kcore-resident-layer-census-20261026-v434.md`.

## 2026-10-26 Proposal B v433 provider bridge size ceiling

Source census shows the *entire* resident provider call/serialize/wait/
unlock bridge contains 30 instruction words, plus four binding slots
and one lock word. It cannot by itself supply the hundreds of KCORE
words sought, and replacing it safely requires preserving sleeping
workspace leases. No production edit; KCORE **8990**. Redirect major
architectural review to larger resident FILE/VFS and syscall regions.
See `design/provider-bridge-size-ceiling-20261026-v433.md`.

## 2026-10-26 Proposal B v432 direct MEMFS dispatch

Removed the redundant missing-service compare from direct MEMFS
dispatch. The existing stub returns -1. Full linked PDP-6 GCC lowmem
KCORE **8990** (two words below v431); native direct/unbound MEMFS,
provider locking, and lease interleaving tests PASS. See v432 note.

## 2026-10-26 Proposal B v431 provider-bridge KCORE shrink

The serialized provider bridge now calls the missing-service stub
through its ordinary ABI and uses the common unlock. Linked PDP-6
GCC lowmem **KCORE 8992** (three words saved), KCORE+MRES 15539,
PERMANENT_LAST 15586. Native provider lock, missing service and
workspace interleaving PASS. Rejected fast-path experiment increased
KCORE 8995 -> 8999; corrected fall-through failure before acceptance.
See v431 design note. Strict 32K remains deferred.

## 2026-10-26 Proposal B v430 TSFS READDIR projection

Removed redundant record-pointer AC transfer in TSFS READDIR while
retaining caller-owned name/type projection. Full linked PDP-6 GCC
lowmem KCORE **8995**, KCORE+MRES **15542**, PERMANENT_LAST **15589**.
Native TSFS multiextent, D6LZ and projection guards PASS. The global
provider lock remains necessary. See v430 design note.

## 2026-10-26 Proposal B v429 TSFS value-record cost gate

Testkit guard PASS: returning owned copies of TSFS records would cost
at least 8 file-record or 4 extent-record stack words per call, while
sleeping transfers still require shared-buffer serialization. No
unproven value-copy code added to production. Accepted KCORE **8995**;
see v429 design note. Strict 32K acceptance remains deferred.

## 2026-10-26 Proposal B v428 native workspace interleaving

The actual provider bridge passes a new PDP-6 deterministic wait and
shared-buffer lease regression in daimos-testkit. TSFS pointer-return
paths need explicit owned record values before workspace ownership can
be shortened. No kernel change; KCORE remains **8995**. See v428 note.

## 2026-10-26 Proposal B v427: provider I/O lease boundaries

Source tracing proves that shared `fs_block_workspace` remains live
across sleeping DSK/DRM/DTC operations and cache publication. A new
testkit guard checks these specific I/O boundaries. Do not shorten
serialization without replacing the live-buffer ownership contract.
Production unchanged; KCORE **8995**. See v427 design note.

## 2026-10-26 Proposal B v426 provider transaction baseline

Existing PDP-6 provider lock native/static gates PASS. New testkit
inventory counts 60 shared-workspace references across seven files;
blocking I/O is reached via driver wrappers, so transaction lifetimes
are not yet proven separable. Production unchanged; KCORE **8995**.
See `design/provider-transaction-call-boundaries-20261026-v426.md`.

## 2026-10-26 Proposal B v425: new provider transaction candidate

Source audit identifies an untried coordinated redesign of shared
filesystem transfer-workspace ownership and provider serialization.
`fs_provider_reg_call` saves seven ACs during blocking DTFS/D6FS/TSFS
operations; the single 128-word workspace also backs BCACHE slot 0,
so simply removing or splitting the existing lock is unsafe. Proposed
transaction-lifetime experiment and native concurrency gates are in
`design/provider-transaction-ownership-redesign-20261026-v425.md`.
No production edit or new RAM reduction claimed; baseline KCORE 8995.

## 2026-10-26 Proposal B v424 branch-free FLOCK mode replacement

Moving UNLOCK's regular state into the FLOCK mode table removes three
instructions from the full linked PDP-6 GCC lowmem image. KCORE
**8995**, KCORE+MRES **15543**, PERMANENT_LAST **15590**. Native FILE
and directmount/PIPE/UMASK regressions PASS. Total saving against
pre-v417 KCORE 9109 is **114 permanent words**, with no process
u-area growth. Strict 32K acceptance remains deferred.

## 2026-10-26 Proposal B v423 mount-unlock instruction reduction

Retaining the packed descriptor in AC4 while extracting its full
six-bit mount ID into AC5 removes a redundant table reload from
`file_unlock_mount`. Full PDP-6 GCC lowmem linked measurements:
KCORE **8998**, KCORE+MRES **15546**, PERMANENT_LAST **15593**,
one word smaller than v417/v422 and **111 words below v388**.
Native FILE and directmount/PIPE/UMASK tests PASS. No u-area growth.

## 2026-10-26 Proposal B v422 complete root-login validation

Full PDP-6 disk/drum/tape login matrix PASS after raising the tape
interactive response timeout to 15 minutes in daimos-testkit;
disk/drum retain five minutes. No production source change or new size
measurement. Accepted v417 KCORE remains **8999 words** (110 saved).
Strict 32K acceptance remains deferred.

## 2026-10-26 Proposal B v420 provider lifecycle regression

Native PDP-6 D6FS multimount, remount and direct DRM backing tests
PASS against v417 full six-bit FILE mount descriptors. KCORE remains
**8999 words** (last measured full linked image); no new saving is
claimed. Root filesystem boot/login and broad provider testing remain
independent acceptance gates; strict 32K is deferred.

## 2026-10-26 Proposal B v419 native RUN lifecycle regression

Both full PDP-6 simulator fixtures PASS: process lifecycle
(RUN/GETPID/WAIT/nohang/zombies/orphans/explicit fdmap) and late-RUN
rollback (16 mapping failures under three active children with stable
accounting/recovery). `MAKEFLAGS=COMPILER=gcc` uses the documented
smaller DSH without relaxing its EXEC image-size limit. No source
change; previous accepted v417 linked KCORE **8999** remains current.

## 2026-10-26 Proposal B v417 direct-mount descriptor cutover

Accepted full linked GCC PDP-6 lowmem change: KCORE **9109 -> 8999**,
KCORE+MRES **15657 -> 15547**, PERMANENT_LAST **15704 -> 15594**
(110 permanent words saved). FILE now stores full six-bit mount IDs
directly in the first descriptor word, retains independent 36-bit
offsets, and uses process-local vnode-wide FLOCK. The mount-extension
bitmap and per-OPEN lock-family token are removed; VFS CREATE reads
the credential UMASK9. Physical process u-area allocation is unchanged.
Focused PDP-6 FILE, directmount, PIPE identity and UMASK tests PASS;
full boot/multiuser acceptance remains pending. Strict 32K acceptance
and unrelated folding experiments remain deferred. See v417 design note.

## 2026-10-26 Proposal B v400 decisions locked; next-chat handoff v401

Recorded the four approved v400 decisions in
`design/file-descriptor-mount-redesign-20261026-v400.md`:
(1) DUP/DUP2 and RUN descriptor remapping copy offsets independently;
(2) use minimal, demonstrably sufficient descriptor flags;
(3) boot/hardware/provider-dependent capacity must support at least 26
mounts when available, without preallocating 26 slots every boot, and
remain expandable; (4) retain existing locking and provider serialization
initially. Canonical full-vnode plus full 36-bit offset in two words is
the proposed B1 target. No implicit shared offsets, semantic relaxation,
or production change is authorized. Next chat: audit actual metadata
users, propose an exact no-growth encoding, quantify complete RAM/speed,
and prepare the full regression matrix **before seeking approval to edit
code**. Latest measured linked baseline: KCORE 9109; KCORE+MRES 15657;
PERMANENT_LAST 15704. See the design document for detailed handoff.

## 2026-10-26 Proposal B descriptor/mount design review v400

User-selected architecture: at least 26 mount slots with future expansion,
canonical-vnode two-word per-process descriptor (alternative B), retain
useful DUP and shared-offset capabilities. Discussion-first constraint:
no production edit without design agreement and an explicit functionality
and regression review. See `design/file-descriptor-mount-redesign-20261026-v400.md`
for the current code inventory, three candidate formats, offset-sharing
clarification, migration gates and open decisions. This is a design draft;
no production code changed, no new size or test acceptance claimed.
Latest measured baseline remains v388 KCORE 9109, KCORE+MRES 15657,
PERMANENT_LAST 15704.

## 2026-10-26 Cross-proposal architectural redesign policy v399

**Applies to the entire A–F KCORE architectural shrink roadmap, not just
Proposal B.** Existing interfaces, internal and public ABIs, descriptor
semantics, locking protocols, error/rollback rules and historical
compatibility requirements are **not immutable constraints**. The team
may rewrite producers, consumers, callers, filesystem providers,
userland and tests together when a simpler design has a demonstrable
memory/performance benefit. Do not reject a proposal solely because
its behavior or interface differs from the current implementation.

The invariants that **do** remain mandatory are:

- Correctness and explicit resource ownership: no corrupted state,
  leaks, deadlocks, use-after-free or half-published objects. Rollback
  may be redesigned or replaced by a simpler transaction protocol, but
  partial failure must leave a well-defined recoverable state.
- Keep the necessary capabilities available to the OS; alternatives
  need not preserve historical invocation or descriptor semantics.
  EXEC and RUN remain resident essential syscalls, notwithstanding
  permitted changes to their calling interface or internals.
- Optimize **actual physical RAM consumption first**, including KCORE,
  MRES, per-process memory and MM-managed allocations, rather than
  moving memory cost between accounting categories. Execution speed is
  nearly as important; disk size is secondary.
- No workaround-driven redesign or feature creep: fix underlying bugs,
  preserve a readable, documented implementation and measure the
  post-fold fully linked image as well as runtime memory costs.
- Before accepting changed semantics, define the *new* contract,
  migrate all affected call sites and userland, and add appropriate
  tests in `daimos-testkit`. Test both normal operation and concurrency,
  failure, resource cleanup, boot and process lifecycle as applicable.

**A:** process/wait/signal/session and teardown semantics may be
simplified together, but essential scheduling/ownership correctness
remains. **B:** FILE/VFS/MonitorFS APIs, descriptor representation,
DUP/DUP2 and offset semantics, mount policy, locking and rollback
may all change in a coordinated rewrite. Compare packed descriptors,
independent local descriptors and minimal shared handles, including
per-process storage and decode costs. **C:** EXEC/RUN validation,
startup and publication APIs may change, but essential resident
EXEC/RUN and sound replacement/creation semantics stay. **D:** MM
registry and allocation rules may be redesigned based on actual
high-water and fragmentation evidence. **E:** inactive-process
representation and swap interfaces are redesignable if correct state
restoration and total RAM benefit are demonstrated. **F:** scratch
workspace ownership/lock conventions may be rewritten after a proven
lifetime and reentrancy analysis.

This policy supersedes *compatibility-only* restrictions in older
proposal reviews, **not** their factual measurements or correctness
warnings. No production code or ABI was changed by this documentation
update. The latest accepted post-link baseline remains v388: KCORE
**9109**, KCORE+MRES **15657**, PERMANENT_LAST **15704**.

## 2026-10-26 Proposal B bounded structural review v389–v398 (ten passes)

User requested ten further review iterations after v388, except that an
unexpected large-saving opportunity should extend the effort. Ten
independent candidate boundaries were inspected in the current
`system/kernel/fs/vfs.s` and `file_runtime.s`, using the existing
A–F roadmap. **These are source-level review passes, not ten new
compiled patches or runtime acceptance results.** No proposed change
met a credible >=10–20 net resident-word saving threshold with a
proved unchanged ABI and lifetime, so no production edit was made.

1. **v389 LOOKUP namespace table:** both three-record namespaces now
   share loop-count initialization (v387); a further common loop would
   add branching without a demonstrated saving.
2. **v390 READDIR namespace merge:** built-in `/MONITOR` and mounted
   root offsets depend on different input traversal states and
   distinct stack frames. No safe common-entry reduction established.
3. **v391 PARENT_NAME:** MonitorFS device/PROCESSES/DOMAINS paths
   publish different counted names, while provider parent lookup has
   its own mount inheritance and ABI. Existing provider dispatch is
   shared where feasible (v386).
4. **v392 MOUNT slot search:** free-slot selection rejects existing
   mounts and uses MM-backed target/root tables; changing scan order
   risks duplicate target acceptance.
5. **v393 MOUNT publication:** target/root table entries and RO/SWAP/
   LOGSTORE owner policy have distinct publication requirements.
   No redundant full-sized resident table is present.
6. **v394 UNMOUNT retirement:** busy checks, sync, prepare-unmount,
   namespace-root substitution and descriptor unlocking precede
   shared slot retirement. Folding into publication would conflate
   failure and lifetime states.
7. **v395 CREATE/MKDIR/MKFIFO:** common `vfs_create_common` and
   `vfs_create_store_result` already share mode handling, provider
   result publication and error unwind; remaining differences are
   provider/operation policy.
8. **v396 SYMLINK/RENAME:** their readonly checks resemble CREATE,
   but arguments, namespace transitions, provider operations and
   stacked node-result publication differ. No verified net gain from
   a generic new frame helper.
9. **v397 bulk READ/WRITE:** common permission validation and
   provider arguments already exist (v376/v377); device dispatch,
   atomic APPEND and offset publication remain operation-specific.
10. **v398 FILE pathname wrappers:** common parent write-access check,
    one-argument cleanup and descriptor constructors are already in
    place. OPEN/FIFO error cleanup, RENAME two-parent validation and
    UNLINK/RMDIR type/busy checks have different ownership semantics.

**Conclusion:** none of these ten source reviews revealed a large
unimplemented consolidation supported by current evidence. The latest
measured linked baseline (v388) remains KCORE **9109**, KCORE+MRES
**15657**, PERMANENT_LAST **15704**; reaching KCORE <8192 needs
**918** further words. No new size build or test run is claimed,
because there is no production source change. Proposal C EXEC/RUN
is the next distinct roadmap phase. Proposal B can be closed after
any desired final dedicated regression/acceptance pass; avoid
continuing single-word instruction churn as a substitute for an
architectural change. Strict 32K acceptance remains deferred.

## 2026-10-26 Proposal B direct READDIR namespace vnode comparison v388

Removed a redundant AC4 load before `CAME` in the resident
`vfs_readdir` `/MONITOR` special-case test: compare AC1 directly
against `monitorfs_names+7`, which already stores the canonical vnode.
The provider fallback, directory index bounds, special namespace
entries and ABI are unchanged; no persistent state or MRES movement.

Full linked GCC PDP-6 lowmem versus v387: KCORE **9110 -> 9109**
(-1), KCORE+MRES **15658 -> 15657** (-1), PERMANENT_LAST
**15705 -> 15704** (-1). All four native daimos-testkit checks
passed: `target-file-pdp6`, `target-file-walk-pdp6`,
`test-daimos-vfs-stat-frame`, `test-daimos-pre-dsh-fs-closure`.
Global below-8192 goal still needs 918 words. Remaining Proposal B
work should prioritize larger FILE/VFS consolidation over repetitive
single-instruction changes; the original Proposal C addresses
EXEC/RUN separately.

## 2026-10-26 Proposal B VFS LOOKUP common namespace loop count v387

Both built-in MonitorFS namespace branches (`/` and `/MONITOR`)
search exactly three static four-word records. Moved their duplicated
`MOVEI 0,3` to `vfs_lookup_builtin_start`, where both paths converge;
all comparison fields, mount-root behavior and provider fallthrough
remain unchanged. No state, MRES or ABI changes.

Full linked GCC PDP-6 lowmem versus v386: KCORE **9111 -> 9110**
(-1), KCORE+MRES **15659 -> 15658** (-1), PERMANENT_LAST
**15706 -> 15705** (-1). All four daimos-testkit checks pass:
`target-file-pdp6`, `target-file-walk-pdp6`,
`test-daimos-vfs-stat-frame`, `test-daimos-pre-dsh-fs-closure`.
The global below-8192 goal still requires 919 words. The
remaining B work needs larger verified consolidation, not repeated
one-word tuning; Proposal C remains the separately defined EXEC/RUN
phase of the original A–F architectural roadmap.

## 2026-10-26 Proposal B VFS PARENT_NAME provider selection v386

Revisited the earlier invalid-assembler trial without retaining it.
The valid PDP-6 `CAIE 7,2` / `CAIN 7,3` skip pair replaces two
independent equal comparisons and jumps in the resident
`vfs_parent_name_dispatch`. Exactly providers 2 and 3 enter
`monitorfs_parent_name`; other providers continue to the existing
FS_MRES_OP_PARENT_NAME path. Both mount-root resolution and the
provider ABI stay unchanged. The rejected experimental `CAINE`
mnemonic is not used anywhere in this accepted edit.

Full linked GCC PDP-6 lowmem versus v385: KCORE **9112 -> 9111**
(-1), KCORE+MRES **15660 -> 15659** (-1), PERMANENT_LAST
**15707 -> 15706** (-1). Native daimos-testkit targets
`target-file-pdp6`, `target-file-walk-pdp6`,
`test-daimos-vfs-stat-frame` and `test-daimos-pre-dsh-fs-closure`
all PASS. The global below-8192 KCORE goal still needs 920 words;
Proposal B remains distinct from original Proposal C EXEC/RUN.

## 2026-10-26 Proposal B VFS READDIR builtin bounds branch v385

Reconfirmed the original 2026-10-24 A–F architectural roadmap:
Proposal B covers resident FILE/VFS/MonitorFS consolidation, while
Proposal C is distinct EXEC/RUN consolidation, preserving their
resident, distinct publication/replacement transactions. Therefore
v385 changes only resident VFS namespace READDIR, not EXEC/RUN.

In `vfs_readdir`, the post-provider namespace insertion index is
rejected if negative or at least three. Replaced `CAIGE 4,3; JRST
vfs_readdir_builtin; JRST vfs_readdir_zero` with `CAIL 4,3; JRST
vfs_readdir_zero`, falling through for valid indices 0..2. The
negative index rejection is unchanged; returned dirent records and
mount-root traversal are unchanged. No MRES, ABI or permanent state
changes.

Full linked GCC PDP-6 lowmem versus v384: KCORE **9113 -> 9112**
(-1), KCORE+MRES **15661 -> 15660** (-1), PERMANENT_LAST
**15708 -> 15707** (-1). Native daimos-testkit targets
`target-file-pdp6`, `target-file-walk-pdp6`,
`test-daimos-vfs-stat-frame`, `test-daimos-pre-dsh-fs-closure`
all PASS. KCORE target <8192 still requires 921 words; completion
of Proposal B is assessed by worthwhile structural savings rather
than that independent global target. Proposal C remains separate.

## 2026-10-26 Proposal B resident storage overlay candidate scan v384

Reviewed large explicit resident `.block` declarations, filesystem
workspace call sites, BCACHE slot ownership, MEMFS snapshot entry
points and MonitorFS swapped-process read dispatch. The only obvious
fixed 0200-octal transfer block is `fs_block_workspace` in `vfs.s`;
`bcache_pdp6.s` already overlays slot 0 there while slot 1 is a
**separately allocated 0201-word MM slab** that is needed to preserve
an old clean workspace block across cache misses. The second cache
slot is not an unused permanent block and cannot serve as the VFS
transfer destination without invalidating the cache's recovery model.

`memfs_runtime.s` calls `memfs_snapshot_mount` and
`memfs_snapshot_shutdown`; `monitorfs_runtime.s` dispatches
`mfsproc_swapped_read_words_v63`, which invokes `backstore_read`
with `fs_block_workspace` and invalidates the BCACHE workspace tag
first. These entry paths cannot be assumed to share the physical
provider's DTFS/TSFS/D6FS serialization merely because they operate
on the same scratch address. The audit did **not** establish a
mutually exclusive 128-word replacement allocation or safe buffer
lifetime, so **no storage overlay or memory saving is accepted**.
No code change was made; the latest measured complete linked image
remains KCORE **9113**, KCORE+MRES **15661**, PERMANENT_LAST
**15708**. Next prioritize structural code savings with demonstrable
linked impact, or first prove an exact lock/call graph for the entire
workspace lifetime. Below-8192 KCORE still requires 922 words;
strict 32K acceptance remains deferred.

## 2026-10-26 Proposal B BCACHE/workspace overlay audit v383

Continued the fixed 128-word `fs_block_workspace` lifetime analysis.
`system/kernel/storage/bcache_pdp6.s` **already aliases** BCACHE
slot 0 to that exact VFS scratch region. Its shared slot-0 tag takes
one fixed resident word. The separate MM-managed slot 1 is a
`0201`-word allocation (128 data plus tag), made lazily with
`mm_alloc_aligned_noreclaim`, not a second permanent KCORE buffer.
On workspace-backed cache misses, slot 0's live clean block is copied
to slot 1 before the workspace is overwritten. Reusing the MM slab as
scratch concurrently would destroy that protection; removing the
resident scratch and retaining another permanently allocated 128-word
buffer would merely relocate the RAM cost. The current sharing
already captures the obvious BCACHE overlay opportunity.

The MonitorFS swapped-process read explicitly invalidates the shared
workspace tag before `backstore_read` and uses a per-call stack source
record. The source comments say that swapping and scheduling preserve
executive syscall completion, but this does not prove safe arbitrary
overlap with every other filesystem/snapshot call. Physical provider
serialization through `fs_provider_reg_call` remains an independent
lifetime guarantee; MEMFS and snapshot paths require separate review.

No code change is accepted in v383. Linked v381/v382 baseline remains
KCORE **9113**, KCORE+MRES **15661**, PERMANENT_LAST **15708**;
no additional size saving or runtime test claim. Before a storage
rewrite, require a complete call graph including boot-before-MM,
reentrant provider callbacks, MonitorFS swapped reads and snapshot
mount/shutdown. Search for a *different* existing mutually exclusive
allocation rather than overlaying BCACHE's required backup slot.
Strict 32K acceptance remains deferred, and 922 additional KCORE
words are required for the below-8192 target.

## 2026-10-26 Proposal B shared filesystem workspace lifetime audit v382

Inspected the 0200-octal (128-decimal)-word `.bss` `fs_block_workspace`
owned by resident `vfs.s`. Direct references extend beyond D6FS and
DTFS: TSFS, MEMFS snapshot, MonitorFS swap, the `fs_mres` zero-block
bridge and block-cache operations also depend on this fixed address.
`fs_provider_reg_call` serializes physical providers DTFS, D6FS and
TSFS across their full requests: those requests may sleep, and the
provider lock prevents cross-provider workspace corruption. The
MEMFS fast path deliberately bypasses this lock because it does not
sleep. The MonitorFS swap reader explicitly invalidates the shared
block-cache workspace tag before loading a swap block and keeps a
cached-block key local to its source.

Consequently an unconditional MM relocation is **not accepted**:
fixed-address references across assembler/C, calls in potentially
different contexts, and synchronization/caching assumptions must
first be audited. In particular we have not proved that every
MonitorFS swap or snapshot access is covered by provider locking, or
that any MM block can be released with no persistent replacement.
Merely moving the 128 words outside KCORE would not establish a net
permanent-RAM saving. No source changed and no size reduction claimed;
linked baseline remains KCORE **9113**, KCORE+MRES **15661**,
PERMANENT_LAST **15708**, as measured in v381. Next investigation:
map each user and caller to the lock domain, establish boot-before-MM
requirements, then identify an *existing* mutually exclusive 128-word
area before attempting a zero-extra-memory overlay. Strict 32K
acceptance stays deferred.

## 2026-10-26 Proposal B VFS dispatch / resident transfer workspace audit v381

Tested removal of the apparent redundant JRST immediately before
`vfs_read_words_provider` in resident `vfs.s`; full linked GCC PDP-6
lowmem image remained **KCORE 9113**, **KCORE+MRES 15661**,
**PERMANENT_LAST 15708**. Reverted the size-neutral experiment.
No accepted production code modification or new runtime acceptance.

Resident VFS also contains `fs_block_workspace`, a 0200-octal
(128-decimal)-word BSS transfer buffer, documented as shared between
filesystem providers. This is potentially much more significant than
single-instruction edits, but no reassignment is safe until all
provider references, boot-before-MM use, locking/sleep lifetimes,
concurrency and MM allocation policies are proven. In particular,
replacing it with another permanent 128-word reservation would not
improve total permanent RAM; lowering KCORE alone is not proof of a
net resident saving. Analyze provider calls and lifetimes before
editing, and retain the documented ABI. Strict 32K acceptance stays
deferred. Baseline KCORE requires 922 fewer words to reach 8191.

## 2026-10-26 Proposal B VFS READ_WORDS provider decode audit v380

Reviewed the resident READ_WORDS VFS dispatch and its fast paths for
PIPE and MonitorFS process data. After the packed provider/kind check,
experimentally replaced the explicit halfword shift that extracts the
provider from AC7 with `LDB 7,[POINT 6,1,5]`. The complete linked
GCC PDP-6 lowmem measurement was unchanged: KCORE **9113**,
KCORE+MRES **15661**, PERMANENT_LAST **15708** words. The size-neutral
change was reverted; no production code or MRES source changed.
These paths remain candidates only for larger semantic-preserving
consolidations, not ineffective register decode rewriting. No new
runtime acceptance is claimed. KCORE strictly below 8192 needs 922
additional words; strict 32K acceptance remains deferred.

## 2026-10-26 Proposal B FILE character I/O register-move audit v379

Examined resident `file_readchar` and `file_writechar` for redundant
AC1 descriptor-pointer reloads immediately after saving the same
pointer in AC10 and before `file_node_from_fd`. Removed one source
MOVE from each call path experimentally. The complete linked GCC
PDP-6 lowmem measurement remained unchanged at KCORE **9113**,
KCORE+MRES **15661** and PERMANENT_LAST **15708** words, equal to
v378. This strongly indicates existing assembly folding already
removes the moves. Reverted the size-neutral source experiment;
there is no accepted runtime modification or new test acceptance.
The KCORE below-8192 target still needs 922 words. Future work
should concentrate on demonstrably larger shared request and VFS
blocks, not textual edits already folded by the toolchain. Strict
32K acceptance remains deferred.

## 2026-10-26 Proposal B remove dead bulk FILE offset setup v378

In resident `file_runtime.s`, the shared READ/WRITE bulk entry had
loaded descriptor offset into AC2 and redundantly moved the descriptor
pointer from AC1 back into AC1 just before calling `file_node_from_fd`.
Inspection of the helper confirms that it requires only AC1 (already
holding the descriptor pointer), preserves AC2, and returns the vnode
in AC1. The subsequent shared request setup overwrites AC2 with the
transfer length; the descriptor offset is loaded at the VFS provider
boundary where needed. Both dead moves were removed. No provider
semantics, public ABI, or MRES source changes.

Full linked GCC PDP-6 lowmem against v377: KCORE **9115 -> 9113**
(-2), KCORE+MRES **15663 -> 15661** (-2), PERMANENT_LAST
**15710 -> 15708** (-2). daimos-testkit `target-file-pdp6`,
`target-file-walk-pdp6`, `test-daimos-vfs-stat-frame` and
`test-daimos-pre-dsh-fs-closure` PASS. No new concurrent physical
I/O acceptance is claimed. Below-8192 KCORE still requires 922
further words; strict 32K acceptance remains deferred.

## 2026-10-26 Proposal B shared bulk VFS provider arguments v377

Resident `file_runtime.s` bulk READ and WRITE previously duplicated
four register moves to prepare fallback VFS provider requests:
AC3 buffer, AC4 count, AC1 vnode, AC2 descriptor offset. A common
`file_words_provider_common_v377` block now prepares these once;
WRITE reaches it by fallthrough, READ jumps to it. The signed
READ/WRITE selector in AC5 then dispatches READ directly to
`vfs_read_words` while WRITE retains the original APPEND provider
EOF-lock decision and `vfs_write_words` path. Direct physical-device
shortcuts and shared descriptor-offset handling remain unchanged.
No ABI, MRES code or permanent state is added.

Full linked GCC PDP-6 lowmem vs v376: KCORE **9117 -> 9115**
(-2), KCORE+MRES **15665 -> 15663** (-2), PERMANENT_LAST
**15712 -> 15710** (-2). daimos-testkit `target-file-pdp6`,
`target-file-walk-pdp6`, `test-daimos-vfs-stat-frame`, and
`test-daimos-pre-dsh-fs-closure` all PASS. Physical-provider
concurrency/APPEND end-to-end acceptance remains separate.
KCORE below 8192 needs 924 further words; strict 32K acceptance
remains deferred.

## 2026-10-26 Proposal B shared bulk READ/WRITE argument save v376

In resident `system/kernel/fs/file_runtime.s`, `file_read_words` and
`file_write_words` duplicated three entry PUSH instructions saving
AC10, AC2 buffer and AC3 transfer length. Moved those three identical
PUSHes into common `file_words_save_args_v376`, after the distinct
READ/WRITE permission selector in AC5 is initialized. READ branches
to the shared prologue and WRITE falls through into it. Both enter
`file_words_common` with identical stack layout, original AC1 fd,
arguments, and selector values; the existing result cleanup and
provider-side append semantics remain untouched. No MRES change.

Full linked GCC PDP-6 lowmem versus v375: KCORE 9120 -> **9117**
(-3 words), KCORE+MRES 15668 -> **15665** (-3), PERMANENT_LAST
15715 -> **15712** (-3). daimos-testkit `target-file-pdp6`,
`target-file-walk-pdp6`, `test-daimos-vfs-stat-frame` and
`test-daimos-pre-dsh-fs-closure` PASS. No new physical-device
concurrency acceptance is claimed. The below-8192 KCORE goal requires
926 further words; strict 32K acceptance remains deferred.

## 2026-10-26 Proposal B FILE OPEN directory flag branch v375

In resident `file_open`, replace the conditional branch around
`MOVEI 3,1` (directory argument to `file_new_fd`) with a conditional
`CAIN 1,1` followed by `MOVEI 3,1`: CAIN executes the MOVEI only
for `VFS_TYPE_DIR=1`, leaving zero for every other type. The first
experiment incorrectly used CAIE, which inverted the condition;
it was caught in review and immediately corrected **before testing
or committing**. Only the correct CAIN version is retained.

Full linked GCC PDP-6 lowmem against accepted v374: KCORE
9121 -> **9120** (-1), KCORE+MRES 15669 -> **15668** (-1),
PERMANENT_LAST 15716 -> **15715** (-1). daimos-testkit
`target-file-pdp6`, `target-file-walk-pdp6`,
`test-daimos-vfs-stat-frame` and `test-daimos-pre-dsh-fs-closure`
PASS on the corrected source. Public FILE ABI, directory semantics
and provider behavior unchanged. No MRES code modified. KCORE below
8192 needs 929 more words; strict 32K acceptance deferred.

## 2026-10-26 Proposal B FILE OPEN access-mask consolidation v374

Consolidated the `file_open` READ/WRITE/TRUNC permission-mask builder
in resident `system/kernel/fs/file_runtime.s`. The prior branch tree
set bit 4 if READ and bit 2 if WRITE or TRUNC. Replaced it with two
conditional `TRNE` / `TRO` pairs, checking combined flag mask 022
for WRITE|TRUNC. The resulting mask for all 8 combinations of these
three input bits is identical, as are the subsequent STAT record
access check, CREATE path and failure handling. No public ABI, extra
persistent state or MRES modification.

Full linked GCC PDP-6 lowmem versus v373: KCORE 9125 -> **9121**
(-4 words), KCORE+MRES 15673 -> **15669** (-4), PERMANENT_LAST
15720 -> **15716** (-4). Native daimos-testkit `target-file-pdp6`,
`target-file-walk-pdp6`, `test-daimos-vfs-stat-frame`, and
`test-daimos-pre-dsh-fs-closure` PASS. Full Open flag/error matrix
and physical-provider concurrency have not been separately accepted;
strict 32K acceptance remains deferred. Below-8192 KCORE requires
930 additional words. Continue resident file-runtime and VFS
structural analysis, prioritizing persistent RAM over speed.

## 2026-10-26 Proposal B KCORE FILE OPEN parent permission reuse v373

The resident FILE OPEN create-on-miss branch performed its parent
write/search access check with an inline `MOVEI 2,3` followed by
`PUSHJ file_check_access`, duplicating the contract already
centralized for SYMLINK/RENAME/MKDIR/UNLINK in the existing
`file_parent_write_access_v367` tail-call helper. Reused that helper
in `file_open` with unchanged parent vnode, permission mask and
failure semantics. No MRES changes, new state or new public ABI.

Full linked GCC PDP-6 lowmem versus v372: KCORE 9126 -> **9125**
(-1), KCORE+MRES 15674 -> **15673** (-1), PERMANENT_LAST 15721 ->
**15720** (-1). Native daimos-testkit `target-file-pdp6`,
`target-file-walk-pdp6`, `test-daimos-vfs-stat-frame`, and
`test-daimos-pre-dsh-fs-closure` PASS. KCORE below 8192 requires
934 additional words. Continue larger structural FILE/VFS analysis;
strict 32K acceptance remains deferred.

## 2026-10-26 Proposal B KCORE MOUNT vnode slot encoding v372

In the resident `vfs_mount_found` path, the public mount ID is the
validated zero-based slot plus one. Replaced `MOVE 6,7` followed by
`ADDI 6,1` with `MOVEI 6,1(7)` before the unchanged mount/kind
field shifts and bitwise packing. No state storage, ABI, root vnode
bit layout or provider interface changed.

Full linked GCC PDP-6 lowmem relative to v371: KCORE 9127 ->
**9126** (-1), KCORE+MRES 15675 -> **15674** (-1),
PERMANENT_LAST 15722 -> **15721** (-1). `daimos-testkit` targets
`target-file-pdp6`, `target-file-walk-pdp6`,
`test-daimos-vfs-stat-frame`, `test-daimos-pre-dsh-fs-closure`
PASS. Multi-provider concurrent MOUNT/UNMOUNT acceptance remains
separate; strict 32K acceptance deferred. KCORE below 8192 requires
935 further words. Larger resident structural consolidations remain
the focus of Proposal B.

## 2026-10-26 Proposal B KCORE UNMOUNT mount-ID preparation v371

After restoring DevSpace2 connectivity, inspected the v370 checkout
and optimized two zero-based-slot-to-public-mount-ID preparations in
resident `system/kernel/fs/vfs.s` `vfs_unmount`:
`MOVE 4,2` + `ADDI 4,1` in the protected owner checks, and
`MOVE 1,2` + `ADDI 1,1` before `file_unlock_mount`, each replaced
by one PDP-6 indexed `MOVEI dest,1(2)`. AC2 remains the source slot,
with unchanged public owner tokens, error order and unmount lock
release. No new state and no MRES modifications.

Complete linked GCC lowmem image versus v370: KCORE 9129 ->
**9127** (-2), KCORE+MRES 15677 -> **15675** (-2),
PERMANENT_LAST 15724 -> **15722** (-2). Native daimos-testkit
`target-file-pdp6`, `target-file-walk-pdp6`,
`test-daimos-vfs-stat-frame`, `test-daimos-pre-dsh-fs-closure`
all PASS. No concurrent physical-provider unmount acceptance is
claimed. Another 936 words are needed to reach below 8192; strict
32K acceptance remains deferred. Larger structural VFS/FILE work
remains the main Proposal B priority.

## 2026-10-26 Proposal B VFS unmount provider-field decode v370

In `system/kernel/fs/vfs.s`, after `vfs_sync` and before deciding
whether a provider requires PREPARE_UNMOUNT, replaced `MOVE 3,1` plus
`LSH 3,-036` with the direct `LDB 3,[POINT 6,1,5]` provider-field
extraction already used elsewhere in resident VFS. The existing
MEMFS/D6FS prepare-only dispatch and unmount completion order remain
unchanged; no MRES code or interface was modified.

Full linked GCC lowmem image against v369: KCORE 9130 -> **9129**,
KCORE+MRES 15678 -> **15677**, PERMANENT_LAST 15725 -> **15724**
(all -1 word). `daimos-testkit` native PDP-6 `target-file-pdp6`,
`target-file-walk-pdp6`, `test-daimos-vfs-stat-frame`, and
`test-daimos-pre-dsh-fs-closure` PASS. The test scope does not include
full multi-provider concurrent physical unmount acceptance. KCORE
strictly below 8192 still needs 938 words. Strict 32K boot acceptance
is deferred; further Proposal B work remains on resident MOUNT/UNMOUNT,
FILE OPEN and provider-boundary restructuring.

## 2026-10-26 Proposal B KCORE structural size census v369

Resumed KCORE-only optimization after v368. Inspected the live
`file_runtime.s` and `vfs.s` source and target DOBJ symbol sections,
using the current v368 GCC lowmem linked baseline. No MRES code,
package metadata, or exports were considered optimization targets.

**Pre-link section census, not a claim of additive linked savings:**
`file_runtime.dobj`: text 1127 words, data 0, BSS 0;
`vfs.dobj`: text 770, data 27, BSS 128. The VFS mount
base pointers and active slot-count form three data words. Several
other VFS counters/policies already alias otherwise unused resident
device-state words, and mount target/root tables are MM-backed;
blindly moving them or adding a second allocation would increase
permanent RAM or break boot publication. Thus a simple mount-table
relocation is not a justified candidate.

DOBJ symbol-entry spans (a useful hotspot ranking, **not** exact
function sizes because local routines and helper code may be included):

- FILE: `file_open` 116; `file_write_words` 117 (includes common bulk
  READ/WRITE dispatch); `file_getpath` 93; `file_lock` 79;
  `file_component` 51; `file_dup2` 43; `file_readchar` 43;
  `file_writechar` 43 words.
- VFS: `vfs_mount_prevalidated` 101 (includes mount setup),
  `vfs_readdir` 91; `vfs_unmount` 82; `vfs_parent_name` 81;
  `vfs_lookup` 64; `vfs_create_op` 41 words.

**Next investigation order:** (1) compare VFS MOUNT/UNMOUNT
request and publication phases, proving lifetime and error unwind
semantics before sharing any state; (2) review FILE OPEN and bulk
READ/WRITE dispatch for large mutually exclusive pathways without
changing device ABI, EOF/APPEND handling or offset publication;
(3) check VFS READDIR/PARENT_NAME for duplicated namespace-name
handling while preserving mount-root traversal. Retain the already
MM-backed mount arrays and serialized physical-provider gateway.
Build-native tests in `daimos-testkit`, including concurrency and
error paths, remain required for any accepted change. No production
edit or new saving is claimed in this census: full linked KCORE
remains **9130** words; below 8192 requires 939 more. Strict 32K
acceptance remains deferred.

## 2026-10-26 Proposal B parent/leaf frame helper experiment v368

Inspected resident parent/leaf request setup in `file_runtime.s` for
SYMLINK, MKDIR/MKFIFO, RENAME and UNLINK/RMDIR. Tested a new tail-call
helper deriving the contiguous leaf output pointer from the directory
output pointer, replacing duplicated `MOVEI AC3` setup at two call
sites. A full linked GCC lowmem image measured KCORE **9130**, all
MRES combined **15678**, PERMANENT_LAST **15725**, equal to v367.
The helper saved no RAM and was reverted immediately; no change to
filesystem semantics or production source is retained, and no new
runtime test is claimed for this rejected candidate. Continue
Proposal B with larger resident file-runtime/VFS request handling;
MRES optimization and strict 32K acceptance remain deferred.

## 2026-10-26 Proposal B KCORE parent-mutation access helper v367

Consolidated the repeated parent write-plus-search permission call
in resident `system/kernel/fs/file_runtime.s`. `file_symlink`,
`file_rename` (both parent directories), `file_make_node` and
`file_remove` now invoke the small internal
`file_parent_write_access_v367` leaf; it loads access mask 3 and
uses a tail JRST to `file_check_access` so the original caller's
return address, arguments and error semantics are preserved.
The common helper has no persistent state and no MRES changes.

Complete GCC PDP-6 lowmem linked image versus v366: KCORE
9133 -> **9130** (-3 words), KCORE+all MRES 15681 -> **15678**
(-3), PERMANENT_LAST 15728 -> **15725** (-3). Native
`daimos-testkit` targets `target-file-pdp6`,
`target-file-walk-pdp6`, `test-daimos-vfs-stat-frame` and
`test-daimos-pre-dsh-fs-closure` PASS. This does not demonstrate
all failure branches of RENAME/SYMLINK or physical-provider
concurrency. Below-8192 KCORE target requires 939 additional
words. Strict 32K acceptance remains deferred.

## 2026-10-26 Proposal B KCORE component clear BLT audit v366

Audited the resident `file_component` five-word output `vfs_name`
zero-initialization and its PDP-6 `BLT` source/destination pointer
setup. Experiment replaced `HRLI 0,(3)` with `HRL 0,3` after loading
`1(3)` into AC0; it compiles, but the complete linked GCC lowmem
image is unchanged: KCORE **9133**, KCORE+all MRES **15681**,
PERMANENT_LAST **15728** words. The experiment was reverted rather
than retain a size-neutral rephrasing. No MRES source changes and no
new runtime acceptance are claimed. Follow-on Proposal B work should
favor large resident file-runtime/VFS structural consolidation over
more zero-gain instruction substitutions. The below-8192 KCORE target
requires 942 further words; strict 32K acceptance remains deferred.

## 2026-10-26 Proposal B KCORE indexed path byte-pointer offset v365

In `system/kernel/fs/file_runtime.s`, `file_component` constructs
its packed-SIXBIT input pointer after dividing the current character
position by six. Replaced `ADD 6,1` / `ADDI 6,1` with indexed
`ADDI 6,1(1)` to include both the path address and the one-word
counted-path header offset in one PDP-6 effective-address operation.
Both path pointers lie within the native lowmem address range; the
byte position remainder, SIXBIT pointer LH and path semantics remain
unchanged. No MRES packaging, exports or runtime was changed.

Full linked GCC lowmem KCORE 9134 -> **9133** (-1), all-MRES
combined size 15682 -> **15681** (-1), PERMANENT_LAST 15729 ->
**15728** (-1). `daimos-testkit` native PDP-6 `target-file-pdp6`,
`target-file-walk-pdp6`, `test-daimos-vfs-stat-frame`, and
`test-daimos-pre-dsh-fs-closure` PASS. A further 942 words are
needed to get strictly below 8192. Strict 32K acceptance deferred.

## 2026-10-26 Proposal B KCORE output SIXBIT pointer setup v364

`file_component` initialized its output SIXBIT byte pointer through a
literal pointer MOVE, a separate effective destination address load,
and HRR register merge. Replaced that three-instruction sequence with
`MOVEI 1,1(3)` followed by `HLL 1,[POINT 6,0]`, preserving the
right-half address and installing the same left-half byte-pointer
format. Source record parsing, component count, trailing separator
consumption and public VFS name semantics are unchanged. No MRES
source or export metadata was changed.

Full linked GCC PDP-6 lowmem KCORE 9135 -> **9134** (-1), combined
KCORE+all MRES 15683 -> **15682** (-1), PERMANENT_LAST 15730 ->
**15729** (-1). daimos-testkit `target-file-pdp6`,
`target-file-walk-pdp6`, `test-daimos-vfs-stat-frame` and
`test-daimos-pre-dsh-fs-closure` PASS. This is an accepted resident
KCORE-only size reduction. KCORE below 8192 requires 943 additional
words; strict 32K acceptance remains deferred.

## 2026-10-26 Proposal B KCORE SIXBIT path pointer construction v363

In the resident `file_component` implementation, after `DIVI` and
addition of the path-data base, AC6 already contains the desired
right-half address for the input SIXBIT byte pointer. Replaced the
three-instruction MOVe literal / HRR / MOVe sequence with one
`HLL 6,[POINT 6,0]`, installing the byte-pointer left-half while
preserving the computed right-half address. No change to byte offset,
separator handling, component-size limit or caller ABI; no MRES work.

Complete GCC PDP-6 lowmem linked image compared with v362:
KCORE 9137 -> **9135** (-2), combined KCORE+MRES 15685 ->
**15683** (-2), PERMANENT_LAST 15732 -> **15730** (-2).
`daimos-testkit` `target-file-pdp6`, `target-file-walk-pdp6`,
`test-daimos-vfs-stat-frame`, `test-daimos-pre-dsh-fs-closure`
all PASS. KCORE below 8192 requires 944 further words.
Strict 32K acceptance remains deferred.

## 2026-10-26 Proposal B KCORE file_getpath index multiply v362

The resident absolute-path formatter `file_getpath_common` uses a
five-word component array of up to sixteen path parts. Both the
upward parent-walk and reverse output walk computed `5 * depth` with
`LSH 4,2` plus `ADD 4,depth`. Replaced each two-word sequence with
single `IMULI 4,5`, preserving AC4, stack record indexing, operation
order, public ABI and pathname semantics. No MRES source was changed.

Full linked GCC PDP-6 lowmem baseline v361: KCORE 9139 -> **9137**
(-2 words), KCORE+all MRES 15687 -> **15685**, PERMANENT_LAST
15734 -> **15732**. daimos-testkit PDP-6 targets
`target-file-pdp6`, `target-file-walk-pdp6`,
`test-daimos-vfs-stat-frame` and `test-daimos-pre-dsh-fs-closure`
PASS. These are general file/VFS regression tests; a dedicated
native deep GETPATH/GETCWD path-component matrix has not yet been
run, and broader acceptance should include one. The reduction
is entirely resident KCORE. To get below 8192 from 9137 requires
946 additional words; strict 32K acceptance stays deferred.

## 2026-10-26 Proposal B shared descriptor clone v361

Consolidated duplicated resident `file_dup`/`file_dup2` descriptor
cloning in `system/kernel/fs/file_runtime.s` into the internal
`file_clone_descriptor_v361` leaf. The shared sequence copies both
contiguous FILE words, copies the descriptor mount-extension field,
reconstructs the canonical vnode and increments pipe references only
for the PIPE provider. `file_dup` preserves its returned fd across the
helper; `file_dup2` retains close-before-overwrite, equality, and error
semantics. Public FILE ABI and MRES sources remain untouched.

Full linked GCC PDP-6 lowmem disk-root measurement against v360:
KCORE **9148 -> 9139** (-9 words); KCORE+all MRES
15696 -> 15687 (-9); PERMANENT_LAST 15743 -> 15734 (-9).
`daimos-testkit` PDP-6 targets `target-file-pdp6`,
`target-file-walk-pdp6`, and checks `test-daimos-vfs-stat-frame`,
`test-daimos-pre-dsh-fs-closure` all PASS. Targeted native extracted
FILE mount-extension contract `daimos-file-mountext-v19.sh` PASS,
covering DUP/DUP2, 26-mount vnode extensions, locks, close and unmount.
This does not replace full concurrent-provider stress testing. KCORE
below 8192 needs another 948 words from the new baseline. Strict
32K acceptance remains deferred; no MRES optimization was performed.

## 2026-10-26 Proposal B shared bulk-I/O register preparation v360

Reduced permanent KCORE by three words in
`system/kernel/fs/file_runtime.s`. Resident `file_words_common` used
to build identical register arguments separately for READ and WRITE:
AC0 canonical vnode, AC1 user buffer, AC2 word count. These three
moves now execute once before the existing READ/WRITE branch. Device
selection, provider register ABI, append-at-EOF handling, lock and
sleep semantics, and descriptor offset updates remain unchanged.

Full linked GCC lowmem disk-root image versus v359: KCORE
9151 -> **9148**, KCORE+all MRES 15699 -> **15696**, and
PERMANENT_LAST 15746 -> **15743**. All MRES objects are unchanged;
the saving is entirely resident KCORE. PDP-6 `daimos-testkit`
`target-file-pdp6` and `target-file-walk-pdp6` PASS, along with
`test-daimos-vfs-stat-frame` and
`test-daimos-pre-dsh-fs-closure` PASS. This is targeted regression
coverage, not a full physical-provider concurrency stress test.
The next KCORE milestone below 8192 requires 957 further words.
Strict 32K acceptance remains deferred. No MRES size optimization
is part of this change.

## 2026-10-26 Proposal B KCORE-only file-runtime entry audit v359

Resumed Proposal B strictly on KCORE (`file_runtime.s`, its VFS
boundary); no MRES packaging or metadata work. Inspected resident
readchar/writechar/readdir descriptor-to-vnode setup and the path,
permission and create/remove request frames. Three places redundantly
copied AC1 from a saved descriptor register back into AC1 immediately
after saving it. Tested removing one such move (v358) and then all
three (v359) against the complete GCC lowmem linked image, using
the current mandatory MonitorFS MRES build configuration. Both
experiments were **size neutral**: KCORE 9151, KCORE+MRES 15699,
PERMANENT_LAST 15746. Reverted all changes, including the three-move
experiment. No runtime test is claimed for rejected experiments.

This verifies that source-level micro-deletions are insufficient after
current assembler/link folding. Continue with actual structural
file-runtime/VFS opportunities: eliminate substantial duplicated
request preparation or persistent state only with ownership/lifetime
proofs. Keep provider sleeping/serialization contract intact. MonitorFS
MRES/export size work is explicitly out of scope in this KCORE phase.
KCORE goal remains below 8192 words; strict 32K acceptance deferred.

## 2026-10-25 MonitorFS MRES linked-overhead audit v357

Audited the always-loaded MonitorFS ABI trampoline file,
`mfsdev_install_minit` in `module_minit.c`, VFS process/domain name
callers, nine export declarations in the boot MRES link, and the
relocated `monitorfs-mres.map`. Nine fixed KCORE jump words correspond
to distinct ABI services, including `mfsproc_format_slot` called by
resident VFS; nine MRES exports have non-contiguous code offsets. Thus
neither dropping the last formatter entry nor assuming contiguous
function addresses is ABI-safe. A staged C simplification caching
`SIXBIT("MFS   ")` in a local name variable was compiled and measured
against the **complete linked lowmem image**; figures remained exactly
KCORE 9151, KCORE+MRES 15699, PERMANENT_LAST 15746. Because it saved
no permanent words, the experiment was reverted without commit.

No accepted production change from v356. The combined +17 words
compared with the historical resident default remain unresolved. A
future reduction should derive export/trampoline overhead from actual
linked package metadata and preserve independent ABI entry addresses,
then validate a native boot and filesystem semantic regression. The
full-profile permanent-address overflow and strict 32K acceptance
remain deferred.

## 2026-10-25 Mandatory MonitorFS MRES install cleanup v356

Removed the remaining compile-time `MONITORFS_MRES_ENABLED` branches
around the MonitorFS MRES package declarations and boot MINIT table
entry in `system/kernel/modules/module_table.s`, and around
`mfsdev_install_minit` in `module_minit.c`. MonitorFS now has an
unconditional MINIT installer in both configurations; the resident
MonitorFS implementation cannot be selected by disabling a build flag.
The kernel assembly copy selection is also unconditional and always
uses the ABI trampoline-only `fs/monitorfs_dispatch_v14.s`. The nine
resident ABI jump words remain necessary under the current interface.

Full-linked GCC 64K lowmem image metrics: KCORE 9151, KCORE+MRES
15699, PERMANENT_LAST 15746 words, **no RAM growth** from v355.
Native `daimos-testkit` `test-daimos-monitorfs-mres-domains-v346`
PASS with an isolated lowmem PDP-6 image, exercising process stat/read,
domain directory enumeration and numeric-domain lookup under the
forced always-on package. Earlier child/zombie test v354 passed under
the same forced package model in v355. This v356 cleanup does **not**
recover the remaining +17 combined words; do not remove ABI trampolines
without proving external callers and MINIT patch relocation remain
correct. Full-profile permanent-address overflow and strict 32K
acceptance remain deferred.

## 2026-10-25 MonitorFS always-loaded MRES, no resident implementation v355

MonitorFS is now **MRES-only by construction**. The PDP-6 boot build
includes the `monitorfs-mres` package unconditionally in both lowmem
and full MRES package sets, and both boot and kernel Makefiles force
`MONITORFS_MRES_ENABLED=1` even against old command-line requests to
disable it. Kernel build `monitorfs_runtime.s` is always assembled from
`fs/monitorfs_dispatch_v14.s` rather than the old full
`monitorfs_runtime.s` + `monitorfs_swap_v63.c` implementation.
Thus the MonitorFS device, process, domain, and swapped metadata runtime
is linked into the pinned, boot-installed MRES and **not resident as
full KCORE implementation**. The unavoidable nine one-word ABI/MINIT
jump slots remain in KCORE and are patched to the relocated package at
boot; VFS's shared generic provider routing stays resident. Do not
claim literally zero MonitorFS-related KCORE words.

Full-linked lowmem GCC default build measured KCORE=9151,
KCORE+MRES=15699, PERMANENT_LAST=15746. An explicit obsolete
`MONITORFS_MRES_ENABLED=0` argument still produces these figures,
proving resident fallback is disabled. Native daimos-testkit
`test-daimos-procinfo-zombie-v354` **PASS** under this forced default
(including RUN/WAIT, PROCINFO and decimal `/PROC/10/STATE`). Existing
native device, process, and domain MRES tests v341/v343/v344/v346
remain useful. No new permanent RAM change compared with prior
opt-in MRES, but compared with former normal non-MRES default,
KCORE is reduced by 1575 words while combined RAM is increased by 17.

Full-profile GCC size-only build yields KCORE=9151,
KCORE+MRES=18463, PERMANENT_LAST=18510 and fails its existing
`040000` permanent-address gate; full-profile boot acceptance is **not
claimed**. Keep the larger size restriction and strict 32K acceptance
on the deferred backlog; do not change this limit by workaround.
Future optimization may recover the 17 combined resident words and
remove unnecessary jump slots only if ABI-safe, with full linked
measurement before acceptance. Feature is no longer opt-in.

## 2026-10-25 Native zombie PROCINFO safety and RAM-neutral fix v354

Root cause of baseline v348-v351 lowmem child-completion halt found in
`system/kernel/proc/syscall_info.s`: `vm_space_destroy` publishes
`VM_SPACE_NONE` (`-1`) in the zombie's `vm_state`, while
`sys_procinfo_user_comm` previously checked only the nonzero RH
image base and could dereference the invalid `0777777` image address
while reconstructing COMM from argv[0]. A negative VM descriptor is
now treated as unavailable and selects the existing `USER` fallback.
The argv[0] offset is also added directly from memory to AC7,
eliminating one intermediate MOVe and offsetting the one-word safety
check without altering pointer interpretation. The first experimental
explicit zombie-state check would have cost +5 KCORE words and was
replaced; the narrower VM sentinel guard initially cost +1 word; the
final combined fix has **zero full-linked permanent RAM growth**.

Native 64K GCC lowmem regression in daimos-testkit
`test-daimos-procinfo-zombie-v354` **PASS**, including the previously
failing child `PROCINFO`, pipe EOF, WAIT and decimal `/PROC/10/STATE`
S6REC lookup checks. With the final source, full-linked default
KCORE 10726, combined 15682, PERMANENT_LAST 15729 words; optional
MonitorFS MRES KCORE 9151, combined 15699, PERMANENT_LAST 15746,
unchanged against v329/v333 respective baselines. Testkit commit
`c772b54`. Related v348-v351 HALTs are resolved in this tested
configuration, independent of MonitorFS extraction. The broader
MRES swapped-process metadata acceptance remains outstanding;
MonitorFS MRES stays opt-in, strict 32K gate deferred.

## 2026-10-25 Native lowmem SYS_PROCINFO immediately after RUN v351

Adapted the baseline native one-child RUN/PIPE test to call
`dsys_procinfo(pid,&info)` immediately after `SYS_RUN` returns,
before closing parent pipe descriptors or waiting for the child.
Using 64K GCC lowmem, `MONITORFS_MRES_ENABLED=0`, native PDP-6 trace
reached `D6FS LOADED`, then `<RSQT` before halting later at
`PC: 000000 (LUUO00 0,300)`. The `T` marker is emitted after the
first `PROCINFO(2)` returned zero; the fixture also checked that
`info.pid` equals 2. Hence initial child-slot PROCINFO dispatch and
record population work. The subsequent `wait_zombie_by_eof` path
still fails when querying PROCINFO after writer close, EOF read and
reader close, as previously shown in v348/v349. The defect is
associated with child completion/zombie state or with its preceding
lifecycle transition, not ordinary PROCINFO dispatch, and reproduces
without MonitorFS MRES.

The experimental fixture was withdrawn after the diagnostic run;
unrelated dirty testkit state was left untouched. No production
source or permanent RAM changes. Next isolate PROCINFO after each
individual teardown operation and study process slot zombie-layout
in `syscall_info.s`; reject any speculative workaround. MonitorFS
MRES native namespace acceptance v341/v343/v344/v346 remains PASS,
feature opt-in with KCORE=9151 and combined 15699 (+17 default),
strict 32K acceptance deferred.

## 2026-10-25 Native lowmem no-child SYS_PROCINFO acceptance v350

Added `daimos-testkit` native 64K PDP-6 GCC lowmem test
`test-daimos-procinfo-current-v350` with `MONITORFS_MRES_ENABLED=0`.
User INIT calls `dsys_procinfo(1,&info)` without RUN, PIPE, WAIT or
other child lifecycle activity, verifies that the UUO returns zero and
that `info.pid == 1`, then emits the completion marker. Target run
**PASS**. Consequently, the native UUO 057 path is functional for
the initial process with a mapped caller result buffer.

Earlier v348-v349 tests consistently halted inside PROCINFO(2)
after completing child RUN, pipe-writer close, pipe EOF read and
pipe-reader close, regardless of the MonitorFS MRES switch. The
remaining fault is therefore related either to non-initial/zombie
process-table record handling or to state left by the preceding
lifecycle operations, not merely to invoking PROCINFO itself.
Check `sys_procinfo` handling of a zombie slot and its `COMM`
reconstruction, plus native syscall mapping/return invariants;
compare a direct PROCINFO(2) after RUN without pipe teardown.

The accepted testkit commit is `7dc7023`; the test touches no kernel
production source. Fixed KCORE baseline 10726, combined 15682;
optional MonitorFS MRES remains KCORE 9151 and combined 15699 (+17).
Feature stays opt-in, strict 32K acceptance deferred.

## 2026-10-25 Baseline lowmem SYS_PROCINFO comparison v349

Compared the one-child PIPE/RUN/EOF/PROCINFO probe against an isolated
64K GCC lowmem image with `MONITORFS_MRES_ENABLED=0`, matching v348
apart from disabling the extracted MonitorFS MRES. Both builds booted
through `D6FS LOADED`, and both emitted `<RSWabcd` before HALT at
`PC: 000000 (LUUO00 0,0)`. Thus the failure in `dsys_procinfo()`
also occurs in the default lowmem baseline. It is **not** evidence of
a defect introduced by MonitorFS MRES extraction. The probe did not
access MonitorFS namespace paths.

Source inspection locates the syscall ABI in `userland/libc/syscall.s`
(`dsys_procinfo: uuo 057,0(1)`), native dispatch in
`system/kernel/proc/syscall_dispatch_pdp6.s` (map result buffer, call
`sys_procinfo`), and the implementation in
`system/kernel/proc/syscall_info.s`. The root cause of the halt remains
unresolved; next step is a minimal zero-child SYS_PROCINFO probe and
trap/return tracing, with assembler/compiler defects prioritized.
The temporary failing baseline testkit fixture was withdrawn with
unrelated dirty testkit files intact. No production code or RAM change.

Prior accepted MRES tests v341/v343/v344/v346 remain PASS; optional
KCORE 9151 and combined 15699 (+17 versus default). Feature stays
opt-in; strict 32K acceptance deferred.

## 2026-10-25 Proposal B one-child SYS_PROCINFO trap localization v348

Created a temporary daimos-testkit native lowmem GCC 64K PDP-6
`MONITORFS_MRES_ENABLED=1` single-child RUN/PIPE lifecycle probe.
Instrumented the four distinct steps of `wait_zombie_by_eof` after
parent's RUN return: `a` before closing parent writer, `b` after
writer close, `c` after reading EOF, `d` after closing reader, `e`
only after returning from `dsys_procinfo`. PDP-6 trace reached
`D6FS LOADED`, emitted `<RSWabcd`, then halted at
`PC: 000000 (LUUO00 0,0)`. Thus writer close, EOF read, reader
close all completed successfully. The failure occurs within or while
returning from `dsys_procinfo(pid,&info)`, before `e` could print.
The libc entry uses `uuo 057,0(1)` in `userland/libc/syscall.s`.
No MonitorFS path is used by this probe. Diagnose the SYS_PROCINFO
UUO dispatch/return path and compare identical lowmem configuration
with `MONITORFS_MRES_ENABLED=0` before attributing to MRES.

The failed tracing fixture was withdrawn; all other dirty testkit
files were preserved. No production code or memory footprint changed.
Accepted native MRES namespaces v341/v343/v344/v346 remain passing.
Optional KCORE 9151, KCORE+MRES 15699 (+17 vs default), opt-in,
strict 32K acceptance deferred.

## 2026-10-25 Proposal B one-child RUN/WAIT isolation v347

Adapted the existing full-profile process decimal native fixture to an
isolated 64K lowmem GCC image with `MONITORFS_MRES_ENABLED=1`, exactly
one child rather than nine, and markers for pipe creation (`N`),
`SYS_RUN` entry (`R`), return from RUN (`S`), and entering the
zombie-by-pipe/PROCINFO check (`W`). The PDP-6 boot reached D6FS
LOADED; userspace emitted `<NRSW` and then halted at
`PC: 000000 (LUUO00 0,300)`. Thus the pipe creation and first RUN
returned to the parent successfully. The issue occurs inside the
completion routine, which closes the parent's pipe writer, consumes
pipe EOF, closes the reader, then checks `dsys_procinfo` for the zombie.
**No MonitorFS path was accessed** in this probe. The remaining
fault must be separated into those pipe and PROCINFO steps, and
lowmem baseline behavior should be compared before attributing to
MRES. The failed experimental testkit fixture was withdrawn; unrelated
dirty files remain untouched.

No production modification or RAM savings. Accepted v341/v343/v344/v346
MonitorFS MRES namespace tests retain PASS status. Permanent optional
KCORE 9151, combined 15699 (+17 over default). Feature remains
opt-in; strict 32K acceptance deferred.

## 2026-10-25 Proposal B MonitorFS MRES domain lookup acceptance v346

The failed v345 domain enumeration-to-STAT test was caused by a testkit
SIXBIT encoding error: literal `/` requires octal `017`, but v345
constructed the prefix with `057`. Corrected the prefix in a new
versioned daimos-testkit native test `test-daimos-monitorfs-mres-domains-v346`.
With `MONITORFS_MRES_ENABLED=1`, a 64K GCC lowmem PDP-6 image
successfully validates the v343 process and v344 domain-root checks,
reads a numeric domain ID from READDIR, constructs the corresponding
`/MONITOR/DOMAINS/<id>` SIXBIT path with correct separators, then
successfully STATs it. Native test **PASS**. The domain identity
mismatch from v345 was a fixture bug, not evidence of a kernel
namespace defect. Testkit commit `26c6404`.

No kernel production code or RAM changes. The optional MRES remains
KCORE 9151 and combined 15699, +17 words versus default v329.
Child RUN/WAIT and swapped-process metadata acceptance remain open;
strict 32K acceptance deferred.

## 2026-10-25 Proposal B MonitorFS MRES domain identity probe v345

Extended v344's successful 64K native MRES domain-root test to require
one `/MONITOR/DOMAINS` directory entry, validate a one-to-three-digit
SIXBIT domain ID, construct `/MONITOR/DOMAINS/<id>` from that returned
name, and call STAT on the resulting domain directory. The native PDP-6
image booted and passed v343's process checks and the v344 domain-root
checks, but reported `<ABCDE!c`: domain-entry identity was numeric,
yet STAT on the constructed path failed. This may be a fixture
path-encoding error or a real MonitorFS domain lookup inconsistency;
not established. The fixture's new code was withdrawn after failure,
and pre-existing dirty testkit files were preserved. Do not cite this
as native domain-leaf acceptance. Future investigation should compare
an independently encoded literal domain path, the returned directory
name and the VFS lookup chain before modifying the kernel.

No production code changes or RAM savings. The accepted native domain
root v344 remains PASS. Optional MonitorFS MRES is KCORE 9151 words,
KCORE+MRES 15699 (+17 versus default); 32K acceptance remains deferred.

## 2026-10-25 Proposal B MonitorFS MRES domain root access v344

Added `daimos-testkit` native 64K PDP-6 GCC lowmem test
`test-daimos-monitorfs-mres-domains-v344` with
`MONITORFS_MRES_ENABLED=1`. Extends the proven v343 no-child
`/PROC/1/STATE` STAT/open/read/S6REC/close checks with
`STAT /MONITOR/DOMAINS`, open the domain directory, one READDIR
(no negative/error return allowed), and close. Native simulator
run **PASS**. This verifies service dispatch and basic domain-root
access; it does **not** yet verify returned entry identities,
child/decimal PID lifecycle, specific domain leaf data, or swapped
process metadata. Testkit commit `9ed13b0`.

No production code changes and no new RAM savings. Opt-in MonitorFS
MRES remains KCORE 9151, KCORE+MRES 15699 (+17 permanent words)
versus default KCORE 10726, KCORE+MRES 15682. Strict 32K acceptance
deferred. Next acceptance should validate specific domain entries
and metadata values, then isolate child process/RUN regressions.

## 2026-10-25 Proposal B native minimal MonitorFS MRES process access v343

Added `daimos-testkit` target `test-daimos-monitorfs-mres-proc-basic-v343`
with a dedicated GCC lowmem, 64K PDP-6 image and
`MONITORFS_MRES_ENABLED=1`. Unlike the failed full process lifecycle
probe v342, the native test does not RUN or create children. It checks
`STAT /PROC`, `STAT /PROC/1/STATE`, OPEN/READ/CLOSE of that state
leaf and a nonempty S6REC text header. Simulator **PASS**. This proves
basic MonitorFS process namespace services are callable through the
relocated MRES; child/RUN/WAIT, decimal PID 10, domain namespace,
and swapped-process argument metadata still require separate tests.

No production source or permanent RAM change. Optional MRES stays
KCORE 9151 and KCORE+MRES 15699 (+17 over default), remains opt-in;
strict 32K acceptance deferred. The native v343 testkit commit is
`3d82596`; unrelated testkit changes were not staged.

## 2026-10-25 Proposal B MonitorFS MRES process acceptance probe v342

Attempted a native process-namespace regression by adapting the
existing full-profile `daimos-monitorfs-process-decimal-v1` fixture
to a 64K GCC lowmem image with `MONITORFS_MRES_ENABLED=1`.
The fixture exercises child RUN/WAIT, two-digit decimal `/PROC/10/STATE`
lookup, and S6REC zombie-state payload. Image build and boot succeeded
and userspace emitted the initial `<` marker; then SIMH halted with
`PC: 000000 (LUUO00 0,300)` before any normal fixture error or final
`P` marker. Thus the test **did not pass**. The defect has not been
isolated to MonitorFS rather than the child/RUN or lowmem assumptions.
The versioned experimental fixture was removed rather than committed
as accepted test coverage. All pre-existing unrelated testkit changes
were preserved. No DAIMOS production change or new RAM saving.

The accepted native opt-in device namespace test v341 continues to
PASS. Further process acceptance should first isolate minimal
`/PROC` path STAT/open/read from the multi-child RUN/WAIT fixture,
then add child lifecycle and swapped metadata cases individually.
MonitorFS remains opt-in, KCORE=9151, KCORE+MRES=15699 (+17 over
v329 default); strict 32K acceptance remains deferred.

## 2026-10-25 Proposal B native MonitorFS MRES devices acceptance v341

Added `daimos-testkit` native PDP-6 target
`test-daimos-monitorfs-mres-device-v341`, running an isolated GCC
lowmem `MONITORFS_MRES_ENABLED=1` image with matching 64K RAM.
The existing full-profile device-view test expected 0377 octal and
initially failed `!5`, as the lowmem profile omits PTR/OCNSLS/DRM.
The new fixture specifies the lowmem expected present set 0153 octal
(CTY0, CLK0, TTY0, DSK0, D6SET0), with absence checks inherited from
the source fixture. Native PDP-6 run **PASS**: MonitorFS device
namespace works through the relocated MRES entrypoints following the
v340 boot-source ordering correction. This is not yet a test of
process/domain leaves, swapped-process record reads, or module unloading.

No kernel production code changed; no new size saving is claimed.
Continue to use the accepted default v329 KCORE 10726 and combined
15682 words; the opt-in MonitorFS MRES is KCORE 9151 and combined
15699 (+17 words). The MRES mode remains opt-in pending broader
runtime acceptance; strict 32K acceptance remains deferred.

## 2026-10-25 MonitorFS MRES package-source ordering boot fix v340

Confirmed that v339's `REVERS` (six-character SIXBIT diagnostic)
actually appeared in the full simulator log, although the earlier
grep for `REVERSE` omitted it. The underlying fault was a mismatch
between MRES source layout and MINIT execution order: lowmem
`MRES_PACKAGE_OBJS` placed `dtfs-mres` **before** `blockset-mres`,
while `module_table.s` runs `blockset_minit` **before** `dtfs_minit`.
`module_run_minits` requires monotonically increasing nonzero package
source addresses to reclaim consumed boot MRES source intervals.

Fixed the lowmem package list in `system/boot/pdp6/Makefile` to link
BLOCKSET before DTFS, matching MINIT. This is the underlying ownership
ordering fix, not a relaxation of MM overlap checks. Fresh isolated
64K GCC lowmem MonitorFS MRES image built and booted in PDP-6 SIMH:
`BSET LOADED`, `D6FS LOADED`, `INIT V1`. Simulator ran until explicit
17-second timeout (exit 124); **no prior post-BSET `kinit_halt`**.
INIT reported `MEMFS NOT MOUNTED ON /TEMP`, not yet investigated.
Native MonitorFS namespace/swapped-process semantic acceptance remains
open, so the optional feature stays disabled by default.

Full linked optional lowmem `permanent-size` unchanged by reordering:
KCORE 021677 (9151), KCORE+MRES 036523 (15699),
PERMANENT_LAST 036602 (15746), +17 words combined versus default v329
(10726 / 15682 / 15729). The +17 cost is provisionally acceptable;
optimization of that overhead follows functional acceptance. Strict
32K gate deferred.

## 2026-10-25 MonitorFS MRES MINIT halt probes v338-v339

Investigated v337's post-BLOCKSET `kinit_halt` with two separate
temporary diagnostic builds, both using matching 64K PDP-6 SIMH
configuration. The first printed `GAPERR` only on failed
`mm_add_free(previous_package, package - previous_package)`; the
second additionally printed `REVERSE` for descending source-package
addresses. Neither marker appeared before the HALT, so neither branch
was conclusively identified as the caller. A failed debug-output
routine or different `kinit_halt` caller has not been excluded.
Both builds still halted after `BSET LOADED`. The instrumentation
was removed entirely; source is unchanged. The previous v337
inference that `mm_add_free` was specifically rejecting an interval
remains **unproven** and must not be used as the basis for an MM fix.
Next: capture the PDP-6 halted CPU's return chain or trace calls to
`kinit_halt` directly, then inspect the implicated MM/initializer
condition. MonitorFS MRES remains opt-in; v333 measured fixed KCORE
9151 words and combined KCORE+MRES 15699 versus default 10726/15682.
No runtime PASS or additional saving claimed; strict 32K deferred.

## 2026-10-25 MonitorFS opt-in MINIT source-reclaim localization v337

Built an isolated 64K lowmem MonitorFS-MRES image with temporary
SIXBIT `NEXT` and `DONE` trace markers around each call in
`module_run_minits` (not retained in production). The PDP-6 simulator
trace shows `BSET LOADED`, then `DONE`, followed by a HALT without the
next `NEXT`. The failure therefore follows successful return from the
BLOCKSET initializer and occurs before the next initializer call.
The intervening branch validates/free-publishes the previous package
source between two successive MRES package addresses. In the
instrumented linker map `blockset_mres_package=075633` and
`d6fs_mres_package=075672` octal, so the expected reclaimed gap is
037 octal words. Since the next `NEXT` marker follows the reclamation
branch and the addresses are increasing, `mm_add_free` rejecting this
source interval is the leading diagnosis; confirm the exact
allocation/arena overlap or MM return code before any fix. `mm_add_free`
explicitly rejects overlapping extents and arenas, and no bypass is
acceptable. The diagnostic KINIT edits were reverted immediately and
no kernel production source was changed. The option remains off by
default; +17 combined permanent words remains provisionally acceptable
if full runtime acceptance is achieved. Strict 32K gate deferred.

## 2026-10-25 MonitorFS MRES post-BSET boot localization v336

Using the existing optional `MONITORFS_MRES_ENABLED=1` image from
v333, compared unmodified boot media under SIMH configured for 64K
and 256K physical memory (versioned simulator INI copies). Both
executions reached the DAIMON KINIT banner and reported
`BSET LOADED`, then halted through `kinit_halt` at octal PC 063116.
This excludes the original 32K/64K configuration mismatch as the
sole reason for the *later* halt; simply increasing simulator RAM does
not resolve it. The module table places `d6fs_minit` after BLOCKSET,
so the next investigation should instrument the exact pre-D6FS/MRES
call boundary, plus early MM reservations, without asserting that D6FS
or MonitorFS caused the halt. A generic `kinit_halt` PC alone cannot
attribute the caller. No production code changed and the default
MonitorFS MRES option remains off. The +17 permanent-word difference
is provisionally acceptable contingent on real boot and service tests;
no additional RAM saving or runtime PASS is claimed.

## 2026-10-25 MonitorFS opt-in boot diagnosis v335

Examined the opt-in 64K MonitorFS-MRES image boot from v334. The
original `?B`, `HALT 000544` was produced with an inconsistent boot
configuration: the lowmem profile's generated `boot.ini` still set
`cpu 32k` while `BOOT_CORE_WORDS=0200000` built a 64K image.
An isolated boot-configuration copy using `set cpu 64k` proceeds past
Stage1 and emits the DAIMON V0.5 / PDP6 / MEM 64K KINIT banner and
MINIT statuses through `BSET LOADED`, but then halts at `kinit_halt`
(PC 063116). Thus there is still a later boot/installation failure;
no functional MonitorFS runtime acceptance is claimed. The boot failure
has not been attributed to the MRES itself. A matching CPU-size option
must accompany larger-image tests; do not weaken stack-reserve checks.

The opt-in MonitorFS MRES link fix from v333 is retained and remains
disabled by default. The proposed +17 combined permanent words is
provisionally acceptable if runtime validation succeeds, but should
be optimized afterward. Memory figures remain KCORE=9151,
KCORE+MRES=15699 (enabled) versus 10726/15682 (default).
The strict 32K gate remains deferred.

## 2026-10-25 MonitorFS MRES build and boot gate v334

The optional MonitorFS MRES full link (v333) saves 1575 KCORE words
but increases KCORE+MRES and PERMANENT_LAST by 17 words. The 17-word
cost is an acceptable provisional tradeoff if native behavior is correct;
seek to recover it only after the runtime acceptance gate, without
adding state or compromising module boundaries.

`BOOT_PROFILE=lowmem MONITORFS_MRES_ENABLED=1` with default 32K
`BOOT_CORE_WORDS` failed `mkimage` because KINIT left less than the
configured stack reserve. Without relaxing that protection, rebuilt
with isolated `BOOT_CORE_WORDS=0200000` (64K) and the image target
completed. A SIMH boot using its generated `boot.ini` then printed
`?B` and halted at PC `000544` (`HALT 544`). Consequently, successful
MRES linking/image creation is *not* successful runtime installation
or MonitorFS namespace acceptance. The failure needs diagnosis before
enabling MRES by default. The default v329 configuration is unchanged.
Strict 32K acceptance stays deferred.

## 2026-10-25 Proposal B MonitorFS MRES missing swap-helper build fix v333

Fixed the opt-in `MONITORFS_MRES_ENABLED=1` package link in
`system/boot/pdp6/Makefile`: compile the existing
`fs/monitorfs_swap_v63.c` with PDP-10 GCC into a distinct MRES object
and link it alongside the generated namespace service object. The
normal non-MRES KCORE build remains unchanged. This resolves the
previous `undefined symbol: mfsproc_swapped_read_words_v63` failure
without dropping functionality, stubbing a syscall, or adding KCORE
resident storage. A fresh isolated GCC lowmem `permanent-size` build
now **links successfully** with MonitorFS MRES enabled.

Measured default v329 versus experimental enabled profile:
KCORE 024746 (10726) -> 021677 (9151), a 1575-word reduction;
MonitorFS MRES adds 003070 (1592) words; KCORE+MRES 036502
(15682) -> 036523 (15699), an increase of 17 permanent words;
PERMANENT_LAST 036561 (15729) -> 036602 (15746), likewise +17.
This is a large **fixed-KCORE size reduction**, but **not a total
permanent occupancy reduction**. The feature stays opt-in, not the
accepted default permanent-RAM optimization baseline.

A successful link does not prove boot, MINIT export patching, pinned
module lifetime, swapped-process reads, or namespace semantics; those
are outstanding native runtime acceptance gates. Existing
`mfsdev_install_minit` already patches nine dispatch entries (contrary
to the earlier assumption that the installer was absent). Do not
mark MonitorFS extraction complete until those gates pass. Strict 32K
acceptance remains deferred.

## 2026-10-25 Proposal B MonitorFS MRES configuration investigation v332

Re-examined the effective build rather than inferring residency from the
KCORE object names. `MONITORFS_MRES_ENABLED` defaults to 0; its normal
KCORE build concatenates `monitorfs_runtime.s` with the compiled
`monitorfs_swap_v63.c` implementation. With the option set to 1,
`system/kernel/Makefile` **instead substitutes**
`monitorfs_dispatch_v14.s` for the resident MonitorFS object, leaving
nine one-word `JRST kret_neg1` service entrypoints. Thus the KCORE
object list alone does not show which implementation actually resides
there. This corrects the earlier interpretation of the Makefile.

Attempted isolated GCC lowmem full-link build with
`MONITORFS_MRES_ENABLED=1` in a distinct TMPDIR build root. KCORE
assembly/link succeeded, but the optional MonitorFS package link failed
with `undefined symbol: mfsproc_swapped_read_words_v63`. The
`monitorfs-mres-probe-v15.py` generator rewrites the full assembler
source, which references that helper; the helper is supplied by
`monitorfs_swap_v63.c` in the normal resident build but was not included
in this package. The generator explicitly says its output is a
**non-installable probe**. `monitorfs_dispatch_v14.s` explicitly says
MINIT must still patch its nine JRST destinations after MRES relocation
and pin the MRES while MonitorFS is reachable. Neither link repair nor
runtime installation was attempted; the enabled configuration is NOT
a working MonitorFS extraction. No complete permanent-size comparison
or native enabled-mode acceptance can be claimed. No source changes
were made. Last accepted standard lowmem baseline remains v329:
KCORE 024746 (10726), KCORE+MRES 036502 (15682), PERMANENT_LAST
036561 (15729), octal values followed by decimal in parentheses.

Next B stage: supply the swapped-reader code/dependencies to the MRES
package properly, verify package relocation/link and resident entries,
implement and test MINIT patching and lifetime/pinning, measure both
profiles, and test MonitorFS device/process/domain namespace on the
PDP-6. Strict 32K acceptance remains deferred.

## 2026-10-25 Proposal B MonitorFS device/namespace audit v331

Inspected the resident `monitorfs_runtime.s` device-name lookup,
READDIR, device-ID validation, special TTYDPY0 name handling,
leaf selection and IO type classification. The already shared
`mfsdev_name_length`, `mfsdev_validate_id`, `mfsleaf_lookup` and
`mfsdev_readdir_store` leaves account for the obvious overlapping
operations. The D6SET0 special leaf count and TTYDPY0 seventh character
must remain exceptional. No safe additional resident reduction was
established: no production code changed, and no RAM saving is claimed.

`daimos-testkit` GCC `test-daimos-monitorfs-devices-online` PASSed on
PDP-6 after its independent boot-image build (roughly 200 seconds).
This is a native MonitorFS device-visibility test, not a comprehensive
process-namespace or multi-process provider test. The attempted v330
UNLINK/RMDIR type-matrix fixture remains unaccepted; no false regression
PASS was recorded. Preserve the last measured v329 GCC lowmem full link:
KCORE 024746 (10726 decimal), KCORE+MRES 036502 (15682 decimal),
PERMANENT_LAST 036561 (15729 decimal). Strict 32K acceptance remains
deferred. Further B source work should target MonitorFS process/domain
namespace lifetime and file-path permission/lookup interactions, with
whole-image measurement before acceptance.

## 2026-10-25 Proposal B remove-type native fixture investigation v330

Attempted a dedicated PDP-6 UNLINK/RMDIR type-matrix fixture by extracting
v328's actual `file_remove` type check and iterating 16 STAT-type / action
cases. The initial fixture had undefined link symbols and incorrect test
expectation/loop instructions; those were corrected, but `p10run`
continued to time out (SIMH status 124) without readable test exit.
No native acceptance is claimed, and the testkit experiment was removed
rather than committing an unverified fixture. Existing unrelated testkit
edits were preserved. No production code changed; retain the last v329
full-image measurement (KCORE 024746, KCORE+MRES 036502,
PERMANENT_LAST 036561 octal). Dedicated native UNLINK/RMDIR semantics
remain unverified. Strict 32K acceptance remains deferred.

## 2026-10-25 Proposal B CREATE/SYMLINK result publication v329

The shared `vfs_create_store_result` epilogue formerly loaded the
caller's output-node pointer from `(17)` into AC4 before storing the
provider-result vnode. PDP-6 indirect addressing can publish the
result directly with `MOVEM 3,@(17)`, eliminating the intermediary
register load. The preceding success check, mount-ID inheritance,
caller-owned pointer and three-word stack unwind remain unchanged;
CREATE/MKDIR/MKFIFO and SYMLINK retain the existing provider ABI.

Fresh full GCC lowmem disk-root linked image compared with v328:
KCORE 024747 (10727 decimal) -> 024746 (10726), KCORE+MRES
036503 (15683) -> 036502 (15682), PERMANENT_LAST 036562 (15730)
-> 036561 (15729). One permanent word saved, no added state.

GCC daimos-testkit `target-file-pdp6`, `target-file-walk-pdp6`,
`test-daimos-vfs-stat-frame`, and `test-daimos-pre-dsh-fs-closure`
PASS. `test-daimos-d6fs-context-dispatch` and
`test-daimos-vfs-storage-reservation` also PASSed in this review.
The existing tests do not separately prove every CREATE/SYMLINK
failure-mode combination. Strict 32K acceptance remains deferred.

## 2026-10-25 Proposal B UNLINK/RMDIR type normalization v328

Consolidated `file_remove`'s duplicated directory and non-directory
STAT result branches. Normalizing `VFS_TYPE_DIR` (1) to a boolean and
comparing it with the existing requested-directory flag preserves UNLINK
(reject directories) and RMDIR (accept directories only), without
changing the parent permission, backing-busy or provider removal paths.
No ABI, new storage or provider dispatch changes.

Full linked GCC lowmem disk-root image compared with accepted v324:
KCORE 024752 (10730 decimal) -> 024747 (10727), KCORE+MRES
036506 (15686) -> 036503 (15683), PERMANENT_LAST 036565 (15733)
-> 036562 (15730). Three permanent words saved, no MRES growth.

GCC daimos-testkit `target-file-pdp6`, `target-file-walk-pdp6`,
`test-daimos-vfs-stat-frame` and `test-daimos-pre-dsh-fs-closure`
all PASS. A dedicated native UNLINK/RMDIR type matrix is still useful
for stronger semantic acceptance. The strict 32K acceptance gate
remains deferred.

## 2026-10-25 Proposal B expanded native provider gateway test v327

Expanded daimos-testkit's v326 PDP-6 gateway oracle into a distinct
`test-daimos-provider-lock-native-v327` target. Native simulator PASS
covers the v326 locked-wait/register-recovery and MEMFS bypass cases,
plus unlocked D6FS and TSFS table dispatch and rejection of provider
IDs outside the 4..7 range. This executes the extracted production
assembly gateway with test-controlled service jumps and wait/wakeup;
it is not an actual competing-process filesystem I/O test. The
multi-process physical-provider contention acceptance remains open.

No production source changed and no new RAM saving is claimed.
Last fully linked GCC lowmem v324 baseline remains KCORE 024752
(10730 decimal), KCORE+MRES 036506 (15686), PERMANENT_LAST
036565 (15733). No strict 32K acceptance work was performed.

## 2026-10-25 Proposal B PDP-6 provider lock native probe v326

Added `daimos-testkit` `test-daimos-provider-lock-native-v326` and ran it
successfully on PDP-6 via `p10run`. The fixture extracts the real
resident provider gateway and uses a controlled wait oracle that initially
holds the lock, releases it through `proc_wait_event`, and deliberately
clobbers request registers before retry. The production save/restore path
returns the original arguments to the provider, preserves its result across
wakeup and releases the lock. The MEMFS path bypasses serialization.
This is native execution of production gateway instructions with oracle
wait/wakeup services, **not** proof of true multi-process scheduler contention
or real physical-provider I/O. No production source changed. The accepted
v324 full linked baseline remains KCORE 024752 (10730 decimal),
KCORE+MRES 036506 (15686 decimal), PERMANENT_LAST 036565 (15733).
Strict 32K acceptance remains deferred.

## 2026-10-25 Proposal B provider lock invariants gate v325

Audited the v324 resident provider dispatch and shared transfer-workspace
lock. The private 7-word request save survives the `proc_wait_event`
loop, the ready lock is acquired before registers are restored, and
provider completion releases the lock and wakes waiters while preserving
the returned AC1. MEMFS still bypasses serialization; DTFS/D6FS/TSFS
continue to serialize over potentially sleeping I/O. No production code
change or memory saving is claimed. The accepted full-linked baseline
remains v324: KCORE 024752 (10730 decimal), KCORE+MRES 036506
(15686 decimal), PERMANENT_LAST 036565 (15733 decimal).

Added `daimos-testkit` target `test-daimos-provider-lock-contract-v325`
(PASS). It enforces assembly source-level register/lock invariants only;
it is not a scheduler contention test. The testkit had unrelated local
edits which were preserved. Further reentrancy changes require actual
multi-process sleep/wakeup and concurrent-provider native acceptance.
Strict 32K acceptance remains deferred.

## 2026-10-25 Proposal B provider table index normalization v324

Normalized the provider number (4 MEMFS through 7 TSFS) to a 0..3
service-table index on entry to `fs_provider_reg_call`. The MEMFS
fast path remains direct and non-sleeping. The other providers retain
the seven-register process-private save, lock wait, dispatch and wakeup;
the index is now saved instead of the original provider number, removing
the redundant subtraction after a lock acquisition. Invalid provider
numbers still return `fs_mres_no_service`. No ABI or persistent storage
changes were made.

Full GCC lowmem disk-root linked image versus accepted v322:
KCORE 024755 (10733) -> 024752 (10730), KCORE+MRES 036511
(15689) -> 036506 (15686), PERMANENT_LAST 036570 (15736) ->
036565 (15733). Saving: three permanent words, unchanged MRES sizes.

GCC daimos-testkit `target-file-pdp6`, `target-file-walk-pdp6`,
`test-daimos-vfs-stat-frame`, and `test-daimos-pre-dsh-fs-closure`
PASS. These tests do not exhaustively exercise concurrent lock waiters;
a targeted stress regression remains useful. Strict 32K acceptance
and further JRST folding remain deferred.

## 2026-10-25 Proposal B SETATTR unwind experiment v323 (reverted)

Re-tested reusing `vfs_mutate3_ro` for `vfs_setattr_ro` in the
current v322 GCC lowmem configuration. Although the source contains
one fewer inline cleanup instruction, the full linked image **grew**:
KCORE 024755 -> 024756 octal (10733 -> 10734 decimal),
KCORE+MRES 036511 -> 036512 octal (15689 -> 15690),
PERMANENT_LAST 036570 -> 036571 octal (15736 -> 15737).
This is a one-word permanent RAM regression. The experiment was
reverted immediately; no semantic acceptance claimed. Retain v322 as
the measured baseline. Continue Proposal B with structural
file-runtime/VFS/provider consolidation, not SETATTR cleanup sharing.
Strict 32K acceptance remains deferred.

## 2026-10-25 Proposal B VFS CREATE mode staging v322

The common CREATE/MKDIR/MKFIFO request staging in `vfs_create_common`
carried the umask-filtered mode through an unnecessary intermediate AC3
assignment before moving it into AC4. Passing the masked value directly
from AC6 into AC4 retains the fifth argument, original call conventions,
provider request order and mode semantics. No new state or dispatch.

Full GCC lowmem disk-root linked image against v321: KCORE 024756
(10734 decimal) to 024755 (10733); KCORE+MRES 036512 (15690) to
036511 (15689); PERMANENT_LAST 036571 (15737) to 036570 (15736).
Savings: one permanent word. Component MRES sizes are unchanged.

`daimos-testkit` GCC `target-file-pdp6`, `target-file-walk-pdp6`,
`test-daimos-vfs-stat-frame`, and `test-daimos-pre-dsh-fs-closure`
all PASS. No dedicated native CREATE/mode matrix was added in this
step. Architectural follow-up should seek meaningful savings in
request frames and provider interactions rather than new dispatch
abstractions. Strict 32K acceptance and JRST folding remain deferred.

## 2026-10-25 Proposal B file lock mount decode v321

Consolidated the identical two-bit mount-extension lookup in the
resident FLOCK conflict and update passes into a single internal leaf
helper, preserving AC4, the public descriptor/mount ABI, lock ownership,
and shared/exclusive/unlock semantics. This is not a new provider dispatch
or permanent storage allocation. The experiment was present as an
uncommitted workspace change before this audit; it was independently
measured against committed v318 source with full GCC lowmem disk-root
links. Baseline: KCORE 024764 (10740 decimal), KCORE+MRES 036520
(15696), PERMANENT_LAST 036577 (15743). Accepted candidate: KCORE
024756 (10734), KCORE+MRES 036512 (15690), PERMANENT_LAST 036571
(15737). Net saving: 6 permanent words; MRES component sizes unchanged.

GCC daimos-testkit target-file-pdp6, target-file-walk-pdp6,
test-daimos-vfs-stat-frame and test-daimos-pre-dsh-fs-closure all PASS.
The file-runtime native fixture exercises shared/exclusive locking,
conflict, unlock and descriptor duplicate behavior. An additional
explicit cross-mount high-extension collision scenario is still a
valuable follow-up; do not claim that narrow scenario was separately
tested here. The 32K acceptance gate and JRST folding remain deferred.

## 2026-10-25 v318 EXEC RT cleanup native acceptance v320

The previously linked and measured v318 two-word EXEC-loader cleanup
is now native regression accepted for the covered failure and ownership
paths. `make COMPILER=gcc test-daimos-exec-rt-owner` in `daimos-testkit`
PASSed with `DAIMOS_EXEC_OBSERVE_SECONDS=120`, verifying RT rollback
at header and post-VM decoder failure, existing-owner preservation,
RT admission and release. The default 15-second run timed out at SIMH
boot without reaching assertions; the extended run completed.
The separate `test-daimos-exec-create-oom` PASS is recorded at v319.

Accepted comparative lowmem KCORE size: v316 `024766` to v318
`024764` octal, saving two words post-fold, with no additional
persistent data. This does not constitute strict 32K acceptance.
The unrelated working-tree file-runtime experiment is not included.

## 2026-10-25 EXEC allocation-failure native retest v319

`make -C ../daimos-testkit COMPILER=gcc test-daimos-exec-create-oom`
PASSed against the current v318 production EXEC loader. The independent
PDP-6 test image printed `__kcore_low_end=025036` and
`daimos-exec-create-oom-v205: PASS (staged VM cannot fit physically;
low-memory EXEC replaced PID 1)`. This is positive native evidence for
the low-memory EXEC retry after a staged allocation failure. The test
image has a different build configuration; its endpoint must **not**
be compared with the v318 `024764` lowmem permanent-size image.

The native `test-daimos-exec-rt-owner` timeout described under v318
remains unresolved; this retest alone does not establish full v318 RT
release semantics. No production change or new permanent-size gain was
made here. An unrelated uncommitted `file_runtime.s` experiment in the
workspace was left untouched.

## 2026-10-25 EXEC loader RT cleanup fold v318

`exec_load.s` now shares the post-lookup RT-reservation release sequence
for ordinary loader failure (`-1`) and VM allocation failure (`-2`).
The early argument/lookup failure still bypasses RT cleanup and returns
`-1`. This avoids changing reservation ownership or error semantics.
The PDP-6 boot Makefile's KERNEL_INPUTS was also missing the included
`exec_load.s` source: add it to force recompilation when that source
changes. A first build without this fix was stale and is not evidence.

After correcting the dependency, GCC lowmem `permanent-size` PASSed:
KCORE `024764` octal (10740 words), KCORE+MRES `036520` (15696),
PERMANENT_LAST `036577` (15743). This is a **two-word post-fold
reduction** from the comparable v316 KCORE `024766` build. The
targeted native `test-daimos-exec-rt-owner` assembled and launched its
256K PDP-6 SIMH guest, but timed out during boot before assertions
(`simulator failed: 124`); `test-daimos-exec-create-oom` did not run
because Make stopped on that failure. Therefore **runtime acceptance is
pending**, despite successful assembly and size measurement. Rerun
native failure-path testing before treating the change as complete.

## 2026-10-25 EXEC/RUN linked-fold audit v317

Inspected the GCC lowmem linked map and `kcore.fold` after the v316
fresh build. Resident entries appear at `exec_replace_current=000130`,
`exec_replace_current_lowmem=000407`, `sixbit_record_words=000540`,
and `proc_run_block=020332` (octal). The fold-plan text has no entries
for either `proc_run.dobj` or `exec_lowmem_pdp6.dobj`: there is no
existing cross-block folding in these two units to harvest directly.

Checked both startup-record loops against the decoder contract.
Their pre-call address test protects the count-word read, and their
post-call address test protects payload extent. The count-zero path
is necessary for argc/envc=0. Preserve these checks, even though
the loops superficially resemble each other. No safe positive-size
candidate was verified, so this audit makes **no production change**.

Retain the v316 `024766` GCC lowmem size measurement as baseline.
Next investigate executable-loader and startup helpers for proved
redundancy, with exact before/after post-fold and native regression
tests; do not infer code savings from source-level similarity.

## 2026-10-25 EXEC/RUN measurement checkpoint v316

Fresh `make -C system/boot/pdp6 BOOT_PROFILE=lowmem COMPILER=gcc
permanent-size` PASSed on the current checkout without production edits.
Its report gives KCORE `024766` octal (10742 words), KCORE+MRES
`036522` (15698), PERMANENT_LAST `036601` (15745), and the single-root
limit `040000` with 638 free words. This is 101 words below the older
documented `025133` lowmem number, but profile/build inputs and tool
versions have not yet been demonstrated equivalent; **do not attribute
the difference to EXEC/RUN optimization**.

The scanned EXEC/RUN record loops both require their input-range checks:
`sixbit_record_words` reads the pointed-to count word, so removing the
pre-call `pointer < end` check permits an out-of-range read even though
the post-call span check remains. Likewise the post-call span bound is
necessary to reject a record claiming data beyond its mapped block.
Do not remove either check as a size shortcut. RUN's descriptors,
hierarchy and publication require distinct cleanup ordering from EXEC.

Next: compare linked instruction sequences and call graphs for proven
intra-path redundancies; use this exact build command as the baseline
for before/after size tests. No code change, runtime retest or new
size-saving claim follows from this measurement alone.

## 2026-10-25 resident EXEC/RUN sharing audit v315

Source-only audit of `exec_lowmem_pdp6.s`, `proc_run.s`, and
`exec_load.s`; no code or native acceptance change. Both entry points
already call `sixbit_record_words` for bounded counted-SIXBIT records
and `exec_load_process` for DXR loading. The remaining record scans
use different persistent registers and failure targets; a separate
shared loop would add call/return and register adaptation, so no
positive post-fold saving has been demonstrated. Do not introduce it
without a measured whole-image reduction.

EXEC replacement stages a VM, creates startup data, preserves the old
swap/RT/extension state on failure and only then commits. Its low-memory
retry deliberately destroys the old VM before loading again and has a
distinct fatal-after-commit result. RUN instead claims a child slot,
creates a new u-area, inherits hierarchy and descriptors, then publishes
the runnable child; all post-claim errors use `proc_slot_discard`.
Merging these transaction tails would risk ownership and rollback
semantics and is not recommended.

Next optimization unit: count individual validation/check sequences
against actual linked object and p10fold output; prioritize proven
redundant work within a single path over generic cross-path helpers.
Preserve the deferred strict-32K policy. The existing comparable lowmem
size reference remains `025133` octal; no fresh measurement was made.

## 2026-10-25 verified whole-core swap-out return snapshots v314

The PDP-6 SIMH debugger stops at the successor instruction when a
breakpoint is requested on an instruction. The v313 attempt stopped
at reclaim's `PUSHJ proc_swap_out` address 003266 and its 003267 return,
producing two identical 256K dumps because the call had already
returned at the first snapshot. Its zero-difference output is an
**invalid before/after transaction comparison**, not proof of no change.

v314 fixes the pre-call stop to 003265 (SIMH displays the following
`PUSHJ 17,2623`) while retaining 003267 for the post-return stop.
The DXR call, prior branch and return instructions are verified before
SIMH runs. Captures contained all 262144 PDP-6 physical words apiece,
identical address coverage, and **75 changed words**: 12 below KCORE,
9 in permanent KCORE code/data range, 2 MM descriptors, and 52 beyond
the descriptor table. `mm_extent_count` changed 031 -> 030 octal;
there were zero unmatched addresses. This is a genuine pre-call to
post-return memory difference; expected kernel stack, MM extent and
swap metadata updates are present. Not all 75 changes have been
semantically proven correct, so it is **not a global non-corruption
proof**. The 20-child native swap guest remains PASS under v311.
The testkit fixture is `daimos-mm-swap-snapshot-v314.sh`, using the
v312 splitter/comparator. No production modification or KCORE growth.
With swap correctness restored, return to EXEC/RUN permanent kernel
minimization; additional memory investigations are optional unless a
new regression appears.

## 2026-10-25 native full-core swap snapshots v312

The testkit captured both 256K-word (262144-word) PDP-6 physical
RAM images for a swap-out, from `proc_swap_out` entry to
`proc_storage_publish_v30` entry (the latter is **pre-publication**, not
post-commit). The two complete, equally addressed dumps differ in
82 words: 20 below KCORE, nine in permanent code/data below the MM
extent table, two MM descriptor words, and 51 words above it.
`mm_extent_count` moved from `031` to `030` octal, consistent with an
extent release, and the swap record/accounting changed as expected.
This is a reproducible memory comparison tool, **not a global
non-corruption proof**, an after-return snapshot, or another guest
acceptance run. In particular, some AC/low-memory words naturally
change and must not be called corruption. The fixed 20-child v311
acceptance run still passes, so no speculative kernel fix is needed.
The testkit scripts `daimos-mm-swap-snapshot-v312.sh`,
`daimos-mm-swap-snapshot-split-v312.py`, and
`daimos-mm-swap-snapshot-compare-v312.py` capture, extract, and
classify the images without consuming guest RAM. The comparator
requires identical full 256K address coverage. More granular
semantic invariants and optional post-return capture can be pursued
separately if another swap regression warrants them.

### Post-INIT swap pressure acceptance v311

The native 256K PDP-6 GCC 20-pressure-child single-cycle fixture
passed with 012000 octal (5120) drum backing blocks, guest completion
marker `Q` and normal HALT. The 16-child fixture already passed with
010000 octal (4096) backing blocks. This resolves the immediate swap
correctness investigation following the real failed-swap-in local-stack
pop repair `3b41e87`; it does not validate smaller backing or all
possible 64-slot pressure workloads. No KCORE or descriptor-size
increase. Return to resident EXEC/RUN minimization and low-RAM
correctness analysis; 32K strict acceptance remains deferred.

## 2026-10-25 v311 twenty-child native swap acceptance — RUNNING / NOT ACCEPTED

The production kernel remains unchanged after the successful v310
16-child 256K PDP-6 GCC full-profile one-cycle acceptance using
`DRUM_BACKSTORE_BLOCKS=010000` (4096 decimal blocks) and a 900-second
SIMH-only ceiling. v311 extends the existing instrumented guest from
16 to 20 pressure children, retaining writer pipe integrity, swapped
writer restore, child wait/reap, and guest completion checks. The
original requested `020000`-octal backstore could not fit the single
`020000`-sector drum with logstore and cachestore and failed at
`mkdsk` before SIMH. v311 was corrected to `012000` octal backing
(5120 decimal blocks); the drum image built and `d6fsck` passed.

The no-breakpoint simulator run has so far emitted
`<wP0P1P2P3P4P5P6P7P8P9PAPBPCPDPEPFPGPHPIPJSR`:
all 20 children were admitted (`P0`..`PJ`), reached sleep (`S`),
and the parent reached its first writer-pipe read (`R`). At this
checkpoint the simulator process is **still running**; no `Q`, guest
HALT, or shell PASS result has been observed. This is *not* 20-child
acceptance, nor proof of a deadlock. The fixture has a 900-second
SIMH ceiling and retains its work directory for post-run inspection.
The two testkit commits are `f775ae4` (fixture) and `96d8711`
(drum geometry correction). Do not alter production swap policy,
KCORE size or descriptor capacity on this incomplete evidence.

Previous v307 snapshot analysis found expected live stack changes,
not a demonstrated memory overlap. v294 measured 28 of 64 idle-stack
words at the nested swap-out entry; v293 doubling the idle stack did
not remove the old UUO. Later `3b41e87` fixed a genuine swap-in
failure-path stack pop error, and v310 passed with sufficient backing.
Further tests belong only in daimos-testkit.

## 2026-10-25 extended 16-child swap acceptance v310 — PASS

The earlier 360-second high-capacity 16-child tests (v304/v305/v308/
v309) ended in host timeout, even though v309 reached marker `R`
just before the parent's first writer-pipe read. The unchanged v309
instrumented guest was rerun with `DRUM_BACKSTORE_BLOCKS=010000`
(4096 octal blocks), 256K PDP-6 full GCC image, no SIMH debugger
breakpoints and a **900-second simulator timeout**. Versioned testkit
fixture `daimos-mm-swap-progress-v310.sh` passed its one-cycle guest
completion checks (`<`, `w`, `Q`, guest HALT): writer pipe payload
integrity, sixteen pressure children, wait/reap and cleanup succeeded.
The full command including image build took roughly 569 seconds;
that duration is **not** the simulator-only elapsed time.

The v309 `proc_wait_event` snapshot was thus insufficient to conclude
a deadlock: under this configuration the program completed with a
longer execution budget. Original 2048-block backing remains a
separate capacity-exhaustion case and is not accepted at 16 children.
The 12-child current-kernel passing baseline remains valid.
No production code change, linked KCORE growth or descriptor-capacity
increase. Do not add debugger-heavy traces to this acceptance fixture.

## 2026-10-25 expanded swap writer-pipe blocking boundary v309

An additional one-byte progress marker `R` was emitted by the native
16-child 4096-block backing-store guest only after `dsys_close(gate_r)`
and `target_is_swapped()` both succeeded, immediately before the
first writer-pipe `dsys_read_words` call. The SIMH run printed
`<wP0P1P2P3P4P5P6P7P8P9PAPBPCPDPEPFSR` then timed out at kernel
`proc_wait_event` PC `017440` after the configured 360 seconds.
All 16 pressure children were admitted and asleep. The parent reached
the first pipe read but never returned from it. This strongly
localizes the high-capacity failure to the writer process resuming,
pipe output availability/wakeup, or related scheduling, not to
pressure-child admission or the prior swap-state guard. The marker
does not itself prove whether the writer ran or attempted to write.
Next inspect the writer's PROC_SLEEP reason, SWAP residency and
run-queue state at the blocked pipe read without high-volume SIMH
breakpoints; compare with 12-child passing trace. No production code
change or KCORE growth.

## 2026-10-25 expanded backstore 16-child progress localization v308

The unmodified 256K PDP-6 kernel, with `DRUM_BACKSTORE_BLOCKS=010000`
(octal, 4096 blocks), ran a lightly marked v308 guest (no SIMH
breakpoints). Its guest output was `<wP0P1P2P3P4P5P6P7P8P9PAPBPCPDPEPFS`.
Each `P` index proves a pressure child was admitted; `S` is printed
only after all sixteen reached PROC_SLEEP. The simulator nonetheless
timed out after 360 seconds at `proc_wait_event` PC `017440` without
`Q`, `a` or `s` error markers. Thus the long-run stall is **after**
all sixteen children sleep, in the `dsys_close(gate_r)` /
`target_is_swapped` check or first writer-pipe read before its
return. It is NOT an admission bottleneck. A v309 check should place
low-cost markers around this boundary, avoiding MM breakpoints.
No production code change or KCORE growth.

### Physical fault snapshots v307

The retained v297 snapshot compared 317 memory words, changing 27 of
64 sampled idle-stack words, 1 of 83 sampled process/MM metadata words,
and 0 of 4 sampled storage-map words. Both idle-stack base pointers
remain `040350` octal. Execution continued between captures, so these
changes cannot by themselves establish illegal writes or a stack
overflow. The v307 testkit report classifies physical changes without
assuming that a changed stack word is a corrupted word. No KCORE
growth or production change.

## 2026-10-25 current-kernel twelve-child regression v306 — PASS

After the production swap-in allocation-failure local-pop correction
(commit `3b41e87`), the original 256K PDP-6 GCC v281 twelve-child
post-INIT guest was rebuilt and rerun unchanged. Native SIMH reported
`PASS (one post-INIT 12-child pressure cycle)`, with allocator
insertion peak 31/136 across 71 samples. This reconfirms writer
restoration, pipe data integrity and child completion at 12 children
under the current code. It does **not** prove the 16-child case,
which still fails due to swap-space exhaustion and an observed writer
TERM event, or resolve the 4096-block v305 timeout. No production
source or KCORE-size change in this regression.

## 2026-10-25 expanded swap capacity experiment v305 — FAIL/timeout

A native 256K full-profile PDP-6 GCC 16-child acceptance attempt
with `DRUM_BACKSTORE_BLOCKS=010000` (octal; 4096 blocks), doubled
from the default `04000` (2048), used the v302 instrumented guest
without SIMH debugger breakpoints. The guest emitted `<w` but never
reached `Q`; the simulator stopped at its 360-second timeout
(`SIGTERM`, PC `017441` in the wait path). The run did **not** show
the previous unexpected UUO, but did not satisfy writer completion
or child reaping. The larger backing area therefore does not yet
establish 16-child acceptance or settle whether the failure is
capacity-only. A concurrent separate v305 run may also exist;
this result belongs to the run preserved as
`$TMPDIR/daimos-mm-swap-capacity-v305.log`.
Next distinguish slow forward progress from blocked writer / scheduler
wait with a lower-overhead trace, and retain correct behavior under
insufficient backing capacity. No production change or KCORE growth.

## 2026-10-25 writer termination event confirmation v303

A diagnostic SIMH run of the 16-child v302 guest placed one breakpoint
at `proc_event_apply` (linked address `015370`). At the breakpoint,
AC1=2, AC2=1 (`SYS_EVENT_TERM`), and `proc_sched_cursor=2`.
The guest emitted `<w!2101c!W` and halted, matching the earlier
v302 observation of writer PID low digit 2, EXITED kind 1, exit status
001 octal instead of 050. This establishes a kernel TERM event for
slot 2 in the failing run, rather than a normal writer exit.
The trace does not yet establish the event call's return address nor
that every exhausted swap-in should be terminated. Previously
captured swap backing space was insufficient for the tenth allocation;
the correct treatment of a temporarily nonresident process requires
investigation before changing scheduler semantics.
The event-only breakpoint took substantially longer than a plain
guest run. No production source or permanent memory change.

## 2026-10-25 writer WAIT exit status capture v302

The versioned 16-child guest v302 instrumented `wait_report()` with
`!` plus low PID octal digit, status kind, and 2-digit octal status.
The unmodified native PDP-6 GCC 256K kernel ran to guest HALT,
producing `<w!2101c!W`, **not** the former unexpected UUO.
The writer wait returned a PID with low three bits 2, EXITED kind 1,
and **exit status 001 octal** rather than the expected writer exit
status 050 octal. The writer source explicitly exits with 041, 042
or 050 only; this implicates kernel-side process termination.
`proc_swap_service_failed` on an unsuccessful restore invokes
`proc_event_apply(...,SYS_EVENT_TERM)`, a fatal event; termination
status 001 is consistent with this path, although no direct event
breakpoint has yet established the causal call for this writer.

Earlier v297 established 2048 swap blocks total, 1872 used, and a
208-block request with only 176 remaining. The 16-child case thus
probes real backing-store exhaustion, not simply MM descriptor or
storage map overlap. Further work must distinguish required graceful
capacity failure from incorrect scheduler termination and evaluate
whether the acceptance test should use an adequately sized swap tail.
Do not claim a 16-child PASS or change swap policy without an observed
failure path. Production code unchanged in v302.

## 2026-10-25 lightweight writer outcome isolation v300/v301

The 16-child 256K PDP-6 fixture used a versioned guest (v300) to
replace the compound `d` writer-failure marker with distinct markers:
`a` first pipe read, `b` first word mismatch, `c` writer WAIT exit
status mismatch, `d` pipe drain, `e` pipe close. A verbose v300 SIMH
breakpoint run timed out before any of these guest markers, so its
result was inconclusive. Version v301 removed the high-volume MM
breakpoints and ran the same guest with a 360-second SIMH timeout.
It completed with `<wc!W` and HALT, not the old unexpected UUO.
The first pipe read returned one word with the expected `A` value;
`wait_report(writer, SYS_WAIT_EXITED, WRITER_STATUS)` failed. The
actual wait PID, status kind and value remain unmeasured. It is not
established whether the writer was terminated on swap-in failure,
returned the wrong status, or produced some other wait result.
This run is still a FAIL, not 16-child swap acceptance. Next obtain
exact wait result and writer state, then repair the verified error
path, preserving correct backing-store exhaustion behavior.
No production kernel modifications in v300/v301.

## 2026-10-25 swap-in failed allocation return correction v299 test — partial

After commit `3b41e87` corrected the missing one-word stack-local
pop on the `proc_swap_in_alloc_fail` path, the existing native 256K
PDP-6 GCC 16-pressure-child v298 diagnostic was rerun. The original
`UUO 000000777777 340000000003` was **not** seen. Instead the
fixture emitted `d!W`: the writer output/read or wait verification
failed after all pressure children had been created. SIMH then ran
until its 300-second timeout (`SIGTERM`, PC `017441`, within
`proc_wait_event` boot wait), so the test is still **FAIL**, not an
acceptance run. `d` comes from the fixture's pipe/read/wait/drain
validation, not the swap allocator directly. The existing trace
shows recurrent `proc_swap_in` failure-return handling and the old
control-flow UUO is no longer reproduced under this workload.

The validated defect is the missing local pop. Do not interpret the
absence of a UUO in one run as complete repair: next identify which
writer outcome caused `d`, distinguish legitimate swap-capacity
failure from bad process termination semantics, and rerun the 12-child
PASS fixture to detect regressions. The old 16-child case cannot be
accepted until pipe integrity and complete child reaping succeed.
No additional production changes in v299.

## 2026-10-25 nested swap and backing-store exhaustion v294-v297

v294 captured the actual idle stack base `040350` and AC17
`000034040404` octal at the failing nested swap-out, i.e. 28
pushed words of the configured 0100-octal (64-word) idle stack.
At v295, execution reached the nested `proc_swap_out` call to
`proc_swap_skip_words` and the subsequent `mm_pin` check, with AC17
`000043040413` (35 pushed words). v296 traced the later
`backstore_alloc` return of -1 with AC17 `000045040415`
(37 pushed words). These snapshots do **not** demonstrate an idle
stack overflow at the sampled points; future deeper use is not
excluded. The separate 128-word experiment in v293 still failed.

v297 directly confirms backing-store exhaustion: total
`backstore_blocks=04000` octal (2048), in-use
`backstore_blocks_used=03520` octal (1872),
`backstore_enabled=1`, and new allocation AC1=`0320` octal
(208). Only `0260` octal (176) blocks remain. Nine prior 208-block
swap writes used 1872 blocks; the tenth allocation correctly returns
-1. This is a capacity refusal, not a detected overlapping MM
extent or a proven stack overflow. The subsequent unexpected UUO
`000000777777 340000000003` remains a **kernel error-path defect**
to diagnose. In particular, trace the propagation of the allocation
error from `proc_swap_out` through `proc_swap_reclaim`,
`mm_alloc_aligned` and `proc_swap_in` into `proc_swap_service_one`.
Do not increase KCORE, the idle stack or backing-store capacity as
a substitute for fixing error handling. The 12-child acceptance
fixture remains valid, the 16-child fixture still fails.

## 2026-10-25 v296/v297 physical pre/post swap-out snapshots

Native 256K PDP-6 full-image, sixteen-pressure-child SIMH breakpoints
captured 9 successful swap-outs as matching pre-entry and pre-storage-
publication snapshots (v296). A tenth swap-out entered but never
reached publication before `UUO 000000777777 340000000003`.
The v297 repeat added checkpoints at `mm_pin`, `backstore_alloc` and
a final dump after the UUO halt. On the failing tenth swap-out,
`mach_kernel_stack_base=040350` octal. AC17 at `proc_swap_out` entry
was `000034040404`, then `000044040414` at `mm_pin`, but at
`backstore_alloc` it was `000001040351`, nearly reset to the idle
stack base. The after-HALT dump again had AC17=`000001040351`.
Comparing the tenth swap-out entry with the post-HALT dump across
captured memory found 28 changed words in the inspected idle-stack
region, one process-table word and one MM extent-descriptor word.
The first packed storage-state word at `041141` remained
`000005252520` octal. These differences span continued execution
and an asynchronous fault, so they do not on their own prove illegal
memory writes. The abrupt stack-pointer change is strong evidence to
inspect PI6/PI7 handler and scheduler stack reset/return handling;
there is not yet proof of the specific mechanism.

Diagnostic files: daimos-testkit `daimos-mm-swap-snapshot-v295.sh`
(reuses boot, but a previously run disk image stopped at `?RT`),
`daimos-mm-swap-snapshot-v296.sh` and
`daimos-mm-swap-fault-snapshot-v297.sh` (fresh image), plus
`daimos-mm-swap-snapshot-diff-v297.py`. No production code change.

## 2026-10-25 nested allocator stack trace v293 — unresolved

The unchanged 16-pressure-child PDP-6 guest was traced through
`mm_alloc_aligned`, `mm_compact`, `proc_swap_reclaim`,
`proc_swap_victim`, `proc_swap_out`, and `proc_swap_in` with AC17.
At the first swap-in for slot 2, `mm_alloc_aligned` entered reclaim,
selected resident slot 1, and entered a nested `proc_swap_out` before
`UUO 000000777777 340000000003`. The trace captured AC17 as
`000034040404` octal at that swap-out entry. The configured idle stack
was 0100 octal (64 words), making stack overrun a hypothesis but not
proof: the full frame depth and actual allocated stack boundary were
not verified from that trace.

A controlled source experiment doubled the configured KINIT and
scheduler idle-stack size from 0100 to 0200 octal words. The same
16-child test still failed with the same UUO and the same captured
AC17. The two source changes were restored. This does not demonstrate
that expanding the idle stack fixed anything, and the unchanged SP
merits independent verification of stack selection/boot initialization.
Next check actual `mach_kernel_stack_base`, its owner-2 MM extent and
saved scheduler stack pointer under SIMH, and test stack bounds rather
than increasing permanent RAM or guessing at a larger stack.
No production code was retained.

### Swap metadata extent overlap inspection v292

At the 16-child failing guest's UUO aftermath, the two-bit storage
map at `041141` octal is within kernel-dynamic owner-4 extent
`[040750,041145)`. Captured neighboring MM extents do not overlap.
Slot 1 is RESIDENT, slot 2 DISK in its first map word
`000005252520` octal. Earlier slot-2 mode and block count validation
succeeded. Post-UUO checks that failed are not evidence of the
initial cause. Full descriptor-table overlap validation and nested
allocator/stack instrumentation remain open. No KCORE size change.

## 2026-10-25 first swap-read return probe v287 — inconclusive

Three iterations of the existing 16-child reproducer attempted
breakpoints around the linked `proc_swap_in` read return and rollback.
The base object-file disassembly exposes instructions after
`PUSHJ backstore_read`, but the inferred relocated breakpoints did
not trigger; early iterations also contained incorrect octal
address arithmetic. The native trace still reproduced the UUO.
A separate direct-image probe attempted to examine the linked code
but halted early at `?RT` and supplied no usable instruction evidence.
Therefore the read return code and the exact rollback cause remain
**unmeasured**. Do not conclude a read failure or a specific code
corruption mechanism from this probe. Next obtain authoritative final
linked disassembly or a simulator return-stack/trace snapshot and
verify each breakpoint address before launching a costly reproducer.
No production code change or KCORE growth.

## 2026-10-25 first swap-in rollback trace v286

The unmodified 16-pressure-child PDP-6 GCC reproducer again emitted
`UUO 000000777777 340000000003` and halted at PC 000001. A new
SIMH-only breakpoint fixture traced storage-state checks and
publication, backstore reads/writes, unpin, and MM-free operations.
The run recorded **one** `proc_swap_in` entry, **one**
`backstore_read` entry, **no** storage-publication entry associated
with that swap-in, and an `mm_free` entry before the subsequent
`proc_swap_out`. This is consistent with the first swap-in falling
into its rollback path; the exact failing return value was not
sampled. Over the entire run there were 10 swap-out entries,
9 backstore-write entries, 9 publication entries, 19 MM-free
entries and 10 unpin entries. Following the first swap-in rollback,
`proc_swap_out` was entered again, then the unexpected UUO appeared.
The 16-child workload remains a **FAIL**. No kernel source was
modified, no instruction-corruption root cause is claimed and
KCORE size/descriptor policy remain unchanged.
Next: sample the return from `backstore_read`, distinguish read
failure from unpin failure, and check saved process state after
failed restore.

## 2026-10-25 native swap-transition event trace v285

The unchanged 16-pressure-child test reproduced the unexpected UUO.
A new SIMH breakpoint fixture sampled `proc_current_slot`,
`proc_sched_cursor`, and AC1 on each `proc_swap_out`, `proc_swap_in`,
`proc_swap_service_one`, `backstore_read`, `backstore_write` entry,
without changing KCORE. The execution logged 10 `proc_swap_out`
entries, one `proc_swap_service_one`, one `proc_swap_in`, one
`backstore_read` and 9 `backstore_write` entries. The final sequence
was swap service request `proc_sched_cursor=0402` (octal),
`proc_swap_in` AC1=2 in slot-0 context, then `proc_swap_out`
AC1=1 in slot-0 context, followed by the same unexpected
`UUO 000000777777 340000000003` and PC 000001 HALT.
These are entry observations, not verified successful transitions.
The process-control corruption origin is still unresolved. Prioritize
return-path/instruction-state tracing of the first swap-in and the
following swap-out; do not increase process or descriptor limits.
No production code or permanent memory change.

## 2026-10-25 v284 unexpected UUO simulator breakpoint diagnostic

The unchanged v283 16-child guest was rerun under a new testkit
SIMH script with a memory-insert breakpoint and an additional PC=000001
breakpoint intended to examine `proc_current_slot`,
`proc_sched_cursor`, `mm_extent_count`, and selected ACs. The
same abnormal terminal output recurred:
`UUO 000000777777 340000000003` followed by
`HALT instruction, PC: 000001`. The new PC=1 breakpoint did **not**
fire, so no process/AC snapshot was captured. The fault remains
unresolved. In particular, PC 000001 in the terminal line must not
be interpreted as a successfully executed instruction at that PC.
No production change or swap high-water acceptance follows from v284.
Further debugging requires an emulator-side trap/event trace or an
earlier breakpoint on the actual UUO entry path, rather than relying
on post-fault PC=1 execution.

### Post-INIT stress escalation v282/v283: unexpected UUO unresolved

v282 (20 pressure children) fails creation at the 17th requested
child and exceeds simulator time. v283 (16 children) instead reaches
an unexpected UUO and halts at PC 000001 before normal guest markers.
SIMH descriptor insert observations for both runs were below 36/136;
neither run passed. A native trap/control-flow investigation now takes
precedence over increasing process counts or descriptor capacity.
No production changes or KCORE size growth.

### Post-INIT twelve-child swap/high-water acceptance v281

The 256K native PDP-6 test launches twelve 24K-word pressure children
after enabling swap and verifies a blocked writer resumes with intact
pipe data. One complete cycle PASS; 71 MM insert-entry samples give
peak descriptor occupancy 31/136. A three-cycle variant exceeded its
300-second simulator timeout during the second cycle and was not
counted as PASS. Full 63-slot concurrent swap remains unverified.
No KCORE source or size change.

## 2026-10-25 256K preload-pressure admission probe v280

Test-only PDP-6 GCC user image enlarged each guest with a private
`010000`-octal (4096-word) BSS array, checked at its first and last
word. The original 63-process preload and a reduced 48-process preload
both built valid boot images but halted without any guest start marker.
Consequently the test never reached runtime swap/reclaim; no high-water
value or allocator defect is established. This is a failed diagnostic,
not a passing or high-water acceptance fixture. The v265 63-process
small-image concurrency PASS and v278/v279 independently exercised
runtime swap remain the supported results.

The next high-concurrency swap fixture should create or enlarge user
memory after INIT has activated swap, not preload 48-63 large images
before the boot handoff. All simulator core settings remain in 16K-word
increments, the DAIMOS process policy in 8K increments.
No production changes or KCORE size increase.

### Three-cycle 256K swap descriptor high-water v279

Native PDP-6 SIMH `mm_extent_insert` breakpoints sampled 101
pre-insertion descriptor counts across three passing pressure
cycles. Maximum resulting descriptor occupancy was 27/136;
109 entries remained unused in that particular test. This is
distinct from the 63-process boot peak of 132/136; combined
63-process swapping has not been measured. No KCORE change.

### Repeated native swap/reclaim and process reuse v278

The 256K PDP-6 GCC full-profile test completed three successive
pressure rounds (blocked writer, eight pressure sleepers, swap
recovery, pipe data checks and child reaping). Testkit v278 PASS.
No permanent KCORE change. Descriptor high-water tracing during
these cycles and longer repeated evictions remain open; 32K GCC
lowmem disk-root is still the size measurement baseline.

### Correct AUXSTORE/root boot ordering and live swap recovery v277

`auxstore_post_minits()` now publishes dedicated backing-store mapping
after `kinit_boot()` mounts D6FS. Previously root mounting could
overwrite the shared direct-tail mapping, leaving backing allocation
capacity inconsistent with the active I/O span. With the corrected
order the 256K PDP-6 eight-child pressure fixture completes, including
blocked-writer restoration and pipe data checks. The standard 32K GCC
lowmem disk-root size run reports unchanged KCORE 10742 and
KCORE+MRES 13605 words; PERMANENT_LAST is 13652.

## 2026-10-25 live backing-tail measurements v276

Corrected test-only SIMH breakpoint examination at `backstore_write`
entry on the 256K PDP-6 pressure failure reads these **octal**
values: `backstore_blocks=04000` (2048),
`backstore_blocks_used=0320` (208), and
`blockset_direct_tail=02000` (1024). The allocator's advertised
backing extent exceeds the active direct-tail span. A first
experimental reading before boot incorrectly showed zero and was
replaced by readings within the execution breakpoint. The guest
still fails at child seven (`p7!W`) after swap-out rollback.
This is a verified configuration inconsistency; the test did not
capture the failing write's exact logical range or return code,
so the inconsistency is not yet proved to cause this particular
failure. Next trace configuration ownership and the write return
status; do not change production memory limits as a workaround.
32K GCC lowmem disk-root stays the KCORE size reference.

## 2026-10-25 backing-store rollback trace v275

On the fresh 256K PDP-6 eighth-pressure-child reproducer,
SIMH execution breakpoints recorded one call each to
`backstore_write`, `mm_unpin`, and `backstore_free`, with no
call to `proc_storage_publish_v30`. The guest still emitted
`<wp7!W`. This matches the `proc_swap_out_unpin` rollback
branch following a failed `backstore_write`, but the return
register after the write was not sampled, so the precise
failure remains to be proved. The absence of storage-state
publication confirms that this attempt did not complete the
normal swap-out commit sequence. The fixture remains a failing
diagnostic, not a swap acceptance test. Next inspect the native
backstore write return code and the blockset/direct-tail I/O
mapping under the tested 256K image. No production code or 32K
KCORE size-reference changes.

## 2026-10-25 256K pressure admission native traces v273/v274

SIMH breakpoints on the unchanged native PDP-6 guest captured one
`proc_swap_reclaim` entry, one `proc_swap_victim` entry, one
`proc_swap_out` entry, one `backstore_alloc` entry and one
`backstore_write` entry before the `p7` admission failure.
The deeper trace also observed 11 storage-state checks and
21 MM-free entries over the boot. Counts are entry observations,
not successful return results. In particular, whether the backing
write or subsequent transition fails remains unresolved.
The v272 eighth-child reproducer still fails; no new production
code or claimed swap acceptance. Continue at backing-store
return/rollback analysis, maintaining the 32K GCC KCORE baseline.

### Live 256K swap pressure: eighth child fails admission v272

The new test-only PDP-6 fixture holds a pipe writer blocked,
then starts sleeping 24K-word pressure children. Seven children
enter PROC_SLEEP; spawning the eighth reports failure (`<wp7!W`).
This reproduces pressure on the process creation path but does
not yet establish why admission failed or exercise successful
swap-out/resume. No KCORE modification or size change.

### 256K test BSS admission and SIMH granularity v271

SIMH physical-core selections use 16K-word increments; the
DAIMOS boot-slot and MM capacity formulas still use complete
8K-word banks. The 32K GCC lowmem disk-root profile remains
the KCORE size reference. The DXR loader rejects BSS values
above `077777` octal (32767 words). A 56K-word test array
therefore prevented INIT startup; a 24K-word test array
successfully reaches INIT on a fresh 256K image, then hits
the expected insufficient-swap-pressure assertion. This is
an admission/negative-control test, not swap acceptance.

## 2026-10-25 256K large-image swap-pressure probe v270

A disposable test-only derivative of the original 32K swap-stress
fixture enlarged each process's pressure array from 8K words to
56K words and built a fresh 256K full-profile GCC PDP-6 image.
The image linked and reached device initialization but halted
before the INIT guest's first `<` marker. The test thus yielded
**no swap-path result**. Simply enlarging each guest is not a
validated way to create pressure; startup / VM image sizing must
be examined before retrying. The failed experimental source was
not installed as an acceptance test or merged into production.
The correct next test should employ more moderate guest images,
additional sleeping pressure processes, verified physical process
residency, and explicit swapped-process resume checks. Existing
v267/v268 saturation and high-water results remain valid.
No production source or 32K KCORE-size reference changed.

## 2026-10-25 swap-pressure acceptance investigation v269

The older daimos-testkit daimos-swap-stress-v1 workload assumes
32K physical memory and forces a blocked pipe writer to swap
using two pressure children. Naively running that identical guest
at 256K does NOT reproduce the needed pressure: the guest emits
`<ws!W`, with `s` identifying failure of the explicit target-swapped
assertion in `run_writer_case`. This is not evidence of a broken
swap implementation: pressure requirements changed. Independently,
rerunning the original 32K profile fails at image construction:
`mkimage: KINIT image leaves less than the configured stack reserve`.
No 32K guest swap result can therefore be claimed from v269.

Next: construct a dedicated 256K test with enough concurrently held
process images to force real reclaim, explicit procinfo-backed
nonresidency checks, and resume/integrity verification. Preserve
32K GCC lowmem disk-root as the KCORE *size* reference even while
its runtime image does not fit. No production code changed in v269.

### Native descriptor saturation and reclaim v268 (2026-10-25)

The PDP-6 production mm_extent_insert/mm_delete test
fills the registry to 132 entries, adds four entries to
reach the 136-entry ceiling, verifies rejection of the
137th allocation without count mutation, and tests a middle
deletion followed by successful reinsertion at full capacity.
Testkit v268 PASS. This verifies the registry boundary and
slot reuse, not physical-memory pressure or process swap.
No KCORE source change; the 32K GCC lowmem disk-root size
baseline remains in force.

### 256K 63-process MM insertion high-water v267 (2026-10-25)

Fresh-image 256K GCC PDP-6 SIMH successfully completed all 63
process starts/finishes while an execution breakpoint sampled
`mm_extent_count` at each `mm_extent_insert` entry. There were
151 observations and a maximum **pre-insertion count of 131**
decimal. The PDP-6 production MM assembler increments this
count only once, at successful insertion. Therefore occupancy
cannot exceed 132 from these observed insertion entries. The
independent v266 point sample directly observed 132 allocated
descriptors, establishing the **132-descriptor peak** for this
specific first-boot workload (capacity 136, spare 4). The
breakpoint-action mechanism adds no persistent KCORE storage.
This does not prove adequacy for concurrent new allocations,
process replacement, repeated reuse or swapping/reclaim under
pressure. Do not change the eight-entry safety allowance before
testing those operations. No production code changed.

### MM native insertion trace v267 (2026-10-25)

A fresh 256K GCC PDP-6 first-boot workload exercising all
63 numbered process slots PASSed with a SIMH execution
breakpoint at `mm_extent_insert`. Breakpoint actions read
`mm_extent_count` at entry and resume. The test captured
151 pre-insert observations, peaking at 131 existing extents
(at most 132 immediately after that insertion); independent
v266 point sampling confirms 132. Capacity was 136 and no
descriptor-pressure failure was reported. This is a workload
trace, not a universal guarantee or validated swap/reclaim
peak; breakpoint actions do not establish whether each
insertion succeeded. No permanent kernel instrumentation
or new KCORE RAM. Size reference remains the 32K GCC lowmem
disk-root profile.

### 256K MM descriptor point observations v266 (2026-10-25)

The fresh-image 63-process 256K GCC PDP-6 SIMH test sampled MM
metadata at the last process's start and completion markers,
using test-only SIMH EXPECT and EXAMINE (no resident instrumentation).
The raw transcript shows `mm_extent_capacity=0210` octal
(136 decimal), `mm_extent_count=0204` octal (132) at
`[3F]`, and `mm_extent_count=0010` octal (8) at
`{3F}`. All 63 guest start/completion tokens were present.
There were only **four spare descriptors** at the last-start
sample; the eight-descriptor non-process/transient allowance
has not been demonstrated sufficient for further overlap.
These are instantaneous point observations, not a measured
peak, and do not prove descriptor safety during concurrent
allocation or swap/reclaim transitions. The simulator places
EXAMINE text on the same line as prior guest console markers;
the v266 fixture therefore extracts embedded address/value
pairs rather than anchoring them to the beginning of lines.
Production KCORE remains unchanged; use the established
32K-word GCC lowmem disk-root configuration for its size.

### Full 256K numbered process range v265 (2026-10-25)

The fresh-image 256K GCC PDP-6 concurrency fixture PASSed
both 48 and 63 guest process start/completion sets, including
guest private-state checks. Hexadecimal slot markers avoid
the earlier single-character marker collision. Slot zero is
reserved, so 63 is the largest numbered process count in the
64-entry table. This does not establish peak MM extent count
or sustained swap/descriptor reuse. KCORE code and the
required 32K-word measurement reference are unchanged.

### PDP-6 thirty-two-process 256K acceptance v264 (2026-10-24)

Fresh 256K full-profile GCC PDP-6 SIMH boot completed 32
process start and completion markers with private-state checks.
An initial false positive was an acceptance-script collision
between slot 24's valid 'X' completion and an error marker;
using '@' for errors resolved the fixture, and the clean
rerun PASSed. The test does not measure MM high-water,
simultaneous residency or swapping. Production KCORE was
unchanged; the 32K GCC lowmem disk-root reference remains.

### PDP-6 sixteen-process 256K acceptance v263 (2026-10-24)

First-boot 256K GCC full-profile native SIMH PASSed 16 process
start markers and 16 completion markers with per-process private
state checks. Test lives in daimos-testkit. The 64-slot policy
includes reserved slot zero, hence at most 63 numbered slots.
No MM peak occupancy was measured, and no KCORE change occurred.
KCORE size reference remains the 32K GCC lowmem disk-root build.

### 256K first-boot scheduler acceptance v262 (2026-10-24)

The separate daimos-testkit `daimos-mm-256k-scheduler-v262.sh`
builds a new GCC full-profile 256K PDP-6 image with five boot
users. Native SIMH PASSed preemption, process context, nice ABI
and disk sleep/wakeup with boot-selected 136-descriptor backing.
This checks real boot/scheduler functionality, but does not
establish 64-process stress or a peak extent occupancy value.
Production source remains unchanged. Continue using 32K-word
lowmem disk-root builds for KCORE size measurements.

### KINIT RAM-sized MM extent capacity v261 (2026-10-24)

Extended the v260 external backing to a boot-selected capacity:
`mm_extent_capacity = 2 * proc_slots_for_core(core_words) + 8`.
This yields 24 descriptors at 32K, 40 at 64K, 72 at 128K,
and 136 at 256K; capacity grows by four for each complete
8K bank. The fixed address of `mm_extents` and the direct
PDP-6 MM/VM memory operands remain unchanged. KINIT reserves
exactly two words per selected descriptor before publishing
its first free arena, and MRES starts afterward.
Native PDP-6 production-assembler tests at 24 and 136 extents
PASS, along with v249/v250 32-entry boundaries, the 32K
layout test and the **fresh-image five-process scheduler**
(preemption, context, nice ABI and disk sleep/wakeup). GCC
32K-profile KCORE is 024766 octal (10742 linked words), but
its 24-entry external registry occupies another 48 words:
actual fixed KCORE-plus-registry is 10790 words, 15 fewer
than the old 10805-word fixed-KCORE baseline. Likewise,
the linked KCORE+MRES report 13605 excludes these 48 words;
combined physical footprint is 13653 versus old 13668.
This is not a guarantee of 64 fully resident processes:
physical memory, pinned allocations and pressure still limit
simultaneous residency. High-load 256K boot acceptance is
still outstanding; only the maximum-capacity native allocator
boundary has been exercised at 136 extents.

### KINIT external MM backing restoration v260 (2026-10-24)

Restored the historical direct-address table boundary design as
an **intermediate 32-descriptor stage**: `mm_extents` aliases
`__kcore_low_end` without linked BSS storage. KINIT reserves and
zeroes the following 64 words before releasing any low arena;
MRES starts after that reserved metadata. The native PDP-6
insertion guard now reads `mm_extent_capacity`, currently reset
to 32 in `mm_boot_init` until dynamic higher capacities are
boot-validated. Native extent v249/v250 insertion/deletion tests
PASS; the 32K GCC disk-root image links and layout test v260
PASS. This change does **not** yet permit more than 32 extents.
32K-profile linked KCORE decreases from 10805 to 10742 words,
but the external 64-word reservation remains resident; with
one added capacity word the actual KCORE-plus-table footprint
is 10806 rather than 10805 words. Do not count moving the
table past the KCORE label as physical-RAM savings. Full
boot acceptance and memory-tier capacity selection remain open.

### KCORE measurement configuration v259 (2026-10-24)

For every KCORE size comparison, use a **32K-word** GCC
low-memory disk-root build, explicitly setting
`BOOT_PROFILE=lowmem ROOT=disk COMPILER=gcc
BOOT_CORE_WORDS=0100000 SIMH_CPU_KWORDS=32`.
The physical 32K boot may remain unsupported; linking and
measuring this profile is nonetheless the mandatory optimization
reference. Report KCORE, KCORE+MRES and PERMANENT_LAST separately.

### Process-slot 8K policy v258 (2026-10-24)

The boot process-slot count now grows from eight at 32K by two
for each complete extra 8K bank up to 64 slots at 256K.
The min-slot contract used by packed/storage metadata is eight.
Testkit checks 8K boundaries and incomplete banks; full GCC
lowmem disk link keeps KCORE+MRES at 032544 octal (13668).
This is process-slot sizing, not restoration of runtime MM
extent capacity; that remains a separate boot-layout task.

## 2026-10-23 MEMFS XIP alignment v203

Large dedicated MEMFS allocations now use `mm_alloc_aligned` with `02000`
alignment. Small packed chunks keep their existing path. Focused native
MEMFS pinning tests PASS on SIMH and AAP. EXEC/RUN remains unchanged.

## 2026-10-23 MEMFS XIP reservation foundation v202

MEMFS MRES has a four-word per-file XIP reference table and reuses the MM
extent pin count. The privileged reservation API currently admits only an
existing dedicated large MEMFS chunk with a `02000`-aligned physical base;
regular DXR files are not launched from this storage. Write, truncate,
reclaim and unmount paths now respect active reservations. No EXEC/VM
ownership changes have been made. The GCC lowmem image links, and focused
SIMH MEMFS and FIFO native target tests pass. AAP also passes the same
MEMFS pinning target fixture; actual AAP XIP process launch is pending.

# KCORE - Historical 2026-10-23 backlog (not current feature checklist)

The sections in this archived planning snapshot include completed,
deferred and cancelled work. The current v1521 feature-only checklist
is at the top of this file. In particular, old 32K optimization and
restricted XIP checkboxes are not active.

## 2026-10-23 native uncompressed EXEC physical OOM v205

daimos-testkit v205 PASSes uncompressed executable replacement on a
256K PDP-6 where old+new VM extents exceed physical memory. The
replacement executes as PID 1 by the v201-corrected low-memory EXEC
path. A false-negative DXR metadata assertion was fixed in the
testkit. No new KCORE text or static data; fresh link endpoint
`025657` octal. 32K acceptance remains deferred.

## 2026-10-23 resident EXEC -2 comparison correction v201

The EXEC staged-loader out-of-memory return must be tested with a
full 36-bit compare (`CAME 1,[-2]`), not `CAIE` with an 18-bit immediate.
The latter bypassed the already implemented low-memory replacement
retry. Native PDP-6 v201 pressured and unpressured compressed EXEC
replacement PASS after the fix; the v200 eight-case staged transaction
test also PASSes. No persistent RAM growth; the fresh full-profile
`__kcore_low_end=025657` octal. 32K acceptance remains deferred and
the full-profile permanent-memory limit remains open.

## 2026-10-23 EXEC late transaction boundary tests v200

daimos-testkit v200 executes the production `exec_replace_current`
assembler under PDP-6 instruction-level fault injection. Eight modes
PASS: load, startup and old-VM destruction failure; PURE backing-bit
commit variants; RT newly-claimed rollback, old-owner preservation and
non-RT successful release. Restored user mapping hold, swap backing
and prior credential mount bit are also checked. External call stubs
are injected, so these are not native MM allocation/destroy failures;
native full-image replacement/rollback tests are tracked separately.
No KCORE growth (`__kcore_low_end=025657`). Successful RT-to-RT EXEC
and direct EXEC ENOMEM under physical pressure remain to be verified.

## 2026-10-23 RUN loader VM creation resource exhaustion v199

Native 256K PDP-6 testkit v199 **PASS**: a separately installed valid
large child DXR RUNs before and after pressure, but four identical RUN
requests fail while the parent's live, unswappable VM consumes enough
physical core that `vm_space_create` cannot allocate another child.
Each rejection preserves caller break/data, live process slots and
process-word accounting. Shrink restores the old baseline, and v197
late descriptor-map rollback/reap/recovery tests also pass. Source
change is test-only: `__kcore_low_end=025657` octal, no added stack or
persistent RAM. EXEC replacement failure under deliberate pressure
remains unverified; 32K acceptance remains explicitly deferred.

## 2026-10-23 256K physical MM exhaustion regression v198

New daimos-testkit v198 native fixture PASSes four unallocatable but
well-formed `BRK(0776000)` growth requests with three live children.
After each failed VM/MM growth attempt, caller break, VM word count,
sentinel contents and process slot count remain intact. The combined
fixture subsequently passes v197's late RUN failure rollback and
healthy child recovery. Kernel unchanged: `__kcore_low_end=025657`
octal. Direct RUN/EXEC loader allocation failure injection and
fine-grained MM descriptor leakage tests remain unverified. The 32K
gate is deferred, not passed.

## 2026-10-23 RUN late descriptor-map rollback regression v197

Native 256K PDP-6 RUN V2 fixture PASSes sixteen late-stage failures
after child VM, startup and u-area setup, with three other children
live. Process slots and reported process words remain stable across
each failed RUN; after releasing/reaping the children, both return to
their baseline, and a subsequent valid RUN/WAIT succeeds. This is
not forced MM allocation exhaustion and does not individually audit
orphaned dynamic MM allocations. No kernel text or persistent data
changed; linked `__kcore_low_end` remains `025657` octal. The 32K
gate remains deferred by request.

## 2026-10-23 RUN native lifecycle/ID reuse pass v196

Existing 256K native PDP-6 process-lifecycle and process-ID-lifetime
testkit targets both PASS: RUN/WAIT/reap/orphan/descriptor cases and
creator-slot reuse protection for pgrp, session and domain IDs.
No resident code change; `__kcore_low_end=025657` octal. Continue
non-32K EXEC/RUN validation; the 32K gate is explicitly deferred.

## 2026-10-23 EXEC/RUN 32K acceptance deferred v196

Per current project direction, stop investigating 32K-only configuration,
KINIT packaging and oversized DSH EXEC errors for now. Keep the 8K/32K
targets documented as deferred, not passed; do not reduce memory reserves
or weaken size gates. Prioritize 256K native EXEC/RUN correctness,
rollback and process lifecycle tests, while preserving the resident RAM
footprint. The most recent 256K linked KCORE end remains `025657`
octal. The independent full-profile permanent-size overage remains open.

## 2026-10-23 native RT_REQUIRED RUN child follow-up v194

The existing compressed RT_REQUIRED child admission/rejection/release
native PDP-6 test passed after v193 post-VM EXEC rollback coverage.
This independent RUN path check introduced no KCORE modifications.
The linked `__kcore_low_end` remains `025657` octal. Functional-only
image-size settings are not strict permanent-memory acceptance.

## 2026-10-23 EXEC compressed decode failure RT cleanup v193

Native PDP-6 testkit v193 PASSes new RT owner cleanup and existing
owner preservation when an RT_REQUIRED compressed DXR2 has valid
header geometry but a truncated D6LZ control/literal stream. The
loader's normal path allocates the staged VM before invoking the
decoder; failure then destroys that staged VM and rolls back only a
new reservation. The earlier header-failure and successful EXEC
scenarios also PASS. No new kernel code or resident memory; current
post-p10fold KCORE end `025657` octal. Production timing, executable
image cap and 16K residency are still unaccepted.

## 2026-10-23 EXEC RT correction cross-regression v192

After correcting the live RT-claim marker/VFS stat overlap, the
versioned native fixture with 28 malformed EXEC/RUN blocks still PASSes
and the normal EXEC replacement completes successfully. Its full log
is `$TMPDIR/daimos-exec-v192-malformed-retest.log`. The v191 RT owner
transaction test independently PASSes. KCC/p10fold linked KCORE and
full permanent-memory measurements are unchanged; strict production
timing and memory acceptance are still open.

## 2026-10-23 EXEC loader RT claim scratch lifetime correction v191

The seven-word VFS stat output at `-011..-003(17)` was overwriting the
RT-claim marker previously stored at `-003(17)`. An existing RT owner
could therefore be released by a failed EXEC. Reuse `-012(17)` only
after its vnode output has been copied to AC12, initialize it before
the VFS stat call, and bypass marker-based cleanup on earlier failures.
The v191 native RT-owner transaction regression reproduces the prior
failure and PASSes after correction (new-claim rollback, old-owner
preservation, admission and normal-EXEC release). Both KCORE text/end
and transient stack usage are unchanged; `__kcore_low_end=025657`
octal after KCC/p10fold. Full memory still exceeds the strict limit.

## 2026-10-23 EXEC/RUN malformed launch regression v190

The PDP-6 native EXEC replacement fixture in daimos-testkit v190
now also PASSes 28 rejected malformed EXEC/RUN launch requests before
completing normal replacement. Invalid headers, count upper halves,
oversized counts, bad SIXBIT length fields and out-of-block records
were rejected without destroying the caller. Functional-only image
size and 35-second observation settings were retained. RT-owner
rollback, strict 15-second/`036000` gates and the full permanent
memory deficit remain open. This test-only checkpoint does not change
the `025657`-octal post-p10fold KCORE exclusive endpoint.

## 2026-10-23 EXEC/RUN native functional retest v189

Native PDP-6 replacement and child-session EXEC tests PASS on v188
(`6b9ce2a`), covering normal image replacement and child-process reap.
This follows the v188 linked `025657`-octal KCORE measurement.
The 35-second test observation and `037000` functional image cap do
not satisfy the strict timing/size gates. Malformed launch-block and
RT reservation failure cases remain open; this documentation-only
checkpoint does not alter the resident size.

## 2026-10-23 EXEC strict record start and low-memory RT cleanup v188

In `exec_replace_current` the argv/env record scanner tests that AC15
points strictly before the validated block end with `CAML/JRST`,
replacing the unused `MOVE/SUB/JUMPLE` difference. A nonzero record
count still requires a valid first word before calling the record
parser; record-end validation is unchanged. In low-memory EXEC the old
RT reservation is released only if AC14 equals `proc_rt_owner`, using
`CAMN/SETZM` directly instead of loading the owner in AC1 first.
The same-checkout KCC/p10fold exclusive KCORE endpoint changed from
`025661` to `025657` octal, two resident words saved. Native runtime
and malformed-block testing on v188 are still pending.

Full KCC `make permanent-size` accounting on the same profile reports:
KCORE 11135 words (`025577`), KCORE+MRES 18670 words (`044356`),
PERMANENT_LAST `044435`, MULTI_LAST `044476`. The strict `040000`
permanent-address limit still fails by 2333 addresses (single root)
or 2366 (multi root), as shown by the report's distance metric.
The KCORE exclusive-end address is not interchangeable with the
report's standalone KCORE size; keep both measurements distinct.

## 2026-10-23 EXEC/RUN native regression follow-up v187

Fresh-image PDP-6 simulator diagnostics for v186 PASS both EXEC current
image replacement and session-leader child EXEC/parent reap. The v185
EXEC replacement diagnostic also PASSed. Observation window was
explicitly extended to 35 seconds, with the functional-only `037000`
DSH image cap; original production timing and executable-size gates
remain open. Targeted RT-owner rollback and malformed launch-block
coverage also remain open. No new code or resident-word change here;
the current exclusive KCORE end remains `025661` octal.

## 2026-10-23 EXEC real-time owner cleanup v186

In the successful old-real-time reservation release and the failure
rollback, compare the saved process slot directly against
`proc_rt_owner` using `CAMN` and conditionally execute `SETZM`.
This replaces each `MOVE/CAME/JRST/SETZM` sequence with two words,
without clearing a reservation transferred to another owner. The
same-checkout post-p10fold KCORE exclusive end changed from `025665`
to `025661` octal (four resident words), without additional RAM or
stack. The v185 native EXEC replacement diagnostic PASSed using the
existing 35-second functional-only observation window; v186 native
runtime regression remains pending at this size measurement.

## 2026-10-23 EXEC direct validated scalar counts v185

`exec_replace_current` now loads the argc and envc launch words
directly into AC13 and AC14 for their existing upper-half-zero and
maximum-count checks. The formerly separate `HRRZ` copies are
redundant: both values are already validated pure 18-bit counts before
their use in the packed count pair or record total. Same-checkout
KCC/p10fold relink reduced exclusive `__kcore_low_end` from `025667`
to `025665` octal (two permanent words), with no additional RAM or
stack requirement. Native EXEC/RUN regression tests for v184-v185
remain pending at the time of this size measurement.

## 2026-10-23 EXEC old real-time ownership comparison v184

`exec_replace_current` now uses `CAMN 13,proc_rt_owner` followed by
`MOVEI 14,1` to set its existing old-RT flag only when the current
slot owns the real-time reservation. This avoids a temporary AC1 load
and a branch, preserving the original owner-equality behavior and
unchanged rollback logic. A same-checkout KCC/p10fold link moved
`__kcore_low_end` from `025671` to `025667` octal (two resident words).
No additional RAM or stack; v184 native EXEC regression pending.

## 2026-10-23 EXEC/RUN runtime verification v183

Native PDP-6 fresh-image EXEC replacement and child-session EXEC both
PASS after v181, using daimos-testkit's opt-in 35-second observation
window and functional-only `DSH_IMAGE_LIMIT=037000`. ABI inspection
PASSes using the live assembler rather than the retired C fallback.
Original 15-second timing acceptance and `036000` production executable
size remain outstanding, as do malformed-input and strict low-memory
regressions. No further code or resident-word change in this test pass.

## 2026-10-23 EXEC in-place path advancement v181

The first path record parse in `exec_replace_current` now advances AC15
directly to the first argv/environment record with `ADD 15,1` instead of
constructing the address in AC2 and copying it back. The existing
`CAMLE` end-of-block check still validates the computed address before
use. In a same-checkout KCC/p10fold KCORE relink, exclusive
`__kcore_low_end` moved from `025673` to `025671` octal, saving two
resident words. No extra stack or persistent RAM; syscall ABI unchanged.
Native EXEC/RUN launch and malformed-input regressions remain pending.

## 2026-10-23 EXEC argument-record end comparison v180

`exec_replace_current` now checks each argument/environment record end
directly (`CAMLE/JRST`) against the validated EXEC block endpoint rather
than subtracting the two addresses and testing the sign. This completes
the interrupted v180 edit without altering error handling or record
semantics. The same-checkout KCC/p10fold link reduced the exclusive
`__kcore_low_end` from `025674` to `025673` octal: one resident word.
Native EXEC/RUN malformed-input and process-launch tests are pending.

## 2026-10-22 EXEC path record endpoint check v179

`exec_replace_current` now checks the parsed path-record end directly
against the validated launch-block end (`CAMLE/JRST`) instead of using
`MOVE/SUB/JUMPL`. The `sixbit_record_words` parse and its nonzero check
remain in place. The same-checkout KCC/p10fold KCORE relink reduced the
exclusive `__kcore_low_end` from `025675` to `025674` octal, saving one
resident word without changing the syscall ABI, stack, or persistent state.
Native malformed-record and EXEC/RUN regression tests remain pending.

## 2026-10-22 EXEC launch size comparison v178

In `exec_replace_current`, replaced `MOVE/SUB/JUMPL` for the declared
launch-block length against available mapped words with the equivalent
`CAMLE/JRST` bound. The earlier minimum-length validations are preserved;
declared words greater than mapped words still fail before record parsing.
Same-checkout KCC/p10fold KCORE link reduced `__kcore_low_end` from
`025676` to `025675` octal, saving one resident word. A separate attempt
to change the RUN slot increment was reverted because it saved no
post-fold words. Native EXEC/RUN malformed-input testing remains pending.

## 2026-10-22 RUN record count combination v177

Following the existing validation of the RUN V2 argc and envc count
fields as pure 18-bit values, the record scanner now sums the two source
words directly (`MOVE` and `ADD`) instead of extracting both RH fields
into separate registers. This retains the exact nonnegative count bounds
and record-scanning semantics, and reduces the post-p10fold linked KCORE
exclusive end from `025677` to `025676` octal (one resident word).
KCC/p10fold link succeeded; native malformed-argument and startup tests
remain pending. No additional RAM or stack requirement.

## 2026-10-22 RUN initial path bound reduction v176

RUN V2 verifies `RUN_MIN_WORDS=010` before computing the path position
at `RUN_FIXED_WORDS=6`; consequently the path record starts strictly
before the validated block end. Removed the redundant `CAML/JRST`
pair in `proc_run_block`, retaining the `sixbit_record_words` validation
and the subsequent record-end bounds check. Same-checkout KCC/p10fold
link reduced exclusive `__kcore_low_end` from `025701` to `025677`
octal (two resident words). No additional stack or permanent storage.
Native malformed RUN block tests remain outstanding.

## 2026-10-22 RUN scalar count checks v175

RUN now checks the three scalar count fields (fd-map, argc, envc) with
`TLNE` on the full loaded value instead of using `HRRZ` followed by
`CAME`.  The left-half nonzero rejection and existing maximum-count
comparisons preserve the validation ABI. A KCC/p10fold relink in the
same checkout reduced `__kcore_low_end` from `025704` to `025701`
octal (three resident words). No extra permanent or stack state.
Native launch and malformed-block regression tests remain pending.

## 2026-10-22 RUN loader error cleanup v174

`proc_run_block` now routes all negative `exec_load_process` return values
directly to the existing claimed-slot discard path. The former comparison
against `EXEC_LOAD_NOMEM` and `MOVEI AC2,011` did not affect that cleanup
or its return value. A same-checkout KCC/p10fold relink moves the exclusive
KCORE endpoint from `025711` to `025704` octal: five resident words saved.
No permanent data or ABI changes; native process-launch regression remains
pending. A separate experimental merge of EXEC loader error rollback paths
linked successfully but yielded zero post-fold savings and was reverted.

## 2026-10-22 RUN validated pointer reuse v173

RUN previously called `sixbit_record_words` again after successful block
validation, to reconstruct the argv/environment record address for
`vm_space_startup`. `proc_run.s` now saves that address in one additional
temporary stack word before scanning the records and reuses it after
child image loading. This removes redundant parsing and its failure branch.
Same-checkout KCC/p10fold relink changed the `__kcore_low_end` exclusive
address from `025715` to `025711` octal (four resident words saved).
Peak RUN kernel stack increases by one word; permanent tables, EXEC/RUN
ABI and process descriptor size are unchanged. Native RUN/EXEC regression
and full image boot acceptance remain pending.

## 2026-10-22 EXEC validation pointer reuse v172

`exec_replace_current` in `proc/exec_lowmem_pdp6.s` now retains the
already-validated argv/environment record pointer in AC14 across its
record scan, rather than calling `sixbit_record_words` again on the path.
AC14 is available after argc/envc are merged and is repurposed for the
old RT-owner flag only after the scan. The next linked EXEC symbol moved
from `000430` to `000425` (octal), corresponding to five fewer words
in the leading EXEC section. The current linked `__kcore_low_end` is
`025715` octal; it is not compared against a freshly rebuilt matching
baseline, and no whole-image net saving or runtime acceptance is claimed.
Keep EXEC/RUN permanently resident; continue with RUN input-validation
and error-cleanup audit after native tests.

## 2026-10-22 additional unused C source sweep v171

The production MEMFS MRES builds `memfs_data_pdp6.s` and
`memfs_snapshot_pdp6.s` directly. Neither portable C source was
compiled by the active rule. Both superseded C files and their stale
source dependencies were removed; the assembly sources remain unchanged
except for a comment describing the replacement. Kernel `kcore` source
generation and boot integration dry run passed. This eliminates obsolete
source, with **no demonstrated resident KCORE or MRES word saving**.

Retain `d6fs_provider.c`: external daimos-testkit validation references
it even though the production D6FS provider uses handwritten assembly.
Also retain the live C portions of the mixed `proc`, `file`, module-runtime,
and swap implementations. No native machine execution is claimed.

## 2026-10-22 redundant C implementation sweep v170

The PDP-6 build now constructs EXEC exclusively from
`proc/exec_lowmem_pdp6.s` and `proc/exec_load.s`; the entirely disabled
`proc/exec.c` fallback was removed.  Likewise `proc/vm_pdp6.c` was removed:
all six functions were already compiled out by `VM_SPACE_*_PDP6_ASM`
defines, and the live routines remain in `proc/vm_pdp6.s`.  The PDP-6
`vm_backend.s` target is now an empty `.text` section; other backend
selections retain their C compilation rule.  These changes eliminate dead
source and compiler work, NOT linked KCORE instructions.  Do not count
the source-line reduction as resident RAM savings.

The same-basename kernel C/assembly inventory identified `vm_pdp6` as
the only exact pair; a build-rule examination additionally found EXEC's
differently named assembly replacement.  Other mixed units must remain:
`proc.c` and `file.c` still contribute active C functions after their
assembly-specific feature guards, and `module_runtime.c` contributes
descriptor/binding storage absent from its assembly implementation.
No runtime acceptance or new size measurement is claimed for this sweep.

## 2026-10-15 GCC 32K disk-root measurement and MM recovery v138

Isolated baseline at commit `37fba7e`, with `BOOT_PROFILE=lowmem`,
`COMPILER=gcc`, `ROOT=disk`, `BOOT=dsk`, `BOOT_CORE_WORDS=0100000`,
`SIMH_CPU_KWORDS=32`, and unchanged `02000` KINIT stack reserve:
complete image build PASS; KCORE exclusive endpoint `023503` octal
(10051 decimal); KINIT image end `072203` octal. The MM object is
520 text + 70 BSS words. The baseline is above the strict 8K target
by 1860 decimal words (exclusive endpoint must be below 020000).

Current committed MM assembler had lost optimizations from v094, v095,
v096, v106, v109, v110 and v111 compared with commit `9d52bb0`.
Restoring that exact v111 assembly in isolation builds a complete GCC
32K image: KCORE endpoint `023435` octal (10013 decimal), KINIT end
`072135` octal, MM object 482 text + 70 BSS words. Net whole-image
improvement is 38 decimal words after folding/layout;
the MM text contribution also falls by 38 words. The optimized image still
requires 1822 more words for a strictly below-8K exclusive endpoint.

This is a GCC link-and-package measurement, NOT native runtime
acceptance. `daimos-testkit` host `mm-core` cannot currently link due to
missing `proc_storage_publish_v30` and
`proc_swap_storage_invariant_v30`; fix the test fixture and rerun native
PDP-6 MM allocation, freeing, relocation and compaction regressions
before calling the recovered code runtime accepted. The older v136
external-descriptor branch additionally changes the insertion capacity
guard; it was not imported into the v138 image.

Date: 2026-10-24 (source reconciliation at v219, a405005)
Basis: checked DAIMOS source, PROJECT-STATE.MD, TODO.MD, and committed
native-test evidence. This is a documentation audit, not a new full link or
native simulator acceptance run. A staged, host-tested implementation is not
completed until linked, measured and accepted on the relevant PDP-6.

### Verified implementation baseline (v219)

- `proc/proc.h` defines three-word descriptors and 24/40/64/88/128/192/256
  boot-selected slot tiers; `proc/proc_boot.c` provides tier selection.
- `mm/mm.h` and `mm/mm_pdp6.s` still define 32 two-word MM extents.
  `ipc/pipe.h` still defines an eight-word FIFO buffer.
- `proc/exec_lowmem_pdp6.s` and `proc/proc_run.s` are the resident EXEC/RUN
  paths. Production `vm_space_startup` in `proc/vm_pdp6.s` writes user VM
  metadata even with zero arguments; it is not XIP-safe.
- The production VM destruction path in `proc/vm_pdp6.s` invokes the MEMFS
  provider's XIP release operation (026), with normal MM owner/type fallback.
  Private MEMFS operation 027 supplies restricted raw PURE XIP admission.
  Ordinary EXEC/RUN does not invoke that admission operation.
- The restricted linker XIP producer is experimental, with a limited
  instruction format. v215-v219 native fixtures validate selected admission,
  release, rollback and non-writing user-mode paths, **not** fully scheduled
  production EXEC/RUN, process exit, or swap interaction.
- The most recent documented **GCC lowmem** linked KCORE exclusive endpoint
  is `025134` octal (10844 decimal words, v213). Reaching a *strictly below*
  `020000` octal endpoint requires at least 2653 fewer words. The distinct
  EXEC/RUN **full-profile** checkpoint is `025657` octal (11183 words,
  v205); profiles must not be mixed. Neither is a newly measured v219 link.
- v190-v205 recorded native malformed EXEC/RUN, staged transaction, RUN
  failure rollback, and compressed/uncompressed low-memory EXEC regressions.
  This is positive targeted evidence, not 32K boot acceptance.
- The 32K strict gate is explicitly deferred by v196; do not report it as
  passing or relax it. The v138 GCC lowmem result (`023435` octal) and the
  older v41/v42 (`021541`) are historical, not current size baselines.

## P0: 32K resident limit and correctness

- [ ] When the deferred strict 32K work resumes, shrink the GCC lowmem KCORE
  exclusive endpoint below `020000` octal. The latest documented comparable
  lowmem link is `025134` (v213); first repeat a fresh v219 full link and
  full accounting, not just an object-level estimate.
- [ ] Measure full permanent KCORE plus committed MRES, irreducible scratch,
  KINIT allocations and process startup headroom; retain the separate general
  below-040000 permanent-address limit where applicable.
- [ ] [Deferred at v196] Pass the strict GCC 32K size gate and native 32K
  boot/INIT/LOGIN/DSH, process-pressure and filesystem tests with the same
  fresh image. Do not conflate 256K EXEC/RUN passes with this acceptance.
- [ ] Fix compiler, assembler and kernel correctness bugs at their source;
  add regression tests in daimos-testkit before marking the sweep complete.
- [ ] Audit source-level size hotspots, generated objects and real post-p10fold
  link maps; reject optimizations that increase the 32K resident footprint.

## Historical P1 packed-process plan (superseded by v1375)

The inactive v33-v46 PACKED registry, eviction and restore prototypes
remain historical research. The old opt-in compressed **disk-swap**
track is cancelled. A new swapless-only design retains mutable process
state in RAM, optionally compresses it by explicit eligibility, drops
immutable PURE text from *inactive* process storage, and restores one
full APR extent before the process runs. Disk raw-swap is unchanged.
No production activation is claimed. See the authoritative
`pdp10-doc/KERNEL/SWAPLESS-MEMORY.MD`.

## P2: kernel data structures and workload-specific sizing

These are profiling/selection tasks, not claims that the existing MM,
scheduler, pipe or slot-tier implementations are absent.

- [ ] Evaluate runtime-sized MM extent descriptors versus the current
  RAM-scaled `MM_MAX_EXTENTS=136` two-word descriptor capacity
  (`mm/mm.h`, `mm/mm_pdp6.s`; normal boot may select fewer entries);
  measure nested MAKE/KCC/KCPP load and relocation-sensitive assembly cost.
- [ ] Compare sorted linear MM scans with optional lookup indexes only for
  measured large-core workloads; avoid permanent small-core dispatch cost.
- [ ] Profile D6FS cache footprint and replacement under native
  compilation; evaluate boot-selected budgets against free memory after
  essential allocations. Defer DTFS/TSFS cache and DECtape seek profiling
  to the post-KCORE filesystem effort (v906).
- [ ] Profile VFS directory lookup under native MAKE/KCC and test compact
  entry caching or optional indexes, reusing D6FS hashing if advantageous.
- [ ] For swapless-only inactive process storage, benchmark the smallest
  MM-authoritative representation against actual 24/40/64-slot usage;
  do not activate the archived dense/sparse PACKED registry without proof.
- [ ] Profile intrusive scheduler queue scans at larger process counts before
  adding priority indexing or extra branches to the common context-switch path.
- [ ] Benchmark the existing fixed eight-word, word-only pipe FIFO ring
  (ipc/pipe.h and pipe.s); change capacity only with demonstrated RAM or
  throughput wins. Do not revive obsolete character-width pipe formats.
- [ ] Confirm boot-selected process slot tiers (proc/proc.h and proc_boot.c)
  against real maximum-concurrency tests; do not reimplement already working
  boot-time selection or add runtime resizing without evidence.
- [ ] Audit remaining permanently resident device/accounting state and shared
  filesystem scratch for outright deletion or genuine buffer reuse; avoid
  moving always-required storage to MM merely to disguise its cost.

## P3: folding and compiler optimization

Priority decision (2026-10-24): further one-JRST folding is a **last-effort**
optimization after source and architectural shrinking opportunities are
exhausted. Preserve existing p10fold behavior, but do not spend current
engineering time expanding cross-domain JRST folding. Reconsider only after
a fresh final linked census and the existing >20-net-word/cold-path gate.

- [ ] Rerun p10fold read-only cross-domain census after final handwritten
  KCORE objects and fold plans stabilize; use explicit `-a` anchors, `-x`
  fold-plan exclusions and `-C` verified cold donors.
- [ ] Implement DLINK external-anchor folding only if nonoverlapping net
  savings exceed approximately 20 resident words with no KCORE growth,
  target-side metadata or hot-path JRST regressions.
- [ ] After self-hosting, use p10super results to improve KCC generator
  canonicalization (common tails, branches, AC allocation and equivalent
  forms), not by editing generated assembly.
- [ ] Defer native streaming low-memory p10fold and larger semantic/SMT
  searches until self-hosting and simpler optimizations justify their cost.

## P1 follow-up: restricted MEMFS XIP production integration

**HISTORICAL (CLOSED):** the former v203-v219 XIP follow-up checklist
is superseded by v1469-v1485's production acceptance of privileged,
root-owned, nonwriting, argument-free MEMFS XIP EXEC/RUN, rollback,
pin lifetime, scheduler/process exit and memory-pressure cases at
128K/192K/256K. It is not an active implementation task. Ordinary
DXR/PURE does not become raw XIP; physical MEMFS minimum is 96K.

## 2026-10-24 architectural shrink roadmap and proposal A start v221

### Proposal D first-boot MM checkpoint fixture v257 (2026-10-24)

Added `tests/validation/daimos-mm-firstboot-v257.sh` to
`daimos-testkit` (commit `32dfb7e`). The fixture builds its
own disposable five-process GCC-profile PDP-6 boot image,
checks the existing scheduler preemption/context/nice and
disk sleep/wakeup assertions, and examines the mapped
`mm_extent_count` during its **first** SIMH boot.
The first acceptance run PASSed the scheduler but deleted its
transcript before the observation was extracted. A second
independent fresh-build run PASSed and emitted one explicit
checkpoint sample of `000000000006` (six decimal) at
octal address 024537. The fixture now derives this address
from the fresh link map and requires an observation.
The test demonstrates successful non-production observation
without replaying modified media. **One point sample is not
the high-water value**: no MM registry reduction or permanent
kernel instrumentation is justified. KCORE stays 025065
octal (10805 words).

### Proposal D simulator replay/root diagnostic correction v256 (2026-10-24)

Revisited the v255 simulator failure. Manual artifact discovery had
used lexicographic sort of `daimos-scheduler-multiproc-v1-<pid>`
paths, selecting an October 20 image rather than the October 24
fixture. Corrected the selection to modification-time ordering and
replayed the newest saved October 24 image (MM count at 024537,
insert entry at 001145 octal), preserving its 8M SIMH throttle.
The subsequent replay also exited with `?RT` before the insertion
breakpoint. Kernel `KINIT_ERR_RT` documentation establishes that
`?RT` means root mount/validation failure or absence of a valid
`/SYSTEM/INIT`; it is **not a scheduler failure code**. The official
single-use scheduler build/run still PASSes, so a repeat execution
of saved mutable media is not a valid workload measurement.
Fresh-build breakpoint telemetry must be installed before the
first boot (with reproducible disposable media), not introduced
into a previously executed image. No MM high-water result or
production change is claimed.

### Proposal D fresh-image insertion tracing attempt v255 (2026-10-24)

Reran the unmodified official five-user scheduler regression with
`MAKEFLAGS=COMPILER=gcc` and retained its isolated build; the
native SIMH acceptance **PASSed**. Its freshly linked `kcore.map`
identified `mm_extent_count` at 023543 and
`mm_extent_insert` at 001167 octal. A separate manual SIMH
replay of that same build, with a one-shot breakpoint on
`mm_extent_insert`, instead emitted `?RT` and never reached
the breakpoint. Therefore the replay environment is not yet
equivalent to the official test invocation. No occupancy or
high-water data were accepted from that replay and there were
no kernel or testkit source changes. Resolve the launch-path
discrepancy before using debugger-based telemetry; keep all
32 extents.

### Proposal D official scheduler rerun v254 (2026-10-24)

After DevSpace2 became reachable again, reran the unmodified
`daimos-testkit` target `test-daimos-scheduler-multiproc`
with `MAKEFLAGS=COMPILER=gcc`. Its full isolated PDP-6 build
and SIMH regression **PASSed**, including preemption, process
context, nice ABI and disk sleep/wakeup. This supersedes the
earlier v253 implication of a persistent scheduler failure
from the ad hoc replay, without establishing a specific cause
for that `?RT` output. It does not turn v252's three extent
snapshots into a high-water measurement: the 32-entry MM
capacity remains unchanged. No kernel or testkit changes.

### Proposal D unstepped scheduler cross-check v253 (2026-10-24)

Replayed the preserved GCC five-process image without SIMH
EXPECT stepping, using the fixture's 8M instruction/second
throttle. SIMH booted the filesystem providers and halted
normally, but the guest emitted `?RT` instead of the expected
multichild scheduler progress markers. Therefore the earlier
v252 scheduler completion failure cannot be attributed solely
to intrusive diagnostic stepping. The only confirmed extent
data remain three point samples of 020 octal (16) from v252;
there is **no validated workload high-water result**. Do not
downsize the 32-entry MM table on these observations. No
production or testkit source changes were accepted.

### Proposal D GCC-profile MM occupancy checkpoints v252 (2026-10-24)

The earlier `036013 > 036000` DSH gate failure was caused by
building the five-process diagnostic with the default KCC
configuration. The previously documented lowmem `COMPILER=gcc`
configuration builds past the unchanged gate and reaches SIMH.
An isolated scheduler fixture inspection of mapped symbol
`mm_extent_count` at octal address 024537 yielded 020 octal
(16 decimal) at each of three scripted checkpoints. However the
diagnostic-stepped scheduler run then reported that timer
preemption did not complete all boot users; this is **not a
passing scheduler regression or an extent high-water measurement**.
No persistent instrumentation, production code or MM allocation
changes were made; the experimental fixture was removed.
Retain the 32-descriptor limit until representative full
workloads are measured with nonintrusive peak tracking.

### Proposal D real workload MM occupancy attempt v251 (2026-10-24)

Attempted a non-invasive PDP-6 runtime snapshot of `mm_extent_count`
at scheduler checkpoints by adapting the existing five-process SIMH
fixture in a separately versioned testkit-only script. The build
failed before simulator execution at the existing strict DSH user
image size gate: `036013` octal exceeds `036000`.
Consequently **no runtime extent occupancy samples or high-water
measurements exist from this attempt**. The DSH acceptance limit
was not relaxed, the experimental test script was discarded,
and unrelated testkit working-tree modifications were preserved.
The tested production MM registry remains 32 entries (64 words),
with no justified shrink or new permanent RAM saving.

### Proposal D native MM sorted insertion verification v250 (2026-10-24)

Added a separately versioned `daimos-mm-extents-v250` test to
daimos-testkit, extending v249's full 32-entry boundary and -3
rejection coverage with deletion followed by insertion into the
middle of the live descriptor array. The fixture checks both the
inserted two-word slot and the displaced old slot. It PASSes
using production PDP-6 `mm_extent_insert`/`mm_delete` leaves,
testkit commit `580e978`. This is deterministic correctness
coverage, not workload high-water telemetry; the 32-entry MM
limit stays unchanged and KCORE remains 025065 octal.

### Proposal D native extent registry boundary fixture v249 (2026-10-24)

Added a native PDP-6 fixture to daimos-testkit:
`tests/validation/daimos-mm-extents-v249.s` and
`daimos-mm-extents-v249.sh`. It extracts the production
`mm_delete` and `mm_extent_insert` leaves, fills all 040 octal
(32) slots, confirms that insertion 33 returns -3, deletes one
slot and successfully refills. It PASSes with p10run/PDP-6;
testkit commit `11dc124`. This tests the boundary, not real
workload high-water, fragmentation or VM compaction. No DAIMOS
production allocation or code change is justified. Preserve the
025065-octal (10805-word) KCORE baseline.

### Proposal D host MM fixture dependency investigation v248 (2026-10-24)

The `mm-core` failure described in v247 is not only an obsolete
Makefile path. A temporary testkit-only build experiment omitted
the missing `vm_pdp6.c` dependency. The host linker then
reported three unresolved VM functions: `vm_extent_move`,
`vm_space_can_swap`, and `vm_space_destroy`. These production
functions belong to the PDP-6 VM implementation, including
`vm_pdp6.s`, and host C tests cannot link the assembly directly.
The fixture asserts actual movement, pinning, live mapping and
swap-destruction behaviors; no dummy symbol implementations
would be valid acceptance evidence. The testkit Makefile was
restored byte-for-byte from a temporary backup, preserving
all unrelated pre-existing testkit edits. A proper test repair
requires an equivalent host implementation with verified semantics
or a PDP-6-native MM fixture. No production code or extent
registry limit changed; baseline remains 025065 octal KCORE.

### Proposal D extent registry acceptance gate v247 (2026-10-24)

The PDP-6 MM implementation has `MM_MAX_EXTENTS=040` (32)
two-word descriptors, occupying 0100 octal (64) resident words.
`mm_extent_insert` explicitly returns MM_ERR_DESCRIPTORS (-3)
when full; shrinking the registry would change real failure behavior.
No representative runtime high-water readings are available here,
so do not reduce this count merely to reclaim static words.

The daimos-testkit `mm-core` target currently fails to compile
before test execution: its Makefile references removed/missing
`system/kernel/proc/vm_pdp6.c`; the checkout contains
`vm_pdp6.s` and `vm_pdp6_swap.c` instead. The testkit checkout
also contains unrelated pre-existing working-tree changes; these
were preserved. Repairing the fixture requires identifying the
correct host-side VM implementation/link requirements, not simply
replacing the path with assembly. No source changes or new
permanent-memory savings were accepted.

### Proposal F static allocator and VM lifetime inventory v246 (2026-10-24)

Following the committed v245 filesystem scratch audit, inspected
resident BSS declarations in MM, MEMFS, IPC, PROC and VM. MM's
`mm_extents` is 0100 octal words (64 decimal) for its live
extent descriptors, not a disposable scratch buffer. MEMFS
`memfs_data_chunks` and `memfs_xip_refs` track allocator
and XIP ownership; the separate VM APR word and IPC/PROC
singletons hold live state. None can be aliased safely to the
128-word filesystem transfer workspace without first proving
non-overlapping use and ownership. No production change is
proposed; retain the v245 GCC lowmem KCORE baseline 025065
octal (10805 decimal). Measure peak extent occupancy before
any MM descriptor count reduction.

### Proposal F filesystem workspace ownership check v245 (2026-10-24)

Inspected resident filesystem static allocations and provider
serialization for possible buffer lifetime overlap. The only
128-word resident VFS transfer block is already `fs_block_workspace`.
DTFS, TSFS and D6FS share it, and `fs_provider_serialized` holds
the provider lock over potentially sleeping physical I/O. Arguments
are held on private process stacks during lock waits. MEMFS bypasses
that serialization because it does not sleep or use this block.
Reusing the transfer block for unprotected path/lookup scratch
would permit an unrelated process to corrupt an in-flight request.
The remaining assembly storage declarations inspected in this
area are small state variables or MEMFS-owned allocations rather
than obvious duplicate permanent transfer blocks. No safe
ownership-preserving static overlay was identified, and no source
change or new saving is claimed. Retain v244 KCORE baseline 025065
octal (10805 words); revisit only with a documented lifetime proof.

### Proposal B file-path request-frame inspection v244 (2026-10-24)

Audited the resident `file_symlink`, `file_rename`, `file_chdir`,
`file_make_node`, `file_remove`, `file_truncate` and
`file_stat_path` paths together with `vfs_create_common` and
`vfs_create_op`. MKDIR/MKFIFO already share their parent/access
frame, UNLINK/RMDIR share a typed removal frame, and the one-argument
TRUNCATE/STAT wrappers already share stack cleanup. The CREATE
provider request and returned-vnode publication are already common
for the relevant operations. A new cross-operation frame would have
to preserve distinct permissions, argument lifetimes and stack
offsets, with no identified positive linked-size candidate. No
production source changes were made.

`test-daimos-vfs-stat-frame` and `test-daimos-pre-dsh-fs-closure`
PASS with GCC. Retain v243 KCORE baseline 025065 octal (10805
decimal). Continue looking for a concrete full-image reduction,
without touching deferred JRST folding or 32K acceptance.

### Proposal B descriptor and GETCWD clear follow-up v243 (2026-10-24)

After v242, the descriptor duplication paths both use bounded BLT
and the mount-extension replacement operation is shared. Examined
the variable-length output zeroing in `file_getpath_common` and
its `file_getcwd_clear` loop. A prospective BLT-based fill needs
additional pointer, destination and end-address preparation and
would add overlap/lifetime complexity. The exploratory source edit
was reverted immediately without accepting or benchmarking it.
No production code change and no new RAM reduction was claimed.
Do not force an additional dispatcher or BLT on this short loop
without a positive complete-link result and path regression.

### Proposal B DUP2 descriptor pair copy v242 (2026-10-24)

`file_dup2` now uses the same bounded two-word BLT as `file_dup`
to copy the packed vnode/metadata and offset. Its subsequent pipe
reference operation obtains the packed metadata from the completed
destination descriptor, rather than relying on the AC3 scratch
value from the former scalar-copy sequence. This retains the
descriptor-copy, mount-extension, pipe reference and old==new
semantics without extra resident state or stack.
Full linked GCC lowmem disk-root KCORE 025066 -> 025065 octal
(10806 -> 10805 decimal), KCORE+MRES 13669 -> 13668,
PERMANENT_LAST 13716 -> 13715. The existing native PDP-6
`daimos-file-mountext-v19` fixture PASSes 26-mount vnode, lock,
DUP/DUP2, close and unmount; file and file-walk tests both PASS.
Strict 32K acceptance and JRST folding remain deferred.

### Proposal B contiguous FILE descriptor duplication v241 (2026-10-24)

`file_dup` copies the packed descriptor node and offset from two
adjacent words. One bounded two-word BLT replaces four MOVE/MOVEM
instructions with a three-instruction pointer setup and BLT, without
changing destination extension replacement, lock-family semantics,
or pipe reference handling. No resident data or stack increase.
GCC lowmem disk-root full linked KCORE 025067 -> 025066 octal
(10807 -> 10806 decimal), KCORE+MRES 13670 -> 13669,
PERMANENT_LAST 13717 -> 13716. The existing daimos-testkit
`daimos-file-mountext-v19` PDP-6 test PASSes its 26-mount vnode,
lock, DUP/DUP2, close and unmount assertions; target-file-pdp6 and
target-file-walk-pdp6 PASS. No JRST folding or 32K acceptance work.

### Proposal B file mount-extension bitmap update v240 (2026-10-24)

`file_mount_ext_store` and `file_mount_ext_copy` previously duplicated
the final two-bit descriptor mount-extension clear/set sequence. Both
paths now enter `file_mount_ext_update` with the destination bit offset
in AC7 and already-positioned replacement bits in AC6. Clearing
outdated destination bits precedes ORing the replacement, as before.
The canonical vnode, descriptor layout, AC1/AC2 preservation and
zero-to-clear behavior remain unchanged. This adds no resident
state or stack frame.

Full linked GCC lowmem disk-root KCORE: 025075 -> 025067 octal
(10813 -> 10807 decimal). KCORE+MRES 13676 -> 13670;
PERMANENT_LAST 13723 -> 13717. PDP-6 target-file-pdp6 and
target-file-walk-pdp6 PASS. Specific full native descriptor mount
extension lifecycle verification remains open; the size result
does not imply that broader acceptance.

### Proposal B provider boundary and leaf-loop audit v239 (2026-10-24)

The v238 follow-up reviewed `vfs_lookup_provider`,
`vfs_parent_return`, `vfs_create_op`, `vfs_symlink` and MonitorFS
`mfsleaf_lookup`/`mfsleaf_readdir`. CREATE/MKDIR already share the
request formatter and SYMLINK already shares vnode publication.
Lookup must additionally resolve mounted targets, unlike PARENT.
MonitorFS leaf lookup must retain the requested count across the
entire loop; AC5 is reused for packed-name comparison, so naively
hoisting the length load would be incorrect. That source experiment
was immediately reverted without a misleading size claim.
No ABI-safe larger common routine was identified, and no production
code or tests were changed. Preserve the 025075-octal KCORE baseline;
further investigation should target a concrete duplicated data
lifetime or full provider request frame, not an extra dispatcher.

### Proposal B VFS mutation epilogue experiments v238 (2026-10-24)

From the clean v237 checkout (KCORE 025075 octal, 10813 decimal),
two cold error-path sharing candidates were compiled and measured
against the complete GCC lowmem disk-root image. Redirecting
`vfs_setattr` errors through the existing three-word unwind
`vfs_mutate3_ro` was size-neutral at 025075, so it was reverted.
Redirecting `vfs_symlink` and `vfs_rename` four-word failure
unwinds through `vfs_write_words_ro` **increased** the full linked
KCORE to 025077 (10815 decimal), KCORE+MRES to 13678 and
PERMANENT_LAST to 13725. It added an extra cold-path branch and was
also reverted. The linker/folder already removes or shares more than
the source-level instruction count suggests. No production changes
were accepted and no native runtime acceptance is claimed for these
discarded experiments. Continue proposal B with larger structural
opportunities rather than tiny branch/epilogue rewrites.

### Proposal B MonitorFS namespace BLT copies v235 (2026-10-24)

Two fixed three-word name-record copies in resident VFS, one in
`vfs_readdir_table` and one in `monitorfs_name_record`, now use
bounded PDP-10 BLT transfers instead of three MOVE/MOVEM pairs.
The source/destination record layout and length are unchanged.
Full GCC lowmem disk-root image: KCORE 025103 -> 025075 octal
(10819 -> 10813 decimal); KCORE+MRES 13682 -> 13676 and
PERMANENT_LAST 13729 -> 13723. No new static data or stack.
Both target-file-pdp6 and target-file-walk-pdp6 PASS on PDP-6.
The preceding one-branch provider fallthrough experiment was
size-neutral after linking and reverted. Further native directory
enumeration and getcwd acceptance remains useful. The native
test-daimos-monitorfs-devices-online on SIMH also PASSes with
COMPILER=gcc and SDL_VIDEODRIVER=dummy.

### Proposal B bulk device argument consolidation v233 (2026-10-24)

The resident file-runtime bulk READ/WRITE dispatch previously reloaded
the buffer pointer and word count separately in ten individual device
leaves (six WRITE, four READ). Both dispatchers now save their canonical
vnode in AC0 and establish the device-call arguments once before selecting
the device. The ordinary VFS provider fallback restores the vnode and
original word offset before calling VFS, preserving descriptor advancement,
append handling, provider ABI and the mapped-buffer contract. No data,
extra stack slots, or provider dispatch layers are added.

Full GCC lowmem disk-root linked measurement: KCORE 025115 -> 025103
(10829 -> 10819 decimal), KCORE+MRES 13692 -> 13682 and PERMANENT_LAST
13739 -> 13729: ten permanent words saved. PDP-6 target-file-pdp6 and
target-file-walk-pdp6 PASS. Native PTR/PTP test compilation completed but
SIMH terminated with status 139 after SDL2 window-event warnings. This
is **not** a native device I/O acceptance pass; separate device validation
remains open. A separate native card WORDTOKEN12/CR/CP round-trip test
PASSed with SDL_VIDEODRIVER=dummy. No changes to JRST folding or the
deferred 32K gate.

### Proposal B CHDIR stat reuse v232 (2026-10-24)

`file_chdir` already obtains a seven-word `vfs_stat` frame to check that
the target is a directory. Its subsequent `file_check_access` call repeated
`vfs_stat` on the same vnode. Use `file_access_stat` directly on that
validated frame for the same search-permission mask (1). This removes a
redundant provider STAT invocation from successful CHDIR, with no ABI,
permission-policy or persistent-state change. GCC lowmem disk-root full
link PASS: KCORE `025115` (10829 decimal), KCORE+MRES=13692; size-neutral.
`daimos-testkit target-file-walk-pdp6` PASSes on PDP-6 including CHDIR.
This is a runtime-path simplification rather than a KCORE shrink. Further
proposal B work must find actual text/data reduction.

### Proposal B MonitorFS namespace/classifier audit v231 (2026-10-24)

Reviewed MonitorFS device lookup, directory enumeration and STAT. They
already share `mfsdev_name_length`, `mfsdev_validate_id`, leaf table
lookup/readdir and (since v230) `mfsdev_io_type`; lookup and enumeration
retain distinct return layouts and device-name matching requirements.
Tested making character devices the default in `mfsdev_io_type` and
checking mount/block exceptions first. The lowmem full link saved only
**one word** (`025114`, 10828 decimal), while adding instructions to the
ordinary character-device path. Rejected the tradeoff and reverted it;
restored full link PASSes at `025115` (10829 decimal), KCORE+MRES=13692.
No production source change was retained. Next proposal B work should
concentrate on larger file/VFS semantics or duplicated provider operations,
not device-classification instruction rearrangements. Dedicated native
STAT/READDIR type-matrix coverage is still desirable.

### Proposal B MonitorFS shared device classification v230 (2026-10-24)

`mfsdev_stat` independently duplicated the device-type classification
already implemented by `mfsdev_io_type` for READDIR. It now passes the
device ID in AC0, calls that existing classifier, and uses returned AC7
as the STAT type. No new data or provider dispatch is introduced. GCC
lowmem disk-root permanent-size link PASS: KCORE `025115` octal (10829
decimal), KCORE+MRES=13692, PERMANENT_LAST=13739 decimal, **10 words**
below the v229 accepted baseline. Native MonitorFS device tests remain
required before calling the change runtime accepted. The daimos-testkit
`test-daimos-monitorfs-devices-online` native SIMH test subsequently
**PASSed** using the supported `COMPILER=gcc` configuration and the unchanged
DSH executable size gate. This confirms the online device view; a separate
STAT/READDIR mode/type matrix remains useful to directly exercise all
device-classification categories.

### Proposal A process-table scan census and next boundary v229 (2026-10-24)

Current `proc_pdp6.s` has distinct hand-written scans with different
contracts: `proc_has_live_user` checks only an active state, whereas
`proc_tty_session_has` excludes a slot and conditionally checks pgrp,
u-area presence and session. `proc_trim_high` traverses backward only over
FREE trailing descriptors; `proc_runq_add/remove` operate the intrusive
runnable queue. Sharing these through a general scan helper would add
calling convention, predicates and/or persistent state to short fast paths;
there is no measured benefit. **Do not implement a generic iterator.**
The existing boot-selected process capacity and three-word descriptors
remain the baseline; no new process metadata is proposed.

After the failed common teardown prototype (v228), proposal A has no
currently justified large source-level rewrite. Treat A's first pass as
measured but not exhausted forever; revisit only with a concrete duplicate
instruction block or demonstrable lifetime/state simplification. Turn to
proposal B (file/VFS/MonitorFS) next rather than spend time on one-word
instruction substitutions. No source change or additional saving at v229.

Initial B measurement from the current available compiled lowmem objects
(section totals **before** final link/folding; not additive potential):
`file_runtime` text=1175, `monitorfs_runtime` text=1565 data=39,
`vfs` text=778 data=27 BSS=128 words. Prioritize the MonitorFS dispatch
surface and file permissions/lookup interactions, but preserve provider
semantics and measure the post-fold whole-image result before editing.

### Proposal A teardown sharing investigation v225 (2026-10-24)

### Proposal A discard assembler prototype v226 (2026-10-24)

### Proposal A supported GCC native rollback verification v227 (2026-10-24)

### Proposal A shared post-VM cleanup experiment v228 (2026-10-24)

Tested one common `proc_cleanup_after_vm` routine to serve both normal EXIT
and failed RUN slot discard after successful `vm_space_destroy`. It shared
TTY session release, u-area release and runnable-queue removal while keeping
distinct VM transitions and process publication. The GCC lowmem disk-root
link **grew** from v226/v227 KCORE `025127` (10839 decimal) to `025136`
(10846 decimal), an increase of seven resident words; KCORE+MRES likewise
grew from 13702 to 13709 words. The additional calls and register saves
outweighed eliminated local operations. **Rejected and reverted**: no
production code change retained and no native test claimed for this
discarded experiment. Do not attempt the same wrapper without a new
measurable reason to expect savings. Further A candidates must eliminate
actual work or data, not merely factor assembly source.

The production DSH size gate of `036000` octal must remain unchanged.
Investigation found the default KCC userland fixture generated DSH `036013`,
but DAIMOS already documents `COMPILER=gcc` as the supported smaller-DSH
policy for low-memory operation (`design/dsh-compiler-size-comparison-20261012-v20.md`).
Without bypassing the gate, reran daimos-testkit's
`test-daimos-run-late-rollback` with inherited `MAKEFLAGS=COMPILER=gcc`.
Native target **PASS**: 16 post-VM RUN descriptor-map failures under three
live children, stable process accounting and successful recovery. This
validates an important failure path of the v226 assembler discard replacement,
not every termination, signal, hierarchy or RT-ownership case. The separate
full process-lifecycle regression is tracked independently.

The independent `test-daimos-process-lifecycle` was also rerun with the
supported GCC configuration and **PASSed** native SIMH execution: RUN,
GETPID, WAIT/nohang, zombie reap, orphan adoption and explicit descriptor
maps. Neither test required a DSH image size override. This lifts the
previous general lifecycle acceptance blocker for these covered paths,
but does not prove RT-owner failure injection or all VM teardown faults.

Replaced the production KCC-generated `proc_slot_discard` with a dedicated
PDP-6 assembler implementation, guarded by `PROC_SLOT_DISCARD_PDP6_ASM` in
the C fallback build. It preserves separate discard semantics: FREE early
return, conditional RT owner release, transition setup and failure rollback,
VM destruction, TTY/u-area release, run-queue removal, descriptor zeroing
and trailing slot trim. This is NOT yet a shared teardown middle with EXIT.
The lowmem disk-root permanent-size build PASSes: KCORE=`025127` octal
(10839 decimal), KCORE+MRES=13702, PERMANENT_LAST=13749 decimal. Net
post-link saving versus v223: **two words**. No persistent data added.

The targeted native RUN late-rollback fixture was attempted but stopped
before executing: DSH `036013` exceeded the unchanged production `036000`
EXEC image gate. Thus the prototype is linked and size-measured, but native
RUN rollback and resource ownership remain unaccepted. Fix the independent
DSH image-size problem, or use a genuinely independent targeted fixture,
before claiming native completion. Do not bypass the gate.

Source comparison identifies a real, but not yet safely merged, code overlap:
`proc_slot_discard` in `proc/proc.c` is a production KCC-compiled resident
function (`build/system/boot/pdp6/kcore-asm/proc.s`), whereas
`proc_finish_slot` is handwritten in `proc/proc_pdp6.s`. Both release RT
ownership, set process TRANSITION, call `vm_space_destroy`, release session
TTY ownership and u-area, remove the runnable slot and trim process slots.

The generated `proc_slot_discard` is about 50 assembly instructions in the
inspected build, including save/restore and local labels. This is *gross*
code to investigate, not 50 words of recoverable savings. Its implementation
differs from finish-slot materially: discard checks FREE, does not perform
session-leader HUP teardown or adopt children, and frees the slot instead
of publishing ZOMB. Preserve VM-failure transition clearing and the exact
MM/u-area release order. `proc_exit_finish` is a separate small generated
wrapper; fatal-event termination can enter `proc_finish_slot` directly.

Candidate A architecture: a single carefully delimited teardown *middle*
operating on a validated slot and explicit caller contract, with distinct
normal-exit and failed-creation publication tails. First measure whether
replacing the generated discard path with an ABI-compatible assembler
implementation can reduce the full post-fold KCORE without an extra hot-path
dispatch or more permanent state. Do not merge paths by inferring lifetime
semantics merely from similar instructions. Native lifecycle/RT/failed-RUN
injection acceptance is a prerequisite. No source change or saving is
claimed from this inventory.

### Proposal A lifecycle/RT audit v224 (2026-10-24)

### Proposal A cleanup ownership and rollback v225 (2026-10-24)

Inspected three distinct cleanup entries: `proc_finish_slot` (normal/fatal
termination), `proc_slot_discard` (failed creation) and
`proc_event_fatal_other` (closes the target's file table before finishing).
Although all involve VM/u-area/runq release, their ordering and ownership
requirements differ. No shared lifecycle state machine or MRES dispatch
has been introduced; a new wrapper is not justified without actual net
post-fold saving and proof of failure behavior.

Existing daimos-testkit host `proc-abort-transition-v40` PASSes against
current `proc.c`, including held TRANSITION during VM destroy, failed
destroy rollback, successful retry and slot reuse. The first invocation
could not find `pdp10-sixbit.h`; a rerun with `CPATH` pointing to the
existing DAIMOS `tools/host` headers PASSed. No test source changes.
This is focused C host coverage, not native PDP-6 assembler acceptance.
The DSH image gate continues to block the full lifecycle simulator fixture.


The RT owner release at the beginning of `proc_finish_slot` is intentional
in the existing C fallback (`proc.c`) as well as PDP-6 assembler. The same
early release is used by `proc_slot_discard` before VM destruction. This
ordering cannot be unilaterally changed in assembler as a size optimization:
failure-path policy must be investigated in both paths and with the event
fatal-process caller. No RT rollback bug is established by code inspection
alone, and no behavior change was made.

The v223 runtime verification attempt using daimos-testkit
`test-daimos-process-lifecycle` failed at image construction, before any
native process assertions: DSH `036013` exceeds the unchanged `036000`
EXEC gate. This repeats the independent v222 fixture blocker; it is not
a pass or evidence of a new process-management failure. A dedicated
failure-injection test of RT ownership during teardown remains open.

Further A work should target duplicated resource acquisition/release
mechanics across process exit, failed slot creation and event termination,
while preserving their distinct ownership and commit points. Do not add a
common service wrapper without proving lower post-fold memory consumption.

### Proposal A first source experiment v222 (2026-10-24)

### Proposal A process-finish RT ownership v223 (2026-10-24)

Replaced the four-instruction `MOVE/CAME/JRST/SETZM` real-time-owner
conditional in `proc_finish_slot` with `CAMN/SETZM`. This preserves the
ownership test and termination order without a new branch or storage.
The GCC lowmem disk-root `permanent-size` link passes with KCORE
`025131` octal (10841 decimal), KCORE+MRES=13704 and
PERMANENT_LAST=13751 decimal words; net reduction from v222 is two
resident words. A native process termination / RT-owner regression still
must be run; linker success is not runtime acceptance. Architectural
cleanup remains the priority over further micro-optimizations.


`proc_adopt_children_pdp6` now uses `CAIN` to branch directly for the
INIT-parent case, eliminating the former `CAIE/JRST/JRST` arrangement.
The production lowmem disk-root `make permanent-size` completed and reports
KCORE `025133` (10843 decimal words), one word below the previously
recorded `025134` baseline; KCORE+MRES is 13706 words in this build.
The lowmem map is profile-dependent, so do not compare it to the stale
full-profile map in another build directory.

Native process-hierarchy regression was attempted via daimos-testkit, but
its fixture build stopped at the *unchanged* strict DSH image size limit:
`036013` > `036000` octal. It did not execute the runtime assertion stage.
Do not call the behavior native-accepted; keep the production DSH size gate
unchanged. The next A step remains a measured call-site/lifetime audit of
process cleanup, signal/session and TTY responsibilities.

Measure total physical RAM (KCORE + MRES + dynamic allocations) first, and
the permanent KCORE endpoint separately. Execution cost is nearly as
important. None of these candidate savings is yet measured or additive.

1. **A: process-management decomposition.** Keep interrupt, scheduler,
   context-switch, runnable-queue, essential sleep/wakeup and exit-state
   paths in KCORE. Audit child hierarchy, terminal/session management,
   signal policy, wait/reap and resource teardown for duplicate cold logic.
   Eliminate code before considering MRES relocation; do not put another
   dispatch on the context-switch or tick path.
2. **B: file/VFS/MonitorFS consolidation.** Share repeated validation,
   exceptional dispatch and error handling where actual paths overlap;
   preserve existing provider ABI, direct fast paths and compatibility.
3. **C: EXEC/RUN consolidation.** Loader code is already shared. Fold only
   further proven duplicated validation and startup paths, while retaining
   distinct resident EXEC replacement and RUN publication transactions.
4. **D: MM architecture.** Preserve MM-authoritative ownership/pinning;
   profile real fragmentation and extent high-water before redesign. The
   current 32 two-word descriptors total only 64 words.
5. **E: packed inactive processes.** Long-term total-RAM candidate; assess
   registry, codec, stack and swap costs; retain disk swap until verified.
6. **F: scratch lifetime sharing.** Reuse nonoverlapping buffers only with
   proved reentrancy/ownership and a positive measured total-RAM benefit.
7. Revisit assembler/compiler cleanup and then additional JRST folding only
   after architecture opportunities are exhausted (subject to >20-word gate).

Proposal A initial source inventory (not an implementation):

- `system/kernel/proc/proc_pdp6.s` is one mixed assembly unit, with
  2164 text, 3 data and 3 BSS words in the available
  `build/system/boot/pdp6/proc_pdp6.dobj` (pre-fold, not a freshly
  rebuilt v221 object). Its size is NOT solely scheduler cost.
- Scheduler selection is near source lines 120-315, runnable queue near
  589-640, sleep/wakeup near 2345-2541 and PI/context-switch/timer work
  near 2597-2944. These are likely hot or latency-sensitive paths.
- Child adoption and finish/reap, u-area/session release and wait/status
  occur near lines 329-1024. Events/signal control and TTY/canonical
  input/output occupy substantial additional code in the same object.
- The current map exports `proc_finish_slot`, `proc_wait_status`,
  `proc_event_apply`, `proc_tty_canon_input`, `proc_sched_pi_resched` and
  `proc_swap_victim`; no cold-path move can yet be declared safe.
- Next: construct an entry/call-site graph including PI/indirect paths,
  then identify *one* duplicated internal routine or policy path to simplify.
  Compare full post-fold RAM and native scheduler/process/TTY tests; do not
  implement dispatch-only movement which merely hides memory in MRES.

## Acceptance and documentation

- [ ] For every candidate record post-p10fold KCORE text/data/BSS, total
  permanent/MRES RAM, boot dynamic words, MM descriptor high-water, peak
  process RAM, worst-case cost and actual maximum concurrency.
- [ ] Update PROJECT-STATE.MD and pdp10-doc's resident limits, GCC/KCC build
  policy and dated 32K acceptance evidence only after corresponding tests pass.

## Completed items removed from the active backlog

The current source already provides boot-selected process-slot tiers starting
at 24 (`proc/proc.h`, `proc_boot.c`), a compact three-word process descriptor,
an intrusive runnable queue, a word-only eight-word pipe, a p10fold build
path, and an explicit strict GCC 32K size gate. These are baselines to test
and optimize, not absent features to implement again. The v33-v45 packed
registry/transaction components are staged prototypes, not production KCORE.

### Proposal B native PTR/PTP acceptance v237 (2026-10-24)

Re-ran `daimos-testkit test-daimos-pt-wordio` using the supported GCC
userland configuration and `SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy`.
Native SIMH **PASS**: PTR/PTP WORDTOKEN8 full and partial physical
round trip. The earlier status-139 window-event failure is not reproduced
with the headless SDL configuration. This strengthens native acceptance
of the v233 bulk-device argument consolidation without changing any kernel
source, persistent data, or the 036000 DSH image limit. KCORE remains
025075 octal (10813 decimal), KCORE+MRES=13676, based on the prior
full lowmem link. Continue proposal B on file/VFS structural savings.

## 2026-11-09 v1379: final KCORE work order

KCORE optimization is the LAST KCORE development stage.  Resolve all
compiler/assembler/kernel correctness blockers first, complete grouped
D6CODEC cutover, BSTORE/BCACHE/LOGSTORE, privileged XIP EXEC/RUN, and
opt-in compressed swap, then pass full integrated and failure acceptance.
Only afterward revisit resident-word reduction, EXEC/RUN consolidation,
JRST folding and final low-memory size gates.  RAM budgets remain active
during functional work, but no new size-only sweeps run concurrently.
TSFS v1.2 sharded metadata is separate, post-KCORE work before 0.9.
## 2026-11-08 v1371: full normal grouped distribution acceptance

The actual normal boot Makefile root manifest (237 files, 29 dirs,
89 DXRs) now packs and boots with the v1367 grouped KCORE and all
27 coherently linked MRES packages. All 71 D6LZ36-compressed
program inputs are converted and independently roundtripped as
grouped DXR2, preserving entry/text/relocation metadata; 18 stay
uncompressed. The complete D6FS volume is CLEAN with 336 FCBs.
Interactive PDP-6 SIMH RUN of grouped ECHO, LS, CAT, GREP, WC and
STAT, grouped LOGIN/DSH, and LOGIN respawn PASS. Native toolchain
files are present but not yet execution-tested. KCORE remains the
same +256-text reserved-slot grouped candidate; production D6LZ36
is untouched. `design/d6codec-full-distribution-v1371.md`.
## 2026-11-08 v1370: grouped KCORE actually boots and runs programs

The isolated grouped KCORE with a non-executable 40-word legacy
low-core reservation, plus all 27 grouped-map MRES packages,
**boots to INIT, LOGIN and DSH and executes grouped ECHO** on a
real D6FS/DSK270 root under native PDP-6 SIMH. Grouped compression
also drives the Stage-1 KINIT boot stream. Complete KINIT embeds the
grouped KCORE word-for-word at `030037..053240`. An independent
original D6LZ36 KCORE control fails to RUN grouped LOGIN, proving
real codec selection on the same root content. No resident codec
size was changed in v1370: +256 linked text words over production
remain for the fixed-layout grouped candidate. Full distribution,
runtime PURE/swap, TSFS media, VFS fault and nested kernel stack
tests remain required. Details `design/d6codec-fullboot-v1370.md`.
## 2026-11-08 v1369: grouped EXEC/PURE/TSFS functionality

The grouped DXR2 test writer preserves entry, TX2 metadata and
relocations. Native PDP-6 tests execute the real `exec_load_process`
and `proc_swap_reload_pure` body using a compiled program that runs
after grouped VM loading, a larger current linked DXR with short
VFS refills, and malformed/provider-error VM rollback. Native
`tsfs_read_words` decodes and copies a grouped 0400-word restart,
freeing temporary memory on success and error. All v1369 test
cases PASS with controlled VFS/VM providers. Real cold boot, file
providers and runtime media migration remain OPEN. The v1368
+216/+256-word linked KCORE increase is unchanged.
## 2026-11-08 v1368: full KCORE and MRES grouped link measurement

The complete KCORE linker reproduces production output byte-for-byte
at 9,601 text/269 BSS. Replacing the runtime frontend with grouped
v1367 and regenerating p10fold yields **9,817 text (+216)** when
the old low-core 40-word image is removed, or **9,857 (+256)** with
its physical range reserved to preserve early EXEC addresses. BSS
is unchanged. Re-linking SWAP, DTFS/TSFS, D6FS, MonitorFS and tape
MRES packages against either grouped map PASSes with unchanged
resident sizes; KCORE RIM and initialized-stream conversion PASS.
The grouped layout still has a substantial RAM penalty and has NOT
been booted as a complete KINIT or used by real VFS/EXEC/TSFS.
See `design/d6codec-kcore-link-v1368.md`. Production is unchanged.
## 2026-11-08 v1367: grouped VFS memory reduction, still staged

Grouped decoder/adapter text drops from v1366 333 to v1367 **317**
words (225 engine + 92 VFS frontend). No static BSS is added.
Reusing the descriptor cursor during ingestion reduces state to 25
words and the temporary VFS frame to 67 words; the leaf-provider
stack canary reaches `0107` octal words. Full native VFS, maximum
descriptor and current-build KCORE/MRES streaming tests are staged.
The remaining **+220 resident words over legacy D6LZ36** are not
accepted under the RAM-first KCORE target. No production codec,
image or permanent memory has changed. See
`design/d6codec-vfs-ram-v1367.md`.
## 2026-11-08 v1366: staged grouped VFS decode and residency budget

The grouped streaming PDP-6 engine and VFS/buffer ABI now PASS
native fake-provider bounded/short-read/trusted backing and error
tests, preserving AC10..AC16. The 32-word refill window plus 26-word
continuation state yields a 68-word temporary frame (legacy 72).
The leaf-provider canary reaches 0110 octal stack words, not a real
provider-nested worst case. **New code totals 333 text words vs
97 old; +236 resident words before folding**. Do not link or
activate the grouped runtime until RAM cost and nested cold-read
stack safety are accepted. Production D6LZ36 remains unchanged.
## 2026-11-08 v1365: staged grouped media boots to KINIT entry

Actual linked 32,670-word KINIT, grouped into 29,023 compressed words,
now reaches KINIT entry 053524 from all five simulated PDP-6 Stage-1
device loaders: DSK, DRM, DTC, MTC, two-volume PTR. The tests boot
through real Stage-0 paper tape and each corresponding virtual
controller. Both oversized-image and malformed-compressed-control
DECtape negatives halt before KINIT with unchanged output guard.
These are test-only staged boot images; permanent KCORE RAM and the
production D6LZ36 runtime/host format are unchanged. Actual
post-entry KINIT and VFS/EXEC/PURE/TSFS grouped cutover remain OPEN.
## 2026-11-08 v1364: staged KINIT loader physical-geometry guard

The generated grouped Stage-1 alternatives now test that the decoded
KINIT plus reserved stack and its separately staged compressed payload
both fit the configured PDP-6 memory profile before loading payload
words. All five loaders pass native guard tests under 32K and 256K;
DTC/PTR additionally pass 64K with custom stack reserve (12/12 native
tests, independent helper boundary suite PASS). PTR rejects first-tape
segment overflow before copying. The new staged helper adds no BSS
and no permanent KCORE RAM; production D6LZ36 is unchanged. See
`design/d6codec-stage1-geometry-v1364.md`.
## 2026-11-08 v1363: direct-linked grouped Stage-1 decoding

Stage-1 never uses NEED_INPUT while decoding KINIT: every loader
already buffers its complete compressed input. Test-only direct-
linked v053 decoder alternatives assemble for all five boot modes
without the fixed-low-core BLT; staged text sizes DSK 432, DRM 424,
DTC 229, MTC 195, PTR 224 words, with zero new BSS. Native PDP-6
tests exercise direct KINIT handoffs at load base 000060 with valid
and invalid grouped streams. This eliminates the need for the
230-word streaming v1361 *inside Stage-1 only*. Runtime VFS still
requires a resumable solution; production D6LZ36 and permanent
KCORE RAM remain unchanged.
## 2026-11-08 v1362: native streamed grouped codec corpus

The staged v1361 assembler decoder now passes 15 representative
native PDP-6 host-golden DXR prefix tests (KCORE, D6FS, MonitorFS,
SWAP, tape at refill widths 1, 7, 64). All reconstructed 128-word
prefixes compare exactly. Fixed-low-core relocation and full
boot/runtime integration remain OPEN; production D6LZ36 is unchanged.
## 2026-11-08 v1361: first native grouped resumable decoder

A staged, nonproduction PDP-6 assembler decoder now implements +1
NEED_INPUT across word-size refill windows with a 26-word caller
state (15 descriptors). Its native literal, split zero descriptor,
30-match stress, malformed/geometry and host-golden KCORE prefix
tests PASS. Object size is 230 text, zero data/BSS, 82 relocations.
It is not fixed-low-core relocatable/installed, C ABI safe or booted.
Production D6LZ36 remains unchanged; see
`design/d6codec-stage1-resume-v1360.md`.
## 2026-11-08 v1360: grouped low-core continuation host reference

An executable host reference demonstrates a grouped-stream continuation
that retains at most 15 descriptor words instead of buffering an entire
group. Six input-window sizes including a one-word refill pass against
27 present-build DXRs (162 successful runs) plus malformed streams.
Native low-core assembler, measured relocation/stack and all production
format migration are **not yet implemented**. See
`design/d6codec-stage1-resume-v1360.md`.
## 2026-11-08 v1359: current linked DXR codec host/native parity

The staged codec now has 27/27 present-build kernel/MRES DXR host
complete-stream roundtrips (19,270 input; 17,899 encoded 36-bit
words). Five native PDP-6 current-image prefix fixtures reproduce
the independent host streams exactly and decode them successfully:
KCORE, D6FS, MonitorFS, SWAP and tape (5/5 PASS). No claim is made
for the old unavailable 81-userland-DXR corpus or full 27-stream
native parity. Production D6LZ36 and resident memory remain unchanged.
## 2026-11-08 v1358: staged grouped-codec host golden parity

Native PDP-6 staged v053/v055 codec tests now reproduce the host
reference's bit-exact 128-word compressed output and decoded input
for configctl, darc and d6lz (3/3 PASS). The older 81-DXR host
manifest has stale absolute paths and cannot be counted as current
corpus acceptance. No production KCORE, boot or format change.
## 2026-11-08 v1357: staged codec error-path stack acceptance

The native v1357 failure-path suite covers malformed decoder input,
physical input wrap and encoder literal, zero-run and repeated-word
match-search capacity failures through the v055 wrapper. It verifies
the error return and the 512-word CRT canary depth of 016 octal
(14 decimal words). This agrees with the v1356 full-DXR stack
measurement but does not establish worst-case nested kernel stack.
No production KCORE source, D6LZ36 boot format or resident RAM changed.
## 2026-11-08 v1356: measured staged codec wrapper stack

The native PDP-6 full-DXR v1356 fixture confirms 14 decimal stack
words (`016` octal) with the v055 saved-register adapter, including
the CRT's initial call. This is not a worst-case KCORE nested-call
bound. Production D6LZ36 boot and KCORE remain unchanged; additional
failure-path and consumer stack tests are still necessary. See
`design/d6codec-cutover-entry-v1352.md`.
## 2026-11-08 v1355: staged D6CODEC worst-case output bound

An optional no-stack staged native helper computes a conservative
maximum encoded word count, N+ceil(N/30) (zero for N=0), rejecting
unrepresentable 18-bit physical buffer capacity. Native PDP-6
v1355 tests pass exact-capacity and one-word-short 30/31-token and
zero-run inputs, full roundtrips and boundary count checks. Earlier
v1353/v1354 bounds and the full-DXR v055 ABI corpus also PASS.
Active D6LZ36 KCORE/boot and permanent RAM remain unchanged.
See `design/d6codec-cutover-entry-v1352.md`.
## 2026-11-08 v1354: staged D6CODEC encoder geometry

The grouped v053 encoder now rejects 18-bit physical source and output
extent overflow and overlapping buffers before output stores. Native
PDP-6 testkit v1354 and existing complete v055 corpus pass, including
the expected negative witness on the previous staged encoder. Active
D6LZ36/KCORE boot and resident code remain unchanged. See
`design/d6codec-cutover-entry-v1352.md` for scope and outstanding
production cutover gates.
## 2026-11-08 v1353: staged D6CODEC decoder bound checks

The isolated v1352 D6CODEC integration branch now has a v1353 native
PDP-6 decoder-bounds and canonical-padding regression PASS, plus the
existing full-DXR v055 ABI/corpus PASS. The unchanged v1351 decoder
fails the new native negative witness. This is staged source only;
production boot/KCORE still uses D6LZ36. New source details and linked
pre-cutover size baseline are in
`design/d6codec-cutover-entry-v1352.md`. Production consumer migration,
stack measurement and exact image acceptance remain OPEN.
## Historical packed-process plan (cancelled v1375; not open)

The v33-v45 packed registry and compressed DISK-swap experiments were
cancelled by v1375. Raw disk/drum swap remains production; later
grouped block-codec migration is complete. Their old unchecked
requirements are removed from the active KCORE list. The separate
**uncompressed swapless inactive PURE** scheduler/MM/APR integration
remains open (v1521 item P1-E). Mutable-only in-RAM compression may
be evaluated **after** PURE with matched total RAM accounting.
