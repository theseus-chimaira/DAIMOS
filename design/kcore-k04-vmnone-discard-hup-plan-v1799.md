# K04 v1799: VM_NONE DISCARD member versus session-leader HUP

Date: 2026-11-22. Status: source-backed physical acceptance design;
**NOT YET PHYSICALLY ACCEPTED**. K04 remains OPEN.

## Distinct state to test

`proc_session_teardown` in `system/kernel/proc/proc_pdp6.s`
must not treat `VM_SPACE_NONE` plus `FINALIZE|DISCARD` as a
committed fatal HUP. The v1762 correction explicitly excludes
DISCARD from the otherwise legitimate `VM_NONE+FINALIZE` skip.
An unpublished RUN rollback is not a delivered signal and must
not satisfy the session leader's HUP completion precondition.

This state is reached by the *actual* `proc_slot_discard` path
after a RUN child acquired a stable u-area and inherited its
session, but failed a later RUN construction step. Successful
`vm_space_destroy` sets VM to NONE; if `proc_uarea_release`
then fails its real `mm_free`, `proc_finish_mark_pending_v892`
records `FINALIZE|DISCARD`, and slot zero must retry the
discard rather than publish a ZOMB.

## Required disposable guest and injection

1. INIT PID 1 runs PID 2 as a session leader. PID 2 calls
   `NEWSESSION` and `TTY_ATTACH(0)`, then initiates a child RUN.
2. A scratch-only `proc_run.s` hook acts **after successful**
   `proc_child_hierarchy`, when child PID 3's inherited session
   has been installed, and diverts only that child to
   `proc_run_hierarchy_bad`. An earlier hook would be invalid:
   the member would not yet belong to PID 2's session.
3. A scratch-only `proc_uarea_release` hook pins PID 3's exact
   kernel-dynamic MM u-area with real `mm_pin`, then delegates
   to the original `mm_free`. VM destruction must complete first;
   the first and subsequent u-area frees must return real busy.
   Hold the pin through enough deferred retries to observe the
   HUP failure; release with real `mm_unpin` and then allow the
   ordinary provider to free the extent. Do not forge VM_NONE,
   FINALIZE, DISCARD, or scheduler state.
4. PID 2 observes the RUN failure and sleeps on a PIPE whose
   writer INIT retains. INIT verifies the PID 3 transition and
   PID 2's TTY ownership, then sends TERM to PID 2.
5. On PID 2's private fatal continuation, HUP must scan PID 3.
   Since PID 3 is VM_NONE+DISCARD, event delivery must refuse
   its TRANSITION state. The leader must preserve its session
   and TTY, remain on the private HUP retry stack, and must not
   publish terminal WAIT or reuse either PID prematurely.
6. After actual `mm_unpin`, PID 3's DISCARD must complete and
   free its unpublished slot without a ZOMB/WAIT report. PID 2
   must then complete its HUP and TTY reset. INIT must observe
   PID 2's ordinary terminal WAIT, zero stale PID 3 ownership,
   and restored process/FILE/MM occupancy.

## Acceptance evidence and isolation

Collect exact process-table and GETTEARDOWN observations,
scratch-only physical MM base/pin/unpin/attempt witnesses,
actual `proc_session_teardown` delivery/refusal/commit counts,
and TTY record ownership at each refusal. The fixture must
distinguish the actual DISCARD marker from an EXIT finalizer;
`VM_NONE` alone is insufficient. A successful result requires
the guest's own privileged HALT, SIMH post-HALT scratch memory
readout, and read-only `d6fsck -n 4` CLEAN media. Include
independent positive and negative verification controls.

Build all hooks and guest code in versioned `daimos-testkit`
fixtures and a disposable 256K PDP-6 image. Do not change
production KCORE or active external test suites. No acceptance
claim follows from this design or from source-only assembly.

## Potential failure mode to distinguish

The discarded child may be removed by slot-zero retry **before**
the leader reaches the HUP scan. That is a legitimate cleanup,
but does not exercise the v1762 distinction. The physical test
must prove the child still has its MM pin and DISCARD marker at
the moment the leader attempts HUP. A timed delay without an
observed overlap is not sufficient.
