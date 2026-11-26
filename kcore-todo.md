# KCORE feature TODO (K01-K17)

Updated: 2026-11-26 (v1947 K07/K08 integrated physical PASS; K01-K08 closed).

This is the **implementation-only** KCORE backlog. Work on these seventeen
items is the current scope, including the mandatory KA10 backend (K17).
Unrelated filesystems, storage utilities, other hardware backends,
toolchains, userland, optimization and release administration remain
outside this list. Feature-specific tests are
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

- [x] **K05 - Reclaim/restore failure handling (implementation CLOSED v1916).** Complete safe RAW
  prefix reclaim and FULL restore on pin, MM, allocator, backing and
  interruption failures. Preserve original process state and ownership
  when publication cannot be completed.
  **v1916 closure:** PURE reclaim now propagates hard compactor
  refusals (-4/-5) before and after DROP. An unprovable DROP owner
  remains TRANSITION-held and stops further allocator pressure.
  Native PDP-6 v1915 reproduction fails at 0601; corrected v1916
  passes five focused cases plus the existing service regression
  and unsafe READY_ONCE negative control. Ordinary resource
  pressure and safely rejected source admission remain retryable.
  Fixed KCORE and persistent RAM unchanged. Prior v1808-v1915
  native and physical results support scoped implementation closure,
  not production enablement. The dated OPEN notes below are
  historical: persistent/ambiguous owner reconciliation belongs
  to K06, backing identity to K07, scheduling to K08, and broader
  physical allocator fragmentation/descriptor/PI6/root/platform
  acceptance to K09 and consolidated TODO.MD. No new physical
  acceptance is claimed for v1916. See
  `design/kcore-k05-closure-v1916.md`.
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
  - v1908 scoped original PDP-6 48K long-held foreign target PASS:
    one real failed PACKED source-free -5, foreign-claimed full
    target owner2, refused genuine parent TERM, then TEN real
    parent `HB` with **no target repair or release**. All MM18
    owners, RAW/PROCESS target, TRANSITION/VMbase0, executive
    CTX_PC/FDCTL, u[067] escrow `040000120000`, ROOT and parent
    WAIT gate remain unchanged. Original parent PCLK advances
    `015624` -> `016724` octal. Shared already-resident retry
    backoff advances `0100,0200,0200,0400,0400,0400,0400,
    1000,1000,1000` octal, reaching/staying at the `01000`
    (512-decimal) tick cap without starving the parent. Three
    targeted negative controls reject; simulator stops by
    **operator request**, no guest HALT/terminal claim. Fixed
    KCORE/PURE images and RAM unchanged. This tests a bounded
    interval, not indefinite safety. K05 remains OPEN for
    unresolvable terminal cleanup and PI6/MM ambiguity. See
    `design/kcore-k05-long-held-owner-v1908.md`.
  - v1909 scoped original 48K PDP-6 real PCLK PI6 PASS during
    unrepaired foreign-claimed FULL owner: exact KCORE handler
    `017225` with source-linked `AOS` opcode `350040024530`
    and saved interrupt word `340000017560` (original slot0
    idle PC `017560`). Verified actual MM18 whole-table,
    ROOT, child TRANSITION/VMbase0, unchanged executive CTX/
    FDCTL/escrow `040000120000` before/during/after PI6. Parent
    advances to HB10 with bounded shared retry `01000` octal,
    no forced owner repair/FINALIZE or native guest HALT.
    v1908 positive oracle and three new targeted PI6 negatives
    PASS on unchanged source-built KCORE/PURE. Source code
    confirms scheduler cannot PI6-preempt ordinary executive
    PURE FULL into a second slot0 reaper before escrow writes.
    This does NOT prove all possible PI6 points; unresolvable
    owner terminal policy remains K05 OPEN. See
    `design/kcore-k05-held-pi6-v1909.md`.
  - v1910 scoped original 48K PDP-6 parent EXIT PASS: after
    real failed FULL PACKED source free -5, target foreign-
    owned by slot2 and refused user TERM, parent actually
    invokes SYS_EXIT. Original `proc_exit_current` and slot0
    finalizer run; MM18 -> MM16 deletes only parent's uarea
    and full PROCESS extent. Child becomes orphan (parent1->0)
    while its exact RAW/foreign PROCESS descriptors, transition,
    VMbase0, executive CTX_PC/FDCTL and u[067] original target
    escrow remain untouched. Real post-EXIT PI6 preserves
    MM16. Three targeted unsafe physical mutants all reject.
    Simulator **operator** stops; this does not prove native
    terminal child cleanup. KCORE/PURE binaries unchanged.
    K05 still OPEN for unrecoverable child, other allocator
    ownership and PI6 cases. See
    `design/kcore-k05-parent-exit-v1910.md`.
  - v1911 scoped original PDP-6 48K last-user PASS:
    after original parent EXIT and permanent foreign FULL,
    ordinary children slots4,5,6 enter real native SYS_EXIT,
    with individual completed idle-loop returns between exits.
    Exactly six child-owned MM extents release: MM16 -> MM10,
    with only child3 left live, parent0, TRANSITION/VMbase0,
    original RAW source, foreign PROCESS target, saved executive
    CTX_PC/FDCTL, u[067] target escrow and ROOT unchanged.
    proc_high_slot trims to 4, current slot0. Three real
    slot0 native recovery calls occur `01000` octal clock
    ticks apart at the original `01000`-tick cap and preserve
    all MM10 extents and context. One actual idle-loop PI6
    has saved PC `017617`; three targeted unsafe mutants
    all reject. No native guest HALT, only operator exit.
    First experimental repeat-breakpoint false 3-EXIT trace
    rejected, fixed with source-linked idle break between
    real child exit events. No production RAM/image changes.
    LAST-USER IDLE gate is closed for this observed physical
    ordering; permanent foreign ownership resolution, other
    allocator/PI6 cases, indefinite safety and broad K05
    acceptance remain OPEN. See
    `design/kcore-k05-last-orphan-v1911.md`.
  - v1912 scoped original 48K PDP-6 terminal recovery PASS:
    the complete v1911 original last-child orphan/three
    capped retries/PI6 is replayed and source-linked oracle
    re-PASSes. The *test-created* foreign owner2 claim is
    corrected only afterward on the active, compacted exact
    target metadata `024707`, NOT old injection index
    `024725` (stale after original sibling EXIT). Native
    target mm_free at `033420/033421` genuinely returns 0,
    MM10 -> MM9, then original transition-clear `033456`
    releases u[067] escrow and private executive resumes.
    Second FULL genuinely frees PACKED source and commits
    RESIDENT at VMbase `130000`; slot3 user emits `w`,
    executes actual EXIT, and original PDP-6 executes a
    **normal native HALT PC=000000**. Final MM6, no slot3
    PROCESS/PACKED owner or leaked uarea; ROOT unchanged.
    FOUR independent unsafe physical mutants reject. First
    v1912 attempt wrongly reused pre-compaction MM index:
    invalid trace discarded, corrected after exact physical
    owner-table analysis; no kernel bug or code edit.
    Production KCORE/PURE, RAM/IMAGE/BSS unchanged.
    LAST-USER HALT AFTER **SAFE RECONCILIATION** is now
    physically tested. Permanently irrecoverable foreign
    owner policy, further allocator/PI6 timing and wider
    platform acceptance remain K05 OPEN. See
    `design/kcore-k05-terminal-reconcile-v1912.md`.
  - v1913 scoped permanent foreign last-orphan physical PASS:
    unchanged original v1910 guest and v1911 prefix PASS;
    one unrepaired MM10 foreign PROCESS target with slot3
    sole live, orphan/TRANSITION/VMbase0. TWELVE additional
    real slot0 retry entries and completed original idle-loop
    returns, `01000` (512 decimal) PCLK ticks apart, with
    original shared pending bit and backoff cap retained.
    Two more actual PCLK PI6 interrupt original idle PC
    `017553` and `017564`. Exact MM10 extents and owners,
    original RAW source, foreign target, ROOT, CTX_PC/FDCTL,
    uarea067 escrow remain invariant. No new MM deposits,
    owner repair, forced cleanup or native guest HALT. Four
    independent owner/transition/escrow/clock negative mutants
    reject; 15 capped native orphan retries across v1911/v1913
    are physically observed, not indefinite safety.
    Last-runnable is NOT last-live: original terminal policy
    remains fail-closed until authoritative MM reconciliation.
    K05 still OPEN for genuinely indefinite/error/wrap cases,
    additional MM/PI6/allocator failures and platform proof.
    No production RAM or fixed binary growth. See
    `design/kcore-k05-permanent-orphan-v1913.md`.
  - v1914 scoped original 48K PDP-6 PCLK clock-wrap PASS:
    byte-identical original KCORE/PURE and source-built guest
    replay whole v1913 physical last-orphan prefix and PASS
    v1913/v1911 independent positive oracles. TEST-ONLY
    synchronized leap of existing clock (`024530`) and
    pending retry epoch (`024532`) to `0377770` low-17-bit
    time preserves cap `01000`; timer_next remains 0.
    SEVEN genuine additional PCLK PI6 interrupted idle
    executions physically cross raw full clock `0377777`
    -> `0400000`, masked 17-bit becomes 0. Native slot0
    reaper follows at `0400770`,`0401770`,`0402770` octal,
    512 ticks each, with masked retry epoch `000770` etc.
    Complete MM10/ROOT, authoritative RAW, foreign target,
    orphan TRANSITION/VMbase0, private CTX/escrow unchanged.
    Four independent clock/epoch/owner physical mutants
    reject. No synthetic free/FINALIZE/HALT or production
    source, resident RAM or image growth. Distinguish
    controlled near-wrap leap from natural 36-minute uptime;
    K05 still OPEN for other timing/MM/PI6 cases and wider
    release. See `design/kcore-k05-clock-wrap-v1914.md`.
  - v1915 scoped original 48K PDP-6 capacity OOM physical
    PASS: newly source-built test-only USER APR reuses
    existing gate and invokes real `BRK(0130000)` AFTER
    foreign target owner2 and rejected TERM(3). Original
    `mm_arena_fits_v1260=002154` receives exact AC1/AC2,
    rejects physically impossible aligned run (single MM
    arena `044633..0140000`, maximum aligned `072000`),
    and real requester-only fatal OOM `013713` enters AC1=1.
    Original parent slot1 EXIT releases ONLY its two own
    extents MM18 -> MM16; distinct children 4,5,6 native
    EXIT release exact six ordinary extents MM16 -> MM10.
    Last-live foreign child3 remains TRANSITION/VMbase0,
    original RAW, foreign PROCESS target, CTX/FDCTL,
    u[067] escrow/ROOT unchanged during three genuine
    capped slot0 retries and a real idle PI6, no HALT.
    Four independent unsafe native BRK/owner/context/EXIT
    physical mutants reject. v1915/v2-v3 snapshot bugs
    discarded: read complete physical `proc_table` at
    `045700`, not its KCORE pointer `024635`; v1915/v4
    explicitly verifies all formerly live nonorphan user
    slots FREE, retaining the last orphan. Source-built USER and TSFS
    differ; fixed production KCORE/PURE, RAM, image and
    defaults unchanged. This is EARLY IMPOSSIBLE BRK
    capacity refusal only, NOT a proof of fragmentation,
    descriptor exhaustion or all allocator failures.
    K05 remains OPEN for remaining allocator/MM/PI6 and
    broad platform tests. See
    `design/kcore-k05-orphan-oom-v1915.md`.
