# K04 v1776: live PDP-6 real MM pin/unpin acceptance (partial)

Date: 2026-11-21. Status: real MM pin/unpin and deferred release
observed, but full post-release acceptance remains OPEN.

## Disposable fixture

Copied the clean production v1773 build to the versioned
`$TMPDIR/daimos-k04-v1776-realpin` directory. Modified only the
scratch generated `kcore-asm/proc_pdp6.s`, not the tracked source.
The test intercepts `proc_uarea_release` for PID 5 only. On its
first call it invokes the production `mm_pin` for that process's
u-area, then invokes the unmodified `mm_free` with its original
base, type and owner. Subsequent calls retry the real `mm_free`;
the tenth call invokes `mm_unpin` before the normal free. Three
test-only BSS words retain base, call count and completion flag.
No permanent production kernel state or ABI is added.

Native DAS assembled the fixture successfully. The full 256K PDP-6
DSK image linked with `__kcore_low_end` at octal 025167; this
is an experimental size, not a production size change.

The first boot reached INIT/LOGIN but later halted in the root
critical-device handler at PC 004275. That trial was invalid and
its disposable disk was discarded. The entire disposable disk and
drum media were regenerated from the instrumented image, avoiding
any mismatch with copied v1773 boot media. Read-only `d6fsck -n 4`
then reported CLEAN seq=1, 179196 blocks and 360 FCBs.

## Live result

The regenerated image booted to ROOT/DSH on DCS0. Foreground TRUE
did not return while its child PID 5 was being finalized. SIMH
monitor inspection of the three fixture words showed:

| Observation | Base | Calls (octal) | Done |
| --- | --- | --- | --- |
| During refused release | 054424 | 000005 | 0 |
| Later retry | 054424 | 000007 | 0 |
| Ninth attempt | 054424 | 000011 | 0 |
| After tenth attempt | 0 | 000012 | 777777777777 |

The actual production `mm_pin`/`mm_unpin` calls, not a substituted
MM_ERR_BUSY return, were used. After the tenth attempt, the DCS
shell printed a new `#` prompt, consistent with its child wait
completing. This confirms physical MM pin accounting was exercised
and the deferred owner eventually progressed after unpin.

**Limitations:** the simulator session was terminated by the
execution environment before a final PS/PID reuse check, guest
HALT, or post-run D6FS check. The halted/terminated writable
experimental media must not be used as a clean baseline. The
fixture's own counters establish the pin/unpin sequence, but the
individual `mm_free` return values and exact backoff intervals
were not independently logged. This is therefore a **partial**
acceptance result, not a K04 closeout. Multi-owner, clock wrap,
FILE refusal, HUP and final clean-shutdown checks remain OPEN.

The external testkit was not accessed or modified.
