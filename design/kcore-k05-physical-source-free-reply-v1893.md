# K05 v1893: original 48K PDP-6 committed source-free reply ambiguity

Date: 2026-11-23. Status: **scoped complete full-system physical
PASS**, including FIVE original child EXIT/WAIT and final HALT.
K05-K09 OPEN. RAW PURE/mode3 OFF by default.

## Failure mode and authoritative MM transaction

v1839 previously fixed a real unsafe FULL restore rollback: the
authoritative PACKED RAW suffix could have been **successfully
freed** while `mm_free(PACKED)` returned an error. The original
transaction blindly rolled back its now-sole complete PROCESS
successor on that failed reply, losing the child's last valid
mutable state. v1839's source-linked KCC native test reproduced
the old bug at `__test_exit=03003` octal, and fixed it by checking
the exact remaining physical MM owner set under PI exclusion.

The original FULL must distinguish all three states: (1) sole
complete unpinned PROCESS owner, meaning source free succeeded
despite an error reply and the target MUST be published; (2) exact
PACKED plus PROCESS owners, both unpinned, meaning source free
genuinely refused and the target may be safely rolled back; and
(3) any missing/foreign/pinned/duplicate/inexact owner, meaning
permanent TRANSITION quarantine without speculative release.
The v1855 shared zero-pin MM scanner implements this exact test
for a failed original source-free response.

v1890-v1892 physically validated a different precommit error:
an extra target pin blocked the source release and quarantined
the process. v1893 first physically executes the **normal
successful MM source deletion** and only THEN spoofs its result.
There is no corresponding production code change in this milestone.

## Source-linked original hardware experiment

The exact current 48K v1855 SOURCE-BUILT five-child original guest
is reused, including plain TSFS-root executable, four compressed
secondary DTC3 executables, genuine PIPE/BRK pressure, original
mutable-data sentinels and typed EXIT/WAIT assertions. No user
INIT heartbeat loop or changed child workload is included.
DTC3 remains attached and CLEAN throughout. Private SIMH media
copies isolate all original drive images.

The original installed PURE base `032417` octal comes from the
**actual physical KCORE reclaim-JRST probe**, as before. The
testkit reads the original compiled `proc_pure_full_v1611.dobj`
symbol table and relocation table, proving that
`proc_pure_ram_restore_v1391` begins at original DOBJ `.text+000504`
and the first of exactly two `mm_free` relocations is at
`.text+000674`; the other belongs to rollback at `.text+000755`.
It cross-checks the linked PURE image call instruction against
`PUSHJ 17,mm_free` with original KCORE symbol `002320`, and
requires the next original compiled instruction to be
`MOVE 010,1` (its return-status consumer). Hence the exact
source-release call is physically at `035310`, with its original
return breakpoint at `035311`. The process FULL service return is
physically at `033067`; these addresses were independently
rederived from the linked image, not guessed from the old v1845
routine offsets.

At FULL entry, the actual original MM table holds the authoritative
slot3 PACKED suffix `111200..120000` pin1 and a separate complete
PROCESS destination `120000..130000` pin1. The original loader
finishes, copies the retained RAW suffix over the new destination,
passes the v1855 exact-owner commit check, unpins both original
MM extents, then executes the **actual original `mm_free(PACKED)`**.
SIMH breaks immediately on its return, before its C caller executes
the result consumer, and captures AC1 and every original MM record.
At this point AC1 is **zero**, the PACKED source is physically GONE,
and the sole original slot3 owner is PROCESS `120000..130000` pin0.
The MM entry count is 17 versus 18 at pinned FULL. Every other
original MM descriptor and selected TSFS root is unchanged.

