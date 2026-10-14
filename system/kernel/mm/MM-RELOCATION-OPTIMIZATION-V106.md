# v106 — First measured MM relocation optimization

Base: v105 production `mm_pdp6.s`, `mm_move_extent` 66 words, MM text 493
words and BSS 70 words. A live alternative was assembled and tested using
actual production PDP-6 MM/VM and word-copy assembler, not a host simulation.

The saved source extent contains length in the left half and physical base
in the right half. After successful `vm_extent_move`, only base changes.
Replace `HRLZ` + `HRR` + `MOVEM` (3 words) with `HRRM` (1 word). This
preserves the saved left-half length without reconstructing it.

The new insertion search starts at the original descriptor slot instead of
zero. Since this function only publishes successful **upward** moves, its
new sorted insertion position cannot precede the original slot. This saves
runtime traversal where the slot is nonzero, with no assembled word-count
change. The current test suite has verified multiple movable extents and
mixed pinned obstacles; no cycle-count speedup is asserted.

| Measure | v105 | v106 | Difference |
| --- | ---: | ---: | ---: |
| `mm_move_extent` | 66 | 64 | -2 words |
| Entire MM `.text` | 493 | 491 | -2 words |
| MM `.bss` | 70 | 70 | 0 |
| MM `.text` + `.bss` | 563 | 561 | -2 |

Measured with `das -F -C -L/-O` and `pdp10-objdump -h`.
The v106 standalone PDP-6 test verifies relocated descriptor length is
preserved. The v099, v102–v106 relocation and v090/v092/v094/v095 allocation
suites PASS. GCC KCORE/KINIT DXR link PASS. These results do not certify
full-OS boot or concurrent scheduler execution, and no post-fold whole-KCORE
benefit beyond measured assembler words is claimed.

ITS-138 and JOSS offer no drop-in replacement for the word-granular
relocation transaction. Larger rewrites of delete/search/restore and final
insertion remain possible but require proving rollback, PI and net code/RAM
benefits, not merely moving code between helpers.