- [x] **K06 - MM ownership uncertainty.** Preserve exact PROCESS/PACKED
  ownership, pins, TRANSITION and READY_ONCE state across persistent
  pin/free/rebase errors, restore cancellation and terminal cleanup.
  Never publish an ambiguous APR or release the sole retained suffix.
  **v1917:** FULL escrows the exact original RAW source before pinning
  and the original temporary APR before loading. The existing u-area
  word 067 holds both identities; no new per-owner RAM. Rollback,
  ordinary retry unlock and deferred recovery share one bounded exact
  owner/overlap/pin proof. Base-only unpins cannot touch a foreign
  claim; a lost target-unpin reply cannot consume a second pin. Source-
  only recovery cannot release an unknown scratch extent. The native
  cancellation, recovery, permanent refusal and unsafe mutation gates
  pass with the repaired KCC.
  **v1918:** DROP/PREFIX save the original RAW suffix identity in
  existing u-area 067 before mutation and share a strict sole-owner
  geometry scan with ordinary retry unlock. Native validation passes
  six transaction faults and seven scanner cases (including preceding
  overlap and independent owner namespaces). Two unsafe source mutants
  fail at 0101 and 0403; v1612 regression and READY_ONCE negative pass.
  Allocation, pin/unpin, loader and PI are fixtures; MM rebase, claim,
  release and commit use real assembly. No new permanent RAM.
  **v1919:** existing word 067 escrow distinguishes DROP/PREFIX from
  FULL and shares the original slot-0 retry armer. Held DROP can
  reconcile its original resident or RAW owner; held full-span PREFIX
  rolls back to RAW with a fresh checked pin, not an assumed old pin.
  Three DROP, four PREFIX and six invalid-marker cases pass, with five
  unsafe mutants rejected; prior v1918/v1612/v1902/v1906 gates pass.
  Folded lowmem KCC PURE MRES 3369 -> 3556 words, BSS zero; fixed
  KCORE low end 025133 unchanged. No persistent RAM added. Existing
  pins must be lawfully retired before deferred rollback; no forced
  release.
  **v1920:** terminal cleanup accepts only reconciled sole RAW, never
  speculative FULL scratch. Exact original sector identity survives
  pinned refusal, false/lost free replies and PREVM retries in word
  067 (bit31/source field). Replacements/foreign intersections and
  malformed MM geometry fail closed. Native 23 cases, five leaf mutants
  and actual MM/FINALIZE/PREVM integrated escrow-loss gate pass; prior
  v1609/v1666/v1680/v1681/v1919 pass. Historical v1900 direct dual-owner
  acceptance is superseded. Folded PURE +29 words; fixed KCORE +3,
  BSS and per-owner storage unchanged.
  **v1921:** sole-owner discovery and FULL proof reject malformed
  sorted-table geometry, including unrelated overlap. FULL reuses
  single-pass discovery and verifies source/target pins before use,
  each intervening unpin, rollback deletion and all commit replies.
  Six FULL faults, five geometry cases and seven unsafe mutations
  pass; prior native rollback/commit/recovery gates remain green.
  Folded PURE 3585 -> 3565 words (-20), fixed KCORE byte-identical.
  **v1922:** shared optional ASM proof validates installed core, all
  managed arenas and sorted nonoverlapping live extent containment.
  It bounds arena count, extent count and capacity; caches the current
  containing arena end, handles empty tables, and rejects malformed
  geometry before PURE compaction, claim, rollback or terminal free.
  Valid RAW with an unavailable prefix remains fragmentation. Exact
  absence on retry still requires managed containment. Native 18 arena,
  12 table, seven mutation-refusal cases, five unsafe ASM mutants,
  two reclaim cases and four extra terminal cases pass. Prior native
  ownership and terminal gates remain green. Folded PURE 3565 -> 3649
  words (+84), fixed KCORE byte-identical, no extra persistent RAM.
  K06 implementation CLOSED; real PI6 and production acceptance
  remain K08/K09. General allocator serialization and valid-table
  contract unchanged; no arbitrary memory corruption claim.
