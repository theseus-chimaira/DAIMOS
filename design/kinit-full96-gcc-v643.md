# Native GCC full-profile 96K KINIT acceptance v643

The `daimos-kinit-full96-gcc-v643.sh` regression builds a fresh native
GCC full-profile disk-root image with `BOOT_CORE_WORDS=0300000` and
`SIMH_CPU_KWORDS=96`. It asserts that `__kinit_image_end + 0600`
does not exceed the configured core, then runs PDP-6 SIMH to a
map-derived `proc_boot_init` breakpoint, before userspace starts.

Native PASS: KINIT image end `107421` octal; `MEM 96 K`, DSK270,
DRM236, BLOCKSET, D6FS, and MEMFS markers; breakpoint at `063067`.
The result validates the real 96K full-profile GCC kernel initialization,
not the later login/DSH workload or KCC variant. No production source
changes are needed.
