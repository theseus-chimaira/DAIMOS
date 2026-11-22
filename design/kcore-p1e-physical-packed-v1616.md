# P1-E v1616: physical RAW PACKED publication and restore boundary

Date: 2026-11-16
Baseline: committed v1615 48K PDP-6 TSFS no-AUXSTORE, genuine
PURE DXR2 and experimental, source-selected v1612 MRES.
Verdict: physical RAW PACKED owner publication observed;
complete BRK, slot-0 restoration, PIPE/WAIT and net RAM
benefit NOT accepted. No production kernel code changes.

## Why v1615 did not capture the failing BRK

The existing syscall return probe stored AC2 in user memory
after the BRK UUO. This cannot execute when the kernel takes
the fatal OOM path. A simulator CPU execution breakpoint at
the presumed fatal instruction was registered but never hit,
even when the original requester subsequently disappeared.
Thus the v1615 trace did not prove that specific fatal branch
executed, or that AC2 was actually MM_ERR_NOMEM.

SIMH's execution breakpoint facility was independently
validated with the linked vm_space_brk_current entry,
physical address 022104. The debugger stopped there on
the genuine initial BRK(0) query with AC1=0 and the
normal five-resident-process MM state. That established
breakpoint operation, not the failure cause.

To avoid installing a slow CPU breakpoint for the entire
boot, the v1616 replay uses SIMH console EXPECT with a
literal guest marker, stopping after the last BRK request
has been printed but before the corresponding call.
This is a test-only simulator command; no guest executable
or production KCORE modification.

## Physical state directly before the 020000 BRK

The actual no-AUX 48K simulator, halted by EXPECT on the
literal <010000+020000> guest output, has the following
MM-owned physical descriptors (all figures octal):

| Process | MM kind | Base | End | Physical words |
|---|---|---|---|---|
| Parent slot 1 | PROCESS | 050000 | 066000 | 016000 |
| Child slot 5 | PROCESS | 066000 | 076000 | 010000 |
| Child slot 4 | PROCESS | 076000 | 106000 | 010000 |
| Child slot 3 | PROCESS | 106000 | 116000 | 010000 |
| Child slot 2 | PROCESS | 116000 | 126000 | 010000 |

The sole managed arena is 042562..140000. There are
ZERO PACKED extents at this point. Physical gaps sum to
013402, and the largest APR-aligned free run is only
011577. The full-image fallback requires a second
020000-word allocation, so it cannot succeed from
this unmodified snapshot without reclaim/compaction.
This is an actual physical MM gap calculation, not the
logical process_words MEMINFO counter.

## Source-matched deferred BRK return breakpoint

The replay also verifies the unique original KCORE
PUSHJ 17,vm_space_brk_current instruction and the
following CAMN/JRST instructions in the actual saved DXR.
For this image the address immediately after the call is
020761. The test waits until the final request marker,
then installs the breakpoint and uses SIMH CONT to
resume at the existing PC. It intends to capture AC1,
AC2 and MM state before the fatal-close check.

The tested guest did NOT hit address 020761 before
the controlled simulator termination. The return value
and exact BRK failure path therefore remain UNKNOWN.
This is not a successful RUN/WAIT or a proven fatal
OOM. The stopped CPU PC was 030762, outside fixed
KCORE and in the removable-module address range.

## New physical PACKED evidence at interruption

The post-SIGTERM monitor MM dump still contained
actual process-associated allocations:

| Process | MM kind | Base | End | Words | Pins |
|---|---|---|---|---|---|
| Child 5 | PACKED | 067000 | 076000 | 007000 | 0 |
| Child 4 | PACKED | 076000 | 106000 | 010000 | 1 |

The current slot was zero and there were no PROCESS
extents. Thus a genuine RAW PACKED transition occurred
in the full physical guest, improving on the v1614
evidence of merely entering DROP and MM rebase.

The different PACKED sizes are source-consistent:
the original mm_pure_claim_prefix_v1384 expands a
pinned short PACKED suffix to the full APR image size
while a potentially slow VFS prefix reload executes.
The process intentionally remains PACKED and pinned
until the original commit or rollback finishes. The
child-4 snapshot has precisely that permitted transient
descriptor shape. An asynchronous SIGTERM snapshot
cannot prove it is a permanent leak or that restoration
eventually succeeded. The original native v1612 tests
still cover successful and refused restoration in isolation.

The unpinned child-5 retained suffix omits 01000
octal physical words compared with its original
010000-word image. This is a gross per-owner omission;
it does NOT establish total net memory saved after
the installed 1758-decimal-word GCC PURE MRES and
transient loader allocations.

## Repeatability and next acceptance gate

All new diagnostics are in daimos-testkit under
tests/diagnostics/pure-brk-refusal-v1616, with
the existing source-map-gated v1615 replay reused.
The audit-v1616.py script validates the two saved,
separately replayed physical images: real arena,
before/after owners, exact lengths, types, pins,
and interrupted status. It PASSes on the original
Termux simulator traces; these traces are runtime
diagnostics, not committed generated binaries.
The testkit audit additionally rejects a private
mutation that removes the observed child-4 MM pin,
demonstrating execution of the physical owner check.

Next, stop at a consistent restoration commit,
rollback or scheduler-READY boundary to determine
whether the pinned owner successfully resumes.
Classify BRK return/fatal branch with AC1/AC2 at
the actual instruction; do not infer the cause
from the final post-cleanup state. Verify VFS source
immutability and PIPE/WAIT/EXIT, then exact net
and peak physical RAM. No workaround, new kernel
state or speculative CPU backend was introduced.

P1-E remains disabled in default builds:
DAIMOS_PURE_MRES_ENABLED=0.
