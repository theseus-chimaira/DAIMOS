# Inline codec Stage-1 v072, inactive source variants

Each of the five PDP-6 Stage-1 devices (dsk, drm, dtc, mtc, ptr) has a
separate `stage1-inline-v072.s` that includes the shared v067 inline-token
fixed decoder through `decompressor-inline-v072.inc`. The include retains
legacy BLT image labels and fixed entry address 000060 so the existing
Stage-1 loader logic is unchanged. Production `stage1.s` and legacy
`decompressor.inc` remain active.

Testkit `tests/validation/d6codec-stage1-v072/run-v072.sh` successfully
assembles all five staged programs with DAS `-F -K`. This does NOT prove
that the larger installed low-core image is safe with each Stage-1's
execution/storage layout or that any boot device successfully loads KINIT.

Next gate: generate inline compressed KINIT payloads from host v068,
validate source/destination/core overlap against five native Stage-1
images, boot every device in the simulator, then activate the complete
image format and TSFS/EXEC/PURE consumers atomically. Resident 8K KCORE
ceiling still applies; optimization deferred by request.
