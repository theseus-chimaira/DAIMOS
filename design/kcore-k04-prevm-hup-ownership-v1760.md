# K04: prevent PREVM fatal HUP redelivery v1760

Status: source-path fix; isolated PDP-6 assembly PASS.
K04 remains OPEN for booted recovery and fault-injection acceptance.

## Reproducible control-flow defect

The remote fatal event path `proc_event_fatal_other` closes the
target's FILE entries and calls `proc_finish_slot`. If a TTY or VM
provider refuses pre-VM release, `proc_finish_slot` publishes
`FINALIZE|PREVM`, retains TRANSITION and the target descriptor,
and returns failure to `proc_event_fatal_other`.

Before v1760, the event path reported this failure to its caller
even though fatal delivery was irreversible and retry was already
owned by slot zero. In `proc_session_teardown`, this caused the
leader's HUP pass to fail. On a later retry, the session scanner
recognized only a VM_NONE FINALIZE owner; it attempted HUP again
against the same live-VM PREVM target. `proc_event_apply` rejects
TRANSITION, so the leader could remain blocked until that VM
provider eventually accepted release. This differs from the
already-supported post-VM deferred-finalizer case.

## Correction

1. A failed foreign fatal finalizer is reported as **committed** if
   the target still has a stable u-area, TRANSITION, FINALIZE and
   either VM_NONE or (for a live VM) PREVM, excluding DISCARD.
   An unmarked or uncommitted failure still propagates as failure.
2. Session HUP retries now skip an already marked EXIT PREVM target
   with a live VM, just as they already skip a marked VM_NONE
   finalizer. Unmarked TRANSITION and DISCARD+PREVM are not skipped.

The fix neither forces release nor retries VM destruction from an
owner's private stack. Slot-zero retry, original process identity,
and shared bounded backoff remain in effect. No extra RAM words,
new ABI, or size-optimization changes are introduced.

Isolated production `proc_pdp6.s` assembled with the native DAS
assembler and `git diff --check` passed. No live multi-owner HUP,
MM refusal, WAIT/PID reuse or timer-wrap acceptance has been run;
the external testkit sweep was not touched.
