# v096 — What the native PDP-6 gap and no-reclaim algorithms actually do

Base: `mm-noreclaim-optimized-v095-20261014` (routine sizes: `mm_find_fit`
90 words, `mm_alloc_aligned_noreclaim` 71 words, total MM text 496 words).

## Actual search contract

`mm_find_fit` walks up to three physical memory arenas and the one sorted
array of allocated extents. The current arena's `cursor` denotes the end of
occupied memory, so an extent's base or the arena end bounds the next free
gap. LOW returns the first aligned start that fits; HIGH remembers the
highest aligned starting address that fits across *all* arenas. On failure
it instead writes the **sum of unallocated managed words**, allowing the
caller to distinguish fragmentation from insufficient aggregate RAM.
The array stores allocations only: no free descriptor is allocated, and
adjacent freed space is reusable immediately. This total-free-output
requirement prevents a trivial early return on failure.

The input alignment is a positive power of two, validated by the allocator.
The original search recomputed `~(alignment-1)` in each LOW/HIGH gap path.
The v096 implementation builds it **once per call** in scratch AC0 and uses
that mask for either candidate. This also shortens the assembler.

## Actual no-reclaim allocation contract

`mm_alloc_aligned_noreclaim` validates six arguments, finds one fitting gap,
then scans the sorted descriptors again to locate the insertion point.
It constructs a two-word extent (size/base and pins/type/owner), calls
`mm_extent_insert`, and only then publishes `*basep`. It does **not**
compact, invoke callbacks, swap, reclaim or allocate temporary descriptors.
The second scan exists because `mm_find_fit` currently returns only a base
(or aggregate free count), not a descriptor insertion index. This double
scan is a real algorithmic optimization opportunity, but giving search a
second output changes its register and stack contract and might enlarge
all callers. Measure both variants rather than assuming it saves code.

The no-reclaim entry cannot be eliminated while its callers require the
absence of recursive reclaim: packed eviction and restoration, block cache,
and internal ordinary allocation rely on that distinction.

## Optimization and tests

| Metric | v095 | v096 | Change |
| --- | ---: | ---: | ---: |
| `mm_find_fit` text | 90 | 85 | -5 words |
| `mm_alloc_aligned_noreclaim` text | 71 | 71 | 0 |
| Total MM text, including literal pool | 496 | 493 | -3 words |
| Static MM BSS | 70 | 70 | 0 |
| Combined text + BSS | 566 | 563 | -3 words |

The 5-word local reduction yields 3 words of whole-object savings in the
actual assembled layout; the distinction prevents claiming unmeasured
improvements. Labels are measured with `das -F -C -L/-O`, and the text
section with `pdp10-objdump -h`. No post-fold savings are claimed.

Native PDP-6 `alloc-v090`, `free-v092`, `gap-v094`, and `noreclaim-v095`
PASS with the real optimized production assembler. Full GCC KCORE and KINIT
DXR link PASS. No real-OS boot or pressure stress acceptance is claimed.

## Next candidates requiring independent experiments

1. Change gap search to optionally return insertion position: remove the
   redundant second table scan if total code (including caller changes)
   shrinks. Test HIGH across multiple arenas and failures without writes.
2. Merge initial extent skipping with the main scan while preserving
   arena-order/extent-order assumptions. Test arena boundaries and overlaps.
3. Simplify no-reclaim argument validation, but do not weaken the public
   C ABI or overflow/invalid-input handling.
4. Compare a direct first-fit scanner or different extent representation;
   judge permanent metadata and failure behavior as well as code size.
5. Keep canonical instruction ordering for equal-cost alternatives; measure
   raw and post-fold KCORE words and hot-loop branches.
