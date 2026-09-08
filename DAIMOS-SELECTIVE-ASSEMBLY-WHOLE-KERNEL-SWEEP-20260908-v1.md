# DAIMOS selective assembly and whole-kernel sweep

Date: 2026-09-08

Base tree: `fedfd523a449f7b3f3f432ebb8de33358ad599e6`

## Selective assembly measurement

The movable-module relocation-map walker was evaluated as the remaining obvious
PDP-6 assembly candidate.  A dedicated sequential two-bit-map helper reduced a
fresh linked KCORE low-init end from `014517` to `014506`, a saving of only
`000011` / 9 words.

Although the helper also shortened the per-word relocation path, module movement
is a cold compaction operation.  The project rule requires a substantial size
or speed win for new assembly and specifically rejects single-digit word
savings.  The helper was therefore not retained.  The existing BLT copy/zero
helpers remain the only assembly used by this optimization series.

## Whole-kernel sweep

Three resident C zero loops still duplicated the existing `fs_zero_words()`
BLT primitive:

- dynamic process-table initialization;
- per-process u-area initialization;
- freshly allocated executable image initialization.

All three now call the shared primitive.  This reduces code and speeds the
potentially large clears without changing allocation, process, or executable
semantics.

Fresh same-toolchain links give:

- Pass-4 KCORE low-init end: `014517`;
- optimized KCORE low-init end: `014467`;
- reduction: `000030` / 24 fixed words.

The KCORE low end falls by the same amount, from `015277` to `015247`, so no BSS
was added.

Relative to the Pass-4 approximate normal resident low set of 11,219 words, the
normal resident set is now approximately 11,195 words.  Dynamic process/swap
metadata sizing is unchanged by this pass.

## Gates

Green on the final tree:

- host `mm-v1`, including movement and PURE/UNKNOWN swap coverage;
- real five-process PDP-6 scheduler/preemption/RL-PL regression;
- fresh protected 32K, 64K, 96K, and 256K boot/EXIT;
- 96K RAMFS threshold behavior;
- DAS relocatable negative-addend regression;
- direct-UUO encoding/DXR regression;
- DXR2 validation of the linked KCORE image.

No assembler/compiler workaround was introduced and no new assembly was retained.
