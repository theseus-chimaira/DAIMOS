# Proposal B v462: native dual-path VM mapping lease validation

The native PDP-6 testkit v462 oracle executes the actual
`syscall_dispatch_pdp6.s:native_sys_two_paths` helper, extracted without
alteration, against deterministic VM mapping hold/translate/release
stubs. All three cases PASS: valid pair, failure translating the first
path, and failure translating the second path. The oracle observes
exactly one mapping hold and one release on each path; successful
translation preserves both pointer values and returns them in AC1/AC2.

The first fixture iteration returned the successful mapped address
instead of zero status from its own probe wrapper; correcting the
fixture wrapper resolved the mismatch. No production code changed.

The current integration tests now cover FILE+VFS RENAME (v461) and
the syscall's dual-pointer mapping helper (v462), but not yet all
three operations together in one native test, nor a serialized
provider wait/wakeup while the mapping hold remains live. These are
required before removing the existing syscall/FILE boundaries.
Baseline KCORE 8961; KCORE+MRES 15508; PERMANENT_LAST 15555.
Strict 32K acceptance remains deferred.