- [x] **K07 - Executable backing lifetime (CLOSED v1947).** Guarantee immutable PURE
  source lifetime across reclaim, source loss/replacement, RO/provider
  failure and unmount attempts. Provide bounded FULL fallback and safe
  owner retention when the source cannot be trusted.
  **v1923:** shared unpacker rejects absent backing/u-area/slab and
  wrong public mount root ID. PREFIX/FULL require PURE flag and
  compare the exact original root, canonical vnode and text before
  payload and after successful I/O. Ten restore faults, five decoder
  checks and five unsafe native mutants pass; earlier loader,
  staged owner, pinned mount guard and FULL rollback gates pass.
  Preserve proven typed source-free refusal (v1921 correction).
  Folded PURE +24 words; fixed KCORE +8; BSS/per-owner data unchanged.
  K07 remains OPEN for durable backing identity, mutation/owner
  admission races, provider/RO handling and unmount verification.
  Root snapshots cannot identify reuse before admission or an
  identical handle ABA replacement; no physical acceptance claimed.
  **v1924:** original provider stored in word 066 bits 28..30 and
  SIDL RH bits 2..4, with no new words. RUN/EXEC/detach preserve
  record/identity pairs; RAW admission and unpacker reject provider
  substitution. Nine native substitution, 16 RUN-transfer cases,
  three unsafe controls, extended v1563 attach and v200 EXEC gates
  pass. Folded PURE +21, fixed KCORE +22, BSS unchanged. Same-provider
  media reuse and mutation/admission/unmount races remain OPEN;
  physical concurrency not claimed.
  **v1925:** stamped DTFS PREFIX/FULL uses a serialized private
  provider operation before loading and after payload, forcing the
  existing physical RBN0 identity validator and invalidating both
  clean cache tags and directory identity. Reject poison/quiescence,
  incomplete identity, changed ID or inconsistent state; preserve RAW
  on refusal. Native 19 media, ten bridge cases and four unsafe controls
  pass. Legacy unstamped operator-trust policy unchanged. Folded PURE
  +13, DTFS +31, fixed KCORE byte-identical; no new data. Not a hardware
  reel lock: same-ID clone, mid-I/O swap-and-return or replaced mounted
  metadata are not covered. D6FS/TSFS identity, known-unavailable
  backing admission before DROP, mutation/admission and integrated
  unmount remain OPEN. No physical concurrency acceptance claimed.
  **v1926:** non-sleeping DROP admission rechecks the original public
  mount/provider under PI, validates the service JRST and refuses known
  DTFS poison/suspect/quiescence or inconsistent V2 identity/armed RO,
  missing D6FS reader or missing TSFS metadata. Refusal preserves the
  resident image and terminal cleanup is independent. Native 108 cases
  and five unsafe controls pass, earlier admission/recovery gates pass.
  Folded PURE +158 words, fixed KCORE byte-identical, no new data.
  D6FS/TSFS physical identity remains OPEN: D6FS runtime discards UUID,
  TSFS runtime retains locators/member map, not set ID/generation.
  Audit mount-owned identity storage and actual physical verification
  next; mutation/admission and integrated unmount remain open.
  **v1927:** D6FS retains UUID in disjoint existing shape/extent
  words and proof failure in media word. Root KINIT publishes UUID;
  secondary mount captures it from the physical selected superblock
  before publication, without changing handoff/reader ABI. Serialized
  proof verifies selected header/UUID/sequence/geometry/state before
  and after reconstruction; failed proof blocks DROP. mkd6fs assigns
  fresh random 72-bit IDs. Native 12 faults/retries, lifecycle checks,
  four unsafe controls, host equal-layout formatter and extended
  bridge/DROP gates pass. Folded D6FS +136, PURE +13, fixed KCORE
  byte-identical, no new data. Selected-copy proof cannot detect
  same-ID clones, legacy deterministic collisions or a pre-publication
  scanner swap. TSFS identity, ordinary provider-loss policy,
  mutation/admission and integrated unmount remain OPEN. No physical
  guest exchange/PI6 acceptance claimed.
  **v1928:** TSFS retains the admitted set ID, generation and TDIR
  locator/checksum once in the optional tape MRES. Fresh physical
  redundant descriptors of every member must agree with the original
  identity before and after reconstruction. Proof failure sets the
  existing map's suspect bit and blocks new DROP. Exact retry clears
  it. Native 40 descriptor/map cases, seven unsafe controls, boot
  handoff and extended bridge/DROP gates pass. Folded tape MRES +149
  words (five BSS), PURE +1; fixed KCORE byte-identical. No added
  per-process words. Not a full content hash or physical reel lock;
  no whole-guest exchange acceptance. Ordinary provider-loss policy,
  mutation/admission and integrated unmount remain OPEN.
  **v1929:** ordinary D6FS failures mark the exact published reader's
  mount READ_SUSPECT or WRITE_UNCERTAIN using the existing media word.
  Invalidate workspace and failed-source private cache tag only.
  Preserve v1538 root fail-stop and secondary error semantics.
  Matching UUID clears only read suspicion; write uncertainty refuses
  PURE proof, and both flags block DROP. Native six added fault cases,
  ten total proof mutants, 118 DROP cases and six DROP mutants pass;
  v1513/v1538/v1544/v1546/v1602 regressions pass. D6FS MRES +34 words,
  PURE and fixed KCORE unchanged, no new data. General write recovery
  is not implemented here; ordinary success is not a physical identity
  proof. Secondary all-resident read retry, mutation/admission and
  unmount lifetime remain open. No physical guest acceptance claimed.
  **v1930:** ordinary D6FS dispatch checks mount media state under the
  provider lease. READ_SUSPECT reuses physical UUID proof and preserves
  six request ACs; original media restores ordinary access and DROP
  eligibility, foreign media refuses. Healthy path performs no extra
  I/O. WRITE_UNCERTAIN/unknown state blocks ordinary operations, SYNC,
  REMOUNT and PREPARE; privileged raw inspection remains available.
  Thirteen native unsafe controls rejected; mount/remount/reader-slab
  gates pass. D6FS MRES +24 words, PURE and fixed KCORE unchanged,
  no new data. Automatic secondary read retry implemented. D6FS
  PREPARE currently frees the reader before VFS's final owner check;
  integrate quiescence, owner recheck and retirement next. Uncertain
  write administrative retirement and executable mutation/admission
  remain OPEN; general transactional repair is not claimed. No physical
  guest or integrated unmount acceptance.
  **v1931:** D6FS normal unmount acquires existing mount quiescence
  before sleeping, blocks new FILE/cwd/PURE owners and ordinary
  provider requests, performs fresh selected-header UUID proof and
  optional CLEAN, then rechecks all owners before deleting the reader.
  VFS retires the root before clearing quiescence. Failed free/owner
  check retains reader and identity; successful CLEAN with late refusal
  leaves the live RO mount; failed CLEAN retains sticky uncertainty.
  REMOUNT/PREPARE publish opposite A/B from verified selected header,
  with private state write separated from ordinary RO file policy.
  Seventeen native unsafe variants and eight helper variants rejected;
  earlier unmount, DTFS seal and native/host attach gates pass. D6FS
  MRES +14, fixed KCORE +37, PURE and BSS unchanged. No new data.
  Normal D6FS reader-retirement ordering fixed; poisoned administrative
  retirement, executable mutation/admission and remaining lifetime
  audit still OPEN. No full-guest PI6 or physical exchange acceptance.
  **v1932:** WRITE/TRUNCATE repeat RO and PURE owner checks after
  acquiring the physical provider lease; refusal uses common unlock.
  D6FS/DTFS UNLINK check the actual resolved child. Native 16 delayed
  mutation and two D6FS child cases pass; four unsafe controls rejected.
  Earlier DTFS unlink, provider lease and rename gateway gates pass.
  Fixed KCORE +13, D6FS MRES +6, tape MRES +3, PURE unchanged; no data
  growth. Still OPEN: whole-load RUN/EXEC reservation, prevention of
  owner admission during already dispatched mutations, direct MEMFS
  namespace admission audit, poisoned administrative retirement and
  integrated physical lifetime acceptance. This is a partial K07 fix.
  **v1933:** reserve the executable vnode before STAT/header/payload
  through a stack-owned two-word record and a single global list head.
  Private gateway operation 032 drains the physical provider lease
  before publication, checks mount identity/quiescence, and the file/
  mount scanners see in-flight loads. Exact non-LIFO release on all
  loader exits. Eight plain DXR1 loader cases, lease/list and mutation
  gates pass; six unsafe controls rejected. Fixed KCORE low end +91
  words, BSS +1, two stack words per active load; MRES sizes unchanged.
  Whole EXEC old/new backing handover, lookup and fatal/private-stack
  lifetime, direct MEMFS mutation audit, compressed/raw-XIP integration,
  poisoned retirement and physical acceptance remain OPEN.
  **v1934:** normal staged EXEC retains old and new PURE backing
  dependencies in the existing transient reservation list until commit
  or rollback. Decode existing published identities without a lease
  wait; zero PURE text needs no hold. NOMEM drops the outer hold before
  destructive retry with the old permanent owner intact. Nine native
  transaction cases pass, three unsafe controls rejected; earlier
  v20/v200/v1719/v1720/v1933 gates pass. Fixed KCORE +29 words, four
  transient stack words, no BSS/process-table growth; PURE unchanged.
  Destructive low-memory EXEC direct publication of stable u-area and
  credentials requires separate audit, as do namespace and fatal
  lifetime, MEMFS mutation, poisoned retirement and integrated PI6.
  **v1935:** destructive EXEC preserves live META/SCHED during actual
  loader attachment and rollback. Shared staged/live publication leaf;
  AC2 entry result; existing mapping hold spans new load/startup.
  Seven native DXR2 cases with real compiled attachment pass, four
  unsafe variants rejected. v1720/v1933/v1934/v1563/v200/v1719 pass.
  Fixed KCORE -2 words, lowmem frame -1 word, BSS/PURE unchanged.
  Lookup-to-reservation, fatal/private-stack lifetime, MEMFS mutation,
  poisoned retirement and integrated physical validation remain OPEN.
  Compressed/XIP execution not covered by the new fixture.
  **v1936:** executable lookup saves a namespace revision before walking
  and checks it after reservation admission, before STAT/image reads.
  Potentially mutating gateway calls advance the revision before and
  after dispatch, even on failure; actual mount publication/retirement
  advance it too. Saturate rather than wrap. 50 provider/opcode cases,
  lookup and lease-wait changes, mount changes and saturation pass;
  five unsafe variants rejected. Fixed KCORE +28 words incl. BSS +1;
  PURE and loader frame unchanged. Conservative unrelated mutation
  refusal; no automatic retry. Controlled native lookup boundaries,
  not full recursive namespace or physical PI6 acceptance. Fatal stack
  lifetime, direct MEMFS mutation, poisoned retirement and integrated
  validation remain OPEN.
  **v1937:** scan published reservation addresses before normal/marked
  EXIT finalization, RUN discard and final u-area release. Return typed
  BUSY at MM boundary, defer foreign fatal events to owner and fail
  stop before an impossible self-EXIT stack abandonment. No new data
  or pin. Eight native cases and six unsafe controls, actual MM v956,
  v1097, v869, v1086 and v1933-v1936 pass. Fixed KCORE +44 words;
  BSS/PURE unchanged. Old v925 fixture stale, other historical runners
  not all rerun. Direct MEMFS mutation, poisoned administrative
  retirement and integrated fatal/physical PI6 lifetime remain OPEN.
  **v1938:** MEMFS WRITE/TRUNCATE and resolved-child UNLINK check
  backing owners using the singleton's published canonical mount.
  PREPARE checks mount owners before data destruction. Existing XIP
  pins retained. Eight native owner configurations (mount 2/18),
  five rejected unsafe controls; earlier allocator/fatal XIP/FIFO
  gates pass. MEMFS MRES +38 words (1854 -> 1892), fixed KCORE
  byte-identical, PURE/BSS unchanged. No new data. Controlled MM/I/O
  boundaries; complete-guest execution remains unverified. Poisoned
  administrative retirement and integrated lifetime validation OPEN.
  **v1939:** root-only existing recovery syscall accepts poisoned
  noncritical D6FS. FD abandonment checks actual provider. Discard
  owns admission barrier before lease wait; private op 033 validates
  root/poison/reader and uses final owner scan/cache invalidation/exact
  MM release without physical I/O or CLEAN. Refusal retains poison and
  reader; success retires root before clearing barrier. Seven new and
  17 previous unsafe native variants rejected; v1531/v1562/v1575
  and earlier D6FS selector/remount/slab gates pass. Fixed KCORE +46,
  D6FS MRES +21, BSS/PURE/tape unchanged. Controlled boundaries;
  K07 remains OPEN for integrated physical/lifetime acceptance.
  **v1940:** all 13 distinct current native regression groups pass.
  Testkit consolidates native and physical runners. Missing installed
  native KCC bootstrap restored with existing install-native target;
  current GCC 48K TSFS full image builds and boots. Three checked
  children (one plain, two compressed) start before 180s host timeout
  in DTC search. Physical lifecycle, FULL geometry, live PI6 and
  source loss acceptance remain OPEN. Longer physical runner added
  but not yet executed; historical replay offsets require updating.
  No kernel code or permanent RAM growth.
  **v1941:** unchanged physical 48K guest admits five children but
  sixth RUN refuses. Interrupted post-failure slot0 MM snapshot has
  four unpinned PACKED owners and one pinned expanded PACKED owner;
  zero reservation head, no slot7 VM/u-area. Parent already exited:
  not pre-RUN NOMEM proof. Current-map FULL replay derives actual
  installed base from service/reclaim jumps. Diagnostic audit and
  seven synthetic verifier negatives pass; incomplete trace rejected.
  Five-child fixture rebuilt with identical kernel images, retains
  sixth refusal and exact WAIT checks, TSFS verified; full replay
  pending. No kernel growth. K07 remains OPEN.
  **v1942:** actual adapted five-child 48K TSFS guest passes
  sixth RUN refusal, parent survival, no-growth BRK and all typed
  WAITs. B/A MM18, five 07000 PACKED extents and empty reservations.
  Original FULL PC040756: slot3 source111000..120000 and separate
  target120000..130000 each pin1, slot0, stable u-area051274.
  Z MM8, no child VM/u-area or reservation leak; normal HALT.
  Nine accepted-transcript mutations rejected; seven synthetic
  geometry negatives pass. Production kernel unchanged. K07 still
  OPEN for live PI6 retirement/reservation race and source loss.
  **v1943:** source-matched disposable PI6 race fixture built for
  both reservation-before-unmount and quiescence-before-RUN. First
  physical run interrupted during DTFS setup, before race marker:
  actual DIRTY media and eight changed data records, slot1 held
  lease, no reservations. Not a demonstrated deadlock or race PASS.
  Revised fixture with seed-stage markers and diagnostic table capture
  built and media verified; execute to bounded completion next.
  Production code unchanged. K07 remains OPEN.
  **v1944:** actual guest seed complete, exact CLEAN reel and
  999 compressed words verified. Corrected monitor flush parsing
  and reuse genuine seeded reel; same disposable kernel/child.
  Real PI6 slot2 UNMOUNT refuses while slot1 sleeps with transient
  stack reservation, no permanent secondary owner and free lease.
  RUN/WAIT succeeds. Reverse live V2 barrier refuses RUN with
  unchanged backing/MM; retirement, same-medium remount, RUN/WAIT
  and final retirement pass. MM8, no child/reservation leak, one
  normal HALT. Final reel byte-identical. Ten altered trace and
  six synthetic media controls rejected. Two test-only widened
  scheduler windows; no production growth. K07 OPEN for integrated
  source loss/recovery and final review.
  **v1945:** current uninstrumented production images pass genuine
  48K physical secondary source loss/replacement/return. At actual
  FULL slot3 source113600..120000 and target122000..130000 pin1,
  both physical removal and distinct-ID CLEAN foreign reel return
  -4; original PACKED pin0, exact 2176 words and backing/volume ID
  preserved, target gone, escrow0, READ_SUSPECT. Same original reel
  restores, all five exact WAITs, final UNMOUNT and MM8, normal HALT;
  both media byte-identical. Independent verifier and 12 altered
  actual transcript controls pass. No production growth. Final review
  retains v1940's 13 native PASS groups (same production source),
  v1942 physical FULL and v1944 admission races. K07 CLOSED in its
  implementation scope. Actual D6FS/TSFS exchange, wider PI6/production
  matrices and KCC remain K08/K09; no same-ID clone or hidden exchange
  claim. RAW PURE/mode3 remain default OFF.
  **v1946 correction:** v1945 foreign stop repeated the stopped PC
  without execution; same-reel recovery from missing remains valid.
  Actual v1946 foreign PREFIX -4 independently verified 299 ticks
  after missing FULL -4, with exact original suffix and identity.
  **v1947:** genuine later foreign PREFIX refusal 302 ticks after
  missing FULL -4, followed by original reel recovery/READY_ONCE,
  five exact WAITs, healthy retired medium, MM8, no child owners,
  normal HALT and both reels unchanged in one run. K07 CLOSED;
  v1945 repeated-stop witness remains withdrawn.
