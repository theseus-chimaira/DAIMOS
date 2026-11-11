# KCORE physical coalesced TERM acceptance, v1503

Date: 2026-11-11. KCORE feature completion, no size optimization.

## Exact physical outcome

The v1499 disposable 256K KCC PDP-6 guest ran to real HALT/EXIT.
It injected one checked BACKSTORE bitmap fault and, after real MM
and controller I/O, recorded two scheduler TERM calls targeting the
same nonresident DISK owner, slot `010` (octal). The directly saved
before/after event state, independently recorded for **each** call:

| Field | Octal value |
|---|---|
| Process metadata | `156000,,400401` |
| VM | `066000,,000000` (DISK, nonresident) |
| Scheduler | `200024,,000000` (RUN, no WAIT) |
| Saved PC | `340000,,015715` (executive saved continuation) |
| FD control | `000004,,004010` (TERM already pending) |
| FINALIZE | 0 |
| Event return | 0 |

Neither event changed process metadata, VM, scheduler, FD-control,
or FINALIZE state. Production `proc_event_apply` checks pending fatal
bits *before* evaluating saved-PC mode, therefore both TERM requests
were **coalesced with a prior pending TERM**. The saved executive PC
independently forbids foreign close. The previous expectation of an
immediate PREVM marker after each request was incorrect, not evidence
of a defective foreign-TERM path. The origin of the prior pending
TERM is not established by the two event snapshots.

The real physical test also recorded BACKSTORE first block `02350`,
length `0330`, one injected fault, three matching checked-free calls,
one bitmap repair, two negative swap-in service outcomes, exactly 18
guest RUN acknowledgments/18 reaps, and final zero process-swap and
BACKSTORE blocks. Independent physical v1159 protected-owner, v1203
MM peak, v1205 process-slot, v1207 swap-stack and v1349 full
owner-lifecycle verifiers PASS when replayed on this exact evidence.
The normal v1336 zero-failed-swap-in verifier intentionally fails on
the two injected faults; its acceptance criteria remain unchanged.

## Strict scope of v1503

Testkit v1503 (`8987b4a`) adds a separate *scoped* verifier for
exact real mapped event snapshots, complete SIMH HALT/EXIT,
unchanged nonresident owner, pending-TERM coalescing and physical
backing cleanup. It rejects 15 executable data/milestone mutations,
including missing TERM, changed owner, saved user PC, unexpected
WAIT, altered event return, leaked backing and missing guest reaps.
The real guest and every negative control PASS.

The original v1499 `foreign=023` and `deferred=023` branch counters
are **invalid**, due to inverted `SKIPN` event-scope guard. Testkit
v1502 (`6c3de2c`) fixes it to `SKIPE`, with native positive and
negative acceptance. These counters were not used by v1503: its
direct pre/post captures were outside the erroneous guard. A fresh
guest remains required *if* the corrected counter traces are to be
used, but not to re-establish the already-pending TERM predicate
proven by the independent physical owner snapshots.

Physical evidence SHA256:

```
addresses          22ae34d76680923eb37a5a2ffec84872e940085afa3a24910e835464bb59713e
simh.out           880d48c6d2e1977aea0de1e1eaef67ac8d162edb0f6c7eab317f2ae32c137ae7
simh-sampler.log   d570d24cdcd2b46a37ff4becb019c1e59239aac14a0abce6d67b5378cbac06f1
build.log          7eeb4e00b8d86921b5ad06055f18ed66f1dbe13b191dd53c3caac6a5446a65a5
kcore.map          29e1db2613dc217b7777c290f88b6b38895c3abbff9c7eee3a777418dbd9e92f
```

## Remaining BACKSTORE acceptance

The exact real owner-return consumption of the pending TERM is NOT
shown by v1503. Native v1501 verifies the real owner-return handler
with controlled restoration and private close. Native v1500 verifies
real immediate event/finalizer/PREVM integration with a controlled
VM/BACKSTORE backend. Neither substitutes for physical completion.

Next: capture real pending TERM consumption and final status on the
selected restored owner; verify permanent bitmap/media corruption
cannot falsely publish RESIDENT, READY_ONCE, or release the DISK
record; and add a dedicated end-to-end physical fault-specific
acceptance. Do not weaken original normal F07/v1336 requirements.
No KCORE production source, resident RAM, or ABI was changed.
