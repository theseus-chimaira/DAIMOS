# P1-C v1586: original BACKSTORE fatal on a physical swap READ/PI2

Date: 2026-11-15. Full 128K KCC PDP-6 on genuine four-member
Type-236 DR0..DR3 D6FSR2 root VOLUME. This acceptance combines the
v1585 independent scheduler PI6 wakeup with an actual mapped
BACKSTORE swap READ descriptor and selected original DRM DR_DONE PI2
failure **while MEMFS snapshot shutdown is still running**.

## Execution path and constraints

The full original KINIT/INIT/DSH guest loads DRM236, BLOCKSET
INTERLEAVE, LOGSTORE, MEMFS and D6FS from four independently formatted
physical drum images. A privileged userland workload starts four
genuine 24K-word private processes and lets all four reach timed
SLEEP with original positive disk-backing ownership. The disposable
test-only v1585 snapshot-boundary helper shortens one actually
sleeping, nonresident, disk-backed child's existing timer to the
next original 60 Hz tick. It does not mark it RUN, enqueue it,
call swap-in, create a software PI6 or fake any DMA completion.

Original PI6 timer service wakes that child and the independent
slot-0 scheduler enters original `proc_swap_in`. The real mapped
BACKSTORE READ selects its original Type-236 descriptor and hardware
group in `drm236_runtime_request`. In a disposable source copy,
only the FIRST actual DRM DR_DONE PI2 for the already identified
slot-0 swap READ is directed into the unmodified `drm236_pi_error`
handler. That handler retires the original descriptor, accounts
the real driver error and publishes failure. The original fixed
`fs_mres.s` critical BACKSTORE gateway then jumps to original
v1515 reason-2 fail-stop. No simulated syscall failure or direct
HALT is injected. No production source, kernel memory footprint,
ABI, disk format or scheduler policy is modified.

## Original linked and raw physical observations

One full 128K physical simulator boot passed and halted at exactly
`004011` octal, the linked original v1515 BACKSTORE reason-2 HALT.

| Independent witness | Observed original value |
| --- | ---: |
| Real timer arm/PI6 timer wake | 1/1 |
| Selected disk-backed sleeping process slot | 6 |
| Independently scheduled swap-ins started | 1 |
| Actual mapped swap READ requests | 1 |
| Matching real DRM DR_DONE swap-READ PI2 | 1 |
| Total real snapshot-window DRM DR_DONE PI2 | `0343` octal |
| Original successful DISK-to-RESIDENT publishes | 0 |
| Original driver DRM error counter (`mfsdev_storage_errors+5`) | 1 |
| Original generic logical storage errors | 0 |
| Original process swap blocks still owned | `01650` octal |
| Original BACKSTORE blocks still in use | `01734` octal |
| Original snapshot-in-progress marker at fatal HALT | all 36 bits set |
| Original unfinished synchronous READ marker | all 36 bits set |
| Original selected descriptor, one-shot fault arm | both 0 |

The real original Type-236 descriptor group is `0375060` octal.
Its selected physical unit is **DR1**, physical block `017506`,
which the original production BLOCKSET interleaving maps to logical
BACKSTORE block `000431` octal. The independent host oracle derives
these addresses only from the original KCORE-linked descriptor group
and actual on-drum D6FSR2 member layouts, rather than trusting the
test injector's descriptions. The selected physical sector contains
real backing payload. The original MEMFS v2 snapshot header at DR3
sector `017777` remains uncommitted (first 36-bit word zero) and
all four physical D6FSR2 descriptors remain byte-for-byte unchanged
from the initial boot images. There are 268 OTHER nonzero physical
BACKSTORE tail sectors outside the final header.

Fifteen independent destructive evidence variants were rejected:
omitted selected PI2/error accounting, uncleared injection arm,
missing actual timer wake, no original scheduler swap or READ,
absent matching physical PI2, a false resident publication,
lost swap ownership, bogus physical group or root VOLUME span,
snapshot marker cleared too early, erased physical swap payload,
falsely committed snapshot header, or altered D6FSR2 layout.

The full new physical regression and all fifteen mutants PASS,
and the retained original v1583 and v1585 physical oracles still
PASS. Primary testkit runner and evidence:
`tests/validation/daimos-p1c-swap-read-fault-physical-v1586.{sh,md}`;
`daimos-p1c-swap-read-pi2-fault-v1586.py` and
`daimos-p1c-swap-read-fault-verify-v1586.py`.
Original retained physical simulator directory:
`$TMPDIR/daimos-p1c-swap-read-fault-v1586.5lg2J3`.

## Limitations and next P1-C work

The DR_DONE and its real descriptor are authentic, but this is a
selected **test-induced error**, not naturally damaged drum media.
This result proves fail-stop correctness and retained nonresident
ownership for an independent first swap READ failure during snapshot
DMA. It does not establish recoverable MM/BCACHE/owner rollback,
all possible later-sector faults, continued operation after an
error, or safe handling when multiple owners are simultaneously
in transition. Those remain P1-C acceptance work.

The v1585 intermittent 180-second probe timeout remains recorded as
an unresolved simulator-load-versus-scheduler-stall observation.
There was no timeout in this v1586 test, but one successful run is
not a repeated-load stability proof. P1-C stays OPEN; P1-D/P1-E
stay OPEN; VERSION 0.5, fixed production KCORE end `024364` octal,
and strict 32K/optimization HOLD remain unchanged.
