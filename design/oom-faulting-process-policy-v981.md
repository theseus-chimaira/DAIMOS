# OOM faulting-process policy (v981)

DAIMOS follows the simple OpenBSD-style choice for irrecoverable user
allocation failure: the requester is the only potential victim. Do not
scan for or terminate an unrelated process with a larger VM or swap span.

The native allocator first performs its existing normal fit, cache reclaim,
compaction and permitted swap steps. Failing a request does not itself
permit unsafe teardown. The old read-only `proc_oom_select_v863` resident
selector is now deleted because no production call site needed it and its
cross-process policy is incompatible with this decision. The independent
`proc_swap_victim` selects processes for *swapping*, never OOM termination;
it must remain available.

Actual automatic killing is still OFF. User-triggered memory failures return
normal errors until an owner-side fatal exit is proven allocation-free and
safe despite FILE sync errors, sleeping providers, session HUP fanout,
VM/backing release, TTY ownership and MM busy failure. Neither INIT nor a
privileged executive continuation may be forcibly destroyed. A failed
kernel-internal allocation must report its own error or use an explicit
safe rollback; never kill an arbitrary process to cover a kernel bug.

Pending acceptance: source-linked and booted tests for real RUN/EXEC and
live BRK exhaustion, cleanup failure, no allocation during teardown,
concurrent provider ownership and no third-party process termination.

## v982: RUN versus live BRK failure distinction

The requesting process is the only OOM candidate for a memory fault in its
own existing address space (for example, live BRK growth). A failed SYS_RUN
is different: the parent is requesting admission for an *unpublished* child.
If executable or child-u-area allocation fails, roll back the claimed child
slot and return an error to the parent. Do not kill the parent for failure to
create the child, and never publish an incomplete child. This is consistent
with a requester-local policy without broadening it into arbitrary kills.

Source-contract tests v982 check these existing failure branches. Actual
OOM killing remains disabled pending real exhausted-memory guest tests and
certification of the cleanup dependency graph.

## v984: booted BRK rejection acceptance

The booted 256K PDP-6 v984 guest grows its own BRK repeatedly until a
request is rejected. It confirms the rejected request did not change BRK,
shrinks to its starting break, and exits normally. The standard v983 BRK
growth/data/shrink regression also passes after fixing its test-only
missing memset link. This is a nonlethal failure-path and rollback proof,
not yet proof that *physical* memory rather than a user-address ceiling
caused rejection. An MM rejection witness and separate full-system RUN
resource-exhaustion acceptance remain open before activating any killer.

## v985: physically pressured RUN and BRK test

A clean isolated committed v197 RUN regression fixture, built in the
256K PDP-6 full-system simulator with the v198 physical-OOM extension,
PASSes four valid 18-bit failed BRK requests. The original break, VM
size, test data, process-slot count and rollback/recovery remain intact.
No unrelated victim is terminated. This supplies physical-pressure
acceptance independently of v984's unclassified gradual-growth ceiling.
The test does not prove allocation-free fatal termination or permit
activating OOM killing.

## v986: owner-private fatal-close gate

The native v926 owner-return fatal cleanup test passes on current assembly.
The v986 source-linked gate checks `proc_fatal_close_current_v879`:
FILE close happens on the private owner stack; a refusal sleeps and retries;
backoff saturates at 01000 octal ticks, but the number of retries is not
bounded. It cannot provide a guaranteed allocation-free OOM reclamation
contract under a permanently nonreturning or rejecting filesystem provider.
Resource ownership remains intact rather than being forcibly discarded.
OOM killing therefore remains OFF.

## v988: precise owner-exit barrier

Booted v978 mixed fatal tests pass transient FILE/MM refusal and later
cleanup (two refused closes and one u-area MM refusal). v988 source-linked
acceptance identifies the still-open pre-VM owner-exit barrier: C
`proc_exit_finish` abandons the current slot and private stack before
calling `proc_finish_slot_owner_v932`. A post-VM u-area MM refusal has
existing slot-zero deferred cleanup; a failure earlier in session HUP
or VM destruction returns -1 and `proc_exit_current` executes diagnostic
HALT 1. This path must not be connected to ordinary user OOM failure.

Resolution must preserve process ownership and normal scheduling on
recoverable pre-VM failures, without a new resident table or allocation.
No arbitrary device recovery or new OOM victim selector is authorized.
Automatic OOM termination remains disabled.

## v989: restriction on idle-stack retry

The slot-zero deferred finalizer only handles post-VM u-area release or
unpublished RUN discard. It is invoked with scheduler PI6 inhibited.
`proc_session_teardown` can deliver HUP and invoke sleeping provider
operations; `vm_space_destroy` can invoke backing/provider callbacks.
Moving either to the existing post-VM finalizer is NOT safe.

The next implementation must explicitly separate potentially sleeping
pre-VM operations from non-sleeping terminal release and preserve a valid
owner continuation until the former finishes. Do not add an idle-stack
retry that sleeps or force-release an owned private kernel stack.
This is part of the existing OOM termination P0, not a new P0 gate.

