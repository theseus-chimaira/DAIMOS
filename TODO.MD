# DAIMOS TODO - current open work (v1675)

Updated: 2026-11-19. This is the *active backlog*, not a chronological
acceptance log. Every checkbox below denotes unfinished work; there are no
completed checkboxes or superseded historical requirements in this file.

The complete former 8,685-line TODO, including its existing local edits, is
preserved in Git at commit `affe58a9` (`git show affe58a9:TODO.MD`). Completed
milestones and measured results belong in `PROJECT-STATE.MD`, `KCORE.MD`,
versioned `design/` reports and `pdp10-doc`, not here. Do not interpret old
open requirements as current blockers without comparing newer accepted gates.

## Scope and working rules

- Current production `main` is not the PURE development branch. The accepted
  PDP-6 RAW PURE 48K physical high-water v1674 belongs to the **unmerged**
  `kcore-pure-reclaim-pressure-v1673-20261118` branch, commit `ae03d40f`.
- KCORE P0-B trusted-operator DTVOL2 (v1575), P1-C storage/swap/BCACHE
  (v1603), and P1-D controller/FSINFO matrix (v1601) are CLOSED under their
  documented contracts. Do not reopen obsolete mechanical reel-interlock,
  all-controller, or already-accepted fault matrices without new evidence.
- **48K words is the working full-boot minimum.** 32K *kernel-only* tests
  remain necessary. Full 32K boot, final size shaving, and MRES runtime
  unload/reload are not current release blockers.
- Correctness first, especially compiler/assembler regressions; minimize
  resident RAM, then runtime. No workaround for a reproduced underlying bug.
  All new tests go into `daimos-testkit` (compiler-specific regressions may
  also belong in `pdp10-testkit`). Maintain source/branch/build identities.

## A. Immediate correctness and release gates

### A1. Native compiler, assembler and build-system correctness

- [ ] Reproduce and fix any current native KPARSE/KCC identifier/constant-
  expression overflow and stalled parse graph from the actual installed and
  source-built compiler. Do not disable overflow checking to hide defects.
- [ ] Recheck the known quadratic `CCKIRWRITE.C` pointer lookup against
  current native KPARSE, then implement a bounded-RAM index only if current
  measurements justify it; measure graph-build time, peak RAM and stack.
- [ ] Verify unoptimized host/native KCC output parity (`-N`), nested-call
  stack-argument ABI, D-format/overflow flags, and the current source-built
  assembler/linker path. Fix any reproducible miscompile before proceeding.
- [ ] Complete native DAS relocatable DOBJ1 `-A -C` serialization, relocation
  ownership, `.set` aliases, LH/RH edge cases and native DLINK resolution;
  run C89 -> DAS -> DLINK -> RUN source-linked regressions.
- [ ] Run compiler/assembler/`p10fold` regressions against the current
  cross-object relocation-identity fix, including reproducing old invalid
  fold plans; avoid unsafe fold reuse for size alone.
- [ ] Complete a genuine native KCC `MAKE ALL` and controlled `INSTALL`
  from freshly source-built KCPP/KPARSE/KOPT/KGEN, then repeat a native
  self-rebuild and single-source invalidation. Measure peak KCC/MAKE RAM.
  Do not trigger an unbounded full native self-build without a bounded plan.
- [ ] Reconcile still-failing GCC/KCC disk-root testkit groups against
  current sources and physical media, keeping correct accepted tests intact.

### A2. KCORE P1-E: PDP-6 swapless PURE

- [ ] Add real physical PI6/scheduler fault-pressure during multi-owner RAW
  PURE reclaim and restore. Inject recoverable and uncertain MM/loader/
  backing failures at reachable stages; prove owner/pin/TRANSITION rollback,
  no premature READY_ONCE, continued bystander progress, and final reaping.
  Respect original slot-0 scheduler/PI6 serialization; do not fabricate an
  impossible parallel user UNMOUNT during a FULL callback.
- [ ] Regress the optional PURE path in matched physical ROOT=auto,
  disk-root, drum-root, DSK/DRM direct/mapped BACKSTORE, and KCC FULL
  profiles, including automatic TSFS-vs-disk service choice. Preserve
  existing disk swap and ordinary KCORE without conditional regressions.
- [ ] Extend production-relevant 48K/64K RAW PURE pressure coverage where
  failures or alternate providers require it. The v1674 48K committed-MM
  low-water (1,255 free words) and the stable v1672 64K acceptance are
  established *only* for their source-linked workloads, not all schedules.
- [ ] Finish the controlled review/merge of proven PDP-6 RAW PURE into
  `main`, retaining default-OFF until required regression gates pass;
  distinguish PDP-6-only support from complete cross-machine P1-E release.
- [ ] Define and validate KA10-specific VM/APR mapping, relocation,
  low/high-segment capability and restore/rollback ABI. Reuse only genuinely
  common immutable-backing and RAW-owner semantics; do not apply the PDP-6
  APR implementation to KA10 without hardware/target acceptance.

