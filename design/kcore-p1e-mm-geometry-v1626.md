# P1-E v1626: exact 48K packed-RAM MM geometry and 64K contrast

Date: 2026-11-16. Base: committed/pushed v1625 original sixth-RUN
VM-creation error, with the same preserved 48K source and simulator
machine-memory snapshot. No KCORE/loader/MM/MRES source change.

## Authenticated physical 48K allocation state

The original six-child PDP-6 guest stopped at actual RUN cleanup
PC `017725`, AC1=-2 (`EXEC_LOAD_NOMEM`), before the failed slot
was discarded. A testkit v1626 audit decodes all 022 octal MM
extent descriptors and the genuine MM arena from original SIMH
memory, matching the original `kcore.map`. It also reads the exact
booted PURE DXR image and verifies the original loader's physical
`vm_space_create` request before rounding to the PDP-6 APR quantum.

All values below are octal 36-bit words or physical addresses:

* MM arena: `042562..140000`, single arena.
* Five distinct PACKED owners 6,5,4,3,2, each with a retained
  `007000`-word mutable suffix, unpinned: total `043000` words.
* Original VM parent: physical base `126000`, size `010000`,
  owner 1, unpinned.
* Physical free gaps: `046605..057000`, `066000..067000`,
  `076000..077000`, `106000..107000`, `116000..117000`,
  `136000..140000`.
* Sum of free gaps `016173`; largest currently legal
  `02000`-aligned gap only `007000`.
* The original DXR PURE has TEXT/image `001043`, BSS `004001`.
  The actual loader adds user origin/stack `002020`: required
  logical VM size `007064`, rounded up to an APR allocation of
  `010000` words. None of the current aligned free gaps fits it.

The five omitted PURE prefixes are not a single coalesced free
extent: their holes are interleaved with still-MM-owned suffixes.
Thus the sixth process cannot be placed **in this captured layout**
despite total free words exceeding its requested image. This is
an exact fragmentation/capacity diagnosis, not compiler or RUN
validation corruption. It does not prove that all possible future
compaction/reclaim policies must fail. No allocator workaround or
speculative PACKED relocation was introduced.

The v1626 source/DXR/physical MM auditor PASSes and rejects a
forged foreign suffix owner and forged overlapping extent. The
preexisting v1625 source-linked typed RUN/AC1 rejection also PASSes.

## Independent physical 64K experiment

A distinct complete GCC 64K lowmem/TSFS/no-AUX/PURE guest was
reconstructed from the original six-child PIPE/WAIT source, with
the v1624 typed RUN/SLEEP refusal and BRK target 017000 octal.
The actual simulator booted and accepted six children. Parent BRK
`010000 -> 017000` returned successfully. MEMINFO was
`[B073470/042522]` before and `[A103470/042522]` after, exactly
the expected extra `010000` APR words. The original strict test
refused this as `!R`: no additional RAW drop was demonstrated
*during* BRK. Six children subsequently printed valid `w` markers
after parent failure cleanup. Original test exits nonzero, correctly.

This establishes a feasible larger-memory execution profile and
parent BRK success, but not full parent WAIT/EXIT or positive
whole-system net RAM. The separate v1627 lifecycle-only guest
must preserve all actual WAIT status assertions while distinguishing
their acceptance from the specific timing of memory reclamation.

## Remaining

P1-E complete six-child lifecycle, immutable backing concurrency,
relocated/full restore, MM owner/failure rollback and physical
peak/net RAM remain OPEN. Default PURE OFF; 32K optimization HOLD.
