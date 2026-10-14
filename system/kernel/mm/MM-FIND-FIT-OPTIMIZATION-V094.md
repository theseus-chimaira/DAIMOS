# v094: First native MM optimization — gap search

Base: v093 measured assembly baseline, 520 text words, 70 BSS words.

`mm_find_fit` used two nearly identical full scan policies: one for a gap
preceding an allocated extent, and another for an arena's trailing gap.
Both independently calculated total free words, LOW alignment and HIGH
alignment. The v094 implementation routes both through one gap evaluator:
AC2 is exclusive gap end, AC4 is free cursor. The extent path falls directly
into the evaluator (no added branch per extent); the arena-tail path loads
AC2 from arena end and branches to the evaluator. The common advance path
checks whether there is another in-arena extent before moving the cursor.

| Measure | v093 | v094 | Delta |
| --- | ---: | ---: | ---: |
| `mm_find_fit` assembler words | 112 | 90 | -22 (-19.6%) |
| Entire MM `.text` words | 520 | 498 | -22 (-4.23%) |
| MM static `.bss` words | 70 | 70 | 0 |
| MM `.text` + `.bss` words | 590 | 568 | -22 |

Word counts from `das -F -C -L ... -O ...` and label-difference comparison;
no unproven `p10fold` savings included. The KCORE and KINIT GCC DXR links
both succeed. The full-KCORE post-fold delta is not separately measured.

Native PDP-6 tests (real `mm_pdp6.s`, no OS boot):
- `alloc-v090`: PASS.
- `free-v092`: PASS.
- `gap-v094`: PASS, including low/high placement over two arenas,
  interior and trailing gaps, alignment-only fragmentation, insufficient
  contiguous space versus insufficient total space, and unchanged descriptor
  count following failed allocation.
- `gap-v094` also PASSes against the unmodified v093 allocator to validate
  the test's baseline expectations.

This test is not exhaustive; real kernel boot and reclaim/compaction stress
remain separate acceptance gates. More aggressive allocator algorithm/data
representation proposals remain governed by PROJECT-STATE.MD v091 policy.
