# v115 — Resolve KCORE.MD MM work before further optimization

## Open MM tasks verified in KCORE.MD

1. v089 native MM allocator pressure/retry acceptance and legacy `mm-core`
   storage-mode fixture migration are still `[ ]`. They are correctness
   prerequisites, not code-size opportunities. The existing standalone
   alloc/free/gap/noreclaim and VM relocation suites do not establish
   complete reclaim/swap/rollback acceptance.
2. P2 runtime-sized extent descriptors versus the current static 32-entry
   two-word table remain `[ ]`, including representative nested MAKE/KCC/
   KCPP loads and relocation-sensitive assembly measurements.
3. P2 optional large-core sorted-linear-scan indexing remains `[ ]`.
   This is conditional on measured large-core workloads and should not
   add permanent lookup structures to small configurations.

Related packed lifecycle and registry sizing work is outside the MM-only
optimization scope but supplies important requirements for descriptor peak
counts and reclaim-pressure scenarios. Do not modify process or filesystem
algorithms under this MM-only effort.

## Current representation and sizing

`mm_pdp6.s` fixes `MM_MAX_EXTENTS=040` (32 decimal); `mm_extents` reserves
`0100` (64 decimal) PDP-6 words. Three arena descriptors, `mm_core_words`,
`mm_extent_count` and `mm_arena_count` consume the remaining six words:
**70 BSS total**. `mm.h` declares an array of `MM_MAX_EXTENTS`, and
`mm.c` defines it statically. `mm_extent_insert` enforces the bound;
`mm_boot.c` and packed process code index the same array.

Possible alternatives must be measured, not assumed better:

- **Fixed 32:** no separate pointer/dynamic ownership or allocation
  failure; smallest implementation complexity but 64 reserved words
  regardless of actual peak occupancy. Baseline 482 assembler text + 70 BSS.
- **Boot-sized bounded table:** reserve only the selected number of
  descriptors, but retaining a fixed symbol requires a new location/ABI
  mechanism. Potential savings of `2*(32-capacity)` descriptor words are
  *theoretical only*, before pointer/capacity/reservation costs and
  integration. Must initialize before any MM allocations or boot extent
  registration, preserving failure atomicity and physical exclusion.
- **Runtime growth:** may reduce idle reservations but resizing the live
  sorted table introduces allocation recursion and metadata relocation
  hazards. This cannot be adopted as a cosmetic substitution of array
  declaration for pointer; must first establish ownership and rollback.

The **first implementation decision must follow** native pressure tests and
measured peak descriptor usage. Theoretically saving 16 or 32 words by
shrinking the table is not evidence that real 32K systems can function
with the smaller bound. Likewise larger process/packed workloads may need
more than 32 descriptors.

## Execution order and acceptance

1. Repair the v089 legacy `mm-core` host fixture using the current packed
   storage-state contract, and verify it against genuine production MM.
2. Add native pressure tests for no-reclaim nested allocation, reclaim,
   swap retry, fragmented-versus-total-free classification, descriptor
   saturation, and correct rollback. Keep all tests in daimos-testkit.
3. Capture peak simultaneous extent occupancy for representative 32K and
   larger configurations and nested MAKE/KCC/KCPP workloads. Do not invent
   measurements where no full-system boot exists yet.
4. Prototype boot-sized capacity *only if* those measurements justify it;
   review C and assembly ABI, bootstrap reservations, pointer stability,
   interrupt atomicity and any dynamic table allocation recursion.
5. Compare code, permanent data, peak stack, runtime and failure cases
   against fixed 32. Retain the demonstrably better architecture, then
   resume MM-only routine-level optimization.

No source representation or allocator implementation has been changed in
v115. The existing v114/production MM remains 482 text and 70 BSS words.
