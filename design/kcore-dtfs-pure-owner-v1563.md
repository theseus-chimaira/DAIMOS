# KCORE P0-B v1563: staged PURE owner exclusion for DTFS unmount

Date: 2026-11-14. **P0-B remains OPEN; writable DTVOL2 is disabled.**

## Correctness defect

The v1562 VFS final-unmount barrier refuses a new FILE/cwd or ordinary
DTFS provider operation, but `proc_swap_attach()` could still publish a
new executable backing record after a sleeping RUN/EXEC load. Worse,
`proc_swap_mount_busy()` and `proc_swap_backing_busy()` skipped a process
with no stable u-area. A RUN child in SIDL owns its executable backing
before the u-area is allocated and can sleep in VM/STARTUP creation.
This left a gap between the initial VFS unmount owner scan and the
publication of a new executable owner. Nonresident and resident SRUN
owners were already checked, but pre-SRUN owners were not.

## v1563 implementation

- The real C `proc_swap_attach()` now invokes the v1562 non-sleeping
  DTFS owner-admission leaf before publishing the executable record or
  its mount-extension bit. Quiescent or retired mounts fail without
  changing process ownership. Non-DTFS backing retains the old path.
- The native `proc_swap_mount_busy()` and `proc_swap_backing_busy()`
  scan SIDL first, using the resident swap backing and the existing
  scheduler RH fifth mount-slot bit even when an MM u-area exists.
  SRUN and swapped owners retain their previous credential/u-area and
  saved-backstore paths. The owner scanner does not sleep or allocate.
- For a SIDL process that already has a u-area (e.g. a staged EXEC),
  `proc_swap_attach()` synchronizes both the u-area extension and the
  SIDL RH staging bit; normal SRUN keeps its own runnable link.
  `proc_swap_detach()` clears the transient SIDL bit in both u-area and
  pre-u-area cases. No new process word, table or owner reference.

The resulting contract is: if a PURE backing was published before VFS
began quiescence, the first owner scan refuses the unmount even if the
process is still SIDL; if publication is attempted afterward, the
admission leaf refuses it before the backing record changes. A load
already in progress that has not published PURE backing must fail its
attach after quiescence; its provider I/O needs separate real scheduling
acceptance. No physical CLEAN is attempted by v1563.

## Coverage

- Host C89 actual production attach: V2 denial and unchanged record and
  SIDL bit; V2 acceptance, 1/16/17/26 mount extension, non-PURE RUN
  denial, and unrelated D6FS unaffected.
- Actual KCC PDP-6 attach linked to the real VFS assembly gate:
  **11 native cases PASS, 2 actual unsafe mutations rejected**.
- Actual native KCORE PURE vnode and mount owner scanners:
  **26 PDP-6 cases PASS, 6 unsafe source mutations rejected**,
  including SIDL before and after u-area allocation, extension bit
  boundaries, resident and swapped SRUN owners and non-PURE exclusion.
- Earlier v960/v961 packing, v1382 native PURE backing, v1523 remote
  FILE/cwd, v1562 helper and full unmount, and v19/v417 FIFO mount
  ownership regressions PASS. The old v1084 pinned FIFO OPEN rollback
  regression passes after repairing its isolated link fixture and its
  now-explicit MM_ERR_BUSY (-5) return expectation.
- Full 256K-profile KCC KCORE and optional DTFS MRES image build PASS.
  Relative to the v1562 fixed KCORE low-end address, +042 octal =
  **34 TEXT words**, with no added permanent data, mount-table or
  process-record word. No size optimization or 32K admission claimed.

## Remaining P0-B

The existing FIFO creation/inheritance paths publish a FILE owner
first, or retain a named FIFO object whose mount ID is counted by the
VFS PIPE owner scan. The unmount test still uses simulated completion,
not actual scheduler alternation while FILE, PIPE, EXEC and DTFS
provider I/O overlap. Run an explicit real full-guest interleaving
matrix; complete physical owner-exclusive CLEAN seal/readback and
Type-551 media-generation/exchange invariants; then partial WRITE,
reinsert, replacement and second cold boot. The v1555 READ-ONLY V2
guard must remain active until every physical durability gate passes.

P0-A accepted; **P0-B OPEN**, P1-C/D/E unchanged. VERSION 0.5.
Strict 32K/optimization HOLD.
