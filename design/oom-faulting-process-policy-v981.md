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
