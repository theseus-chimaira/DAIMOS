## Size refresh and pending work (v513, 2026-10-27)

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
and compressed swap change occupancy and lifetimes. After those features
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

- [ ] After BSTORE/XIP/compressed swap integration, measure physical allocation high-water, total free words, largest free
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
- [ ] Validate KINIT failure-path stack reserve and 32K kernel-only boot;
  retain separate 48K/96K/256K DSH functional gates. Repair stale MM
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

Architecture-changing features requiring an explicit pre-sweep decision:

All five decisions are resolved below. An unchecked item now means
implementation/acceptance is outstanding, NOT that scheduling is undecided.

- [ ] **DECISION 1 ACCEPTED (2026-10-27):** Complete the D6 codec production
  streaming low-core decoder and atomic producer/consumer cutover before the
  final KCORE sweep. This is a version 0.9 release prerequisite, not an
  optional future experiment. Keep the old D6LZ36 system bootable until
  the new format passes native regression; do not ship mixed formats or a
  permanent legacy decoder. The decision is recorded, NOT implemented.
- [ ] **DECISION 2 ACCEPTED (2026-10-27):** Complete TSFS v1.2 sharded
  metadata, reader, boot/format compatibility and native acceptance AFTER
  the final KCORE optimization but BEFORE the 0.9 release. Keep the TSFS
  implementation in its provider MRES and host tools; preserve existing
  KCORE/VFS service interfaces. Audit any unavoidable permanent KCORE or
  KINIT ABI impact before freezing those interfaces. Decision only; v1.2
  is not yet implemented.
- [ ] **DECISION 3 ACCEPTED (2026-10-27):** Complete the remaining BSTORE
  migration to generic shared backing/BCACHE/LOGSTORE BEFORE final KCORE
  optimization as a version 0.9 prerequisite. Audit remaining gaps against
  the accepted AUXST1/BACKSTORE implementation; preserve raw AUXSTORE and
  existing D6FS-root validation contracts. Verify all production consumers,
  removable MRES boundaries, dynamic RAM and native rollback/recovery before
  freezing the service ABI. Decision recorded; implementation not complete.
- [ ] **DECISION 4 ACCEPTED (2026-10-27):** Complete production raw MEMFS
  XIP BEFORE the final KCORE optimization as a version 0.9 prerequisite.
  Implement privileged non-writing EXEC/RUN, provenance, swap/BRK exclusion,
  unpin and rollback; validate scheduled process lifetime on native PDP-6
  simulators. Existing admission and isolated VM tests are not production
  process-launch acceptance. Decision recorded, implementation outstanding.
- [ ] **DECISION 5 ACCEPTED (2026-10-27):** Implement and benchmark opt-in
  compressed process swap BEFORE final KCORE optimization, restricted to
  explicitly allowed executable images (for example, the shell). Mark
  eligibility in a validated executable-header flag at EXEC; unmarked or
  unapproved images MUST use raw swap without attempting compression.
  Fork inherits the opt-in only as appropriate to the image contract; EXEC
  replaces it. Keep an independent swap-record compressed bit recording
  actual encoding, plus atomic failure/non-shrinking raw fallback. Confirm
  eligibility cannot be forged by untrusted executables or writable media.
  Benchmark PDP-6 ticks, compressed blocks, transient RAM and full linked
  resident size. Accept production only with no net permanent RAM growth
  and passing mixed raw/packed rollback and process tests in daimos-testkit.
  Decision recorded; not yet implemented or benchmarked.

Explicitly outside the 0.9 final KCORE functional freeze unless a verified
release dependency is found: new dynamic/unloadable MRES framework, user
overlays, runtime module movement, hardware pager, online D6FS packer,
Type-340 input/chess, PDP-6 TS 1.4 foreign DECtape, and full 32K DSH
command execution (separate DSH shrink effort). Never auto-load MEMFS
based on AUXSTORE presence. Do not treat KCORE `proc_swap`, `bcache`,
`fs_mres`, p10fold or dead-data audits as functional prerequisites.

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
- [ ] Exercise simultaneously mounted D6FS/DTFS/TSFS filesystems above slot
  four on real/emulated attached devices when the boot MRES provider matrix
  supports those combinations; current D6FS-root and TSFS-root profiles do
  not install the other family's MRES, so do not advertise phantom slots.
