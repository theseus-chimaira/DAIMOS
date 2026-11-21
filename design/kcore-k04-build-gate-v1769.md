# K04 acceptance build gate v1769

Status: isolated full `system` build PASS. Physical K04 fault-injection
acceptance remains OPEN; external testkits were not run or modified.

After the v1768 parent/child ownership changes, the isolated PDP-6
assembly passed but a full `make -C system build` initially stopped
before kernel acceptance. All five production PDP-6 Stage-1 Makefiles
(DSK, DTC, DRM, MTC, PTR) still declared a dependency on the deleted
`common/decompressor.inc`, although their Stage-1 sources actually
include `common/block_decompressor_v1458.inc`.

v1769 corrects only the stale Makefile prerequisites, matching the
existing source include and preserving the block decoder and media
format. No source-level decompression changes, resident RAM changes,
or runtime workarounds were needed.

Validation: a fresh isolated `make -C system build` using a versioned
`$TMPDIR` build root and the existing PDP-10 toolchain completed,
including the five Stage-1 targets. This establishes a full-system
build baseline for future K04 emulator acceptance, but does not
demonstrate successful boot, fault injection, or WAIT/PID reuse.
