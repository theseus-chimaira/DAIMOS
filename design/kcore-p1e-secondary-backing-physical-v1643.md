# P1-E v1643: physical secondary DTFS FULL read failure and recovery

Date: 2026-11-17. Base: accepted v1642 selected TSFS-root
fail-stop and v1641 simulated loader-error rollback. No permanent
KCORE, PURE MRES, MM, VFS, TSFS, native DTFS, user ABI or physical
media-format change. All new executable and diagnostic code is in
daimos-testkit and uses the already installed original toolchain.

## Why a secondary executable filesystem is necessary

The original v1538 root I/O policy requires terminal HALT after a
genuine selected-root read failure, as physically verified in
v1642. Such a read must NOT return as an ordinary FULL loader
error. v1643 therefore retains the three-member TSFS namespace
ROOT (physical DTC0..2, public mount ID1) throughout the failure,
and instead executes compressed PURE children from a separate,
real native DTFS filesystem on physical DTC3 (mount ID2).

The original six-child C89 v1638 guest is source-derived. The
parent and its first plain PURE child still RUN from the selected
root `/SYSTEM/INIT`. Five compressed PURE children instead RUN
the same C89 image from `/MOUNT/COMP` on secondary DTFS. Before
RUN, the parent mounts the already-stamped DTC3 reel RW, copies
the root's compiled `/SYSTEM/COMP` compressed DXR2 file using
normal user `open/read_words/write_words/close` syscalls, sets
executable mode 0555, performs a checked CLEAN unmount, and
remounts the same authenticated reel read-only. An ordinary
`FSINFO` checks the real secondary DTFS provider/mount IDs.
The original TSFS manifest contains `/MOUNT` but not the deeper
`/MOUNT/DT0`. The first diagnostic mount refused (initial `!f`,
then exact `!1`); the test guest's mountpoint was corrected to
the existing `/MOUNT`, without changing VFS or making the
immutable root writable. A subsequent original full boot passes.

## Genuine 48K six-PACKED admission and source-specific RAM size

The unchanged guest test protocol admits six sleeping children,
then naturally refuses a seventh RUN and checks parent no-growth
BRK, all six inherited PIPE/private writable DATA sentinels,
EXIT-057, six exact PID/EXITED/status-057 WAITs, and final
normal simulator HALT. The original full console ends:

`aaaaaax[B013470/042501]<010000+010000>[A013470/042501]{013470-013470}LwwwwwwZ`

Original source-linked physical MM B/A samples show **six distinct,
unpinned PACKED_PROCESS owners 2..7**, all nonoverlapping and
each containing `006600` octal retained writable suffix words.
This guest has more immutable text because it contains the
new source-seeding routine: the compiled DXR2 TX2 text is
`001227` octal words. The ORIGINAL
`vm_pdp6_pure_range_v1604.h` calculation is rounded DOWN:

`skip = (text+020)&~0177 = 001200` (octal),
`raw = 010000-001200 = 006600` (octal).

The earlier v1638 guest used `007000` retained words, which
must not be attributed to this DIFFERENT source executable.
No comparable v1639 PURE-on/off net RAM saving or transient
peak is claimed by this v1643 test.

## Actual non-root Type-551 read fault during v1391 FULL

The source-identical guest, source-matched fixed KCORE/PURE
linker maps and private physical copies of all original tapes
are replayed. The simulator first stops at original bounded
FULL loader PC `035252`. The exact original MM table shows
compressed slot3 concurrently owning two distinct PINNED extents:

| Owner | Physical octal words | Original MM type | Pins |
|---|---|---|---:|
| 3 | `111200..120000` | PACKED_PROCESS suffix | 1 |
| 3 | `120000..130000` | PROCESS full APR | 1 |

The simulator then physically DETACHes **only DTC3**, confirming
`not attached`, while leaving selected-root DTC0..2 attached.
The genuine native DTFS backing access fails and original v1391
returns `MM_ERR_INVAL=-4` to the v1612 PURE service at
source-linked physical PC **`032461`**. The complete original
MM table at this return shows the **unchanged slot3 RAW suffix
`111200..120000`, now PINNED=0**, and its temporary full
destination `120000..130000` **entirely freed**. No other child
has a leaked MM pin. The original namespace ROOT remains
TSFS provider7/public mount1 and the system has not HALTed.

Testkit reattaches the **same original private DTC3 volume**.
The next observed original bounded FULL callback at PC `034361`
returns AC1=0 success for compressed slot4, with independently
pinned RAW source `101200..110000` and PROCESS full target
`120000..130000`. Slot3's failed source still survives unpinned
at this point; the scheduler eventually restores all remaining
children and each performs its original user checks. Final MM Z
contains **zero child PROCESS or PACKED owners**; all six WAITs
pass and normal HALT occurs. No `003776` selected-root HALT
occurs. The actual DTC3 read-suspect MM flag was not sampled
directly; original v1522 identity revalidation remains a source
contract, and this test asserts same-medium reattachment plus
observable eventual complete execution, not a fabricated flag.

The source/DXR opcode/complete MM/console auditor PASSes and
rejects two independently forged traces: a pin left on slot3
after failed FULL, and an invented successful return from the
failed callback. Native PURE rollback v1395 and service v1612
regressions PASS on the same unchanged production source.

## Independent older DTFS testkit caveat

The old `daimos-dtfs-mounted-id-v1522.sh` source-extraction test
does NOT currently execute against this newest tree: its original
FSINFO end anchor was removed by v1575. A trial updating the
anchor exposed a subsequent missing linked v1561 native write-
gate dependency. That incomplete trial was reverted rather than
linking an unsafe gate stub or claiming the test PASSed. This is
an older TESTKIT compatibility issue, not a v1643 physical
kernel/assembler regression, and deserves separate source-backed
test modernization before being counted as a new regression gate.

## Acceptance boundary

**ACCEPTED:** genuine non-root backing failure while a separate
FULL target is pinned, exactly preserved/unpinned PACKED source,
released target, continuing healthy TSFS root, same-reel return,
subsequent genuine compressed loader success and all six child
WAIT/EXIT/MM cleanup. This complements v1642 intentionally fatal
root reads and v1641 injected loader errors.

Still OPEN: executable mutation/unmount while PACKED, concurrent
PI6 and uncertain MM cleanup, independent 32K/64K BCACHE profiles,
the legacy native v1522 fixture modernization, and COMPLETE
matched physical transient peak RAM (including FULL's simultaneous
pinned raw source, destination APR, and filesystem scratch).
PURE remains OFF by default. Strict 32K whole-KCORE size work
remains HOLD until P1-E acceptance.
