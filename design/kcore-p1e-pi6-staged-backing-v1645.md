# P1-E v1645: physical PI6 staged wake and PURE backing owner lifetime

Date: 2026-11-17. Based on fully accepted v1644 physical writable
secondary-DTFS mutation/rename/unmount guards and v1643 physical
FULL read-error rollback. No production KCORE, PURE MRES, VFS,
MM, DTFS, TSFS, scheduler, PIPE or public ABI change.

## Exact question

v1644 proved the existing backing guards while all six children
were asleep and PACKED, but did not show real PI6 scheduling while
an owner began restoring and other child executable sources still
required protection. v1645 therefore source-derives the same
physical 48K PDP-6 guest and exposes one PIPE reader to an
individual wake while keeping the other five unable to see EOF.

The source-built parent first executes all original v1644
immutability checks on a real RW mode0777 compressed PURE file
`/MOUNT/COMP`, native DTFS DTC3 public mount2. All six real
children are PACKED and the seventh RUN is refused. The parent
writes EXACTLY ONE 36-bit token into the shared PIPE while the
writer stays open (`H`), then repeats the original UNMOUNT,
UNLINK, TRUNCATE, O_TRUNC and WRITE denials and same-file-slot
rename checks (`P`). It next calls the **real** `dsys_sleep(0200)`
and waits for the original PI6 scheduler to run the awakened
child. Exactly one child completes its original DATA/PIPE
sentinel/EXIT-057 checks (`w`) before the parent's `T` marker.
With the writer STILL open and FIVE children still PACKED, the
same guard checks succeed once more (`P`). Finally the writer
closes, five remaining children finish, all six exact PID/WAIT
and status-057 checks pass, native DTFS UNMOUNT succeeds (`U`),
parent prints `Z` and machine reaches normal HALT PC000000.

Original full physical guest console:

`aaaaaax[B013470/042501]<010000+010000>[A013470/042501]P{013470-013470}LHPwTPwwwwwUZ`

## Physical original MM and clock witnesses

The actual source-built v1645 DXR2 has `001454` octal text
words. Original PDP-6 PURE floor-sector calculation excludes
`(001454+020)&~0177=001400` octal words from each full `010000`
APR, giving exact **`006400` octal RAW words per PACKED owner**.
This is source-specific; v1643's guest used `006600` RAW words.

Original unmodified KCORE physical MM B/A/first P, post-H P,
post-T P samples show original separate nonoverlapping PACKED
source extents, correct owner/pin bitfields and identical source
addresses for each surviving owner:

| Checkpoint | Original CLK tick count (decimal) | PACKED owners |
| --- | ---: | --- |
| B / A | 7895 | 2..7 (6) |
| First P, all six protected | 8673 | 2..7 (6) |
| H, one original PIPE token published | 8673 | 2..7 (6) |
| First backing guard after H | 9098 | 2..7 (6) |
| T, original scheduler sleep returned | 10364 | 3..7 (5) |
| Second backing guard after T | 11015 | 3..7 (5) |

Both original `clk_tick_count` and `proc_timer_clock` advance
**1691** ticks from H to T. `clk_tick_count` is incremented only
by the qualified original hardware CLK PI6 service, rather than
software rescheduling. A further 651 real ticks pass during
the second backing guard. The first guard immediately after H
had already seen 425 real ticks without any source ownership
change. The separate original `proc_sched_pi_tick` source
contract prevents switching a still-running executive syscall
on an ordinary clock tick, but explicitly sleeping executive
contexts are scheduled by PI6. This test exercises an actual
sleeping user/kernel transition and child wake, not an injected
or synthesized clock tick or altered process/PI6 memory word.

Physical RAW extents for surviving owners 3..7 remain exactly:

| Owner | Octal physical RAW suffix | Pins |
| --- | --- | ---: |
| 7 | 051400..060000 | 0 |
| 6 | 061400..070000 | 0 |
| 5 | 071400..100000 | 0 |
| 4 | 101400..110000 | 0 |
| 3 | 111400..120000 | 0 |

Owner 2 was already gone at T: its one real PIPE token completed.
The other five remain PACKED with the same source identity,
unmodified suffix words, zero leaked pins and no temporary
PROCESS full destinations at the sampled checkpoints. The
selected ROOT remains original TSFS provider7/mount1, independent
of RW secondary DTC3 provider5/mount2, with original
`vfs_mount_policy=000000000001` (only ROOT marked RO).

Original KCORE mount/vnode guard breakpoints are hit at
PC **003431** (AC1=2, mount2) and PC **003322**
(AC1=050202000000, native DTFS provider5/mount2/file slot0).
Those are genuine original KCORE instructions, not guest hooks.
The existing complete physical v1644 source/guard oracle is
reused for B/A/P and final MM. The v1645 auditor additionally
requires EXACT one user `w` before T, five correct surviving
sources, real original hardware CLK and scheduler epoch deltas,
unchanged RW mount policy, all six final EXIT/WAITs and successful
unmount. U/Z MM has no child PROCESS/PACKED owner or pin.

Three independently forged physical witnesses are REJECTED:
zero real-clock advancement, an invalid retained RAW pin, and
a falsified RO bit on secondary mount2. The simulator may write
an ordinary DTC3 buffer-flush diagnostic to its console in the
middle of user chars. The auditor ignores only that recognizable
host diagnostic for counting actual guest `w` characters; it
does not remove a snapshot, change a clock, or alter a status.

The original source PI6 slot-0 guard v1086, PURE staged owner
v1563 and rename/PI6 mapping v468 source tests also PASS.

## Scope / remaining gates

**ACCEPTED:** real hardware PI6 clock time, single-child wake,
five surviving RAW PACKED owners, the original writable-DTFS
mutation/unmount guards before and after that wake, all original
exit/WAIT checks and successful mount retirement after owners
vanish. No production text, stack, MM table or BSS growth.

**NOT CLAIMED:** instruction-level race between an ongoing
DTFS WRITE/UNLINK or mount retirement and *new* PURE RUN/EXEC
admission, and an actually simultaneous pinned FULL target at
the backing-guard checkpoint. The H->first-P snapshot still
contained six unpinned PACKED suffixes. Further real physical
PI6/provider-sleep interleaving and uncertain MM rollback remain
open. General 32K/64K BCACHE regression, entire transient
physical peak RAM and old native DTFS v1522 source-fixture
modernization remain open. PURE default OFF and strict 32K
whole-KCORE size optimization HOLD pending P1-E closure.
