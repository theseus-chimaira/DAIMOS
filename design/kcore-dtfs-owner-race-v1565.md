# KCORE P0-B v1565: real scheduler crossover under V2 unmount seal

Date: 2026-11-14. **Physical full-guest scheduler race PASS.**
No production KCORE, MRES, process or mount-table source changes.

## Why v1565 was needed

v1562's native source-linked test invoked a synthetic foreign owner
attempt while production `vfs_unmount` entered provider SYNC. v1563
then covered executable PURE backing publication during RUN/EXEC.
Neither showed a real, independently scheduled PDP-6 process
attempting to create a new DTFS mount owner while the first process
was suspended *after acquiring the V2 seal*.

v1564 proved two real Type-551 PDP-6 boots on independently stamped
DTVOL2/CLEAN and WRITE_INTENT tapes. v1565 reuses that same physical
TSFS-root/DTC3 fixture, a genuine second RUN child and two anonymous
pipes, without mocking VFS, FILE, provider dispatch, syscall or the
process scheduler.

## Real scheduling witness

To force the very narrow overlap deterministically, the testkit
copies the current full 256K KCC boot image into an isolated build
directory and inserts a **test-only** 768-tick `proc_sleep_ticks`
after production `vfs_dtfs_quiesce_begin_v1562` successfully
published its seal, but before the original `vfs_sync` call. The
v1565 label is exported in the generated `kcore.map`, proving the
ordinary KCORE assembler/fold/link/boot image actually contains the
sleep. The original mounted-owner check, admission leaf, provider
dispatch, SYNC implementation, release and retirement remain
untouched. No source in the real DAIMOS tree or base image changes.

PID1 mounts the genuine DTC3 physical V2/CLEAN volume read-only,
spawns PID2 and sends it a pipe token immediately before UNMOUNT.
During the forced real scheduler interval, PID2 calls the actual
FSINFO syscall until its slot-1 DTFS SPACE provider returns -1:
the mount is still published but the v1562 admission seal forbids
provider operations. FSINFO returns +1 for an ordinary healthy
mount and zero for a retired empty slot; the test explicitly FAILS
if it misses the seal and observes retirement first.

PID2 then attempts OPEN and CHDIR on the exact mounted target.
Both refuse, and GETCWD stays `/`. It sends a race proof before
waiting for PID1 to finish real SYNC and VFS retirement. PID1
verifies FSINFO slot empty, receives the proof and releases PID2.
PID2 rechecks the empty slot and uses the now-accessible **TSFS**
`/MOUNT/DT0` directory normally, then verifies the DTFS slot is
still empty. The child exits cleanly and PID1 waits for it.
Independent full-tape SHA-256 verification proves no physical
DTVOL2 bytes changed during this read-only race.

The first run failed only in the test oracle: it incorrectly
expected the underlying TSFS mountpoint directory to be
inaccessible after DTFS retirement. That premise is false;
the corrected oracle checks empty DTFS slot plus valid TSFS
access. The rerun passed all native syscall and physical-media
checks. No production workaround or VFS behavior change was made.

## Result and still-open boundary

This closes the deliberately scheduled FILE/cwd owner-admission
crossover against the **published DTFS V2 unmount barrier**. It
demonstrates that independent processes cannot acquire a new
DTFS FILE or cwd during that interval, and cannot retain a stale
DTFS vnode after retirement. The v1562 native mutation tests
already cover skipped gate/refused admission in source-linked runs.

The test-only sleep runs **before actual provider SYNC**, so
v1565 does not establish overlapping in-flight Type-551 transfers,
the physical final DIRTY-to-CLEAN write/readback protocol,
media-generation/exchange guarantees or interrupted physical RW
data recovery. Preserve the v1555 unconditional V2 READ-ONLY veto.

**P0-A CLOSED; P0-B OPEN; P1-C/D/E unchanged.** VERSION 0.5.
Fixed KCORE and retained MM data growth: zero. Strict 32K and
general optimization remain HOLD.

Testkit: `tests/validation/daimos-dtfs-v2-race-v1565.sh`,
`tests/target/daimos-dtfs-v2-race-v1565.c`, and
`tests/validation/daimos-dtfs-v2-race-v1565.md`.
