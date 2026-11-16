# P1-E v1631: matched 48K PURE-ON/OFF capacity regression

Date: 2026-11-16. Base: v1630 first physical v1384 in-place
restore accepted on five-PACKED-child v1629 guest. This work
changes only testkit diagnostics and documentation, not kernel,
compiler, MM, swap, MRES, ABI, or boot-media implementation.

## Source-matched 48K experiment

The SAME original C89 source, GCC/PDP-6 assembler output and
`/SYSTEM/INIT` DXR were installed into separate genuine PDP-6
48K no-AUX TSFS root boots. The linked `kcore.map` files are
byte-for-byte identical. The one build selector is
`DAIMOS_PURE_MRES_ENABLED=1` (v1629) versus `=0` (v1631).
The original `kinit.map` contains
`proc_pure_mres_package_v1612` only in the PURE-enabled build;
both boot correctly, with PURE LOADED only when enabled.

Enabled image: original guest prints
`aaaaax[B013060/042502]<010000+010000>[A013060/042502]{013060-013060}LwwwwwZ`.
Five children pack, then the sixth RUN returns -1 due to the
already-established 010000 APR contiguous-fit failure. The
parent survives and collects all five exact EXIT statuses.

Disabled image: original guest prints `aaaaa!uwwwwww` and HALTs.
Here the fifth child's `a` marker precedes a sixth RUN that
**succeeds**. The original source explicitly treats that as
unexpected (`!u`) and exits through its negative assertion;
the six children then each print their `w` marker. This is
NOT a normal full parent WAIT/Z test, but it is unambiguous proof
that the disabled kernel admitted six distinct child RUNs.

## Real MM owner snapshot before the disabled parent exits

The unchanged PURE-disabled guest was replayed with a SIMH
console stop immediately after `aaaaa!u`, before the parent
completes assertion cleanup. The original KCORE map was used
to read all physical MM arena/extent descriptors. All values
are octal PDP-6 words.

| Source-backed physical property | PURE-on v1629 B | PURE-off v1631 after sixth RUN |
|---|---:|---:|
| MM arena | 042562..140000 | 037224..140000 |
| Extent count | 022 | 025 |
| Live child owners | 5 PACKED | 6 PROCESS |
| MM occupied | 057023 | 074634 |
| MM free | 016173 | 003720 |
| Largest 02000-aligned gap | 007000 | 002000 |

In the disabled case all seven unpinned full PROCESS owners
(six children and the parent) are consecutive APR images:
slot 7 `046000..056000`, slot 6 `056000..066000`, slot 5
`066000..076000`, slot 4 `076000..106000`, slot 3
`106000..116000`, slot 2 `116000..126000`, parent slot 1
`126000..136000`. The owner table is sorted, overlap-free and
contains no PACKED descriptors. The paired physical testkit
auditor PASSes and rejects a forged foreign PROCESS owner and
forged missing sixth-RUN success.

The enabled arena starts at 042562, while disabled starts at
037224. Their exact difference is `003336` octal = 1758 decimal
words, identical to the installed PURE MRES image length. In
the enabled case, each of five `001000`-word omitted prefixes
is a SMALL free gap between retained `007000` suffixes. The
total physical free words are higher after packing, but none
of those gaps combines to fit one `010000` complete child APR.
Without the optional MRES, the allocator can place six full
child images contiguously before running out of aligned space.

## Interpretation and next measurement

**The current RAW PURE mechanism is a process-admission
regression for this particular 48K workload.** It does not
turn aggregate free memory into a useful contiguous VM extent;
the installed module also consumes permanent RAM. The six-child
disabled and five-child PACKED enabled snapshots have unequal
live population, so do NOT divide the occupied/free difference
into an alleged equal-load net RAM saving. This is not evidence
of an assembler bug or an MM correctness failure.

The next meaningful experiment is a genuine SOURCE-IDENTICAL
five-child snapshot in BOTH configurations, before another RUN
or cleanup changes the owner count. Compare managed arena/free,
largest aligned extent, and actual footprint with MRES and
owners counted. Later investigate whether PACKED owner placement
can support enough contiguous RAM to make the feature worthwhile
without adding permanent per-process registries or unsafe copies.

Full/full-relocated restoration under forced obstruction,
source-loss/PI6 rollback, transient physical peak and positive
net RAM remain OPEN. Keep PURE default OFF; strict 32K HOLD.
