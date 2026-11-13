# KCORE P0-A v1545: mounted secondary DSK270/DRM236 physical loss

Date: 2026-11-13. KCORE feature-completion work; no optimization.

## Production bug discovered by the full guest

The v1544 source-linked D6FS/native direct adapter checks PASSed,
but the first booted `ROOT=drum` test exposed a genuine MINIT ABI
misbinding: a D6FS secondary DSK270 mount returned success, while
its first FCB read failed even before any detach.

`system/kernel/modules/module_minit.c` previously patched the
ordinary and root-accounting DSK direct-adapter slots (exports
7, 8, 11, 12) directly to the **relocated DSK MRES read/write
service**. The direct filesystem adapter calls `(unit, sector,
buffer)`, but the MRES entry accepts `(encoded Type-270 physical
address, buffer)`. The resident `dsk270_read_sector` and
`dsk270_write_sector` bridge provides the required validation,
conversion and argument reshaping. The DRM direct-adapter slots
already correctly used the analogous fixed DRM236 bridge.

The fix patches all four DSK slots to the appropriate existing
fixed DSK270 bridge, conditional on the original MRES service
presence. It does not change the direct-adapter ABI, raw device
service, resident code, per-process state or media layout. Only
transient KINIT initialization code changes. The native v1545
source checker rejects four incorrect raw-service MINIT targets;
the PDP-6 executes the real `fs_backing.s` and `dsk270.s` conversion
path and rejects an actual native direct-to-MRES mutant.

## Actual secondary D6FS media and mount

The v1545 testkit builds an independent full D6FS V2 set with four
physical members using normal installed `mkdsk` and `mkd6fs`.
It runs the normal `d6fsck` clean validation, scans and verifies
all four actual DBOOT geometry descriptors and both CLEAN
superblocks, and derives the read-only 14-word `d6fs_reader`
handoff from those on-media structures. Six test-only corrupted
media/handoff variants are rejected. This is a **test-only host
scanner**: no new production userland utility or kernel trust rule
is introduced, and the handoff is not guessed from hardcoded
geometry. A live PID1 mounts using the existing privileged
`dsys_d6fs_mount` syscall and observes FSINFO public mount ID 2,
provider 6. The secondary `/MOUNT/DT0/SYSTEM/INIT` file must
exist and be opened successfully before the fault.

## Full 256K PDP-6 hardware acceptance

The testkit uses opposite controller types for the selected root
and the secondary to make loss independent of ROOT/BACKSTORE:

* Selected `ROOT=drum BOOT=drm` D6FS on four DR0..DR3 devices;
  independently validated secondary D6FS on DSK0..DSK3.
  After `SECD6FS V1545 READY_DETACH`, the host physically DETACHes
  only DSK0..DSK3. A previously opened secondary INIT's first
  data read returns an I/O error to userspace without HALT.
  A separate selected-root INIT data read succeeds. **PASS.**
* Selected `ROOT=disk BOOT=dsk` D6FS on DSK0..DSK3;
  independently validated secondary D6FS on four DR0..DR3.
  Physically detaching only the secondary drums produces the
  same nonfatal secondary file-data error, followed by successful
  root file-data I/O. **PASS.**

No synthetic device return is used by either booted physical
test. The host requires the exact ordered PID1 checkpoints
`START`, `MOUNTED`, `READY_DETACH`, `SECONDARY_FAILED`,
`ROOT_ALIVE`, `PASS`, rejects any `FAIL_*` or any HALT, and
checks that the real selected root is still readable afterward.
The earlier v1544 integrated native 24 cases/six source mutants,
v1513 secondary-return and v1515 fatal-PC tests still PASS.

## Outstanding P0-A

Single and complete root tape loss, disk and drum D6FS root loss,
Type-236 PI2-owned BACKSTORE failure and real negative mounted
secondary disk/drum loss are accepted. Outstanding: **combined
ROOT/BACKSTORE shared DEVICE/VOLUME physical failure ordering**
and exact owner-reason acceptance under a single failing physical
root/backing member. That is distinct from optional secondary
filesystem failure, which must never HALT on its own.

No new fixed KCORE instruction, static RAM/data allocation,
syscall ABI, or media-format change. VERSION 0.5, strict 32K
and optimization HOLD. Testkit reproducer and negative oracles:
`tests/validation/daimos-secondary-d6fs-physical-v1545.md`.
