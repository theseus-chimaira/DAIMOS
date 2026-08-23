# DAIMOS Errata

## PDP-6 Type 630 DCS versus current upstream SIMH

Status: documented emulator/hardware semantic discrepancy, audited 2026-08-23.

DAIMOS follows current upstream SIMH by default for the Type 630 DCS transmit
operation.  This keeps the normal DAIMOS build interoperable with the simulator
used for development and testing without requiring a locally patched simulator.

The PDP-6 Handbook documents different Type 630 transmit semantics:

- `CONO DCSB,E` loads the send buffer used to select an idle transmitter.
- `DATAO DCSA,E` transmits through the transmitter selected by that send buffer.
- `CONI DCSB,E` reads the receiver/scanner counter.
- `DATAO DCSB,E` transmits through the receiver-counter-selected line, clears
  the receiver/scanner flags, and releases the receiver scanner.

Current upstream SIMH routes the send-buffer-selected transmit path through
`DATAO DCSB` instead.  A test using only line 0 can hide this discrepancy,
because the simulator scanner state initially also selects line 0.  DAIMOS
therefore tests DCS output on line 1 while keeping line 0 connected.

### DAIMOS build selection

The normal build uses current upstream SIMH behavior:

```sh
make kinit-test
```

This is equivalent to:

```sh
make kinit-test DCS_SIMH_COMPAT=1
```

For the PDP-6 Handbook transmit instruction without modifying source:

```sh
make kinit-test DCS_SIMH_COMPAT=0
```

The switch changes only the resident transmit IOT (`DATAO DCSB` versus
`DATAO DCSA`).  The public DCS service ABI, receive path, PI level, and resident
size are unchanged.

### Unapplied errata patches

This directory intentionally contains two source patches.  They are reference
errata and are **not applied** by DAIMOS or by the release `.shar`:

- `daimos-pdp6-dcs-handbook-default-v1.patch` changes DAIMOS so the handbook
  transmit instruction is the default build behavior.
- `sims-pdp6-dcs-handbook-semantics-v1.patch` changes the uploaded/current
  PDP-6 SIMH Type 630 implementation toward the handbook's DCSA/DCSB semantics.

The simulator patch is not required for the normal DAIMOS build.  It is kept as
an explicit record of the discrepancy rather than silently making DAIMOS depend
on a private simulator fork.

## PDP-6 SLV probe result across simulator variants

DAIMOS has no resident SLV driver.  MINIT only distinguishes whether device
`0020` responds to its temporary PIA probe:

- `SLV NO DRV` means the SLV device responds, but DAIMOS intentionally has no
  driver for it.
- `SLV NO DEV` means the device is absent, disabled, or not mapped by the
  simulator.

Some PDP-6 simulator trees map the SLV device while current upstream-compatible
configurations may leave device `0020` effectively unmapped even after `set
slave enabled`.  Both diagnostics are valid DAIMOS behavior.  Kernel tests
therefore require the `SLV ... NO` diagnostic family rather than assuming that
the simulator implements SLV.  The explicit disabled-SLV test still requires
`SLV NO DEV`.
