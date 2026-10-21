## 2026-10-22 PRIORITY: SIMH PROVENANCE AND PROCESS FAILURE

Correction to V39-V44 acceptance notes below: the testkit image labeled
V42 in fact contains V43's executable; a 115/115 result obtained from it
cannot certify the genuine V42 or reverted V44 standalone binaries.  The
same installed V43 executable and root disk have passed and failed under
separate isolated run histories.  No V43 source/compiler fault is proven.
The genuine V44 fixture exhibited external-command lookup failures in one
long test but succeeded in a shorter diagnostic run.  High priority: obtain
minimal reproducible process/loader/pipe regression with a fingerprint-
checked image, isolate process PC/UUO `340000000004`, and fix the underlying
cause.  The *last fully accepted* revision remains V41, image 030657,
BSS 012755 (octal), requiring 0660 additional image words to get strictly
below 030000.  Do not claim V42/V43/V44 accepted until correctly rebuilt
and fully tested.  Testkit provenance check: commit 6c41bb7.

# DSH TODO

## 2026-10-21 V39-V44 ACCEPTANCE / HIGH-PRIORITY V43 REGRESSION

V39 shared bounded SIXBIT-record append (image 030717); V40 shared name
slice copying (030674); V41 shared path-building without merging its
distinct slash semantics (030657); V42 shared their checked append loop
(030645).  All BSS measurements remain 012755 octal words.  V39 passed
161/161; V41 passed 171/171; V42 passed 115/115 standard conformance and
6/6 isolated pipeline/background probes.  No shell features removed.

V43 tried factoring the four NAME=VALUE split callsites (image 030636),
but two full conformance runs each failed five process-related cases,
starting with ECHO PIPEOK ! CAT.  The identical six-case process subset
passes when run in isolation.  The suspect helper is not called by the
prefix preceding the first failure; unchanged pipeline/launch routines
have identical normalized assembler listings versus V42.  Investigate
the code-layout-sensitive compiler/linker/loader/kernel process interaction
as a **priority bug**, not a reason to add branch padding or disable pipes.
The original implementation idea remains sound, but V43 is not accepted.
V44 reverted it so the current image (030645/012755) is the V42-tested
source while the V43 commit and failure logs remain available for analysis.

Still open: 0646 octal image words (422 decimal) to get STRICTLY BELOW
030000, full physical 32K boot acceptance, peak stack/BRK accounting,
and proper root-cause resolution of the V43 pipeline/UUO regression.


## 2026-10-21 V34-V37 PARSER OPTIMIZATION ACCEPTANCE

All parser language features remain in the 32K-default build.  V34 shared
token-to-AST word/quote-mask transfer; V35 cached keyword dispatch;
V36 cached AND/OR classification; V37 shared binary AST node linking.
GCC DXR image octal: 031326 (V32) -> 031105 -> 031071 -> 031036 ->
031021 (V37), saving 0305 octal/197 decimal words.  BSS stays 012755.
Every version passed 161/161 isolated target SIMH probes and the host
media-isolation test; source D6FS roots remained CLEAN.

Remaining gates: save at least 01022 octal (530 decimal) further image
words for *strictly below* 030000; measure stack/BRK collision risk and
physical 32K acceptance.  Do not reduce the grammar or job-control
capabilities as a shortcut.  Continue identifying shared, demonstrably
cold or inexpensive code paths and measure GCC link size per experiment.


## 2026-10-21 V31/V32 TEXT AND BOOT-TEST ACCEPTANCE

Implemented packed six-character-word equality for names shared by
alias and function lookup; removed the redundant `UNALIAS` character
loop in favor of that same helper.  GCC image 031365 -> 031353 ->
031326 octal; BSS remains 012755.  Net saving 037 octal words, without
changing full scripting, command substitution, jobs or line editing.
Target SIMH tests pass: packed alias/function 4/4, packed UNALIAS 4/4,
full shell conformance 115/115.  The root image remains CLEAN after tests.

The previous intermittent `?RT` resulted from testkit runs reusing a
root fixture marked DIRTY by a prior simulated session.  A new testkit
`--isolate-media` option makes per-run copies for DSH targets, leaving
true persistence/recovery tests in control of their media semantics.
D6FS code and its mount safety policy were not modified.

Still open: 01327 octal words (727 decimal) of text reduction to reach
strictly below 030000; stack/BRK and physical 32K runtime acceptance.
Do not fold the 1174 generic P10FOLD candidates indiscriminately: no
zero-execution-cost fold was found, and the safe cold-path criterion
must be established before inserting additional JRSTs.


## 2026-10-21 V30 GCC TOOLCHAIN CORRECTION ACCEPTED

The compact-HELP failures inherited by v26-v29 are resolved by rebuilding
and installing GCC from `gcc-pdp10` after the existing backend fix
`df27247` (the installed `cc1` was stale).  The v45 byte-boundary
target-executed compiler regression passes -O0/-O1/-O2/-Os; the two HELP
cases pass; the entire new GCC-built DSH v29 suite passes **115/115**.
No source-level HELP workaround was needed; 32K-default completion-off
GCC image/BSS remain 031365/012755 octal.  Still open: save 01366 octal
words to reach strict below-030000 image target, check stack/BRK and
physical 32K boot, and address the other deferred scripting/editor work.


