# K04 teardown recovery: source audit v1752

Status: investigation only; K04 remains OPEN. No production changes.

## Existing recovery paths

* `proc_fatal_close_current_v879` (`system/kernel/proc/proc_pdp6.s`)
  retains its private executive stack and FILE entries when a close or
  session teardown refuses. It retries after sleeps doubling from one
  tick to 01000 octal ticks. `PROC_FATAL_CLOSE_BIT` prevents another
  fatal event from destroying the suspended owner stack.
* `proc_exit_current` publishes PREVM+FINALIZE before abandoning the
  private stack. The slot-0 finalizer must not repeat session HUP.
* `proc_idle_loop` gates deferred finalization with a shared epoch and
  capped retry delay. The finalizer owns the decision whether work is
  still pending. Disk PI3/PI5 remain enabled during this service.
* `file_close_all_at_v865` preserves failed descriptors for retry and
  records the first provider error in the fatal owner's u-area.

## Outstanding correctness and policy questions

1. Permanent provider refusal is deliberately **not** bounded
   reclamation. The owner remains quarantined; automatic forced
   release would violate K01 and K02. Specify an operator-visible,
   owner-preserving administrative recovery protocol before K04 closes.
2. Distinguish private-stack provider-close retries from slot-0
   post-VM finalization. They have different retry state, ownership,
   and permissible operations; a shared retry policy must not imply
   interchangeable cleanup contexts.
3. Verify that multiple deferred owners cannot starve one another
   under the single shared slot-0 retry epoch, including wraparound,
   repeated -4/-5 refusals and a successful independent release.
4. Verify zombie and WAIT publication only after irreversible resource
   release; a retained owner must not become waitable or reusable.
5. Preserve exact first-failure diagnostics across repeated retries,
   subsequent successful releases and later independent failures.

## Next implementation gate

Audit the source paths for `proc_finish_service_v881`, zombie/WAIT
publication and finalizer descriptor release, then design the smallest
correctness fix for any demonstrated ownership or progress defect.
Do not enable K03 OOM killing. Run source-level or physical validation
only after the ongoing external testkit sweep finishes; do not edit
`pdp10-testkit` during that sweep. No size optimization is in scope.
