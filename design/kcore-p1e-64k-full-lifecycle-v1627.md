# P1-E v1627: complete physical six-child PIPE/WAIT/EXIT at 64K

Date: 2026-11-16. Base: v1626 real 64K no-AUX TSFS/PURE six-child
BRK success followed by the original test's BRK-time reclaim refusal.
No production KCORE, MRES, assembler, linker, MM, process or ABI edit.

## Narrow independent lifecycle acceptance

The original v1614 C guest retained all child data/PIPE and parent
WAIT checks. Testkit v1627 generates a source-authenticated 64K
variant with six children, parent BRK target 017000, and distinct
RUN/SLEEP failure codes. Only its earlier assumption that another
whole process must be reclaimed specifically *between* B and A
MEMINFO samples is replaced by a bounded no-excess BRK accounting
check. The resulting `L` marker means lifecycle-only, NOT a RAM
reclamation claim. The old strong v1614 test stays the default.

The real source-built 64K PDP-6 no-AUX TSFS/GCC PURE guest prints:

`aaaaaa[B073470/042522]<010000+017000>[A103470/042522]{073470-103470}LwwwwwwZ`

It then executes the simulator's normal HALT. The original linked
executable is DXR2 purity=PURE, selected by its TSFS root manifest.
The original child's only `w` path is reached after its PIPE read
and both private mutable sentinels. Original exit status is
057 octal. The only parent `Z` path occurs after six consecutive
successful `dsys_wait(pid[i], &status, 0)` calls, each requiring the
same child PID, EXITED status kind and exact status 057, followed by
reverification of the parent's two private mutable sentinels.
Failure on any check calls `p14_fail` and cannot reach `Z`.

The native/source-linked physical v1627 auditor PASSes and rejects
both a forged success marker and a forged after-BRK process-word
counter. The original no-AUX build, source and simulator media are
preserved by testkit in unique private directories.

## Meaning and remaining work

This accepts the six-child **functional** PIPE/read/private-data,
normal EXIT, exact parent WAIT and parent BRK return on the physical
64K guest. It does not prove that PURE omitted prefixes were still
resident in the desired PACKED representation at every instant,
that source unmount/refusal recovery is complete, or that the
installed 1758-word PURE MRES has positive whole-system net/peak
RAM savings. Full displaced-prefix restoration, simultaneous PI6
faults and rollback cases remain separate open P1-E acceptance.

Keep default PURE OFF and strict 32K optimization on HOLD.