## 2026-10-21 V29 OPTIONAL TAB COMPLETION

`DSH_ENABLE_COMPLETION=0` by default omits the external TAB completion
helper/worker and TAB insertion/listing code.  Explicitly compile with
`DSH_ENABLE_COMPLETION=1` to retain full interactive completion.  Both
variants link; image 031365 vs 032104 octal respectively, BSS 012755.
Cursor editing, line input, all scripting constructs and job control
remain enabled.  The 25-case `$<...>` suite passes with completion off.
Full conformance/real DCS editing and strict below-030000 text still open.


## 2026-10-21 V28 SHARED QUOTE-MASK LOOP

Combined repeated quote-mask propagation in `dsh_expand_quoted` into one
shared branch.  GCC image 032307 -> 032104 octal, saving 0203; BSS remains
012755.  SIMH angle-expansion suite 25/25; full conformance unchanged at
113/115 (the two prior compact HELP format discrepancies).  No script,
job-control or interactive features removed.  Next: independently gate TAB
completion for the 32K GCC build while keeping line editing and scripting.


## 2026-10-21 V27 ANGLE PARAMETER EXPANSION

Implemented the bounded, nonrecursive `$<NAME>` and `:-`, `:=`, `:+`, `:?`
forms, with explicit adjacent-prefix disambiguation from shell redirection.
Quoted and escaped dollar signs retain their existing literal behavior.
`:=` preserves a variable's existing export flag; defaults use bounded
caller-stack SIXBIT storage, with no new BSS.  Host lexer and 25-case SIMH
tests are in the testkit and pass; full conformance passes 113/115 with
the same two preexisting compact HELP failures as v26.  See
`design/DSH-ANGLE-PARAMETER-20261021-v27.MD`.

Beyond the present v27 subset: nested expansions, recursive defaults,
embedded `>`, general escape/quote expressions inside defaults, and longer
than 36-character parameter expressions are not supported.  No feature
limits have been increased.  Full conformance and below-030000 GCC text,
stack and real 32K physical-occupancy gates remain independent TODOs.


## 2026-10-21 V26 SHARED EDITOR / LOOP CONTROL

An experimental, allocation-free `edcore` shared line buffer now implements
packed-SIXBIT get/insert/delete/replace for DSH and future ED/EX frontends.
The 32K shell retains a command-line editor; it does **not** trade editing
correctness for removal of completion/history.  The multi-line ED/EX
store, ranges, ex command interpreter and terminal frontend are subsequent
layers, not duplicated in DSH.  See `design/DSH-EDCORE-20261021-v1.MD`.

BREAK/CONTINUE now have bounded level counts, loop action propagation and
a per-function loop floor.  The host editor test passes; dedicated SIMH
loop-control and full conformance must pass before this is accepted.

Next correctness milestones, independent of parameter expansion:

1. EXEC replacement preserving PATH search, argument/environment records
   and FD state, with no extra waiting DSH process.
2. COMMAND/WHENCE lookup diagnostics and bounded GETOPTS semantics.
3. Multi-variable READ and documented EOF handling; function locals.
4. Minimal EXIT/INT/TERM/HUP TRAP ABI integration, respecting DAIMOS events.
5. Descriptor redirections and bounded here-documents, including nested
   scripts and pipeline failure cleanup.
6. ED/EX line-store and command layers on top of the shared editing core;
   subsequently integrate Vi-style interaction without a second editor.
7. HISTORICAL V26 DEFERRED ITEM: `$<NAME...>` lexical disambiguation.  This
   was implemented in V27; the limitations are documented above.

All new features must retain the strict below-030000 GCC text target as
an explicit optimization gate, with separately measured BSS and stack.
Scripts, functions, job control, globbing and pipelines remain mandatory.


## V25 OPTIONAL HISTORY BUILD (2026-10-13)

DSH history is now compile-time optional and **off by default** for the
32K memory objective.  Define `DSH_ENABLE_HISTORY=1` in GCCFLAGS to
restore interactive Ctrl-P history and HISTFILE loading/appending.
No history source was removed.  History-on now uses the shared heap for
the sixteen-record cache, rather than 496 fixed BSS words; this is
compatible with nested temporary argv lifetimes.  The enabled mode must
still receive dedicated history/HISTFILE regressions before acceptance.

GCC PDP-6 linker measurements (octal words):

| | v24 | v25 off | v25 on |
| --- | ---: | ---: | ---: |
| Image | 032104 | 031321 | 032120 |
| BSS | 013742 | 012751 | 012763 |

The default build saves 0563 octal image and 0771 octal BSS words.
The history-disabled GCC build passes full SIMH conformance 115/115.
The history-enabled build links but its dedicated HISTFILE tests remain
to be run with `DSH_ENABLE_HISTORY=1`.
Remaining image reduction to strictly below 030000 is 01322 octal.
Continue examining AST/function payload BSS, stack and physical BRK
high-water; do not reduce language limits or introduce overlays.


