# v1670: physical KINIT allocator-event RAM high-water, 48K/64K

Date: 2026-11-18. Accepted parent DAIMOS v1669:
`e2c1d0d68eb1e46e9a7b6b27f79863ce10d9cc91`.

## Architecture: trace outside guest, no permanent RAM

DAIMOS v1668 removed the unused DISK SWAP MRES package from an
explicit TSFS swapless-only KINIT image, saving 1877 GCC lowmem
linked words. v1669 measured one live KINIT_LATE *pre-reclaim*
checkpoint, showing that total MM free improved by only 66
words there while the largest contiguous free run grew by 1877.
Neither result alone described the highest allocator occupancy
across the boot sequence. v1670 addresses the remaining KINIT
interval without adding instrumentation to fixed KCORE or KINIT:
all trace code lives in daimos-testkit and runs in host SIMH.

The source-matched builds are GCC lowmem `ROOT=tape` +
`DAIMOS_PURE_MRES_ENABLED=1`, with
`DAIMOS_SWAPLESS_ONLY_V1668=0` (general) or `=1` (strict), and
`DAIMOS_PBM_V1667=0` in both. Each genuine PDP-6 image runs at
48K and 64K actual simulator core, with four separately copied
DECtape media and no attached DSK/DRM auxiliary backing.
Original boot code chooses and installs PURE. No second MM
backend, process layout change or compressed mode3 is enabled.

## Exactly defined measurement interval and invariants

The first retained physical sample occurs at `mres_init`, after
`mm_boot_init` has initialized the arenas, but BEFORE permanent
MRES installation. The last sample is the first instruction of
`vm_enter_initial_user`, after late KINIT has published its own
source/image/stack ranges to MM and created the first initial
user VM. Sampling uses SIMH breakpoints and `examine` commands at
actual original guest linker-map addresses:

* `mm_extent_insert`: process/module/dynamic allocation insert;
* `mm_boot_reserve`: MRES conversion from temporary MM owner to
  permanent unmanaged memory;
* `mm_add_free`: real KINIT source and stack arena publication;
* `mm_free`: exact allocated owner release;
* `mm_process_shrink_pinned`, `mm_process_grow_pinned`,
  `mm_compact`: paths that could alter MM extent size/movement;
* `mres_init` and `vm_enter_initial_user`: unambiguous start/end.

For each boundary, the simulator reads real `mm_core_words`,
`mm_extent_capacity`, `mm_arena_count`, all three `mm_arenas`,
`mm_extent_count`, the first 40 packed two-word `mm_extents`,
and `sys_resident_words_immediate`. The source-linked host oracle
requires valid core-size/capacity, exact sorted nonoverlapping
MM descriptor ownership, permitted owner types, arena bounds,
and conservation of free+allocated words in each managed arena.
It rejects missing start/end, corrupt physical data, stale
link-map references and a forged high-water total. An unexpected
compaction/VM-in-place sizing entry aborts the accepted trace,
because those paths require separate accounting beyond this
KINIT source interval. The actual four traces contain NONE of
those events. No allocation instrumentation changes the guest.

The maximum recorded physical `core_words - free_mm_words` is the
**stable allocator-event-boundary high-water**. It includes
occupied/reserved transient KINIT and permanent low memory, not
just assigned dynamic MM owners. It is NOT claimed to cover
instruction-level intermediate states within a mutation call or
any later user process-pressure episodes.

## Measured native PDP-6 source-matched results

| Physical RAM | General events | Strict events | High-water event |
| --- | ---: | ---: | --- |
| 48K | 53 | 52 | `mres_init` |
| 64K | 53 | 52 | `mres_init` |

Both general and strict images record 20 extent insertion
entries, 12 boot-reservation entries and one first-user entry.
General has 19 free-arena publication entries after MM-ready,
strict has 18. No ordinary `mm_free`, process resizing or
compaction entry occurred within these KINIT runs. The physical
largest nonfree sample occurs at the initialized `mres_init`
boundary in all four simulations, not during late INIT creation.

| Physical RAM | General minimum free | Strict minimum free | Gain |
| --- | ---: | ---: | ---: |
| 48K | 11944 | 13821 | **1877 words** |
| 64K | 28312 | 30189 | **1877 words** |

| Physical RAM | General max nonfree | Strict max nonfree | Reduction |
| --- | ---: | ---: | ---: |
| 48K | 37208 | 35331 | **1877 words** |
| 64K | 37224 | 35347 | **1877 words** |

The 1877-word v1668 GCC lowmem *image* saving is thus observed
as exactly **1877 fewer occupied/reserved core words at the
sampled KINIT memory high-water** in physical 48K and 64K
PDP-6 guests. This is stronger than the v1669 one-checkpoint
linker-size observation, yet remains specifically bounded by
the KINIT MM event sampling contract.

At first `vm_enter_initial_user`, both general/strict profiles
have identical MM free, resident and process allocation state:

| Physical RAM | Free MM in BOTH | Low resident in BOTH | MM allocations in BOTH |
| --- | ---: | ---: | ---: |
| 48K | 26421 | 17725 | 4958 words / 8 extents |
| 64K | 42773 | 17741 | 4974 words / 8 extents |

The allocated extents contain one 4096-word process APR and
862/878 words of kernel-dynamic owners at 48K/64K. The matching
terminal free totals show that the transient candidate pruning
does NOT produce net extra free memory after the KINIT image is
fully reclaimed, because installed PURE residency is unchanged.

## Reproduction and remaining work

`daimos-testkit/tests/validation/pure-kinit-peak-v1670/`
contains the versioned physical runner, map-driven simulator
probe generator, full-event conservative MM decoder, source-
matched comparator and deliberate unsafe measurement variants.
Both per-core pairs pass; corrupt arena values, missing final
handoff, and forged high-water summaries are independently
rejected by executed tests. Four real SIMH console transcripts,
physical JSON traces, independent core-size media copies, and
source-built linker maps are retained in the evidence archive.

Still open: physical *instruction-level* micropeak accounting
within each MM transaction, KINIT stack-watermark pressure,
complete post-user RUN/EXIT/WAIT/PI6/BCACHE/VM allocation loads,
real bitmap mode3 compression admission, and strict 32K physical
geometry. No DAIMOS kernel production code changed in v1670.
