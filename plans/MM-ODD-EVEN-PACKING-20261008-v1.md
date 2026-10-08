# DAIMOS PDP-6 MM odd/even descriptor packing — implementation plan v1

Date: 2026-10-08. Status: **design / experimental; NOT implemented in the kernel**.

## Objective and baseline

Reduce resident RAM for allocated MM extent descriptors without duplicating ownership metadata or weakening correct free, pin, compaction, swap, and boot behavior. The baseline permanent PDP-6 allocator uses a sorted array of `size(18),base(18)` spans and a second 36-bit metadata word per extent (18-bit owner, allocation type, 8-bit pin count). `system/kernel/mm/mm_pdp6.s` currently allocates 21 descriptor slots (42 words). A separate native KCC `MAKE -> KCC -> KCPP` failure revealed that 21 is insufficient; a 32-slot increase is separately being investigated and MUST be integrated/baselined before claiming savings. Do not confuse the extent-descriptor shortage with the still-open nested-KCC HALT after increasing capacity.

## Proposed layout (odd/even)

Use two arrays: an unchanged 36-bit `size,,base` span per extent, and one 36-bit metadata word shared by two adjacent descriptors. Even numbered descriptor `2n` uses the **left** 18 bits of `metadata[n]`, odd `2n+1` the **right** 18 bits. Read with `HLRZ`/`HRRZ`, write with `HRLM`/`HRRM`. No coarse allocation granularity and no additional owner table. Total table words = `N + ceil(N/2)` instead of `2*N`; for N=32 save 16 words gross, N=64 save 32, N=128 save 64. All arrays remain sized by the same MM_MAX_EXTENTS constant. An odd last slot has an unused right half which must be initialized and cleared.

A *candidate* metadata layout is 12-bit owner, 2-bit type and 4-bit pin count (18 bits exactly). The original owner is 18 bits, type reserves 3 bits, and pin count 8 bits, so **this is not yet ABI-compatible**. Preserve four type classes (`FREE`, `PROCESS`, `MODULE`, `KERNEL_DYNAMIC`); prevent pin count overflow/underflow and never truncate owner IDs. A C99 host encoding test already proved the arithmetic round trip, not its workload safety.

## Blocking semantic checks, before kernel modification

1. Census every owner value and its lifetime, in C and PDP-6 assembly. Include KINIT MRES owner IDs (`mres_owner_next`), process image owner slots, u-areas at octal `01000+slot`, TTY lines at `02000+tty`, MEMFS, D6FS/DTFS/TSFS, BCACHE, PIPE, swap, kernel stacks, modules, badmap, text/DPY. Prove that all allocated owners fit 12 bits for supported configurations **or redesign the 18-bit halfword encoding before implementation**. Do not silently narrow public arguments in `mm_alloc*` or `mm_free`.
2. Census maximum simultaneous pin references and all `mm_pin`/`mm_unpin` callers. Prove <=15 with explicit error behavior or choose a different design. Do not downgrade a reference count to a boolean. Ensure pin underflow and overflow retain existing error codes.
3. Audit all users of `mm_extents`: `system/kernel/mm/mm_pdp6.s`, `mm.c` (portable reference), `mm_boot.c` (disposable KINIT), `system/kernel/proc/vm_pdp6.s` (VM compaction/moves), and any additional direct readers. A partial conversion is unsafe.
4. Preserve physical-base sorting and non-overlap, 18-bit exact sizes/addresses, high/low aligned fit policy, all failure codes, and descriptor count bounds. Keep MM tables resident; boot-only routines may be simpler but must agree on representation.

## Implementation sequence

