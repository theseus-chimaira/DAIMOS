# KCORE v1606: finish PDP-6 before another processor

Date: 2026-11-15
Status: PDP-6 source-contract clarification; no hardware port.
The v1605 build-time CPU source map remains the sole selector.

## Decision

Complete, stabilize and physically test PDP-6 KCORE first. In
particular, P1-E uncompressed swapless PURE reclaim/restore and
all its ownership/PI/EXEC/EXIT/WAIT gates must be accepted before
any KA10/KI10 implementation work begins. Optional in-RAM mutable
compression follows only if measured whole-system benefit warrants it.
P0-B hardware ownership and the strict 32K/optimization HOLD remain
separately tracked. No KA10 or KI10 implementation schedule or
milestone is authorized by this document.

Prepare PDP-6 code as if a second source backend will be substituted
next, but avoid introducing code that exists only for that backend.
Selection remains link-time; never add a runtime CPU switch, page
table, second process state registry, or extra call to a PDP-6 hotpath.

## Minimal replacement boundary

| Owner | Common contract worth retaining | Current PDP-6 implementation |
|---|---|---|
| CPU control | Scheduling policy, wait/event/TTY/EXIT | proc_pdp6.s mixes these with user/executive context, PI6 and privileged state |
| Interrupt/trap | Registered service ownership and syscall selectors | kcore_pi_pdp6.s, mach_user_pdp6.s, KINIT PI/UUO setup |
| Virtual memory | Logical mapping and complete, atomic activation | vm_pdp6.s, vm_pdp6_swap.c, vm_pdp6.h |
| Physical storage | MM ownership, allocation failure and rollback | mm_pdp6.s: contiguous two-word, 18-bit physical descriptors |
| Swap/PURE | Victim/owner/immutable backing policy | proc_swap_pdp6.s and PDP-6 whole-image, prefix rebase/restore |
| Executables | DXR validation, admission and rollback | exec_load.s combines policy with base+020 and pinned APR XIP |
| User buffers | Checked logical user reads and writes | syscall_dispatch_pdp6.s, syscall_info.s, monitorfs_runtime.s still require physical pointers in some paths |
| Device/MRES | Filesystem policy, resource ownership, IO failures | CPU PI, physical DMA and relocation boundaries are target-specific |

PDP-6 continues using the existing direct source call sites; a
future CPU does not have to copy all scheduling or filesystem
policy to replace hardware machinery. However, source grouping is
not equivalent to an already portable ABI. The current CPU process
and disk-swap files contain mixed policy and machine operations.

## Explicitly NON-generic assumptions to keep visible

1. The RH of `proc.vm_state` is a PDP-6 relocation base, not a
   portable physical address, page-map pointer, or presence flag.
   `VM_SPACE_ACTIVE` currently examines that RH; on a different
   mapping representation the backend must supply its own active
   test without changing the three-word process descriptor merely
   for anticipated hardware.
2. `VM_SPACE_WORDS` is a logical size, not proof that every word
   below it has the same permissions, backing, or one contiguous
   physical address. Holes and segment boundaries must not be
   hidden by a successful size check.
3. A pinned mapped user span returned by `vm_user_words` is
   presently a physical PDP-6 pointer. Future mapping may only
   permit a shorter safe span or require backend-owned copying.
   Do not generalize a long-lived physical pointer to all CPUs.
4. `mm_extent` records an 18-bit physical base and an aligned
   contiguous region. The kernel/device allocator need not become
   a frame allocator. Any wider or scattered physical user page
   ownership must be represented separately if actually needed.
5. Packed PURE state records *private writable bytes plus immutable
   reconstruction ownership*, not necessarily an APR-aligned suffix.
   PDP-6 zero-copy rebase and full APR reconstruction remain valid
   and local to PDP-6. Other mapping forms must not release code
   backing still shared by another owner.
6. Fast-AC, low-core/trap, executive access, PI and physical DMA
   requirements are CPU- and device-specific. A new VM backend
   alone cannot make KINIT, the scheduler, MRES or I/O portable.

These are portability constraints, NOT new public runtime functions
or a proposal for a multi-CPU framework.

## Which sources to reorganize now

The v1605 source map has already extracted the one safe, useful
build-level selection boundary and kept the PDP-6 link identical.
Retain whole `vm_pdp6.s`, `kcore_pi_pdp6.s` and `mm_pdp6.s`
as replaceable files. Do not split them purely by line count.

The remaining worthwhile *potential* source split is in
`proc_pdp6.s`: machine context and PI6 leaves versus scheduler,
signals, TTY, WAIT/EXIT and process policy. Similarly,
`proc_swap_pdp6.s` joins pressure policy to physical transfers.
Both have numerous testkit fixtures that directly inspect and
mutate source labels. A split without complete native and guest
acceptance would add maintenance complexity for no PDP-6 benefit.
**Do not perform either split now.** Retain their source-map
ownership and document exact hardware leaves when implementing
or testing a real PDP-6 feature that already touches those paths.

Fix true abstraction leaks (compressed DXR destination, MEMFS XIP
physical mapping, MonitorFS/INFO raw VM reads, storage-mode
interpretation and swap transfers) only when doing their real
PDP-6 work or a future backend integration. Avoid speculative
user-copy dispatch or per-word VM wrappers.

## Acceptance rule for any PDP-6-only cleanup

- Compare a fresh full PDP-6 KCC KCORE DXR and map against the
  exact source-matched baseline. No additional permanent TEXT,
  DATA, BSS, per-process memory, or hotpath call is acceptable
  for a portability-only change.
- Keep fixed link order, `kcore_end` last, PI low-core aliases,
  process/u-area layout and executable/syscall ABI unchanged.
- Place new tests in daimos-testkit and repair all affected
  exact-source extraction/mutation fixtures before a source move.
- Validate actual PDP-6 process switching, PI6 wake, APR/BRK,
  EXEC/EXIT/WAIT, swap/MM and PURE under hardware pressure before
  marking P1-E or any physical ownership path accepted.
- No subsequent CPU implementation is in this milestone. The
  existing source manifest is sufficient preparation for now.

Historical hardware compatibility distinctions are discussed in
`design/kcore-multi-cpu-portability-v1605.md` and in pdp10-doc,
`KERNEL/MULTI-CPU-PORTABILITY-V1605.MD`. Their CPU-specific
examples are context, not current implementation tasks.

## Verified v1606 evidence

The full KCC PDP-6 `ROOT=tape` `BOOT_PROFILE=full` linker
output and KCORE symbol map were compared with the original
v1603/v1605 baseline: both files are byte-identical. Baseline
and new DXR SHA-256:
`830559842b15985c986b9145437d193cffacb7b8b654a7fb14f437788ce646ae`.
The v1605 daimos-testkit source-map check PASSes with the
new comments and still rejects `DAIMOS_MACHINE=ki10`.
No new independent PDP-6 user-space behavior was introduced.
