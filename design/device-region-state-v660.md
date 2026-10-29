# DEVICE/VOLUME/REGION runtime state vocabulary v660

The resident backing transfer implementation already uses exactly three
configuration words. This change gives them semantic source-level names:

* `device_region_map`: packed physical device selector and base block;
* `volume_data_blocks`: mapped filesystem span, or zero for direct DEVICE;
* `backing_region_count`: logical backing-region capacity.

The previous `blockset_direct_map`, `blockset_direct_blocks`, and
`blockset_direct_tail` symbols remain aliases at the **same addresses**.
`backing_region_blocks()` and historical `blockset_tail_blocks()` also
share the same entry; process swap boot now uses the unambiguous name.
This is a vocabulary and ownership clarification, not an added layer,
new memory object, change to device dispatch, or size optimization.

No on-media AUXST1/BLOCKSET formats or legacy MRES ABIs are changed.
Static testkit v660 verifies alias declarations and swap-boot call site;
v659/v658 verify fail-closed VOLUME and direct DEVICE routing.

Fresh native 32K GCC D6FS/disk image built and passed D6FS validation;
PDP-6 SIMH reached `proc_boot_init` with BSET and D6FS loaded.
The linked map confirms `volume_data_blocks` and historical
`blockset_direct_blocks` both at `020460`, `backing_region_count` and
`blockset_direct_tail` both at `020461`, and both capacity query entry
names at `005071`. Full multi-member acceptance is still outstanding.
