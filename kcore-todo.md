# KCORE feature TODO (K01-K16)

Updated: 2026-11-19.

This is the **implementation-only** KCORE backlog. Work on these sixteen
items is the current scope. Filesystems, storage utilities, hardware backends,
toolchains, userland, optimization, consolidated acceptance matrices and
release administration remain outside this list. Feature-specific tests are
part of each implementation task and belong in daimos-testkit.

The accepted PDP-6 RAW PURE v1674 48K GENERAL/STRICT committed-MM
high-water result is not a claim of PI6 fault-pressure, bitmap mode3,
KA10, alternate-root or mainline-merge acceptance. RAW PURE remains
unmerged from its development line; mode3 remains disabled.

## Mandatory KCORE: process and memory correctness

- [x] **K01 - Permanent MM failure handling (CLOSED v1718).** Correct ownership and
  recovery when memory cannot be freed, pinned, rebased, relocated or
  restored. Fail closed on ambiguous ownership; retain exact diagnostics
  and provide bounded retry or safe administrative recovery, never forced
  release of unproven extents.
  **Accepted (v1678-v1718):** MM compaction and reclaim preserve hard
  refusal types. Swap-out now fails closed on unsuccessful pin acquisition,
  exact type/owner/length/pin verification, PURE verification rollback
  unpin refusal, post-write MM-free refusal (v1683), and swap-prepare
  AC10/AC11 preservation during pinned MM growth (v1684), and direct
  fail-stop on refused swap-in unpin (v1685), and exact MM ownership
  verification before restoring a swapped image (v1686), and rejection
  of stale process-table source bases before VM relocation (v1687).
  Native PDP-6 v1688 also verifies no-fit descriptor rollback and PI
  restoration, including an unsafe rollback mutation. v1689 checks
  descriptor capacity and source slot before destructive relocation.
  v1690 fail-stops at distinct HALT PCs if either MM relocation
  descriptor reinsertion or post-VM publication unexpectedly fails.
  v1691 fail-stops if diagnostic PURE verification cannot restore the
  original resident image after erasing the PURE prefix.
  v1692 validates relocated MM process-slot numbers against the actual
  runtime-sized process table before accessing a process descriptor.
  v1693 rejects MM relocation lengths that differ from the resident
  process VM extent, before physical copy or process-state mutation.
  v1694 closes the compactor's PI-to-descriptor race: relocation locks
  PI before reading the source and rechecks PROCESS type, pin count,
  table bounds and slot integrity before deleting any owner.
  v1695 publishes VM_NONE after a successful ordinary process-MM free,
  before fallible backing detach, so teardown retries cannot free the
  already-retired extent or retain a stale physical VM pointer.
  v1696 preserves the exact hard MM reclaim result in the early BRK
  rebase fallback, preventing additional pressure after -4/-5 while
  retaining the bounded -1 resource-pressure allocation path.
  v1697 serializes the core MM free type/owner/pin check with descriptor
  deletion under global PI exclusion, preventing a new interrupt-side
  physical pin from being lost during release.
  v1698 serializes MM pin and unpin lookup, pin-count validation and
  update under the same PI exclusion, preventing interrupted stale
  writes or lost concurrent pins.
  v1699 likewise serializes the pinned PROCESS transfer, shrink and
  grow descriptor checks with publication. Exact owner/pin/size
  refusals remain intact; no permanent RAM is added.
  v1700 protects the read-only exact pinned-PROCESS verifier against
  mixed-time extent size and owner/type/pin observations under PI.
  v1701 likewise snapshots the u-area MM owner, type and pins for
  GETTEARDOWN detail under PI, preventing a stale diagnostic slot.
  v1702 also protects the allocation free-gap search through MM
  descriptor insertion; a PI-side owner cannot steal the selected gap.
  The existing slot-0 finalizer has a capped 16..512-tick retry interval
  (v1102/v1098 tests), with offline operator recovery for permanent
  ownership corruption; neither forced release nor a new retry table
  is permitted.
  v1703 confirms permanent real-MM pin (-5), wrong-owner (-4) and a
  distinct held filesystem-provider lease in three individually booted
  256K PDP-6 full-system guests: owners and WAIT remain quarantined,
  retries are bounded in rate, diagnostics stay exact and independent
  users progress. The v1703 testkit gate records per-case evidence.
  v1704 revalidated the real-PI6 mixed physical recovery guest after
  repairing old test-only MM-free hooks for v1697 PI restoration:
  PIPE, DPY, VM and u-area each reject a live pin then release exactly
  once; full FILE->VM->u-area->ZOMB ordering, 55 qualified PI6 ticks,
  live bystander progress and 48 WAIT/PID cycles pass without errors.
  v1705 realigned the strict F07 swap-in refusal *test* with v1507's
  per-owner retry epoch; the full guest now boots but fails its earlier
  18-child RUN admission gate at 6/18, before the injected bitmap fault
  can arm. Neither swap-refusal nor F07 final acceptance is claimed.
  v1706 stops destructive low-memory EXEC if release of its transient
  launch-copy MM extent returns -4/-5; its stack-local owner must never
  vanish on a reported EXEC result. No permanent RAM is added.
  v1707 likewise fail-stops on refused MM release of unpublished PIPE
  objects, D6FS mount readers and both TSFS decoder scratch cleanup
  paths. Four native PDP-6 owner-refusal mutants are rejected; all
  ordinary successful-release paths retain their previous behavior.
  v1710 skips the intentionally immovable current PROCESS during MM
  compaction; otherwise its expected -5 poisons allocation as a false
  hard refusal. Genuine other-owner busy errors stay fail-closed.
  v1711 snapshots the advisory `mm_is_pinned` base lookup and pin read
  atomically under PI; swap-out still requires its own checked pin.
  v1712 corrects the test-only v1494 TERM stack witness to -4(17)
  and checks it in native PDP-6; the full physical F07 gate remains open.
  v1713 physical faulted F07 PASS: 18/18 children, two deferred TERM
  events, exact backing repair, independent MM owners and final
  BACKSTORE zero. The clean no-fault v1349 F07 remains a separate gate.
  v1714 restores the native v870 PIPE-close source extraction;
  v1715 passes 33 source-linked native K01 regressions; v1716 audits
  all 26 MM-free callers in 12 files and rejects five unsafe changes.
  Final v1717 integrated acceptance was rerun and all six gates passed:
  clean v1336/v1303/v1349 F07, faulted v1713 F07, 33 native source-linked
  PDP-6 regressions and v1716's 26-site MM-free refusal audit. Permanent
  -4/-5 quarantine (v1703) and real PI6 retry-to-completion (v1704)
  also passed independently. K01 is closed under the existing bounded
  retry-rate and controlled offline-recovery contract, without forced
  release or added permanent RAM. See
  `design/kcore-k01-mm-refusal-v1678.md` and
  `design/kcore-k01-swap-owner-v1682.md`. Hardware interrupt tests do
  not exhaust all possible interleavings. K02 process-resource cleanup
  and K03 OOM policy remain independent open work.
