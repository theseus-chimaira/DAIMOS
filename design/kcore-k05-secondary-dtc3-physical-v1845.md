# K05 v1845: current-source 48K physical FULL source-loss/return

Date 2026-11-23. Scope: **original physical PDP-6 PASS** for a real
secondary-DTC3 read failure during bounded FULL restoration. K05
remains OPEN. This is testkit-only work; neither KCORE, MM, PURE,
source-provider drivers nor process descriptors changed.

## Current-source test geometry

The original v1643/v1654 six-child fixture was reconstructed from
current DAIMOS source after v1839. KCORE and PURE MRES compiled and
booted on physical SIMH 48K, but the guest failed its sixth RUN
(`aaaaa!r`) before B. That is not a source-loss rollback failure and
cannot be accepted by an old six-child physical witness.

The corrected new v1845 physical fixture is derived from the exact
old v1643 user source, changing only the test-only child count from
six to five. It retains all native PIPE/RUN, original secondary DTC3
DTFS seeding and CLEAN seal, root-based first child, four secondary-
backed compressed children, one actually refused extra RUN, BRK,
mutable DATA sentinels, exact EXIT/WAIT and final HALT checks.

The baseline 48K source-built guest completed its original pressure
sequence and all five child `w` witnesses without an assertion failure.
The changed guest is not silently substituted into the historical
six-child result. Both actual image builds have byte-identical fixed
KCORE and PURE MRES bytes:

* Fixed KCORE DXR SHA256: `33bf700218a73fefcb36ebb1f1d835c1817f3a291a642898ccb3cbd5d9039d8b`
* Opt-in PURE MRES DXR SHA256: `ad246ce14079a821cd69f4c4f7b7661f5d1a0f58c118bdfa5cfa7adf2b2bfe3f`

## Original physical breakpoint and MM owner proof

The testkit boots a separate private-media source-matched simulator
probe, stops at the actual native pressure checkpoint and reads the
installed permanent reclaim trampoline. At octal `024143`, its
instruction is `254000032627` (real patched JRST). Subtracting the
original source-linked MRES map `proc_pure_mres_reclaim_v1612` offset
`000210` yields actual loaded base `032417` (octal).

The testkit independently locates the current FULL callback in the
original MRES map and checks its DXR code word. It derives the
service-return breakpoint from the actual compiled direct PUSHJ to
`proc_pure_ram_restore_v1391`, not a historic fixed instruction:

* Physical FULL bounded loader `036153`, instruction `200000000004`.
* Physical service callback return `033067`.

At the original FULL entry, slot 3 owns two physically distinct,
simultaneously pinned extents: PACKED suffix `111200..120000` with
one pin and complete temporary PROCESS APR `120000..130000` with
one pin. Selected TSFS ROOT remains mount 1; immutable backing is
on separate mounted DTC3. The host then detaches **only** DTC3 and
confirms `not attached` in SIMH before continuing original VFS I/O.

The original bounded loader returns `777777777774` (-4) at the actual
source-matched service return. The original v1391 rollback frees the
temporary target and unpins exactly the unchanged RAW suffix. After
the failed read, slot 3 retains its exact original PACKED extent and
zero pins; no incomplete PROCESS APR is published. The exact same
private DTC3 reel is reattached, four secondary-backed children
resume, and all five PIPE/EXIT/WAIT paths finish at ordinary HALT 0.

## Original MM checkpoint measurements

The following are computed solely from the original simulator
`mm_arenas`, `mm_extents`, live extent count, physical types and
sizes. These are snapshots, not continuous MM write-watch peaks or
estimates inferred from earlier baseline images.

| Original physical stage | Free words | Largest gap | Live extents |
| --- | ---: | ---: | ---: |
| B: five unpinned RAW PACKED children | 7391 | 4702 | 18 |
| A: five unpinned RAW PACKED children | 7391 | 4702 | 18 |
| FULL: one source pin + one complete target pin | 6886 | 4702 | 18 |
| Real loader failure and original target rollback | 10982 | 4702 | 17 |
| Z: all five child PROCESS/PACKED owners retired | 25876 | 25876 | 8 |

The source-linked v1845 verifier parses the actual original MM
descriptor table at all five stages, validates nonoverlap, physical
size, owner, pin, namespace identity and exact absent target after
rollback. The first TSFS-root child emits its `w` before the FULL
failure breakpoint; the four secondary-backed children emit `wwwwZ`
after reattachment. The initial verifier wrongly expected contiguous
`wwwwwZ` across simulator breakpoints; its test-only expectation was
corrected against the unchanged original physical transcript.

Two independent forged original transcripts must fail the *same*
source-linked verifier: a PACKED pin remaining after failed cleanup
and a zero FULL-loader return in place of actual -4. Both are rejected.
No production fault injection, MM owner registry or kernel test flag
was introduced. The source-linked testkit directory is
`tests/validation/pure-k05-secondary-physical-v1845/`.

## Remaining K05 boundary

This proves one source loss and same-reel return with correct FULL
rollback and independent children on source-built 48K PDP-6. It does
**not** prove simultaneous unbounded PI6 interrupt interleavings,
permanent allocator or pin uncertainty, RAW prefix corrupted source,
complete real-time continuous MM peak, alternate roots, KA10, mode3,
or production enablement. K05-K09 remain OPEN; K01-K04 CLOSED. PURE
remains OFF by default.
