# Native PDP-6 MM size baseline v093

Measured from production `system/kernel/mm/mm_pdp6.s`, DAIMOS v092
(`e6763af`), using `das -F -C -L ... -O ...` and `pdp10-objdump -h/-t/-d`.
Addresses and sizes are **octal** unless marked decimal. Each row counts
all assembler words from its entry label to the next major entry. Private
helper branches are charged to their enclosing algorithm. The assembler's
nine-word final literal pool is recorded separately.

| Algorithm | Entry address | Decimal words | Octal words |
| --- | ---: | ---: | ---: |
| `mm_delete` | `000000` | 19 | `0023` |
| `mm_extent_insert` | `000023` | 30 | `0036` |
| `mm_find_base` | `000061` | 10 | `0012` |
| `mm_is_pinned` | `000073` | 11 | `0013` |
| `mm_find_fit` | `000106` | 112 | `0160` |
| `mm_alloc_aligned_noreclaim` | `000266` | 73 | `0111` |
| `mm_move_extent` | `000377` | 66 | `0102` |
| `mm_compact` | `000501` | 62 | `0076` |
| `mm_alloc` | `000577` | 9 | `0011` |
| `mm_alloc_aligned` | `000610` | 65 | `0101` |
| `mm_free` | `000711` | 25 | `0031` |
| `mm_pin_adjust` | `000742` | 25 | `0031` |
| `mm_pin` | `000773` | 2 | `0002` |
| `mm_unpin` | `000775` | 2 | `0002` |
| Final literal pool (not part of `mm_unpin`) | `000777` | 9 | `0011` |
| **Total .text** | | **520** | **01010** |

**Static `.bss`: 70 decimal / `0106` octal words.**
The exported metadata layout consists of 64 words for 32 extent records,
three arena words, and three scalar words (`mm_core_words`, `mm_extent_count`,
`mm_arena_count`). **Total object text + static BSS = 590 decimal /
`01116` octal words**. This excludes managed physical arenas themselves,
allocated process memory, the stack and external dependencies.

The top five code sections (`mm_find_fit`, no-reclaim allocation,
`mm_move_extent`, `mm_alloc_aligned`, `mm_compact`) total **378 decimal /
`0572` octal words**, about 72.7% of the object text.

## Folding and comparison rules

This is **before folding**. The existing full KCORE fold plan does not fold
the fixed `mm.dobj` object; it folds eligible donor objects. An isolated
`p10fold -m 2 -P ... mm.dobj` inspection finds duplicate sequences, but
this is *candidate analysis*, not proof of realizable net savings: overlapping
sequences and additional JRSTs cannot be counted independently. No
post-fold reduction is credited to MM without rebuilding and measuring a
link in which the optimizer actually transforms its code.

The baseline is an accounting tool, not an endorsement of the current
algorithms. For each future MM algorithm alternative, report changes in
instructions, literal pool, permanent metadata, stack usage, native test
results, and full-KCORE post-fold size when applicable. Follow the permanent
v091 evaluation policy in `PROJECT-STATE.MD`.