## v990: implementation constraint verified, no new P0

The v990 source-linked test verifies that ordinary EXIT closes FILE entries
on the private stack, then crosses to slot zero before HUP/VM teardown.
The latter phase can still invoke MEMFS XIP provider and swap callbacks.
Neither merely moving VM teardown earlier nor retrying it from the idle
post-VM finalizer is safe without a tested scheduler/ownership transition.
No production OOM termination is enabled by this test. The remaining work
is unchanged: implement safe owner-phase finalization and boot its OOM test.

## v991: first production owner-side EXIT stage

Ordinary EXIT now performs session HUP after successful FILE closure but
before leaving the dying process's private kernel stack. HUP/provider refusal
returns a normal -1 while the process still owns its stack and VM; the
existing finalizer rechecks HUP after the stack switch. This does not move
VM destruction or enable automatic OOM termination. The full-system BRK
runtime regression and a source contract test pass. A historical standalone
v905 HUP fixture fails to link due to its missing
`proc_tty_line_try_reset_v897` reference; it did not execute and is not
counted as validation of v991. Dedicated injected HUP refusal acceptance
is still required under the existing OOM teardown P0.

## v992: native HUP retry fixture restored

The historical v905 native HUP fixture now supplies a test-only empty-line
reset callback in the executable text section. The normal VM_NONE+FINALIZE
retry test passes; its deliberately old-behavior negative control fails
with exit 010. This verifies session retry semantics, not a booted v991
preflight refusal or VM teardown under physical OOM. Both OOM P0 items
remain unchanged.

## v993: direct owner EXIT preflight instruction test

The native PDP-6 testkit v993 runs the extracted production v991 EXIT
branch with simulated FILE and session-HUP helpers. First HUP refusal
returns -1 without switching to permanent-stack termination. A second
EXIT reaches the normal handoff. This passes; it does not yet establish
a full-system HUP failure under live processes, nor safe VM teardown or
physical-OOM requester termination. No new kernel functionality was added.

## VM release branch audit after v993

Tests v957 (production swap-detach C branches) and v880 (native PDP-6 VM
release rejection/retry) PASS against the current kernel. The disk-backed
case preserves a failed backing allocation record for retry; resident
detach does not enter that fallible backing-release branch. The remaining
problem is the control transfer following a failed `vm_space_destroy`: an
owner EXIT is already on the permanent stack, and `proc_exit_finish` returns
-1 (currently HALT 1) for pre-VM failure. This is not a request to add a
second recovery framework. A safe, existing-stack continuation must own the
retry until VM destruction succeeds, after which u-area release may use the
slot-zero finalizer. No production logic was modified in this audit.

## v995: fatal owner HUP staging

The APR/EXEC fatal owner exit now executes `proc_session_teardown` after
successfully closing FILEs but before abandoning the private kernel stack.
Recoverable HUP refusal uses the existing capped-delay, unbounded owner
retry; no extra resident word or new recovery worker is allocated. Native
PDP-6 v970 owner-return and v995 injected FILE/HUP refusal tests PASS.
This improves owner cleanup but does not resolve the potentially partial
`vm_space_destroy` failure after switching to slot zero. The automatic
OOM killer remains OFF until this boundary and physical-memory termination
are validated; the two existing P0 items are unchanged.

## v997: deferred owner VM release on slot zero

A failed ordinary/fatal EXIT `vm_space_destroy` now retains TRANSITION
and a distinct FINALIZE+PREVM marker in the existing u-area word 067.
The shared slot-zero retry clock discovers only explicitly marked owners.
The retry re-enters the already validated non-sleeping VM release subset
(MEMFS direct XIP operation, MM type/owner/pin-checked deletion, and
backing bitmap detach); it skips further session HUP. On success it clears
the PREVM marker and enters the existing post-VM u-area release sequence.
Neither a fresh resident table nor another kernel stack is allocated.

Booted 256K PDP-6 testkit v997 PASS: synthetic first VM refusal before
MM deletion, second VM attempt success, subsequent u-area MM refusal and
second-attempt success; two FILE-close refusals, stable held FS lease, CHLD
notification, bystander progress and zero integrity errors. Unmodified
v978 full-system, native v881 deferred u-area, and v869 finalization guard
also PASS. The physical-memory OOM kill trigger is not enabled or proven.

## v998: BRK failure classification gate

The native MM allocator returns distinct causes: -1 means fewer free
words than requested after reclaim; -2 is fragmentation, -3 extent table
exhaustion, -4 invalid parameters, and -5 pin/busy. The current
`vm_space_brk_current` funnels validation failures and every allocation
failure through a single -1 user-visible return. Consequently it cannot
safely trigger requester termination based solely on the BRK result.
Native testkit v998 verifies this gap and the existing reclaim chain.
The booted v985 RUN/BRK resource-regression remains PASS after v997.
Automatic OOM killing is still OFF. Extend the existing two OOM P0 gates,
not the task count: preserve internal allocation cause through BRK before
activating termination in an isolated full-system test.
