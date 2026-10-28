# Native KCC full-profile 256K KINIT acceptance v645

Fresh KCC full-profile disk-root build with `BOOT_CORE_WORDS=01000000`
and `SIMH_CPU_KWORDS=256` PASS. Its KINIT image ends at octal
`113573`, retaining the octal `0600` stack reserve.

Native PDP-6 SIMH reports `MEM 256 K`, DSK270 and DRM236 OK, and
BSET/LOGSTR/MEMFS/D6FS loaded. It stops at the map-derived
`proc_boot_init` (`066510`) before executing any userland code.

Testkit `daimos-kinit-full256-kcc-v645.sh` verifies those results.
An initial test assertion rejected the valid 256K banner due to
different padding; it was corrected to a whitespace-tolerant match
and the complete regression then PASSed. No production changes.

Interactive DSH, 256K GCC full-profile and remaining failure-path
stack-watermark measurements are separate outstanding gates.
