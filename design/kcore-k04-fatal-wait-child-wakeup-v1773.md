# K04 v1773: fatal WAIT_CHILD wakeup, real PDP-6 negative control

Date: 2026-11-21. Status: **real PDP-6 before/after regression PASS**;
K04 physical MM/FILE refusal and multi-owner acceptance remain OPEN.

## Failure mechanism

`proc_event_fatal_other` deliberately queues a fatal event instead of
freeing another process that owns a private executive continuation.
Before v1773, `proc_event_fatal_other_defer_term_v926` woke only an
interruptible PIPE/FIFO waiter (`PROC_WAIT_INTR`) or the serialized
filesystem gateway (`PROC_WAIT_EVENT` at its exact wait address).
`PROC_WAIT_CHILD` was left sleeping even though its own return path,
`proc_wait_intr_return`, already detects pending INT/TERM/HUP/PIPE.

Consequently, if process A was blocked in `SYS_WAIT` for child B and
another terminal delivered TERM to A, event queuing succeeded but
A could never resume its own fatal-close continuation until B
completed. If B waited indefinitely, A's pending fatal event and
the otherwise live FILE, VM, u-area, TTY and process slot could
remain pinned indefinitely. This also obstructs deferred HUP
completion for session teardown.

There was a complementary entry race: `proc_wait_child` checked only
ALRM before sleeping, not an already-pending fatal event. A caller
with an earlier pending TERM could enter WAIT_CHILD and become stuck
without another wakeup. No resource is allowed to be released from
a foreign private executive stack.

## Correction

DAIMOS `6d99565a` extends the existing fatal-event wake classifier
to `PROC_WAIT_CHILD`, calling the same SLEEP-to-RUN / wait-class-clear
path already used for interruptible waiters. STOP remains STOP;
noninterruptible internal EVENT waiters retain their continuation;
the exact serialized FS gateway cancellation is unchanged. The
owner resumes its private stack, sees the pending fatal event at
`proc_wait_intr_return`, and processes it at its ordinary
`mach_user_pdp6.s` user-return boundary. There is no foreign FILE
close, MM release, or stack reclamation.

`proc_wait_child` now checks the full existing ALRM + INT/TERM/HUP
mask and the fatal PIPE bit before arming sleep, as already done
by `proc_wait_event_intr`. Both changes reuse existing state.
The measured linked `__kcore_low_end` moves from octal `025121`
(v1770) to `025126` (v1773): five additional TEXT words, no
new permanent DATA/BSS, no process ABI change.

## Native and physical validation

Isolated native PDP-6 DAS on the entire process module PASS.
Fresh 256K PDP-6 KCORE and complete production DSK boot-image
build PASS; initial read-only D6FS check was CLEAN seq=1.

**Positive v1773 boot:** Real Open SIMH PDP-6 loaded INIT and
accepted independent ROOT logins on CTY and DCS0. DCS `PS` showed
CTY shell USER PID 2, DCS shell USER PID 3. CTY ran foreground
`CAT`, which stayed waiting for console input, and DCS `PS`
showed sleeping USER PID 2 and sleeping CAT PID 5 (PPID 2).
DCS `KILL 2` returned success. Subsequent DCS `PS` showed PID 2
occupied by `LOGIN`, PID 5 reused by the running `PS`, and no CAT,
pending TD, RETRY or PVM owner. Guest `HALT` finished normally at
octal PC `022471`. Read-only `d6fsck -n 4` reported superblock
742, seq=3, CLEAN; 179196 blocks, 360 FCBs.

**Unmodified v1770 negative control:** Made an isolated copy of
the previously verified v1770 complete image and writable media.
Only the host image paths and DCS/GE ports were changed. Confirmed
that the copied kernel DXR has exactly the same SHA-256 as v1770
and `__kcore_low_end` is `025121`. Under the same dual-console
ROOT/DSH sequence, CTY CAT blocked beneath USER PID 2, DCS
`KILL 2` returned success, but *two subsequent PS commands* still
showed sleeping USER PID 2 and CAT PID 5 with no progress.
The DCS shell then issued guest `HALT` at PC `022464`, preserving
read-only D6FS CLEAN seq=11. The v1770 source and baseline media
were not modified for this comparison.

These observations distinguish a real guest hang in the old
implementation from the corrected fatal-event delivery in v1773.
They do not prove every wait class, pending-before-WAIT race or
provider refusal interleaving: separate injected tests remain
required for those cases. External `daimos-testkit` work was not
run or modified; all experimental media reside in versioned
`$TMPDIR` locations.
