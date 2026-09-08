# DAIMOS C-level structural sharing census

Date: 2026-09-08

Base tree: `07b2980e1803ea89404fd756fc1927f068c7cc1c`

## Implemented sharing

Two fixed-KCORE duplication classes were worth sharing at C level.

### MM base lookup

`mm_is_pinned`, `mm_move_process`, `mm_move_module`, `mm_free`, `mm_pin`, and
`mm_unpin` each contained their own linear search for the extent whose physical
base matched an argument.  They now share `mm_find_base()`.

This keeps the same sorted descriptor representation, failure semantics, and
linear-time behavior.  No allocator policy changes and no extra permanent
state are introduced.

### Process slot cleanup

`proc_slot_release` and `proc_exit_finish` duplicated the same tail:

- detach swap metadata;
- release the stable u-area;
- clear the three-word process descriptor;
- trim `proc_high_slot` over trailing free slots.

They now share `proc_slot_cleanup()`.  EXIT still frees the movable user extent
and clears `proc_current_slot` before entering the shared cleanup path, so the
existing race and lifetime ordering is preserved.

## Linked result

Fresh same-toolchain production builds from the Pass-1 tree and this pass give:

| Item | Pass 1 | Pass 2 | Delta |
| --- | ---: | ---: | ---: |
| KCORE DXR image | `014634` | `014543` | `-000071` / -57 words |
| KCORE BSS | `000560` | `000560` | 0 |
| relocation map | `000267` | `000265` | -2 words |

The reduction is entirely fixed resident KCORE code/image; no dynamic table or
module capacity was reduced.

## Gates

Green with both changes present:

- host `mm-v1`;
- real five-process PDP-6 scheduler regression;
- 32K, 64K, 96K, and 256K protected-user boot/EXIT;
- RAMFS threshold behavior remains 32K/64K off and 96K/256K on.

The next phase is compaction/movement optimization.  The structural-sharing
pass does not alter compaction policy or swap semantics.
