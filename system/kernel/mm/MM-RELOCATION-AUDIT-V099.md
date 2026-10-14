# v099 — Native PDP-6 extent relocation audit

Production base: v098, `system/kernel/mm/mm_pdp6.s`. Assembly measurement
with `das -F -C -L ... -O ...`: **`mm_move_extent` 66 words** (octal 0102),
`mm_compact` 62 words (octal 0076), whole MM text **493 words** and static
BSS **70 words**. No production code changed in this checkpoint.

## Necessity and contract

`mm_move_extent` is a private helper of `mm_compact` and cannot be discarded
while compaction physically relocates resident process extents. It saves
original metadata, disables PI, temporarily deletes its source allocation,
searches HIGH for a destination, *restores* the source descriptor, and only
then calls `vm_extent_move(owner, old_base, words, new_base)` if a strictly
higher address was found. This order is required: `vm_extent_move` checks
process state, translated mappings, u-area state and run queue membership;
it performs the copy and publishes the VM base. After success MM removes
the old descriptor and inserts its updated sorted descriptor. It restores
PI along both success and failure exits. It is not a simple BLT operation.

## Simulator regression

`daimos-testkit/tests/native-kernel/mm/relocation-v099` executes the actual
PDP-6 MM assembler with deterministic callbacks for the independent VM
operation and interrupt state. It verifies successful upward move across a
fragmented arena; the original descriptor is visible inside VM callback;
MM publishes the new descriptor in sorted order and restores PI. It then
makes VM refuse relocation, checks the original allocation is retained,
and checks PI restoration. **PASS in SIMH PDP-6 through p10run.** This tests
MM's transaction with a simulated VM callback, **not** the live production
`vm_extent_move` or process data-copy correctness; that needs a separate
integration regression before changing relocation ordering.

## Optimization review and alternatives

- The helper's one-delete/search/reinsert sequence is potentially expensive.
  Instead of deleting the descriptor, a search with a single ignored extent
  could find a destination without mutating metadata. However this changes
  the shared gap-search ABI, adds branch logic to the hot allocator path, and
  must be measured for total word count and execution cost; **not approved**.
- A direct scan of the free gaps *above* the source can skip the temporary
  deletion but duplicates some of `mm_find_fit`; investigate only if its
  total linked footprint is smaller than the current approach.
- The final sorted insertion search could be combined with the destination
  search or performed while copying descriptors, provided PI and descriptor
  publication invariants are preserved. Validate all boundaries.
- BLT opportunities exist in two-word descriptor manipulation only if its
  setup actually saves words; the larger process copy already belongs to
  `vm_extent_move`/`fs_move_words`, not this routine.
- The current helper discards the return from its descriptor reinsertion
  calls. Those calls are expected to succeed because the helper just freed
  a slot under disabled PI, but this should be documented as a formally
  established invariant and stress-tested for metadata corruption.
- ITS-138's `CORE ALLOCATOR` and JOSS's main-loop compaction are relevant
  design references for relocation exclusion and publication timing, not
  drop-in replacements: block granularity and process coordination differ.

**Decision:** retain the 66-word implementation pending a native integration
suite with real `vm_extent_move`, adversarial pin/mapping/scheduler cases,
and independent before/after data preservation. An unmeasured shortening is
not a safe optimization here.
