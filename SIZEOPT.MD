# DAIMOS SIZE OPTIMIZATION WORKLIST

DATE: 2026-10-06

THIS FILE CONTAINS ONLY LIVE SIZE-OPTIMIZATION WORK.  COMPLETED, REJECTED,
SUPERSEDED, OR EXHAUSTED IDEAS BELONG IN GIT HISTORY AND MUST NOT ACCUMULATE
HERE.

THE PRIMARY TARGET IS THE FULLY CONFIGURED PERMANENT PDP-6 IMAGE:

    MULTI_LAST < 040000

THE CURRENT MEASURED BASELINE AFTER THE REMOTE SHRINK SERIES, THE PDP-6
MM/D6FS/MEMFS ASSEMBLY PASS, AND CORRECTED PERMANENT-MRES ACCOUNTING IS:

    KCORE      022400   9472 WORDS
    KCORE+MRES 040552  16746 WORDS
    MULTI_ROOT 040613
    MULTI_LAST 040672

THE IMAGE IS THEREFORE 442 ADDRESS WORDS ABOVE 040000.

NOTE (2026-10-27): THE ABOVE OCTOBER 6 BASELINE IS HISTORICAL, NOT THE
CURRENT LOWMEM LINK. ACCEPTED V496 GCC PDP-6 LOWMEM MEASUREMENTS ARE
KCORE 8548, TAPE KCORE+PACKAGED MRES 13339 AND DISK 13415 WORDS.
THESE ARE DIFFERENT BUILD CONFIGURATIONS, NOT A VALID SHRINK DELTA.
ACCOUNT FOR ACTUALLY INSTALLED MRES AND PERSISTENT MM ALLOCATIONS TOO.

THE PREVIOUS REPORTER COUNTED ONLY MEMFS_RUNTIME FOR THE MEMFS PACKAGE AND
OMITTED BADMAP.  THE INSTALLED MEMFS PACKAGE ALSO CONTAINS MEMFS_DATA AND
MEMFS_SNAPSHOT; BADMAP IS A REAL RESIDENT MRES WHEN ENABLED.  THE REPORTER NOW
COUNTS ALL OF THESE INSTALLED OBJECTS, SO PREVIOUS BELOW-LIMIT RESULTS ARE NOT
COMPARABLE WITH THIS CORRECTED FULL-CONFIGURATION TOTAL.

MEMFS IS THE ACTIVE SOURCE-LEVEL TARGET.  ITS CURRENT COMPLETE PACKAGE IS 1382
PERMANENT WORDS.  THE FIRST PDP-6 PASS ALREADY REPLACED THE KCC DATA AND
SNAPSHOT OBJECTS; THEIR TEXT FELL FROM 796+343 TO 477+216 WORDS, A 446-WORD
TEXT SAVING BEFORE LINK-TIME EFFECTS.

THE BCACHE CHANGE INTENTIONALLY SPENDS 21 PERMANENT WORDS TO REMOVE 130
DYNAMIC CACHE WORDS.  THE RECLAIMABLE PRIVATE SLAB IS NOW 0201 OCTAL WORDS
(129 DECIMAL), DOWN FROM 0403 OCTAL (259 DECIMAL), WHILE THE EXISTING
FS_BLOCK_WORKSPACE SERVES AS CACHE SLOT 0.  THE NET ACTIVE-CACHE SAVING IS
109 PHYSICAL RAM WORDS.

SIZE WORK MUST NOT ADD HIDDEN PERMANENT RAM, GENERAL METADATA, OR A HOT-PATH
INDIRECTION WHICH COSTS MORE THAN THE SAVING.  RAM USE IS THE PRIMARY
OPTIMIZATION GOAL; EXECUTION SPEED IS ALMOST AS IMPORTANT.  KEEP TINY FIXED
TABLES LINEAR UNLESS MEASUREMENT PROVES A BETTER REPRESENTATION.

## 1. IMMEDIATE PERMANENT-IMAGE CANDIDATES

## 2. DATA-LAYOUT AUDITS FOR PDP-6 INSTRUCTION COST

REVIEW compact resident structures for layouts which reduce both storage and
instruction count.  On PDP-6, a theoretically denser field can lose if every
access needs extra shifts/masks.

ACTIVE REVIEW TARGETS:

