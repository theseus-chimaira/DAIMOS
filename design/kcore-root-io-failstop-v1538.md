# KCORE P0-A v1538: selected-root I/O fail-stop, secondary isolation

Date: 2026-11-13. Partial P0-A feature implementation, building on v1537.
No KCORE optimization or 32K shrink. VERSION remains 0.5.

## Explicitly revised failure policy

The earlier v1513-v1537 policy demanded proof of *permanent physical
device loss* before halting on a root provider fault. That is no longer
the required root policy. The selected root filesystem is essential:
if an otherwise valid root operation cannot complete its backing read
or write after the physical driver's existing bounded error/timeout
recovery, KCORE immediately stops. It does not wait for evidence of
physical permanence. A transient controller fault that exhausts the
existing recovery budget can now stop a root guest even if a later
manual retry might have succeeded; this is the deliberate fail-stop
trade-off, not a hardware-presence detector.

Secondary filesystem errors are local. A non-root DTFS/D6FS/TSFS
filesystem retains its mount, FILE/BCACHE/dirty-owner protections and
normal error propagation. A hardware controller is not automatically
critical merely because a critical and optional volume share it.
BACKSTORE remains separately critical: this patch does NOT convert a
negative process-swap completion or PI6 timeout into a machine HALT.
The existing safe held-owner/rollback/retry tests v1511-v1512 remain
required, and a policy for truly unrecoverable BACKSTORE is still open.

## Fixed KCORE classification at the service boundary

`proc/proc_storage_runtime_v30.s` exports
`kcore_root_fs_io_failure_v1538`: AC1=filesystem provider 6 (D6FS)
or 7 (TSFS), AC2=public mount ID. It compares those fields to the
current `vfs_namespace_root` top twelve bits, which encode provider
and public mount ID. Only an exact match calls the already accepted
`kcore_critical_device_lost_v1515` with AC1=1. That routine disables
PI and halts at the root-specific no-return linked address. Otherwise
the classifier returns -1 with no side effects. An unpublished root,
an auxiliary mount, or a provider different from the selected root
cannot trigger this new fatal path. No permanent state, duplicate
mapping or new filesystem ABI is introduced.

## Real provider error sites

- D6FS: after `fs_backing_read` fails during a genuine cache miss in
  `d6fs_reader_get_block`, or after the checked write-through
  `fs_backing_write` fails in `d6fs_reader_commit_cache`. The actual
  active reader's low-six-bit public mount ID identifies the owner.
  The existing negative/NULL non-root return and no-false-BCACHE-store
  behavior is retained. Malformed inputs and logical EOF do not enter
  the root-I/O fault path.
- TSFS: failed physical `dtfs_dtc_read` in `tsfs_table_record` (FILE or
  EXTENT metadata), and failed physical reads of STORED and BLOCK
  file-data restarts in `tsfs_read_words`. Each uses the requesting
  vnode's public mount ID, not the physical DTC unit alone.
  `tsfs_table_record` now saves AC11 to preserve mount attribution
  across the DTC service. Invalid record index, missing file record,
  decode error, MM failure, EOF and zero-length reads preserve their
  original ordinary return behavior.

The older v1537 trusted confirmed-physical-root-member dispatcher is
retained, but has no production caller and is not needed to make the
new root-I/O failure decision. Root I/O is recognized by the mounted
provider, not an inferred reel-removal flag.

## Validation

`daimos-testkit` v1538 source-linked tests run the real PDP-6 CPU
instructions, extracting the actual fixed KCORE gate, v1515 HALT leaf,
and actual D6FS/TSFS error sites. Only the lower hardware callback,
cache glue and unrelated MM operations are stubbed. Exact linked HALT
PC, return values and error/success I/O counts are asserted:

- D6FS: 11 cases and five executed unsafe source mutants PASS.
  Failed root read AND root write stop at the root HALT PC; same
  secondary operations do not. Invalid logical block, unpublished
  root, different provider and successful requests do not stop.
- TSFS: 12 cases and four executed unsafe source mutants PASS.
  Root FILE-table metadata, STORED data and BLOCK data read failure
  stop at the root HALT PC; equivalent auxiliary TSFS requests do not.
  Invalid table record, other root, missing published root, valid
  metadata/data read and zero-length operation stay nonfatal.
- Prior native v1513 D6FS secondary-media retry, v1515 fatal leaf,
  v1516 root member, v1517 controller status, v1537 trusted dispatch,
  v15 TSFS slab and v1 TSFS multiextent tests PASS after source
  extraction/link dependencies were updated. The older v1 D6LZ test
  has a pre-existing absent `core/d6lz_pdp6.s` source dependency and
  is not a current-source acceptance gate.
- Full KCC 256K PDP-6 tape-root fixed KCORE and DTFS/TSFS MRES link
  PASS. The disk-root fixed KCORE and D6FS MRES link also PASS.
  Fixed new entry contains ten instructions and zero new permanent
  data words. The optional providers have small source additions.

## Full physical PDP-6 acceptance

The maintained v1534 full 256K KCC tape-root guest has been rerun with
the v1538 kernel: genuine DTC3 native DTFS WRITE failure under detach,
independent DTC4 native DTFS and writable MEMFS, and selected TSFS
root remain live. DTC3 checked owner retention, later same-medium
return and healthy provider retirement PASS. This proves an unrelated
Type-551 fault does not stop the root merely because the controller
is shared with an optional mount.

The new v1538 physical root-outage test boots a real KCC PDP-6
256K TSFS-root image. PID1 first successfully STATs `/SYSTEM/INIT`,
`/SYSTEM`, and `/CONFIG`, then reports `ROOTLOSS V1538 READY_DETACH`.
The host interrupts SIMH, physically detaches *all three selected*
root DECtapes DTC0, DTC1 and DTC2, and resumes through a queued CTY
acknowledgment. The guest opens and actually reads `/SYSTEM/INIT`
file **data** (not just metadata) to force new physical I/O.
It HALTs at **octal PC `003743`**, precisely three instructions
after `kcore_critical_device_lost_v1515` in the current linked
KCORE map, which is the root-specific `HALT .`. A userspace error
return, wrong HALT PC, missing HALT or failure marker is rejected.

Two test harness issues were diagnosed separately: an initial guest
used only warm-STAT metadata and never requested physical data, then
an initial hot-swap wrapper terminated SIMH on Control-E. Both were
corrected inside testkit. Neither was counted as physical success;
neither required a kernel workaround.

## What remains open

Native D6FS and TSFS selected-root/secondary classification and the
physical complete-TSFS-root outage/secondary-DTC separation PASS.
Still open: physical D6FS disk/drum-root loss, loss of one selected
member of a multi-tape TSFS root while others remain attached, and
actual BACKSTORE critical-failure escalation without regressing
v1511/v1512 transient swap recovery. The mixed root/BACKSTORE
shared-device physical fault suite is also outstanding.

No forced FILE close, resource release, process finalization, retry
count escalation or hardware-specific permanent-device-loss bit has
been added. This is feature completion, not optimization.
