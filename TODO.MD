# RAW PURE Git histories consolidated on main v1809

The RAW PURE and testkit histories are now fast-forwarded to
main with no unmerged named topic heads. Native v1807,
physical 64K PURE boot and
independent 48K GENERAL/STRICT pressure A/B/C acceptance
PASS; the v1808 K05 lost-success restore correction and
combined native matrix also PASS. The v1808 change removes
one removable MRES instruction; no fixed KCORE growth.
K05/K06 implementation closed v1916/v1922; K07/K08 closed v1947
after complete physical PI6/foreign refusal/recovery/WAIT replay.
K09 production acceptance remains separate work.
Clean unattached superseded refs were safely retired in v1809;
only worktree-attached topic refs remain.
Preserve existing dirty worktrees and default-OFF feature
selection. See `design/daimos-branch-consolidation-v1809.md`,
`design/daimos-branch-integration-validation-v1807.md`
and `design/kcore-k05-restore-reply-v1808.md`.

# DAIMOS TODO - current open work (v1675r2)

Updated: 2026-11-22 (v1809 ancestry). This is the consolidated
open-only backlog for DAIMOS mainline, including default-OFF
PDP-6 PURE source history.
Tasks are grouped by remaining acceptance condition, not by
historical version or failed experiment.

Both historical checklists remain recoverable in Git: the
8,685-line mainline version (including existing local edits) at
`git show affe58a9:TODO.MD`, and the 10,190-line PURE-branch
chronicle at `git show ae03d40f:TODO.MD`. Completed milestones
belong in `PROJECT-STATE.MD`, `KCORE.MD`, source-linked design
reports and `pdp10-doc`, not in this active TODO.

Priority: fix **compiler/assembler correctness** before new features;
then KCORE correctness and missing physical acceptance; then the
required 0.9 applications/storage tools; optimize resident RAM **last**.
No workaround may replace a correctness fix. All tests belong in the
appropriate daimos-testkit (or toolchain testkit) repository.

**Release boundary:** PDP-6 RAW PURE v1674 48K GENERAL/STRICT physical
committed-MM high-water is accepted: 3122 original MM writes per profile,
minimum 1255 free words, complete nine-owner drop/restore and post-WAIT
recovery. It does NOT accept PI6 fault pressure, bitmap mode3,
KA10, alternate roots, or production RAW PURE enablement.
P0-B trusted-operator
DTVOL2, P1-C and P1-D are closed under their documented contracts.
The existing grouped D6 block codec and restricted privileged MEMFS
XIP are production-integrated; do not reopen their obsolete proposals.

## P0. Correctness blockers (fix before optimization)

- [ ] **v1817 DAS host expression fix native acceptance:**
  standalone DAS correctly rejects differences of independent DOBJ1
  relocation bases; source-linked PDP10-testkit DAS fast checks pass
  13/13. Commits `ed2d2e5` and `aa74f42` were pushed to the authorized
  DAS HTTPS remote in v1827 using the existing DAIMOS Git credential
  helper. Publication is closed; separately verify full native DAIMOS
  guest assembly behavior against matching images.

- [ ] **v1814 source-linked CAT/MAKE guest acceptance:** focused host
  CAT S6REC and native MAKE KCC/DAS frame tests pass; CAT and MAKE DXRs
  link. The fresh 256K MAKE guest image was blocked by DSH TEXT
  `036133` exceeding its `036000` limit. Reproduce and minimize the
  actual DSH image or identify a source/compiler/folding regression;
  do not raise the limit or claim physical runtime acceptance without
  evidence. Retest native MAKE option/goal semantics and long CAT
  record behavior on a source-matched boot. See
  `design/cat-make-dirty-consolidation-v1814.md`.

- [ ] **Native KCC KPARSE correctness:** investigate the still-reported
  constant-expression overflow on ordinary IDENTWDS and array sizes,
  including the original CHKOVF/JFCL semantics, generated code and
  source/toolchain identity. Retest with current KCC; do not disable
  legitimate overflow detection. Capture native KPT4 progress.
- [ ] **Native KPARSE progress and RAM:** diagnose the KIR graph writer's
  linear FINDPTR lookup (captured 7679-entry scan, quadratic work),
  design/test a bounded-memory fix if still reproducible, and measure
  native runtime/RAM. Separately examine the event 050636 stalled
  storage-state observation before changing I/O wakeup behavior.
