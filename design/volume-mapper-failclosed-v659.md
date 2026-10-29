# VOLUME mapper failure must not fall back to DEVICE addressing (v659)

The permanent `fs_mres.s` backing I/O gateway previously used direct
single-device translation whenever `blockset_runtime_service_jump`
pointed to `fs_mres_no_service`, even if a multi-member VOLUME was
configured. This silently selected physical sectors on the wrong
device. A configured VOLUME now returns -1 before any raw transfer
when its mapper is unavailable. A direct DEVICE range (zero logical
VOLUME blocks) still bypasses the mapper and uses the fast path.

Source routing tests v658/v659 PASS. Fresh 32K lowmem GCC D6FS/disk
image builds; native PDP-6 SIMH boot reaches `proc_boot_init`, with
BSET/D6FS loaded, and its D6FS checker reports CLEAN. The missing-
mapper path is statically verified but not yet fault-injected on the
native simulator. Full multi-member failure regression remains open.

This is a correctness/feature-boundary fix, not a size optimization.
