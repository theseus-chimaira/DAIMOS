# P1-E v1650: PI6 slot-0 FULL MRES serialization

Date: 2026-11-17. **Native PDP-6 instruction test PASS and
original v1643 physical-48K trace correlation PASS.**
No new physical guest was run for v1650. No production KCORE,
VM, MM, VFS, scheduler, loader, ABI or resident RAM changes.

## Question settled

v1649 proved the original v1391 FULL callback owns both the
authoritative RAW PACKED suffix and a separately pinned complete
PROCESS APR. Would another user process get CPU time to UNMOUNT
the executable's DTFS provider *during that same callback*?

The original PURE restoration service executes on slot 0,
through the existing `proc_swap_service_one` trampoline.
The original PDP-6 PI6 scheduler permits a slot-0 switch only
if the saved interrupt PC belongs to the range
`[proc_idle_loop, proc_idle_loop_end)`. A pending slot-0
swap-service cursor bit `0400` excludes even that ordinary
idle-loop dispatch, except the timer-due path inside idle.
The physical bounded-FULL entry is outside this interval.
Original software PI6 rescheduling applies the same saved-PC
interval and pending-service exclusion.

Therefore, under this scheduler contract, **no independent
user process is allowed to execute in parallel with the
slot-0 FULL MRES continuation**. Forcing a competing user
UNMOUNT at this exact point would require changing or
bypassing the scheduler. It would not be a valid unmodified
DAIMOS concurrency experiment. PI3/PI5 device completion and
other machine interrupts are separate from user dispatch.

The original VFS unmount also checks the existing packed
executable mount owner before acquiring DTFS V2 quiescence;
v1647 separately verified new PURE RUN refusal after that
barrier was held. Neither admission check is disabled here.

## Actual native and physical evidence

The source-linked native v1650 test extracts the original
PDP-6 `proc_sched_tick_idle_check` instructions. Its test
continuation preserves the original fall-through into
`proc_sched_tick_ready` and counts the would-be selection.
Eleven native cases exercise lower/upper PC boundaries,
timer-due/non-due, pending/not-pending swap service, and
positive permitted idle scheduling. Three independent
compiled instruction mutations are executed and rejected:
omitted lower boundary, omitted upper boundary, and
bypassed slot-0 swap ownership.

The separate physical auditor reopens the **original v1643
48K accepted trace**, with its unchanged KCORE/PURE MRES
symbol maps, and correlates:

* Bounded FULL MRES loader entry **035252 octal**.
* Idle-only scheduler interval **017216..017264 octal**,
  outside the MRES load entry and identified from the
  original KCORE map/source ordering.
* Physical `proc_current_slot=0` at the real v1643
  FULL_PINNED checkpoint.
* Exact slot 3 pinned RAW **111200..120000** and separate
  pinned PROCESS **120000..130000** (octal).
* Original saved RAW after failed read, successful later
  source restoration and no child MM owner after WAIT/exit.

Both deliberately forged physical inputs (FULL running as
slot 1; missing RAW source pin) are rejected. The complete
v1643 read-failure/restore source-linked audit is rerun and
passes, including its earlier two independent falsifications.
The v1650 auditor does not claim a second physical execution.

Testkit:
`tests/validation/pure-slot0-serial-v1650/`,
branch `test-kcore-p1e-slot0-serial-v1650-20261117`,
commit `39febe6`. Reproduce both entry points using the
versioned README there.

Evidence archive:
`$TMPDIR/daimos-p1e-v1650-acceptance-20261117.tar.gz`.
SHA256:
`fb9099e47f33c4c710bd83a79a0ffabd11cd40a6d005b87dbaca43087dca681b`.
It includes native and physical-audit results, original
read-only 48K trace/map copies, both false witness logs,
and a versioned SHAR with commit/push as its final step.

## Accepted scope and remaining P1-E gates

**ACCEPT:** ordinary independent user UNMOUNT/WRITE during
the slot-0 FULL two-pin callback is excluded by original
PDP-6 scheduler semantics; this specific proposed race
is not a reachable normal execution. v1649 retains a
separate native proof of original backing owner guard.

**NOT ACCEPTED:** all possible hardware-interrupt or
external-media failures; permanently uncertain MM owner
pins; full transient 32K/48K/64K RAM/BCACHE peak; distinct
KA10 VM/APR implementation. The old generic wording
"prove independent concurrent user UNMOUNT during FULL"
must not remain an unconditional acceptance requirement,
since it contradicts the verified slot-0 scheduling
contract. P1-E remains OPEN. PURE stays OFF by default and
strict 32K KCORE optimization remains on HOLD.
