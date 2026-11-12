# KCORE v1517: simulated Type-551 selection fault versus root media loss

Date: 2026-11-12. Source and historical controller-documentation sweep,
followed by an executed real-PDP-6 SIMH status regression. This is a
completed **classification investigation**, not automatic root HALT
integration. There is no KCORE code, state, ABI, or RAM change.

## Controller and emulator source

The DEC Type-551 control reports an illegal/select error through
status B bit `000004` (octal). DAIMOS currently tests combined DTC
error bits `0000034` (select/illegal, parity, timing) in
`system/kernel/drivers/tape_io.s`, with bounded search retries,
completion failures, and independent watchdog timeouts. It does not
expose an authoritative media-change event in its I/O return ABI.

In the Open SIMH PDP-6 model, `sims/PDP10/pdp6_dtc.c::dtc_devio`
sets `DTB_ILL` on a `CONO` selecting a `UNIT_ATT`-clear unit. That
is **simulator behavior** and is not an independently verified signal
on every original Type-551/555 installation. The TU55 maintenance
manual describes REMOTE/OFF/LOCAL switching and stopped transport
motion. The TU56 manual specifically notes that the external analog
SELECT ECHO detector used in later systems is **not** used when
connected to 550/551/552 controllers. Consequently, interpreting
SIMH's not-attached error as a real-hardware physical-reel-removal
notification would be an unjustified assumption.

Historical reference URLs (sources, not copied into the repo):

- `https://bitsavers.trailing-edge.com/pdf/dec/dectape/tu55/H-TU55_MaintMan_Nov66.pdf`
- `https://bitsavers.org/pdf/dec/dectape/tu56/DEC-00-HRTC-D_TU56_Mantenance_Manual_Jan73.pdf`
- Full Type-551 manual downloaded for later detailed inspection:
  `https://decdoc.itsx.net/bitsavers/pdf/dec/dectape/H-551_551dectapeCtlr.pdf`

## Executed simulator evidence

Testkit `daimos-dtc-select-status-v1517.sh` programs the PDP-6 CPU
with actual `MOVEI`, `CONO 0210`, `CONI 0214`, and `MOVEM` instructions.
The same legal SEARCH/SELECT of unit 0 is run before attachment,
while attached, and after `DETACH DTC0`. Then the same command is
executed against unrelated, unattached physical unit 7. The
controller status read is stored in ordinary PDP-6 memory and
verified from the SIMH console, with exactly four genuine HALTs.

| Selected transport | CONI 0214 status (octal) |
| --- | ---: |
| DTC0, never attached | `000004` |
| DTC0, attached | `001000` |
| DTC0, then detached | `000004` |
| DTC7, unrelated and unattached | `000004` |

The scripted test PASSes under the local real SIMH PDP-6 (`3536d98`
testkit). The repeated `000004` value is a controller select error,
not proof by itself that the *selected TSFS root* has vanished.
Even after identifying the physical unit, KCORE must use the exact
v1516 active TSFS root-member mask, not assume any selected DTC
unit is critical. A transport remaining logically selected but
not producing mark/data transitions can instead only appear as an
expired search or transfer watchdog, also not a physical-loss proof.

## Decision / future acceptance gate

`DTB_ILL`, a generic negative Type-136 read result, and a watchdog
timeout are **not authoritative permanent media loss by themselves**.
Do not call `kcore_critical_device_lost_v1515` on any of them alone.
The v1515 fail-stop leaf and v1516 ownership query remain the two
validated building blocks, but the DTC hardware producer of a
trustworthy offline/media-change event is not established.

P0 still requires reliable real Type-551 loss semantics, a routed
physical root-member-loss event and a real physical fatal-PC test.
An unrelated non-root DTFS tape must remain nonfatal and retain its
mount until checked identity and BCACHE recovery. Without a
trustworthy hardware indication, do not invent a device-loss
state or a repeated-error threshold; report this detection limit
explicitly instead of claiming removal detection is complete.
