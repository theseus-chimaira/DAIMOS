# P1-E v1608: source admission before RAW PURE MM mutation

Date: 2026-11-15
Status: **staged source implementation and native negative tests PASS**.
P1-E production scheduler integration remains OPEN. This stage adds no
resident KCORE code, process ABI, MM descriptor, runtime registry or
disk format. It is the first implementation increment after the
v1607 KCORE remaining-work audit.

## Implemented

`system/kernel/proc/proc_pure_admit_v1608.h` supplies a static,
non-sleeping and CPU-independent process-executable admission check.
The staged v1381 RAW DROP and v1391 bounded FULL RESTORE now call it
after the existing v1604 quiescent/TRANSITION preflight and before
PDP-6 PURE-prefix geometry or any MM pin/allocation.

Admission fails closed when *any* process disk backing is active or
still owned: BACKSTORE enabled, nonzero advertised blocks, allocated
bitmap, owned BACKSTORE blocks or owned process swap blocks. This
is more restrictive than a disabled-swapping flag: if a backing pool
exists, the machine is not a truly swapless configuration.

The packed executable record encodes a zero-based four-bit mount
slot; the saved stable u-area credential bit 27 carries its fifth
bit. Admission reconstructs the complete public mount ID, checks
that KINIT's runtime-sized root slab exists and covers the slot,
and checks its actual published mount identity and accepted
executable provider (MEMFS through TSFS). An absent/retired mount,
wrong provider, wrong public ID or stale fifth slot bit refuses
DROP/RESTORE without touching the owner. The already existing
`proc_swap_attach()`, VFS backing-busy scans and mount-owner rules
remain responsible for original node publication, immutable
content guards and concurrent source lifetime; the new helper
does not attempt to replace those contracts.

Terminal PACKED discard deliberately skips the source-admission
check. A dead process must still release an exact MM owner even
after an unavailable provider, invalidated mount or changed
backing configuration. All existing MM type/owner and TRANSITION
checks still apply to discard.

There is NO new runtime vtable, per-process word or physical
address calculation in the new helper. The PDP-6 mapping and
sector-rounded PURE skip remain in `vm_pdp6_pure_range_v1604.h`.

## Executed native PDP-6 tests

The new versioned daimos-testkit entry point is
`tests/validation/pure-ram-admission-v1608/run-v1608.sh`.
It compiles real source-linked KCC transactions, actual existing
MM PURE rebase/expand assembly, the original storage-mode ASM,
and controlled 36-bit-word source/mount fixtures. PASS:

- v1381 DROP, no-copy suffix retention and full restoration.
- v1382 exact existing VFS backing/mount guards, including public
  mount 18 and the fifth slot bit, with authoritative live roots.
- v1384 same-base prefix claim/restore and refusal paths.
- v1385 actual executable mount/slot reconstruction.
- v1390 shared FULL-image loading with the accepted test-only
  decoder-symbol adapter after the v1464 production codec cutover.
- v1395/v1397 bounded FULL restore and MM rollback failures.
- v1604 generic lifecycle vs PDP-6 geometry tests.
- v1608 new negative admission cases for all five non-swapless
  states, missing root/slab, wrong provider/mount/extension bit,
  no MM/code/data mutation, source-loss restore refusal and
  retryable inactive owner; terminal source-loss discard.

The v1608 runner compiles a private mutated v1381 source that
unconditionally bypasses admission. The original native program
rejects it at its executed eligibility expectation. No production
source is modified by the mutant.

Two older native fixture defects were also corrected without a
kernel workaround: v1382 now publishes the public mount 18 which
its backing record already requires, and v1390 links the real
`device_state.s` owner plus a test-only four-register alias
from the historical controlled decoder to the v1458 entry point.

The full KCC `ROOT=tape` PDP-6 KCORE build remains PDP-6-only
and includes no new P1-E object. Its DXR and linked symbol map
were compared byte-for-byte with the v1606 exact baseline:
both match; unchanged DXR SHA-256
`830559842b15985c986b9145437d193cffacb7b8b654a7fb14f437788ce646ae`.
No new KCORE TEXT/DATA/BSS was introduced by the staged change.

## Still required; NOT accepted

1. E1 full production proof: select an actually diskless boot
   and establish a durable executable-provider hold through
   inactive lifetime, including concurrent VFS unmount, rename,
   write, identity change, physical read interruption and XIP.
   The current native fixture controls the provider; it does
   not constitute those lifecycle/concurrency tests.
2. E2: schedule v1381 zero-copy drop under actual slot-0/PI6,
   with atomic VM/storage owner publication, MM pressure and
   source-protection checks.
3. E3: complete bounded v1391 FULL/prefix v1384 restoration
   without a second resident decoder or unsafe overwrite, and
   preserve retryable owners on failure.
4. E4: live RUN/EXEC/EXIT/WAIT/BRK/HUP/TERM/finalizer integration,
   no-partner pressure and unchanged normal raw DISK swap.
5. E5: full KCC PDP-6 physical/negative matrix and matched
   whole-system RAM high-water measurements.
6. Only then evaluate optional mutable-only RAM compression;
   no compression codec is production-linked at v1608.

Reference architecture: `design/kcore-p1e-ka10-portable-boundary-v1604.md`
and `design/kcore-remaining-work-v1607.md`.
