# PDP-6 memory granularity P0 source audit (v521, 2026-10-27)

Status: source inspection completed; implementation and native acceptance open.

## Actual units and mechanisms

- `02000` octal is 1024 decimal words. `01000` octal is 512 decimal
  words. Neither number implies a generic 2048-decimal-word allocator.
- `system/kernel/mm/mm.c`: `mm_alloc()` calls `mm_alloc_aligned()` with
  alignment **1**. Extents record exact lengths. Gaps, not per-quantum
  bitmaps, represent free space. `mm_alloc_aligned_noreclaim()` also honors
  caller-selected power-of-two alignment and exact requested length.
- `system/kernel/proc/vm.h`: `VM_EXTENT_ALIGN_WORDS` is `02000` octal;
  `system/kernel/proc/vm_pdp6.s` uses APR-dependent size rounding and
  relocation at that boundary. The backend cannot grant hardware-protected
  process mappings at a 512-decimal-word boundary on the existing PDP-6.
- `system/kernel/mm/mm.c` compacts movable process extents using
  `VM_EXTENT_ALIGN_WORDS`; changing just the compactor to 512 would violate
  process relocation invariants.
- Existing software allocations include variable-size mount slabs,
  process tables, u-areas, and 0200-octal-word cache allocations using
  unaligned `mm_alloc()`. Do not rewrite those as fixed 512-word slots.

## What remains to be measured before editing MM

1. Trace exact process logical sizes versus rounded APR physical extents
   across RUN/EXEC/BRK, resize, swap and XIP; compute aggregate rounding
   waste and actual allocation failures under 32K, 48K and 256K profiles.
2. Audit each provider, backing, MEMFS and startup MM request for avoidable
   alignment or size rounding; distinguish required DMA/device geometry.
3. Confirm any additional useful 512-word *software-only* suballocation
   within larger APR extents can preserve ownership, protection, pinning,
   isolation, relocation, rollback and descriptor RAM without new hot paths.
4. Implement only a proven safe reduction, add regression cases to
   daimos-testkit, measure permanent RAM, peak RAM and timing, and publish
   implementation documentation in pdp10-doc after acceptance.

Decision: the P0 optimization target remains active, but changing the generic
MM allocation quantum is **not** an implementation task: no such global
quantum currently exists. Do not misrepresent a software suballocator as a
512-word PDP-6 APR capability. No source code changes or savings are claimed.

## V523 native APR contract guard (2026-10-27)

The accepted policy is now native PDP-6 APR mapping at 1024 decimal words,
not smaller emulated user maps. `daimos-testkit` adds
`tests/validation/daimos-mm-apr-granularity-v523.py`: a source-contract
regression for exact-size unaligned MM allocation, VM and swap alignment,
BRK rounding, and process compaction. It passes on the current source.
This is not a physical fragmentation/pressure benchmark; no RAM saving or
new runtime implementation is claimed. V524: postpone representative
fragmentation profiling and optimization until BSTORE, production XIP and
eligible-image compressed swap have stabilized. Use current measurements
only to diagnose correctness failures. Preserve native 1024-word APR maps;
verify descriptor/pin/compaction/rollback invariants first, then profile
realistic 32K/48K/256K workloads and assess net RAM and CPU benefits.

## V525 native descriptor checks (2026-10-27)

Under daimos-testkit using the current DAIMOS checkout, both existing
PDP-6 step-execution tests pass:

- `daimos-mm-extents-v250.sh`: 32 descriptor entries, capacity rejection,
  insertion and shifted contents.
- `daimos-mm-pressure-v268.sh`: 132-to-136 descriptor pressure, rejection,
  middle removal and refill.

These establish descriptor-table arithmetic only. APR hardware isolation,
physical compaction, pin/release and transactional swap/BRK rollback still
require independent native runtime acceptance; no fragmentation optimization
or RAM-saving source change is warranted on this evidence alone.

## V526 native pin lifetime regression (2026-10-27)

New daimos-testkit `daimos-mm-pin-lifetime-v526.{s,sh}` extracts the actual
PDP-6 MM assembly and verifies missing-pin underflow, first pin, pin query,
refusal to free while pinned, saturated 255-pin rejection, last unpin,
wrong-owner free rejection and final valid free. The initial diagnostic
fixture mistakenly cleared the extent's type bit when setting a one-pin
value; corrected packed metadata to `[011,,07]` (one pin, type one, owner
seven) and `[03771,,07]` (255 pins, type one, owner seven). Native v526,
v250, v268 and v523 source-guard tests now pass. No production MM change
or RAM saving follows from these contract tests. Physical compaction and
APR process isolation remain separate native acceptance tasks.
