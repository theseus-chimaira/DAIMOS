# v1667: stage compressed PURE terminal/FULL restore only in PURE MRES

Date: 2026-11-18. Exact parent DAIMOS v1666:
`0d30d2c4eac0fd3466f0bdeeb6c69f35931c334e`.

## Memory-backed TSFS only, with a strict second opt-in

The approved v1665/v1666 design chooses one memory-management
MRES only after root selection and validated BACKSTORE discovery.
Disk swap and PURE must not both occupy permanent memory. v1667
stages the previously tested v1659..v1664 bitmap compressed owner
logic into PURE MRES without adding a KCORE compatibility scanner
or a second per-process storage word. Its explicit build-time
guard is `DAIMOS_PBM_V1667=1` in addition to
`DAIMOS_PURE_MRES_ENABLED=1`. Both ROOT=tape and ROOT=auto can
select a TSFS root, so either supports the opt-in. Dedicated
ROOT=disk/drum or PURE-disabled builds reject the option at Make
configuration time. A ROOT=auto KINIT image may contain both
transient MRES candidates; KINIT installs the PURE one only for
actual DTC root with NO positively validated BACKSTORE. A disk
swap MRES never contains a bitmap decoder or bitmap MM scanner.

**This is only staged linkage, not mode3 production activation.**
The fixed `vm_space_destroy` still refuses bitmap mode3, and the
current PURE reclaim path only drops resident processes into
RAW mode2. The compression v1663 C frontend is not invoked by
live pressure reclaim; no real RUN/EXIT/WAIT lifecycle currently
creates mode3. Enabling this build flag alone therefore incurs
code cost WITHOUT saving process memory. Leave it disabled on
production machines until full physical admission is complete.

## Terminal ownership and publication

The selected PURE MRES terminal export index2 changes from direct
RAW `pdp6_pure_detach_v1609` to C89 `proc_pure_term_v1667` only
in the staged build. It validates a real process pointer, slot,
mode-map bounds and the current two-bit storage state, then:

* mode2 => original exact RAW PDP-6 MM owner release, with v1666
  mode2 and TRANSITION/backing/size checks;
* mode3 => source-linked v1661 compressed owner terminal, with
  PI-excluded single unpinned PACKED_PROCESS owner, canonical
  compressed-length bounds and atomic mode3->RESIDENT publication;
* mode1 or mode0 => explicit refusal (the disk MRES owns mode1;
  fixed KCORE handles harmless mode0 bookkeeping without dispatch).

The same fixed `proc_swap_detach` awaits successful physical
release before clearing the existing executable backing record,
u-area and staging credential. No fallible action follows source
retirement, and an invalid compressed owner remains quarantined.
This staged dispatch is tested by invoking the common KCORE
detach entry on native PDP-6 even though the production VM
destroy gate still rejects mode3.

## FULL restore and scheduler retry

The opt-in compilation of existing
`proc_pure_mres_service_v1612()` checks mode3 directly through
the original two-bit map. The permanent `proc_storage_expect_v30`
still refuses mode3 (unchanged contract) and continues to check
mode2 or mode0 for the historical RAW restore path. When a
slot-0 RUN service request targets mode3, PURE uses v1664's
bounded FULL APR restore and the standard full executable-loader
ABI; no second RAW-sized buffer is created. v1660 validates the
entire canonical bitmap stream before decoding, the original
30-word v1451 bitmap codec performs the actual word expansion,
and the v1664 PDP-6 MM finish deletes the exact pinned source
and consumes the destination pin before nonfallible PI-excluded
VM-base/mode0/TRANSITION publication.

A failed restore returns through a v1667-only retry helper in
`proc_pure_bitmap_restore_v1664.c` that reuses its existing exact
source owner scan, checking the immutable backing, current mode3,
the sole unpinned encoded MM owner and absence of any APR/other
process owner. Only after that proof may the service release
TRANSITION for a future attempt. Uncertain MM, mode, executable
identity or pin state leaves TRANSITION held; no spurious
READY_ONCE is published. Normal RAW mode2 restore semantics are
unchanged when the stage flag is zero.