- FILE descriptors.
- storage requests.
- small provider/device descriptors.
- MonitorFS device metadata where address and flags can safely share a word or
  halfword.

PREFER layouts naturally accessed by HLRZ/HRRZ, DPB/LDB, or a single mask.

## 3. FINAL FOLDING / INITIALIZATION-TIME SPECIALIZATION

### 3.1 CROSS-DOMAIN P10FOLD

RUN THE FINAL CROSS-DOMAIN CENSUS ONLY AFTER SOURCE-LEVEL SHRINKING STABILIZES.
THE CURRENT broad all-MRES-symbol experiment found about 46 words of possible
one-JRST folds, which is only an upper bound because hot paths were deliberately
not filtered.

PROCEED ONLY IF A CONSERVATIVE COLD-SYMBOL SET STILL SAVES MORE THAN ABOUT 20
PERMANENT WORDS AND KCORE DOES NOT GROW.

P10FOLD CURRENTLY ANALYZES THESE CROSS-DOMAIN EXTERNAL-ANCHOR FOLDS BUT DOES
NOT EMIT AN EXECUTABLE CROSS-DOMAIN FOLD PLAN.  IF THE CONSERVATIVE CENSUS
STILL QUALIFIES, COMPLETE THAT TOOLING IN TOOLS/HOST RATHER THAN hand-coding
ad-hoc DAIMOS trampolines.

### 3.2 PATCH-TO-FOLD / BOOT-CONSTANT SPECIALIZATION

FOR NEAR-DUPLICATE COLD ROUTINES WHICH DIFFER ONLY IN A FEW AC, EA, DEVICE,
OR TARGET FIELDS, CONSIDER INITIALIZATION-TIME PATCHING SO ONE common body can
serve multiple cases.

USE THIS ONLY WHEN:

- the differences are boot-constant or otherwise immutable while executing;
- no runtime branch/dispatch cost is added to a hot path;
- the final linked saving is material, preferably more than about 20 words;
- patch sites and invariants are documented as part of the internal ABI.

DO NOT introduce general runtime self-modification merely to make source look
more uniform.

## 4. FINAL IMAGE-ONLY DEAD-DATA AUDIT

AFTER THE REMAINING SOURCE CHANGES, inspect the final KCORE/MRES maps for:

- exported symbols no longer referenced after MRES linking;
- runtime tables used only during KINIT/MINIT;
- duplicated immutable metadata retained in more than one permanent domain;
- alignment/padding or package descriptors which can be removed without
  changing ABI or loader requirements.

THIS AUDIT MUST USE THE FINAL LINKED IMAGE.  OBJECT-FILE SIZE ALONE IS NOT A
VALID PERMANENT-SIZE RESULT.

## 5. CURRENT ORDER

1. DATA-LAYOUT AND MONITORFS METADATA AUDIT.
2. FINAL CROSS-DOMAIN P10FOLD / PATCH-TO-FOLD CENSUS.
3. FINAL LINKED-IMAGE DEAD-DATA AUDIT.

CONTINUE THIS SWEEP UNTIL EVERY LIVE IDEA ABOVE HAS BEEN MEASURED AND EITHER
LANDED OR REMOVED AS EXHAUSTED.  CROSSING 040000 IS A REQUIRED MILESTONE, NOT
THE STOP CONDITION FOR THIS FINAL SWEEP.

## 6. DEFERRED KCORE OBJECT AUDIT — AFTER MEMFS AND SWAP (2026-10-27)

DO NOT START THESE OBJECT EXPERIMENTS UNTIL THE ACTIVE LOWMEM STORAGE WORK
IS FINISHED: SWAPLESS AND MEMFS-FREE TAPE BOOT WITHOUT BACKING, OPTIONAL DRM, AND
KINIT-ONLY D6FS RESERVATION/BLOCKSET DISCOVERY. THE NATIVE 32K TSFS+AUXST1
SWAP/WAKE/WAIT TEST PASSED IN V496. WORK ON THE ITEMS ONE BY ONE, BASED ON
THE MOST RECENT ACCEPTED PATCH.