V24 SCRIPT WORKSPACE INITIALIZATION (2026-10-13): `dsh_script_init`
now clears its word-sized token/workspace record with one linear pass,
instead of calling the SIXBIT token clearer 32 times.  GCC DXR image
decreases from 032117 to 032104 octal (013 words); BSS stays 013742.
Nested SOURCE SIMH tests pass 4/4 and full GCC DSH conformance 115/115.
The strict below-030000 text target still requires 02105 octal words
of reduction.  Next measure AST/function storage and history
lifetimes, plus peak stack and BRK; no capacity reductions by assumption.


V23 stored-function traversal experiment (2026-10-12): the separate
preflight-count and recursive-copy walkers have been combined into one
bounded traversal with an optional output buffer.  This retains the
preflight-before-mutation rule on function redefinition.  GCC image
changes from 032147 to 032117 octal (030 words saved); BSS 013742.
The direct host parser test and full GCC SIMH conformance (115/115) pass.
Strict image target remains below 030000.


## ACTIVE NO-OVERLAY SHRINK GATE (2026-10-12, V21)

V22 parser-node-clear experiment: one linear word-wise initialization
replaces the 22 repeated SIXBIT clears in `dsh_node_clear`.  GCC image
changes from 032177 to 032147 octal (030 words saved); BSS stays 013742.
Host direct parser test, 7/7 live variable-heap regression and full
115/115 GCC SIMH conformance pass.  The 030-word saving is retained.
Next prioritize genuine code simplifications: this alone is far short
of the strict 030000 image goal.  No overlays.

The target is STRICTLY BELOW 030000 octal DXR image words (not merely
meeting the production 036000 gate).  The current GCC `-Os` shared-heap
branch measures image 032177, BSS 013742.  Therefore at least 02200
octal image words must be removed.  Image, BSS, stack and heap/high-water
are separate budgets: BSS reductions alone cannot satisfy the text gate.

Prioritized open work, without overlays:

1. Profile GCC linked/map output by translation unit and function; remove
   duplicate parser/executor paths and cold code with *measured* image
   differences.  Preserve all 115 shell cases and the seven heap probes.
2. Revisit AST representation and function-node copying for unnecessary
   generic fields and duplicated traversal code.  Measure image delta
   independently of the potentially large BSS reduction; protect the
   16-node parser and 8 stored-function-node limits.
3. Audit lexer, expansion and executor scratch lifetime for safe BSS
   sharing.  Do not reuse buffers across nested SOURCE/function frames.
4. Measure maximum stack words under nested SOURCE, function recursion,
   glob fallback and 20-argument expansion; reduce proven dead stack
   reservations and generated GCC frame sizes, not safety margins blindly.
5. Measure shared-heap BRK high-water/free-list fragmentation with 64
   populated variables, repeated grow/unset/reuse and temporary argv.
   Tune allocator metadata only if peak physical APR pages improve.
6. Compact bounded history, job/alias/function and RUN request storage
   where real live-range evidence permits; retain 16 history records and
   the frozen user-visible limits.
7. Run GCC 32K root DSH EXEC and long-lived variable/command stress to
   verify INIT occupancy and physical allocation.  This remains distinct
   from the passing image-size and functional tests.

An exploratory GCC `-Oz` build was rejected by cc1 (`invalid option
'-Oz'`); it is not a viable size optimization on this compiler.
No overlays and no functional-language feature growth in this phase.


V20 COMPILER SIZE OPPORTUNITY (2026-10-12): the corrected shared-heap
experimental shell links under GCC with DXR image 032177 and BSS 013742
octal, versus KCC 037000/013742.  This eliminates 04601 octal image
words and passes the production 036000 image gate without overlays.
The GCC seven-case shared-heap SIMH regression passes.  Still measure
peak BRK and physical allocation before claiming resident RAM savings.
32K build policy is COMPILER=gcc; KCC remains supported for larger
machines.  GCC full DSH conformance also passes 115/115; 32K boot/EXEC
occupancy is not yet accepted by image-size success alone.
See `design/dsh-compiler-size-comparison-20261012-v20.md`.

V17 SHARED-HEAP CORRECTION (2026-10-12): fixed an actual double-free
when a variable slot was UNSET and reused.  A dedicated seven-case
daimos-testkit SIMH regression reproduced the timeout before the fix
and passes 7/7 after the fix.  Image 037000, BSS 013742 octal.
The corrected implementation passes 115/115 full SIMH conformance cases.
Separate argument-frame acceptance passes 9/9, and the corrected testkit
resource-contract check passes all eleven fixed limits (16 history entries).
Continue with actual resident
heap/BRK high-water measurement; the production 036000 image gate
remains open.  Work stays on the versioned experimental branch.

