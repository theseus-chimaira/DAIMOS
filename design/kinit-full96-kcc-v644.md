# Native KCC full-profile 96K KINIT acceptance v644

The separate KCC full-profile disk-root image builds successfully with
`BOOT_CORE_WORDS=0300000`, `SIMH_CPU_KWORDS=96`, and `COMPILER=kcc`.
Linked `__kinit_image_end` is octal `113573`, preserving the `0600`
octal KINIT stack reserve. Native PDP-6 SIMH reports `MEM 96 K`,
successful DSK270/DRM236 and loaded BSET/LOGSTR/MEMFS/D6FS, and
reaches `proc_boot_init` (octal `066510`) before any userspace startup.

Testkit `daimos-kinit-full96-kcc-v644.sh` verifies those conditions
with a map-derived breakpoint. No production code changed. This does
not constitute DSH, full end-to-end or 256K full-profile acceptance.
