# KCORE P1-C v1580: physical swap ownership across persistent HALT

Date: 2026-11-14. Scoped physical PDP-6 P1-C acceptance. P0-A and
operator-trust P0-B remain closed. P1-C remains OPEN for true
PI-overlapping swap/snapshot I/O and controller fault isolation.
VERSION 0.5, strict 32K/optimization HOLD.

## Physical guest result

Two separately initialized 128K KCC PDP-6 machines booted the same
production-linked fixed KCORE on DSK root with the identical retained
8 MiB Type-236 AUXSTORE backing. Actual MEMFS `/TEMP` was mounted
persistently. Boot one created and ran a strict MEMFS XIP executable
and three-word data file, and used the genuine LOGCTL syscall to
physically append/verify an LSREC1 on the separate LOGSTORE range.

The v1580 disposable *user test* then launched four independent
24K-word private-image children, verified each actual PROCINFO
SLEEP state and called root HALT before the parent exited. Its
isolated snapshot MRES adds only a test assertion at the entry of
the original snapshot shutdown: a zero fixed KCORE
`proc_swap_blocks_used` value routes to the existing refused-HALT
leaf. The resulting accepted physical HALT therefore establishes
that actual process BACKSTORE ownership was still live when the
original MEMFS snapshot writes began. No scheduler/service code or
process accounting was modified to create this witness.

An independent 128-word/sector, 36-bit physical Type-236 reader
verified the real BACKSTORE v2 committed snapshot header in its
last sector. The recorded effective MEMFS data pool is 3,648 words
and its exact reservation is 35 sectors, octal 07735..07777
relative to BACKSTORE. The header's pool, range and version match
the current assembly's real `ceil(pool/128)+5+1` allocation rule.
There are 248 nonzero physical sectors outside the snapshot run.
The descriptor and four AUXSTORE region boundaries remained intact.

Boot two recovered and executed the boot-one MEMFS XIP, recovered
the exact data file, discovered the first boot's committed LOGSTORE
record, appended/read a second LSREC1 and again had four sleeping
private-image children when the test-guarded original physical
snapshot HALT succeeded. Both simulator transcripts show actual
`HALT instruction, PC: 021777` and clean exit. After boot two,
physical snapshot and geometry were independently checked again,
with an unchanged AUXSTORE descriptor. No media/format change.

Source of physical guest and media oracles:
`daimos-testkit/tests/validation/daimos-p1c-swap-snapshot-physical-v1580.sh`,
`daimos-p1c-swapped-media-v1580.py`, and the v1580 opt-in in the
existing v1469/v1477 userland and two-boot fixtures.

## What has and has not closed

This is stronger than v1579's synthetic I/O callbacks: the process
swap allocator and physical Type-236 backing are live, and the
snapshot commits while a real process remains swapped. However,
the HALT runs synchronously; an outstanding swapped/sleeping process
does not prove its swap-in ran *during* snapshot physical DMA or
PI completion. No actual in-flight DEVICE versus mapped VOLUME
failure was injected in this run. **P1-C remains OPEN** for those
distinct physical concurrency/failure tests and exact BCACHE/MM/PI
rollback evidence. P1-D alternate hardware inventories and P1-E
uncompressed PURE remain OPEN separately.

No production KCORE/MRES TEXT, fixed DATA/BSS, process table, MM
state, ABI or persistent layout changes. The fixed KCORE low end
remains `024364` octal. No optimization/32K work is authorized.