- [x] **K02 - Process resource cleanup.** Complete failure-safe cleanup
  of process-owned MM, VM, FD, PIPE/FIFO and related resources during
  EXIT, EXEC and failed or unpublished RUN. Do not claim provider-specific
  filesystem or device implementation work as part of this task.
  **In progress (v1719):** transactional EXEC now identifies a surviving
  staged VM after loader failure, checks staged teardown results instead of
  discarding its only stack-owned descriptor, uses the new backing record
  when rolling back a failed old-VM destruction, and returns fatal rather
  than resumable failure if the old VM is already VM_NONE. Native staged
  backing and failure-path tests pass. Full EXIT/RUN/EXEC cleanup and
  physical fault-pressure acceptance remain open; K02 is NOT closed.
  **v1720:** destructive low-memory EXEC now distinguishes refused VM
  teardown before physical release from post-release backing refusal;
  the latter enters fatal cleanup instead of resuming a freed APR.
  Replacement loader failure also restores the original stable u-area
  descriptor before fatal FILE/VM/EXIT. Seven source-linked native cases
  and two unsafe mutants pass. K02 remains OPEN.
  **v1721:** the full 256K PDP-6 guest now exercises two intentionally
  refused backing-detach calls *after* the real PROCESS MM free. The
  original owner enters fatal EXIT, slot zero retries twice, and the
  third call accepts without a second PROCESS MM release. Backing
  identity survives both refusals; FILE retry and parent WAIT pass.
  K02 remains OPEN for other resource and provider interleavings.
  **v1722:** source-linked PDP-6 native integrated RUN/PIPE/FILE test
  covers malformed unpublished FD-map rollback with no ref acquisition,
  duplicated inherited PIPE endpoints, full six-bit TSFS mount isolation,
  independent VFS_SYNC and final PIPE MM_ERR_BUSY close refusals, exact
  per-FD witness and successful retry without double release. Two
  ownership-mutating negative controls are rejected. K02 remains OPEN.
  **v1722a:** the existing 256K PDP-6 physical named-FIFO guest v900
  passes against current KCORE with a real MM pin, -5 final-close
  refusal, FIFO re-listing/reopen, unpin and successful retry. Native
  typed -4/-5 FIFO tests v1099 (four mutants), pinned OPEN rollback
  v1084 and direct-mount isolation v417 also pass. No KCORE change.
  **v1723:** full 256K PDP-6 guest now runs an actual parent/child RUN
  inheritance of both anonymous PIPE endpoints. The child waits until
  the parent has released its own refs; its first ordinary EXIT closes
  the reader but gets a real pinned-MM BUSY refusal on the final writer.
  The FILE entry/ref count remains owned, EXIT returns an error, the
  child retries, and its second EXIT unpins/frees exactly once. Parent
  WAIT returns the correct exit status; no process/FILE/VM word leak.
  Isolated guest calls 2, pins 1, unpins 1, BUSY 1, accepted free 1,
  errors 0. No KCORE text or data change. K02 remains OPEN.
  **v1724:** another complete 256K PDP-6 guest extends that same RUN
  child with an inherited D6FS regular-file FD6. The first ordinary
  EXIT retains both the pinned anonymous PIPE writer FD4 and a refused
  VFS_SYNC FD6. The second EXIT commits one real PIPE MM free but the
  independent file sync refuses again; the third exits after successful
  original VFS sync. FILE occupancy transitions 2 -> 1 -> 0, parent
  WAIT and MEMINFO succeed; PIPE attempts 2, actual BUSY 1, pin/unpin
  1/1, successful free 1, file sync calls/refusals 3/2, errors 0.
  Refusals at the VFS boundary are test-injected, not organic provider
  failures. No production changes; K02 and remaining K03 gate OPEN.
  **v1725:** the real 256K PDP-6 RUN/PIPE/regular-FD EXIT sequence now
  injects two sync refusals inside the *D6FS provider entry*, after
  registered MRES dispatch, rather than at VFS. All three targeted
  calls observe the serialized provider lease held by child slot 2
  (owner_slot+1 in LH, busy in RH); zero lease violations. The real
  PIPE MM pin/BUSY/unpin/free remains 1/1/1/1 across two final-close
  calls; provider calls/refusals 3/2, all ownership errors 0 and parent
  WAIT PASS. Provider errors are still test-injected, not actual media
  faults. K02 OPEN; K03 automatic OOM termination disabled.
  **v1726:** a real three-process 256K PDP-6 guest runs independent
  children: B(slot2) retains an inherited regular-file FD6 after a
  refused D6FS provider SYNC, while A(slot3) attempts last-reference
  PIPE MM release. The PIPE hook physically verifies B's live u-area
  FD6 and its first refused provider call at BOTH A's MM attempts.
  The real MM pin/BUSY/unpin/free sequence is 1/1/1/1, overlap 2/2,
  provider calls/refusals 3/2, correct leases 3, violations 0,
  ownership errors 0. Both children finish through ordinary EXIT,
  parent WAIT receives both exit statuses, and MEMINFO returns to
  baseline. No production KCORE change; K02 remains OPEN.
  **v1727:** a physical 256K PDP-6 guest keeps B(slot2)'s D6FS FD6
  permanently refused while a real APR fault enters its private-stack
  fatal FILE-close continuation. Eight sleeps follow exponentially
  doubled intervals 1..0200 octal ticks with no mismatches; all nine
  provider SYNC calls are refused under the correct MRES lease.
  A(slot3) recovers from genuine pinned PIPE MM_BUSY, finishes
  self-HUP, and is reaped independently. Foreign HUP against B is
  refused without freeing its protected stack or FD6. B is the only
  additional occupied process slot and remains nonwaitable.
  Two physical overlap observations, zero ownership errors; four
  verifier mutations rejected. Test-only fixes corrected caller-local
  MEMINFO FILE accounting and single-character console formatting.
  This proves **bounded backoff and scheduler progress**, not bounded
  reclamation. K02 OPEN; K04 policy and K03 OOM remain gated.
  **v1728:** native source-linked PDP-6 integration of actual session
  teardown and fatal event-delivery routines. Five consecutive
  permanently refused member FILE closes retain its stable FD6 and
  the leader TTY; a separate committed VM_NONE+FINALIZE member is
  skipped. Three additional HUPs after marking the refused member's
  protected FATAL_CLOSE owner retain FD6 and TTY without re-closing
  the private-stack owner. Three unsafe source mutations rejected.
  Separate v922 session HUP, v1720 lowmem EXEC fatal and v970
  fatal-owner/HUP native regressions all pass. This is source-linked
  acceptance only; a combined physical session/provider/EXEC guest
  and K04 permanently refused ownership policy remain OPEN.
  **v1729:** full 256K PDP-6 combined session-HUP/permanent D6FS/
  destructive-EXEC guest PASS. Job2 slot4 permanently refuses real
  provider-entry SYNC for FD3, then a true APR fault enters its
  private-stack FATAL_CLOSE owner. Three session HUPs refuse without
  re-HUPing already committed job1 slot3, releasing the leader TTY,
  touching job2's protected stack or losing FD3. Physical D6FS
  calls/refusals 5/5, correct leases 5, wrong leases 0; session
  failures 3, ownership errors 0. Independent EXEC child slot5
  physically destroys old VM to VM_NONE, encounters a replacement
  load refusal and completes its genuine fatal exit; INIT WAIT and
  MEMINFO PASS. Six unsafe transcript mutations rejected; v1728,
  v1720, v970 and v922 native gates PASS. Prior foreign-HUP test
  designs failed because the safe scheduler path deferred them,
  not because the D6FS vnode mask needed a workaround. No KCORE
  code/data delta. K02 OPEN for organic media and post-free backing
  failure gates plus full regression/size audit; K04 and K03 remain
  separately gated.
  **v1730 CLOSED:** nine native source-linked PDP-6 cleanup gates and
  four full-system 256K PDP-6 physical guests all PASS on the
  unchanged v1720 production kernel: post-real-MM-free backing
  detach refusal and retry (v1721), concurrent PIPE/D6FS RUN owners
  (v1726), permanent D6FS/session HUP with independent destructive
  fatal EXEC (v1729), and named FIFO pinned-MM final-close retry
  (v900). No unowned FD, double MM release, invalid old VM resume,
  repeated finalized-member HUP, premature TTY retire, private-stack
  destruction or stuck unrelated scheduler progress. Prior native
  unsafe mutations and six v1729 transcript mutations are rejected.
  The v1729 postfree OPEN note is superseded: that exact physical
  gate already passed v1721 and passes again in v1730. Provider-
  specific device/media implementation is excluded by K02 scope.
  No production KCORE text/RAM changes since v1720; total K02
  executable growth was 30 words in v1719/v1720, fixed/per-process
  growth zero. The strict 32K permanent-address limit still fails
  independently. K03 OOM killing remains disabled; K04 permanently
  refused recovery policy remains OPEN. Full closure details:
  design/kcore-k02-closure-v1730.md.
