# D6FS free-FCB bound correction (2026-11-16 v1)

Status: **ACCEPTED**, native PDP-6 acceptance PASS.

## Failure and root cause

A fresh D6FS boot image contained 295 populated nodes and 360 FCB
slots, but both MKDIR and `OPEN(O_CREAT)` failed under native DAIMOS.
The root was mounted read-write. Existing-file OPEN for writing and
CHMOD succeeded, eliminating basic path, permission and metadata-write
failures. Temporary diagnostic returns identified the allocator's
`d6fs_provider_free_fcb` failure; a separate probe confirmed a vacant
FCB at index 300 could be used.

In `system/kernel/fs/d6fs_provider_mutation.s`, the allocator compared
its candidate index (AC011) against `011(5)` of `d6fs_active_reader`.
`struct d6fs_reader` word `011` is `super_block[0]`, an on-media block
number, **not** the FCB count. The allocation limit must be word `6`
(`fcb_count`). This is a field-selection bug, not an exhaustion policy.

Fix: substitute `caml 011,6(5)` for `caml 011,011(5)`. The FCB scan
still starts at index 1. No instruction-count, static RAM, file format,
or allocator policy change is necessary.

## Verification

The independent `daimos-testkit` target
`target-d6fs-free-fcb-pdp6-v1` executes the production allocator on an
original PDP-6, injecting controlled free/live FCB decode results.
Its four cases cover a superblock below the FCB table limit, a
superblock above it, an early reusable slot, and an empty table.

The native DAS boot/assembly acceptance with the corrected scan
executes `chmod`, MKDIR/RMDIR, scratch-file CREATE, DAS1 and DAS2,
exact DXR word comparison, and execution of the assembled image.
The original PDP-6 guest and testkit report PASS.

The distinct crash-atomic D6FS CREATE/recovery design remains open;
this correction neither introduces nor certifies crash recovery.
