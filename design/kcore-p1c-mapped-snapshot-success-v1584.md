# P1-C v1584: physical mapped snapshot recovery; overlap not established

Date: 2026-11-15. Full 128K PDP-6 acceptance of healthy four-member
Type-236 mapped BACKSTORE snapshot persistence and its actual second
cold-boot restore. This is NOT independent swap-in during DMA acceptance.

## Production path and bounded test-only observers

The genuine DR0..DR3 D6FS root, BLOCKSET INTERLEAVE mapper, mapped
BACKSTORE, MEMFS v2, LOGSTORE, KCC userland and physical Type-236
DR_DONE PI2 ran on two independent CPU/RAM cold boots with the same
four member images. Four 24K private children caused actual swap
ownership. Only one child was made periodically runnable; three
retained the v1580 long sleep. No child calls kernel swap or snapshot
functions. A temporary copy of the actual MRES refuses the shutdown
unless original `proc_swap_blocks_used` is positive at its entry.

Test-only, disposable source instrumentation observes genuine
independently scheduled slot-0 `proc_swap_in` entry, physical
`backstore_read`, a matching original Type-236 PI2 completion, and
the completed DISK-to-RESIDENT publication. It separately counts
real snapshot DR_DONE interrupts; no I/O return or scheduler action
is manufactured. Nine fixed diagnostic words are allocated only
in the temporary kernel, with no production source, RAM, ABI or
persistent format changes.

## Physical acceptance and limits

Two real 128K KCC PDP-6 boots PASS: each HALTed normally at octal
`021777` with `01310` octal original process swap blocks owned and
`01374` BACKSTORE blocks used. The original Type-236 driver produced
`051` octal (=41 decimal) real snapshot DR_DONE PI2 events in each
boot, with no observed logical storage error. The snapshot marker
was clear at both HALTs.

The host oracle reads all four raw 36-bit drum images, finds the
committed v2 header at DR3 sector `017777`, verifies the true
3,648-word MEMFS pool and 271 additional nonzero tail sectors,
and checks all four D6FSR2 descriptors remained unchanged. Boot 2
restored original persistent MEMFS data/XIP and the prior LOGSTORE
record, then successfully appended and read another record.

Ten deliberately corrupted original map, post-HALT values and
physical-media witnesses were rejected. The accepted full guest
and full physical evidence are retained in
`$TMPDIR/daimos-p1c-swap-overlap-v1584.kR7MBC`.

**Independent swap-in started=0, completed=0, matching swap READ
PI2=0 on BOTH boots.** The optional verifier `--require-overlap`
correctly rejects the same physical evidence. Positive process
ownership and 41 real snapshot PI2 events cannot be substituted
for independent process swap-in during snapshot DMA. V1584 closes
healthy mapped-VOLUME cold-boot recovery only, complementing the
v1583 mapped snapshot PI2 fail-stop and v1580 healthy direct DEVICE.
P1-C remains OPEN for true scheduler overlap and exact recoverable
MM/BCACHE/owner rollback. P1-D and P1-E remain OPEN, VERSION 0.5,
KCORE production low end `024364` octal, 32K/optimization HOLD.

Reproducible runner and strict oracle live in daimos-testkit:
`tests/validation/daimos-p1c-snapshot-swap-physical-v1584.sh`,
`daimos-p1c-snapshot-swap-pi-v1584.py`,
`daimos-p1c-snapshot-swap-verify-v1584.py` and companion `.md`.