1. Freeze and measure a clean baseline (kernel `PERMANENT_LAST`, permanent code words, BSS, descriptor data, 96K and 256K simulator boots, current MM and KCC regressions). Integrate/verify the independently proposed 32-slot capacity fix first; its footprint is a separate change.
2. Develop a host C99 behavioral reference model in **daimos-testkit**, including odd/even get/set, sorted inserts/deletes crossing parity, compaction move rollback, owner/type checks, pin saturation, and odd-capacity tails. The preliminary `daimos-mm-odd-even-20261008-v1.c` exercises 32-slot insertion/deletion cycles; expand it to cover the error paths and compare results with the two-word model.
3. Implement central PDP-6 metadata-halfword access helpers/macros using odd/even slot parity; avoid general bitfield shifts for metadata accesses. Span array must remain directly indexable in the hot fit/compaction scans. Measure the helper code overhead.
4. Convert `mm_delete`, `mm_extent_insert`, `mm_find_base`, `mm_alloc_aligned_noreclaim`, `mm_move_extent`, `mm_compact`, `mm_free`, `mm_pin_adjust` in `mm_pdp6.s`. Insert/delete must shift spans and paired metadata in the same logical order, including transient descriptor removals and rollback. Keep interrupts/PI semantics around compaction unchanged.
5. Convert `mm_boot.c`, `mm.c`, and direct `vm_pdp6.s` consumers, retaining matching portable and assembly implementations. Do not leave a temporary duplicate metadata array in resident storage. Any temporary data must be stack-local or disposable boot code.
6. Add MM regression probes to **daimos-testkit**, exercise swaps, process exit/wait, nested DSH -> MAKE -> KCC -> KCPP, module/MRES initialization, FS caching/pinning, repeated allocation fragmentation and compaction, and both even/odd descriptor counts. Run clean 96K and 256K simulator tests and `d6fsck`.
7. Measure final `PERMANENT_LAST`, permanent code growth, packed BSS reduction, execution time of allocation/insert/delete and full boot. **Keep packing only if total resident words decrease without correctness regressions**, and code remains understandable. Otherwise revert production changes and retain test results.
8. Only after complete validation, update canonical explanations in `pdp10-doc/KERNEL/MEMORY-MANAGEMENT.MD`, `DAIMOS/PROJECT-STATE.MD`, and TODO status; commit/push all related repos. Preserve unrelated working-tree changes.

## Fallbacks and decision gates

- If 12-bit ownership or four-bit pin counts cannot be proven safe, DO NOT truncate. Consider a representation with type-specific implicit owners only where derivable from existing authoritative records, but assess code and lookups against the 16-word saving at 32 slots. Avoid shadow tables that reproduce the very data being eliminated.
- If odd/even metadata shifts add >=16 permanent words at 32 descriptors, the 1.5-word representation is a **net loss** at that size. Compare 64 and 128 descriptor configurations separately before deciding.
- A one-word descriptor is a different design: size/base already exhaust 36 bits, so it would need coarse length units and external ownership/pin semantics. Defer it until a proven lossless scheme exists; do not mix it into this patch.
- Keep swapping I/O/block granularity independent from metadata packing. This plan changes descriptor representation only, not swap policy or disk formats.

## Current artifacts / handoff

The prior host encoding experiment is in daimos-testkit branch `experiment/mm-packed-20261008-v1`, commit `94cb80c`, with a shar in `$HOME/tmp/daimos-testkit-mm-packed-20261008-v1.shar`. A second uncommitted testkit host model currently exercises paired metadata insertion/deletion. **Neither validates real allocator semantics or native boot.** The current source worktree contains no production allocator edits from this design. This plan is the authoritative next-step checklist for the experiment; it is not a completion claim.

## Investigation update — 2026-10-08

- Verified the direct process-VM dependency: `system/kernel/proc/vm_pdp6.s` `vm_brk_find_extent` explicitly computes a two-word record address then validates metadata type, owner (against `proc_current_slot`), and physical pin count. It also retains a direct descriptor pointer for resizing. This routine must be converted in the same kernel change; touching only `mm_pdp6.s` is unsafe.
- Boot-time `mm_boot_reserve()` directly validates owner/type/pins, compacts `mm_extents[]` as C structs, and calls code that scans exact spans. Both `mm_boot.c` and the portable `mm.c` model currently assume interleaved two-word structs. Those consumers cannot interpret packed metadata without new accessors.
- The PDP-6 assembly allocator also manipulates paired descriptors in insertion, deletion, compaction rollback, `mm_free`, `mm_is_pinned`, and `mm_pin_adjust`; simply changing `.block` would corrupt memory.
- `mm_pin` and `mm_unpin` are used by kernel-stack initialization and process swap/VM mapping. A four-bit pin field needs a checked workload bound or explicit policy; silently truncating pins is unacceptable.
- `mres_owner_next` currently checks against `MM_OWNER_MASK` (18 bits), not 12. Before narrowing that mask, calculate the maximum number of staged MRES and test the overflow boundary. Process u-area owners use octal `01000+slot`; TTY buffers use octal `02000+tty`; owner encodings require a complete census.
- Host-side behavioral test `daimos-testkit/tests/host/daimos-mm-odd-even-20261008-v1.c` is committed on branch `experiment/mm-odd-even-model-20261008-v1` at `a9d9cdb`. It verifies 32-slot odd/even insertion/deletion for 2,000 cycles against independent reference entries at every operation. This proves packing mechanics only, not production boot correctness.
- **No production MM source edited.** The exact owner/pin capacity and full kernel compatibility remain blocking; no permanent-word saving has yet been measured in a linked kernel.
