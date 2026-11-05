# F07 physical-MM/foreign-provider simultaneous snapshot v1203

Date: 2026-11-06. **Partial F07 evidence; F07 remains OPEN.**

## Exact source and execution

- DAIMOS: `cf4e85d47b968b14410e1415988ef5b3434dcfa5`.
- daimos-testkit: `aae6d2d3b3d18b0375deb1ecc56b6d9a8c0d83d5`.
- PDP-6 256K full KCC DSK/D6FS release-media, 60 Hz PCLK, CTY,
  explicit protected-process and held-foreign-provider scenario.
- Testkit `daimos-f07-mm-peak-v1203.sh` passes separately with
  `V1203_OOM_ENABLED=1` and `V1203_OOM_ENABLED=0`, and fresh unique
  `V1203_EVIDENCE_DIR` paths. Production OOM remains disabled.
- Both full guest runs PASS v1075 physical requester fault, v1150
  provider owner/private stack/waiter, v1159 protected bystanders,
  and the new v1203 read-only MM snapshot verifier.
- Ten negative host-verifier mutations are rejected, including invalid
  table capacity, overlapping/unaligned extents, mis-owned provider,
  truncated snapshot, and improper shared workspace overlap.

The test-only probe samples the live MM table and arena words at the
**genuine physical BRK -1/-1 decision**, before selecting the victim,
while another process owns the filesystem-provider lease. It copies
at most 272 descriptor words to disposable BSS without allocating,
waiting, locking a provider or growing the kernel stack. The original
source, resident production image and ABI are unchanged. The extra
snapshot BSS exists **only in the disposable instrumented guests**.

## Observed simultaneous state (36-bit words)

| Quantity | Test OOM on | Production OOM off |
| --- | ---: | ---: |
| Total physical words | 262144 | 262144 |
| Managed arena words | 242714 | 242728 |
| Allocated managed words | 112692 | 112692 |
| Free managed words | 130022 | 130036 |
| Active MM descriptors | 19/136 | 19/136 |
| Process extents | 6 | 6 |
| Kernel-dynamic extents | 13 | 13 |
| Managed free gaps | 3 | 3 |
| Largest free gap | 128984 | 128998 |
| LOW APR placement prefix slack | 990 | 1012 |
| Upper bound per gap (511 * 3) | 1533 | 1533 |
| Logical break rounding upper bound (511 * 6) | 3066 | 3066 |
| Shared resident VFS block workspace | 128 | 128 |
| Original foreign provider lease | 7,,0 | 7,,0 |

Each live process extent is aligned to `02000` octal words and its
physical length is APR-quantized. All recorded extents are sorted,
nonoverlapping, lie in their managed arena, and exactly reconcile
allocated plus free words with managed words. The VFS workspace
resides outside managed MM extents. Both modes have the same live
allocated footprint and descriptor types; the enabled guest has 14
more resident test-policy words and therefore 14 fewer managed words.

**Definitions:** LOW placement prefix slack is the exact number of
words before the next APR-aligned base in each sampled free gap; it
is not all possible fragmentation loss. Logical BRK rounding is
**bounded**, not read directly from each process's u-area.

## Evidence

`$TMPDIR/daimos-f07-oom-lease-v1203r2.tar.gz`

SHA256:
`e4fdbd54d04a1052ffec87a621d2abc53840f1f76712e71bf76cca6c71bac0ff`

The archive includes both complete raw SIMH and build logs, link
maps, snapshot address list, boot configurations, media and tool
hashes, per-run verified file hashes, original wrapper logs and a
cross-policy JSON comparison. Each guest's recorded Git revisions
match the frozen sources above. This is not a production RAM map.

## Outstanding F07 requirements

This was a **real physical OOM** triggered by a large BRK request;
it was *not* a near-zero-free-core or descriptor-saturated peak.
There were approximately 130000 managed words free and 117 unused
MM descriptors when the request failed. Accordingly, this successful
probe does **not** close F07.

Still required: a genuine maximum concurrent process/extent load
with held foreign provider and active shared buffer, observed peak
descriptor/physical occupancy, per-process logical BRK rounding
or a justified conservative bound, and APR/gap accounting under
that same maximum load. Preserve v1195 stack limits, the v1196
136-descriptor/fragmentation negative tests, and v1202 A04/F02
owner-safety gates. E13/F11 remain separately open. Requester OOM
production switch stays OFF; total checklist remains **59/64**.