- [ ] **K03 - OOM handling.** Define and implement an allocation-free,
  safe victim-selection and termination path only after provider, VM,
  descriptor, zombie, EXEC and unpublished-RUN teardown is safe.
  Automatic OOM killing stays disabled until that condition is proved.
- [ ] **K04 - Process teardown recovery.** Handle incomplete teardown,
  retained owners, zombies, finalizer/reaper retries and permanently
  refused resources without double release, lost ownership or unbounded
  retry loops.
  **Progress v1754:** Opt-in GETTEARDOWN/PS distinguishes retained
  pre-VM EXIT from unpublished RUN rollback without altering existing
  query ABI. Isolated assembler and C compilation pass. Still OPEN:
  booted permanent/temporary multi-owner retry, WAIT/slot reuse and
  controlled operator recovery; external testkit sweep is untouched.
  **Progress v1755:** Fixed stale fatal-FILE diagnostic precedence for
  opt-in PREVM EXIT and stale initial epoch for newly refused SIDL
  rollback. Kernel assembly PASS; physical acceptance remains OPEN.
  **Progress v1756:** Guarded zero stable-u-area base before deferred
  finalizer marker read; preserve pending quarantine on inconsistency.
  Isolated PDP-6 assembly PASS; physical acceptance remains OPEN.
  **Progress v1757:** Changed the preexisting zero-base PREVM guard
  from SKIP to FAILED to preserve pending quarantine. The original
  path already avoided low-memory dereference; DAS PASS, physical OPEN.
  **Progress v1758:** EXIT resume rejects FINALIZE|DISCARD; rollback
  cannot be misclassified as an EXIT owner. DAS PASS; K04 OPEN.
  **Progress v1759:** Opt-in PREVM diagnosis bypasses fatal FILE
  precedence only with exact FINALIZE|PREVM marks. DAS PASS; K04 OPEN.
  **Progress v1760:** Prevented HUP redelivery to committed PREVM
  EXIT owners; foreign fatal delivery now accepts exact pending PREVM
  state while provider release remains deferred. DAS PASS; K04 OPEN.
  **Progress v1761:** Post-VM MM-only finalizers no longer count as
  TTY session members; PREVM and incomplete owners retain their
  ownership. Invalid u-area bases fail closed for release and SETFG.
  DAS PASS, physical multi-owner acceptance OPEN.
  **Progress v1762:** VM_NONE FINALIZE|DISCARD rollback cannot satisfy
  fatal HUP completion; EXIT and rollback remain distinct. DAS PASS;
  K04 physical acceptance OPEN.
  **Progress v1763:** Added an owner-local armed marker in existing
  FINALIZE word 067 so a newly failed prepublished EXIT PREVM owner
  starts its first retry interval at the current epoch even when
  another owner is already pending. No added resident RAM; DAS PASS.
  **Progress v1764:** WAIT no longer consumes stale STOP/CONT from
  TRANSITION finalizers; zero-base u-area owners fail closed in WAIT
  reporting and session HUP. DAS PASS; physical K04 acceptance OPEN.
  **Progress v1765:** Moved shared backoff advancement before the
  reaper scan so a first refusal registered during that pass keeps
  its 020-octal-tick initial interval. Fully completed scans clear
  the unused delay. DAS PASS; physical K04 acceptance OPEN.
  **Progress v1766:** Session HUP now requires terminal or committed
  FINALIZE publication, not merely a queued asynchronous fatal event,
  before retiring the controlling TTY. DAS PASS; physical K04 OPEN.
  **Progress v1767:** INIT cannot adopt orphans while it owns a
  deferred EXIT or fatal FILE-close continuation; zombies use the
  parentless cleanup fallback. Parent CHLD notification checks a
  valid resident u-area before writing. DAS PASS; K04 physical OPEN.
  **Progress v1768:** A child exiting while its parent is already
  in deferred EXIT/fatal FILE close now publishes under healthy
  INIT or frees its terminal slot if INIT cannot WAIT. Adoption,
  zombie publication and CHLD use matching teardown ownership
  checks. DAS PASS; physical refusal/WAIT/PID tests remain OPEN.
  **Progress v1769:** Repaired obsolete Stage-1 Makefile decoder
  prerequisites for DSK/DTC/DRM/MTC/PTR. Fresh isolated full
  `make -C system build` PASS; emulator fault-injection and
  WAIT/PID acceptance still OPEN. External testkit untouched.
  **Progress v1770:** Native KCC bootstrap packaging repaired and
  installed; fresh 256K PDP-6 full boot image PASS. Actual SIMH
  INIT/ROOT/DSH/PS/TRUE/guest HALT PASS; a second clean boot and
  D6FS CLEAN fsck PASS. No fault injection performed. Retained
  owner, multi-owner, clock-wrap, HUP, WAIT/PID and operator
  recovery physical acceptance still OPEN; testkit untouched.
  **Progress v1771:** Source-audited physical acceptance matrix
  committed (design/kcore-k04-multiowner-acceptance-plan-v1771.md).
  No refusal injection run; all K04 recovery acceptance gates OPEN.
  **Progress v1772:** Real PDP-6 ROOT/DSH ran five TRUE children;
  subsequent PS reused PID 5, all normal teardown fields zero.
  Guest HALT and D6FS CLEAN seq=7 PASS. Intermittent host SIMH
  signal 11 on an earlier launch remains unresolved. Refusal-held
  owner/WAIT/PID acceptance OPEN; external testkit untouched.
  **Progress v1773:** Fixed fatal-event WAIT_CHILD hang and pending
  event at WAIT entry. Native DAS/full 256K image PASS; real PDP-6
  CTY/DCS negative control v1770 kept blocked USER/CAT after TERM;
  v1773 released them, reusing PID5. Guest HALT/D6FS CLEAN both.
  +5 KCORE TEXT words, no fixed DATA/BSS. MM/FILE refusal,
  multi-owner, clock-wrap and refused-owner WAIT/PID still OPEN.
  **Progress v1774:** Real PDP-6 with synthetic MM_ERR_BUSY only at
  u-area release retained child PID5 as post-VM FINALIZE+ARM over
  556 qualified ticks. Restored original MM call; TRUE resumed,
  PS reused PID5, HALT/D6FS CLEAN PASS. Physical MM pin and
  multi-owner/clock-wrap/FILE/HUP acceptance still OPEN.
  **Progress v1777:** Two simultaneous real MM-pinned post-VM
  owners tested on disposable PDP-6: PID6 unpinned and reused by
  PS while PID5 stayed pinned in TD=2 RETRY=512 DETAIL=6.
  Both owners finally released; PID5 attempt 030, PID6 005,
  guest HALT and D6FS CLEAN seq=3 PASS.
  Independent-owner progress PASS; exact first-refusal timing,
  SIDL/PREVM, clock wrap, FILE/HUP, dying-parent WAIT still OPEN.
  **Progress v1776:** Real physical u-area MM pin exercised on a
  disposable PDP-6 image using mm_pin/mm_unpin, with normal mm_free
  rejecting the live pin. PID5 post-VM TD=2, RETRY=512, DETAIL=6
  (one physical pin) persisted; release on attempt ten completed
  and PID5 was reused. Guest HALT/D6FS CLEAN PASS. No production
  RAM growth, testkit untouched. Multiple owners, SIDL, FILE/HUP,
  clock wrap and dying-parent WAIT still require physical tests.

  **Progress v1778:** Physical PDP-6 two-owner maximum-delay
  acceptance PASS: owner A saturated shared interval 01000;
  genuinely new owner B reset to 020 after first real MM refusal.
  Captured qualified clock delta to B's next retry exactly 020.
  B unpinned and PID6 reused while A stayed pinned at RETRY=512.
  Guest HALT/D6FS CLEAN seq=7; no production/testkit changes.
  SIDL/PREVM, 17-bit wrap, FILE, HUP, dying-parent WAIT OPEN.

