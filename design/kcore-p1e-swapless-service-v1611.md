# P1-E v1611: experimental PDP-6 swapless PURE MRES

Date: 2026-11-16
Baseline: committed v1610 slot-0 mode separation and v1609 terminal
owner cleanup.
Verdict: **native source-linked implementation PASS, KINIT/MRES link
PASS, default boot unchanged, physical/full-guest RAM gate OPEN**.
This is NOT an accepted production P1-E feature.

## Implemented module and boot selection

`proc_pure_service_pdp6_v1611.c` is a source-selected PDP-6
contiguous-memory backend. A dedicated relocatable
`proc_pure_mres_package_v1611` provides two exports for the
original permanent scheduler trampolines:

1. `proc_pure_mres_reclaim_v1611` tests physical compaction,
   scans only inactive SLEEP/STOP processes other than the caller and
   excluded owner, takes existing TRANSITION, executes the v1381
   MM suffix rebase with zero copy, and checks the next requested
   contiguous allocation. It refuses any enabled, advertised or owned
   BACKSTORE; a failed drop only releases TRANSITION after checking an
   exact single unpinned original PROCESS descriptor.
2. `proc_pure_mres_service_v1611` handles an existing slot-0
   request for a RUN/PACKED owner; it tries original v1384 in-place
   PURE prefix loading and on documented fallback calls the v1391
   FULL restore with exact pinned MM capacity. It publishes the
   existing READY_ONCE cursor only after actual restored RESIDENT
   mode, nonzero APR mapping and released TRANSITION. A failed load
   does not claim DISK I/O completion or send TERM. Retry unlock
   requires an exact single unpinned retained suffix; uncertain MM
   ownership remains transition-held.

The MRES includes the existing source-linked v1381 zero-copy DROP,
v1384 fast-prefix claim/release/commit, v1391 bounded FULL restore,
the v1381/v1384 MM assembler and the shared v1386 image loader.
The historical unbounded v1381 restore and retired terminal helper
are excluded from the MRES build. The accepted v1609 terminal
MM cleanup remains in fixed KCORE. No process table, MM descriptor,
storage mode, resident queue, disk format or KA10/KI10 state changes.

The legacy isolated size fixtures were reconciled with current
source, not left with stale historical thresholds: original EXEC
TEXT is now 218 versus the v1390 fixture's 183-word baseline;
the standalone v1391 FULL restore now costs 521 words versus
the pre-source-admission 368-word budget. Those increases are
visible and capped in testkit; 521 is accepted only as part of
the **opt-in experimental MRES**, not as resident KCORE growth
or proof that compression/reclamation saves net RAM.

The built-in KINIT source map selects both PURE and DISK MRES
packages in the *experimental* dedicated TSFS image. After
`auxstore_boot_discover()`, the existing SWAP MINIT installs
PURE only if AUXST1 has **no** backing capacity. If AUXST1
exists, the original DISK MRES remains installed; other
root families still use the unchanged disk service.

**Crucial release gate:** this experimental behavior requires
`DAIMOS_PURE_MRES_ENABLED=1` at build time. Default is **0**.
With the flag absent KINIT omits the PURE module entirely,
and accepts the original v1610 no-BACKSTORE behavior. This
avoids silently deploying a module whose physical placement,
immutable-source lifetime and net RAM advantage are unproved.
The switch uses a versioned KINIT stamp so changing it
invalidates generated code.

Example opt-in research build:

```sh
make -C system/boot/pdp6 ROOT=tape BOOT_PROFILE=full \
    COMPILER=kcc DAIMOS_PURE_MRES_ENABLED=1
```

## Source-linked tests and actual size

DAIMOS-testkit `tests/validation/pure-service-v1611/run-v1611.sh`
executes the real KCC PDP-6 service and original PURE/MM/storage
transactions in the simulator with controlled loader and
compaction calls. It verifies successful no-copy DROP from
SLEEP, exact mutable-suffix retention, RUN/slot-0 fast
prefix restore, bounded FULL fallback, backing/device and
source-loss refusals, and preserved PACKED owner on failed
full loading. Original v1608 admission, v1609 terminal
cleanup and v1610 DISK/PACKED separation are separately tested.
The suite also rejects a private source mutant that removes
the final RESIDENT-mode/VM/TRANSITION guard before READY_ONCE,
demonstrating an executed refusal rather than a source grep.
The independent `boot-mode-v1611.sh` regression builds
default-OFF, experimental-ON and default-OFF again in the
same KCC build directory: PURE's module must appear only
when enabled, while fixed KCORE bytes remain unchanged.

Full KCC ROOT=tape KCORE image remains byte-for-byte identical
to v1610 (`__kcore_low_end = 024534` octal).

| Component | KCC | GCC lowmem |
|---|---:|---:|
| New KCORE fixed TEXT/DATA/BSS | 0 | 0 |
| PURE MRES installed words | **2275** | **1864** |
| PURE MRES BSS | 0 | 0 |
| Extra per-process words | 0 | 0 |

The preliminary `permanent-size` tool adds all linked MRES
packages, even though SWAP and PURE are mutually exclusive
at runtime on a dedicated TSFS boot. With the opt-in flag,
its 16K single-root placement check FAILS: KCC full
`PERMANENT_LAST=055660` octal (7089 words above limit)
and GCC lowmem `PERMANENT_LAST=046702` octal (3523 above
limit). These are *aggregate link-placement gates*, not
measurements of actual installed swapless RAM at physical
boot. They cannot be used to assert a viable runtime fit.
The matched default full KCC profile already fails the same
strict gate with `PERMANENT_LAST=051315` octal (4814 words
above the limit); the extra 2275 words are exactly the
new linked PURE MRES. The default image is not made larger
by the optional module.

Net savings require reclaiming more text-prefix words across
inactive processes than the installed PURE MRES and any
temporary MM/loader peak, before counting a benefit. A
0400-octal (256 decimal) prefix would need at least nine
inactive owners merely to offset 2275 KCC module words
(eight for the 1864-word GCC module), ignoring restore
peaks. Neither a beneficial full-system workload nor
48K/64K real physical PDP-6 guest has been accepted.

## Additional build correction

The historical KINIT Makefile defined the stack-watermark
default *after* expanding its versioned stamp target.
Calling KINIT source compilation directly could therefore
produce an unbuildable `.kinit-stack-v655-0400-0`
dependency. v1611 moves that default before stamp expansion.
The existing `daimos-tsfs-swapless-minit-v490.sh` GCC
test now passes without any special test fixture bypass.

## Work still OPEN

* Verify VFS executable-content immutability and unmount owner
  lifetime under actual concurrent KINIT/scheduler operation.
* Boot an actual PDP-6 TSFS root without AUXST1 and validate
  memory placement, MM high-water, PI6/READY_ONCE, multi-owner
  reclaim, full/prefix restore, RUN/EXEC/EXIT/WAIT/BRK and
  forced MM/source/IO failures in the complete physical guest.
* Measure net reclaimed physical words and transient high-water,
  including MRES placement; avoid enabling v1611 if net negative.
* Optimize or share the immutable executable loader only after
  source/ABI tests and measured size gain; do not duplicate
  another parser or introduce speculative backend interfaces.
* Only after production RAW acceptance evaluate optional
  mutable-RAM compression. Strict 32K optimization remains on HOLD.
