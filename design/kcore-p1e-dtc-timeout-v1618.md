# P1-E v1618: Type-551 DECtape search timeout for full rewind

Date: 2026-11-16
Base: pushed v1617 physical RBN search diagnosis.
Status: timeout correction and real physical PDP-6 timer
progress PASS. Full PURE prefix restoration and net RAM
acceptance remain OPEN.

## Why the existing bound was too short

The Type-551 DECtape search and reversal code uses the
single DTC_SEARCH_TIMEOUT_TICKS countdown in tape_io.s.
The v1617 value 05214 octal is 2700 ticks at 60 Hz, or
45 seconds. A genuine rewind/reversal can take as long
as 60 seconds, so the old timer could abort a healthy
transport before it reached a usable block mark.

v1618 changes the per-search timeout to 016040 octal:
7200 line-clock ticks = **120 seconds**. This allows
a full 60-second movement plus another 60 seconds of
headroom. The PI3 RBN handler continues to rearm the
countdown on progress; it does not delay a successful
matching block or data transfer. This change increases
the bounded no-progress wait per attempt, not the
minimum latency of successful I/O. It does not add
an overall request deadline: the existing controller
can retry or restart its per-search timer on block
marks, as it did before v1618.

The Type-516 MTC rewind/spacing timeout remains
0151440 octal (15 minutes). The DECtape data watchdog
remains 0454 octal (5 seconds), and the MTC data
watchdog remains 012430 octal (90 seconds).

## Source- and image-authentic proof

The sole production source edit is to
system/kernel/drivers/tape_io.s. Compiling the v1617
and v1618 versions against the SAME fixed 48K KCORE
map gives identical linked TAPE MRES image, BSS and
relocation sizes: respectively 000607, 000017 and
000013 octal words.

Exactly two linked instructions differ, both in
their timeout immediate, not in their opcode:

| TAPE image offset | v1617 (45s) | v1618 (120s) |
|---|---|---|
| 000343, initial search | 561200772564 | 561200761740 |
| 000405, PI3 mark rearm | 561100772564 | 561100761740 |

The old executable is deliberately rejected by
the new exact-image testkit oracle. Historical
v1617 physical evidence continues to be audited
against its matching archived linked 45s DXR,
not misrepresented as new 120s physical evidence.

The complete v1618 GCC lowmem 48K TSFS root image,
with opt-in PURE, was rebuilt. Fixed KCORE DXR
remains byte-identical to the prior v1617 48K
image, and __kcore_low_end remains 024254 octal.
There is **zero** new fixed KCORE RAM, no changed
process descriptor or MM mode, and no TAPE MRES
word growth.

## New physical PDP-6 evidence

The rebuilt v1618 48K PDP-6 no-AUX TSFS guest
reached the genuine Type-551 RBN search wait.
The simulator stopped on the source-checked
TAPE MRES instruction, then executed two bounded
simulated-time intervals in the same guest.
The real native physical values were:

| Sample | Clock ticks | storage_state | storage_count | Last RBN mark |
|---|---|---|---|---|
| 0 | 015435 | 000200 | 777777761740 | 0341 |
| 1 | 015441 | 000200 | 777777761740 | 0133 |
| 2 | 015446 | 000200 | 777777761740 | 0126 |

All values above are octal. The clock increments
were 4 and 5 ticks, and the observed block-mark
word changed at both intervals. The timeout was
rearmed at its new signed -016040-tick initial
value in the original PI3 search loop. This
confirms line-clock activity and block-mark
progress; it does NOT prove that the requested
block was finally found or that the RAW PURE
prefix was committed.

The testkit now provides:

* tests/validation/daimos-dtc-search-timeout-v1618.sh:
  exact old/new compiled binary comparison, native
  timeout checks, no module growth, reject old 45s.
* tests/diagnostics/pure-brk-refusal-v1618/
  tape-timer-v1618.sh: real three-sample PDP-6
  source-linked time progression.
* run-timer-audit-v1618.sh: verify the real 120s
  physical trace and reject an unsafe old-count
  mutation in the captured memory.

Original tape-watchdog, reverse-write and first-mark
regressions are also part of the validation matrix.

## Remaining correctness gates

The observed Type-551 search still needs a
successful RBN/data transfer or bounded failure,
followed by actual VFS prefix reload commit or
rollback, release of the MM pin, scheduler
READY_ONCE, PIPE/WAIT/EXIT, immutable executable
source lifetime, and exact net/peak physical
RAM accounting. A 120-second bound is not evidence
that the complete I/O operation is successful.

DAIMOS_PURE_MRES_ENABLED remains OFF by default.
VERSION 0.5, strict 32K HOLD. No codec, alternate
machine backend or new permanent RAM allocation.