### A3. P1-E optional bitmap mode 3 (separate from RAW PURE)

- [ ] Integrate the bounded mode3 compressor into the *real* inactive-process
  scheduler admission path and physical MM storage accounting. No duplicate
  backing registry, unsupported compression format or resident RAM creep.
- [ ] Prove atomic compressed commit and refusal: exact buffer capacity,
  codec validity, pinned/duplicate owner guards, source immutability,
  fragmentation/low-memory fallback and recoverable uncertain failures.
- [ ] Run real compressed RUN/EXEC, EXIT/WAIT, reaping, multiple owners,
  restore from source, terminal release, scheduler PI6 retry, failed FULL
  fallback and source/media-loss recovery with authenticated physical MM.
- [ ] Verify measured net RAM savings against added PURE MRES code and
  transient compression/decompression peak. Keep mode3 production-OFF
  until complete; do not equate staged native codec tests with live support.

### A4. Final KCORE release acceptance

- [ ] Complete mandatory 32K **kernel-only** assembler/linker/MM/critical
  error-path tests. Full 32K DSH boot is deliberately deferred.
- [ ] Run a final matched-source native and physical PDP-6 release matrix
  after permitted feature integration: ROOT selection, media error/return,
  MM/VM/PIPE, EXEC/RUN/WAIT, process owner/PI, failure atomicity, full
  KCC/GCC profiles, 48K viability, 64K and larger supported configurations.
- [ ] Record exact KCORE/MRES/KINIT permanent and transient RAM, largest
  free extent, source/build identity, process/owner invariants and remaining
  target-specific exclusions before declaring feature freeze.

## B. Other system, filesystem and tooling work

### B1. KINIT, providers, storage and administration

- [ ] Audit D6FS BLOCKSET discovery on non-root and secondary DSK/DRM
  units, avoiding unsafe probes of absent hardware. Count known BLOCKSETs
  as one logical mount and unknown non-members as possible independent
  mounts; do not silently assume DRM0 is the only eligible AUXSTORE.
- [ ] Revalidate boot-selected VFS capacity and provider service bounds
  for newly introduced controller combinations. Retain safe published
  mount IDs, exact allocated slab capacity, and no phantom service slots;
  do not revive already accepted P1-D FSINFO tests without a regression.
- [ ] Finish the native MKSWAP provisioning utility, with non-destructive
  geometry/ownership checks, actual DSK/DRM/BLOCKSET media and error tests.
  Complete supported tape+drum, disk+drum and non-root service combinations.
- [ ] Implement a real online D6FS packer, distinct from offline `packfs`:
  respect open-file and VFS lifetimes, swap/LOGSTORE/BADMAP exclusions,
  crash-safe extent moves, and bounded per-file/per-extent exclusion.
- [ ] Verify D6FS CREATE/recovery failure atomicity and provider MM/FILE
  refusal paths under current source, rather than treating the repaired
  FCB scan-bound bug as an end-to-end crash-recovery proof.
- [ ] Add privileged software wall-clock SETTIME/DATE SET with an accepted
  clock source and persistence policy; do not require PCLK for setting time.
- [ ] Extend current KINIT stack watermark and error-path coverage to
  unusual roots, missing devices and fatal exits before considering a
  reduction of the existing 256-word stack reserve.
- [ ] Add host-tool round-trip/format tests for MKDT, MKSTREAM and WORDS2PT
  in `pdp10-testkit`, including malformed input and representative images.

### B2. Restricted MEMFS XIP and process ownership

- [ ] Close remaining privileged raw MEMFS XIP admission and user-lifetime
  cases: nonwriting image/startup contract, no user writes/stack/argv,
  exact source provenance, MM pins, owner refcounts and immutable backing.
- [ ] Validate real scheduled XIP RUN/EXEC/EXIT, shared repeated launches,
  EXEC rollback, final VM teardown/unpin, provider unmount/remount,
  swap/BRK refusal, post-admission u-area failure and low-RAM pressure.
  Reuse accepted 128K+ cases; do not infer 48K or arbitrary MM-fault proof.
- [ ] Complete persisted MEMFS/LOGSTORE refusal/retry and recovery tests
  that are not covered by accepted P1-C contracts, especially uncertain
  provider/MM ownership and crash-atomic storage updates.

### B3. Process events, OOM and error containment (separate audit)

- [ ] Reconcile historical P0-D/E/F OOM and fatal owner-teardown
  requirements with the current accepted KCORE contract before enabling
  broader automatic OOM victim termination. Source history includes
  stale and superseded requirements; prove a current gap first.
- [ ] If automatic OOM victim termination is enabled, prove bounded,
  allocation-free FILE/PIPE/TTY/DPY/VM/provider cleanup, exact-once
  owner retirement, PID reuse, group HUP/TERM/CHLD ordering, and healthy
  bystander progress under real low-MM and persistent refusal.
