# P1-E v1628: exact physical MM owners in accepted 64K lifecycle

Date: 2026-11-16. Base: v1627 complete six-child native PDP-6
PIPE/WAIT/EXIT guest. Replayed the EXACT source-built 64K executable
and read-only copies of its original TSFS root Type-551 tapes,
without rebuilding or modifying KCORE, MRES, MM, ABI or user source.

## Actual physical source-matched MM snapshots

SIMH console EXPECT stopped the genuine guest during the parent
before-BRK `[B`, after-BRK `[A`, and terminal `Z` markers. Each stop
captured the entire original MM table and arena from the matching
`kcore.map`, not synthetic MMINFO counters. The owner snapshots
were decoded from actual simulator memory, checked for ordering,
extent boundaries, pins, unique PACKED owners and total arena size.
The parent remained current slot 1. Original arena low/high was
`042602..200000` throughout (all values octal PDP-6 words).

| Stage | MM extent count | PACKED owners | Physically occupied | Free gaps | Largest aligned gap |
|---|---:|---:|---:|---:|---:|
| B | 025 | 0 | 074654 | 040322 | 036000 |
| A | 025 | 0 | 104654 | 030322 | 016000 |
| Z | 010 | 0 | 021556 | 113420 | 071577 |

The parent BRK's additional `010000` word allocation is reflected
exactly in the B/A owner table. After all six source-authentic WAIT
status checks, no orphaned PACKED extent remains. Original simulator
HALT follows the terminal Z. Source/physical testkit auditor PASSes
and rejects two forged witnesses: an overlapping owner extent and
a missing post-WAIT snapshot. The replay runner's initial strict
line-start check was corrected for ECHO markers emitted directly
after an unfinished SIMH console EXPECT (`[B`, `[A` or `Z`); the
actual full snapshots and HALT were already present in its trace.

## Critical limitation and next experiment

**No process was actually PACKED during B, A or Z in this 64K
profile.** The v1627 parent/child lifecycle is therefore a valid
functional RUN/BRK/PIPE/WAIT/EXIT regression with PURE MRES loaded,
but does NOT prove concurrent RAW inactive ownership or return
from PACKED. These three samples also do NOT prove transient peak
or net RAM advantage; the installed PURE module costs 1758 GCC
resident words even when no process omits PURE text.

The next functional stress guest should use genuine 48K pressure,
where prior physical v1625 shows five actual PACKED suffixes.
Cause the expected sixth RUN resource refusal WITHOUT terminating
the parent, then release the five blocked children and verify their
return from PACKED with original MM owner snapshots and parent
WAIT/EXIT. Keep the existing proper owner/PI/source contracts;
do not add a fake memory allocation or bypass the RAM allocator.

P1-E still OPEN, default PURE OFF, strict 32K optimization HOLD.
