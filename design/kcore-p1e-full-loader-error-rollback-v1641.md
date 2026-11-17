# P1-E v1641: test-injected FULL callback failure and MM rollback

Date: 2026-11-17. Base: v1640 physically verified natural FULL
allocation NOMEM and later successful retry. This v1641 test
uses the original v1638 compressed PURE image, original 48K
PDP-6 no-AUX/TSFS private tapes, and unchanged kernel/MRES.
No production code or permanent source/backing data is modified.

## Explicit one-shot simulator injection, not a VFS read failure

The original v1638 guest uses one plain PURE first child and
five compressed PURE children, admits all six, naturally refuses
the seventh RUN, then executes all original child PIPE/data/
EXIT-057 and exact parent PID/WAIT/status-057 checks. Source-
linked MM B/A snapshots show six original unpinned PACKED
suffixes of 007000 octal words each.

The physical simulator stops at the ORIGINAL compiled v1391
loader-callback return, PC 034361 (MRES offset 002333,
`HRROI 10,777774`), immediately after the genuine compressed
FULL source loader returns **AC1=0 success**. Its original MM
table has two simultaneously pinned, distinct extents for
the same compressed child slot3:

| Slot | Octal physical words | MM type | Pins |
|---|---|---|---:|
| 3 | 111000..120000 | PACKED_PROCESS source | 1 |
| 3 | 120000..130000 | PROCESS full target | 1 |

The test script then deposits **only AC1=777777777777 (-1)**
into the simulator at this exact public callback boundary.
It does NOT change the production instruction, MM extent,
source DXR, mutable suffix or VFS read result. This models a
loader-reported error after the target has already been
allocated and pinned; it does NOT prove a genuine hardware,
filesystem or VFS I/O fault.

The following ORIGINAL v1391 error branch returns AC1
777777777774 (MM_ERR_INVAL=-4) to the v1612 PURE service at
source-linked physical PC 032461. The slot3 original PACKED
suffix **still occupies exactly 111000..120000, PINNED=0**;
the allocated PROCESS target 120000..130000 is completely
absent from MM. All other child owner extents and pins remain
untouched. Neither a fake resident VM nor an unowned APR is
advertised to the scheduler.

The simulator then continues UNMODIFIED. The next observed
original successful bounded FULL callback at the same source-
linked site belongs to compressed slot4: AC1=0, original
PACKED source 101000..110000 pin=1 and separate full target
120000..130000 pin=1. At that time slot3 remains RAW PACKED,
unpinned, awaiting its own scheduled restoration. The guest
continues to complete all SIX original child PIPE/private-data/
EXIT-057 and parent WAIT checks, prints terminal `LwwwwwwZ`,
and ends in normal simulator HALT. Final MM Z contains no
child PROCESS or PACKED owner, demonstrating eventual recovery
for all children without a leaked or falsely runnable owner.

The testkit source/opcode/MM/console audit PASSes and rejects
both a forged retained slot3 pin after rollback and a fabricated
failed original loader return before the simulator injection.

## Acceptance boundary

The positive result closes the **injected loader-callback error**
transaction: pinned RAW and target existed; one error followed;
original v1391 correctly unpinned and freed the destination,
preserved the RAW source and allowed later executable recovery.
This complements v1640 natural NOMEM rollback and v1638
successful physical FULL relocation, without a new kernel word.

Still OPEN: a genuine VFS read failure or immutable source
removal/unmount during FULL reconstruction; PI6 concurrent
interleavings and uncertain MM failure rollback; 32K/64K
BCACHE layout regressions; and matched transient whole-system
peak RAM. PURE remains disabled by default and strict 32K
whole-KCORE size optimization HOLD until P1-E acceptance.