EXPERIMENTAL V17 (2026-10-12): an assembly-variable/shared-heap candidate
links with DXR image 036773, BSS 013742 (octal), compared to current-main
036523/015241.  The temporary image gate passes; production gate and full
SIMH acceptance do not.  Testkit's boot-image preparation is blocked by
missing `$HOME/cross/lib/kcc/bootstrap/KCC.dxr`.  Do not merge the
experimental allocator before real target execution, memory high-water
measurement, and regression of reassignment during wide argument expansion.
See `design/dsh-variable-allocator-shrink-20261012-v16.md`.

PHASE-ZERO VARIABLE SHRINK FOLLOW-UP (2026-10-12, V16): a shared allocator
prototype compiled and linked, reduced BSS from 015241 to 013742 octal, but
grew DXR image from the clean current-main 036523 to 037213 octal.  The
combined image+BSS improved by 0607 octal words; the image acceptance gate
failed.  Runtime behavior was not tested and the code was fully reverted.
The allocation idea remains valid: implement a lower-code-cost assembly
variant and run the target regressions.  See
`design/dsh-variable-allocator-shrink-20261012-v16.md`.  No overlays.

REVIEWED: 2026-10-10 (115/115 TARGET CONFORMANCE VERIFIED)

THIS FILE IS THE AUTHORITATIVE OPEN-WORK LIST FOR DSH.  THE FROZEN LANGUAGE
AND INVOCATION CONTRACT REMAINS PDP10-DOC `USER/DSH_DESIGN.MD`.  COMPLETED
ITEMS SHOULD MOVE TO GIT HISTORY AND CURRENT DOCUMENTATION RATHER THAN REMAIN
HERE AS STALE CHECKLIST ENTRIES.

## 0. TEST COVERAGE AND DOCUMENTATION FIRST

CURRENT TARGET BASELINE: 115/115 DSH-SPECIFIC SIMH PROBES PASS, COVERING THE
FROZEN COMMENT/EMPTY-ARG/QUOTING RULES, BASIC CASE/GLOB, VARIABLES/STATUS,
EXPORTED ENVIRONMENT, -C/SCRIPT POSITIONALS, SOURCE FRAME RESTORATION,
FUNCTION RETURN, IF/FOR/WHILE/UNTIL/NOT/AND/OR, STATUS 126/127, INPUT/OUTPUT/
APPEND REDIRECTION, REDIRECTION FAILURE, PIPELINE DATA FLOW, AND LAST-STAGE
PIPELINE STATUS.  THE SAME TARGET SUITE NOW COVERS THREE- AND EIGHT-STAGE
PIPELINES, PARTIAL-LAUNCH FAILURE CLEANUP, LOGIN -L PROFILE ORDER/ERROR
STATUS, MISSING PROFILES, THE NON-LOGIN NO-IMPLICIT-PROFILE POLICY, ADJACENT
QUOTED/UNQUOTED FRAGMENTS, `$$`, MISSING POSITIONALS, BEGIN/PAREN GROUPS,
FUNCTION REDEFINITION, FUNCTION-TABLE EXHAUSTION, AND RECOVERY AT THE MEASURED
THREE-CALL FUNCTION-RECURSION LIMIT.  JOB-CONTROL COVERAGE NOW ALSO INCLUDES
STABLE `%N` REFERENCES, DONE-STATUS REPORT/RECLAIM, STOPPED BACKGROUND TTY
READS, WAIT/BG RE-STOP BEHAVIOR, MOST-RECENT DEFAULT BG, FOUR JOB SLOTS, AND
DETERMINISTIC JOB-TABLE EXHAUSTION.  NATIVE S6REC STDIN SCRIPT EXECUTION AND
EXPLICIT `BEGIN ... END &` BACKGROUND GROUPS ARE ALSO COVERED; GROUP WORKERS
INHERIT EXPORTED ENVIRONMENT ONLY, KEEP THEIR SHELL-STATE CHANGES OUT OF THE
PARENT, AND REPORT THE GROUP'S FINAL STATUS THROUGH THE NORMAL JOB TABLE.
THE SAME TARGET SUITE NOW LOCKS THE SIXMD COMMAND-HELP CONTRACT: COMPACT
`HELP`, FULL `MAN`, ONE-LINE `APROPOS`, DAS OPTION HELP, AND SAFE
MISSING-METADATA DIAGNOSTICS.  IT ALSO COVERS THE 64-SLOT COMPACT VARIABLE
TABLE BEYOND THE FORMER 16-SLOT LIMIT AND SET/GROW/UNSET/REUSE BEHAVIOR.

- ADD DIRECT LEXER/PARSER REGRESSIONS FOR TOKEN-BOUNDARY COMMENTS, EMPTY
  QUOTED ARGUMENTS, QUOTED KEYWORDS, ESCAPING, REDIRECTION, PIPELINES,
  BACKGROUND MARKERS, ALL COMPOUND FORMS, MALFORMED INPUT, AND ARENA LIMITS.
- EXTEND TARGET/SIMH EXECUTION COVERAGE WITH GLOB/ARENA LIMITS, `$@`, AND JOB
  REFERENCES.
