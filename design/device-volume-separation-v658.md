# Direct DEVICE and multi-member VOLUME routing v658

The legacy `blockset_direct_configure` interface conflated physical
single-device ranges with multi-member BLOCKSET tail mapping. In v658
the KINIT consumers use two distinct names and contracts:

* `device_range_configure(selector, physical_start, block_count)` for a
  bounded contiguous physical DSK/DRM range, with no VOLUME mapper;
* `volume_tail_configure(selector, direct_base, volume_blocks,
  tail_blocks)` for multi-member root tails translated by BLOCKSET.

The original `blockset_direct_configure` ABI remains as an alias.
Both paths converge on the existing single physical DSK/DRM transfer
code. This first increment is a semantic separation, **not** a new
generic abstraction or a change to the on-disk BLOCKSET/AUXST1 layout.
The runtime backing state and native multi-member layout remain intact.

Vocabulary for subsequent source migrations: DEVICE is physical,
VOLUME maps one or more devices, REGION is a bounded address span,
BACKING allocates authoritative temporary blocks, LOG records durable
sequences, and CACHE holds discardable copies. Swap is a consumer of
BACKING, not an independent physical layer. Existing symbol names must
be retired only with native compatibility tests, not mass-replaced.

Testkit `daimos-device-volume-routing-v658.sh` covers source routing
and ABI alias invariants. Full multi-member and fault-path validation
remains part of future integration gates; DTFS/TSFS is deferred.

Fresh native 32K lowmem GCC D6FS/disk image built and booted under
PDP-6 SIMH through `proc_boot_init` at `060426`, reporting BSET and
D6FS loaded. This validates the singleton direct-path bootstrap, not
the full multi-member tail runtime behavior.
