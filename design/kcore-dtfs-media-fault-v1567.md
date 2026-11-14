# KCORE P0-B v1567: real Type-551 partial CLEAN and cold-boot refusal

Date: 2026-11-14. **Real physical partial-write/fail-closed cold boot PASS.**
No production source or RAM change. Production DTVOL2 stays READ-ONLY;
P0-B remains OPEN.

## Actual controller and media-generation audit

`system/kernel/drivers/tape_io.s` serializes Type-136/DTC transfers,
stores the requested physical unit and RBN, and handles PI completion,
timeout and controller write errors. There is no physical reel
generation number, prevent-eject interlock or atomic checked-media
identity bound to later hardware writes. The 72-bit RBN0 ID is a
persistent *record identity*, not a proof that a Type-551/TU55 reel
remains attached between two transfers. No software-only extra
read or volatile generation counter can solve a reel exchange
made externally between the check and the next write. Thus the
production read-only veto must remain while a real media-change
interlock/operating policy is specified and tested.

## Accepted on-media crash states

Eleven independent, full 256K production KCC guest cold boots on
real Type-551 DTC3 images PASS. Ten intentionally torn or DIRTY
images must refuse BOTH RO/RW mount without publishing DTFS VFS
ownership; full-tape SHA-256 is unchanged in every case. They
include incomplete RBN0 DIRTY and CLEAN state/seal writes, half-
word CLEAN, bad magic/identity complement/zero padding, and
physically damaged DATA/DIR blocks *with valid DIRTY RBN0*.
These ten images are host-generated crash simulations, not
interrupted hardware. One valid CLEAN RBN0 with a damaged unrelated
DATA block is deliberately **admitted RO**: the current RBN0
record does not checksum all arbitrary DATA blocks. This negative
control limits, rather than extends, durability claims.

## Genuine interrupted physical Type-551 CLEAN transaction

The v1566 Type-551 success test is extended using an **isolated
source-matched disposable image**. The test-only private v1551/v1552
DIRTY arm first physically writes and verifies the complete on-media
WRITE_INTENT record, then retains the mount for a second unmount.
Only the disposable test VFS has a temporary writable/ARMED owner.
Two exact linked resident KCORE instruction breakpoints stop SIMH
at the start of the real production v1566 CLEAN provider call and
on the actual v1566 sticky write-failure exit. They replace an
earlier failed, timing-sensitive asynchronous CTRL-E monitor attempt.

At the first breakpoint, SIMH DETACH flushes the actual physical
DTC3 buffer. Independent byte-level comparison proves original
CLEAN->DIRTY changed only complete physical RBN0 words 4 and 5,
retaining both 36-bit identity halves, padding, all DATA/DIR and
the rest of the tape. The diagnostic Type-551 emulator then
reattaches the SAME physical tape and arms `PARTIALWRITE=0000:1`.
The **real production CLEAN writer** reaches the Type-136 controller,
which interrupts after the first *changed* 36-bit RBN0 word.
The failure is delivered through real controller PI/FS provider
paths. The second breakpoint proves VFS took its sticky fail-closed
branch with the original mounted owner still published.

DETACH again persists the genuinely partially written RBN0.
Independent byte-level verification proves word4 changed from
WRITE_INTENT to CLEAN but word5 retained the old DIRTY salted seal;
no other physical tape byte changed and both saved 36-bit IDs
are intact. The guest confirms FSINFO/CHDIR/UNMOUNT refusal while
ordinary TSFS root operations remain healthy.

Finally this **exact damaged tape**, not a reconstructed lookalike,
is mounted on a second fresh boot of entirely unchanged production
KCORE/MRES. Both RO and RW admission refuse, VFS slot remains
unpublished and full physical tape SHA-256 remains identical.
The old v1529 genuine partial DATA-block diagnostic still PASSes.

See `daimos-testkit/tests/validation/daimos-dtfs-media-fault-v1567.md`
for 11 exact crash modes, test-only instrumentation, reproduction,
host/physical evidence distinction and full guest acceptance.

## Scope and remaining P0-B

This proves correct v1566 failure handling for a *real physically
interrupted RBN0 CLEAN*, plus second cold-boot rejection. It does
not close the mechanical reel exchange between RBN0 verification
and a later DATA/CLEAN transfer, an interrupted V2 DATA/DIR write
and replacement/reinsertion after ordinary user-data RW, nor
independent checksums for CLEAN data sectors. These remain
acceptance blockers before the v1555 RO veto could be removed.

Zero permanent kernel/MRES source, ABI, mount slab, fixed DATA/BSS,
or fixed KCORE TEXT change. `__kcore_low_end` remains octal
`024364` from v1566. **P0-A CLOSED, P0-B OPEN**, P1-C/D/E unchanged;
VERSION 0.5, strict 32K optimization HOLD.
