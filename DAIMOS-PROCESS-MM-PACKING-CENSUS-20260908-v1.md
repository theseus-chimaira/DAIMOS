# DAIMOS process/MM representation packing census

Date: 2026-09-08

Base tree: `21ddbf384e740726d0b59c5e23e2666a26114df0`

## Implemented

96K systems now retain the 128-slot process/swap metadata policy used by 64K
systems.  The 256-slot policy starts above 96K.

Per logical process slot the current dynamic tables consume seven words:
three process-descriptor words plus four swap-record words.  Therefore the 96K
change removes 128 unused slots and saves:

- `001600` / 896 words of dynamic kernel RAM;
- old 96K process+swap tables: `003400` / 1,792 words;
- new 96K process+swap tables: `001600` / 896 words.

With one protected DSH, the post-UUO census 96K occupied/free figures become:

- occupied: `052355` / 21,741 words;
- free: `225423` / 76,563 words.

The architectural 8-bit slot/PID representation remains unchanged.

## Process descriptor packing review

The current three-word descriptor cannot be reduced to two words without
removing semantics or imposing new address/range restrictions:

- `mem_layout` consumes a full word: 18-bit resident extent size plus 18-bit
  physical base;
- `sched` consumes a full word: 18-bit wait channel plus all 18 scheduler bits;
- `meta` carries the 18-bit entry/u-area base plus exactly 18 RH bits of PID,
  parent slot, and process flags.

No process-descriptor packing change is made in this pass.

## MM extent descriptor review

The two-word MM extent descriptor is also already structurally tight:

- `span` requires two full 18-bit halves for physical base and extent size;
- `meta` carries owner, type, and pin count.

A one-word descriptor would require lossy alignment/range assumptions that are
not valid for movable modules and arbitrary free extents.  No such change is
made.

At first protected-user entry on a normal 256K production boot,
`mm_extent_count` measures `021` / 17 descriptors out of 32.  This is not enough
headroom evidence to lower `MM_MAX_EXTENTS`: allocator splitting and optional
modules can raise the transient count, and descriptor exhaustion must remain a
separate error from physical-memory exhaustion.

## Module runtime metadata review

The owner-indexed runtime table remains at 20 two-word entries.  Shrinking it to
normal-boot occupancy would discard support for optional modules or require a
new sparse lookup representation; that is not justified by the present
measurement.

The eight-word dynamic-binding table is not oversized in the supported maximal
configuration.  Source inspection gives a possible simultaneous maximum of
exactly eight movable-module republish sites:

- DPY clock binding: 1;
- TTY CTY/DCS/GE bindings: up to 3;
- DTFS DTC read/write bindings: 2;
- D6FS DISKSET read/write bindings: 2.

Therefore `MODULE_DYNAMIC_BIND_MAX == 8` is retained.

## Result

The safe representation/table saving in this pass is the 96K slot-policy
change: 896 words.  Further process/MM field packing would currently trade
correctness/generality for small or unproven gains, so the next optimization
phase should proceed to C-level structural sharing.
