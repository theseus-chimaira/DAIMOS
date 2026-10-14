# Packed registry default linkage v087

The previous DAIMOS_PACKED_DEV_V086 build-time switch has been removed.
KINIT now unconditionally prepares the process-packed registry after
proc_boot_init. Its persistent runtime descriptor is defined in permanent
KCORE via proc_packed_runtime_v086.c rather than in reclaimable KINIT.
The v038 allocator remains transient and KCORE owns the descriptor and
layout routines. A fresh GCC PDP-6 full KCORE/KINIT DXR link succeeds.

This replaces the earlier experimental boot configuration; it does not yet
activate the packed-process scheduler, native encoder/decoder transaction
or v084 VM teardown in the full kernel. The DISK BACKSTORE mode and its
implementation are still needed and must not be deleted just because
PACKED RAM is available. End-to-end boot and lifecycle validation are
required before removing superseded swap dispatch/codec files.

Resident size limits remain deferred at the user's request.
