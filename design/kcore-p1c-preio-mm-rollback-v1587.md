# P1-C v1587: recoverable MM-pin refusal during physical snapshot overlap

Date: 2026-11-15. Accepted two independent full-profile 128K KCC PDP-6
cold boots on a genuine four-member Type-236 DR0..DR3 D6FSR2 root
VOLUME, original BLOCKSET INTERLEAVE mapped BACKSTORE, MEMFS v2/XIP
and LOGSTORE. This increment proves **pre-I/O MM allocation rollback
when one independently selected slot-0 swap-in's MM pin is refused**.
It does not claim recoverable failed physical swap I/O, which is
intentionally fatal for active/owned BACKSTORE under v1541.

## Source-authentic transaction and failure selection

Four genuine 24K-word private-process children are allocated and
backed by real physical swap sectors, then wait in ordinary timed
SLEEP. A disposable v1585 snapshot-entry hook aligns one already
sleeping DISK process's original timer to the next 60 Hz PI6 tick.
The original hardware timer PI6 wakes it and the original slot-0
scheduler independently calls `proc_swap_in`; no software PI,
run-queue change, direct swap call or fake device completion occurs.

The original `mm_alloc_aligned` actually allocates new aligned
process VM. Immediately after that allocation, but before the real
`mm_pin`, the disposable test copy returns one synthetic `MM_ERR_BUSY`
(-5) to the original `proc_swap_in_pin_fail` branch. The normal
original `mm_free` must release the new extent, then the original
`proc_swap_in_drop_local` drops its result local and clears the
process's `TRANSITION` flag without changing the authoritative DISK
storage mode, disk backing record or owner count. The original
slot-0 refusal path then issues its own TERM event. The refusal is
nonfatal and before any physical READ from the refused swap-in;
other process swaps and snapshot I/O continue normally.

Only the temporary test kernel adds observability to both sides
of that narrow failure window. Immediately after the new extent
allocation and again just after original `mm_free`/transition unlock,
it captures the actual MM extent count and tests that the allocated
base is absent from the live MM table; it copies and compares EVERY
word of the authoritative BACKSTORE bitmap (29 original words for
the `02000`-block region) and the original process swap counters,
packed disk record and process descriptor. It invokes the original
read-only `proc_storage_expect_v30` to require DISK mode. It also
compares original BCACHE slab pointer, shared-workspace tag and
private-slot tag at those exact boundaries. This is a metadata
unchanged check, NOT a repair test for corrupt BCACHE contents.
Production MM, BCACHE, swap, BACKSTORE and MRES sources are unchanged.

## Real physical acceptance

Both independent full 128K PDP-6 cold boots completed normally at
original privileged HALT `022004` octal. Boot 2 used the same actual
four-member drum media, fresh CPU/RAM, restored the earlier MEMFS
snapshot and LOGSTORE record, and appended/read another LOGSTORE
record. Both boots recorded the same one-time source-authentic
rollback at the original transaction boundary:

| Original linked witness | Boot 1 | Boot 2 |
| --- | ---: | ---: |
| Independently woken sleeping process slot | 6 | 6 |
| Synthetic pin refusal / original MM rollback / TERM | 1/1/1 | 1/1/1 |
| New actual aligned process extent base | `0220000` | `0220000` |
| Original MM extent count before/after | `024` / `023` | `024` / `023` |
| New base still in original MM extent table | 0 | 0 |
| Original `proc_swap_blocks_used` before/after | `01650` / `01650` | `01650` / `01650` |
| Original `backstore_blocks_used` before/after | `01734` / `01734` | `01713` / `01713` |
| Exact BACKSTORE bitmap words compared | 29 | 29 |
| Backing bitmap differences | 0 | 0 |
| Packed DISK record and other process words | unchanged | unchanged |
| Original `TRANSITION` state | set then cleared | set then cleared |
| Original `proc_storage_expect_v30(DISK)` | 0 | 0 |
| Original BCACHE pointer, workspace/private tags | unchanged | unchanged |

Both full physical snapshots continued despite one MM-pin refusal:
the original slot-0 scheduler started four physical swap-ins and
successfully published THREE other DISK-to-RESIDENT restores per
boot, with 672 matched real Type-236 swap-READ PI2 completions each.
The final physically committed MEMFS v2 header was on DR3 sector
`017777`; the true MEMFS pool is 3,648 words, 275 other BACKSTORE
sectors contain nonzero data, and all original D6FSR2 member
descriptors remain intact. Original logical storage errors: zero.

The independent executable raw-media/KCORE oracle rejected 33
deliberately false linked/physical witnesses (13 inherited v1584
physical/scheduler/PI2/commit mutations, plus 20 new exact MM,
bitmap, process, storage-mode, TERM and BCACHE metadata changes).
The previously accepted full physical v1585 overlap and v1586
critical swap-READ fault regressions remain passing against their
retained original evidence. No permanent production code or data
size change is made; all diagnostic words and one 64-word snapshot
buffer exist exclusively inside disposable full-guest builds.

## Remaining P1-C and preservation

This closes ONE recoverable **pre-I/O** MM rollback subgate. The
injected pin failure is synthetic, not a naturally saturated MM pin
count. The test does not make active owned BACKSTORE device faults
recoverable, does not exercise later-sector swap restore failure,
does not establish BCACHE data rollback after a real mutation, and
does not prove all interleavings of multiple concurrent owners.
The v1585 intermittent 180-second timeout remains an open
stability investigation. Thus **P1-C stays OPEN** for true post-I/O
recoverable owner/MM/BCACHE transaction failures and combinations.
P1-D/E remain OPEN; VERSION 0.5, fixed production KCORE low end
`024364` octal, strict 32K/optimization HOLD remain unchanged.

Original retained two-boot physical evidence:
`$TMPDIR/daimos-p1c-mm-pin-rollback-v1587.b37MyN`. The repeatable
runner, source-only diagnostic, oracle and detailed engineering
report live in daimos-testkit `tests/validation/`:
`daimos-p1c-pin-rollback-physical-v1587.{sh,md}`,
`daimos-p1c-pin-rollback-v1587.py`,
`daimos-p1c-pin-rollback-verify-v1587.py`.
