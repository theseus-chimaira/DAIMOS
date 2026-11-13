# KCORE v1533: real high-slot FSINFO mount ownership

Date: 2026-11-13. P1-D physical feature acceptance; no optimization.

Following v1532's boot-published seven-slot FSINFO acceptance,
the full 256K PDP-6 TSFS-root guest now occupies more than
four actual mounts on real Type-551 media. The guest uses the
already accepted v1532 production KCORE and DTFS/TSFS MRES,
not mock filesystem callbacks.

Four separate native DTVOL1-stamped DTC3-DTC6 tapes are
generated inside the test's own private media directory.
The testkit validates each RBN0 stamp (marker, two nonzero
36-bit identity halves, complement) and requires four distinct
72-bit volume identities. Its private SIMH boot script enables
and attaches DTC4-DTC6 before GO. TSFS root DTC0-DTC2 remains
unmodified and contains four separate mount directories.

The real PID1 mounts all four native DTFS volumes. FSINFO
verifies public mount IDs 2, 3, 4 and **5**, their provider
and path; slot 0 remains TSFS root provider 7. The guest
scans all seven published slots and checks empty slots
5 and 6, then unmounts *middle* DTFS mount ID 3.
After this hole, the real high mount ID 5 still reports
the original DTC6 volume pathname. All other secondary
mounts are retired and all six nonroot slots return 0;
the count and TSFS root remain intact.

Full physical guest reports
`FSINFOHI V1533 MOUNTED5`, `SCANNED7`,
`HOLE_STABLE`, `RETIRED`, `PASS`, with
the host runner exiting zero. The v1532 physical capacity
test is independently rerun with the new harness and PASSes.

No production KCORE instructions/data, DTFS/TSFS MRES,
on-media structure or permanent RAM changes in v1533.
The changed code is isolated to the daimos-testkit
guest, disposable TSFS root manifest, extra physically
attached tapes and admission assertions.

This proves a *live* mount above the previous four-slot
limit, but not an actual 26-volume hardware configuration.
That number is a representational/planner ceiling.
P1-D cross-inventory maximum remains unproved. P0-A
authoritative critical loss, P0-B durable discarded-media
admission, P1-C mixed faults and P1-E PURE/MM/APR are
separate. VERSION 0.5, strict 32K/optimization HOLD.
