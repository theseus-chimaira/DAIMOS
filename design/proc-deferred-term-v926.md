# v926: Deferred fatal delivery across executive continuations

Status: design and baseline validation only; not implemented.
Date: 2026-11-01

## Verified baseline

Native PDP-6 `daimos-fatal-handoff-context-v925`,
`daimos-finish-handoff-context-v925`, and
`daimos-oom-handoff-context-v925` all PASS against current source.
These are extracted production assembly tests, not an actual full-system
CHLD-wakeup injection. Existing remote fatal delivery correctly refuses a
foreign target with WAIT_EVENT, WAIT_CHILD, WAIT_INTR, or a saved executive
PI continuation after its wait class is cleared.

## Problem

The refusal prevents a remote sender from releasing a live private executive
stack. However, TERM is not queued by this refusal. After CHLD wakeup clears
WAIT_CHILD, the saved CTX_PC still identifies an executive return until its
owner is restored. Merely retrying remote fatal cleanup can race that return.

## Required contract

1. Authenticate PID/group delivery exactly as today, before any new state.
2. Distinguish accepted-but-deferred termination from actual completion.
   Do not report success if no durable pending request was recorded.
3. A deferred request must not close FILEs, change VM ownership, release
   the u-area, or discard the saved continuation remotely.
4. Preserve at most the semantics needed for the existing fatal event set:
   INT, TERM, HUP, PIPE. Determine status precedence explicitly before
   reusing any packed pending bit. Never silently alias the ALRM/CHLD bits.
5. The target consumes its own pending fatal request at a safe return boundary
   after restoring its private continuation; do not run its FILE teardown
   inside the PI6 scheduler or with a foreign `file_table` pointer.
6. If FILE SYNC or another provider refuses cleanup, preserve the pending
   request and retain ownership through the existing private retry path.
7. Repeated sends coalesce without duplicate FILE close, HUP delivery,
   VM destroy, or finalization. ZOMB/FREE and reused PID cannot inherit a
   stale request.
8. No persistent kernel words, process-table growth, or ABI extension unless
   all existing packed fields have been audited and a demonstrated need remains.

## Test progression

- Native extracted assembly: CHILD, EVENT and INTR wait classes; saved
  executive handoff with cleared wait bits; repeated fatal send; runnable
  user-return control; genuine FILE failure and retry; exact-once cleanup.
- Booted 256K PDP-6: stall a parent in real WAIT_CHILD; arrange actual CHLD
  wakeup; inject TERM before its executive resume and verify that the parent
  completes its private continuation before teardown. Track pending request,
  target identity, independent bystander progress and ownership witnesses.
- Repeat with held FS provider lease and a blocked foreign waiter. A
  permanently nonreturning provider is an independent P0 recovery problem;
  deferred TERM cannot forcibly unlock that lease.
- Run v921 safety inventory and v918-v925 native regressions after any
  implementation. Automatic OOM killing remains disabled.

## Implementation gate

Before changing `proc_event_apply`, audit all bits in PROC_FDCTL_OFFSET,
PROC_FINALIZE_OFFSET, event pending bitmap, and scheduler state. Then identify
the self-owned safe return path where a pending fatal action can be consumed
without violating the PI6 context restore and stack lifetime invariants.
Do not substitute forced remote cleanup for this handoff.