## Mandatory KCORE: PDP-6 RAW PURE

- [ ] **K05 - Reclaim/restore failure handling.** Complete safe RAW
  prefix reclaim and FULL restore on pin, MM, allocator, backing and
  interruption failures. Preserve original process state and ownership
  when publication cannot be completed.
- [ ] **K06 - MM ownership uncertainty.** Preserve exact PROCESS/PACKED
  ownership, pins, TRANSITION and READY_ONCE state across persistent
  pin/free/rebase errors, restore cancellation and terminal cleanup.
  Never publish an ambiguous APR or release the sole retained suffix.
- [ ] **K07 - Executable backing lifetime.** Guarantee immutable PURE
  source lifetime across reclaim, source loss/replacement, RO/provider
  failure and unmount attempts. Provide bounded FULL fallback and safe
  owner retention when the source cannot be trusted.
- [ ] **K08 - Scheduler integration.** Complete real PI6 interruption,
  wake/retry, READY_ONCE, WAIT and EXIT behavior during multi-process
  memory pressure. Respect slot-0 FULL serialization and guarantee
  progress without fabricated concurrent user dispatch.
- [ ] **K09 - Mainline integration.** Reconcile the PDP-6 RAW PURE
  implementation with current mainline KCORE interfaces and integrate
  it only after its correctness dependencies are satisfied. Preserve
  default-OFF behavior until accepted. Repository-wide branch cleanup,
  hardware/root matrices and release acceptance are separate work.

