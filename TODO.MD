## CURRENT KCORE FEATURE TODO (authoritative after v1544, 2026-11-13)

This block supersedes historical dated *status claims* lower
in this file; retain those entries for acceptance evidence,
not as an additional set of currently open tasks.
KCORE **feature completion only**. No standalone MRES
development, generic userland work, optimization or 32K
shrink. VERSION 0.5; strict 32K/optimization gate HOLD.

### Accepted, do not reimplement

- [x] Root and secondary DTFS 72-bit physical identity
  checks on error and return; non-root physical failed
  WRITE and genuinely partial 36-bit WRITE remain
  sticky WRITE_UNCERTAIN; owner-retaining CLOSE/UNMOUNT;
  explicit root-only checked poisoned-slot/FD discard
  and different-volume administrative replacement.
  v1522-v1531, v1535 combined physical regressions PASS.
- [x] Boot-inventoried MM-backed dynamic mount capacity:
  FSINFO count query, occupied/empty/error distinction,
  MOUNTS/DF consumers, real seven-slot PDP-6 and live
  mount IDs 2..5 with holes/stable retire. v1492,
  v1532/v1533 PASS; a physical 26-volume machine
  was not demonstrated.
- [x] Mixed live TSFS root, independent native DTC3/DTC4
  DTFS and writable MEMFS under physical DTC3 failed
  write and matching-media return. Original DTC3
  quarantined; healthy files/read/write/retire PASS v1534.
- [x] **v1536** committed-snapshot KINIT BACKSTORE bitmap
  reservation: nine source-linked cases, four unsafe
  mutants rejected; actual PDP-6 allocator/free guards
  old process plus snapshot sector ownership, three
  unsafe mutants rejected. Fresh 128K physical MEMFS
  PERSIST cold two-boot data/XIP restore and untouched
  AUXSTORE descriptor PASS. No new production code.
- [x] SIMH physical guest CTY acknowledgments corrected
  to instruction-delayed console SEND; strict v1527,
  v1528, v1529, v1531 and v1534 regressions PASS v1535.
- [x] **v1538 root I/O fail-stop source integration:** D6FS
  read/write and TSFS metadata/STORED/BLOCK actual negative
  I/O on the selected mounted root reach v1515's root HALT.
  D6FS 11 native/5 mutants and TSFS 12 native/4 mutants
  PASS, plus prior native secondary isolation and regressions.
  This replaces permanent-device-loss detection as the ROOT
  trigger. Full 256K physical TSFS-root DTC0/DTC1/DTC2
  detachment plus required root data read HALTs at PC 003743;
  physical nonroot DTC3 write failure/return stays nonfatal.
- [x] **v1539 single-member TSFS root loss:** the running full
  256K KCC PDP-6 guest detaches only DTC0 of the selected
  DTC0/DTC1/DTC2 root set. Required uncached root-file I/O
  HALTs at the exact v1515 root PC 003743; the other two
  root members remain attached. No production code or RAM growth.
- [x] **v1540 disk-root physical loss:** real KCC PDP-6 D6FS
  DSK0..DSK3 root physically detached; required root data read
  reaches exact root HALT PC 003743 (testkit v1540 PASS).
- [x] **v1541 BACKSTORE critical-service fail-stop:** a valid
  active or still-owned BACKSTORE DEVICE/VOLUME I/O failure
  after bounded lower-driver recovery reaches v1515 reason 2
  BACKSTORE HALT. Inactive unowned boot probes and invalid
  requests do not HALT. Native PDP-6 17 cases/six mutants and
  tape-root KCC KCORE/MRES link PASS. No permanent state.
  This supersedes earlier final-negative-I/O scheduler retries
  from v1511/v1512; lower driver retries still run normally.
- [x] **v1542 BACKSTORE full-guest physical positive:** actual
  256K KCC PDP-6 Type-236 PI2 error completion for an owned
  BACKSTORE WRITE HALTs exactly at fixed swap reason PC 003747;
  counter 1, enabled 1 and octal 0320 owned blocks recorded.
  Four clean pre-error samples and nine corrupt-evidence
  mutations PASS. No new production source/RAM/ABI.
- [x] **v1543 DRM236 physical D6FS-root outage:** complete
  `ROOT=drum BOOT=drm` 256K KCC PDP-6 guest boots from
  D6FS DR0..DR3. Detach all selected physical members, read
  required root data, reach exact root HALT PC `003743`.
  No production code/RAM changes; corresponding DSK270
  disk-root v1540 already PASSes.
- [x] **v1544 actual D6FS->DSK/DRM direct adapter native
  integration:** 24 real PDP-6 cases/six source mutants PASS;
  selected root/secondary mount, direct and three-member sets,
  read/write/error and physical kind/unit/block accounting.
  No executable KCORE changes; stale comments corrected.

### Outstanding, strictly KCORE features

- [ ] **P0-A, remaining highest safety gate:** selected ROOT and
  active/owned BACKSTORE services HALT after an exhausted required
  I/O failure, not a speculative permanent-media-loss register bit.
  Physical TSFS all/one-root-member, D6FS disk-root, noncritical
  DTC3 and Type-236 owned BACKSTORE reason-2 HALT PASS through
  v1543. Both DSK270 and DRM236 root positive physical tests
  PASS. Source-linked integrated secondary DSK/DRM v1544 PASS;
  still qualify *real mounted* secondary DSK/DRM physical negative
  controls and shared DEVICE/VOLUME faults;
  source-linked native DSK270/DRM236 mapped coverage already PASS.
  Do not HALT for invalid requests, inactive/unowned probes,
  successful driver recovery or noncritical FS errors; never
  reconstruct or falsely release critical owners.
- [ ] **P0-B, media durability policy:** after a
  genuinely partial WRITE_UNCERTAIN non-root DTFS
  volume is explicitly discarded, prevent its later
  physical reinsertion/reboot from being silently
  admitted as a healthy filesystem just because its
  RBN0 identity/directory still validate. Define
  durable rejection/repair/reformat authorization
  without inventing successful old-data recovery;
  test old/same/replacement volumes across cold boot
  and distinct owner/cwd/PIPE/swap lifetimes.
  Optional strengthening: full physical multi-process
  administrative owner scan (native and separate
  cross-process guests already accepted).
- [ ] **P1-C, combined storage faults:** one actual
  full guest with concurrent process swap/restoration
  plus MEMFS persistent snapshot commit, and independent
  LOGSTORE work. Verify no overlapping BACKSTORE bitmap
  sectors, MM descriptors, pinned owners or stale BCACHE.
  Add in-flight DSK270/DRM236/DTC DEVICE vs mapped
  VOLUME provider loss/return, shared Type-136 routing,
  dirty/write-through failure and owner/PI retry
  rollback. Use prior v1484/1485/1490/1511-v1514
  gates, not duplicate storage layers. v1536 only
  establishes reservation and native allocation
  separately, NOT physical concurrency.
- [ ] **P1-D, alternate hardware inventories:** extend
  the real boot-inventoried mount table acceptance
  beyond seven slots with different actual controller/
  root configurations, empty/unpublished and provider
  failure semantics. Check high mount IDs as hardware
  allows. 26 is a *representation maximum*, not a
  demand to invent 26 physical devices. Existing
  1/4/8/17/26 native cases and live mount ID 5 PASS.
- [ ] **P1-E, uncompressed swapless PURE:** integrate
  staged inactive PURE omission into production
  scheduler/VM/MM/APR. Retain immutable original
  executable backing, own pinned writable suffix,
  reconstruct all omitted PURE text within aligned
  APR bounds before dispatch, and atomically roll
  back on RUN/EXEC/WAIT/EXIT, OOM, provider unmount,
  PI retry and insufficient memory. Native mutation
  and repeated physical pressure/owner tests mandatory.
  Ordinary physical disk/drum swap remains unchanged;
  isolated +180 TEXT shared-DXR prototype is NOT
  production. Optional mutable-only RAM compression
  is conditional on successful uncompressed PURE and
  measured whole-system peak RAM benefit.
- [ ] **Final KCORE feature freeze:** full current-source,
  matching-image PDP-6 native/physical matrix after
  P0/P1 closure; report exact fixed KCORE/MRES/boot
  state, failure/return, owners, PI and RAM footprint.
  Do not start optimization or 32K placement until
  feature completeness is demonstrated and authorized.

## 2026-11-13 v1535: physical-test CTY ACK transport

- [x] Diagnose v1527 READY_SWAP physical rerun stall:
  distinguish host-console handoff from a real DTFS
  controller/provider error. Guest ACK witness, CTY
  source read and SIMH `SHOW SEND` identify premature
  input before READCHAR; do not label timeout a
  permanent critical-device loss.
- [x] Replace back-to-back `CONT` plus raw monitor
  `A\r` with queued SIMH console SEND before CONT,
  delayed for 20 actual CTY output periods and spaced
  for two periods per character. No wall-time sleep,
  test-only kernel patch or physical assertion change.
- [x] Repeated 256K PDP-6 v1527 original/different
  physical return and v1528 sticky write-failure PASS.
- [x] Repeat v1531 owner-checked administrative media
  replacement and v1534 mixed-provider fault physical
  guests with the queued console delivery. Both PASS
  with unchanged KCORE/DTFS correctness assertions.
- [x] v1529 test-only physical Type-551 interrupted
  block WRITE: real RBN0145 contains one changed 36-bit
  word and one unchanged old word; host media oracle
  and PID1 sticky WRITE_UNCERTAIN PASS after same-media
  return with the revised CTY transport.
- [ ] Continue outstanding P0-A/B and P1-C/D/E
  feature gates.
  VERSION 0.5; 32K/optimization HOLD.

## 2026-11-13 v1534: mixed physical storage-provider fault

- [x] Actual full PDP-6 256K KCORE with live
  TSFS root, DTC3 and DTC4 distinct 72-bit-ID
  writable native DTFS and a writable MEMFS:
  verify real healthy file contents and FSINFO.
- [x] Warm DTC3 last, physically DETACH it
  and force a real metadata WRITE failure.
  Original DTC3 becomes WRITE_UNCERTAIN;
  same-media return cannot repair or unpoison.
  Healthy DTC4 native reads and fresh CHMOD
  WRITE, MEMFS contents and TSFS root stay
  available. Healthy mounts retire; poisoned
  FILE and mount remain. PID1/host PASS.
- [x] Restore stale v1522 native source-linked
  extractor after v1531 added another physical
  identity check. All eight executed unsafe
  mutations reject without production edits.
- [ ] P1-C other mixed hardware/device mapping,
  DSK/DRM swap backing, concurrent persistent
  LOGSTORE/MEMFS snapshot and multi-owner
  rollback remain OPEN. P0-A critical loss,
  P0-B durable poisoned-media admission,
  P1-D broader inventory, P1-E swapless PURE
  and final feature freeze unchanged.
  VERSION 0.5, 32K/optimization HOLD.

## 2026-11-13 v1533: real high mount slot accepted

- [x] Four separate, full 72-bit stamped native DTFS
  physical tapes on DTC3-DTC6 mounted together in
  real 256K PDP-6 TSFS-root guest: IDs 2,3,4,5,
  complete FSINFO path/provider and live inventory.
- [x] Retiring middle mount ID 3 leaves high mount
  ID 5 unchanged and readable by FSINFO; all remaining
  secondary mounts cleanly retire. Root and capacity
  stay stable. v1532 physical capacity regression PASS.
- [ ] P1-D remaining: distinct hardware inventory
  profiles and maximum representational capacity 26,
  which is not physically available in the current
  seven-slot DTC-root configuration.
- [ ] P0-A critical authoritative loss; P0-B durable
  discarded media admission; P1-C mixed faults;
  P1-E swapless PURE/MM/APR and feature-only freeze.
  VERSION 0.5; strict 32K/optimization HOLD.

## 2026-11-13 v1532: dynamic FSINFO query and physical scan

- [x] Preserve dynamic six-array KINIT mount slab and read-only
  FSINFO AC2=-1 capacity query. Reject uninitialized capacity;
  keep occupied=1/empty=0/error=-1 for ordinary slots.
- [x] MOUNTS/DF iterate returned capacity, never fixed four,
  and propagate provider errors. Source-linked PDP-6/native
  ABI test and exact C consumer tests 1/4/8/17/26 PASS;
  deliberate capacity-check mutation rejected.
- [x] Rebuilt 256K PDP-6 real TSFS-root KCC guest accepts
  7 boot-published slots, DTC3 physical mount/retirement,
  root provider, empty slots 2..6 and bounds.
- [ ] P1-D further physical acceptance: actual occupied
  slot beyond four, distinct hardware inventories and
  upper representational bound 26 (not implied by a
  seven-slot TSFS-root test).
- [ ] P0-A authoritative loss; P0-B durable old-media
  admission; P1-C mixed provider faults; P1-E swapless
  PURE/MM/APR; final full feature regression freeze.
  VERSION 0.5; 32K/optimization HOLD.

## 2026-11-13 v1531: explicit poisoned-DTFS replacement accepted

- [x] Implement root-only recovery UUO 077
  EXT054: explicit DROP_FD (selected local
  poisoned native DTFS FILE; no SYNC) and
  REPLACE_SLOT. Verify actual physical
  **different**, nonlegacy 72-bit-DTVOL1
  identity AND native directory before
  retiring an otherwise owner-free poisoned
  noncritical DTFS mount. Same original
  damaged medium always refuses. No
  in-place repair or silent poison clear.
- [x] Recheck remote/swapped FILE/cwd,
  PIPE and swap owners both before and
  after potentially sleeping physical
  provider validation. Native source-linked
  tests for stable 0410-word u-areas,
  post-I/O open race, poison-only FD drop,
  and four unsafe source mutations PASS.
- [x] Full real 256K PDP-6 DTC3 fault,
  same-ID refusal, root explicit FD drop,
  residual cwd denial, different-volume
  physical replacement, new mount, clean
  retirement and independent root PASS.
  Rebuilt v1526-v1529 physical regressions
  PASS. Existing v1514/v1523 source-linked
  test extraction corrected for new API.
- [ ] Optional strengthening: direct physical
  multi-process administrative replacement
  acceptance (native all-process owner scan
  and v1526 physical cross-process ownership
  are already separately proven).
- [ ] **P0-B durability:** decide and implement
  the required cross-reboot/physical-reinsertion
  policy for previously WRITE_UNCERTAIN media.
  v1531 retains no durable blacklist and
  a valid old RBN0 ID/directory alone cannot
  prove unchanged user data. Explicit discard
  requires keeping the old tape offline.
- [ ] **P0-A:** authoritative permanent
  critical TSFS root/BACKSTORE loss and
  precisely qualified HALT.
- [ ] **P1-C/D/E:** mixed provider faults,
  dynamic FSINFO capacity/inventory and
  swapless PURE/MM/APR, then final
  feature-only regression freeze.
  VERSION 0.5; strict 32K and
  optimization gate HOLD.

## 2026-11-13 v1529: genuinely partial physical DTFS write

- [x] Native PDP-6 source-linked empty-file
  one/two-word resize; independent physical
  Type-551 read/write of DTC3 RBN0145.
- [x] Compile-time simulator-only Type-551
  fault targeting a specific unit/block after
  one actual changed 36-bit word. Cover
  forward/reverse transfers, preserve errors
  across retries; use complete default SIMH
  platform CC flags and versioned local.tar.gz.
- [x] Full isolated 256K KCC PDP-6 root guest:
  create/write native FILE, externally verify
  RBN0145 seed, interrupt rewrite in mid-block,
  compare all 128 flushed physical words,
  require exactly one new value and one old,
  then reattach matching damaged 72-bit tape.
  Guest FSINFO, STAT, CLOSE, UNMOUNT refuse;
  root TSFS remains healthy. Target and host
  PASS; independent v1528 and v1527 physical
  regressions PASS. No mock provider.
- [ ] **P0-B remaining:** design and physically
  accept a separately privileged, explicitly
  checked administrative repair/discard path
  for sticky WRITE_UNCERTAIN. Require complete
  media validation/repair, and handle old
  FILE/cwd owners before retiring mount IDs.
  Never auto-clear on matching DTVOL1 alone.
- [ ] P0-A root/BACKSTORE permanent-loss
  classification; P1-C mixed faults, P1-D
  FSINFO capacity, P1-E swapless PURE/MM/APR;
  final feature regression freeze.
  Strict 32K / optimization HOLD.

## 2026-11-13 v1530: independent physical guest media

- [x] Fix overlapping v1525-v1529 physical
  guests overwriting the same prebuilt KCC
  DTC3 image and shared headless SIMH script.
  Use private DTC0-DTC3 tapes and a private
  boot do-file for every runner, including
  the v1527 additional DTC4 and replacement
  tapes. Do not alter the KCC build inputs.
- [x] Revalidate the 256K PDP-6 physical
  v1529 partial-write media oracle, v1528
  failed-write quarantine, v1527 DTC3/DTC4
  same/different-volume return and v1526
  independent FILE/cwd ownership. All PASS.
  No production code or RAM growth.
- [ ] **Next P0-B:** explicit checked
  WRITE_UNCERTAIN administrative repair/
  discard and retained FILE/cwd/PIPE/swap
  owner lifetime, without trusting only
  RBN0's full media identity. P0-A/P1
  outstanding feature gates unchanged;
  32K and optimization HOLD.

## 2026-11-13 v1528: KCORE WRITE_UNCERTAIN physical containment

- [x] Real booted 256K PDP-6 DTC-root KCORE
  + DTFS/TSFS MRES mounts correctly stamped
  native DTC3 RW, writes a real file and
  metadata, DETACHes physical DTC3, observes
  failed native metadata write, and ATTACHes
  original identical tape. FSINFO/STAT refuse
  the original mount after return. Repeated
  FILE CLOSE and ordinary UNMOUNT refuse
  instead of silently losing old ownership.
  Target and host v1528 gate PASS; Type-551
  CONO diagnostic repeat PASS. No production
  bytes or persistent RAM added.
- [ ] **P0-B:** prove a real partially completed
  DTC3 physical block/transaction WRITE,
  not only failure with media already detached.
- [ ] **P0-B:** implement and verify explicit
  administrator-checked uncertain-media
  repair/replacement and owner cleanup,
  retaining fail-atomic VFS slot protection.
  Do not clear poison because original 72-bit
  volume ID matches; do not silently release
  FILE/cwd, swap, or PIPE references.
- [ ] **P0-A/P1:** authoritative critical
  root/BACKSTORE loss, mixed storage faults,
  dynamic FSINFO capacity, swapless PURE,
  then final feature-only regression freeze.
  Strict 32K/optimization gate HOLD.

## 2026-11-13 v1527: verified physical DTFS read-fault return

- [x] Fix false BCACHE slot-0 identity after
  raw physical DTC3 RBN0 revalidation:
  `dtfs_identity_revalidate_v1522` now calls
  existing `bcache_workspace_invalidate`
  **before** raw I/O into shared
  `fs_block_workspace`. Do not discard
  independent clean slot 1. Only +1
  optional MRES instruction word, zero BSS
  or fixed KCORE data/text growth.
- [x] Correct v1522 source-linked fixture
  to use actual BCACHE state labels instead
  of duplicate test BSS. The real workspace
  tag is seeded; the raw RBN0 replacement
  must invalidate it and produce an actual
  miss. All seven older source mutations
  and the eighth stale-tag mutant rejected.
- [x] Complete **real** 256K PDP-6 physical
  DTC3/DTC4 media return: two independent
  stamped DTFS mounts, DTC3 real DETACH
  failure with old FILE/FSINFO refusal,
  unchanged DTC4 access, replacement by
  a different 72-bit stamped DTC3 and
  strict old-volume rejection, DTC4
  continued access, return of original
  DTC3 with full 72-bit identity
  revalidation, old FILE reuse and clean
  retirement of both mounts. Full guest
  `PASS`, v1526 cross-process regression
  `PASS` on the corrected MRES image.
- [ ] **P0-B next:** actual physical
  partial/failed DTC3 **write**, sticky
  WRITE_UNCERTAIN and an explicit checked
  administrative recovery policy. A matching
  volume ID must not silently clear
  uncertain partial write state.
- [ ] **P0-A:** authoritative physical
  permanent TSFS-root/BACKSTORE loss,
  precisely qualified v1515 HALT.
- [ ] **P1:** outstanding mixed-provider
  fault cases, v1492 dynamic FSINFO
  physical inventory, swapless PURE/MM/APR
  lifecycle and full feature regression
  freeze. No 32K optimization yet (HOLD).

## 2026-11-13 v1526: KCORE physical cross-process DTFS ownership

- [x] Real PDP-6 DTC-root KCORE/PID1 mounts
  stamped native DTC3, runs independent child
  via RUN V2 and synchronizes with two actual
  kernel pipes. The child alone owns cwd:
  UNMOUNT refuses. The child then owns only
  an open DTFS FILE after leaving cwd:
  UNMOUNT again refuses. Both failures preserve
  FSINFO provider 5. After child closes FILE,
  verified non-zombie child remains alive while
  unmount succeeds and FSINFO retires the slot.
  Child then exits and WAIT reaps it. The full
  physical test passes twice; v1523 kernel
  owner guards needed no changes.
- [ ] **P0-B next:** force an actual noncritical
  DTC3 physical READ fault while an old DTFS
  vnode/FILE remains live; verify fail-closed
  FSINFO/read/SYNC, authenticated 72-bit
  same-volume return, changed high/low ID
  rejection, and preserved open ownership.
- [ ] **P0-B:** sticky WRITE_UNCERTAIN after
  true partial/failed DTC3 write, and explicit
  administrator-checked recovery without
  stale directory/FILE reuse.
- [ ] **P0-A and P1:** unchanged remaining
  authoritative critical-loss, mixed-provider
  fault, FSINFO capacity and swapless PURE
  work; strict 32K and optimization HOLD.

## 2026-11-13 v1525: physically mounted DTFS feature state

- [x] Repair actual native directory layout mismatch:
  RBN0144 is reserved in `dir[14]` decimal, PDP-6
  assembly `+016` octal, not old `+014`.
  Source-linked v1519 fixture updated with an explicit
  negative old-index mutation; native tests PASS.
- [x] Repair TSFS parent return convention:
  `tsfs_parent_common` returns a nonzero parent vnode
  on success, negative on error. Both `tsfs_parent`
  and `tsfs_parent_name` now use `JUMPL`, fixing
  filesystem-boundary path reconstruction for FSINFO/
  GETCWD. Native v1525 source-linked parent/name test
  and two reverted-branch negative mutations PASS.
- [x] Rebuild current host `mkdtfs` as `mkdtfs-v1525`,
  install via versioned `local.tar.gz` under
  `PDP10_PREFIX`, retaining previous stale/legacy
  binary. Enforce correct nonzero 72-bit DTVOL1
  stamp, checksum and native directory owner index
  in full-guest test setup.
- [x] Actual 256K KCC DTC-root guest: `DTFS LOADED`;
  physical mounted native DTC3; FSINFO shows provider
  5 `/MOUNT/DT0`; STAT/GETCWD succeed; busy-cwd
  unmount refuses; open directory FILE alone also
  prevents unmount after cwd leaves; closing FILE
  permits unmount and FSINFO disappearance. The
  short disposable test-only PID1 program avoids
  unrelated long TSFS external EXEC loading.
  Optional DTFS/TSFS MRES unchanged octal `003746`
  text, `000002` BSS. Zero additional resident RAM.
- [ ] **P0-B physical cross-process and return:** use
  a second genuine process holding DTFS cwd/FILE;
  verify no unmount while either owner lives,
  actual DTC3 read error and 72-bit same-media
  return/mismatched swap fail-closed, including
  open FILE, FSINFO and SYNC behavior.
- [ ] **P0-B dirty write:** partial DTC3 failure,
  sticky WRITE_UNCERTAIN and explicit checked
  administrative recovery without silent mount
  replacement or stale vnode reuse.
- [ ] **P0-A:** authoritative physical permanent
  root/BACKSTORE/selected TSFS-member loss and
  precisely qualified v1515 fatal HALT caller.
- [ ] **P1:** remaining mixed-provider faults,
  v1492 FSINFO dynamic inventory guest acceptance,
  swapless PURE/MM/APR lifecycle, and final
  feature-only freeze. No optimization or 32K
  placement work yet (HOLD).

## 2026-11-12 v1524: current KCORE feature completion state

- [x] Identify why physically formatted secondary DECtape
  could not use native reserved RBN0: the simulator skipped
  its first forward search mark while `DTB_REQ` was pending.
  Repair this in the Type-551 SIMH model (not in KCORE),
  keeping native v1520 `DTVOL1` at reserved block zero.
  Real DTC3 RBN0, RBN1, RBN0144 source-linked writes/reads
  PASS; negative pre-fix simulator fails RBN0. Existing tape,
  v1522 and v1523 native regressions PASS. Install tested
  `pdp6-v1524` from `local.tar.gz` in `PDP10_PREFIX`;
  no KCORE RAM/code growth.
- [x] Physical DTC3 native MKFS/FSCK via a real 256K
  disk-selected auto-root guest, corrected SIMH:
  `FSCK.DTFS NATIVE OK`. This tests RBN0 and real
  physical format/validation, NOT DTFS mounting.
  A disk-selected root intentionally omits installed
  DTFS/TSFS MRES, so MOUNT.DTFS fails by design.
  Fix the older v578 guest false PASS caused by
  treating the existing D6FS mountpoint STAT as
  proof of mounted DTFS; require real FSINFO
  publication and successful UNMOUNT retirement.
- [ ] **P0-B physical mounted acceptance:** use a
  truly DTC-selected root with installed DTFS MRES,
  mount verified native DTC3, test open FILE/cwd,
  same-volume error/return and mismatched replacement,
  dirty/partial writes and safe mount retirement.
  The tape-root guest still does long DTC0 root work
  before its secondary operation. On a fresh rebuilt
  scratch D6FS root, the disk-root **negative-policy**
  guest PASS confirmed MKFS/FSCK, MOUNT.DTFS refusal,
  and D6FS/MEMFS-only MOUNT inventory. It does not
  establish actual mounted DTFS recovery.
- [ ] **P0-B policy:** explicit failure-atomic handling of
  sticky WRITE_UNCERTAIN and zero-ID legacy volumes;
  no silent automatic reattachment, forged identity or
  forced FILE owner invalidation.
- [ ] **P0-A critical loss:** find authoritative permanent
  root/BACKSTORE or selected TSFS member-loss evidence
  and test exact v1515 no-return HALT routing, without
  treating a timeout/PI/illegal select as permanent loss.
- [ ] **P1-C:** remaining real mixed DEVICE/VOLUME,
  BCACHE, LOGSTORE, MEMFS and backing/swap faults only,
  preserving already accepted source paths.
- [ ] **P1-D:** reconcile uncommitted FSINFO mount-capacity
  query and MOUNTS/DF 1/4/8/26 scan into production;
  finish linked physical hole/error/multi-mount tests.
- [ ] **P1-E:** real scheduler/VM integration for
  uncompressed inactive swapless PURE drop and complete
  MM/APR restore, with OOM/error-atomic ownership.
- [ ] **Final feature freeze:** matched physical/native
  current-source regression and documentation cleanup;
  decide on mutable-only RAM compression only after
  measuring net peak RAM (NO COMPRESSION is acceptable).
  No KCORE optimization and no 32K size gate (HOLD).

## 2026-11-12 v1523: all-process mount/FILE lifetime gate

- [x] Implement `file_mount_busy_v1523` using existing process
  metadata and stable u-areas; scan all 16 FILE descriptors and
  cwd for the full public mount ID, even if process user VM
  has been swapped out. No new permanent registration table.
- [x] Fix PIPE/SWAP owner-ID corruption caused by helper AC2
  clobber, and recheck the complete owner set after
  potentially sleeping SYNC/PREPARE, immediately before VFS
  mount-slot retirement.
- [x] Native PDP-6 v1523 FILE/cwd/unmount test PASS with
  seven executed unsafe mutations rejected; v1514 retry
  regression PASS. Full KCC 256K tape-root image link PASS.
  Controlled +54 KCORE text words, no new data words.
- [ ] Finish full physical secondary DTC3 mount/cwd/FILE
  fault-recovery guest. Initial boot and DSH PASS but
  target MKFS/FSCK on DTC3 stalls: investigate physical
  medium/controller/fixture before accepting any actual
  multi-process mounted-media scenario.
- [ ] Finish P0-A permanent-loss evidence/dispatch and
  remaining P0-B sticky uncertain-write recovery, then
  P1 features per the current v1521/v1522 checklists.

## 2026-11-12 v1522: mounted DTFS identity-after-error native acceptance

- [x] Flag only actual mounted DTFS members sharing a failed
  physical DTC unit using spare per-mount media RH bits; distinguish
  recoverable read suspicion from sticky uncertain partial writes,
  without new MM/static data or a false critical HALT.
- [x] Gate regular `dtfs_load`, FSINFO and SYNC before stale
  cached-directory/vnode reuse: raw physical RBN0, same v1520
  checksum/padding parser, exact both-half saved volume-ID match;
  refuse changed, malformed and legacy zero-ID return.
- [x] Actual PDP-6 source-linked v1522 suite PASS with seven
  negative mutations (including actual AC6 raw-I/O clobber).
  Existing DTFS, BCACHE and TSFS regressions PASS; isolated
  256K KCC tape-root KCORE + optional DTFS/TSFS MRES LINK.
  Controlled +86 optional MRES text, no new fixed KCORE or
  per-mount data. Independent strict single-root size gate
  remains FAILED (32K HOLD).
- [ ] **P0-B remains open:** full booted multi-process guest
  with genuine open FILE/vnode, dirty write, repeated medium
  error/return, mismatched replacement, mount retirement and
  independent root progress. Define safe explicit recovery for
  WRITE_UNCERTAIN; do not automatically clear it after same-ID
  read. Silent unreported exchange remains a detection limit.
- [ ] **P0-A remains open:** authoritative controller-level
  permanent boot-root/BACKSTORE loss signal and qualified v1515
  fatal caller. Type-551 select error/PI timeout is not proof.
  Continue remaining v1521 P1 mixed/FSINFO/swapless PURE work
  only after the relevant P0 correctness gates.

## 2026-11-12 v1521: CURRENT KCORE feature-only work (source audited)

This is the **sole current KCORE feature checklist**. Older dated
unchecked tasks below are original historical checkpoints, not
additional obligations. See `KCORE.MD` v1521 and
`design/kcore-feature-source-audit-v1521.md` for source paths,
existing acceptance and exact verification limits.

- [ ] **P0-A critical media:** obtain authoritative permanent
  root/BACKSTORE and selected TSFS physical member-loss evidence,
  distinguish PI/watchdog/illegal select, then source-link actual
  v1515 HALT caller. Physical confirmed-loss and false-positive
  negative tests, exact root/swap HALT PC mandatory. Do not attempt
  root/BACKSTORE reconstruction or forced owner release (v1513).
- [ ] **P0-B non-root media:** on DTFS observed error/return, physically
  match v1520 saved identity before old FILE/vnode/BCACHE reuse.
  Reject changed or unauthenticated legacy zero-ID volume safely;
  account for dirty/partial writes, mount-slot lifecycle and
  non-root return without corrupting the independent root.
- [ ] **P1-C remaining combined storage:** complete only missing
  in-flight DEVICE/VOLUME and shared BCACHE failure/return, mixed
  DSK/DRM/DTC backing and LOGSTORE/MEMFS/process-swap ownership
  matrix. Retain v1484/v1485/v1490/v1511-v1514 acceptances;
  no second allocator, cache, log or mapping interface.
- [ ] **P1-D FSINFO:** commit or reconcile the current **uncommitted**
  v1492 capacity query and MOUNTS/DF changes/testkit. Its native
  source-linked query/mutation and real command-body 1/4/8/26
  tests PASS v1521; linked physical holes/provider-error/>4 mount
  and small-inventory checks remain open.
- [ ] **P1-E uncompressed swapless PURE:** integrate staged inactive
  PURE drop, zero-copy MM rebase, complete bounded APR reload,
  immutable backing + mutable suffix and exact scheduler/EXEC/
  EXIT/WAIT/unmount/PI/OOM rollback on the production link.
  Test native and physical memory pressure. Ordinary disk swap
  remains unchanged; v1487 +180-TEXT prototype is not production.
- [ ] **Feature freeze:** matched current-source regression of
  affected physical and native guest cases, including failure,
  cancellation and restored healthy operation, without reopening
  already accepted F07/OOM/XIP/48K/codec profiles.
- [ ] **Conditional decision only:** after uncompressed PURE passes,
  enable mutable-only RAM compression *only if* exact matched peak
  whole-system RAM wins; otherwise record NO COMPRESSION.

Accepted and removed from active work: v1506 repaired BACKSTORE TERM
and owner completion, v1507 bounded retry, v1509/v1510 independent
bystander progress, v1511/v1512 real DRM PI2/watchdog failure,
v1518-v1520 native DTFS mount/ID groundwork, v1469-v1485 XIP,
v1464/v1466 block codec and v1486 non-XIP 48K boot. Strict 32K
HOLD, unrelated MRES/userland, D6FS online packer, unified TSFS
and optimization are **not KCORE feature tasks**. VERSION 0.5.

## 2026-11-12 v1520: native DTFS persistent identity groundwork

- [x] Define optional native `DTVOL1` 72-bit identity record in
  already reserved RBN0, keeping old RBN0144 directory unchanged.
  Host `mkdtfs` generates strong random IDs, C89 native probe
  accepts all-zero legacy and validates stamped record/checksum.
  Kernel DTFS mount physically validates header before VFS and
  retains its two ID halves in existing dynamic per-mount
  shape/extent words. Native PDP-6 source-linked identity tests
  and five executed unsafe mutations PASS; target `MKFS.DTFS`
  erases an old ID before formatting legacy tape, never copies it.
  KCC/GCC tape-root builds and older DTFS/BCACHE tests PASS.
  +46 optional MRES code, zero new data/fixed KCORE words.
  Independent strict 32K size gate still FAILS.
- [ ] Integrate verified **post-error** identity comparison before
  previously open FILE/vnode or cached DTFS data can be reused.
  Protect against partial dirty writes, mount-slot reuse, mismatched
  replacement and legacy zero-ID volumes. Current format stamp
  does NOT yet mean safe automatic same-tape recovery.
- [ ] Obtain authoritative Type-551 tape loss/offline signal and
  route confirmed TSFS root-set member loss to the v1515 fatal
  HALT, without treating ordinary errors/watchdogs as loss.

## 2026-11-12 v1519: physical native DTFS mount admission

- [x] Direct DTFS MOUNT_UNIT validates native magic and reserved
  block tags on a **fresh raw physical directory read** before
  publishing VFS mount/media ownership, even if callers bypass
  userspace DTFSPROBE or a cached directory looks valid.
  Native PDP-6 source-linked test PASS and six actual negative
  source mutations rejected. Existing BCACHE/DTFS regressions,
  isolated KCC 256K and GCC 32K KINIT/MRES links PASS.
  +27 optional folded MRES instructions, +0 data and fixed KCORE.
  Strict 32K KCORE size gate remains FAIL, not accepted.
- [x] Define persistent *native DTFS volume identity*: v1520
  introduced checked `DTVOL1` stamps and zero-ID legacy rules.
- [ ] Use that identity to revalidate **different** replacement
  media and old FILE/vnode/dirty-owner lifecycles after initial mount.
  Format tags alone do not guarantee safe mounted-volume reuse.
- [ ] Authoritative physical Type-551 tape-loss event and checked
  TSFS root-member v1515 fatal HALT remain P0 and OPEN; a generic
  read timeout and live cached directory are not such evidence.

## 2026-11-12 v1518: non-root DTFS failed-I/O cache invalidation

- [x] Negative physical DTFS read/write invalidates both shared
  clean BCACHE tags and the per-directory cached mount identity,
  without forced unmount, MM release, retry state or extra RAM.
  Tested in PDP-6 source-linked actual DTFS loader and BCACHE
  with four executed unsafe source mutations. Existing v1490
  BCACHE and DTFS/TSFS regression suites PASS. Cost +6 optional
  MRES instructions, +0 data, +0 fixed KCORE words. Full KCC
  and GCC tape-root KINIT/MRES link; strict 32K size gate still
  FAILS independently.
- [ ] Obtain authoritative tape-loss notification, not a generic
  PI error/timeout, then route confirmed TSFS root-set loss to
  v1515 HALT. Native v1518 does not claim physical removal.
- [ ] On non-root DTFS replacement, revalidate unique volume
  identity, BCACHE, dirty/open handles and safe remount before
  resuming old filesystem identity. Silent detach without physical
  I/O remains undetectable by the current driver.

## 2026-11-12 v1517: physical DTC loss signaling remains an OPEN P0 gate

- [x] Execute a real SIMH PDP-6 Type-551 select/CONI regression
  for unattached, attached and detached DTC0, plus unrelated
  unattached DTC7; all four status/PC checks PASS. Type-551
  select error `000004` is not itself a media-removal event.
  No production code or RAM change; testkit `3536d98`.
- [ ] Obtain original Type-551-compatible positive loss/offline
  evidence that distinguishes media/transport unavailability
  from illegal commands and transient errors. Only then route
  confirmed selected v1516 TSFS root-member loss to the v1515
  fatal stop. Do not infer loss from a select error or timeout.
- [ ] Full physical guest root-set removal HALT proof and
  unrelated non-root DTFS removal/return with identity, BCACHE,
  dirty/open resources and surviving root operation.

## 2026-11-12 v1516: exact TSFS physical root member identification

- [x] Resolve the unit-zero/trailing-zero ambiguity for roots spanning
  1..8 DECtapes. KINIT validates/exports exact 8-unit physical mask
  in existing mount shape bits 24..31; runtime helper validates real
  namespace root, mount bounds and mask. KINIT host tests, PDP-6
  native source and four mutation negatives, TSFS read regressions,
  isolated GCC/KCC tape-root build links PASS. No data words; about
  25-27 optional tape MRES instructions. 32K fixed size gate still
  FAILS and has not been reclassified as accepted.
- [ ] Obtain reliable DTC unit removal/changed-media status distinct
  from ordinary Type-136 errors and tape search/transfer watchdogs;
  qualify physical removal of each active TSFS root-set member and
  route confirmed loss to v1515 root fatal HALT (AC1=1).
- [ ] Validate auxiliary non-root DTFS removal/replacement stays
  nonfatal, preserving independent root, mounted ownership and
  correct cache/volume identity revalidation. No spurious fatal
  branch for unattached unrelated DTC units.

## 2026-11-12 v1515 DECtape role-based device loss

- [x] Confirm that **any selected TSFS root-set member**, including
  removable DECtape, is critical: confirmed removal must HALT
  KCORE, without online root replacement. Auxiliary non-root DTFS
  removal is nonfatal unless it shares a critical owner.
- [ ] Source-linked/physical DTC removal classification distinct
  from bounded search/data-transfer timeouts; use actual selected
  TSFS root-member mapping, not all DTC units. Route confirmed
  root-set loss to the v1515 fatal leaf and verify the HALT PC.
- [ ] Non-root DTFS removal/return or replacement, member identity,
  stale BCACHE invalidation, dirty/open FILE policy and independent
  root operation during a fault. Do not infer recovery from retry alone.

## 2026-11-12 v1515: authoritative critical-device fatal leaf

- [x] Implement versioned fixed KCORE terminal leaf for confirmed
  root/BACKSTORE device loss; fixed/non-removable media loss shares
  device-loss category. Disable global PI and distinguish root,
  BACKSTORE, invalid-caller halt PCs without console buffers.
  Six actual native PDP-6 cases and three negative mutations PASS.
  +11 fixed code words, zero data words, no process state/ABI change.
- [ ] Classify confirmed permanent device loss distinctly from PI
  errors/PI6 timeouts. Route through existing device/volume critical
  ownership to fatal leaf and physically test true device absence.
  No error-count threshold and no spurious panic on transient -1.
- [ ] Non-root identity/cached-block/dirty FILE recovery, mixed
  providers, F1 PURE/MM/APR and F3 dynamic FSINFO still OPEN.

## 2026-11-12 v1514: secondary-media unmount failure ordering

- [x] Source-linked native PDP-6 v1514: secondary D6FS failed
  sync preserves mount/RO/namespace/FILE and returns -1; same
  medium return permits checked sync, D6FS preparation, FILE
  unlock and mount retirement without altering the root.
  Three executed negative VFS mutations rejected, no KCORE RAM.
- [ ] Full physical hot-unplug/return, checked replacement-media
  revalidation, BCACHE and dirty/open FILE ownership remain OPEN.
- [ ] Confirmed critical root/BACKSTORE loss classifier and
  minimal HALT (no recovery), plus remaining mixed providers,
  PURE/MM/APR and dynamic FSINFO features, remain OPEN.

## 2026-11-12 v1513: media-loss classification and recovery scope

- [x] Adopt crash-only policy for **confirmed** root filesystem
  or BACKSTORE media loss: drop earlier reconstruction/recovery
  requirements. Avoid creating a quarantine process state or
  releasing suspended private owners; optional small fatal text.
- [x] Source-linked PDP-6 v1513 native D6FS non-root read
  failure/retry test: two failed secondary reads return NULL
  without new clean BCACHE publication, an independent root
  read succeeds, and restored same secondary media succeeds
  twice after separate losses. Three actual negative mutants
  rejected; v1 multi-mount oracle corrected; no KCORE RAM growth.
- [ ] Distinguish *confirmed* offline/lost root or BACKSTORE
  from ordinary PI errors/PI6 timeouts; implement concise fatal
  HALT/diagnostic and physical critical-loss acceptance. Do NOT
  turn any single `-1` into a terminal media-loss verdict.
- [ ] Non-root removal/reattachment and replacement-media
  verification, BCACHE invalidation and dirty/open-owner safety;
  actual full guest acceptance remains OPEN.
- [ ] Mixed DEVICE/VOLUME, DTFS/BCACHE, LOGSTORE/MEMFS errors;
  F1 swapless PURE/MM/APR; F3 FSINFO dynamic mount enumeration.
  Compression/optimization only after feature completion.

## 2026-11-12 v1512: physical PI6 watchdog rollback accepted

- [x] Full 256K PDP-6 source-linked selected Type-236 completion
  loss: two real DR_DONE notifications suppressed, two original
  300-tick PI6 watchdog timeouts; exact DRM accounting, request
  event wakeup, negative BACKSTORE read/MM rollback. Sixteen
  physical fault-on samples preserve original slot `010` DISK
  backing/u-area/TERM/retry with 30 later checked-free refusals,
  32 total failed restores and no false READY_ONCE/zombie.
- [x] Confirm independent healthy swapped PID `011` restore and
  562 real user SLEEP calls, and resident PID 5's 992 genuine
  additional SLEEP calls. Strict v1512 physical verifier PASS,
  four source and twelve evidence negatives plus missing-key
  parser test rejected. No production RAM/process-state growth.
- [ ] Qualify actually unreadable/disappeared DEVICE/media and
  safe owner recovery or long-lived ownership policy, without
  forced backing release or unverified parent WAIT completion.
- [ ] Mixed DEVICE/VOLUME, BCACHE/DTFS, LOGSTORE/MEMFS provider
  failure/recovery; F1 swapless PURE/MM/APR; F3 FSINFO dynamic
  mount enumeration. Optimization/compression only afterward.

## 2026-11-12 v1511: real DRM PI2 BACKSTORE error propagation

- [x] Physically route two selected Type-167/236 PI2 error
  completions through original DRM error accounting, the actual
  BACKSTORE read failure and checked MM rollback. Thirteen real
  fault-on samples preserve slot `010` exact DISK/u-area/TERM/
  bitmap and produce 2 controller errors plus 25 refused frees,
  exactly 27 failed slot-0 restorations. The independently swapped
  PID `011` restores and runs 397 times; resident PID 5 progresses.
  Strict verifier PASS; five source and eleven evidence mutations
  rejected, zero new permanent RAM/process-state words.
- [x] v1512 accepted the full physical missing-Type-236-PI2-completion
  scenario using the original PI6 300-tick watchdog and exact owner
  rollback, with independent swapped/resident progress.
- [ ] Genuine physical DEVICE/media loss/reconstruction and safe
  unrecoverable-owner policy; never claim false backing free/ZOMB.
- [ ] Mixed DEVICE/VOLUME, DTFS/BCACHE, LOGSTORE/MEMFS failures;
  F1 swapless PURE/MM/APR; F3 FSINFO mounts. Optimize only later.

## 2026-11-12 v1510: real swapped bystander after sustained BACKSTORE failure

- [x] Boot authentic 256K PDP-6 F07 full stress guest; prove an
  independently DISK-backed healthy slot `011` restores through
  the original checked production swap transaction **after**
  original slot `010` backing release remains permanently refused.
  Slot `011` went `SLEEP/DISK` -> `SLEEP/RESIDENT`, then executed
  331 real user SLEEP syscalls in 13 fault-on snapshots. Original
  owner preserved exactly: first `02350`, length `0330` octal,
  1..24 refused releases, u-area/bitmap/pending TERM retained;
  resident PID 5 also progressed. v1510 strict verifier PASS,
  eight source and eleven false-evidence mutants rejected.
  No new KCORE RAM/state or kernel source changes.
- [ ] Physically qualify genuine permanent DEVICE/media failure,
  timeouts and safe recovery or explicitly retained unresolved
  ownership. Never fabricate a ZOMB/WAIT or free the bad run.
- [ ] Complete mixed DEVICE/VOLUME, BCACHE/DTFS/LOGSTORE/MEMFS
  failure gates, swapless PURE/MM/APR and dynamic FSINFO mounts.
- [ ] Reassess optional compression only after feature acceptance
  and a proven net RAM saving; do not start a size-only sweep.

## 2026-11-12 v1509: physical scheduler fairness with failed BACKSTORE

- [x] Keep the actual F07 PID 5 spinner runnable through extra-child
  TERM/WAIT in a disposable test guest and source-sample its real
  SLEEP syscall counter during repeated checked-release refusal.
  Six physical fault-on samples retained failed slot `010` backing
  while independent resident PID 5 completed 358 more SLEEP calls.
  Strict v1509 verifier PASS; four guest-source and nine evidence
  negative mutations rejected. Zero production RAM/process-state
  growth; source and SHA256 in design/v1509.
- [x] The later physical v1510 test demonstrated a distinct healthy
  *swapped* process restoring/executing while the old DISK owner
  remained in its original failure state.
- [ ] Genuine permanent DEVICE/media failure: verified safe recovery
  or explicit retained-ownership policy, never forced backing release.
- [ ] Mixed DEVICE/VOLUME, DTFS/BCACHE, LOGSTORE/MEMFS error gates;
  F1 swapless PURE/MM/APR; F3 dynamic FSINFO enumeration; optional
  compression only after functional acceptance.

## 2026-11-12 v1508: physical permanent BACKSTORE refusal gate

- [x] Boot a disposable 256K PDP-6 and refuse every checked release of
  one real ordinary DISK owner without changing its original bitmap.
  Three actual fault-on snapshots prove 1/4/7 refusals, advancing PI6
  clock, unchanged DISK/u-area/pending TERM and zero false finalizers.
  Strict v1508 physical verifier PASS, ten unsafe mutants rejected.
- [ ] Demonstrate real independently runnable/swapped bystander
  progress during a sustained physical failure, not merely native
  scheduler fairness or clock progress.
- [ ] Resolve irreparable backing/DEVICE loss via genuine recovery,
  reconstruction, or explicitly safe retained ownership; no forced
  release and no unverified zombie. Physically validate mixed media.
- [ ] Continue KCORE feature completion: mixed DEVICE/VOLUME,
  BCACHE/DTFS/LOGSTORE/MEMFS error gates, F1 swapless PURE/MM/APR,
  F3 dynamic FSINFO mount enumeration. Compression only after PURE.

## 2026-11-12 v1507: failed BACKSTORE retry bounded in native scheduler

- [x] Fix a permanently failing swap-in's uncontrolled repeated
  selection without inventing a new quarantine process state or
  allocating more resident per-process words.
- [x] Preserve independent healthy swapped admission and realtime
  fallback even with two different failing DISK owners; retain the
  original global allocator-pressure cooldown.
- [x] Validate source-linked PI6 expiry at 0100 octal ticks, clock
  wrap, marker retirement on successful checked restore/exchange,
  exact DISK accounting and pending TERM policy. Native v1507
  PASS, three negative mutants PASS, earlier native v1493/v1505/
  v1316-v1317 regressions PASS; isolated 256K KCC image build PASS.
- [x] Native v788 paired publish and v854 sector/PURE exchange
  retire the incoming owner's retry tag without skipping checked
  MM/storage publication.
- [x] v1508-v1512 subsequent physical sustained-failure guests
  accepted clock-qualified retries, both kinds of bystander
  progress and stable DISK/bitmap/MM/executive ownership without
  false zombie or READY_ONCE.
- [ ] Permanent media/DEVICE identity loss: safe recovery or
  explicit long-lived owned failure; never declare irreparable
  backing reclaimed without an accepted resource release.
- [ ] Other KCORE work: mixed DEVICE/VOLUME and BCACHE/DTFS/
  LOGSTORE/MEMFS failures, PURE-text reclaim/APR, F3 FSINFO mount
  enumeration and final feature acceptance. Not optimizing yet.

## 2026-11-11 v1506: physical owner-side TERM completion accepted

- [x] Boot real 256K KCC/PDP-6 v1504 F07 with one genuine bitmap
  fault, two rejected restore attempts and one precise backing repair.
- [x] Observe exact affected DISK slot `010` enter the real owning
  deferred-TERM consumer once, clear pending TERM and set fatal-close
  latch; real private close/finish/zombie each once at status `0400001`.
- [x] Strict physical target-specific verification plus 20 negative
  mutations PASS, inherited v1503 15 mutations and independent
  MM/owner/F07 18/18 checks also PASS. Normal v1336 unchanged.
- [ ] Design, implement and qualify permanently unrecoverable bitmap/
  DEVICE fault behavior: bounded retry or safe quarantine without
  false RELEASE, lost DISK owner or private executive continuation.
- [ ] Close remaining mixed-media/DTFS/LOGSTORE cases, swapless PURE,
  mount enumeration and (later) optional RAM compression.
  See `design/kcore-backstore-owner-return-physical-v1506.md`.

## 2026-11-11 v1505: repeated invalid BACKSTORE transaction

- [x] Source-link real PDP-6 swap-in, slot-0 scheduler, bitmap free
  across six *forced* permanently invalid checks: same DISK owner,
  no false RESIDENT/READY_ONCE, exact MM/accounting/bitmap invariants.
- [x] Explicitly repair missing bit, verify one successful restore
  and exact zero backing; rerun two v1493 unsafe mutations.
- [x] Correct inline swap-service success/result documentation, no
  functional resident code change.
- [x] v1506 subsequently accepted actual physical owning-return TERM
  consumption and single real owner cleanup after repaired backing.
- [ ] Define and implement permanently-corrupt-backstore terminal
  handling and bounded retry behavior without releasing a private
  executive continuation or losing authoritative DISK backing.
- [ ] Complete combined fault-specific physical acceptance, leaving
  normal v1336 zero-terminal-swapin failure acceptance unchanged.
  See `design/kcore-backstore-repeated-refusal-v1505.md`.

## 2026-11-11 v1503: physical TERM coalescing accepted, recovery OPEN

- [x] Attribute two real 256K F07 scheduler TERM calls to an
  already-pending TERM on stable DISK slot `010`: no WAIT, saved
  executive PC, return zero, no changed owner or FINALIZE.
- [x] Strict physical snapshot/bitmap/F07 verifier v1503 and all 15
  negative mutations PASS; replay protected-MM/process/swap-stack/
  owner-lifecycle physical audits PASS.
- [x] Exclude inverted v1499 SKIPN branch counters; v1502 corrected
  observer polarity in testkit without altering production code.
- [x] v1506 subsequently demonstrated actual pending TERM consumption
  at owning return, fatal close and exact repaired victim resolution.
- [ ] Test permanent corruption fail-closed policy without unsafe
  repeated swap-in, false RESIDENT/READY_ONCE or lost DISK owner.
- [ ] Complete end-to-end physical fault acceptance; retain normal
  v1336 zero-failed-swap-in rules. Other F1/F2/F3/F4 tasks below
  remain open. See `design/kcore-backstore-coalesced-physical-v1503.md`.

## 2026-11-11 v1502: live KCORE feature-completion checklist

- [x] Native v1498 real nonresident TERM WAIT/saved-PC decision,
  v1500 real immediate event+PREVM finalizer and v1501 real deferred
  owner-return event consumer; positive and negative controls PASS.
- [x] Capture two v1499 physical pre/post owner snapshots; invalidate
  original out-of-scope `SKIPN` event counts. Testkit v1502 `SKIPE`
  native guard and negative mutation regression PASS.
- [x] Later v1503 accepted corrected physical TERM coalescing and
  the map-backed owner snapshots; v1506 accepted owning return and
  cleanup. The invalid original v1499 SKIPN counts stay excluded.
- [ ] Test permanently corrupt BACKSTORE bitmap/DEVICE failure with
  exact DISK ownership, no false READY_ONCE or unsafe repeated restores,
  and eventual safe deferred-owner/finalizer recovery.
- [ ] Complete strict physical corruption-specific TERM -> FINALIZE/
  PREVM -> checked backing-release acceptance; preserve normal F07/
  v1336's zero-fatal-swap-in criteria without exception.
- [ ] Close remaining mixed DEVICE/VOLUME/BCACHE/LOGSTORE/MEMFS swap
  failure gates and physically installed DTFS tape-root integration.
- [ ] Integrate uncompressed swapless PURE scheduler/MM transaction,
  immutable backing, mutable suffix, complete APR restore and every
  lifecycle/rollback case; do not link the +180-word v1487 pilot.
- [ ] Finish v1492 dynamic FSINFO mount enumeration contract and
  1/4/8/26-slot consumer/physical acceptance; currently uncommitted.
- [ ] Evaluate optional mutable-RAM compression only after PURE and
  positive matched RAM accounting, then remaining clock/device tests.

Reference: pdp10-doc `STATUS/KCORE-FEATURE-STATE-V1502.MD`.
This is documentation reconciliation, not KCORE optimization. 32K
remains HOLD, unified TSFS/online D6FS packer are separate 0.9 tasks,
and MRES code unload/reload is POST-1.0.

## 2026-11-11 v1500: integrated nonresident TERM and PREVM owner

- [x] Source-link real `proc_event_apply` + `proc_finish_slot` +
  `proc_finish_service_v881` with nonresident DISK VM and controlled
  refused release, followed by successful idle VM/u-area retries.
- [x] Check exact retained TERM/PREVM status, stable disk ownership,
  single FILE/HUP/VM commit and no duplicate zombie/reaper handling;
  reject negative mutation removing the PREVM mark.
- [ ] Accept equivalent real physical BACKSTORE/VM integration,
  including actual deferred TERM classification (v1499) and safe
  permanent-corruption policy. v1500 is native-only for VM backend.
  See `design/kcore-backstore-event-finalizer-v1500.md`.

## 2026-11-11 v1498: classify actual nonresident fatal-event policy

- [x] Native source-link real `proc_event_apply` with a DISK owner;
  prove WAIT and executive-PC deferred TERM, pending coalescing,
  rejected VM release owner retention and distinct successful commit.
- [x] Reject executable mutation bypassing saved-executive-PC guard;
  keep normal production syscall, scheduler and backing allocator.
- [ ] In physical 256K guest capture the exact WAIT/CTX_PC, pending
  FD-control event bits, return status and chosen fatal-event branch.
- [ ] Qualify permanent corrupt backing and safely completed deferred
  or immediate TERM without weakening the normal v1336 acceptance.
  See `design/kcore-backstore-term-policy-v1498.md`.

## 2026-11-11 v1497: actual physical BACKSTORE fault evidence

- [x] Trace real v1495 256K physical swap, checked bitmap refusal,
  TERM call/return and backing ownership through complete guest HALT.
- [x] Fix F07 guest-child miscount from SIMH dirty-build banner in
  testkit v1496; 18/18 real guest acknowledgments/reaps unchanged.
- [x] Replay independent protected-OOM, MM peak, process slot,
  swap-stack and full owner-lifecycle checks: PASS.
- [ ] Identify why `proc_event_apply` defers or refuses synchronous
  finalization for the faulted nonresident owner; observe actual
  WAIT/CTX_PC, pending TERM and event return value.
- [ ] Prove owner-safe deferred TERM delivery or PREVM retry on a
  finalizable target; prevent permanent corrupt backing from becoming
  an unsafe repeat-restore loop. Keep stack-safety invariants.
- [ ] Add separate fault-specific full-guest acceptance. Normal v1336
  still correctly requires zero fatal swap-ins. v1494 immediate PREVM
  expectation is disproved by v1495. No KCORE optimization yet.
  See `design/kcore-backstore-physical-f07-diagnostic-v1497.md`.

## 2026-11-11 v1494: PREVM finalizer failure/retry

- [x] Source-link real `proc_finish_slot` and
  `proc_finish_service_v881` with controlled refused VM release.
- [x] Retain FINALIZE+PREVM, TRANSITION, VM/u-area owner and exit
  status; retry VM, then rejected u-area releases, with no repeated
  sleeping HUP or double VM release.
- [x] Reject premature VM-commit mutation; rerun v1081/v1086.
- [ ] Source-link the *entire* real BACKSTORE detach/TERM/finalizer
  chain and complete the corrupt-bitmap 256K physical full guest.
  Separate native v1493 and v1494 tests do not imply this acceptance.

## 2026-11-11 v1493: BACKSTORE release refusal through scheduler service

- [x] Run real production `proc_swap_in`, checked bitmap release and
  slot-0 `proc_swap_mres_service_v481` with injected incomplete bitmap.
- [x] Verify exactly one TERM request, no false RESIDENT/READY_ONCE,
  preserved original DISK owner and accounting, successful MM rollback.
- [x] Reject executable mutations removing either the checked release
  or the scheduler's negative-result failure dispatch. Rerun v1405.
- [ ] Observe complete `proc_event_apply`/finalizer ownership and actual
  physical backing aftermath in a full 256K corrupt-release guest.
  Native v1493 uses a controlled event observer, not full guest cleanup.
  See `design/kcore-backstore-scheduler-refusal-v1493.md`.

## 2026-11-11 v1490: cache write/refusal gate

- [x] Correct live DTC write ABI across BCACHE: preserve AC1=unit,
  AC2=block, AC3=buffer, without a pre-write MM release.
- [x] Invalidate clean cache identities even if `mm_free` refuses;
  retain the still-owned private slab for retry, never a stale hit.
- [x] PASS source-linked native PDP-6 device read/write, MM refusal,
  retry, two executable negative controls and existing BCACHE tests.
- [ ] Complete real tape-root/secondary-DTFS guest acceptance and the
  remaining distinct BACKSTORE/BCACHE mixed-storage failure cases.
  Disk-root DTFS mount refusal is intentional; historical v572's
  mount-success predicate is superseded by the v1 online policy test.
  See `design/kcore-bcache-write-reclaim-v1490.md`.

## 2026-11-11 v1488: documentation sweep and current 0.9 KCORE gates

Authoritative CURRENT scope after source/linked-map/testkit cross-check:

- [x] Grouped D6 block codec v1464 is production-linked (Stage-1,
  KCORE v1458, EXEC/PURE, TSFS, host producers). Do not reopen
  D6LZ36 cutover solely because older v512/v1029/v1395 notes say OPEN.
- [x] Restricted root-only write-free MEMFS XIP RUN/EXEC and v1469-v1485
  pin, rollback, pressure, remount and cold-snapshot lifecycle accepted
  at 128K/192K/256K. MEMFS/XIP requires at least 96K.
- [x] v1486: separate 48K non-XIP physical LOWMEM guest PASS with five
  stable DATE/MEMSTAT snapshots, MEMFS absent. 32K is explicitly
  deferred and must not be tested until separately requested.
- [x] v1487: isolated PDP-6 common-DXR EXEC pilot PASS on normal DXR
  cases; original strict XIP branch retained by construction, not
  newly guest-tested. The candidate adds 180 isolated TEXT words
  (163+235 versus 218); REJECT production linkage as a RAM saving.
- [x] F12/F13 requester-only BRK OOM production gate accepted by v1298
  (64/64 in the defined 256K release profile). Raw disk swap unchanged.
- [ ] Finish any independently identified BSTORE/BCACHE/LOGSTORE
  production-interface/failure/combined-media gates not covered by
  accepted v1484/v1485 evidence. Do not reimplement accepted backing.
- [ ] Integrate swapless PURE-text dropping/restore with real process
  scheduler/VM/MM ownership and tests; the v1381/v1384/v1395/v1397
  component paths pass, but are NOT production-linked. Do not add
  the v1487 +180-word shared loader merely to reach this milestone.
- [ ] Integrate optional in-RAM mutable-suffix compression only if its
  exact net physical RAM cost (code, buffer, full-APR restore headroom)
  is favorable. v1451's 30-word trusted codec is test-only.
- [ ] Complete the remaining 0.9 online D6FS packer and unified TSFS
  post-KCORE implementation, and final accepted KCORE size sweep
  LAST, after production functional/rollback gates are closed.

Scope: dynamic MRES unload/reload is retained for POST-1.0, excluded
from 0.9 AND 1.0 acceptance, and receives NO new work until 0.9 ships.
Historical unchecked v512/v1029/v13xx/v14xx items below are chronological
evidence, not an additive current release checklist; the old 32K XIP,
32K PURE and compressed-disk-swap proposals are superseded. Version
in DAIMOS/VERSION remains 0.5; no 0.9 release is claimed.

## 2026-11-11 v1487: MRES runtime unload/reload is POST-1.0 ONLY

Dynamic runtime unloading and reloading of boot-installed MRES modules,
including the MEMFS provider, remains an explicit POST-1.0 feature.
It is NOT a 0.9 or 1.0 release prerequisite, and must not receive
implementation, investigation, regression gates or prioritization
until DAIMOS 0.9 has been released. Revisit its design only in the
post-0.9 planning phase, with implementation scheduled after 1.0.
Boot-time MRES installation and filesystem instance mount/unmount
remain part of the current functionality; neither implies dynamic
MRES unloading. Older unchecked runtime-unload entries below are
historical and superseded by this release-scope decision.

## 2026-11-11 v1486: 48K non-XIP LOWMEM gate

- [x] A real physical 48K PDP-6 LOWMEM DSK/D6FS image booted to
  ROOT/DSH. Five consecutive DATE/MEMSTAT samples matched:
  TOTAL 49152, RESIDENT 16271, PROCESS-WORDS 23320,
  PROC-SLOTS 4/12 and MEMFS-CAPACITY 0.
- [x] Put 48K-only repeatable acceptance in daimos-testkit v1486
  while leaving historical v480 32K scripts unchanged.
- [ ] 32K testing remains explicitly deferred until requested;
  it is not a current KCORE feature gate.

## 2026-11-11 v1485: corrupt MEMFS snapshot admission rollback

- [x] Commit an actual persistent root-owned MEMFS XIP image on DR0,
  flip one metadata bit after HALT while preserving the v2 commit
  header and AUXSTORE descriptor, and cold-boot with the damage.
- [x] Reproduce pre-fix unpublished singleton/namespace MM retention
  on refused snapshot checksum; verify MEMINFO false capacity and
  inability to mount a new volatile MEMFS in the same boot.
- [x] Share real mount-failure cleanup with refused snapshot admission:
  MM release before three-word ownership clear; preserve ownership if
  MM release refuses. No BSS or process RAM growth; +1 MRES text word.
- [x] Real 128K corrupted-snapshot cold-boot: reject persistent mount
  and subsequently create a same-boot volatile MEMFS, verify strict XIP
  readback and RUN/WAIT, then HALT with zero status.
- [x] Rebuild and PASS independent post-fix 128K/192K/256K two-cold-boot
  normal persistent snapshot/XIP restore and AUXSTORE integrity matrix.
- [x] Source-linked v1094/v1471 and native MEMFS normal/v1403/v1468
  regression tests PASS with the new cleanup.
- [ ] Continue independent sub-96K non-XIP release tests and swapless
  PURE RAM reclamation. Dynamic MRES unload/reload is not a completed
  or implied capability.

## 2026-11-11 v1484: persistent MEMFS/LOGSTORE damage and recoverability

- [x] Identify DR0 descriptor corruption: instrumented Type 167/236
  simulator confirms LOGSTORE writes to physical block zero using KCORE
  text as DMA source, due to raw MRES ABI target mismatch at KINIT.
- [x] Patch LOGSTORE AUXSTORE read/write to existing public KCORE
  DRM236/DSK270 bridges after validating installed driver services.
  Testkit source-level negative mutation catches the previous target.
- [x] Correct v1477 snapshot capacity checks, invalidate shared BCACHE
  workspace before direct snapshot I/O, and reserve v2 snapshot runs at
  swap boot. Preserve on-media v1 support and strict commit-last design.
- [x] Validate real 128K and 192K two-cold-boot PERSIST mounts, exact
  XIP/payload restore, and intact AUXSTORE descriptor after each HALT.
- [x] Complete the 256K full-profile two-boot integrity/recovery test,
  matching passing 128K and 192K guests.
- [x] Complete isolated 128K partial-write/refused-HALT retry across
  two cold boots with exact snapshot recovery and the same AUXSTORE
  descriptor-integrity assertions. The RAM-only fault resets once per
  boot; both boots explicitly refuse then safely retry HALT.
- [x] Finish independent 128K normal MEMFS remount, XIP source-linked
  v1094/v1471 teardown/retry and native LOGSTORE append/direct-I/O,
  MEMFS normal/v1403/v1468 fatal regression checks; all PASS.
- [ ] Independently define/test dynamic MEMFS provider MRES unload/reload
  only if future runtime module unloading is made available.

## 2026-11-11 v1479: XIP physical memory floor

- [x] Enforce the MEMFS/XIP 96K minimum physical-memory policy at the
  shared XIP guest driver: reject sub-96K requests before any build.
- [x] Keep XIP acceptance strictly on the existing 128K, 192K and
  256K full-profile guests. There were no active 32K/48K XIP guest
  tests to remove.
- [x] Preserve separate 32K/48K low-memory non-XIP release testing.

## 2026-11-11 v1476: MEMFS lifetime and state bounds

- [x] v1474 baseline full-profile PDP-6 at 128K, 192K and 256K:
  three unmount/remount cycles each with exact resource accounting,
  stale XIP invalidation, recreated XIP RUN/WAIT, and full EXEC tests.
- [x] v1475 injected MEMFS data-destroy preflight refusal: retained
  mount, file readback, executable RUN, followed by successful retry.
- [x] Fix two-word overwrite on MEMFS unmount: clear exactly three
  memfs_mres_fs words rather than five. Native v1475 test includes
  directly adjacent MRES BSS canary; 128K guest PASS.
- [x] Native XIP v1094/v1471 and MEMFS v1403/v1468 regression PASS.
- [x] Complete post-fix v1474 128K/192K/256K physical guest matrix:
  all normal remount cycles, original XIP RUN/EXEC, and orderly HALT
  PASS; fixed kernel was rebuilt independently for all three sizes.
- [ ] Test persistent snapshot failure/retry across HALT/reboot and
  distinguish it from filesystem unmount. Dynamic module unload/reload
  must be specified and tested separately if available.

## 2026-11-11 v1473: XIP pressure and pinned provider refusal

- [x] Real physical 128K/192K/256K full-profile KCC guest: three queued
  XIP RUN/WAIT rounds per memory size, up to 56 independent child slots
  per batch, with exact process-slot, process-word and MEMFS accounting
  recovery; full original v1469 RUN/EXEC regression also PASS.
- [x] Distinguish queued zombies from truly pinned XIP processes. A
  short-lived XIP child's occupied slot is not evidence of a live pin.
- [x] Re-run full 128K v1472 fault-injected guest with an explicit
  `dsys_unmount(/TEMP)` refusal after CHMOD proves the SIDL owner still
  holds its MEMFS XIP pin. Deferred cleanup and both fault counters PASS.
- [x] Keep changes entirely in testkit; no KCORE/MM/MRES growth.
- [ ] Exercise successful MEMFS unmount/re-mount and provider shutdown,
  reload and injected-failure lifetime under independently verified pins.
- [ ] Complete separate 32K/48K non-XIP release gates and swapless PURE
  reclamation with the already validated bitmap codec.

## 2026-11-11 v1472: XIP post-admission u-area failure recovery

- [x] Inject one u-area MM refusal immediately after **real certified
  MEMFS XIP pin and staged VM** in RUN, plus two test-only owner-side
  unpin refusals, without changing permanent KCORE/MRES code.
- [x] Verify the unpublished RUN remains at stage 3 (RUN_VM), CHMOD is
  refused while pinned, reaper eventually unpins and discards it, no
  child zombie is returned by WAIT, slots and physical process-word
  accounting return to baseline, and later XIP RUN/WAIT and EXEC work.
- [x] Independent full KCC disk-root SIMH acceptance at 128K, 192K and
  256K: three PASSes, KINIT memory verified, post-HALT fixed-KCORE
  counters exactly 1 allocation refusal and 2 unpin refusals each.
  Testkit provides both individual and three-size v1472 runners.
- [ ] Extend concurrent process-slot and physical-memory pressure with
  pinned XIP images and test source mount removal/provider lifetime
  failures; do not expand beyond the strict PDP-6 no-write XIP profile.
- [ ] Continue separate swapless PURE release and v1451 bitmap reclaim
  integration, plus 32K/48K non-XIP acceptance and later size reduction.

## 2026-11-11 v1471: XIP release ordering and refusal retry

- [x] Fix XIP `vm_space_destroy` pin-ownership loss on a refused
  `proc_swap_detach`: for verified pinned XIP only, detach the resident
  zero-text swap record before releasing the MEMFS pin, then publish
  VM_NONE. Keep private-MM teardown unchanged and use saved base after
  read-only provider query.
- [x] Native production source-linked rejection/retry at both failure
  boundaries, plus private-MM base check and two negative mutants.
  Existing v1094/v1108/v1109 native fault cases pass.
- [x] Rebuild and boot the changed full-profile 128K, 192K and 256K KCC
  images; ROOT/DSH and real v1469 XIP lifecycles pass at all three.
  Exact matched KCC fixed cost +19 words.
- [ ] Inject RUN u-area MM allocation refusal *after* a certified XIP
  is pinned, and demonstrate slot-0 deferred recovery and no ref loss.
- [ ] Extend live concurrent RAM/slot pressure and multi-mount teardown.

## 2026-11-11 v1470: full-profile XIP 128K/192K/256K accepted

- [x] Run actual independent full-profile MEMFS XIP cold boots at 128K,
  192K and 256K, with matching boot core-word limits, KINIT-reported
  physical RAM and loaded MEMFS. All reach INIT, ROOT LOGIN, DSH and pass
  the v1469 real privileged RUN/WAIT and EXEC/negative-lifecycle guest.
- [x] Reject false simulator memory-size results: Open SIMH `196K` typo
  corrected to `192K` in sims commit `16dc791`; new testkit v1470
  preflights `set cpu`/`show cpu` and verifies the KINIT `MEM ... K`
  banner. Re-ran the real 192K guest successfully after the fix.
- [x] Preserve lowmem's exclusion of MEMFS and revert the unsupported
  exploratory 48K XIP build changes. No size optimization or DAIMOS
  kernel source change in v1470.
- [ ] Continue XIP MM/u-area fault injection, transaction recovery,
  post-commit teardown and high-pressure acceptance. Separately complete
  32K/48K lowmem non-XIP release gates and swapless PURE integration.

## 2026-11-11 v1469: privileged raw MEMFS XIP integrated in RUN/EXEC

- [x] Select the certified root-only MEMFS raw-XIP profile in the real
  `exec_load_process` path, retaining the existing DXR1/DXR2 loader and
  no-write, pinned-image safety policy. Use the ordinary staged-VM
  destruction path for exact pin ownership and EXEC rollback.
- [x] Enforce zero argc/envc and write-free XIP startup through an actual
  pinned-base lookup (private MRES operation 031). Refuse XIP BRK mutation;
  reuse existing MM-pinned victim and swap-out exclusions, with no new
  persistent per-process words.
- [x] Fresh 256K KCC physical guest PASS: normal ROOT/LOGIN/DSH, three
  serial and three queued XIP RUN/WAIT, malicious nonzero image rejection,
  nonroot denial, argv refusal, EXEC argv rollback, successful root EXEC,
  no final MEMFS pin leak and shell STATUS:0. Native v1468/v1403/v216/v1394,
  EXEC v200/v20 compatibility PASS.
- [x] Measure whole-link functional growth without optimizing: GCC lowmem
  KCORE 9765 words (+64 from v1466), matched KCC MEMFS MRES 1849 -> 1851.
- [ ] Extend to 32K/48K resident and pressure acceptance, injected UAREA
  allocation/rollback/teardown failures, concurrent mount/unmount and
  provider/MM failure recovery. Do not authorize arbitrary shared text:
  PDP-6 APR still has no user-write protection for MEMFS code.

## 2026-11-11 v1468: certified XIP provider, remaining process integration

- [x] Add production MEMFS private operation 030 for root-owned write-free
  XIP certification, while retaining the legacy format-only 027 for native
  hardware diagnostics. Check every image word, retain the pin until all
  tests pass, preserve outputs on rejection and fail closed if cleanup
  refuses MM unpin. Native success/negative/fatal/red-green PASS.
- [ ] Connect this strict provider operation to privileged RUN/EXEC,
  zero-write startup, BRK and swap exclusions, full rollback and actual
  scheduled user-mode 32K/48K/256K tests. Do not optimize yet.
- [x] Confirm that the RAM-only compressor is the existing v1451 30-word
  bitmap encode/decode pair, tested on PDP-6; its process integration and
  pressure acceptance remain OPEN, without a replacement codec.

## 2026-11-11 v1467: raw MEMFS XIP source ownership

- [x] Reject non-root-owned raw-XIP vnode before MM pinning, using existing
  MEMFS UID9 metadata, with unchanged base/entry outputs and no leaked
  provider reference on rejection. Source-linked native red/green PASS;
  SIMH/AAP XIP lifecycle and v1394 no-write policy checks also PASS.
- [ ] Connect privileged raw-XIP RUN/EXEC to this gate and the separately
  staged strict instruction-policy verifier; provide a no-write startup
  path, correct VM/swap/BRK exclusion, and failure-atomic rollback. Verify
  actual scheduled guests under constrained physical memory. No folding
  or final size-only optimization until feature acceptance is complete.

## 2026-11-10 v1464: requested single-format block codec cutover

- [x] Supersede the previous D6 format deferral by explicit decision.
  Replace production D6LZ36 with the v1421 block LZ/RLE wire format,
  the 53-word v1458 Stage-1/KCORE decoder and one 58-word VFS frontend.
- [x] Delete old D6LZ36 source and header implementations, host encoder,
  userland utility and manual; no legacy decoder or format detection.
- [x] Convert boot/KINIT compressed images, all D6FS userland compressed
  DXRs, EXEC, PURE reload and TSFS restart extents to the block format.
  Use source-matched versioned host block encoder and strict TSFS checker.
- [x] v1458 native core/VFS, five Stage-1 assemblies and 53-word KCORE
  identity PASS. Fresh 256K KCC DSK/D6FS cold boot to ROOT/DSH/ECHO PASS.
- [x] Source-matched TSFS: strict checker accepts 53 files/401 extents;
  independent LOGIN, DSH and ECHO reconstruction PASS. Actual LOGIN final
  restart decoded by native PDP-6 fixed core/VFS PASS.
- [x] Full 256K TSFS-root login/DSH/external EXEC cold boot passed with
  the actual TSFS media, completed external ECHO and returned shell
  prompt. A first attempt exhausted 240 seconds while sampled in
  `dtc_search_wait`; the independent rerun completed with a 480-second
  DECtape-motion bound, with no source changes or relaxed guest checks.
- [ ] Accept DTC/MTC real device boot, 48K runtime and 32K strict
  permanent placement; KCORE is 9701 words, current 32K placement needs
  another 45 words. Preserve checked storage ownership invariants.
- [ ] Rerun full 256K swap/OOM/recovery guest with block-only executables
  before carrying over the earlier v1406 full-guest acceptance to v1464.

## 2026-11-10 v1462: merge complete, production D6LZ36 retained

- [x] Integrate all fetched feature-branch tips and their common ancestors
  in DAIMOS, daimos-testkit and pdp10-doc. Preserve unique historical
  status sections and independently committed native toolchain advances.
- [x] Reconcile the divergent v1459/v1461 codec branch histories; retain
  new versioned sources/tests without selecting the new production decoder.
  Restore original D6LZ36 Stage-1/KCORE/host/userland release inputs.
- [x] Build fresh lowmem disk-root and measure KCORE 9687, SWAP MRES 1762,
  KCORE+MRES 16367; the known strict single-root overage remains 31 words.
- [x] Re-run native PDP-6 BACKSTORE ownership, checked swap-in rollback,
  metadata and exchange cleanup tests against the merged source.
- [ ] Repair the 31-word strict single-root placement deficit after the
  remaining correctness/integration work, without changing kernel ABI.
- [ ] Complete trusted MEMFS raw-XIP RUN/EXEC and swapless PURE pressure
  scheduling; then independently consider the block-codec production
  cutover using the retained v1461 and earlier acceptance suites.

## 2026-11-10 v1406: current physical swap acceptance and remaining scope

- [x] Fresh strict 256K PDP-6 KCC DSK/D6FS F07 full guest on committed
  v1405 checked-release code: HALT/EXIT, original F07 postflight, real
  no-partner recovery, independent MM owner lifetime PASS. 34 swap-ins,
  MM pressure/cooldown 1/1, all 4 independent MM extents freed, no leaked
  process or BACKSTORE blocks. Versioned, SHA256-verified evidence archive
  `$TMPDIR/daimos-swap-checked-release-v1406-evidence-r1.tar.gz`.
- [ ] Exercise refused BACKSTORE release through actual slot-0 scheduler
  and TERM event cleanup; the native v1405 test checks a direct-call retry,
  not automatic scheduler retry. Abnormal bitmap corruption remains a
  separate fault-injection acceptance criterion.
- [ ] Diagnose/fix historical v812 integrated exchange test's pre-existing
  runtime assertion, also reproduced against unchanged v1404 source.
- [ ] Strict 32K single-root permanent-placement gate is **31 words over**;
  remove the remaining excess without losing checked ownership behavior.
- [ ] Complete privileged raw MEMFS XIP RUN/EXEC and swapless PURE physical
  RAM reclamation/scheduler integration. Grouped D6 codec stays deferred.

## 2026-11-10 v1405: verified checked SWAP-MRES release paths

- [x] Move `backstore_free` before disk->resident publication/accounting
  in production `proc_swap_in`. A release refusal rolls back its new MM
  owner without changing disk record, backing identity, bitmap or counters.
- [x] Fail closed on refused BACKSTORE cleanup after exchange account
  commit, extension prepare/cancel, failed swap-out, and optional PURE
  verifier scratch release. Never return as if an unpublished run was freed.
- [x] Source-linked native PDP-6 v1405 refusal/retry with actual allocator
  and a successful retry, plus executable ignored-result mutation negative.
  v787/v781/v782 success and refusal branches, native swap-PURE/verifier,
  v958/v803/v1241/v666/v667/v1393 all PASS.
- [x] Measured exact 32K GCC lowmem footprint: fixed KCORE still 9687;
  movable SWAP MRES 1757 -> 1762 (+5), KCORE+MRES 16362 -> 16367,
  strict single-root placement overage 26 -> 31 words.
- [ ] v812 historical integrated fixture still fails on committed v1404
  as well as v1405 (not a new regression); independently diagnose the
  fixture and establish a passing integrated exchange acceptance.
- [ ] Verify the slot-0 scheduler/event response to BACKSTORE bitmap
  release refusal: the direct v1405 transaction can roll back and retry,
  but `proc_swap_mres_service_v481` currently sends a negative result to
  `SYS_EVENT_TERM`, not the allocator-pressure cooldown. Confirm exact
  disk owner retention through fatal cleanup or fail closed as needed.
- [ ] Historical v1316 user-modified fixture does not link the real v795
  verifier. Leave existing uncommitted user edits untouched. Add an exact
  source-linked verifier to that runner only with explicit ownership of it.
- [x] Rerun strict v1349/v1351-equivalent 256K guest postflight on v1405:
  genuine physical no-partner MM pressure, 34 swap-ins, one cooldown, all
  four dynamic MM owners released at sample 13, zero swap/backstore owners
  at HALT, original F07/held-provider/18-child cleanup and six negative
  owner-evidence controls PASS. Archive evidence as v1406/r1.
- [ ] Continue trusted raw-XIP RUN/EXEC and swapless PURE scheduler work.
  Grouped D6 codec remains deferred until those tracks are accepted.

## 2026-11-10 v1401-v1404: MEMFS owner and build regressions closed

- [x] Fix the internal final-chunk MM free: do not delete the only chunk
  descriptor until `mm_free` returns zero. A refusal leaves the resident
  node intact and returns -1 to TRUNCATE; speculative cleanup refuses to
  resume with an untracked candidate extent.
- [x] Verify all sector bitmap owner bits before clearing any. Reject a
  duplicate sector release while another file still shares its chunk and
  reject a second free of an already reclaimed MM chunk.
- [x] Check BACKSTORE rollback free after failed MEMFS eviction write.
  A refused free of an unpublished allocation must hit the established
  fatal storage invariant, not silently leak the reservation.
- [x] Native PDP-6 red/green v1401-v1403 and existing SIMH/AAP v216
  non-writing XIP lifecycle PASS. Final MEMFS MRES 1752 -> 1771 resident
  words (+19), BSS stays 13; KCORE is unaffected by these movable changes.
- [x] Correct stale direct-MRES build dependencies: track both raw MEMFS
  data and snapshot assembler sources in boot `KERNEL_INPUTS`. Verified
  `make ... memfs-mres.dobj` reruns the kernel generator and relinks fresh.
- [ ] Live low-core spill/reclaim and complete unmount MM fault/rollback
  acceptance at 32K/48K/256K; audit remaining SWAP-MRES free consumers.
- [ ] Finish production restricted MEMFS XIP RUN/EXEC and swapless PURE RAM
  scheduler integration. Grouped D6 codec remains deferred.

## 2026-11-10 v1399/v1400: checked MEMFS backing owner transitions

- [x] Keep the backing descriptor until a verified `backstore_free` returns
  success; WRITE must propagate failure before overwriting file data.
- [x] Fail-atomic backing owner release during zero truncate, same-sector
  growth and relocation. Only the new tentative extent is freed on a
  refused release; already-evicted tight-core spill extents are not freed
  a second time by the common success path.
- [x] Scan all 64 vnode backing owners before unmount releases the first
  one. Reject invalid first/later owner references without discarding
  resident/backed state. Return failure through the real MEMFS MRES VFS
  prepare-unmount dispatch. Fail closed on unexpected late MM/bitmap errors.
- [x] Source-linked PDP-6 red/green v1399/v1400 tests and existing SIMH/AAP
  v216 raw-XIP VM lifecycle PASS. MEMFS MRES 1719 -> 1752 words (+33),
  no extra static BSS or fixed KCORE instructions.
- [ ] Verify/propagate failures of `memfs_data_free` and failed eviction-
  write BACKSTORE cleanup, plus low-core spill/reclaim and unmount MM fault
  injection. Do not treat an owner-invalidating partial release as success.
- [ ] Full 32K/48K/256K multi-owner boot and pressure acceptance, trusted
  production XIP EXEC/RUN, swapless PURE scheduler integration remain open.
  Keep the grouped D6 codec deferred.

## 2026-11-10 v1398: checked BACKSTORE free and existing-owner lifecycle

- [x] Change the historic `backstore_free` void result to 0/-1 while
  preserving old PDP-6 call ABI, and reuse the original validated bitmap
  scan for verification and release; no new persistent metadata.
- [x] Make existing-run verification independent of the flag that disables
  *new* BACKSTORE allocations. This permits swap-in/exchange and legal
  teardown after STORAGECTL disables new swapping.
- [x] Make production `proc_swap_detach` check the release result before
  publishing RESIDENT or decrementing used blocks. v1393/v795/v777 native,
  v880/v957 host and v959/v1083/v1086/v1087/v921 source gates PASS.
- [x] Correct the native swap-PURE verifier fixture to link the real
  0200-word SWAP-owned scratch buffer (not the filesystem DMA workspace).
  Normal and verifier-enabled PDP-6 tests PASS.
- [x] Controlled, matching full 32K GCC disk-root baseline comparison:
  fixed KCORE 9694 -> 9687 (-7 words); permanent-address overage 33 -> 26.
- [ ] Propagate checked BACKSTORE release results through MEMFS write,
  resize, reclaim and unmount: do not drop the sole vnode/backing owner on
  a refused free, and do not mutate resident data before a fallible cleanup.
- [ ] Check the remaining swap-transaction MRES callers that historically
  discard void BACKSTORE return; retain disk and MM ownership if refusal is
  possible, and validate the full 256K live swap/OOM pressure matrix.
- [ ] The grouped D6 codec is still deferred. Continue safe restricted raw
  MEMFS XIP and swapless PURE production lifecycle work independently.

## 2026-11-10 v1396/v1397: verified correctness increments

- [x] MEMFS XIP: freeze CHMOD, CHOWN and UTIME while a raw image pin is
  active; preserve the existing vnode mode/owner/timestamp and allow those
  operations after final unpin. Source-linked native PDP-6 PASS; SIMH and
  AAP restricted user lifecycle PASS. MRES resident +7, KCORE +0.
- [x] Fix MEMFS native test helper ABI (swap detach success is zero, wall
  clock service available); fixture now validates real VM teardown rather
  than a stale success convention. No production VM behavior changed.
- [x] AUXSTORE: canonical zero,zero output for newly formatted absent
  regions, preserving v1 reader compatibility with preexisting zero-length
  regions whose start is nonzero. Host v636/v507/v492/v657 PASS. Do not
  reject legacy media or create a redundant v2 descriptor parser.
- [x] PURE DROP: after MM rebase refusal, fail closed if cleanup unpin fails
  (MM_ERR_BUSY, TRANSITION retained). Native v1397 fault injection and
  v1381/v1384 regressions PASS. Unlinked 518->521 TEXT, DATA/BSS zero.
- [ ] Continue full AUXSTORE/BACKSTORE physical media and swap/OOM recovery
  matrix; restricted trusted XIP vnode admission and RUN/EXEC scheduling;
  and measurable 32K/48K PURE RAM reclamation under process pressure.
- [ ] Grouped D6 codec stays deferred until all three feature tracks are
  complete and accepted. No format cutover or duplicate decoder now.

## 2026-11-10 v1393-v1395: active integration order and D6 deferral

**Current priority decision:** Continue AUXSTORE/BACKSTORE, privileged raw
MEMFS XIP, and swapless PURE RAM reclamation. The grouped D6 codec format is
still under discussion and its atomic production cutover is **DEFERRED until
all three preceding feature tracks are finished and accepted**. This
supersedes the earlier ordering that made the v053/v055 codec a precondition
for the other tracks; it does not cancel the eventual codec replacement.
Keep production D6LZ36 unchanged. Plain DXR2 PURE prefix restoration and
the existing safe full-image fallback can progress independently; compressed
PURE_PREFIX remains unsupported until a selected format has a bounded
decoder. Do not add another permanent decoder or a mixed-format fallback.

- [x] v1393: BACKSTORE free first checks every bitmap owner bit before
  releasing any blocks or decrementing usage; a disabled allocation policy
  still permits release of existing valid owners. Native PDP-6 ownership,
  boundary and bitmap-verifier tests PASS. No additional BSS.
- [x] v1394: stage an exact, root-only, trusted raw-XIP instruction policy:
  one stackless MOVEI AC2,immediate then UUO 040 EXIT, no other nonzero
  image words, no argv/env and no user stores. Host and PDP-6 tests PASS;
  isolated 80 TEXT, zero DATA/BSS. It is NOT linked into production.
- [ ] Connect XIP only after real pinned-vnode source trust, immutable
  provenance, atomic RUN/EXEC transaction, APR/scheduler, BRK/swap exclusion
  and native booted lifetime acceptance. Keep ordinary EXEC/RUN unchanged.
- [x] v1395: staged PURE FULL restoration now refuses failed target/source
  unpin or target free on rollback, retaining TRANSITION. Native PDP-6
  four-fault rollback/commit test and existing shared FULL image test PASS.
  Separate C transaction 359 -> 368 TEXT, zero DATA/BSS; NOT linked.
- [ ] Complete real scheduler/PI/VFS and 32K/48K guest acceptance for PURE
  reclamation, compare total net RAM (not just dropped PURE text), and only
  then replace old resident transaction code without duplication.
- [ ] Complete remaining AUXSTORE/BACKSTORE consumer and media/recovery
  integration, preserving exact owner accounting and no-disabled-allocation
  assumption during cleanup; rerun the full 256K swap/OOM matrix after the
  production MRES release-path change.

## 2026-11-10 v1391/v1392: bounded FULL safety and compiler integrity

- [x] Stage FULL restore with a four-argument, MM-owned APR capacity
  callback in `proc_pure_ram_restore_v1391.[ch]` and the common DXR
  assembler; remove the unsafe unbounded shared FULL entry. Reject
  oversized initialized images *before* reading or decompressing.
- [x] Native PDP-6 v1391 acceptance: plain/compressed oversized DXR,
  exact-size image, zero/non-APR capacity, invalid physical base,
  upper physical bound, mount 18, immutable suffix overlay and
  failure rollback PASS. C89 syntax and KCC/DAS assembly PASS.
- [x] Fix root-cause optimized KCC constant-index symbolic ADDI
  fold in `kcc/ccopt.c`: locally proven immediate constants are
  embedded in the address, not left as a stale AC index. Native
  v1392 fixed-slot assembly and runtime regression PASS.
- [x] Reconcile the historical v1387 test VFS provider with v1391:
  the new oversized-image modes only compile in FULL tests. The
  original and corrected compilers both PASS v1387 baseline.
- [x] Record the v1391 isolated size gate: 161 common DXR TEXT,
  74 bounded adapter TEXT, 359 separate restore C TEXT, zero
  static DATA/BSS. Replacing EXEC plus old FULL loader is still
  **+121 words before folding**, excluding the extra C transaction.
- [ ] Minimize a compiler reproducer that fails under the original
  KCC binary and passes with the v1392 optimizer fix without
  relying on a long DAIMOS diagnostic fixture. The archived
  diagnostic codegen failure is established, but today's cleaned
  regression no longer fails on the old compiler.
- [ ] Make FULL bounded and RAW suffix restore a single production
  implementation only after KCORE fold/size and 32K/48K guest RAM
  high-water acceptance. Do not link both v1381 and v1391 FULL
  restore bodies; do not switch raw disk/drum swapping until its
  ABI and behavioral regressions pass.
- [ ] Complete grouped compressed PURE_PREFIX with guaranteed
  output boundary, hardware PI and VFS concurrency, EXEC/EXIT/WAIT
  lifecycle, and full boot acceptance.

## 2026-11-10 v1389/v1390: shared DXR size and FULL reload proof

- [x] Reduce duplicate callee-save frames in the staged common
  parser/backing bridge using the established PDP-6 BLT ABI.
  Native v1386/v1387/v1388 PASS after this change; shared module
  TEXT **225 -> 212**, without additional static data.
- [x] Implement the staged two-instruction FULL-backing entry into
  the existing shared DXR inspector/reader and verify it against
  real source-linked v1381 FULL restore plus suffix overlay.
  Native v1390 PASS for plain and compressed DXR2, mount 18,
  corrupted/short input and safe rollback. This is *not* the
  production disk-swap loader.
- [x] Fix the native VFS fixture header-read contract: a two-word
  request must never write the third TX2 word. The previous extra
  store caused a stack-layout-dependent v1387 mount-18 failure.
  Exact count-bounded writes pass both the earlier and broader
  test-only compression-mode conditions. No compiler change needed.
- [x] Add executable size regression
  `daimos-testkit/tests/validation/exec-image-size-v1390`:
  production EXEC=183, staged EXEC caller=119, shared core=161,
  shared backing adapter=51, old FULL leaf=50. +97 words for
  candidate EXEC or **+98** with independent FULL replacement,
  isolated and pre-fold. No DATA/BSS.
- [ ] **RAM-first gate:** measure the actual p10fold-linked
  mandatory KCORE with all candidate modules, and the full
  32K/48K swapless tape-root resident and high-water costs,
  before activating. The hypothetical 98-word DXR increase
  excludes the much larger MM/scheduler transaction implementation.
  Evaluate released PURE sectors for real concurrent workloads.
- [ ] **Memory safety:** final FULL ABI needs explicit available
  destination APR extent size, and MUST reject any DXR image larger
  than that extent *before* the file reader or decoder writes it.
  A trusted file key plus matching TX2 text words is insufficient
  proof of full image length. Add native oversized-header/overlap
  mutation cases with real MM ownership before production.
- [ ] Finish grouped compressed PURE_PREFIX decoding, physical
  scheduler/PI/VFS source-lifetime tests and actual boot acceptance.

## 2026-11-10 v1386-v1388: shared image engine proven, link size OPEN

- [x] Stage a **single** DXR1/DXR2 parsed metadata and payload engine
  `exec_image_common_v1386.s`, with FULL and PURE_PREFIX modes,
  unchanged plain and bounded-compressed FULL behavior, a strict
  writable-suffix output boundary, and unsupported compressed prefix
  fallback. No new per-process descriptor or disk swap behavior.
- [x] Verify on PDP-6: v1386 parser/payload positive and malformed
  negative vectors, v1387 use through actual PURE MM drop/restore and
  mount-18 backing, v1388 disposable EXEC caller using the common
  parser/loader with DXR1/2, compressed FULL, RT root gate,
  pre-VM failure, no-memory and rollback. All have native PASS.
- [ ] **Reduce text before production link.** Original EXEC loader
  183 target words; candidate caller 119 plus common core 169 = 288
  **(+105 isolated, pre-fold)**, with the restoration adapter adding
  another 56 text words. Optimize common/parser call overhead,
  consider safe fold opportunities and compare actual full-profile
  linked sizes before accepting any permanent KCORE growth.
  All newly staged source remains unlinked.
- [ ] Fold existing `proc_swap_reload_pure` full-image semantics into
  the same validated loader without regressing raw disk/drum swap,
  including source lifetime and FULL compressed decode.
- [ ] Complete grouped D6codec bounded compressed PURE_PREFIX,
  native concurrency and true 32K/48K swapless tape-root boot
  acceptance. The v1385 independent parser stays testkit-only and
  must never become a second production loader.

## 2026-11-10 v1384/v1385: prefix-only RESTORE staged, shared loader pending

- [x] Stage `mm_pure_expand_v1384.s`: claim/release/commit the
  original PURE prefix without another allocation, retaining pinned
  PACKED_PROCESS ownership during reads. Fail atomically if the
  original gap is occupied/unmanaged, owner/type/size/pins disagree,
  or original APR alignment is invalid.
- [x] Stage `proc_pure_restore_v1384.c`: safe in-place prefix recovery,
  rollback of a partial/unsupported loader, PI-atomic resident
  publication, and explicit retry/fallback for occupied gaps.
- [x] Host v1384 and PDP-6 native v1384/v1385 testkit acceptance:
  exact suffix integrity, no new RAM allocation, MM preflight and
  pressure, failed/short read, unsupported compressed input,
  high mount 18 backing and subsequent full-image fallback PASS.
  The native assembler arena-high comparison defect was fixed.
- [ ] **Complete production cutover only after the v1386-v1388 code
  size and full-system gates above pass**:
  `exec_load_process` (FULL initial image),
  `proc_swap_reload_pure` (FULL reconstruction) and the testkit-only
  `exec_prefix_fixture_v1385.s` (PURE_PREFIX) must use ONE
  validated DXR header/payload engine. Keep the v1385 loader as an
  unlinked native test fixture only; it must not become a permanent
  second DXR parser. The restore transaction alone handles MM claims,
  error rollback and process publication.
- [ ] For compressed DXR, add a bounded prefix decoder or independent
  text-frame encoding as part of the atomic grouped D6codec cutover.
  A truncated output limit alone currently fails on MATCH tokens
  crossing the text boundary. Until accepted, use the already
  validated full-image restore fallback and memory-pressure retry.
- [ ] Prove the integrated full 32K/48K swapless guest, hardware PI,
  immutable filesystem source, EXEC/EXIT/WAIT and true linked KCORE/
  RAM high-water acceptance. Isolated native object sizes are 323
  transaction + 130 MM + 78 test-only plain loader TEXT words; no
  linked production size increase has been measured or accepted.

## 2026-11-10 v1383: one-word size geometry and prefix-only RESTORE

- [x] Confirm the sizing model from actual DXR2/EXEC and VM code:
  36-bit `TEXT_WORDS,,LIVE_MUTABLE_SUFFIX_WORDS` fits both 18-bit
  counts. DXR2 already records uncompressed initialized-image and TX2
  text lengths; `vm_state` holds current allocated size. The existing
  PURE backing uses a 14-bit text field. A further permanent word
  is NOT needed solely for inactive raw-PURE size recovery.
- [ ] Implement an **atomic reverse MM rebase** only if the released
  prefix gap remains wholly free. Require original APR alignment,
  exact suffix ownership/type/size and one pin, plus safe PI/scheduler
  serialization. Never change mutable suffix bytes during prefix
  reconstruction; on failure restore the inactive representation.
- [ ] Add a plain DXR prefix-only load and an independently validated
  **bounded compressed prefix decode** for the selected grouped codec
  format. The currently linked D6LZ core rejects a token whose match
  overlaps the requested output boundary; the existing compressed
  PURE reload writes the full initialized image. Size metadata alone
  does not make either operation safe. Avoid a permanent legacy
  decoder or scratch-buffer requirement.
- [ ] If the low prefix has been reused, fall back to the existing
  tested full-extent restore under memory-pressure rules, with
  correct deferral when no contiguous region exists. Native
  daimos-testkit cases must cover exact size, boundary-crossing
  compressed matches, occupied gaps, plain/grouped DXR, EOF and
  rollback. The v1383 note is a design reconciliation, NOT a
  production implementation or regression pass.

## 2026-11-10 v1381/v1382: zero-copy PURE release staged, not live

- [x] Design a **no-second-allocation** PURE drop: retain mutable words
  in place, drop the physically lower PURE prefix from the owner-tagged
  MM descriptor and atomically publish saved backing, inactive VM and
  existing two-bit mode while PI is disabled. No data copy, no new
  descriptor slot, no per-process state, no disk swap changes.
- [x] Implement the staged `mm_pure_rebase_v1381.s` primitive with
  original-size/base, 18-bit physical address, sector-prefix, exact
  owner/type and one-pin preflight. Validation failure leaves
  original data and MM descriptor unchanged. An unpin failure after
  commit preserves TRANSITION for explicit recovery.
- [x] Host C89 regression and native PDP-6 KCC/DAS v1381 PASS:
  repeated drop/restore, zero allocations/frees during drop, fail
  injection, owner/pin/type/size/range checks and terminal cleanup.
  Native v1382 also PASSes existing vnode/mount PURE guards, including
  fifth-bit mount-id 18. The old v1377 host MM mock now rejects
  overlapping physical extents; previous tests continue to pass.
- [x] Measure isolated code/physical-RAM effects: v1379 567 C words
  versus v1381 518 C + 70 ASM = **588** pre-fold text words, zero
  static data/BSS. For sample 02000-octal process/0400-octal PURE
  prefix, peak RAM during DROP falls from 1792 to 1024 decimal words,
  while final inactive suffix remains 768. This is NOT a production
  linked-KCORE measurement.
- [ ] Integrate only after verifying actual hardware PI/allocator and
  VFS serialization, process selection/wake/teardown, and full
  swapless no-partner boot. The restore transaction still needs a full
  destination while holding its source suffix, so RESTORE pressure,
  recovery and peak RAM remain a separate priority. Prevent publishing
  any invalid runnable state if an MM unpin fails after commitment.
- [ ] Native 32K/48K tape-root regression under concurrent full MM,
  EXEC/RUN, STOP/SLEEP/WAIT/EXIT and constrained memory pressure;
  measure overall linked KCORE size, total RAM high-water and CPU
  before enabling the new path.

## 2026-11-09 v1379/v1380: registry-free PURE staging (superseded drop)

- [x] Remove redundant dense/sparse inactive process registry from the
  **raw PURE-only** transaction. The owner/type/size of one
  MM_TYPE_PACKED_PROCESS extent and the original VM/immutable PURE
  metadata are enough to restore it; use the existing assembler
  storage-mode map and avoid a second permanent per-slot index.
- [x] Native KCC PDP-6 v1379 host/target tests PASS three cycles,
  allocation/restore/MM failure rollback and terminal cleanup. Native
  v1380 reuses actual resident PURE vnode and mount busy scanners:
  mount 18 fifth-bit disambiguation and every old process/retained
  suffix release window PASS.
- [x] Measure the pre-link improvement: transaction 672 -> 567 KCC
  text words, zero DATA/BSS. The staged C registry dependencies
  (121+310+273 = 704 words) are removed from the new test link.
  This is not a full linked KCORE size/CPU/RAM improvement claim.
The previous transient-second-suffix DROP issue was solved by v1381
using an in-place **MM descriptor rebase**, with no data copying.
The existing `mm_process_shrink_pinned` retains its APR alignment
contract. The new primitive is still separate and not live.
- [ ] Finish the actual minimal swapless build and process lifecycle:
  KINIT registry-free mode selection, VFS/provider serialization,
  real MM compaction, process scheduling/reload, EXEC/EXIT/WAIT
  fault recovery and native 32K/48K high-water measurements.
  Leave v1377/v1379 out of production until accepted.

## 2026-11-09 v1377: historical prototype (superseded by v1379)

- [x] Implement an **unlinked** PURE-only stop/drop/restore/discard
  transaction using owner-checked managed RAM, the existing storage
  map/registry and exact mutable-suffix copies. Require the explicit
  `DAIMOS_SWAPLESS_PURE_RAM_V1377=1` compile gate; no BACKSTORE
  dependency, no codec and no disk-swap record change.
- [x] Validate host dense/sparse repeated drop/restore, no-eligible
  image, interrupted/failing MM and loader operations, terminal ZOMB
  release and mount-extension cleanup; native KCC/PDP-6 unit tests for
  real in-core suffix copies, reconstruction, MM shortage/retry and
  discard all PASS in daimos-testkit v1377. The failed-restore MM pin
  leak found during testing was corrected.
The old registry allocation and 672+704 word prototype footprint are
retired from the implementation queue. The remaining full-system
acceptance, zero-disk mode and transient-RAM questions are tracked in
the current v1379/v1380 section above. Preserve the historical v1377
regression evidence; do not reactivate its per-slot layout metadata.

## 2026-11-09 v1375: swapless reclamation replaces compressed disk swap

The former decision 5/P0-D executable-opt-in **compressed process disk
swap** feature is CANCELLED. Production raw disk/drum swap stays intact.
Do not implement an encoded disk-swap record type or use the historical
PACKED v1-v46 code as a production-ready subsystem.
The separate v1298 **P0-D OOM test suite** remains closed; the name
collision is only with the previous summary's workstream label.

- [ ] **PURE dropping on swapless systems:** after validating immutable
  executable backing and the PURE prefix, quiesce a process, retain all
  writable user words in managed RAM, free the old APR-aligned process
  extent, then reconstruct PURE from its guarded vnode and restore all
  writable bytes before scheduling. Preserve mount/file lifetime, process
  state, PID/WAIT and failure rollback. No partial runnable images.
- [ ] **Opt-in mutable-RAM compression, swapless systems only:** default
  uncompressed; authorize per validated executable/privileged policy;
  record eligibility separately from actual RAM representation. Compress
  only after a two-phase ownership-preserving transaction into separate
  RAM, and only with proven net memory savings. Preserve raw-RAM
  fallback, exact restored word count and no temporary peak-RAM hazard.
  Never allocate or write a disk swap record.
- [x] **v1376 staged codec proof:** implement a bounded, allocation-free
  36-bit-word RAM run codec; pass host roundtrip/error/count limits,
  native KCC-generated PDP-6 encode/decode/error ABI and DAS/DLINK
  acceptance. Standalone KCC object is 449 code words with zero BSS/data.
  This is a component PASS, NOT process eviction or a net-RAM improvement.
- [ ] **v1376 next:** measure a PURE-only no-compression inactive-state
  lifecycle, then compare a smaller grouped-codec reuse or faster/lower
  text-size alternatives before linking the staged run compressor.
- [ ] Measure 32K/48K swapless KCC linked residency, temporary workspace,
  scheduler/VM overhead, APR fragmentation and restore headroom. Native
  SIMH/AAP tests in daimos-testkit: multiple inactive processes, PURE
  reconstruction, mutable bytes, successful resume, EXEC/EXIT/WAIT/reap,
  denied mutation/unmount, non-PURE/incompressible fallback, MM/provider
  failures, OOM and negative rollback. No production implementation has
  been enabled yet.
- [ ] Implement only in explicitly swapless profiles. Existing
  `ROOT=tape TAPE_AUX=none TAPE_TMPFS=0` continues to omit swap,
  BACKSTORE, AUXSTORE, LOGSTORE and RAM-only TMPFS/MEMFS. Swap-enabled
  systems retain their present raw process swap without the new policy.
  See `../pdp10-doc/KERNEL/SWAPLESS-MEMORY.MD`.

## Unified DECtape filesystem redesign (documentation direction, 2026-11-09)

- [ ] Replace the two native DECtape formats and providers with one TSFS
  namespace/read/extent format. TSFS root is ALWAYS read-only; only non-root
  one-member TSFS may be writable; multi-member TSFS is always read-only.
  Drop legacy media/foreign format
  compatibility and do not link two resident readers.
- [ ] Measure native 32K/64K PDP-6 tape-root memory AND tape latency.
  Preserve the strict low-core budget, shared 128-word block workspace,
  transient mount validation and compact per-drive state. Host-side placement
  optimization must not impose persistent kernel tables/buffers.
- [ ] Use grouped D6codec for compressed immutable TSFS payloads, not the
  current production D6LZ36. Reuse the shared grouped decoder and perform
  codec/boot/TSFS format activation as one validated transition.
- [ ] Decide write durability, mtime and per-file access classes for non-root
  one-member RW TSFS volumes. Prove controller multi-drive prepositioning
  against real Type-551/555 semantics and corrected AAP behavior.
- [ ] Implement build-time `TAPE_AUX=none|drum|disk` and `TAPE_TMPFS=0|1`.
  The minimal `ROOT=tape TAPE_AUX=none TAPE_TMPFS=0` build has NO process
  swap, BACKSTORE, LOGSTORE, AUXSTORE, or RAM temporary filesystem modules,
  tables, media reservations or initialization. No runtime disk/drum probe
  may enable services absent from that build. Explicit disk/drum backing
  requires a matching configured device; failure must be reported clearly.
  Verify RO root, denied RW remount, out-of-memory behavior and optional
  secondary single-member RW TSFS on real 32K/64K boots.
- [ ] Introduce only unversioned active source, utility, ABI and test names;
  rename existing suffixed identifiers only in a coordinated, validated
  caller/build/test cutover. See `../pdp10-doc/KERNEL/DECTAPE.MD`.

## v1351: booted 256K F07 owner lifetime and native codec baseline accepted

- [x] Complete a **second fresh** physical 256K PDP-6 guest, with
  original 18-child RUN/ACK and TERM/WAIT, requester OOM, protected
  held provider, independent spinner count 8 >= 5, HALT, SIMH EXIT;
  original v1075/v1150/v1207/v1336 postflight PASS.
- [x] Prove one real no-partner `MM_ERR_NOMEM`/cooldown, 34 successful
  swap-ins, zero exchanges, final zero swap and backstore blocks.
- [x] Integrate source-linked two-phase owner-lifetime verification
  in the *same* original guest postflight: four 8K extents and real
  spinner pin before shortage, exact post-cooldown independent
  free/unpin, prepared=4, freed=4, count=0, error=0 at HALT.
  Six owner mutations plus two missing/duplicate mandatory-field
  mutants rejected. Testkit `f156746`; docs `ac34157`.
- [x] Preserve immutable v1351 evidence:
  `$TMPDIR/daimos-swap-complete-lifecycle-v1351-evidence-r1.tar.gz`,
  SHA256 `4ab99004129151bc475e7a5a577d39ebf336e6c290acb701696c5f1655e7e644`.
- [x] Finish the reproducible staged grouped D6CODEC native
  regression baseline: v045/v047/v048/v049/v051/v055 38/38 PASS,
  testkit `239a191`, docs `ac34157`. Evidence:
  `$TMPDIR/daimos-d6codec-native-baseline-v1350-evidence-r1.tar.gz`,
  SHA256 `a1052dfa875e91213faf39da2643c9b87721492d2d365c281f98ddfe13ce5ea3`.
- [ ] Next: atomic grouped codec production/boot/consumer cutover,
  then remaining BSTORE, restricted XIP, swapless PURE/RAM reclamation, grouped
  integration and final permanent KCORE RAM minimization. The native
  codec staging PASS is not a production cutover acceptance.
- [x] No production KCORE/MM/FS/PI6/swap source modification, new
  permanent RAM growth or historical v1208/v1211 fallback required
  for the accepted 256K raw-swap no-partner workload.

## v1348: actual booted no-partner and original held-provider F07 CLOSED

- [x] Complete the genuine 256K PDP-6 F07 full-system guest with all
  18 RUN/ACK and TERM/WAIT children, real held-provider/foreign
  waiter, requester-only OOM, correct spinner SLEEP count (8 >= 5),
  original HALT, and complete SIMH postflight/EXIT. The v1348
  disposable-only INIT barrier waits for actual scheduled spinner
  progress; no original safety threshold was weakened (`8ea7d66`).
- [x] Reverify booted no-eligible-partner swap-in: 1 genuine
  `MM_ERR_NOMEM`, 1 pressure cooldown, 34 successful swap-ins,
  zero exchanges, zero outstanding swap and backstore blocks.
- [x] Independently verify source-linked two-phase real MM owner
  lifetime against the completed v1348 guest: four genuine 8K
  allocations, resident process pin, exactly one post-cooldown
  independent release, four matching frees, no outstanding owner,
  and six negative controls rejected (v1349 offline verifier).
- [x] Retain validated v1348 original suite evidence:
  `$TMPDIR/daimos-swap-original-suite-v1348-evidence-r1.tar.gz`,
  SHA256 `d97d0d05ef0c0cf6bd047d99e8c81ca64c5496b8b81690b50f588dea409e9951`.
  Docs `pdp10-doc/KERNEL/SWAP-FULLGUEST-COMPLETE-V1348.MD` (`453e103`).
- [ ] Complete separate v1349 full-guest rerun with two-phase owner
  verifier wired into its own after-HALT evidence manifest. The
  already completed v1348 full-system acceptance does NOT depend
  on claiming this later repeat passed.
- [ ] Continue distinct remaining KCORE feature tracks, grouped
  integration regressions and RAM minimization. No extra fallback,
  MM/swap production change or permanent kernel growth was needed.

## v1347/v1348: full physical guest HALT, strict spinner witness pending

- [x] Complete the unbounded 256K PDP-6 physical no-partner guest:
  18 authentic RUN/ACK and TERM/WAIT reaps, requester/witness KWP,
  original HALT and SIMH EXIT, one real MM_ERR_NOMEM/cooldown,
  34 genuine swap-ins, zero final swap and backstore blocks.
- [x] Correct v1303 fixed postflight schema to ignore newer optional
  source-linked live sampler addresses not emitted at final HALT.
  Require all established original counters; missing/duplicate
  mandatory fields are rejected. Testkit `a7454e4`, docs `77fb0ed`.
  Evidence `$TMPDIR/daimos-swap-fullguest-halt-v1347-evidence-r1.tar.gz`,
  SHA256 `dcab588ef04e13280b8417ce017e7e47a012ea89cb99fe4a1246eb3cc4d65045`.
- [ ] Close the *remaining* v1150 held-provider spinner witness:
  the completed v1347 guest has genuine spinner count 4, but the
  original test requires 5. The separate disposable v1348 test
  adds an INIT read-only wait barrier for five actual scheduled
  spinner SLEEP events before TERM. Preflight/source injection PASS;
  complete and verify the full new guest without relaxing this gate.
- [ ] Do not change kernel/MRES or add fallback: no production
  memory, device, scheduler, or swap defect is established.

## v1346: fix final no-partner postflight refusal classification

- [x] Distinguish common `proc_swap_out_fail` preflight rejection
  from a proven fatal swap-out transaction. It rejects ineligible
  victims, including pinned VM; a nonzero entry count is not an
  independently fatal published backing failure. Testkit `f12880d`.
- [x] Preserve strict end-of-guest HALT, all 18 child reaps,
  actual typed MM pressure/cooldown, correct swap-in accounting,
  zero fatal swap-in and zero final swap/backstore allocated words.
- [x] Original v1303 selftest and v1345 linked physical guest
  evidence verifier PASS; seven v1346 unsafe negative controls PASS.
  Documented in pdp10-doc `25a1772`.
- [ ] Finish 256K booted physical no-partner follow-up guest and
  prove HALT, process termination and zero leaked MM/backing; the
  v1346 verifier correction is not a full-system acceptance result.

## v1345: real slot-0 DRM event and independent provider return verified

- [x] Confirm nine booted PDP-6 `proc_wait_boot` samples have the
  actual DRM236 active-request completion address in AC1. Existing
  native DRM watchdog and MINIT regression PASS. No established
  production I/O/PI6 defect.
- [x] Preserve v1343 26-sample stalled guest read-only evidence before
  stopping its disposable simulator, with SHA256 manifest.
- [x] Bounded v1344 physical guest: one true no-partner MM_ERR_NOMEM,
  four original 8K physical owner frees, actual process unpin,
  successful swap-ins 13 to 18, then three real fatal-busy failures
  and one normal held-provider return through guarded v1343 release.
- [x] Source-linked v1345 independent evidence checker rejects six
  mutation controls; testkit `453e269`, docs `178d5f0`.
  `$TMPDIR/daimos-swap-slot0-device-recovery-v1345-evidence-r1.tar.gz`,
  SHA256 `8045c4f8f79e4893f31a0d39aaa347c8573b1619603d1ac5807f10e22d70ccd4`.
- [ ] Finish strict 256K full-guest acceptance: 18 real child
  TERM/WAIT, requester/provider/witness milestones, guest HALT,
  exact MM extents/pins and zero backing allocation. The v1344
  run intentionally stopped before this phase; it is NOT a PASS.
- [ ] Keep production KCORE/MRES footprint unchanged and exclude
  v1208/v1211 speculative fallback absent a measured kernel defect.

## v1343: original provider release preconditions verified, fixture fixed

- [x] v1342 booted 256K PDP-6 source-linked counters: real OOM=1,
  fatal provider-busy=12, holder entered=1, release=0, holder
  returned=0, INIT CHILD wait, holder test event wait. Proven
  disposable v1150 release dependency, not production kernel bug.
- [x] Preserve original raw SIMH/map/source/addresses with verified
  manifest: `$TMPDIR/daimos-swap-provider-dependency-v1342-evidence-r1.tar.gz`,
  SHA256 `9a286c2a48e635d19411fab3fa2cbc453c99763561608200fcf704130bbb875b`.
- [x] v1343 test-only spinner may call original guarded provider
  release after real OOM and >=2 busy failures; original holder
  must still execute normal unlock. Source preflight and three
  unsafe injection mutations PASS (testkit `28bd504`).
- [ ] Fresh booted v1343 full guest: prove holder release, requester
  exit, provider/witness milestones, all 18 TERM/WAIT, HALT,
  zero MM owners/pins and zero swap backing. No final acceptance
  before the original strict postflight verifier passes.

## v1342: diagnose provider-held guest after real MM release

- [x] Decode the last 24 genuine v1340 SIMH samples using linked
  `proc_table`/`proc_timer_clock`: clock advances; INIT CHILD wait,
  requester timer sleep, gateway provider-event wait, spinner timer,
  holder on disposable v1150 hold event (testkit `24dd9bd`).
- [x] Identify v1150's *test-only* provider-release prerequisite:
  INIT must enter sleep after genuine OOM and >=2 fatal provider
  busy observations. It was instead waiting for the requester.
  Do not infer a production deadlock from this dependency alone.
- [x] Stage v1342 read-only actual `real_oom`, `fatal_busy`, release,
  provider, holder and predispatch counter sampling; selftests PASS.
- [ ] Interpret fresh v1342 full-guest counters. If the original
  release prerequisite was reached but its INIT-only hook was
  missed, fix the disposable workload's release ownership, not
  production MM or scheduler. Preserve exact provider lease
  unlock, independent real user execution, and strict final checks.
- [ ] Complete original 18-child RUN/TERM/WAIT and OOM/provider
  milestones, final HALT, exact MM owner/pin and backstore cleanup.

## v1340: independent no-partner recovery subcase PASS, final cleanup OPEN

- [x] Booted 256K PDP-6: one real swap-in `MM_ERR_NOMEM`, one no-
  partner cooldown, independent slot-5 user sees cooldown=1 and
  invokes exactly one original typed owner release; four 8K-word
  real MM owner extents freed, spinner MM pin released, owner/error
  counters 0; normal swap-in then grows from 13 to 18. No kernel
  code, additional permanent memory, or speculative fallback.
- [x] Correct testkit SIMH Ctrl-E race dropping guest `@%&`
  immediately before stop acknowledgment; v1340 selftest and
  early-HALT/truncated-postflight controls PASS (`e9dc25c`).
- [x] Preserve v1338 source-linked traces and SHA256 evidence:
  `$TMPDIR/daimos-swap-independent-release-v1340-evidence-r1.tar.gz`,
  `cb5ebee862a0129e9955ef3beaba4635bca8f796aeda19166855b5241f63573e`.
- [ ] Complete v1340 source-linked `proc_timer_next` frontier
  investigation following INIT `^`: sleeping process wakeups,
  run queue, final requester/provider milestones, 18 TERM/WAIT,
  full guest HALT and zero outstanding MM/backing resources.
- [ ] Do not promote immediate post-release swap progress to
  full-system no-partner acceptance until strict final verification.

## v1336: genuine booted no-partner shortage observed; recovery still OPEN

- [x] Verify booted 256K v1335 live source-linked MM pressure: 93
  actual MM_ERR_NOMEM swap-in failures, 93 no-partner cooldowns,
  zero exchanges/fatal swap-ins, 54 swap-outs, 37 swap-ins, request
  26624 words, and four real 8K-word owner extents. Testkit
  `c642d28` independently verifies raw SIMH + addresses with eight
  executable negative counter mutations; docs `d143f08`.
- [x] Classify blocked INIT versus real production kernel liveness:
  INIT held test physical owners while awaiting the 18th RUN/ACK,
  so no independent release occurred. No final guest HALT or leak
  acceptance; do not label it a production deadlock.
- [x] Prepare a source-authentic independent resident spinner
  with real `mm_pin`/`mm_unpin` and typed owner `mm_free` after an
  observed cooldown (testkit `745690a`); disposable guest preflight PASS.
- [ ] Complete booted v1336 guest: prove release from scheduled
  independent user, subsequent normal swap-in, all 18 RUN/TERM/WAIT,
  original OOM/provider milestones and zero MM/backing leak.
- [ ] Keep KCORE/MRES/production MM and swap unchanged unless the
  complete test reveals an actual defect; no speculative fallback.

## v1330: late MM owner test failed; existing-resident-pin guest pending

- [x] Complete v1323 genuine 256K F07 execution: 18 admissions,
  51 swap-outs/35 swap-ins; one real 1024-word owner allocation
  failed (`!t`), owner count=0/error=-1, swap-in MM failures=0,
  no-partner cooldown=0 and exchange=0. Guest HALTed early with
  3440 backing blocks still outstanding; NOT a completed guest PASS.
- [x] Preserve exact negative traces and linked counters:
  `$TMPDIR/daimos-swap-physical-owner-v1323-setup-failure-r1.tar.gz`,
  SHA256 `04944fa347f6b259ca779ea96126ab440fa1eeeb9081794b56f0ed4d1bf3cb25`.
- [x] New testkit v1326 (`1a0693e`) pins resident MM extents through
  production `mm_pin` and unpins through `mm_unpin`; requester stays
  excluded. v1329 wrapper (`5a0adb8`) selects the prompt-clean
  sampler v1326 (`e436459`); full 256K guest preflight PASS.
- [ ] Complete v1329 booted 256K test and require actual typed MM
  swap-in pressure, no eligible exchange partner, PI6 cooldown,
  independent user work, normal restore, and zero MM/backing leaks.
  No partial sample or fixture-only test is valid acceptance.
- [ ] Do not alter KCORE, MRES or add v1208/v1211 fallback unless
  this controlled full-system test demonstrates a production defect.

## v1325: completed owner-8K negative; live owner-1K test pending

- [x] v1322 real 256K guest reached all 18 additional child
  admissions, 51 swap-outs and 35 swap-ins. The subsequent real
  `mm_alloc_aligned_noreclaim` of one test-owned 8K-word extent
  failed; original guest HALTed with `!t`. Test owner held no leaked
  MM extent. Swap-in allocator errors, cooldowns and exchanges = 0.
- [x] Independent executable-output v1325 verifier and four unsafe
  modified-counter rejections PASS. Real negative guest evidence
  SHA256 `388b09a28389749931eb8208ac008ff2f642fba753aeff933de44ad3ef56fd55`.
  Independent testkit `e6e0d26`, pdp10-doc `2f990d6` and
  separately retained negative evidence SHA256
  `bf5304eb2ce0de91d463b8137c601e9c77f4223bea7cf67ebb3a3e4582800e31`.
- [x] v1323 one-1K-word physical owner extent fixture preflight PASS
  (testkit `c43fe7b`); 256K guest was still running at checkpoint.
- [x] Fix PTY diagnostic's premature-normal-HALT false failure with
  independent v1324 (`7bc46c0`); early HALT and truncated-postflight
  controls PASS. Original v1306 test remains PASS.
- [ ] Verify v1323's final genuine owner admission, MM pressure,
  no-partner branch and backing cleanup; do not infer result from
  a partial SIMH sample or merely from owner allocation success.
- [ ] Keep KCORE and swap MRES unchanged pending reproduction of a
  real production defect; historical v1208/v1211 fallback excluded.

## v1322: actual booted MM-owner no-partner pressure experiment

- [x] Versioned v1321 test-only full guest combines 24K process images
  with real owner-tagged 8K-word physical MM extents. Native syntax,
  generated-runner preflight and real-MM injector PASS; ordinary
  production v1207 mode contract remains unchanged (`6dab196`).
- [x] v1321 exploratory guest deliberately interrupted BEFORE MM
  owner allocation after 3 live samples, 15 swaps out/8 in, no MM
  failures. Not full-guest or no-partner acceptance.
- [x] Quantified the completed v1308 physical F07 MM snapshot:
  15840 free managed words at OOM, insufficient for six 8192-word
  additional extents. Created v1322 with ONE such extent, six-to-one
  count adjustment in disposable fixture and strict owner verifier;
  preflight PASS, testkit `b961c8f`.
- [ ] Complete and independently verify the v1322 booted 256K guest:
  exact owner allocation/free, typed actual MM pressure, no-partner
  cooldown, independently runnable progress, backstore restore and
  complete MM/backing cleanup. At this checkpoint the run has no
  accepted final result; never treat partial samples as PASS.
- [ ] Keep production kernel and KCORE/MRES footprint unchanged; do
  not merge the speculative 16-word v1208/v1211 swap fallback.

## v1320: production DEVICE backing gateway in native swap-in test

- [x] Execute the production `fs_mres.s` DEVICE `backstore_read`
  bridge during the native source-authentic `proc_swap_in` test;
  DSK270 hardware dispatch remains a controlled test endpoint.
- [x] Verify eight sector translations (021..030), sequential
  physical VM buffer progression in 0200-word increments, correct
  restored image, backing bitmap release and RESIDENT publication.
- [x] First- and third-sector hardware errors exercise production
  MM unpin/free rollback, DISK/backing retention and one TERM.
- [x] Executable PDP-6 mutation disabling gateway buffer advancement
  is rejected. Testkit `d65899c`; docs `a4fa226`.
- [x] Versioned validated evidence:
  `$TMPDIR/daimos-swap-device-gateway-v1320-evidence-r1.tar.gz`,
  SHA256 `c63d3f14a09d0873bb4958bbf40de7dd61bff84c3a4cf7901d1c54e06348bbab`.
- [ ] Full booted 256K no-partner liveness with actual hardware
  read, independent process progress and exact MM/backing cleanup.
  No production kernel RAM growth or v1208/v1211 fallback.

## v1318: native production swap-in transaction and rollback accepted

- [x] Real physical MM occupancy produces native typed allocation
  pressure, preserved swap ownership, no eligible test exchange
  candidate, actual production PI6 tick countdown and scheduler gate.
- [x] Production `proc_swap_in` (not a stub) successfully allocates,
  pins, reads physical words through a test disk provider, unpins,
  publishes VM and packed RESIDENT mode, clears backing allocation
  with actual `backstore_free`, and grants one-shot dispatch.
- [x] A failing test disk provider exercises production rollback:
  exact MM unpin/free, retained disk bitmap/record, DISK mode,
  no leaked physical extent and one fatal TERM. Three executable
  mutations (missing MM free, bitmap free, storage publication)
  fail their assertions. Testkit `7ffd840e`.
- [x] Retained v1318 positive/rollback/three negative native PDP-6
  maps and reports, extracted production code and SHA256 evidence:
  `$TMPDIR/daimos-swap-in-real-transaction-v1318-evidence-r1.tar.gz`,
  SHA256 `4459b6c916c52d7cc71c5062ff727bb76989edb4a2b6e93cb056f962dc47d8eb`.
- [ ] Booted 256K guest: genuine no-partner swap-in pressure, independent
  runnable execution during clock backoff, true DSK/D6FS restore and
  rollback, final MM/backing accounting. Do not confuse native disk
  stub success with full-system acceptance or add the rejected fallback.

## v1315: native clock aging and real scheduler gate PASS

- [x] Production PI6 clock timekeeping code executes 63 ticks with
  cooldown still 1, then the 64th releases the pressure backoff.
- [x] Actual `proc_select_rt_normal_swap_v879` selects independent
  runnable work during cooldown and raises a slot-0 swap request
  only after expiry; no synthetic cursor overwrite is needed.
- [x] Real MM owner free and retry succeed with READY_ONCE.
  Six executable native mutations rejected, including clock aging
  and scheduler-gate bypass. Testkit `bfb629ae`.
- [x] Evidence with verified SHA256:
  `$TMPDIR/daimos-swap-native-clock-gate-v1315-evidence-r1.tar.gz`,
  `17f4d19c56b7776c608cfc88a0e94f0c3674cb62ce2d09094e503193ebb87134`.
- [ ] Booted 256K full no-partner scenario: real PI6 device and
  scheduler execution, genuine backstore swap-in/rollback, independent
  user process freeing occupied MM, final leak-free cleanup.
  No speculative v1208/v1211 fallback or kernel code growth.

## v1314: real-MM no-partner native recovery, full guest still pending

- [x] Production MM allocator on PDP-6 reports actual typed -1 when
  a real 010000-word arena is filled by a kernel-dynamic extent;
  pending slot 2 requests 02000 words. Two FS and one process
  reclaim calls fail, as intended by the controlled physical owner.
- [x] Unmodified slot-0 service scans all three rejected candidates,
  preserves disk ownership, avoids TERM, and sets cooldown 0100.
- [x] Exact production `mm_free` releases the independent owner;
  subsequent production MM allocation succeeds, checks extent owner
  2 and grants READY_ONCE. Test-only disk and exchange admission
  stubs are explicitly excluded from full-system acceptance.
- [x] Executable PDP-6 mutations (pressure classification, FS
  reclaim, swap reclaim, cooldown) all rejected; neighboring v756,
  v1300/v1301, v1111 and v1260 remain PASS. Testkit `d40a1158`.
- [x] v1314 source/map/positive/four-negative evidence SHA256
  `c30cccf491176a4488ffcf8933dff0b0485ce85dc198c3d10db60e06c884ed92`.
- [ ] Full booted 256K no-partner liveness including real swap disk
  read, retry after PI6 clock delay, resident owner scheduling,
  backing admission, transaction rollback and leak-free cleanup.
  Historical v1208/v1211 fallback remains excluded.

## v1312: completed live F07 guest, strict offline postflight replay

- [x] v1308 real 256K KCC PDP-6 24K-BSS trace: all 15 live samples,
  18 RUNs, 18 TERM/WAIT reaps, OOM provider/witness and HALT.
  Final: 51 successful swaps out and in, 0 MM-pressure returns,
  0 exchanges, 0 cooldowns. The **original runner failed** on
  SIMH post-HALT `sim> ` formatting, not an asserted kernel fault.
- [x] v1309 testkit corrects interactive PTY prompt prefixes in
  guest-only postflight records; protocol and negative selftests PASS.
- [x] v1311 testkit replays the unchanged original v1075 postflight
  verification against a copied prompt-normalized console: v1150,
  v1159, v1203, v1205, v1207, v1160 plus manifest checks PASS.
  Deliberately corrupted provider-owner counter fails as expected.
  This is offline recorded-guest validation, NOT a new direct run.
- [x] Archive original console, 15 snapshots, full raw debugger,
  build/map, copied replay and negative evidence with SHA256:
  `$TMPDIR/daimos-swap-no-partner-evidence-v1312-r1.tar.gz`,
  SHA256 `e5224db0e4266bea68c7e953c22f49c6074443ea930a5b360665725ab8a1b669`.
- [ ] Full physical no-partner liveness with actual MM failure and
  independent freeing owner: UNEXERCISED, not accepted. Do not
  enable v1208/v1211 fallback or reopen accepted requester OOM gates.

## v1309: live no-partner pressure trace and acceptance boundary

- [x] v1303 physical 256K F07 evidence parser and five negative
  mutants: exact ordinary-swapping and unexercised-pressure outcomes
  distinguished, with exit 3 for incomplete/no-pressure evidence.
- [x] v1304 isolated 24K-BSS process-image full-guest workload:
  original 900-second diagnostic run admitted all 18 children and
  counted 49 actual swap-outs and 33 swap-ins, but was interrupted
  before TERM/WAIT; this is NOT a full-guest PASS.
- [x] v1306 PTY periodic read-only live PDP-6 PC/scheduler/swap/MM
  probe: two real SIMH samples and interrupt/examine/GO/post-HALT
  protocol selftest PASS. Guest transcript and raw debugger output
  separated; v1309 strips PTY prompts after HALT. Testkit commits
  `5e23ff0`, `3fa4792`; documented in pdp10-doc `9ff31eb`.
- [ ] v1308 24K guest with periodic sampler: at this checkpoint
  still running; full original strict F07 completion not accepted.
- [ ] Genuine 256K full-system swap-in MM pressure with **no eligible
  exchange partner** and later bounded progress/cleanup: NOT
  demonstrated by successful ordinary swap-outs and swap-ins.
  No production kernel change or speculative fallback authorized.

## v1302: swap-in pressure/current-source regression (PARTIAL)

- [x] Clean production v852 256K PDP-6 GCC/DSK/D6FS physical
  16-child post-INIT scheduler-pressure workload: PASS.
- [x] v855 incoming-image, v958 failed swap-in rollback, v666
  late swap-out rollback, and v757 pressure wiring: PASS.
- [x] New daimos-testkit v1300 current-source no-partner cooldown,
  candidate scan, fatal-vs-pressure, backing ownership and scheduler
  gate guard: PASS; six unsafe mutations rejected.
- [x] New native PDP-6 v1301 executes current slot-0 service with
  allocator PRESSURE, no viable exchange candidate, then fatal and
  successful restore: intact disk backing, 0100 cooldown, no wrongful
  TERM and one-shot successful dispatch all PASS.
- [x] Repair old v756 stale C-result assertion to inspect the native
  assembler, reject four unsafe result-classification mutations;
  native compile/result contract PASS.
- [ ] Current-source genuine full-system no-equal-partner pressure
  with eventual forward progress remains UNPROVEN. Require bounded
  256K guest with verified pressure/cooldown counters, independently
  runnable freeing owner, exact backing-space and MM state, plus
  failure rollback and leak checks. Only change kernel code if this
  reproduces a defect; do not merge v1208/v1211 fallback on inference.

## v1298: F12/F13 requester-only BRK OOM release accepted (2026-11-07)

- [x] F12: production `PROC_OOM_BRK_ENABLED_V1000=1` and exact-source
  256K KCC PDP-6 DSK/D6FS release acceptance PASS. All 14 F03
  policy guests, 18 P0-D native stages, 17 individually completed
  physical P0-D guests, both 64-slot/high-MM F07 policies and source
  cause/negative mutant controls PASS.
- [x] Independent post-activation E13 evidence: recovered original
  v1285 Android and Linux x86-64 archives with production OOM ON
  for both guest policies. The identical candidate `78259106` is
  the sole one-word switch relative to F11 `e6993a76`. Both hosts'
  corresponding linked KCORE maps are byte-identical and simulator
  binary SHA256 identities differ. Per-guest SHA256 manifests PASS.
  Strict v1190 F10 audit of all four host/media builds PASS with
  zero unexplained compiler, assembler or media diagnostics.
- [x] F13: post-enable review and protected-requester/provider,
  fault-cause, memory-accounting and rollback acceptance PASS.
  The release profile completes 64/64 mandatory OOM gates.
  Evidence: `$TMPDIR/daimos-f13-release-evidence-v1298-r1.tar.gz`,
  SHA256 `5dcb4d75ea6ea2291e5ec01f54874ce8d704713c1608fff9dcd64ab8aab83221`.
- [ ] Separate pre-final-KCORE feature tracks: D6CODEC streaming
  cutover, BSTORE consumer integration, restricted MEMFS raw XIP,
  and swapless PURE/RAM reclaim. No size optimization yet.

## v1296: F12 exact-source guest coverage, host qualification outstanding

- [x] Completed the 11 remaining P0-D physical guests individually
  on the production-OOM-ON source: v1104, v1105, v1119, v1121,
  v1122, v1120 OFF/ON, v1106, v1107, v1077 and v1101 all PASS.
  Together with the six v1293 physical guests, all 17 physical
  fault-injection cases now have actual PASS results. The earlier
  interrupted monolithic P0D_FULL run is NOT an uninterrupted PASS.
- [x] F03: complete both-policy 14-guest set on OOM-ON source.
  Prior accepted v1075 OFF/ON and v1110 ON, plus v1120 OFF/ON in
  P0-D; new v1110 OFF, v1112 OFF/ON, v1114 OFF/ON,
  v1115 OFF/ON and v1116 OFF/ON all PASS.
- [x] F10 v1190 diagnostic checker self-test and direct audit of
  both fresh v1288/v1289 64-slot full release-media build logs PASS,
  with 12 approved diagnostic classes per log and no unexpected
  compiler, assembler or media warnings/errors.
- [x] SHA256-verified archival evidence saved at
  `$TMPDIR/daimos-f12-production-oom-evidence-v1296-r1.tar.gz`;
  SHA256 `13411b8dbbf548a425567a6ed2a460e57cf3e8d2155a9ad32e4bbf84ba2a6e07`.
- [ ] F12: independent second-host exact source/media qualification
  still missing; two configured SSH hosts were unreachable on this
  attempt. Preserve original F11 E13 evidence but do not mislabel
  it a post-activation independent-host run. Freeze/commit only after
  the required host acceptance and final release audit.
- [ ] F13: post-enable signoff, production commit and push still OPEN.

## v1293: F12 full P0-D physical matrix checkpoint (2026-11-07)

- [x] Re-run all 18 native `daimos-p0d-release-matrix-v1096.sh`
  component stages plus v1083/v1087 fatal-release graph on the
  production OOM-ON working tree: PASS.
- [x] First six real 256K PDP-6 P0-D guests PASS in order: v1125
  wrong-owner MM/provider, v1124 TTY/VM/u-area, v997 pre-VM retry,
  v1103 PIPE/FILE/HUP/VM/u-area, v1117 repeated WAIT/PID/event,
  v1118 simultaneous PIPE/VM pin/MM_BUSY and swap detach.
- [ ] Full P0-D remains INCOMPLETE, not failed or accepted: the
  sequential run was stopped during the seventh v1104 guest build;
  the interrupted guest did not produce an accepted result. Continue
  v1104 and all subsequent physical cases before F12 signoff.
  Partial log: `$TMPDIR/daimos-f12-p0d-full-v1293-r1.log`.
- [ ] F03 14-guest and grouped dual-host/F10 release checks remain
  open; F13 release signoff remains blocked until F12 is complete.

## v1292: F12 production OOM activation in progress (2026-11-07)

- [x] Set production `PROC_OOM_BRK_ENABLED_V1000=1` after accepted F11.
- [x] v1283 both-policy source mutation, v1000 branch classification,
  v998/v999 causes, v1113 native -2/-3, and v1123 writable RUN/EXEC
  scope checks PASS on the production-enabled tree.
- [x] Fresh booted v1075 and provider-held v1150 256K KCC/D6FS
  negative/positive policies PASS with provider owner, scheduler progress,
  requester-only selection and resource accounting intact.
- [x] Fresh v1274 F07 64-slot maximum-pressure enabled/disabled guests
  PASS with 63 live users, 127/136 MM descriptors, eight swapped,
  31642 APR rounding words, and bounded private/idle stack peaks.
  Evidence in `$TMPDIR/daimos-f12-f07-64slot-v1288-r1` and
  `$TMPDIR/daimos-f12-f07-64slot-disabled-v1289-r1`.
- [x] v1096 native P0-D ownership/release matrix PASS (without the
  17 physical guests); booted v1110 invalid-BRK enabled-policy PASS.
- [ ] F12 remains OPEN: complete exact-source F03 14-case matrix,
  full P0D_FULL=1 physical guests, grouped F10/E13 and release
  diagnostics on the production-enabled image; independent host
  verification and evidence hashes remain to be signed off.
- [ ] F13 post-enable signoff or safe revert on any regression.

## v1284: F11 OOM preactivation signoff complete (2026-11-07)

- [x] F11 exact-source 256K KCC DSK/D6FS integration review: 14/14
  Android F03 and 14/14 Linux F03 full guests PASS; uninterrupted
  Android P0D_FULL 18 native stages + 17 booted physical guests PASS.
- [x] The interrupted Linux full P0-D v1121 SIGKILL is preserved, not
  accepted as a monolithic PASS. All missing v1121..v1101 physical
  guests were independently retested with clean successful checks.
- [x] Recheck original F07/E13, F04/F10, v1087 allocation-free release,
  v1092/v1282 provider quarantine, v1123 writable-VM/XIP scope;
  none requires an unrelated process as OOM victim.
- [ ] F12: enable requester-only BRK OOM in production only now, with
  the exact conditional no-memory cause and full negative controls;
  redo the actual booted release candidate in both guest policies.
- [ ] F13: only after the post-enable matrix PASS, record release
  evidence, keep production ON; on failure revert safely.

## v1279: correct Linux toolchain and complete F11 exact-revision regressions

- [x] Isolate Linux v1112 failed-RUN guest crash at the first
  v1257-linked image. The scheduler source was sound: Linux's
  installed `p10fold` predated the v1141 local-relocation identity
  fix and reproduced the exact unsafe cross-object fold; the user
  INIT executable was byte-identical to Android.
- [x] Rebuild portable six-column fold-plan parsers from corrected
  `daimos-tools` `61dcb15`, validate native v1141/v1257 plus seven
  malformed-plan mutants, and install via versioned `local.tar.gz`
  to the Linux `$PDP10_PREFIX`, preserving old binaries. Current
  Linux v1112 now PASSes and linked KCORE `023267` matches Android.
- [x] Correct the F04/v1113 isolated native source fixture to link
  actual `mm_arena_fits_v1260` instead of omitting the production
  allocator's new helper; Android native true -2/-3 and all three
  negative mutants PASS (testkit `69830c9`).
- [x] Run an additional full 64-slot OOM-disabled F07 negative guest:
  all 57 children admitted/reaped; original requester returns,
  63 live user slots, 127/136 descriptors, real swap, safe stacks,
  final `NWP` HALT and SHA256 evidence manifest PASS.
- [ ] F11: re-run and accept complete exact-current-source F03/F04,
  P0-D and E13 candidate matrices with the v1277 corrected tools.
  Concurrent Linux jobs lost their SSH connection when the device
  could no longer reach the home LAN; interrupted suites are NOT
  accepted. A frozen Android F03 matrix is running independently.
- [ ] F12/F13: do not change production OOM OFF until that final
  F11 integration/ownership/rollback review is signed off.

## v1276: E13 exact-revision dual-host release media acceptance PASS

- [x] Run current DAIMOS `226e7a5f` / testkit `ee9d92b` on
  independent Android aarch64 and Linux x86_64 Open SIMH PDP-6
  V4.1-0 simulator builds, both normal OOM-OFF and disposable OOM-ON,
  on the frozen 256K KCC DSK Stage 1 / D6FS root S07/S08 profile.
- [x] Verify independent simulator binary hashes, exact identical
  linked KCORE low endpoints (`023503` disabled and `023521` test-on),
  original foreign FS lease ownership, slot-3-only fatal selection,
  protected bystanders, WAIT/FILE/MM cleanup and complete logs.
- [x] Verify host evidence archives and SHA256:
  Android `c87799ba1ecda40f07bbbf718d5ca20e0d5be896549df3be80d075ed0c8990ef`,
  Linux `a59d9b90338a0c23fd084fb7153160bd4068fd4233eabfcf7cd0205fb8b6a720`.
  The F10 audit finds zero unexplained compiler/assembler diagnostics;
  the one already-approved legacy D6FSR2 layout note differs between
  otherwise identical diagnostic sets on the two hosts.
- [x] E13 CLOSED for the specifically frozen S07/S08 first OOM
  release profile. Other emulator, boot/media and OpenBSD-host
  combinations remain separate, explicitly out-of-scope work.
- [ ] F11 final candidate readiness: the Linux-specific v1112 failure
  was fixed by updating the stale v1141 host linker (v1277); the
  remaining exact-source comprehensive F03/F04/P0-D regression suite
  still needs completion. Do not enable production requester OOM.

## v1274: full 64-slot F07 maximum-load acceptance PASS

- [x] In the frozen 256K KCC DSK/D6FS guest, admit 57 additional
  real RUN/BRK children: all 63 user slots live at one physical OOM
  snapshot, 55 resident process APR extents and eight swapped images.
- [x] At genuine requester-only OOM under the held foreign FS lease,
  reconcile 127/136 MM extents, 239576/242085 managed words allocated,
  2509 free words, 31642 words of exact APR rounding, one 128-word
  provider buffer, private-stack 180/206 and idle-stack 63/256 words.
- [x] Confirm all 57 individual TERM/WAIT/reaps, witness survival,
  exactly one requester selection, zero failed provider/ownership
  diagnostics and final MM/FILE/backing cleanup. The expected KWP HALT
  and original 900-second timeout PASS; 13 negative host mutants fail.
- [x] Archive full SHA-256 verified simulator, build, map, media and
  snapshot evidence: `daimos-f07-64slot-acceptance-v1274-r1.tar.gz`,
  SHA256 `0fb22920bfd9f66c155ef3bb61f8db0c0f146f80f89bbc3e6e1b3406d9599e1b`.
- [ ] E13/F11 exact-release approval and production OOM enablement
  remain open; F07 is CLOSED for the selected S07/S08 profile.

## v1269: clean 24-process F07 milestone PASS (superseded by v1274)

- [x] Rebuild clean committed DAIMOS `9a841363` without experimental
  v1208/v1211 swap code; run the unchanged 900-second 256K KCC/D6FS
  18-child F07 batch-TERM guest to expected `KWP` HALT and validate
  v1150/v1159/v1203/v1205/v1207 plus evidence checksums. PASS.
- [x] Qualify the experimental branch separately: native assembler,
  nonrecursive source/mutant tests and actual fallback counts PASS.
  Identically instrumented SWAP MRES grows 16 words; KCORE unchanged.
- [x] Reject unproven v1208/v1211 from production, retaining only a
  reversible archive. A distinct failure plus rollback/size evidence
  would be needed to reconsider it, not to accept F07.
- [x] The outstanding maximum-slot plus concurrent high descriptor/
  APR/provider/stack test was completed in v1274. E13/F11 OPEN;
  60/64 OOM gates, production requester OOM OFF.

## v1205: actual concurrent process and logical BRK/APR pressure

- [x] Populate twelve real user process slots with real executable
  VM/u-area/BRK while another FS process holds its in-flight lease.
- [x] At genuine physical BRK OOM, snapshot all twelve live process
  table rows, real MM owners and APR extents; independently reconcile
  **5499 exact logical BRK/APR padding words**, 231012 allocated MM
  words and 31/136 descriptors (both disposable policy variants).
- [x] Preserve full 256K release-media evidence with exact frozen
  DAIMOS/testkit revisions; see v1205 design acceptance report.
- [ ] F07: combine actual resident processes with high MM descriptor
  occupancy in one physical-OOM snapshot, then exercise wider
  64-slot swap/stack concurrency and assess the combined budget.
- [ ] Keep production requester OOM OFF until E13/F07/F11 signoff.

## v1204: F07 high physical/descriptor pressure under held provider

- [x] Trigger physical BRK OOM in the full release-media guest with
  a foreign FS owner live, 12 real 8192-word APR-aligned extents and
  96 one-word extents held concurrently; 127/136 descriptors and
  211092 allocated words observed.
- [x] Reproduce the high-load MM state under both OOM test policies;
  all 108 test-owned extents released by exact typed MM free and
  original WAIT/provider/TTY process cleanup PASS.
- [x] Correct F07 verifier's APR quantum bound to `02000-1` octal
  (1023 decimal), with host negative mutations and physical rerun.
- [x] Archive frozen source/testkit/boot-media and raw simulator
  evidence: `design/F07-HIGH-PRESSURE-MM-ACCEPTANCE-V1204.md`.
- [ ] F07 still OPEN: test actual high concurrent process-slot load
  and snapshot per-process logical BRK waste, not merely synthetic
  MM_TYPE_KERNEL_DYNAMIC pressure. Do not enable production OOM.

## v1203: F07 genuine OOM/provider MM snapshot, partial completion

- [x] Sample genuine live physical-MM state at BRK -1 with the
  foreign D6FS provider held; both disabled and disposable-enabled
  256K disk-root guests PASS on exact DAIMOS/testkit commits.
- [x] Reconcile all allocated and free physical MM arena words,
  19 live extents, 136 configured descriptors, APR alignment and
  the resident 128-word VFS workspace; reject 10 unsafe mutants.
- [x] Archive original simulator/build/map/media/tool/counter proof:
  `design/F07-LIVE-OOM-MM-PEAK-AUDIT-V1203.md`.
- [ ] F07 remains OPEN: measure maximum *simultaneous* process/MM
  extent and descriptor use, APR slack and actual logical BRK
  rounding, while a foreign provider and shared buffer remain live.
- [ ] Do not infer a production size fit or peak budget from a
  disposable test kernel with 130K words still free at physical OOM.

## v1202: close A04/F02 complete exact-revision matrix

- [x] Execute the full `P0D_FULL=1` matrix uninterrupted on frozen
  DAIMOS `5db15ecf`, testkit `122c61f`: 18 component stages,
  allocation-free v1083/v1087 graph, 17 booted invocations PASS.
- [x] A04 complete: full final-candidate source-linked release graph
  and native/error release matrix revalidated with no new edges.
- [x] F02 complete: all physical P0-D release guests PASS against
  the identical frozen DAIMOS candidate.
- [ ] E13: finish remaining independent simulator/media coverage.
- [ ] F07: measure combined peak physical MM/descriptor/APR/provider
  buffer allocation budget, not just individual pressure cases.
- [ ] F11: final exact release-candidate integration/signoff;
  only then consider F12/F13 production OOM activation.

## v1201: A04 full-guest individual row coverage complete

- [x] v1120 OOM default OFF and isolated test ON: real physical
  shortage, requester/witness distinction, 24 PID reuse cycles each.
- [x] v1106 real process-VM MM_BUSY and swap backing cleanup PASS.
- [x] v1107 physical DISK bitmap release refusal and repair PASS.
- [x] v1077 D6FS SYNC and MM_BUSY under real requester OOM PASS.
- [x] v1101 permanent wrong-owner MM -4 quarantine PASS.
- [x] Every individual v1096 P0D_FULL booted test has passed across
  the v1197-v1201 staggered evidence batches.
- [ ] Run the complete frozen `P0D_FULL=1` script uninterrupted on
  the final exact candidate and review release counters, build
  diagnostics, resource peaks and revision provenance.
- [ ] Keep A04/F02 OPEN, 57/64 acceptance, production OOM OFF
  until the complete exact-candidate signoff is satisfied.

## v1200: A04 PIPE/DPY/disk full-guest acceptance progress

- [x] v1119 physical PIPE plus Type-340 DPY/HUP retry PASS.
- [x] v1121 DPY MM_BUSY across real disk swap-out/in PASS.
- [x] v1122 PIPE MM_BUSY followed by disk bitmap refusal PASS.
- [ ] Continue v1120 disabled/enabled, v1106, v1107, v1077,
  v1101; then rerun the complete exact-candidate `P0D_FULL=1`
  matrix and inspect final resource/diagnostic evidence.
- [ ] A04 and F02 remain OPEN; requester OOM production OFF.

## v1199: A04 full-system regression progress

- [x] v1117: mixed teardown, 48 stale WAIT/PID/event checks and
  stable process-MM/slot accounting PASS on PDP-6 guest.
- [x] v1118: combined real PIPE and process VM MM_BUSY PASS, with
  exact VM/swap and WAIT ownership accounting.
- [x] v1104: member HUP plus pinned PIPE final-close retry PASS.
- [x] v1105: member HUP plus pinned DPY and PI6 replay PASS.
- [ ] Continue v1119 then v1121/v1122/v1120 off+on/v1106/v1107/
  v1077/v1101; perform one final frozen `P0D_FULL=1` acceptance.
- [ ] Keep A04/F02 OPEN pending final signoff. OOM default OFF.

## v1198: A04 full-guest integration progress (still OPEN)

- [x] Boot isolated current-source v1124 physical TTY, VM and
  u-area MM_BUSY; exact three-stage owner release PASS.
- [x] Boot v997 physical pre-VM/post-VM retry with held foreign
  provider; final EXIT/WAIT and bystander PASS.
- [x] Boot v1103 combined PIPE/HUP/VM/u-area teardown with held
  provider and independent bystander; zero guest errors PASS.
- [ ] Continue the remainder of `P0D_FULL=1` starting at v1117,
  re-run final frozen full matrix and review all candidate counters.
- [ ] Keep A04/F02 OPEN and production requester OOM OFF until the
  complete exact-revision acceptance evidence is signed off.

## v1197: A04 full guest matrix partial progress (still OPEN)

- [x] Rerun all 18 P0-D component leaves and v1083/v1087 audits on
  current-source DAIMOS; first booted v1125 foreign-provider/MM -4
  guest PASSed. See `design/A04-P0D-FULL-PARTIAL-V1197.md`.
- [ ] Complete the interrupted `P0D_FULL=1` matrix, including v1124
  and every remaining full guest, on one exact frozen candidate.
- [ ] Close A04/F02 only after final build/boot/ownership counter
  evidence is reviewed. Production requester OOM remains OFF.

## v1194: fix AUXSTORE conversion of v1192 DBOOT descriptors

- [x] Accept only exact expected DBOOT zero/explicit-1 reserved-layout
  flags in the destructive `mkauxstore --convert-dboot` entry.
- [x] Preserve existing nonempty/unknown high-bit rejection, dry-run
  immutability and filesystem payload, verified by testkit v638/v636.
- [x] Re-run the complete F07 idle-stack guest on newly generated
  auxiliary-drum images: v1194 PASS, idle highwater `025` octal
  below `060` limit. This does not close the F07 peak-MM gate.

## v1192: DBOOT D6FSR2 explicit-zero reserved-layout ambiguity fixed

- [x] Distinguish modern explicitly zero bootstream/LOGSTORE/BADMAP
  ranges using DBOOT descriptor word `020` flag 1, without inventing
  reservations or suppressing the legacy warning.
- [x] Preserve old flag-0 D6FSR2 media and verify rejection of
  unknown 36-bit flag values and mixed-member descriptors.
- [x] Recheck KINIT BLOCKSET decoder and update testkit v1192,
  DBOOT/D6FS documentation and F10 historical-log diagnostic audit.

## v1163: second E13 simulator target and host-tool compiler fix

- [x] Boot independent 256K full KCC DSK/D6FS physical-OOM enabled
  and disabled guests on Linux x86_64 Open SIMH `a6b511e5`. Real
  foreign provider ownership, requester-only selection, protected
  bystanders, 160 spinner advances and final accounting PASS.
- [x] Cross-check Linux archive against Android aarch64 Open SIMH
  `edd19b17` v1160 evidence with testkit v1163; compare raw counters,
  simulator SHA256, boot profile and identical disposable KCORE low
  endpoints. Full Linux evidence SHA256 is recorded in PROJECT-STATE.
- [x] Fix Linux strict-C99 host formatter missing POSIX declarations
  at canonical `tools/host/mkauxstore.c` (DAIMOS `90dfc8d0`);
  independently rebuild and PASS v636/v638 safety tests on both hosts.
  Install corrected Linux utility with previous binary backed up.
- [ ] E13 remains OPEN: first Android archive predates the host-tool
  fix, OpenBSD host was unreachable, and full required target/media
  coverage is not signed off. Do not infer production 16K memory
  budget from instrumented 256K test-kernel low endpoints.
- [ ] F10: review Linux KCC identifier-truncation/unused-symbol notes
  and legacy D6FS reserved-layout warning; zero *unexplained* tool
  diagnostics must precede final release approval.
- [x] No production OOM enable, KCORE/MRES, RAM or ABI change. 51/65
  checks complete, 14 remain (12 pre-enable plus F12/F13).

## v1161: provisional final-release graph and native P0-D retest

- [x] Re-run testkit `test-daimos-p0d-release-matrix-v1096` on
  current frozen 256K KCC candidate source revision `fe0f3c0e`.
  All 18 source-linked/native release stages, v1083 direct scope,
  v1087 allocation-free transitive teardown and unsafe mutants PASS.
- [ ] A04/F02 remain open: this matrix did not use `P0D_FULL=1`,
  and final production-candidate exact-revision booted regression,
  review and size acceptance are still required.
- [x] No production OOM enable, kernel code/data or ABI change;
  51/65 mandatory checks remain complete.

## v1160: first requester-OOM release candidate profile and E13 evidence

- [x] S07: Freeze first acceptance profile at 256K, KCC, DSK Stage 1,
  real D6FS disk root, 60 Hz PCLK, CTY headless, available DSK/drum
  backing and SWAP=AUTO policy; document actual tested versus merely
  configured swap and MRES. Strict 32K GCC/DSH remains separately gated.
- [x] S08: Explicitly exclude optional raw-XIP process admission,
  alternative boot/root media, strict 32K and interactive GUI/multiTTY
  *from first OOM activation scope*, without removing those facilities
  from DAIMOS. v1123 guard again PASSes six unsafe source mutations.
- [x] Two fresh 256K KCC PDP-6 E13 v1160 DSK/D6FS full guests PASS
  enabled/disabled genuine physical BRK OOM, held foreign FS owner,
  exact requester-only victim and subsequent original-owner release,
  160 independent spinner advances each and zero integrity errors.
  Full logs/maps/boot.ini/tool and media checksums are retained under
  `$TMPDIR/daimos-e13-release-media-v1160.qel3Zm.tar.gz`.
- [ ] E13 stays open: other required independent simulator targets
  and media configurations are not certified by one local `pdp6`.
  A04 and F02-F11 still require exact release candidate signoff.
- [x] S07/S08 closed for this bounded profile: 51/65 complete,
  14 open (12 pre-enable plus F12/F13). Production requester OOM OFF.

## v1159: E12 protected critical and unrelated nonrequesters (2026-11-05)

- [x] Boot full 256K PDP-6 physical BRK OOM under a real foreign
  filesystem gateway lease, in default-off and disposable-enabled
  policies, retaining original v1150 owner/retry/WAIT checks.
- [x] Snapshot production scheduler table at the actual OOM decision:
  INIT, blocked witness, foreign gateway waiter, independent spinner
  and in-flight original gateway owner remain active, with one exact
  real requester slot and unchanged original lease.
- [x] Verify exactly one fatal-selection attempt on the requesting
  slot when enabled; zero selections and ordinary BRK -1 return when
  disabled. Both guests PASS original exact cleanup, 160 spinner
  advances and zero integrity errors.
- [x] E12 accepted for nonrequester protection. The test does not
  establish an exemption for INIT if INIT itself is the requester.
  49/65 checks complete, 16 open (14 pre-enable plus F12/F13).
  No production kernel, ABI, resident RAM or OOM-default change.

## v1158: E11 independent child RUN failure and requester OOM (2026-11-05)

- [x] Boot two fresh 256K PDP-6 full-system v1158 guests with
  default-disabled and disposable-enabled requester-only OOM.
- [x] Have a distinct live RUN parent produce real MM -3 child
  admission refusal under descriptor exhaustion, verify exact
  rollback, and acknowledge readiness before the BRK requester.
- [x] Keep that RUN parent alive across real physical OOM, verify
  exclusive requester termination only when enabled, then exact
  WAIT and process MM/slot recovery in both policies.
- [x] E11 closed: 48/65 complete, 17 open (15 pre-enable plus
  F12/F13). No production kernel, ABI or persistent RAM changes.

## v1157: E10 APR/EXEC/RUN with held foreign provider (2026-11-05)

- [x] Fresh full-system 256K PDP-6 v1157 guest PASS against DAIMOS
  `a7b721b4` and testkit `436552f`, with asynchronous APR fault,
  postcommit EXEC FILE failure, real foreign gateway lock refusal,
  deferred unpublished RUN and physical u-area MM_BUSY rollback.
- [x] Original holder returns via provider unlock; exact final
  FILE/VM/WAIT ownership, six VFS calls/five refusals, seven FILE
  passes/six sleeps, one physical pin/busy/unpin, zero test errors.
- [x] E10 closed: 47/65 complete, 18 open (16 pre-enable plus
  F12/F13). No production change; requester OOM remains disabled.

## v1156: E09 physical versus backing exhaustion accepted (2026-11-05)

- [x] Run frozen v1156 matrix: nine real-MM/owner/backing tests,
  plus unchanged-source classification; all PASS, logs retained at
  `$TMPDIR/daimos-oom-error-matrix-v1156.gzw27g`.
- [x] Verify authentic physical MM -1, fragmentation -2, MM
  descriptor shortage -3, invalid/wrong-owner -4, physical busy -5.
- [x] Saturate the real backing-store allocation bitmap (not a
  fabricated -1), reject an additional allocation atomically,
  recover and preserve ordinary BRK/live requester with default OOM
  disabled and in a disposable kernel with requester OOM enabled.
- [x] E09 closed: 46/65 complete; 19 open (17 pre-enable plus
  F12/F13). E10-E13, S07/S08, A04 and F02-F11 remain prerequisites.
  Production requester OOM remains OFF.

## v1155: E08 concurrent sessions/group/PID lifetime accepted (2026-11-05)

- [x] Two actual separate-session leaders remain alive while two
  concurrent, distinct two-member process groups stop and continue.
- [x] Real group STOP/CONT and TERM/INT notifications yield exact
  per-member WAIT statuses and CHLD, no duplicate event or wrong
  session owner exit; reaped PIDs reject stale WAIT/TERM.
- [x] While foreign sessions remain live, reuse one numeric PID
  across 64 additional child RUN/event/WAIT lifetimes; reject stale
  events after each reap, then reap both sessions independently.
- [x] Check final process-slot, process-word and FILE accounting.
  Two complete full-system v1155 PASS runs and fresh original v979
  control PASS; testkit commit `0ac6c83`.
- [x] E08 closed: 45/65 complete, 20 open (18 pre-enable plus
  F12/F13). E09-E13, S07/S08, A04 and F02-F11 remain mandatory.
  No production code or OOM default change.

## v1153-v1154: independent scheduler progress E07 (2026-11-05)

- [x] Boot v1153 with a real held FS provider and physically distinct
  PIPE, Type-340 DPY, process VM and u-area pin/BUSY/unpin/free owners;
  measure the qualified PI6 clock and unrelated-user sleep count
  between first PIPE refusal and exact final ZOMB publication.
- [x] v1153 PASS: PI6 1297->1352 (55 ticks), three independent
  user sleeps inside that interval, all original physical owner,
  release order and held-provider counters PASS with zero errors.
- [x] Boot v1154 with first-member physical PIPE MM_BUSY followed by
  second-member genuine DISK bitmap verifier fault/retry, plus a
  separate-session user running between the two actual verifications.
- [x] v1154 PASS: PI6 1015->1032 (17 ticks), eight independent user
  sleeps, two real verifier calls, one bitmap corruption/restore,
  exact final backing/WAIT/TTY status and zero errors. v1148 covers
  pinned TTY/HUP/VM/u-area (32 PI6 ticks, 48 bystander sleeps).
- [x] E07 closed: 44/65 mandatory checks complete; 21 open (19
  pre-enable plus F12/F13). E08-E13, S07/S08, A04 and F02-F11
  remain. Production requester OOM remains OFF.

## v1151r1: sustained physical-OOM endurance E05/E06 (2026-11-05)

- [x] Define E05 endurance threshold: an initial real physical OOM,
  32 more true physical OOM/reclaim waves, 32 successful RUN/BRK
  growth/shrink recovery children, 256 numerical PID-reuse lifetimes,
  512 rejected stale WAIT/event operations and at least 1024 real
  qualified PI6 clock ticks per fresh 256K PDP-6 guest.
- [x] Run the whole threshold twice with production-disabled and
  disposable test-enabled requester OOM; both full-system guests PASS.
  Enabled mode kills only its requesting process; disabled mode returns
  -1 for all 33 physical OOM requests and correctly prints exactly
  32 additional ordinary `D` return markers.
- [x] After each OOM and recovery RUN, verify exact MM process slots,
  process words, FILE slots, non-cache extents and swap-backed words;
  clean BCACHE's unique 0201-word owner-6 slab is explicitly observed
  and must be reclaimed by subsequent real physical pressure.
- [x] E05/E06 accepted: **43/65 checks complete; 22 open** (20
  pre-enable plus final F12/F13). E07-E13, S07/S08, A04, F02-F11
  and the actual RAM budget still remain. Production OOM stays OFF.

## v1149: B07 bounded delay is not bounded owner lifetime (2026-11-05)

- [x] Audit current fatal FILE/HUP owner, foreign FS lease, remote
  fatal deferral and slot-0 finalizer paths. No path may translate
  the 01000-tick retry cap into a forced owner/FILE/MM reclamation.
- [x] Testkit v1149 source contract rejects eight unsafe mutations
  including an owner stop, provider lock steal/unlock, early slot-0
  finalization, and illegal direct MM free.
- [x] Native v885 PASSes 14 FILE close refusals (multiple retries at
  the capped delay) and one later success; native v1102 PASSes four
  reaper-backoff negative mutations. The unrecovered provider remains
  fail-closed and may require operator intervention indefinitely.
- [x] B07 closed: 40/65 mandatory checks complete, 25 open
  (23 pre-enable plus F12/F13). B06 physical-OOM/provider concurrence,
  sustained E05-E13 and F02-F11 remain open; production OOM stays OFF.

## v1147-v1148: C04 integrated owner/PI6 acceptance (2026-11-05)

- [x] Rebuild v1147 with the real physical TTY/VM/u-area MM_BUSY
  chain and independently advancing PI6 clock and user bystander.
- [x] Boot v1148 twice with a live original-session member: a real
  HUP is deferred to its own private return, handled exactly once,
  and reaped by WAIT after the leader's first pinned-TTY EXIT fails.
  The second owner EXIT then completes VM and u-area retries on the
  real slot-0 clock, with one final commit and zero integrity errors.
- [x] Both v1148 guests show 32 additional real PI6 ticks after the
  busy phase, 48 independent bystander sleeps and one exact HUP.
  v1098 retry-clock contract and three negative mutations also PASS.
- [x] C04 complete: 39/65 checks, 26 open (24 pre-enable plus F12/F13).
- [ ] Continue B06/B07, E05-E13 and F02-F11; requester BRK OOM OFF.

## v1146r3: D16 exact-once release matrix accepted (2026-11-05)

- [x] Complete all nine clean/frozen `test-daimos-oom-exact-once-matrix-v1146`
  stages with source revision and installed host-tool checksum guards.
- [x] Verify FILE/PIPE/DPY, canonical TTY, VM, u-area, swap bitmap,
  unpublished RUN discard, provider lock and exact-once owner release
  across the full physical fault/rollback cases.
- [x] Verify 24 successive PID/slot reuse cycles with alternating WAIT
  status and stale-event rejection under both production-disabled and
  isolated test-only enabled requester OOM. No unintended victim.
- [x] D16 complete; mandatory OOM checklist 38/65 checked, 27 open
  (25 pre-enable blockers plus F12/F13 post-signoff actions).
- [ ] Continue C04 PI6 owner/bystander full guest, B06/B07, sustained
  E05-E13, F02-F11 and actual 256K/32K RAM budget signoff.
  Production requester OOM remains disabled.

## v1145-v1146: D16 frozen-matrix preparation (2026-11-05)

- [x] Rerun all v1087 release-graph and native checks after restoring
  v912 precise DPY MM_BUSY and v995 current event/wait fixture ABI.
- [x] Fresh v1122 256K physical PIPE plus DISK bitmap-refusal guest:
  one real pin/busy/unpin/free, one backing restore, exact final WAIT,
  no MM/TTY accounting errors.
- [x] Stage testkit v1146r1 sequential D16 exact-once matrix with
  fixed Git revisions, host tool checksum guard and retained logs.
  Rejecting a dirty testkit checkout works as a negative control.
- [x] Run every full matrix guest on one frozen candidate, then
  inspect all exact-once/owner counters before marking D16 complete.
- [ ] C04, E05-E13, F02-F13 and resident/peak RAM proof remain open.
  Keep production requester-OOM OFF.

## v1144: D14 integrated physical teardown acceptance (2026-11-04)

- [x] Boot one 256K PDP-6 guest with real PIPE, Type-340 DPY,
  process-VM and u-area MM pin/BUSY/unpin/free on four distinct
  allocated owners, plus the original held foreign FS provider.
- [x] Assert FILE/PIPE/DPY release before VM destroy, VM/u-area
  release before ZOMB, an initial nonreportable WAIT(NO-HANG),
  final exact WAIT, CHLD, provider lease, and independent spinner.
- [x] Verify one pin, BUSY, unpin and physical free for each owner;
  48 stale WAIT/PID/event checks, one swap detach and zero hook errors.
- [x] Independently rebuild original v1118 mixed guest after extending
  the testkit; no regression. v1087 transitive-release suite PASSes.
- [ ] Continue D16 exact-once comprehensive accounting, C04 and
  sustained E/F OOM signoff. Keep normal requester OOM disabled.

## v1143: D15 XIP scope verified and excluded (2026-11-04)

- [x] Verify production RUN and EXEC still use writable VM startup:
  v1123 scope audit PASS including all six unsafe mutations.
- [x] Rerun current source-linked MEMFS MRES XIP fault/retry tests
  v1108 and v1109; both PASS with unsafe mutations rejected.
- [x] Keep scheduled XIP processes explicitly EXCLUDED from the
  requester-only BRK OOM activation scope. D15 is complete for this
  scope; production trusted XIP launch X03-X08 remains a separate
  release prerequisite and cannot be counted as implemented.
- [ ] Continue D14/D16, C04, E/F and full RAM measurements before
  changing the production-disabled OOM policy.

## v1142: real-MM_BUSY nested RUN rollback (2026-11-04)

- [x] Boot complete APR/EXEC/late unpublished-RUN guest with a pin on
  the child's real u-area, observe MM_BUSY (-5), unpin and allow the
  original deferred rollback to retry the MM release successfully.
- [x] Check two real free attempts, one pin/busy/unpin, parent return,
  restored accounting, and final EXEC child ZOMB/WAIT in one guest.
- [ ] Continue with D14-D16, C04, E/F and measured permanent/peak
  runtime RAM; never infer OOM activation from the D13 subcase.

## v1141: cross-object fold fix and D13 uninstrumented recovery (2026-11-04)

- [x] Prove the old P10FOLD/DLINK cross-object LOCAL TEXT alias with
  a two-word distinct-target fixture; corrected analyzer and independent
  linker verifier both pass testkit v1141 (old tools fail controls).
- [x] Install corrected binaries under PDP10_PREFIX via versioned
  `local.tar.gz`; no DAIMOS production source modification.
- [x] Rebuild original uninstrumented nested 256K APR/EXEC/failed-RUN
  guest. Its late rollback, parent return, WAIT and EXEC ZOMB now PASS.
- [x] Rebuild historical v890 EXEC FILE/WAIT and native v1129 VM_NONE:
  both PASS with corrected tools.
- [x] Rebuild independent v197 RUN rollback (16 failed post-VM fd
  maps under three live children) and repeat fresh uninstrumented
  combined D13 guest: all PASS with corrected tools.
- [ ] Run remaining independent RUN, physical MM_BUSY rollback and
  final mixed OOM acceptance; do not enable requester OOM yet.

## v1140: debugger-sensitive rollback (2026-11-04)

- [x] Fresh stage-level trace with MM-return and rollback-return
  breakpoints reaches late RUN epilogue and EXEC ZOMB=1.
- [ ] Reproduce original invalid UUO without breakpoints and locate
  timer/PI6/stack/ownership dependence causing divergent outcomes.
- [ ] Verify proposed actual production repair against uninstrumented
  failures, physical MM_BUSY, and earlier RUN/EXEC controls.

## v1139: MM free reached; return remains unproved (2026-11-04)

- [x] Rebuild fresh unmodified-production guest; breakpoint confirms
  entry to `mm_free` from failed RUN u-area cleanup.
- [ ] Break at MM-free return and descriptor-clear continuation;
  distinguish pretrap execution from subsequent trap cleanup.
- [ ] Validate saved parent stack/syscall return and independent
  physical MM_BUSY rollback before D13 signoff.

## v1138: stage breakpoints narrow D13 (2026-11-04)

- [x] Run fresh simulator-only breakpoints through real failed RUN
  rollback; record VM-done and child u-area-release entry.
- [ ] Identify exact last pre-UUO instruction and check `mm_free`
  return, descriptor validity and parent kernel-stack integrity.
- [ ] Reproduce and repair D13 with uninstrumented and independent
  RUN/EXEC/physical-MM_BUSY controls before any production OOM change.

## v1139: post-slot-trim return investigation (2026-11-04)

- [x] Extend simulator-only D13 trace: late child reaches `mm_free`,
  `proc_runq_remove`, and `proc_trim_high` before invalid UUO.
- [ ] Capture late child's stack and return word across slot trimming;
  determine whether an interrupt, return-address corruption or
  post-fold branch error prevents `proc_discard_return`.
- [ ] Validate a source-level fix with fresh uninstrumented D13 and
  independent RUN/EXEC/physical-MM_BUSY controls. OOM remains OFF.

## v1138: isolate late RUN u-area release (2026-11-04)

- [x] Boot unchanged-production nested D13 guest with SIMH-only stage
  breakpoints; establish TTY/VM completion and entry into u-area release.
- [ ] Trace the late child through `mm_free`, u-area completion and
  return-state handling, distinguishing other child cleanups from the
  late rejection; fix proven source defect, not code-layout symptoms.
- [ ] Reboot uninstrumented D13 and independent RUN/EXEC/physical
  MM_BUSY controls before considering OOM activation.

## v1134-v1137: D13 failure isolated before late RUN epilogue (2026-11-04)

- [x] v1134 unreachable-text placement-only fresh guest reproduces
  `<EA`/unexpected UUO/zero EXEC ZOMB. This placement change does not
  produce the v1131 traced success; never ship padding as a fix.
- [x] v1135 fresh guest samples the existing PDP-6 invalid-UUO and
  monitor return words; v1136 live SIMH breakpoints show valid saved
  parent return 350000000104 at the failing claimed-child entry.
  A 340/350 status prefix is not itself an illegal saved JSR word.
- [x] v1137 sequential breakpoints enter the late claimed-child
  rollback in slot 1 with stable user/kernel SP, but its corresponding
  RUN epilogue breakpoint is never reached before invalid UUO/HALT.
  Prior v1133 return counts belong to earlier successful RUNs.
- [ ] Pinpoint which nested `proc_slot_discard` stage loses progress
  (TTY, VM, u-area, or final cleanup). Retain source bytes, compare
  before/after scheduler/PI6 states and preserve a fresh uninstrumented
  failing guest while testing the actual repair. D13 still OPEN;
  physical MM_BUSY unpublished RUN rollback remains outstanding.

## v1130-v1133: diagnose nested D13 RUN syscall return (2026-11-04)

- [x] Independently boot v197 and v1130 256K PDP-6 RUN rollback
  controls: 16 invalid post-VM fd maps with three live children,
  inherited versus NEW process groups, identical process slot/word
  accounting, parent recovery and successful subsequent RUN/WAIT.
- [x] Reboot the uninstrumented nested v1126 guest after the v1129
  production VM fix: the real late fd-map rejection executes, but
  `<EA` is followed by simulator UUO `340000774047` before R and
  no EXEC-child zombie is published. Do NOT close D13.
- [x] Run disposable v1131 full rollback trace: exactly one child
  discard/new/VM/u-area/return, no rollback refusal/defer/fail, and
  correct `<EARH` with one ZOMB. The probes change the observed
  outcome and are not a safe workaround or production acceptance.
- [x] Run v1132 BSS-only and v1133 two-counter diagnostics: BSS-only
  still fails at `<EA`/UUO, and the minimal counters observe one
  claimed-child rollback plus two RUN return entries (one was the
  successful APR-child RUN), still without parent R or final ZOMB.
- [ ] Isolate actual parent APR/user continuation and mapped-user
  `native_sys_run` return/`vm_user_mapping_release`; distinguish a
  corrupted saved syscall state, user stack, trap return, or text
  placement/scheduler interaction. Preserve exact pre/post snapshots
  and test on an independent fresh boot. No speculative padding.
- [ ] Finish D13 by combining APR, postcommit EXEC FILE retry,
  physical MM_BUSY refusal/rollback for an unpublished child RUN,
  final exact-once owner VM/u-area/ZOMB/WAIT and bystander liveness.
  Current OOM gate status remains 34/65 mandatory checked and 29
  pre-enable blockers. Production OOM remains OFF.

## v1129: VM_SPACE_NONE exact sentinel in fatal EXEC release (2026-11-04)

- [x] Repair a real source defect in `vm_space_destroy`: an already
  destroyed post-commit EXEC VM uses full-word `VM_SPACE_NONE` (-1),
  whose nonzero RH previously flowed into MEMFS XIP probing and
  invalid MM free, permanently retaining PREVM/FINALIZE. After the
  storage-mode safety check, detect the exact *full* sentinel and
  skip only the physical free, continuing through checked swap
  backing detach and normal `VM_SPACE_NONE` publication.
- [x] Source-linked real PDP-6 `daimos-vm-none-idempotent-v1129`
  verifies two repeated destroy calls, no MM or XIP release, two
  backing-detach calls, unchanged VM_NONE, and invalid storage-mode
  rejection; both unsafe sentinel and skipped-detach mutations fail.
- [x] Independently build/boot the **unmodified historical v890**
  post-commit EXEC fatal FILE-close guest against the corrected
  production kernel: PASS, two VFS refusals followed by success,
  exact FILE owner/VM destruction, eventual EXIT and parent WAIT.
- [ ] D13 is still OPEN: the more complex v1126 concurrent APR child
  plus failed unpublished RUN guest reaches `<EA` but then encounters
  an unrelated simulator UUO before terminal WAIT. Diagnose this
  separately, add genuine physical VM rollback refusal to the same
  boot, and pass independent/repeated regressions before checking D13.
- [ ] Rerun the complete optional `P0D_FULL=1` guest matrix and
  final OOM-enabled/disabled qualification before production enable.

## v1126: D13 integrated APR/EXEC/failed RUN investigation (OPEN)

- [ ] **P0 v890 baseline regression (v1126r1):** The exact original
  v890 full PDP-6 test (source restored from committed testkit, not
  the v1126-extended generator) prints `<E` and times out under both
  its original 65-second limit and a separate 180-second host-only
  timeout extension. Rebuild a diagnostic-only baseline tracing
  `proc_exit_finish`, owner/non-owner finalization, exact original
  FILE and VM/u-area disposition, pending PI6 reaper and WAIT state.
  Distinguish a real production regression from simulator progress
  before modifying production. A longer timeout alone did not fix it.

- [x] Add an opt-in disposable full-PDP-6 test that starts a real
  post-commit fatal EXEC owner, retains its private FILE-close owner
  across injected VFS refusals, runs an independent genuine APR fault
  and verifies flagged TERM WAIT, and forces a child RUN to reject
  its real fd map before publication without changing the parent's
  MM process-slot/word accounting.
- [x] Observe successful recovery of the isolated EXEC owner's
  VFS FILE-close provider after 4 refusals/5 calls, with original
  descriptor and witness cleared and one owner EXIT entry. Distinguish
  this from full process finalization: the combined parent NOHANG
  WAIT still failed to see the child reaped (`<EAR1!t`).
- [ ] D13: Determine whether the remaining child is in a valid
  PREVM/post-VM deferred-finalizer state, has an incorrect WAIT parent
  or an erroneous test-only finalizer hook. Confirm the real reason
  for the separately instrumented `<EA`/UUO timeout before altering
  production; do not replace the kernel's checked release with a
  forced success or raise the retry duration without a stage proof.
- [ ] Add and verify a genuine MM pin/refusal/retry for the failed
  unpublished RUN in the *same* guest; existing v893/v895 full-system
  tests establish only independent VM/u-area rollback safety.
- [ ] Rebuild and boot original v890 mode after the extended fixture
  change, finish the combined final WAIT/slot/VM exact-once check,
  and only then mark D13 checked. Production OOM remains OFF.

## v1125: genuine wrong-owner MM plus foreign FS lease (2026-11-04)

- [x] Boot a disposable full PDP-6 guest with the actual slot-2
  u-area MM descriptor changed to an invalid slot-3 owner at its first
  real `mm_free` call. All seven retries return physical MM -4; no
  fabricated return value or forced process release. A separate slot-4
  process acquires the production serialized filesystem-provider lease,
  retains its private kernel continuation and sleeps with the lease
  held. Both the first MM refusal and final HALT observe the real
  `fs_provider_ready` word `5,,0`. The faulting slot remains unreaped,
  WAIT(NO-HANG) refuses a premature result, spinner advances, and
  there is no double release, wrong-owner free or test error.
- [x] Preserve v1101's original wrong-owner mode separately; include
  v1125 in the optional full P0-D boot matrix. D13-D16, sustained
  P0-E and P0-F sizing/activation remain open. Production OOM OFF;
  every v1125 hook and counter is disposable test-only state.

## v1124: physical TTY/HUP, VM and u-area release across real EXIT (2026-11-04)

- [x] D11: Full 256K PDP-6 guest proves a real last-session canonical
  TTY input extent remains pinned after the first user EXIT attempts
  owner-side HUP; EXIT returns -1 and the original session stays
  attached. A second genuine user EXIT unpins and releases the same
  canonical buffer. Its PREVM finalizer then hits a real process-VM
  MM_BUSY (-5), retains the live VM and stable owner, and the actual
  clock-driven slot-0 reaper unpins/finishes that VM. The same reaper
  reaches a separate physical u-area MM_BUSY, preserves exact
  VM_NONE/FINALIZE ownership, then unpins/frees the original u-area.
  PASS counters: TTY/VM/u-area busy each 1, all three pin/unpin
  pairs each 1, VM retained/destroyed each 1, u-area retained 1,
  final commit 1, no errors or leaked terminal TTY/slot. No fabricated
  allocator errors or manual finalizer shortcuts. Test-only hooks.
- [x] Refresh the historical v969 standalone TTY-only guest to arm
  at the current v991 owner-side HUP preflight, not after it.
  Independently rebuild/reboot v969 and v1124: both PASS, with
  preserved original session, one refusal, exact subsequent release.
- [ ] D13-D16 and the remaining pre-enable fault, liveness and
  full-memory-budget requirements remain open after v1125;
  production OOM OFF.

## v1123: fine-grained production OOM enablement checklist (2026-11-04)

This is the active prerequisite list for changing
`PROC_OOM_BRK_ENABLED_V1000` from 0. The default policy is
**requester-only** fatal BRK on proven `MM_ERR_NOMEM` (-1), after
ordinary reclaim/compaction. A failed child RUN must not kill its
parent. P0-A, P0-B (ownership safety), and P0-C are CLOSED;
P0-D/E/F are OPEN. At v1125, 31 mandatory items remain unfinished;
29 are prerequisites for signoff. F12 and F13 are the two post-signoff
implementation/verification actions and must not block the act of
signing off. Existing checked items are prior verified evidence,
not permission to bypass the final combined regression. See
`pdp10-doc/KERNEL/OOM-ENABLEMENT-CHECKLIST-V1123.MD` for gate rules.

### S: freeze scope, ABI and actual trigger

- [x] S01: Keep faulting requester as the sole possible OOM victim;
  exclude third-party/size-based selection (v981-v982).
- [x] S02: Preserve ordinary BRK success, unchanged user ABI and
  exact internal AC2 MM error cause (v999-v1000).
- [x] S03: Prove real BRK physical exhaustion (-1) with normal return
  while disabled and requester-only fatal cleanup when test-enabled;
  unrelated blocked witness survives (v1075).
- [x] S04: Prove invalid BRK/address/floor (-4) stays nonfatal in both
  policies (v1110).
- [x] S05: Prove fragmentation (-2), MM descriptor shortage (-3),
  pinned MM (-5) and failed child RUN do not become fatal requester
  OOM; preserve the exact underlying cause (v1112-v1116).
- [x] S06: Verify the production switch is still disabled in KCORE
  and all current normal RUN/EXEC paths call the *writing* VM startup;
  no raw MEMFS XIP admission in those loaders. Source guard v1123
  rejects six unsafe mutations.
- [x] S07: Freeze the precise release configuration, intended memory
  profiles, boot media, providers/MRES, foreground TTY and available
  swap; distinguish a 256K acceptance profile from deferred 32K DSH.
- [x] S08: Freeze which optional facilities are excluded; do not
  enable root-only raw-XIP RUN/EXEC as part of OOM activation.

### A: allocation-free transitive release graph (closed; revalidate)

- [x] A01: Trace BRK/APR/EXEC/EXIT, FILE/VFS/SYNC, PIPE/DPY, HUP,
  TTY, VM/XIP, SWAP bitmap, u-area and WAIT release edges (v1087).
- [x] A02: Show no new MM extent, backing sector, heap, private
  provider buffer or removable helper required during fatal release.
- [x] A03: Run the v1087 source/negative-mutation contract; owner
  and error paths remain bounded in *allocation*, not necessarily time.
- [x] A04: Rerun v1087 and the full source-linked P0-D matrix at the
  final production candidate revision; fail on any new call edge.

### B: provider ownership and fail-closed behavior (safety closed)

- [x] B01: Preserve original in-flight DTFS/D6FS/TSFS gateway owner,
  its process/private stack, FILE and shared provider workspace.
- [x] B02: Refuse recursive and foreign fatal acquisition safely;
  never steal or reset a held provider lease (v1092).
- [x] B03: Keep a returning but failing FILE/SYNC owner live through
  checked retry; no premature VM/u-area/PID reclamation.
- [x] B04: Verify permanent -5 CLOSE quarantine and recovering
  twelfth CLOSE with one final EXIT (v891/v953).
- [x] B05: Define the explicit limit: a provider that never returns
  may hold its owner indefinitely; repair or controlled restart is
  required without a provider-specific quiescence proof (v1092).
- [x] B06: Test genuine physical-OOM pressure *while* another process
  holds the gateway, including pre-dispatch and already-in-flight
  owners, error retry, correct failure stage and independent spinner.
- [x] B07: Confirm no new code treats a capped retry interval as
  bounded total lifetime or forces reclaim of a blocked owner.

### C: non-sleeping slot-0 and owner-phase transitions (closed; recheck)

- [x] C01: Keep PI6-masked slot-0 POSTVM/PREVM finalizers non-sleeping;
  prohibit disk swap-in and blocking provider acquisition (v1086).
- [x] C02: Run potentially sleeping FILE/HUP preflight on the real
  owner's retained private stack before permanent handoff (v991/v995).
- [x] C03: Preserve VM_NONE/FINALIZE/PREVM identity and exit status
  across checked release refusal, then commit exactly once (v997).
- [x] C04: Boot real PI6-clock/owner-continuation adversarial
  combinations with pending HUP, pinned TTY and VM failures; verify
  independent timer and bystander progress without PI6 deadlock.

### D: idempotent physical release and cross-provider failure (OPEN)

- [x] D01: Exact PIPE/FIFO final-close MM -4/-5 owner retention and
  later release (v870/v1099/v1104).
- [x] D02: Exact Type-340 DPY raw-list pin/busy/unpin/free and live
  PI6 refresh during refusal (v1100/v1105).
- [x] D03: Process resident VM MM_BUSY, saved owner, later VM_NONE and
  no premature u-area free (v1106).
- [x] D04: DISK-backed swap bitmap verifier refusal, real backing
  record retention, repair and one physical free (v1107).
- [x] D05: Physical PIPE plus VM MM_BUSY with FILE/u-area/FS lease
  and bystander WAIT checks (v1118).
- [x] D06: Two distinct physical PIPE and DPY errors during one HUP,
  correct PI6 replay and no duplicate finalized event (v1119).
- [x] D07: Physical DPY MM_BUSY across real swap-out and *successful*
  swap-in, not falsely counted as bitmap refusal (v1121).
- [x] D08: First-member physical PIPE MM_BUSY plus distinct second-
  member real DISK bitmap-refusal/retry in one session (v1122).
- [x] D09: Wrong MM owner/type/base -4 never force-frees another
  resource; quarantined owner and bystander survive (v1101).
- [x] D10: Source-linked real MEMFS XIP MRES-owned unpin/retry and
  VM destroy without process-MM fallback (v1108/v1109).
- [x] D11: Booted physical canonical TTY -5 across user-side HUP,
  process VM and post-VM u-area retry, with the original session
  preserved and all three physical MM refusals repaired (v1124).
- [x] D12: Booted unrecoverable wrong-owner -4 combined with a live
  foreign provider; no forced finalization or slot recycling (v1125).
- [x] D13: Booted nested APR/EXEC fatal request and unpublished RUN
  rollback across failed FILE/HUP/VM or swap backing ownership.
- [x] D14: Mixed PIPE/DPY/VM/u-area plus provider-lock fault in one
  boot, with exact release order and zero premature parent WAIT.
- [x] D15: Decide XIP scope: if production EXEC/RUN cannot admit an
  XIP process, keep it explicitly out of this release and rerun
  v1108/v1109; if enabled, complete separate X01-X08 below first.
- [x] D16: Verify final exact-once FILE/PIPE/DPY, TTY, VM, SWAP,
  process-slot and MM-owner accounting across all supported retries.

### E: whole-system OOM stress and liveness (OPEN)

- [x] E01: Boot real physical exhaustion with unrelated witness
  survival in disabled and test-only enabled policies (v1075).
- [x] E02: After exhaustion, sequentially RUN/reap 24 processes
  reusing numeric PID, verify alternating WAIT status and stale
  requests *between* lifetimes (v1120).
- [x] E03: Boot mixed fatal owner faults with independent spinner
  activity and repeated stale WAIT/PID checks (v1117/v1118).
- [x] E04: Boot two different actual resource refusals in one session
  teardown while maintaining original WAIT/TTY ownership (v1122).
- [x] E05: Define a sustained repeatable duration/cycle threshold
  exceeding the short existing 24-PID/37-spinner/48-stale checks.
- [x] E06: Repeatedly force true physical OOM, reclaim and subsequent
  RUN/BRK success without monotonic MM slots, words or backing leaks.
- [x] E07: Run at least one competing runnable user process during
  held-provider, PIPE/DPY, TTY, VM, and DISK retry intervals; prove
  qualified PI6 clock and observable forward progress.
- [x] E08: Exercise multiple concurrent process groups/sessions,
  stopped processes, parent WAIT/CHLD and numeric PID recycling
  without duplicate event delivery or wrong-owner termination.
- [x] E09: Combine physical RAM exhaustion, disk backing full/failed
  allocation and fragmentation in separate guest scenarios; preserve
  -1/-2/-3/-4/-5 distinctions and no kill on nonphysical shortage.
- [x] E10: Execute APR fault and EXEC post-commit failure while a
  separate provider is busy; ensure the faulting owner retains its
  continuation and final WAIT status is exact.
- [x] E11: Repeat child RUN admission failure/rollback alongside a
  genuinely OOM-killed BRK requester: never kill the RUN parent.
- [x] E12: Confirm no unauthorized INIT/kernel critical process,
  in-flight gateway owner or unrelated bystander is terminated.
- [x] E13: Repeat on the required PDP-6 simulator targets and actual
  release boot/media configuration; archive complete counter logs.
  Exact DAIMOS `226e7a5f`/testkit `ee9d92b` Android aarch64 plus
  independent Linux x86_64, each with OOM off and test-only on,
  SHA256 verified archives and F10 diagnostics all PASS for S07/S08.
  Other platform/media variants are outside the frozen first profile.

### F: OOM safety, acceptance, default enable and rollback (OPEN)

- [x] F01: Keep production `PROC_OOM_BRK_ENABLED_V1000,0` during
  all experiments; enable it only in disposable positive guests.
- [x] F02: Run all P0-D component and optional `P0D_FULL=1` booted
  matrix cases on the *exact* candidate revision, not stale results.
- [x] F03: Rebuild and run disabled/enabled OOM positive and negative
  guest variants v1075/v1110/v1112/v1114/v1115/v1116/v1120.
  All 14 Linux 256K full-guest combinations PASS on DAIMOS
  `96e6c695`, corrected KCC `b9cd764`, with both simulated
  OOM policies. The v1114 test-only descriptor list was corrected
  to cover 136 MM slots (testkit `03e8325`). Exact logs, tool
  fingerprints and SHA256 are archived in
  `$HOME/tmp/daimos-f03-verified-evidence-v1181.tar.gz` on Linux.
  This is F03 acceptance only, not F11/F12 enablement approval.
- [x] F04: Check precise executable/VM/heap/BRK and MM error causes
  under both policies, including no interrupted APR/VM publication.
  On candidate `96e6c695` and corrected KCC `b9cd764`,
  F03 v1110/v1115/v1116 plus the eight v1184 Linux cases PASS:
  v1113 genuine MM -2/-3, v1156 real backing-full in both policies,
  v1158 failed RUN parent plus independent OOM in both policies,
  v1157 APR/postcommit EXEC under a held provider, v1106 pinned
  VM/u-area and v1077 physical held-provider retry. The eight
  additional logs are archived at
  `$HOME/tmp/daimos-f04-cause-evidence-v1184.tar.gz`
  (SHA256 `5c12dce02fe1877a47c19afa9e9339e1a466a39f79526d1960617a048ee69096`).
  Acceptance is for ordinary writable RUN/EXEC/BRK only.
  F05 RETIRED FROM OOM ACCEPTANCE (v1189): the static 16K
  KCORE/MRES permanent-residency budget is a separate kernel-size
  optimization objective, not a condition for requester-only OOM.
  Its measured overflow remains an outstanding KCORE/MRES issue;
  it is NOT a passing OOM test, and is not counted among OOM gates.

  Separate KCORE/MRES size TODO: optimize and independently verify
  the permanent 16K address-space footprint, without making this
  an OOM release prerequisite or misreporting existing overflow.
- [x] F06: Re-evaluate current full/auto packaged permanent-image
  overflow versus `040000` octal; measure the *actually resident*
  module set rather than treating all packages as simultaneously live.
  Independent v1174 SYS_MEMINFO + patched-MOVEI + v1176 MINIT
  inventory reconcile 22 boot-installed MRES modules: 8837 words
  Android and 8839 Linux. The disabled Android disposable full
  DSK/D6FS guest has 9944 KCORE + 272 fixed MM-descriptor words
  + 8837 MRES = 19053 permanent words, **2717 words above**
  the `040000` low-core exclusive limit including origin `060`.
  Enabled test OOM adds 14 KCORE words and remains over budget.
  F06 closes the measurement/inventory audit only; it does **not**
  claim 16K production fit (now outside OOM scope), and MRES remains
  boot-only/permanent with no runtime unload or OOM reclaim.
- [x] F07: Check peak physical MM extent/descriptor use, APR aligned
  slack, startup stacks and provider buffers under maximum load.
  v1195-v1196 partial evidence PASS: real 136-slot descriptor
  saturation/recovery (v1114), physically fragmented APR-aligned
  extents and nonfatal cause -2 (v1115), private/idle stack peaks
  `0300`/`025` octal, and existing single 128-word serialized
  D6FS/DTFS block workspace. These were partial early observations;
  the later combined 64-slot qualification is recorded below. Outputs:
  `$TMPDIR/daimos-f07-mm-evidence-v1196.tar.gz`.
  v1203 adds an exact-source physical-OOM snapshot with the foreign
  provider held: 19/136 live descriptors, six aligned process extents,
  112692 managed allocated words, three gap-prefix APR slack totals,
  and a 128-word shared buffer. Both OOM modes PASS, but 130K words
  remained free; maximum-load and per-process break-rounding proofs
  still remain. See `design/F07-LIVE-OOM-MM-PEAK-AUDIT-V1203.md`.
  v1204 high-pressure full guests supersede the low-occupancy limit:
  127/136 descriptors and 211092 allocated words with 108 extra real
  MM allocations while the foreign FS owner is held, all freed
  exactly; both OOM modes PASS. This is still six real process
  extents, not maximum concurrent process-slot/BRK usage.
  v1274 now closes F07 for the frozen 256K release profile: 63 live
  user processes, 55 resident and eight swapped, 127/136 real
  descriptors, 239576/242085 managed physical words, 31642 exact
  APR-padding words, a still-owned 128-word FS workspace, private
  and idle stack peaks 180/206 and 63/256 words, and 57 real
  TERM/WAIT/reaps with zero final MM/FILE/swap leaks. Every child
  has a distinct verified logical BRK. Complete evidence SHA256:
  `0fb22920bfd9f66c155ef3bb61f8db0c0f146f80f89bbc3e6e1b3406d9599e1b`.
  The test-only 1K BSS per 64-slot image does not weaken these
  measured pressure or ownership requirements. F07 CLOSED; E13/F11
  and production activation remain separate gates.
- [x] F08: Retain deferred strict 32K KCORE/DSH optimization as a
  **separate** gate; do not misreport a passing 256K guest as 32K.
  The frozen first requester-OOM acceptance profile is strictly
  256K KCC/full DSK/D6FS; no 32K GCC/DSH size or boot acceptance is
  claimed here. Strict 32K optimization remains deferred until the
  first OOM release is accepted and is not silently substituted for
  the separately tracked 16K permanent-residency optimization.
- [x] F09: Record all outstanding fail-closed noncompletion cases and
  operator recovery/restart policy explicitly before release signoff.
  See `pdp10-doc/KERNEL/OOM-FAIL-CLOSED-RECOVERY-V1185.MD` for
  in-flight provider/private-stack retention, TTY/FILE HUP,
  VM/u-area BUSY, permanent wrong-owner -4 quarantine,
  backing bitmap errors, boot-only MRES failure, read-only
  GETTEARDOWN/PS diagnostics, and controlled restart versus
  unsafe force-unlock/force-free. This is a liveness limitation,
  not a runtime MRES OOM/reclaim mechanism.
- [x] F10: Require zero unexplained compiler/assembler diagnostics,
  no unsafe tests in production and clean post-stress bookkeeping.
  v1190 full release-media diagnostic scan PASSes 4 archived
  Android/Linux disabled/test-enabled builds, 13 identical approved
  diagnostic classes, zero unexplained warnings/errors and three
  rejected diagnostic mutants. F07 endurance/proc-stack evidence
  remains recorded separately; the v1190 checker is testkit-only.
  Final *exact candidate* source/diagnostic rerun stays mandatory
  under A04/F02/F11; no production OOM change is authorized.
- [x] F11: Explicitly review and sign off S/A/B/C, all applicable
  D/E tasks and F02-F10 excluding retired F05; never infer activation readiness from one
  matrix PASS or from unreviewed checkbox changes.
- [ ] F12: Only after F11, change default-off requester OOM in the
  *production* code, rebuild and rerun the entire regression set.
- [ ] F13: Record post-enable result, revert the switch on any
  regression, update this checklist/PROJECT-STATE/pdp10-doc, then
  commit and push the verified release.

### X: OPTIONAL raw-MEMFS-XIP admission (not part of default OOM scope)

- [x] X01: MEMFS has private MRES op 027 admission and op 026 VM
  release; source-linked XIP/MM tests v1108/v1109 PASS.
- [x] X02: Current normal RUN and EXEC use writing `vm_space_startup`;
  no production XIP launch wired in. v1123 source/mutant guard PASS.
- [ ] X03: If requested later, add strictly root-only trusted raw-XIP
  format verification; reject mutable/PURE-only DXR impostors.
- [ ] X04: Add real no-stack/no-argv/no-env, no-user-write startup;
  do not reuse ordinary writing `vm_space_startup`.
- [ ] X05: Reject XIP BRK, regular swap-out and non-whitelisted writes;
  preserve MEMFS pinned provider provenance for exact unpin.
- [ ] X06: Boot real scheduled XIP RUN, EXEC, fatal EXIT/WAIT and
  failures under pressure with physical pin/refcount/MM checks.
- [ ] X07: Exercise MEMFS unload/reclaim/unmount and foreign provider
  refusal while XIP mappings remain live, with correct denial/retry.
- [ ] X08: Only after X03-X07 pass, extend D/E/F and the OOM scope
  fixture; XIP is otherwise OFF and does not block core-only OOM.

### v1122: real PIPE MM_BUSY and distinct DISK bitmap failure (2026-11-04)

- [x] Combine a resident first member's physical PIPE final-close
  MM_BUSY with a separate second member's actual DISK swap bitmap
  verifier refusal in one session-HUP sequence. Require a retry of
  the first exact PIPE owner before reaching the second DISK owner.
- [x] Full 256K PDP-6 guest PASS: PIPE pin/busy/unpin/free each once,
  two calls/zero wrong-owner errors, one first-member HUP revisit;
  real SWAP-MRES swap-out and two original bitmap-verifier calls,
  one deliberate on-disk backing-bit refusal and exact repair,
  one final backing free and zero leaked record/blocks. WAIT, TTY,
  original u-area pin and delayed completion all PASS.
- [x] Independently rebuild and boot both original v1104 PIPE-only
  and v1107 DISK bitmap-only full-system regressions against the
  extended generator. Both PASS with unchanged owner, fault/repair,
  pinned-u-area, WAIT and foreground-TTY assertions.
- [ ] Continue with booted actual MEMFS XIP, sustained multiple
  in-flight provider/liveness checks and full P0-D/E/F gates before
  production requester OOM activation.

### v1121: DPY release versus real DISK swap-out/restore (2026-11-04)

- [x] Compose v1105 physical Type-340 DPY final-close MM_BUSY with
  v1107 actual production swap-out of the same second HUP recipient.
  Observe and preserve both resources through one guest process.
- [x] Correct the initial assumption that the DISK bitmap verifier
  must execute. An incomplete private DPY FILE close causes a real
  SWAP MRES swap-in, restoring residency before VM teardown. Count
  both the original `proc_swap_in` call and successful return, not an
  inferred restore. A clean boot and simultaneous injected bitmap
  hook alone do not prove that a bitmap error occurred.
- [x] Full 256K guest PASS: one physical swap-out, one actual swap-in,
  exact backing free/cleared terminal DISK record, one physical DPY
  pin/MM_BUSY/unpin/free, 118 PI6 refreshes (90 while held), partial
  HUP retry, correct WAIT/TTY and zero ownership errors.
- [x] Reboot standalone v1107 after the generator extension: original
  true bitmap verifier failure/restoration and ownership PASS unchanged.
- [ ] The genuine v1107 bitmap verifier fault must still be combined
  with an independently owned, concurrently failing resource using
  a valid DISK lifecycle; booted XIP integration and P0-D/E/F remain
  open. Production OOM remains OFF.

### v1120: actual numeric PID reuse after real physical OOM (2026-11-04)

- [x] Boot independent 256K PDP-6 default-OOM-disabled and test-only
  OOM-enabled guests through real physical BRK exhaustion, requester
  completion/fatal retirement, surviving blocked witness and WAIT.
- [x] After exact MM/slot recovery, RUN and reap 24 alternating-status
  children; require the same numeric PID for all 23 adjacent pairs,
  the correct status 062/063 for every WAIT, failed stale WAIT/TERM
  between lifetimes and exact post-OOM MM/slot baseline each cycle.
  Both full guest modes PASS; C89 and kernel ABI remain unchanged.
- [x] Native real XIP/VM v1109 and transitive release audit v1087
  rerun PASS. This closes sequential numeric PID reuse, not the
  in-flight concurrent multi-provider and pressure cross-products.
- [ ] Finish physical XIP/VM and DISK-bitmap mixed ownership across
  distinct processes, long-duration concurrent fault/liveness cases,
  and full P0-D/E/F acceptance before enabling production OOM.

### v1119: booted PIPE plus DPY physical session HUP (2026-11-04)

- [x] Combine the separate v1104 PIPE and v1105 Type-340 DPY final-
  release MM hooks in the same session member and actual HUP delivery.
  Compose original MM entry/busy/delete injection points while
  preserving each typed owner and independent test-only counters.
- [x] Two complete 256K guest boots PASS; each resource has two free
  attempts, one real physical pin/MM_BUSY, one unpin, and one actual
  release. DPY refresh remains active after the busy refusal; first-
  member committed HUP, leader/member WAIT and TTY handoff PASS.
  Correct combined fixture's C89 declaration order and add a versioned
  independent Make target and optional full P0-D matrix row.
- [ ] Complete physical VM/MEMFS-XIP and DISK swap bitmap fault overlap,
  numeric PID reuse, extended mixed-provider liveness and final P0-D/E/F
  acceptance. OOM production activation remains disabled.

### v1118: physical PIPE and process-VM MM fault combination (2026-11-04)

- [x] Compose previously independent real PIPE/MM and process-VM/MM
  fault hooks at the same production `mm_free` entry in a disposable
  kernel. Both typed owner paths reach real MM; the initial conflicting
  test-only insertion anchor is corrected in daimos-testkit.
- [x] Full 256K PDP-6 boot PASS: one PIPE MM_BUSY, one process-VM
  MM_BUSY, exact later releases, VM pin/unpin and swap detach, FILE
  close and HUP-stage retries, one u-area refusal and recovery,
  one finalization/CHLD, intact foreign FS lease, 37 spinner cycles
  and 48 stale WAIT/PID rejections with no process accounting growth.
- [ ] Continue with physical DPY/HUP and XIP plus SWAP cross-stage
  combinations, real numeric PID reuse, long-run liveness and final
  P0-D/E/F enablement acceptance. The v1118 HUP fault is synthetic,
  and production requester OOM is still OFF.

### v1117: integrated stale PID/WAIT and multi-stage fault liveness (2026-11-04)

- [x] Extend the real booted v1103 PIPE/FILE/HUP-stage/VM/u-area
  fault/retry guest with 48 rejected stale WAITs and 48 rejected
  PID-directed TERM events after one exact child finalization.
  Confirm non-growing process MM/slot usage and concurrently runnable
  bystander with the foreign FS lease still held. The original
  release/refusal/CHLD/owner counters and the full guest PASS.
- [x] The first proposed post-fault RUN/WAIT stress was invalid:
  new executable loads legitimately require the deliberately held
  FS-provider lease. Replace that probe with provider-independent
  stale-PID and event checks; no production change or workaround.
- [ ] Still need actual numeric PID-reuse, mixed physical DPY/HUP/
  PIPE/XIP/swap fault combinations, long-lived provider/liveness
  verification and complete P0-E/P0-F activation acceptance.

### v1116: booted exact RUN MM descriptor cause (2026-11-04)

- [x] Saturate the real PDP-6 MM descriptor table through private
  tracked allocations. Record the actual MM return code immediately
  after both possible RUN allocation stages (process VM and u-area).
  Require precisely one -3 at the failing stage and distinguish the
  nonexecuted stage from an actual successful allocation. A valid
  RUN must return -1 and leave no child slot/process-word leak.
- [x] Independently boot default OOM-disabled and disposable
  OOM-enabled guests. Both PASS with exact MM owner cleanup, invalid
  RUN/BRK controls, subsequent physical-OOM requester, bystander and
  WAIT accounting; only the later truly exhausted requester is
  killed in the enabled image.
- [ ] Do not enable production requester OOM yet. P0-D still needs
  remaining XIP/combined release faults and long-run PID/WAIT tests;
  P0-E needs an integrated fault-and-liveness matrix; P0-F requires
  final cross-gate acceptance and an explicit enable decision.

### v1115: booted real physical fragmentation and BRK -2 (2026-11-04)

- [x] Build independent 256K PDP-6 guests under both OOM policies.
  Create physical fragmentation with real MM allocations and typed
  frees. Verify the production fit search reports at least 64K
  aggregate free RAM without a contiguous 64K extent, and the
  production VM BRK returns -1 with exact internal MM cause -2.
  Both policy modes preserve the requester, original break and
  process MM; all test-owned extents are released. The subsequent
  physical-OOM requester, invalid BRKs, failed RUN and blocked
  witness produce the expected WAIT/cleanup behavior. Both PASS.
- [x] The exact MM descriptor-exhaustion cause (-3) from a booted
  failed child RUN is now captured at the real allocator in v1116,
  rather than inferred from a full table. Both OOM modes PASS.
- [ ] Mixed P0-D/E liveness and final P0-F acceptance are still needed
  before enabling production OOM.

### v1114: full-system MM descriptor-pressure child RUN (2026-11-04)

- [x] In disposable 256K PDP-6 images, use genuine MM allocations to
  fill the live descriptor table to its configured capacity. Check
  that valid RUN then refuses without fatal retirement or retained
  process MM/slot; release every acquired test extent by its exact
  base/type/owner. Both default-disabled and test-only OOM-enabled
  guests PASS with existing invalid BRK, OOM requester and bystander
  checks. No production kernel source or resident RAM growth.
- [x] v1115 booted actual fragmented-core BRK -2; v1116 captured
  the raw -3 MM descriptor result of a failed RUN. Both have enabled
  and disabled policy guest checks.
- [ ] Continue P0-D/E mixed-fault and WAIT/PID acceptance before
  considering production OOM activation.

### v1113: native integrated MM/VM/BRK OOM gate (2026-11-04)

- [x] Combine unchanged production PDP-6 allocator fit/insert,
  bounded retry/compaction, VM BRK and opt-in OOM-enabled syscall
  dispatch in a source-linked native test. Actual disjoint MM gaps
  yield -2 and actual full descriptor capacity with a fitting gap
  yields -3. Neither condition fatally retires the requester;
  original process VM, logical BRK and MM count are unchanged.
  Three unsafe error-collapsing mutations rejected.
- [ ] Boot real resource-pressure BRK and RUN conditions with
  fragmented physical core and exhausted MM descriptors. Native
  witnesses use no-progress FS/swap reclaim and relocation refusal,
  not the entire runnable process/WAIT/PID graph. P0-D/E/F remain
  open; do not enable production OOM before full acceptance.

### v1112: booted P0-F failed RUN validation negative (2026-11-04)

- [x] Boot two 256K PDP-6 test images: ordinary default-disabled OOM
  and a disposable OOM-enabled image. In both, a valid RUN V2 request
  naming a nonexistent executable returns -1 with unchanged parent,
  process slots and process-memory words. Parent, v1110 BRK-negative
  children, real physical-OOM requester and bystander behave exactly
  as expected. Both sets of WAIT outcomes and final MM counters PASS.
- [ ] This does not cover a RUN that fails specifically because MM
  descriptors or contiguous core are exhausted. Continue physical
  descriptor/fragmentation fault acceptance through BRK and RUN,
  P0-D/E cross-product tests, and the production enable decision.

### v1111: native MM resource-cause classification (2026-11-04)

- [x] Link real PDP-6 `mm_find_fit`, `mm_extent_insert` and
  `mm_alloc_aligned_noreclaim` to test -1 insufficient total RAM,
  -2 fragmentation with enough aggregate free words, and -3
  descriptor-capacity exhaustion with an available fitting gap.
  Verify failed allocations do not consume the original extents,
  and allocation succeeds after increasing only descriptor capacity.
  Two unsafe -2/-3-to--1 mutations rejected. Native v1000 OOM
  dispatch classification also PASS.
- [ ] Boot both OOM-disabled and disposable enabled BRK requests
  with actual fragmented MM and exhausted descriptors; verify no
  incorrect requester termination, WAIT/PID leak or owner loss.
  P0-F failed-RUN, P0-D/E and production OOM enablement remain OPEN.

### v1110: booted P0-F invalid BRK classification (2026-11-04)

- [x] Extend the 256K PDP-6 v1075 requester-only guest in an opt-in
  test mode: BRK below image floor and a rounded 18-bit address-limit
  overflow both return -1 with unchanged break, expected child WAIT
  and recovered process-MM accounting. Confirm alongside the original
  genuine physical exhaustion requester and independent witness.
- [x] Both booted configurations PASS: default-disabled OOM returns
  -1 for physical exhaustion without killing the requester; isolated
  test-enabled OOM retires only the exhausted requester, never either
  invalid-BRK child. No production OOM enablement or permanent RAM.
- [ ] P0-F still needs MM descriptor exhaustion, fragmentation,
  failed-child RUN negative cases and final P0-A..E activation gates.

### v1109: VM + physical MEMFS XIP release integration (2026-11-04)

- [x] Combine the actual resident VM-destroy assembly, MEMFS XIP
  provider dispatch/ownership/unpin and physical MM pin/unpin in one
  executable native PDP-6 fault test. First -4 retains VM and XIP,
  skips process MM free and backing detach; the original MM extent is
  repinned, then one successful VM retry unpins/releases and commits
  VM_NONE with one backing-detach call. No serialized provider lock
  acquisition or clock wait.
- [x] Reject unsafe VM fallback-on-owned-XIP-error and inverted detach
  success mutations. Tests use a stub resident-mode/backing witness,
  not fabricated on-disk allocations. Expanded native P0-D matrix PASS,
  optional full booted guest matrix not rerun in v1109.
- [ ] Boot a genuine MEMFS-XIP executable only after trusted no-write
  certification exists for its dedicated file extent. Validate real
  EXEC admission, VM/FINALIZE/WAIT/PID ownership and physical refusal;
  do not silently expose an ordinary writable DXR file as shared XIP.
  P0-D/E/F remain OPEN, requester OOM stays disabled.

### v1108: native real MEMFS XIP pin-state fault (2026-11-04)

- [x] Extend the source-linked MEMFS release fixture to test a real
  unpinned MM extent with an owned XIP reference. The original MM
  unpin returns -4, ownership is retained, and a real MM pin followed
  by the original XIP release succeeds once; second release is inert.
  Foreign FS provider lease stays held and no slot-0 clock wait occurs.
  Two new unsafe XIP reference-drop/retain mutations rejected; v1086
  and v1094 compatibility and negative mutation tests PASS;
  expanded v1096 native P0-D matrix PASS.
- [ ] Boot a real process with an actual MEMFS-owned executable mapping
  and inject a physical XIP release refusal during VM teardown. Check
  PREVM/FINALIZE, WAIT/PID and MM/XIP owner preservation through the
  slot-0 retry before closing this P0-D branch.

### v1107: finalizer/run-queue collision (2026-11-04)

- [x] Decode SIDL state before interpreting scheduler RH bit 02 as an
  unpublished RUN discard-VM marker. Preserve live PREVM+FINALIZE retry
  when the same field contains run-queue successor slot 2. Source-linked
  v1107 test PASS and old unsafe order rejected.
- [x] Booted 256K fatal session HUP/WAIT and deferred slot-0 PREVM
  finalization PASS; original v907 and physical PIPE/DPY/VM guests
  v1104-v1106 rerun PASS. Full production KCC build PASS with
  `__kcore_low_end` unchanged at `023171` octal.
- [x] Genuine SWAP MRES DISK allocation (16 blocks, first block zero)
  and a test-only bitmap verifier refusal preserve the exact owner.
  Slot-0 retry restores the bit, validates the extent, frees physical
  backing exactly once and clears the DISK record. Counters: 2 bitmap
  verification calls, 1 corruption/repair, 1 backing free, 5 reaper
  scans, 0 duplicate HUPs, 0 ownership errors. HUP/WAIT/TTY PASS;
  native P0-D matrix PASS. No production OOM activation.
- [ ] P0-D still requires a genuine MEMFS-owned XIP release-refusal
  lifecycle, more simultaneous resource failures and sustained WAIT,
  PID reuse, leak and liveness acceptance. P0-E/F remain OPEN. This
  bitmap case does not cover physical storage-I/O errors.

### OOM activation gates (current as of v1107, 2026-11-04)

Assumptions: a functioning PDP-6 APR line clock and ordinary hardware
interrupt delivery. A physically failed system clock is a hardware
failure outside OOM recovery scope. Kernel-caused PI6 inhibition and
software/device deadlocks remain kernel correctness requirements.

- [x] Requester-only policy, BRK MM_ERR_NOMEM classification, default-
  disabled production gate, 256K physical exhaustion OOM child exit
  with surviving independent process, and combined D6FS SYNC/MM_BUSY
  refusal/recovery guest (v1000/v1075/v1077).
- [x] Direct release leaf allocation checks, successful TTY/PIPE/DPY
  MM refusal/retry, and unpublished RUN TTY-before-VM rollback owner
  (v1081-v1085); no additional owner/reaper table is introduced.
- [x] P0-A (v1087): Current BRK/APR/EXEC/EXIT and foreign/self fatal
  release transitive graph CLOSED through FILE/VFS SYNC/PIPE/DPY,
  session HUP, TTY, VM/MEMFS-XIP, swap bitmap, MM and notification.
  No new MM/backstore allocation; refused release keeps FD/process
  owner, partial HUP retries safely, committed finalizers remain marked.
  Executable contract rejects 11 unsafe source mutations; native PDP-6
  rollback and full physical-OOM guest regressions PASS. A foreign
  provider *already executing* is the separate P0-B lifetime gate.
- [x] P0-B (v1092, safety/containment gate): Establish owner-safe lifetime
  for in-flight provider
  operations, including a foreign live lease, a reentrant request,
  a permanently rejecting provider, and an active call which does
  not return. Do not steal provider locks, invent wakeups or free
  stack/FILE/MM buffers behind their owner. Specify whether permanent
  refusal is fail-closed quarantine or demands provider-specific
  recovery; bounded *retry delay* is not bounded *total completion*.
  v1088 validates the live provider-stack lease with two unsafe native
  negative controls; v1089 bounds DTC/MTC data-transfer hardware stalls
  on qualified clock ticks. v1090-v1091 allow TERM/HUP/INT/PIPE on an
  *undispatched* FS lease waiter to wake and abort its own blocked
  request, without waking or freeing the in-flight provider owner.
  Arbitrary nonreturning provider software and permanent close refusal
  use **fail-closed quarantine**: retain the executing provider owner,
  lease, FILE/MM references, private kernel stack, process slot and
  retry continuation. An undispatched fatal waiter may withdraw its own
  request (v1091). Never free, force unlock, fabricate a completion,
  or promise eventual reclamation. Existing GETTEARDOWN/PS diagnostics
  report failed FILE close; KCORE map and read-only lease inspection
  identify active owner. Provider-specific verified quiescence or a
  controlled system restart is required when an owner never returns.
  Native v1092 contract rejects 14 unsafe mutants; v891 permanent -5
  and v914 permanent -4
  and v953 recovery-from-11-refusals booted guest PASS on current code;
  v1088/v1090 and v1091 live held-owner guest previously PASS.
  This gate establishes *safety*, not bounded liveness. OOM OFF until
  P0-D/E/F have their own independent acceptance.
- [x] P0-C (v1086): Both PI6-masked slot-0 entries now have a complete
  non-sleeping call graph: immediate EXIT publishes PREVM+FINALIZE
  before stack switch and never repeats HUP; the periodic idle reaper
  visits only marked EXIT/SIDL rollback. MEMFS XIP uses direct provider
  dispatch and real MM unpin; swap detach is bitmap-only, and
  disk-backed swap-in runs only after PI6 restoration. Source contract,
  unsafe mutation controls, native PDP-6 and full guest tests PASS.
  No hardware-clock recovery or new watchdog required.
- [ ] P0-D: Establish fault-stage commit/idempotence for failed HUP,
  TTY line, XIP/VM, swap backing, PIPE/DPY, and final u-area frees.
  Distinguish retryable MM_BUSY from invalid type/owner (-4); never
  return success while an allocation still lacks a retry owner.
  v1093 now preserves real -5/-4 results in checked TTY line release,
  with shared best-effort/checked free implementation. v1094 prevents
  an *owned* MEMFS XIP unpin failure from falling through to ordinary
  MM_TYPE_PROCESS deletion. v1097 now propagates the underlying -5/-4
  from the final u-area MM release instead of collapsing to -1; wrong
  base/type/owner retains exact process descriptor and MM retry owner.
  v1098 fixes the first PREVM retry clock not being armed when EXIT
  prepublished FINALIZE+PREVM before a refused VM destroy. The repaired
  v997 booted retry/CHLD/provider ownership guest PASSes; its first VM
  rejection is followed by exactly one successful retry, then an
  MM_BUSY u-area rejection and successful second attempt. All stages
  retain original owners until actual completion.
  v1099 preserves typed MM_FREE error (-5 pinned, -4 invalid) across
  final anonymous PIPE and FIFO CLOSE while retaining the descriptor,
  references and FIFO list; four unsafe native variants are rejected.
  Full cross-stage fault matrix remains OPEN.

### v1106: booted physical VM extent -5 and committed swap-detach (2026-11-04)

- [x] Extend the original v997 full guest with a v1106 optional mode
  that pins the actual dying slot-5 process-VM MM extent, rather than
  returning a synthetic VM failure. The original allocator returns
  MM_BUSY (-5) and preserves PREVM/FINALIZE and VM ownership.
- [x] Slot-0 retry physically unpins the exact original extent, then
  validates and frees it once through real `mm_free`, invokes the actual
  `proc_swap_detach` once and publishes VM_SPACE_NONE. Counts: two VM
  calls, two process MM frees, one pin/unpin, one -5 refusal, one
  successful deletion, one swap-detach execution, zero owner errors.
- [x] Same booted guest retains two FILE close refusals, a live FS
  provider lease, one real final u-area MM_BUSY and independent runnable
  bystander (36 cycles); EXIT/CHLD/finalization once. Native v1096
  fault-stage matrix, original booted v997 and optional booted v1103
  multi-stage guest all PASS. No production KCORE, MRES or OOM change.
- [ ] Swap-detach here handles an *empty* resident backing record;
  booted failure of allocated DISK backing, physical MEMFS-XIP release
  and larger WAIT/PID/leak cross-products remain OPEN. P0-D/E/F OPEN.

### v1105: booted physical member HUP + pinned Type-340 DPY (2026-11-04)

- [x] A second real session member installs a 32-word raw DPY0 program,
  then gets remotely HUPed while the first member owns a previously
  committed, physically pinned u-area. FILE/DPY STOP calls production
  `mm_free`, which returns real -5 on a test-only physical pin.
- [x] The published DPY raw list and descriptor survive refusal;
  real PI6 clocks replay 8 frames after MM_BUSY and before unpin.
  Subsequent HUP unpins and frees precisely that original DPY extent
  once. Two MM frees, 1 pin/unpin, 1 MM_BUSY, 1 successful free,
  14 total raw refreshes, zero owner errors.
- [x] Parent WAIT_NOHANG cannot reap the leader on first failed HUP;
  session retry skips the committed first member. Existing HUP/WAIT,
  TTY handoff, pinned first-member u-area and duplicate-delivery
  assertions PASS. Separate booted v1104 PIPE/HUP and original v907
  HUP regressions, plus complete native P0-D matrix, all PASS.
  No production kernel, KCore, MRES or OOM change.
- [ ] Still missing booted combined XIP/VM/swap refusals and broad
  WAIT/PID/resource leak failure cross-products. P0-D/E/F OPEN.

### v1104: booted real member HUP and pinned PIPE final close (2026-11-04)

- [x] Extend the existing v907 disposable physical-HUP guest: real
  session leader and two members, each with a private anonymous PIPE.
  The first member has its original pinned u-area while the second
  member's HUP-triggered final PIPE close hits real `mm_free` -5.
- [x] Preserve the last PIPE writer reference and FILE descriptor on
  refusal; reattempt the exact extent and release it only after unpin.
  Counters verify 2 free attempts, 1 pin, 1 unpin, 1 MM_BUSY, exactly
  1 successful physical free and zero other errors.
- [x] The leader remains non-waitable after partial session HUP. On
  retry, skip the first member's committed VM_NONE+FINALIZE owner,
  finish the second member and retire TTY input/session only once.
  The original first-member u-area pin/unpin and HUP/WAIT validations
  remain in force; no repeated HUP to an already finalized member.
- [x] Test-only isolated source/guest; P0-D native source-linked matrix
  and original booted v907 baseline PASS; no production KCORE changes,
  resident state, MRES or OOM switch.
- [ ] P0-D still needs combined DPY + HUP, VM/XIP and swap physical
  failure cases and additional WAIT/PID/reclamation acceptance; P0-E
  and P0-F remain OPEN. Production OOM termination remains OFF.

### v1103: booted mixed PIPE/FILE/HUP-stage/VM/u-area retry (2026-11-04)

- [x] Added one disposable 256K PDP-6 multi-stage guest variant to the
  existing v997 owner/provider/WAIT/bystander acceptance harness. The
  selected child retains a private, real anonymous PIPE until fatal
  FILE cleanup; its actual final-close `mm_free` returns -5 once and
  then succeeds, with original PIPE reference counts checked at both
  attempts. Two initial FILE closure refusals precede PIPE teardown.
- [x] A separate test-only refusal at `proc_session_teardown` makes the
  same fatal owner retry the HUP *stage* once after FILE closure.
  This is not physical TTY member HUP delivery; that is covered by
  the independent booted v907 real HUP/partial-commit regression.
- [x] The same owner then encounters the pre-existing v997 single VM
  refusal followed by an actual u-area MM_BUSY -5 refusal; both recover.
  The prepublished PREVM marker prevents repeat HUP on slot-0 retries.
- [x] Booted v1103 counters PASS: FILE 3 attempts/2 rejects; PIPE
  2 physical MM frees/1 reject, correct retained reference counts;
  HUP stage 2 calls/1 reject; VM 2 calls, u-area 2 calls/1 reject;
  one EXIT/CHLD/finalization, live foreign provider lease, independent
  bystander 36 cycles, zero ownership/provider errors. No production
  kernel changes, new permanent data, MRES, or enabled OOM policy.
- [x] Unmodified v997 baseline rerun with the extended opt-in test
  harness also PASS; full v1096 native release matrix PASS.
- [ ] P0-D remains OPEN for true simultaneous session-member HUP plus
  PIPE/DPY and XIP/swap fault combinations, and further PID/WAIT
  release-order verification; P0-E/F still OPEN, production OOM OFF.

### v1101-v1102: permanent wrong-owner MM and reaper backoff (2026-11-04)

- [x] Booted v1101 wrong-owner test uses the real MM extent table and
  production `mm_free`: poison only the dying slot-2 u-area extent's
  owner once (01002 -> 01003). `GETTEARDOWN` reports MM stage and
  WRONG_OWNER (0402); WAIT/slot reclamation must not commit while the
  extent is misowned. An independent process keeps running.
- [x] v1101 initially exposed a real v1098 backoff regression: the
  idle loop cleared the global pending bit BEFORE running deferred
  teardown. On repeated -4/-5 refusal, `proc_finish_mark_pending`
  mistook the cleared bit for a first, unarmed prepublished EXIT and
  reset the interval to 020 octal on every attempt. The first guest
  observed 41 invalid frees and 40 idle-reaper calls, exceeding the
  previously accepted permanent-failure retry bound.
- [x] v1102 keeps the pending bit asserted during the entire idle
  retry scan. AC11's unused bit 1 tracks any rejected release; after
  scanning, clear pending only if no retry failed, and restore AC11's
  previous completion-result ABI. The existing capped delay therefore
  doubles 020/040/0100/.../01000 octal on persistent refusal. No new
  reaper table, permanent words or process state.
- [x] Native source-linked PDP-6 v1102 validates two failures,
  exponential backoff and exact successful retirement. Four unsafe
  variants (including the original early-clear bug) rejected.
  v881/v1098 finalizer tests and full v1096 native release matrix PASS.
- [x] Booted 256K v1101 wrong-owner: one actual extent corruption,
  seven invalid MM frees, six idle-reaper calls, retained owner,
  no WAIT completion, runnable bystander and zero unrelated errors PASS.
  Booted v909 physical-pin -5 baseline also PASS: one real pin,
  seven busy MM frees, six reaper calls, owner retained and bystander.
- [x] Full production KCC image PASS: `__kcore_low_end` changes from
  `023162` to `023171` octal (+7 resident words). No BSS or MRES growth.
- [ ] P0-D still requires a simultaneous multi-provider HUP/PIPE/DPY/
  VM/swap/u-area stage matrix and further booted owner/WAIT/PID checks.
  P0-E/F remain OPEN. Production OOM kill remains DISABLED.

### v1100: retain original DPY old-list MM failure (2026-11-04)

- [x] A failed MM free of the *published* Type-340 display list
  previously reported generic -1 when release of its newly allocated
  unpublished replacement succeeded. This lost the distinction between
  a pinned old list (-5) and an invalid old extent owner/type (-4).
- [x] Reuse the existing allocation-result stack local, no additional
  kernel data: save the old-list MM return before rolling back the
  unpublished allocation, then return that exact result after a
  successful rollback. On a second failed free, retain the unpublished
  orphan owner and return its own failure as before. Installed list,
  BLKO descriptor, refresh and FILE ownership are preserved.
- [x] Actual PDP-6 v874 replacement/rollback fixture PASS; new v1100
  source-linked cases test -5 and -4 with two unsafe error-losing
  variants rejected. Production full KCC image PASS with unchanged
  `__kcore_low_end` `023162` octal; v1100 P0-D native matrix PASS.
  The previous booted v997/v1077 combined fault guests PASS.
- [ ] P0-D still needs simultaneous cross-stage HUP/DPY/PIPE and
  permanent invalid owner boot tests; OOM production bit remains OFF.

### v1099: typed PIPE/FIFO final-close MM refusal (2026-11-04)

- [x] `pipe_close_ref` already retained live PIPE reader/writer counts
  and restored named-FIFO list ownership on failed final `mm_free`.
  However, it discarded the actual MM error and returned generic -1.
  Propagate the original -5 busy or -4 invalid type/owner through
  FILE close without changing the release transaction or retry owner.
- [x] Anonymous failure returns directly through `pipe_close_done`;
  named FIFO unlink rollback now uses scratch AC3, leaving AC1's MM
  error intact. No premature reference publication, failed-free success,
  new allocation, forced MM release or FIFO orphan is permitted.
- [x] Source-linked native PDP-6 v1099 runs production FILE entry,
  PIPE close and FIFO unlink with deterministic -5 and -4 MM returns;
  verified retained FD and FIFO references, exact later success and
  normal nonfinal close. Four unsafe mutations (anonymous error collapse,
  FIFO error collapse, lost list owner, premature reference publication)
  are all rejected.
- [x] Expanded v1096 PIPE/DPY/HUP/TTY/XIP/SWAP/u-area component matrix
  PASS. Updated v1087 allocation-free graph audit PASS; its eleven
  unsafe source mutations still all reject.
- [x] Full 256K KCC production image PASS, same KCore low-end
  `023162` octal as v1098. No additional fixed BSS, owner words,
  MRES, process metadata or ABI fields.
- [ ] P0-D still needs all requested combined booted stage faults
  and permanent invalid-owner diagnostics. Production OOM kill OFF.
- [ ] P0-E: Booted PDP-6 fault-injection matrix: real memory pressure,
  FILE+HUP+VM/MM combinations, pending EXEC/APR fatal, unpublished
  RUN rollback, pinned TTY/XIP, concurrent provider owner, and a
  continuously runnable bystander. Check no unrelated process kill,
  duplicate HUP/finalization, premature PID reuse, incorrect WAIT,
  blocked PI6, or leaked MM/FILE/backing references.
- [ ] P0-F: Verify all standard OOM-disabled negative cases (invalid
  BRK, address limit, MM descriptors/fragmentation, failed child RUN)
  remain ordinary errors; only exhausted requester BRK terminates
  when enabled. Preserve full KCC/KCORE/MRES size and 16K mapping
  constraints; enable the production gate only after P0-A..E pass.

### v1096-v1097: P0-D component matrix and typed u-area release (2026-11-04)

- [x] Fix `proc_uarea_release`: valid u-area MM errors were previously
  collapsed into generic -1 after `mm_free`. The real status now survives
  (-5 pinned/busy, -4 invalid type/owner or missing extent); an invalid
  base with the live-u-area flag set explicitly returns -4. On all
  failures the original process descriptor and allocation remain owned.
  Zero is returned only after the exact free or when no u-area exists.
- [x] Source-linked real PDP-6 v1097 extends v956 tests for wrong owner,
  wrong type, pinned extent, zero base with set u-area flag, exact
  success, and repeated idempotent release. Three unsafe variants
  (return collapse, early owner clear, wrong base classification) fail.
- [x] v1096 component matrix executes production PIPE final-close,
  DPY replacement rollback, swap detach, HUP committed-child skip,
  u-area release, TTY PREVM and unpublished child rollback, typed
  TTY and XIP/VM release, plus v1083/v1087 no-new-allocation contracts.
  All component tests PASS on this tree.
- [x] Full production KCC 256K image builds with `__kcore_low_end`
  `023154` -> `023156` octal (+2 words), no new persistent data,
  process table, MRES or syscall ABI fields.
- [x] Booted v997 PREVM + u-area two-stage refusal after v1098 PASS:
  VM calls 2, u-area calls 2, injected MM_BUSY 1, prior FILE rejects 2,
  correct CHLD/EXIT, live foreign provider lease, bystander 36 cycles,
  zero ownership errors. Before v1098 the same guest timed out with
  VM calls 1, u-area calls 0, and bystander progress; that result was
  an actual unarmed-slot-0-clock defect, not a random simulator stall.
- [x] Rerun v1077 real physical-OOM D6FS/MM refusal guest on exact
  v1098 source: 2 rejected D6FS SYNC, 1 MM_BUSY, correct requester
  recovery and surviving independent witness PASS. Only this
  disposable guest enables requester OOM; production remains OFF.
- [ ] P0-D remains open for additional *simultaneous* HUP/PIPE/DPY,
  swap backing and u-area fault combinations, permanent bad owner
  classification through finalizer diagnostics, and an exhaustive
  booted no-leak/no-reused-PID matrix. OOM stays OFF.

### v1098: arm the first prepublished EXIT PREVM retry (2026-11-04)

- [x] Fix a real P0-D/P0-C interaction: v1086 publishes FINALIZE+PREVM
  while transferring EXIT off its private stack. `proc_finish_mark_pending`
  previously assumed any marked finalizer already had an armed shared
  retry clock, and skipped first-time delay initialization after the
  first refused VM destruction. The resulting `proc_finish_last_tick`
  pending bit with a zero delay was (correctly) ignored by idle reaper,
  leaving a permanently unreaped, owned process despite healthy ticks.
- [x] If an EXIT marker is prepublished but the shared pending flag is
  absent, initialize the first 020-octal-tick retry from the current
  bounded APR epoch before publishing pending. A *previously* pending
  owner retains its existing backoff; a new finalizer resets it. No
  bypass of zero-delay safety, forced MM release, PI6 sleep or new RAM.
- [x] Native PDP-6 v1098 executes the real retry publication for
  first premark, already pending owner, new owner and stale epoch;
  three unsafe clock/backoff mutations rejected. Existing v1081/v1085
  TTY/PREVM, v881 u-area, v1086 PI6 and v1087 fatal allocation closure
  tests PASS with correctly modeled test-only clock word.
- [x] Previously known-failing v997 full-system fault guest now PASS:
  exactly two VM and two u-area release calls, one physical MM refusal,
  correct parent CHLD and completed fatal owner, retained live foreign
  lease, 36 bystander cycles, no integrity error.
- [x] Exact production 256K KCC full image PASS: KCORE endpoint
  `023156` -> `023162` octal (+4 instructions), no new permanent
  owner, timer, process or MM data. v1077 physical OOM/D6FS/MM guest
  and the v1096 component matrix including v1097/v1098 PASS.
- [ ] P0-D stage matrix continues with multi-resource mixed failure,
  long-term invalid owner and failure-diagnostic cases. OOM stays OFF.

### v1093-v1094: typed TTY/MM and XIP/VM teardown stage fixes (2026-11-03)

- [x] v1093: share one canonical-line MM release between checked TTY
  teardown and the original void reset. The checked helper now returns
  actual `mm_free` status: -5 pinned/busy; -4 wrong owner/type, missing
  extent or invalid tty; zero only after a completed free or if already
  absent. All refusals retain the line base and session owner, so a
  later valid retry frees exactly once. No new permanent data.
- [x] v1093 real PDP-6 C+MM native v897/v968 PASS: typed busy/invalid,
  pointer and session retention, repaired ownership and exact free.
  Two unsafe variants (error collapse and early pointer clear) rejected.
  Booted 256K v898 pinned/unpin TTY session/child regression PASS:
  5 shared helper calls, 1 pin, 1 unpin, 1 busy, zero errors. Unchecked
  reset now shares the helper and therefore has one additional counted
  call; neither physical pin nor release is duplicated.
- [x] v1094: distinguish the absent MEMFS XIP provider's -1 from an
  *owned* XIP pin-release error. `memfs_mres_xip_release` now returns
  -4 for owned unpin refusal. VM teardown will only use MM_TYPE_PROCESS
  fallback for non-XIP status 0 or an absent-provider -1; other negative
  statuses preserve the original XIP VM/slot for PREVM retry. No stale
  VM_NONE/bitmap publication, unexpected MM free or new owner table.
- [x] Native v1094 source-linked VM/MEMFS release stages PASS: owned XIP
  refusal and recovery, normal/absent provider fallback, MM -4/-5
  refusal and retry, failed disk backing detach and retry; four unsafe
  mutations rejected. Old v1086 PI6, v880 swap-detach, v1081 PREVM
  finalizer and v1087 transitive no-allocation regressions PASS.
- [x] Full 256K KCC production image build PASS; `__kcore_low_end`
  `023161` -> `023154` octal (-5 resident instructions), no new BSS.
- [x] Booted v1077 256K physical-OOM D6FS/MM guest PASS on this tree:
  two rejected D6FS SYNC calls, one rejected MM_BUSY free, correct
  requester recovery and surviving independent witness. The enabled
  OOM bit exists only in the disposable test image, never production.
- [ ] Continue P0-D with combined HUP/PIPE/DPY/u-area and swap backing
  faults, including permanent -4 ownership corruption versus retryable
  -5 busy. These partial leaf fixes do not close P0-D, nor enable OOM.

### v1092: P0-B fail-closed quarantine contract (2026-11-03)

- [x] Decide policy: a permanently rejecting FILE provider remains an
  owned, diagnosable, throttled retry on the same live private stack;
  a provider whose call never returns owns its original serialized
  lease, shared block workspace and process slot indefinitely. A
  second process blocked *before* entry may consume a pending fatal
  event and leave safely, but may not alter the held provider.
- [x] No generic asynchronous cancel, lease theft, forced PID reuse,
  fabricated wakeup or MM/FILE free. Read-only diagnostic `PS TD=1`
  and ERR expose an owner-side rejected close; the owner-encoded
  `fs_provider_ready` word is available in the KCORE symbol map for
  non-mutating console inspection. Neither gives permission to free.
- [x] Source-linked v1092 safety contract rejects fourteen independent
  unsafe mutations involving lease publication/unlock, recursion,
  foreign fatal wait, fatal owner reuse, targeted wake/WAIT class,
  resource commit, private backoff and premature OOM activation.
  v954 owner audit and v1088/v1090 native gateway tests PASS.
- [x] Booted 256K v891 *permanent* VFS -5 refusal: 11 retries, 11
  sleeps, every rejected FD retained, actual GETTEARDOWN ERR 5,
  capped 512-tick backoff, no premature EXIT or FD clear, independent
  user progress and normal diagnostic HALT PASS.
- [x] Booted v914 *permanent invalid* VFS -4 refusal: 11 failures,
  11 sleeps, FD and error witness retained, capped delay, no exit,
  and independent bystander progress PASS. Invalid provider results
  are not disguised as successful release or blind MM_BUSY retries.
- [x] Booted v953 *revival*: 11 refused calls/backoff followed by one
  successful provider return; FD/error owner clears, EXIT executes
  exactly once and independent witness progresses PASS.
- [x] v1091 booted nonreturning lease owner plus targeted TERM of an
  undispatched waiter remains accepted; new v1092 source contract
  rejects forced unlock or remote stack destruction.
- [ ] Provider-specific **online recovery** of an unreturned call may
  be developed only with positive device quiescence and owner-side
  completion proof; no generic runtime recovery is claimed. Fix any
  demonstrated driver or provider infinite loop at its source.
- [x] No DAIMOS production code, resident RAM, MRES, syscall or ABI
  change at v1092. P0-B safety gate CLOSED under fail-closed policy;
  physical OOM termination remains disabled until P0-D/E/F finish.

### v1090-v1091: cancel pre-dispatch FS waits on fatal event (2026-11-03)

- [x] Distinguish an executing serialized filesystem provider owner from
  a different process blocked before gateway dispatch. Foreign fatal
  delivery already deferred to a live kernel continuation, but the
  blocked waiter could not consume the event if the owner never returned.
- [x] Preserve the existing event bitmap and process-private stack;
  after recording INT/TERM/HUP/PIPE, wake only a `PROC_STATE_SLEEP`
  process with exactly `PROC_WAIT_EVENT_LH` and the existing FS provider
  wait-event address. WAIT_CHILD, WAIT_INTR, job STOP and arbitrary
  wait channels remain untouched. No other wakeup or owner-lock change.
- [x] The awakened waiter checks pending fatal bits before attempting
  a lease acquire, unwinds its saved gateway request to -5 without
  provider dispatch, and consumes the fatal event at user return.
  A process already in fatal FILE teardown bypasses the new abort check
  and may enter a free provider to release its own resources.
- [x] Native PDP-6 v1090 source-linked test PASS: exact owner lease,
  targeted wake, unmodified private stack, arbitrary-event/STOP/INTR
  exclusion, fatal-close-owner completion and four rejected unsafe
  variants. Updated v1087 transitive allocation-closure contract PASS.
- [x] Booted v1091 disposable 256K PDP-6 PASS: slot 3 stays suspended
  inside provider with lease `4,,0`; slot 4 sleeping behind that lease
  consumes remote TERM, returns to user, exits and is reaped with exact
  WAIT status. Independent slot 2 completes 73 sleep cycles; 2 deferred
  fatal events, zero errors and unchanged in-flight owner. No forced
  lock release, orphaned FILE/MM buffers, or premature PID reuse.
- [x] Production KCC full-image rebuild PASS. Matched `__kcore_low_end`
  `023127` -> `023161` octal (+26 resident words); no additional
  persistent KCORE data, MRES data, process fields or owner table.
- [x] Rerun booted v1077 physical OOM guest against the v1091 tree:
  2/3 D6FS SYNC refusals, 1/2 MM_BUSY refusals, correct requester
  recovery and independent witness PASS. Production kill stays OFF.
- [ ] This recovers an unstarted waiter, not arbitrary software stuck
  inside a provider. Keep P0-B open and production OOM kill disabled.

### v1089: Type-136 tape data-transfer watchdog (2026-11-03)

- [x] Find and fix a reachable in-flight provider liveness defect:
  `tape_transfer_wait` had no bounded error completion once DECtape search
  or Type-516 record I/O entered its negative `storage_state` transfer.
  A missing PI3/PI5 completion could retain the owning filesystem lease
  and process-private stack indefinitely despite healthy APR ticks.
- [x] Reuse the now-inactive DTC search retry word as one 60-Hz transfer
  countdown: 300 ticks for DTC and 5400 ticks for MTC. On expiry the
  tape driver stops Type-136/controllers, marks state 7, accounts the
  appropriate tape error and wakes its original transfer waiter. The
  waiter completes through its existing error path; only its returning
  provider owner unlocks the shared lease. No forced unlock or new BSS.
- [x] Patch an optional KINIT-installed tape-MRES clock export after the
  ordinary storage countdown tick, preserving DSK/DRM hooks and the
  existing five tape exports. Native PDP-6 timeout/state/source-contract
  and negative-mutant tests PASS; online MTC read/write/control, DTC
  two-block stream and reverse read/write regressions PASS.
- [x] Clean full 256K KCC image build and full-media simulator startup
  reach `INIT V1` and `LOGIN`, with DTC/MTC both reporting `OK`.
  Matched KCC `__kcore_low_end` `023126` -> `023127` octal (+1 code
  word). Pre/post tape MRES source and package comparison: text
  360 -> 390 (+30), resident 375 -> 405 (+30), BSS 15 unchanged;
  combined KCORE+installed tape resident cost +31 words, no new
  persistent state. Stop at LOGIN deliberately after boot.
- [x] Rerun v1077 disposable 256K physical-OOM provider/MM guest on the
  updated production tree: two injected D6FS SYNC refusals, one MM_BUSY
  refusal, correct requester/witness recovery and exit accounting PASS.
- [ ] P0-B still requires classification of arbitrary software-stalled
  provider calls and permanent refusal. A healthy clock plus a driver
  watchdog is not a general cancellation or forced resource-release proof.

### v1088: true in-flight serialized filesystem lease acceptance (2026-11-03)

- [x] Call the production `fs_provider_reg_call` on a native PDP-6 while
  its provider oracle has a live stack frame and real held lease.
  Nested same-owner recursion and foreign fatal-close contention both
  refuse with -5 without sleeping, unlocking, corrupting the owner
  stack or running a nested provider. The original provider returns an
  error and unlocks/wakes exactly once; a later call succeeds normally.
- [x] Deliberately mutate recursive and fatal foreign refusal into
  sleeping waits; both negative controls are rejected. Existing v1078
  fatal contention and v327 general provider gateway regressions PASS.
  No KCORE or MRES source modifications at v1088; P0-B remains OPEN.

### v1087: P0-A transitive fatal teardown closure (2026-11-03)

- [x] Inventory and validate normal EXIT, fatal requester BRK, APR,
  EXEC and deferred/remote event paths through FILE close, VFS/SYNC,
  HUP, TTY, VM/XIP, PIPE/DPY, swap bitmap, MM and final notification.
- [x] Confirm DTFS/D6FS only CLOSE-SYNC providers and direct bounded
  no-allocation SYNC; fatal private provider wait preserves FILE owner,
  foreign active lease remains P0-B rather than forcibly unlocked.
- [x] Fail-closed transitive source contract with 11 negative mutation
  tests, plus native v870/v904/v912/v957/v905/v922/v995/v1078/
  v1081/v1085 PDP-6 retry/owner acceptance PASS.
- [x] 256K v1077 physical OOM guest with D6FS/MM injections PASS.
  No production code or resident data modification, KCORE 023126
  octal unchanged. P0-A closed; P0-B/D/E/F still open.

### v1086: P0-C slot-0 PI6 liveness closure (2026-11-03)

- [x] Close the HUP-after-idle-switch bug by publishing the existing
  PREVM+FINALIZE owner under global PI disable and before PI6 mask.
  No second HUP, process-private sleeps or FILE provider operations
  may run on the permanent idle stack.
- [x] Route immediate self-fatal delivery through the existing private
  FILE/HUP owner retry, not a direct PI6-masked EXIT handoff.
- [x] Exercise actual MEMFS register dispatch and MM XIP unpin from
  slot 0 while a foreign serialized provider lease is held; assert
  refusal of other provider access without a scheduler wait.
- [x] Native EXIT marker old/new comparison and marked idle finalizer
  with deliberately refusing HUP stub PASS; source contract rejects
  seven deliberately unsafe mutations in the protected graph.
- [x] v1077 full 256K physical OOM D6FS/MM, session teardown and
  process-events full guests PASS; v970/v974/v995 HUP/fatal owner
  and v926/v993 native owner-stage tests PASS.
- [x] Matched KCORE size `023117` -> `023126` octal (+7 code words),
  zero new resident data/MRES slots. P0-C CLOSED; P0-A/B/D/E/F OPEN.

### v1085: deferred unpublished RUN TTY retirement (2026-11-03)

- [x] Move last-session TTY release ahead of VM destruction in the
  unpublished RUN discard path, and propagate MM refusal through the
  existing SIDL/slot-0 retry owner.
- [x] Positive/old-source negative PDP-6 tests prove refused TTY
  retains VM and u-area, later retry frees exactly once; v892/v894
  fixture and native v1081 finalizer checks PASS.
- [x] Full v969 MM-pinned last-session TTY and v1077 256K physical
  OOM guest with combined D6FS SYNC/MM_BUSY refusal and retry PASS.
- [x] KCORE low end +1 instruction word (`023116` -> `023117` octal),
  zero new resident data or reaper state.

### v1084: FIFO OPEN interrupted-rollback ownership (2026-11-03)

- [x] Reproduce a real PDP-6 MM-pin refusal during a named FIFO's
  interrupted provisional OPEN; confirm old code loses its only FD
  retry owner while the MM extent and PIPE reference stay live.
- [x] Make failed rollback commit the existing provisional FD as
  successful OPEN; keep the ordinary interrupted error if rollback
  succeeds. One instruction replacement, no new code/data words.
- [x] Source-link real FILE OPEN/NEWFD/CLOSE and PIPE/MM routines in
  v1084 native positive and pre-fix negative tests. Check unpin/retry,
  normal interruption, reference counts and exact FD publication.
- [x] Full named-FIFO simulator regression PASS; update the obsolete
  word-FIFO-v2 character-I/O expectation to the existing v901 ABI.
- [x] Full word-FIFO-v2 and true anonymous PIPE v902 guests PASS;
  test-daimos-pipe-anon now includes the v902 regression.
- [x] KCC KCORE end unchanged at `023116` octal; no persistent data.
- [x] Transitive HUP and FILE release graph checked in v1087; provider
  in-flight liveness remains independently open as P0-B. Test-only MM
  pins demonstrate fault safety, not ordinary FIFO pin reachability.

### v1083: direct fatal teardown allocation-free leaf audit (2026-11-03)

- [x] Executable source contract for direct FILE/PIPE/DPY/TTY/VM/XIP,
  stable U-area and disk swap release leaves. New allocation in any
  guarded destructor must fail the test rather than pass silently.
- [x] Native v870, v904, v912 and actual C v957 regression PASS.
- [x] No KCORE/MRES, permanent data, ABI or driver change.
- [x] Transitive HUP/fatal event closure audited in v1087. Already
  executing foreign provider lease completion remains P0-B.
- [x] Interrupted FIFO OPEN preserves its provisional FILE descriptor
  on refused last MM free (v1084), with positive and negative PDP-6
  tests; no speculative global garbage collection needed.

### v1082: transactional NEWSESSION TTY release (2026-11-03)

- [x] Preflight last-member canonical input release before rewriting
  pgrp/session/TTY state. Failed MM free preserves the old live owner,
  returns -1 and remains retryable without an extra state word.
- [x] Native PDP-6 v1082 positive/negative regression: pinned refusal,
  successful retry and other-member ownership. Original v1081 path
  correctly fails the negative control.
- [x] Matched full KCC KCORE image: +4 instruction words
  (`023112` -> `023116` octal); zero persistent data words.
- [x] Transitive fatal-close and VM/MM allocation-free graph closed
  in v1087; production OOM still disabled pending P0-B/D/E/F.

### v1081: final-session TTY MM ownership and targeted rollback (2026-11-03)

- [x] Preserve a non-leader's final TTY canonical input on a refused
  physical MM release. Return busy from proc_tty_release_session,
  and retain process VM/u-area via existing PREVM+FINALIZE retry.
- [x] Share the current PREVM VM-failure path instead of adding a
  separate TTY retry stage, persistent owner record or generic
  provider abort. Keep the shared provider lease and fatal contention
  guard.
- [x] Comparable full-image KCC low-end growth: 2 code words
  (`023110` -> `023112` octal); no new permanent data.
- [x] Native v967/v968 physical MM refusal and v1081 staged finalizer
  PASS; existing v881 deferred MM owner test PASS; full 256K v1077
  physical-OOM FILE/MM regression PASS.
- [x] End-to-end transitive no-allocation/owner retry graph closed in
  v1087 for MEMFS-XIP, PIPE, DPY, TTY and MM/VM. P0-B still owns
  active foreign provider quiescence before production OOM kill.
  Actual D6FS/DTFS/TSFS SYNC has no device transfer, so do not add a
  generic close-provider cancellation framework solely for it.

### v1080: DRM236 full-FIFO wait regression (2026-11-03)

- [x] Fix the v1079 loss of conditional branching in
  `drm236_runtime_queue_full`: a third caller must wait for an
  active owner's descriptor, not retry forever while the single
  pending slot is already occupied.
- [x] Native PDP-6 source-linked v1080 test PASS: third request waits
  on active owner, retries and claims engine, waits on its own
  descriptor and returns with correct completion. Original v1079
  source FAILS negative test.
- [x] v1079 native watchdog, online real-SIMH Type-167/236 transfer
  and full 256K physical-OOM FILE/MM regression PASS.
- [x] One-word instruction replacement, with no KCORE/MRES or
  persistent data growth.
- [ ] Close P0-C by proving that slot-0 PI6-masked cleanup never
  enters clock-dependent I/O or a sleeping provider; fix a real
  reachable path rather than implementing a speculative timer.
  Physical APR clock failure is outside scope. Foreign active
  provider quiescence is separately tracked by P0-B.

### v1079: bounded runtime DRM236 device completion (2026-11-03)

- [x] Add a five-second (300 qualified 60-Hz ticks) deadline to DRM236
  runtime PI2 requests; the earlier boot-time polled path already has
  its own bounded deadline. Pack countdown into the LH of the
  existing active request word, preserving RH descriptor ownership.
- [x] Share a single PI2/timeout descriptor completion path; on
  timeout clear the Type-167/236 controllers, publish error, wake the
  original private-stack owner and promote at most one pending request
  with a *new* full watchdog interval.
- [x] Native PDP-6 two-request watchdog and negative-control tests
  PASS. Existing simulator DRM online read/write/queue test PASS
  with its stale read-trampoline test symbol corrected.
- [x] Full KCC/256K boot-image link and physical-OOM requester
  FILE/MM regression PASS with the new clock/DRM code. KCORE +1
  word (`023157` -> `023160`, octal), DRM MRES +17 resident words
  (124 -> 141), no extra persistent data words.
- [x] Independent `ROOT=drum` Type-167/236 Stage1 simulator boot
  with DSK0 unattached PASSes through DRM236 MRES, D6FS, INIT, LOGIN.
- [ ] Still need owner-safe provider quiescence policy for active
  nonreturning calls and permanent refusal (P0-B). PI6 slot-0 proof
  closed v1086 (P0-C) and transitive fatal release graph closed
  v1087 (P0-A). The normal DRM watchdog requires operational APR
  ticks, not recovery from hardware clock failure.

### v1078: foreign provider contention during fatal FILE close (2026-11-03)

- [x] On a foreign-held serialized provider lease, use the existing
  private fatal-close bit to refuse immediately with `-5` instead
  of blocking the dying owner in `proc_wait_event`. Preserve the
  foreign owner's lease, the failed FILE descriptor and the normal
  owner-side retry/backoff.
- [x] Native PDP-6 v1078 verifies fatal fast refusal, free-provider
  dispatch, ordinary wait/revalidation and recursive refusal. Old
  gateway fails negative control; the current gateway and inherited
  provider/rename target and static ownership tests PASS.
- [x] Full-system 256K v1077 provider/MM-faulted requester-only OOM
  regression PASS on v1078 gateway: D6FS 3/2, MM 2/1, independent
  witness and memory accounting intact.
- [x] No new permanent RAM words. Six additional KCORE code words in
  comparable full-profile KCC image (`023151` to `023157` octal).
- [ ] A provider that has *already entered* an unbounded operation
  still has an owner and live stack/IO buffers; no safe force-unlock,
  abort or memory reclamation follows from v1078. Design provider
  quiescence/cancellation only with an explicit completion proof.
  Automatic requester OOM termination remains OFF.

### v1077: combined FILE-provider/MM refusal during physical OOM (2026-11-03)

- [x] Inject two real D6FS SYNC failures and one u-area MM_BUSY in a
  single genuine 256K physical-OOM requester exit, with the production
  FILE, VM and retry paths unchanged. Count exactly 3/2 FILE
  calls/refusals and 2/1 MM free calls/refusals. Full guest PASS.
- [x] Verify the parent reaps only the OOM requester, a blocked PIPE
  witness completes independently, and process memory accounting
  returns to its starting value. `<KWP`, normal HALT; PASS.
- [ ] Prove bounded recovery/quiescence of permanently nonreturning
  providers and allocation-freedom across *all* fatal teardown
  callbacks before activating production OOM termination.

### v1075: full-system requester-only physical OOM (2026-11-03)

- [x] Boot a disposable 256K PDP-6 with only the test copy of
  `PROC_OOM_BRK_ENABLED_V1000` enabled. A valid physical-memory shortage
  terminates only the requesting child with status 1; a PIPE-blocked
  bystander survives, resumes, and exits normally. Memory accounting
  returns to the original process-word and slot count.
- [x] Full-guest default-off negative control: identical BRK returns -1
  to the requesting child rather than killing it. Both behaviors are
  reproduced by versioned `daimos-testkit` tests; no production flag or
  kernel resident data was changed.
- [ ] Prove bounded, allocation-free owner teardown across all FILE,
  session HUP and MM/provider refusal paths, including indefinitely
  stalled providers. Keep automatic OOM killing disabled until safe.

### v1074: BRK rollback ownership under two MM failures (2026-11-03)

- [x] Prevent a silent orphan of the unpublished replacement VM extent
  when both old-extent release and provisional-extent rollback fail.
  The second MM refusal enters the existing resident invariant HALT.
- [x] Preserve the exact old MM failure cause (-4 versus -5) when the
  replacement extent is successfully freed. Native PDP-6 branch tests
  PASS; both pre-fix and missing-guard negative controls FAIL as expected.
- [x] Build full-profile KCC image and GCC KCORE; KCC boots to LOGIN.
  Full native KCC 256K BRK/SBRK growth, heap preservation and shrink
  regression PASS.
- [ ] Continue owner-side FILE/MM/provider teardown and physical-OOM
  process-retirement acceptance. Automatic OOM kill remains disabled.

### v1065: final branch-tip closeout

- [x] Classify the final two divergent KCORE source tips against
  current source, current MRES layout and prior acceptance evidence.
- [x] Preserve original KCORE source tip commits as main ancestors
  without replaying the deferred optimization candidates.
- [x] Recover and test the v101 swap dependency guard (v1063) and
  the FILE fixture extension storage correction (testkit v1064).
- [x] Preserve the matching startup-snapshot testkit tip in main
  ancestry, and remove obsolete branch references after verification.
- [ ] If KCORE work is explicitly resumed, reconsider syscall service
  tail folding, optional MRES placement and snapshot ownership against
  present code and measure **total** permanent RAM, not KCORE alone.
- [ ] Maintain the deferral on KCORE optimization and 32K-specific
  tests until it is explicitly lifted.

### v1063: recover stale swap-source dependency regression

- [x] Restore the historical testkit v101 swap-composite dependency
  check as a versioned v1063 host-only regression.
- [x] Add the four missing outer swap source prerequisites in the
  PDP-6 boot Makefile; test positive and historical negative controls.
- [ ] Classify final two KCORE source tips and the original startup
  test branch, preserving their historical code without running 32K
  tests or restarting the deferred optimization program.

### v1061: reduce archival branch count without losing history

- [x] Copy the four archive-only historical summary files into DAIMOS
  main before merging archive ancestry; keep unaccepted source out.
- [x] Preserve original archive tips as main ancestors, then retire
  standalone branch references after remote verification.
- [ ] Resume the two deferred KCORE branches only on explicit request.
- [ ] Keep native MAKE ALL self-hosting and actual RAM-peak measurement
  open; historical diagnostic logs do not prove those issues fixed.

### v1059: historical shrink and native VM branch resolution

- [x] Verify all five native-KCC VM commits and the compact process
  teardown commit are patch-equivalent to current main; preserve their
  ancestry and retire redundant branch names.
- [x] Preserve three divergent October backup/shrink/KCC diagnostic
  checkpoints in the v1059 archive before deleting old references.
- [ ] Reproduce historical native KCC `MAKE -N ALL` / `MAKE ALL`
  behavior against current MAKE and a fresh supported 256K guest before
  declaring the earlier dependency-recursion failure resolved.
- [ ] Keep further KCORE optimizations and 32K-specific tests deferred.

### v1058: native KCC staging history

- [x] Verify the old minimal KCC native-source converter and Makefile
  are superseded by current staging with ABI/bootstrap packaging.
- [x] Preserve the original historical source commits in main ancestry
  without replaying unrelated old kernel and storage modifications.
- [ ] Native KCC complete self-rebuild remains unaccepted; keep its
  real guest validation separate from packaging checks.
- [ ] Preserve the KCORE optimization and 32K-specific test deferral.

### v1057: early-boot branch consolidation

- [x] Preserve seven early-boot/Stage-1 remote tips and one local
  Stage-1 WIP tip in an immutable-by-convention, versioned Git archive.
- [x] Retire their original refs only after verifying the pushed
  archive contains all eight original commits.
- [x] Archive the obsolete migrated Type-340 C fixture in testkit;
  retain the current native-block tests as the relevant coverage.
- [ ] Review remaining native KCC toolchain and backup/checkpoint
  histories for unique useful work; do not replay old kernel sources.
- [ ] Leave KCORE optimization and 32K-specific tests deferred.

### v1056: TTY and display branch reconciliation

- [x] Verify TTYOUT, privileged TTY routing, Type-340 text, KILL and
  DPYVIEW features already exist in current main; do not replay the
  older kernel implementation.
- [x] Run detached-TTY, native text-block, and DPY interrupt-policy
  regressions; all pass.
- [ ] Retire the obsolete migrated Type-340 C fixture, which still
  includes removed `dpy_text.c`; current native-block test covers the
  current implementation.
- [ ] Continue remaining boot and toolchain branch consolidation;
  KCORE optimization and 32K tests remain deferred.

### v1055: branch reference cleanup after artifact recovery

- [x] Preserve all untracked v115 DTFS diagnostics and v54 nested-UUO
  evidence with SHA-256 manifests in testkit main; clean and detach
  their original worktrees and retire the local branch refs.
- [x] Preserve original remote D6CODEC v053/v054 staging histories in
  the existing experimental archive and retire the remote-only refs.
- [ ] Review surviving boot, toolchain and device branches without
  changing KCORE until its optimization deferral is explicitly lifted.

### v1054: native MAKE per-depth scratch recovery

- [x] Recover the historical native MAKE lazy recursion scratch scheme
  in current userland source, without adding the obsolete C `memcpy`.
- [x] Build native KCC MAKE and 256K full-profile guest; pass six-level
  dependency traversal and cycle rejection, with a baseline control.
- [x] Provide a versioned opt-in 256K testkit target with source guard.
- [ ] Measure actual peak native MAKE process memory at shallow and
  maximum recursion depths, and complete a full self-host `MAKE ALL`.
- [ ] Continue historical branch consolidation; KCORE optimizations
  and 32K-specific acceptance remain deferred.

### v1050: legacy source and test branch consolidation

- [x] Integrate SIMH interactive PC/breakpoint/step regression v1048.
- [x] Preserve divergent historical DSH and INIT source commits in
  archive v1050, and related obsolete testkit commits in archive v1049.
- [x] Remove original safe local/remote branch names after verifying
  ancestry, without enabling unimplemented EDCORE or supervisor paths.
- [ ] Reconcile the untracked DTFS diagnostic files in the old v115
  SIMH testkit worktree before deleting its attached local branch.
- [ ] Continue reviewing remaining non-KCORE experimental source branches.
- [ ] Leave KCORE optimizations and all 32K tests deferred.

### v1047: userland branch consolidation, KCORE deferred

- [x] Compare historical two-word DSH folding against current source
  and identical KCC objects; reject the optimization after invalid UUOs
  in native PDP-6 APROPOS, despite 28 linked words saved.
- [ ] Investigate the two-word `p10fold`/`DLINK` fold-plan failure as a
  distinct toolchain correctness defect before enabling shorter folds.
- [x] Retire historical AAP TSFS 64K testkit branch after verifying the
  exact script and target already exist in testkit main.
- [ ] Continue non-KCORE userland/simulator branch consolidation.
- [ ] Resume KCORE optimization only when the deferral is lifted.

### v1042: consolidation priority; KCORE optimizations on hold

- [x] Stop further KCORE source optimization at the user's request;
  preserve the current syscall-folding and selective-MRES candidates.
- [x] Recover and verify the historical D6FS host-checker owner-width
  test against source, fixing independent `pdp10-tools` source v1040.
- [x] Classify and retire the incompatible MM packed-descriptor
  experiment; retain its source commit in testkit main ancestry.
- [x] Restore optional PTR/TSFS evidence validation in testkit v1042;
  do not count it as a new boot acceptance run.
- [ ] Continue semantic consolidation of remaining non-KCORE DSH,
  supervisor, simulator, and test-infrastructure branches.
- [ ] Resume KCORE optimization only after the deferral is lifted.

### v1000: gated requester OOM dispatcher (2026-11-02)

- [x] Default-off BRK dispatch checks AC1 failure and AC2 MM_ERR_NOMEM;
  isolated PDP-6 assembler control-flow test PASS.
- [ ] Genuine physical shortage, requester exit, full resource release,
  and bystander survival still require booted acceptance before activation.

### v998: BRK physical-OOM classification (2026-11-02)

- [x] Booted v985 resource-pressure regression PASS after v997; native
  v998 confirms BRK combines invalid/limit/allocator failures into -1.
- [x] v999 preserves the MM failure cause in scratch AC2, distinguishing
  physical shortage (-1) from fragmentation, descriptors, invalid, and busy;
  booted BRK growth/data/shrink PASS without new resident state.
- [ ] Isolated requester-only physical-OOM exit and reclamation acceptance
  remains within the existing OOM P0 gates. Production OOM kill stays OFF.

### v995: fatal owner HUP stage (2026-11-02)

- [x] APR/EXEC fatal owner performs HUP before leaving private kernel
  stack, reusing existing FILE retry/backoff. Native v970 and injected
  v995 FILE/HUP refusal tests PASS; no new resident state.
- [ ] VM destruction failure after stack switch remains the existing OOM
  termination blocker. v997 now implements explicit PREVM slot-zero
  retry without new resident data; injected VM+MM failures and the existing
  v978 full-system regression PASS. Automatic OOM kill stays OFF pending
  physical exhaustion termination testing.

### v991: owner-side EXIT HUP stage (2026-11-02)

- [x] Stage ordinary EXIT session HUP on the live private owner stack,
  after FILE closure and before slot-zero switch; errors retain ownership.
  Booted 256K BRK runtime PASS, v991 source guard PASS.
- [ ] Booted fault-injected HUP failure/retry and actual OOM exit still
  part of the two existing OOM P0 gates. v905 standalone HUP harness
  now links and passes with its test-only TTY reset stub (v992);
  its negative-control variant fails as expected. Booted v991 HUP
  preflight refusal remains untested. Booted v907 partial-HUP physical-MM
  fault injection PASS on current v991 kernel; this confirms retry and
  TTY ownership but not direct preflight fault injection.
  v993 native PDP-6 execution of the actual v991 EXIT dispatch with
  injected HUP rejection and successful second attempt PASS. Full-system
  preflight fault-injection acceptance remains outstanding.
  VM detach audits v957 (C resident/disk) and v880 (native failure/retry)
  PASS; owner EXIT handoff on VM refusal is the remaining implementation
  blocker, not another independent P0.

### v987: permanent-provider OOM containment audit (2026-11-02)

- [x] Source-linked provider-lease audit confirms that only normal
  provider return unlocks; waiters revalidate and fatal FILE retry does
  not force unlock or free an owned MM extent. Testkit v987 PASS.
- [x] Remove stale v863 cross-process victim-selector reference from the
  permanent provider failure policy; v981 uses requester-only OOM policy.
- [ ] Provider cancellation or quiescence remains unimplemented and
  unproven. A permanently nonreturning provider cannot safely be reset
  or have its owner freed; OOM termination remains OFF.

### v986: owner fatal teardown OOM safety audit (2026-11-02)

- [x] Native v926 owner-return deferred TERM with real extracted FILE
  close retry, sleep and final exit PASS on current PDP-6 source.
- [x] Source-linked v986 audit proves owner-only FILE retry and capped
  backoff interval (01000 ticks), no forced MM release during refusal,
  and no restored cross-process victim selection.
- [ ] Repeated retries are not bounded in *count*. A permanently refusing
  provider can still retain the owner, FILE references and its u-area.
  Complete provider recovery/quarantine (P0-B); transitive release
  allocation-freedom was closed separately by v1087 (P0-A). Keep
  automatic OOM disabled pending remaining gates.

### v985: physical OOM regression on clean RUN fixture (2026-11-02)

- [x] Booted 256K PDP-6 isolated clean v197-based guest with v198
  physical MM pressure assertions: four valid 18-bit BRK requests
  fail under pressure, caller break/VM/data stay unchanged, RUN
  rollback and subsequent recovery PASS. No unrelated process kill.
- [x] Preserve locally modified v197 source; v985 uses an independent
  committed testkit copy of the previously committed v197 fixture.
- [ ] v984 gradual-growth rejection has no independent physical-failure
  witness; v985 supplies the separate physical-pressure coverage, but
  does not establish why v984 individually rejected growth.
- [ ] Allocation-free and bounded fatal FILE/TTY/VM/provider teardown
  remains unverified: automatic OOM termination stays disabled.
  v988 specifically identifies the pre-VM owner-exit failure path: after
  switching to idle stack, recoverable HUP/VM refusal currently leads to
  HALT 1; post-VM MM refusal already has a deferred slot-zero retry.
  Booted v978 transient FILE/MM retries PASS.
  v989 confirms slot-zero retry is post-VM only and cannot safely
  execute sleeping HUP/provider cleanup while scheduler PI6 is inhibited.
  v990 owner-phase source check PASS: ordinary EXIT closes FILE on its
  private stack, but HUP/VM still run after idle-stack switch. No new P0.

### v983-v984: booted BRK progression and growth rejection (2026-11-02)

- [x] Repair testkit BRK guest's missing libc memset linkage; full
  256K PDP-6 runtime BRK/SBRK growth, heap contents and shrink PASS.
- [x] v984 full-system BRK repeated-growth test reaches rejection,
  proves failed request leaves BRK unchanged, restores original break
  and exits normally. No automatic OOM termination or unrelated kill.
- [ ] Attribute v984 rejection to physical RAM/backing exhaustion versus
  address-space ceiling with an explicit MM witness before certifying
  the exhaustion gate. Follow with real RUN failure/rollback in a clean
  fixture and allocation-free fatal cleanup certification.

### v982: faulting requester RUN and BRK audit (2026-11-02)

- [x] Deleted five obsolete source-linked test pairs for the removed
  cross-process OOM selector; retain provider/teardown safety audits.
- [x] BRK fallback uses the current process's allocation owner, tries
  mm_alloc_aligned, and returns an error on failure. Source guard PASS.
- [x] Failed SYS_RUN MM allocation discards its unpublished claimed child
  through proc_slot_discard and returns error to the parent. Killing the
  parent for child-admission failure is not authorized. Source guard PASS.
- [ ] Neither source guard is a full exhausted-RAM guest regression; add
  booted negative tests before any fatal OOM action is connected.
- [ ] Owner-side fatal FILE/VM/provider teardown remains unproven
  allocation-free and bounded: automatic OOM termination stays OFF.

### v981: faulting-process OOM policy (OpenBSD-style) (2026-11-02)

- [x] Choose the process requesting the failed user allocation, never
  another process scored by memory footprint. Keep existing normal
  compaction, reclaim and swap attempts before declaring true exhaustion.
- [x] Delete the uncalled largest-extent OOM victim scanner from KCORE;
  do not replace it with another scan or duplicate C implementation.
- [ ] Actual faulting-process termination remains OFF until its owner-side
  FILE/TTY/VM/provider teardown is allocation-free, bounded and safe;
  today unrecoverable failures still return the normal allocation error.
- [ ] Update historical selection tests in daimos-testkit and prove
  no third-party victim is killed. Test RUN and live BRK failures.

### v980: delete obsolete whole-file C duplicates (2026-11-02)

- [x] Delete unused `system/kernel/mm/mm.c`, `system/kernel/fs/memfs.c`,
  and `system/kernel/fs/d6fs_provider.c`: their production PDP-6 routines
  are owned by `mm_pdp6.s`, `memfs_runtime.s`, and the D6FS namespace
  assembly. Remove stale dependency-only Makefile entries.
- [x] Delete the disabled DTFS resize C algorithm from `dtfs.c` and remove
  `DTFS_EXTERNAL_RESIZE`; retain the active distinct DTFS C services and
  the authoritative `dtfs_resize.s` implementation.
- [x] Full 256K PDP-6 image builds after deleting obsolete C files;
  separately compile the DTFS translation unit after resize cleanup.
- [x] v978 mixed fatal FILE/MM retry full-system regression PASS after v979.
- [ ] Finish v980 post-deletion booted regressions and clean up remaining
  references in historical source-contract tests. Do not weaken unrelated
  assertions to hide existing problems.

### v979: delete duplicate C/assembly kernel implementations (2026-11-02)

- [x] Delete unused guarded C alternative implementations in proc.c,
  vm_pdp6_swap.c, and file.c. Preserve distinct shared C services.
- [x] Delete obsolete *_PDP6_ASM compile flags from kernel Makefile.
- [x] Document single-authoritative-implementation rule in
  design/assembler-ownership-v979.md; never add equivalent C fallback
  algorithms alongside assembly implementations.
- [x] Full 256K PDP-6 image built and linked; native v971/v974 tests PASS;
  18-second headless simulator smoke reaches DAIMON V0.5 and loaded
  MEMFS/D6FS. This smoke is not the full booted test suite.
- [x] Source diff deletes 961 lines from four kernel files; these guarded
  bodies were already excluded from current KCORE, so no size savings
  from deletion alone are claimed.
- [ ] Re-run broader booted regression suite after cleanup.

### v979: process-group sender self-membership and C build audit (2026-11-02)

- [x] Booted 256K PDP-6 sender self-membership: a child sends HUP to
  its own group and the parent observes the correct fatal WAIT exit.
  Existing PID/group, TSTP/CONT, CHLD and ALRM event regression PASS.
- [x] Both `system/kernel/Makefile` and PDP-6 boot image build select
  `PROC_EVENT_APPLY_PDP6_ASM=1` plus `proc_pdp6.s`; portable C
  `proc_event_apply` is excluded from current PDP-6 production builds.
  Do not alter C semantics speculatively; require a supported C-port
  configuration and source-linked tests before porting owner deferral.
- [ ] Final regression and documentation review remain; KCORE shrinking
  and automatic OOM killing still deferred.

### v978: booted mixed fatal FILE/MM retry (2026-11-02)

- [x] Full-system 256K PDP-6 repeated TERM followed by HUP on the
  WAIT_CHILD parent, using the v939 real FILE-close refusal and MM-free
  failure hooks. Selected exit status HUP, FILE close=3/refusals=2,
  MM free=2/refusals=1, VM destroy=1, CHLD=1, EXIT=1, finalizer=1,
  finalizer failures=0, bystander=36, FS lease unchanged. PASS.
- [x] Superseded by v979: remove duplicate C `proc_event_apply` entirely;
  never add equivalent fallback algorithms alongside PDP-6 assembly.
- [ ] Group sender self-membership and full regression gate remain.

### v977: independent booted fatal-event slot reuse (2026-11-02)

- [x] Isolated v976 failure: INIT printed R after its first WAIT but not
  U after second RUN. The inherited held-provider test deliberately
  leaves the FS gateway lease occupied indefinitely, so loading a new
  `/SYSTEM/INIT` cannot complete. This was a test fixture deadlock,
  not evidence of stale event bits or incorrect CHLD restoration.
- [x] Independent v977 booted 256K PDP-6 regression without an occupied
  provider: TERM and reap slot 2, recreate process in slot 2, verify
  no pending INT/TERM/HUP/PIPE through GETEVENTS, then complete the
  existing PGRP/session/domain lifetime tests. PASS. Test launch uses
  text-only consoles to avoid SDL crashing in the test environment.
- [ ] Group delivery including sender self-membership, booted mixed
  FILE/provider refusal/retry, and portable C fallback remain.

### v975-v976: process-group delivery and slot reuse (2026-11-02)

- [x] v975 booted 256K PDP-6 group HUP delivery to the WAIT_CHILD
  parent after repeated PID TERM: selected HUP exit, CHLD=1, EXIT=1,
  finalization=1, failure=0, 36 bystander cycles, unchanged held FS
  provider lease 4,,0. PASS, no production change.
- [x] Source audit: `proc_run.s` zeroes the complete 0410-octal-word
  freshly allocated u-area before publishing a RUN process. This clears
  all deferred fatal bits at PROC_FDCTL_OFFSET for slot reuse.
- [ ] v976 guest-visible slot-reuse and clean pending-event test FAILS
  to complete: both the initial in-descendant GETEVENTS probe and the
  isolated new-process probe timed out in disposable 256K builds after
  original descendant marker Z, with INIT never reaching final P. The
  unchanged v975 group-delivery fixture passes. Do not infer stale
  pending state or change production code until the altered fixture's
  CHLD/finalization/scheduler stages are independently witnessed.
- [ ] v976 reduced reproduction (2026-11-02): removing the additional
  GETEVENTS and mode-N code, leaving only a post-WAIT recycled-slot
  spawn, still timed out after the first descendant's Z. Guest never
  reached P despite CHLD=1, owner checks=3, EXIT=1, finalizer=1,
  failure=0, and intact provider lease. A new R checkpoint immediately
  after WAIT was added to the preserved v976 testkit fixture but has
  not yet run. Investigate CHLD-to-WAIT resume rather than guessing
  that the u-area contains stale pending bits.
- [ ] Full group delivery including sender self-membership, provider
  failure during mixed-event cleanup, and portable C behavior remain.

### v973-v974: owner self-delivery precedence (2026-11-02)

- [x] Fix native PDP-6 `proc_event_apply` current-process fatal path:
  if a fatal event is already pending, self-delivery records/coalesces
  the new event instead of closing FILEs and overriding the queued
  higher-priority exit status. The owner-return gateway still selects
  the final status and performs private, retryable teardown.
- [x] v973 native owner test: pending TERM then self HUP, no premature
  FILE close, selected HUP, two FILE retries and one owner completion.
- [x] v974 native owner test: pending HUP then self TERM must retain HUP;
  both tests pass alongside v926, v970 and v971 regressions.
- [ ] Booted self/group event coalescing, slot/PID reuse, and mixed
  provider-origin failure tests remain before full feature closure.

### v971-v972: fatal-event precedence regression and booted acceptance (2026-11-02)

- [x] Adapt v925/v926 extracted PDP-6 safety fixtures to deferred HUP:
  protected CHILD/INTR/CTX_PC continuations record HUP without closing
  resources; clear the fixture's preexisting PIPE bit before testing the
  independent immediate-fatal path. Fatal, finish, OOM, deferred delivery
  and owner-return regression tests PASS.
- [x] v971 native owner-side priority matrix PASS: HUP over INT/TERM/PIPE,
  INT over TERM, TERM over PIPE, and PIPE alone, including private FILE
  close refusal/retry and consumption of all pending bits.
- [x] v972 full-system 256K PDP-6 held-provider CHLD wakeup PASS with
  repeated TERM followed by HUP: correct HUP WAIT status, CHLD=1,
  EXIT=1, finalizer=1, failure=0, unchanged lease 4,,0, 36 bystander
  cycles and zero errors. First fixture accidentally inserted sends
  into descendant rather than INIT; the corrected fixture passed.
- [ ] Group/self delivery, PID reuse, failure under the mixed-event
  path and cross-architecture C fallback behavior remain open.

### v970: four-fatal-event deferred precedence (2026-11-02)

- [x] PDP-6 foreign protected continuation now accepts/coalesces INT,
  TERM, HUP and PIPE without remote FILE/VM release. Native v970
  source-linked delivery test PASS.
- [x] Owner chooses HUP > INT > TERM > PIPE, claims private fatal-close
  ownership before clearing the pending bits, and uses the existing
  retry/EXIT path. Native HUP owner test with two FILE refusals PASS.
- [ ] Booted mixed-event CHLD wakeup with exact exit status, repeated
  sends, PID reuse, self/group delivery and FILE/MM teardown checks.
  Keep automatic OOM killing OFF; KCORE shrink remains deferred.

### v926: deferred fatal delivery across saved executive handoff (2026-11-01)

- [x] v969 P0 booted PDP-6 final-session TTY/MM fault acceptance:
  `proc_session_teardown_commit_v875` prematurely cleared the owning
  TTY record before `proc_tty_release_session` could attempt its MM
  release. The later v967 guard therefore could not preserve ownership.
  Production session teardown now calls the existing checked canonical
  line release before clearing the TTY record and returns failure if
  MM refuses. Disposable native 256K boot pins the actual MM line at
  the final EXIT handoff: first finalizer refusal preserves session and
  extent, real unpin permits retry, second finalizer releases both.
  Witnesses: one pin, one retained session, one completed retry, zero
  errors. No static RAM or process table growth.
- [ ] Continue proving the OOM victim-retirement dependency graph with
  simultaneous FILE/provider and MM failures; automatic killing OFF.
- [x] v968 native PDP-6 tests the v967 session-retirement code against
  the actual C canonical-line reset and MM assembly: a pinned TTY line
  remains allocated and retains session 5; another session does not
  release it. After real `mm_unpin`, the original owner retries and
  releases both extent and session. Wrong MM owner similarly preserves
  both until the extent owner is corrected. PASS, no production change.
- [ ] Booted last-session EXIT with physically pinned TTY canonical
  input remains outstanding; v968 is actual native MM/TTY code but a
  constructed process/session fixture, not a complete guest EXIT.
- [x] v967 P0 TTY session retirement ownership fix: the last-member
  `proc_tty_release_session` used to drop the owning session after
  `proc_tty_line_reset` even when pinned/wrong-owner MM prevented the
  canonical input extent from being freed. Recheck its base after
  reset, and retain session ownership until the extent is genuinely
  gone. Native PDP-6 failed-release then successful-retry test PASS;
  no static RAM or new process state. Earlier v897 attach/detach checks
  already reject inherited stale input on new-session attach.
- [ ] Follow up with a booted session-owner exit and MM-pinned TTY
  line fixture; a persistent MM refusal can retain old TTY ownership
  indefinitely and must not be hidden by automatic OOM killing.
- [x] v966 native PDP-6 fifth mount-slot bit lifetime acceptance:
  a resident and then swapped PURE owner at public mount 17 blocks
  mount 17, but not mount 1 (identical low four slot bits); a non-PURE
  resident process does not pin the mount. Actual
  `proc_swap_mount_busy` extracted from production assembly PASS.
  No production code, data or permanent RAM change.
- [ ] Full-system non-root PURE mount refusal still requires a guest
  mount/unmount transaction where administrative/root policy is not
  independently responsible for rejection. Native v966 validates the
  critical high slot-bit ownership distinction without that ambiguity.
- [x] v965 booted 256K PDP-6 PURE DXR2 guest exercises actual VFS
  syscalls: root unmount returns -1, `/SYSTEM/INIT` opens writable
  but `write_words` returns -1 while PURE child processes execute;
  the guest prints its completion witness and normal TERM/CHLD cleanup
  finishes (VM/u-area once, 44 bystander cycles, no integrity errors).
  The test includes a D6FS provider retry (one rejected close of two),
  and checks the write denial beyond the open boundary. PASS.
  The root unmount refusal is not solely attributable to PURE lifetime;
  v961 native guards prove that specific ownership predicate.
- [ ] Add a separate booted non-root mount lifetime test if isolation
  of the PURE mount-busy cause is required; avoid DTFS-specific work.
- [x] v964 booted 256K PDP-6 with actual user DXR2 linked `--pure`:
  RUN/TERM and D6FS close-retry acceptance PASS (three sync calls,
  two injected refusals, VM and u-area released once, CHLD once,
  44 independent scheduler cycles and zero integrity errors). This
  checks real PURE executable load/scheduling/teardown compatibility
  with v961/v962 guards, not a live guest unmount/write denial.
- [ ] Complete the guest-level PURE ownership test by attempting
  `vfs_unmount` and backing writes from a second process while the
  executing PURE process remains resident; native v961/v962 tests
  verify the denial leaves but not their full syscall chain.
- [x] v963 VFS/EXEC PURE backing source-contract regression checks
  `file_lookup_path` -> stat/access/read -> `proc_swap_attach`, live
  mount-root provider validation, unmount's `proc_swap_mount_busy` and
  writes' `proc_swap_backing_busy` dispatch guards. Native v961 attach,
  resident/swapped mount and write lifetime PASS. Full-system booted
  256K PDP-6 v948 after v961/v962 fixes PASS (D6FS SYNC twice rejected,
  third succeeds, VM and u-area each released once, 45 independent
  scheduler cycles, no integrity errors). No production change in v963.
- [ ] A booted, real PURE executable must attempt unmount and backing
  write while resident; the v963 source guard plus v948 boot regression
  does not constitute that exact system-level operation test.
- [x] v961/v962 resident PURE backing ownership fixes. Both
  `proc_swap_mount_busy` (VFS unmount guard) and
  `proc_swap_backing_busy` (VFS write guard) previously inspected only
  the u-area's saved backing word, which is populated after swap-out.
  Resident PURE processes keep their executable backing instead in
  `proc_swap_records[slot]`. Select by nonzero resident VM base, retain
  the previous u-area path for swapped images; both native PDP-6
  negative controls failed before the fix and PASS afterwards.
  Non-PURE and unrelated mount/index controls PASS. No new static RAM.
- [ ] Verify the executable vnode's mount/provider association across
  lookup and EXEC publication in a booted PDP-6 test; the corrected
  mount and write guards now retain resident PURE backing ownership.
- [x] v961 resident executable backing now validates its exact live
  VFS mount-root provider and public mount ID before publishing the
  one-word swap record or staged mount-extension bit. A syntactically
  valid but mismatched or unmounted vnode fails closed. v961 native
  mount-binding matrix, adapted v960 matrix and v959 contract PASS;
  PDP-10 C89 cross compilation PASS. No new permanent RAM words.
- [ ] Booted PDP-6 check of vnode/mount-root disagreement and complete
  RUN/EXEC rollback is still required before claiming full-system
  acceptance. DTFS-specific tests remain deferred.
- [x] v960 host C89 executable-backed resident-record test links the
  actual `proc_swap_attach`: all mount IDs 1..26 preserve exact index,
  low mount bits, PURE text and staged fifth mount bit. Out-of-range
  mount, provider, kind and text size reject without altering record;
  an owner without a u-area must be SIDL. v959/v957 controls PASS.
- [ ] Determine whether the EXEC loader's validated vnode guarantees
  provider identity agrees with its live VFS mount root at attach time.
  `proc_swap_attach` validates vnode fields/range but does not itself
  compare the provider against the mounted root. Do not add costly
  duplicate checks without auditing the caller's binding contract.
- [x] v959 source-contract audit of live PDP-6 one-word swap record:
  resident entry is executable index/mount/PURE metadata, while disk
  entry is first-block/count with the old record in the stable u-area.
  Checked attach, prefree backing validation in detach, swap-out MM
  release before disk publication, swap-in reload before resident
  publication, and v958 rollback exclusion. Source gate PASS; native
  v788/v806/v958 and v957 C branch controls PASS. No production change.
- [ ] A preexisting corrupted resident backing record is not reliably
  recognizable from its raw word alone, because resident and disk
  layouts overlap. Do not guess a disk span from the word bits; audit
  metadata creation and contextual mount/provider validation instead.
- [x] v958 fix real PDP-6 swap-in rollback ownership bug: the read-
  failure `mm_unpin` and both post-allocation `mm_free` paths previously
  ignored MM errors and cleared TRANSITION. Failure now enters the
  existing fatal storage-invariant handler before clearing ownership.
  Native three-case MM-refusal fixture PASS; v667, v788, v806, v880
  and v957 rollback/publication regressions PASS. No persistent RAM.
- [ ] Continue swap transaction preflight/rollback review, including
  verification of any preexisting malformed resident swap record.
- [x] v957 actual C `proc_swap_detach` resident-vs-disk branch:
  nonzero resident VM base bypasses fallible disk-backing validation;
  disk-backed image with zero VM base rejects unallocated backing
  before clearing its swap record. v880 native VM destroy and v880
  C detach regressions PASS; v955/v956 MM ownership regressions PASS.
  The suspected resident-MM-free then fallible disk-detach double-free
  is not reachable through these validated production branches.
- [ ] Audit creation and publication of resident swap records so that
  malformed nonzero disk spans cannot be silently ignored by the
  resident detach branch; v957 does not establish corruption recovery.
- [x] v956 native PDP-6 u-area release ownership acceptance: the real
  `proc_uarea_release` and `mm_free` leaves the process meta word and
  extent untouched on wrong owner, wrong MM type and pinned-extent
  refusals. A subsequent valid release clears both the stable u-area
  flag and stored base, and another release is a no-op. PASS; no
  production code or permanent RAM change.
- [ ] Audit resident VM destruction and `proc_swap_detach` ordering for
  any post-MM-free error that might leave a stale VM base on retry.
- [x] v955 native PDP-6 MM failure matrix: three-pin type-3 u-area
  reports the exact pin count, `mm_free` rejects it with -5 and does
  not modify extent metadata; wrong type and wrong owner return -4
  without mutation; normal release after unpin succeeds, and the
  missing extent is rejected. Existing v888 release diagnostics remain
  covered. No allocator code or RAM growth; PASS.
- [ ] Permanent missing, wrong-type or wrong-owner extents remain
  unreclaimable without reconstructing correct ownership. Do not
  force-free them or interpret -4 as an ordinary retryable pin.
- [x] v954 provider-lock/quiescence source-contract test and native
  v866/v915/v916/v917 regressions PASS. Lock owner is encoded as
  owner-slot+1 in LH, RH=0; waiters sleep on a distinct event and must
  revalidate after wake. Only the dispatch-return unlock path releases
  the lease. OOM excludes known owner and rejects all victims with an
  unknown held-lease owner; private fatal retries never force-unlock.
  No production changes or extra KCORE words.
- [ ] No safe forced recovery of a nonreturning provider is demonstrated.
  The provider may retain a live continuation, private stack, shared
  workspace and MM/FILE references. Administrative action must not
  clear the lease, fake wakeup, recycle the PID or free those resources
  without a provider-specific quiescence and completion proof.
- [x] v953 booted 256K PDP-6 provider revival after persistent
  refusal: a genuine user APR fatal victim holds its private FILE
  descriptor through eleven failed VFS sync calls and eleven sleeps;
  the capped 01000-octal tick backoff is reached. The twelfth call
  succeeds, FILE descriptor and failure witness clear, owner EXIT
  executes once, and independent user spinner continues: PASS.
  The disposable provider refusal is test-only; no production changes.
- [ ] An indefinitely nonreturning or permanently rejecting provider
  still cannot be forcibly reclaimed without risking live references.
  Administrative recovery must operate at the owning subsystem and
  prove outstanding references quiesced first; no blind forced unlock.
- [x] v951 booted 256K PDP-6 persistent D6FS SYNC diagnostics:
  three failed provider calls retain FILE-stage GETTEARDOWN with actual
  -1 error, no premature VM/u-area release, 33 bystander cycles. PASS.
- [x] v952 correct syscall.h FILE-count decoder: the count occupies
  bits 2..6 (maximum 16 FILEs); previous 0377 mask overlapped FD index
  bits 8..9 and could misreport 193 descriptors for one live FD.
  Native C89 exhaustive count/FD/error matrix PASS, zero kernel RAM
  or on-wire diagnostic format change.
- [ ] Permanent provider recovery/quarantine remains a policy and
  implementation gate; diagnostic visibility alone is not recovery.
- [x] v950 booted 256K PDP-6 bounded persistent D6FS provider SYNC
  failure: disposable D6FS provider refuses three closes and HALTs
  on attempt four. The deferred TERM target retains its VM and u-area
  (both release counters zero); descendant CHLD=1, 33 independent
  bystander cycles, and provider lease free. No forced reclamation.
- [ ] An indefinitely rejecting D6FS provider still has no safe
  automatic reclamation policy. Administrative quarantine/recovery and
  OOM termination safety remain open. DTFS tests remain deferred.
- [x] v949 full-system 256K PDP-6 combined D6FS/MM teardown refusal:
  two failures returned by disposable `d6fs_provider_sync` precede one
  test-only type-3 u-area `mm_free` busy result. D6FS FD closes=3
  (2 rejected), MM frees=2 (1 rejected), VM destroy=1, u-area release=2,
  CHLD=1, finalizer failures=0, 45 bystander cycles and successful
  INIT TERM reap: PASS. Production cleanup path unchanged.
- [ ] DTFS-specific provider failure testing is DEFERRED until the DTFS
  rewrite; do not block this P0 termination/OOM work on the old DTFS.
- [x] v949 booted 256K PDP-6 combined-stage teardown: D6FS provider
  SYNC rejects two genuine close calls on a held file, succeeds on the
  third; subsequent owner u-area `mm_free` returns busy once and
  succeeds on retry. FILE calls=3, provider rejects=2, MM calls=2,
  MM rejects=1, VM destroy=1, u-area release=2, CHLD=1,
  finalizer errors=0, 45 independent scheduler cycles, INIT reap PASS.
  This is sequential cross-stage fault acceptance, not a guarantee for
  all simultaneous provider failures or permanent-resource deadlock.
- [x] v948 full-system 256K PDP-6 D6FS provider-origin SYNC refusal:
  a real D6FS-root executable FD is held by the deferred-TERM owner;
  disposable `d6fs_provider_sync` rejects twice, then returns success.
  Production VFS/FILE close retries 3 times with 2 errors; VM and
  u-area release once each, CHLD=1, finalizer errors=0, 45 spinner
  cycles, free final provider lease and correct INIT TERM reap: PASS.
  This is a fault inside the real provider routine, not a naturally
  occurring disk error; DTFS and persistent physical errors remain.
- [x] v947 native post-commit event guard: after irreversible VM_NONE /
  FINALIZE state with deferred u-area release, HUP, INT, TERM and PIPE
  are all rejected. FILE, VM and session HUP counters do not increase;
  slot-0 reaper still completes u-area release, and TERM after final
  commit also refuses. The original v942 lifecycle remains PASS.
- [ ] Booted full-system post-commit fatal race and independent progress
  acceptance; native v947 does not model asynchronous timer/PI races.
- [x] v945/v946 native mixed-state group TERM dispatch: the dispatcher
  visits eligible PGRP members in slot order and stops on first failed
  `proc_event_apply`. If a protected WAIT_CHILD member accepts deferred
  TERM before a TRANSITION member refuses, the request returns -1 but
  the first member retains TERM (v945). If a TRANSITION member refuses
  first, a later eligible member remains untouched (v946). Both PASS.
- [ ] Specify the process-group API contract: current implementation is
  partial, slot-ordered, stop-on-error, not atomic. Do not add rollback
  or prevalidation without establishing required behavior and RAM cost.
- [x] v944 native process-group TERM acceptance: actual `proc_event_send`
  group dispatcher matches PGRP 7 to a WAIT_CHILD member in slot 2,
  records owner-side deferred TERM without FILE/VM teardown, and retains
  v943 reverse-order and v942 duplicate/competing fatal safeguards: PASS.
- [ ] Mixed-state process group with multiple members: group delivery
  applies members sequentially and stops on first refusal, so partial
  delivery semantics must be specified and tested before OOM enablement.
- [x] v943 native fatal-event ordering: INT and HUP arriving first at a
  WAIT_CHILD target are rejected, with no FILE/VM cleanup and without
  setting deferred TERM; subsequent TERM is accepted, duplicate TERM
  coalesces, and competing later INT/HUP are refused. Native PASS.
- [ ] Verify process-group event delivery and fatal events following
  committed teardown; OOM termination remains disabled.
- [x] v942 native fatal precedence/coalescing control: a repeated TERM
  on an existing deferred TERM owner succeeds without FILE/VM cleanup;
  INT and HUP targeting the same protected private stack are refused.
  The original v926 foreign-context fatal lifecycle also passes.
- [ ] Verify race ordering when INT/HUP precedes TERM, simultaneous
  process-group delivery, and actual post-commit fatal/retry status.
- [x] v941 bounded persistent FILE refusal in 256K booted PDP-6:
  four rejected closes of a live descriptor, no VM destroy or u-area
  release for the TERM target, no descriptor integrity error, 25 peer
  scheduler cycles, held FS lease still 4,,0. A controlled test HALT
  at refusal four permits full counter inspection. Descendant EXIT and
  CHLD occur normally; the TERM target itself remains unfinalized.
- [ ] Assess policy for an indefinitely rejecting provider; no automatic
  resource reclamation or OOM kill is safe on the blocked owner stack.
- [x] v940 booted 256K PDP-6 MM pinned-extent busy path: disposable
  injection sets an actual extent pin bit at the matching slot-5 type-3
  u-area `mm_free` validation; the allocator's own busy branch rejects
  release, and a second attempt clears the test pin and frees normally.
  MM calls=2, busy=1, u-area calls=2, VM destroy=1, FILE close=3 with
  two refusals, CHLD=1, finalizer failures=0, 36 bystander cycles,
  intact FS lease 4,,0, correct INIT TERM reap: PASS.
- [ ] Natural concurrent pin ownership and permanent refusal tests;
  mounted DTFS/D6FS provider-origin sync failure, fatal-event precedence,
  and OOM safety remain open.
- [x] v939 256K PDP-6 `mm_free` entry-fault regression: slot-5
  kernel-dynamic u-area free refused once with allocator busy result,
  followed by successful second call. MM calls=2, u-area releases=2,
  VM destroys=1, FILE closes=3 (2 refused), CHLD=1, finalizer failures=0,
  37 bystander cycles, held provider lease 4,,0, correct TERM WAIT: PASS.
- [ ] Test internally generated MM errors (busy/pinned extent) rather
  than entry injection; mounted DTFS/D6FS VFS_SYNC provider-origin refusal,
  permanent refusal policy, fatal-event precedence, and OOM safety remain.
- [x] v938 booted 256K PDP-6 post-VM u-area release retry: test-only
  `proc_uarea_release` first attempt returns -1, deferred slot-0
  finalization retries and succeeds. U-area release calls=2, VM destroy
  calls=1, live FILE entry close calls=3 with two refusals, CHLD=1,
  zero finalizer failures and successful INIT TERM reap. Held provider
  owner/waiter lease stays 4,,0, 36 independent bystander cycles.
- [ ] Inject a real MM `mm_free` failure at u-area release and verify
  post-VM retry, then test provider-origin VFS_SYNC rejection on mounted
  DTFS/D6FS media, permanent refusal, and other fatal events.
- [x] v937 booted 256K PDP-6 exactly-once teardown under two real FILE
  descriptor-close refusals: VM destruction called once, u-area release
  called once, descriptor close attempted 3 times (2 refusals), CHLD=1,
  finalization failure=0, 36 bystander cycles, held lease 4,,0 unchanged,
  and INIT TERM reap PASS. Test-only fault is at FILE entry, not the
  provider's own sync operation.
- [ ] Inject provider-origin VFS_SYNC refusal on a mounted regular file;
  separately test failed VM teardown, deferred u-area-release retries,
  permanent provider refusal, and fatal-event precedence. OOM remains OFF.
- [x] v936 full-system 256K PDP-6 descriptor-level close refusal:
  the deferred-TERM target owns an actual additional FILE descriptor;
  disposable `file_close_entry_v865` injection rejects its first two
  closes and then runs the normal close. Three targeted attempts,
  two refusals, zero empty-entry errors, finalizer success, CHLD=1,
  correct INIT reap, held FS lease 4,,0 and 36 peer cycles: PASS.
- [ ] Test rejection returned by the actual VFS/provider sync path,
  and witness exactly-once VM/u-area teardown through failure/retry.
- [x] v936 booted descriptor-level VFS_SYNC refusal/retry: disposable
  kernel intercepts real slot-5 FD1 `file_close_entry_v865` sync boundary,
  refuses twice, and allows the third unchanged `vfs_sync` call. 256K
  PDP-6 held-provider scenario PASS: 3 attempts, 2 refusals, finalizer=1,
  failure=0, CHLD=1, owner returns=3, 36 peer cycles, lease 4,,0 stable,
  correct INIT TERM reap. This strengthens v935 whole-table refusal test.
- [ ] Prove exact-once VM/u-area teardown and fatal-event precedence;
  exercise permanent provider refusal separately. Keep OOM killing OFF.
- [x] v935 booted 256K PDP-6 deferred TERM retry with two injected
  `file_close_all` refusals for slot 5, followed by one successful pass:
  3 calls, 2 refusals, successful finalizer, CHLD=1, correct WAIT status,
  36 peer cycles, and held FS owner/waiter lease unchanged at 4,,0.
  Test-only entry injection does not yet demonstrate a real FILE entry's
  provider refusal or exact-once VM/u-area release under all failures.
- [ ] Exercise an actual open descriptor failing close inside the provider;
  instrument FILE, VM and u-area release multiplicity, and establish
  fatal-event precedence before allowing automatic OOM killing.
- [x] v934 full-system 256K PDP-6 CHLD/TERM completion with *both* FS
  owner and waiter blocked: real lease remains 4,,0 at owner, waiter and
  final HALT; CHLD=1, descendant EXIT=1, finalizer entry=1, failure=0,
  owner return checks=3, 36 bystander cycles and correct TERM WAIT status.
  This closes the previously missing held-provider CHLD/TERM control.
- [ ] Booted FILE-close refusal/retry during owner TERM and exact-once VM,
  descriptor, and u-area release, followed by other fatal-event precedence.
- [x] v933 full-system 256K PDP-6 CHLD/TERM non-provider control: guest
  reaches child EXIT, `proc_finish_slot` success, CHLD publication, owner
  return gate, correct `SYS_WAIT_STATUS(SYS_WAIT_EXITED, 0400001U)` reap,
  INIT `P` marker, and controlled HALT. Test's stale contiguous-spinner
  assertion corrected and rerun against preserved guest transcript: PASS.
- [ ] Rerun the original provider-held scenario with a genuine CHLD wakeup,
  verify exactly-once FILE/VM cleanup under retry, then resolve the remaining
  fatal-event precedence and OOM termination safety gates.
- [ ] v932 owner finalization fix restores child EXIT completion in booted
  256K PDP-6 test: slot-6 finalizer enters once, failure count zero,
  `proc_notify_parent(5)` count one, and INIT reaches status check.
  The expected TERM status mismatches (`!A`), so end-to-end delivery
  remains open. Capture actual parent status and owner-side EXIT status,
  then fix/verify status propagation. Never weaken foreign finish guard.
- [ ] v931 booted finalization witness: descendant slot 6 reached
  `proc_exit_current` once and `proc_finish_slot` once, then took
  `proc_finish_fail` once; `proc_notify_parent(5)` remains zero. No
  artificial FS lease. Next isolate the precise rejection inside
  `proc_finish_slot` (foreign-context guard, missing u-area, finalize
  marker, or live FILE entry). In particular, `proc_exit_finish` clears
  `proc_current_slot` before calling `proc_finish_slot`, so audit whether
  the foreign-target guard is incorrectly applied to self EXIT.
- [ ] v930 booted control: descendant reaches `proc_exit_current` exactly
  once (slot 6); `proc_notify_parent(5)` remains unobserved, owner-return
  gate count 2, and INIT does not complete. No held provider. Instrument
  `proc_exit_finish`, `proc_finish_slot` and their failure/commit exits to
  locate the first blocked or failed cleanup step. No kernel fix yet.
- [ ] v929 staged no-provider diagnostic: instrumented owner-return helper
  was entered twice, but `proc_notify_parent(5)` was never observed (CHLD
  counter zero). With shortened test-only sleeps the descendant printed
  its pre-EXIT `Z` marker; INIT still did not finish. The prior v928 timing
  theory is insufficient. Next instrument descendant `proc_exit_current`,
  `proc_exit_finish`, and parent notify/WAIT before changing kernel logic.
  The v929 fixture must remain marked FAIL until that chain is proven.
- [ ] Corrected v929 boot: guest printed `Z` after descendant timer sleep,
  immediately before EXIT. CHLD notification witness remains zero and
  owner-return witness is two; three deferred TERM guards, one CHILD wait.
  Therefore investigate descendant EXIT/finalization before changing CHLD
  handling. The run also reported DCS0/GE0 port conflicts (1101/1201):
  eliminate fixture port collisions in the next controlled repeat.
- [ ] v929 stage diagnostic (initial run FAIL): real guest observed owner
  return-gate entry twice, but no `proc_notify_parent(5)` witness. The
  simulator reached its terminal HALT without INIT completion, with no
  provider hold and no independent sleep-witness activity. This does not
  prove a notification defect: first establish whether the descendant
  returned from its timed sleep and executed EXIT. A new guest `Z` marker
  was added to distinguish timer wakeup from EXIT/CHLD failure; rerun the
  corrected fixture before further kernel changes.
- [ ] v928 control: the booted CHLD/TERM test also fails with holder and
  waiter kept entirely outside the filesystem provider gateway. All three
  deferred TERM requests were observed, including one WAIT_CHILD; there was
  no held FS lease or provider wait. INIT still did not reach final exit
  or print its completion marker. Thus the artificial provider stall is
  NOT necessary for the v927 failure. Instrument CHLD notification, parent
  CTX_PC restore, owner-side TERM consume, FILE completion, process EXIT,
  and parent WAIT/reap independently. Retain all failures as regressions.
- [ ] v927 diagnostic: full 256K guest with a genuine descendant EXIT/CHLD
  after TERM is deferred to WAIT_CHILD has NOT completed. With the
  deliberately held FS provider, the guest stopped before INIT's final
  marker (`final_lease` sample zero; 88 bystander cycles). The recorded
  deferred TERM guards still saw three requests and one WAIT_CHILD.
  Separate the provider-stall dependency from the CHLD/TERM handoff,
  instrument exact parent/child teardown stages, and retain failure media
  and full SIMH transcript before treating this as a kernel defect.
- [x] Booted 256K PDP-6 `daimos-provider-term-live-v926` PASS: three
  protected foreign TERM requests accepted, including real WAIT_CHILD and
  held FS owner/waiter, unchanged lease 4,,0, 72 independent bystander
  cycles, no errors and controlled HALT. The test leaves its WAIT_CHILD
  parent blocked, so owner-side termination after CHLD wakeup is NOT yet
  full-system accepted.
- [x] Re-run current v925 native fatal, direct-finish and OOM handoff
  regressions: all three PASS on PDP-6. These are extracted source tests,
  not full-system CHLD wakeup-handoff injection.
- [x] Record the required ownership, pending-delivery, PI6 and test contract
  in `design/proc-deferred-term-v926.md`. No production behavior changed.
- [x] Audit event bitmap layout and use the existing TERM bit for an
  initial TERM-only deferred request. Other fatal-event precedence is open.
- [x] Implement owner-return TERM consumption through the private fatal
  close/retry path, retaining all v925 foreign executive-stack guards.
- [x] Native v926 isolated deferral/coalescing and v925 fatal lifecycle PASS;
  the three original v925 fatal/finish/OOM regressions still PASS.
- [x] Native owner-return helper execution with two FILE refusals, retry,
  exact-once TERM consumption and no replay (`daimos-deferred-owner-return-v926`).
- [ ] Establish precedence for remaining fatal events and prove a complete
  booted CHLD/TERM handoff with real process scheduling.
- [ ] Prove repeated-send coalescing, FILE refusal/retry, exact-once teardown,
  and booted CHLD wakeup-to-executive-restore delivery. Keep OOM killing OFF.

### v925: guard saved executive wakeup handoff (2026-11-01)

- [x] Detect CHLD wakeup gap: WAIT_CHILD cleared before the private
  saved executive continuation is restored, so wait bits alone are not
  an adequate noncurrent teardown guard.
- [x] Protect read-only OOM selection, remote fatal FILE close, and
  direct finalization using existing CTX_PC user-return bit in addition
  to the v924 wait mask. Twelve instructions, zero new resident words.
- [x] Negative controls: OOM exit 010, fatal 032, finish 034 on old code;
  corrected native PDP-6 tests and historical teardown regressions PASS.
- [x] Booted v924 provider/child-wait regression on v925 PASS:
  three fatal refusals, one WAIT_CHILD, stable 4,,0 lease,
  73 independent sleep cycles, zero errors, controlled HALT.
- [ ] Direct full-system wakeup-handoff injection and deferred cooperative
  TERM delivery remain open. Do not release a live executive stack;
  bounded provider recovery and allocation-free OOM killing still P0.

### v924: protect all private wait classes (2026-11-01)

- [x] Fix v918/v920 guards in OOM selection, foreign fatal delivery and
  direct process finalization: PROC_WAIT_LH_MASK covers EVENT, CHILD,
  and INTR continuations. No growth in instructions, RAM or ABI.
- [x] Native PDP-6 negative baselines expose old bug (OOM exit 010,
  fatal 032, finish 033). Repaired CHILD/INTR tests and historical
  EVENT/finalizer/FILE regressions PASS; v921 static audit strengthened.
- [x] Booted 256K PDP-6 test: real WAIT_CHILD parent, living child,
  stalled FS owner/waiter and bystander. Three fatal refusals including
  one WAIT_CHILD, stable lease 4,,0, 72 cycles, zero errors, HALT.
- [ ] Restore useful cooperative TERM semantics to private WAIT_CHILD
  without freeing the live executive stack or allocating resident RAM.
- [ ] Prove bounded cancellation/recovery for permanently stalled FS
  providers and allocation-free OOM termination. No forced unlock;
  automatic OOM killing remains OFF.

### v923: repeated FILE SYNC refusal retains ownership (2026-11-01)

- [x] Native PDP-6 regression: two refused SYNC attempts preserve the
  failed FD; previously successful closes are not replayed, and the
  caller's global FILE table stays intact.
- [x] Final successful retry closes only the outstanding FD and
  finalizes the target once. v923/v867/v865/v885 regressions PASS.
- [x] Existing code correct for tested sequence; no KCORE/RAM/ABI change.
- [ ] Permanently unresponsive providers and complete allocation-free
  OOM teardown remain P0. Automatic OOM killing stays OFF.

### v922: repeated partial session HUP retry (2026-11-01)

- [x] Confirm a member with TRANSITION+VM_NONE+FINALIZE is never HUPed
  again while a later session member keeps refusing fatal delivery.
- [x] Native PDP-6 test: two consecutive HUP failures preserve the first
  pending VM/u-area owner and the attached TTY; a final successful retry
  commits TTY release. Negative pre-v905 control fails with exit 023.
- [x] Native v905/v875/v918 and v921 source audit PASS. No kernel/RAM
  or ABI change required.
- [ ] FILE/provider refusal without bound, failure-atomic recovery for
  all teardown stages and allocation-free OOM termination remain P0.
  Automatic OOM victim killing stays OFF.

### v921: OOM teardown safety inventory (2026-11-01)

- [x] Audit actual PDP-6 FILE, session-HUP, VM, backing and u-area
  teardown call edges; add executable testkit static safety gate.
- [x] Check resident selector is not wired into MM pressure killing and
  v918/v920 suspended-stack guards remain in the source. Audit PASS.
- [ ] Establish bounded behavior and failure-atomic ownership of FILE
  sync, HUP fanout and VM/backing release under exhausted RAM; prove
  complete termination chain allocation-free before enabling a killer.
- [ ] Recover or safely quarantine permanently nonreturning provider
  requests without forced release. No production RAM/ABI change in v921;
  OOM killing stays OFF.

### v920: protect direct noncurrent process finalization (2026-11-01)

- [x] Reject `proc_finish_slot` against a foreign target with suspended
  private kernel event continuation, before changing ownership.
- [x] Native PDP-6 v920 PASS: no VM/u-area release under blocked event;
  normal fatal lifecycle still succeeds when wait state clears.
- [x] Native v883, v918 OOM/fatal and v868 regressions PASS. No new RAM.
- [x] Booted v919 compatibility on v920 source PASS: two fatal refusals,
  stable lease, 72 bystander cycles, zero errors, controlled HALT.
- [ ] Prove all OOM termination helpers allocation-free before enabling
  any killer.
  Recovery of permanently stalled providers remains open P0.

### v919: booted remote fatal refusal for private FS waits (2026-11-01)

- [x] Full 256K PDP-6 guest: real owner holds FS gateway lease,
  independent waiter sleeps, and third process remains runnable.
- [x] INIT sends TERM to holder and waiter; both reach the actual
  non-current fatal guard and return refusal. Two guard rejections,
  lease 4,,0 preserved, 72 independent sleep cycles, zero errors,
  controlled HALT. Fault hooks only in disposable testkit kernel.
- [x] No production source or RAM change; native v918 protection remains
  accepted. Documentation synchronized with pdp10-doc.
- [ ] Permanently stalled provider owner reclamation and the full
  allocation-free OOM termination proof remain P0. OOM killing OFF.

### v918: protect private kernel-event stacks from remote teardown (2026-11-01)

- [x] Refuse non-current fatal close/VM release when victim has an active
  PROC_WAIT_EVENT_LH continuation. Do not reclaim an active stack.
- [x] Exclude such waiters from read-only OOM candidate selection.
  No new resident RAM, metadata or syscall ABI changes.
- [x] Native v918 fatal and OOM regressions PASS; native v866/v883/v879,
  v868/v867/v865/v885 regression chain PASS.
- [x] Booted 256K PDP-6 v917 foreign-provider regression on v918 source
  PASS: one held lease, one waiter, 72 independent bystander cycles,
  unchanged owner, zero errors, controlled HALT.
- [ ] Booted full-system fatal-event acceptance, stalled-owner recovery
  and allocation-free OOM cleanup remain P0. OOM killing stays OFF.

### v917: correct serialized FS lock event and booted scheduler (2026-11-01)

- [x] Repair busy retries caused by using the nonzero owner-encoded
  lock as the scheduler event. Use one separate event word; preserve
  owner identity and normal unlock notification.
- [x] Reject a foreign held lease from slot zero without bootstrap spin.
- [x] Native v917 and earlier provider-lock tests PASS.
- [x] Booted 256K PDP-6 test: owner held, foreign waiter blocked,
  owner 4,,0 unchanged, 72 bystander ticks, zero errors, normal HALT.
  Negative control had 8193 busy retries without scheduler progress.
- [ ] Event-level progress does not guarantee disposal of permanently
  unresponsive providers. Allocation-free fatal/OOM cleanup remains P0;
  automatic OOM killing stays OFF, TSFS v1.2 remains deferred.

### v916: foreign provider-lock ownership/waiter regression (2026-11-01)

- [x] Native PDP-6 FS gateway test holds another process's provider
  lease through four simulated waiter retries. No early provider
  dispatch, stolen ownership or forced unlock. A fifth wait observes
  release, dispatches and restores request registers and stack.
- [x] Native v915 recursive-owner and v866 owner-encoding regressions
  PASS. No production code, RAM, syscall or module ABI change.
- [ ] Full booted live scheduler test with a truly stalled provider,
  diagnosis of foreign owner death, and allocation-free fatal/OOM
  resolution remain open P0. No forced unlock; OOM killing OFF.

### v915: prevent recursive provider-lock self-deadlock (2026-11-01)

- [x] Detect a DTFS/D6FS/TSFS gateway call by the existing lease owner
  before blocking in `proc_wait_event`; return -5 with the saved
  seven-word request frame unwound and without unlocking its lease.
- [x] Reuse the existing owner_slot+1 LH encoding in `fs_provider_ready`;
  nine additional KCORE instructions, no extra BSS, per-process lock
  table, provider ABI or MEMFS changes.
- [x] Native PDP-6 v915 recursive-owner and other-process waiter tests
  PASS. Existing v866, v326 and v327 tests PASS.
- [x] Full 256K PDP-6 v914 fatal FILE-provider refusal regression rebuilt
  against the final nine-instruction v915 KCORE and PASS (11 provider
  -4 refusals, 11 stable FD checks, 512-tick backoff, normal test HALT).
  Not yet a full-system recursive-provider-lock injection.
- [x] Post-v915 full-system regression: 256K PDP-6 v914 APR fatal-FILE
  provider invalid-refusal workload PASS. 11 VFS refusals, 11 private
  owner sleeps, 512-tick cap, retained descriptor and independent
  runnable user, zero errors, controlled HALT. This does not simulate
  a held-provider-lock deadlock.
- [ ] A different process owning a permanently stalled provider remains
  a separate unresolved progress case. Full-system PI6/clock provider
  lock acceptance and allocation-free fatal/OOM termination remain P0.
  OOM killing stays OFF; TSFS v1.2 and KCORE shrink stay deferred.

### v914: permanent invalid FILE-provider refusal containment (2026-11-01)

- [x] Booted 256K PDP-6; real APR fault causes permanent VFS_SYNC
  error -4 on the victim's FD 0, not the previously tested -5.
- [x] 11 rejected close passes and owner-stack sleeps with a 512-tick
  retry cap; FILE ownership, error-class-4 witness and process identity
  persist while an independent user runs. No premature close or EXIT;
  normal controlled test HALT. No production code or RAM change.
- [x] Native v888 MM error classification, v909 fatal MM stage and
  v885 private FILE backoff regressions PASS.
- [ ] This does not repair corrupted/missing MM extents or ensure
  eventual resource reclaim. Provider-lock refusal, fatal teardown
  completeness and allocation-free OOM killing remain P0. OOM stays
  OFF; TSFS v1.2 and KCORE final shrink remain deferred.

### v913: live PDP-6 double-MM release refusal acceptance (2026-11-01)

- [x] Booted 256K PDP-6 with Type-340 PI6 refresh/PI7 completion and
  real MM pinning of two distinct owner-014 raw-list extents.
- [x] Original list base 19760 equals published `dpy_list_base`;
  unpublished replacement base 19792 equals `dpy_list_orphan_v912`.
  Four rejected frees (replacement, rollback, next WRITE and CLOSE),
  2 pins, 0 unpins, 0 errors, 18 PI6/PI7 cycles and 12 post-error
  refreshes; guest <D and normal HALT. No extra allocation.
- [x] Existing native v912/v874/v904 regressions PASS. No production
  source/ABI/RAM change. Fault hooks remain only in daimos-testkit.
- [ ] Permanent refusal preserves both physical allocations; forced
  reclamation is not permitted. Fatal FILE/MM provider teardown,
  provider locks and allocation-free OOM termination remain P0.
  OOM killing stays OFF; TSFS v1.2 and size optimization stay deferred.

### v912: DPY double-free replacement failure containment (2026-11-01)

- [x] Replace the HALT on consecutive old-list and new-list MM-free
  refusal with one tracked unpublished DPY-MRES extent.
- [x] On every subsequent WRITE_WORDS or STOP, attempt orphan release
  before allocating or mutating the active display. Refusal propagates
  to FILE without losing either owner; recovery drains before proceeding.
- [x] Native PDP-6 v912 double-fault and recovery test PASS. Existing
  v871/v874/v904 and PI7 interrupt-policy regressions PASS.
- [x] Full-system post-change v910 permanent-MM_BUSY regression PASS on
  v912: 3 BUSY, one pin, 18 PI6/PI7 frames, normal HALT; no live-path
  regression. This is not the double-fault injection test.
- [ ] Full booted guest with live PI6/PI7 double-fault injection is not
  yet accepted. Also outstanding: permanent fatal FILE/MM providers,
  provider-lock refusal and safe allocation-free OOM termination.
  OOM killing remains OFF, KCORE shrink and TSFS v1.2 deferred.

### v911: repair resident SYS_MEMINFO FILE counting (2026-11-01)

- [x] Correct the 16-entry FILE scan and reported capacity from octal
  015 (13) to octal 020 (16); do not alter FILE allocator semantics.
- [x] Move the existing count past the temporary MEMFS request whose
  seven-word BLT erased result word 7. No added instructions or BSS.
- [x] Booted 256K PDP-6 C89 guest fills all 16 FILE slots by real DUP,
  checks MEMINFO after every allocation, verifies final FD 15, closes
  the duplicates, checks baseline, reports <F and halts normally.
- [x] Two isolated pre-fix negatives: old capacity yields !1; old
  pre-MEMFS FILE count yields !4. Native v708/v874/v904 PASS. Full v910
  live DPY permanent-MM_BUSY guest also PASSes with meaningful updated
  FILE occupancy accounting.
- [ ] This does not enable automatic OOM victim termination or recover
  permanently refusing FILE/MM providers. Replacement double-failure,
  lock/deadlock acceptance, and fatal FILE teardown stay P0; TSFS v1.2
  and final KCORE shrinking remain deferred.

### v910: persistent DPY/MM refusal across three live CLOSE attempts (2026-11-01)

- [x] Booted 256K PDP-6 Type-340, kept the raw-list extent physically
  pinned across three real MM_BUSY final CLOSE attempts. All returned -5;
  the already-open FILE/process ownership counters stayed unchanged,
  and the guest halted normally without forcibly freeing any memory.
- [x] Measured raw-list owner 014, base 19732 decimal, three MM frees,
  one pin, zero unpins, three MM_BUSY results, 18 PI6 raw refreshes,
  18 PI7 completions, 12 post-first-refusal refreshes and zero errors.
  v871, v874, v904 and the PI7 policy tests remain PASS.
- [ ] A permanently rejecting provider *retains* its physical RAM. This
  proves safe DPY ownership, not eventual cleanup. Still open: injected
  replacement double-fault, provider lock/deadlock handling, fatal FILE
  teardown and zero-allocation OOM victim retirement. OOM stays OFF;
  TSFS v1.2 and KCORE size optimization stay deferred.

### v909: P0 permanent physical-MM refusal and capped idle reaper (2026-11-01)

- [x] Reproduced a true CPU-intensive leak of retries under permanent
  physical `mm_pin`: the PDP-6 slot-0 idle loop retried a still-owned
  u-area every clock tick (1,281 rejected `mm_free` calls, 1,280 reaper
  passes across the guest's 02400-octal-tick observation period). The
  timer-driven scheduler admission path had a cap, but the already-idle
  loop ignored it. Negative control and original simulator transcript
  retained in v909 test evidence.
- [x] The idle loop now checks wrapped elapsed ticks against the existing
  shared retry interval before trying deferred MM release. No new BSS,
  per-process retry table, forced MM free or module dependency. The real
  guest records 7 refused frees, 6 reaper passes, 1 physical pin and
  no test errors; the independent bystander keeps running.
- [x] Once fatal FILE closure and VM destruction have committed, clear
  stale `PROC_FATAL_CLOSE_BIT` on the post-VM pending-u-area owner. A
  TRANSITION+VM_NONE+FINALIZE marker still quarantines it; GETTEARDOWN
  now correctly reports MM stage and the capped 01000-octal (512-tick)
  interval rather than the obsolete FILE stage. Native v909 verifies
  exact-once finalization and the old-code negative control fails.
- [x] Full-system v909 on 256K PDP-6: real APR fault, permanent pinned
  u-area, `GETTEARDOWN` MM and physical pin diagnostics, nonblocking
  WAIT refusal, independent runnable child, controlled test HALT PASS.
- [ ] Permanent failure still retains physical RAM indefinitely; this
  is safe ownership and bounded work, NOT successful recovery or safe
  automatic OOM termination. Continue locked-provider, nested fatal
  teardown and allocation-free OOM policy work. KCORE shrinking and
  TSFS v1.2 remain deferred.

### v908: live DPY raw-list MM_BUSY and PI7 completion (2026-11-01)

- [x] Full-system 256K PDP-6 headless Type-340 (`DPY OK`) verifies the
  *actual* raw-list PI6 refresh and PI7 BLKO completion path, not just
  static source/isolated assembly. A disposable test-kernel MM hook pins
  the exact dynamic owner 014 on final CLOSE. The native MM refuses
  release with -5; FILE and raw-list ownership remain live and clock
  refresh resumes. Retry physically unpins/frees; a new DPY program
  installs and closes; FILE/process counts recover; guest HALT normal.
- [x] v908 guest counters: MM base 19730 decimal, raw-list MM frees 3,
  pins/unpins 1/1, MM_BUSY 1, raw PI6 refresh 18, raw PI7 completion 18,
  *six* refreshes after the failed CLOSE but before unpin, errors 0.
  Standalone target C89 build and shell syntax PASS. Native v709, v871,
  v874, v904 and the DPY PI7 policy regressions also PASS.
- [ ] Indefinitely refusing FILE/MM providers, fatal process isolation,
  and allocation-free automatic OOM termination are NOT accepted by a
  single recoverable MM_BUSY. Do not force-release or enable OOM killing.
  DPY-specific persistent-refusal and double-fault behavior remain P0.
  TSFS development and KCORE shrinking remain deferred.

### v907: full-system HUP partial failure with physical MM (2026-11-01)

- [x] Disposable copied-kernel hooks pin the first real session child's
  u-area before normal fatal HUP, reject a second child's first HUP,
  then retry the leader's fatal HUP through the real PROCCTL syscall.
  The first child's VM_NONE+FINALIZE ownership remains physically pinned
  during the second scan and is skipped only by production v905 logic.
- [x] Booted 256K PDP-6 PASS: first child PID 3, leader PID 2, exact
  pinned u-area 20020 decimal; 1 MM pin/unpin, 1 injected HUP rejection,
  1 second-member delivery, 0 duplicate first-member HUPs, 1 observed
  deferred finalization, 0 errors. Parent WAIT and TTY reattach pass;
  normal HALT. No production kernel change or KCORE size work.
- [ ] A persistently rejecting FILE/MM provider still must never be
  force-released. The DPY live interrupt/MM_BUSY acceptance is covered
  by v908; the allocation-free OOM safe-victim gate remains closed.

### v906: reconcile older TSFS entries after KCORE deferral (2026-11-01)

- [x] Supersede the remaining unchecked cross-provider TSFS mount and
  DTFS/TSFS metadata/capacity experiments in older V13/V15/V22 lists.
  They are post-KCORE work, not active KCORE feature or size gates.
- [x] Preserve existing TSFS boot and VFS/codec compatibility checks
  whenever KCORE changes their shared contracts; this does not reopen
  TSFS v1.2 feature work. D6FS, generic VFS and backing work remain
  KCORE tasks. The TSFS feature sequence resumes only after KCORE.

### v905: P0 partial session-HUP retry (2026-11-01)

- [x] During session-leader teardown, skip an already terminal member
  only if its descriptor is TRANSITION, VM is NONE, and the original
  u-area contains PROC_FINALIZE_MARK_LH. This is the precise committed
  fatal-event/deferred-MM owner; `proc_event_apply` correctly rejects a
  second HUP, so retry must not try it again after another member fails.
- [x] Native PDP-6 partial HUP regression v905 PASS: first child commits
  with deferred u-area; second member fails, TTY remains; retry skips the
  committed child, HUPs only the failed member, then detaches. Cases with
  non-NONE VM or absent finalize mark cannot be skipped. Pre-v905 negative
  control fails as expected (guest failure code 010). Historical v875 PASS.
- [x] Existing full-system `daimos-session-teardown-v1.sh` on the v905
  kernel PASS: booted PDP-6 session-leader exit, HUP of running/stopped
  children, controlling-TTY detach and later reattach.
- [x] Full-system injected HUP partial-fault/actual-MM rollback PASS
  as v907, including one pinned terminal victim and later TTY handoff.
- [ ] Indefinitely refusing FILE/MM providers still require safe ownership
  handling; automatic OOM termination remains disabled.

### v904: P0 DPY teardown quiesce-before-unpublish (2026-11-01)

- [x] STOP disables the Type-340 controller and PI7 before removing the
  active raw-list base. This closes an interrupt window where PI7 could
  treat the still-running raw-list completion as a retained-text frame.
  No new instructions or permanent allocation; instruction order only.
- [x] v904 native PDP-6 checks actual MM pin/unpin/free: pinned extent
  refuses CLOSE without losing FILE or DPY ownership; invalid MM owner
  also refuses; correct owner then frees exact extent, and empty CLOSE
  succeeds. Pre-v904 STOP-order negative control rejects the old order.
  Existing v709, v871 and v874 tests PASS.
- [ ] Full-system DPY STOP under a live PI7 replay and permanent MM provider
  refusal remain separate P0 acceptance. Continue session/HUP partial
  failure and safe automatic OOM teardown; do not force-release MM.

### v903: TSFS scheduled strictly after KCORE (2026-11-01)

- [x] Remove TSFS v1.2 format/metadata implementation and its acceptance
  from the active KCORE feature and size backlog. Complete KCORE features,
  their regressions and the deferred KCORE size/freeze work first. Then
  resume the separate TSFS v1.2 sharded-metadata effort before 0.9 release.
- [x] Preserve existing TSFS boot compatibility and regression coverage
  while changing other subsystems. Such preservation is a KCORE invariant,
  not permission to start new TSFS work during KCORE completion.
- [ ] Next active P0: DPY raw-list failure-safe teardown and real-MM
  acceptance; then session/HUP partial-failure recovery, permanent FILE/MM
  provider repair, and safe allocation-free OOM victim termination.

### v902: historical anonymous PIPE regression rerun (2026-11-01)

- [x] The full-system historical `daimos-pipe-anon-v1.c` user workload
  now passes on the v901 kernel with only headless simulator settings
  changed. Verified PIPE create, character I/O, EOF, EPIPE, duplicated
  references, `RUN` descriptor inheritance and blocking wakeups.
  The versioned v902 harness preserves the legacy test program.
- [ ] Keep feature completion and allocation-free teardown as the
  priority. OOM killing remains disabled until permanent FILE/MM
  provider faults, DPY cleanup and session-HUP failure graphs have
  safe ownership and recovery semantics.

### v901: PIPE character-stream feature restored (2026-11-01)

- [x] Native `READCHAR` and `WRITECHAR` on PIPE and named FIFO
  descriptors now dispatch to the existing one-word PIPE queue
  operations. These syscalls exchange one complete character word
  rather than packed SIXBIT; no new pipe buffer or permanent state.
- [x] Native PDP-6 positive test: WRITECHAR/READCHAR transfer,
  pending queue count, EOF after last writer, broken-pipe error and
  unchanged MonitorFS device dispatch PASS. Pre-v901 VFS source
  rejects PIPE WRITECHAR; negative control FAILS as expected.
- [x] Full 256K PDP-6 user syscall test PASS: writechar/readchar,
  cross character/word I/O in both directions, EOF after writer
  close, exact FILE-slot accounting and normal HALT. The underlying
  VFS gap also explains the earlier v899 character-read assertion;
  its separate real-MM word-I/O ownership test remains valid.
- [ ] Remaining feature work includes DPY raw-list teardown,
  session/HUP partial failure, permanent provider repair and safe
  allocation-free OOM victim termination. No size sweep attempted.

### v900: named FIFO MM refusal and list-ownership acceptance (2026-11-01)

- [x] In a booted 256K PDP-6, inject one actual `mm_pin` of the
  named FIFO's final physical pipe object. Its regular last CLOSE
  receives real MM_ERR_BUSY and restores the named object to its
  active linked list, retaining the original FILE descriptor.
- [x] A second O_RDWR OPEN joins that same object after the failure;
  writing through the original descriptor and reading from the new
  one proves cross-descriptor stream identity. The later final CLOSE
  unpins and frees the exact extent. A new FIFO OPEN/CLOSE and
  normal HALT pass. Counters: target 19687 decimal, calls 3,
  physical pins 1, unpins 1, busy 1, errors 0.
- [ ] Automatic OOM killing still requires complete allocation-free
  fatal FILE/MM/TTY/session teardown. An indefinitely refusing or
  invalid provider must not be forcibly released.

### v899: real PIPE final-close MM_BUSY recovery (2026-11-01)

- [x] Native v870 final PIPE close retry remains PASS.
- [x] Full-system 256K PDP-6 acceptance with actual MM pin,
  `mm_free` MM_ERR_BUSY and `mm_unpin` before retry PASS. The
  failed final close retains the original read FILE and PIPE
  reference, which can still report EOF. Later close releases
  the exact extent, and a new PIPE can be created/closed.
  Isolated guest counters: target 19687 (decimal), final-close
  calls 3 (including reuse), pins 1, unpins 1, busy 1, errors 0.
  The test kernel alone contains injection/counters, and the
  production KCORE is unchanged. No shrinking work attempted.
- [ ] Still open: named FIFO lifetime under real MM refusal, DPY
  raw-list cleanup, session-HUP partial commit, persistent FILE/MM
  provider refusal, and allocation-free automatic OOM retirement.
  An indefinitely pinned or inconsistent provider cannot be
  forcibly discarded.

### v898: booted PDP-6 real TTY MM_BUSY release acceptance (2026-11-01)

- [x] Production v897 control path accepted with actual physical
  `mm_alloc`, `mm_pin`, rejected `mm_free`, `mm_unpin`, and successful
  retry, all in a booted 256K PDP-6 guest. The test kernel is an
  isolated overlay; the production source is unchanged.
- [x] Session owner stays attached through the first rejected
  `TTY_DETACH`; the second attempt succeeds only after unpin.
  Subsequent attach/child session activity and normal HALT PASS.
  Test counters: four checked operations, one physical pin, one
  MM_BUSY, one unpin and zero errors. No KCORE size work was run.
- [ ] Remaining feature gates: refused FILE/MM providers, nonreturning
  fatal cleanup, session HUP partial-commit cases, and allocation-free
  OOM victim retirement. Automatic OOM termination stays disabled.

### Feature completion priority and TTY ownership v897 (2026-11-01)

- [x] Defer the KCORE 8,192-word size-reduction sweep until functional
  work and its real-system regression gates are complete. Memory
  footprint remains an invariant to avoid unnecessary growth, not
  the present optimization target.
- [x] v897: prevent `TTY_DETACH` from dropping controlling-session state
  if the canonical SIXBIT line MM extent is pinned or has invalid owner/
  type metadata. Reject `TTY_ATTACH` by a new session until a leftover
  line is successfully released; preserve existing TTY output routing.
  `proc_tty_mode_set` reuses the same checked, allocation-free helper.
- [x] v897 native PDP-6 `procctl` tests PASS failure/retained session,
  output-route preservation, retry after unpin, and successful handoff.
  The pre-v897 negative control incorrectly transfers a pinned line,
  as expected. A second native harness builds the actual C helper and
  executes real PDP-6 `mm_free` and `mm_unpin` for pinned, wrong-owner,
  missing-extent and successful-release cases: PASS.
- [x] v897 production full-system PDP-6 session attach/detach/reattach
  and independent child regression PASS. The headless test harness
  uses the original session-state user workload and normal kernel.
- [ ] Continue allocation-free teardown audit, persistent FILE/MM
  provider recovery, fatal process isolation, and safe OOM admission.
  Do not enable automatic OOM killing on a failed-provider path.

### v896: RUN startup owns allocated u-area before fallible work (2026-11-01)

- [x] Move the existing u-area descriptor publication before
  `vm_space_startup`, preserving SIDL exclusion and the same three-word
  process descriptor. A failed startup must enter `proc_slot_discard`
  with actual ownership and release the exact MM extent, including
  v892/v894 deferred cleanup on downstream provider errors.
- [x] Full-system PDP-6 startup-branch fault injection PASS: one
  unpublished RUN discard, exact MM u-area probe confirms 0 leaks,
  valid second RUN/WAIT, and normal HALT. The v895-ordering negative
  control independently detects exactly 1 leaked MM extent while
  process slot/VM accounting appears normal. Same-profile GCC lowmem
  KCORE 9,426, KCORE+MRES 15,912, endpoint 15,959, all unchanged.
  Native v885/v892/v894 cleanup regressions PASS.
- [x] Original v197 full-system regression PASS after the v896 change:
  16 post-VM RUN fdmap failures under three live children, stable
  accounting and recovery. Its test harness now disables SDL window
  consoles on headless hosts; the first GUI attempt crashed inside
  the simulator before guest assertions, not in DAIMOS.
- [ ] Persistent FILE/MM provider owner inconsistencies and safe OOM
  termination remain open. No forced MM release is authorized.

## Size refresh and pending work (v513, 2026-10-27)

### v864: OOM handling for already-running processes (2026-10-31)

- [x] v895: real full-system PDP-6 pre-VM RUN MM_BUSY recovery.
  Test-only `mm_pin` of the actual child process VM before a normal
  destructor produces two genuine MM_ERR_BUSY failures, then one
  `mm_unpin` permits the third attempt to complete. Slot-zero retry
  calls 2, independent child runs, no unpublished-child WAIT,
  zero errors and normal HALT. No production size or ABI changes.
- [x] v894: failed VM destruction during unpublished RUN rollback,
  **including before u-area allocation**, retains SIDL+TRANSITION
  and one spare scheduler RH bit 02 for reaper retry. A later
  successful VM destroy clears that mark and continues to v892
  post-VM cleanup if u-area free fails. Read-only GETTEARDOWN stage
  3 reports the ownership and shared delay. Native PDP-6 failure/
  retry, stage-3/older query and deferred-cleanup regressions PASS;
  pre-v894 negative control FAILS. Same-profile GCC KCORE +45
  code words, no static RAM, ABI syscall slot or process-table growth.
- [x] v894 full-system VM-pin follow-up: v895 now verifies actual
  booted PDP-6 `vm_space_destroy` MM_BUSY, retry/unpin, independent
  runnable work, and absence of a fabricated parent WAIT. v893's
  separate post-VM u-area busy case remains accepted.
- [x] v893: full-system PDP-6 RUN rollback with actual u-area
  `mm_pin`, two genuine MM_ERR_BUSY (-5) releases, one `mm_unpin`,
  two slot-zero retries and final FREE. Test-only kernel overlay;
  independent child runs and exits, no unpublished-child WAIT.
  SIMH counters 1/3/2/1/2, zero errors, normal HALT. No production
  change, size growth or static data.
- [x] v892: retain ownership after an unpublished RUN child has its
  VM destroyed but its u-area MM free fails. Register FINALIZE|DISCARD
  in existing word 067, reuse deferred slot-0 retries, and publish
  FREE (never a zombie or parent WAIT) only after successful free.
  Native fault-injection and prior deferred-EXIT tests PASS; the
  full-system v197 RUN fdmap rollback test PASSes 16 cases. Old
  v891 code FAILS the negative control; same-profile GCC KCORE +30
  words with zero new static words.
- [x] v892 full-system follow-up: real pinned u-area MM rejection,
  independent process, retry, exact-once discard and no parent event
  accepted by v893. Keep automatic OOM termination disabled until
  all provider and transaction ownership gates are met.
- [x] v891: full-system *permanent* FILE provider refusal containment.
  Genuine APR fault, copied-kernel VFS-sync rejects FD 0 indefinitely,
  while an independent user runs. Through 02400 octal clock ticks the
  private owner performs 11 rejected passes/sleeps, retains FD and
  real ERR 5 diagnostic, and reaches the 01000 octal (512-tick)
  exponential backoff cap with no forced release or premature exit.
  Actual PDP-6 SIMH PASS, no DAIMOS changes or additional kernel RAM.
- [ ] v891 follow-up: classify and repair invalid/missing/type/owner MM
  extents and other permanently failing FILE providers in their owning
  subsystems. A saturated retry delay does not authorize forced free.
  Define what an administrator can safely do without invalidating
  outstanding provider references. Automatic OOM termination remains
  disabled until complete allocation-free teardown is guaranteed.
- [x] v890: real full-system post-commit EXEC fatal FILE failure
  recovery. Valid user `EXEC` enters true low-memory fallback
  after injected pre-stage NOMEM; the production VM destroy
  commits `VM_SPACE_NONE` before test-only replacement-load
  rejection. Simulated VFS close fails twice; private kernel
  continuation retries after 1/2 ticks, retains FD and recorded
  ERR 5, clears witness on success, and parent WAIT returns
  fatal EXEC status 1. Booted PDP-6 SIMH PASS, no production
  code/data changes (KCORE 9,300).
- [ ] v890 next: investigate permanently refusing FILE/MM
  providers while another user remains runnable; retention,
  diagnostic availability, bounded CPU usage, and a safe
  administrative remedy. Do not enable automatic OOM killing
  without proving resource-lifetime and admission guarantees.
- [x] v889 follow-up: booted post-commit EXEC fatal failure
  acceptance completed in v890; APR path accepted in v889.
- [x] v889: full-system, actual user APR fault to private-stack
  fatal FILE cleanup acceptance. Disposable test kernel injects
  -5 from VFS-sync on the first two FD-0 close attempts.
  SIMH PASS: 1 real APR fault, 3 VFS calls, 2 rejections,
  3 fatal close passes, 1/2-tick sleeps, FD retained across
  failures, ERR class 5 visible, FD/witness cleared on
  success, correct parent WAIT and clean HALT. Zero production
  code/data changes; KCORE remains 9,300 words.
- [x] v889 EXEC follow-up: booted PDP-6 post-commit fatal
  provider-failure acceptance completed in v890.
  Persistent rejection diagnosis/recovery and the admission
  policy for automatic OOM termination remain open.
- [x] v888: preserve the first actual rejecting FILE provider
  result and FD during private fatal close using otherwise-unused
  u-area word 067. Expose a compact 0..6 result class through
  existing GETTEARDOWN detail bits 15..17 and a new numeric
  `ps ERR` column. A full successful close clears the record;
  nonfatal FILE closes leave it untouched. MM mismatches remain
  distinguishable through the v887 exact-extent snapshot. Native
  source/negative tests PASS. Real MM allocator busy/invalid
  distinction test PASS and full-system ordinary session/HUP/TTY
  regression PASS. KCORE +26 text words, no static data.
- [x] v888 transient FILE regression: proven in a full-system
  private-stack APR fatal continuation (v889), followed by an
  irreversible EXEC continuation (v890).
  Continue investigating invalid MM owner/type metadata failures
  rather than forcibly reclaiming such allocations.
- [x] v887: add on-demand exact provider diagnostics to the existing
  GETTEARDOWN(pid) syscall using PID flag 0400. FILE stage reports
  count, first remaining descriptor and VFS/PIPE/DPY class. MM
  stage reports verified exact pin count or missing/type/owner
  inconsistency. The existing stage/retry query is unchanged.
  Native PDP-6 tests PASS, zero new permanent RAM; KCORE +72
  text words over v886. No forced owner release is permitted.
- [ ] v887 follow-up: diagnose actual rejecting provider operations,
  not merely snapshots of surviving references. A persistent
  mm_free MM_ERR_INVAL needs a different recovery investigation
  from a legitimate pin. Automatic OOM killing remains disabled.
- [x] v886: user-visible readonly teardown state, via
  `procctl(GETTEARDOWN, pid)` in unused UUO-077 sub-op 041 and `ps`
  TD/RETRY columns. Distinguish normal/zombie (0), private
  fatal FILE owner (1), and deferred VM-free MM owner (2 with
  the existing shared 16..512-tick retry delay). No new
  permanent data or privileged forced-release operation.
  Native PDP-6 leaf/dispatch tests, KCORE/PS builds, and
  full-system actual-UUO/MM_BUSY/spinner/HUP tests PASS.
- [ ] v886 administrative follow-up: no claim that a provider is
  permanently failed can be made solely from a stage or retry
  interval. Investigate provider-specific recovery and diagnostic
  checks before allowing any automatic OOM victim termination.
- [x] v885: bound fatal FILE-close and quarantined MM u-area retry
  frequency without discarding any owned resource. FILE sleeps
  exponentially from 1 to 01000 octal ticks on its private stack.
  The shared MM reaper uses the existing last-service word for a
  020..01000 octal tick exponential delay, reset for a new owner
  and cleared when no pending u-area remains. Native tests and the
  real full-system transient MM-busy spinner regression PASS.
  No permanent RAM growth; KCORE +30 words over v884.
- [ ] v885 remaining: permanent MM/FILE provider rejection cannot
  guarantee eventual reclamation. Define diagnostic/administrative
  recovery policy without an unsafe forced free. Automatic OOM
  victim killing remains disabled.
- [x] v884: slot-0 MM cleanup is now eligible for one throttled
  maintenance turn after 020 octal timer ticks despite another
  runnable user; runnable realtime and disk-swap service retain
  priority, while dormant RT leases cannot starve cleanup.
  Pending is packed into the existing last-reaper-tick word, no
  permanent data growth. Native scheduler fairness and timer-wrap
  test PASS; pre-v884 negative control FAILs as expected. Full-system
  real MM-busy injection with a continuously spinning companion
  PASSes (reaper runs twice with nonempty run queue, 2 busy frees,
  1 pin/unpin pair and normal HUP/TTY completion).
- [x] v884 follow-up: permanently refusing MM/FILE providers now
  have bounded retry frequency in v885. They still retain ownership,
  so reclamation itself remains explicitly unbounded.
- [x] v883: non-current fatal HUP returns success once VM destruction
  commits and u-area release is explicitly deferred under the
  TRANSITION/VM_NONE/word-067 marker; duplicate events and precommit
  failures remain rejected. Native PDP-6 test PASS; old code FAILs
  the negative-control fixture. Full-system non-current MM-busy
  injection PASS (3 frees / 2 busy / 1 pin / 1 unpin / no error);
  the older current-EXIT injected test also PASSes.
- [x] v883/v884 follow-up: prove reaper progress alongside an
  independently spinning user in a booted guest (v884 PASS).
- [ ] v883 follow-up: genuine pre-VM fatal-event error propagation
  in a booted guest and permanent MM/resource-provider failure policy.
- [x] v882 full-system transient MM_BUSY test for process-owned EXIT:
  real u-area MM pin, two rejected frees, clock-yielded idle cleanup,
  one unpin, and eventual normal session/HUP/TTY HALT PASS. All
  instrumentation stays in the disposable daimos-testkit kernel
  overlay, zero production resident words.
- [x] v882/v883 follow-up: non-current fatal teardown now reports
  committed delivery correctly, and a full-system MM-busy/HUP
  fault-injection regression PASSes. Sustained continuously runnable
  companion progress and permanent-MM-error policy remain open
  under the v883 follow-up above.
- [x] v881 defers a failed post-VM stable u-area MM free to the
  clock-throttled slot-0 reaper, using unused u-area word 067 for
  marker/status. No per-process queue, unchanged session bits,
  one new permanent retry-clock word. Native retry/compiled EXIT
  and old teardown tests PASS; v882/v883 now demonstrate genuine
  full-system MM-busy recovery for current and non-current victims.
  Continuously runnable competing-user PI6 fairness remains open.
  The idle-only mechanism is not a bounded OOM killer.
- [x] v880 makes disk-swap detach ownership-preserving under a
  missing/invalid backing span: the complete bitmap run is verified
  before free, and failed detach prevents VM_SPACE_NONE publication.
  Native VM and actual C detach tests PASS with expected failure
  against pre-v880 code. The SWAP MRES verifier is reused through
  a patched KCORE entry (+16 resident words); full-system ordinary
  session HUP/TTY regression also PASSes. Post-VM u-area failure is
  handled by the v881/v882/v883 deferred-finalizer series.
- [ ] v879: APR and post-commit EXEC now use a process-owned kernel
  FILE-close continuation with timer-yielded retries; duplicate fatal
  events, native direct swap-out/exchange and OOM selection exclude
  that owner. VM_SPACE_NONE (-1) is now correctly excluded by native/C
  scheduling unless it has a tagged executive continuation. Native
  selection, direct swap guard and APR-safe restoration PASS; the
  pre-change swap guard FAILs the negative control. Obtain genuine
  full-system failing-provider/PI6/clock acceptance (the ordinary
  full-system session regression already PASSes), bound *total*
  cleanup latency or define unrecoverable-provider policy, and fix
  post-VM-destroy finalization errors before enabling OOM termination.
- [x] v878: post-commit fatal EXEC now enters FILE cleanup; v879 moves
  it onto the process-owned retry continuation. Native EXEC and prior
  pre-v878 negative control passed. Full-system injected failure stays
  open under v879, not under this completed wiring prerequisite.
- [ ] v877: ordinary full-system session teardown PASSes on the post-v876
  code under headless CTY/DPY test isolation. This does *not* validate
  injected HUP/VM failures during the full-system run or guarantee
  asynchronous cleanup after a nonreturning APR/EXEC fault.
- [ ] v876 now excludes a victim from scheduling before possibly sleeping
  session-HUP callbacks, and preserves RT ownership on failed HUP or VM
  destruction. Native failure/rollback test PASS; old-code negative
  control FAIL as expected. Prove remaining teardown stages can either
  complete or keep a nonreturning process safely quarantined before
  enabling any OOM kill.
- [ ] v875 delays controlling-TTY detach until all session HUP calls
  succeed; failed HUP leaves original attachment for retry. Native
  fault-injection and pre-change negative control PASS/expected FAIL.
  Earlier members may still exit before a later HUP failure, so audit
  recursive teardown and nonreturning fatal EXIT before OOM killing.
- [ ] v874 closes the Type 340 replacement old-extent ownership leak.
  Native transactional rollback tests PASS, including a failing old-code
  negative control. Still verify replay after an injected free failure
  in a full-system DPY guest, and explicitly test the MM invariant HALT
  when even the new allocation cannot be rolled back.
- [ ] v873: the KCC full-system named-FIFO workload times out both
  before v870 (v869) and on v871 after D6FS startup. Diagnose guest
  progress and the failing stage; do not weaken the test or claim the
  v870 native MM-rollback check provides full FIFO acceptance.
- [ ] v871 fixes DPY CLOSE error propagation and loss of the Type 340
  raw-list base after failed MM release (native fault injection PASS).
  Verify native refresh after a failed stop before declaring Type 340
  teardown fully allocation-free and retry-safe.
- [ ] v870 fixes final PIPE/FIFO close retry after an MM free error;
  native fault-injection PASS. Audit and prove remaining release helpers,
  and test full process cleanup with PIPE descriptors before activating
  OOM victim termination.
- [ ] v868-v869 now preserve current-process FILE resources on ordinary
  close failure and reject `proc_finish_slot` while any FD remains live.
  Native PDP-6 branch tests PASS. Resolve nonreturning APR/EXEC fatal exits
  and other cleanup failures without orphaning resources or hanging before
  using this path as an OOM killer.
- [ ] v867 closes a resource-orphaning hole in fatal-other cleanup: a failed
  VFS sync now prevents freeing the victim u-area. It does not guarantee
  OOM progress. Finish the allocation-free termination graph, including
  failed/sleeping provider operations, current-process exit and nested
  session HUP, before enabling automatic kills.
- [ ] Apply the same allocation-free, static KCORE OOM policy to **every
  memory allocation or growth requested by an already-running process**,
  including BRK/heap, process-VM expansion, and other user-triggered MM
  allocations. Do not limit OOM recovery to EXEC/SYS_RUN admission.
- [ ] First retry normal allocation after packing/compaction and permitted
  reclaim. If sufficient realizable, aligned free RAM exists but packing
  fails, fix the allocator regression; do not use OOM to hide it. Measure
  available words, alignment, pinning and fragmented gaps before deciding.
- [ ] If the running request still cannot be satisfied because resources
  are genuinely exhausted, invoke the **minimal static KCORE OOM path**.
  Terminate the requesting process when required rather than letting it
  spin or leave a half-updated address space. All kill, cleanup, backing
  release, parent notification, and zombie/WAIT bookkeeping must run
  without new RAM, new backing sectors, dynamic buffers, or removable
  modules; never swap in the victim merely to terminate it.
- [ ] Bound victim selection and retry attempts, preserve protection for
  INIT, kernel, current in-flight transitions, pinned or privileged critical
  processes where applicable, and define the requesting-process fatal case
  explicitly. Verify failure atomicity and zero-allocation teardown with
  native daimos-testkit fault injection for both RUN and live BRK/heap
  expansion, with RAM-only, backing-only, simultaneous exhaustion and
  no-killable-victim tests. OOM must not require a victim-held provider
  lock to make progress.


The **fresh GCC linked/package sizes** for the current source are:

| PDP-6 profile | KCORE | KCORE + packaged MRES | Permanent endpoint | Below `040000` |
| --- | ---: | ---: | ---: | ---: |
| 32K lowmem TSFS/tape | 8548 | 13463 | 13510 | 2873 free |
| 32K lowmem D6FS/disk | 8548 | 13539 | 13586 | 2797 free |
| 256K full/auto | 8548 | 18271 | 18318 | **1934 over** |

The full/auto `permanent-size` target **fails** its `040000` limit; its
multi-root endpoint is 18351 (1967 over). Package size includes optional
MRES components that may not be simultaneously installed; it is **not** a
measured runtime RAM high-water mark. Dynamic MM allocations, actual loaded
MRES combinations, stacks, and process footprints must be measured separately.
The strict lowmem KCORE `<020000` target still requires at least 357 words
removed (current KCORE 8548 versus octal 020000 = 8192).

- [ ] Resolve or correctly gate the **full/auto packaged permanent-image
  overflow** (1934 words single-root, 1967 multi-root). Audit which MRES
  packages really require permanent residency before changing the loader;
  do not hide genuinely always-resident RAM as a nominally removable module.
- [ ] Measure runtime physical MM allocations and peak working set for both
  lowmem roots and the actual full/auto loaded-module combinations; keep
  linked-package totals separate from peak resident RAM.

### Remaining XIP integration (active checklist; v201-v219 tests completed)

- [ ] Add an **opt-in root-only** RUN/EXEC admission branch for the distinct
  raw MEMFS XIP format (`PURE` header, mode no-write, exact aligned resident
  base), using MRES op `027` and retaining its pin through process lifetime.
  Ordinary DXR/PURE executables must continue through the normal loader.
- [ ] Enforce the **no-user-memory-write contract**, not just a PURE header:
  only admit the restricted verified instruction/exit class, refuse any
  unsupported or mutable image, and explicitly define the credential/trust
  boundary. Never treat a user-writeable PURE file as trusted executable.
- [ ] Add no-stack/no-argv/no-environment XIP startup/entry; bypass the
  ordinary `vm_space_startup` write into user memory, initialize APR and
  scheduler-owned state without modifying shared MEMFS bytes. Reject
  requests needing a private stack, data, BSS, or writable brk.
- [ ] Keep XIP mappings unswappable and non-growable. Verify MM pins,
  credential inheritance, provider unmount/reclaim resistance, scheduler
  transitions, and the existing process-descriptor provenance under load.
- [ ] Complete failure-safe RUN/EXEC replacement (admission rollback,
  no partial process publication, old/new image replacement, unpin/retry
  on failures), plus normal EXIT/wait/reap teardown via the tested VM path.
- [ ] Add **actual production-kernel** SIMH and PDP6-AAP tests for root
  launch, scheduled user instruction fetch, syscall EXIT, WAIT/reap,
  reuse/unmount, denied non-root/XIP-bad-image launches, swap/BRK refusal,
  and EXEC success/failure rollback. Existing v216 tests use stubs, not
  a scheduled production process. Measure KCORE and dynamic RAM on both.
- [ ] Keep XIP disabled for production until these tests pass; 32K lowmem
  must not acquire an automatic MEMFS dependency just to enable XIP.

### P0: efficient physical RAM management using native PDP-6 APR (pre-sweep gate)

POLICY REVISED 2026-10-27 v522: 512-word process mapping is no longer a
target. Preserve the PDP-6 APR's native `02000`-octal (1024 decimal word)
relocation and protection granularity. Never bypass, simulate or weaken APR
to obtain smaller process extents. General MM allocation is already exact
word-granular. Optimize measured physical RAM utilization, allocation
placement, fragmentation, compaction, extent lifetime and reuse, with CPU
cost as the next criterion; reject added metadata or indirection without
proven net benefit. Native 1K process granularity is an accepted outcome.

V524 SEQUENCING (2026-10-27): current fragmentation readings are useful
as correctness diagnostics, not final optimization evidence. First validate
native APR isolation, MM ownership, pin/release, compaction and transactional
swap/BRK failure recovery without resident telemetry. BSTORE, production XIP
and swapless process reclaim change occupancy and lifetimes. After those features
are accepted, measure representative 32K/48K/256K physical high-water,
free gaps, APR rounding and fragmentation and only then optimize placement
or compaction based on net physical RAM and CPU measurements.

SOURCE AUDIT 2026-10-27 v521: `mm_alloc()` already requests precise word
counts with alignment 1; `mm_alloc_aligned()` and the extent descriptors do
not impose a global 02000-word allocation quantum. `vm_pdp6.s` and
`vm_pdp6_swap.c` enforce 02000-octal alignment for user APR images.
Therefore a global 512-word allocator conversion would be a regression,
not an optimization. Next measure rounding waste per user VM and enumerate
any software-managed consumers that actually overallocate at 02000 octal.
Only implement reductions where supported by hardware and measurements.

- [ ] After BSTORE/XIP/swapless reclaim integration, measure physical allocation high-water, total free words, largest free
  gap, process APR rounding waste and extent-descriptor overhead on 32K,
  48K and 256K workloads in daimos-testkit.
- [ ] Evaluate placement and compaction against measured fragmentation;
  eliminate only proven redundant allocations or retention. Keep APR
  process granularity intact and avoid new persistent metadata.
- [ ] Run protection, swap, BRK, XIP, reclaim and compaction regression
  tests after any accepted change; record total resident RAM and CPU cost.

## Pre-final-KCORE functional freeze inventory (2026-10-27 v512)

This is the authoritative functional checklist before the final linked-image
RAM sweep.  Historical unchecked v13-v21 mount and v208-v219 XIP substeps
below are evidence, not independent commands to reimplement accepted work.
`SIZEOPT.MD` tracks size-only experiments; `KCORE.MD` and
`PROJECT-STATE.MD` record accepted chronological evidence.  Check source
and testkit immediately before implementing an item.

Release-critical or prerequisite correctness/acceptance:

- [x] Regression row 028 (v580): `kernel-runtime` and `stand-runtime` PASS.
- [x] Regression row 029 (v581–v583): D6FS HDD runtime, two/four-member
  HDD tests and dual-superblock recovery PASS. Existing multimember fixture
  omitted external DSH commands; accepted v582 testkit fixture packages
  nine required binaries without altering the kernel or weakening assertions.
- [x] Regression row 030 runtime-build (v589-v590): the dedicated native
  v586-v588 SIXBIT pathname accidentally encoded `/MONITOR/DEVICES/DTY0/STATS`
  rather than `CTY0/STATS`. Corrected v589 OPEN/READCHAR/CLOSE test passes
  on PDP-6 SIMH. The v590 probe manifest preserves all 65 valid command
  probes (65 PASS, zero FAIL), replacing only the inapplicable CAT/STATS
  oracle with mandatory native character-stream acceptance in runtime-build.
  Complete runtime-build PASS; no kernel source change or optimization.

- [ ] **Preserve and diagnose every future `?RT` boot failure before
  rebuilding/resetting media.** Save SIMH transcript, exact boot image and
  mutable media, selected-root/handoff evidence and filesystem state;
  distinguish root selection, root mount/validation, and missing/nonregular
  `/SYSTEM/INIT`. Reproduce against the failing image and repair DAIMOS if
  defective. A clean-image rebuild is only a comparison, not a fix.

- [ ] **P0 efficient native-APR memory management**: preserve 1024-word
  PDP-6 process mapping alignment, measure real waste and fragmentation,
  improve placement/compaction/retention only where total RAM decreases,
  and verify execution cost and regressions in daimos-testkit.
- [ ] Settable UTC wall clock without PCLK: privileged SETTIME, DATE SET,
  safe PCLK presence detection, APR 50/60-Hz calendar accumulation, atomic
  GETTIME/SETTIME, and TIME36 timestamp preservation.
  Native milestone v592-v593: absent SIMH PCLK now produces unknown TIME36
  rather than a fabricated date; native PCLK-on/off GETTIME tests PASS.
  v595-v599: shared TIME36 service now falls back to APR-tick-driven UTC
  when PCLK is absent; root-only SETTIME validates TIME36 and rejects
  attempts to modify physical PCLK. Native PDP-6 SETTIME acceptance PASS
  in both PCLK modes. SETTIME extension opcode moved from the conflicting
  PROCCTL SETGID 025 to unused 037. Native ROOT LOGIN on an unset clock
  invokes DATE, initializes UTC, then enters DSH (testkit v597 PASS).
  Native v604 leap-day rollover, invalid calendar and non-root SETTIME
  denial PASS in both PCLK modes with restored pristine boot media.
  The targeted v602 SETTIME handler size pass saves three linked lowmem
  words (KCORE+MRES 15983 -> 15980), with no new data structure.
  50-Hz configuration, concurrency/timestamp stress and physical-device
  presence policy remain acceptance work; repeated-boot ?RT forensics
  are tracked separately.
  v606 source audit: PDP-6 APR CONI exposes clock flag/enable, **not**
  line frequency. SIMH's `rtc_tps` is simulator-internal. No automatic
  50/60-Hz discovery is possible from APR ticks alone; use a verified
  independent reference or explicit platform/boot configuration instead.
  v607: `WALL_CLOCK_HZ=50|60` hardcodes the operator's installation-time
  choice in the software UTC accumulator, default 60. Both cross-builds
  and lowmem links pass; invalid rate rejected. Native v608 configured
  50- and 60-tick UTC rollover/privilege tests PASS. Physical 50-Hz
  accuracy and system-wide scheduler/device timeout rate audit remain
  open; SIMH's native APR source itself stays 60 Hz.
  Native v609 D6FS mtime regression PASS at 60 Hz, both PCLK modes;
  verifies timestamp propagation after SETTIME and leap-day rollover.
  Native rerun with WALL_CLOCK_HZ=50 PASS in both PCLK modes as well.
  Source concurrency audit found PI-masked snapshots and guarded
  publication; native v612 multiprocess GETTIME/SETTIME race PASS at
  50 and 60 configured Hz, with three concurrent reader processes and
  a privileged clock writer. Native v610 MEMFS mtime
  regression PASS (both PCLK modes, leap-day rollover); `memfs_touch_slot`
  now preserves caller-scratch AC5 across the software clock gateway.
  Native v610 rerun at WALL_CLOCK_HZ=50 PASS in both PCLK modes too.
- [ ] Finish tested, non-destructive AUXSTORE swap provisioning (MKSWAP),
  including explicit destructive confirmation and existing-format/overlap
  protections. No automatic formatting at boot.
- [ ] Complete combined tape+drum, disk+drum, BLOCKSET member and non-root
  storage inventories, including other safely discoverable DRM units, and
  real supported mixed-provider mount/capacity tests.
- [x] Revalidate swapped-process MonitorFS NAME/CMDLINE/ENVIRONMENT reads
  and subsequent wake/WAIT: fresh 48K PDP-6 SIMH testkit v562 PASS
  (2026-10-28). The earlier v74 restore stall no longer reproduces.
- [x] Recheck GCC multiprocess timer preemption using the unchanged native
  five-user scheduler test with SIMH rate 8M: 2026-10-28 fresh run PASS,
  all users start before slot-1 checkpoint `A`; process-context and disk
  sleep/wakeup assertions also pass. No test-oracle or kernel modification.
- [ ] Rerun remaining GCC/KCC grouped acceptance; the focused GCC
  preemption PASS does not establish that the complete groups pass.
- [ ] Complete additional KINIT failure-path stack reserve checks;
  32K kernel-only and 96K/256K full-profile DSH acceptance are PASS.
  Repair stale MM
  testkit linking and secondary DTFS/TSFS mount test assumptions.
  v638 recovered 30 transient KINIT words: 32K image end `077236` ->
  `077200` at unchanged octal `0600` stack reserve. Native TSFS+AUXSTORE
  v493 and independent PDP-6 32K diagnostic/INIT boot PASS; the image
  exactly meets the limit without extra margin. Failure-path stack
  watermark, kernel-only and other-profile acceptance remain open.
  v639 fixes the watermark fill/scan exclusive-endpoint off-by-one; a
  native 48K watermark-enabled boot PASS reached INIT V1. The 32K
  failure-path stack high-water measurement remains outstanding.
  v640 adds an instrumented fatal-HALT stack sample. A native 48K
  invalid-root-selector test measured octal `0164` (116 decimal) words
  against the unchanged octal `0600` (384 decimal) reserve. Other
  failure paths remain open. v641 native 32K kernel-only boot PASS:
  SIMH breaks at the map-derived `proc_boot_init` entry after KINIT
  initializes DTC/DSK/DTFS, before running `/SYSTEM/INIT` or DSH.
  v642 additionally verifies the same instrumented lowmem KINIT image
  at 48K, 96K and 256K core: all three native SIMH kernel-only runs
  PASS through the process-bootstrap boundary. These are not full
  profile or DSH functional acceptance tests.
  v643 native GCC 96K full-profile disk-root KINIT PASS: image end
  `107421` octal, full device markers and `proc_boot_init` breakpoint.
  The 96K userland/DSH, KCC and 256K full-profile gates remain open.
  v644: 96K KCC full-profile disk-root KINIT also PASS, with image
  end `113573` octal and process-init breakpoint at `066510`. KCC
  96K kernel-initialization gate is closed; userland/DSH and 256K
  functional gates remain open.
  v645: native 256K KCC full-profile KINIT PASS; exact memory banner,
  DSK/DRM and filesystem markers and process-init breakpoint verified.
  256K GCC and interactive DSH functional acceptance remain open.
  v646 closes the 256K GCC full-profile kernel-only boot gate with a
  native SIMH PASS at `proc_boot_init` (`063067`), image end `107421`.
  Full-profile 96K/256K GCC/KCC kernel initialization is complete;
  interactive DSH and remaining KINIT failure paths stay open.
  v647: interactive ROOT login, DSH ECHO, external TRUE, STATUS:0,
  orderly HALT, and postflight CLEAN D6FS all PASS for GCC and KCC
  at both 96K and 256K. Source media must be CLEAN before reuse;
  a stop at `proc_boot_init` leaves writable root DIRTY, so it is
  unsuitable as a second-boot fixture. See
  `design/kinit-dsh-full-profile-v647.md`.
  v648: instrumented 48K tape-root boot with explicit disk selector
  `1` reaches HALT with stack high-water `0164` octal (116 words),
  below the unchanged `0600` reserve. This adds selector coverage;
  independent D6FS mount and MM allocation failures remain open.
  v649: independent D6FS superblock-validation failure is now measured:
  native 96K GCC full-profile boot with both superblock magic words
  invalidated in disposable disk copies emits `?RT`, HALTs and records
  `0172` octal (122 words) against `0600` reserve. Allocation, I/O
  and missing-INIT error paths remain unmeasured.
  v650: targeted native KINIT idle-stack `mm_alloc` failure PASS:
  inject only an impossible request into simulator RAM after Stage1
  loads, then measure fatal HALT high-water `0172` octal (122 words)
  against `0600` reserve. Disk media remains CLEAN. Independent
  I/O and missing-INIT fatal paths remain open.
  v651: BLOCKSET read-service I/O error injection during full-profile
  D6FS root mount PASS: `?RT`, HALT, `0172` octal (122-word) stack
  watermark, CLEAN source media before/after. Physical controller
  errors and absent/nonregular `/SYSTEM/INIT` still need coverage.
  v652: KINIT bootstrap reserve reduced from `0600` to `0400` octal
  (256 words); prior failure-path high-water maximum is `0172`
  octal (122 words). Fresh native 32K lowmem watermark-enabled image
  boots through `proc_boot_init`, and invalid-root HALT records `0164`.
  v653: missing and nonregular `/SYSTEM/INIT` acceptance PASS after
  successful D6FS initialization on native PDP-6 SIMH, using clean
  disposable disk copies. Physical-controller errors remain a
  separate coverage item.
  v655: physical DSK0 media-loss after Stage1 now PASS in native 96K
  GCC full-profile SIMH: KINIT HALT, validated watermark `0172`
  octal (122 words) inside the `0400` reserve. A prior `0577` reading
  was invalid because a reused build mixed watermark-disabled KINIT C
  with watermark-enabled assembly. KINIT reserve/watermark changes now
  invalidate both generated files; mode-toggle 1/0/1 regression PASS.
  Other transient physical-controller error modes remain separate.
  Remaining `/SYSTEM/INIT` validation is separate from this size change.

Architecture-changing features requiring an explicit pre-sweep decision:

The original KCORE decisions were 1, 3, 4 and 5; decision 2 is
explicitly POST-KCORE. v1488 audit: decision 1 is complete as v1464,
decision 4 is complete as v1469-v1485, and decisions 3 (remaining
storage service gaps) and 5 (swapless PURE) remain OPEN. The historical
v657-v670 notes below remain evidence, not current kernel blockers.

- [x] **DECISION 1 IMPLEMENTED (2026-11-10 v1464):** Grouped block
  streaming decoder and atomic producer/consumer cutover are deployed
  in Stage-1, KCORE, EXEC/PURE, TSFS, and image-building tools.
  The old D6LZ36 production format and permanent decoder are gone.
  This is no longer an open 0.9 KCORE feature gate.
- [ ] **DECISION 3 ACCEPTED (2026-10-27):** Complete the remaining BSTORE
  migration to generic shared backing/BCACHE/LOGSTORE BEFORE final KCORE
  optimization as a version 0.9 prerequisite. Audit remaining gaps against
  the accepted AUXST1/BACKSTORE implementation; preserve raw AUXSTORE and
  existing D6FS-root validation contracts. Verify all production consumers,
  removable MRES boundaries, dynamic RAM and native rollback/recovery before
  freezing the service ABI. Decision recorded; implementation not complete.
  v657 first feature increment: shared bounded raw AUXSTORE region
  read/write interface for BACKSTORE/LOGSTORE/CACHESTORE landed;
  LOGSTORE bootstrap I/O now delegates through that interface.
  Host partition-bounds regression and earlier AUXSTORE choice/overlap
  checks PASS. BACKSTORE conversion, recovery and native acceptance
  remain open; no KCORE optimization or completion claim.
  v658: direct DEVICE regions are now named/configured separately from
  multi-member VOLUME tails, keeping the legacy BLOCKSET entry alias
  and one shared physical DSK/DRM transfer path. Source routing
  regression PASS. Remaining full storage feature cutover is open.
  v659: a configured multi-member VOLUME now fails I/O if its mapper
  is unavailable; it no longer silently falls back to DEVICE 0
  addressing. Source regression and fresh native 32K D6FS boot PASS.
  Native missing-mapper fault injection remains open.
  v660: the existing three resident backing-region state words now
  have explicit DEVICE/VOLUME/REGION source names, while legacy
  symbol aliases share exactly the same storage and entry point.
  Process swap boot uses `backing_region_blocks()`.
  v661: LOG boot I/O directly selects the bounded AUXSTORE LOG REGION;
  redundant LOG-only AUXSTORE wrappers were removed. Legacy root-backed
  LOG remains supported. Persistent LOG recovery and native fault
  injection remain independent acceptance work.
  v662: MonitorFS SWAP statistics uses `backing_region_blocks`, as
  process swap already does. Legacy BLOCKSET query/transfer symbols
  remain ABI aliases; runtime mapping fault injection remains open.
  v663: native multi-member D6FS HDD suite PASS on two-/four-member
  volumes after rebuilding AArch64 host relay helpers. New testkit
  v663 launcher builds versioned native helpers and runs the unchanged
  v582 suite.
  v664: targeted native PDP-6 BACKING gateway test PASS: unavailable
  multi-member mapper fails without device I/O; direct DEVICE still
  succeeds. Full-system in-flight failure tests remain outstanding.
  v665: native PDP-6 partial BACKING write test PASS (second write
  fails, third never issued). Swap/MEMFS publication-order source
  regression PASS. Full-system in-flight fault and late MM rollback
  remain open; BACKING media writes are not physically atomic.
  v666: late swap-out MM unpin/free error branches audited; source
  regression PASS. Real native MM failure injection and complete
  scheduler/recovery acceptance remain outstanding.
  v667: subsequent audit found `mm_unpin` errors were not handled
  safely by normal rollback. Both unpin failure paths now halt via
  the existing storage invariant instead of unlocking the process;
  v667 source check and native PDP-6 branch fault tests PASS.
  Full-system MM fault injection remains open.
  v668: isolated native PDP-6 `mm_free` failure after successful unpin
  PASS: resident process record preserved, provisional BACKING released,
  staging word and TRANSITION cleared. Full scheduler fault injection
  and live I/O recovery remain open.
  v669: full-system swap scheduler regression is BLOCKED: historic
  v298 expects `proc_swap_out` in KCORE rather than SWAP MRES; updated
  v669 simulator run reaches startup `?RT` and HALT at `065206` without
  workload completion. Determine root cause, then run fault injection.
  v670: native D6FS boot trace confirms pre-VFS failure after
  BLOCKSET discovery/reads and superblock select entry. Capture
  precise failing return/check and root media fields; do not
  infer swap failure or claim full-system rollback acceptance.
- [x] **DECISION 4 IMPLEMENTED (2026-11-11 v1469-v1485):** Production
  restricted MEMFS-only non-writing privileged XIP RUN/EXEC is accepted,
  including scheduled 128K/192K/256K physical guests, MM pin lifecycle,
  ownership/BRK/swap exclusion, injected rollback and MEMFS snapshot
  recovery. Minimum physical MEMFS/XIP RAM is 96K, and no 32K/48K
  XIP acceptance is expected or required.
- [ ] **DECISION 5 REPLACED (2026-11-09 v1375):** Complete swapless-only
  PURE image dropping and opt-in in-RAM mutable-state compression, with
  exact MM/process ownership and full native lifecycle acceptance before
  the final KCORE size sweep. Disk-swap compression is not a requirement.
  See the active v1375 checklist above and the subject design in
  `pdp10-doc/KERNEL/SWAPLESS-MEMORY.MD`.

### Deferred post-KCORE work: unified TSFS (not a KCORE gate)

- [ ] **DECISION 2 UPDATED (2026-11-09 v1374):** After KCORE feature
  completion, regression acceptance and optimization, implement **one
  unified TSFS** namespace/read engine and the new root/auxiliary profiles.
  No legacy DTFS/TSFS or TENEX/ITS compatibility is needed. The historical
  sharded-metadata proposal is input for measuring metadata locality,
  not a mandated format. Keep FS-specific work in the provider MRES
  and host tools; audit necessary KCORE/VFS/KINIT ABI changes separately.
  Implement and test before 0.9, not as a current KCORE sweep gate.

Explicitly outside the 0.9 final KCORE functional freeze unless a verified
release dependency is found: new dynamic/unloadable MRES framework, user
overlays, runtime module movement, hardware pager,
Type-340 input/chess and full 32K DSH
command execution (separate DSH shrink effort). Never auto-load MEMFS
based on AUXSTORE presence. Do not treat KCORE `proc_swap`, `bcache`,
`fs_mres`, p10fold or dead-data audits as functional prerequisites.

The **online D6FS packer and other agreed storage/admin extensions are
required for 0.9**, not post-0.9. This release-scope correction does not
bring the deferred DTFS/TSFS work into the current KCORE feature sequence.
Unified TSFS is explicitly scheduled after completion of KCORE.
Implement these features before release acceptance; KCORE feature
implementation takes precedence over any new size-optimization sweep.

## TSFS-root auxiliary backing and command tools (2026-10-27)

### P-1 KCORE TODO reconciliation (v511)

Before beginning any unchecked KCORE item, inspect current source, latest
commits, linked maps and tests to decide whether it is already implemented,
superseded, or no longer needed. Preserve historical evidence in Git; do
not implement stale proposals. `SIZEOPT.MD` is the active, ordered RAM-size
worklist; `KCORE.MD` and `PROJECT-STATE.MD` are chronological evidence.

Run kernel-only 32K boot/initialization tests, including no-DSH device and
swap tests, at 32K. Functional tests requiring DSH use 48K, 96K or 256K.
The complete ROOT -> DSH -> command workload at 32K is explicitly deferred
to a separate DSH optimization run and is not a gate for this sweep.
Keep the KCORE below-020000 size goal as a tracked, deferred acceptance
milestone while exhausting verified no-growth RAM optimizations.

- [ ] DEFERRED: design explicit loadable MRES modules only after KINIT size
  optimization makes module loading feasible. Do not automatically load
  MEMFS when a backingstore is detected. The earlier automatic lowmem
  MEMFS proposal was cancelled on 2026-10-27.

- [x] Keep the 32K `lowmem` profile free of MEMFS: it currently omits the
  MEMFS MRES and `memfs_minit` through `KINIT_FULL=0`, and deliberately
  leaves `/TEMP` unmounted as MEMFS. Do not introduce a MEMFS dependency
  into tape-only optional-swap discovery.
- [ ] Extend the existing **DATE** utility (`userland/exec/COMMANDS`,
  `commands.c`, and `manual/DATE.SIXMD`) to **set** as well as display the
  UTC date/time. DATE must be able to initialize/correct wall time when the
  optional PCLK device is absent; require a privileged SETTIME operation,
  strict calendar validation (including leap years), and native 32K TSFS-root
  coverage. Preserve the existing no-argument display behavior.
- [ ] Add a **settable software wall clock** driven by the existing APR
  50/60-Hz scheduler-clock interrupt when PCLK is unavailable. Detect PCLK
  availability safely at boot in KINIT; never blindly issue PCLK I/O against
  absent hardware. Advance the UTC calendar clock using the actual tick
  frequency with correct subsecond accumulation and rollover (including
  day/month/year and leap years); handle concurrent GETTIME and SETTIME
  atomically. Keep the 50/60-Hz monotonic scheduler clock independent of
  wall-clock adjustments, preserve TIME36 encoding and existing GETTIME/
  filesystem timestamp semantics, and minimize KCORE resident RAM/CPU cost.
  Test both PCLK-present and PCLK-absent boots and both 50/60-Hz settings
  exclusively in daimos-testkit; document behavior after passing tests.
- [ ] Add **MKSWAP** as a userspace provisioning utility for deliberately
  selected DSK/DRM swap and optional LOGSTORE reserved regions. Provide a
  non-destructive inspection/dry-run mode, exact controller/unit/range
  identification, explicit destructive confirmation, and strong overlap and
  existing-format checks. No automatic formatting or overwrite at boot;
  INIT may offer an interactive provisioning action, but unattended boot
  must safely continue with no swap.
- [x] Reject unsafe AUXST1 overlap/range layouts without installing D6FS;
  preserve unformatted optional D6FS partition compatibility (v507-v510).
  D6FS-root dual-superblock and BLOCKSET validation already exists. The
  proposed generic KINIT-only superblock parser is **superseded**: add
  validation for a specifically selected non-root D6FS/BLOCKSET only when
  a real consumer needs it; do not parse D6FS for raw AUXSTORE.
- [x] Have KINIT select tape-only (no physical swap) versus tape with validated
  AUXSTORE (DSK/DRM), using existing resident service bindings and backing
  capacity rather than adding a new KCORE boot-mode word. Lowmem DRM was
  enabled in v506; the 32K DSK AUXST1 positive swap regression passed v507.
  The positive 32K TSFS+DRM0 AUXST1 swap/wake/WAIT regression passed v508.
  Continue separate combined-provider/BLOCKSET coverage below.
- [ ] Validate combined tape+drum, disk+drum and complete BLOCKSET member
  inventories when corresponding supported boot profiles exist; verify
  installed MRES and persistent dynamic allocation footprints separately.

## Deferred acceptance gate: strict 32K (not active priority)

- [ ] Pass strict GCC PDP-6 KCORE endpoint **below** octal `020000`.
  Accepted v507 lowmem KCORE = 8548 words, 357 words above the
  strict below-8192 target. Do not merely move required resident
  code to an always-loaded MRES to mask permanent RAM usage.
- [ ] Keep native 32K **boot without DSH** and KINIT stack/headroom checks.
  v503-v508 passed native 32K TSFS/AUXST1 tests with a 384-word KINIT
  stack; 129/136/201-word highwaters are not exhaustive failure-path
  maxima. DSH-dependent commands, filesystem and process-pressure tests
  run at 48K, 96K or 256K in this sweep. The full 32K ROOT -> DSH ->
  command acceptance is deferred to a separate DSH optimization effort.
- [ ] Repeat all size and correctness gates after each accepted source
  reduction; fix compiler/assembler/kernel correctness bugs first.

**Strict 32K acceptance is explicitly deferred; current storage work and
source-level RAM optimization continue independently.** See
`design/p0-strict-32k-gates-20261027-v474.md`.

## XIP v219 MRES gateway

- [x] Publish raw PURE XIP admission through existing private MEMFS MRES
  register gateway (op 027), tested on SIMH and AAP, with no new KCORE slot.
- [ ] Connect a privileged loader path only with zero argv/environment and
  no writes to the shared VM during RUN/EXEC startup.
- [ ] Test scheduled process EXIT and low-memory EXEC rollback end to end;
  reject any trust/pin failure before changing process VM state.

## XIP restricted linker producer v218

- [x] Implement an opt-in, distinct dlink raw XIP output mode reusing PURE
  and existing relocation resolution; reject writable/op-unsafe images.
- [x] Cover exact format, alignment padding, plain DXR compatibility,
  writable instructions, alternative UUOs, DATA/BSS, and invalid options
  in daimos-testkit host regressions.
- [ ] Add an opt-in **privileged** EXEC/RUN path for raw XIP with no argv,
  environment, writable startup, or stack; preserve ordinary PURE/DXR.
- [ ] Verify a production scheduled RUN/EXIT and EXEC replacement rollback
  on both PDP-6 simulators before declaring XIP runtime enabled.

## MEMFS XIP PURE reuse v217

- [x] Require the existing PURE flag in the distinct raw-XIP file header;
  reject absent PURE without leaving a pin. No new permanent process state.
- [x] Implement the restricted raw-image producer for statically pure code
  (v218), preserving logical origin 020 without loader-time relocation;
  its intentionally narrow instruction subset is not general PURE XIP.
- [ ] Add narrowly gated EXEC/RUN use only when both trust and no-writing
  runtime conditions are met; ordinary DXR PURE keeps its existing behavior.

## XIP v216 user-mode lifecycle

- [x] Native-test strict non-writing MEMFS-backed user instruction fetch,
  monitor-UUO handoff, and production VM teardown on SIMH and AAP PDP-6.
- [x] Verify image immutability, pin lifetime, VM state reset, and reclaim
  after termination without adding KCORE or per-process state.
- [ ] Test real scheduler process exit and EXEC replacement/rollback for a
  trusted XIP image, including storage-map state and swap interactions.
- [ ] Define and enforce the privileged no-user-write contract before
  enabling this distinct image format in production EXEC/RUN.

## XIP v215 native VM teardown

- [x] Native-test production `vm_space_destroy` on a pinned MEMFS VM
  descriptor with injected MM-unpin failure and successful cleanup retry.
- [ ] Exercise a full native process exit/EXEC replacement, including MM
  ownership checks and non-writing user startup, on both PDP-6 emulators.

## XIP v214 release rollback proof

- [x] Inject an MM unpin failure through the MEMFS MRES release dispatcher;
  verify retained reference, exact-base ownership, and successful retry.
- [ ] Test staged process VM teardown (not only its MRES dispatcher) and
  both EXEC commit/rollback, with a trusted non-writing user startup.

## MEMFS XIP teardown v213

- [x] Route VM destroy through the movable MEMFS provider's private release
  operation, falling back to MM owner/type validation for regular processes.
- [x] Native-test the release dispatcher and quantify KCORE growth (+9 words).
- [ ] Prove teardown of a staged XIP-backed *process VM* on both emulators,
  including EXEC rollback, then implement verified non-writing startup.
- [ ] Enable separately trusted process XIP launches only after lifecycle
  and swap/heap guards are covered by process-level tests.

## MEMFS XIP v212 provenance

- [x] Implement exact-base pinned MEMFS ownership lookup in MRES without
  a new process descriptor or persistent reference-table word; native tested.
- [x] Route VM destroy to MRES unpin for XIP-owned extents (v213), with unloaded-MRES
  and failure/rollback handling. Never free a MEMFS extent as MM_TYPE_PROCESS.
- [ ] Prevent XIP swap and brk changes; implement non-writing startup and
  actual privileged EXEC/RUN process lifecycle tests.

## MEMFS XIP v211 lifetime primitive

- [x] Implement native-tested exact-base reverse MEMFS XIP unpin for VM
  teardown without a vnode in the three-word process descriptor.
- [ ] Add explicit process VM provenance, swap exclusion and brk rejection,
  then call the reverse release instead of `mm_free(MM_TYPE_PROCESS)`.
- [ ] Validate non-writing startup and actual EXEC/RUN lifetime on PDP-6.

## MEMFS XIP admission gate v210

- [x] Require executable/non-writable file mode before XIP pin/admission;
  test mode 0000, 0444, writable mode, and successful 0555 on PDP-6.
- [ ] Establish process owner/provenance and MEMFS unpin on VM teardown;
  exclude XIP allocations from normal process swap and brk growth.
- [ ] Implement a guaranteed non-writing user startup/termination contract,
  then integrate a separately trusted XIP image path into EXEC/RUN.

## MEMFS user APR XIP validation v209

- [x] Validate PDP-6 user-mode APR instruction fetch and physical relocation
  from admitted, pinned MEMFS backing on AAP. Demonstrate that user stores
  also modify backing (test-only). Add repeatable testkit Make target.
- [ ] Implement safe XIP process provenance and teardown/unpin, swap
  exclusion, and non-writing startup without copying the executable.
- [ ] Test a strictly non-writing user program and syscall handoff from
  pinned storage on both SIMH and AAP before production EXEC/RUN admission.

## MEMFS XIP v208

- [x] Execute a real PDP-6 instruction from pinned MEMFS backing on SIMH
  and AAP using a testkit-only executive XCT trampoline.
- [ ] Add user-mode APR instruction-fetch/native process test with an
  explicitly non-writing startup and guaranteed teardown/unpin.
- [ ] Only after this passes enable privilege-gated production EXEC/RUN XIP.

## XIP integration gate v207 (2026-10-23)

- [x] Implement distinct MEMFS raw-image admission with stable pinned
  backing and rollback, native-tested on SIMH and AAP.
- [ ] Implement privileged non-writing image contract, VM MEMFS provenance,
  swap exclusion, and startup/teardown without shared-memory stores.
- [ ] Only then enable the new image kind through the existing EXEC/RUN
  loading interface and test native process launches on SIMH and AAP.

## Post-optimization MEMFS XIP safety gate v206

- [x] Audit optimized EXEC/RUN VM call sites and establish linked KCORE size.
- [ ] Implement restricted image admission and non-writing startup: current
  `vm_space_startup` writes into memory even for empty argv/envp.
- [ ] Add VM MEMFS ownership and unpin-on-teardown; reject ordinary swap,
  writable break growth, and mutable text before switching physical base.
- [ ] Only then connect the MEMFS XIP path and test actual launches on both
  native PDP-6 simulators. Do not substitute MEMFS base in ordinary DXR.

## MEMFS XIP v204 (2026-10-23)

- [x] Native aligned-allocation `MM_ERR_FRAGMENTED` failure injection on
  SIMH and AAP, followed by successful retry without leaked storage.
- [ ] Run real MM fragmentation/compaction pressure tests and measure wasted
  free words caused by `02000` alignment.
- [ ] Integrate safe XIP VM provenance, process teardown and image admission;
  do not enable ordinary DXR shared writable images.

## MEMFS XIP alignment milestone v203 (2026-10-23)

- [x] Request 02000 alignment for dedicated large MEMFS allocations; both
  SIMH and AAP MEMFS pinning target fixtures PASS.
- [ ] Test aligned allocation fragmentation under pressure, then implement
  executable admission and an XIP-aware VM teardown without changing normal
  EXEC/RUN behavior.

## MEMFS XIP implementation progress (2026-10-23 v202)

- [x] Add per-file XIP pin/refcount reservation with existing MM chunk pins,
  plus write/truncate/reclaim/unmount exclusion. Target MEMFS/FIFO fixture
  passes on SIMH and AAP; ordinary EXEC/RUN remains unchanged.
- [ ] Add dedicated `02000`-aligned MEMFS image allocation, image admission,
  VM provenance/lifetime and read-only execution contract; then validate
  native XIP process behavior on both SIMH and AAP. No general DXR XIP yet.

## MEMFS execute-in-place (XIP) requirement (2026-10-23)

- [ ] Support genuine zero-copy MEMFS executable launch: execute from the
  resident MEMFS backing allocation rather than copying the image into a
  separate process VM allocation. Treat this as a MEMFS design requirement,
  not as completed functionality.
- [ ] Determine a viable PDP-6 mapping strategy before implementing XIP.
  The current APR exposes one contiguous relocated/protected process space
  (02000-word-aligned physical base); it does not independently map shared
  text and private writable data. Do not claim shared-text XIP until private
  data, stack, and relocation semantics are demonstrated.
- [ ] Require executable-backend pin/refcount ownership, immutability while
  mapped, and release at exit/exec failure. Coordinate with MEMFS resize,
  unlink, reclaim/spill, swap, and process teardown; prevent dangling
  physical addresses and mutation of live executable instructions.
- [ ] Check executable-format alignment, header offset, relocation, writable
  text, RT-required mode, and compressed DXR2 behavior. Preserve conventional
  EXEC/RUN fallback for images not eligible for direct execution.
- [ ] Add native tests in daimos-testkit covering repeated/shared launches,
  simultaneous execution, unlink/truncate/write attempts, memory pressure,
  failed EXEC rollback, process exit, and swap interaction. Measure total
  RAM savings and KCORE/MRES word changes before enabling the path.

## V64 validation follow-ups (2026-10-20)

- [x] Swapped-process NAME/CMDLINE/ENVIRONMENT reads in MonitorFS:
  live 48K PDP-6 SIMH v562 verifies exact records for a swapped process,
  its wake, expected child exit and WAIT completion. Existing reader retained;
  no persistent per-process metadata added. The v74 wake/WAIT stall from
  2026-10-20 does not reproduce on the current source. This does not close
  the separate GCC preemption or grouped regression gates.
- [x] Validate the corrected three-root AUTO/login testkit fixture at 256K:
  all-media selects DISK, no-DSK selects TAPE, and drum-only selects DRUM.
  The three individual fresh-media probes pass; tape `CAT /CONFIG/SYSTEM`
  needs the measured 360-second allowance.  Testkit commit `ab730fe`.
  An atomic rerun of the updated three-case script is still separate work.

## GCC PDP-6 functional regression recovery (2026-10-19)

- [x] GCC group 004 fresh targeted run passes all five boot origins,
  four-member DSK root discovery, Stage1 KINIT entry, SIXBIT lifetime,
  and corrected compiler-codegen validation.
- [x] Restore MonitorFS online-device test: detach SIMH DTC3 as well as
  DTC0-DTC2 before disabling the controller. Direct GCC test PASS; all
  other originally failing group-023 subtests pass independently.
- [x] Repair D6FS host checker power-cut regression: current optimal repair
  uses exactly three durable writes, with CLEAN already published after
  cut 3. Verify DIRTY recovery, final CLEAN, no fourth write and ambiguous
  no-modification. Full group-032 host-tools suite PASS, testkit 11feecb.
- [x] Recheck complete regression row 023 (ten storage targets): v564
  repaired the MonitorFS device test's SDL2 crash by applying the existing
  headless SIMH policy, then the unchanged guest oracle passed. V565 ran
  the other nine row-023 targets in matrix order; all PASS (2026-10-28).
  This is row-level evidence, not full GCC/KCC matrix acceptance.
- [x] Recheck matrix row 024 (LOGSTORE and MTC516 online): both targets
  PASS in v566 (2026-10-28); no source or fixture changes.
- [x] Recheck matrix row 025 (DSK270 online and elevator): both targets
  PASS in v567 (2026-10-28); no source or fixture changes.
- [x] Recheck matrix row 026 (DRM236 capacity, online, MINIT/MRES, Stage1
  boot): all four targets PASS in v568–v570. The MINIT-only test fixture
  now uses supported `BOOT_PROFILE=lowmem ROOT=drum` rather than the known
  oversized default full profile; all original module checks remain.
- [x] Row 027 has all ten targets passing in focused, ordered continuation
  runs v573–v579 (not a single clean-sweep invocation). V573 proves
  native DTFS media survives
  two boots and passes second-boot FSCK; disk-root `MOUNT.DTFS` refusal is
  intentional because `dtfs_minit()` installs its MRES only when the selected
  root is DTC. Testkit v573 now asserts this exact supported contract.
  v575 updates the eight-member TSFS scanner regression for its current
  direct-result API (old SYS_TSFS handoff ABI retired). V577 verifies TSFS
  root directory lookup/listing/STAT on a real tape-root SIMH image, replacing
  the obsolete secondary `MOUNT.TSFS` oracle in the matrix. V578 verifies native DTFS format,
  FSCK, secondary mount, root STAT and unmount on a DTC-selected root;
  v579 verifies production reverse-direction single-block read/write.
  Retain strict complete-matrix release sweep as a separate gate.
- [x] Rework secondary DTFS mount acceptance to match the intentionally
  root-selected MRES package: v578 native secondary mount on tape root PASS;
  v573 disk-root mount refusal PASS. Retired MOUNT.TSFS secondary fixture
  replaced by v577 root-selected TSFS directory acceptance. No extra MRES.
- [x] Repair GCC local 9-bit char-array loads across 36-bit words at
  -O1/-O2/-Os; preserve the selected stack word rather than interpreting
  the word address as a byte offset.  Testkit v45 target execution passes
  at -O0/-O1/-O2/-Os, compiler commit df27247.
- [x] Retest full GCC DSH conformance with a fresh 256K disk image:
  115/115 PASS, including HELP ECHO and HELP DAS previously failing.
- [x] Update three outdated D6FS assembly test oracles, TSFS eight-member
  test compilation and SIXBIT headers, and the PDP-6 stack-adjustment
  codegen oracle.  All six focused cases pass, testkit commit 2804838.
- [x] Reproduce GCC multiprocess scheduler preemption on current code:
  the unchanged `daimos-scheduler-multiproc-v1.sh` returned PASS with
  `<abB2cCxxxxdDeEyyyyAxxxxyyyy314D5` (v563). All boot users run before
  slot-1's first `A`. The historical failing sequence is not reproduced
  using the runner's documented 8M SIMH throttle; no compiler/kernel fix
  is attributed to this observation. GCC/KCC group matrix remains open.
- [ ] Address remaining GCC and KCC disk-testkit group failures without
  re-running already passing groups.  32K and size gates are separate.

## Native MAKE V0.9 feature freeze (2026-10-11)

- Native MAKE is feature-complete for V0.9. No new features unless required
  for correctness or demonstrated native self-hosting compatibility.
- Retain SIXBIT-only uppercase parsing and serial execution; -J remains
  an accepted no-op.
- The final RAM sweep removes redundant MAKEFLAGS environment decoding;
  native SIMH verification is tracked in daimos-testkit.

## Native MAKE compatibility follow-up (2026-10-11)

- Recursive -N and inherited -I MAKEFLAGS pass on PDP-6 SIMH (15/15).
- Command-line variable override propagation is deliberately deferred.

- Maintain strict uppercase SIXBIT-native syntax; never add lowercase ASCII
  aliases to native MAKE.

- Target-side PDP-6 SIMH verification passed for .SUFFIXES reset/filtering,
  -r, -t, and accepted-but-serial -j/-J (7/7 probes).
- Target-side .SILENT, .IGNORE, AND -I PASS (10/10 TOTAL SIMH PROBES).
- Target-side .PRECIOUS preservation passed (13/13 TOTAL SIMH PROBES).
- Decide whether actual native makefiles require double-colon rules or
  recursive MAKEFLAGS propagation before accepting additional complexity.
- Verify suffix precedence/order with BSD Make and GNU Make test cases.
- Evaluate portable double-colon rule semantics and automatic MAKEFLAGS
  propagation for recursive native builds.
- Never add parallel execution for native MAKE: -j/-J are compatibility-only.

# DAIMOS TODO

## AAP PDP-6 boot and CTY work (2026-10-18)

- [x] V23B CTY PI4 10-byte packed FIFO: eliminate one-character mailbox
  overwrite; native PDP-6 FIFO and TTY routing tests pass.  AAP diagnostic
  boot with CTY-only INITTAB reaches DSH.  Permanent CTY MRES cost +54 words,
  including +2 resident data words.  Optimize only after proving equivalent
  lossless handoff and PI4 wakeup behavior.
- [x] V23C recover **default** three-session INITTAB startup on AAP without
  reducing the configuration: INIT tests the MINIT-published CTY/DCS/GE
  MonitorFS input endpoint and starts LOGIN only for detected controllers.
  ROOT reaches DSH; no failed respawn/auth lines, correct 255 audited tape
  blocks. C99 source-leaf and AAP evidence tests pass.
- [x] Retest AAP DSH external command execution after ROOT boot: v37
  headless AAP passes ECHO at 300.2 seconds and PWD at 548.2 seconds;
  the former 240-second command timeout was insufficient for DECtape seeks.
  The comparable SIMH Type-551 tape-root path also passes both probes.
  This is 256K functional acceptance only, not 32K or persistence testing.
- [x] Accept the Type-551 PI3 DECtape block-mark search/transfer rewrite:
  rebuilt SIMH with the corrected transport controller booted the 256K
  TSFS root, reached ROOT/DSH, and passed external ECHO and PWD (2/2).
  The independent PI3 driver/router commit is `2c85325`.
- [ ] Complete strict 32K residency/boot and combined-provider persisted
  media acceptance.  The 256K PI3 command test is not either of these gates.


## KINIT storage inventory and VFS mount sizing (2026-10-15)

- [x] V20: fifth PURE backing mount-slot bit in spare credential bit 27,
  transiently staged in SIDL scheduler RH bit0 until RUN allocates u-area.
  Full 18-bit index, 14-bit PURE text length and one-word swap record are
  retained. EXEC rollback, swap raw/compressed/verify, busy guards and mount
  26 native tests pass. No additional process words allocated.
- [x] V21 provider bound phase: replaced D6FS dispatch/DTFS SPACE fixed
  four-slot checks and removed optional TENEX/ITS four-word personality XCT
  table; validated mount 26 and invalid 27. No RAM growth.
- [x] V22 capacity activation: KINIT derives selected physical root members
  and installed provider/controller slots from MINIT state, calls the
  26-slot planner, allocates the exact six-array MM slab, then publishes
  `vfs_mount_limit` last. Native full/reduced inventory tests pass.
- [x] Historical V22 multi-provider experiment deferred post-KCORE (v906):
  exercising simultaneously mounted D6FS/DTFS/TSFS volumes above slot four
  requires an MRES provider matrix the current profiles do not install.
  Keep existing boot-selected mount bounds safe; do not advertise phantom
  slots or make cross-provider TSFS development a KCORE acceptance gate.
- [ ] Reduce the accrued permanent KCORE descriptor/dispatch cost after the
  VFS activation path is stable; no workarounds or unsupported 32K claims.
- [x] Repair stale daimos-testkit `mm-core` host fixture: its Makefile
  still references removed `system/kernel/proc/vm_pdp6.c`. VM lifecycle
  functions now live in native PDP-6 assembly; simply dropping that
  source leaves `vm_extent_move`, `vm_space_can_swap` and
  `vm_space_destroy` unresolved on the host. Implement a verified host
  reference matching current production VM semantics, or restructure
  the fixture to test only available host policy functions. Earlier
  TODO claims of missing `vfs_mount_limit`, `proc_storage_publish_v30`
  and `proc_swap_storage_invariant_v30` definitions were stale: the
  fixture already provides all three. Native PDP-6 swap/EXEC/RUN
  acceptance passes independently. Source audit v655; no outdated
  VM C implementation restored.
  v656: completed in testkit with C99 host reference for the assembly
  VM lifecycle calls and corrected dynamic MM descriptor/process-slot
  expectations. `make mm-core` now reports `mm-v1: PASS`. Production
  VM assembly remains authoritative; MEMFS XIP provider teardown is
  not covered by this host fixture.

- [ ] V19: native FILE mount-extension map now preserves mount IDs up to 26
  across file operations, DUP/DUP2, FLOCK, FIFO substitution, unmount, pipe
  dispatch and RUN fd-map inheritance, with unchanged two-word descriptors
  and u-area size. Restore the +138 permanent-word instruction cost in a
  dedicated source-level KCORE optimization sweep once remaining consumers
  are stable. Next: PURE backing fifth-bit lifetime and optional DTFS
  personality/bounds, then safe boot-selected mount count.

- [x] V18/V19: credentials UID9/GID9/UMASK9 are packed into word 066;
  formerly dedicated umask word 067 can hold all 16 two-bit FILE mount
  extensions at no process-memory increase. Native FILE open/read/write,
  dup/close/lock/FIFO/unmount and RUN-inheritance paths now use word 067;
  separately resolve PURE backing's 37-bit requirement.
  One of credential word 066's nine remaining spare bits may hold PURE's
  additional mount-slot bit if its entire process lifecycle is audited.
  See `design/file-mount-packing-20261016-v18.md`.

- [ ] V17: one-word 26-mount policy and 26-slot six-array slab layout tested
  natively; boot capacity stays four. Next resolve the two-bit FILE descriptor
  mount-ID overlay collision and the mathematically overfull PURE backing
  record (18 index + 5 mount + 14 PURE-text = 37 bits). Preserve all file
  metadata and PURE length semantics; never silently truncate IDs. Audit
  optional TENEX/ITS personality arrays and the remaining provider bounds
  before KINIT activates larger mount counts. Current 32K gate still fails.

- [ ] V16 compact mount policy now handles sixteen slots in the original
  one-word MonitorFS counter alias. Finish native provider bounds and the
  optional foreign DTFS personality dispatch, then connect the boot inventory
  to select a 1..16 limit. Full v16 policy adds 25 instruction words over v15:
  recover the instruction cost in the post-cutover RAM optimization sweep.

- [x] Historical V15 four-slot mount activation proposal superseded by
  the completed V21/V22 boot-selected capacity and native bounds checks.
  The remaining DTFS/TSFS media, shape and optional foreign-personality
  enhancements are post-KCORE. Retain existing provider isolation and
  mount safety while working on generic VFS or D6FS functionality.

- [ ] V14 KCORE audit eliminated nine redundant resident words. See
  `design/kcore-legacy-audit-20261016-v14.md` for protected compatibility
  paths and remaining table candidates. Optimize dynamic-pointer hot paths
  after the other provider tables have migrated; the 32K KCORE end still
  exceeds its strict gate by 1879 words.

- [x] Historical V13 four-slot proposal superseded by V22: D6FS now
  uses the MM slab and VFS boot-selected capacity is live. DTFS/TSFS
  media, shape/extent and format enhancements follow KCORE (v906).
  The older v13 32K KCORE size measurements are historical evidence,
  not current size-sweep authorization.

- [ ] Fix drum D6FS BLOCKSET discovery separately: root selection currently
  discovers a selected DSK/DRM root set, while AUXSTORE discovery checks only
  DRM0 (then DSK0). Audit other drum units and complete/non-root BLOCKSET
  detection without unsafe probes of unattached hardware. Preserve existing
  root-selection and AUXSTORE semantics until native tests establish the fix.
- [ ] For the current VFS capacity plan, count every disk/drum unit outside
  *known* BLOCKSET membership as potentially independently mountable. Do not
  assume an unexamined drum is reserved for swap or belongs to a BLOCKSET.
  Count a known BLOCKSET as one logical filesystem. Exclude independently
  impossible non-root TSFS sets; the boot TSFS root counts as one mount.
- [ ] Audit generic VFS mount tables, mount-ID dispatch, D6FS/MEMFS
  mount state and packed policy against the active boot-selected capacity.
  Preserve existing DTFS/TSFS boot/mount ABI compatibility, but defer new
  DTFS/TSFS provider metadata and format work until after KCORE.
  Enforce a maximum of 16 mounts and size dynamically at boot; a theoretical
  17th simultaneously mountable volume is not supported. Account for future/removable
  media rather than sizing solely to currently mounted filesystems. Audit and
  implementation sequence: `design/vfs-mount-capacity-20261015-v1.md`.
  V12 progress: active target/root arrays now reside in a KINIT-allocated MM
  slab and pass diagnostic native boot; provider-specific reader, media,
  shape/extent state and policy still use four-slot layouts. Retire the
  redundant resident fallback tables and measure the pointer hot paths after
  all consumers are converted; do not raise `vfs_mount_limit` prematurely.
- [ ] Defer block-cache policy changes pending a separate design discussion.


## LIBC ASSEMBLY OPTIMIZATION CANDIDATES — DEFERRED (2026-10-14)

- Investigate `qsort` first: profile the existing generic comparator-based
  implementation, generated PDP-6 code, swap cost, code size, and worst-case
  auxiliary RAM/stack use. Prefer bounded-memory sorting over a literal
  translation of the C algorithm; compare native SIMH timing and words.
- Then evaluate `strcpy`, `strncpy`, `strcat`, `strncat`, `strchr`, and
  `strrchr` for compact byte-pointer assembly. Measure actual savings before
  replacing C implementations.
- Leave `strcspn`, `strtok`, `atoi`, `abs`, `calloc`, `brk`, `sbrk`,
  `stat`, `time`, `exit`, and general heap management in C unless testing
  demonstrates worthwhile memory or performance gains. `calloc` should
  continue to benefit from assembly `memset`.
- Keep regression tests in `pdp10-testkit`. Do not interrupt the ongoing
  upstream semantic-test import for these optimizations.

## D6 CODEC PRODUCTION CUTOVER — STREAMING LOW-CORE DECODER (V057 PLAN)

The production-format v053 encoder/complete-buffer decoder are staged; the
v055 kernel ABI wrapper is a *test milestone*, not the production design.
The active fixed-address D6LZ36 decoder and its callers remain unchanged.
Before any production cutover, replace the decoder **implementation**, not
its external use. The old `system/stand/pdp6/common/decompressor.inc` at
executive address `000060` is the definitive calling-contract reference.

- Implement a directly callable new-format decoder at `000060` with the
  existing resumable register state/entry behavior: AC12 destination cursor,
  AC13 output words remaining, AC3 input cursor, AC4 input words remaining,
  AC14 output origin, and AC0 result (0 complete, +1 NEED_INPUT, -1 invalid).
  Preserve caller-visible state across NEED_INPUT. Keep existing Stage-1
  installation and runtime `d6lz36_decode_vfs` / `d6lz36_decode_buffer`
  interfaces intact; names may change only together with all callers.
- Decode the **new** grouped-control / paired-18-bit-descriptor stream,
  including word copies, zero runs and supported halfword matches, without
  materializing a complete compressed image. The current v053 whole-buffer
  decoder is NOT suitable as a runtime EXEC/VFS replacement.
- A group contains at most 30 tokens. With `m` descriptor-bearing tokens,
  its complete encoded size is `1 + (30 - m) + ceil(m/2) <= 31` 36-bit
  words, including control. Reuse the existing **64-word VFS refill
  window**: retain/compact a truncated group at a window boundary and
  refill the available space. Do not allocate a second group buffer,
  image-sized workspace, or permanent decoder BSS. Validate the 31-word
  bound and the final partial group against actual encoder output.
- Preserve the low-core decoder ABI **directly**. Do not link a generic
  wrapper or permanently retain the v055 AC-save adapter: it exists solely
  for native ABI tests. Implement the encoder's required ABI and callee-save
  behavior directly as well. Measure combined code words, stack high-water
  and persistent RAM; prioritize RAM, then execution speed. Decoder RAM
  should stay close to the old implementation, not grow with image size.
- Validate resumptions at every possible control/descriptor/literal boundary,
  exact compressed-stream consumption, truncated/malformed input, output
  bounds, grouped descriptor padding, odd/even halfword-distance behavior,
  overlap, and direct-buffer decoding. Test against host-produced golden
  streams on native PDP-6 in **daimos-testkit**; do not assume the existing
  v053 decoder's even-distance-only restriction is acceptable.
- Compare the old and replacement `000060` footprints, Stage-1 relocation
  and overlap, VFS stack frames, full linked KCORE text/data/BSS and peak
  process RAM. Avoid extra hot-path dispatch or an unnecessary buffer copy.
- Perform the **single-format atomic cutover** only after tests pass:
  update all five Stage-1 loaders, KINIT compression, host/userland DXR and
  TSFS producers, VFS/EXEC/PURE/TSFS consumers, and replace the dummy kpack
  codec with the real resident encoder/decoder. Rebuild every compressed
  artifact; do not ship mixed D6LZ36/new-format images or runtime fallback.
  Retain disk-swap fallback until packed eviction/restoration and scheduler
  integration pass their own end-to-end tests.

Detailed producer/consumer checklist:
`system/kernel/core/D6CODEC-INTEGRATION-TODO-v054.md`.
Acceptance is **not** claimed by this plan; no production codec or decoder
was activated as part of the TODO update.

## Experimental user overlays and shared text (planned, not implemented)

Implementation plan: `design/user-overlay-shared-text-experiments-20261012-v1.md`.
Start with measured/optional user-space overlays; subsequently benchmark
PDP-6 shared execution areas and optimized private-state copying. Keep
the architecture replaceable by the selected VM backend. No acceptance
or production enablement is claimed.

DATE: 2026-10-07

PROJECT-STATE.MD DEFINES THE CURRENT ARCHITECTURE.  THIS FILE CONTAINS OPEN
WORK ONLY; COMPLETED WORK BELONGS IN GIT HISTORY AND THE CURRENT DOCUMENTATION.
KCORE WORK IS TRACKED EXCLUSIVELY IN KCORE.MD.

## NATIVE KCC BUILD AND INSTALL GATE

- COMPLETED (HOST/STAGING): KCC'S ROOT MAKEFILE NOW HOLDS THE SHARED
  NATIVE MODULE INVENTORIES AND INCLUDES UNIX.MK OR DAIMOS.MK. THE OLD
  PYTHON PHASE MAKEFILE GENERATOR WAS REMOVED. THE DAIMOS STAGER CONVERTS
  THESE TWO SOURCE FILES DIRECTLY INTO SIXBIT-COMPATIBLE S6REC AND STAGES
  THEM ALONG WITH THE INSTALLED BOOTSTRAP. HOST AND CROSS DRY RUNS AND
  STAGING/PREPREREQUISITE GRAPH TESTS PASS.
- PARTIAL PASS: SIMH NATIVE MAKE -Q (DEFAULT/ALL), MAKE -N FOR THE FIRST
  OBJECT, AND MAKE CLEAN PASSED WITH THE SPLIT GRAPH. THE REAL FIRST-OBJECT
  COMPILATION TIMED OUT AFTER REACHING KCC; RETEST IT SEPARATELY.
- REMAINING: RUN A FOCUSED NATIVE MAKE INVALIDATION REGRESSION
  ON THE CONVERTED SHARED GRAPH; MEASURE ITS PEAK MEMORY AND RULE LOOKUP
  TIME. THEN PERFORM A USER-OPERATED NATIVE KCC MAKE ALL/INSTALL ONCE THE
  KPARSE COMPILER CORRECTNESS AND PERFORMANCE GATES ARE CLEARED. DO NOT
  START LONG NATIVE KCC SELF-BUILDS AUTOMATICALLY.
- NEXT FOR NATIVE MAKE: VERIFY GROWTH BEYOND 768 DEPENDENCY LINKS AND 384
  RECIPE LINKS; MEASURE THE NEW SORTED-INDEX RULE LOOKUP ON LARGE GRAPHS
  (PDP-6 OBJECT COMPILES; NATIVE RUNTIME TEST STILL REQUIRED); THEN PROVIDE THE
  REMAINING RECIPE/VARIABLE SYNTAX REQUIRED BY A COMMON KCC MAKEFILE.

- COMPLETED: PDP-6 NATIVE CONSTANT-FOLD OVERFLOW FLAG MASK CORRECTED
  (`JFCL 10`, NOT `JFCL 11`), NATIVE COMPILE REGRESSION PASSES.
  THE STALE INSTALLED HOST KCC MISCOMPILED A SIMPLE INDEXED POINTER
  LOOP; THE REBUILT REPOSITORY HOST COMPILER PASSES A NEW PDP-6
  RUNTIME TEST, AND REBUILDING ALL NATIVE PHASES WITH IT ELIMINATES
  THE OLD SMALL/MALFORMED KIR GRAPHS. BEFORE SELF-HOSTING, ENSURE
  THE INSTALLED HOST COMPILER IS REFRESHED FROM CURRENT SOURCE.
- CURRENT: COMPLETE NATIVE `MAKE ALL` WITH THE FRESH HOST-BOOTSTRAPPED
  KCPP/KPARSE/KGEN/KOPT; MEASURE KPARSE KIR GRAPH BUILD TIME AND RAM.
  REJECT ANY PER-OBJECT CACHE/INDEX UNTIL ITS SAVINGS JUSTIFY RAM.
  THE POINTER-BOUNDS EXPERIMENT IS NOT YET AN ACCEPTED OPTIMIZATION.
- NATIVE KCPP AND D6FS RESIZE CALL-FLOW/READER-FIELD DEFECTS ARE FIXED;
  KCPP PREPROCESSES CC.C COMPLETELY (220672 INPUT CHARACTERS, STATUS 0).
  FIRST REMAINING GATE IS KPARSE: IT REPORTS SPURIOUS-LOOKING CONSTANT
  EXPRESSION OVERFLOW ON ORDINARY IDENTWDS AND ARRAY-SIZE EXPRESSIONS,
  THEN DID NOT COMPLETE WITHIN A 260-SECOND NATIVE TEST. USE INSTRUMENTED
  KPT4 TOKEN PROGRESS AND ANALYZE CHKOVF/JFCL HARDWARE FLAGS OR A BAD
  KCC OPTIMIZATION. FIX THE COMPILER'S UNDERLYING SEMANTICS; DO NOT
  DISABLE OVERFLOW DETECTION AS A WORKAROUND. TESTS BELONG IN DAIMOS-TESTKIT.
- KPARSE SPEED AND RAM: USER PC 134041..134057 MAPS TO CCKIRWRITE.C'S
  LINEAR FINDPTR POINTER LOOKUP (CONFIRMED VIA KPARSE.MAP AND OBJECT
  DISASSEMBLY). A CAPTURED SCAN HAD 7679 ENTRIES, GIVING QUADRATIC
  GRAPH WRITER WORK. DESIGN A BOUNDED-MEMORY INDEX BEFORE OPTIMIZING;
  MEASURE EXTRA RAM AND EXECUTION TIME UNDER NATIVE 0.25 MIPS CONDITIONS.
  ALSO RECHECK THE SEPARATE STALLED-STORAGE-EVENT SNAPSHOT (EVENT 050636,
  STORAGE_STATE ZERO) BEFORE CHANGING I/O WAKEUP SEMANTICS.
- VALIDATE KPARSE, THEN KOPT AND KGEN THROUGH NATIVE MAKE ALL/INSTALL;
  FOLLOW WITH A NATIVE REBUILD AND SINGLE-SOURCE INVALIDATION TEST.

## 1. SUBSTANTIAL DSH

THE LANGUAGE/INVOCATION CONTRACT IS FROZEN IN PDP10-DOC USER/DSH_DESIGN.MD.
THE DETAILED OPEN-WORK LIST IS `USERLAND/DSH/TODO.MD`; KEEP THIS TOP-LEVEL
SECTION AS THE ROADMAP POINTER RATHER THAN DUPLICATING A FAST-MOVING CHECKLIST.

- FIRST PRIORITY IS DSH-SPECIFIC TEST COVERAGE AND DOCUMENTATION SYNCHRONIZATION.
- THEN CLOSE FROZEN LANGUAGE/CASE/GLOB CORRECTNESS, JOB-CONTROL TTY SEMANTICS,
  THE BOUNDED INTERACTIVE EDITOR, AND ADVISORY SIXMD COMPLETION.
- BASIC COMMAND EXECUTION MUST NEVER DEPEND ON INSTALLED HELP/COMPLETION
  METADATA.

## 2. CORE USERSPACE UTILITIES

PORT ALGORITHMS AND COMMAND BEHAVIOR FROM DAIMOSV1 WHERE USEFUL, NOT OBSOLETE
KERNEL ABIS.

- PRIORITIZE THE MINIMUM COMMAND SET REQUIRED BY THE DAIMOS BUILD AND NORMAL
  ADMINISTRATION.
- PORT/FINISH ECHO, BASENAME, DIRNAME, TRUE, FALSE, EXPR, CAT, CP, CMP, GREP,
  HEAD, WC, TEE, STRINGS, LS, FIND, STAT, MKDIR, RM, MV, PWD, CHMOD, XARGS,
  TAIL, DD, CHOWN, RMDIR, AND TOUCH AS NEEDED.
- REWRITE PS/FREE/DEVSTAT/MEMSTAT/ADMINISTRATION TOOLS FOR MONITORFS;
  DO NOT RECREATE DAIMOSV1 SYSCTL/DEVIO INTERFACES.

## 3. NATIVE BUILD AND SELF-HOSTING PATH

- NATIVE MAKE, DAS, DARC, DLINK, AND OBJDUMP ARE PRESENT.  KEEP THEIR
  LINK-AND-EXECUTE TARGET REGRESSION PASSING.
- DEFER: COMPLETE A DAIMOS-NATIVE MAKEDEPEND.  AN X.ORG 1.0.10 HOST
  REFERENCE IS ALREADY IMPORTED IN `USERLAND/MAKEDEPEND`; SEE ITS
  `PORT-STATUS.MD` FOR NATIVE LIBC, SIXBIT FILE/PATH, AND MEMORY BLOCKERS.
  FIRST CHECK WHETHER NATIVE KCC PREPROCESSING CAN ASSIST DEPENDENCY
  GENERATION.  IF THE UPSTREAM PORT IS TOO LARGE, CONSIDER A SMALL C89
  CONSERVATIVE INCLUDE SCANNER WITH RECURSIVE `#INCLUDE`, `-I`, CYCLE
  DETECTION, AND MAKEFILE OUTPUT; CONDITIONAL BRANCHES MAY OVERAPPROXIMATE
  DEPENDENCIES BUT MUST NOT MISS THEM.  VERIFY THE RESULT ON TERMUX AND
  DAIMOS.  DO NOT IMPLEMENT DURING THE CURRENT NATIVE BUILD WORK.
- REBUILD A CONTROLLED USERLAND COMPONENT WITH THE NATIVE BUILD TOOLS, THEN THE
  FULL USERLAND.
- MEASURE REAL DSH/MAKE/DAS/DLINK/EDITOR/KCC WORKING SETS BEFORE FREEZING
  USERSPACE OVERLAY BOUNDARIES.
- IMPLEMENT USERSPACE OVERLAYS ONLY AFTER THE NON-OVERLAY PROGRAMS WORK AND THE
  MEASUREMENTS JUSTIFY THEM.
- AFTER OVERLAYS, REVISIT LOADABLE/UNLOADABLE/RELOCATABLE RUNTIME MODULES.
  DO NOT RESTORE RESIDENT MOVEMENT/RETARGETING MACHINERY BEFORE THEN.
- PORT/BUILD KCC/CPP, LIBC, CRT0, AND LIBSYS TO RUN UNDER DAIMOS; THEN REBUILD
  THE KERNEL WITH THE NATIVE TOOLCHAIN.
- AFTER THE CORE BUILD PATH WORKS, PORT ELVIS VI/EX WITH SPELLING AND FOLDING;
  KEEP COLOR, SYNTAX HIGHLIGHTING, AND URL FEATURES OUT.

## 4. STORAGE / ADMINISTRATION

- ADD A TRUE ONLINE D6FS PACKER WHICH OPERATES ON A LIVE, MOUNTED FILESYSTEM.
  IT MUST COORDINATE WITH VFS/D6FS ALLOCATION AND OPEN-FILE STATE, MOVE EXTENTS
  WITHOUT INVALIDATING ACTIVE FILE REFERENCES, PRESERVE SWAP/LOGSTORE AND
  BADMAP RESERVED RANGES, UPDATE METADATA CRASH-SAFELY, AND ALLOW NORMAL I/O
  TO CONTINUE EXCEPT FOR THE MINIMUM NECESSARY PER-FILE/PER-EXTENT EXCLUSION.
  THIS IS DISTINCT FROM `TOOLS/HOST/PACKFS`, WHICH REMAINS THE OFFLINE
  MAINTENANCE COMPACTOR.

## 5. HOST TOOLING

### 32K LOWMEM BOOT FOLLOW-UP (NEXT CHAT)

- TARGET PERMANENT KCORE BELOW 8192 DECIMAL WORDS (CURRENT MEASURED
  LOWMEM KCORE: 10020).  MEASURE EVERY CHANGE AGAINST THE SINGLE-DISK
  CTY-LOGIN CONFIGURATION; KEEP THE BOOT MINIMUM AT 32K.
- THE 48K SINGLE-DISK BOOT REACHES DSH AND MEMSTAT.  MEASURED LIVE
  RESIDENT=12725, PROCESS-WORDS=28440, FREE BY DIFFERENCE=7987;
  THE IDENTICAL WORKLOAD EXCEEDS 32K BY 8397 WORDS.  REVISIT USER
  PROCESSES/VM AFTER KCORE REDUCTION; THE GAP IS NOT ALL KCORE.
- PRESERVE THE 8K-WORD CORE-BOUNDARY PROBE IN KINIT.  ADD COVERAGE FOR
  32K, 40K, 48K, AND OTHER SUPPORTED BOUNDARIES IN DAIMOS-TESTKIT.
- BUILD A TRUE CTY-ONLY KERNEL IMAGE, NOT JUST A SIMH CONFIGURATION WITH
  DCS/GE UNATTACHED, AND RECHECK RESIDENT SIZE AND 32K DSH EXEC.
- DO NOT RELAX THE EXISTING DSH IMAGE-SIZE GATE OR COUNT A 48K SUCCESS
  AS PROOF OF 32K BOOT SUCCESS.

- ADD DIRECT PDP10-TESTKIT REGRESSION COVERAGE FOR `TOOLS/HOST/MKDT`,
  `TOOLS/HOST/MKSTREAM`, AND `TOOLS/HOST/WORDS2PT`, USING FORMAT VECTORS OR
  ROUND-TRIP CHECKS WHERE A READER EXISTS.

## 6. POST-0.9 / EXPLICITLY DEFERRED

- PORT MIT MAC HACK VI (`ITS SRC/RG/C.562`) TO DAIMOS AFTER REDESIGNING THE
  TYPE-340 APPLICATION INTERFACE.  DO NOT REUSE THE REJECTED EXPERIMENTAL
  PER-WORD/PERSISTENT KERNEL DISPLAY-LIST FEEDER.  START FROM THE ACTUAL
  TYPE-340/344 HARDWARE MODEL AND FIND A FUNDAMENTALLY SMALLER DESIGN THAT
  PRESERVES DAIMOS'S RESIDENT-MEMORY GOALS, THEN PORT THE CHESS ENGINE AND ITS
  ORIGINAL TYPE-340 PIECE ARTWORK ON TOP OF THAT NATIVE INTERFACE.  KEEP ITS
  ITS-SPECIFIC BOOK/HASH/UTAPE AND MONITOR DEPENDENCIES SEPARATE FROM THE
  CHESS/DISPLAY CORE AND ADD THEM ONLY WHEN THEY ARE JUSTIFIED.

- COMPLETE THE AAP-EMULATED TYPE-340/344 INTERACTIVE HARDWARE PATH.  AAP
  IMPLEMENTS THE LIGHT PEN, LATCHED LIGHT-PEN X/Y DATAI COORDINATES, A SEPARATE
  SPECIAL-CONDITION PIA, HORIZONTAL/VERTICAL EDGE FLAGS, PROGRAMMED STOP, AND
  RESUME.  DAIMOS CURRENTLY USES THE SPECIAL PIA ONLY WHILE PROBING AND KEEPS
  IT DISABLED DURING NORMAL OPERATION.  ADD THE SMALLEST USEFUL RUNTIME
  INTERFACE FOR LIGHT-PEN/SPECIAL-CONDITION INPUT WITHOUT GROWING THE ORDINARY
  DISPLAY-REFRESH HOT PATH; KEEP DATA/DONE PI7 HANDLING SEPARATE FROM THE
  SPECIAL-CONDITION PIA AND EXPOSE RAW HARDWARE SEMANTICS TO USERSPACE RATHER
  THAN INVENTING A LARGE GRAPHICS INPUT SUBSYSTEM.  COVER LIGHT-PEN HIT, DATAI
  COORDINATES, EDGE/STOP CONDITIONS, ACKNOWLEDGEMENT, AND RESUME ON AAP-PDP6.

- RECOVERY/SINGLE-USER MODE: BOOTABLE RECOVERY DECTAPE MEDIA USING THE NORMAL
  MAINTENANCE TOOLS; DO NOT ADD AN INITFS ROOT.
- SLAVE REVERSE INTERRUPTS AND PCLK INTERRUPT DELIVERY.
- A HARDWARE-PAGER VM BACKEND FOR LATER PDP-10 MACHINES.

## 2026-11-09 v1410: native compiler ABI correction and assembler groundwork

- [x] Correct KCC's outgoing stack-only argument lifetime across
  nested argument evaluation.  Preserve VREG spills and use the
  current SP displacement, not a fixed slot relative to transient
  pushes.  No added heap allocations or ABI changes.
- [x] Execute old/optimized/non-optimized C89 five-/six-argument
  test on PDP-6: old exits 5, both corrected variants exit 7.
  The frontend `-n`, not omission of `-O`, disables optimization.
- [x] Rebuild all five native KCC phases and pass v1401 16/16 tests.
- [x] Share native DAS spill-backed symbol hash/record operations;
  pass v1405 5/5 and v1402 17/17; reduce linked DAS1 image by
  octal 056 words versus v1405 with the same corrected KCC.
- [ ] Complete native DOBJ1 relocation serialization, DAS -A -C,
  native DLINK and C89 RUN.  The versioned v1403 link test still
  imports its DOBJ from the host; it is not full self-hosting.
- [ ] Run independent unoptimized host/native compiler equality,
  native 96K working-set and full self-hosting acceptance before
  installing new production binaries.

Final KCORE size-only optimization remains LAST, per v1379.
## 2026-11-09 v1405: native DOBJ1 phase metadata accepted

- [x] Introduce spill-backed native global symbol indices, preserving
  `.globl`, `.extern`, `.entry` and public common symbols without
  allocating a permanent array of names.
- [x] Extend private DASIR2 state with global count and entry ID and
  validate every symbol marker and index after phase import.
- [x] Validate 5/5 native PDP-6 testkit v1405 cases with real KCC
  output, duplicate declarations, external names and an entry.
- [ ] Finish native DOBJ1 relocation records and writer, including
  section-relative addends, left/right-half relocations, symbol
  definitions and undefined references.  Preserve low resident RAM;
  no host transcode or fake DXR object output.
- [ ] Native KCC -> DAS -A -C -> DLINK -> RUN, then 96K and full
  self-hosting acceptance, followed by functional KCORE completion.

Size-only KCORE work is LAST, per v1379.
## 2026-11-09 v1403: native relocatable DOBJ1 output remaining

- [x] Prove native DLINK links DOBJ1 C89 main/CRT0/syscall objects
  into a byte-identical host/native DXR that runs and exits 7.
  daimos-testkit v1403: 5/5 strict PDP-6 guest checks PASS.
- [ ] Implement native DAS1/DAS2 `-C` output with bounded symbol and
  relocation metadata, correct DASIR2 phase transport and DOBJ1
  serialization; do not substitute a host conversion workaround.
- [ ] Re-test native KCC -> DAS -A -C -> DLINK -> RUN with all
  relocatable objects produced on target, then independent -N output
  equality, 96K peak memory and native self-hosting coverage.

The final KCORE size-only sweep stays LAST per v1379.
## 2026-11-09 v1402: native DAS packed ASCII7 path

- [x] Add explicit native DAS `-A` reader for KCC packed ASCII7
  without a conversion file or a second resident word-input buffer.
- [x] Accept mixed-case mnemonics/directives, preserve symbol spelling
  and quoted data, and reject invalid spare bits/NUL/trailing words.
- [x] Rebuild all three native DAS images and the five v1401 compiler
  phases in a full grouped D6FS test root; 17/17 strict PDP-6 probes
  PASS, including compiled C assembly word identity and direct DAS RUN.
- [ ] Complete native `DAS -C` relocatable DOBJ and execute C89
  built entirely through native tools.  Native DLINK with the host
  DOBJ1 input is accepted by v1403; object generation remains OPEN.
  Measure 96K memory and release correctness before installation.

Size-only KCORE optimization remains LAST after all functional and
integration acceptance per v1379.
## 2026-11-09 v1401: native compiler KIR1 and KGEN correctness

- [x] Correct host KCC's call-unsafe induction-pointer optimization
  which had corrupted native KGEN local symbol classes (KCC v1395).
- [x] Correct nested KIR1 record stack lifetime; native KGEN -N and
  optimized modes now return zero on minimal/multi-function inputs.
- [x] Rebuild corrected host KCC and all five native phases; pass the
  strict 16/16 PDP-6 KCC/KIR1/KGEN/KOPT and host assembly comparison.
- [ ] Prove independently generated unoptimized -N host/native output
  equality.  v1402 now proves native packed ASCII7 KCC -> DAS DXR
  output and direct DAS RUN; native DOBJ -> DLINK -> compiled C RUN
  remains OPEN.
- [ ] Complete native 96K peak heap/stack acceptance, compiler source
  coverage and full self-hosting release gates before installing KCC.

The KCORE size-only sweep remains the final KCORE stage after all
feature and functional/integration acceptance, per v1379.
## 2026-11-09 v1379: KCORE work order (size optimization LAST)

The final KCORE memory/code-size sweep is **the last KCORE development
stage**, not a parallel activity and not a prerequisite for completing
any remaining functionality.  First fix compiler/assembler/kernel defects;
finish grouped D6CODEC, BSTORE/BCACHE/LOGSTORE, privileged non-writing
XIP EXEC/RUN and opt-in compressed process swap; then complete their
integrated native boot, media, pressure and failure/rollback acceptance.
Only after all required KCORE functionality and correctness gates have
passed may the final KCORE size sweep, EXEC/RUN consolidation, p10fold
selection and strict size measurements begin.  Maintain the current RAM
budgets and avoid unnecessary growth meanwhile, but do not start new
size-only optimizations.  Keep strict 32K DSH separate; TSFS v1.2 is
post-KCORE and must not block this order.

The v1378 native diagnostic reproduces the still-open multi-function
KIR1 global-symbol Sclass 0 and standalone KGEN UUO/126 faults.  Its
original five probes match command transcripts rather than requiring
successful compiler exit status; they are diagnostic, not acceptance.
## v1373: correct KCC nested-call ABI register lifetime

- [x] Find bad KGEN opcode origin: the wrong opcode was already an
  incoming argument to NEWCODE, not a KPCODE2 writer/KOPT defect.
- [x] Fix KCC GCC-ABI side-effecting call argument evaluation, so
  incoming register arguments survive nested calls.
- [x] Rebuild a patched host KCC and all five native KCC phases in
  isolation, then verify real grouped-root PDP-6 KCC/KGEN/KOPT
  output equality and nested-call executable (9/9 PASS).
- [x] Fix native split KIR symbol with Sclass 0 (host compiler v1395).
- [x] Fix standalone native KGEN -N UUO/exit 126 (v1401).
- [ ] Independently verify -N host/native assembly identity.
- [ ] Verify native compiler -> canonical SIXBIT assembler -> native
  DAS/DLINK -> executable end-to-end before native self-hosting.
## v1372 native CSIX conversion and new KCC compiler blocker

- [x] Reuse the documented reversible C-SIX/S6REC source format,
  including case escapes and punctuation; no silent semantic casefold.
- [x] Extend host CSIX to read packed native KCC ASCII7 words using
  the existing C-SIX encoding rules; test exact byte identity.
- [x] Validate test-only native CSIX converter on grouped PDP-6:
  canonical S6REC equality against host, normal text readability,
  KCC input, malformed-input rejection and cleanup.
- [x] **Resolved in v1373:** native KCC's minimal `return 012345`
  output corruption was caused by cross-host KCC losing incoming
  register arguments across nested calls. The fixed host rebuilds
  native KGEN, yielding the expected `MOVEI` and matching host bytes.
  The later v1395/v1401 fixes resolve the separate multi-function
  KIR and standalone KGEN failure paths; see the current v1401 record.
- [ ] Complete full native KCC -> ordinary SIXBIT assembler source
  -> native DAS/DLINK -> execution validation; C-SIX is for C input,
  while DAS expects uppercase SIXBIT S6REC assembly.
- [ ] Integrate native CSIX into normal Makefile/root only after
  compiler correctness and userland toolchain regression gates PASS.
## v1371: full distribution grouped cold boot

- [x] Expand the actual Makefile D6FS root manifest, stage its
  native DAS, KCC-source, manual, configuration and tooling inputs.
- [x] Generate and roundtrip all 71 normally legacy-compressed
  DXR programs as grouped DXR2, without mixed codec payloads.
- [x] Boot actual 237-file/29-directory, 336-FCB CLEAN D6FS root
  with grouped KCORE/KINIT and 27 MRES packages under PDP-6 SIMH.
- [x] Verify grouped LOGIN/DSH, ECHO/LS/CAT/GREP/WC/STAT output,
  native-tool directory entries and LOGIN respawn after shell exit.
- [ ] Replace legacy-producing `D6LZ` user utility in final format
  conversion; it remains present as an uncompressed executable.
- [ ] Execute native KCC/DAS toolchains, TSFS root/restart media,
  physical PURE/swap reload and device/provider failure regression.
- [ ] Measure nested runtime stack and do atomic production
  host/media/kernel switchover only after all release gates pass.
## v1370: actual complete grouped PDP-6 minimal-system cold boot

- [x] Rebuild full grouped KCORE/KINIT and all **27** MRES packages
  against the same map, with byte-identical original-map controls.
- [x] Install grouped KCORE into resized KINIT stream word-for-word;
  guarded Stage-1/Stage-0 boot to new KINIT entry.
- [x] Create native KCC userland and a CLEAN 15-node D6FS test root
  containing plain INIT/MOUNTALL and grouped LOGIN/DSH/ECHO.
- [x] Native Type-136 DECtape Stage-1 -> full grouped KCORE/MRES ->
  DSK270/D6FS -> INIT/LOGIN -> interactive DSH -> grouped ECHO output
  -> DSH exit/LOGIN respawn; independent D6LZ36 failure control PASS.
- [ ] Rebuild the entire standard distribution's compressed commands
  and full filesystem/boot media as a single grouped-format set.
- [ ] Test actual grouped TSFS-root restarts, PURE eviction/swap and
  real nested-provider kernel-stack watermark and injected I/O errors.
- [ ] Commit a validated atomic host/media/runtime production switch
  only after all cold-boot/EXEC/failure acceptance gates PASS; never
  install a permanent old/new codec fallback.
## v1369: functional grouped EXEC/PURE/TSFS integration

- [x] Generate versioned grouped DXR2 test executables without
  changing header metadata or relocation bitmaps.
- [x] Native actual `exec_load_process`: compiled C89 program runs
  after grouped load; larger linked image, short VFS refills and
  malformed/provider-error staged VM rollback all PASS.
- [x] Native actual `proc_swap_reload_pure` decodes evicted immutable
  grouped backing, rejects corrupt compressed input.
- [x] Native actual `tsfs_read_words` decodes grouped restart extent
  and releases the transient buffer on success or error.
- [ ] Build an isolated complete grouped KCORE, KINIT, MRES, D6FS
  root and userland image and cold boot through INIT/LOGIN/RUN.
- [ ] Test real VFS/VM provider call stack, swap backing identity,
  media failure recovery, and atomic host/runtime format switch.
## v1368: real grouped KCORE residency and MRES relocation

- [x] Reproduce the production complete KCORE DXR/RIM/stream
  byte-for-byte and link the v1367 grouped runtime against actual
  KCORE objects with both original and regenerated p10fold plans.
- [x] Measure +216 text words with a reclaimed low-core decoder
  slot or +256 with a 40-word reservation; verify unchanged BSS.
- [x] Relink five current MRES packages against both grouped maps,
  retaining unchanged resident sizes and export resolution.
- [ ] Verify that Stage-1/KINIT can reclaim `000060..000127`
  without breaking early EXEC or PI low-core ownership; full
  grouped runtime boot still untested.
- [ ] Reduce remaining +216 linked resident words; measure actual
  provider-nested stack and group decoded EXEC/PURE/TSFS payloads
  before synchronized production cutover.
## v1367: RAM-minimize grouped runtime decoder

- [x] Reduce native grouped engine + adapter from 333 to 317 text
  words without changing the format. Reuse descriptor counters to
  shrink continuation state 26 -> 25 and VFS frame 68 -> 67 words.
- [x] Verify PDP-6 VFS ABI, exact/trusted mode, maximum descriptors,
  1/7/64-word DXR refills, stack canary and no static data/BSS.
- [ ] Eliminate or justify remaining +220 resident text words versus
  the legacy 97-word D6LZ36 runtime on a complete staged KCORE link.
- [ ] Measure true cold-provider/EXEC/PURE/TSFS nested kernel-stack
  usage, plus failure rollback and caller ownership, before cutover.
## v1366: grouped runtime VFS continuation and RAM accounting

- [x] Implement staged PDP-6 VFS and buffer decoder entries with
  exact framing, trusted AC3=0 immutable backing, 32-word refill
  windows, 26-word decode state, and AC10..AC16 preservation.
- [x] Native fake-provider and stack-canary tests PASS; verify
  68-word frame and 0110-octal test stack high-water. No production
  linkage or format change.
- [ ] Address measured +236-word text growth (333 grouped vs 97
  legacy) under the RAM-first KCORE budget. Analyze code sharing,
  instruction reduction and full linked KCORE size before cutover.
- [ ] Test actual nested VFS/FS-provider stack watermark, PURE/EXEC
  trusted backing failures, TSFS restarts and all caller paths.
- [ ] Apply final grouped runtime and host/media formats only as a
  single atomic conversion, without a long-lived dual-codec mode.
## v1365: grouped KINIT full-image boot devices

- [x] Rebuild the real linked full-profile KINIT, encode and verify
  all 32,670 words through the independent host grouped reference.
- [x] Boot isolated staged grouped KINIT through actual Stage-0 and
  all five simulator media controllers to KINIT entry (5/5 PASS).
- [x] Reject oversized header and zero-token grouped corruption on
  actual DECtape controller media before KINIT or output writes.
- [ ] Extend negative media/EOF checks to all devices; verify wider
  reconstructed KINIT image coverage and post-entry boot behavior.
- [ ] Complete synchronized grouped host producer, KCORE resident,
  VFS, EXEC/PURE, swap and TSFS integration; no mixed-format fallback.
## v1364: Stage-1 physical geometry before first payload copy

- [x] Implement staged PDP-6 word-accurate image/stack and
  image/compressed-staging admission checks, parameterized by physical
  memory size and KINIT stack reserve. Native geometry and 12
  generated-loader header-guard tests PASS (32K, 256K, 64K/custom
  reserve). PTR also validates first tape segment count pre-write.
- [ ] Wire the accepted geometry helper into real Stage-1 only as
  part of atomic grouped producer/runtime format conversion; boot
  all five devices and exercise media errors and rollback.
## v1363: direct-linked full-buffer grouped Stage-1 prototype

- [x] Audit DSK/DRM/DTC/MTC/PTR: all already buffer the complete
  compressed KINIT before decode. Generate smaller linked v053
  Stage-1 alternatives and measure read-in text/BSS geometry.
- [x] Native PDP-6 test generated handoffs with one-literal valid
  grouped input and malformed-control rejection at load base 000060.
- [x] Validate output+compressed staging geometry *before* payload
  writes in staged v1364 variants (native tests only).
- [ ] Boot all five real loader variants with rebuilt grouped KINIT
  images as part of an atomic producer/runtime format cutover.
- [ ] Runtime VFS streaming remains separate; no double-decoder
  production KCORE footprint accepted without quantified net savings.
## v1362: grouped native streaming representative corpus

- [x] Verify staged native grouped NEED_INPUT with current KCORE,
  D6FS, MonitorFS, SWAP and tape host-golden 128-word prefixes under
  1/7/64-word refill windows (15/15 PDP-6 PASS).
- [ ] Reduce/relocate the native decoder, prove kernel caller ABI
  and provider failure semantics, and migrate all five Stage-1
  loaders and runtime consumers in a synchronized format cutover.
## v1361: staged native PDP-6 grouped resumable engine

- [x] Implement separate assembler-only grouped NEED_INPUT decoder:
  26-word state, 15 packed descriptors, native split-word/control
  and KCORE host-golden prefix PASS. No production linkage.
- [ ] Reduce/restructure measured 230-word text to satisfy an actual
  Stage-1 low-core placement/relocation contract; measure caller
  state, stack, provider refill and native invalid-stream cases.
- [ ] Atomically migrate all Stage-1, KCORE and host producers only
  after a complete boot/runtime acceptance gate.
## v1360: grouped decoder resumable interface staging

- [x] Test a bounded descriptor-only resumable grouped-format host
  state machine at all refill positions: 27 current DXRs x 6 windows
  (162 successes), ten malformed examples x 6, and a maximum
  15-descriptor group. Host reference PASS only.
- [ ] Implement and measure the native PDP-6 low-core continuation,
  including VFS/Stage-1 state preservation, stack and relocation.
  No production decoder/format change is accepted yet.
## v1359: source-authentic grouped codec DXR parity

- [x] Host reference roundtrip all 27 currently linked kernel/MRES
  DXRs, and verify five representative 128-word prefixes bit-exactly
  on PDP-6 against the host golden streams.
- [ ] Rebuild the full userland DXR corpus with current paths; extend
  complete native/host compatibility and production format consumers.
## v1358: grouped codec host/native source parity

- [x] Verify three independent host-golden 128-word real-DXR streams
  against the staged native PDP-6 encoder and decoder (3/3 PASS).
- [ ] Reconstruct the current full host DXR corpus with real paths,
  compare native/host format over representative complete programs,
  and complete atomic boot/runtime/producer format migration.
## v1357: staged codec malformed/capacity stack

- [x] Native PDP-6 test staged decoder malformed/invalid-address
  returns and encoder short-capacity literal, zero-run and match-search
  failures with a 512-word canary stack: 016 octal / 14 decimal words.
- [ ] Prove native stack upper bounds under real nested VFS/EXEC/TSFS
  consumers and wider malformed-input coverage before codec cutover.
## v1356: staged native codec stack baseline

- [x] Record 512-word-canary full-DXR v055 wrapper-inclusive stack:
  016 octal = 14 decimal words, native PDP-6 v1356 PASS.
- [ ] Check malformed-input/insufficient-capacity stack and real nested
  VFS/EXEC/TSFS call peaks before claiming a production maximum.
## v1355: staged grouped compressor output-capacity guarantee

- [x] Prove the bound N + ceil(N/30) words for N input words (zero for
  empty), implement an allocation-free native staged bound helper and
  reject unrepresentable physical output counts. Testkit v1355 native
  exact/short 30/31-token and zero-run cases plus edge counts PASS.
- [ ] Measure worst-case codec stack/high-water, host/native parity,
  and complete coordinated boot/consumer cutover. No production switch.
## v1354: staged grouped encoder physical bounds

- [x] Validate source/capacity 18-bit geometry and reject source/output
  overlaps before writing a control word. Native PDP-6 v1354 negative
  suite and v055 full-DXR ABI test PASS; original encoder fails the
  expected negative witness.
- [ ] Continue codec producer format/capacity contract and native stack
  proof, then the indivisible Stage-1/KINIT/VFS/EXEC/PURE/TSFS consumer
  cutover and full image/rollback acceptance. No production activation.
## v1353: staged D6CODEC geometry/canonicality validation

- [x] Native staged decoder rejects buffer pointer/length overflow,
  aliasing, unused control bits and unused descriptor-half padding.
  Staged native v055 full-DXR roundtrip and new v1353 negative tests
  PASS; original v1351 decoder fails the negative witness as expected.
- [ ] Continue D6CODEC production cutover: bounded encoder, native stack
  peak, full producer parity, stage-1 streaming interface, KCORE/VFS,
  EXEC/PURE and TSFS integration, atomic artifacts and boot tests.
  Existing bootable D6LZ36 remains unchanged until all gates pass.
## Deferred: executable opt-in for compressed process swap (2026-10-15)

### v763: variable-sized in-place swap exchange (critical correctness)

- [ ] Establish the bounded, lossless sector-transfer order for unequal
  resident/swap sizes and unequal PURE boundaries; see
  `design/swap-exchange-v763.md`. RAM capacity, disk extent capacity,
  contiguous allocation, and workspace lifetime are distinct invariants.
  No persistent journal/crash recovery is required for volatile swap.
- [ ] Implement and test exchange as a new transaction only when source
  preservation, MM ownership, BACKSTORE bitmap accounting, and process
  descriptor publication are proven. Keep the current disk format unless
  variable-sized exchange demonstrably requires a replacement. Accept only
  after native writer WAIT/EXIT and failure-injection regression pass.

### v761: optional link-time compression of PURE executable backing


- [ ] Evaluate opting PURE DXR2 executables into the already-supported
  compressed executable-payload format at link time. Measure raw versus
  encoded storage blocks and estimate PDP-6 decode time for disk and DECtape;
  select compressed backing only when it pays off. Reuse existing executable
  size/PURE-boundary metadata and swap-in reload; do not confuse executable
  payload compression with runtime process-state compression.
- [ ] Validate original-image reconstruction and repeated PURE eviction /
  reload for compressed and raw DXR2, without storing another compressed
  text copy in RAM by default. Keep this strictly deferred and independent
  from the currently failing native raw-swap pressure regression.


### v760 design separation: capacity recovery first; compression later

- [ ] Treat RESIDENT, PACKED-in-RAM, and DISK as *alternative storage
  representations*, not a mandatory RESIDENT -> PACKED -> DISK sequence.
  A disk-backed image may be raw or compressed. The reclaim policy must
  select a representation using measured RAM savings, BACKSTORE blocks,
  compression CPU cost, restore latency, and scratch/rollback headroom.
  On a PDP-6, do not assume compressing before each disk swap is faster.
- [ ] Preserve the original *logical process image size* in the stable
  process descriptor across every storage transition, independently of
  the encoded payload length. For PACKED, also retain the omitted PURE
  boundary and decoded writable extent; for compressed DISK, retain the
  encoded block count and exact decoded word count, including any padding.
  First audit the existing three-word process descriptor, one-word
  overloaded disk record, stable u-area, and two-bit storage mode map;
  do not allocate another per-slot word or steal an encoding bit without
  proving non-overlap. Reject mismatching sizes on restore.
- [ ] Before activation, benchmark PURE prefix omission alone against
  raw disk swap, compressed disk swap, and PACKED-in-RAM using actual
  process data and PDP-6 timing; record worst-case incompressible images
  and temporary peak RAM, not only compression ratios. Reuse lossless
  codec primitives where safe, not disk/RAM transaction semantics.
- [ ] Keep the native v758/v759 writer deadlock as a separate critical
  correctness problem. Fix resource-pressure admission/forward progress
  without making either compressed swap or PACKED a prerequisite. First
  establish which eligible victims require 208 blocks after PURE omission;
  then specify an admission/headroom invariant covering the case with no
  runnable resident producer of free RAM or swap space. Preserve valid
  swapped processes on retryable pressure and require passing native
  writer WAIT/EXIT acceptance. A cooldown is not a capacity guarantee.

- [ ] Reserve an unused, backwards-compatible DXR executable-header flag
  (`COMPRESSIBLE` or equivalent) after auditing existing DXR flag allocations.
  This flag describes the expected compressibility of *runtime process memory*;
  it is distinct from the existing DXR `COMPRESSED` executable-payload flag.
- [ ] On `exec`, initialize the process compressibility bit from the executable;
  inherit it across `fork`, and replace it on a subsequent `exec`. Keep the
  default uncompressed for unmarked executables; no adaptive statistics,
  heuristics, size-only prepass, or runtime policy controls initially.
- [ ] Swap-out: bypass the codec entirely for unmarked processes. For marked
  processes, attempt the native compressor; if it cannot encode safely or
  produces no useful reduction, retain the existing raw swap transaction.
  Preserve atomic allocation/write/rollback semantics on any failure.
- [ ] Keep a separate *swap-record* `COMPRESSED` bit to describe the actual
  stored representation, independent of the process opt-in bit. Swap-in must
  decode according to the record and validate the exact restored word count.
- [ ] Before activation, benchmark representative non-text 36-bit process
  images with AAP logical ticks for encode/decode, compressed block count,
  temporary stack/RAM, and I/O tradeoffs. Test roundtrip, incompressible
  fallback, allocation failure, rollback, `fork`/`exec` inheritance and
  mixed compressed/raw swap records in daimos-testkit. Do not modify the
  running parallel kernel implementation as part of this TODO entry.

- [x] v618 mutually exclusive PCLK/SCLK MRES: KINIT boot selection,
  shared APR scheduler, target GETTIME/SETTIME acceptance, and measured
  linked low-memory occupancy. Both calendar providers are optional.

- [x] v618 native exclusive-provider gate: testkit
  `daimos-clock-mres-v618` PASS at 50 and 60 Hz, with PCLK enabled
  and disabled in separate clean boots. Lowmem GCC links PASS at both rates.

## 2026-11-09 v1405: native DAS relocatable DOBJ1 staged

- [x] Implement native DAS1/DAS2 `-A -C` DOBJ1 output using
  scratch-backed globals and relocation records with object-only
  DASIR2 transport; preserve the default S6REC/DXR format.
- [x] Native KCC -> DAS `-A -C` -> DLINK -> RUN passed 29/29
  strict PDP-6 tests for external function, data and local-data
  C89 fixtures.  Each native DOBJ/DXR matched its host golden.
- [x] Pass full v1402 compatibility suite (17/17) with final
  native DAS source, including literal equality and malformed input.
- [ ] P0: Investigate intermittent native KCC stall under sustained
  sequential compile stress: final-source full-run retry passed
  16 probes then hung at local-data compile; the isolated identical
  fixture passed 7/7.  Test guest swap/proc/FS resource lifetimes;
  diagnose root cause, do not mask by splitting/rebooting tests.
- [ ] Verify relocatable `.set` alias semantics and uncommon LH/RH
  relocation expressions; correct any metadata-lifetime defect.
- [ ] Complete 96K peak RAM/stack and broader self-hosting, media
  fault-recovery and negative-object input regressions before
  replacing production compiler/assembler binaries.

All size-only KCORE optimization is deferred until the final KCORE
development stage, after functional and integration acceptance.

## 2026-11-09 v1403: native DOBJ1 link acceptance

- [x] Independently verify native KCC C89 compiler output against
  host KCC and native DLINK of known-good DOBJ1 CRT0/MAIN/SYSCALL.
  Native DLINK produces an exact host DXR and executed C89 returns 7;
  5/5 strict testkit probes pass on the grouped 237-file PDP-6 root.
- [ ] Implement native DAS `-A -C` with complete relocatable DOBJ1
  symbol and relocation semantics through its split phases, using
  file-backed metadata instead of large resident relocation arrays.
- [ ] Pass full native C89 -> DAS `-A -C` -> native DLINK -> RUN,
  followed by exact 96K stack/heap and self-hosting qualification.

The KCORE size sweep remains LAST, after functional acceptance.

## 2026-11-10 v1461 TSFS producer and executable acceptance

- [x] Identify old host `mktsfs` generating an incompatible
  LOGIN compressed restart extent, despite valid v1458
  decoder and source-compatible block encoder.
- [x] Fix `tsfscheck` REG filtering: PARENT_FLAGS low half
  includes mode; mask file-type bits before verifying extents.
- [x] Make boot-image generation build versioned host TSFS
  producer/checker from checked-out sources, then strictly
  verify generated TSFS media before boot.
- [x] Reject the old corrupt TSFS image; verify the fresh
  53-file/401-extent image and native PDP-6 decoding of
  LOGIN's actual 149-word final compressed restart.
- [x] Cold boot 256K TSFS-root to ROOT LOGIN and DSH V1.
- [x] Execute external ECHO from the real TSFS root and
  return to DSH prompt. The 240-second guest-command
  allowance passes; the earlier 70-second allowance timed out.
- [x] Reconstruct complete LOGIN, DSH and ECHO executables
  directly from the real TSFS restart extents; compare every
  decoded/stored word against the linked source DXRs.
- [x] Complete guest ECHO execution and compressed LOGIN
  restart-file acceptance in the full 256K TSFS-root image.
- [ ] Expand TSFS guest acceptance to additional external
  executables and low-memory profiles. Preserve bounded
  simulator timeouts and never treat timeout as success.
- [ ] Retain strict 32K/48K memory gates as open.
## 2026-11-10 v1460 five-device Stage-1 completion

- [x] Identify DTC/MTC boot failure after data loading: KINIT
  entry retained in AC5 was overwritten by the block decoder's
  match-source calculation. A captured header entry of octal
  053537 became octal 120366 before the bad entry jump.
- [x] Preserve the entry in unused AC16 through both tape Stage-1
  decoder calls (also for DTC raw/hybrid); no extra instructions.
- [x] Native-assemble v1459 and v1460 loaders and prove identical
  physical sizes: DTC 160 words, MTC 125 words; only the intended
  five/four register-reference instruction words differ.
- [x] Run real full 256K DTC and MTC cold boots through KINIT,
  ROOT login, DSH and ECHO with isolated media. All five Stage-1
  devices now pass disk-root cold-boot tests.
- [ ] Continue independent TSFS-root compressed-file and executable
  restore acceptance. v1452/v1459 both reported INIT RUN FAILED
  despite booting INIT; investigate rather than changing codec checks.
- [ ] Retain strict 32K/48K lowmem acceptance as open, with measured
  KINIT/KCORE gates and baseline comparisons; no arbitrary reserve
  or memory-size bypass.
## 2026-11-10 v1459 integration checkpoint

- [x] Switch all five Stage-1 sources and the actual KCORE build
  lists to the 53-word fixed v1458 decoder; select the 58-word
  VFS adapter and migrate EXEC/PURE/TSFS callers.
- [x] Build and link the real KCORE, KINIT and 256K disk images;
  verify only one decoder is linked, with exact 53-word identity
  across KCORE and five Stage-1 read-in images.
- [x] Compare like-for-like KCORE links: 022627 to 022617 octal
  initialized words, saving eight physical words; BSS unchanged.
- [x] Full 256K cold-boot acceptance for DSK, DRM and PTR
  Stage-1: ROOT login, DSH and ECHO passed on isolated media.
- [ ] Obtain full 256K DTC and MTC Stage-1 cold-boot acceptance;
  their standalone assemblies and exact core-image checks pass,
  but both timed out before the boot banner (180s DTC, 240s
  MTC). Investigate the controller/read-in path against v1452
  before blaming the decoder. Original v1452 Stage-1 control
  tapes also failed to produce a banner within 75 seconds
  against the same media, for both controllers.
- [ ] Diagnose TSFS-root INIT RUN failure independently from
  v1459: the v1452 control has the same failure. Verify TSFS
  compressed restart-extent decode with a dedicated booted test.
- [ ] Complete strict 32K and 48K memory gates: the 32K
  KINIT/KCORE size checks still fail and both v1452 and v1459
  48K GCC lowmem boots stall after INIT V1.
- [ ] Finish the full boot matrix and final documentation before
  declaring the v1459 cutover production-accepted.
## 2026-11-10 v1458 fixed-low-core block decoder acceptance

- [x] Assemble the v1458 block decoder at 000060: 53 words including
  one in-image byte pointer; zero static DATA/BSS/relocations.
- [x] Native test the single copied low-core decoder with six valid
  and five malformed/streaming controls; verify eight VFS refill/direct
  tests, nine malformed input tests, five complete real DXRs, and one
  stack-canary; all 34 tests pass.
- [x] Assemble all five Stage-1 device sources against the v1458
  decoder; measure exact read-in sizes versus v1452 (8/9 words less).
- [ ] Select the v1458 code and 58-word VFS adapter in the actual
  Stage-1/KCORE build, preserving ONE shared fixed decoder; verify
  full link and ABI, then perform real cold boot on all supported
  devices, bounded TSFS image and EXEC/PURE restores, and 32K/48K
  low-memory regression acceptance. This is a separate integration
  phase, not completed by native unit tests.
- [ ] Continue the existing D6LZ36-free replacement validation;
  never reintroduce the old format or ship two permanent file
  decoders for compatibility.
## 2026-11-10 v1451: swapless TSFS bitmap MRES

- [x] Reduce the native RAM-only bitmap encoder+decoder from
  55 to **30 total PDP-6 TEXT words**, zero DATA/BSS;
  preserve the 36-bit presence bitmap representation and exact
  314/524/677-word DSH payloads. Validate eight native
  roundtrips and register preservation on AC10..AC15.
- [x] Prove native 30-word image / 36-word transient two-export
  MRES package, and perform bidirectional p10fold analysis
  against the block v1441 decoder. No safe >=2-word
  cross-MRES folds; reject the one-word POPJ dependency.
  Record native encode+decode instruction counts.
- [ ] Integrate optional bitmap MRES into the MINIT table
  **only** when a swapless TSFS-root boot profile is selected
  and independently verified at activation. Keep it permanently
  resident while packed processes can exist; never read
  the decoder from tape during OOM.
- [ ] Finish the PURE-only reclaim/restore path, then implement
  exact bitmap preflight and allocation, stable process
  ownership, atomic pack/restore, APR/fragmentation headroom,
  cold-inactive-victim admission, and full 32K/48K pressure
  acceptance. Do not pack on every SLEEP.
- [ ] File/executable compression remains the distinct
  block-v1441 format; replace and delete D6LZ36 after a
  complete Stage-1/TSFS/EXEC/PURE boot migration. Never
  use bitmap to decode TSFS files.
## 2026-11-10 v1441: complete single-file-decoder cutover

- [x] Minimize experimental block v1424 core/VFS adapter to
  v1441: 75+89 -> **60+58 = 118** total native TEXT,
  39-word temporary VFS frame and zero DATA/BSS.
  Preserve v1421 block format, resumable refills, malformed
  stream rejection and exact compressed-payload bounds.
  Native 8 positive, 9 malformed, stack, 5 full-DXR = 23
  PASS; measure mixed DXR/DSH/KCORE decoder steps.
- [x] Remove repeated validation of **kernel-owned** physical
  destination/source extent, caller-provided buffer
  disjointness, and decoder stack overlap. Record those as
  explicit trusted KCORE/MM caller contracts. Keep validation
  of untrusted format fields and externally supplied counts.
- [ ] Install a **single block decoder** in place of D6LZ36,
  including shared bounded TSFS/EXEC/PURE_PREFIX decode,
  block-format producers, and all Stage-1 device loaders.
  Prove fixed low-core and memory placement without
  increasing Stage-1 hardware limits or bypassing any
  compiler/assembler bug. Measure exact linked/folded KCORE,
  real nested VFS stacks, 32K/48K boot, fault/restart and
  compressed-file integrity. Do not leave parallel file decoders.
- [ ] After complete block-only acceptance, **delete D6LZ36**
  decoder core, runtime adapter, userland/host encoders,
  old Stage-1 includes and now-unused format paths; search
  every build rule, kernel file, doc and boot fixture for stale
  references. Do not harden or optimize the old decoder.
- [ ] Keep bitmap RAM encoder+decoder a separate optional MRES
  strictly for the verified swapless TSFS-root profile,
  pending full PURE-only restore and process MM transactions.
  No per-SLEEP compression or bitmap TSFS executable files.
## 2026-11-10 v1427-v1429: balanced native process codec decision

- [x] Decode three real DSH memory snapshots with unmodified
  D6LZ36 and compare compressed lengths against grouped, block,
  and RAM RLE. Measure native PDP-6 decoder instruction counts.
- [x] Replace the common D6LZ36 match-copy loop with forward
  BLT. 40 -> 39 core words; 9601 -> 9600 linked KCORE words,
  no BSS change. 23 native tests PASS; independent full image
  boots to DSH and executes ECHO. No new bitstream format.
- [x] Prototype three native C89 encoders in daimos-testkit,
  matching the independent existing host encoders exactly,
  without static DATA/BSS. Measure code size and three real
  DSH full encode/decode cycle instruction costs, including
  real RAM RLE exact-size pass and separate native decode.
  RAM RLE is fastest complete cycle in all three samples.
- [ ] Optimize existing RAM RLE decoder with PDP-6 BLT and
  measure code, stack, CPU and peak buffers before choosing
  any integrated process-memory compression format.
- [ ] Measure final native encoding+decoding, exact-size
  admission, linked/folded KCORE cost and complete restore
  transaction under memory pressure. Do not compare test-only
  encoder core sizes with production whole-image costs as
  though they were directly interchangeable.
- [ ] Investigate the older DTC Stage-1 106-word regression
  cap separately; its original unoptimized image is already
  140 words and the BLT variant is 139.
## 2026-11-10 v1426: actual DSH RAM snapshot acceptance

- [x] Boot a real PDP-6 DAIMOS DSK-root guest from copied writable
  media, log into DSH, examine real process descriptors and confirm
  five relocated instruction signatures against the linked DSH DXR.
  Capture complete physical suffixes at idle, after six SET
  commands and after defining/executing a function; inspect
  source/stack/VM bounds after process relocation.
- [x] Exact grouped v1420, block v1424 and actual native/host
  kpack_ram_rle_v1376 compression comparison on all three real
  process snapshots. 18 host roundtrips and 9 native PDP-6
  decode/encode checks PASS. Preserve guest memory only in TMPDIR,
  commit reproducible scripts/results/documentation.
- [ ] Implement and benchmark a **native** compact block encoder
  if its full encode+decode RAM footprint can beat the existing
  449-word standalone v1376 RLE codec. Alternatively choose
  v1376 if adding another kernel codec is not cost-effective.
  Include encoder CPU time, peak transient RAM, and all required
  code/metadata/stack in the comparison.
- [ ] Under a real PURE-eligible DSH, test per-process MM admission,
  exact packed extent ownership, atomic restore and rollback,
  32K/48K resident pressure and net physical RAM high-water.
  Present v1426 only as read-only capacity evidence, not an
  enabled process compression feature.
## 2026-11-10 v1425: DSH mutable-suffix compression experiment

- [x] Simulate the actual DSH DXR2 text/data/BSS/startup geometry
  using real compiled BSS symbol sizes and modeled SIXBIT state,
  environment, heap, stack, jobs and function allocations. Compare
  raw sector-aligned suffix and strictly writable-only sizes with
  grouped v1420 and block v1424. All ten host roundtrips and ten
  full native PDP-6 suffix decompressions PASS.
- [ ] Capture a real running DSH's process memory in the PDP-6
  simulator at idle and after command/function activity; repeat
  the native exact-compression comparison to validate this
  simulated sparsity against measured guest data.
- [ ] For any swapless mutable-RAM compression admission, count
  codec/code/frame/metadata/temporary-buffer overhead, reject
  incompressible blocks before MM commit, preserve raw fallback
  and immutable backing, and prove in-place compressed ownership
  and error rollback. Do not activate it from v1425 alone.
## 2026-11-10 v1424: streaming decompression decision pending

- [x] Benchmark the historical grouped v1367 decoder, aggressively
  tune its literal/match/zero paths with PDP-6 BLT, and implement a
  separate compact 36-bit block LZ/RLE alternative with a host
  producer. Keep all experiments unlinked to production.
- [x] Optimize the block decoder to v1424, using register-resident
  continuation and the legacy-style refill ABI. Native PDP-6 VFS,
  negative, stack and five full-DXR regressions PASS. Measure text,
  temporary frame, corpus compression, simulator steps and BLT
  transfer-adjusted approximate timing. See pdp10-doc
  KERNEL/D6CODEC-STREAMING-OPTIMIZATION-V1424.MD.
- [ ] Before any atomic production cutover, measure real linked/
  folded KCORE and all MRES effects, actual nested cold-provider
  stacks, and full 32K/48K RAM high-water. Even v1424 (164
  standalone TEXT words) grows code by 67 versus current 97-word
  D6LZ36, although its temporary frame is 32 words smaller.
- [ ] Decide the canonical block-versus-grouped format and implement
  bounded PURE prefix reconstruction. If block wins, rebuild *all*
  Stage-1/KINIT/DXR/TSFS host and native artifacts atomically, run
  all five boot devices, EXEC/PURE/TSFS and media-failure regression.
  Do not install dual decoders or let the optional encoder inflate
  mandatory KCORE RAM.
