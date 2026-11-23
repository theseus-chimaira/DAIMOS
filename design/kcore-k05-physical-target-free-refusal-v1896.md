# K05 v1896: original PDP-6 native PROCESS-target cleanup free refusal

Date: 2026-11-23. Scoped original 48K PDP-6 full-system physical
**PASS**. K05-K09 OPEN; RAW PURE/mode3 OFF by default. No permanent
code or RAM change in fixed KCORE or removable PURE MRES.

## Failure case and required ownership policy

The original v1839/v1855 FULL restore must preserve the authoritative
PACKED RAW suffix and temporary reconstructed PROCESS extent if
the latter cannot be safely freed during `fail_target` rollback.
`mm_free(target,MM_TYPE_PROCESS,slot)` may return `MM_ERR_BUSY`
because another physical pin exists. Treating this as successful
rollback would lose track of a still-owned full APR or allow an
unsafe duplicate allocation, even though the process has no
valid VM base. The existing original C logic returns -5 without
clearing TRANSITION, publishing an APR, freeing either extent or
assuming the MM operation completed.

Prior original machine acceptance: v1893 proved source deletion
committed despite a forged failure reply; v1894 proved real
source-free refusal with an external pin released before exact
two-zero-pin owner-set reconciliation and successful target cleanup;
v1895 held the external source pin indefinitely to prove source
owner quarantine plus already-resident user progress. v1896 instead
reaches the ORIGINAL target rollback `mm_free(PROCESS)` and forces
that original native operation to refuse, retaining a permanent
external target pin through two independent user parent heartbeats.

## Real original linked hardware entry and runtime

The source-built original 48K v1855 fixed KCORE and optional PURE
MRES are BYTE-IDENTICAL to the accepted v1890-v1895 images. The
original five-child RUN/PIPE/BRK/secondary-DTC3 PURE workload and
its writable-data checks remain intact; only the accepted testkit
v1892 parent-only user HB continuation is used instead of a WAIT
that cannot finish with an intentionally pinned quarantined child.
No production kernel, MM routine, media or child source changes.

The actual installed PURE base `032417` octal is proven using the
original running KCORE reclaim JRST. The compiled original FULL
DOBJ has two `mm_free` relocations `.text+000674` and
`.text+000755`, relative to original function `.text+000504`.
The linker-map PURE FULL address and real linked native PUSHJ
opcodes are verified. Original machine addresses (octal):

* bounded FULL loader `036202`;
* authoritative PACKED source free `035310`, return `035311`;
* provisional PROCESS-target cleanup free `035371`, return
  `035372`, original `fail_target` path, not the source free;
* original FULL service return `033067`;
* physical PACKED source metadata `024703`, target `024705`.

Original pinned FULL has 18 MM extents, slot3 RAW PACKED source
`111200..120000` pin1, separate complete PROCESS target
`120000..130000` pin1, and already-resident parent slot1
`130000..140000` PROCESS pin0. The original loader completes,
passes exact one-pin owner check and unpins both source/target.
The test-only physical controller then briefly pins the source,
allowing the ORIGINAL fixed-KCORE `mm_free(PACKED)` to genuinely
return `MM_ERR_BUSY=-5` with source pin1 and target pin0. Only
this initial external SOURCE pin is released. The original v1839
exact MM-owner reconciliation observes two zero-pin owners and
chooses the actual `fail_target` cleanup. At physical `035371`
it is about to execute `mm_free(PROCESS)` for the full target:
both source and target original MM descriptors are STILL pin0,
with MM count18 and unchanged selected root.

ONLY NOW the private test-only controller raises target pin0 to
pin1 by depositing its existing physical MM descriptor:

    024705: 000001000003 -> 000011000003

The original unchanged fixed-KCORE `mm_free(PROCESS)` executes
its original PI-atomic pin/type/owner check and returns **genuine
MM_ERR_BUSY=-5**, physical AC1 `777777777773` at real PC `035372`,
without deleting the target. The external target pin is NEVER
released. The original FULL returns typed `MM_ERR_BUSY=-5` at
original service PC `033067`, with original slot3 TRANSITION set,
VMbase0, PACKED source pin0 intact and PROCESS target pin1 intact.
Both extents remain owned by slot3, MM count18. No other source,
target or unrelated MM record is changed by original kernel code.
Original TSFS ROOT `070101000000` octal stays unchanged.

