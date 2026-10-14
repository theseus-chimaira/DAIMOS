# v109 — Measured PDP-6 MM descriptor deletion optimization

Source baseline: v108 production `mm_pdp6.s` (491 assembled MM text words,
70 BSS), based on the successful v106 `mm_move_extent` changes.

`mm_delete` shifts two-word descriptors downward after freeing a slot.
Originally its loop separately computed the next source index and the
current destination index. Because the source is always exactly one
*descriptor* (two words) later, the same doubled index addresses both:

- `MOVE 4,3` followed by `LSH 4,1` computes the destination offset.
- `mm_extents+2(4)` and `mm_extents+3(4)` read the next descriptor.
- Existing stores write to `mm_extents(4)` and `mm_extents+1(4)`.

This removes three PDP-6 instructions without adding an auxiliary
allocation, stack slot or descriptor field. It keeps forward overlap-safe
copy order and original count/deletion semantics.

| Measurement | v108 | v109 | Delta |
| --- | ---: | ---: | ---: |
| Complete MM `.text` | 491 | 488 | -3 |
| Complete MM `.bss` | 70 | 70 | 0 |
| Combined words | 561 | 558 | -3 |

Measured through actual PDP-6 assembler `das -F -C -L/-O` and
`pdp10-objdump -h`. Native PDP-6 suites `alloc-v090`, `free-v092`,
`gap-v094`, `noreclaim-v095`, `relocation-v099`, `vm-live-v102`,
`vm-live-v105`, and `vm-live-v106` all **PASS**. GCC KCORE/KINIT DXR
link **PASS**. These do not represent a complete boot or prove post-fold
KCORE size or worst-case cycle counts.

`mm_extent_insert` and higher-level algorithm alternatives remain separate
candidates. ITS-138/JOSS comparisons are limited to relevant techniques;
their block-granular allocator architecture cannot replace DAIMOS's
word-granular extent representation.
