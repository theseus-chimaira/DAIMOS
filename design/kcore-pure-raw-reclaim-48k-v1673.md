# PDP-6 PURE RAW reclamation: physical 48K stable-checkpoint acceptance v1673

Date: 2026-11-19. DAIMOS source: `ae84bb37` on the *unmerged*
`kcore-pure-reclaim-pressure-v1673-20261118` branch. Testkit target:
`test-pure-runtime-reclaim-v1673`, testkit branch
`test-pure-reclaim-pressure-v1673-20261118`.

## Contract exercised

The testkit's tiny PID1 on disposable three-member TSFS media runs
nine actual simultaneous sleeping PURE children, attempts a tenth RUN,
then WAITs for all original children. The physical PDP-6 simulator has
48K words; DSK/DRM auxiliary swap is detached. The general and strict
swapless KINIT variants use the same PURE MRES; bitmap mode3 stays OFF.
The host uses three CTY input holds and original physical MM/process
symbols, not user-PC breakpoints or kernel test hooks.

## Verified original physical evidence

Two independent retained source-matched general/strict report pairs
passed the original `compare-v1673.py` source-linked oracle. All four
JSON reports are identical, SHA256:

`d1065373bc172be41ecbb859261f056ae5c5361336c607631ceb5eefdf2118a6`

| Checkpoint | MM free | Largest free gap | Resident APRs | PACKED RAW owners |
| --- | ---: | ---: | ---: | ---: |
| Before RUN | 27441 | 27441 | 1 | 0 |
| Pressure | 2794 | 1641 | 1 | 9 |
| After EXIT/WAIT | 27441 | 27441 | 1 | 0 |

Each of the nine original PACKED process images is precisely 128
physical words smaller than its original logical APR image. Total
reclaimed PURE text: 1152 words. A tenth real RUN fails with a spare
process slot and 2794 *aggregate* free words, because the largest free
extent of 1641 words cannot contain a new 2048-word APR image. All
original MM extents and pins return to the pre-RUN baseline after WAIT.

The general console was independently remeasured using original
source-built kernel and fixture maps, producing byte-identical JSON.
The original strict console was likewise remeasured during the negative
acceptance runner. The same `run-negative-v1673.sh` executable oracle
rejected five physical transcript corruptions: PACKED storage mode,
MM owner type, immutable backing, CTY stage release, and native
SYS_MEMINFO. It also rejected a separately forged result JSON.

## What this proves, and what it does not

**PASS v1673:** Physical 48K RAW PURE owner reclamation under real
process APR fragmentation, bounded stable checkpoints, exact cleanup
and equality between general and strict modes, plus six executed
negative checks. This is retained physical source-matched evidence
replayed independently; it is **not** a newly completed all-in-one
physical simulator invocation.

No kernel or MRES implementation was changed for this gate. This does
**not** prove a continuous scheduler/high-water trace, injected PI6
failure handling, compressed bitmap mode3 integration, ROOT=auto/DSK/DRM,
KA10, physical 32K or production merge. Those remain separate gates.
The next RAM gate is continuous post-user high-water measurement under
real reclaim pressure, followed by non-TSFS root regression.

Only the testkit owns the scripts and fixtures. No new KCORE data or
resident program words were added for acceptance.
