# KCORE feature TODO (K01-K16)

Updated: 2026-11-24 (v1907 focused foreign-owner terminal refusal physically PASS; K01-K04 closed).

This is the **implementation-only** KCORE backlog. Work on these sixteen
items is the current scope. Filesystems, storage utilities, hardware backends,
toolchains, userland, optimization, consolidated acceptance matrices and
release administration remain outside this list. Feature-specific tests are
part of each implementation task and belong in daimos-testkit.

The accepted PDP-6 RAW PURE v1674 48K GENERAL/STRICT committed-MM
high-water result is not a claim of PI6 fault-pressure, bitmap mode3,
KA10, alternate-root or feature acceptance. Its default-OFF source
history is now reachable from main (v1809); mode3 remains disabled.

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
- [x] **K03 - OOM policy (CLOSED v1803).** Retain the accepted
  requester-only BRK physical-NOMEM termination (F12/F13 v1298,
  `PROC_OOM_BRK_ENABLED_V1000,1`), and prefer existing process
  swapping over killing unrelated processes. **Cross-process OOM
  victim termination is explicitly rejected** as unnecessary kernel
  complexity and does not become a deferred implementation task.
  K04 still quarantines permanently refused owners rather than
  promising immediate reclamation. No production code/RAM change.
  See `design/kcore-k01-k04-closure-v1803.md` and the v1802 audit.
- [x] **K04 - Process teardown recovery.** CLOSED v1801 (bounded
  physical ownership/recovery acceptance; see
  `design/kcore-k04-closure-v1801.md`). Handle incomplete teardown,
  retained owners, zombies, finalizer/reaper retries and permanently
  refused resources without double release, lost ownership or unbounded
  retry loops.
  **Closure v1801:** physical PREVM/post-VM/SIDL, FILE, HUP,
  permanent refusal, healthy/ineligible INIT and late-child FREE/WAIT
  accepted. Consolidated testkit verifier PASS; final v1800-A/B
  post-HALT readouts and CLEAN D6FS reverified. Untested mixed-owner
  wrap, pinned published-member HUP and organic media errors are
  recorded as bounded residual risks, not accepted physical tests.
  K03 cross-process OOM killing is explicitly out of scope (v1803).
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

  **Progress v1780:** Real SIDL VM MM-pin rollback PASS on disposable
  PDP-6: one fully owned RUN child failed; actual mm_pin held its VM
  extent through four refused destroy attempts, real mm_unpin on
  fifth allowed discard. PID5 reused, guest HALT and D6FS CLEAN
  seq=3. No production/testkit edits. Permanent SIDL/mixed owner,
  PREVM EXIT, clock wrap, FILE/HUP, dying-parent WAIT remain OPEN.

  **Progress v1781:** Physical PREVM EXIT VM pin/refusal/recovery
  PASS in disposable PDP-6. Actual mm_pin blocked four VM destroy
  attempts; real mm_unpin on fifth completed deferred teardown.
  PID5 reused with zero TD diagnostics; guest HALT and D6FS CLEAN
  seq=3. No production/testkit edits. Permanent PREVM refusal,
  two-owner ordering, clock wrap, FILE/HUP, dying-parent WAIT OPEN.

  **Progress v1782:** Real PDP-6 mixed PREVM PID5 / post-VM PID6
  physical MM pins PASS. With A already saturated at octal 01000,
  B first refusal resets to 020; measured second attempt exactly
  020 qualified ticks later. B MM release/reuse completes while
  A retains TD=3/PVM=1, then A releases on its 24th attempt.
  Shared pending resets, both slots reusable, guest HALT and
  D6FS CLEAN seq=3. Scratch fixture only. Reverse ordering,
  SIDL B, clock wrap, FILE/HUP, dying-parent WAIT remain OPEN.

  **Progress v1783:** Real PDP-6 reverse mixed post-VM A /
  PREVM B physical MM pin/refusal/recovery PASS. A capped at
  octal 01000, B's new PREVM failure reset first retry to 020;
  measured second attempt exactly 020 qualified ticks later.
  B released and PID6 reused while A stayed TD=2 RETRY=512,
  then A released after 24 attempts and PID5 reused. Final
  guest HALT, D6FS CLEAN seq=3. Fixture-only image. Both
  mixed PREVM/post-VM orderings now PASS. SIDL mixed ordering,
  clock wrap, FILE/HUP, dying-parent WAIT remain OPEN.

  **Progress v1784:** Real PDP-6 post-VM PID5 / unpublished
  SIDL PID6 mixed physical MM owners PASS. A at saturated 01000;
  B new SIDL first refusal resets to 020 and second actual VM
  destroy attempt is exactly 020 qualified ticks later. B
  unpinned and PID6 reused while A remains TD=2 RETRY=512;
  A later unpinned at attempt 030, PID5 reused, pending clears,
  guest HALT and D6FS CLEAN seq=3. Scratch-only fixture.
  Reverse SIDL/post-VM, SIDL/PREVM, clock wrap, FILE/HUP and
  dying-parent WAIT remain OPEN.

  **Progress v1785:** Real PDP-6 unpublished SIDL PID5 /
  post-VM PID6 physical-MM mixed owners PASS. With SIDL A at
  maximum octal 01000, B first refusal resets to 020; its
  second actual MM attempt is exactly 020 qualified ticks
  later. B unpins on attempt 005 and PID6 is reused while A
  remains SIDL TD=3 RETRY=512. A unpins on attempt 030, PID5
  reused, pending clears; guest HALT and D6FS CLEAN seq=3.
  Only disposable scratch instrumentation, no production edit.
  Both SIDL/post-VM orderings PASS. SIDL/PREVM, clock wrap,
  FILE/HUP, dying-parent WAIT and permanent refusal OPEN.