- [ ] Reduce the accrued permanent KCORE descriptor/dispatch cost after the
  VFS activation path is stable; no workarounds or unsupported 32K claims.
- [ ] Repair stale daimos-testkit `mm-core` host fixture linking of
  `vfs_mount_limit` and `proc_storage_publish_v30`/
  `proc_swap_storage_invariant_v30`. These services are already used in the
  target kernel; the current host fixture predates their integration. Native
  PDP-6 swap/EXEC/RUN validation is passing independently.

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

- [ ] V15: all six MM slab table regions now have active DTFS/TSFS/D6FS/VFS
  callers, but the fixed four-slot runtime limit remains. Migrate the
  packed RO/SWAP/LOGSTORE representation and *all* provider bounds, including
  optional foreign DTFS personality XCT indexing, before raising the limit.
  The pointer-access conversion currently costs 3/15 permanent words in
  disk/tape-root profiles over v14: optimize for RAM as part of cutover.

- [ ] V14 KCORE audit eliminated nine redundant resident words. See
  `design/kcore-legacy-audit-20261016-v14.md` for protected compatibility
  paths and remaining table candidates. Optimize dynamic-pointer hot paths
  after the other provider tables have migrated; the 32K KCORE end still
  exceeds its strict gate by 1879 words.

- [ ] V13: the D6FS reader now uses the MM slab and the bootstrap fallback
  is down to one word. Finish DTFS/TSFS media and shape/extent, policy, and
  hardware-derived capacity before raising the live mount limit above four.
  Separately investigate the strict 32K KCORE end limit: fresh lowmem GCC
  disk/tape profiles end at 023540, exceeding 020000 by 1888 decimal words.

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
- [ ] Audit VFS mount tables, mount-ID dispatch, D6FS/DTFS/TSFS/MEMFS mount
  state, and packed per-mount policy before selecting a boot-time capacity.
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

- ADD DEC PDP-6 TIMESHARING SYSTEM 1.4 DECTAPE FORMAT COMPATIBILITY TO DTFS AS
  ANOTHER FOREIGN MEDIA PERSONALITY.  START READ-ONLY: DETECT/VALIDATE THE
  MONITOR 1.4 FORMAT, READ ITS BLOCK-1 DIRECTORY AND FOUR-WORD DIRECTORY
  ENTRIES, AND MAP LOOKUP/READDIR/STAT/FILE READS THROUGH THE EXISTING
  DTC/BCACHE/VFS PATH.  REUSE THE EXISTING PACKED PERSONALITY STATE; `030` IS
  CURRENTLY AVAILABLE AFTER NATIVE `000`, TENEX `010`, AND ITS `020`.
  CONSIDER WRITABLE COMPATIBILITY ONLY AFTER READ-ONLY SUPPORT HAS GOLDEN-IMAGE
  TESTS FOR DIRECTORY, BLOCK-CHAIN, AND ALLOCATION SEMANTICS.  DO NOT IMPLEMENT
  THIS BEFORE 0.9.
- RECOVERY/SINGLE-USER MODE: BOOTABLE RECOVERY DECTAPE MEDIA USING THE NORMAL
  MAINTENANCE TOOLS; DO NOT ADD AN INITFS ROOT.
- SLAVE REVERSE INTERRUPTS AND PCLK INTERRUPT DELIVERY.
- A HARDWARE-PAGER VM BACKEND FOR LATER PDP-10 MACHINES.

## Deferred: executable opt-in for compressed process swap (2026-10-15)

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
