# K04: opt-in pre-VM EXIT owner diagnostics v1754

Date: 2026-11-21. Status: implementation and isolated assembler/C
compilation PASS; physical recovery acceptance NOT claimed. K04 OPEN.

## Source defect and correctness boundary

`proc_exit_current` publishes `FINALIZE|PREVM` and `TRANSITION`
before switching from the owner's private stack to slot 0. If
`vm_space_destroy` subsequently refuses, the slot and its live VM
remain quarantined and the capped finalizer retries correctly.
However, `proc_control_getteardown_v886` only recognized the
`VM_SPACE_NONE` post-VM owner or an unpublished SIDL rollback.
`GETTEARDOWN(pid)` therefore returned NONE for this *retained EXIT
owner*. That prevented an operator from distinguishing the withheld
owner from a healthy process by its public teardown diagnostic.

The v1754 extension is opt-in to avoid changing the existing v886,
v887 and v894 caller contracts:

* Legacy `GETTEARDOWN(pid)` and `GETTEARDOWN(pid|0400)` are unchanged.
* `GETTEARDOWN(pid|01000)` recognizes a marked `TRANSITION`, live
  u-area, non-NONE VM and both `FINALIZE` and `PREVM` bits.
* That response is `(shared_retry_ticks << 2)|3|01000000` (octal).
  Low stage 3 remains the pre-VM class; bit 18 distinguishes a
  committed EXIT from the existing unpublished RUN rollback, which
  still returns stage 3 without bit 18.
* `SYS_TEARDOWN_RETRY_TICKS` masks off bit 18 before decoding the
  shared interval. Combined detail/extension queries are allowed.
* `PS` uses the opt-in query and prints `PVM` (1 for a pre-VM EXIT,
  0 otherwise), while leaving its legacy detail call unchanged.

No new mutable state, owner table, automatic retry, forced free,
or process-state transition was introduced. This reports current
quarantined ownership, not the historical source of a failed VM
operation. The shared delay can describe another pending owner.

## Isolated local compilation

The production `proc_pdp6.s` was prefixed with the same zero-valued
stack-watermark `.set` directives as the kernel make rule and
assembled with `das -F -C` into an isolated `$TMPDIR` object: PASS.
The `CMD_PROGRAM_PS` translation unit compiled with installed
PDP-6 KCC, then assembled with `das -F -C`: PASS. KCC reported the
pre-existing unused `cmd_err` note for this standalone translation.
No generated build artifact was written into the checked-out tree.
No testkit was edited or invoked during the external sweep.

## Safe administrative interpretation

1. Stage FILE indicates a live private-kernel continuation and
   still-owned descriptors. Repair the refusing provider; do not
   unwind that stack, steal its lease or clear its descriptors.
2. Stage 3 with `PVM=1` indicates a deferred EXIT before VM release.
   Do not alter its VM or u-area descriptor. Repair the underlying
   backing, pin or provider condition; normal slot-0 retry owns
   completion. Stage 3 with `PVM=0` remains unpublished RUN rollback.
3. Stage MM indicates post-VM u-area release. Use the existing typed
   read-only MM detail snapshot to distinguish pins from missing,
   mistyped or wrong-owner extents. Never force-free ambiguous MM.
4. Only after the native owner releases resources may the normal
   finalizer publish zombie/WAIT or reuse a descriptor. If ownership
   cannot be proven, maintain quarantine and plan controlled offline
   recovery or restart; a direct console DEPOSIT is not an online
   recovery protocol.

## Remaining K04 verification

Physical fault injection must show pre-VM EXIT stage 3/PVM=1 during
real refused VM destruction and its disappearance after retry.
Separately verify simultaneous permanent and temporary finalizer
owners across timer wrap, WAIT/slot reuse, and read-only diagnostics
for permanently failed FILE/MM stages. These tests belong in
daimos-testkit when the ongoing sweep permits; K03 stays disabled.
