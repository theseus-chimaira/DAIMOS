# K04 v1796: fatal private-stack FILE SYNC refusal on PDP-6

Date: 2026-11-22. Status: **PASS** for a real RUN child entering
fatal-close cleanup with a still-owned D6FS FILE descriptor, repeated
SYNC rejection, private-stack retry, GETTEARDOWN FILE diagnostics,
foreign fatal-event exclusion, eventual FILE release, session-teardown
ordering, successful WAIT/reclamation and clean guest HALT. K04
overall remains OPEN.

## Why FILE cleanup is separate from the non-sleeping MM reaper

`system/kernel/proc/syscall_dispatch_pdp6.s` implements ordinary
`SYS_EXIT` by calling `file_close_all` on the current private
kernel stack. If any provider refuses closure, EXIT returns `-1`
to the still-live caller and preserves the failed FILE descriptor.
The already accepted K02 v1724 case exercised ordinary EXIT
with two injected VFS SYNC failures and actual descriptor retention.

Fatal termination is a different owner flow. A pending INT/TERM/HUP
or PIPE event resumes on the affected process's private executive
continuation. `proc_fatal_close_current_v879` marks
`PROC_FATAL_CLOSE_BIT`, calls `file_close_all` repeatedly and sleeps
with exponential delay capped at octal 01000 ticks. FILE closes and
session teardown must succeed **before** `proc_exit_current` publishes
PREVM+FINALIZE and switches to the slot-zero stack. The non-sleeping
MM reaper has no right to close an unfinished FILE descriptor or
release the sleeping private owner stack.

## Disposable full-system fixture

Testkit artifacts (all v1796 and source versionized):

    tests/validation/daimos-k04-file-fatal-v1796.c
    tests/validation/daimos-k04-file-fatal-fixture-v1796.py
    tests/validation/daimos-k04-file-fatal-run-v1796.sh
    tests/validation/daimos-k04-file-fatal-verify-v1796.py

The isolated scratch guest is under:

    $TMPDIR/daimos-k04-v1796-file-fatal

The small C89 INIT test uses normal `SYS_RUN` to create a child PID2.
It opens actual `/SYSTEM/INIT` on the D6FS root as a regular FILE,
maps that FD to the child's FD6 and preserves a separate PIPE as FD5.
The parent drops its own D6FS FD and PIPE reader but retains the
PIPE writer, keeping the child blocked on real `READCHAR`. The test
observes the child's actual sleeping process state before sending
TERM; this ensures the queued fatal event wakes the child's own
private continuation, not a foreign direct-close path.

The only provider-error hook is in a copied
`system/kernel/fs/vfs.s`. It matches the actual PID2 FD6 vnode
through `file_table`; it returns `-5` for the first ten `vfs_sync`
calls for that exact vnode. It does not zero FD6, bypass
`file_close_entry_v865`, corrupt data or change real D6FS media.
On the eleventh matching call it delegates to the **unaltered
VFS/provider dispatch**. A separate scratch-only hook at
`proc_session_teardown` counts PID2's subsequent preflight and
verifies that the real FD6 entry is already zero and the complete
eleven-call SYNC sequence has taken place.

This is **injected VFS-boundary refusal**, not an organic D6FS
provider, disk controller or media error. It validates the
kernel's real descriptor, process, event, FILE and HUP-preflight
ownership logic against that error code, not storage fault handling.
The child inherits a session and is not itself the leader, so the
one successful session-teardown preflight does **not** prove actual
HUP delivery to additional live session members.

## Rejected first run and ordering correction

The first disposable run generated `!7` rather than the expected
FILE-stage witness. All four VFS hook counters remained zero. It
halted normally with read-only D6FS CLEAN, but **was not accepted**.
The parent had sent TERM before confirming that the child was
actually sleeping on the PIPE. This allowed a different eligible
foreign-event path to act before private-stack FILE closure.

The corrected C89 fixture polls `SYS_PROCINFO` until PID2 reports
SLEEP state 3, then sends TERM. The guest emits `S` for this real
ordering witness; `F` after a `GETTEARDOWN` FILE-stage query returns
the actual first `-5` failure code; and `P` after the competing TERM
is rejected, the parent successfully WAITS, the last PIPE writer
closes and process/FILE occupancy returns to its initial baseline.
The correctly ordered run emits **`SFP`** and no error indicator.
The initial bad transcript is preserved separately in versioned
scratch files and does not count as K04 evidence.

## Physical result

Native C89 compilation, DAS, dlink, full 256K image construction
PASS. Initial read-only `d6fsck -n 4` selected superblock 777,
sequence 1, CLEAN. The corrected guest called privileged `HALT`
normally at PC octal 022537, and SIMH exited without host-forced
termination. Post-HALT scratch-only memory witnesses (octal):

    025032: 000000000013  ; eleven matching FD6 SYNC calls
    025033: 000000000012  ; ten injected VFS -5 refusals
    025034: 000000000001  ; one accepted real VFS dispatch
    025035: 000000000000  ; no FILE fatal-bit ownership violation
    025050: 000000000001  ; one PID2 session preflight after FILE
    025051: 000000000000  ; no session/FD closure ordering violation

The child remained protected on its private owner stack during
the refused provider sequence. The parent observed an actual
`SYS_TEARDOWN_FILE` state with detailed error code `5`, and a
second TERM while the owner was closing FILEs returned `-1`.
After the eleventh SYNC accepted, the normal child finalizer
completed, parent WAIT returned the child, and process/FILE counts
matched their pre-RUN baseline. Final read-only D6FS check:

    d6fsck: selected superblock 777 seq=3 state=CLEAN
    d6fsck: 179196 blocks, 360 FCBs: clean

The independently executed read-only testkit verifier accepted
the true physical transcript, witness symbol map and clean disk,
and passed its synthetic positive and eight corrupted negative
controls. A fresh source fixture replay against the current
production checkout reproduced both copied instrumented source
files byte-for-byte. No production KCORE, ABI or fixed kernel
RAM changes were made.

## Acceptance boundary and next work

**PASS:** fatal FILE-close SYNC refusal and recovery without
owner abandonment, foreign fatal reentry, HUP-before-FILE,
or premature WAIT/reuse. **Still OPEN:** real HUP to multiple
session members, especially a VM_NONE DISCARD member and one
refused member; organic provider/media failures; dying-parent
WAIT/adoption/PID reuse; additional multi-owner clock-wrap stress.
The permanent MM refusal operator-shutdown case v1795 and both
temporary SIDL/PREVM orderings v1793/v1794 passed separately.