- [ ] **Compiler/assembler regression gate:** rerun current PDP-6
  codegen/ABI/relocation and native KCC phase tests after each fix;
  correctly rebuild and install a current source-matched host toolchain
  before judging native KCPP/KPARSE/KOPT/KGEN. Never accept stale
  binaries or paper over compiler errors in the kernel.
- [ ] **v1812 native linker acceptance (host fixed):** the recovered
  unused-EXTERN and P10RUN expiry defects now pass source-linked host
  and negative-control regressions in both host implementations.
  Native file-backed DLINK compiles and links from current sources
  with a bounded three-word reference bitmap and zero BSS growth.
  Execute actual guest-native selective-archive/missing-symbol and
  RH18/LH18 tests before claiming native runtime acceptance. See
  `design/toolchain-extern-expiry-v1812.md`.
- [ ] **Permanent FILE and MM provider failures:** diagnose
  missing/type/owner MM extents and permanently refused VFS/FILE,
  PIPE/FIFO, display and process-teardown resources. Preserve exact
  owners and readable diagnostics; provide safe bounded retry and
  administrative recovery without forced release. Extend whole-system
  fault injection to actual scheduler/EXIT/EXEC failure cases.
- [ ] **Real `?RT` failures:** retain failing image, original media,
  full SIMH transcript and root/INIT handoff; reproduce and identify
  the fault rather than using a clean rebuild as a purported fix.
  Revisit the old v669/v670 pre-VFS D6FS root failure only if still
  reproducible on current source and matching media.

## P1. KCORE P1-E: swapless PURE RAW (release-blocking)

- [ ] **PI6 reclaim/restore fault pressure:** test actual multi-process
  RAW prefix reclamation and restoration with independently triggered
  MM/pin/allocator, backing and real PI6 interruption conditions.
  Preserve exact PROCESS/PACKED owner, TRANSITION, READY_ONCE and
  source/backing lifetime on failure and retry; validate scheduler
  progress, WAIT/EXIT and final conservation. Respect slot-0 FULL
  serialization; do not fabricate impossible concurrent user dispatch.
- [ ] **Uncertain MM failure policy:** fully exercise persistent
  pin/free/rebase errors, restore cancellation and terminal/reaper
  disposition under live RAW pressure. Fail closed if an owner cannot
  be proved singly owned and unpinned; do not publish an ambiguous APR
  or silently free the sole retained suffix.
- [ ] **Executable source lifetime under pressure:** extend the real
  physical RO/mount/provider failure and return tests to nine-child
  RAW reclamation and bounded FULL fallback, including immutability,
  source loss/replacement and owner-safe unmount refusal. Retain the
  accepted v1643/v1650 failure and PI6 scheduling contracts.
- [ ] **Alternate-root boot matrix:** source-build and physically test
  ROOT=auto for TSFS without AUXSTORE, positive DSK/DRM BACKSTORE,
  selected disk/drum roots, and missing or disappearing controllers.
  Verify selected DISK SWAP versus PURE MRES, no phantom service,
  native RUN/EXEC/BRK/EXIT/WAIT and recovery. Do not regress backed
  swap or the P0 critical root/BACKSTORE fail-stop paths.
- [ ] **KCC FULL and RAM profiles:** run current-source matching-image
  KCC FULL physical root/service matrix and source-linked 48K/64K
  general/strict runtime ownership, temporary peak and persistent
  MRES cost. Compare feature OFF versus RAW with installed/transient
  KCORE+KINIT+MRES accounting and actual APR placement; no assumption
  of 32K full boot success.
- [ ] **PDP-6 production PURE acceptance:**
  v1809 integrated the source history into main and passed the native
  matrix plus default-OFF build; K05 implementation closed v1916.
  K06 closed v1922; K07/K08 closed v1947 after genuine physical
  loss, distinct foreign refusal, same-medium recovery, READY_ONCE
  and five exact WAITs with complete MM conservation. K09 production
  safety/physical gates remain open, including
  organic allocator fragmentation/descriptor
  exhaustion and wider PI6 cases. PURE must remain OFF by default. Every named
  DAIMOS and testkit topic tip is now reachable from local main.
  v1809 retired 95 clean unattached local and 94 remote merged
  topic refs. The remaining checked-out integration and dirty
  worktrees retain their branch refs and ignored/untracked files;
  do not remove them without separate preservation review.
  Historical procedure: `design/daimos-branch-consolidation-v1677.md`;
  current manifest: `design/daimos-branch-consolidation-v1809.md`.
