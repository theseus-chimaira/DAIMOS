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
- [ ] **K02 - Process resource cleanup.** Complete failure-safe cleanup
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
- [ ] **K03 - OOM handling.** Define and implement an allocation-free,
  safe victim-selection and termination path only after provider, VM,
  descriptor, zombie, EXEC and unpublished-RUN teardown is safe.
  Automatic OOM killing stays disabled until that condition is proved.
- [ ] **K04 - Process teardown recovery.** Handle incomplete teardown,
  retained owners, zombies, finalizer/reaper retries and permanently
  refused resources without double release, lost ownership or unbounded
  retry loops.

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
