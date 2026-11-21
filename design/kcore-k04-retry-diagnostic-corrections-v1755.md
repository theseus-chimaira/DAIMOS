# K04: fatal EXIT diagnostic precedence and retry epoch v1755

Status: source correctness fixes; isolated PDP-6 assembly PASS.
K04 is OPEN pending physical refusal and recovery tests.

## Fatal EXIT after successful FILE close

`proc_fatal_close_current_v879` sets `PROC_FATAL_CLOSE_BIT` to
protect the private owner continuation. Once FILE/session close
succeeds, the path enters `proc_exit_current`, which publishes
`FINALIZE|PREVM` before handing off to slot zero. The fatal bit is
not cleared at this handoff. If VM destruction subsequently refuses,
the process remains an EXIT PREVM owner with a live VM, but the
v1754 opt-in `GETTEARDOWN` query checked the stale fatal bit first
and incorrectly returned stage FILE.

v1755 checks for a marked EXIT PREVM owner before the FILE bit **only
when the 01000 opt-in query flag is present**. The exact checks still
require TRANSITION, stable u-area, non-NONE VM, FINALIZE and PREVM.
The ordinary query and 0400 detail request preserve their historical
precedence; an actual fatal FILE-close retry has no PREVM marker and
continues to report FILE. The opt-in PS column now reports PVM=1
for both ordinary and fatal EXIT pre-VM refusals.

## Newly refused unpublished RUN rollback

The first `proc_slot_discard` SIDL pre-VM refusal registers its
retry owner by setting the SIDL-only descriptor marker and the
shared pending bit. Previously it set the shared delay to 020
octal ticks but copied the **previous** retry epoch, which could
be stale after earlier owner cleanup. The next idle pass could
therefore retry immediately instead of respecting the initial
delay. v1755 initializes the epoch from the current masked
`proc_timer_clock`, matching `proc_finish_mark_pending_v892`.
Already-marked SIDL retries preserve the existing shared backoff.
No new storage or forced-release behavior is introduced.

## Verification still required

Compile and assemble the updated kernel with the installed PDP-6
assembler in isolated `$TMPDIR` outputs. Once the running external
testkit sweep completes, run native fault scenarios for (1) fatal
FILE-close success followed by VM refusal, (2) new SIDL owner
after a stale shared epoch, and (3) multiple owners across clock
wrap, WAIT and descriptor reuse. Do not infer physical acceptance
from source inspection or assembly alone.