- KEEP THE DEDICATED FOREGROUND ACCEPTANCE FOR STOP/FG/CONTINUE HANDOFF,
  STOPPED-JOB RAW/COOKED TTY-MODE SAVE/REFRESH/RESTORE, AND SHELL RAW-MODE
  RECOVERY UNDER REGRESSION.
- ADD RESOURCE TESTS FOR DSH IMAGE/WORKING-SET SIZE AND THE FIXED PARSER,
  VARIABLE, FUNCTION, ALIAS, JOB, PIPELINE, AND COMPLETION LIMITS.
- KEEP `userland/manual/DSH.SIXMD`, PDP10-DOC `USER/DSH.MD`, AND
  `USER/DSH_DESIGN.MD` SYNCHRONIZED WITH TESTED BEHAVIOR.  DO NOT DOCUMENT
  ACTIVE WORK AS COMPLETE.

## 1. FROZEN LANGUAGE CORRECTNESS

- KEEP THE CORRECTED TOKEN-BOUNDARY `#` RULE, REAL `''`/`""` EMPTY ARGUMENTS,
  QUOTED-KEYWORD HANDLING, AND DOUBLE-QUOTE ESCAPING UNDER REGRESSION.
- KEEP ADJACENT QUOTED/UNQUOTED FRAGMENT CONCATENATION AND QUOTED VARIABLE
  EXPANSION WITHOUT IMPLICIT WORD SPLITTING UNDER REGRESSION.
- EXTEND POSITIONAL/SPECIAL-VARIABLE COVERAGE TO `$3`..`$8` AND `$@`.
- KEEP LOOP MEMORY BOUNDED WITHOUT AN ARTIFICIAL ITERATION LIMIT.
- EXTEND COMPOUND STATUS TESTS TO DEEPER NESTING AND FAILURE PATHS.

## 2. CASE AND GLOBBING

- DEFINE AND TEST DETERMINISTIC MATCH ORDER USING NATIVE DIRECTORY ORDER; DO
  NOT ADD SORTING MEMORY UNLESS A REAL REQUIREMENT JUSTIFIES IT.
- TEST ROOTED, RELATIVE, MULTICOMPONENT, MULTIMATCH, NO-MATCH, AND QUOTED
  PATTERNS, PLUS ARGUMENT-LIMIT EXHAUSTION.

## 3. SCRIPTS, FUNCTIONS, AND INVOCATION

- TEST STORED-NODE EXHAUSTION AND KEEP REDEFINITION, RETURN/POSITIONALS,
  TABLE EXHAUSTION, AND THE MEASURED THREE-CALL RECURSION LIMIT UNDER
  REGRESSION.
- KEEP BOUNDED STANDALONE `$()` SUBSTITUTION UNDER REGRESSION:
  NATIVE S6REC WORKER, 36-CHARACTER RESULT LIMIT, ERROR HANDLING AND CLEANUP.
  FULL INTERPOLATION AND NESTING ARE NOT PART OF THIS BOUNDED SUBSET.
- ADD THE HIGH-VALUE PARAMETER-EXPANSION SUBSET `${VAR}`, `${VAR:-DEFAULT}`,
  `${VAR:=DEFAULT}`, `${VAR:+ALT}`, AND `${VAR:?MESSAGE}` WITHOUT ADDING THE
  FULL KSH PARAMETER-EXPANSION LANGUAGE.
- ADD `EXEC` SO A SCRIPT OR LOGIN SHELL MAY REPLACE ITSELF WITH THE TARGET
  PROGRAM WITHOUT AN EXTRA WAITING SHELL PROCESS.
- ADD COMMAND-LOOKUP INTROSPECTION (`COMMAND` OR `WHENCE`) WHICH REPORTS
  BUILTIN, FUNCTION, ALIAS, EXPLICIT PATH, OR PATH-RESOLVED EXECUTABLE.
- ADD A BOUNDED `GETOPTS` BUILTIN FOR SERIOUS DSH SCRIPTS; REUSE THE COMMON
  OPTION POLICY WHERE PRACTICAL RATHER THAN DUPLICATING A LARGE PARSER.
- ADD MINIMAL TRAPS FOR EVENTS DAIMOS CAN REPRESENT CLEANLY, STARTING WITH
  EXIT, INT, TERM, AND HUP.  KEEP THE TABLE BOUNDED AND DO NOT INVENT A UNIX
  SIGNAL MODEL BEYOND THE KERNEL ABI.
- ADD FUNCTION-LOCAL VARIABLES IF THEY CAN REUSE THE EXISTING BOUNDED CALL
  FRAME/VARIABLE STORAGE WITHOUT A LARGE SECOND VARIABLE SYSTEM.
- ADD `BREAK` AND `CONTINUE` WITH BOUNDED LOOP-NESTING STATE.
- EXTEND `READ` TO MULTIPLE VARIABLES AND WELL-DEFINED EOF STATUS; ADD ONLY
  OPTIONS WHICH HAVE A REAL DAIMOS SCRIPTING USE.