- [ ] **KA10 target backend (mandatory K17 in kcore-todo.md):**
  implement contiguous low-segment relocation/protection, without
  a pager or mandatory KT10A high segment. Complete target boot,
  APR/MM, PI/trap, context, RUN/EXEC/BRK/EXIT/WAIT, swap and RAW PURE
  with actual KA10-specific native/simulator acceptance. Reuse common
  ownership policy. PDP-6 physical evidence cannot prove KA10 support.
  Schedule after K09 and before optional K10-K16 mode3.

## P2. KCORE P1-E: optional compressed mutable RAM (mode3)

- [ ] **Executable opt-in policy:** finalize unused, backwards-compatible
  DXR admission/header representation for COMPRESSIBLE_MUTABLE, keeping
  COMPRESSED executable payload and BSS bits distinct; prove fork/EXEC
  replacement, legacy/invalid headers and privileged admission.
- [ ] **Source-linked bounded overlap codec:** complete read-only
  preflight, 35/36/37-word and sparse/dense tests, exact encoded size,
  RAW fallback, partial overlap and forward/backward encoding/decoding.
  Reject nonshrinking outputs. Do not rely on a disjoint codec test as
  proof of in-place safety.
- [ ] **Temporary memory ownership:** reuse the still-owned prefix only
  where its full lifetime is proved; evaluate a VFS workspace solely
  with an explicit exclusive lease and callback/BCACHE safety.
  Otherwise acquire bounded pinned no-reclaim MM scratch. Never
  borrow freed memory or allocate an always-resident compression slab.
- [ ] **Atomic mode3 DROP publication:** integrate the staged v1659-
  v1667 bitmap frontend with the real scheduler/reclaim path. Prove
  source survival before the first overwrite, exact sole MM ownership,
  source/target overlap safety, complete encoder completion, and
  failure-safe RAW-vs-BITMAP commit with no partial publication.
- [ ] **Mode3 FULL restoration and terminal path:** integrate bounded
  decode, immutable backing/restore fallback, 2048-word APR limits,
  target-pin ordering, raw/bitmap terminal detach and VM destroy.
  Add exact wake/retry/EXIT/WAIT/finalizer accounting and preserve
  pin/owner identity on every error path.
- [ ] **PI6, MM and backing failure matrix for mode3:** verify actual
  process scheduler wake, READY_ONCE, source loss, corruption,
  allocation refusal, torn/overlapping data, uncertain MM and
  terminal cancellation under native/physical fault injection.
  Do not lift the permanent mode3 destroy refusal until safe.
- [ ] **End-to-end mode3 acceptance:** source-built 48K/64K physical
  marked and unmarked process RUN/EXEC/EXIT/WAIT; repeat sleep/wake,
  real DSH mutable-state samples, kernel/process memory high-water,
  compression payback versus OFF/RAW, negative witnesses and failure
  rollback. Enable opt-in mode3 only after net RAM and safety pass;
  disk-swap compression is not part of this feature.

## P3. KCORE platform and release matrix

- [ ] **32K kernel-only regression:** preserve native PDP-6 32K
  KINIT/boot-to-process-entry and error-path checks, without falsely
  requiring a full 32K ROOT/DSH workload. Repeat at 40K/48K/64K and
  supported high-core configurations when changing KINIT/MM.
- [ ] **KINIT remaining stack/error paths:** bound physical controller
  loss, abnormal MM/IO/root-selection and INIT-launch stack maxima
  with the existing 0400-octal (256-word) reserve; do not shrink the
  stack without complete fatal/error-profile measurements.
- [ ] **Current-source final kernel matrix:** GCC/KCC, tape/disk/drum,
  TSFS/D6FS, 48K/64K/96K/256K as supported; PI6/PIO, swaps,
  mount/provider loss and return, failed writes and correct HALT,
  process/FILE/PIPE ownership, BCACHE/MM pin conservation, KINIT
  unload and selected-MRES placement. Recheck known P0-B/P1-C/P1-D
  acceptances; do not reimplement them.
- [ ] **Combined storage/controller regressions:** exercise the
  current-source ROOT/service combinations and late controller
  disappearance, including independent mounted secondary failures,
  BACKSTORE/BCACHE/LOGSTORE owner and dirty/write-through isolation,
  physical DSK/DRM fault routing, PI2/PI6 scheduling and fair retry.
  The 26-volume mount bound is a logical capacity, not 26 drives.
- [ ] **RAM/fragmentation quality:** measure exact physical MM free,
  largest gap, process APR rounding waste, owner-table footprint,
  allocation peaks and scheduler overhead at 32K kernel-only and
  48K/64K/256K supported profiles; improve compaction/placement
  only where accepted behavior and whole-machine RAM improve.
