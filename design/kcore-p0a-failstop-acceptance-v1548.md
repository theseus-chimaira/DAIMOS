# KCORE P0-A v1548: critical storage fail-stop accepted

Date: 2026-11-13. KCORE feature-completion acceptance; no optimization.

## Final policy

An otherwise valid required I/O request to the *selected* ROOT
filesystem that cannot complete after the driver's existing bounded
recovery is fatal. A similarly exhausted required BACKSTORE I/O is
fatal if the swap service is enabled or still owns backing blocks.
Failure of an independent mounted secondary filesystem is not fatal.
Invalid calls, EOF, inactive/unowned boot probes, and successful
lower-level recovery are not evidence of critical service loss.

Fixed v1515 has distinct no-return HALT instructions, with PI disabled:
ROOT at octal `003743`, BACKSTORE at `003747` in the physical KCC
256K builds. v1538 classifies ROOT by exact provider+mount ID;
v1541 classifies BACKSTORE by valid failed service and actual
enabled/owned state. The first *failed requested logical service*
selects the reason even when ROOT and swap share a physical unit.
No attempt is made to guess permanent media absence or release
unreachable processes, FILEs or swap sectors.

## Accepted evidence

| Scope | Evidence |
| --- | --- |
| Root/secondary classifier | v1538 real PDP-6 native D6FS and TSFS, 23 cases/9 mutants |
| Selected TSFS root | v1538 all DTC0..DTC2 physical detach, v1539 individual member, exact ROOT HALT |
| Selected D6FS root | v1540 actual DSK270 root outage and v1543 DRM236 root outage, exact ROOT HALT |
| Noncritical filesystems | v1534 independent mounted DTFS, v1544 native DSK/DRM, v1545 physically mounted D6FS on opposite controller |
| BACKSTORE direct and VOLUME | v1541 source-linked PDP-6 17 cases/6 mutants, v1542 real Type-236 PI2 selected descriptor error with owned swap |
| Both shared DEVICE/VOLUME orders | v1546 native D6FS+BACKSTORE source linked, 12 cases/5 mutants, distinct HALTs and unchanged ownership |
| Shared DRM first BACKSTORE failure | v1547 full-guest mapped D6FS root and live swap on DR0..DR3, swap HALT `003747`, octal `0320` blocks owned |
| Shared DRM first ROOT failure | **v1548** full-guest same physical mapped DR0..DR3, genuine root data read failing after PI2 error-path selection, ROOT HALT `003743`, owned `0320` both at arm and at HALT |

## v1548 final physical proof

The actual 256K KCC PDP-6 boots `ROOT=drum BOOT=drm`. The normal
F07 swapped-process workload establishes live backing ownership.
An independent PID5 then makes ordinary root `/SYSTEM/INIT` FILE
reads. A test-only disposable D6FS cache-miss witness arms the
v1511 selected Type-236 descriptor observer *only after* the
selected root and owned BACKSTORE are verified. The real PI2
driver completion is routed through its existing error branch;
the genuine v1538 root classifier, not a test stub, reaches the
actual ROOT HALT. Observed: exactly one root miss arm, one selected
PI2 error, five earlier clean samples, enabled BACKSTORE, octal
`0320` owned at arm and at HALT, selected root `060101000000`,
DRM device selector `400000000000`, mapped VOLUME data blocks
`000000075774`. Thirteen corrupt-evidence/source variants fail
the strict independently executed v1548 verifier.

This is a real simulated Type-236 request/PI2 path with a selected
injected error completion, **not** natural removal of the drum or
proof of all hardware error modes. The earlier physical root
detach tests and secondary detach negative tests remain separate
evidence. Full details, evidence hashes and executable scripts:
`daimos-testkit/tests/validation/daimos-shared-drm-root-first-v1548.md`.

## Disposition

**P0-A COMPLETE for the accepted KCORE fail-stop feature and its
specified native/full-guest fault matrix.** v1548 required no
production code change, no additional resident RAM, no new ABI,
and no altered storage format. The earlier requirement to prove
hardware *permanence* before root HALT has been explicitly superseded
by the user's critical-service-unavailability policy.

Next authoritative KCORE feature gate is P0-B: durable rejection
of already discarded, partly written DTFS media across reinsertion
and cold boot. P1-C: concurrent swap and MEMFS persistent snapshot
with independent LOGSTORE fault activity. Neither is closed by P0-A.
VERSION 0.5 and strict 32K/optimization HOLD remain unchanged.
