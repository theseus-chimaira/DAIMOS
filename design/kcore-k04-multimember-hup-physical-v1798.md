# K04 v1798: real multi-member session HUP with refused FILE SYNC

Date: 2026-11-22. Status: **PASS** on a booted 256K Open SIMH
PDP-6 for a session leader whose HUP target holds a real D6FS
FILE while VFS SYNC is temporarily refused. The leader retains
TTY ownership during HUP refusal, the target retains its private
fatal-close stack, and cleanup succeeds with no repeat-HUP owner
violation, normal WAIT and a clean guest halt. Overall K04 OPEN.

## Why the earlier physical FILE case was insufficient

The v1796 guest proved fatal `PROC_FATAL_CLOSE_BIT` ownership and
FILE retry, but its child was not a session leader. Its single
session-teardown preflight did not actually HUP another live
member. K04 additionally requires a leader to retain its TTY
through deferred or refused HUP, and to avoid re-signalling a
member which has already irreversibly entered FINALIZE.

Production `proc_session_teardown` scans live members of the
leader's session. It accepts neither a queued event nor a
`VM_NONE + DISCARD` transition as completed fatal HUP. It may
skip only a committed, non-DISCARD FINALIZE owner. On refusal,
the leader stays on its private continuation and must retry,
without prematurely clearing its TTY record or taking over the
member's FILE/MM owner.

## Full physical guest layout

The v1798 userland test is a standalone C89/ASCII INIT image,
compiled and linked by the native PDP-10 cross-toolchain. INIT
PID 1 creates two real PIPEs and opens `/SYSTEM/INIT` as an
ordinary D6FS FILE. It runs PID 2 as the session leader,
passing a readiness PIPE writer, a member PIPE reader, and a
separately mapped D6FS regular-file descriptor FD6.

PID 2 calls the ordinary `NEWSESSION` and `TTY_ATTACH(0)`
operations, then RUNs PID 3 as a member of its newly owned
session. PID 3 inherits the real regular-file FD6 and a PIPE
reader FD5. INIT retains the last PIPE writer so PID 3 stays
in a real interruptible PIPE wait; PID 2 sends its readiness
marker and enters real `WAIT_CHILD`. INIT confirms both PID 2
and PID 3 are SLEEP before sending fatal TERM to PID 2.

During PID 2's session teardown, HUP is delivered to PID 3.
PID 3's fatal continuation must close its own FILE FD6, but the
test fixture refuses its first ten matching `vfs_sync` calls.
PID 2's private HUP continuation must not regard queued or
refused HUP as completion. Both fatal owners must remain intact
until PID 3's eleventh SYNC succeeds. After the member finishes,
the session leader retries and finally retires the TTY. INIT
observes the completed HUP and both ZOMB/WAIT retirements and
checks that process/FILE occupancy returns to baseline.

Only the *copied scratch* VFS boundary returns the synthetic
`-5` result. It checks PID 3's exact FD6 vnode in the real
process FILE table, and uses unmodified provider dispatch on
the eleventh matching call. Production FILE/MM/VFS behavior,
TTY encoding, and scheduler policy are unchanged. This is not
organic D6FS media corruption, nor a synthetic process-table
or scheduler state.

## Scratch-only instrumentation and reproduction

Versioned testkit artifacts:

    tests/validation/daimos-k04-multimember-hup-v1798.c
    tests/validation/daimos-k04-multimember-hup-fixture-v1798.py
    tests/validation/daimos-k04-multimember-hup-run-v1798.sh
    tests/validation/daimos-k04-multimember-hup-verify-v1798.py

Disposable image and exact symbol map:

    $TMPDIR/daimos-k04-v1798-multimember-hup
    build/system/boot/pdp6/kcore.map

The fixture derives VFS refusal instrumentation from accepted
v1796 and adds independent HUP/TTY session witnesses to the
copied `proc_pdp6.s`. These counters are **scratch-only BSS**.
HUP calls and failures count only the real PID 2 leader.
Failures verify that the CTY0 record still names the original
session; success checks that it has been reset. Delivery attempts
and attempts against a member already holding its fatal FILE
stack are independently counted. The generated VFS and process
source files were independently replayed byte-for-byte from
clean production plus the recorded v1796 fixture source.

