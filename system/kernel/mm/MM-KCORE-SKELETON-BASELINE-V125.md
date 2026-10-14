# v125: MM KCORE skeleton size and regression gate

The boot-selected extent capacity does not require replacing the MM code.
Retain the allocator in KCORE while KINIT reserves a permanent descriptor
table outside the linked KCORE image before it publishes free arenas.
This removes the bootstrap dependency cycle described in v124 for the
boot-sized-table use case. Runtime MM replacement remains a separate,
deferred architecture choice.

## Baseline and hard acceptance conditions

- Native PDP-6 MM: 482 executable words and 70 BSS words, of which 64
  words are the current 32-entry extent array. Baseline total: 552 words.
- Initial target: no allocator text growth. A maximum of eight extra
  executable words is an experiment budget, not automatic acceptance.
- Total permanent KCORE image size must decrease, counting every new
  pointer, capacity word, literal, veneer and indirect addressing sequence.
- The descriptor backing remains permanent physical RAM; moving it outside
  the KCORE image does not make it free or reclaimable memory.
- Do not expose any free range overlapping the selected table or the
  KINIT stack reserve. The table must be stable for the whole boot.
- Preserve the 32-descriptor policy until measured workloads justify
  alternate capacity thresholds. Never silently shrink to 16 or 24.

## ABI audit gate

The native MM assembler currently contains 27 `mm_extents` references,
and each VM assembly backend (`vm_pdp6.s`, `vm_pdp6_packed_v088.s`)
contains six. These counts include symbol declarations/definitions, not
just runtime memory instructions. C code also directly indexes the array.
An array-to-pointer substitution without simultaneously converting those
references is incorrect. A single cached table base per operation may
reduce overhead, but register pressure and spill instructions must be
measured in the *linked* image. Any boot-time instruction patching must
also establish text writability, relocation correctness and native tests
before it can be preferred over pointer-based addressing.

## Native regression baseline

On 2026-10-14 all 15 versioned tests under
`daimos-testkit/tests/native-kernel/mm/` passed with `p10run --machine pdp6`
against the unmodified v124 MM, covering alloc/free, gap, no-reclaim,
pressure and descriptor-capacity, relocation, pinned VM and cross-arena
compaction. The v125 `run-all-v125.sh` runner makes this repeatable.

Mandatory gate for each representation change: run native MM regressions
before and after, host `make mm-core`, native KCORE/KINIT links, then compare
post-fold resident text, BSS, total permanent core, and dynamic descriptor
backing. Test the KINIT physical reservation and small-core failure path
before claiming any usable memory-adaptive implementation.

This checkpoint adds no production allocator code and makes no new
size or runtime claims. The pointer conversion remains open.
