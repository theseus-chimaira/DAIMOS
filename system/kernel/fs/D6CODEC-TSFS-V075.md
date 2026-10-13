# TSFS 1.3 transient validation stage v075

TSFS metadata is validated outside the resident filesystem: the userspace
MOUNT.TSFS scanner (`userland/tsfs/tsfs_scan.c`) and KINIT tape-root
scanner (`system/kernel/boot/tsfs_boot.c`) currently require format 1.1.
The staged v075 copies change the exact descriptor/TDIR version gate to
1.3, and the MOUNT scanner accepts stored extents (0) or inline extents
(2), rejecting legacy D6LZ (1). These are inactive alternate translation
units: no active userland or KINIT build recipe was replaced.

PDP-6 GCC compilation of both staged files passes using project include
paths (`daimos-testkit/tests/validation/d6codec-tsfs-v075`). They have
not been native-executed, nor has a complete TSFS 1.3 mount/read or
KINIT tape-root boot passed. Runtime v074 extent dispatch must be linked
atomically with these validators and v070/v067 decoder if activated.

Optimization remains deferred. KCORE 8K acceptance gate is unchanged.
