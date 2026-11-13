# KCORE P0-A v1537: guarded TSFS root-member terminal dispatch

Date: 2026-11-13. Partial feature implementation; P0-A remains OPEN.
Baseline: DAIMOS v1536. Strict 32K and general size optimization HOLD.

## Hardware evidence and safety decision

The established Type-551/Type-136 physical controller status does not
provide an accepted positive, hardware-compatible *permanent* loss
notification. In particular, v1517's actual PDP-6 SELECT/CONI tests
show `DTB_ILL=000004` for both an unattached unit and an unrelated
unattached unit. v1511/v1512 separately show that real controller
errors and watchdog expiration can leave BACKSTORE recoverable.
Neither a negative I/O result, a timeout, nor a retry count may drive
the v1515 fatal HALT. See `design/kcore-dtc-select-status-v1517.md`
and `design/kcore-media-loss-policy-v1513.md`.

The Type-551/TU55 transport has operator-controlled OFF/LOCAL/REMOTE
states. A controller select failure is not an authenticated permanent
reel-loss assertion. No simulator-only UNIT_ATT test or fictitious
controller bit has been introduced into production code.

## Implemented first stage

Added `tsfs_confirmed_root_member_loss_v1537` to the existing
`fs/tsfs_runtime.s` optional filesystem package. This is an
**internal trusted-event entry**, not a syscall or a hardware event
source. Input AC1 is the physical DTC unit (0..7) **after** a separate
authoritative permanent-loss witness has been established.

The entry calls the exact current `tsfs_root_owns_dtc_unit_v1516`
predicate. It returns 0 for a nonmember or non-TSFS root, -1 for an
invalid physical unit/root shape/slot, and on a verified active root
member transfers irreversibly to fixed
`kcore_critical_device_lost_v1515` with reason AC1=1. The latter
disables PI and halts at its unique boot-root instruction address.
No new membership table, resource teardown, retry heuristic or BSS.

The entry intentionally has **no production caller** until physical
loss evidence can be trusted. This completes only root-member routing,
not loss detection or runtime automatic HALT acceptance. Ordinary
auxiliary DTFS read/write errors remain recoverable and cannot invoke
this entry. Fixed DSK270/DRM236 DEVICE/VOLUME and BACKSTORE role
routing remain separate P0-A work.

## Source-linked native PDP-6 acceptance

Testkit `tests/validation/daimos-tsfs-confirmed-loss-v1537.sh` extracts
the actual v1516 membership and new v1537 dispatch instructions,
links the actual v1515 fatal leaf, and runs the real PDP-6 instruction
engine. Eight cases PASS: physical root members 0 and 5 reach the
precise linked **root** HALT address; nonmembers 1/7, a D6FS root,
invalid unit, invalid root mount, and missing root mask return without
HALT. Four deliberately unsafe source mutations are executed and
rejected: bypass membership, invert member test, remove HALT routing,
and misclassify root as BACKSTORE. The test checks the real HALT PC,
not an emulated boolean result.

Existing v1515 no-return leaf PASS (three positive, three mutations),
v1516 physical-unit root member PASS (four mutations), and v1517
Type-551 selection-status negative evidence PASS. v1516's historical
source extractor was constrained to the actual predicate terminator
so an unrelated new helper cannot leak extra link dependencies into
that older test. This is a testkit correction, not kernel behavior.

`make -C system/kernel kcore COMPILER=kcc` PASS. Full tape-profile
KCC PDP-6 fixed KCORE linked with low init end `023341` and low end
`023756`; optional DTFS/TSFS MRES independently linked with the new
cross-package fatal address resolution. This does **not** establish a
new full guest physical unplug/boot acceptance.

## Exact footprint

Controlled old v1536 versus v1537 MRES relink used identical current
KCORE map, DTFS/TSFS objects, and newly computed fold plans, with
only `tsfs_runtime.s` changed. `tsfs_file_record` moved from octal
`003060` to `003065`, exactly **+5 instruction words** in the optional
filesystem MRES, with **zero additional DATA/BSS**. Fixed KCORE
unchanged. Source uses a five-instruction trusted dispatcher.

## Remaining P0-A dependencies

1. Establish a positive hardware-compatible physical permanent-loss
   producer. Type-551 `DTB_ILL`, Type-136 errors and PI/watchdogs
   cannot be substituted. If original hardware cannot distinguish
   permanent absence, an explicitly trusted operator confirmation
   is a separate policy decision, not an automatic hardware detector.
2. Route confirmed DSK270/DRM236 loss through selected root device
   members, direct auxiliary BACKSTORE, and mapped VOLUME ownership;
   distinguish root and BACKSTORE, and give critical ownership priority
   over unrelated mounts. Preserve exact selected controller **class**
   and physical units across KINIT reclamation.
3. Accept real full-guest confirmed root/BACKSTORE HALT PCs, physical
   positive events and transient/unrelated-device negative controls,
   including multi-tape TSFS and shared Type-136 routing. Recheck
   previous v1511/v1512 fault/timeout recovery and v1527-v1535
   noncritical DTFS isolation.

P0-A is **not closed** by the v1537 entry. Nothing here permits
reconstructing missing critical owners, discarding swap blocks,
or calling the terminal leaf on an unverified controller status.
