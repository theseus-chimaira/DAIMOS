# v1598 P1-C/P1-D: physical high DTFS failure, live MEMFS, missing DRM

Date: 2026-11-15. This acceptance uses original KCORE, KINIT/FSINFO,
DTFS/TSFS MRES and RAM ownership. Only testkit and a separately
identified genuine Type-551 PDP-6 simulator repair changed.

**v1595 P1-D:** 256K PDP-6 full guest, two-member TSFS root DTC0/1
with DTC2 empty, capacity **8**. Four distinct DTVOL1 native tapes
on DTC3..DTC6 mount at public IDs 2..5. Real DTC3 physical detach
returns FSINFO -1 on its still-published mount; neither returns 0
nor disturbs surviving high slot identities. Different valid
72-bit physical reel is rejected; original reel revalidates, all
mounts retire. Original CTY/guest PASS plus independent five-media
RBN0 identity/checksum oracle, five deliberate negative controls.

**v1596 P1-C:** same real physical TSFS/DTFS setup and additional
writable MEMFS at `/MOUNT/RAM` public ID6. After an actual DTC3
loss/replacement/return, physically detach highest mount DTC6.
Its original FSINFO refuses, other native DTFS mount IDs and
MEMFS remain intact, and userland writes/reads an exact three-word
MEMFS file while DTC6 is missing. Return of original DTC6 then
revalidates, and all mounts retire. Full 256K guest and independent
raw-media oracle pass with eight rejected unsafe mutations.

The *original* simulator repeatedly failed DTC6 revalidation even
with the exact unchanged valid reel (DTVOL1 header, 72-bit original
identity/checksum), whereas DTC3 already recovered. Inspection
of `sims/PDP10/pdp6_dtc.c` showed stale per-drive command,
motion and delay state retained while physical file buffers and
tape position were replaced. A separately built Type-551 simulator
which resets `CMD`, `DSTATE`, `DELAY` on attach/detach makes the
**same untouched DAIMOS guest** pass. This is the underlying
simulator defect, not a kernel workaround. Simulator repair
`theseus-chimaira/sims` commit `fe97912` v1598 was committed/pushed.
Versioned `$PDP10_PREFIX/bin/pdp6-v1598` now selected by the
default `pdp6` link; old v1524 binary preserved. Raw media oracle
also proves genuine five distinct physical 72-bit identities,
two-member root DTC0/DTC1 and empty DTC2.

**v1597 P1-D:** full physical 128K KCC PDP-6 DSK0..DSK3 D6FSR2
root with genuinely absent Type-167 `DP` controller and Type-236
drum units. Original `minit_drm236_probe` performs CONO/CONI on
device code `0010` (Type-167 DP). Merely disabling DR left the
actual controller service present and correctly preserved capacity
6; the test was corrected at the true hardware probe, not KCORE.
With DP+DR disabled, original KINIT advertises only **2** slots:
D6FS root ID1, MEMFS ID2; invalid higher slots, no four phantom
DRM drives, full ordinary XIP RUN/WAIT/EXEC PASS. Four real DSK
member identities and four mutated negative oracles pass.

These complete P1-D's previously open genuine absent service and
high mounted-provider loss acceptance slices. Other root/service
combinations remain part of final feature matrix; 26 is a maximum
representation, not real hardware. P1-C still needs synchronous
physical BACKSTORE ownership faults during concurrent process
swap plus different provider writes, dirty write-through and
loaded-simulator repeatability. Actual owned-media failure remains
critical fail-stop, never a recovered media error by assumption.

Testkit reports:
`daimos-fsinfo-high-media-return-physical-v1595.md`,
`daimos-p1c-highslot-memfs-return-physical-v1596.md`,
`daimos-fsinfo-dsk-no-drm-physical-v1597.md`.
No production KCORE/MRES/RAM/ABI/media changes. DAIMOS VERSION 0.5,
fixed low end `024364` octal and strict 32K/optimization HOLD.
P1-E remains held for later KA10-aware architecture review.
