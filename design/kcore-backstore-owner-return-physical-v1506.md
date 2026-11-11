# KCORE BACKSTORE: repaired physical bitmap fault through owner TERM, v1506

Date: 2026-11-11. KCORE feature-completion evidence; no optimization.

## Source-authentic physical guest and tested recovery

A new disposable 256K KCC PDP-6 F07 v1504 guest completed its
physical disk-backed workload and reached genuine HALT/EXIT.
It source-linked the existing v1494 checked-bitmap corruption and
v1499 real before/after event snapshots, plus new map-backed witnesses
at production `proc_deferred_term_v970`, `proc_deferred_commit_v970`,
`proc_fatal_close_current_v879`, `proc_finish_common_v932` and
`proc_finish_zombie`. These witnesses change no control-flow
decision; all additions are confined to the disposable test build.

The selected process was slot `010` (octal), nonresident DISK, with
original backing first block `02350` and length `0330`. The bitmap
was corrupted once after an actual successful payload read; exact
checked release refused twice with real scheduler TERM requests;
the injector repaired precisely the removed bit at the third
matching free. Physical pre/post event records prove both scheduler
TERM requests coalesced with the **already-pending TERM** without
changing the DISK/scheduler/u-area owner; the saved PC was executive
and WAIT clear. The process's FD-control word was already
`000004,,004010` at the time the injected fault was armed.

After the backing was repaired, the exact same process reached the
real owner-side deferred-event consumer **once**, with TERM pending.
The consumer cleared all pending fatal bits and set the existing
private fatal-close latch: FD control became `000000,,004014`.
`proc_fatal_close_current_v879` was entered exactly once with
status `0400001`. `proc_finish_common_v932` was entered once with
the same status, and `proc_finish_zombie` ran once for that slot.
The private-return swap record was octal `037`, not the original
DISK run; this is consistent with `proc_swap_in` replacing the run
with its restored resident backing identity. Final process-swap and
BACKSTORE block counts were both zero. The F07 guest emitted exactly
18 child RUN acknowledgments and 18 WAIT reaps.

The per-slot deferred-event witness recorded three deferral entries
for slot `010`, with TERM as the last kind and a previously pending
TERM in the FD-control word. The workload's source explicitly sends
TERM to each child before its WAIT, consistent with an earlier parent
TERM and the two later scheduler requests. Only the last kind was
retained; do not claim every earlier kind has separate provenance.

## Strict verification and limits

Testkit `2bf10ff` `daimos-backstore-owner-return-verify-v1504.py`
requires the prior strict v1503 fault/coalescing/bitmap/F07 verifier
and **exact** target-specific pending-bit consumption, fatal status,
one finish and one zombie publication. It rejects **20** wrong
owner/exit/bit/slot/record mutations. The inherited v1503 verifier
also rejects 15 negative owner/bitmap/milestone mutations; both PASS
on the same full guest. Independently, the physical v1159 protected
owner, v1203 MM peak, v1205 process-slot, v1207 swap-stack and
v1349 full owner-lifecycle verifiers all PASS.

The original v1336 normal-swap verifier remains unchanged and
correctly refuses this deliberately faulted guest's two failed
swap-in counts. It is not a valid negative control for a deliberately
injected failure. v1504 is the separate strict **repairable-fault**
physical acceptance, not a claim that normal no-failure acceptance
has been weakened.

Evidence SHA256 from the completed v1504 guest:

```
addresses          a7c1b547d8950196ae8300cebb566e96b76f0c54bd3052fd3b68790402d027c8
simh.out           c0f2a9e7ad74b15dfa62130a8befb9f07571b543ab17c3192bf3d80e1c37a3a9
simh-sampler.log   3f3762460228fb4208e46919c34fcb9baa89976d7c667dfb8fdfa8296e2b9701
build.log          033887138aa349b41f6d36463b02f5eb39dbb38181e72f518f57ddf60c536467
kcore.map          0327d346b65c04540044ebb7c968f62ff5443f179b46f8edc4170a907cc3148f
```

## Remaining KCORE BACKSTORE correctness

v1505 separately ran six forced checked-free refusals in native PDP-6,
proving that the same DISK/bitmap owner, counts and MM rollback remain
consistent through repeated attempts, then restore after repair.
Those six requests were explicitly imposed by the test driver; they
do not establish the production scheduler's cadence or a terminal
resolution if the bitmap cannot be repaired.

**OPEN:** permanently corrupt backing/DEVICE identity, bounded retry
or safe quarantine policy with stable DISK/u-area/private executive
continuation ownership, and physical qualification of that policy.
Do not infer this unrecoverable-fault behavior from a repaired bitmap.
Also OPEN: remaining mixed-device/BCACHE/DTFS/LOGSTORE integration,
swapless PURE-text/MM/APR lifecycle, dynamic mount enumeration and
optional mutable-RAM compression after PURE.

No production KCORE behavior, ABI or resident RAM was changed in
v1504/v1506. The v1505 production assembly modification corrected
only its swap-service return-value comment.
