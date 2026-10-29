# KINIT controller media loss and diagnostic build-mode coherence v655

## Failure injection and native result

Native PDP-6 SIMH, 96K GCC full-profile disk-root boot, fresh KINIT
build with `KINIT_STACK_WATERMARK=1` and `KINIT_STACK_RESERVE_WORDS=0400`.
After Stage1 loads KINIT, the test breaks at the linked `kinit_enter`
and detaches DSK0, then resumes. The controller reports DSK270 OK,
but the missing root medium prevents complete filesystem initialization.
KINIT reaches its fatal HALT without publishing D6FS as loaded; no
`/SYSTEM/INIT` executes. The genuine stack watermark is `0172` octal,
122 decimal words, leaving 134 words inside the 256-word reserve.
Source disks stay CLEAN; the test mounts isolated writable copies.
Testkit `daimos-kinit-controller-media-loss-v655.sh` PASS.

## Build-system defect discovered and fixed

The preliminary v654 media-loss experiment reused the v649 build
directory after the v653 INIT validation had invoked `make image` with
watermarking disabled. KINIT C had been regenerated **without** its
`kinit_stack_watermark_begin`/record calls, whereas the older assembly
helper still had watermarking enabled with a `0600` reserve. Its
reported `0577` octal (383 words) was a sentinel-scan artifact, not
evidence of a 383-word stack. The observed pushdown pointer was near
the bottom of the stack, and the final reserved word contained
non-sentinel data. Do not use this inconsistent image as sizing proof.

The PDP-6 boot and kernel Makefiles now track a versioned build-mode
stamp keyed by both the KINIT reserve and watermark options. Switching
either invalidates generated C assembly and the assembly helper as one
unit. Testkit `daimos-kinit-mode-invalidation-v655.sh` verifies that
reusing a build while toggling watermark 1 -> 0 -> 1 correctly changes
**both** generated files. Native and build-mode tests PASS.

The `0400` reserve remains accepted on the measured failure paths.
Neither production KINIT instructions nor permanent KCORE RAM grow.