- [x] **K08 - Scheduler integration (CLOSED v1947).** Complete real PI6 interruption,
  wake/retry, READY_ONCE, WAIT and EXIT behavior during multi-process
  memory pressure. Respect slot-0 FULL serialization and guarantee
  progress without fabricated concurrent user dispatch.
  **v1946:** current-source native v1650/v1808/v1677 PASS. Actual
  pinned FULL PI6 in TAPE, unchanged MM18/2176 words, real loss
  rollback, postfailure PI6, slot4 execution with DTC3 absent and
  later foreign PREFIX refusal independently verified. Diagnostic
  deadline expired during reattachment recovery; slot3 READY_ONCE
  dispatch and final WAIT/EXIT/conservation pending. No kernel growth.
  **v1947:** full 1150-second-deadline replay completed in 284.728s.
  Actual slot0 FULL PI6 saved TAPE PC032060 with both pins; physical
  timeout, source retention, peer slot4 execution, distinct foreign
  refusal, original reel return, slot5 then slot3 READY_ONCE, actual
  first dispatch AC1=3/cursor3, unchanged MM13, five exact EXIT/WAITs
  and final MM8/high slot2/mode0, healthy unmount, HALT. Independent
  full and supplementary oracles plus 14 altered traces PASS. No
  production growth. K08 CLOSED in scoped implementation acceptance;
  K09 wider physical/root/controller and KCC acceptance separate.
