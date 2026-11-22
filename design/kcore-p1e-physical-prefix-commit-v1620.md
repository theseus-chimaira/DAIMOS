# P1-E v1620: real PDP-6 PURE MM commit, unpin and READY_ONCE

Date: 2026-11-16
Baseline: pushed v1619 physical DCT read findings, using the
identical preserved 48K PDP-6 TSFS/no-AUX build produced in v1618.
Conclusion: real VFS prefix load, original v1384 MM retype,
MM pin release and scheduler READY_ONCE/current-slot selection
are PHYSICALLY OBSERVED. Completed user-mode PIPE/WAIT/EXIT and
positive net/peak RAM benefit are NOT accepted.

## Source-linked boundary, no instrumentation in kernel

The original full 48K PDP-6 guest uses a real linked PURE DXR2
process image and a separately installed opt-in PURE MRES.
There is no AUXST1 backing and the writable DTC tape images are
private clones in each simulator replay.

The original linked v1612 PURE MRES map resolves these
two v1384 MM transactions:

| Function | MRES offset | Physical PC | Original instruction |
|---|---|---|---|
| mm_pure_release_prefix_v1384 | 002563 | 034612 | MOVEI 0,1 |
| mm_pure_commit_prefix_v1384 | 002565 | 034614 | MOVEI 0,2 |

The physical PURE module base is 032027 octal, established
by the real scheduler service/reclaim trampolines in v1617.
The v1620 testkit refuses a changed MRES map or altered
DXR instruction bytes before installing either breakpoint.
The debugger waits for the guest's final 020000 BRK request
marker so boot does not pay CPU breakpoint overhead.

## Physical prefix-COMMIT entry

The actual PDP-6 reaches PC 034614 (COMMIT, not RELEASE).
The source only invokes this path after the original VFS/DXR
prefix loader returned success. The original CPU arguments
at entry are:

    AC1  000000116000  physical APR base
    AC2  000000000002  process owner slot
    AC3  000000001000  reclaimed immutable prefix words
    AC4  000000010000  full APR process words

Current process slot is zero and proc_sched_cursor is
000002. The actual MM table has 017 octal live extents.
Slot 2's full physical extent at 116000..126000 is
PACKED_PROCESS with exactly one pin. Children 3, 4 and
5 retain separate unpinned PACKED suffixes of length
007000 octal words. The slot-2 descriptor is physical
word 024307, with metadata 000014000002.

This is a correct, still-unrunnable intermediate state.
A call to COMMIT alone does not prove that it executed.

## Machine-memory watchpoints prove retype and unpin

The second guest installs original PDP-6 memory-write
watchpoints on descriptor 024307 immediately after
stopping at COMMIT:

| Entry/write boundary | Actual metadata (octal) | Interpretation |
|---|---|---|
| 034614 before MM commit | 000014000002 | PACKED, pin 1 |
| 034765 after MM retype write | 000011000002 | PROCESS, pin 1 |
| 002467 after mm_unpin write | 000001000002 | PROCESS, pin 0 |

All three values come from one running real PDP-6 guest.
The original MM owner identity and 116000..126000
APR span remain intact; the original mutable suffix
does not move or require a duplicate physical image.

The testkit asserts these transitions individually
and refuses a private mutation that leaves a pin set
at the final watchpoint. It also refuses wrong slot
arguments and a missing initial pin in the standalone
entry-state physical audit.

## Physical scheduler writes and restored selection

The third 48K guest follows the same two MM descriptor
writes before installing memory-write watchpoints on
the original resident scheduler fields:

| CPU PC on write | Physical word | Value | Meaning |
|---|---|---|---|
| 032477 | proc_sched_cursor at 024235 | 001002 | READY_ONCE plus slot 2 |
| 016770 | proc_current_slot at 024233 | 000002 | slot 2 selected |

The original MM descriptor remains PROCESS, pin 0 at
both scheduling stops. This proves that the v1612
removable service published the one-shot restored
owner, and fixed KCORE selected that process after
returning from slot-0 service. The watcher stops
before all subsequent CPU context restoration;
do NOT call this full user-mode execution proof.

This is an actual complete physical chain:

    successful original VFS prefix load
      -> v1384 MM commit and PROCESS retype
      -> original MM pin release
      -> v1612 READY_ONCE publication
      -> original KCORE proc_current_slot = 2

No new process metadata, resident memory field, mutable
backing, restart workaround, or driver timeout was added.

## Resource and remaining correctness gates

The original GCC PURE MRES is 1758 decimal resident
words. Three other 010000-word child images remain
as 007000-word PACKED suffixes at the commit boundary:
3 * 01000 octal = 1536 decimal gross reclaimed
physical words. This does NOT yet offset the whole
installed PURE module (short by 222 decimal words)
before accounting for other transient allocations.
The measured snapshot is therefore not a positive
net-memory acceptance case. Full no-AUX peak and
net physical RAM under a fitting workload remain open.

The parent BRK pressure target 020000 octal also does
not fit the observed largest aligned free run 017000
at this intermediate point. Do not misclassify the
parent's possible BRK refusal as a failed restore
of slot 2.

Next: observe original user-context restore and a
successful user instruction after slot 2 selection;
run a fitting complete PIPE/WAIT/EXIT pressure test,
verify exact immutable source lifetime through
unmount/teardown, and measure the real net and peak
physical RAM on 48K/64K configurations. Only then
consider default enablement.

Test-only source, wrappers and executed fail-closed
mutations reside exclusively in daimos-testkit:
tests/diagnostics/pure-prefix-boundary-v1620.
DAIMOS production KCORE, PURE MRES, Type-551 driver,
MM format, process ABI and build flag are unchanged.
Default PURE OFF; VERSION 0.5; strict 32K HOLD.
