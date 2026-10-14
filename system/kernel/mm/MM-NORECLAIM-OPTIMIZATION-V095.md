# v095 — No-reclaim allocator necessity and size audit

The `mm_alloc_aligned_noreclaim` entry is **required**: resident callers
include packed-process eviction (`proc_packed_evict_v077.c`), restoration
(`proc_packed_restore_v46.c`), native block-cache allocation
(`bcache_pdp6.s`), and ordinary MM's internal bounded retry. It provides
exact-size allocation *without invoking* recursive swap/filesystem reclaim
or compaction, which is essential while the source process is pinned.
Replacing it with `mm_alloc_aligned` changes failure and reentrancy
semantics. A merged implementation would need an extra policy parameter and
would burden the critical non-reclaim path, with no proven size benefit.

The native PDP-6 implementation uses a sorted descriptor table, so the
allocation policy is a small search and insertion. No additional free-list
or lookup index is justified by this stage's evidence.

Two 3-word full-word validation checks (`MOVE` + `AND` + `JUMPN`) are
replaced by equivalent 2-word `TDNE` + `JRST` pairs. Preserve all 36-bit
invalid values, not only the right-half bits. The first attempted
control-flow simplification yielded zero assembled savings and was discarded.

| Measure | v094 | v095 | Delta |
| --- | ---: | ---: | ---: |
| `mm_alloc_aligned_noreclaim` | 73 words | 71 words | -2 |
| Full MM assembler `.text` | 498 words | 496 words | -2 |
| Static MM `.bss` | 70 words | 70 words | 0 |
| Combined MM text + BSS | 568 words | 566 words | -2 |

Measured with `das -F -C -L ... -O ...`; literal pool included in text.
The native PDP-6 `noreclaim-v095`, `gap-v094`, `alloc-v090`, and `free-v092`
regressions all PASS. GCC KCORE and KINIT DXR links PASS. These tests do not
prove end-to-end scheduler packing or full-OS boot. Fold savings beyond the
assembled word delta are not claimed.

Further opportunities to assess separately: an optional policy unification
only if its real post-fold size is smaller; descriptor-slot search and
insertion coupling; reducing frame scratch/AC live ranges; and algorithms
for handling descriptor exhaustion without sacrificing memory safety.
