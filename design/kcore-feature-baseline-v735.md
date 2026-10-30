# KCORE feature-phase size baseline v735 (2026-10-30)

Source HEAD: `eb7a407` (v734), with pre-existing working-tree changes
in KCORE.MD, PROJECT-STATE.MD and userland/dsh/dsh_exec.c left untouched.

Command: `make -C system/boot/pdp6 permanent-size BOOT=dsk ROOT=disk`.
This is a fresh linked GCC/package measurement; the report exits nonzero
because the complete optional package inventory exceeds address 040000.

| Item | Octal | Decimal words |
| --- | ---: | ---: |
| KCORE endpoint | 021030 | 8728 |
| KCORE + all included MRES packages | 041406 | 17158 |
| Single-root permanent endpoint | 041465 | 17205 |
| Multi-root permanent endpoint | 041526 | 17238 |

The report marks the single-root endpoint as 822 words past the allowed
040000 boundary (distance -821, free -822), and the multi-root endpoint
as 855 words past it (distance -854, free -855). These figures refer to
linked package footprints, not the real simultaneously installed MRES
set or runtime physical RAM high-water.

The v513 historical baseline is 8548 KCORE words: net +180 linked words.
Do not attribute this difference to any one optimization; intervening
changes also add features and bootstrap functionality. The strict KCORE
under-020000 target is deferred until the KCORE feature freeze.

## Feature and correctness gates (not size-optimization tasks)

1. Diagnose the native v669/v670 full-system scheduler/`?RT` D6FS root
   bootstrap blocker; isolated swap failure tests do not replace this gate.
2. Complete shared AUXSTORE/BACKING integration, native provisioning and
   failure acceptance; retain fail-closed VOLUME behavior.
3. Complete D6 codec integration and TSFS v1.2 sharded metadata against
   the agreed VFS/KCORE interfaces.
4. Complete the privileged raw MEMFS XIP process path and full native
   process-lifecycle acceptance before enabling it in production.
5. Complete and benchmark opt-in compressed process swap, including
   rollback and mixed compressed/uncompressed records.
6. Revalidate all KINIT/MRES direct bindings and driver watchdog failure
   paths, including removable-provider lifetime behavior.
7. Measure actual loaded-module sets and peak physical RAM independently
   of packaged linked totals, and rerun GCC/KCC native acceptance.

There is no claimed feature completion in this measurement-only record.
No code-size reduction should be attempted until these functional gates
are accepted or explicitly scoped out of the feature freeze.

The existing `daimos-testkit/tests/validation/daimos-dsk-watchdog-fix-v734.md`
records a native corrected-image countdown regression PASS: the left-half
count changes `000454` to `000453` while the right-half descriptor remains
`035024`. This does not establish full timeout or scheduler-pressure PASS.

## v736: Full-system scheduler fixture reproduction attempt

Ran the native `daimos-mm-swap-scheduler-v669.sh` runner against the current
checkout, with the pressure INIT compiled anew and an isolated `BUILD`
directory. An outer 115-second deadline expired while the full userland
image was still compiling. Make terminated and removed unfinished output;
the simulator did not start. **No scheduler or root-mount result is claimed.**
The compiler emitted repeated cross-include search-order and libc builtin
prototype warnings, but no identified fatal compiler diagnostic preceding
the timeout. A subsequent run needs a longer build allowance or a verified
prebuilt CLEAN image with the matching pressure-INIT hash; do not bypass
the fixture's hash and CLEAN-media preflight checks.

Source review confirms `d6fs_super_select` intentionally rejects DIRTY
generations for a normal mount; the explicit read-only recovery path uses
`d6fs_super_select_recovery`. Do not change that policy based solely on
the historical v670 trace. The earlier v670 failure is still unresolved.

## v737: Full image built; pressure writer fails (not accepted)

Re-ran `daimos-mm-swap-scheduler-v669.sh` from a fresh isolated BUILD with
a 540-second outer limit. The image and separate test INIT compiled, disk
creation completed, and `d6fsck` reported `state=CLEAN` for the generated
root. The PDP-6 SIMH run reached the KINIT device banner and reported
`D6FS LOADED`, so the historical pre-VFS `?RT` did not recur in this image.
The workload emitted `<wd!W` and remained without the required success
markers until the runner's own 300-second simulator timeout (exit 124).
The runner returned failure; **this is not scheduler/swap acceptance**.

The existing testkit `daimos-mm-swap-writer-diagnostic-v741.md` records
more detailed independent follow-up: the writer's WAIT sees TERM rather
than the expected normal EXIT. The source includes a TERM-on-swap-in-failure
branch, but the cause of TERM has not yet been established. Continue at
that narrower fault boundary, rather than weakening D6FS mount policy or
repeating the original coarse v283 workload without extra instrumentation.
