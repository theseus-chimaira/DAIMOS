# KCORE v1574: Type-136 reverse write completion count

Date: 2026-11-14. Correctness repair, not a new filesystem feature
or a general KCORE optimization. Tests live in `daimos-testkit`.

## Root cause

`system/kernel/drivers/tape_io.s` has separate Type-136 PI3 paths for
ordinary BLKI/BLKO and single-block reverse DECtape transfers. Reverse
transfers use one explicit DATAI/DATAO for each of 0200 36-bit words;
the reverse driver walks its DMA word address downward. After the 128th
word, `tape_dct_reverse_advance` reduced `storage_count` from 1 to 0,
but unconditionally reloaded it with 0200 before dispatching the
write-completion handler.

On a reverse WRITE the pending Type-136 PI3 requests therefore entered
`tape_dct_reverse_word` again, instead of the `tape_dct_select` dispatch
patched by `tape_dct_dtc_write_arm`. They sent additional memory DATAO
words while the Type-551 was still in its write state and damaged
physical DECtape blocks outside the requested run. The higher-level
v1554 RBN0 read before each subsequent DTFS write happened to change
the motion/transfer sequence and masked this driver error in v1572.
Operator-trust media policy does not eliminate this hardware I/O bug.

The correction adds just one `skipn dtc_request_write` before the
existing `movem 1,storage_count`: a reverse READ retains its established
0200 count reset, whereas a reverse WRITE leaves the decremented zero
until the two existing DCT write acknowledgments finish. Do not skip
the acknowledgments or stop the Type-136 channel immediately after the
final explicit DATAO: those attempts left the leading two physical words
unchanged. A direction-comparison-only change also failed to prevent
the original physical corruption. No new state or workaround was added.

## Executable evidence

`daimos-testkit/tests/validation/daimos-dtc-reverse-write-count-v1574.sh`
directly assembles/executes actual production `tape_io.s` and
`storage_router.s` on the PDP-6 with physically attached DTC0. Cases:

- Normal two-block write/read: full tape changed only RBN0200/0201.
- A repeated write with no intervening RBN0 read: only RBN0200/0201.
- A genuine reverse DATAO write, forward/reverse readback: only
  RBN0200/0201; exact 36-bit physical values checked.
- Original `daimos-dtc-reverse-online-v1.sh` and
  `daimos-dtc-stream-online-v1.sh` regressions remain passing.
- Two independently executed deliberately unsafe instruction mutants
  that restore unconditional `storage_count=0200` both fail the full
  physical tape oracle, changing unrelated blocks. Guest success alone
  is explicitly insufficient to accept a transfer.

The v1573 investigation also corrected a test-harness mistake: its
reverse-fixture-drain mode initially selected a forward-stream test,
not the reverse fixture. A real reverse-fixture drain with write count
left zero and the existing two DCT acknowledgments passed. That
experiment was a disposable source copy; v1574 is the minimal
production fix with a permanent executable oracle.

## Size, scope and follow-up

The fixed KCORE lower end is unchanged at octal `024364`. The Type-136
driver belongs to the optional tape MRES; its TEXT grows by one PDP-10
instruction, without DATA/BSS or mount slab allocation. No DTVOL2
on-media format, syscall, identity, MM or provider-lease changes.

Do not remove v1554's repeated RBN0 verification from production as a
side effect of this driver patch: a separate source-matched physical
guest, including real DATA/DIR, CLEAN/unmount, partial-write failure,
recovery and cold boot, must explicitly accept that simplification.
The prior v1572 working tree remains uncommitted and separate. P0-B
operator-trust decision and the P1-C/D/E workstreams are unchanged by
this Type-136 correction; VERSION 0.5 and the 32K HOLD remain.
