# K05 v1839: FULL restore source-free reply and authoritative MM ownership

Date: 2026-11-23. Base: integrated main after v1809/v1808.
Status: **scoped native PDP-6 PASS; K05 remains OPEN**.

## Reproduced failure

The v1654 bounded FULL restore correctly unpins its provisional
complete APR before unpinning/freeing the retained PACKED suffix.
It then publishes the new PROCESS APR, saved backing and RESIDENT mode
under PI exclusion. But its failure path unconditionally rolled back
the destination when `mm_free(PACKED)` returned nonzero.

In the original source-linked native PDP-6 transaction, an isolated
MM caller-boundary fault allowed the original `mm_free` to complete,
then changed only its successful return into `MM_ERR_BUSY`. The old
transaction destroyed the sole surviving complete APR and left the
process TRANSITION-held with no PROCESS or PACKED owner. The original
guest failed at octal `__test_exit=03003` before the source correction.

## Correction

On a reported failure from the source `mm_free`, inspect the original
MM extent descriptors while PI remains excluded. Count exact physical
PROCESS and PACKED owners for the process, enforcing unique bases,
word counts, types and zero pins. Reject a foreign type/owner at the
source or target base before interpreting a missing PACKED descriptor
as successful source deletion. No new permanent owner registry exists.

* Exactly one unpinned PROCESS target and no PACKED source: the free
  completed despite its return. Publish the existing complete APR,
  clear saved backing and TRANSITION, and return success.
* Exactly one unpinned PROCESS target plus the original exact unpinned
  PACKED source: the free genuinely refused. Use the existing checked
  temporary-target rollback path; do not publish the incomplete APR.
* Any other state (wrong owner, pinned, missing, duplicate or changed
  extent): retain the target without rollback, keep TRANSITION and
  return `MM_ERR_BUSY` for fail-closed quarantine.

Normal successful `mm_free` returns execute the original path without
a new scan. This code is in optional `proc_pure_ram_restore_v1391.c`,
assembled into PURE MRES only when `DAIMOS_PURE_MRES_ENABLED=1`.
The default-OFF fixed KCORE does not link this transaction.

## Native source-linked tests

New testkit directory:
`tests/validation/pure-full-free-reply-v1839/`.
The `test-pure-k05-source-free-native-v1839` target executes the real
KCC-built PDP-6 FULL, DROP, prefix and original PURE MM assembler
transactions with the existing v1384/v1395 MM and VFS fixture.
It includes the entire original v1395 baseline before the new cases.

Accepted native cases: physically committed source free with lost
return, real source-free refusal with exact rollback, foreign source
owner requiring unchanged destination retention; each checks the
mutable suffix, original backing, process storage mode, MM owner and
pins, and balanced PI. Two separately compiled production-source
mutations are executed and rejected at their intended native oracles:

* Old destructive post-free rollback: octal `03003`.
* Ignoring a foreign source owner: octal `05002`.

The complete current-source
`make test-pure-integration-native-v1839` matrix PASSes. This includes
v1677, v1678, v1679, v1680, v1681, v1684, v1674, v1804,
v1807 and v1808 native/regression gates; it is not a new whole-machine
physical pressure or alternate-root PASS.

The separate `test-pure-k05-mode3-compat-native-v1808` check also
PASSes the existing v1667 original-MM bitmap restore/service baseline
and rejects its unsafe retry-owner mutant. This is a shared-interface
compatibility regression only; mode3 is still disabled and unaccepted.

Two clean isolated KCC `system/kernel kcore` assembler builds also PASS:
the original default-OFF profile and opt-in
`DAIMOS_PURE_MRES_ENABLED=1 DAIMOS_SWAPLESS_ONLY_V1668=1` with
`DAIMOS_PBM_V1667=0`. These are source compilation gates, not booted
physical guests or completed linked kernel RAM measurements.

## RAM and remaining limits

The matched standalone KCC `-Pgnu99 -O -x=pdp6 -m=gas` FULL-restore
assembler listing changed from 814 to 961 lines; a filtered count
of instruction-shaped lines changed 469 to 578 (+109). This is only
a listing comparison, not a linked MRES word or run-time peak
measurement. No new process fields, resident data or fixed KCORE
source code were added. Measure exact linked PURE MRES cost in the
final current-source 48K/64K size matrix before production enablement.

K05 remains OPEN for genuine full-system PDP-6 PI6 interruption,
simultaneous physical owners, source/allocator/provider disappearance,
retry and irrecoverable pin/owner error cases, and continuous MM
high-water. K06-K09 and optional mode3 remain separately OPEN.

**Physical follow-up v1893 ACCEPTED (2026-11-23):** an actual
source-built 48K original PDP-6 run physically executed the real
`mm_free(PACKED)` successfully, deleted its sole RAW suffix, then
had only its AC1 return changed from `MM_OK` to `MM_ERR_BUSY=-5`
by private SIMH control. The original v1839 zero-pin owner
reconciliation retained its sole verified complete PROCESS successor,
published RESIDENT and cleared TRANSITION. All five native child
EXIT/WAIT and final HALT completed. Six negative physical trace
controls PASS; no production RAM/source growth. This closes only
the **committed physical source free with erroneous failure reply**
case, NOT genuinely refused free, foreign ownership or other
persistent MM ambiguity. Full trace and derived source-linked
return at `035311` are documented at
`design/kcore-k05-physical-source-free-reply-v1893.md`.
