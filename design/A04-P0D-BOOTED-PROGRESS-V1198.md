# A04 P0-D physical-guest progress v1198 (2026-11-05)

This is additional booted integration evidence for A04/F02, **not**
completion of the entire `P0D_FULL=1` regression matrix.

## Frozen provenance

- DAIMOS `22df1a0c38120466f28287c3edcc0ffee08f72d2`.
- daimos-testkit `122c61ffa81310c3fd6c038f8f7e5db2b5114e09`.
- Installed `$PDP10_PREFIX=$HOME/cross` PDP-6 toolchain/simulator.
- Test commands, run individually from `daimos-testkit` with
  `DAIMOS_REPO=$HOME/git/DAIMOS`, `$PDP10_PREFIX/bin` in PATH and
  `TMPDIR=${TMPDIR:-$HOME/tmp}`:

```
sh tests/validation/daimos-tty-vm-uarea-v1124.sh
sh tests/validation/daimos-oom-pre-vm-retry-v997.sh
sh tests/validation/daimos-oom-multistage-v1103.sh
```

## Booted results

1. v1124 **PASS**: genuine TTY, process VM, and u-area MM_BUSY;
   retain original owners; unpin VM and u-area; release exactly once;
   final error count zero.
2. v997 **PASS**: physical pre-VM rejection, subsequent VM retry,
   post-VM u-area retry; original held foreign provider unchanged;
   one final EXIT/CHLD/WAIT, independent bystander 37, error zero.
3. v1103 **PASS**: real PIPE MM_BUSY, HUP refusal, VM and u-area
   release retries in a single process cleanup; held foreign lease
   unchanged; original process and WAIT owner preserved; bystander
   36; error zero.

Evidence archive: `$TMPDIR/daimos-a04-evidence-v1198.tar.gz`.
SHA256: `94387cc3ba4de549d58cf5aa23237d2180a6c9562572f8adc0bf6d463e32442f`.
Archive contains the three original logs and a source/testkit
revision and individual-log checksum manifest. The first v1125
booted guest and all 18 component stages were already accepted as
partial evidence in `design/A04-P0D-FULL-PARTIAL-V1197.md`.

## Unfinished

The remainder of the full-guest matrix (beginning with the v1117
WAIT/PID churn guest) has not been rerun in this v1198 batch. A04
and F02 remain OPEN until **all** booted cases pass on a frozen
candidate and final ownership, diagnostic and size evidence is
reviewed. No production code, permanent KCORE/MRES RAM, ABI or OOM
default has been changed. Mandatory OOM checklist remains 57/64.
