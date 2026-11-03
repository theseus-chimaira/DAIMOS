# Historical DSH and INIT experiments (v1050, 2026-11-03)

This archive preserves four divergent October 2026 source histories:

- `dsh-phase0-history-acceptance-20261012-v7`
- `supervisor-idle-tty-activate-20261013-v5`
- `diag-dsh-v43-entry-20261022-v52`
- `dsh-v53-acceptance-state-20261022-v55`

Their exact commits remain available as merge ancestors. Each archival
merge uses `ours` deliberately; it does not integrate or endorse the
historical source trees. They contain different process, DSH and INIT
assumptions and must not be wholesale-merged into current DAIMOS.

The current production INIT uses its own INITTAB service loop rather
than the experimental standalone SUPERVISOR image. Current DSH does
not include the historical `edcore` implementation or the old lock-
based HISTFILE repair. The proven v53 partial-pipeline child reaping
fix was recovered separately into main in v1019, with a passing native
256K regression. Historical diagnostic changes to the PI dispatcher
are KCORE-related and stay archived while KCORE work is deferred.

The corresponding historical tests are preserved by the testkit branch
`archive/legacy-dsh-supervisor-tests-v1049-20261103`. Some old tests
exercise features absent from present main and are not current
acceptance evidence. No new 32K tests or KCORE optimization were run
for this archive.

If any feature is reconsidered, inspect its original branch commit and
reconcile the APIs and memory constraints before implementing it. The
archival merge graph does not imply source-level compatibility.
