# v117 — Strict MM pressure-test storage-mode prerequisite

The legacy `daimos-testkit` `make mm-core` target still cannot link:
`proc_storage_publish_v30` and `proc_swap_storage_invariant_v30` exist in
production PDP-6 assembly, not the host-linked C fallback. In addition,
`vm_pdp6_swap.c`'s host fallback currently does not publish RESIDENT->DISK
on successful swap-out or DISK->RESIDENT on successful swap-in, whereas its
detach code requires DISK->RESIDENT publication. The old test must not be
made green by adding unconditional-success stubs.

A new *separate* C99 test in
`daimos-testkit/tests/system/mm-storage-v117` now executes the real
`proc_storage_map_v28.c` implementation. It initializes a 40-slot,
three-word storage map (18 slots/word); checks initially RESIDENT modes,
all 39 nonzero slots' legal RESIDENT->DISK->RESIDENT transitions, rejection
of stale expected modes, and rejection of reserved slot zero or an
out-of-range slot. **Host test PASS**. This is a strict state-machine
prerequisite, **not** a replacement for the blocked `mm-core` integration
regression or a claim that the PDP-6 swap transaction was executed.

Next MM-only acceptance gate: migrate the legacy host swap fixture to
faithfully enforce both successful publication transitions and failure
rollback, then run the actual host allocator pressure/reclaim scenarios.
No production code was changed, and no BLT optimization is accepted.

BLT remains a candidate for `mm_delete`'s forward-overlapping shift, but
`mm_extent_insert` shifts backward for correctness; `fs_move_words`
already uses BLT when forward-copy-safe. Compare any proposed BLT code
against current production **482 text + 70 BSS** rather than assuming
it reduces resident memory.