- [ ] Define a fail-closed policy for indefinitely nonreturning providers
  and permanently corrupt/missing MM/FILE owner records. Do not claim
  forced recovery, unbounded retry success, or reclaimed physical RAM
  while a pin/provider remains owned.

### B4. Native MAKE and broader self-hosting

- [ ] Finish native MAKE correctness for the source-staged KCC graph:
  dependency/recipe tables beyond 768/384 links, sorted-index RAM/timing,
  recursive invalidation and required recipe/variable syntax.
- [ ] Verify current suffix search precedence and recursive MAKEFLAGS
  contract against GNU/BSD Make only where actual native makefiles need
  the behavior. Preserve SIXBIT/uppercase, serial execution and the
  feature-frozen V0.9 scope; no unnecessary -j worker implementation.
- [ ] Rebuild a bounded ordinary userland component with native
  MAKE/DAS/DARC/DLINK/OBJDUMP, then incrementally self-host the required
  core userland. Record RAM/stack/latency rather than assuming host parity.
- [ ] Complete source-compatible native MAKEDEPEND only after mainline
  native build reliability; prefer the existing imported reference or a
  small accurate C89 include scanner with cycle and -I handling.

### B5. DSH and core utilities

- [ ] Complete DSH-specific tests and synchronization with the frozen
  language contract; remaining case/glob, scripts/functions, pipelines,
  TTY/job control, bounded interactive editor and SIXMD/help completion.
  Detailed active checklist: `userland/dsh/TODO.MD`.
- [ ] Complete the minimal standard command set needed for a functional
  shell, build and administration (ECHO, CAT, CP, CMP, GREP, HEAD, TAIL,
  LS, FIND, STAT, MKDIR, RM, MV, CHMOD, CHOWN, DD, XARGS, WC, etc.).
  Reuse working programs; do not rewrite already accepted commands.
- [ ] Port/finish MonitorFS-native PS, FREE, DEVSTAT, MEMSTAT and admin
  tools. Do not recreate DAIMOSV1 SYSCTL/DEVIO compatibility layers.
- [ ] After native build stabilization, integrate a small-footprint
  Elvis vi/ex with its essential editing functionality; defer cosmetic
  syntax/URL/color features and unmeasured overlays.

## C. Deliberately deferred or separately authorized work

- [ ] **Full 32K boot and final KCORE size sweep:** revisit only after
  functional feature freeze. Measure true resident/process/stack peaks,
  8K permanent image limits and CTY-only kernel configuration. Evaluate
  dynamic table/pointer costs and the 256-word KINIT stack reserve.
- [ ] **Post-freeze code folding/superoptimization:** evaluate safe
  p10fold/p10super use on non-hot JRST paths and natural KCORE/MRES
  dependencies. Require measured meaningful net RAM savings and never
  weaken relocation or removable-module correctness for small gains.
- [ ] **D6 codec single-format cutover:** existing production D6LZ36
  remains authoritative. If separately authorized, implement the new
  grouped decoder with identical resumable low-core ABI, bounded
  64-word refill and <=31-word groups; measure full source/target RAM,
  run error/stack tests and atomically replace all five Stage-1 loaders,
  KINIT, host/TSFS, VFS/EXEC/PURE and native producers. No mixed-format
  image fallback. Detailed plan:
  `system/kernel/core/D6CODEC-INTEGRATION-TODO-v054.md`.
- [ ] **Unified DECtape filesystem design:** revisit the proposed
  root/secondary TSFS/DTFS consolidation, write durability, access classes,
  mtime and build-time AUX/TMP combinations only after a new design decision.
  Do not replace accepted trusted-operator DTFS media policy implicitly.
- [ ] **Loadable/unloadable runtime MRES and userspace overlays:** defer
  runtime unload/reload, shared text and user overlays until measured
  non-overlay programs work and memory savings justify new machinery.
- [ ] **Libc assembler tuning:** profile qsort and compact byte-pointer
  string primitives; do not rewrite C code without measured RAM/runtime
  improvements and target native regressions.
- [ ] **AAP Type 340/344 interaction:** add the smallest useful
  light-pen/special-condition PI input interface, coordinates, stop/edge
  acknowledgement and resume, with AAP hardware-model tests; do not grow
  the ordinary display refresh hot path.
- [ ] **MIT MAC HACK vi and Type 340 chess display:** port only after a
  smaller native display interface is proved, keeping old ITS dependencies
  separate. Do not reinstate the rejected persistent display-list feeder.
- [ ] **Recovery media and other hardware:** bootable recovery DECtape,
  SLAVE reverse/PCLK interrupt behavior and hardware-pager backend for
  later PDP-10s are independent post-0.9 projects.

## Completion protocol

When a checkbox is fully accepted, **remove it from this file** and put
its tested state, versioned source/fixture identity, memory impact and
unaccepted exclusions in `PROJECT-STATE.MD`, the appropriate `design/`
report and, for completed kernel documentation, `pdp10-doc`.
Do not recreate the dated DONE/OPEN diary in this TODO. Before deleting
an older checkbox, verify whether later work actually closed it.
