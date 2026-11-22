# P1-E v1652: genuine PDP-6 64K nine-PACKED-owner physical acceptance

Date: 2026-11-17. Original source-built physical 64K
PDP-6 lifecycle, genuine Type-551 secondary DTFS, nine
PACKED children and final MM owner-release replay PASS.
No changes to production KCORE, MM, PURE, scheduler,
VFS, guest ABI, hardware driver or filesystem format.

## Independent 64K physical build

The original v1643 48K physical C89 diagnostic generates
an exact copied secondary native DTFS executable source,
using original mount/OPEN/READ/WRITE/close/CHMOD/UNMOUNT
syscalls, then remounts DTC3 RO. The v1652 source generator
only changes the number of required child RUN/WAITs from
six to nine. Eight children use the source-derived compressed
DXR2, one uses the plain ROOT DXR2. The parent retains
the original PIPE, DATA, status-057, per-PID WAIT and
refused tenth RUN checks. There is no custom MM code,
kernel instrumentation, user MM introspection or bypass.

An exploratory ten-child 64K guest admitted exactly nine
children, then the tenth RUN returned -1 and the source
printed !r. All nine already admitted children exited
and the guest HALTed normally, but the original ten-child
expectation correctly reported FAIL. The nine-child
source-derived run instead checks the same real tenth
RUN refusal and prints the expected x marker.

The accepted original 64K no-AUX lowmem boot loaded
PURE and DTFS, checked the source-matched compressed
DXR2 and three-member TSFS original media. Its console
contains nine a, one x, B/A, L, nine w, Z and one
normal PC000000 HALT with no guest ! marker.

## Separate original physical MM B/A/Z replay

The unmodified physical machine is replayed with PRIVATE
copies of all four original DTC Type-551 tapes. The
SIMH monitor examines the real KCORE symbol-linked
MM arena and sorted two-word original descriptors at
guest checkpoints B/A/Z. No standalone C MM imitation,
kernel patch or fake owner is used.

Original 64K MM arena: octal 042601..200000
(47,743 decimal physical 36-bit words).

| Stage | PACKED child owners | Child pins | Free | Max gap |
| --- | ---: | ---: | ---: | ---: |
| B | 9 | 0 | 9,404 | 4,155 |
| A | 9 | 0 | 9,404 | 4,155 |
| Z | 0 | none | 42,769 | 42,769 |

At B and A, nine original distinct process slots 2..10
each own one PACKED_PROCESS RAW suffix, octal 006600
(3,456 decimal) physical words, pin0. The nine
exact MM extents are unchanged B to A; the normal
root namespace remains TSFS. Original v1604 text
omission derives 001200 octal from source-linked
compressed text 001227 octal. Parent slot1 retains
its original full 010000-octal unpinned PROCESS APR.

At Z all nine child PROCESS/PACKED owners are gone;
the parent PROCESS remains unpinned. The same original
user checks enforce nine exact PID/EXIT-057 WAITs.

Three separately forged physical trace copies must
fail the SAME MM-owner auditor: compressed PACKED
extent misidentified as PROCESS without full APR size,
duplicate process owner, and fake final WAIT success.
All three invalid witnesses are rejected. The complete
source-compressed DXR/console check is independent.

Testkit:
tests/diagnostics/pure-64k-owners-v1652/
Branch test-kcore-p1e-64k-owners-v1652-20261117.
Commit 86cdbdd.

Independent full original evidence and versioned SHAR:
$TMPDIR/daimos-p1e-v1652-evidence-20261117.tar.gz
SHA256:
ded569d26f2daf7efdd9a795434a563f242719713568992df81e419a72b37ca8.
The archive includes exact source-built original C89
guest, plain/compressed DXR, KCORE and PURE MRES
linker maps, private replay script, physically sampled
MM trace, real nine-child lifecycle, ten-child negative,
three rejected forged owner witnesses, original DTC
media SHA identities and the parent-branch SHAR.

## Limits

This is NEW genuine 64K physical PACKED-owner
lifecycle acceptance, not projection from 48K. The
original v1639 +1,314-word PURE ON/OFF free-memory
gain was measured in a different matched 48K workload;
it is NOT a demonstrated 64K net saving. A continuous
64K transient FULL source+target peak was not sampled.
Uncertain permanently pinned MM ownership, 32K
physical acceptance and distinct KA10 VM remain OPEN.
PURE remains opt-in/OFF by default, strict 32K KCORE
optimization HOLD.
