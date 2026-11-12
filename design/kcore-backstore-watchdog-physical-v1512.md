# KCORE BACKSTORE: physical Type-236 lost completion / PI6 watchdog v1512

Date: 2026-11-12. Feature acceptance: full disposable 256K KCC PDP-6,
production KCORE unchanged from v1507. No new process state, resident
RAM word, permanent descriptor or ABI. This is deliberately **not**
an optimization or a claim of unrecoverable physical media repair.

## Distinct failure boundary

Earlier physical v1511 redirected two actual Type-236 `DR_DONE`
completions of a selected real swapped owner through production
`drm236_pi_error`; that checked negative read and MM rollback.
It did not test a controller whose completion notification is
missing. Type-167/236 already has a 300-tick PI6 watchdog with
its timeout countdown packed into the LH of the live active
descriptor pointer (no extra resident data).

Testkit v1512 retains the v1510 healthy swapped user and v1511
real drum descriptor observers in an isolated full-guest copy.
After a genuine physical swap payload transfer arms one selected
bad DISK owner, the next two actual reads of that *same* owner
execute real Type-236 DMA. Their real `DR_DONE` interrupts are
deselected **without publishing completion** for the still-live
stack descriptor. The test source neither returns a fake error
from the swap service nor changes the watchdog deadline, MM
rollback, recovery logic, scheduler or BACKSTORE allocation.
The original `drm236_watchdog_tick_v1079` must age and timeout
each request, invoke `drm236_finish_active_v1079` with -1, account
its device error, wake the original caller and propagate negative
BACKSTORE read through `proc_swap_in_read_fail` and checked MM
unpin/free rollback. Further real reads of that owner return to
the original checked-free refusal. Other processes' I/O is not
selected by the injector.

The independent v1512 `lost_pi2` witness records suppressed real
Type-236 completions; `watchdog_errors` records **only** timeout
entry for the matching still-owned descriptor; actual DRM
`mfsdev_storage_errors+5` counts failures. The direct v1511
PI2-error counter remains zero. This isolates missing completion
from immediate hardware-error completion and post-read free refusal.

## Accepted physical evidence

The full 256K guest completed 39 mapped read-only SIMH samples
with **16 fault-on samples 24..39**. Real selected owner was
slot `010` octal; BACKSTORE first `02350`, length `0330` octal.
Its exact authoritative record `002350000330`, stable u-area,
DISK/nonresident VM, allocated bitmap and used-block accounting,
v1507 retry marker and pending TERM remained unchanged. No
false READY_ONCE, RESIDENT, repair, owning fatal close, finish,
zombie or final resource free occurred.

| Sample | PI6 clock decimal | Lost PI2 | Real PI6 timeout | Refused free | PID 5 SLEEP | PID 011 SLEEP |
| ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 24 | 36219 | 1 | 0 | 1 | 42 | 0 |
| 25 | 37740 | 2 | 2 | 2 | 141 | 0 |
| 26 | 39259 | 2 | 2 | 4 | 207 | 0 |
| 27 | 40778 | 2 | 2 | 6 | 273 | 0 |
| 28 | 42298 | 2 | 2 | 8 | 339 | 0 |
| 29 | 43818 | 2 | 2 | 10 | 405 | 0 |
| 30 | 45337 | 2 | 2 | 12 | 471 | 0 |
| 31 | 46855 | 2 | 2 | 14 | 488 | 16 |
| 32 | 48381 | 2 | 2 | 16 | 572 | 100 |
| 33 | 49897 | 2 | 2 | 18 | 638 | 166 |
| 34 | 51417 | 2 | 2 | 20 | 704 | 232 |
| 35 | 52938 | 2 | 2 | 22 | 770 | 298 |
| 36 | 54460 | 2 | 2 | 24 | 836 | 364 |
| 37 | 55980 | 2 | 2 | 26 | 902 | 430 |
| 38 | 57501 | 2 | 2 | 28 | 968 | 496 |
| 39 | 59022 | 2 | 2 | 30 | 1034 | 562 |

At sample 24, `lost_pi2=1`, `watchdog_errors=0` and the
real active descriptor still had a nonzero timeout countdown.
The selected request therefore did **not** instantly return
failure. By sample 25, both selected completions had been lost,
both original PI6 deadlines had expired, and DRM recorded two
errors. Direct PI2 error count stayed **zero** throughout.
The exact final failed slot-0 service count was **32**:
**2 genuine watchdog failures + 30 checked-free refusals**.
The original bad owner was not discarded or repaired.

Over **22,803** actual PI6 clock ticks (36219..59022),
independent resident PID 5 executed **992** additional genuine
user SLEEP syscalls (42..1034). A distinct healthy child,
PID `011` octal, was physically SLEEP/DISK at sample 30,
committed one checked ordinary swap-in by sample 31, and was
SLEEP/RESIDENT while executing **562** genuine user SLEEP
syscalls by sample 39. The alternate exchange witness stayed
zero. Thus lost completion and watchdog rollback did not
prevent a separate swapped child from recovering and running.

The strict v1512 verifier PASSED on the final live physical
logs, including the inherited v1511 exact-owner and healthy
contender requirements. The source-injector selftest rejected
four broken watchdog/descriptor sites. The v1512 witness
verifier rejected twelve wrong ownership/failure/progress cases
and separately rejected a missing digit-bearing `lost_pi2`
field. The older native source-linked v1079 300-tick watchdog,
v1080 full-queue, and v1247 PI-register/negative-control tests
also PASS. Testkit acceptance revision: `3cd5e6f`, preceded
by preparation `25e0287` and parser correction `27814c7`.

## Evidence fingerprints (SHA256)

Guest `$TMPDIR/daimos-oom-requester-live-v1075.VY224I`.
Runner `$TMPDIR/daimos-swap-no-partner-v1304.ThXyOe`.

```
addresses          5a4f58057cf89e3b6eb14e5d13f314dcfb20c2061d416de3bc1221637416c540
simh-sampler.log   6f88c9084d9258ed907f81d375b8c60aef940f0e3061a2e2b603bae1b1d4812c
simh.out           4fdcf2867c18ef99a8a58827a590e879b25c59672cf2def2d26b15c2b07074c4
build.log          24902fc2b1926f4c951f996f46cb4562457d07134c27ffba614d8f04d769840a
kcore.map          0de350b2b9afbcb2414bea441151f9c11fbc389d613a2849074b478fd23ad218
```

The sampler **intentionally stopped at sample 39**, not at
normal F07 HALT. Normal F07 postflight rc=1 is expected while
the bad process remains a valid owned resource that cannot
consume private TERM. Do not report a successful complete-system
teardown or reclaim. This tests suppressed PI2 publication
after a real transfer, **not** actual magnetically damaged media,
complete controller disappearance, or verified reconstruction
of a truly unavailable backing extent.

## Remaining feature boundaries

The v1510 checked healthy swapped recovery, v1511 real PI2
error, and v1512 original PI6 watchdog/rollback now all have
scoped physical acceptance without additional permanent KCORE
state or RAM. Next: true long-lived DEVICE loss/reconstruction
and safe retained-ownership policy without forced backing free
or false zombie/parent WAIT; mixed DEVICE/VOLUME, BCACHE/DTFS,
LOGSTORE/MEMFS, swapless PURE/MM/APR and F3 dynamic FSINFO.
Any compression or size work follows functional acceptance and
must show a net RAM benefit. VERSION 0.5 and 32K HOLD unchanged.
