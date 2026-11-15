# P1-C v1603: final combined BACKSTORE, swap, provider and BCACHE acceptance

Date: 2026-11-15. **P1-C feature acceptance CLOSED** on the real
original PDP-6 kernel, with the scope and fatal/nonfatal ownership
distinctions below. DAIMOS VERSION 0.5, KCORE fixed low end
`024364` octal. No optimization or permanent code/RAM change.

## Authoritative production contract

The existing BACKSTORE allocation bitmap and process swap records
have separate owners. No other filesystem is entitled to release a
disk-backed process's owned blocks. The original PI6 timer can wake
independently sleeping DISK processes; the original slot-0 scheduler
performs their separate swap READ transactions and publishes
RESIDENT only after checked MM/bitmap/BCACHE/record success. A
refused post-DMA checked release must roll back its own transaction,
preserving the other process's state, original bitmap, BCACHE tags,
and allocated backing. An actual selected fatal BACKSTORE I/O error
requires reason-2 HALT without postfault publication or cleanup.

The D6FS/DTFS shared `bcache_pdp6.s` is **clean-only write-through**.
There is no dirty writeback queue, dirty-list or flush daemon to
test. The meaningful failure is authoritative WRITE rejection before
a fresh clean tag can be published. D6FS invalidates its shared
workspace tag before original `fs_backing_write`, and inserts a
clean tag only after successful backing completion. Failed nonroot
I/O must not be classified as selected-root failure or evict an
unrelated private clean block. Media identity/revalidation rules
for Type-551 DTFS remain separately mandatory.

## Accepted physical and source-linked matrix

| Acceptance slice | Real evidence and boundary |
|---|---|
| v1599 fatal mapped VOLUME BACKSTORE | Full 128K DR0..DR3 root, two independently PI6-woken disk slots 6/7, one selected mapped swap READ for slot6, real Type-236 DR_DONE PI2, original reason-2 HALT `004011`; 23 corrupt controls |
| v1600 fatal direct DEVICE BACKSTORE | Full 128K DSK0..DSK3 D6FS root, independent DRM0 AUXSTORE direct `400000005770`, logical `000431` at physical DR0:`006421`, two genuine PI6 slot 6/7 wakeups; one selected swap READ slot6 and original reason-2 HALT `004011`; 27 corrupt controls |
| v1593 owner publication and v1588 rollback | Actual separately timer-woken slot7 RESIDENT publication, slot6 rollback on controlled post-DMA refusal; exact MM pins, swap record, bitmap, BCACHE and transition ownership |
| v1596 independent failed DTFS provider | Physically detach high DTC6 mount ID5 in a two-member TSFS-root 256K guest, independent mounted DTFS identities intact, actual unrelated writable MEMFS file modification and readback, original DTC6 return/revalidation; 8 corrupt controls |
| v1602 D6FS WRITE failure/success | Original native PDP-6 D6FS, BCACHE, FS_BACKING DSK/DRM direct adapter and root classifier, four executed cases and 5 unsafe source mutants rejected. No forged dirty cache or new persistent RAM |
| v1590 physical clean D6FS cache + LOGSTORE fault | Fresh two original 128K boots on Type-236 root, real uncommitted LOGSTORE physical WRITE and selected error, same-slot committed retry, clean 128-word D6FS block unchanged/HIT both times, subsequent actual PI6 swap, 4 READs/896 PI2 each boot |
| v1592 loaded repeatability | Fresh two 128K original PDP-6 boots, mapped VOLUME with actual MEMFS snapshot, slots 6/7 each one PI6 wake; 5 physical swap READ attempts, 1120 Type-236 PI2, exactly one original post-DMA rollback each boot. Separate concurrent host compile/load observed |
| **v1603 final simultaneous-owner/provider composite** | **Two full original 128K cold boots**, both with two real PI6 wakeups (slots 6/7), five actual swap READ attempts and 1120 PI2, one checked rollback, actual torn LOGSTORE WRITE + committed physical retry, clean populated D6FS cache with 0/0 differences across 128 words and HIT/HIT on both events. **44** independently rejected corrupt source-linked/raw-media observations |

