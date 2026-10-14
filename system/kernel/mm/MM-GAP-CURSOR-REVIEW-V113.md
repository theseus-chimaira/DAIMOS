# v113 — `mm_find_fit` cursor-guard analysis and path experiment

Base: v112, complete native PDP-6 MM text 482 words and BSS 70 words.

## Why the v112 guard deletion failed

`mm_find_fit_extent` has **two** entry edges: one from `mm_find_fit_skip`
while an extent is already known to exist, and one from
`mm_find_fit_after_gap` after the processed descriptor index advances.
The latter uses `AOJA 015,mm_find_fit_extent`, and the index may equal
`mm_extent_count` after processing the last extent. The bounds check at
the top of `mm_find_fit_extent` then correctly transfers control to the
arena tail. Without it the code reads a descriptor *outside* the active
extent table. This invalidates the v112 hypothesis; the failed HIGH
allocation test was an appropriate regression signal.

## Tested path reuse, size-neutral

A candidate redirected the already-bounds-checked `mm_find_fit_skip`
branch into a label after the `mm_find_fit_extent` count/index/base-load
sequence, reusing its known-current AC2 base. The `after_gap` edge still
passes through the original guard. Assembly: **482 `.text` / 70 `.bss`**,
identical to v112. PDP-6 native `alloc-v090`, `gap-v094`,
`noreclaim-v095`, `vm-live-v106` all **PASS**. No instruction-count or
cycle-based performance comparison was collected, so this is not a proven
performance improvement; the candidate was **reverted** rather than
complicate control flow without demonstrated total footprint reduction.

## Next approach

Retain the count guard. Any further source-level shortening must be
compared against its entire MM and KCORE footprint and verified for LOW,
HIGH, no extents, last extent, multi-arena boundaries, fragmentation, and
callback-driven relocations. The ITS-138/JOSS block-granular approaches
are design references but not algorithmic substitutes for DAIMOS's
word-granular extent layout.

No production code changed; baseline remains **482 text + 70 BSS words**.
