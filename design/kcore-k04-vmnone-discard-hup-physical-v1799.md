# K04 v1799: physical VM_NONE DISCARD versus fatal session HUP

Date: 2026-11-22. Status: **PASS** on a real 256K PDP-6 guest,
with actual `mm_pin`/`mm_unpin`, original VM destroy and MM free,
actual session HUP retry, final guest HALT and CLEAN D6FS.
Overall K04 remains OPEN.

## The v1762 correctness distinction

The production session scanner must not treat a member with
`VM_SPACE_NONE` and `FINALIZE|DISCARD` as a committed fatal HUP.
The marker denotes an unpublished RUN rollback, not EXIT.
The v1762 code branches from the marked-owner skip to ordinary
HUP delivery when the DISCARD bit is present. `proc_event_apply`
then refuses a member already in TRANSITION. The leader must
retain its TTY, remain on its private HUP continuation and
retry after the DISCARD owner is physically released.

This case differs from the v1798 test of a published, live
session member whose FILE close is refused by VFS SYNC.

## Disposable physical guest

The C89 INIT guest PID 1 opens real PIPEs and runs PID 2.
PID 2 creates a new session and attaches CTY0, then attempts
to RUN PID 3. A scratch-only `proc_run.s` hook rejects this
RUN **after** successful `proc_child_hierarchy`, so PID 3 has
already inherited PID 2's real session. The failure enters
the production `proc_slot_discard` path. Its real VM destroy
completes, setting VM_NONE, before its stable u-area is freed.

A scratch-only `proc_uarea_release` hook pins the exact PID 3
u-area with real `mm_pin` before the original `mm_free`. It
holds the pin for twelve actual MM release attempts (octal
`014`), then calls the real `mm_unpin` and delegates to the
original MM free. No fabricated process table, scheduler,
FINALIZE or DISCARD state is used. The RUN failure occurs
only once and is never published as a process.

PID 2, having received RUN failure, waits on a real PIPE whose
writer INIT retains. INIT confirms PID 2 is sleeping and PID 3
reports deferred MM teardown, then sends fatal TERM to PID 2.
The leader must scan PID 3 and refuse HUP completion while
the child is VM_NONE+DISCARD. INIT observes the leader's
FILE-stage private HUP retry and PID 3's MM-stage deferred
owner simultaneously. Another foreign HUP to PID 3 is
rejected. After MM release, PID 3 becomes FREE without a
ZOMB, PID 2 completes HUP, relinquishes TTY and publishes
ordinary WAIT to INIT. INIT checks resource occupancy against
its original baseline and performs a privileged guest HALT.

## Reproducible fixture and acceptance evidence

Versioned testkit files:

    tests/validation/daimos-k04-vmnone-discard-fixture-v1799.py
    tests/validation/daimos-k04-vmnone-discard-v1799.c
    tests/validation/daimos-k04-vmnone-discard-run-v1799.sh
    tests/validation/daimos-k04-vmnone-discard-verify-v1799.py

Disposable image and transcripts:

    $TMPDIR/daimos-k04-v1799-vmnone-discard-physical
    $TMPDIR/daimos-k04-v1799-corrected.simh.log

The fixture generator's instrumented `proc_run.s` and
`proc_pdp6.s` reproduce byte-for-byte from clean production
sources in a separate replay tree. Both modules passed native
DAS using the actual KCORE build definitions. The C89 guest
passed cross-compiler, DAS and linker, and the full 256K
boot image was constructed from disposable copied sources.

The initial exploratory guest reached PID 3's real pinned
u-area but halted with diagnostic `!w` because the guest
had accidentally closed its PIPE reader before waiting on it.
That diagnostic halt did not perform normal D6FS shutdown;
the resulting DIRTY disk was **discarded** and not accepted.
The guest source was corrected to keep the reader open and
recompiled. The image was regenerated from fresh CLEAN media,
with post-HALT SIMH witness addresses taken from the current
authoritative `kcore.map`. The corrected run completed.

The accepted guest emitted `SHP` in order: `S` for real
sleeping leader and deferred MM child before TERM, `H` for
leader FILE-stage HUP refusal while the child remains an MM
owner, and `P` for eventual terminal publication and
successful WAIT/resource cleanup. The guest halted normally
at PC octal `022572`; SIMH exited after read-only memory
examination.

Post-HALT physical memory (octal):

    025107: 777777777777  ; post-hierarchy PID3 RUN failure injected
    025077: 000000000000  ; exact pinned u-area base now released
    025100: 000000000014  ; twelve actual MM free attempts
    025101: 777777777777  ; real mm_unpin completed
    025102: 000000000017  ; fifteen leader HUP delivery attempts
    025103: 000000000017  ; fifteen saw real VM_NONE+DISCARD
    025104: 000000000017  ; fifteen refused leader HUP passes
    025105: 000000000001  ; one committed HUP success
    025106: 000000000000  ; zero TTY/ownership ordering violations

The exact `VM_NONE+FINALIZE|DISCARD` witness is checked at
the actual session HUP delivery site. The fact that all fifteen
HUP attempts observed this state, while none was accepted as
terminal delivery, is the key v1762 acceptance evidence.

The independent read-only verifier accepted the SIMH console,
post-HALT physical memory and final D6FS media. Its positive
and eight corrupted negative controls PASS. Read-only D6FS
selected superblock 1000, sequence 3, state=CLEAN, with
179196 blocks and 360 FCBs.

## Acceptance boundary

The v1799 **physical VM_NONE DISCARD member/HUP** case is
accepted. It proves the session leader does not treat an
unpublished rollback as a committed HUP and does not retire
TTY until the real MM owner is released. It does not establish
all K04 gates: dying/ineligible INIT, organic provider errors,
additional physical pinned EXIT/HUP combinations, and
multi-owner clock wrap remain separate acceptance targets.
No production KCORE code or permanent RAM change was needed.
