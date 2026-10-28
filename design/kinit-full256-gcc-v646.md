# Native GCC full-profile 256K KINIT acceptance v646

The fresh GCC full-profile disk-root image built with
`BOOT_CORE_WORDS=01000000` and `SIMH_CPU_KWORDS=256` PASS.
KINIT image end is octal `107421` with the unchanged `0600` stack
reserve. Native PDP-6 SIMH reports `MEM 256 K`, DSK270/DRM236 OK,
BSET/LOGSTR/MEMFS/D6FS loaded and reaches the linked
`proc_boot_init` breakpoint `063067` before any userspace runs.

Testkit `daimos-kinit-full256-gcc-v646.sh` independently repeats
this image-bound and native boot check. No production code changed.
Full-profile 96K and 256K GCC/KCC KINIT acceptance is complete;
interactive DSH and additional failure-path stack tests remain open.
