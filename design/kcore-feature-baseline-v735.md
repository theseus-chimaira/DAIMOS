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
