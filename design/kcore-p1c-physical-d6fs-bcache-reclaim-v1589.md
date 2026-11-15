# P1-C v1589: actual D6FS cache reclamation during physical swap overlap

Date: 2026-11-15. Two complete 128K KCC PDP-6 cold boots on the
original four-member Type-236 DR0..DR3 D6FSR2 root VOLUME, production
BLOCKSET INTERLEAVE, mapped BACKSTORE, MEMFS v2/XIP and LOGSTORE.
The completed acceptance is **exact-data validation of a populated
private D6FS cache at its authentic MM reclamation boundary**, followed
by no stale cache hit and correct, independent post-DMA swap rollback.
It is NOT proof that a populated cache entry remains resident if
reclaim is unnecessary, and it is not an owned-media recovery case.

## Exact execution and corrected experimental hypothesis

Ordinary opt-in XIP userland OPEN and two 0200-word READs of the real
root `/CONFIG/XIP1469.TEST` cause the original D6FS reader/BCACHE to
populate the dynamic two-block clean cache. The shared filesystem
workspace is slot 0; slot 1 uses the existing 0201-word MM-backed slab.
The test observes the actual private slab, full D6FS source/block tag
and all 128 36-bit data words at MEMFS snapshot entry. One original
timer-tagged sleeping DISK child is woken by genuine PI6 at snapshot
entry, and the original slot-0 scheduler independently executes a
mapped BACKSTORE swap-in. The v1588 test-only post-DMA checked-release
refusal still exercises the original MM rollback and DISK ownership.

An earlier attempt to invoke `d6fs_reader_get_block` from the MEMFS
snapshot module failed because D6FS and MEMFS are separately linked
removable modules. The design was corrected to use ordinary VFS from
the guest; no cross-MRES dependency or production linking change was
introduced. The first successful two-boot experiment disproved the
original cache-survival hypothesis: the private cache had been freed
by ordinary kernel memory pressure before the later swap rollback.
Both boots correctly reported a cache miss, not a stale hit.

The accepted experiment therefore adds a **read-only test observer at
the original `bcache_reclaim` entry**, matching the exact populated
slab, key and active snapshot. All 128 words are compared before the
original invalidation. The original BCACHE/MM free and its return
are observed without changing any recovery rule or allocator.
After reclaim, the original cache must not claim the old identity.
The separate post-DMA rollback must still preserve the original
DISK process record, packed mode, MM state and full BACKSTORE bitmap.

## Two full-guest results

| Linked original/test observer | Boot 1 | Boot 2 |
| --- | ---: | ---: |
| Populated MM-backed private cache slab | `0307577` | `0307577` |
| Original D6FS source/block key | `100100001010` | `100100001010` |
| Nonzero words of the 128-word physical D6FS block | 28 | 28 |
| Matched original BCACHE reclaim calls | 1 | 1 |
| Original independent slot-0 swap-ins started at reclaim | 1 | 1 |
| Differences in all 128 cache words BEFORE invalidation | 0 | 0 |
| Original MM frees of that cache slab | 1 | 1 |
| Refused/incomplete original cache reclaims | 0 | 0 |
| Stale private BCACHE fetch hits after reclaim | 0 | 0 |
| Physical mapped BACKSTORE swap READs | 4 | 4 |
| Matching original Type-236 swap READ PI2 completions | 896 | 896 |
| DISK-to-RESIDENT publications | 3 | 3 |
| Checked-release refusals after physical READ | 1 | 1 |
| Independent post-read MM extent rollback | 20 to 19 | 20 to 19 |
| Original process-owned backing blocks across rollback | `01650` unchanged | `01650` unchanged |
| Changed original backing bitmap words | 0 of 29 | 0 of 29 |

Both native privileged HALTs were normal at octal `022004`. The second
boot independently restored the first boot's MEMFS/XIP and LOGSTORE
record, then appended and read another log record. The physical
four-member root has a valid v2 snapshot header at DR3 block `017777`,
a 3,648-word pool, 282 other nonzero backing sectors and intact D6FSR2
member descriptors. The independent host oracle rejected **48 unsafe
witness mutations**: 13 inherited v1584 physical/PI cases, 21 v1588
original rollback cases and 14 new populated-cache, reclaim/identity,
exact-data and stale-hit cases. The retained v1588 physical two-boot
oracle also independently passes on these same media/transcripts.

## Scope, non-regression and remaining acceptance

This is genuine populated D6FS cache and original BCACHE reclaim, not
a synthetic valid cache tag or an injected reclaim call. The selected
checked-release refusal remains test-induced, at the original provider
entry, while actual owned-BACKSTORE media failure remains fatal under
the v1541/v1586 policy. Because original MM reclaim intentionally
discards the cache, **retention of an otherwise valid populated cache
entry during a recoverable unrelated provider rollback** remains OPEN.
Also open: mixed DEVICE/VOLUME provider failures, multiple simultaneous
owner transitions and loaded-simulator timing stability including the
isolated v1585 timeout. P1-D and P1-E remain OPEN.

The separate legacy native BCACHE/DTC fixture v1490 currently fails
to link because its extracted modern `dtfs_dtc_write` veneer depends
on `dtfs_intent_mount_gate_v1561` (not included in that old minimal
source link). Do not stub/bypass a production write-intent gate to
make the fixture pass. This testkit maintenance issue is distinct
from the accepted full physical v1589 result.

All v1589 instrumentation lives in disposable test kernel and opt-in
userland copies. Production KCORE source, RAM, ABI and on-disk format
are unchanged: VERSION 0.5, low end `024364` octal and strict 32K/
optimization HOLD. Source-backed details and reproduction:
`daimos-testkit/tests/validation/daimos-p1c-populated-cache-physical-v1589.md`.
Retained full physical evidence:
`$TMPDIR/daimos-p1c-populated-cache-v1589.LkkpE8`.
