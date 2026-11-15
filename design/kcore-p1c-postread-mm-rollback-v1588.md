# P1-C v1588: real post-DMA rollback on a refused checked BACKSTORE release

Date: 2026-11-15. Acceptance of original MM transaction rollback
AFTER an actual physical swap READ has successfully completed, while
the original 128K KCC PDP-6 guest is persisting MEMFS v2/XIP and
LOGSTORE on the original four-member DR0..DR3 Type-236 root VOLUME.
This is distinct from v1587's earlier, pre-I/O MM pin failure.

## Source-authentic transaction

The disposable test kernel observes the production timer PI6 waking
a real sleeping, disk-backed child at snapshot entry; the original
slot-0 scheduler independently selects a genuine mapped BACKSTORE
swap-in. Original MM allocation, pin, physical Type-236 READ and
PI2 completion, and MM unpin all execute successfully. A one-shot
test-only provider refusal is returned at the original checked
`backstore_free` entry before modifying its authoritative bitmap.
This causes the original `proc_swap_in_free_fail` path to use the
original `mm_free`, clear the transition lock, retain the original
DISK record/mode/owner and return a nonfatal refusal to the normal
scheduler, which issues its original TERM event. Subsequent valid
swap-in requests and MEMFS snapshot commits continue normally.

Before the checked release and immediately after original MM cleanup
(BEFORE later TERM cleanup), a disposable observer compares all
actual backing bitmap words, MM table count and allocated-base
absence, process descriptor/record, DISK packed storage mode, original
process and BACKSTORE ownership counters and BCACHE slab/tags.
No production source/RAM/ABI/media format or scheduler policy changes.

## Physical acceptance

Two full, independent 128K PDP-6 cold boots complete the original
normal privileged HALT at `022004` octal on the same four physical
DR0..DR3 media. Each boot independently executes four real mapped
swap READs with **896 matching Type-236 PI2 DR_DONE completions**.
Three swap-ins publish RESIDENT; the fourth is refused at the
post-DMA release and returns to original DISK ownership.

| Original rollback witness | Boot 1 | Boot 2 |
| --- | ---: | ---: |
| One controlled post-DMA provider refusal | 1 | 1 |
| Original MM rollback and TERM event | 1/1 | 1/1 |
| Original MM extent count | `024` to `023` | `024` to `023` |
| Selected original aligned MM base | `0220000` | `0220000` |
| New allocated base remaining in MM | 0 | 0 |
| Process-owned BACKSTORE before/after | `01650` unchanged | `01650` unchanged |
| Backstore blocks used before/after | `01734` unchanged | `01713` unchanged |
| Exact backing bitmap words inspected | 29 | 29 |
| Modified backing bitmap words | 0 | 0 |
| Original DISK record and process fields | unchanged | unchanged |
| Original TRANSITION and packed storage mode | cleared / DISK | cleared / DISK |
| BCACHE slab pointer and metadata tags | unchanged | unchanged |

Boot 2 successfully restores the previous boot's MEMFS XIP/data and
LOGSTORE, then appends/reads the next LOGSTORE record. Independent
raw 36-bit physical verification confirms the committed MEMFS v2
header on DR3 sector `017777`, real 3,648-word pool, 275 other
nonzero backing sectors and intact D6FSR2 descriptors in all four
physical members. The verifier rejects **34 deliberately corrupted
PI2/bitmap/MM/BCACHE/owner/record/media/commit witnesses**:
13 inherited physical v1584 controls and 21 post-DMA rollback
controls. Retained v1587 and v1586 physical guest oracles still pass.

Evidence and independent verifier: daimos-testkit
`tests/validation/daimos-p1c-postread-release-physical-v1588.{sh,md}`,
`daimos-p1c-postread-release-v1588.py`,
`daimos-p1c-postread-release-verify-v1588.py`.
Retained full two-boot physical evidence:
`$TMPDIR/daimos-p1c-postread-rollback-v1588.qWn8Gy`.

## Residual P1-C and explicit exclusions

The mapped physical READ and PI2 are real; only the release refusal
is synthetic, at the genuine original provider entry. It is NOT
a naturally corrupted backing bitmap or a physical medium failure;
the mandatory fail-stop for active/owned BACKSTORE I/O remains as
proven by v1586. The observed BCACHE slab pointer is zero, so these
checks establish exact metadata stability but NOT restoration of
populated cache DATA or valid cache-tag identity after other I/O.

Still OPEN: mixed DEVICE/VOLUME/BCACHE provider failure and valid
cache cases, simultaneous owner transitions, later-stage failure
timings, and repeat-load stability (including the intermittent
v1585 timeout). P1-D and P1-E remain OPEN; VERSION 0.5, original
KCORE low end `024364` octal and strict 32K/optimization HOLD
remain unchanged. Production code/RAM/ABI/media format unchanged.
