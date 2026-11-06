# F07 high physical/descriptor MM pressure v1204 (2026-11-06)

**Result: PASS for genuine high-occupancy held-provider fault and
exact rollback. F07 as a whole remains OPEN.**

## Frozen revisions and media

- DAIMOS: `75ccef85a8d2a4723685cd2bac9dde8a1bc382bd`.
- daimos-testkit: `aa7cb8694c0a61e1cd041368e47031b5cbb969e0`.
- 256K PDP-6, KCC, DSK stage 1/D6FS disk root, full profile, CTY,
  60 Hz PCLK. Production `PROC_OOM_BRK_ENABLED_V1000=0` throughout.
- Disposable test: `daimos-f07-mm-high-v1204.sh`, separately with
  `V1204_OOM_ENABLED=1` and `V1204_OOM_ENABLED=0`, each with its own
  versioned `V1204_EVIDENCE_DIR`.
- Both independent exact-commit simulator builds exit 0 and PASS
  v1075 physical OOM, v1150 in-flight foreign provider and waiter,
  v1159 protected INIT/witness/spinner, v1203 live MM snapshot, and
  v1204 high-pressure exact cleanup assertions.
- Host verifiers PASS 10 v1203 and seven v1204 negative mutations.

## What was actually loaded

The disposable copy of BRK obtains 12 real MM_TYPE_KERNEL_DYNAMIC
blocks of `020000` octal physical words (8192 decimal), each aligned
to APR `02000` octal (1024 decimal), and 96 one-word blocks. These
108 *additional* extents hold 98400 words with one unique test owner.
The allocations are made **after** the six original processes have
RUN and the foreign D6FS owner has acquired its original provider
lease, but before waking the OOM requester. The snapshot is captured
on the real physical `BRK -1/-1` error path, before choosing the
requester-only OOM policy action. No pretend MM return is injected.
The final cleanup calls the original owner/type-checked MM free for
all 108 recorded bases. No test-owned extent remains afterward.

The immutable, original 128-word VFS block workspace is resident
and serialized by the in-flight gateway. The test-only instrumentation
does not allocate another provider workspace or change runtime KCORE.

## Genuine simultaneous snapshot (36-bit physical words)

| Measurement | OOM test-enabled | OOM production-disabled |
| --- | ---: | ---: |
| Core words | 262144 | 262144 |
| Managed MM words | 242518 | 242532 |
| Live allocated MM words | 211092 | 211092 |
| Managed free words | 31426 | 31440 |
| Live MM descriptors / capacity | 127 / 136 | 127 / 136 |
| Process / kernel-dynamic extents | 6 / 121 | 6 / 121 |
| Extra pressure extents, exactly released | 108 / 108 | 108 / 108 |
| Pressure-owned words | 98400 | 98400 |
| Free gaps | 3 | 3 |
| Largest free gap | 29696 | 29696 |
| Measured LOW APR placement-prefix slack | 706 | 720 |
| APR prefix-slack upper bound for 3 gaps | 3069 | 3069 |
| Logical BRK rounding bound for 6 extents | 6138 | 6138 |
| One shared filesystem transfer workspace | 128 | 128 |
| Original provider lease | 7,,0 | 7,,0 |

The physical allocation fraction on enabled core is
`211092 / 242518 = 87.04%` (disabled 87.04%); descriptor occupancy
is `127 / 136 = 93.38%`. The two policy states have equal live
allocation and descriptor counts. The extra 14 test-enabled OOM
instruction words reduce that guest's free/managed space by 14.

Both original owners and the distinct in-flight FS waiter retain
their correct identity; the witness/spinner advance, the faulted
requester is retired only in the test-enabled variant, and exact
WAIT, FILE, u-area, physical MM and final baseline checks PASS.

## APR verifier correction

The v1203 testkit incorrectly used a 511-word maximum for each
APR-alignment loss and per-process logical break rounding. Since
APR alignment is **02000 octal = 1024 words**, the correct
conservative maximum is **1023 words** per free gap or process
extent. v1204 fixes both calculations, extends host controls, and
reruns the physical snapshots. The historic v1203 archive remains
unchanged and must not be used for those two derived bounds.
Actual v1203/v1204 measured gap slack, alignment validation and
allocated/free extent counts are otherwise unaffected.

## Complete evidence

`$TMPDIR/daimos-f07-high-mm-v1204r2.tar.gz`

SHA256:
`00bb9942c78455d616160649b77756823b3715b74ba14fbe5e1dc73aeb91513d`

Archive contains both exact-revision boot logs, counters, complete
SIMH output, build diagnostics, KCORE/INIT maps, boot configurations,
root-media checksums, host-tool hashes, checksummed MM snapshots,
cross-policy JSON comparison and manifest. All embedded archive
file checksums were independently rechecked before signing this
partial result.

## What F07 still needs

This is high occupancy under real MM errors, but the 108 test-owned
kernel-dynamic allocations do not represent 108 runnable processes.
Only six real process image extents were present and process-slot
concurrency was **not** stressed to its architectural/profile bound.
Actual logical BRK values in each live u-area were not snapshotted;
the 6138-word rounding value is a conservative upper bound.
Earlier independent v1195 measurements remain valid for the startup
stacks (private process highwater `0300` octal versus `0310` warning
limit; idle stack `025` octal versus `060` limit), but those are not
simultaneous high-concurrency stack highwater measurements.
Finish true high concurrent process-slot/stack occupancy and exact
per-process logical-brk/APR quantization before closing F07.
E13 and F11 remain separate pre-activation requirements; deferred
F12/F13 are not authorized. Overall requester-only OOM gate status
remains **59/64**. Production policy remains OFF; no production
kernel code, permanent RAM or ABI changed.
