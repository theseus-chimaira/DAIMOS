# K04 v1800: ineligible INIT and late-child terminal publication

Date: 2026-11-22. Status: source-audited acceptance design only.
**NOT YET PHYSICALLY ACCEPTED.** K04 remains OPEN.

## Source-backed invariant

The existing v1767/v1768 implementation shares
`proc_parent_waitable_v1768` between
`proc_adopt_children_pdp6` and `proc_finish_slot`.
An adoptive or WAIT-capable parent must have RUN, SLEEP or STOP
state, no TRANSITION, a flagged nonzero u-area, and no
`PROC_FATAL_CLOSE_BIT` in its FD control word. A parent on a
private fatal-FILE continuation or an MM finalizer is not
eligible merely because its process slot still exists.

`proc_adopt_children_pdp6` makes released orphan ZOMBs FREE
when INIT cannot adopt. `proc_finish_slot` independently checks
the original parent when a child completes its own teardown;
if both that parent and INIT are ineligible, it must publish
FREE directly instead of an uncollectable ZOMB. A live orphan
may remain parentless until its own terminal cleanup; that is
not equivalent to premature reclamation of a live MM owner.

## Required physical gates

### A. Late child after parent enters irreversible teardown

1. Run INIT PID1, parent A PID2 and child B PID3 in a disposable
   256K guest. A and B must have real, distinct process/VM and
   u-area owners, with an observed parent-child relationship.
2. Hold A's exact VM with real `mm_pin` during fatal PREVM EXIT,
   preserving the ordinary `vm_space_destroy` refusal and its
   pending finalizer marker. Do not forge TRANSITION or PREVM.
3. Allow B to finish real FILE/VM/u-area cleanup **after** A
   is already pending. Verify B is adopted to healthy INIT,
   publishes at most one terminal WAIT record and never becomes
   a ZOMB under A. Confirm no PID reuse while A remains pinned.
4. Release A by real `mm_unpin`, finish its own teardown and
   prove exact owner release and eventual safe PID reuse.

### B. INIT itself ineligible

1. Establish A and B as above, but first hold PID1 on its own
   private fatal-FILE continuation using a real FILE descriptor
   and an injected provider-boundary SYNC refusal. The refusal
   must preserve INIT's u-area and FILE owner; do not set
   `PROC_FATAL_CLOSE_BIT` directly. An INIT that has already
   freed its stack cannot be used to run further guest commands.
2. Obtain an **independent** control path before faulting INIT:
   a live session/guest monitor or DCS operator must be able to
   observe process state, trigger the controlled release and
   perform privileged HALT without relying on INIT's private
   stack. If that path cannot be demonstrated, stop before
   injecting the INIT fault.
3. While INIT is physically ineligible, make A ineligible and
   let B complete its real teardown. B must become FREE, never
   an uncollectable ZOMB owned by PID1 or PID2. If B is still
   live during A's adoption scan, its parent may become zero;
   that live owner must not be freed early.
4. Release INIT's real FILE refusal, allow its ordinary fatal
   continuation and controlled shutdown to complete. A
   deliberately dead INIT may prevent normal userspace
   progress; that is not permission to force-kill the emulator
   while D6FS is writable.

## Evidence and rejection criteria

Record live PS/GETTEARDOWN before/after every ownership change,
physical MM pin/unpin and provider refusal counters, exact
parent fields and scheduler states, ZOMB/FREE transitions,
and no early reuse of either held owner. The independently
verified terminal run requires an actual guest HALT, SIMH
post-HALT scratch witnesses, and read-only `d6fsck -n 4`
state=CLEAN. A crash, a test-hook-created synthetic ZOMB, a
host-forced shutdown, or a lost independent operator path is
**NOT ACCEPTED**.

Use only versioned `daimos-testkit` fixtures and disposable
source/image copies. Preserve the unrelated long-running test
suite and do not change production KCORE. The earlier v1797
dying-parent/healthy-INIT test is a baseline, not a substitute
for either of these two new orderings.

## Recommended implementation order

Build and physically accept A first. It is the smaller
extension of v1797 and directly tests the v1768 late-child
publication branch. Only then build B, which needs a safe
independent operator path before INIT may enter fatal FILE.
