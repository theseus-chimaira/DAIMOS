## V64 validation follow-ups (2026-10-20)

- [ ] Repair swapped-process NAME/CMDLINE/ENVIRONMENT reads in MonitorFS
  without allocating persistent per-process metadata.  KCC/GCC compilation
  and fresh 256K image linking succeed, but the live `CAT 2/NAME` and absolute
  process-name checks fail.  The reader, assembler integration and Makefile
  remain uncommitted; retain the unchanged failing test oracle.
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
- [ ] Recheck group 023 as a single group and resolve remaining GCC/KCC
  matrix failures; focused subtest success is not a complete suite pass.
- [ ] Rework secondary TSFS/DTFS mount acceptance to match the intentionally
  root-selected MRES package. Do not introduce extra resident MRES solely
  to satisfy a fixture that boots the wrong root class.
- [x] Repair GCC local 9-bit char-array loads across 36-bit words at
  -O1/-O2/-Os; preserve the selected stack word rather than interpreting
  the word address as a byte offset.  Testkit v45 target execution passes
  at -O0/-O1/-O2/-Os, compiler commit df27247.
- [x] Retest full GCC DSH conformance with a fresh 256K disk image:
  115/115 PASS, including HELP ECHO and HELP DAS previously failing.
- [x] Update three outdated D6FS assembly test oracles, TSFS eight-member
  test compilation and SIXBIT headers, and the PDP-6 stack-adjustment
  codegen oracle.  All six focused cases pass, testkit commit 2804838.
- [ ] Diagnose the distinct GCC multiprocess scheduler preemption failure:
  corrected v45 GCC still produces `<aAbB2cCxxxxxxxxdDeEyyyyyyyy314D5`
  and does not schedule all boot users before the first CPU checkpoint.
  Verify clock/quantum behavior and compare KCC before changing assertions.
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