The image includes v1661 terminal C, v1660 bounded bitmap decode,
v1664 FULL restore C and MM-finish assembler, the original v1451
bitmap encode/decode assembler, and a small mode2/mode3 terminal
dispatcher. These are located exclusively in the opt-in PURE
MRES. KCORE still has the single v1666 terminal entry; the disk
MRES exports its unchanged independent disk release code.

## Staged memory overhead (NOT net RAM savings)

Using the exact v1666 source-built KCC FULL and GCC lowmem image
baselines, respectively:

| Component | v1666 ordinary PURE | v1667 staged PURE | Growth |
| --- | ---: | ---: | ---: |
| KCC FULL installed PURE MRES | 2307 | 3992 | **1685 words** |
| GCC lowmem installed PURE MRES | 1863 | 3186 | **1323 words** |

The fixed KCORE end does not change: KCC FULL remains octal
`024353`, GCC lowmem remains octal `024076`. For ROOT=auto,
the staged PURE package also grows the transient KINIT image;
both candidates remain linked until the selected root and swap
availability are known. Physical guest high-water has not been
measured. The v1663 example bitmap compresses 3456 RAW words
to 161 encoded words, a 3295-word isolated process saving, but
this is NOT a demonstrated live compressed process or net RAM
result, and peak double residency during restore must be counted.
The separately linked **transient KINIT image end** changes
from octal `140230` to `143612` with KCC FULL (+1778 words), and
from octal `113732` to `116516` with GCC lowmem (+1396 words).
These temporary boot staging costs must not be confused with
installed MRES or actual physical RAM high-water.
An independent source-built KCC FULL ROOT=auto image with
ordinary PURE enabled but `DAIMOS_PBM_V1667=0` also links with
exactly **2307** PURE MRES words, fixed `024353` and KINIT end
`140230` octal, identical to the accepted v1666 normal PURE
baseline. Therefore the added v1667 branch does not penalize
the existing uncompressed PURE configuration.

## Validation and remaining release gates

Native PDP-6 terminal source-linked tests execute actual fixed
`proc_swap_detach`, original v1609 RAW assembler, original v1661
bitmap owner release and new dispatcher. 21 positive/refusal
cases PASS, including a same-ID MODULE owner and invalid
mode/owner/pin/backing/TRANSITION states. An independently
compiled unsafe RAW-for-mode3 dispatch mutant is rejected by
the native oracle. The service suite first runs the original
v1664 source-linked MM allocator, bitmap commit/codec and FULL
restore baseline, then links the actual new opt-in v1612 service
and v1664 restore: 16 RUN/wakeup cases PASS, including sparse and
zero-stream exact APR reconstruction, invalid compressed bitmap,
NOMEM, false pin, partial loader writes, and unproven rollback.
READY_ONCE is emitted only on successful atomic restore.
An independently compiled unsafe retry mutant bypasses the exact
MM-source proof; it is executed and rejected by the PDP-6 oracle
because a quarantined target pin cannot permit TRANSITION unlock.

The real top-level PDP-6 Makefile rejects incompatible opt-in
ROOT/PURE configurations; KCC and GCC KCORE/PURE/DISK image maps
prove identical fixed KCORE, and show all new bitmap entry points
exclusively in PURE MRES. A physical staged 64K PDP-6 with
three-member TSFS root and no AUXST1 boots to `MEM 64 K`,
`DTC OK`, `PURE LOADED` and `INIT V1`. The first v1667 smoke
script checked the wrong version of the fixed terminal symbol
and stopped before SIMH; the corrected assertion uses the
unchanged v1666 KCORE entry, and the same source-built image
PASSes. This boot does not demonstrate mode3 RUN/EXIT/WAIT.

NEXT: route production reclaim through v1663 only after the
existing physical limits and MM layout are accepted; carefully
enable mode3 in `vm_space_destroy` only with an installed PURE
service contract, then test asynchronous EXIT, WAIT, slot-0
RUN/PI6 and actual net/peak RAM across 48K/64K configurations.
An unselected disk-swap candidate can be removed from a separate
swapless-only TSFS image after it rejects positive BACKSTORE.
PURE default OFF, bitmap stage default OFF; strict physical 32K
acceptance remains deferred.
