# K05 v1899: original native source-unpin -4 and checked target disposal

Date: 2026-11-23. Scoped original 48K PDP-6 full-system physical
**PASS**, including five original child typed EXIT/WAIT and real
parent HALT. K05-K09 OPEN; RAW PURE/mode3 OFF by default.
No production KCORE/PURE/MRES source, RAM or format changes.

## Purpose and precise failure model

The original FULL restore commits a completely reconstructed
PROCESS target while the authoritative PACKED RAW source still
exists. Under the original v1855 PI-atomic one-pin owner guard,
the first native `mm_unpin(target)` completes successfully.
Then the separate original source `mm_unpin(source)` can report
a non-success reply. Original C must mark its source pin status
uncertain, avoid issuing another speculative unpin, release
ONLY the physically complete unpinned temporary target if native
MM accepts it, retain the PACKED source and leave the child
TRANSITION/VMbase0 until a subsequent safe restore attempt.

To exercise an ACTUAL original source native `mm_unpin` refusal,
the test-only simulator changes the original source descriptor
pin1 to pin0 while stopped on the real original source-unpin
instruction. The unchanged fixed-KCORE `mm_unpin` then genuinely
returns `MM_ERR_INVAL=-4`. The normal kernel cannot lose the pin
there from an ordinary asynchronous PI6 interrupt: the original
exact-one-pin guard and commit unpins are PI-disabled. This
physically proves defensive handling of an injected unexpected
MM state, **not** a naturally occurring MM interleaving.

## Original source-linked PDP-6 execution

The original source-built v1855 48K fixed KCORE and optional
PURE MRES are byte-identical to v1890-v1898, with the original
unchanged five-child RUN/BRK/PIPE workload, TSFS ROOT child,
four compressed secondary DTC3-backed children, writable-data
sentinels, typed EXIT/WAIT and parent Z/HALT. Physical original
KCORE reclaim JRST locates installed PURE base `032417` octal.

The compiled original FULL DOBJ symbol `.text+000504`, all four
`mm_unpin` relocations `.text+000655`, `.text+000662`,
`.text+000746`, `.text+000766`, and both `mm_free` relocations
`.text+000674` and `.text+000755` are verified against linked
original `PUSHJ 17,mm_unpin/mm_free` instructions. Actual runtime
PCs (all octal):

* first target-unpin `035271` (normal original success);
* **source-unpin `035276`, return `035277`**;
* `fail_target` original target `mm_free(PROCESS)` `035371`,
  return `035372`;
* original FULL service return `033067`.

At pinned FULL, slot3 PACKED source `111200..120000` and full
temporary PROCESS target `120000..130000` both have pin1,
original MM count18. The complete loader and target unpin run
normally, leaving physical source pin1 and target pin0 at the
ACTUAL source-unpin invocation. The test-only SIMH controller
changes ONLY physical source MM metadata word `024703`:

    000014000003 -> 000004000003
    PACKED pin1       PACKED pin0

The ORIGINAL fixed-KCORE native `mm_unpin(source)` genuinely
returns AC1 `777777777774` (`MM_ERR_INVAL=-4`) at `035277`,
with BOTH authoritative source and temporary PROCESS target
physically retained and pin0; MM count18. The original C sets
`source_uncertain=1` and safely enters `fail_target`. It NEVER
repeats an unverified source unpin. The exact unpinned PROCESS
target is physically freed by the ORIGINAL unchanged native MM
at `035371`; its actual return at `035372` is AC1=0. MM count
changes **18->17**, with sole slot3 owner PACKED source RAW
`111200..120000` pin0, and no provisional target.

At FULL service `033067`, original AC1 remains **-4**, slot3
TRANSITION set and physical APR base zero. Physical process
meta `044723=046031600401`, VM `044724=010000000000`. The
selected TSFS ROOT `070101000000` octal and all unrelated MM
owners/pins/geometry remain identical through original source
native -4, target rollback and failed FULL service return.

The unchanged original five-child guest retries restoration,
completes all original typed EXIT/WAIT, PIPE and mutable sentinel
checks, prints parent `Z`, and executes actual hardware
`HALT instruction, PC: 000000`. Final original MM count8,
slot3 PACKED and PROCESS owner allocations both gone. This is
stronger than a mere resident-parent heartbeat because all five
children and ordinary terminal cleanup finish.

## Physical checkpoints

| Original stage | MM records | PACKED source | PROCESS target |
|---|---:|---|---|
| Pinned FULL | 18 | pin1 | pin1 |
| Original source-unpin call | 18 | pin1 | pin0 |
| Genuine original source `mm_unpin` -4 | 18 | pin0 | pin0 |
| Original target cleanup-free call | 18 | pin0 | pin0 |
| Original target free committed | 17 | pin0 | deleted |
| Original FULL -4 / TRANSITION VM0 | 17 | pin0 | deleted |
| All five child EXIT/WAIT and Z/HALT | 8 | gone | gone |

The independent physical audit verifies original linked MM call
sites and native AC1, complete exact owner/pin/length tables,
conserved RAW source, successful original target deletion, slot3
TRANSITION/VM0, unchanged TSFS root/unrelated owners and all
original typed WAIT/HALT. **Thirteen separately executed unsafe
physical transcript negatives** are rejected at intended original
pin, native return, target free, MM owner, publication, process
table, root or HALT checks.

Accepted original normalized physical transcript SHA256:
`32b178432dce4404a405b15cb7a812718115205ad4a119f376256142abc68844`.
Unchanged fixed KCORE DXR SHA256:
`33bf700218a73fefcb36ebb1f1d835c1817f3a291a642898ccb3cbd5d9039d8b`.
Unchanged opt-in PURE MRES SHA256:
`a0fb25c51cf1c57f4bc70ca1dc83d8b9ea39b1ac3ddb3bba7b9ffcea17e5b3fd`.
Original physical evidence:
`$TMPDIR/daimos-k05-source-unpin-refusal-v1899-real/`.
Testkit:
`tests/validation/pure-k05-source-unpin-refusal-v1899/`.
Permanent reference:
`pdp10-doc/KERNEL/PURE-K05-SOURCE-UNPIN-REFUSAL-V1899.MD`.

The exact native unexpected source pin0 and subsequent original
cleanup/retry case is now accepted, NOT source unpin success with
lost failure reply, persistently pinned/foreign source, foreign
owner, terminal cleanup of unresolved quarantined extents,
additional PI6 interleavings, KA10/64K or K05-K09 release.
RAW PURE/mode3 remain OFF. ZERO additional KCORE/PURE words.

**Physical/native terminal follow-up v1900 (2026-11-23):** An
original native terminal/VM guest showed that a retained exact
PACKED RAW plus complete PROCESS target cannot be disposed by
the old single-owner terminal: octal assertion `001001` before
the correction. The corrected optional PURE MRES terminal now
checks both exact owners under PI exclusion and releases the
scratch target before RAW, using existing FINALIZE+PREVM retry
on uncertainty. Real PDP-6 MM/idle-finalizer native and updated
48K original source-free refusal/parent heartbeat compatibility
PASS, but physical foreign EXIT of the quarantined owner remains
OPEN. See `design/kcore-k05-terminal-dual-owner-v1900.md`.
