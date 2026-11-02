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
