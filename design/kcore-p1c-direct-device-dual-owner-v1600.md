# P1-C v1600: independent root and direct DRM DEVICE BACKSTORE fatal

Date: 2026-11-15. Full original 128K KCC PDP-6 with **four separate
DSK270 D6FSR2 root members** and **independent DRM0 AUXSTORE direct
DEVICE BACKSTORE**; genuine LOGSTORE/MEMFS snapshot. Previous v1599
ran a four-member DRM236 *shared ROOT VOLUME* with an INTERLEAVE
BACKSTORE mapping. v1600 explicitly checks this new different
controller/ownership topology rather than repeating mapped VOLUME.

The raw DRM0 AUXSTORE RBN0 and preboot `mkauxstore --inspect` agree:
SIXBIT AUXST1, version 1; BACKSTORE at physical `05770+010000`,
LOGSTORE `05370+0400`. Original source-linked KCORE
`device_region_map=400000005770`, `volume_data_blocks=0`,
`backing_region_count=010000` octal. DSK0..DSK3 each hold a
physically valid, distinct 45056-sector D6FSR2 root member;
the original four member identities and matching geometry are
examined independently of the drum image. Original D6FS root,
MEMFS/LOGSTORE were loaded, original persistent LOGSTORE data
was written/read before the physical fault.

The existing disposable v1584/v1585/v1592 PI6 timer observer
retimes two existing, distinct, genuinely backed SLEEP/DISK
children (slots **6** and **7**) to the next timer tick during
MEMFS shutdown. Original PI6 service wakes each exactly once;
neither RUN/slot-0 swap-in nor simulated I/O completion is faked.
Original slot-0 service selects slot **6**, starts direct DRM0
BACKSTORE READ; the actual physical address is DR0 `006421`,
logical BACKSTORE `000431` octal. First authentic DR_DONE PI2
is test-selected through the original DRM error handler and
unchanged critical BACKSTORE gateway, which reaches the mandatory
reason-2 HALT **`004011`** octal. Positive original ownership
`proc_swap_blocks_used=01650`, `backstore_blocks_used=01734`
octal remains, no RESIDENT commit, no falsely committed MEMFS
snapshot, 268 other physically nonzero backing sectors and four
valid root layouts. All original linked-address/physical oracle
checks and **27 deliberately false evidence controls PASS**.
The previous mapped VOLUME v1599 (**23** mutants) and single-owner
v1586 (**15**) accepted regressions still PASS after the shared
verifier gained a strictly explicit direct-DEVICE mode.

One correction to prior v1599 prose: a recheck of the original
v1599 source-linked post-HALT `daimos_v1584_last_slot` actually
selects **slot 6**, not slot 7. Both slots really woke on PI6;
the accepted v1599 oracle correctly required its selected slot
to match one of the two. That historical explanatory typo has
been corrected in active/project/historical documentation, while
v1599's physical PASS and 23 rejected witnesses remain valid.

These cases prove a genuine selected one-READ critical PI2 error
with two independently awakened owners. They do **not** prove
two simultaneous swap DMAs or continued work *after* the fatal
halt; original P0-A requires fail-stop. Neither the real physical
root's post-fault dirty superblock state nor arbitrary pending
user writes are guaranteed by a mandatory abrupt halt. Remaining
P1-C feature tasks are physical dirty/write-through provider
failure and cache isolation, repeated loaded timing and final
current-source matrix; P1-D final root/service inventory and
P1-E KA10-aware PURE design remain open. No permanent KCORE
source/ABI/RAM/media change; VERSION 0.5, fixed low end
`024364` octal, strict 32K optimization HOLD.

Testkit full report and original retained guest:
`tests/validation/daimos-p1c-device-two-owner-fatal-physical-v1600.md`
and `$TMPDIR/daimos-p1c-device-two-owner-v1600.kaKCCA`.
