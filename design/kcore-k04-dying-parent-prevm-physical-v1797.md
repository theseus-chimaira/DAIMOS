# K04 v1797: dying WAIT parent and physically pinned PREVM child

Date: 2026-11-22. Status: **PASS** for physical INIT-adoption,
refused PREVM ownership, final release and safe slot retirement
on a booted 256K PDP-6 guest. Overall K04 remains OPEN.

## Acceptance scenario

The CTY ROOT/DSH shell occupies process PID 2. It starts foreground
`TRUE` PID 5, then waits in the ordinary kernel WAIT_CHILD path.
Only the child's generated **scratch** PREVM EXIT path is
instrumented with a real `mm_pin` on its exact VM extent. The
unmodified `vm_space_destroy` therefore refuses the pinned extent
on each attempt, with physical `mm_unpin` only on attempt twelve
(octal `014`). This leaves child PID 5 in transition + PREVM,
with its original parent still executing the private WAIT stack.

An independent DCS0 ROOT/DSH session issues `KILL 2` while this
child is physically refused. The original parent's fatal TERM
must unwind its WAIT_CHILD continuation, release its own FILE
and VM ownership, and become ineligible for WAIT publication.
Before the child is physically released, K04 must recognize that
PID 2 is no longer an eligible parent and adopt the remaining
child under healthy INIT (PID 1), without publishing the child's
unreleased resources as a ZOMB or freeing PID 5 prematurely.
Eventually the original child's real MM pin is released, normal
PREVM and u-area cleanup finishes, and its slot becomes FREE.

The reference source implementations are
`proc_parent_waitable_v1768`, `proc_adopt_children_pdp6`,
`proc_notify_parent` and the v1773 fatal WAIT_CHILD wake path.
This is a **real VM/MM provider refusal**, not a synthetic error
or forged process-table state.

## Reproducible scratch guest

The versioned generator, driver and read-only validator live in
`daimos-testkit`:

    tests/validation/daimos-k04-dying-parent-fixture-v1797.py
    tests/validation/daimos-k04-dying-parent-run-v1797.py
    tests/validation/daimos-k04-dying-parent-verify-v1797.py

The fixture starts from the already validated generated v1781
PREVM physical pin assembly in a **disposable** 256K boot image,
sets the physical pin release threshold to octal `014`, and
versions all scratch-only symbols to v1797. Its independent
replay produces byte-identical generated process assembly. It
checks that `proc_finish_mark_pending_v892` retains its mandatory
`popj 17,` return, and never edits production `proc_pdp6.s`.

The image is at:

    $TMPDIR/daimos-k04-v1797-dying-parent/system/boot/pdp6

Native DAS, full image construction and initial read-only
`d6fsck -n 4` PASS; initial superblock 742 sequence 1 CLEAN.
The simulator runs at reduced priority on isolated DCS/GE ports,
with CTY/DCS ROOT logins and no optional SDL window. SIMH reads
the actual scratch witness words at 025037..025041 **only after**
the guest itself executes privileged HALT. No host interrupt or
forced termination of attached writable D6FS media is accepted.

## Actual live parent/adoption observations

Before the DCS TERM, a normal process-table `PS` recorded:

    PID PPID S WORDS TD RETRY PVM DETAIL ERR COMM
    2   1    3 18432 0   0     0      0   0 USER
    5   2    2  2048 3  16     1      0   0 TRUE

Immediately after `KILL 2`, DCS observed the dying parent freed,
while the child remained PREVM and was adopted by INIT:

    PID PPID S WORDS TD RETRY PVM DETAIL ERR COMM
    5   1    2 2048 3  32     1      0   0 TRUE

The PINNED VM still belonged to PID 5 at the moment of this
adoption. This distinguishes correct parent reassignment from
an incorrect premature child release or uncollectable zombie.

## Reclamation test and an exploratory harness false negative

An initial exploratory driver incorrectly required DCS `PS`
itself to occupy reclaimed PID 5 under parent PID 3. That is
not the allocation contract: after TERM frees CTY parent PID 2,
the lowest available slot is PID 2, so a new DCS `PS` may use
PID 2 while the successfully retired child PID 5 remains FREE.

In that exploratory run, the guest actually completed its real
physical release, and post-release `PS` scans contained **no PID
5**. The guest operator HALT occurred normally; a read-only
independent verification of the real console logs, SIMH memory
and D6FS disk passed. The original driver's wrong PID5-reuse
assertion is a test-harness false negative, not a kernel failure.
The corrected driver treats an absent PID5 as reclaimed, and
also accepts an actual new PID5 with zero teardown diagnostics.

Physical post-HALT results of the exploratory run (octal):

    025037: 000000000000  ; exact VM extent released
    025040: 000000000014  ; twelve real VM destroy attempts
    025041: 777777777777  ; real mm_unpin completed

Guest `HALT` PC 022532; D6FS superblock 742, seq=3 CLEAN,
179196 blocks, 360 FCBs. The independent validator's positive
and eight negative controls passed, and it accepted the live
evidence with ordered parent, adopted, reclaimed scans.

The corrected fixture was then rerun with freshly regenerated
CLEAN media. This full acceptance run again captured the child's
PREVM stage under PID 2, its adoption by PID 1 while still pinned,
and the eventual absence of PID 5 from the process table. The
new DCS `PS` process used the newly available lower PID 2; its
teardown fields were all zero. The corrected driver verified
both actual physical VM unpin and slot retirement in the same
session, and then used a normal guest DCS `HALT`.

The accepted rerun's post-HALT SIMH evidence was likewise:

    025037: 000000000000  ; released VM base
    025040: 000000000014  ; twelve real VM destroy attempts
    025041: 777777777777  ; real MM unpin complete

Read-only D6FS selected superblock 742, seq=3 state=CLEAN,
179196 blocks and 360 FCBs. The independent verifier accepted
the corrected actual CTY/DCS/SIMH transcripts and disk, and its
positive and eight negative controls passed again. No guest
or simulator process remained running.

## Scope and remaining gates

The parent has been physically killed while waiting on a child
whose real MM owner is not yet releasable; the child remains
quarantined, receives a healthy adoptive parent and is released
only after real `mm_unpin`. This tests the critical v1768/v1773
WAIT and adoption combination without production changes.

It does **not** prove the alternative where INIT is also an
ineligible dying parent, actual multi-member session HUP with a
refused FILE/member, or additional mixed-owner 17-bit clock-wrap
behavior. Do not mark the entire K04 acceptance plan closed.
