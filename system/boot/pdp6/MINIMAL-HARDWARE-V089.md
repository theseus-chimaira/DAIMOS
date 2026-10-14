# Minimal PDP-6 hardware acceptance configurations v089

The normal full configuration remains unchanged. For truly minimal 32K
systems, `BOOT_PROFILE=lowmem MINIMAL_HW_V089=1` omits the optional DCS,
GE, TTY, MTC, and (for tape root) DSK MINIT/MRES packages instead of merely
disabling their SIMH devices. The native CTY and PCLK clock remain required.
Stage-1 uses PTR briefly even when the actual boot device is DSK or DTC;
PTR is an explicitly transient bootstrap reader, not part of the running
root device set. The hardware filter in `daimos-testkit` disables all
other controllers and does not attach their media.

The following three profiles have reached `INIT V1` in PDP-6 SIMH with
`PACKED_RUNTIME_V088=1` and GCC PDP-6 kernel compilation:

| Profile | Hardware and media | KINIT end | Verdict |
| --- | --- | --- | --- |
| 32K D6FS | CTY, PCLK, PTR bootstrap, DCT + DSK0 | `075162` | PASS |
| 32K TSFS | CTY, PCLK, PTR bootstrap, DCT + DTC0 boot and DTC1 root | `075535` | PASS |
| 256K full | All simulator-supported DAIMOS controllers and attached disk, drum, DECtape, magnetic tape and console media | varies | PASS |

The bootstrap stack reserve remains `02000` octal words in both 32K tests,
requiring `__kinit_image_end <= 076000`. No stack reserve reduction or
resident decoder size optimization was used. The single-disk D6FS root
uses `D6FS_ROOT_MEMBERS=1`; normal four-disk images retain the default
`D6FS_ROOT_MEMBERS=4`. TSFS uses one boot DECtape plus one root DECtape.

These are real boot-to-INIT tests, **not yet** a test of live scheduler
compression, packed MM ownership, and fault-free restoration. Disk-root
INIT reports the expected missing `/TEMP` MEMFS mount in this minimal
configuration; boot itself succeeds. A fully attached 256K boot exercises
initialization of all supported device families.

Testkit entry point: `tests/validation/packed-boot-matrix-v089/run-v089.sh`.
