# K04: do not reparent orphans to an irreversibly exiting INIT v1767

Status: source-level correctness fix; isolated native PDP-6 DAS PASS.
K04 remains OPEN for booted fault-injection acceptance.

## Reproduction from the process-table state transitions

The old `proc_adopt_children_pdp6` treated INIT (slot 1) as eligible
whenever the low scheduler-state bits differed from FREE/ZOMB. Thus
it could select INIT while INIT already held a deferred EXIT owner:
the scheduler state was still RUN, SLEEP, or STOP, but TRANSITION
and FINALIZE prevented it from executing WAIT. Fatal APR/EXEC close
can also strand INIT on a nonreturning private stack before it has
published TRANSITION.

If another parent successfully exited while INIT was indefinitely
refused, its children (including completed zombies) were reparented
to INIT. Those zombies were neither parentless (and eligible for
immediate orphan reap) nor waitable by a live INIT. Slot and PID
reclamation could be held indefinitely by an unrelated provider
refusal, despite the other parent's successful cleanup.

## Source fix

An adoptive INIT must have a committed RUN, SLEEP, or STOP state,
no TRANSITION marker, a nonzero flagged u-area, and no
`PROC_FATAL_CLOSE_BIT` on its control word. SIDL, FREE and ZOMB
cannot be adopters. Without an eligible INIT, retain the existing
fallback: live orphans become parentless and orphan zombies are
immediately reaped, rather than bound to a dead session owner.

The related `proc_notify_parent` helper previously derived the
control-word address from any non-FREE/non-ZOMB parent's metadata.
It now requires the u-area flag and a nonzero base before posting
CHLD; a corrupt parent cannot cause a low-core write. The ordinary
live INIT adoption and parent WAIT notification paths are unchanged.

No new persistent words, extra process-table fields or user ABI.
Native PDP-6 DAS assembly and diff checks pass. No external testkit
or physical guest was run. Required K04 acceptance: hold INIT in
PREVM/post-VM MM refusal or fatal FILE close, finish another parent,
confirm independent zombie reap/no PID leakage, then verify normal
INIT orphan adoption and WAIT, recovery, and invalid-base behavior.
