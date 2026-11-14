# KCORE P1-C v1576: physical MEMFS snapshot + LOGSTORE recovery

Date: 2026-11-14. A verified mixed-storage P1-C slice, NOT full
P1-C closure. P0-A and operator-trust P0-B closed. VERSION 0.5,
strict 32K optimization HOLD.

## Executed full guest

An unmodified production KCORE and selected MRES were cold-booted
twice on a 128K-word PDP-6 using the same backing DSK and 8 MiB
AUXSTORE DRM236 images but new RAM each time. The real user test
made a persistent MEMFS `/TEMP` file and a valid XIP executable,
checked file contents and RUN/WAIT, and appended a uniquely marked
LSREC1 through the actual root LOGCTL syscall. It first read
LOGSTORE STATUS (sequence, capacity, next slot), wrote via APPEND,
read the exact mapped physical record through READ_BLOCK, checked
full 36-bit magic/sequence/payload and the one-step cursor advance.

Normal HALT committed MEMFS after the LOGSTORE write. On the new
cold boot, the guest recovered the exact MEMFS XIP and three-word
data file, found exactly one physical LOGSTORE record from boot 1
by scanning the existing ring through the native syscall, then
appended and read another record with a distinct code. Both full
guest probes exited STATUS:0; the original AUXSTORE region descriptor
was preserved after each boot and matched after boot 2. The shared
physical drum has independently mapped BACKSTORE, LOGSTORE,
CACHESTORE and D6FS regions. No source changes in DAIMOS production.

Versionized testkit:
`tests/validation/daimos-p1c-logstore-snapshot-v1576.sh/.md`.

## Related real PDP-6 regressions rerun

`daimos-snapshot-swap-reserve-v1536.sh`: actual source-linked KINIT
v1/v2 committed snapshot bitmap reservation plus native BACKSTORE
allocate/free ownership; four KINIT and three native unsafe mutants
rejected. `daimos-swap-in-real-transaction-v1316.sh`: genuine
source-linked MM pin, swap-in restore and retry/rollback; three
mutants rejected, but its disk backing reads are test-controlled.
Native LOGSTORE append/direct read tests also PASS. No new KCORE
TEXT/DATA/BSS, MRES, MM or permanent state; the two 128-word LOGCTL
scratch arrays are in the test-only guest executable.

## Not yet accepted (P1-C remains OPEN)

The physical full guest did not have a process actively swapping in
or out while the MEMFS snapshot was committed, and did not inject an
in-flight DSK270/DRM236/DTC controller loss, LOGSTORE partial write,
or mapped DEVICE versus VOLUME failure. Sequential/coexisting real
provider operations are not proof of simultaneous rollback/ownership.
Do not mark the entire P1-C item complete until these occur in a
single instrumented real guest with explicit swapped-sector, MM pin,
PI ownership, BCACHE and snapshot commit evidence.
