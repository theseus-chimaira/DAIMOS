# P1-E v1625: authentic sixth RUN refuses at VM creation

Date: 2026-11-16. Base: committed/pushed v1624 diagnostic,
with exact original 48K PDP-6 TSFS/no-AUX PURE six-child media.
No production KCORE, loader, MRES, MM, ABI or media change.

## Original source-linked physical witness

The v1625 daimos-testkit replay installs four breakpoints from the
actual guest `kcore.map` before restarting the original source-built
six-child guest. These correspond to the existing resident RUN
cleanup/return leaves, not test-only code:

* `proc_run_startup_failed`: `017722` octal
* `proc_run_mm_bad`: `017724` octal
* `proc_run_claimed_bad`: `017725` octal
* `proc_run_bad`: `017727` octal

The real guest emits `aaaaa`, then stops FIRST at original
`proc_run_claimed_bad` PC `017725`, before discarding the sixth
claimed process slot. Machine AC1 is `777777777776` octal (-2).
The preceding `exec_load_process` caller checks a negative result
and branches directly to this cleanup label. The original
`exec_load.s` returns exactly -2 (`EXEC_LOAD_NOMEM`) after a nonzero
`vm_space_create` result. Thus executable loading failed at initial
child VM creation; it is NOT the later child u-area allocation or
`vm_space_startup` path, and NOT the parent's sleep-poll timeout.

At the original pre-discard breakpoint, `proc_current_slot` is 1,
`proc_high_slot` is 010 octal, `proc_slots` is 014 octal,
`mm_extent_count` is 022 octal and `mm_arena_count` is 1. The
process slot array has remaining capacity; do not report the return
as slot-table exhaustion. The allocator's specific internal
rejection cause, MM free-run geometry and reclaim opportunities
are NOT yet known from this breakpoint alone.

The v1625 source-linked auditor verifies exact map labels, AC1,
RUN/EXEC loader error flow and owner counts, and rejects both a
forged -1 error result and a forged MM-stage breakpoint. The
original physical guest source is unchanged from the v1624
six-child fixture. The v1624 negative guest had already completed
with five child `w` markers; the new breakpoint replay intentionally
stops BEFORE terminal cleanup.

## Remaining P1-E work

1. Capture the live MM descriptors/arena alignment and the specific
   `vm_space_create`/MM admission refusal. Do not patch scheduler,
   loader or compiler without a proven correctness error.
2. Choose a physically feasible 48K/64K RAW pressure workload
   whose children can all initially EXEC and reach SLEEP; use the
   aligned memory budget, not the count of child `a` markers alone.
3. Verify child RESTORE, parent BRK completion, exact PIPE/WAIT/EXIT,
   retained PURE source and rollback paths with original PI6/MM.
4. Measure complete physical peak/net RAM including the 1758-word
   GCC PURE MRES; no positive reclaim is accepted yet.

P1-E remains OPEN, default PURE OFF. Strict 32K optimization HOLD.
