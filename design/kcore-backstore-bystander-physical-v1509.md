# KCORE BACKSTORE: real independent resident bystander under sustained failure, v1509

Date: 2026-11-12. KCORE feature acceptance only, no code-size sweep,
userland redesign, MRES unloading or new process status. Production
KCORE is unchanged from the accepted v1507 retry implementation.

## Physical setup and why a new F07 diagnostic was necessary

The v1508 256K KCC PDP-6 physically accepted preservation of one
DISK owner through a permanently refused checked BACKSTORE release.
However, the original F07 INIT explicitly TERM/WAITed for its PID 5
spinner *before* the extra-child teardown that selected the fault.
No independent runnable bystander remained during the final three
v1508 post-fault snapshots. Clock progress alone did not close
the scheduler-fairness requirement.

Testkit v1509 reuses the exact selected-owner v1508 fault injector,
which operates after an authentic swap-in payload read and refuses
every checked release of that one real extent. The physical
BACKSTORE bitmap is never corrupted. The only additional behavioral
change is in a **disposable copied F07 INIT**: its original PID 5
spinner TERM/WAIT is moved after extra-child TERM/WAIT. If a child
cannot recover, INIT blocks in WAIT and the independent original
PID 5 SLEEP loop remains alive. This is an unchanged real process,
not a synthetic scheduler descriptor or counter override.

The pre-existing v1150 kernel-side diagnostic increments the
physical `daimos_test_spinner_v1150` word only when PID 5 executes
`proc_sleep_ticks`, and only after the original held-provider and
gateway prerequisites have occurred. The sampler captures this
counter, the real PI6 clock, and the actual slot-05 process state.
Diagnostic observer memory is in the **disposable guest only**.

## Accepted mapped physical evidence

Real fault owner: slot `010` octal, original BACKSTORE first
`02350`, length `0330` octal. Every fault-on sample retained the
exact swap record `002350000330`, a stable u-area, RUN/DISK mode,
the v1507 retry marker and pending TERM `000004004010`.

| Sample | PI6 clock (decimal) | Refused checked frees | PID 5 real SLEEP entries |
| ---: | ---: | ---: | ---: |
| 24 | 37630 | 3 | 98 |
| 25 | 39214 | 5 | 174 |
| 26 | 40794 | 7 | 240 |
| 27 | 42375 | 9 | 306 |
| 28 | 43955 | 11 | 372 |
| 29 | 45534 | 14 | 456 |

Over 7,904 real clock ticks, the failed owner suffered eleven
additional checked refusals, with no premature READY_ONCE,
RESIDENT publication, BACKSTORE release, repaired bitmap or
owner-side TERM/fatal close/finalizer/zombie. Its original run
remained included in swap and BACKSTORE used accounting. Every
independently mapped slot-05 state was `SLEEP/RESIDENT`, and
its real SLEEP count advanced in **every** post-fault interval:
98 to 456, a total of 358 further genuine syscalls. This proves
healthy **resident** bystander progress despite continued physical
restore failures of the unrelated DISK owner.

The strict physical v1509 verifier, including all v1508 exact
ownership/clock invariants, PASSed on the finalized snapshot and
console logs. The v1509 source-order self-test rejected four
invalid F07 mutations, the v1509 verifier rejected nine unsafe
progress/ownership mutations, and the extended local v1508 verifier
rejected thirteen additional changes. The v1509 runner was also
prepared and Python-compiled successfully against the **committed
clean v1508 sampler**, without importing the parallel uncommitted
v1508 adaptive-sampling edit.

The diagnostic sampler intentionally stopped after six fault-on
snapshots and **did not require F07 HALT**. Its normal postflight
returned nonzero because this process's backing was never repaired;
the independent v1509 physical verifier passed. This is not a
claim of terminal reclaim or that every scheduled class made progress.

## Physical evidence fingerprints

Guest: `$TMPDIR/daimos-oom-requester-live-v1075.jY2tjW`.
Runner: `$TMPDIR/daimos-swap-no-partner-v1304.v72VGD`.

```
addresses          6d62c489d6852778d4ea544a445328e550f96259247dcbc8caa7740ae9e2622b
simh-sampler.log   9bf554e74aef79f1e27175e1ec2bfd6f4f3fb7d8b510bfdccdf0234ec5ed0096
simh.out           830a2f91cb2452a2734fcb0e9730531e654af1942628e9082d2009400c5916f6
build.log          848318984daec816611743ba75d9d163b00b41a3196bacb319659b477ac1ed72
kcore.map          5e918bb3b6a5aa5fdcc04f66cc938964025ee03f9f8267c1e65de4497f029c7c
```

Testkit revision `b035da1` records the physical evidence and
`cb0f9b1` ensures clean-checkout sampler reproducibility.

## Explicit remaining KCORE boundaries

An independently healthy **swapped** contender has not yet been
proved to restore and execute during permanent fault. The observed
PID 5 was resident at all six snapshots. An actual broken DEVICE
or irrevocably lost media, its recovery/reconstruction, and safe
policy for a long-lived owner still holding backing/private
executive resources remain OPEN. No remote forced free, phantom
ZOMB, new quarantine state, extra per-process RAM or relaxed
checked-storage invariant is introduced. The previously repaired
physical owner-return/TERM path v1506 remains accepted unchanged.