- [ ] **K09 - Production integration acceptance.** The v1809 Git
  fast-forward has incorporated the PDP-6 RAW PURE source history
  into main with the feature OFF by default. Do not confuse source
  ancestry with accepted enablement. Complete K05-K08 correctness
  dependencies and current-source physical/root matrices before
  promoting a production PURE configuration. Repository-wide branch
  retirement, hardware coverage and release acceptance are separate.
  v1948: KCC FULL OFF/RAW builds and disk boot PASS. Fixed
  insufficient disposable KINIT prefix (1195 transient words,
  zero permanent growth). Corrected FULL AUTO disk/drum,
  TSFS noaux/PURE and TSFS DRM AUXSTORE/SWAP selections PASS.
  Explicit TSFS with full-size DSK AUXSTORE PASS, AUTO FAIL
  (PURE selected instead of SWAP). v1950: fixed in SIMH cb6a255
  after DEC H-270 review; six-case root matrix PASS. Withdrawn
  v1949 CLEAR-to-IDS assumption; no kernel workaround or growth. KCC 64K GENERAL/STRICT build and bounded
  physical noaux boot PASS. Remaining
  physical pressure/lifecycle and controller matrix gates OPEN.
  v1951: actual current KCC 48K GENERAL/STRICT mixed FULL
  five-child lifecycle and MM conservation PASS on v1950 SIMH.
  Matched 64K OFF/RAW physical budget PASS in measurement;
  16K fixed ceiling FAIL 17046/20925 incl MM table before
  scratch, installed RAW delta3879. Nine-owner v1674 geometry
  no longer fits current KCC baseline; recalibrated two-grown
  fixture build/A reached, full run pending. Fresh 64K active
  mixed builds ready, unexecuted. No production growth.
  v1952 user decision: defer size optimization and 16K resident
  target; neither blocks K09 functional closure. Preserve actual
  RAM accounting and the measured budget miss. Nine-owner/64K
  pressure, descriptor exhaustion, transient ownership and
  controller loss/recovery remain required. Default flags unchanged.
  v1953: 64K GENERAL/STRICT mixed nine-child FULL native
  lifecycle and MM conservation PASS. Native MM classification
  and organic 48K GENERAL descriptor exhaustion/free/retry/WAIT
  PASS. Type-270 SNA/ADT selected-media-loss and actual KINIT
  fail-closed correction PASS on SIMH 12dcd35. Later READ loss
  stalls real KINIT beyond 301 ticks; DFR/busy loss semantics
  and bounded boot recovery remain OPEN. No kernel change.
  48K GENERAL/STRICT nine-owner physical pressure B PASS;
  both intentionally stopped after pipe release, final WAIT/C
  not accepted. Remaining lifecycle and later-loss tests OPEN.
  v1954: issued READ loss fixed in SIMH cec5436; real KINIT
  PI5 error and fail-closed boot PASS. Independent silent
  controller timeout in DAIMOS b98f64d7 PASS at 300 clock
  ticks; wait for legal IDS/ADT before new DATAO. Both paths
  clear storage ownership/countdown. Six FULL root cases
  and elevator PASS. KCORE unchanged, DSK MRES +23 words,
  no BSS growth. Runtime/WRITE and active DISK SWAP recovery
  not accepted; 48K final lifecycle still outstanding.
  v1955: native 64K KCC active DISK swap/restore/WAIT PASS.
  Six 8192-word children; real slot2 64-sector run matches
  bitmap and counters; both canaries, all six WAITs, exact
  final owners, pipe and bitmap cleanup. MM9/20/8.
  Runtime provider loss and 48K final lifecycle remain open.
  v1956: actual critical BACKSTORE READ and WRITE loss
  both reach dedicated reason2 HALT, original owner/bitmap
  and entire MM table conserved, failed request cleared.
  Runtime fail-stop accepted for issued commands; no general
  interrupted WRITE recovery or exhaustive queued/mid-sector
  coverage claim. Both fresh 48K write-watch runs armed all
  71 watches, intentionally stopped during second RUN.
  Complete 48K lifecycle/high-water still outstanding.
  v1957: both current KCC 48K GENERAL/STRICT nine-owner
  native lifecycle and complete physical MM write-watch
  PASS. Valid compact split-member TSFS with unchanged
  guest/boot bytes; no timing or production changes.
  MM9/26/9, free24224/2649/24224, exact baseline MM
  restoration, nine WAIT/data checks, unchanged tapes.
  2931 writes, 170 count commits, 18 PURE and 2 BRK resizes;
  peak47940/49152, MM28/32; six negatives rejected/profile.
  Original parent pipe remains open, harness stops after PASS.
  Remaining controller matrix/final acceptance review OPEN.


