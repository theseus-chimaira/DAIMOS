# KINIT larger-memory detection regression v642

Testkit `daimos-kinit-memory-profiles-v642.sh` runs the same GCC
lowmem TSFS-root, watermark-enabled image with 48K, 96K and 256K
installed PDP-6 core in SIMH. The test stops at map-derived
`proc_boot_init` before userspace, checks the exact `MEM ... K`
banner and verifies DTC and DSK270 initialization. All three native
configurations PASS, including the 256K memory-probe upper bound.

This is KINIT lowmem-image core-size detection and early initialization,
not acceptance of the full-profile 96K/256K image or of DSH.
No DAIMOS production code changed. Full-profile KCC/GCC and userspace
functional tests remain separate gates.
