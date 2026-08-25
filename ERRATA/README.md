# DAIMOS Errata

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
