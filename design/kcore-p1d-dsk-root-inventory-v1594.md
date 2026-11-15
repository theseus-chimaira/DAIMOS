# P1-D v1594: real four-member DSK270 root FSINFO physical acceptance

Date: 2026-11-15. Full 128K KCC PDP-6 hardware/simulator boot from
original DSK0..DSK3 DSK270 physical D6FSR2, as a separate class
from v1593's Type-236 drum root and v1591's Type-551 tape root.
Original KINIT root-select and MM mount-slab provisioning, original
FSINFO, MEMFS and the full XIP RUN/WAIT/EXEC guest are unchanged.

The guest requires FSINFO capacity **6**: one four-member DSK root,
four potential DRM236 transports and the available MEMFS mount.
Root slot 0 is D6FS provider 6 / public mount 1, slot 1 is MEMFS
provider 4 / `/TEMP` public mount 2, slots 2..5 remain empty and
6/7 are invalid. The real guest emits
`P1D-D6FS-DSK-CAPACITY6-V1594` and completes ordinary XIP.

An original first run was invalidated by competing test simulators
sharing the legacy default DCS/GE TCP ports (1101/1201); its
functional guest behavior was correct but its SIMH attach produced
errors. The corrected disposable runner gives each run independent
listeners and refuses simulator errors. Its complete physical
re-run exits zero. Independent host verification reads all four
actual DSK270 raw images (45,056 128-word sectors each), checks
their original D6FSR2 member marker, distinct member identities
and matching geometry: base `000001`, span `127377`, tail
`000400` octal. Three deliberately corrupted root/FSINFO
witness controls are rejected. The accepted runner and oracle
are in `daimos-testkit/tests/validation/
daimos-fsinfo-dsk-root-physical-v1594.md`.

P1-D now has physical selected-root coverage for DTC/TSFS capacity
7 or 8, DRM/D6FS capacity 6, and DSK/D6FS capacity 6. Source-linked
native tests cover broader representational capacity 1/4/8/17/26
but do not create 26 real devices. Remaining P1-D tasks are missing
hardware/provider failure inventory and high-slot errors under
alternate physically mounted layouts. P1-C remaining mixed provider
faults are separate; P1-E stays on hold for KA10-aware design.
No production KCORE, RAM, ABI, disk format or size optimization.
