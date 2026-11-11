# KCORE BACKSTORE corruption: physical F07 lifecycle evidence v1497

Date: 2026-11-11. KCORE feature correctness only; no optimization.

## Original full guest and false admission error

The disposable 256K KCC PDP-6 F07 v1494 test intentionally cleared
one allocated bit after a real swap payload read and prior to the
checked backing release. The guest reached physical SIMH HALT and
EXIT and finished with zero `proc_swap_blocks_used` and
`backstore_blocks_used`. It nevertheless did **not** pass the
fault-specific v1494 verifier: the originally instrumented TERM
witness did not establish immediate FINALIZE/PREVM ownership.

The original F07 `19/18` child-count error is an independent testkit
parser defect: the SIMH source banner
`edd19b17+uncommitted-changes` contributed a host `+`. The guest had
exactly 18 `+` acknowledgments and 18 `~` reaps after its `<` user
marker. Testkit v1496 (`f106218`) counts only the guest milestone
region without changing the exact expected counts. Its source-extracted
test passes a banner-contamination positive control and four negative
controls. This correction does not qualify the BACKSTORE fault.

## Fresh physical v1495 independent diagnostic

Testkit v1495 (`4716dae`, followed by `c364423`) adds read-only
counters to a disposable copy of the same guest: actual swap-in
service failure entry, return from `proc_event_apply`, TERM witness
entry and selected slot, plus independently counted TRANSITION and
PREVM mismatches. The physical guest reached HALT/EXIT. Exact
map-backed final observations:

| Evidence | Count or octal value |
|---|---:|
| Faulted slot | `010` |
| BACKSTORE starting block | `02350` |
| BACKSTORE allocated block count | `0330` |
| Injected bitmap faults | 1 |
| Exact matching checked-release calls | 3 |
| Injected bitmap repairs | 1 |
| `proc_swap_service_failed` entries | 2 |
| Returns from real `proc_event_apply` | 2 |
| TERM witness executions, selected slot correct | 2 |
| TRANSITION absent at TERM return | 2 |
| PREVM absent at TERM return | 2 |
| TRANSITION absent at third matching free | 0 |
| PREVM absent at third matching free | 1 |
| Aggregate v1494 diagnostic errors | 5 |
| Instrumented terminal `proc_swap_in_fail` entries | 2 |
| Successful swap-outs / swap-ins / restores | 34 / 34 / 34 |
| MM resource pressure / cooldown | 1 / 1 |
| Final process-swap / BACKSTORE blocks | 0 / 0 |
| Genuine guest RUN acknowledgments / reaps | 18 / 18 |

Independent physical owner, protected-OOM, MM peak, process-slot,
swap-stack and v1349 lifecycle verifiers PASS on this evidence.
The normal v1336 verifier instead correctly rejects nonzero fatal
swap-in failures; it cannot be silently reused as a fault-injection
acceptance profile. The v1494 dedicated verifier rejects `terms=2`
against its expected immediate-finalization count of one.

Hash-linked v1495 original evidence, from the disposable guest:

```
simh.out          e4e5b84b5d52c713ef2f87e4baee76c87a40ec8d23cc1775eed92e16c947100d
addresses         a5e5dd67988f2c335a8377137f2e6db825480a3406600dfa6b654787127bd7a3
build.log         f6fe9a8c4cbadf448753453753a9f346590bec9317e4419a9e1ce09f957ad8c1
simh-sampler.log  f0612f67d05c70d2bdbe0de05f959be278f7f4ecb0f006e41983f46d65c236b1
kcore.map         142bc81fc827d4de91db71e47eae5e7348d276b6c8655ac1ca678d5d99db935b
```

Guest evidence directory (execution host only):
`$TMPDIR/daimos-oom-requester-live-v1075.cdYRNI`.

## Source interpretation and remaining requirements

In `proc/proc_pdp6.s`, foreign fatal-event delivery explicitly
defers INT/TERM/HUP if an existing fatal event, WAIT-class executive
continuation, or non-user saved return prevents safe remote FILE/VM
destruction. A deferred event is recorded in the existing u-area
FD-control word for delivery at the owning return boundary. This
protects private executive stacks. v1495 proved that real event
calls returned with no immediate FINALIZE/PREVM owner, but it did
**not** establish which specific deferral predicate was taken. Do
not equate a deferred TERM with a completed synchronous finalizer or
label it an unsafe production regression without branch evidence.

The v1494 injector's claim that the third matching `backstore_free`
must be an idle PREVM finalizer is **disproved** by its zero PREVM
marker. The third call occurred in a TRANSITION-owned transaction;
the existing v1494 repair policy consequently does not prove
release by the intended idle finalizer. Final zero backing blocks
prove eventual owner cleanup, not the path that achieved it.

To complete this feature gate:

1. Capture the exact foreign TERM decision path, event return status,
   pending event bits, WAIT class and saved user/executive PC for the
   selected nonresident owner. Do not modify the legitimate stack-
   safety deferral rule merely to satisfy a test assertion.
2. Define and test a terminal failure policy when backing remains
   corrupt: pending TERM must not silently lose the DISK owner or
   cause unbounded unsafe retries. If backing is repaired, verify
   deferred TERM is delivered through the owner-safe return path.
3. Separately test a genuinely foreign-finalizable owner (without
   an active executive continuation) through checked-free refusal,
   real `proc_event_apply`, FINALIZE+PREVM, idle retry and commit.
4. Add a *distinct* fault-specific physical verifier. Leave normal
   v1336's zero-`swapin_fail` acceptance unchanged, require explicitly
   accounted failure identities and terminal state, and preserve the
   strict original nonfault guest matrix.
5. Rebuild from committed testkit sources for final qualification;
   this diagnostic used a local, pre-v1496 generated runner and
   preexisting uncommitted v1494 instrumentation edits.

The full physical corrupt-BACKSTORE end-to-end release gate is OPEN.
No KCORE source, ABI or resident RAM was modified in v1495-v1497.