The original slot3 process meta `044723=046031600401` remains
TRANSITION; VM word `044724=010000000000` has physical VM base
zero. At two later **new** genuine user `HB` events, parent slot1
is the original current process 1, SRUN with valid resident APR
`130000..140000` pin0. Original real PCLK advances from `015401`
at HB1 to `015564` octal at HB2, with the authoritative full MM
extent table identical to the failed target-free result.

## Physical ownership observations (all octal)

| Runtime stage | MM count | source pin | target pin | slot3 VM |
|---|---:|---:|---:|---|
| Pinned FULL loader | 18 | 1 | 1 | 0 |
| Source-free call | 18 | 0 | 0 | 0 |
| Native source `mm_free` -5 | 18 | 1 | 0 | 0 |
| External SOURCE pin released | 18 | 0 | 0 | 0 |
| Original target cleanup-free call | 18 | 0 | 0 | 0 |
| Native target `mm_free` -5 | 18 | 0 | 1 | 0 |
| Original FULL returns -5 | 18 | 0 | 1 | 0 |
| Two independent user parent HBs | 18 | 0 | 1 | 0 |

## Evidence, audit and remaining gates

The independent read-only physical verifier checks BOTH original
native `mm_free` return values, source-linked DOBJ/MM call targets,
exact original pin/base/owner/type/extent lengths and all unrelated
MM descriptors at each stage, original TSFS ROOT, slot3
TRANSITION/VM0, parent slot1 resident SRUN/user dispatch and real
advancing PCLK across TWO newly emitted original user HBs.
**THIRTEEN separately executed unsafe physical transcript mutants**
are rejected at intended source-pin, target-pin, rollback,
process-publication, original return, clock/user/heartbeat checks.

Accepted original normalized PDP-6 physical SHA256:
`b408a84b27b39b351e56196356fb4abaac2b60c64175abe4bb74d38382ab7601`.
Unchanged original fixed KCORE DXR SHA256:
`33bf700218a73fefcb36ebb1f1d835c1817f3a291a642898ccb3cbd5d9039d8b`.
Unchanged optional PURE MRES DXR SHA256:
`a0fb25c51cf1c57f4bc70ca1dc83d8b9ea39b1ac3ddb3bba7b9ffcea17e5b3fd`.
Real physical evidence:
`$TMPDIR/daimos-k05-target-free-refusal-v1896-real/`.
Testkit sources, executable original physical read-only audit and
reproduction:
`tests/validation/pure-k05-target-free-refusal-v1896/`.
Stable reference:
`pdp10-doc/KERNEL/PURE-K05-TARGET-FREE-REFUSAL-V1896.MD`.

The original simulator exits after two independent parent HBs;
all-child EXIT/WAIT, guest HALT, target unpin and terminal
quarantined-owner cleanup are intentionally NOT claimed. This
closes the specific **genuine original temporary-target cleanup
`mm_free` refusal and fail-closed retained-owner quarantine**
with independent resident parent progress, NOT successful target
free with lost reply, foreign owner, ambiguous allocator/unpin
errors, terminal cleanup, arbitrary PI6 orders, 64K/KA10 or full
K05-K09 release. No production code/RAM or on-disk format change.

**Physical follow-up v1897 (2026-11-23):** The original same
`fail_target` target cleanup `mm_free(PROCESS)` was entered with
target pin0 and physically SUCCEEDED, deleting the whole target
while the authoritative PACKED source survived. The private
controller changed ONLY postcommit AC1 0->-5; the original FULL
conservatively returned BUSY but retained source/TRANSITION/VM0,
and the unchanged five-child guest subsequently completed real
typed EXIT/WAIT and HALT with no slot3 leaks. This covers the
opposite successful-deletion/lost-reply outcome to v1896's
genuine pinned target-free refusal. See
`design/kcore-k05-physical-target-free-lost-reply-v1897.md`.
