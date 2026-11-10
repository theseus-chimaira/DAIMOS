# v1370: grouped D6CODEC complete PDP-6 KCORE cold-boot acceptance

Date: 2026-11-08. Status: **native grouped cold boot through LOGIN,
DSH, a grouped RUN command and LOGIN respawn PASS**. This is an
isolated minimal root, not a production codec-format deployment.

## Functional objective

Prior v1365 work proved Stage-0/Stage-1 grouped KINIT entry; v1368
proved a full isolated grouped KCORE and selected MRES relocations;
v1369 proved actual grouped EXEC/PURE/TSFS routines with deterministic
providers. v1370 now proves that one coherent *running* PDP-6 system
can use grouped compression across Stage-1, resident KCORE, actual
D6FS/VFS and user executable loading. No instruction-size work or
production code modification was performed in this milestone.

## Exact grouped kernel and module set

`daimos-testkit/tests/validation/d6codec-fullboot-v1370` first runs
the v1368 exact KCORE baseline/link/fold regression and then replays
the **27 complete MRES dlink recipes** from the actual boot Makefile.
Each original-map MRES link is byte-for-byte identical to production.
Each grouped-map MRES link retains its original initialized, BSS and
resident word dimensions. The control KINIT also relinks byte-for-byte
identically to the current production KINIT.

The grouped KCORE is v1367's decoder/VFS adapter, with the historical
40-word low-core decoder location retained as a non-executable
reservation. This preserves the linked early EXEC position and avoids
a second executable codec. The grouped kernel's text increase remains
**+256 words** compared with the original KCORE; BSS is unchanged.

The new full KINIT links all 27 grouped-map MRES packages and a
correctly resized embedded KCORE load area. Its entry is `054124`
octal, and its KCORE load span is `030037..053240` (exclusive upper
bound). The unchanged `image/mkimage.sh` places the new grouped KCORE
stream into this span; the test verifies every installed word against
the grouped KCORE initialized stream, not just its size or header.

## Stage-1, root media and native simulator

The test encodes the **32926-word** full new KINIT payload into
**29235 grouped words**, preserving its DAIMON header and exact entry.
The pre-write guarded v1364 Stage-1 is built for native PDP-6 and
loaded using the Type-760 paper-tape Stage-0 boot loop. It reads
the compressed KINIT from the simulated Type-136 DECtape controller.

An isolated KCC userland build produces ordinary linked DXR programs.
The test-only grouped DXR2 producer transforms three programs while
preserving their entry, flags, TX2 text count and relocation bitmap:

| Program | Decoded image words | Grouped compressed words |
|---|---:|---:|
| LOGIN | 3131 | 2541 |
| DSH | 15331 | 12385 |
| ECHO | 1126 | 918 |

The D6FS root is intentionally a **15-node smoke-test filesystem**:
it includes the original plain INIT and MOUNTALL, grouped LOGIN,
DSH and ECHO, and unmodified INITTAB/FSTAB/PASSWD/SYSTEM config.
The host `mkdsk`, `mkd6fs`, `logstore` and `d6fsck` utilities create
four complete DSK270 member media and verify a CLEAN superblock.
This is NOT the full normal distribution with its many commands.

The 256K PDP-6 simulator runs headless. Its optional DPY/WCNSLS/
OCNSLS host presentation backends are disabled to avoid unavailable
host SDL/EGL/terminal services, without modifying guest code or
storage. Console switch `1` selects the disk root explicitly.

## Native functional acceptance

The complete isolated system boots:

    Type-760 Stage-0 -> grouped Stage-1 -> grouped full KINIT
      -> grouped KCORE + 27 MRES -> DSK270 D6FS -> INIT V1
      -> grouped LOGIN -> grouped DSH -> grouped ECHO
      -> DSH exit -> INIT LOGIN respawn

The interactive test logs into ROOT on CTY, requires the DSH `# `
prompt, executes `ECHO V1370GROUPED` through actual VFS/EXEC/RUN,
and verifies that the program emits its own standalone output line
`V1370GROUPED` before the next prompt. It then executes `exit` and
verifies the subsequent `LOGIN: ` prompt. Thus a guest terminal
merely echoing the input does not satisfy the test.

The separate negative control runs original D6LZ36 KCORE/KINIT
with its own pristine *grouped-executable* D6FS root. It reaches
INIT V1 but correctly reports `INIT: RUN FAILED`; grouped LOGIN
cannot be decoded by the old codec. It does NOT produce LOGIN.

Each boot receives separate copies of the initial CLEAN disks.
This is necessary because guest D6FS mount/initialization may
publish a newer DIRTY superblock: reusing a failed-boot disk makes
subsequent tests fail `?RT` correctly and contaminates the control.
An earlier boot experiment encountered this issue, which was
resolved by independent pristine media, **not** by bypassing
superblock validation or changing the filesystem.

## Reproduction and remaining release constraints

In the daimos-testkit checkout, run:

    DAIMOS_REPO=/path/to/staged/DAIMOS sh \
      tests/validation/d6codec-fullboot-v1370/run-v1370.sh

All generated links, disk images, host executables and SIMH logs
are versioned below `$TMPDIR/d6codec-fullboot-v1370`. No permanent
DAIMOS KCORE, boot medium, root image, D6LZ36 implementation or
production Makefile is changed. The grouped runtime continues to
use the same compressed DXR flag, which supplies **no codec ID**.
The final host/runtime/media switch must therefore be atomic.

Remaining: full distribution image conversion, actual grouped TSFS
root with multiple extents, real PURE eviction and swap/reload,
complete nested provider kernel-stack watermark measurements and
injected media/provider errors. v1370 proves an end-to-end minimum
system operates; it does not yet accept those other workload classes
or authorize a production format cutover.