## 4. EXTERNAL EXECUTION AND PIPELINES

- KEEP BUILTIN -> FUNCTION -> EXPLICIT PATH -> PATH LOOKUP ORDER UNDER TEST.
- VERIFY COUNTED-SIXBIT RUN V2 ARGUMENT/ENVIRONMENT RECORD LIMITS.
- KEEP THE EIGHT-STAGE PIPELINE LIMIT, INCREMENTAL PIPE-FD LIFETIME, EOF
  DELIVERY, AND PARTIAL-LAUNCH TERMINATION UNDER REGRESSION.
- KEEP STATE-CHANGING BUILTINS/FUNCTIONS OUT OF PIPELINE CHILD CONTEXT UNTIL A
  REAL DSH WORKER IMAGE IS MEASURED AND JUSTIFIED.
- ADD FILE-DESCRIPTOR REDIRECTION NEEDED BY REAL ADMINISTRATION/BUILD SCRIPTS,
  AT MINIMUM `2>`, `2>>`, `2>&1`, AND `1>&2`; PREFER A SMALL GENERAL `N>&M`
  IMPLEMENTATION IF IT COSTS NO MORE THAN SPECIAL CASES.
- ADD HERE-DOCUMENTS `<<` USING A BOUNDED TEMPORARY FILE OR PIPE; DO NOT KEEP
  AN ARBITRARILY LARGE DOCUMENT IN RESIDENT DSH MEMORY.

## 5. JOB CONTROL

- VERIFY MOST-RECENT DEFAULT `BG` WITH MULTIPLE LIVE JOBS; MULTIJOB DEFAULT
  `FG` RECENCY AND SINGLE-JOB DEFAULT FG/BG ARE NOW COVERED.
- KEEP THE FROZEN COMPACT `%N STATE [STATUS] COMMAND` `JOBS` CONTRACT AND
  DONE-RECORD RECLAIM UNDER REGRESSION.
- KEEP EXPLICIT `BEGIN ... END &` WORKER EXECUTION, PARENT-STATE ISOLATION,
  EXPORTED-ENVIRONMENT INHERITANCE, AND GROUP FINAL STATUS UNDER REGRESSION.
- KEEP MULTISTAGE FOREGROUND PIPELINE STOP/FG AS A WHOLE PROCESS GROUP,
  SINGLE-PROCESS FOREGROUND STOP/FG, AND BACKGROUND TTY READ
  STOP/CONTINUE/RE-STOP UNDER REGRESSION.

## 6. INTERACTIVE EDITOR

- KEEP THE BOUNDED HISTORY AND BASIC CURSOR-AWARE LINE EDITOR UNDER REAL-DCS
  REGRESSION; DO NOT ADD A FULL-SCREEN TERMINAL DATABASE.
- ADD A VI-STYLE COMMAND-LINE EDITOR WITH SMALL INSERT/COMMAND MODES.  TARGET
  THE HIGH-VALUE COMMANDS `H L 0 $ W B E X X R I A I A CW DW D C`, HISTORY
  `J/K`, `/` AND `?` SEARCH, `N`, AND A SMALL UNDO.  DO NOT IMPLEMENT VI
  REGISTERS, MACROS, MARKS, VISUAL MODE, TEXT OBJECTS, OR OTHER FULL-EDITOR
  FACILITIES.
- PERSISTENT `HISTFILE` WITH SIXTEEN-COMMAND 496-WORD CACHE IS IMPLEMENTED
  AND SIMH REBOOT-VERIFIED. COMPLETE CORRUPTION/TRUNCATION-INJECTION TESTS,
  MULTIPROCESS APPEND ACCEPTANCE, AND A SMALL `HISTORY` LIST/CLEAR BUILTIN
  IF IT DOES NOT DUPLICATE EDITOR CODE.
- SUPPORT HISTORY SEARCH/PREFIX RECALL THROUGH THE VI EDITOR.  ADD ONLY SMALL
  HISTORY SUBSTITUTION SUCH AS `!!`/`!N` IF MEASUREMENT SHOWS IT IS WORTH THE
  CODE; DO NOT IMPORT A FULL CSH/KSH HISTORY-EXPANSION LANGUAGE.
- KEEP CTRL-C SHELL-LINE CANCELLATION AND FOREGROUND-CHILD KERNEL CTRL-C
  PROCESS-GROUP HANDLING UNDER REAL-DCS REGRESSION.
- KEEP CTRL-Z FOR FOREGROUND JOB STOP; DO NOT TURN SHELL RAW INPUT INTO A
  SECOND INCOMPATIBLE SIGNAL PATH.
- EXTEND THE CURRENT GENERIC COMMAND/PATH COMPLETION TO VARIABLES AND JOBS.
  KEEP EXPENSIVE SCANNING IN DSHCOMP/HELPER CODE RATHER THAN RESIDENT DSH.
- DO NOT ADD AN EMACS COMMAND-LINE EDITING MODE.

## 7. SIXMD HELP/GETOPT/COMPLETION

