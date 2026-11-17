# P1-E v1647: physical DTFS V2 unmount versus new PURE RUN

Date: 2026-11-17. Testkit fixture based on the original v1645
PDP-6 code and v1646 staged source-derived user test. This is a
source-linked 48K physical admission-race acceptance, NOT the
complete P1-E swapless lifecycle or a production code change.

## Failure found in the original v1646 experiment

The inherited ordinary DTC3 HOME image was physically DTVOL1
(octal magic `446466575421`). Real
`vfs_dtfs_quiesce_begin_v1562` deliberately takes the legacy
path on that medium. A test expecting the V2 owner barrier
therefore failed `!l`; its replay script mistakenly accepted
subsequent simulator `CONT` snapshots after the first guest
HALT at PC000000, including false repeated PC000001 halts.
Neither observation proves a PURE owner or V2 admission bug.

v1647 constructs the disposable DTC3 medium using the original
`tools/host/mkdtfs.c --durable-v1550` formatter. The new
testkit-only checker reads actual 18-bit little-endian halfword
pairs and requires exact native DTVOL2 (`446466575422`),
nonzero 72-bit ID, original complement check, CLEAN state,
correct seal and zero RBN0 trailing words. Seven altered physical
records (V1, zero ID, checksum, DIRTY, bad seal, payload,
short block) are all rejected. The original TSFS DTC0..2
selected root reels are generated separately and left intact.

The source-generated guest keeps its single control PIPE writer
open until the unmount helper exits; it no longer introduces an
unrelated CLOSE syscall before the barrier observation. The
complete original VFS source is compiled unchanged except for
one **test-only** `proc_sleep_ticks(01400)` call after acquiring
the original V2 barrier and before provider SYNC. This creates a
real 60Hz/PI6 scheduling interval. The independent auditor
checks exact whole-source equivalence to that one insertion;
there are no owner/MM/VFS/EXEC or production KCORE changes.

## Physical guest and source-backed result

The 48K no-AUX PDP-6 image boots with PURE and DTFS loaded.
Source-matched INIT DXR2 image has octal `002055` image words,
compressed `001775` payload words and `000036` relocation
words; immutable text would omit `002000` octal words if
subsequently reclaimed. The two existing child extents at B/A
are actually **RESIDENT PROCESS**, not PACKED, each `012000`
octal words, unpinned. This experiment is designed to test
new compressed-PURE RUN admission during mount retirement,
not to repeat v1639/v1645 PACKED or RAM-savings acceptance.

The original parent and independent TSFS-backed unmount helper
exercise real PIPE, FSINFO, VFS unmount, compressed DXR2 RUN,
WAIT, DTFS remount and post-remount RUN. The physical original
test produces checkpoint order:

`B A WAKE PI6_DONE RACE_START BARRIER RUN_REFUSED
UNMOUNT_DONE REMOUNT RUN_OK RELEASED Z`.

At the live V2 barrier, genuine FSINFO returns -1 (not 0 for
a retired slot), then the original `dsys_run` of the compressed
executable returns -1. The same FSINFO again reports -1.
The original mount root, DTFS media owner pointer, all sampled
process MM extents and 16 swap-record words are identical at
the barrier and immediately after the refused RUN. The secondary
mount is RW (VFS policy low bit 1, **no** RO bit for mount2);
selected TSFS ROOT remains provider7/mount1. On unmount-complete,
the actual executing slot is the TSFS-backed helper in slot2;
the rest of the checkpoints sample the parent in slot1.

The original hardware `clk_tick_count` advances from 19279 to
20283 decimal = **1004 real ticks** between RACE_START and
UNMOUNT_DONE, with corresponding scheduler epoch progress.
The helper exits correctly, the parent remounts the same DTC3
medium, its second real compressed PURE RUN succeeds and exits
with the exact required status, and final WAIT/unmount cleanup
leaves **no child PROCESS/PACKED MM owners**. Console includes
`Q R M w A U Z`, exactly one normal
`HALT instruction, PC: 000000`, and no user `!` marker or
post-HALT PC000001 continuation.

The source-linked v1647 auditor **PASS**es all physical
ownership/clock/source checks. It separately rejects three
forged witnesses: zero clock progress, altered process swap
backing during refusal, and fabricated RO mount2 policy.
Seven malformed DTVOL2 records also fail the new preflight.

## Decision

**ACCEPTED**: genuine V2 quiescence blocks a concurrent new
compressed PURE RUN; it leaves existing MM/backing intact and
permits later remount/RUN after successful retirement, with
real PI6 clock progress. No production KCORE/MM/VFS/ABI, fixed
RAM, controller or media-format change in v1647.

**Still OPEN**: an actual simultaneously pinned FULL restore
versus V2 quiescence and provider I/O, uncertain MM/unpin
rollback, complete transient peak RAM, additional 32K/64K
BCACHE footprints and KA10 backend acceptance. Existing
v1639 PACKED RAM and v1645 PI6 owner tests remain separate
accepted evidence. PURE is OFF by default; strict 32K
optimization and VERSION 0.5 gates remain unchanged.

Testkit sources:
`tests/diagnostics/pure-quiesce-run-v1646/`,
including the versioned `check-v2-v1647.py` and
`test-v2-v1647.py`. Source-built disposable evidence used
`$TMPDIR/pure-quiesce-v1646.pcSPNb` and
`$TMPDIR/pure-quiesce-run-v1646.0lyMqh`.

Portable evidence archive with full original physical trace,
verified test-only VFS source, map, compiled DXR binaries,
guest source, actual DTC3 reel, auditor log and SHA256 manifest:
`$TMPDIR/daimos-p1e-v1647-evidence.VG7qxo.tar.gz`.
Archive SHA256:
`63329268ecb03ecca114d825de0e1da508a40af0e940854a83fc09ccb588868d`.
