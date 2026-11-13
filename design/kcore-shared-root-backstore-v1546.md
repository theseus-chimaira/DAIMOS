# KCORE P0-A v1546: shared ROOT/BACKSTORE first-failure ordering

Date: 2026-11-13. Source-linked native PDP-6 feature acceptance,
not full-guest physical combined-device acceptance.

The current two fatal service boundaries are intentionally separate:

- D6FS `d6fs_reader_get_block` uses
  `kcore_root_fs_io_failure_v1538`, which recognizes the selected
  root's exact filesystem provider and public mount ID.
- BACKSTORE `backing_region_io` uses the existing v1541 final
  transfer gate. A real failed required I/O when enabled or when
  blocks remain owned enters the v1515 BACKSTORE reason-2 HALT.

Both paths may address the same Type-270 or Type-236 physical unit.
The first failed requested service decides the fatal HALT instruction;
no controller-level global-loss flag is needed. In particular,
disabled-but-still-owned BACKSTORE remains critical. An entirely
unowned inactive BACKSTORE probe is not critical.

Testkit v1546 executes actual D6FS reader, complete `fs_backing`
direct adapter, BACKSTORE DEVICE/VOLUME gateway, and existing root/
swap HALT leaves together on the PDP-6 CPU. Only bottom physical
completion, optional mapper result and BCACHE are test doubles.
The same physical unit is selected for both logical services.
Twelve native cases and five actual-source unsafe mutants PASS:
DSK/DRM direct and mapped VOLUME, root-first vs swap-first,
mapper failure, success followed by second-owner failure, disabled
but owned, entirely unowned inactive probes, and successful I/O.
The check asserts distinct exact HALT PCs, unchanged owned-block
count and no second service after a first fatal failure.
No KCORE code, permanent RAM, ABI or media format changed.

P0-A remains OPEN for a combined real full-guest shared
ROOT/BACKSTORE DEVICE/VOLUME fault. A source-linked fixture cannot
establish simultaneous DMA/controller behavior, mapped BLOCKSET
MRES installation or live swapped process ownership during a
physical outage. Already accepted separately: v1542 real PI2
BACKSTORE failure, v1540/v1543 D6FS root loss, v1538/v1539 TSFS
root loss and v1545 unrelated secondary DSK/DRM failed I/O.

See `daimos-testkit/tests/validation/daimos-shared-root-backstore-v1546.md`.
VERSION 0.5, strict 32K/optimization HOLD.