- GENERATE BOUNDED COMMAND/OPTION/OPERAND COMPLETION METADATA FROM VALIDATED
  SIXMD; DO NOT PARSE FULL MANUALS ON EACH TAB.
- USE THAT METADATA FOR OPTION COMPLETION AFTER `-` AND, WHERE THE MANUAL
  DESCRIPTOR IS PRECISE ENOUGH, OPERAND-TYPE COMPLETION FOR PATH/COMMAND/JOB.
- KEEP METADATA ADVISORY: MISSING OR CORRUPT DATA MUST NEVER PREVENT COMMAND
  EXECUTION.
- SHARE MANUAL-DERIVED OPTION DESCRIPTORS WITH ADAPTED USERLAND COMMANDS WHERE
  THAT REDUCES DUPLICATION.

## 8. SIZE, CLEANUP, AND COMPLETION GATE

- MEASURE DSH IMAGE SIZE AND RUNTIME WORKING SET AFTER CORRECTNESS, NOT BEFORE.
- REMOVE DUPLICATE/DEAD EXECUTOR OR PARSER PATHS FOUND BY THE TEST PASS; DO NOT
  ADD OVERLAYS UNTIL REAL WORKING-SET MEASUREMENTS JUSTIFY THEM.
- OVERLAY-FIRST EXPERIMENT AND FUTURE SHARED-TEXT COMPATIBILITY: SEE
  `design/user-overlay-shared-text-experiments-20261012-v1.md` AT REPOSITORY
  ROOT. KEEP OVERLAYS OPTIONAL; PROFILE ACTUAL WORKING SETS FIRST.
- RUN THE COMPLETE DSH HOST + TARGET + LOGIN/MULTITTY REGRESSION SET.
- MARK SUBSTANTIAL DSH COMPLETE ONLY WHEN THE ACCEPTANCE CRITERIA IN
  `USER/DSH_DESIGN.MD` ARE VERIFIED.  KEEP CURRENT-STATE DOCUMENTATION
  ACCURATE THROUGHOUT DEVELOPMENT, EVEN BEFORE THE FINAL COMPLETION GATE.

## CURRENT PHASE-ONE BLOCKERS (2026-10-11)

- BOUNDED `$()` STANDALONE SUBSTITUTION IS NOW SIMH-VERIFIED FOR SUCCESS,
  WORKER FAILURE, OUTPUT OVERFLOW, MALFORMED SYNTAX AND CLEAN HALT.
  A SEPARATE `-S` WORKER EMITS S6REC AND RETAINS AT MOST 36 SIXBIT CHARS.
  DSH IMAGE 036444 EXCEEDS 036000: USER APPROVED TEMPORARY
  `DSH_IMAGE_LIMIT=037000` DURING PHASE ONE. SHRINK AFTER PHASE ONE.
- HISTFILE NOW RETRIES SHORT WRITES USING `u_write_words_all`, REPORTS
  FAILED WRITE/CLOSE, AND RECOVERS EXISTING RECORDS AFTER A CLEAN REBOOT.
  MID-RECORD WRITE FAILURE CANNOT BE MADE TRANSACTIONAL WITHOUT A FILESYSTEM
  PROTOCOL; ADD FAULT-INJECTION/TAIL-RECOVERY ACCEPTANCE.
- BACKGROUND GROUP RETURN-TO-LOGIN FIXED: NONINTERACTIVE WORKERS MAY
  INHERIT A CONTROLLING TTY BUT MUST NOT PERFORM FOREGROUND PGRP HANDOFF.
  SIMH GROUP/PIPELINE/WAIT/FOREGROUND/SUBSTITUTION REGRESSION PASSES 9/9.
  KEEP THIS UNDER TEST; MULTIPROCESS HISTFILE AND FAULT INJECTION ARE OPEN.

FULL CONFORMANCE RETEST (2026-10-11): ALL 115 DSH SIMH CASES PASS.
THE TESTKIT PROMPT MATCHER NOW REQUIRES `# ` AT THE START OF A LOGICAL
TERMINAL LINE; THE EARLIER 9 FAILURES WERE HARNESS ECHO-BOUNDARY ERRORS.
HISTFILE CORRUPT/SHORT-TAIL FAULT INJECTION AND MULTIPROCESS WRITER
VALIDATION REMAIN OPEN BEFORE PHASE ONE CAN BE DECLARED CLOSED.

HISTFILE ACCEPTANCE (2026-10-11): TWO LIVE BACKGROUND WRITERS PASSED
THE D6FS 6/6 APPEND/WAIT/READBACK/HALT PROBES. A MALFORMED S6REC
LOAD NOW WARNS WHILE RETAINING THE VALID PREFIX; NO UNSAFE UNLOCKED
TRUNCATION. NORMAL HISTORY RELOAD AFTER REBOOT PASSES. ADD A LIVE
CORRUPT-TAIL INJECTION/RECOVERY TEST AND DESIGN CROSS-PROCESS SAFE
ATOMIC HISTORY RECORD COMMIT BEFORE CLOSING PHASE ONE.