DECISION (2026-10-27): CANCEL AUTOMATIC MEMFS ACTIVATION BASED ON BACKING
STORE, INCLUDING SUB-64K BUILDS. KEEP CURRENT LOWMEM NO-MEMFS POLICY.
MEMFS ON SMALL SYSTEMS MAY BE REVISITED LATER ONLY THROUGH EXPLICITLY
LOADABLE MODULES, AFTER KINIT HAS RECEIVED ITS OWN SIZE OPTIMIZATION.
DO NOT IMPLEMENT A SPECIAL AUTO-LOADER OR CONDITIONAL KINIT-EMBEDDED MEMFS.

THE FIGURES BELOW ARE V496 PRE-FOLD INPUT OBJECT TEXT+DATA+BSS WORDS,
NOT EXPECTED SAVINGS. RE-MEASURE THE FULL LINKED KCORE, ACTUALLY INSTALLED
MRES, AND PERSISTENT DYNAMIC ALLOCATIONS: MM EXTENT REGISTRY, VFS MOUNT
SLAB, PROCESS/U-AREA TABLES, FILE/TTY BUFFERS, BACKING MAPS, IDLE STACK
AND PROVIDER CACHES. COMPARE UNBACKED TAPE, TAPE+AUXST1, DISK AND
RELEVANT DRM/BLOCKSET PROFILES. PRESERVE ABI, FILESYSTEM AND PROCESS
SEMANTICS. RUN ALL REGRESSIONS FROM DAIMOS-TESTKIT. REVERT ANY NET RAM
GROWTH; EXECUTION TIME IS THE SECOND PRIORITY.

### 6.1 FIRST — MINIMAL TAPE-ONLY KCORE

- [ ] **proc_swap: 353 (349 text + 4 BSS).** Trace precisely which calls
  are necessary for RUN/EXEC, PURE executable backing and zero-capacity
  process bookkeeping, versus physical swap-out/in and victim reclaim.
  Consider split/conditional residency only after tracing entry points and
  linked implications. Include proc_swap_entry_v481 (3) and
  backstore_entry_v484 (8) in the dependency audit. Do not break the
  zero-backstore boot or change the documented KCORE ABI.
- [ ] **bcache_pdp6: 101 (99 text + 2 BSS).** Determine whether any cache
  routing is needed with tape-only TSFS and no auxiliary device. Audit the
  shared fs_block_workspace as cache slot zero and the existing 129-word
  dynamic private cache slab. Do not double-count already reclaimed RAM.
- [ ] **fs_mres: 201 (192 text + 5 data + 4 BSS).** Inventory gateways,
  dispatch tables and patchable entry points. Separate minimal TSFS/root
  needs from optional D6FS, MEMFS, BLOCKSET and other provider operations
  while preserving removable provider semantics and public entry points.

### 6.2 SECOND — SERVICE BRIDGES, CLOCK AND COMPRESSION

- [ ] **dsk270: 20, drm236: 17, storage_router: 18.** Audit always-linked
  hardware service/interrupt bridges; preserve optional DSK/DRM backing.
- [ ] **monitorfs_runtime: 9.** Audit resident forwarding and the larger
  MonitorFS MRES dependency (1592 words in the lowmem image).
- [ ] **backstore_entry_v484: 8, proc_swap_entry_v481: 3.** Review their
  patchable ABI stubs and four authoritative backing state words with the
  proc_swap investigation, not independently as an unsafe removal.
- [ ] **pclk: 24.** Hardware PCLK must be optional. Complete the earlier
  DATE SETTIME and APR 50/60-Hz software wall-clock fallback first;
  measure any always-resident clock state and tick-path cost, preserving
  TIME36 and filesystem timestamps.
- [ ] **d6lz: 57, d6lz_fixed: 40.** Establish why two compression decoder
  objects are included, map all callers and boot/runtime requirements,
  and assess safe consolidation. Their 97 words are NOT necessarily
  redundant; do not merge them on naming evidence alone.
- [ ] **fs_block_workspace: 128 BSS words in vfs.** Check mandatory
  128-word sector I/O, callers, non-reentrancy, interrupt safety and
  relationship to cache slot zero. The buffer is already shared among
  providers; avoid adding copies or violating I/O contract assumptions.

THIS IS A QUEUE FOR SEPARATE EXPERIMENTS, NOT PERMISSION TO MAKE ALL
CHANGES TOGETHER. FINISH MEMFS/SWAP FIRST; THEN MEASURE, TEST, DOCUMENT,
COMMIT AND PUSH EACH ACCEPTED CHANGE.
