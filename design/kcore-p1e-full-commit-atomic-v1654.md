# PDP-6 P1-E v1654: FULL MM target-unpin before source deletion

Date: 2026-11-17. **Production opt-in PURE MRES correctness fix.**
The original source-linked native PDP-6 test and a new source-built
48K physical Type-551 DTC3 read-error/rollback/retry guest PASS.
Permanent KCORE and all process/MM persistent layouts are unchanged.

## Original correctness defect

The original v1391 bounded FULL restore kept both the retained
PACKED mutable source and the newly reconstructed PROCESS APR pinned
through its source-backed loader and RAW overlay. However, once the
loader finished, it unpinned and destroyed the source, published
the new APR and RESIDENT storage, then attempted mm_unpin(target).
A failure of that final fallible MM operation left RESIDENT with
TRANSITION still held, a pinned destination and NO authoritative
PACKED source from which the established service could retry.
This was a transaction/ownership failure, not a compiler bug or
a justification for a second process registry.

## Correct production commit order

In system/kernel/proc/proc_pure_ram_restore_v1391.c, after the
original exact-capacity bounded file load and RAW suffix overlay,
the short commit interval now runs:

1. Disable PI; unpin the complete destination while the original
   pinned PACKED suffix is still intact.
2. Unpin the original source and check the exact owner/type free.
3. Only after every fallible MM operation has succeeded, publish
   the complete APR, clear saved PURE backing, switch packed mode
   to RESIDENT and clear TRANSITION.
4. Restore PI.

The PI-disabled interval contains NO file/device I/O, MM allocation
or arbitrary blocking operation. The previous v1391 original
loader still runs under normal device PI with both MM extents
independently pinned and the immutable source guaranteed.

On a ONE-SHOT destination-unpin refusal, the checked rollback
can unpin and free the provisional target, unpin the retained
source and return MM_ERR_BUSY with exactly one unpinned PACKED
suffix, untouched RAW contents/backing and TRANSITION held.
The next source-linked restore retries successfully. If a
destination unpin refuses permanently, target and source
remain pinned with a TRANSITION-held PACKED owner. If source
unpin refuses permanently, the temporary full destination
is freed but the original RAW remains pinned and unavailable
for dispatch. Neither case publishes RESIDENT or invents
recovery from unknown MM state. The separate permanently
uncertain ownership recovery gate is still OPEN.

No new persistent bit, per-process record, dynamic dispatch
table, second MM registry, storage ABI or disk format.
PURE remains explicitly opt-in/OFF by default.

## Actual native PDP-6 acceptance

Testkit v1654 updates the original v1395/v1397 native MM fault
oracle to require a one-shot final-target-unpin refusal to
roll back while the source remains authoritative and unpinned,
then accept exact successful retry. Its earlier native
source-free/target-pin and rollback suite v1648 also passes,
including two separately executed invalid overlay/target-free
mutants. Two NEW original C transaction cases inject
permanently refused destination and source unpins and verify
the complete owner type/size/pin, VM, backing and TRANSITION.

An independently compiled and EXECUTED test-only mutation
restoring the old post-publication target-unpin order is
rejected by the original native oracle. It did not merely
fail assembly or linking. No mutant entered production.

Testkit branch:
test-kcore-p1e-full-commit-v1654-20261117
commit 01a6932.
Detailed reproduction:
tests/validation/pure-full-commit-v1654/README-v1654.md.

## NEW real 48K original PDP-6 physical validation

The user guest and full KCORE were newly rebuilt from the current
DAIMOS source on the original PDP-6 GCC 48K/no-AUX/TSFS-root
profile with actual native secondary DTC3 DTFS and original
immutable compressed DXR2 backing. A first native 48K execution
accepted all six child RUN/PIPE/private-DATA/EXIT-057/PID/WAIT
checks after one refused seventh RUN and normal PC000000 HALT.

The independent second 48K replay retained ALL original
v1643 source-linked physical strict checks and changed ONLY
the validated MRES bounded-loader entry offset:
003224 -> 003233 octal. Actual loader PC 035261 octal,
original restore entry offset 002207, original PURE service
000346, and original loader-return offset 002333.

At the real bounded FULL callback, slot 3 concurrently owns:

- Pinned PACKED RAW 111200..120000 octal, one 006600-octal suffix.
- Separate pinned PROCESS target 120000..130000 octal,
  one full 010000-octal APR.

Physical detachment of ONLY DTC3 causes the original
non-root bounded read to return -4, leaves the exact
source unpinned/unchanged and releases the full target.
The original TSFS ROOT remains online. Reattaching exactly
the same private DTC3 reel permits a later FULL success
and exact six EXIT/WAIT/status checks, final no child MM
owner and normal HALT. The original v1643 source-linked
auditor PASSes and independently rejects a forged RAW pin
and a forged successful I/O return. These are genuine
original physical device conditions, not host-only callbacks.

## Exact footprint

The newly linked GCC 48K fixed KCORE map still has
__kcore_low_end=024253 octal; the actual fixed KCORE
stream is byte-for-byte identical to the v1652 accepted
source-built GCC KCORE:

SHA256 88970ee633dc22094565172348bc1dcb9f49ba0417e104fb5608b71a87668d01.

The optional PURE-ON KINIT link end moved 113621 -> 113631
octal (+8 decimal PDP-10 words) because of the fuller
check/PI sequence in the transient removable MRES.
This is NOT fixed KCORE growth. The strict 32K source
capacity remains deferred and must not be reinterpreted
as a new 32K boot acceptance.

Evidence archive:
$TMPDIR/daimos-p1e-v1654-evidence-20261117.tar.gz
SHA256 294ea909464d5d42226042296affeb8a7c1f768d2f035873b62354afe47d438d.
The archive retains source-linked maps, both original DXRs,
actual 48K trace and private media hashes, native and physical
regression logs, invalid mutant execution and versioned testkit SHAR.

## Scope and next work

**ACCEPT:** transactional source/destination cleanup and resident
publication with short PI atomicity, original native failures,
negative mutant, and actual physical secondary-media FULL
rollback/retry/WAIT.

**NOT ACCEPTED:** recovery from permanently corrupted/uncertain
MM owner/pin state, continuous KCORE/MRES/BCACHE/loader peak,
new 64K ON/OFF net savings claims, optional mutable suffix
compression, full matched-current-source KCORE freeze.
Strict 32K boot/size acceptance is DEFERRED until after functional
freeze by explicit request. Distinct KA10/KI10 VM targets are not
implemented PDP-6 KCORE release blockers.
See design/kcore-finish-priorities-v1654.md.
