# v926: Deferred fatal delivery across executive continuations

Status: first TERM-only implementation and isolated event-delivery test; full
owner-return and booted integration acceptance outstanding.
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

## First implementation and results

- Booted `daimos-provider-term-live-v926` uses a disposable 256K PDP-6
  image with a real held FS lease, foreign waiter, WAIT_CHILD parent and
  independently runnable bystander. Three deferred TERM deliveries pass,
  lease 4,,0 is unchanged, 72 bystander sleep cycles complete and no errors
  appear. The parent does NOT receive CHLD in this test; full owner-side
  continuation acceptance remains open.
- The existing event-1 bitmap position (LH 000004 in FDCTL) records deferred
  TERM after either the WAIT-class or saved executive CTX_PC guard rejects
  foreign cleanup. Recording coalesces repeated TERM requests, without
  releasing FILE, VM, or u-area ownership. Other fatal events still refuse.
- `mach_user_pdp6.s` checks pending TERM on the owner-side monitor-UUO return
  path after restoring `mach_kernel_sp`, before returning to user mode. It
  clears the bit and enters `proc_fatal_close_current_v879` with TERM status;
  that path already retries provider close on the owner's private stack.
- Native `daimos-deferred-term-v926.sh` PASS: two deferred deliveries across
  CHILD-wait and cleared-wait saved-executive handoff, coalesced bit and no
  remote FILE/VM release, followed by v925 HUP lifecycle assertions.
- Native v925 fatal, finish and OOM handoff regressions PASS unchanged.
- A full packaged `permanent-size` check was attempted but the current
  configured package exceeds the octal 040000 endpoint (`045714`); that is
  not a complete booted acceptance or an isolated size delta.

### Still unproved

- Native `daimos-deferred-owner-return-v926.sh` now executes the production
  helper and the existing private fatal-close/retry path. PASS: TERM is
  consumed once, two FILE refusals cause two sleeps, then successful close
  completes with TERM exit status. A second helper invocation does not
  repeat cleanup. The no-TERM return preserves AC2 and AC3 explicitly, with
  a native test asserting both values. This is extracted assembly, not a
  booted process.
- Actual booted CHLD wakeup-to-CTX restore TERM injection, exact-once
  completion and independent bystander progress.
- Determine and test precedence of deferred TERM against other pending fatal
  events, STOP/CONT and nonfatal event visibility. The current increment
  deliberately handles TERM only.
- Repeat full linked image and runtime acceptance with a size-valid profile.
