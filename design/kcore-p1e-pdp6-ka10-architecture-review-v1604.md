# P1-E v1604: PDP-6 / KA10 swapless PURE architecture review

Date: 2026-11-15. **Review completed; implementation NOT authorized.**
This review supersedes any assumption that the experimental v1381,
v1384, or v1391 PURE transactions may be linked directly into the
production scheduler. P1-C v1603 and P1-D v1601 stay CLOSED. No
production KCORE/MM/driver/ABI/format code changed during this review.

## Hardware facts and the current DAIMOS backend

* PDP-6: `system/kernel/proc/vm_pdp6.s` programs one resident user
  relocation/protection word via `DATAO 0000,vm_pdp6_apr` in
  `vm_activate_current`. The current `vm_state` stores user allocation
  words in LH and the physical base in RH. APR allocation and base
  use octal `02000` (1024 decimal) word granularity; there is NO
  independent hardware write-protected executable segment. The
  `proc_restore_user_tail` and `proc_restore_kernel_tail` branches
  in `proc_pdp6.s` depend on this precise backend. In particular,
  fatal VM_NONE executive teardown cannot attempt user APR restore.
* KA10: DEC's contemporary processor/reference descriptions permit
  contiguous low-segment relocation/protection and, with the
  appropriate KT10A option, a distinct high segment. Segment
  relocation/protection quantities also use 1024-word blocks.
  A high segment may be write protected; it must NOT be assumed
  installed in every KA10 configuration. These are KA10 hardware
  properties, NOT currently tested DAIMOS KA10 VM behavior.
* The PDP-6 kernel source provides no accepted `vm_ka10` resident
  restore backend or physical KA10 DAIMOS boot acceptance. Reusing
  the literal `vm_pdp6_apr` packed word, its `DATAO 0000`, or its
  private base-word arithmetic as a KA10 API is therefore invalid.

Primary DEC references consulted for the KA10 distinction:

* DECsystem-10 *Monitor Calls Manual*, 1974, section 1.2.1,
  `https://ftpmirror.your.org/pub/misc/bitsavers/pdf/dec/pdp10/TOPS10/DEC-10-OMCMA-A_D_Monitor_Calls_Ver_6.01_May74.pdf`
* DECsystem-10 *Technical Summary*, 1975, KA10 protection and
  relocation register discussion,
  `https://archive.computerhistory.org/resources/access/text/2024/09/102737527-05-0002-acc.pdf`

The target distinction must be carried into *functional* acceptance,
not inferred only from a successful cross-assembler invocation.

## What the original code actually implements

`proc_swap_pdp6.s` and `proc_pure_backing_pdp6.s` can omit full
sector-sized PURE text from ordinary **disk** swap and restore
immutable executable text from a validated vnode. This does NOT
provide swapless reclamation. `proc_swap.h` packs the executable
index, **14-bit** PURE length and mount identity in the existing
single swap-record word, with the fifth mount ID bit in
the stable u-area. `proc_storage_runtime_v30.s` provides the real
two-bit-per-slot RESIDENT/DISK/PACKED mode operations.

The unlinked experiments `proc_pure_ram_v1381.c`,
`proc_pure_restore_v1384.c`, and
`proc_pure_ram_restore_v1391.c` assume `VM_PDP6_BASE` and
`VM_PDP6_ALIGN_WORDS`. They model in-place MM descriptor rebase,
APR-prefix claim, full relocation restoration, and rollback under
a serialized `PROC_TRANSITION`. They do not own the actual
scheduler/PI6/EXEC/EXIT/WAIT, source durability or KA10 mapping
contract. Their isolated native success must not be called a
booted swapless implementation.

Review regression v1604 against the current checkout:

| Native PDP-6 check | Status | Exact significance |
|---|---|---|
| v1381 zero-copy in-place transaction | PASS | isolated MM/owner representation |
| v1384 prefix claim/restore transaction | PASS | isolated PDP-6 prefix placement |
| v1395/v1397 rollback/refusal | PASS | isolated pin/free failure handling |
| v540 production compaction -> APR -> runqueue | PASS | real current `vm_activate_current` and MM handoff |

The first three testkit runners had become invalid because the
original `proc_storage_runtime_v30.s` now refers to
`vfs_namespace_root`. All three were repaired to link the **entire
actual `system/kernel/drivers/device_state.s`**, which owns the
real zero-initialized alias at MonitorFS input-counter slot octal
`010`. No fabricated root word, stub classifier or kernel change
was accepted. Fresh repeated runs PASS. This is a test-fixture
link-dependency repair; it does not unlock P1-E production code.