- [ ] **Functional freeze:** run one clean, source-matched, repeatable
  complete testkit/release matrix with all required KCORE features
  accepted or explicitly deferred; document fixed/resident KCORE,
  optional MRES, transient KINIT, process RAM, PI and faults.

## P4. Storage, VFS and time administration

- [ ] **AUXSTORE and BLOCKSET inventory:** discover other usable DSK/
  DRM units and complete member sets safely, without probing absent
  devices or guessing reserved membership. Count independent D6FS
  volumes as mounts, BLOCKSETs as one; test physical tape+drum,
  disk+drum and root/secondary combinations on supported hardware.
- [ ] **VFS capacity and device-loss follow-up:** validate the already
  boot-selected dynamic mount slab and 26-slot/file-mount overlays
  across full root/provider/controller combinations, sparse high
  mount IDs, late removal and returned media. Preserve installed
  provider bounds and ABI; do not revive historical four/16-slot
  activation or theoretical 26-drive hardware requirements.
- [ ] **MKSWAP utility:** supply inspection/dry-run and explicit
  destructive confirmation for separately chosen DSK/DRM swap and
  optional LOGSTORE regions; reject existing formats and overlaps.
  Unattended boot never autoformats swap.
- [ ] **Wall-clock physical validation:** source-matched PCLK-present/
  absent, configured 50/60-Hz scheduler-versus-calendar timing,
  rollover and PCLK/controller loss under load. Preserve accepted
  SETTIME/DATE, privilege, TIME36, GETTIME race and filesystem mtime
  semantics; do not reimplement their already proven code.
- [ ] **Online D6FS packer (0.9):** move extents in a mounted and
  active D6FS using VFS ownership and minimum per-file exclusion;
  preserve open FILEs, reserved swap/LOGSTORE/BADMAP regions, crash
  recovery and concurrent safe I/O. Distinguish from offline packfs.
- [ ] **Persistent LOGSTORE/BSTORE isolation:** test in-flight mapped
  DEVICE/VOLUME errors, dirty cache isolation, complete rollback and
  LOG recovery on current native/physical KCORE. Add only verified
  missing migration steps; P1-C is not reopened wholesale.
- [ ] **Safe recovery mode and maintenance media:** design a small
  single-user/recovery DECtape with existing tools, separate from a
  new INITFS root; test cold boot and no-data-loss repair policy.

## P5. Toolchain, build and userland (0.9)

- [ ] **Finish native KCC self-hosting:** once P0 compiler bugs are
  fixed, run native MAKE ALL/INSTALL, measure KPARSE/phase RAM and
  time, rebuild kernel and one controlled userland component using
  native CPP/KCC/libc/CRT0/libsys, then test full native rebuilding.
  The archived v1413 proposal for staging DAS/MAKE C-SIX sources and
  native MAKEFILEs was never guest-accepted; re-evaluate that source
  before reusing it. Do not lift DSH_IMAGE_LIMIT from 036000 to
  037000 merely to enable the staging phase. See v1823 in state.
- [ ] **Native MAKE large-graph correctness:** test graph limits above
  768 dependency/384 recipe links, sorted-index cost, actual KCC
  graph invalidation, suffix rule ordering, and the exact required
  recursive variable/MAKEFLAGS and recipe syntax. Keep SIXBIT-only
  uppercase input and serial execution; do not add parallel builds.
- [ ] **Native MAKEDEPEND (deferred within 0.9):** evaluate using
  native KCC preprocessing for dependency extraction; otherwise port
  a minimal conservative C89 recursive include scanner with cycle,
  -I and Makefile-output handling. Test on host and DAIMOS against
  the imported `userland/makedepend/PORT-STATUS.MD` contract.
- [ ] **Full GCC/KCC regression groups:** rerun only remaining failed
  disk-testkit groups and complete an end-to-end clean matrix after
  toolchain fixes; previously passed individual groups are not new
  implementation tasks.
- [ ] **DSH:** finish its dedicated test coverage and documentation,
  frozen language/case/globbing, scripts/functions, external pipelines,
  job-control TTY, bounded interactive editor, optional SIXMD
  completion and acceptance. Detailed task authority:
  `userland/dsh/TODO.MD` and `pdp10-doc/USER/DSH_DESIGN.MD`.
