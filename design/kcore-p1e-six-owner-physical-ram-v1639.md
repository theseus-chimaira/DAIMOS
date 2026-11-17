# P1-E v1639: byte-matched 48K six-owner physical RAM comparison

Date: 2026-11-17. Base: v1638 accepted physical v1391 FULL restore
after v1636 low-BCACHE placement and v1637 six genuine PACKED
restorations. No production KCORE/MM/ABI/MRES/code change here.

## Two source-identical original PDP-6 physical workloads

The genuine 48K no-AUX/TSFS/PDP-6 PURE-enabled v1637 guest and
the new PURE-disabled v1639 guest use byte-identical C89 source,
compiled user INIT DXR and fixed `kcore.map`. Both use exactly
the same v1636 corrected low-BCACHE production code and admit
six sleeping child processes, then make the seventh RUN fail
cleanly under physical memory pressure. The parent survives,
checks no-growth BRK and confirms all original child PIPE reads,
two mutable-data sentinels, EXIT-057, six exact PID/EXITED/
status-057 WAITs and terminal Z, followed by normal HALT.

The ON guest installs the optional PURE MRES, whose physical
1758 decimal words are measured in the managed-arena low-bound
shift of 003336 octal from the otherwise identical OFF image.
The ON child extents are all PACKED (007000-word retained
suffixes), while the OFF children stay complete resident APR
images (010000-word PROCESS extents). The earlier v1632 study
sampled equal child counts with a slightly different admission
history. v1639 instead uses the same six-child + seventh-refusal
progression in both modes, with full parent WAIT acceptance.

## Source-linked live MM snapshots

The exact source-built guest images and Type-551 tape copies
were replayed separately in SIMH with complete original sorted
MM extents at parent B/A and after all WAITs (Z). Values below
are octal PDP-6 **physical 36-bit words**, not the MEMINFO
logical process counters printed by the user program.

| Stage | PURE ON occupied | PURE ON free | PURE OFF occupied | PURE OFF free | Free ON - OFF, decimal |
|---|---:|---:|---:|---:|---:|
| B | 066433 | 006564 | 074433 | 004122 | +1314 |
| A | 066433 | 006564 | 074433 | 004122 | +1314 |
| Z | 011536 | 063461 | 011335 | 067220 | -1887 |

At B/A, PURE enabled leaves **3444 decimal physical free words**
compared with **2130 decimal** without PURE, a measured +1314
decimal-word free-memory advantage INCLUDING the installed
1758-word removable service. Six omitted 001000-octal prefixes
gross 3072 decimal words; 3072 - 1758 = **1314**, matching
the two original MM free-space tables exactly. The parent APR
remains 130000..140000 in both images and all six child owners
are distinct, unpinned and nonoverlapping.

After all six child WAITs, no PROCESS or PACKED child MM owner
remains in either guest. PURE ON has 1887 decimal fewer free
words at Z, NOT a persistent process-owner leak: its 1758-word
resident MRES is still installed, and the ON fixture also owns
an additional valid 0201-octal-word (129 decimal) low BCACHE
private slab at this sample while the OFF image does not.
These terminal cache states are not identical; both facts must
be included to explain the full 1887-word difference.

The original-source-linked testkit auditor PASSes and rejects
both a corrupted OFF resident owner identity and a false final
parent WAIT success marker. Both real guests reach normal HALT
with the same original user program and fixed KCORE map.

## Acceptance boundary

The sampled physical net-free RAM gain for six simultaneously
PACKED children is **accepted**. This is stronger evidence than
the earlier five-child v1632 snapshot, and now holds after
exactly matched sixth-child/seventh-denial source progression.
However, it is NOT a transient peak trace; the v1391 FULL restore
itself briefly holds both the packed suffix and a full APR,
as physically demonstrated in v1638. Nor is it proof of higher
steady-state throughput or of positive benefit with fewer RAW
inactive children. With no children, the still-installed PURE
MRES has a real RAM cost.

Still OPEN before feature freeze: matched whole-system peak RAM
including simultaneous FULL source/destination pins and all
scratch/cache; immutable backing source loss and unmount; forced
loader/MM errors and PI6 failure-atomic rollback; 32K/64K
low-BCACHE regressions and complete source-matched feature
acceptance. Optional PURE remains OFF by default. Strict 32K
whole-KCORE size optimization remains HOLD.
