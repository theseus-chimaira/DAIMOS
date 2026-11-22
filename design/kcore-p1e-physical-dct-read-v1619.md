# P1-E v1619: physical DECtape reverse RBN and DCT read completion

Date: 2026-11-16
Base: pushed v1618 48K PDP-6 TSFS/no-AUX image with the
120-second Type-551 search timeout and opt-in PURE MRES.
Verdict: the real controller locates the requested block
and completes the Type-136 DCT read in the full PURE
pressure workload. VFS prefix commit/rollback, MM
pin release, slot-0 RUN/WAIT and net physical RAM
acceptance remain OPEN. No production code changed.

## Isolate the root DTC0 block-search question

The prior full guest repeatedly observed Type-551
RBN marks during a reverse search for physical
block 000004, without proving that the match
was ever recognized. It was important not to
blame the tape controller, change PI3 handling
or increase the already corrected v1618 search
timeout without a direct experiment.

The v1619 native PDP-6 read-only test uses the
actual v1618 generated TSFS root DTC0 media
copied to a private test directory. The original
DAIMOS tape_io.s and storage_router.s are linked
with the historical source-authentic PI3/PI5
interrupt wrappers. Two separate programs PASS:

* Near control: read block 000004 twice.
* Physical long reversal: read block 000341
  first, then reverse and read block 000004.

Both require zero from dtc_read_block and
nonzero content in the 0200-word read buffer.
They also check the copied TSFS root media
remain byte-identical. The tests do not
write a fabricated RBN match or bypass the
native Type-551 search/data-transfer path.

This rules out a general inability of the
original controller/driver to read physical
block 4 after a high-to-low reversal. It
does not alone prove that VFS reads the
correct prefix under full KCORE scheduling.

## Real full 48K PURE workload

To follow the actual busy scheduler and
restoration, the testkit compiles an
instrumentation-only copy of tape_io.s
that adds global source labels to the
assembler symbol table. It compares
**every linked IMAGE word** to the actual
installed v1618 TAPE MRES DXR. The images
are identical; only the unused symbol
exports differ. The production image
and process executable are untouched.

The exact original linked tape MRES base
is 030431 octal. Exported source labels
give the following physical addresses:

| Source label | Relative | Physical PC |
|---|---|---|
| dtc_search_pi_found | 000415 | 031046 |
| dtc_search_fail | 000463 | 031114 |
| dtc_search_pi_read | 000455 | 031106 |
| tape_dct_done_keep_dtc | 000124 | 030555 |

The PDP-6 is booted with a genuine PURE DXR
on a three-member TSFS root with no attached
DSK/drum backing. The console marker for the
final 020000 BRK pressure request triggers
the late installation of CPU breakpoints.
Thus boot time is not slowed by per-instruction
debugger checks.

The actual CPU stops at 031046 with:

    storage_state       000200  Type-551 search
    dtc_request_unit    000000  DTC0
    dtc_request_write   000000  read
    dtc_request_reverse 010000  reverse
    dtc_target_block    000004
    dtc_last_mark       000004

This is a genuine *matching* Type-136 RBN
interrupt, not an inferred decrement of
the tape's block-position estimate.

The same simulator then CONTinues to the
original PI3 read setup at PC 031106,
with storage_state still 0200 and
last mark still 000004.

Finally, it reaches the original
tape_dct_done_keep_dtc label at PC 030555.
At that entry the guest reports
storage_state=-1 (777777777777 octal),
storage_count=0200 and transfer event=0.
The original reverse-read DCT handler
reaches this label after finishing its
0200-word transfer; the following original
instructions publish read completion and
wake the storage waiter. This proves
entry to the actual native DCT completion
path, but the probe stops *before* that
completion event is published.

## Incorrect PI5 diagnostic corrected

The first attempt used a breakpoint at
tape_pi_done on PI5. The genuine Type-551
READ completion goes instead through
tape_dct_done_keep_dtc on the Type-136
data-channel interrupt path. The first
probe therefore saw the PI3 block match
and read setup but did not hit PI5.
The test instrumentation was corrected
after examining the original control
flow. No driver workaround was made.

The v1619 source-map auditor validates
the physical unit, request direction,
target, matched RBN and full DCT state.
It rejects two executed unsafe mutations:
an incorrect last RBN mark and a wrong
data-completion storage_state. A test
cannot pass merely because unrelated
tape activity occurs on the machine.

## Remaining acceptance gates

The v1619 result closes the uncertainty
about *finding and transferring* block 4.
It does NOT establish that the VFS layer
finishes loading the executable prefix,
that mm_pure_commit_prefix_v1384 (or
mm_pure_release_prefix_v1384) completes,
that a pinned PACKED owner becomes an
unpinned PROCESS owner, or that the
scheduler publishes READY_ONCE and
finishes the PIPE/WAIT/EXIT workload.

For the next physical experiment, the
already-linked original PURE MRES map
exports mm_pure_release_prefix_v1384 at
offset 002563 and mm_pure_commit_prefix_v1384
at 002565. At the physically verified
PURE MRES base 032027, they correspond
to PCs 034612 and 034614 octal. Use
source-mapped breakpoints at those
transaction boundaries; inspect the
original process state, MM extent type,
pin count and VFS error before accepting
any restored RUN or claiming a net gain.

No permanent KCORE, MRES, MM, process table,
device driver, backing format, or boot
selection code changed in v1619.
DAIMOS_PURE_MRES_ENABLED remains OFF
by default. Version 0.5 and the strict
32K optimization hold remain in force.