The v1603 combined raw VOLUME oracle verified physical DR3:`017777`
octal snapshot sector, pool 3648 blocks, 283 other nonzero backing
sectors, original process allocation 936 blocks (decimal), unchanged
across the controlled rollback, no bitmap difference, and genuine
MM allocation/release counts 20 to 19 per boot. Cache private slab
`0377577` octal, actual 28 nonzero data words in every original
snapshot; all 128 words are identical after the first failed
physical LOGSTORE write and after the committed retry.

The same guest first writes the persistent MEMFS snapshot and
successfully restores it after a separate PDP-6 cold boot.
Both real boot transcripts complete normal guest HALT at
`022011` octal. The source-linked oracle requires actual
post-HALT KCORE addresses, not console-only success markers.

The **44** deliberate v1603 negative cases comprise 15 new
cache/LOGSTORE source-linked faults, 16 inherited independent
timer-owner/rollback errors and 13 inherited mapped media/swap
PI2 errors. The independent current-source native v1544 DSK/DRM
root/nonroot adapter matrix also passes 24 real source-linked
cases and six mutants. v1490 original BCACHE/DTFS security-gated
cache invalidation and MM-refused release, and v1538 original
root HALT PC/nonroot return acceptance, also pass.

## Timing and physical error claims

The v1592 and v1590 current-source two-boot regressions ran while
other full-guest builds/native tests consumed the same host; the
v1592 second-boot original simulator and the v1590 build were
observed concurrently at `2026-11-15T20:49:58+01:00`. Both
subsequently passed their exact physical/source-linked verifiers.
v1603 independently passed twice in one cold-boot/restart test.
These observations establish repeated loaded physical progress,
not a benchmark or proof of arbitrary load schedules.

The v1599 and v1600 actual swap error is a test-selected failing
original Type-236 completion after one genuine physical request,
not spontaneous disk decay. One fatal READ is observed before the
mandatory HALT, and **no** RESIDENT publication, post-HALT retry,
second overlapping swap DMA or continued unaffected-provider
service is claimed. v1590/v1603 LOGSTORE failures are deliberately
recoverable and operate on a *different* backing service; their
successful retry is not evidence that a critical owned BACKSTORE
failure can or should recover.

The source-linked v1602 write-through test controls the final DSK
or DRM device-completion return and is not independently a full
physical detach. Earlier v1545 real secondary Type-270 and Type-236
device detach with a healthy independent selected D6FS root already
establishes genuine nonroot failure isolation. The older native
DTFS v1518 fixture assumes a pre-v1561 write entry and did not pass
its source-mutation generation on current code. No security bypass
or false retroactive pass was introduced; the source-authentic
v1490 and the physical native-DTFS failure/return work remain the
applicable tests. Do not claim an unrun v1518 regression passed.

## Final closure and boundaries

**P1-C CLOSED:** actual dual-PI6 process ownership, checked
RESIDENT/rollback, physical mapped VOLUME and direct DEVICE fatal
swap owner error, separate Type-551 provider loss with healthy MEMFS,
clean write-through cache failure and successful authoritative
replacement, real recoverable physical LOGSTORE torn retry while
two separately PI6-woken owners are active, and repeated physical
loaded execution are all accepted in the evidence above.

P1-D dynamic FSINFO/controller inventory is separately CLOSED by
v1601. The **P0-B physical removable-reel retention/interlock** is
an independent unresolved policy gate and is NOT silently waived
by this P1-C closure. **P1-E KA10 PURE/MM/APR** design and the
strict 32K size/optimization gate remain on HOLD. No permanent
KCORE, MM, driver, ABI, MRES or physical media-format changes are
made by the final v1602/v1603 test-only acceptance.

Primary testkit reports:
`tests/validation/daimos-p1c-write-through-cache-v1602.md`,
`tests/validation/daimos-p1c-two-owner-cache-logfault-physical-v1603.md`.
Accepted full physical composite evidence:
`$TMPDIR/daimos-p1c-composite-v1603.Z4dCmb` and
`$TMPDIR/daimos-p1c-v1603-fullguest-run1.log`.
