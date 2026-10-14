# v105 — Two real VM relocations in one PDP-6 compaction request

The native `daimos-testkit/tests/native-kernel/mm/vm-live-v105` fixture
executes the real production `mm_pdp6.s`, `vm_extent_move` and
`fs_move_words`/`fs_copy_words` assembler under the SIMH PDP-6 emulator.

In two separate managed physical arenas, three pinned descriptors constrain
available gaps. Two independent sleeping process images (owners 7 and 010,
each 01000 octal words) must relocate into the second arena before a
04000-word gap becomes available in the first. The test checks that
`mm_compact` returns success only after both moves, that the process VM
bases are 046000 and 042000, that descriptor count remains five, both PI
disable/restore hooks each execute twice, and that every word in both
payloads matches independent address-derived expected patterns. Native
SIMH PDP-6: **PASS**. Earlier full payload, mapped, pin, u-area,
cross-arena and idempotent compaction checks remain present.

Two fixture corrections were necessary during development: offsets must be
written in octal and sleeping state must be initialized explicitly for both
process records (the actual VM mover correctly rejects runnable processes
without a stable u-area).

No production MM code changed. Assembled baseline remains 493 MM text words,
70 BSS words, including `mm_move_extent` 66 and `mm_compact` 62. The next
candidate is measured elimination of redundant descriptor mutation and
search. Historical ITS-138/JOSS core managers have different block-granular
metadata and are references, not drop-in routines. Real PI scheduling,
rollback under failed descriptor publication, and randomized stress are
still outstanding.
