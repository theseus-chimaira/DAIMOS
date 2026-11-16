# P1-E v1617: identify the physical DECtape restore wait

Date: 2026-11-16
Base: committed v1616, 48K PDP-6, TSFS root, no AUXST1,
genuine immutable DXR2 PURE process images.
Status: physical MRES attribution and Type-551 RBN search
entry proved. The search's later progress and completed
slot-0 restoration are NOT yet accepted.

## Correct the v1616 interrupted-PC ambiguity

v1616 captured a SIGTERM-interrupted CPU PC of 030762
but did not establish which removable module contained
that instruction. It must not be classified as PURE
restoration C code merely because it is removable.

The v1617 physical replay uses SIMH console EXPECT to
stop immediately before the 020000 BRK pressure request.
Its actual resident scheduler trampoline words read:

    proc_swap_reclaim      023554: 254000032237
    proc_swap_service_one  023555: 254000032375

The *original* opt-in PURE MRES link map places its two
exports at 000210 and 000346. Both published trampolines
therefore identify the SAME installed module base:

    032237 - 000210 = 032027
    032375 - 000346 = 032027

The stopped CPU 030762 lies BELOW this PURE module.
The original linked TAPE MRES DXR image has instruction
336000024205 at image offset 000331, and the real
interrupted CPU memory at 030762 contains exactly this
word. Thus the installed TAPE MRES base is:

    030762 - 000331 = 030431

Its instruction is SKIPN 0,storage_count; the exact fixed
KCORE map address of storage_count is 024205. Original
source system/kernel/drivers/tape_io.s places the
instruction in dtc_search_wait after the Type-551
controller-status and mark-track error tests. The tape
module and PURE module do not overlap.

The executable attribution is checked by the testkit
auditor using the real KCORE map, PURE link map,
TAPE DXR, physical trampoline examination and the
interrupted SIMH CPU trace. It rejects a modified
one-word-offset PURE export. No guessed module base,
invented symbol, or new resident state was used.

## Actual Type-551 wait boundary captured

A second physical 48K PDP-6 guest was started with
a SIMH console trigger for the last BRK request, then
the source-verified tape instruction 030762 was armed
as a CPU execution breakpoint. The guest reached it.

Original live physical controller state (octal):

    CPU PC          030762   SKIPN 0,storage_count
    storage_state   000200   DECtape RBN search
    storage_count   777777772564
    current slot    000000

The count is signed -05214 octal ticks: the EXACT
initial 45-second RBN search timeout configured in
tape_io.s for a 60 Hz clock. This shows that the
actual DECtape block search was entered and its
timeout was initialized. A single sample does NOT
show that clock ticks advance it, that PI3 RBN
interrupts arrive, or that the request terminates.
It is incorrect to call this either a finished
restoration or a confirmed controller deadlock.

At the same simulator stop the MM extent array has
four PACKED owners, no runnable PROCESS extent:

| Slot | Base..end (octal) | Words | Pins | Meaning |
|---|---|---|---|---|
| 5 | 067000..076000 | 007000 | 0 | retained suffix |
| 4 | 077000..106000 | 007000 | 0 | retained suffix |
| 3 | 107000..116000 | 007000 | 0 | retained suffix |
| 2 | 116000..126000 | 010000 | 1 | expanded PREFIX claim |

Each retained 007000-word suffix omits 01000
octal words from its original 010000-word APR
image. The owner-2 full-size pinned PACKED extent
matches the original v1384 prefix-claim transaction
while VFS I/O runs with the process TRANSITION held.
It does NOT prove that a pin was leaked. The parent
is no longer a resident PROCESS in this snapshot,
and no normal BRK return or user WAIT completion
is established. No net-memory claim is made.

## Regression and release gate

daimos-testkit contains only test diagnostic code
under tests/diagnostics/pure-brk-refusal-v1617:

* probe-mres-v1617.sh: read both installed scheduler
  service export addresses and the known interrupted
  instruction from actual physical core.
* audit-module-origin-v1617.py: cross-check the
  original KCORE/PURE maps, linked TAPE DXR and
  two independent physical simulator traces.
* tape-wait-v1617.sh: defer the original Type-551
  search-wait breakpoint until after the BRK marker,
  then inspect controller and physical MM state.
* run-audit-v1617.sh: accept the original physical
  observations and reject both an unsafe export
  mutation and an altered negative timeout word.

The original v1615 replay accepts the new opt-in
diagnostic selectors; its existing modes remain
unchanged. No production KCORE, PURE MRES, tape
driver, process table, MM descriptor or boot flag
was modified. This is a source-and-binary-backed
diagnosis, not a workaround.

Next must verify storage_clock_tick changes the
negative timer during an active tape search and
that the Type-551 either reaches RBN/data transfer
and completes the exact prefix restoration, or
returns a bounded, recoverable failure. Only
then accept slot-0 READY_ONCE, PIPE/WAIT/EXIT
and the physical peak/net RAM accounting.

DAIMOS_PURE_MRES_ENABLED remains zero by default.
VERSION 0.5; strict 32K HOLD; no compression,
KA10/KI10 runtime code or extra resident RAM.
