# v116 — MM host-pressure blocker and PDP-6 BLT review

Scope: MM and its necessary storage-mode test contracts. No source algorithm
outside MM is optimized.

## Current host MM regression reproduced

Ran `make mm-core` in daimos-testkit against the v115 DAIMOS checkout.
**FAIL at link**, with two undefined symbols:
`proc_storage_publish_v30`, `proc_swap_storage_invariant_v30`.
The actual PDP-6 implementations are in
`proc_storage_runtime_v30.s` and `proc_swap_pdp6_inline_v083.s`.

This is not merely a missing linker stub: the host build links the C
`vm_pdp6_swap.c` fallback. Its swap-out path writes a disk-span record and
clears the VM base, but does **not** update the two-bit per-process storage
mode. On the other hand, `proc_swap_detach` calls
`proc_storage_publish_v30(slot, DISK, RESIDENT)` for disk-backed slots.
A contract-faithful test shim therefore rejects the transition unless the
host swap path has published DISK. Simply defining permissive symbols to
make the host regression green would mask the missing transition and is
not acceptable. The production assembly swap paths already use storage
publication explicitly. The repair needs a complete host transition model
with strict state validation and failure-path tests; the old test fixture
must initialize the packed storage map (18 slots per word, two bits/slot).

## Can BLT reduce the MM code?

- **`mm_delete`: yes, semantically safe direction.** It closes a table hole
  by shifting overlapping descriptors *towards lower addresses*. PDP-6 BLT
  copies forward and can implement this. A candidate must account for
  zero shifted words, correct ending address, preservation of ACs and exact
  total MM size; the recently optimized explicit loop is already short.
- **`mm_extent_insert`: no direct BLT for its main overlap case.** Opening
  an interior slot shifts descriptors to *higher addresses*. Forward BLT
  overwrites source words not yet copied. The current backwards loop is
  correct; `fs_move_words` likewise falls back to reverse copying.
- **`vm_extent_move` physical copies:** already call `fs_move_words`, whose
  forward or non-overlapping path uses `BLT` and whose upward-overlap path
  copies backward. Adding another BLT cannot safely replace that logic.
- **Metadata initialization:** zero-fill by BLT is possible for bulk memory,
  but a 32-entry descriptor array is BSS and boot-cleared; duplicating a
  kernel copy routine would not establish a saving.

The next MM-only work should first repair the host storage-mode fixture and
then add native pressure/retry tests. A separately measured BLT variant of
`mm_delete` may be compared against the current 482-text/70-BSS footprint;
do not substitute BLT for backward insertion or remove guards to save words.

**No production change or host regression PASS is claimed in v116.**
