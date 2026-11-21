# K04: child EXIT after parent entered deferred teardown v1768

Status: source correctness fix, isolated PDP-6 DAS PASS.
K04 remains OPEN for real MM refusal and WAIT/PID-reuse acceptance.

## Reproduced source path

v1767 prevents a dying INIT from adopting orphan children when a
*different parent finishes*. It does not protect children which
terminate **after their own parent has already entered an irreversible
teardown**, before that parent's MM/FILE provider accepts release.

Suppose parent A has reached TRANSITION+FINALIZE+PREVM with a pinned
VM or u-area (or has entered the fatal FILE-close owner continuation).
Its descriptor, parent ID, and scheduler state remain allocated;
it cannot execute WAIT. Child B subsequently finishes its own
resources successfully. Before v1768, `proc_finish_slot` looked
only at B's nonzero saved parent slot and published B as ZOMB for A.
If A's provider refuses indefinitely, B stays an unreapable zombie
and its slot/PID cannot be reused despite B having no live resources.
The old `proc_notify_parent` also posted CHLD into A's control word
and could attempt to wake the nonreturning parent.

## Correction

The existing v1767 adoptive-INIT eligibility rule is now shared by
`proc_adopt_children_pdp6` and terminal child publication through
`proc_parent_waitable_v1768`:

- Parent state must be RUN, SLEEP, or STOP, without TRANSITION.
- Parent must retain a flagged nonzero u-area and not own the fatal
  FILE-close continuation.
- If B's original parent is eligible, publish the original ZOMB and
  CHLD as before. Otherwise, publish B under a healthy INIT (slot 1).
- If INIT is also ineligible, immediately FREE B's already released
  descriptor rather than creating a parentless or uncollectable ZOMB.

The notification helper now suppresses CHLD and WAIT wakeups for
TRANSITION and fatal FILE-close owners, in addition to its prior
zero-base/u-area guard. The helper uses scratch accumulators and
does not allocate or introduce a process-table or u-area field.

## Validation and remaining gate

The complete process assembly passed isolated native PDP-6 DAS and
Git whitespace checks. No physical guest or testkit was run.

Physical K04 acceptance requires holding A's VM/MM or FILE provider
in refusal, finishing B while A is still pending, and verifying
that B is waitable by healthy INIT or immediately reusable when
INIT is also dying. Check subsequent independent MM recovery,
ordinary parent WAIT, PID reuse, multi-owner clock wrap, and HUP.
