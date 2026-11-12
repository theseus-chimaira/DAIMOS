# KCORE v1508: physical sustained checked BACKSTORE refusal

Date: 2026-11-12. KCORE feature-acceptance evidence, not an
optimization or a new process-state design.

## Physical fault and source scope

Testkit `25a7993` adds a disposable v1508 256K KCC PDP-6
F07 guest descended from the accepted v1504 owner/TERM physical
suite. The guest reaches a real physical swap-in payload read,
chooses the first eligible ordinary owner (slot >= 010 octal),
and permanently refuses the checked `backstore_free` of exactly
that run. In contrast to v1494/v1504's repairable bitmap corruption,
the allocation bitmap itself stays intact, so the original run
cannot accidentally be donated to an unrelated allocator.
All real KCORE MM, slot-0 service, PI6 timer, scheduler and
deferred-event logic remain production source.

Extra diagnostic counters and snapshots are confined to this
disposable guest, never the production resident image. The
new v1508 read-only owner witness captures record, meta, VM,
scheduler word, retry epoch and FDCTL after each selected failed
service. The live Ctrl-E sampler records independent linked
process tables, scheduler/clock, real swap counters and the
original F07 milestone stream.

## Exact bounded physical outcome

The source-authentic guest produced 21 live samples and 18
F07 RUN acknowledgments. No fault was armed in samples 1-18.
Samples 19, 20 and 21 retained the same selected DISK owner:

| Observation | Sample 19 | Sample 20 | Sample 21 |
| --- | --- | --- | --- |
| Selected slot | `010` | `010` | `010` |
| Selected checked release refusals | 1 | 4 | 7 |
| Physical PI6 timer (decimal) | 36599 | 38542 | 40484 |
| Source-linked latest DISK record (octal) | `002350000330` | same | same |
| Repair/finalize/zombie | 0/0/0 | 0/0/0 | 0/0/0 |

The original run is first BACKSTORE block `02350`, length
`0330` (octal). Clock advances of 1943 and 1942 ticks
accompanied only three additional checked refusals per
interval, safely below the fixed 0100-octal-tick retry maximum.
After each failure, the owner remained RUN but nonresident DISK
(`066000000000` VM), with stable u-area and no TRANSITION.
Its control word `000004004010` retained pending TERM;
retry words `040000106357` (sample 19) and
`040000116733` (sample 21) had the transient retry bit,
not a FINALIZE marker. The slot-0 cursor was `000410`,
not the prohibited READY_ONCE for a failed slot.

The live process-swap and BACKSTORE used counters still
included the original run. There was no owner-side TERM
consumption, private fatal FILE close, finish, zombie, repaired
bitmap or false RESIDENT publication.

Strict `daimos-backstore-persistent-verify-v1508.py` PASS
on this actual physical transcript; its 13 deliberately
unsafe synthetic mutations are all rejected. The sampler
stopped deliberately after 21 snapshots. F07 emitted one
reap marker by that point but **did not HALT**: a parent
cannot complete WAIT for the still-owned, unrestored child.
The ordinary full-guest HALT/EXIT acceptance is not claimed.

## Accepted vs still open

**ACCEPTED:** real physical owner-specific clock-bounded
repeat refusal, exact DISK record and u-area ownership,
pending TERM preservation, no premature finalization, no
new process state or resident data allocation.

**OPEN:** after-fault bystander progress under a workload
with independent runnable healthy processes. The F07 sample
21 process table had the selected failed slot as its only
runnable user; it cannot prove a competing bystander's
execution. Also OPEN: actual DEVICE read failure/media loss,
reconstruction or administrative recovery, and terminal
resource reclamation for genuinely unrepairable backing.
Do not use v1508 to claim these boundaries.

## Retained exact physical evidence

```
addresses          707ef6eb96f2a1683ebdbf2da11388174ad70deb326b7452a5ceff74fd72f717
simh.out           4405fc8d981ecf8717ff38c3e6c9c8d5f2e5b64c7cbda56310cd2d2156035666
simh-sampler.log   c0c2cb2595e69f283a66d9cb677d2dc6346208741edae019208c37ead231af66
build.log          133cd2fd95855db07a6cee4a354d9cbc80ef9acaf922008c67f8ce4a172e42ca
kcore.map          5e918bb3b6a5aa5fdcc04f66cc938964025ee03f9f8267c1e65de4497f029c7c
```

Captured guest:
`$TMPDIR/daimos-oom-requester-live-v1075.2rog6j`.
Testkit report:
`tests/validation/daimos-backstore-persistent-physical-v1508.md`.
Production code remains the already committed and 256K-built
v1507 retry implementation; v1508 contributes only disposable
physical instrumentation and acceptance evidence.
