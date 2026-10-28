# Native 96K/256K full-profile DSH acceptance v647

The four native PDP-6 SIMH full-profile disk-root configurations
(`gcc`/`kcc` at 96K/256K) now pass complete ROOT login, DSH startup,
an `ECHO` command, external `TRUE` followed by `STATUS:0`, and an
orderly `HALT`. A host-side D6FS check confirms CLEAN media after
each shutdown. Testkit `daimos-dsh-full-profile-v647.sh` is the
repeatable regression; it uses independent copies of disk/drum media
and rejects sources whose D6FS root is not CLEAN before boot.

## Important media-state discovery

An earlier `?RT` diagnostic was caused by reusing media after a
KINIT-only SIMH breakpoint at `proc_boot_init`. That breakpoint is
**after** writable D6FS root mounting, which has published a newer
DIRTY alternate superblock. Stopping there without a normal shutdown
leaves a valid crash-recovery marker; rebooting that same media
intentionally rejects the latest DIRTY generation. This is not a
missing-root or superblock-parser regression. Both initial and final
host D6FS checks must be considered when repeating a boot test.

The regression builds fresh images by default. When an existing
`DAIMOS_DSH_FULL_BUILD_V647` is supplied, the source diskset must
already be CLEAN; otherwise the test aborts rather than masking the
unsafe state. The guest always writes to a copied disk/drum set.

All four variants passed. Normal D6FS mount, DSH external command
execution and clean HALT were verified without production code
changes. KINIT failure-path stack watermark coverage beyond the
already verified invalid-root path remains open.
