# P1-E v1637: six genuine PACKED children restored after seventh RUN

Date: 2026-11-16. Base: v1636 production BCACHE 0201-word slab
allocation corrected from MM_ALLOC_HIGH to LOW, restoring the
physical 48K parent APR at 130000..140000 and six child admissions.
This increment adds source-backed physical test evidence only;
no further production KCORE, MM, BCACHE, MRES or ABI changes.

## Physical pressure and exact child lifecycle

An original C89 v1614 testkit child/PIPE/WAIT fixture was changed
only to request six children, attempt a seventh RUN, require that
additional RUN to fail with -1, perform a no-growth BRK and check
bounded process-word accounting without assuming a new BRK-time
drop. Original child PIPE read and both writable sentinels, EXIT
status 057, parent PID/EXITED/status-057 WAITs and terminal parent
data checks all remain source authenticated. The parent remains
alive after the seventh VM allocation refusal and reaps six
existing children, rather than making a failed-run cleanup look
like successful resident restoration.

The complete original PDP-6 48K TSFS/no-AUX/GCC PURE console is:

`aaaaaax[B013470/042501]<010000+010000>[A013470/042501]{013470-013470}LwwwwwwZ`

Normal simulator HALT follows. Six initial `a` markers prove
admitted/sleeping children; `x` is the expected refused seventh RUN.
The final `Z` follows six `w` child messages and the parent's six
successful exact WAIT checks. `L` is a lifecycle marker, not a
claim of additional BRK-time or whole-system net RAM savings.

## Original MM B/A/Z snapshots from the same compiled guest

Read-only SIMH replay used the same guest image, root Type-551
tapes and linked source map, capturing complete original MM
extents at B/A and final Z. Both B and A contain SIX distinct
unpinned PACKED_PROCESS owners with exact 007000-word mutable
suffixes. Their physical intervals, all octal PDP-6 words:

| Process slot | PACKED physical extent |
|---|---|
| 7 | 051000..060000 |
| 6 | 061000..070000 |
| 5 | 071000..100000 |
| 4 | 101000..110000 |
| 3 | 111000..120000 |
| 2 | 121000..130000 |

The long-lived parent remains at `130000..140000`. No PACKED
owner moves between B and A, no extents overlap, and pins are
zero. The clean BCACHE slab has been safely reclaimed during
pressure and may reappear at low memory after normal filesystem
activity. At Z all six original child PROCESS/PACKED owners have
been released, without leaked pins or child extents; the parent
remains resident. Original `p14_data` sentinels and exact child
WAIT statuses are checked before the final Z.

The v1637 source/link/physical auditor PASSes, and both forged
foreign-owner and forged-final-WAIT witnesses are rejected.

## Meaning and remaining acceptance

Together v1636 and v1637 prove that the low-BCACHE placement
restores SIX complete initial child APR admissions under 48K
and continues to support SIX genuine PACKED->user execution->
EXIT/WAIT restorations after additional memory pressure.
The earlier v1631 PURE-on admission regression with a high cache
slab is removed without shrinking the PURE MRES, moving running
processes, changing MM owner records or bypassing honest resource
errors. The cache's original owner/size and physical invalidation
and MM-reclaim tests also remain intact.

Still OPEN: independent physical forced FULL relocated v1391
restore (versus already observed fast v1384); immutable executable
source lifetime/unmount and failure-atomic rollback; concurrent
PI6 restore and OOM paths; 32K/64K general cache placement
regressions; matched whole-system transient peak and net RAM
including the installed 1758-word PURE MRES. The current 48K
source-matched physical successes do not close these P1-E gates.
PURE stays OFF by default and strict 32K optimization HOLD.
