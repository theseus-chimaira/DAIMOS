# KCORE P0-A v1547: actual shared DRM-root BACKSTORE failure HALT

Date: 2026-11-13. Actual 256K KCC PDP-6 guest, feature acceptance.

The v1541 full F07 swapped-process guest has been reproduced with
disposable `ROOT=drum BOOT=drm` instead of its earlier DSK270 root
and independent DRM236 BACKSTORE. The root's D6FS four-member DR0..DR3
image and the corresponding BLOCKSET VOLUME swap-tail region are
constructed from the normal production boot media. Fixed KCORE,
the v1541 BACKSTORE fail-stop gateway and selected-root classifier
remain unchanged.

The v1511 disposable driver observer selects a real owned Type-236
BACKSTORE WRITE descriptor and redirects its PI2 completion through
the original driver error path. The genuine PDP-6 HALTs at the
BACKSTORE-specific octal PC **003747** (v1515 entry `003740+7`),
not the selected-root HALT `003743`. Post-HALT physical memory
checks give `backstore_enabled=1`, `backstore_blocks_used=0320`
octal, and exactly one selected Type-236 PI2 error. Four pre-fault
samples had zero selected errors. Strict v1542 error/ownership
verifier plus nine corrupted-evidence mutations PASS.

The extra shared-media checks require boot switch octal 3, an
attached real DR0 root image, `vfs_namespace_root=060101000000`,
`device_region_map=400000000000` (DRM236), and nonzero mapped
`volume_data_blocks=075774`. Those prove the owned BACKSTORE
used the selected root's Type-236 drum VOLUME rather than the
independent auxiliary drum in the original v1542 test.

This is a **real hardware-simulator PI2 completion path with a
test-only selected error injection**, not natural physical drum
removal. It proves BACKSTORE-first loss while ROOT shares the same
device members and the system holds live backing ownership.
The opposite **ROOT-first fault with live BACKSTORE owners** is
still physically outstanding. Native v1546 already proves both
logical orders but controls the physical callback. P0-A remains
open for the final combined physical ordering case; do not claim
all root+swap shared-loss acceptance yet.

No new KCORE executable words, RAM, syscall ABI, disk format or
production fault workaround. Testkit reproducible script and raw
evidence SHA-256s:
`tests/validation/daimos-shared-drm-root-backstore-v1547.md`.
VERSION 0.5, 32K/optimization HOLD.
