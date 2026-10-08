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

## Fresh-image MAKE ALL regression and diagnostic narrowing

The failure is reproducible on a **new** 256K PDP-6 boot image:
`CD /OPTION/SOURCE/KCC; MAKE ALL; ECHO STATUS:$?` emits
`MAKE: BUILD B`, `!/SYSTEM/EXEC/INSTALL -D -M 0777 B`, then
`STATUS:131073` (octal `0400001`). Direct `INSTALL -D -M 0777 B` and
`MAKE B` both return zero when tested separately from fresh images.

All of the following were **test-only experiments** and were restored:

* Replaced the `proc_swap_service_failed` TERM event with HUP: no change.
* Replaced DSH cleanup TERM events with HUP: no change.
* Remapped TERM to HUP inside PDP-6 `proc_event_apply`: no change.
* Tagged a matching `0400001` ordinary EXIT syscall: no change.
* Increased `EXEC_DXR_STACK_WORDS` from octal `02000` to `04000`: no change.

Temporary MAKE markers established that the INSTALL child returns via
`dsys_wait` and the recipe status is **zero**. `make_build("B")` returns
zero, and its parent resumes at depth zero. MAKE subsequently traverses:

`ALL -> B/KCPP.DXR -> B/KCPP1.DARC -> B/CC-CPP-V1.DOBJ ->
B/CC-CPP-V1.S -> CC.C`.

It fails before KCC launches. Changes to the instrumentation shift the last
observed point in that dependency traversal. KCC-generated `make_build` uses
an octal `0121`-word frame (81 decimal words) plus saved registers, so
recursion/stack **or KCC-generated code** should be inspected next, without
assuming a particular root cause. No production source fix has been shown.

Testkit regression `test-daimos-kcc-make-all-fresh-20261008-v1` explicitly
requires `STATUS:0` and rejects `STATUS:131073`. It **correctly fails** on
current production DAIMOS. Source and simulator logs are retained under
`$HOME/tmp/daimos-kcc-make-all-fresh-20261008-v1.*`.

This investigation currently rules out the specific instrumented swap-service
TERM branch, not all other possible MM/process problems. Do not merge these
temporary instrumentation changes into main.

## Dry-run and recursion-depth isolation

Additional clean-image reproductions with the unmodified MAKE:

| Command | Observed exit |
| --- | --- |
| `MAKE -N B/CC-CPP-V1.S` | `STATUS:0` |
| `MAKE -N B/KCPP1.DARC` | `STATUS:0` |
| `MAKE -N B/KCPP.DXR` | `STATUS:0` |
| `MAKE -N ALL` | `STATUS:131073` |

The `-N` result is crucial: INSTALL, KCC, DAS, DARC, and DLINK do **not**
execute. The failing work is MAKE's own graph traversal. The extra `ALL`
frame compared with building `B/KCPP.DXR` is a concrete discriminant, but
does not distinguish stack exhaustion, invalid memory access, or incorrect
compiled recursion on its own. An earlier 1024->2048-word startup-stack
experiment did not change the fatal status. The generated `make_build` assembly
uses an octal `0121`-word local frame plus saved registers. Next: build a
synthetic depth-graded dependency graph to locate the exact boundary, then
check the corresponding generated assembly before modifying production code.
