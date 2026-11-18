# v1669: real PDP-6 physical MM geometry before KINIT reclamation

Date: 2026-11-18. Accepted parent DAIMOS v1668 commit:
`d2c9ed455966d4be611048afb63abc7566ba2be5`.

## What was measured, and what was NOT

The v1668 strict TSFS/PURE image removes an unused disk-swap MRES
candidate and lowers the **linked KINIT image end** by 1877 GCC
lowmem words (1856 KCC FULL). Previous evidence carefully did
not call these additional free runtime words. v1669 measures
the corresponding *actual physical PDP-6 allocator state* with
four independently executed, source-matched GCC lowmem guests:
general ROOT=tape/PURE and strict ROOT=tape/PURE-only, each at
48K and 64K installed physical memory. Every guest has a real
three-member TSFS root with DSK/DRM media detached.

SIMH stops at `kinit_late_start`, using the actual entry point
from that guest's `kinit.map`. This is AFTER resident MRES has
been installed and the early runtime tables allocated, but
BEFORE the late code reclaims KINIT source, finishes INIT process
creation, and enters userspace. The unmodified physical kernel
provides `mm_core_words`, arena count/table, `mm_extent_count`,
extent capacity and original two-word allocated extents. There
is **no guest-side probe** and no new KCORE instrument or word.

The testkit's `measure-v1669.py` checks actual physical word
values, sorted nonoverlapping arenas/extents, owner types, core
capacity, and conservation: each managed arena's physical size
equals its free gaps plus exact allocated extent words. The
permanent low resident size is read from the real patched
`sys_resident_words_immediate`, not copied from linker estimates.
The paired comparator requires identical permanent low resident,
identical MM allocated words and exact linked displacement proofs.
An impossible arena, missing MM physical count and forged MM free
total are independently produced and rejected by the oracle.

## Measured native PDP-6 late-KINIT checkpoint

| Physical core | General MM free | Strict MM free | Net free gain |
| --- | ---: | ---: | ---: |
| 48K | 21947 | 22013 | **66 words** |
| 64K | 38299 | 38365 | **66 words** |

| Physical core | General largest free | Strict largest free | Gain |
| --- | ---: | ---: | ---: |
| 48K | 10022 | 11899 | **1877 words** |
| 64K | 26406 | 28283 | **1877 words** |

At 48K, the installed permanent low resident is **17725** words
in both images, and MM dynamic allocations occupy **598** words
in six extents. At 64K, resident low is **17741** words in both,
and six dynamic extents occupy **614** words. The 16-word change
between physical core sizes is due to the core-sized process/MM
structures. Every live extent in these snapshots has type 3
(kernel dynamic). No real compressed process mode3 is active.

The high arena begins immediately above the KINIT image and
reserved stack, so shrinking the image grows that largest
contiguous free run by all **1877 words**. But an earlier managed
arena shrinks by **1811 words** because KINIT's late/source
layout shifts. The `__kinit_late_begin` symbol moves only
**66 words**; the physical net managed-free gain at this exact
checkpoint is consequently 1877 - 1811 = **66 words**, not 1877.
Both memory sizes independently reproduce the same geometry.

This distinction matters for PDP-6: a larger contiguous free
region can admit a process APR that could not fit into the
smaller high gap, even if total MM free words barely change.
The measurement validates that improvement without pretending
it is new permanent RAM or a universal allocation success.

## Separate sustained physical 48K boot

The same strict source-built, 64K-capable TSFS root image is
run on SIMH with actual physical core restricted to 48K and
no attached disk/drum backing. It reaches `MEM 48 K`,
`DTC OK`, `PURE LOADED` and `INIT V1`, then remains running
until a controlled 85-second timeout. This is a completed
48K boot/reachability observation, **not** a 48K peak-workload
or compressed-process admission. The v1668 64K boot and
positive BACKSTORE fail-stop tests remain unchanged.

## Outstanding physical peak acceptance

The present four snapshots are from one named pre-reclaim
checkpoint, NOT continuous temporal peak measurements. During
late-KINIT INIT creation, executable loading and source
reclamation, the allocator's free/occupied state changes;
routine process reclaim/restore could also overlap transient
buffers with APRs. Static `__kinit_image_end` and a single
snapshot cannot bound every such interleaving. Future work
should measure additional validated checkpoints or a bounded
event trace with source MM/allocator invariants, plus actual
48K/64K workload net/peak, KINIT stack watermark, and safe
compressed mode3 RUN/EXIT/WAIT/PI6. Physical 32K remains
deferred. No v1669 kernel code or runtime ABI changes were made.

All v1669 validation code resides only in daimos-testkit under
`tests/validation/pure-kinit-physical-memory-v1669/`.
