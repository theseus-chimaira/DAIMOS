# A04 P0-D full-guest partial run v1197 (2026-11-05)

This is a partial execution record, **not** A04/F02 acceptance.

- DAIMOS: `d3e02fdcd91bf3043a51f2ccc2c467d20bc5382e`.
- daimos-testkit: `122c61ffa81310c3fd6c038f8f7e5db2b5114e09`.
- Command: `P0D_FULL=1 sh tests/validation/daimos-p0d-release-matrix-v1096.sh`
  with `DAIMOS_REPO=$HOME/git/DAIMOS`, `PDP10_PREFIX=$HOME/cross`,
  and the installed `$PDP10_PREFIX/bin` in PATH.
- Log: `$TMPDIR/daimos-a04-full-matrix-v1197.log`, SHA256
  `5421a6fcc00e7b2e4e4229cd2d7a93a382b4b5937d8407cf9c3f073a2382271a`.

All 18 component/source-linked P0-D release stages PASS, as do the
v1083 direct allocation-free release audit and v1087 transitive
BRK/APR/EXEC release-graph audit. The first booted test, v1125,
PASSes real persistent wrong-owner MM `-4`, retained owner and
foreign FS lease, six bounded reaper retries, 80 independent
bystander sleeps and zero injected error-count failures. Its recorded
`invalid` counter is 7, and the final lease remains with the
original provider.

The next booted test, v1124 (physical TTY/HUP/VM/u-area MM_BUSY),
was interrupted during its isolated KCC build, before any simulator
result or test assertion. The full matrix was not completed. Do not
claim v1124 failure or success, nor A04/F02 closure, from this run.

To continue acceptance, rerun the complete `P0D_FULL=1` matrix to
completion on a frozen exact candidate; inspect all stage results,
full-guest build diagnostics and final owner/counter checks. Preserve
the production requester-OOM default OFF until all release gates
are signed off. No production source or policy was changed here.
