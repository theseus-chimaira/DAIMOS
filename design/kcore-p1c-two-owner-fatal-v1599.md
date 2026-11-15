# P1-C v1599: two independent PI6 owners during physical BACKSTORE fatal

Date: 2026-11-15. Full real 128K PDP-6 KCC guest on four
Type-236 DR0..DR3 D6FSR2 root VOLUME members, original BLOCKSET
INTERLEAVE BACKSTORE, genuine LOGSTORE/MEMFS persistent snapshot.
Original fixed KCORE, process scheduler and swap transaction,
MM/BCACHE, BACKSTORE gateway and DRM236 driver error branch
are unchanged. Versioned disposable testkit instruments only
original state addresses and redirects one selected authentic
Type-236 DR_DONE completion into the original driver-error leaf.

The v1584/v1585/v1592 source observers select **two different**
original timed SLEEP/DISK processes holding positive original
backing records, slots **6 and 7**. They retime only existing
deadlines to the same next 60 Hz tick during actual MEMFS snapshot
shutdown; no invented RUN, injected PI6, swap call or device
completion. The original PI6 timer service wakes each **once**.
The original independent slot-0 service enters `proc_swap_in`
for slot **7**, issues a real physically mapped DRM swap READ,
then receives one authentic Type-236 DR_DONE PI2; a test-only
first-READ arm diverts that completion to original
`drm236_pi_error`, and original critical BACKSTORE reason-2
HALT occurs at linked octal **`004011`**. Do not retry after
fatal owned-storage error; P0-A policy requires a no-return HALT.

Physical mapped member **DR1**, sector **`017506`**, actual
hardware group **`0375060`**, original BACKSTORE relative block
**`000431`**. Original `proc_swap_blocks_used=01650` and
`backstore_blocks_used=01734` octal remain nonzero; selected
physical swap READ and matching PI2 both 1, driver error 1,
RESIDENT publishes 0, and snapshot remains in-progress and
uncommitted. The four real physical D6FSR2 member layouts
are unchanged, actual selected backing block has nonzero data,
and 268 other backing sectors are nonzero. Full physical
guest/source-linked raw media oracle **PASS**; eight new PI6
owner witnesses and all fifteen inherited v1586 physical PI2,
DMA, HALT, ownership and media mutants fail (**23** rejected).
The existing single-owner v1586 physical oracle still PASS.

This *adds* a simultaneous two-owner timer-wake critical-fault
boundary to v1586. Only **one actual swap READ DMA** completes
before the required HALT; it does not prove multiple concurrent
DMAs, spontaneous magnetic failure or postfatal recovery.
Distinct direct DEVICE-backed DRM AUXSTORE with an independent
DSK root, dirty/write-through faults and repeated loaded timing
remain open P1-C. Final P1-D service matrix and P1-E KA10
architecture review remain open. No permanent KCORE source/
RAM/ABI/media changes; VERSION 0.5, fixed low end `024364`
octal, strict 32K optimization HOLD.

Testkit source, runner and complete physical report:
`tests/validation/daimos-p1c-two-owner-fatal-physical-v1599.sh`,
`tests/validation/daimos-p1c-two-owner-fatal-verify-v1599.py`,
`tests/validation/daimos-p1c-two-owner-fatal-physical-v1599.md`.
Accepted evidence path:
`$TMPDIR/daimos-p1c-two-owner-fatal-v1599.OjP5cW`.
