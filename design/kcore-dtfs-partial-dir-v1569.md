# KCORE P0-B v1569: actual Type-551 torn directory and cold boot

Date: 2026-11-14. **Physical DIR acceptance PASS**, no production
code or RAM change. Writable DTVOL2 remains disabled by v1555.

An isolated source-matched 256K PDP-6 KCC image retained the real
VFS/DTFS provider and v1551-v1554 Type-551 DIRTY intent/MM lease.
The production v1555 RW admission refusal was observed before a
test-only exact DTC3/mount2 policy exception was applied after
published mount and both 36-bit volume-ID halves. The test-only
source also installed two exact VFS CREATE instruction breakpoints
without altering either production DTFS CREATE or physical I/O.

The guest first created a native file and physically wrote valid
DTVOL2 WRITE_INTENT and native directory RBN0144. At the real
pre-second-CREATE breakpoint, SIMH DETACH persisted that complete
reel; independent bytewise comparison proved only RBN0 and RBN0144
changed, retaining original volume identity and untouched DATA.

The second CREATE encountered the actual diagnostic Type-551
`PARTIALWRITE=0144:1` and refused success. The post-provider
breakpoint and a separate DETACH persisted the interrupted physical
DIR update. An independent whole-media comparator proved precisely
one 36-bit word in RBN0144 had changed (six stored bytes), the
complete valid DIRTY RBN0 was identical, and nothing else was
modified. Poisoned FSINFO/CHDIR/UNMOUNT were refused while TSFS
root operations remained functional.

A second entirely unmodified **production KCORE/MRES** cold boot
mounted that exact persisted reel and refused both RW and RO,
published no DTFS owner and left the complete tape SHA-256 exactly
unchanged (`a85673f46bc72ce3707186c159c794253e091878a5ad5fe398b4e2dae864f8ae`).
The previous v1568 physical partial DATA and v1567 physical partial
CLEAN thus join this real partial DIR test as three accepted
on-media write-failure classes. No old file recovery is inferred.

Reproducer and exact hardware/source distinction:
`daimos-testkit/tests/validation/daimos-dtfs-v2-partial-dir-v1569.md`.

**P0-B remains OPEN:** Type-551 has no permanent reel-generation
or eject interlock; physical same/replacement reel reinsertion and
external exchange between verified ID and DATA/DIR/CLEAN transfers
need a defensible hardware/operating policy. This remaining safety
condition prevents lifting the v1555 production V2 READ-ONLY veto.
No production ABI, fixed KCORE TEXT/DATA/BSS or movable MRES growth;
`__kcore_low_end` remains octal `024364`. P0-A closed, P1-C/D/E
unchanged. VERSION 0.5, strict 32K optimization HOLD.
