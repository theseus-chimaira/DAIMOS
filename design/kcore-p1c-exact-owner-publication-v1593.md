# P1-C v1593: exact resident publication of second physical swap owner

Date: 2026-11-15. Full 128K PDP-6 two-cold-boot acceptance on real
four-member Type-236 DR0..DR3 D6FSR2 root VOLUME. Unmodified KCORE
swap scheduler, actual PI6/DR_DONE, MM, mapped BACKSTORE, MEMFS v2
snapshot, XIP and LOGSTORE. All added witnesses are test-only in
an isolated source copy, not installed resident RAM or production ABI.

The v1592 physical test proved two independently woken SLEEP/DISK
owners, slots 6 and 7, with one selected post-DMA checked-release
refusal but counted successful RESIDENT publications only in the
aggregate. v1593 instruments the actual original successful
`proc_storage_publish_v30` return after clearing TRANSITION,
and records the unmodified AC10 slot identity, process descriptor,
aligned VM, original swap record and stable u-area backing word.

Both real boots independently show slot **7** published RESIDENT
exactly once, actual base `0220000`, original nonzero image record,
stable u-area, TRANSITION clear, and old disk backing pointer zero.
Slot 6 also had one success during another transaction even though
the selected later failed operation was associated with that slot.
This is valid process reuse, not an MM leak or a false publication:
the separate refused transaction retains DISK ownership, restores
the exact bitmap, cleans MM and emits TERM through the existing
v1588 handling. The original physical swap count remains five
READs, 1,120 Type-236 PI2, four successful publications per boot.

Both normal HALTs, second-boot restored MEMFS/XIP/LOGSTORE and
valid physical snapshot DR3 `017777`, 3,648-word pool and 280
other nonzero backing sectors. The source-linked oracle rejects
10 new exact-publication/VM/owner/FSINFO witnesses and preserves
v1592's 16 and v1584's 13 negative tests: **39** rejected
unsafe cases in total. Full evidence and reproduction in
`daimos-testkit/tests/validation/
daimos-p1c-owner-publish-physical-v1593.md`.

The same real guest also tests **P1-D** on a *selected four-member
DRM236 D6FS root*: original FSINFO capacity **6**, root provider
6 at ID 1, MEMFS `/TEMP` provider 4 at ID 2, four empty DSK slots
and invalid higher slots. Both guests print the exact stage only
after validating every query. This extends hardware-root coverage
beyond the v1591 two-member TSFS test.

P1-C exact per-slot counterpart proof is accepted, but later-stage
physical DEVICE/VOLUME/DTFS loss, dirty/write-through failure and
loaded-simulator timing still require tests. P1-D provider/failed
inventory semantics also remain open. Owned BACKSTORE device I/O
errors remain fail-stop, not silently retryable. No production
KCORE/ABI/media change, VERSION 0.5, strict 32K HOLD.
