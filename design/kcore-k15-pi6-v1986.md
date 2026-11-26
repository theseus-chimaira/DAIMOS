# K15 v1986: actual directory scheduler and PI6 validation

Parent 361ad933. Validation-only change; reuse the accepted v1985
source-built 64K GCC image and original INIT/COMP bytes. No production
source, resident code, BSS or process-storage change.

## Native actual service and selector

Testkit83e3ff1, make test-pure-directory-selector-v1986. Source-link
complete v1981 directory service and actual native MM/FULL/codec with
unmodified original proc_sched_resched_select/proc_sched_tick_select
and full native selector instructions from proc_pdp6.s. Actual
independent resident peer and exact separate u-area, controlled
provider/fault responses. Original-base mode3 geometry in this native
fixture; relocated source is tested physically. No simulated scheduler
predicate and no claim of actual hardware interrupt in the native test.
Each GCC/KCC and PDP6/KA10 combination 44 selections,16 service calls.
Actual selected nonresident becomes slot-zero request, clean refusal
permits peer, after cooldown real request and retry. Lost success
still publishes READY_ONCE; higher-priority ordinary peer cannot steal
first restored dispatch, next selection obeys normal ranking. Dirty
false success quarantined with original escrow; even expired cooldown
or stale ready hint cannot dispatch it. Stale READY rejected for
nonresident, STOP or TRANSITION. Six executed unsafe assembly variants
rejected per compiler: no READY, retain READY, no ready transition,
no ready APR, no ready state, no normal transition gate. Initial
fixture declared selector return type differently from existing header;
remove duplicate declaration, no production compiler issue.
Evidence pure-bitmap-service-v1981.yR19lT (KCC),
pure-bitmap-service-v1981.SzbumO (GCC), relative TMPDIR.
Historical v1650 range guard, v1677 RAW two-owner fault, v1808 reply,
default v1981 all PASS, pure-directory-pi6-regression-v1986.KVv6Xc.

## Physical observer and first scoped result

Read-only private tape replay of pure-placement-build-v1985.s17eYq/build;
no deposits or synthetic interrupts. Actual 12-child pressure, 4 bitmap
239,1 RAW3584,7 resident4096, same original guest and KCORE/MRES hashes.
Break at actual relocated FULL and bounded loader, then real
proc_sched_pi_tick (only called by actual clock service). Guard earlier
dispatch and service return. Validate original exact pinned pair,
complete MM table, stable u-area, marked backing, cursor, original
source/destination escrow, all239 bitmap words at loader and PI6.
Saved executive PC bound to installed PURE/TAPE/DTFS/TSFS using actual
trampolines and matching link artifacts. At finish require same MM
pair/slot/mode/transition and positive clock advance. Locate original
idle-service return from actual PUSHJ instruction, check committed
mode0/APR, sole unpinned target and cleared escrow/backing, retained
record/credential, READY_ONCE in both AC1 and cursor. Next original
first dispatch must choose that slot and clear ready cursor, with no
new service entry. Require same child's actual checked w and all
original WAITs, final MM and normal HALT. Independent verifier checks
actual snapshots and preserves guest output count across observer stops.

First physical evidence pure-directory-pi6-v1986.yw48Uh/physical.clean.
Actual loader/PI6/finish slot3 from061400/239 to160000/4096, exact once
pinned pair, stable owner identity and all239 bitmap words. PI6 handler
017745, saved TAPE PC032131, tick15814, atomic finish/READY tick16929.
Actual READY return020335, cursor/AC1=01003, resident publication proven.
Observer then rearmed its current return PC as a redundant guard and
SIMH immediately re-hit020335 without executing it, so first dispatch
not captured. Independent --scoped verifier passes through READY only;
7 falsified traces rejected. Reproduce SIMH behavior with isolated
v1946 probe k08-simh-break-rearm-v1946.Nqo9HS. Corrected observer does
not rearm current PC when proceeding to dispatch, still guards service
entry. This was an observer fault, not a proven kernel failure.

## Corrected complete physical acceptance

Second unchanged-image replay pure-directory-pi6-v1986.aHbEE0.
Real TAPE saved PC032131, PI6 handler017745, tick17905. Same exact
slot3 source061400/239 and target160000/4096; all239 words unchanged
across interrupt. Atomic finish/READY/dispatch/guest tick19058.
Original READY return020335, AC1/cursor01003, actual first dispatch
020043 chooses slot3, cursor consumed to3, no premature service entry
or dispatch. Actual same child emits checked data, all12 WAITs,
exact final MM and normal HALT. Independent full verifier PASS;
all ten corrupted trace controls rejected, including wrong first
dispatch, stale ready and wrong guest. Evidence verified-final-v1986.txt,
trace-controls-v1986.log; testkit96889a8. KCC repeat Makefile run
pure-bitmap-service-v1981.pWDyrI PASS with identical counts/mutants.
Healthy physical PI6/READY_ONCE subgate closed without production
source changes or RAM growth. No physical fault injection claimed.

## Scope and next physical lifecycle boundary

A compressed PIPE waiter retains a returning executive stack: foreign
TERM must not remotely destroy it. Actual proc_finish_slot checks
stack reservations, wait class, saved user PC and FILE retirement.
For successful nonresident terminal physical test, use a genuinely
user-mode CPU spinner stopped by authorized TSTP, no private wait
continuation, then real pressure and TERM from authorized process.
Require actual mode3 owner and no restoration before terminal detach;
observe original VM destruction/finalizer, exact WAIT status, final MM.
Separate actual EXEC after restored user continuation still required.
Do not weaken returning-stack guards to manufacture terminal success.
K16 dirty recovery, RT latency/stack, real DSH/48K remain open.
Default OFF.
