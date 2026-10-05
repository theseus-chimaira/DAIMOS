# DAIMOS SIZE OPTIMIZATION WORKLIST

DATE: 2026-10-05

THIS FILE CONTAINS ONLY LIVE SIZE-OPTIMIZATION WORK.  COMPLETED, REJECTED,
SUPERSEDED, OR EXHAUSTED IDEAS BELONG IN GIT HISTORY AND MUST NOT ACCUMULATE
HERE.

THE PRIMARY TARGET IS THE FULLY CONFIGURED PERMANENT PDP-6 IMAGE:

    MULTI_LAST < 040000

THE CURRENT MEASURED BASELINE AFTER THE COMPACT PDP-6 SWAP TRANSACTIONS IS:

    KCORE      022615   9613 WORDS
    MULTI_ROOT 037624
    MULTI_LAST 037703

THE IMAGE IS THEREFORE 61 ADDRESS WORDS BELOW 040000.

SIZE WORK MUST NOT ADD HIDDEN PERMANENT RAM, GENERAL METADATA, OR A HOT-PATH
INDIRECTION WHICH COSTS MORE THAN THE SAVING.  RAM USE IS THE PRIMARY
OPTIMIZATION GOAL; EXECUTION SPEED IS ALMOST AS IMPORTANT.  KEEP TINY FIXED
TABLES LINEAR UNLESS MEASUREMENT PROVES A BETTER REPRESENTATION.

## 1. IMMEDIATE PERMANENT-IMAGE CANDIDATES

### 1.1 FILE DESCRIPTOR REPRESENTATION AND TABLE SCANS

FILE_RUNTIME.S REPEATEDLY WALKS THE 16-ENTRY, TWO-WORD FILE TABLE IN:

- FILE_DUP.
- LOCK CONFLICT/UPDATE PATHS.
- FILE_UNLOCK_MOUNT.
- FILE_CLOSE_ALL.

DO NOT ADD A CALLBACK ITERATOR.  AUDIT THE PACKED DESCRIPTOR REPRESENTATION
FOR A CHANGE WHICH removes tests/masks from several loops at once.  A useful
change should reduce both permanent instructions and per-descriptor RAM or
leave RAM unchanged.

EXPECTED VALUE: SMALL TO MODERATE.

## 2. RESIDENT/DYNAMIC RAM STRUCTURE AUDITS

### 2.1 TRIM POST-MOUNT FILESYSTEM AND STORAGE STATE

AUDIT LIVE STATE AFTER KINIT/MINIT and remove fields used only for discovery,
validation, or handoff.  Do not introduce a second copied runtime structure
unless it is smaller overall.

PRIMARY TARGETS:

- D6FS runtime reader/provider state.
- DTFS/TSFS mount state.
- MEMFS provider state.
- LOGSTORE state.
- BLOCKSET/BADMAP state.
- AUXSTORE handoff state.

D6FS is especially worth checking for superblock fields whose invariants are
already established before ordinary runtime I/O.

EXPECTED VALUE: potentially high dynamic-RAM saving; possible secondary KCORE
saving from fewer copies and offset operations.

### 2.2 U-AREA AND KERNEL-STACK LIFETIME OVERLAYS

AUDIT process-private temporaries and stack frames whose lifetimes cannot
overlap.  Candidate storage may be overlaid only when the sleeping-kernel
continuation model proves that the same words cannot be live simultaneously.

DO NOT make the u-area swappable in this pass.  Sleeping executive
continuations require a stable kernel stack.

GOAL: reduce per-process resident RAM without adding scheduler state.

### 2.3 LARGE SHARED SCRATCH / REFILL WINDOWS

RECHECK large non-process scratch which is serialized by existing VFS/provider
locking.  Shared scratch is preferable to duplicating full-block or refill
windows per provider/process when the serialization proof is already present.

THIS IS A RAM OPTIMIZATION FIRST; KEEP IT OUT OF THE permanent-image sweep if
indirection grows KCORE more than it saves.

## 3. DATA-LAYOUT AUDITS FOR PDP-6 INSTRUCTION COST

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

## 4. FINAL FOLDING / INITIALIZATION-TIME SPECIALIZATION

### 4.1 CROSS-DOMAIN P10FOLD

RUN THE FINAL CROSS-DOMAIN CENSUS ONLY AFTER SOURCE-LEVEL SHRINKING STABILIZES.
THE CURRENT broad all-MRES-symbol experiment found about 46 words of possible
one-JRST folds, which is only an upper bound because hot paths were deliberately
not filtered.

PROCEED ONLY IF A CONSERVATIVE COLD-SYMBOL SET STILL SAVES MORE THAN ABOUT 20
PERMANENT WORDS AND KCORE DOES NOT GROW.

P10FOLD CURRENTLY ANALYZES THESE CROSS-DOMAIN EXTERNAL-ANCHOR FOLDS BUT DOES
NOT EMIT AN EXECUTABLE CROSS-DOMAIN FOLD PLAN.  IF THE CONSERVATIVE CENSUS
STILL QUALIFIES, COMPLETE THAT TOOLING IN PDP10-TOOLS RATHER THAN hand-coding
ad-hoc DAIMOS trampolines.

### 4.2 PATCH-TO-FOLD / BOOT-CONSTANT SPECIALIZATION

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

## 5. FINAL IMAGE-ONLY DEAD-DATA AUDIT

AFTER THE REMAINING SOURCE CHANGES, inspect the final KCORE/MRES maps for:

- exported symbols no longer referenced after MRES linking;
- runtime tables used only during KINIT/MINIT;
- duplicated immutable metadata retained in more than one permanent domain;
- alignment/padding or package descriptors which can be removed without
  changing ABI or loader requirements.

THIS AUDIT MUST USE THE FINAL LINKED IMAGE.  OBJECT-FILE SIZE ALONE IS NOT A
VALID PERMANENT-SIZE RESULT.

## 6. CURRENT ORDER

1. DSK/DRM VALIDATION AND COMPLETION/WAIT AUDIT.
2. FILE-DESCRIPTOR REPRESENTATION/SCAN AUDIT.
3. STORAGE-REQUEST PACKING AUDIT.
4. POST-MOUNT FILESYSTEM/STORAGE STATE TRIM.
5. DATA-LAYOUT AND MONITORFS METADATA AUDIT.
6. FINAL CROSS-DOMAIN P10FOLD / PATCH-TO-FOLD CENSUS.
7. FINAL LINKED-IMAGE DEAD-DATA AUDIT.

CONTINUE THIS SWEEP UNTIL EVERY LIVE IDEA ABOVE HAS BEEN MEASURED AND EITHER
LANDED OR REMOVED AS EXHAUSTED.  CROSSING 040000 IS A REQUIRED MILESTONE, NOT
THE STOP CONDITION FOR THIS FINAL SWEEP.
