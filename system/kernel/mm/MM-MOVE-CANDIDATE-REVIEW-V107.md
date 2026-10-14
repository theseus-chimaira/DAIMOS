# v107 — Relocation transaction optimization experiment

Base is v106, whose `mm_move_extent` is 64 words and full MM `.text` 491
words, `.bss` 70. The current relocation transaction is PI-bracketed:
delete temporary extent, search HIGH, restore original extent, call live VM
relocation, and publish a sorted moved descriptor only after VM succeeds.

## Measured rejected candidate: branch before restoring the descriptor

An alternative checked the gap-search result before restoring the original
extent, duplicating the restoration call on fit and no-fit paths.
The live assembler measured **494 text words** (versus baseline 491),
70 BSS unchanged: **+3 words**. This variant also increases source code
branching without eliminating any descriptor movement. It was **reverted**;
no correctness claim or speed benefit is made for it.

## Other alternatives and constraints

- An ignored-source filter in `mm_find_fit` avoids delete/reinsert but adds a
  comparison on every gap-search iteration and affects normal allocations.
- Fusing destination selection with sorted insertion index would require
  more live state and complicates HIGH selection over multiple arenas.
- Direct descriptor shifting after VM success can avoid two calls but must
  preserve sorted order, rollback and metadata atomically across all slots.
- An upward-only insertion search was already improved without code growth
  in v106. The `HRRM` descriptor publication reduced code by two words.

No additional production code savings are established by this checkpoint;
production stays at v106's **491 text + 70 BSS**. ITS-138 and JOSS
block-granular relocation designs are not interchangeable with this
word-granular transaction. Before attempting a larger rewrite, measure
whole-MM code, stack and instruction cost, and run real PDP-6 MM/VM
regressions for success, failed VM move, pinned islands, and multiple moves.
