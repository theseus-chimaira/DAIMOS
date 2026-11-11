# KCORE foreign TERM on nonresident DISK owners, native v1498

Date: 2026-11-11. KCORE feature correctness only.

## Source and acceptance

The production `proc_event_apply` in `system/kernel/proc/proc_pdp6.s`
guards foreign fatal delivery before FILE/VM teardown. Existing
pending fatal bits, a WAIT-class private executive continuation, or
a saved executive-mode PC require deferring TERM to the owner. The
FD-control pending TERM bit is coalesced on subsequent requests. These
guards apply even when the process's user VM is nonresident DISK;
the stable u-area is still the authoritative owner of its private
executive continuation.

The new daimos-testkit v1498 (`f9ab499`) extracts the **real**
`proc_event_apply` assembly and executes it natively on the PDP-6.
A disk-backed process is tested with WAIT_CHILD, repeated TERM,
cleared WAIT but saved executive PC, cleared pending bits and saved
user-mode PC, rejected VM finalization, successful finalization, and
later rejected duplicate delivery. In the test, FILE close and the
finalizer backend are controlled witnesses, not production teardown.

All native stages PASS. A negative control bypassing the saved-PC
gate is rejected, verifying that the test does not silently accept
unsafe remote destruction of a still-live private stack. No KCORE
production code, per-process state, resident RAM or ABI was changed.

## Physical integration still open

The v1495 256K guest had two failed swap-ins and two real calls into
`proc_event_apply`, without immediate FINALIZE+PREVM. v1498 proves
that this can be legitimate, but **does not identify which source
condition the physical guest encountered**. A subsequent v1499
disposable-guest instrumentation task captures before/after process
meta/VM/scheduler, WAIT, saved PC, pending FD bits and actual return
status, with scoped branch counters. It must pass the physical
HALT and exact map-backed postflight before any classification.

Next: classify physical TERM return, prove owner-safe eventual
delivery or finalizer retry, and separately test permanently refused
backing. The strict normal v1336 zero-swap-in-failure acceptance must
remain unchanged. The corrupt-bitmap full-guest gate remains OPEN.
