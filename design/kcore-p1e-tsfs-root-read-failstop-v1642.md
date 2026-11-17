# P1-E v1642: actual TSFS selected-root read loss during FULL restore

Date: 2026-11-17. Base: v1641 tested injected loader-return
rollback; v1640 proved natural FULL NOMEM recovery. v1642 tests
an ACTUAL Type-551 filesystem read outage in a full PDP-6 guest.
No production KCORE, MM, PURE MRES, TSFS, device or ABI changes.

## Why selected-root failure must not be treated as loader rollback

Source audit of `system/kernel/fs/tsfs_runtime.s`, `dtfs_runtime.s`
and `proc/proc_storage_runtime_v30.s` confirmed the deliberate
v1538 essential-root fail-stop policy. A negative physical
`dtfs_dtc_read` during TSFS file-data access reaches
`tsfs_read_words_io_error_v1538`, which calls the fixed
`kcore_root_fs_io_failure_v1538(provider=7, mount_id)`.
If the provider/mount matches the published namespace ROOT,
the fixed KCORE disables PI and HALTs at the root-specific PC.
For a secondary filesystem, the same classifier returns a
normal error instead. A selected-root outage cannot be used
to assert ordinary PURE FULL rollback/retry or child WAIT.

## Read-only original PDP-6 physical experiment

The previously accepted v1638 genuine 48K TSFS/no-AUX/PURE
guest, compiled kernel and compressed DXR2 executables were
replayed unchanged with fresh private copies of original
three-member Type-551 root media. Six sleeping children are
genuinely PACKED, the seventh RUN refuses, and the first plain
child restores and frees an APR. Simulator breakpoint PC 035252
is the original linked `exec_image_full_bounded_v1391` callback.
At this instruction the compressed slot3 owns BOTH:

| Slot | Octal physical words | Original MM type | Pins |
|---|---|---|---:|
| 3 | `111000..120000` | PACKED_PROCESS suffix | 1 |
| 3 | `120000..130000` | PROCESS full APR | 1 |

The two extents are separate, correctly owned and still pinned
BEFORE the FULL loader accesses the compressed backing data.
Testkit then physically DETACHes DTC0, DTC1 and DTC2 from the
suspended SIMH machine. Individual `show dtcN` lines confirm
that every selected root member is no longer attached.
No MM descriptor, executable DXR or kernel instruction is
injected or changed; this is an actual device-backed read loss.

The next original TSFS read failure stops at the fixed KCORE
classifier, physical **PC 003761**, with AC1=7 (TSFS) and
AC2=1 (actual public mount). The source-linked live
`vfs_namespace_root` word identifies the SAME provider/mount.
At this point the slot3 source and destination MM extents and
both pins are unchanged. The kernel then follows the original
critical-root branch to **HALT PC 003776 (`HALT 3776`)**,
exactly three original instructions beyond the linked
`kcore_critical_device_lost_v1515` entry at 003773.

No normal child EXIT/WAIT or parent `Z` occurs after the
outage, which is the CORRECT fatal result for selected ROOT.
Ordinary FULL retry and failed-read rollback are not reachable
on this path and are NOT claimed.

The testkit original source/map/physical-MM/TSFS-root/console
auditor PASSes and rejects a forged other-mount witness and an
incorrect terminal HALT address. An initial genuine run hit
the same source-linked root HALT, but the test-only script
printed only DTC2's detach evidence due to an indentation
error. The error was corrected and a fresh full replay with
individual DTC0/1/2 detach witnesses completed before the
milestone was accepted. No production code change was needed.

## Remaining gates

For recoverable FULL read-error testing, backing must be
provided by a SECONDARY non-root filesystem, with the
selected ROOT kept healthy. Still OPEN: physical non-root
source read failure and FULL transaction rollback, immutable
source modification/unmount guards, concurrent PI6/uncertain
MM failure handling, general 32K/64K low-BCACHE regressions,
and matched transient physical peak RAM. PURE remains disabled
by default; strict 32K whole-KCORE optimization stays on HOLD.
