# P1-E v1636: BCACHE low placement restores six 48K APR admissions

Date: 2026-11-16. Base: v1635 exact physical first-VM proof that
the BCACHE 0201-word high slab occupies 137577..140000 and forces
the first long-lived parent APR down to 126000..136000. The
v1633 fixed-parent 389-word bound did not apply if the parent
could be placed at the otherwise free physical top.

## Minimal production correction

`system/kernel/storage/bcache_pdp6.s` has one functional change:
the private clean-cache slab requests low MM allocation instead
of high. The old two instruction `movei 1,1; movem 1,(17)` argument
is replaced by `setzm (17)` for `MM_ALLOC_LOW`. The 0201-word
owner-6 dynamic extent, no-reclaim allocation, fetch/store cache
semantics, physical buffer contents, invalidate and reclaim paths
are unchanged. User APR allocations continue to request HIGH.

This follows the existing allocator contract: a small reclaimable
cache need not occupy the physical top and block the next full
02000-aligned process APR. It does not relocate a live APR,
modify PACKED metadata, bypass an allocation refusal or change
the default state of the optional PURE service. One instruction
word disappears from KCORE as an incidental improvement.

## Independent complete 48K guest result

An original-source-derived C89 testkit guest, compiled with fresh
DAIMOS GCC, KCORE/MRES, linker, TSFS tapes and no auxiliary media,
admitted six children under actual 48K PDP-6 simulation. Parent
uses a no-growth BRK (010000 -> 010000); original child PIPE read,
two private mutable-data sentinels, EXIT 057, six exact parent
PID/EXITED/status-057 WAITs and final parent data checks all pass.

Real complete guest console:

`aaaaaa[B073470/042501]<010000+010000>[A073470/042501]{073470-073470}LwwwwwwZ`

The guest ends in normal simulator HALT. `L` means lifecycle only;
the test does not claim another RAW prefix drop during BRK or
positive whole-system net/peak RAM. Original BCACHE v1490
cache-source/isolation and refused-free invalidation tests PASS.

## Physical MM source-linked replay

Without recompilation, the same compiled guest/root tapes were
replayed with physical MM snapshots at parent B/A/Z. All original
source/link MM symbols and the live extent table validate:

| Stage | Parent APR, octal words | Children | PACKED | BCACHE | MM extents |
|---|---|---:|---:|---|---:|
| B | 130000..140000 | 6 | 0 | low | 025 |
| A | 130000..140000 | 6 | 0 | low | 025 |
| Z | 130000..140000 | 0 | 0 | low | 010 |

The cache's 0201-word extent is present at the low end rather
than in the top APR. The parent resides in the physical top
which was previously obstructed. All six PROCESS extents are
owned while asleep; after six original WAITs no child PROCESS
or PACKED owner remains. The source-linked testkit audit passes
and rejects forged parent placement and false final WAIT result.

The v1631 old high-cache PURE-on 48K guest refused its sixth
RUN, while the v1636 low-cache guest succeeds and reaps all six.
This removes that physical process-admission regression. It is
not equivalent to a full RAW restore or whole-system RAM gate:
**none of the six children had been PACKED** in this fitting
workload. The new v1637 seventh-RUN pressure profile must create
real PACKED suffixes, then test restoration and all WAITs.

Remaining P1-E gates: physical fast/FULL restoration cases,
immutable executable source loss and unmount, concurrent PI6/
MM rollback, 32K/64K cache placement regressions and matched
physical RAM net/peak measurement. PURE stays OFF by default,
strict 32K whole-KCORE optimization HOLD pending feature freeze.
