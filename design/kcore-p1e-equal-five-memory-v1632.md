# P1-E v1632: source-matched five-child physical RAM snapshots

Date: 2026-11-16. Base: v1631 matched 48K PDP-6 TSFS/no-AUX
PURE-on/off capacity regression. Testkit and documentation only.

## Original source-matched owner population

The genuine 48K original INIT C guest, compiled DXR and KCORE map
are byte-identical with PURE enabled or disabled. On the disabled
guest, SIMH examined the complete MM owner table after the fifth
child `a` marker, before the sixth RUN. The PURE-enabled snapshot
is the v1629 parent B stage after a sixth RUN was refused. Both
have exactly five living children and the same parent, but their
preceding failed-RUN history is not exactly identical.

| Original physical value, octal words | PURE on | PURE off |
|---|---:|---:|
| MM arena | 042562..140000 | 037224..140000 |
| MM extent count | 022 | 023 |
| Child representation | 5 PACKED | 5 PROCESS |
| MM occupied | 057023 | 064224 |
| Physical free words | 016173 | 014330 |
| Largest aligned free extent | 007000 | 012000 |

The arena start differs by 003336 octal, exactly 1758 decimal
words: the installed PURE MRES size. The source-matched original
snapshot contains **+001643 octal / +931 decimal free words**
with PURE enabled, already including the module resident cost in
the managed-arena boundary. This is actual sampled free physical
memory, not a measured temporal peak or complete workload net.
Five omitted 001000-octal PURE text prefixes gross 2560 decimal
words, but the observed net sampled gain is smaller.

The testkit's original-source/MM audit PASSes, rejecting both
a forged PACKED-to-PROCESS owner type and a duplicated owner.

## Crucial admission regression

The next child needs 010000 octal contiguous APR words. PURE on
has a largest legal aligned free run of 007000, not enough for
another process. PURE off has a 012000 run and physically admits
the sixth child (v1631). Hence **more aggregate free physical RAM
does not imply more available process slots**. Five isolated
omitted text gaps are interleaved with writable suffixes.

Do not address this by overriding the allocator's refusal or
reclassifying still-owned suffix extents. Before production changes,
prove a correct physical layout/ownership and restoration solution,
including fault rollback. FULL v1391 relocation, source life,
PI6 contention and true matched transient peak remain OPEN.
PURE default OFF, strict 32K optimization HOLD.