- [ ] **Core userspace commands:** complete the genuinely missing
  required ECHO/BASENAME/DIRNAME/TRUE/FALSE/EXPR/CAT/CP/CMP/GREP/
  HEAD/WC/TEE/STRINGS/LS/FIND/STAT/MKDIR/RM/MV/PWD/CHMOD/XARGS/
  TAIL/DD/CHOWN/RMDIR/TOUCH variants, checking current sources
  first. Do not re-port commands already accepted by the testkit.
- [ ] **MonitorFS administration:** complete PS/FREE/DEVSTAT/MEMSTAT,
  clock/media diagnostics and other native admin utilities without
  reviving obsolete DAIMOSv1 SYSCTL/DEVIO interfaces.
- [ ] **Elvis VI/EX:** port after native build tools and baseline
  commands function; include required spelling/folding, not optional
  visual syntax highlighting, colors or URL features.
- [ ] **Host image-tool regressions:** add target-source-format and
  roundtrip tests for MKDT, MKSTREAM and WORDS2PT; keep current
  production grouped-codec Stage-1/DXR/TSFS format fully matched.

## P6. RAM-first optimization (only after functional freeze)

- [ ] **Final linked KCORE sweep:** follow `SIZEOPT.MD`, auditing fixed
  `proc_swap`, `bcache`, `fs_mres`, controller dispatch, PCLK,
  `fs_block_workspace`, process descriptors and dynamic mount paths
  against current images. No old pre-v1464 codec size assumptions.
- [ ] **Folding and boot specialization:** rerun p10fold/p10super
  against the final matching link graph, checking relocations,
  cross-MRES ownership, removable-module dependency and JRST hot
  paths; implement only source- and runtime-proven net RAM savings.
- [ ] **KINIT reserve, APR alignment and final image:** optimize
  actual memory high-water, register-liveness, dead data, startup
  specialization and temporary workspaces only after the remaining
  physical faults/root profiles are accepted.
- [ ] **Strict full 32K boot (deferred):** aim for fixed KCORE below
  octal 020000, preserve 8K boundary probe, measure total process/
  DSH/LOGIN footprint and CTY-only build. Full 32K DSH/commands
  are a separate size and userspace optimization goal, NOT a KCORE
  0.9 feature acceptance blocker; kernel-only 32K tests remain mandatory.
- [ ] **LIBC assembly candidates (deferred):** profile bounded-RAM
  `qsort` and only then `strcpy`, `strncpy`, `strcat`, `strncat`,
  `strchr`, `strrchr`, with native timing, size, stack and semantic
  regressions; keep ordinary C unless measured benefit is clear.

## P7. Post-KCORE / explicitly deferred architecture

- [ ] **Unified TSFS before 0.9 release:** after KCORE freeze and its
  RAM sweep, implement a compact single TSFS namespace/read engine and
  supported root/auxiliary profiles. Use the sharded-metadata proposal
  as research input, not a mandated on-disk format.
- [ ] **Optional userspace overlays and shared TEXT:** after working
  self-hosted non-overlay programs and measured DSH/KCC/editor memory,
  experiment with safe opt-in overlays and PDP-6 shared execution
  areas without growing KCORE's permanent footprint.
- [ ] **Type-340/344 runtime input:** implement minimal hardware-faithful
  light-pen coordinate, edge/stop, acknowledgement and resume support
  on AAP without burdening ordinary display refresh/PI7 handling.
- [ ] **MIT MAC HACK chess:** after that display interface is accepted,
  port the original ITS game and Type-340 art while isolating its
  ITS-specific monitor/book/storage code.
- [ ] **Post-1.0 MRES dynamic unload/reload:** revisit relocatable,
  removable and dynamically sized runtime modules only after 0.9;
  do not create mandatory resident retargeting infrastructure now.
- [ ] **Future hardware and process models:** complete slave reverse
  interrupts, PCLK-delivery variants and later PDP-10 hardware pager
  only under independent architectural requirements and test coverage.

## Documentation and gate discipline

- `PROJECT-STATE.MD` and `KCORE.MD` retain accepted chronological
  evidence; `SIZEOPT.MD` holds the deferred RAM optimization details.
- `userland/dsh/TODO.MD` owns the DSH-specific functional checklist;
  `pdp10-doc` owns completed architecture descriptions, not handoffs.
- Keep unresolved tasks here, in their proper section; do not append a
  new dated history entry for each completed test. Delete a completed
  checkbox from this file after the source-linked evidence is accepted.
- Before starting a task, inspect current implementation, native tests,
  optional/boot-selected MRES, physical maps and the main/development
  branch boundary. Do not turn superseded historical ideas into features.
