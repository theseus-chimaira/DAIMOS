# PDP-6 KCORE remaining work after v1653: 32K acceptance deferred

Date: 2026-11-17. Priority/order decision for the current PDP-6
KCORE release. The v1607 feature inventory and all dated
source-linked physical results through v1653 remain evidence.
This replaces their priority ordering, not their test history.
No claim that all outstanding functional gates are closed.

## Immediate scope decision

**Strict 32K KINIT boot and footprint acceptance are DEFERRED**
by explicit direction. The current exact source-matched
GCC PURE-ON image is 6,289 PDP-10 words above the 32K
bootstrap stack-safe end (v1653); the PURE-OFF image is
already 4,394 words above. That unresolved target remains
documented, but it is not a prerequisite for closing
the remaining functional PDP-6 KCORE gates. Do not lower
the required 0400-octal KINIT stack, remove essential MRES,
fake a 32K successful boot, or start the final KCORE size
optimization before the functional freeze.

No new KA10/KI10 VM implementation is part of this
PDP-6 freeze: the v1605 source selector is preparatory
and explicitly has no accepted non-PDP6 backend.
Segment/paging/CPU hardware-specific work remains a
later independent port, not a fake PDP-6 APR alias.
Likewise TSFS v1.2 and dynamic MRES unload are post-KCORE.

## Accepted original production functionality

* P0-A selected ROOT/BACKSTORE failure policy: accepted
  source-linked native/physical fatal and rollback matrix.
* P0-B DTVOL2 writable DTFS: accepted under the explicit
  no-reel-exchange trusted-operator policy, NOT an
  unproven physical reel interlock.
* P0-D/E/F requester-only OOM: accepted for its documented
  original kernel/CPU profile, including F07 no-partner.
* P1-C combined storage, swap, clean BCACHE and LOGSTORE:
  original mixed-provider physical closure v1603.
* P1-D selected root, missing physical controller and
  dynamic FSINFO capacity: physical v1601 closure.
* P1-E RAW zero-copy PURE: installed only on opt-in
  no-BACKSTORE profile. Real PDP-6 48K/64K PACKED
  process RUN/EXEC, PI6 SLEEP/WAIT/EXIT, immutable
  secondary DTFS backing, V2 quiescence admission,
  physical independent pinned FULL/failed DTC3 read,
  exact rollback and subsequent successful loader are
  accepted by v1637-v1652. Original 64K has nine
  simultaneous PACKED child owners. The original
  matched six-owner 48K ON/OFF test reports a
  1,314-word net gain in that specific workload,
  not an unmeasured 64K or global gain.

## Work that must finish before PDP-6 KCORE freeze

1. **MM failure atomicity.** v1654 removes the
   post-publication destination-unpin hazard in the
   actual FULL transaction; retain the original source
   until every target release and source-free operation
   can succeed. Validate source-linked v1395/v1648
   native and an independent genuine 48K physical
   DTC3 fault/recovery/WAIT replay of the rebuilt
   MRES. Permanently uncertain MM pins must fail
   closed and cannot be blindly unlocked.
2. **Physical memory accounting.** Reconcile complete
   fixed KCORE, installed removable MRES, dynamic MM,
   BCACHE/provider scratch and simultaneous RAW/full
   APR peak under the same source-built workload.
   v1651 is a sampled 48K MM table, not a continuous
   full-machine high-water. Repeat a matched ON/OFF
   workload if making new net savings claims.
3. **Optional mutable suffix compression decision.**
   Measure actual 36-bit writable suffix entropy,
   codec MRES/RAM/stack and both-source-and-target
   decode peak. Implement exclusive encoded state
   only if it strictly improves net words including
   its permanent runtime costs. A documented
   **NO COMPRESSION** decision is acceptable and
   completes the optional gate without new code.
4. **Current-source regression/freeze.** Rerun native
   PI6, RUN/EXEC/WAIT/BRK, F07/OOM, ordinary DISK
   swap, D6FS/DTFS/TSFS, selected-root fault and
   BCACHE/LOGSTORE owner tests plus source-accurate
   48K/64K physical profiles. Modernize or explicitly
   supersede the older stale v1518/v1522 DTFS
   extractor after v1561/v1575; do not count a
   broken source fixture as passing.
5. **Final release audit.** Pin revisions, build
   options, original media identities and linked
   maps; document exact permanent KCORE and MRES
   words, temporary buffers, limits, supported
   CPU/profile, fail-closed policies and freeze
   verdict. Only AFTER this stage start a separate
   whole-KCORE optimization and reconsider
   strict 32K within the measured 6,289-word
   minimum original transient KINIT reduction.

Do not interleave size-only p10fold/p10super sweeps,
speculative cross-MRES folding or other features with
unclosed correctness gates. Correctness and minimum
RAM remain the constraints for each functional edit.
