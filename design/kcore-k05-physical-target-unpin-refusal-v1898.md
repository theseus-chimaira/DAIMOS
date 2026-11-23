# K05 v1898: physically refused original target unpin and cleanup retry

Date: 2026-11-23. Scoped **original 48K PDP-6 physical PASS**.
K05-K09 remain OPEN; RAW PURE/mode3 OFF by default. Zero fixed
KCORE or optional PURE MRES production source/RAM changes.

## Contract and failure injection boundary

The original v1855 PURE FULL transaction proves the exact one-pin
source/target pair while PI is disabled. Its first fallible commit
operation is `mm_unpin(target)`. If that fails, the original
`fail_target` path retries target unpin only if `target_pinned`
still indicates the caller's own pin, and **must not free an
uncertain temporary target** if cleanup unpin also refuses.
The RAW PACKED source must remain authoritative, and the child
must not be published as resident.

This test is deliberately a *fault injection into an otherwise
PI-atomic transaction*. The original allocator cannot normally
lose the target pin between the exact-owner check and the native
unpin via asynchronous scheduler interleaving: PI is disabled.
The private simulator deposits an unexpected zero pin while the
CPU is stopped at the actual original call, to execute the
genuine fixed-KCORE `mm_unpin` invalid-pin branch twice. The
test proves defensive behavior under a corrupted/unexpected MM
pin state, not a naturally occurring interrupt race.

## Source-linked actual hardware

Original v1855 48K fixed KCORE and opt-in PURE MRES binaries are
byte-identical to earlier physical K05 milestones. The original
five-child RUN/BRK/PIPE root and secondary compressed DTC3
workload runs with the accepted v1892 test-only already-resident
parent `HB` continuation. The original fixed KCORE reclaim JRST
gives the real physical installed PURE base `032417` octal.

The compiled original `proc_pure_full_v1611.dobj` FULL symbol
`.text+000504` has **four** native `mm_unpin` relocations:
`.text+000655`, `.text+000662`, `.text+000746`, `.text+000766`.
All four are verified as original linked `PUSHJ 17,mm_unpin`
opcodes, not guessed debugger return locations. The target
calls used in this experiment are (octal):

* original commit target unpin `035271`, native return `035272`;
* original `fail_target` cleanup target unpin `035362`, native
  return `035363`;
* original FULL service return `033067`.

At the original first target call both exact original slot3
owners remain pin1, verified by the running physical MM table:
PACKED RAW `111200..120000` and complete PROCESS target
`120000..130000`, with MM descriptor count 18. Only private
test-only SIMH changes target meta word `024705` from
`000011000003` (PROCESS slot3 pin1) to `000001000003`
(PROCESS slot3 pin0). No other descriptor, original code,
memory image, TSFS root or executable backing is modified.

The ORIGINAL fixed-KCORE `mm_unpin` executes against physical
pin0, returning its OWN `MM_ERR_INVAL=-4` in AC1
`777777777774` at PC `035272`. Original C retains
`target_pinned=1`, restores its saved PI state, and calls
the actual `fail_target` retry at `035362`. Its target is STILL
physical pin0, so original native MM unpin returns the SAME
genuine `-4` at `035363`. The original C returns `MM_ERR_BUSY=-5`
at FULL service PC `033067`; it does NOT call target free or
source unpin/free, and does not publish an invalid RESIDENT APR.

The full 18-entry original MM table is identical from the first
unpin refusal through the second refusal, original FULL service
return and two later genuine user `HB` stops: PACKED source pin1,
complete target pin0, all unrelated MM owner/type/base/wordcount/
pin records conserved, selected TSFS root `070101000000` octal.
Original slot3 process meta `044723=046031600401` has TRANSITION
set, VM word `044724=010000000000` has physical APR zero.

Unrelated parent slot1 is independently resident PROCESS at
`130000..140000` pin0 and SRUN; at two genuine new user `HB`
emissions, `proc_current_slot=1` and its valid APR is intact.
The original PCLK advances **`016325` to `016425` octal** across
the two heartbeats. The controller deliberately exits after
the two HBs instead of falsely claiming child EXIT/WAIT/HALT.

## Original MM proof

| Actual original stage | MM records | PACKED source | PROCESS target |
|---|---:|---|---|
| Pinned FULL / precommit native unpin | 18 | pin1 | pin1 |
| Private zero-target-pin deposit | 18 | pin1 | pin0 |
| First original `mm_unpin` returns -4 | 18 | pin1 | pin0 |
| Original cleanup retry returns -4 | 18 | pin1 | pin0 |
| Original FULL returns -5 | 18 | pin1 | pin0 |
| New parent HB1 and HB2 | 18 | pin1 | pin0 |

Independent read-only testkit verifier checks actual two linked
native call/return PC pairs, both physical AC1 -4 results, exact
owner geometry, original process table pointer/TRANSITION/VM0,
full unchanged unrelated MM/TSFS ROOT, resident parent current
slot/SRUN/APR and progressing original PCLK. **Thirteen** separate
unsafe original physical transcript mutants are executed and
rejected at the intended owner, pin, return, process or heartbeat
check. Only testkit was changed, not production kernel source.

Accepted original normalized physical PDP-6 trace SHA256:
`c7d5350098f69474e1fd2398c0822d3abf11bd21ad754c5a69d973a7ef3e215e`.
Original fixed KCORE DXR SHA256:
`33bf700218a73fefcb36ebb1f1d835c1817f3a291a642898ccb3cbd5d9039d8b`.
Original opt-in PURE MRES DXR SHA256:
`a0fb25c51cf1c57f4bc70ca1dc83d8b9ea39b1ac3ddb3bba7b9ffcea17e5b3fd`.
Original private evidence:
`$TMPDIR/daimos-k05-target-unpin-refusal-v1898-real/`.
Testkit runnable source/negative audit:
`tests/validation/pure-k05-target-unpin-refusal-v1898/`.
Permanent kernel reference:
`pdp10-doc/KERNEL/PURE-K05-TARGET-UNPIN-REFUSAL-V1898.MD`.

## Remaining K05 gates

v1898 closes only the original **double native target-unpin -4
under a test-only unexpectedly-zero target descriptor**, safe
quarantine and two independent original parent HBs. It does not
prove native source unpin failures or false unpin success replies,
foreign MM owner states, permanent original owner cleanup,
normal full child EXIT/WAIT, alternate PI6 orders, 64K/KA10 or
overall K05-K09 release. With the existing exact-one-pin PI
atomic owner guard, the injected zero-pin transition is NOT a
legitimate native asynchronous race. Zero production RAM growth.