HISTFILE FAULT INJECTION (2026-10-11): LIVE TWO-BOOT TRUNCATED-S6REC
REGRESSION PASSES: VALID PREFIX RECALLED WITH CTRL-P, DAMAGE WARNING
VISIBLE, D6FS CLEAN AFTER SECOND HALT. AUTOMATIC TAIL REPAIR STILL
NEEDS CROSS-PROCESS-SAFE COMMIT/COORDINATION; DO NOT TRUNCATE ON LOAD.

## HISTFILE CRASH-ATOMIC RECORD COMMIT — DESIGN GATE (2026-10-11)

THE CURRENT `O_APPEND` GUARANTEES UNIQUE WRITE-TIME EOF RESERVATION WHILE
THE D6FS PROVIDER LOCK IS HELD. IT DOES NOT GUARANTEE CRASH-ATOMIC
MULTIWORD S6REC: THE D6FS READER COMMITS INDIVIDUAL 128-WORD BLOCKS,
AND RESIZE AND FCB SIZE PUBLICATION ARE DISTINCT OPERATIONS. A FAILED
WRITE MAY THEREFORE LEAVE A HEADER WITHOUT THE FULL PAYLOAD. SERIALIZED
PROVIDER ACCESS IS NECESSARY BUT NOT SUFFICIENT.

DESIGN REQUIREMENTS BEFORE IMPLEMENTATION:

1. A PROVIDER-LEVEL RECORD APPEND TRANSACTION MUST PUBLISH THE RECORD'S
   LOGICAL SIZE ONLY AFTER ALL DATA WORDS ARE DURABLE; ON FAILURE, THE
   PREVIOUS COMMITTED EOF MUST REMAIN THE READABLE EOF. DO NOT SIMPLY
   UPDATE THE EXISTING FCB SIZE LAST WITHOUT PROVING RECOVERY AFTER A
   PARTIAL FCB/BLOCK/ALLOCATION-MAP WRITE OR POWER LOSS.
2. PROVIDE A CRASH-RECOVERABLE COMMIT POINT (E.G. A SHADOW COW EXTENT
   WITH VERSION/CHECKSUM AND A/B COMMIT METADATA, OR A JOURNALED RECORD
   RESERVATION). TEST THE ACTUAL D6FS ON-DISK UPDATE ORDER AND BLOCK
   MAP ALLOCATION BEFORE CHOOSING THE FORMAT. NEVER CLAIM ATOMICITY
   FROM THE PROVIDER MUTEX ALONE.
3. DO NOT ADD A PER-OPEN OR PER-FILE RESIDENT BUFFER; A SINGLE TEMPORARY
   128-WORD TRANSFER WORKSPACE IS THE EXISTING BUDGET. LIMIT ANY
   RECORD-ATOMIC REQUEST TO A BOUNDED WORD COUNT (HISTFILE <= 31 WORDS).
4. MULTIPLE DSH WRITERS MUST USE THE SAME PROVIDER COMMIT OPERATION.
   DSH MUST NOT RETRY A RECORD AT A NEW EOF AFTER AMBIGUOUS COMMIT
   FAILURE (DUPLICATE RISK); DEFINE RETURN STATUS FOR COMMITTED,
   DEFINITELY UNCOMMITTED, AND OUTCOME UNKNOWN.
5. ON STARTUP, RETAIN THE VERIFIED PREFIX AND WARN ON DAMAGE. DO NOT
   TRUNCATE OR SKIP UNVERIFIED RECORDS WHILE OTHER WRITERS MAY BE LIVE.
6. DAIMOS-TESTKIT MUST INJECT FAILURE AFTER ALLOCATION, FIRST DATA
   BLOCK, EACH FCB COMMIT WRITE, AND BETWEEN SHADOW COMMIT STEPS;
   VERIFY BOOT RECOVERY, CROSS-PROCESS APPEND, SIZE, AND NO DUPLICATES.

THIS IS A PROPOSED DESIGN CONTRACT, NOT A COMPLETED IMPLEMENTATION.
THE TWO-BOOT TRUNCATED S6REC TEST CURRENTLY VERIFIES DETECTION/PREFIX
RECALL ONLY. KEEP PHASE ONE OPEN UNTIL THE ON-DISK UPDATE ORDER HAS BEEN
AUDITED AND THE ATOMIC COMMIT PATH IS TESTED UNDER FAULT INJECTION.

V2 D6FS STAGED GROWTH (2026-10-11): NORMAL REGULAR-FILE APPEND NOW
WRITES PAYLOAD BEFORE PUBLISHING NEW FCB SIZE; TESTED 444-WORD
CROSS-BLOCK GROWTH, TWO-PREOPENED DESCRIPTORS, TWO BACKGROUND WRITERS,
AND HISTFILE REBOOT. BACKING-WRITE FAULT INJECTION/POWER CUTS AND
CRASH-ATOMIC RECORD COMMIT REMAIN OPEN. DO NOT DECLARE PHASE ONE
FAILURE-PATH COMPLETE UNTIL THESE ARE RESOLVED.
