# DAIMOS compaction/movement optimization census

Date: 2026-09-08

Base tree: `68265490a56b5a6cb9ac8cbb554feb9c43ef6232`

## Change

Process movement and movable-module movement no longer use compiler-generated C
word-copy loops.  Both paths call the existing fixed-KCORE `fs_copy_words()`
helper, whose PDP-6 implementation uses one `BLT` transfer.

The helper already existed for resident filesystem paths, so this introduces no
new resident helper code and no new state.

Semantics are unchanged:

- process moves still validate state/ownership/pins before allocation;
- process images remain moved only while transition state is set;
- module moves still copy the complete image+BSS+retained-relocation extent;
- module relocation-map walking and fixed/dynamic binding republish are unchanged;
- move accounting still charges the complete moved word count.

## Linked result

Fresh same-toolchain builds:

| Item | Pass 2 | Pass 3 | Delta |
| --- | ---: | ---: | ---: |
| KCORE DXR image | `014543` | `014520` | `-000023` / -19 words |
| KCORE BSS | `000560` | `000560` | 0 |
| relocation map | `000265` | `000264` | -1 word |

Besides reducing fixed resident code, actual compaction copies are materially
faster because PDP-6 `BLT` replaces per-word C loop control.

## Tests

The host MM harness now supplies a host implementation of `fs_copy_words()` so
its process movement, module movement, relocation, pinning and swap tests still
exercise the unchanged target call structure.

Green after the change:

- host `mm-v1`;
- real five-process PDP-6 scheduler regression;
- fresh 32K, 64K, 96K and 256K protected-user boot/EXIT;
- RAMFS policy boundary remains correct.

The next focused phase is swap optimization.
