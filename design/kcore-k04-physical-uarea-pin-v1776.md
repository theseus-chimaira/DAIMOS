# K04 v1776: PDP-6 physical MM u-area pin refusal and recovery

Date: 2026-11-21. Result: **PASS** for a single physically pinned
post-VM u-area; multiple independent owners and the other K04
acceptance gates remain OPEN.

## Reproducible isolation and mechanism

Build a disposable instrumented 256K PDP-6 DSK image using the
production v1773 image as a starting point. The test build in this
run was `$TMPDIR/daimos-k04-v1776-realpin`; it was a separate copy
of `$TMPDIR/daimos-k04-v1773-boot`. Production `proc_pdp6.s`, all
production binaries, and the externally managed `daimos-testkit`
were not edited.

Modify **only** the generated test build's
`system/boot/pdp6/kcore-asm/proc_pdp6.s`, then assemble/link with
the normal DAIMOS native DAS/dlink image build. The test fixture
augments `proc_uarea_release` for **slot 5 only**, after proving
that a nonzero stable u-area exists and saving the descriptor:

- On the first u-area release call, invoke the real `mm_pin` on
  the exact descriptor's current physical u-area base. Save that
  base and set a test-only attempt counter to one.
- Proceed to the **unaltered** `mm_free(base, 3, 01000+slot)` call.
  The allocator's real pin check returns `MM_ERR_BUSY` (-5) and
  preserves both the physical MM extent and process owner.
- On each later retry, compare the descriptor's current base with
  the saved base (fail closed on mismatch), then advance the
  test-only attempt count. Leave the real MM pin in place for
  attempts 1 through 9.
- On attempt 10 (octal `012`), invoke the real `mm_unpin` and
  mark the fixture complete. Proceed to the ordinary `mm_free`;
  do not inject any replacement return code or alter any other
  process's cleanup.

The fixture adds three words of scratch-only BSS:
`k04_pin_base_v1776`, `k04_pin_calls_v1776` and
`k04_pin_done_v1776`. All are exported in the **test** map for
read-only inspection. The linked test `__kcore_low_end` is octal
`025167`, versus the uninstrumented v1773 `025126`. This size
difference exists only in the disposable test image and is
**not** a proposed production KCORE change.

Native DAS assembly, full incremental KCORE/KINIT relink and
complete D6FS image generation passed. Before boot, read-only
`d6fsck -n 4` showed CLEAN, sequence 1. The test image used the
ordinary 60Hz realtime PCLK and DCS0 port 21186. The SIMH
monitor was left available only to examine memory: no guest
memory was changed through the simulator.

## Actual live guest observations

Both CTY and DCS0 logged in as ROOT and entered DSH. CTY ran
foreground `TRUE`; the waiting shell did not return immediately.
DCS0 ran `PS` and observed the physical refused owner:

```
PID PPID S WORDS TD RETRY PVM DETAIL ERR COMM
5   2    2 262143 2 512   0   6      0   USER
```

The `DETAIL=6` field is stage 2 plus `(pin_count << 2)`;
`SYS_TEARDOWN_MM_PINS(6)` is exactly **one physical MM pin**.
The 512-tick retry interval is the expected capped backoff.
The u-area and process identity stayed owned, so slot 5 was
not reused while the pin remained present. The DCS shell and
its PID 6 `PS` child could still run.

CTY entered the SIMH monitor using Ctrl-E **for read-only
inspection**. The exported test-fixture words were:

```
sim> examine 25034-25036
25034: 000000054424   ; still-owned u-area base
25035: 000000000010   ; eight release attempts (octal)
25036: 000000000000   ; not yet released
```

After `continue`, another DCS `PS` still showed PID 5 in stage
2 with `RETRY=512`, `DETAIL=6`; the parent did not resume early.
After the tenth cleanup attempt, the real `mm_unpin` succeeded,
the normal `mm_free` committed, and the waiting CTY shell
returned to its `#` prompt. DCS `PS` then showed:

```
5 3 2 3072 0 0 0 0 0 PS
```

This is PID 5 reused by an unrelated PS process with all
teardown diagnostics clear. A final read-only fixture snapshot:

```
sim> examine 25034-25036
25034: 000000000000   ; pin base cleared
25035: 000000000012   ; ten attempts (octal)
25036: 777777777777   ; unpin completed
```

DCS issued guest `HALT`; SIMH reported HALT at octal PC
`022527`. The simulator then exited normally. Post-shutdown
read-only `d6fsck -n 4` reported superblock 742, sequence 3,
CLEAN, 179196 blocks and 360 FCBs.

## Acceptance boundary

This validates the actual MM extent pin accounting and
post-VM deferred retry/release for **one** process on a running
PDP-6 guest. It is stronger than the synthetic `MM_ERR_BUSY`
return-code experiment v1774. The observed 512-tick cap does
not measure every intermediate 020/040/0100/... interval.
No clock-wrap, two-owner shared epoch, SIDL discard, FILE
provider refusal, terminal HUP or dying-parent WAIT test was
performed here. Those K04 requirements remain OPEN. All tests
were executed using disposable `$TMPDIR` fixtures, not through
the externally managed testkit.
