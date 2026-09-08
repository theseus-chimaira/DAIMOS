# DAIMOS swap optimization census

Date: 2026-09-08

Base tree: `1eb0ed4a712f286b6af1d6a36c55c157207879e0`

## Implemented

The swap pass keeps the existing PURE/IMPURE/UNKNOWN semantics while reducing
both fixed KCORE code and per-process metadata.

### Shared copy/zero helpers

The swap partial-block paths and swap-in initialization now reuse the existing
`fs_copy_words()` and `fs_zero_words()` BLT helpers instead of compiler-generated
C loops.  `proc_swap_boot_init()` also uses `fs_zero_words()` for the dynamic
record table.

### Three-word swap record

The old four-word record stored:

- backing vnode;
- initialized/text image span;
- swap-disk span;
- meta word containing PURE plus DXR header length.

The meta word was redundant.  The DXR header length is needed only when PURE
text is reconstructed, and PURE executables are accepted only with the current
three-word extended DXR header.  Therefore no header-length state needs to be
stored.

The remaining PURE bit is stored in the high unused bit of the text-length
halfword in `image_span`.  DAIMOS caps executable images at `036000` words, so
text length fits comfortably below that bit.  The text extraction mask keeps
the flag separate from the length.

The record is therefore reduced from four to three words without changing the
backing vnode or either 18-bit span.

## RAM saving

Swap metadata falls by one word per logical process slot:

| Core size | Slots | Old swap table | New swap table | Saving |
| --- | ---: | ---: | ---: | ---: |
| 32K | 64 | `000400` / 256 | `000300` / 192 | `000100` / 64 |
| 64K | 128 | `001000` / 512 | `000600` / 384 | `000200` / 128 |
| 96K | 128 | `001000` / 512 | `000600` / 384 | `000200` / 128 |
| 256K | 256 | `002000` / 1,024 | `001400` / 768 | `000400` / 256 |

Combined process-descriptor plus swap-record tables are now six words per slot:

- 32K: `000600` / 384 words;
- 64K: `001400` / 768 words;
- 96K: `001400` / 768 words;
- 256K: `003000` / 1,536 words.

## Fixed KCORE saving

Fresh same-toolchain links from the compaction base and this pass give:

- base KCORE low-init end: `014600`;
- optimized KCORE low-init end: `014517`;
- fixed KCORE reduction: `000061` / 49 words.

Together with the preceding ABI, C-sharing, and compaction passes, the normal
resident low kernel set is now approximately `025723` / 11,219 words, with no
module removal or relocation-map removal.

## Semantics retained

- PURE writes only the non-reconstructible tail to swap.
- PURE swap-in reloads clean text from the executable using the extended DXR
  header offset.
- IMPURE/UNKNOWN swap the complete process image conservatively.
- scheduler state and nice remain in the process descriptor and are unaffected.
- swap-run allocation remains linear and unchanged.
- descriptor and physical-memory exhaustion behavior is unchanged.

## Gates

Green with the complete swap change:

- host `mm-v1`, including PURE and UNKNOWN round trips;
- real five-process PDP-6 scheduler regression;
- fresh protected 32K, 64K, 96K, and 256K boot/EXIT;
- RAMFS threshold behavior;
- DAS relocatable negative-addend regression;
- DAS file/pipe phase transport regression;
- direct-UUO encoding regression.

The broad `test-das-fast` run was also started and passed many earlier cases,
but exceeded the command execution timeout before completion; this was not a
test failure.  The required DAS gates above were rerun directly and passed.
