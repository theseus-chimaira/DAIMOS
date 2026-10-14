# v103 — Cross-arena native PDP-6 relocation test

`daimos-testkit/tests/native-kernel/mm/vm-live-v103` executes actual
`mm_pdp6.s`, production `vm_extent_move` and `fs_move_words`/`fs_copy_words`
in the standalone PDP-6 emulator. It extends the verified v102 cases with a
**second disjoint arena** and a successful relocation from the first arena
into a free region in the second. The test validates sorted extent metadata,
unchanged descriptor count, the updated process VM base, balanced PI
callbacks, and every one of the 01000-octal-word (512-word) payload words.
The original single-arena, pinned, mapping-held, transition and stable-uarea
cases remain covered. Native PDP-6 regression: **PASS**.

## MM relocation algorithm review

`mm_move_extent` remains a private 66-word helper of `mm_compact` (62 words).
Its delete/search/restore sequence is intentional: omitting the source
extent when finding a destination permits the source to be reused, but the
original descriptor must be restored *before* `vm_extent_move` examines the
process. After VM successfully copies and publishes the new process base,
MM deletes the old extent and inserts the new address in sorted order.
Interrupt masking brackets descriptor mutation and VM publication.

A fused search returning both gap address and insertion slot would eliminate
one post-copy descriptor traversal, but changes the shared `mm_find_fit`
interface and increases its live state. An ignored-source search avoids the
temporary delete/restore but adds comparisons to every normal allocation.
Neither is established to save *whole-KCORE* words or RAM; do not introduce
them as an unmeasured workaround. ITS-138 and JOSS use block-granular
relocation and are not interchangeable implementations.

No production MM code was modified. Code footprint remains `mm_move_extent`
66, `mm_compact` 62, full MM `.text` 493 and static `.bss` 70 words.

Outstanding: repeatable multi-relocation stress with several movable processes,
real runqueue/PI concurrency, mixed pinned islands, descriptor-table
exhaustion during relocation, and overlapping copy cases. PI/runqueue hooks
in this standalone test are deterministic fixtures, not the kernel's live
scheduler.
