# KCC source-build TERM / PDP-6 swap-in instrumentation (v1)

Status: diagnostic experiment, **not a confirmed kernel bug fix**.

User-observed decimal status `131073` = octal `0400001`: the event flag
`0400000` plus `SYS_EVENT_TERM` (1). Multiple paths can send TERM;
the scheduler's failed swap-in is one candidate, **not yet proven**.

Two diagnostic symbols in `system/kernel/proc/proc_swap_pdp6.s`:

* `proc_swap_diag_slot`: process slot of last attempted swap-in.
* `proc_swap_diag_reason`: last attempted swap-in result: 0=success,
  1=invalid state/metadata, 2=allocation failure, 3=pin failure,
  4=reload/backstore I/O, 5=unpin failure.

The routine still returns the same 0/-1 and follows its original cleanup,
reclamation and TERM paths. The two words are temporary resident overhead.

Current full-image map: `proc_swap_diag_slot=023036`,
`proc_swap_diag_reason=023037` (octal), from
`$HOME/tmp/daimos-swap-diag-boot-20261008-v1/kcore.map`.
Addresses may change with rebuilds. Inspect them in SIMH immediately after
reproducing the termination, before another swap operation overwrites them.

Validation passed: DAS assembly, full DAIMOS image link, D6FS check,
and daimos-testkit process lifecycle (RUN/WAIT/zombie/orphan). Real native
KCC source reproduction with diagnostics is still open. If no swap-in
failure appears, inspect DSH's process-group TERM delivery separately.
Do not merge into main until the culprit is verified and fixed.

## Target-source replay (2026-10-08)

The diagnostic image was used for a 120-second simulator-backed run of
`MAKE -C /OPTION/SOURCE/KCC B/CC-CPP-V1.S; ECHO STATUS:$?`.
The simulator reached the KCC recipe but produced no final status; the
harness failed with **timeout**. This does not reproduce the separately
reported `131073` TERM event, and the diagnostic words were therefore not
interpreted as evidence of a swap-in fault. Logs are retained at
`$HOME/tmp/daimos-kcc-swap-diag-probe-20261008-v1/`.

The diagnostic cleanup was corrected so a backing-store read failure retains
reason 4, while an actual unpin failure records reason 5. Earlier, the shared
cleanup label could overwrite reason 4 with 5. The amended source assembles
successfully with DAS. The amended build has not yet been simulator-tested.
