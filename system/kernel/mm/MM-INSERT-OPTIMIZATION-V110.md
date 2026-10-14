# v110 — PDP-6 MM descriptor insertion optimization

Source baseline: v109 production `mm_pdp6.s`, whole MM `.text` 488 words,
static `.bss` 70 words. `mm_extent_insert` shifts two-word descriptors
**backward** to open a sorted insertion slot.

Previously each loop iteration computed two doubled indices, `(slot-1)*2`
and `slot*2`. The destination is always one descriptor (two words) later:
calculate only `slot*2` and read `mm_extents-2(4)` and
`mm_extents-1(4)`, then store at `mm_extents(4)` and
`mm_extents+1(4)`. The reverse overlap-safe copy order, capacity check,
count update, and public result contract remain unchanged.

Measured via native assembler `das -F -C -L/-O` and `pdp10-objdump -h`:

| Measure | v109 | v110 | Delta |
| --- | ---: | ---: | ---: |
| Whole MM `.text` | 488 | **485** | **-3** |
| MM `.bss` | 70 | **70** | 0 |
| Text + BSS | 558 | **555** | **-3** |

Actual PDP-6 native suites `alloc-v090`, `free-v092`, `gap-v094`,
`noreclaim-v095`, `relocation-v099`, `vm-live-v102`, `vm-live-v105`, and
`vm-live-v106` **PASS**. GCC KCORE/KINIT DXR link **PASS**. These tests
do not prove full operating-system boot, post-fold KCORE savings or
worst-case instruction latency.

Next opportunities include `mm_compact` validation and call setup and
further searches in `mm_find_fit`. ITS-138 and JOSS use different memory
representations, so their allocator implementations are references only,
not drop-in substitutes.
