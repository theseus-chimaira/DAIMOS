# K16 v1991: physical FULL rollback and ordinary restore retry

Validation only, parent ff8bd0c0. Testkit4748e68. Actual source-built
64K PDP6 GCC strict TSFS directory image with idle recovery enabled.
KCORE byte-identical to accepted v1987; PURE MRES identical to
accepted v1990 GCC package,5917 resident words. Original INIT,
COMP and POST DXRs byte-identical, TSFS3 members/55 files/316 extents.

## Matched pressure

Both v1987 and v1991 admit all12 children (fixture ceiling,
not maximum): four bitmap251, one RAW3584, seven resident4096.
33 valid MM descriptors, free1789->1332 exactly -457 MRES words,
largest APR-aligned gap512 unchanged. New bitmap bases octal
062400,063600,065000,066200. Genuine stopped mode3 terminal
slots4/5/6; all10 typed TERM waits. Slot3 actual PIPE wake/FULL.

## Physical known-pin rollback

Actual slot3 atomic finish after bounded backing load and verified
decode; source062400/251, provisional target160000/4096, each pin1.
Three scoped test-only instruction replacements at stopped original
MM leaf entries: finish, initial target unpin, first deferred target
unpin. Replace entry with JRST fixed kret_neg1, observe actual live
stack return PC and AC1=-1, immediately restore original instruction.
No deposited pin/descriptor, process, clock, scheduler, backing or
payload state. This is controlled refusal, not hardware failure.

Original synchronous rollback records PAIR; actual fixed armer,
then idle retry deadlines ticks21375/21407, delay16->32->64.
Original bytes and entire MM table, original backing/credential,
saved PC, both pins and heap escrow unchanged during held phases.
Second actual recovery retires exact target, unpins bitmap, restores
BRK010000,,010000, clears escrow and TRANSITION, no READY.
Original pending clock cleared before later ordinary FULL.
Actual second atomic finish then marked-to-unmarked EXEC, same
slot/u-area, changed backing, new2048-word APR, no reservation.
All12 exact EXIT waits, final MM and storage map, normal HALT PC0,
root tapes unchanged.

Independent verifier validates full fault segment and reuses original
v1987 healthy lifecycle oracle with actual final finish. Eleven
corrupted trace controls reject: wrong opcode, unrestored instruction,
wrong result, length/pin/heap/payload change, backoff reset, stale
READY, stale pending, missing HALT. Historical v1987/v1988 physical
audits and v1987 eleven controls re-PASS. No production changes,
no new compiler/assembler defect observed.

Evidence relative TMPDIR:
pure-directory-recovery-build-v1991.WDZiGC/build,
pure-directory-recovery-physical-v1991.OxbcV9/physical.clean,
audit-v1991.log. Replay about404s.

## Remaining

Physical known-pin PAIR gate accepted. FREE/SOURCE and ambiguous
ownership remain native-only fault coverage. Dirty DROP known-owner
RAW rebase and pin retirement, irrecoverable source-loss/corruption
policy, RT latency/stack, real DSH/48K and KA10 full optional boot
acceptance remain open. No default activation.
