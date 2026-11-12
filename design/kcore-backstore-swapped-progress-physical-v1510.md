# KCORE BACKSTORE: independent swapped-process progress under held owner, v1510

Date: 2026-11-12. This is a **feature acceptance** for a booted full
256K KCC PDP-6, not an optimization, silent resource reclamation or
production KCORE change. The accepted v1507 scheduler still uses
per-owner retry epochs in existing u-area word 067, without a new
process state or permanent resident data allocation.

## Motivation and exact source setup

The repaired v1506 physical test proved eventual owning return and
deferred TERM after repair. v1508 proved bounded retries retaining
the exact original DISK owner under permanent checked free refusal.
v1509 proved a distinct **resident** PID 5 continued executing real
SLEEP syscalls while that DISK owner remained unresolved. The
remaining scheduler gate was whether a distinct **nonresident**
process could actually be restored and execute under this failure.

Testkit v1510 modifies only a disposable copy of the existing F07
256K test guest. The selected permanent BACKSTORE free refusal remains
the original source-linked v1508 test injector: an actual successful
payload read first selects an ordinary owner, leaves the real bitmap
intact, then refuses all future checked frees of that original run.
Normal production swap, MM, PI6, scheduler, event delivery and private
continuations remain unchanged.

The genuine extra child PID `011` octal performs its original RUN,
per-process BRK and ACK. Instead of waiting on a PIPE, it executes a
single `SLEEP(0120000)`-tick timer wait, then an independent repeated
`SLEEP(2)` syscall loop. All other C children retain their original
PIPE waits. The independent resident PID 5 spinner from v1509 is
preserved. This delayed child was demonstrably DISK-backed before
the fault and before its own due timer, so a later successful restore
cannot be inferred from an already-resident process.

Additional *disposable-only* test instrumentation counts (1) a
real successfully committed ordinary `proc_swap_mres_service_v481`
restore of PID `011`, (2) a checked successful swap-exchange result
for the same slot, and (3) true `proc_sleep_ticks` entries by that
slot, all strictly **after** the original refusal fault armed.
The linked SIMH map and real process-table snapshots independently
establish the candidate's DISK/RESIDENT transition and preserve
the original fault owner's identity, backing and fatal event.

## Final physical results: PASS

The booted 256K PDP-6 completed the original F07 admission and
authentic MM allocation-pressure/cooldown phase, then returned
37 source-linked read-only SIMH snapshots. The original failed
owner was slot `010` octal, first block `02350`, length `0330`
octal, authoritative DISK record `002350000330` throughout all
13 fault-on samples 25..37. Its u-area remained stable, the v1507
retry marker and pending TERM persisted, and its original allocation
remained counted in swap and BACKSTORE accounting. Checked free
refusals increased from **1 to 24** without phantom READY_ONCE,
RESIDENT, repair, owning fatal-close, finish or zombie.

| Sample | PI6 clock decimal | Refused checks | Slot 011 checked restores | Slot 011 user SLEEP | PID 5 user SLEEP |
| ---: | ---: | ---: | ---: | ---: | ---: |
| 25 | 36273 | 1 | 0 | 0 | 42 |
| 26 | 37731 | 3 | 0 | 0 | 108 |
| 27 | 39192 | 5 | 0 | 0 | 174 |
| 28 | 40652 | 7 | 0 | 0 | 240 |
| 29 | 42115 | 9 | 0 | 0 | 306 |
| 30 | 43574 | 11 | 0 | 0 | 372 |
| 31 | 45036 | 13 | 0 | 0 | 438 |
| 32 | 46495 | 14 | 1 | 1 | 440 |
| 33 | 47955 | 16 | 1 | 67 | 506 |
| 34 | 49417 | 18 | 1 | 133 | 572 |
| 35 | 50875 | 20 | 1 | 199 | 638 |
| 36 | 52336 | 22 | 1 | 265 | 704 |
| 37 | 53794 | 24 | 1 | 331 | 770 |

At samples 25..31 the independent contender was `SLEEP/DISK`;
at sample 32 its **real checked restore committed**, and it was
`SLEEP/RESIDENT`. The source-linked ordinary restore count went
from zero to **1**; the alternate checked exchange count stayed
zero. A true user syscall then occurred, with additional calls
continuing through count **331** at sample 37. This is restored
user execution, not merely clock progress, a successful MM
allocation, a speculative scheduler cursor or a post-fault
resident-by-default witness. PID 5 separately progressed from
42 to 770 authentic SLEEP entries, proving resident fairness
continued too. Real clock advanced 17,521 ticks during the fault.

The strict v1510 verifier PASSed on all finalized evidence,
including inherited v1508 exact-owner/retry checks and v1509
resident-bystander fairness. Native source injector self-tests
reject **eight** malformed guest/assembly changes; the stricter
v1510 witness verifier rejects **eleven** false ownership,
restore or user-progress observations. An initial v1510 verifier
incorrectly expected a slot `011` process-table row before PID 011
was admitted at early boot. That test-parser error was fixed in
testkit revision `d79991e`: every *fault-on* sample still strictly
requires the real mapped row. The self-test now includes the
legitimate pre-admission case. Both local extended v1508 and a
**clean committed v1508** sampler generate and compile the
v1510 runner successfully. Production KCORE was not changed.

## Evidence integrity

Testkit revisions: `0de641c` initial test, `d79991e` strict
fault-on row scope, `0808309` physical acceptance.

Guest: `$TMPDIR/daimos-oom-requester-live-v1075.2kswMw`.
Runner: `$TMPDIR/daimos-swap-no-partner-v1304.aIcepd`.

```
addresses          01571a8dd8951b11df1b2a66b64b4e83b2fb33a8fdad82d41827505015d9d3ec
simh-sampler.log   174f8d1bf811b93edb53e80d6a00acee0922cd0ee80b3f861936abd03f72efe7
simh.out           c96807be2bd8b69902e23b8b1c933ff5876a020bfc6cb05707ca235628945eb2
build.log          bdf3582a2e8a4df8ab0b3a5bccd238c5776bb312e36e6e653f744844133fa1d1
kcore.map          6ea6c47386dd5d37b8a71205198c3d49284870d40a32096babb35d891c71222e
```

The diagnostic deliberately stopped after the 37th source-linked
snapshot, **not** by normal F07 HALT. Because the failed owner was
never repaired or given its private executive return, normal
postflight rc=1 is expected; the independent strict physical
v1510 validator PASSed. No resource release or parent WAIT
completion is claimed for the irreparably blocked process.

## Remaining P0 and feature-completion boundaries

The three progressive BACKSTORE scheduler tests now cover:
checked retry/ownership (v1508), resident bystander execution
(v1509), and actual **independent swapped contender restoration
and user execution** (v1510). No quarantine state or KCORE RAM
growth was needed.

**Still OPEN:** true underlying DEVICE/media controller loss,
its timeout/recovery behavior and a safe policy for backing that
cannot be reconstructed, without unverified free or false ZOMB.
Then mixed DEVICE/VOLUME, BCACHE/DTFS, LOGSTORE/MEMFS provider
failures, F1 uncompressed swapless PURE/MM/APR, F3 dynamic
FSINFO mounts and later measured optimization/compression.
