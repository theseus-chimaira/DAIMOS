# KCORE P0-A v1541: BACKSTORE service failure is fatal

Date: 2026-11-13. Feature completion only, no KCORE optimization.
Base: v1540 disk-root and v1539 TSFS root-loss acceptance.
VERSION 0.5; strict 32K and size optimization HOLD.

## Policy

Treat BACKSTORE like the required root filesystem: a *valid* critical
backing read or write that cannot complete after the storage driver's
ordinary bounded recovery must HALT the machine. It does not matter
whether physical media absence is permanently detectable. The final
transfer failure is an unusable required service. It must not return
to the process scheduler to schedule another swapped-process attempt.

This deliberately supersedes the v1511/v1512 historical acceptance
of a negative physical Type-236 PI2 completion or PI6 watchdog
returning to a process for later retry. Hardware retries that complete
successfully within the driver are still fine; the new terminal gate
operates only on the final failed service result. Any final failure
on an active or already-owned BACKSTORE halts, even if that failure
might have been transient and a future request might succeed. This
is the same deliberate availability-versus-integrity tradeoff as
v1538 selected-root fail-stop.

## Actual source path

`system/kernel/fs/fs_mres.s` implements `backstore_read` and
`backstore_write` through a single fixed `backing_region_io` gateway.
It checks zero-length, NULL buffer, negative/out-of-bounds range and
unconfigured storage before transfer. A valid request then routes
either to DSK270/DRM236 direct DEVICE I/O or the installed BLOCKSET
VOLUME mapper followed by the same physical transfer. The shared
completion point restores its saved registers and caller stack.

The v1541 failure tail tests AC1 after this checked completion.
A successful transfer returns normally. A failed transfer dispatches
with AC1=2 into fixed `kcore_critical_device_lost_v1515` when
`backstore_enabled` or `backstore_blocks_used` is nonzero. The v1515
leaf disables PI and HALTs at the **BACKSTORE** diagnostic PC,
distinct from root and invalid reason. There is no retry there and
no attempt to drop, reconstruct, unpin or free missing owners.

Both state checks are needed. `backstore_enabled` is an admission
switch, not proof that every old DISK owner was retired; turning it
off while a process still owns blocks cannot make its image optional.
Conversely, KINIT may probe a not-yet-enabled, wholly unowned
optional snapshot region: those failed probes return the usual
negative result and are not fatal. Logical argument and capacity
errors also remain nonfatal without touching physical storage.

No duplicate backstore lifecycle counter, new syscall, new persistent
DATA/BSS, or controller-specific media-loss bit is introduced.

## Source-linked PDP-6 evidence

`daimos-testkit/tests/validation/daimos-backstore-failstop-v1541.sh`
extracts actual fixed gateway and v1515 fatal source and executes
17 native cases plus six unsafe source mutations. Cases cover direct
DSK270/DRM236 READ/WRITE failure, an error after one successfully
transferred block, unavailable mapped VOLUME service, a disabled
but still-owned BACKSTORE, disabled/unowned errors, valid successes,
zero-length, invalid/out-of-range/NULL-buffer and no configured
region. Every fatal result is checked against the actual **swap**
HALT PC, not just that the simulator stopped. Mutation controls
reject missing fatal branch, wrong root reason, ignoring existing
owners or enabled state, false-positive success and skipping mapped
VOLUME failure.

Full tape-root KCC fixed KCORE and matching optional filesystem MRES
relink PASS. This integration is in fixed KCORE's shared backing
gateway and is independent of whichever removable FS MRES is loaded.

## Physical acceptance and remaining gates

The testkit's `daimos-backstore-failstop-fullguest-v1541.sh` reuses
the v1511 source-linked selected Type-236 PI2 error injector in a
disposable booted 256K KCC guest. The test must demonstrate that an
actual failed BACKSTORE transfer reaches the current linked v1515
BACKSTORE HALT PC; a preexisting v1511 source expectation of later
scheduler progress is no longer an acceptance criterion.

Physical failure injection and any successful results must be
recorded separately from native testing. Source-linked native PASS
alone cannot establish the full-guest actual-PI failure/HALT case.

Remaining P0-A even after BACKSTORE acceptance: direct DRM root
physical outage, unrelated secondary disk/drum negative control,
and shared DEVICE/VOLUME root-plus-BACKSTORE fault matrix, as
hardware profiles support. P0-B and P1-C/D/E are separate gates.