The only test-only fault is then the physical simulator's
`deposit 1 777777777773`, immediately re-examined to prove it
changed exactly the returned AC1 from `MM_OK=0` to
`MM_ERR_BUSY=-5`. It does not recreate or delete any extent,
patch an instruction, modify the original source media, or change
the kernel. Original v1839/v1855 reconciliation observes exactly
one unpinned PROCESS successor and correctly publishes it,
clearing the saved backing and TRANSITION and returning `MM_OK=0`
at original service return `033067`.

The real slot3 process descriptor at that point is (octal):

* meta `046031400401`: TRANSITION cleared;
* VM `010000120000`: restored physical APR base `120000`;
* scheduler `200024000000`, its original correctly published state.

The source remains deleted, the target remains sole unpinned owner,
the complete original MM table has 17 entries and the TSFS root
is unchanged. The actual original five-child user console emits
its first `w` before the FULL fault breakpoint, then the other four
`w` characters after recovery, followed by parent `Z`, and
real `HALT instruction, PC: 000000`. This original fixture checks
each child's typed exit code and writable-data sentinels in the
user program. At the final `Z` snapshot, MM extent count is 8 and
there is no residual slot3 PROCESS or PACKED owner: the recovered
child's terminal cleanup completed, not just its publication.

## Physical audit and original images

The actual complete source-linked physical simulator returned
success. Its independent read-only audit verifies source/target
MM geometry before actual original deletion, physical deletion
at the `mm_free` return, exact AC1 failure injection, original
reconciliation publication and unchanged unrelated MM owners/root,
five genuine user `w` markers across debugger snapshots, final
parent `Z` and HALT. SIX independently executed forged original
physical transcripts are rejected at their exact checks:
failed actual original free, source successor still pinned, foreign
successor, missing injected -5, false rejection at recovered FULL,
and restored process still TRANSITION. All mutations are test-only
trace negatives, and the original trace is restored after each.

The first v1893 full-system physical run passed the transaction
and produced true user/WAIT/HALT evidence, but the first test-only
verifier wrongly required the literal uninterrupted text
`wwwwwZ`. The original source-linked monitor snapshot necessarily
interrupted the five user `w` outputs (1 before + 4 after).
Corrected only the verifier to count five original user `w`
outputs between its original physical checkpoints; all other
original MM/return/ROOT/scheduler/terminal checks were retained,
and the saved original real simulator transcript passed unchanged.

Accepted ORIGINAL normalized PDP-6 physical transcript SHA256:
`8d852bc8f40944035bb27a6bc773b8b4b11913b1683ae32ec2e39d41e968f0cf`.
The image hashes are byte-identical to the earlier accepted
physical v1855-v1892 binaries:

* Fixed KCORE DXR SHA256:
  `33bf700218a73fefcb36ebb1f1d835c1817f3a291a642898ccb3cbd5d9039d8b`.
* Optional PURE MRES DXR SHA256:
  `a0fb25c51cf1c57f4bc70ca1dc83d8b9ea39b1ac3ddb3bba7b9ffcea17e5b3fd`.

The complete original MM/console evidence is preserved at
`$TMPDIR/daimos-k05-source-free-reply-v1893-real/`; the original
48K source image is at `$TMPDIR/pure-brk-pressure-v1614.qnobNT/`.
Testkit only: `tests/validation/pure-k05-source-free-reply-physical-v1893/`,
targets `test-pure-k05-source-free-reply-physical-v1893` and
`test-pure-k05-source-free-reply-audit-v1893`.

## Scope and remaining K05 work

This physically accepts the **successful deletion with a failed
reply** branch of the original v1839 source-free MM reconciliation.
It does NOT establish the independent case of a genuinely refused
free with both unpinned extents still present, any foreign owner
at the free boundary, a permanently wrong physical pin/unpin result,
allocator unknown success, terminal cleanup with an unresolved
ambiguous owner, every arbitrary PI6 ordering, 64K/KA10 or K05-K09
production release. K05-K09 remain OPEN. RAW PURE/mode3 remain OFF
by default. This milestone adds ZERO production KCORE/MRES code,
RAM, registers, file formats or processes.