Native C89 compile, DAS, link and full 256K image build PASS.
Initial read-only `d6fsck -n 4` selected superblock 777 seq=1
CLEAN (179196 blocks, 360 FCBs). The guest ran at reduced host
priority with SDL WCNSLS disabled. Only **after** the guest's
privileged HALT did SIMH examine ten map-derived witness words.

## Two rejected scratch-harness starts

The initial booted exploratory image completed the actual guest
scenario (`SHP`) and final CLEAN D6FS, but an independent verifier
rejected its witness `k04_hup_bad=11`. Source review identified
an error solely in the disposable HUP-failure hook: it compared
the entire TTY record right half with session PID 2, although
that half contains **both** the foreground process group and
session ID. The proper comparison masks low eight bits (`0377`).
The kernel's own session-control path already does so. The
versioned fixture generator was corrected and independently
replayed. No production defect or patch was implicated.

One intermediate launch tried to reuse the previous map addresses
after the changed generated image shifted scratch symbols by one
word. Its address-consistency assertion caught the difference,
but that exploratory shell invocation continued without the
headless boot edits; optional SDL initialization then failed on
the host before a complete guest test. The guest media remained
CLEAN seq=1. This host crash was not accepted as a K04 result.

The final launch used strict shell error handling, took **all**
witness addresses from the new authoritative `kcore.map`, verified
headless configuration and CLEAN initial media, and ran the
corrected real guest to an ordinary privileged HALT.

## Accepted physical evidence

The guest emitted `SHP` in the expected order:

* `S`: PID 2 waits on PID 3 and PID 3 sleeps on its PIPE, as
  verified by real `SYS_PROCINFO` before PID 2 receives TERM.
* `H`: both leader and member are still fatal FILE owners; the
  member reports a real FILE-stage diagnostic with error 5, and
  another foreign HUP cannot reclaim the member's private stack.
* `P`: both have completed terminal publication and WAIT,
  descriptor counts and process occupancy have returned to the
  original baseline, and the real TTY has been relinquished.

Accepted post-HALT physical memory (octal):

    025047: 000000000013  ; eleven matching member FD6 SYNC calls
    025050: 000000000012  ; ten -5 provider-boundary refusals
    025051: 000000000001  ; one real VFS/provider dispatch
    025052: 000000000000  ; no fatal owner flag mismatch
    025065: 000000000014  ; twelve leader HUP preflight calls
    025066: 000000000013  ; eleven refused HUP passes
    025067: 000000000001  ; exactly one committed HUP success
    025070: 000000000000  ; zero TTY ownership/order violations
    025071: 000000000013  ; eleven real HUP delivery attempts
    025072: 000000000012  ; ten protected fatal-FILE owner encounters

These counters show that the session leader kept its TTY over
each refused HUP attempt while PID 3 retained the FILE owner,
that the foreign HUP path did not forcibly close the protected
member's descriptor, and that the leader committed TTY release
exactly once after its member completed physical cleanup.

The guest halted normally at PC octal `022554` and SIMH exited.
Final read-only D6FS check: superblock 777 seq=3 CLEAN,
179196 blocks, 360 FCBs. The independent read-only verifier
accepted the real source-linked physical transcript and disk,
and its positive plus ten corrupted negative controls passed.

## Acceptance boundary

This **closes the booted multi-member HUP + temporary fatal
FILE refusal case**, including an actual TTY owner and member
running on the full PDP-6 simulator. Still OPEN: the separate
`VM_NONE + DISCARD` HUP member case, HUP interaction with a
physically pinned post-VM/PREVM owner, dying/ineligible INIT,
organic provider/media errors, and further mixed-owner clock
wrap. Earlier v1797 independently passed dying-parent physical
PREVM adoption and safe slot retirement. Do not declare K04
overall complete.
