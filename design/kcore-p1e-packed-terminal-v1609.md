# P1-E v1609: production PDP-6 terminal RAW PACKED owner cleanup

Date: 2026-11-16
Baseline: v1608 source admission and v1607 remaining-work checklist.
Status: production KCORE owner cleanup implemented and native-tested.
RAW PURE reclaim, runnable restoration, actual scheduler/PI6 guest and
no-backing boot remain OPEN.

## Why this increment precedes live reclamation

In v1608, production `vm_space_destroy` only admitted RESIDENT or
DISK modes. A future PACKED/RAW process with a zero APR base and an
executable backing record could not finish EXIT. Calling the existing
`proc_swap_detach` directly would instead mistake that executable
record for a disk-run descriptor and invoke backstore_free on the
wrong owner. Enabling live RAM eviction without correcting terminal
teardown first would strand or corrupt process ownership.

## Implemented in KCORE

- `vm_space_destroy` now accepts storage mode PACKED (2) **only**
  when the VM base is zero, just like the nonresident DISK gate.
  The normal vm_space_destroy mode check still rejects invalid mode
  3 or inconsistent live VM and does not bypass resident XIP handling.
- `proc_swap_detach` inspects the existing storage mode before
  interpreting its one-word record. PACKED enters the machine-
  specific `pdp6_pure_detach_v1609` leaf; RESIDENT and DISK take
  the historical detach path.
- The new assembly leaf validates TRANSITION, a real stable u-area,
  zero APR base, matching saved executable backing, positive
  aligned original VM length and sector-rounded PURE prefix.
  It then finds **exactly one** unpinned MM_TYPE_PACKED_PROCESS
  extent with the exact process-slot owner and writable suffix
  word count, rejecting any competing PROCESS or PACKED extent.
- `mm_free` commits only the validated physical packed extent.
  A successful free publishes PACKED -> RESIDENT in the existing
  two-bit mode map; a post-free mode mismatch HALTs rather than
  releasing an uncertain owner. The existing C code then clears
  the one-word backing, saved u-area backing, and fifth mount bit.
  Finally vm_space_destroy publishes VM_SPACE_NONE.
- The terminal operation is deliberately independent of a live
  executable mount, file provider, BACKSTORE bitmap or fresh
  swap allocation. Failed scans/free retain the exact PACKED
  extent/mode/record for a TRANSITION-held retry.

This is a PDP-6 contiguous-core operation selected through
`DAIMOS_VM_PURE_DETACH_SOURCE` in the existing CPU source map.
No second per-process registry, new mode, dynamic CPU dispatch,
new persistent DATA/BSS or disk-format change was introduced.

## Why the assembly leaf

An initial correct but compiler-expanded C MM descriptor scan grew
the linked KCORE low end by **169 words**. This was rejected for
the PDP-6 memory objective. An equivalent, directly reviewed
PDP-6 assembly scan reduced linked growth to **104 words**,
while preserving exact type/owner/pin and record validation.
The source-map and final linked KCC image were then checked for
identical output across build-manifest cleanup.

Full KCC ROOT=tape KCORE measurements:

| Metric | v1608 | v1609 |
|---|---|---|
| `__kcore_low_end` octal | `024364` | `024534` |
| Permanent increase | baseline | `0150` octal = 104 words |
| Per-process RAM | unchanged | unchanged |
| New permanent DATA/BSS | none | none |
| CODEC or RAW reclaim in scheduler | none | none |

DXR SHA-256 for the final v1609 source:
`ce1c003e6541976a1df6b233a8f06a1f5cf225ae52486964c045a12f158d06f0`.
This is a production feature cost, not a claimed 32K optimization.
The strict 32K/optimization HOLD remains in force.

## Native PDP-6 acceptance

`daimos-testkit/tests/validation/pure-ram-teardown-v1609/run-v1609.sh`
links the **actual** C `proc_swap_detach`, the **actual**
`vm_space_destroy` assembly excerpt, the compact new assembly
leaf, real storage-mode operations, and controlled MM/provider
failures in a PDP-6 native guest. PASS:

- terminal PACKED succeeds when executable source/mount is absent;
- exactly one MM free and PACKED -> RESIDENT publication;
- VM_SPACE_NONE and backing/mount-extension cleared once;
- double VM destroy is idempotent without a second MM free;
- missing TRANSITION, mismatched backing, bad extent length/type,
  pin, duplicate process owner, refused MM free and nonzero APR
  base all refuse without discarding the saved owner;
- no attempt to interpret PACKED as a disk block run;
- a private test-only mutant deleting the saved-backing equality
  check is rejected by the executing native guest.

The old v880 disk detach, v957 resident/disk detach order, v1129
VM_NONE/EXEC idempotence, and v1605 CPU source selector still PASS.
Staged v1384 fast-prefix restore now also applies the v1608
immutable-source mount admission before claiming any MM prefix.
Native v1384, v1385 and v1395/v1397 prefix/rollback suites PASS.

These are compiled source-linked/isolated native tests plus a full
KCORE build. They do **not** constitute the remaining physical
scheduler/PI6 no-swap acceptance matrix.

## Next actual P1-E work

1. Bind a provably no-BACKSTORE profile and immutable backing
   lifetime to a real slot-0 scheduling/reclaim entry.
2. Connect the already staged v1381 zero-copy drop and v1384
   fast/v1391 bounded full restore, while keeping all fallible
   filesystem work outside PI6 critical sections.
3. Classify runnable nonresident owners by authoritative storage
   mode. DISK continues to use the established disk-swap service;
   PACKED must be completely restored **before** APR activation
   or READY_ONCE publication.
4. Test actual RUN, EXEC, EXIT, WAIT, BRK, unmount, OOM and MM
   rollback under physical 48K and larger PDP-6 guests; report
   exact reclaimed RAM, peak restore memory and MRES footprint.
5. Only once RAW P1-E is accepted, decide if bitmap mutable-state
   compression produces a positive net whole-system RAM benefit.

The v1609 cleanup closes the EXIT-side prerequisite but does
**not** enable an inactive RAM process to be produced by the
production scheduler.
