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

- [ ] **K01 - Permanent MM failure handling.** Correct ownership and
  recovery when memory cannot be freed, pinned, rebased, relocated or
  restored. Fail closed on ambiguous ownership; retain exact diagnostics
  and provide bounded retry or safe administrative recovery, never forced
  release of unproven extents.
  **In progress (v1678-v1693):** MM compaction and reclaim preserve hard
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
  Native PDP-6
  tests and KCC KCORE source builds pass;
  see `design/kcore-k01-mm-refusal-v1678.md` and
  `design/kcore-k01-swap-owner-v1682.md`. Still open: other permanent free,
  rebase, restore and other owner failure paths, bounded administrative
  recovery and complete physical guest acceptance.
- [ ] **K02 - Process resource cleanup.** Complete failure-safe cleanup
  of process-owned MM, VM, FD, PIPE/FIFO and related resources during
  EXIT, EXEC and failed or unpublished RUN. Do not claim provider-specific
  filesystem or device implementation work as part of this task.
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
