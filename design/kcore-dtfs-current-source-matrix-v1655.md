# KCORE v1655: native DTFS current-source regression, not final freeze

Date: 2026-11-17. **Current PDP-6 source-linked DTFS/BCACHE
regression PASS.** No production KCORE, DTFS, VFS, MM,
MRES, boot, ABI or resident memory changes.

## Historical fixture defect and actual policy

The old v1518 raw DTC I/O/cache and v1522 mounted-volume-ID
standalone test scripts contain extractor/mutation anchors
predating v1561's pre-write MM intent/ownership gate and v1572's
trusted-operator V2 writable policy. Against the present source,
both STOP BEFORE NATIVE EXECUTION. Their older dated PASS
results are retained as historical evidence, but neither is a
fresh current-source acceptance. This is a testkit maintenance
defect, not evidence of a new production DTFS failure.

The later v1556 read-fault fixture also contained obsolete
source mutation anchors and two old RO-only V2 expectations.
In the current approved v1572 operator-trust policy, an
unarmed V2/CLEAN volume can be writable prior to verified
first DIRTY intent; mid-read RW promotion of the unchanged
unarmed identity is not automatically denied solely for
having RW policy. The testkit was changed to expect those
actual source-backed outcomes. A new native case tests
the distinct forbidden V2/DIRTY ARMED-under-RO condition;
the obsolete RO polarity mutation was replaced with an
executed mutation that ignores ARMED on RO. All original
physical RBN0 and source-identity safety checks remain.

## Actual independently executed native PDP-6 matrix

The versioned v1655 runner requires the following modern
source-linked test suites to execute against the CURRENT
DAIMOS tree, not merely to find historical PASS logs:

| Source-linked domain | Native cases | Bad-source mutants rejected |
| --- | ---: | ---: |
| v1556 current V1/V2 physical RBN0 recovery | 30 | 14 |
| v1554 physical V2 DIRTY identity and readback | 17 | 10 |
| v1561 mounted physical-unit/intent/MM ownership | 35 | 13 |
| v1575 poisoned-owner discard (actual leaf) | Leaf | 2 |
| **Total enumerated cases** | **82** | **39** |

Original source high/low mounted volume-ID halves, physically
read reserved RBN0, read-fault marker policy, short transfer
rejection, shared BCACHE slot-0 invalidation, WRITE_UNCERTAIN
fail-closed protection, V2 CLEAN/DIRTY seal, RO/ARMED policy,
stable mount/provider state through blocking device I/O,
exclusive MM mount/physical-unit owner, and poisoned-owner
cache-drain safety are all exercised by actual PDP-6
instructions. Each of 39 deliberate source mutations
executes and fails its native oracle. None of the mutations
or additional state are linked into production.

Testkit branch:
test-kcore-dtfs-current-matrix-v1655-20261117
commit a048ab3.
Runner: tests/validation/dtfs-modern-v1655/run-v1655.sh.
Original modified fixture:
tests/validation/daimos-dtfs-v2-revalidate-v1556.sh.

Evidence archive:
$TMPDIR/daimos-kcore-v1655-evidence-20261117.tar.gz
SHA256 f39b620a6d9462d62e3dcd5d15b9d7ad928afb4ebd291bf18b898ce0e81dc9db.

## Exact limits

This is a true **current-source native provider/owner matrix**,
not a new full multi-process physical 48K DTFS guest.
The original v1518 and v1522 standalone integration
fixtures still do NOT pass with current source. Their
precise combined raw-read/write-cache and mounted-volume
revalidation path must either be modernized in a separate
versioned task or replaced by equivalent actual physical/
native integration proof. Do not mark those two stale
scripts PASS based on the v1655 aggregated result.

The accepted v1575 writable/removable-media policy assumes
a trusted operator prevents reel exchange during an RW mount.
No test claims an unavailable mechanical interlock.

This closes the v1556 current-source recovery-oracle
regression and adds a substantial KCORE freeze matrix.
Remaining KCORE work includes source-sensitive v1518/v1522
integration, complete transient RAM, optional mutable
compression decision, permanent uncertain MM ownership
and full PDP-6 release-freeze regression. Strict 32K
size/physical boot acceptance is explicitly DEFERRED.