## Mandatory KCORE: PDP-6 RAW PURE

- [ ] **K05 - Reclaim/restore failure handling.** Complete safe RAW
  prefix reclaim and FULL restore on pin, MM, allocator, backing and
  interruption failures. Preserve original process state and ownership
  when publication cannot be completed.
  - v1808 PASS: native PDP-6 both original prefix and FULL
    restores remain runnable after a lost successful return.
    Fixed KCORE unchanged; opt-in PURE MRES shrinks by one
    instruction. Remaining: source/allocator loss and
    genuine physical interrupted/faulted restore with exact
    owner, pin and persistent failure witnesses. See
    `design/kcore-k05-restore-reply-v1808.md`.
  - v1839 PASS: original source-linked PDP-6 FULL restore now
    distinguishes a successfully freed PACKED suffix with a lost
    `mm_free` reply from a genuine refusal and a foreign-owner
    ambiguity. An exact sole unpinned PROCESS APR completes restore;
    exact retained source rolls back only the temporary destination;
    uncertain owners stay TRANSITION-held. Native v1839 and its two
    executed unsafe mutants pass, alongside the v1807/v1808 combined
    native integration matrix. This adds code to optional PURE MRES,
    not fixed KCORE. Full physical PI6/fault, source/allocator and
    persistent owner acceptance remain OPEN. See
    `design/kcore-k05-source-free-reply-v1839.md`.
  - v1845 scoped physical PASS: current-source 48K PDP-6 original
    FULL restore survives actual secondary DTC3 disappearance during
    a pinned two-extent transaction. The original loader reports -4;
    unchanged RAW PACKED suffix remains unpinned and the provisional
    PROCESS target is freed; exact same reel return permits four
    secondary-backed children to finish and parent WAIT/HALT passes.
    Five-child pressure fixture is explicitly distinguished from the
    obsolete six-child fixture that fails admission on current source.
    All five physical MM stages and two original-trace negative mutants
    PASS, with identical fixed KCORE/PURE DXRs between those fixtures.
    No production code or fixed RAM growth. Physical active PI6 MM
    interruption, persistent allocator/pin uncertainty, other source
    failures and full K05 acceptance remain OPEN. See
    `design/kcore-k05-secondary-dtc3-physical-v1845.md`.
  - v1847 scoped physical PASS: after original v1845 FULL source read
    fails on detached DTC3, the actual permanent KCORE PCLK PI6 handler
    executes at 017205 octal before DTC3 reattachment. Its instruction
    is the source-matched `AOS 1,proc_timer_clock`; clock/cooldown/cursor
    are octal 021431/0100/3. Physical MM extent/pin/owner descriptors
    and selected ROOT are identical to the failed restore rollback.
    Same-reel recovery, all five EXIT/WAIT and HALT pass; two forged
    physical clock/owner witnesses are rejected. An earlier apparent
    second MRES FULL callback with invalid AC2/AC4 was rejected, and a
    test-only KCORE DXR-offset opcode expectation was corrected. No
    production source or RAM change. Real PI6 INSIDE pinned restore,
    permanent MM faults and K05 closure remain OPEN. See
    `design/kcore-k05-postfault-pi6-v1847.md`.
  - v1848 scoped physical PASS: actual PDP-6 PCLK PI6 at permanent
    KCORE `017205` interrupts original FULL with BOTH slot3 extents
    still singly pinned (PACKED source `111200..120000`, PROCESS
    target `120000..130000`). Original saved PI6 return PC `037637`
    lies in source-linked PURE MRES, offset `005220` from the physical
    reclaimed-JRST-derived load base. All original MM descriptors,
    pins and TSFS root are identical before/at the clock interrupt.
    Only AFTER the clock does actual DTC3 disappear; the original FULL
    loader returns -4 and rolls back the target without losing the
    RAW source. Same-reel return, five EXIT/WAIT/HALT and three
    executed source-trace negative controls PASS. No kernel/driver/MM
    code or fixed-RAM increase. This closes the specific physical
    mid-FULL PI6 evidence gap, NOT arbitrary PI6 scheduling timing,
    persistent unknown MM pin/free/allocator faults or K05 closure.
    See `design/kcore-k05-midfull-pi6-v1848.md`.
  - v1855 production bugfix and scoped native/physical PASS: a second
    target pin could survive successful `mm_unpin(target)` and allow
    deletion of the sole RAW suffix before RESIDENT publication.
    Original KCC PDP-6 native FAIL `002002` reproduced this. The
    corrected PI-atomic exact two-owner check requires a singly pinned
    PACKED source and singly pinned complete PROCESS target, with no
    duplicates/foreign ownership, before any commit unpin/free.
    It shares the existing v1839 zero-pin source-free reply scan;
    unknown owners remain TRANSITION-quarantined. Full native matrix,
    original v1654/v1839 mutants, new allocator/source/target overpin
    fixtures and two unsafe v1855 mutants PASS. A fresh physical 48K
    five-child normal PURE RUN/PIPE/EXIT/WAIT/HALT also PASSes; fixed
    KCORE byte-identical, optional PURE MRES +000027 octal (23 decimal)
    words, no BSS change. Physical extra-pin fault injection and
    bystander progress during quarantine remain OPEN. See
    `design/kcore-k05-exact-pinned-full-v1855.md`.
  - v1890 scoped original physical PASS: a freshly source-matched
    actual 48K PDP-6 FULL restore has its slot3 target MM metadata
    physically changed from pin1 to pin2 by test-only SIMH deposit
    at the live loader checkpoint `036202`. The original bounded
    restore returns MM_ERR_BUSY (-5) at PC `033067` without losing
    the sole PACKED RAW source (pin1) or provisional PROCESS target
    (pin2). All unrelated MM extents, root and current-slot state are
    unchanged. A later real PCLK PI6 at `017205` sees the exact same
    retained MM owner set. Four independent forged physical traces
    are rejected by source-linked negative controls. No source code,
    fixed KCORE, optional MRES, data RAM or defaults changed. This is
    **not** a guest WAIT/HALT, observed TRANSITION-flag or unrelated
    bystander-progress witness; those and other persistent fault
    cases remain OPEN. See `design/kcore-k05-physical-overpin-v1890.md`.
  - v1891 scoped original physical PASS: after the exact v1890
    real 48K overpin/-5/PCLK witness, the actual KCORE proc_table
    pointer resolves to `044712` octal. Original slot3 at `044723`
    has TRANSITION **set**, VM base zero, SRUN; unrelated slots4..6
    remain distinct nontransition SRUN owners with VM base zero.
    Parent slot1 stays asleep awaiting the incomplete child. Four
    independently executed process-word negative controls reject
    missing quarantine, false APR publication, invented bystander
    quarantine and stale proc_table pointer. The full actual MM/root
    oracle and postfault PCLK PASS; unchanged KCORE/PURE image and
    zero new RAM. NO user peer completion or guest HALT is claimed:
    the retained pin2 blocks all remaining aligned `010000`-word
    FULL destinations in the observed 48K arena. A new resident
    user peer or appropriately scoped free-memory fixture is needed
    to test bystander user-level progress. K05/K06-K09 still OPEN.
    See `design/kcore-k05-process-quarantine-v1891.md`.
  - v1892 original 48K physical resident-user progress PASS: a new
    test-only parent INIT loop runs AFTER unchanged five-child
    RUN/PIPE/BRK pressure/`L`, without permanently consuming another
    full PROCESS APR. Its actual parent slot1 remains resident at
    `130000..140000`, and emits TWO new user-mode `HB` markers
    after a physically injected target pin2 forces genuine original
    FULL `MM_ERR_BUSY=-5`. At both new physical output stops the
    original scheduler current slot is 1, parent PROCESS is SRUN,
    its resident APR is pin0, and the source-linked MM table keeps
    slot3 PACKED RAW pin1, temporary target pin2, TRANSITION set,
    VM base zero and all unrelated owners unchanged. The original
    clock advances `021610` -> `021710` octal between new user
    heartbeats. Full original physical owner/root/PI6 audit and SIX
    executed forged heartbeat/owner/clock controls PASS. Fixed KCORE
    and optional PURE MRES are byte-identical; ZERO production RAM
    or code growth. This closes the resident-parent forward-progress
    gate, NOT sibling PACKED restores, child EXIT/WAIT/HALT after
    quarantine, terminal cleanup, permanent allocator/unpin/free
    variants or full K05-K09 acceptance. See
    `design/kcore-k05-resident-parent-v1892.md`.
  - v1893 original 48K physical PACKED source-free lost-reply PASS:
    compiled original source DOBJ relocs and linked PURE instruction
    uniquely identify source `mm_free` at `035310` and physical
    return `035311` octal. The original kernel REALLY completes
    successful `mm_free(PACKED)` after both original unpins:
    AC1=0, source `111200..120000` truly deleted, sole complete
    unpinned PROCESS `120000..130000` retained; original MM extent
    count 18 -> 17, unrelated owners and TSFS root unchanged.
    A single TEST-ONLY SIMH AC1 deposit replaces only that success
    reply with `777777777773` (-5). Unmodified original v1839
    exact zero-pin owner reconciliation sees sole PROCESS and
    correctly publishes slot3 VM `120000`, clears TRANSITION,
    returns MM_OK=0 at original service return `033067`.
    Original five-child user EXIT/WAIT/status/sentinel checks,
    parent Z and HALT complete, slot3 MM extents finally freed.
    Independent full physical audit and SIX executed owner/reply/
    false-publication negative controls PASS. Permanent KCORE/PURE
    byte-identical, ZERO production RAM/source changes. This closes
    the **physically committed free + falsely failed reply** case,
    not a genuinely refused/persistent free with BOTH original
    MM owners, foreign owner, rollback failure, allocator/pin errors
    or K05-K09. See
    `design/kcore-k05-physical-source-free-reply-v1893.md`.
  - v1894 original 48K physical genuine MM source-free refusal PASS:
    original compiled/linker source call `035310` executes REAL
    original `mm_free(PACKED)` while an external test-only source
    MM pin1 is held; its physical return at `035311` is ACTUAL
    MM_ERR_BUSY=-5, not a forged return. Both original owners stay
    intact. External pin is released, proving exact PACKED RAW
    `111200..120000` and complete PROCESS `120000..130000`, both
    pin0. Original v1839 zero-pin owner reconciliation safely
    releases ONLY the temporary PROCESS target, keeps RAW source
    pin0 and slot3 VMbase0, returns actual -5 at `033067`.
    Original five children recover and complete PIPE/sentinel/
    typed EXIT/WAIT, parent Z, real PDP-6 HALT. Final slot3 MM
    owners gone, TSFS ROOT preserved. Full original physical audit
    and SEVEN separately executed unsafe physical negative controls
    PASS. Production fixed KCORE/PURE MRES byte-identical, no RAM
    growth. This closes transient REAL source-free refusal and
    checked two-owner rollback, NOT permanent extra pin/refusal,
    failed target release, unknown allocator ownership, terminal
    quarantine cleanup, arbitrary PI6 or K05-K09 release.
    See `design/kcore-k05-physical-real-source-free-refusal-v1894.md`.
  - v1895 persistent original 48K physical source-free -5 PASS:
    original unchanged fixed-KCORE `mm_free(PACKED)` genuinely
    refuses source physical pin1 at native call `035310`/return
    `035311` with actual AC1=-5. Unlike v1894, the test-only
    external source pin is NEVER released. Exact source RAW
    `111200..120000` pin1 and complete temporary PROCESS
    `120000..130000` pin0 both remain physically owned by slot3.
    Original v1839 zero-pin MM owner test fail-closes: FULL returns
    -5 at `033067`, keeps slot3 TRANSITION/VMbase0, does NOT
    free source/target or publish a runnable APR. All 18 exact
    original MM descriptors and TSFS root remain unchanged through
    the original service return and two **new** parent user-mode
    heartbeat outputs. Parent slot1 resident PROCESS at
    `130000..140000` pin0 remains running; original PCLK clock
    advances `016640` -> `016740` octal between heartbeats.
    Original physical independent audit and TEN executed unsafe
    physical MM/owner/process/HB/PCLK trace negatives PASS.
    KCORE/PURE images byte-identical and ZERO production RAM
    growth. This closes **persistent source pin -> safe quarantine
    + independent resident parent progress**, NOT eventual owner
    release, other child restore, normal WAIT/HALT, terminal
    quarantined-owner cleanup, unknown allocator/rollback errors
    or K05-K09. See
    `design/kcore-k05-physical-persistent-source-pin-v1895.md`.
  - v1896 original 48K physical genuine temporary target cleanup
    `mm_free(PROCESS)` refusal PASS. Original compiled FULL DOBJ
    has two distinct original linked `mm_free` calls: source
    `035310` and target cleanup `035371` octal. After the REAL
    original source `mm_free(PACKED)` refuses external source pin1,
    private source pin is released. Original v1839 exact-owner
    check sees both pin0 and calls actual cleanup free for the
    temporary full PROCESS target. Test-only target physical
    MM meta `024705` is now pinned 0->1 at this exact original
    native call; actual unchanged fixed-KCORE `mm_free(PROCESS)`
    returns its own `MM_ERR_BUSY=-5` at `035372`, retaining BOTH
    authoritative source `111200..120000` pin0 and full target
    `120000..130000` pin1. Original FULL returns -5 without
    releasing target/source or publishing a runnable APR; slot3
    remains TRANSITION/VM0, all 18 original MM extents and root
    unchanged. Two new original user `HB` parent syscalls execute
    from independent resident slot1 (`130000..140000` pin0),
    original clock advances `015401` -> `015564` octal. Original
    read-only physical audit and THIRTEEN executed unsafe physical
    negative mutations PASS. Production KCORE/PURE byte-identical,
    ZERO additional RAM. This closes actual native refused target
    rollback with persistent pin and parent progress, NOT target
    free lost-successful-reply, final owner cleanup, permanent
    unpin/allocator ambiguity, other PACKED child restore, all
    child WAIT/HALT or K05-K09. See
    `design/kcore-k05-physical-target-free-refusal-v1896.md`.
  - v1897 original 48K physical successfully completed temporary
    PROCESS-target cleanup free with externally lost success reply
    PASS. Original source-linked fixed-KCORE `mm_free` call for
    PACKED source `035310` genuinely refuses a temporary external
    source pin with its own -5; source pin is then released so the
    exact v1839 zero-pin two-owner scan safely chooses rollback.
    The original `fail_target` cleanup `mm_free(PROCESS)` at
    `035371` runs NORMALLY with target pin0; native original AC1=0
    at `035372` and actual physical MM table drops 18->17, with
    target GONE and sole authoritative PACKED source pin0.
    ONLY AFTER true deletion is native AC1 overwritten by one
    test-only SIMH deposit to -5. Original FULL returns -5 at
    `033067`, slot3 remains TRANSITION/VMbase0, RAW source and
    all unrelated MM/ROOT preserved. The original five child
    PIPE/sentinel/typed EXIT/WAIT tests subsequently RETRY and
    complete, parent Z/real PDP-6 HALT, final 8 MM extents and
    no residual slot3 owners. Original physical independent audit
    and TWELVE separately executed unsafe native MM/process/HALT
    transcript mutants PASS. Fixed KCORE/PURE binaries remain
    byte-identical; ZERO added production words. This closes
    the completed target-free + false failed reply branch,
    NOT permanent unpin/foreign owners/uncertain terminal cleanup,
    arbitrary PI6 or K05-K09. See
    `design/kcore-k05-physical-target-free-lost-reply-v1897.md`.
  - v1898 original 48K PDP-6 physical double target `mm_unpin`
    genuine -4 PASS. The compiled original FULL function's first
    target commit `mm_unpin` at `035271` and cleanup retry at
    `035362` octal are checked against original linked native
    opcodes and both actually execute. After the v1855 precommit
    owner pair is physically pin1/pin1, one test-only debugger
    deposit makes target `120000..130000` pin0 before its original
    native unpin call. The unmodified KCORE `mm_unpin` REALLY
    returns `MM_ERR_INVAL=-4` twice, preserving the sole RAW
    PACKED source `111200..120000` pin1 and complete temporary
    PROCESS target pin0 (18 original MM records). Original
    `fail_target` returns BUSY, preserves slot3 TRANSITION/VM0,
    does not free either owner. Two new genuine resident parent
    HBs from slot1/SRUN/valid APR and original PCLK advancement
    `016325` -> `016425` octal PASS. Full physical read-only audit
    and THIRTEEN separately executed unsafe original MM/process/
    clock trace mutations PASS. Original fixed KCORE/PURE binaries
    byte-identical; ZERO added production words.
    IMPORTANT: The test-only injected pin0 after the original
    PI-atomic exact-one-pin guard is not an ordinary scheduler
    race. This closes *defensive actual double-unpin refusal with
    unexpected MM state*, NOT genuine naturally occurring source
    unpin failure, foreign owner, quarantined terminal cleanup,
    child WAIT/HALT or K05-K09 release. See
    `design/kcore-k05-physical-target-unpin-refusal-v1898.md`.
  - v1899 original 48K PDP-6 genuine SOURCE `mm_unpin` -4 physical
    PASS, distinct from v1898 target double unpin -4. Original
    linked source-unpin call `035276`/return `035277` runs after
    successful target unpin and v1855 exact owner pair guard.
    Private test-only SIMH zeros only original RAW source MM
    metadata at `024703` pin1->pin0. The ORIGINAL native MM
    executes and refuses -4, leaving both source RAW
    `111200..120000` pin0 and full PROCESS target
    `120000..130000` pin0, MM18. The original `source_uncertain`
    path avoids a speculative second source unpin and safely
    releases ONLY the temporary PROCESS via REAL native
    `mm_free` at `035371`/return `035372` AC1=0. Actual target
    vanishes, MM18->17, sole authoritative PACKED source pin0.
    Original FULL returns -4 with slot3 TRANSITION/VM0 and
    unchanged ROOT/other MM owners. Unchanged original five
    children retry/typed EXIT/WAIT, parent Z and hardware HALT,
    terminal MM8 with no slot3 owner. Independent physical audit
    plus THIRTEEN separate unsafe source/MM/return/APR/HALT
    negative transcripts PASS; original fixed KCORE/PURE bytes
    and RAM unchanged. As v1855 commit is PI-disabled, the
    test-only pin0 state is NOT a natural interrupt race.
    Remaining: lost successful source unpin reply, foreign owner,
    permanently quarantined terminal cleanup, PI6 variations,
    KA10/64K and K05-K09 release. See
    `design/kcore-k05-physical-source-unpin-refusal-v1899.md`.
  - v1900 scoped correction: original terminal RAW PURE MRES
    incorrectly rejected the real post-FULL **two-owner** state
    (PACKED source and complete temporary PROCESS target) even
    after external pins cleared. An original native PDP-6
    reproduction failed octal `001001` before the fix. The opt-in
    PURE terminal ASM now validates exactly one unpinned RAW
    source and optional exactly one unpinned complete aligned
    PROCESS target, with PI-excluded bounded MM scan and actual
    `mm_extent_capacity` check. Release target first, source last;
    uncertain/free-refused outcomes retain the authoritative RAW
    for original FINALIZE+PREVM retry; lost source-free success
    can publish only after exact PI-excluded owner reconciliation.
    Native original terminal/VM/KCORE detach and FOUR executed
    unsafe ASM mutants PASS. A stronger actual original PDP-6
    allocator `mm_pin`/`mm_unpin`/`mm_free` + idle FINALIZE+PREVM
    retry and full zombie completion PASS. Old native v1609,
    v1680, v1681 and v1855 suites PASS. Rebuilt full-system 48K
    PDP-6 physical original FULL genuine source-free -5,
    unchanged MM18 slot3 TRANSITION/VM0 and two separate resident
    parent HBs with advancing PCLK PASS; runtime proc_table
    correctly relocates `044712`->`045014` after PURE MRES growth.
    TEN executed unsafe physical transcript mutants PASS; all
    v1893-v1899 physical audits and 74 controls re-PASS. Optional
    PURE terminal text +66 words, fixed KCORE **ZERO** growth,
    no new resident process state or BSS. **Still OPEN**: full
    48K physical foreign EXIT/kill/idle retry on a quarantined
    two-owner child, persistent pin release, final actual cleanup
    and WAIT/HALT; other foreign owner/PI6 cases. See
    `design/kcore-k05-terminal-dual-owner-v1900.md`.
  - v1901 original source-built 48K PDP-6 physical user
    `SYS_PROCCTL_EVENT_PID` / `SYS_EVENT_TERM` on EXACT slot3
    two-owner FULL failure: **safe refusal PASS**. Parent INIT
    uses a test-only user-APR gate `135265` installed only AFTER
    real original MM source-free -5/first postfault HB, preventing
    premature TERM. Genuine user syscall returns refusal `TR`;
    child still has source PACKED RAW pin1, target PROCESS pin0,
    MM18, TRANSITION+VMbase0. Original saved context PC
    `340000016664` has USER-return bit CLEAR, so freeing its
    private executive stack would be unsafe. Original FDCTL,
    FINALIZE=0 and all MM owner/pin identities remain unchanged,
    independent parent HB/real PCLK `022015` -> `022115` PASS.
    NINE new + TEN historical independently corrupted physical
    controls correctly rejected. Zero production KCORE/PURE/
    data/RAM/default change. Actual missing lifecycle stage:
    safely reconcile exact two-owner failed FULL AFTER release
    of the held pin, preserving saved executive continuation;
    current `ppure_retryable_v1612` insists on sole MM owner and
    leaves dual-owner TRANSITION frozen. Do NOT bypass the
    original foreign-event guard or forge FINALIZE. Add a bounded
    slot-0 exact-owner target rollback/recovery path, then test
    real owner continuation, fatal delivery, terminal MM release
    and parent WAIT/HALT. See
    `design/kcore-k05-transition-term-v1901.md`.
  - v1902 scoped production recovery: optional PURE MRES verifies
    exact two-owner PACKED RAW + complete PROCESS target after
    genuine FULL `mm_free(PACKED)` refusal, and a 14-word
    permanent KCORE idle-backoff extension reuses the existing
    swap-retry marker and service trampoline. From slot0, native
    MM frees only the unpinned provisional PROCESS extent;
    original MM table postflight handles lost success replies,
    preserves private u-area/FDCTL/backing and only then clears
    TRANSITION. No new per-process fields or BSS. Source-built
    original 48K PDP-6 compressed TSFS, five-child workload
    and resident parent physical PASS: native -5, held source
    pin, actual external pin release, MM count 18 -> 17 -> 18,
    original slot3 TRANSITION held -> cleared -> reacquired
    by a new FULL attempt, u-area `046635` retained. Native
    KCC refusal/ambiguity/FINALIZE/backing tests PASS; all
    TWELVE independent physical negative controls reject.
    Stronger independent original 48K physical proof reaches
    actual native `mm_free(PROCESS)` call `033245`, return
    `033246` (MM_OK=0), and original transition-clear store
    `033275` in slot0; saved executive PC/FDCTL remain exact,
    independent parent PCLK advances. Two independent physical
    builds pass and SIX additional native-target negative
    controls reject (**18 total v1902 physical controls**).
    Original v1900 native FINALIZE+PREVM/dual-owner terminal
    regression PASS with the new idle dispatch branch.
    This is not the final FULL commit or parent WAIT/HALT.
    Next: demonstrate the new FULL commit, preserved executive
    continuation, original child EXIT and typed parent WAIT/Z;
    then permanent foreign MM ownership and terminal teardown.
    See `design/kcore-k05-dual-owner-reconcile-v1902.md`.
  - v1903 original source-built 48K PDP-6 physical PASS closes the
    v1902 missing end-to-end retry gate. The first PACKED source
    native free genuinely returns -5 while pinned, original slot-0
    native PROCESS target free returns 0 after test-pin release,
    and the same private executive context survives TRANSITION
    clearance. A new FULL allocates a second target and the actual
    second PACKED source free returns **0** at original native
    return PC `036031`. Physical MM counts 18 -> 17 -> 18 -> 17;
    slot3 publishes RESIDENT/VMbase `120000` with TRANSITION clear.
    Only then a test-only USER-APR gate lets the unchanged five-child
    parent enter its original typed WAIT. All five child EXIT/WAIT
    values, parent Z and real PC-0 HALT PASS; final MM8 at Z,
    MM6 at HALT, zero residual slot3 owner, original ROOT preserved.
    Ten independent unsafe physical transcript mutations reject.
    Testkit initially exposed and corrected a SIMH `nobreak`-without-
    `all` stale-breakpoint error; final original trace is separately
    source-linked. Current installed GCC shifts linked KCORE IMAGE
    `024165` -> `024167` octal without DAIMOS source edits; optional
    PURE IMAGE stays `004615`; fixed BSS unchanged. This is NOT an
    accepted production RAM optimization nor a compiler bug claim.
    The source remains default OFF. Remaining K05: permanent pins,
    foreign/ambiguous owners, uncertain cleanup and real PI6 timing;
    K06-K09 separately OPEN. See
    `design/kcore-k05-full-commit-v1903.md`.
  - v1904 source-matched original 48K PDP-6 physically demonstrates
    exact target-pin refusal under the installed v1902 idle recovery.
    Initial native PACKED source free -5, then one test-only pin on
    the complete slot3 PROCESS target (actual MM meta `024725`,
    NOT the unrelated parent at `024727`). Three genuine resident
    parent HBs preserve MM18, authoritative RAW pin0 and PROCESS
    target pin1, TRANSITION, VMbase0, saved executive CTX_PC/FDCTL
    and original ROOT. PCLK advances, the existing KCORE retry word
    stays armed, and its delay grows `0100` -> `0200` -> `0200`
    octal without any native target-free during the pinned stages.
    After the external pin release, original native target free
    returns MM_OK=0 after `000060` PCLK ticks; old RAW survives,
    TRANSITION safely clears, next FULL commits RESIDENT, and all
    original five typed child EXIT/WAIT, parent Z/HALT PASS.
    12 independently executed unsafe physical transcript controls
    reject. v1903 source-built fixed and optional binaries remain
    byte-identical, no added resident RAM. Preliminary controllers
    accidentally pinned parent or assumed an unexecuted folded
    return PC; neither was accepted, and the actual owner/clock
    diagnostic was used to correct the test rather than discard it.
    This does **not** prove permanently held or foreign MM owner
    recovery; ambiguous cleanup, terminal refusal and other PI6
    combinations stay OPEN. See
    `design/kcore-k05-target-pin-idle-v1904.md`.
  - v1905/v1906 **production bugfix**: source-built original 48K
    PDP-6 v1905 physically reproduces premature TRANSITION unlock
    after the temporary full PROCESS target's MM owner is test-
    corrupted from slot3 to slot2. The target remains physically
    at `120000..130000` while original MM count stays 18;
    no native target free occurred. The v1902 recovery owner scan
    saw sole RAW, retired the retry bit and unblocked a process
    with an orphaned full target and live saved executive stack.
    Its original native test had deliberately allowed this
    ambiguous case. v1906 fixes the missing target *identity*,
    never by freeing a foreign owner: optional PURE records the
    original target physical base in the right half of existing
    u-area word 067 under TRANSITION, validates exact same-slot
    ownership before native free and rejects all MM extents
    overlapping that target after free, regardless of type/owner.
    No new fixed KCORE/process-table/u-area word or BSS; byte-
    identical KCORE IMAGE `024167`, BSS `000415`. Optional PURE
    IMAGE `004615` -> `004761` octal (+100 decimal words).
    KCC native original refusal/lost-reply and escrow/foreign-
    overlap tests PASS. Full original 48K PDP-6 three real parent
    HBs retain foreign owner2 target with slot3 TRANSITION and
    escrow `040000120000`, PCLK/backoff advance and exact MM18.
    After explicit test-only foreign-owner correction, original
    native target free returns 0, original FULL commits RESIDENT,
    all five typed EXIT/WAIT, Z and true HALT PASS. All 16 unsafe
    real physical transcript mutants reject. This proves safe
    quarantine, not automatic foreign-owner repair. Permanent
    foreign owner refusal, unknown allocator state, terminal and
    PI6 cases remain OPEN. See
    `design/kcore-k05-foreign-target-escrow-v1906.md`.
  - v1907 scoped original 48K PDP-6 PASS: one real parent user
    PROCCTL TERM(3) issued while actual failed FULL PROCESS target
    is foreign-claimed, slot3 TRANSITION-held, escrowed u[067]
    `040000120000`. Original event dispatcher returns refusal
    (`TR`) without touching MM18, RAW/PROCESS owners, saved private
    executive CTX_PC/FDCTL, TRANSITION, ROOT or closed WAIT gate.
    Three foreign-owner parent HBs in total remain safe. After
    explicit correction of test-only owner corruption, original
    native rollback, next FULL, five typed WAITs and HALT PASS.
    Full v1906 positive oracle re-PASSes; only four targeted
    additional unsafe transcript mutations run, ALL rejecting.
    No production source/RAM/image change. Do NOT broaden this
    into forced TERMINATE/FINALIZE: permanently unresolvable
    foreign/pinned owners and other PI6 interleavings remain
    K05 OPEN. See `design/kcore-k05-foreign-term-v1907.md`.
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
- [ ] **K09 - Production integration acceptance.** The v1809 Git
  fast-forward has incorporated the PDP-6 RAW PURE source history
  into main with the feature OFF by default. Do not confuse source
  ancestry with accepted enablement. Complete K05-K08 correctness
  dependencies and current-source physical/root matrices before
  promoting a production PURE configuration. Repository-wide branch
  retirement, hardware coverage and release acceptance are separate.

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