## Mandatory KCORE: KA10 nonpaged backend

- [ ] **K17 - KA10 contiguous relocation/protection backend.** Implement
  an actual KA10 target, not a PDP-6 build relabeled as KA10. Initial
  target uses one low segment without a pager and without requiring
  optional KT10A high-segment hardware. Reuse common MM/process/VFS
  ownership policy rather than forking KCORE. Complete the following:
  - Target-specific boot image, APR initialization, PI/trap and
    executive/user context-switch handling.
  - Contiguous low-segment allocation/alignment and verified KA10
    relocation/protection programming; activate only a complete
    authoritative resident image. Separate machine-specific VM
    load/copy/activate/restore interfaces from common policy.
  - RUN/EXEC/BRK, fork where supported, EXIT/WAIT, raw swap and RAW
    PURE compatibility; preserve ownership and rollback semantics.
  - Actual native KA10 simulator boot, mapping/protection, interrupt,
    multi-process and memory-pressure tests, including 64K target
    configuration and measured resident/transient RAM.
  - Target-specific acceptance; PDP-6 results cannot substitute.
  No KI10 pager, speculative shared high text or unrelated new feature.
  Mandatory after K09, before optional K10-K16. Existing architecture
  boundary: design/kcore-multi-cpu-portability-v1605.md.

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
3. Finish K17 mandatory KA10 nonpaged backend and acceptance.
4. Implement K10-K16 only as the optional compressed-memory feature;
   mode3 stays disabled until safe and accepted.

Each item is a feature-completion task, not a claim of a newly reproduced
bug. Full physical test matrices, RAM tuning, optimization, other hardware
backends, filesystem features and post-feature release work are tracked
separately in the consolidated `TODO.MD` and related documentation.
