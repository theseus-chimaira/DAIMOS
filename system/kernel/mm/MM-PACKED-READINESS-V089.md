# v089: Real PDP-6 MM readiness audit for packed-process reclaim

Source base: v088 KCORE, `mm_pdp6.s`, `proc_packed_evict_v077.c`,
`proc_packed_restore_v46.c`, `proc_swap_pdp6_inline_v083.s`.

## Verified source-level mechanisms

- `mm_alloc_aligned_noreclaim` allocates aligned extents without invoking
  swap or filesystem reclaim. Packed eviction uses this entry to prevent
  recursive reclamation while the source remains pinned.
- `mm_pin_adjust` bounds the pin count; `mm_free` refuses pinned extents,
  and checks both owner and type before removing a descriptor.
- MM maintains sorted allocation extents, not a separate list of free
  blocks. `mm_find_fit` derives gaps between allocated extents on each
  allocation; deleting an extent therefore exposes its free space, and
  adjacent freed ranges are reusable without an explicit coalescing pass.
- `mm_compact` searches movable, unpinned PROCESS extents; ordinary
  `mm_alloc_aligned` performs bounded compaction and swap/FS reclaim.
- The packed transaction measures, allocates exactly, encodes, transfers
  ownership, and publishes PACKED storage; the v081 mock-MM failure/retry
  tests establish its transaction contract, not real-MM pressure behavior.

## Required acceptance before live automatic packing

1. **Real-MM allocation and failure:** show that failed compressed-destination
   allocation leaves the source pinned only for the operation and does not
   publish PACKED; verify recovery and retry on the native PDP-6 allocator.
2. **Reclaim when free RAM is inadequate for destination:** demonstrate a
   non-recursive bounded policy (or prove that the existing free reserve is
   sufficient) without allocating a speculative compressed extent. There
   is no demonstrated production guarantee for this case yet.
3. **Fragmentation and descriptor pressure:** test 32 descriptors/3 arenas,
   pinned islands, repeated pack/restore cycles and aligned restore failure
   using *real* `mm_pdp6.s`, not just host simulated extent tables.
4. **Scheduler integration:** automatic eligibility/eviction trigger is not
   established by the v088 link. Restore and teardown routing alone are
   insufficient.
5. **Invariants and concurrency:** exercise storage publication, MM free,
   process runqueue/TRANSITION and exit in one booted kernel. Do not infer
   process safety from separate host and native component tests.

## Test suite blocker found

`daimos-testkit` `mm-core` currently omits the target-only symbols
`proc_storage_publish_v30` and `proc_swap_storage_invariant_v30` from
its host linkage. A C99 test equivalent resolves those symbols, but the
existing fixture then hits the invariant trap: it has not initialized the
new storage-state assumptions for all its old swap cases. This is a test
harness migration blocker, **not proof of an MM bug**. Repair and run the
existing tests before altering production MM.

## Priority

First restore the legacy host-MM regression using accurate storage-mode
setup, then add native PDP-6 real-allocator pressure and rollback tests.
Only then change reclaim integration and attempt an automatic packed-process
cycle. The separate 8K KCORE limit remains deferred by request.
