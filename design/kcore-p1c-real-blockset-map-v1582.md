# P1-C v1582: real BLOCKSET volume map and BACKSTORE fatal routing

Date: 2026-11-15. Native source-linked PDP-6 integration increment,
not a full physical PDP-6 guest or final P1-C closure. VERSION 0.5,
strict 32K/optimization HOLD.

The accepted v1582 test links the unmodified real compact runtime
`blockset_dispatch.s` (including `blockset_map_block`, its 12-word
root descriptor and INTERLEAVE mapping), the real fixed KCORE
`fs_mres.s` backing gateway and the production v1515 critical-media
HALT dispatch. The native PDP-6 fixture installs the actual runtime
mapper at `blockset_runtime_service_call`, as production MINIT does.
Only hardware completion leaves and the three-member descriptor
contents are supplied by the test, not a substitute software mapper.

The descriptor has three physical units 1, 2 and 3 with physical
bases octal 0100, 0200 and 0300, with 0200 root data sectors and
0100 tail sectors per unit. The first six BACKSTORE sector reads/
writes must interleave exactly to (1,0300), (2,0400), (3,0500),
(1,0301), (2,0401), (3,0501). The controlled physical leaf asserts
the unit, physical block, READ/WRITE direction, DRM-versus-DSK route
and exact 0200-word buffer stride for every sector. Successful reads
return distinct markers through the real gateway to the real
buffers. All 13 native baseline scenarios PASS: DSK and DRM multi-
sector reads/writes; DSK second and DRM third transfer failures;
enabled and disabled-but-owned terminal failures; disabled/unowned
normal negative returns; missing mapped service active/inactive;
tail bounds overflow/end checks and zero-length I/O. Critical cases
halt at the linker-resolved BACKSTORE reason, not the ROOT reason;
one fatal mapped DRM test halted at octal 001342 with exactly three
physical accesses and unchanged octal 0300 owned BACKSTORE blocks.

Eight executed unsafe source mutations all FAIL the native oracle:
wrong mapper block, wrong member, omitted data offset, wrong physical
DRM route, missing fatal, bypassed missing-mapper failure, or false
disabling of the owned-block gate, or a corrupt 0100-word instead of
0200-word transfer-buffer stride. The controller fixture now checks
every source/destination buffer address. All tests live in daimos-testkit:
`tests/validation/daimos-p1c-volume-realmap-v1582.sh`, companion
`.s` fixture and `.md` report.

**Precise scope:** v1582 is actual source/assembler semantics with
simulated last-mile controller success/failure, not a physical Type-
236/Type-270 PI completion or real multi-member full-guest volume.
It extends v1546 by replacing the latter's stub mapper with the
source-authentic BLOCKSET mapper and checking each physical sector.
P1-C remains OPEN for a real multi-member VOLUME physical guest,
actual mapped PI controller failure/return, independently scheduled
swap-in during synchronous snapshot I/O, and recoverable owner/MM/
BCACHE rollback. P1-D alternate physical inventories and P1-E PURE
remain OPEN. There is no production KCORE/MRES code, fixed RAM,
process-table, media layout or ABI change; the fixed KCORE low end
remains `024364` octal.
