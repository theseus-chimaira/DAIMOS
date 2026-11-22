# v1614: correct the DLINK PURE TEXT address proof

Date: 2026-11-16
Base: committed PDP-6 KCORE/P1-E v1613.
Scope: host and native DLINK direct-TEXT-write validation,
plus a calibrated physical 48K TSFS test workload.

## Compiler/linker bug: absolute EA compared with relative length

The host linker `tools/host/dlink.c` examined assembled
instructions to reject definite direct writes to immutable
TEXT. Its `definite_text_write` compared an 18-bit *absolute*
effective address with `text_words`, which is a *length*,
not an ending physical address. For a user program linked
at octal `020` this gave two incorrect answers:

* `POP 17,10`, a direct write to accumulator AC10,
  was mistaken for a write to immutable TEXT when the
  program had more than ten octal TEXT words. Actual
  pressure-guest instruction octal `000257` triggered
  `dlink --pure contradicted by definite TEXT write`.
* The last `020` octal words of legitimate relocated
  TEXT could evade the test, since the test compared
  address directly to the length rather than to the
  absolute linked TEXT interval.

The corrected host predicate requires all of:

    !indirect && index == 0
    opcode_writes_ea(op)
    ea >= 020
    ea >= image_base
    ea - image_base < text_words

The subtraction is safe because it happens only after
checking the lower bound. AC0..AC17 are registers even
when the selected link base is zero. The native DAIMOS
toolchain `userland/toolchain/dlink.c` already used
an absolute TEXT base but also misclassified AC0..17
on a base-zero image. It now uses the same guarded
subtraction. Neither path introduces extra resident
KCORE/MM/process state.

The check is *conservative*, not a proof that arbitrary
TEXT is immutable. Indexed/indirect writes, block moves
and computed stores can evade any one-instruction direct
write analysis. It only detects a definite direct
TEXT write that cannot be accepted when `--pure` is
requested.

## Accepted source-linked tests

`daimos-testkit/tests/host/dlink-pure-ac-range-v1614.sh`
builds the real host linker from source under strict
C89 and assembles genuine PDP-6 objects. PASS:

* direct `POP 17,10` and `MOVEM 1,10` can link with
  `--pure -b 020` as legal AC-register writes;
* direct `MOVEM 1,020` is rejected as a TEXT write;
* relocated `MOVEM 1,target` is rejected when
  `target` resides at octal `0402` and link base
  is `0400`;
* base-zero output still accepts AC0..17 writes.

`tests/validation/dlink-pure-native-range-v1614/run-v1614.sh`
executes the **original native target** predicate as
PDP-6 machine code, not a reimplementation. Its
private mutation removing the accumulator exclusion
is rejected by executed PDP-6 assertions. The
existing host raw-XIP `dlink-xip-v218.sh` tests
still pass.

The fixed host linker was installed under
`$PDP10_PREFIX/bin/dlink` and its versioned alias
`dlink-v1614`; the previous executable was preserved
as `dlink-pre-v1614-20261116`. The source remains
authoritative for every later rebuild. Both host
source and native target source are version-controlled.

## Why earlier physical pressure tests could not prove RAW

The v1613 exploratory DISK-derived pressure workload
and the first v1614 sleeping-owner BRK workload linked
their user executable without `--pure`. Inspecting
the *actual* guest DXR returned
`purity=UNKNOWN` from `dxrcheck -d`. These trials
were never eligible to omit an immutable PURE prefix;
therefore their reported memory pressure failures
cannot demonstrate failure of a genuine P1-E DROP.

After this compiler/linker fix, the same v1614 guest
links with `--pure` and `dxrcheck -d` confirms
`purity=PURE`, with the original DXR2 TX2 text header.
An actual 48K PDP-6 TSFS guest, without AUX media,
reached the source-selected `PURE LOADED` service.
SIMH breakpoints hit the original resident
`proc_swap_reclaim` entry twice and the relocated
v1381 `proc_pure_ram_drop` and physical
`mm_pure_rebase_pinned` entries four times each.
This proves execution of the RAW code path, but the
first large BRK target still returned `!G`. It did
not prove successful allocation, PACKED owner
publication, subsequent slot-0 restore, immutable
backing lifetime or positive net reclaimed RAM.

The working physical guest script must insist on
`purity=PURE` **before** creating boot media.
No change to production KCORE, P1-E MRES,
scheduler or MM geometry is justified by the
historical oversized allocation failures.

## Production release status

The linker correctness defect is fixed and covered
with host/target regressions. Full physical
reclaim/restore and precise MM/peak-memory accounting
remain separate acceptance gates; do not enable
`DAIMOS_PURE_MRES_ENABLED` by default. There is
no new codec, KA10/KI10 backend or speculative
runtime abstraction in this increment.
