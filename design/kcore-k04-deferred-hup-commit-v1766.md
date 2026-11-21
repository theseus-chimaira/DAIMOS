# K04: queued HUP is not committed session teardown v1766

Status: source correctness fix, isolated PDP-6 DAS PASS. K04 OPEN.

The session leader's HUP scanner called `proc_event_apply` for each
member and accepted a zero return as successful terminal delivery.
However, the foreign fatal event path returns zero also when it
only records a pending HUP on a target that owns a saved executive
continuation (CHILD, EVENT, INTR, or an unsafe saved PC). The member
can still have open FILE entries, a live VM, and an intact u-area.
The scanner then proceeded to reset the controlling TTY before that
member had executed the pending HUP. This violated its own resource
ordering contract and could orphan the still-live session member.

v1766 verifies the target descriptor after a zero return. Terminal
FREE/ZOMB publication with no u-area is accepted. A retained u-area
is accepted only with TRANSITION and an exact non-DISCARD FINALIZE
owner, additionally requiring PREVM while VM is live. Otherwise
the session teardown reports failure and keeps the leader's TTY
and private-stack retry continuation. Subsequent scans coalesce
the queued fatal event and wait for the target to complete its
own delivery; they do not repeatedly close its FILE entries.

No new resident words, no forced VM/FILE release, and no change to
the general asynchronous event-send success contract. Native PDP-6
DAS assembly PASS; no testkit or physical guest was run. K04 OPEN
pending deferred HUP, permanent refusal, clock wrap and WAIT/PID
reuse fault-injection acceptance.
