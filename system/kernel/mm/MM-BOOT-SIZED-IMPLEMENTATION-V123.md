# v123 — Boot-sized physical extent table: implementation contract

**Decision:** KINIT selects a fixed-for-this-boot capacity from discovered
physical memory. The table is permanently reserved before KINIT publishes
any free arena; it does not resize and does not call MM to allocate itself.
This work is required before further MM instruction-level optimization.

## Initialization and permanent address lifetime

`kinit_enter()` calls `kcore_load()`, reads `kinit_memory_kwords()`, derives
`core_words`, `__kcore_low_end` and `__kinit_image_end`, and then initializes
MM arenas. The safe new ordering is:

1. Choose conservative descriptor capacity from `core_words`, at least 32
   until representative workloads demonstrate a smaller table is safe.
   Exact larger-machine thresholds are **policy choices still to implement
   and test**; no workload-derived numbers exist yet.
2. Reserve `2*capacity` physical words adjacent to the *high-side* KINIT
   bootstrap stack reserve, before calling `mm_add_free` on any high range.
   Validate address arithmetic and total memory first. Zero exactly the
   descriptor backing words; publish a pointer and active capacity.
3. Set `mm_extent_count=0`, `mm_arena_count=0`, then expose low/high free
   arenas, *excluding* the permanent descriptor region. Do not reclaim the
   table with KINIT or treat it as ordinary allocatable MM core.
4. Bound insertion with the selected capacity; no resizing, pointer movement,
   or table allocation through MM during a boot.

## ABI migration is mandatory

Current `mm_extents` is an exported **array symbol** fixed in resident
`.bss`, not a pointer. `mm.c`, `mm_boot.c`, packed process C code, and the
three assembler units `mm_pdp6.s`, `vm_pdp6.s`, and
`vm_pdp6_packed_v088.s` directly form `mm_extents(index)` effective
addresses. Merely substituting a pointer at that address silently accesses
wrong memory on the PDP-6. The transition must update *all* references
atomically, and native fixtures must initialize the new table location.

The C reference implementation must use the same selection and initialization
API as the PDP-6 backend, including failure handling, without introducing a
separate host-only capacity policy. The 32-entry fixed-table oracle and
native 31->32->31 boundary must remain valid at minimum capacity.

## Selection and tests

Proposed **initial** capacities for comparative builds, not workload proof:
32 on small memory, 48 on medium memory, 64 on large memory. Test KINIT's
mapping for 32K/64K/128K/256K and memory too small to hold the selected
permanent table plus its stack reserve. All cases must be deterministic.
Do not shrink 32K below 32 descriptors without real demand measurements.

Acceptance gates: native PDP-6 production MM and VM tests; descriptor
insert/delete at capacity-1/capacity; two-arena compaction with pinned
processes; KINIT physical reservation exclusion and no overlap; both C
and assembler representation agreement; host mm-core pressure/rollback;
full KCORE/KINIT link; measured total code/static/reserved permanent RAM
and stack. Full-system boot remains a separate acceptance gate.

**Status:** This document is an implementation prerequisite and source/ABI
audit only. The actual table-pointer conversion and KINIT reservation have
**not** been implemented, so no size or runtime improvement is claimed.
The shipped v122 MM remains 482 executable + 70 BSS words.
