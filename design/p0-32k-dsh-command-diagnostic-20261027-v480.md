# P0 v480: 32K DSH prompt succeeds, command execution fails

Re-ran the uncommitted testkit v478 strict 32K lowmem disk-root boot
probe against the live checkout on PDP-6 SIMH. KINIT and INIT started;
DCS accepted ROOT and printed `DSH V1`. The subsequent `MEMSTAT`
request did not produce the expected `RESIDENT` output. The simulator
transcript showed a returned shell prompt without output.

Created a separate testkit v480 diagnostic, based on v478, which sends
`ECHO V480OK` before external `DATE` and `MEMSTAT`. The first version
only matched `V480OK` as a substring and was falsely satisfied by the
terminal echo of `# ECHO V480OK`; `DATE` also had no response. Revised
v480 asserts an *exact output line* `V480OK`, excluding echoed input.
The rerun **FAILS at this first builtin**: transcript shows `DSH V1`,
`# ECHO V480OK`, then another `#` prompt, with no `V480OK` output line.

Thus ROOT authentication and DSH startup are demonstrably functional
in this image, but general shell command execution is not. Do not
attribute the previous failure specifically to MEMSTAT, or assume a
memory-allocation root cause without checking. There is a concurrent
uncommitted dynamic DSH AST allocation experiment in `dsh_exec.c`;
its interaction with strict 32K heap pressure warrants focused tests
but must not be overwritten or accepted solely from this symptom.

No DAIMOS production change in v480. The previously accepted linked
lowmem KCORE baseline is 8960, KCORE+MRES 15507, PERMANENT_LAST 15554;
the strict KCORE <8192 and full 32K command gates remain FAIL.
