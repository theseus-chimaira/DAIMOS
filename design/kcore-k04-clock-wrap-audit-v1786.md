# K04 v1786: deferred finalizer clock-wrap source audit

Date: 2026-11-21. Status: source audit only; physical PDP-6 wrap gate OPEN.

## Existing implementation

`system/kernel/proc/proc_pdp6.s` defines `PROC_TIMER_CLOCK_MASK` as
`0377777` (17 bits). The PI6 clock increments `proc_timer_clock` at
`proc_sched_pi_tick`. Both the scheduler's slot-zero admission test
(`proc_sched_tick_reaper_check_v884`) and the idle reaper due check
(`proc_idle_loop`) subtract the stored retry epoch from the current
clock and mask the difference to 17 bits. The pending flag lives in
the left half of `proc_finish_last_tick_v881`; the interval is a
separate left-half field and is capped at octal `01000` (512 ticks).
A new SIDL refusal resets the epoch and initial interval to octal
`020`; new FINALIZE owners use their armed marker to establish the
first-refusal epoch. Interval advancement precedes the scan.

For an epoch `e`, current clock `t`, and interval `d`, the comparison
is `((t - e) & 0377777) >= d`. Because `d <= 01000`, this produces
the intended first due tick across one wrap provided the reaper can
observe the interval within one complete 17-bit epoch. This source
property does NOT prove a real PI6 interrupt or scheduling gate.

## Exact boundary vectors (octal)

| Epoch | Delay | Current | Masked elapsed | Due |
| --- | --- | --- | --- | --- |
| 0377770 | 020 | 0377777 | 007 | no |
| 0377770 | 020 | 0000000 | 010 | no |
| 0377770 | 020 | 0000010 | 020 | yes |
| 0377770 | 040 | 0000030 | 040 | yes |
| 0377770 | 01000 | 0000770 | 01000 | yes |

## Physical acceptance still required

Use a disposable instrumented PDP-6 image, not the production timer
policy or the running external testkit. Arrange a real MM-pinned
post-VM or PREVM owner, seed only the qualified 17-bit timer epoch
near octal `0377770`, and record the real owner attempts and timer
at `0377777`, `0000000`, `0000007`, and `0000010` (first 020
interval). Check no early attempt, exactly one due retry, and
subsequent 040 and capped 01000 intervals. Repeat while a second
owner registers a fresh refusal across wrap; it must not inherit the
first owner's capped interval. Verify eventual real unpin, WAIT/PID
reuse, guest HALT, and read-only D6FS CLEAN fsck. Retain console,
read-only memory witnesses and exact qualified tick deltas.

A delay spanning a complete 17-bit clock cycle without slot-zero
service is outside the proof of the masked elapsed comparison and
must not be silently described as accepted. K04 remains OPEN for
this physical gate, SIDL/PREVM ordering, FILE/HUP, dying-parent WAIT,
and permanent-refusal operator recovery.
