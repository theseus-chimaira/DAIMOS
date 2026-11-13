# KCORE v1534: physical mixed-provider write-fault isolation

Date: 2026-11-13. P1-C KCORE feature-completion acceptance; not
optimization, new userland features, or filesystem changes.

## Implemented configuration

The v1530 process-private full PDP-6 256K TSFS-root physical
guest uses the accepted v1532-v1533 KCC KCORE and the real
loaded DTFS/TSFS/MEMFS providers. Four mounted filesystems
coexist: TSFS root on DTC0-DTC2, two independent native
72-bit-DTVOL1 stamped DTFS volumes on DTC3 and DTC4,
and writable MEMFS at `/MOUNT/RAM`.

No volume identity or normal boot KCC image is modified by the
host. The testkit independently validates both nonzero 36-bit
identity halves, full RBN0 checksums and different DTC3/DTC4
volume IDs. Only the disposable TSFS test root gains mount
directories for the second tape and RAM filesystem.

PID1 mounts DTC3/DTC4 writable via the production DTFS
service and MEMFS via the actual INIT-only administrative
veneer, linked only into this disposable guest. It creates
a one-word DTC3 FILE, a separate two-word DTC4 FILE
and a three-word MEMFS FILE. Real STAT/OPEN/READ/CLOSE
confirm both healthy files' exact data.

## Physical fault ordering and why it matters

The native DTFS directory cache is shared among mounts.
The first attempt read the healthy DTC4 and MEMFS file
last, displacing the DTC3 directory. Detaching DTC3
then caused an *early directory READ* error on the
next DTC3 CHMOD, never reaching a physical WRITE.
After physically reattaching the original ID, that
READ_SUSPECT state was correctly recoverable.
This is an invalid test of sticky WRITE_UNCERTAIN,
not evidence of KCORE cross-provider corruption.

The corrected v1534 guest always performs a final
real DTC3 STAT while DTC3 is still attached, warming
the exact DTC3 directory after all healthy preflight
reads. At the synchronized `READY_DETACH` stage
the host pauses Type-551 SIMH, physically DETACHes
**only DTC3** and resumes PID1. CHMOD then reaches
the real DTC3 directory WRITE and returns a negative
hardware result. The existing DTFS per-mounted-unit
WRITE_UNCERTAIN flag is therefore set, as opposed to
a mere transient read-suspect bit.

The test requires ordinary CLOSE and UNMOUNT of the
poisoned DTC3 FILE to refuse without losing its
descriptor or VFS mount. FSINFO of DTC3 refuses.
While DTC3 remains physically absent, the *other*
providers continue to pass:

* Real TSFS root FSINFO/provider 7.
* Real independent DTC4 DTFS FSINFO/provider 5,
  path and exact two-word physical file content.
* Real writable MEMFS FSINFO/provider 4, path
  and exact three-word RAM file content.
* KINIT's boot-published seven-slot FSINFO count.

The host physically ATTACHes the **same** DTC3
volume; the resulting DTC3 FSINFO, pathname STAT
and FILE CLOSE all remain refused. That explicitly
distinguishes sticky WRITE_UNCERTAIN from the first
read-fault experiment. A **new DTC4 CHMOD** metadata
WRITE succeeds after the return, and all three
healthy providers pass their independent content
checks again. DTC4 and MEMFS then UNMOUNT normally,
their slots become empty, TSFS root remains active
and the unresolved DTC3 FILE/mount stay quarantined.

The accepted real PDP-6 guest reports:

```text
PASS v1534: independent physical DTC3/DTC4 volume identities
MIXEDIO V1534 START
MIXEDIO V1534 READY_DETACH
MIXEDIO V1534 WRITE_FAILED
MIXEDIO V1534 HEALTHY_DURING
MIXEDIO V1534 READY_RETURN
MIXEDIO V1534 HEALTHY_RETURN
MIXEDIO V1534 RETIRED_HEALTHY
MIXEDIO V1534 PASS
```

The test does not patch raw read/write callbacks,
use a mocked FS provider, or force off/on mounted
state from outside the kernel. The production
KCORE/MRES remains unchanged. The only additional
INIT veneer is linked into the test executable,
not common libc or fixed resident code.

## Regression and remaining scope

The v1522 native mounted-identity fixture needed
one **test-only** extraction boundary update because
the v1531 administrative replacement code added a
second physical ID comparison before the original
MRES vector. Extracting that newer function into the
old v1522 test produced duplicate mutation anchors.
With the original FSINFO space routine extracted
alone, native baseline and all eight unsafe
mutations are again checked against actual PDP-6
instructions. No production change is warranted.

P1-C **this physical provider-isolation slice is
accepted**, not the whole mixed DEVICE/VOLUME matrix.
Still OPEN: combined DSK270/DRM236/DTC routing,
in-flight mapper and backing faults, physical
BACKSTORE ownership, persistent LOGSTORE/MEMFS
snapshot plus simultaneous process swap, and
their owner/BCACHE/PI rollback interactions.
P0-A authoritative critical-media loss and
P0-B durable quarantine admission remain separate.

Zero change to fixed KCORE, optional MRES, MM
allocation policy, on-media formats or permanent
RAM. VERSION 0.5, strict 32K/optimization HOLD.