## Optional KCORE: compressed mutable memory (mode3)

- [ ] **K10 - Compression admission.** Define a backwards-compatible
  DXR COMPRESSIBLE_MUTABLE opt-in, distinct from COMPRESSED executable
  payload and BSS flags. Handle legacy/invalid headers, fork, EXEC
  replacement and privileged admission correctly.
- [ ] **K11 - Bounded compression.** Complete source-linked, bounded
  overlapping encoding and decoding with read-only preflight, exact
  encoded size, RAW fallback and nonshrinking-output rejection.
  Cover 35/36/37-word boundaries and sparse/dense data.
- [ ] **K12 - Temporary memory ownership.** Prove scratch lifetime and
  exclusive ownership, preferentially using the still-owned prefix.
  Otherwise use bounded pinned no-reclaim MM scratch. Never borrow
  freed memory or introduce an always-resident compression slab.
- [ ] **K13 - Atomic compressed ownership.** Integrate bitmap DROP with
  real reclaim and MM ownership. Prove source survival, overlap safety,
  complete encoding and atomic RAW-versus-BITMAP publication, with
  rollback and no partially published state.
- [ ] **K14 - FULL restoration.** Integrate bounded decoding, immutable
  backing/fallback, 2048-word APR limits, target pin ordering and safe
  restoration into a complete runnable process image.
- [ ] **K15 - Compressed process lifecycle.** Complete RUN, EXEC,
  SLEEP, wake, WAIT, EXIT, PI6 retry, terminal detach and VM destroy
  for mode3, with exact finalizer/owner accounting. Keep the existing
  mode3 destroy refusal until the lifecycle is demonstrably safe.
- [ ] **K16 - Failure recovery.** Handle corruption, source loss,
  allocator refusal, torn or overlapping data, uncertain MM ownership,
  restore cancellation and terminal failure without leaks, double
  release or unsafe process resumption.

## Execution boundary

1. Finish K01-K04 correctness prerequisites.
2. Finish K05-K09 mandatory PDP-6 RAW PURE implementation.
3. Implement K10-K16 only as the optional compressed-memory feature;
   mode3 stays disabled until safe and accepted.

Each item is a feature-completion task, not a claim of a newly reproduced
bug. Full physical test matrices, RAM tuning, optimization, hardware
backends, filesystem features and post-feature release work are tracked
separately in the consolidated `TODO.MD` and related documentation.
