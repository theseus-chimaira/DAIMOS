# KCORE v1515: confirmed critical-device-loss terminal dispatch

Date: 2026-11-12. This is **feature implementation** of a minimal
no-return fatal mechanism, not yet automatic loss detection.

## Correct classification and ownership policy

For a non-removable disk or drum, permanent medium loss is treated as
whole-device loss; there is no separate media-removal state. A *single*
bad block is not a missing device and remains under existing badmap and
I/O error handling. A negative request, controller PI2 error, or expired
PI6 watchdog is not proof of whole-device loss: physical v1511 and v1512
show two of each can recover while healthy user processes continue.

On **authoritatively confirmed** device/provider loss, a provider serving
boot root, BACKSTORE, or both is fatal. A device used solely by non-root
mounts is not fatal, even if its medium is fixed; its mount and dirty
ownership must remain fail-closed for possible device replacement and
checked recovery. A critical owner wins when one device serves both
critical and optional consumers. Device and ownership classification
are separate questions; there is no per-media recovery state.

Current Type-270 and Type-167/236 drivers report only generic negative
transfer status for PI errors/watchdog timeouts and maintain simple error
counters. Neither exposed status is an accepted authoritative permanent
loss indication. No number of repeated failures is promoted to loss.
The controller-to-volume-to-critical-owner classification/dispatch is
still an **OPEN P0 integration gate**, including physical unplug tests.

## Completed fixed KCORE entry

`system/kernel/proc/proc_storage_runtime_v30.s` now exports
`kcore_critical_device_lost_v1515`. Its accepted contract is:

- AC1=1, *confirmed boot-root device loss*: stop on root HALT instruction.
- AC1=2, *confirmed BACKSTORE device loss*: stop on swap HALT instruction.
- Any other AC1: stop on invalid-caller HALT instruction.

It disables global interrupts with the same PDP-6 PI command already
used in `proc_exit_halt`, then HALTs at one of three distinct linked
code locations (`HALT .`). The following `JRST .-1` causes an attempted
CONTINUE to repeat the same HALT. No process-context save, FILE cleanup,
storage I/O, KINIT code, logging buffer or memory allocation is involved.
The halt PC itself is a zero-data-word diagnostic: a crash observer can
look up its root/BACKSTORE reason in the KCORE map. The dangerous initial
experiment with `HALT 4/5/6`, which set the halted PC into low core,
was rejected before source acceptance.

**No active driver currently calls this leaf.** This is intentional
until reliable offline/absent classification exists. Calling it for a
generic `-1` or a 300-tick timeout would violate accepted v1511/v1512
transient error recovery. The extra routine is not a new process or
device state, and it does not change the current I/O return API.

## Exact tests and size

Testkit v1515 links the actual source into the PDP-6 simulator and
checks the three distinct reason-specific HALT program counters against
the linked labels; none may escape into low core. Each case is no-return
and does not execute normal user continuation. Three executed negative
mutants remove one HALT each; all are rejected. Six tests PASS.

The full KCC fixed KCORE build/link completes. A *controlled* old/new
link using the identical current fold plan, all identical other objects
and only removal/readdition of this leaf reports:

| Map symbol | Controlled without leaf | With leaf | Difference |
| --- | --- | --- | ---: |
| `__kcore_low_init_end` | `023075` | `023110` | +11 words |
| `__kcore_low_end` | `023512` | `023525` | +11 words |
| `mm_extents` | `023512` | `023525` | +11 words |

All addresses are octal; +11 is **decimal words**. There are **zero
new data words** and **11 extra resident instruction words**. An older
uncontrolled prior-build map gave a much larger difference because of
other source changes already present in the working tree; that number
is not attributed to v1515. The controlled link and source builds used
`$TMPDIR` for all scratch files and did not alter unrelated changes.

## Remaining

P0: identify a reliable authoritative physical device-offline/loss
signal, then route it through existing DEVICE/VOLUME and boot-root/
BACKSTORE ownership to this no-return leaf. Physically verify genuine
device removal results in the expected fatal PC while individual errors
still return through normal rollback. Do not infer permanent loss from
an error-count threshold.

P1: non-root media identity/geometry validation, BCACHE invalidation,
dirty/open owner safety, and full physical return/replacement. Also
mixed storage, PURE/MM/APR, and dynamic FSINFO remain open; compression
and size optimization are deferred until feature completion.
