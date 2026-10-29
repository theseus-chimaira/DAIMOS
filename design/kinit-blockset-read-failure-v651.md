# KINIT BLOCKSET read-I/O error watermark v651

Native PDP-6 SIMH 96K GCC full-profile KINIT with the unchanged
`0600`-octal stack reserve: testkit
`daimos-kinit-blockset-read-failure-v651.sh` injects a storage read
failure without modifying production source or persistent images.

The test verifies CLEAN D6FS media and resolves `kinit_enter`,
`blockset_boot_read`, and the watermark from the linked maps. After
Stage1 finishes loading, SIMH replaces the in-memory read-service
entry with `SETO 1,` followed by `POPJ 17,`, forcing the normal
error return. Root selection still uses the physical disk read path.
KINIT loads D6FS, rejects the failing root mount with `?RT`, and HALTs.
High-water: `0172` octal, 122 decimal words, below 384 reserved.
Both media preflight and postflight are CLEAN. Native regression PASS.

This covers an unsuccessful BLOCKSET read during mount, not the full
range of physical-controller error handling. Missing/nonregular
`/SYSTEM/INIT` remains a separate pending failure-path gate.