## Recommended cross-target boundary (proposal, not approval)

**Common logical transaction, target-specific physical mapping.**

1. Eligibility/ownership stays in existing process storage mode,
   stable u-area, MM extent descriptors, immutable executable
   backing and VFS mount/provider guards. Avoid extra fixed-slot
   records unless measured savings require them. Treat any
   untrusted PURE bit as insufficient for admission.
2. Before drop: process must be stopped or asleep with saved user
   AC/PC/flags, no user-mapped argument, no conflicting PI work,
   no unresolved mount/FILE/XIP/PIPE/swap owner, and a validated
   executable whose source cannot be changed or removed. Hold the
   existing process transition and source lifetime while the image
   is not complete. Do not drop an actively running text segment.
3. The *representation* is machine-independent: original logical
   image size, exact immutable text boundary, complete retained
   mutable suffix, source identity/generation, storage mode and
   checked MM owner. Preserve partial-sector mutable words,
   heap/stack/BSS and saved AC context. The backing record's
   14-bit text limit remains a hard admission bound, not license
   to silently truncate larger images.
4. The *physical* operations are per-machine: APR alignment,
   contiguous extent reserve/rebase, user-segment limit, MM pin
   protocol, protection policy and saved context restoration.
   No portable caller programs APR I/O registers or assumes the
   meaning of PDP-6 `vm_state` RH outside its backend.
5. PDP-6 first: one contiguous user image, inactive RAW suffix,
   in-place owner-checked rebase only when every failure has been
   excluded **before** overwriting authoritative MM descriptors.
   Fall back to preserving the full resident image if no safe
   reduction or later full APR allocation is possible. Never
   invent a separate writable or protected hardware text segment.
6. KA10 initial compatibility target: **one low segment only**,
   only on configurations with verified relocation/protection
   hardware. Require a separately accepted KA10 `vm` activation,
   context, PI and MM test suite before an inactive representation
   is permitted. Optional KT10A high-segment text sharing or
   read-only protection is a **separate design**. It must not
   change the common process owner contract or be assumed present.
7. Restore: reserve the *complete* legal user mapping first,
   reconstruct text within a bounded destination from the guarded
   source, overlay the authoritative RAW mutable suffix, validate
   full word count, mapping geometry and MM ownership, then
   publish RESIDENT/RUNNABLE and program the target's registers.
   On OOM/short read/identity loss/MM-unpin ambiguity, keep the
   original inactive image and TRANSITION owner; never publish a
   partially restored user process. Fatal owned-media policy does
   not become recoverable because PURE exists.
8. Retain ordinary disk/drum swap exactly as presently accepted.
   Do not add mutable RAM compression or enable STRICT 32K
   optimization until net RAM consumption (including temporary
   peak, scratch, metadata, saved context and code) is proved.

## Acceptance before any production feature enable

All tests belong in `daimos-testkit`. Required next, in order:

* Source-linked actual PDP-6 scheduler/PI6 stop, transition and
  terminal EXIT/TERM/WAIT/EXEC replacement across PURE DROP and
  full restore; independent owner and MM unpin/free fault tests.
* 32K/48K genuinely swapless cold boots, measured actual available
  APR-aligned space and peak RAM; valid plain and compressed
  executable reload bounded to the reserved destination; corrupted
  headers, source removal/replacement, mount quiesce and changed
  backing generation rejected. Multiple inactive owners and OOM
  restore must not deadlock or drop the only retained suffix.
* KA10 system prerequisites: a verified target build and original
  KA10 single-low-segment APR/context boot, plus hardware-option
  detection and rejected unsafe high-segment assumptions. This
  test must observe actual target register semantics, not simply
  reuse a PDP-6 source fixture with a different `-x` flag.
* Repeat all previously accepted P1-C critical BACKSTORE, P1-D
  VFS/provider and process owner/PI/MM regressions. Measure final
  RAM/linked text growth before any optional compression decision.

## Decision and gate

**Architecture review DONE, design recommendation PROVISIONAL.**
The minimal safe common contract is an inactive RAW mutable-suffix
transaction with a machine-specific full-image restoration backend;
the KA10 high-segment facility is optional, not the first target.
**P1-E implementation remains on HOLD until this cross-target
contract is explicitly agreed and real KA10 backend acceptance is
planned.** No compression, no p10fold/p10super optimization, no
strict-32K acceptance and no claim of P1-E completion. P0-B
physical Type-551 reel-retention remains separately OPEN.
