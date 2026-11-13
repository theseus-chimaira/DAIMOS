# KCORE P0-A v1544: D6FS secondary DSK/DRM adapter isolation

Date: 2026-11-13. Feature acceptance without new resident logic.

The v1538 root-failure caller discriminates actual D6FS provider
and public mount ID; the direct physical adapter in
`system/kernel/storage/fs_backing.s` has no public mount ID and
must not HALT from its own Type-270/236 error. Its original root
accounting flag affects D6SET I/O counters only. `fs_backing.s`
and the fixed v1515 leaf comments have been corrected to describe
the accepted v1538/v1541 *critical service* policy rather than the
obsolete positive-permanent-media-loss prerequisite. These are
source comments only: **no new executable instruction or data**.

The v1544 testkit now links the actual D6FS reader, write-through,
full `fs_backing.s` direct adapter (including original MINIT
patch slots), root+mount classifier and three different terminal
HALT PCs. The PDP-6 executes **24** complete tests: DSK270 and
DRM236 direct root/secondary READ/WRITE failures and successes,
secondary three-member INTERLEAVE failures in both directions,
and invalid-block negative controls for both controller classes.
Actual controller kind, physical unit, RBN, transfer count, BCACHE
effects and root-only logical I/O counters are checked. Only the
very last controller result and BCACHE are test doubles.

Six source mutants are rejected, including missing root gate,
false secondary-to-root matching, DRM-to-DSK misrouting, wrong
physical member, false root accounting and wrong fatal reason.
v1513, v1515 and the independent direct-backing v1 regression
tests PASS; normal tape-root KCC fixed KCORE and D6FS MRES link
PASS. No KCORE code, RAM, layout, ABI or media format change.

This is complete source-linked native CPU evidence, **not** a
real full-guest secondary D6FS mount/detach test. The latter still
needs actual validated mount handoff: the existing syscall accepts
a 14-word transient handoff, but there is no current userland D6FS
scanner publishing it. It is incorrect to claim that detaching an
unused DSK/DRM is a failed secondary filesystem request. The
physical negative and shared ROOT/BACKSTORE tests stay OPEN.

Test details and reproduction command are in daimos-testkit
`tests/validation/daimos-secondary-d6fs-direct-error-v1544.md`.
