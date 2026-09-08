# DAIMOS historical resident-memory sweep: expendable initialization

Date: 2026-09-08

Base tree: `c3ab97804e15e6e160427c28367fe1348f514f7b`

## Rationale

Small and virtual-memory operating systems commonly distinguish bootstrap-only
machinery from the permanent executive.  DAIMOS already discards KINIT after
boot, but process-table and swap-record initialization still lived in fixed
KCORE and ran only after the KINIT handoff.

This pass moves only that one-shot initialization machinery into KINIT.  It does
not change process-table or swap-record representation, scheduler semantics,
swap semantics, module relocation, or any normal runtime call path.

## Change

`proc_slots_for_core()` and `proc_boot_init()` move from `proc.c` to the new
KINIT-only `proc_boot.c`.  `proc_swap_boot_init()` moves from `proc_swap.c` to
the new KINIT-only `proc_swap_boot.c`.

KINIT allocates and clears the persistent process/swap metadata immediately
before its final handoff.  This placement is deliberately after filesystem boot
planning and other KINIT work, so the metadata does not reduce the transient
working space needed by earlier boot stages.

The runtime process/swap code and data stay in KCORE.  The swap-record pointer
and scheduler age phase become package-visible only so the disposable boot code
can initialize the existing runtime state; no permanent state is added.

## Measurement

Fresh same-toolchain PDP-6 links:

- previous `__kcore_low_init_end`: `014467`;
- optimized `__kcore_low_init_end`: `014323`;
- reduction: `000144` octal / 100 words.

`__kcore_low_end` falls by the same amount (`015247` to `015103`), so fixed BSS
is unchanged.  The saving is entirely one-shot executable code removed from the
steady-state kernel.

At 32K, the persistent tables become allocated while KINIT is still present,
but only at the final handoff.  The real 32K boot regression passes, so the
transient-memory margin remains sufficient.

## Gates

Green on the optimized tree:

- host `mm-v1`, including movement and swap coverage;
- real five-process PDP-6 scheduler/preemption/RL-PL regression;
- protected 32K, 64K, 96K, and 256K boot/EXIT;
- 96K RAMFS threshold behavior.

No runtime indirection, functionality loss, or slower steady-state path is
introduced.
