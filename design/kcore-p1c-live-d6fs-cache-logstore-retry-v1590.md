# P1-C v1590: live D6FS BCACHE survives recoverable physical LOGSTORE retry

Date: 2026-11-15. Two real 128K KCC PDP-6 cold boots on the original
four-member Type-236 DR0..DR3 D6FS root VOLUME, BLOCKSET INTERLEAVE,
mapped BACKSTORE, MEMFS v2/XIP, and LOGSTORE. Acceptance demonstrates
the missing v1589 counterpart: a clean, allocated, populated D6FS
private BCACHE entry remains **live, unchanged and readable** across
an unrelated LOGSTORE provider failure and its successful retry.
The later original independent PI6 swap/MM rollback remains mandatory
but is a separate, subsequent transaction with its own rules for
legitimate memory-pressure cache reclamation.

## Original provider and physical transaction boundaries

The opt-in guest performs ordinary VFS OPEN and two 128-word READs
from the real physical D6FS root `/CONFIG/XIP1469.TEST` before the
LOGSTORE append. The original D6FS reader fills the standard shared
workspace, and the original MM-backed BCACHE private slot retains a
valid clean block with SOURCE12,,BLOCK24 tag. This is not a forged
cache key or synthetic cached block. The observed private slab is
`0377577` octal, key `100100001010`, with 28 nonzero of 128 data
words. The disposable observer copies all 128 36-bit data words.

At the first selected LOGSTORE append, the existing v1577 test-only
hook uses the **original mapped physical Type-236 LOGSTORE write**
to persist a complete record with the commit trailer cleared, then
returns an intentionally injected completion failure. The original
LOGSTORE producer must not advance its sequence or physical ring
cursor. Userland reads back the uncommitted physical block through
LOGCTL, verifies it, then retries the same slot with an intact commit
trailer and checks the readback/sequence again. The original logical
failure is controlled; this does NOT claim a spontaneous controller
error or recoverable owned-BACKSTORE media corruption.

The source-linked v1590 observer runs once at the original provider's
pre-fault boundary, immediately after the physically written torn
record, and immediately after the successful original same-slot
retry. It verifies the **same original cache slab, valid source tag,
all 128 full cache words and a genuine private `bcache_fetch` HIT**
with all 128 independently copied words unchanged. A cache miss,
silent tag invalidation, cache-owner replacement, or any changed word
rejects the test. None of these probes manufacture a controller
result, call a private filesystem reader across MRES boundaries, or
modify production cache or reclaim policy.

The guest subsequently creates four real sleeping 24K-word children
and issues its normal privileged HALT request. During the original
MEMFS snapshot, timer PI6 wakes an independently selected DISK-backed
child; the original scheduler executes mapped BACKSTORE swap READ
with genuine Type-236 PI2. The separate v1588 one-shot checked
BACKSTORE release refusal occurs **after** successful physical READ
and MM unpin. Original MM cleanup preserves the process's DISK
record, mode, owner and all 29 backing bitmap words. At this later
memory-pressure point, ordinary BCACHE reclamation remains allowed;
v1590 only requires cache retention during the earlier LOGSTORE
provider failure/committed retry, not during a later unrelated MM
pressure transaction.

## Accepted two-boot original evidence

| Linked physical/cache/rollback witness | Boot 1 | Boot 2 |
| --- | ---: | ---: |
| Genuine populated D6FS private BCACHE | 1 | 1 |
| MM-owned slab and 36-bit cache key | `0377577` / `100100001010` | `0377577` / `100100001010` |
| Nonzero words in exact 128-word physical cache block | 28 | 28 |
| Actual uncommitted physical LOGSTORE record write | 1 | 1 |
| Cache inspected after logical failure | 1 | 1 |
| Cache inspected after same-slot committed retry | 1 | 1 |
| Changed original cached words after failure/retry | 0 / 0 | 0 / 0 |
| Genuine original `bcache_fetch` after failure/retry | HIT / HIT | HIT / HIT |
| Changed 128-word fetch data after failure/retry | 0 / 0 | 0 / 0 |
| Independently scheduled mapped swap READs later | 4 | 4 |
| Actual Type-236 swap READ PI2 completions later | 896 | 896 |
| Successful subsequent RESIDENT publications | 3 | 3 |
| Original process-owned BACKSTORE after rollback | `01650` preserved | `01650` preserved |
| Changed original BACKSTORE bitmap words on rollback | 0 of 29 | 0 of 29 |

Both original full-guest HALTs completed normally. Boot 2 verified
and restored the first boot's committed MEMFS/XIP state and LOGSTORE
record before its own physical torn/retried append and next snapshot.
The independent oracle checks linked KCORE addresses, actual post-HALT
SIMH transcripts and raw four-member media including the committed
snapshot header at DR3 block `017777`. Exactly **47 deliberately
corrupted witnesses were rejected**: 13 inherited v1584 physical/
PI/scheduling, 21 v1588 MM/backing/rollback and 13 new v1590 live
cache/physical torn/retry/full-data/fetch controls.

In the same v1590 phase the separately committed v1490 native
BCACHE/DTC fixture was repaired to source-link the original modern
DTFS v1561 write-intent gate and its genuine dependencies. Original
V1 write admission, V2 and unpublished-owner refusal, BCACHE write
invalidation, failed MM reclaim and both native negative mutations
pass. No successful dummy write-intent gate was substituted.

## Residual P1-C and unchanged constraints

P1-C remains **OPEN** for concurrent independent process/other
provider owner changes at the same failure boundary, additional
later-sector fault timing and repeated-load stability including the
isolated v1585 timeout. The mixed D6FS/LOGSTORE valid cache case
is accepted; v1589 had independently accepted correct reclamation
when MM pressure genuinely requires the cache slab.

No production source, KCORE RAM, ABI, on-disk structure or scheduling
policy was changed. VERSION 0.5, original fixed low end `024364`
octal, and strict 32K/optimization HOLD remain in force. P1-D and
P1-E remain OPEN. The full two-boot physical test and reproduction:
`daimos-testkit/tests/validation/daimos-p1c-live-cache-logfault-physical-v1590.md`.
Retained physical evidence:
`$TMPDIR/daimos-p1c-live-cache-logfault-v1590.jhggKy`.
