# F07 simultaneous real-process BRK/APR/MM occupancy v1205

Date: 2026-11-06. **Subcase PASS, F07 overall still OPEN.**

## Frozen provenance

- DAIMOS `684d45044629b0bb909692a68855a4ed0fde084d`.
- daimos-testkit `f2222e27e906233d1d94d82fedb972c455bd647f`.
- Full 256K PDP-6 KCC DSK/D6FS release-media boot with CTY and
  60 Hz PCLK; production requester OOM remains disabled.
- Two independent disposable guest rebuilds with
  `V1205_OOM_ENABLED=1` and `V1205_OOM_ENABLED=0`, each exited 0,
  passed v1075/v1150/v1159/v1203/v1205 and passed raw evidence
  checksum validation.

## What is different from v1204

The original six live users (INIT, witness, requester, waiter,
spinner, foreign FS lease holder) are supplemented with six
**genuine RUN child processes**, each with its own user VM, APR,
stable executive u-area, and non-quantized published logical BRK.
Each new child sleeps independently until explicitly terminated;
the original held-provider OOM chain then runs without modification.
The test-only kernel snapshots all process descriptors and each
resident u-area's exact BRK at the same real physical BRK failure
that captures MM descriptors. It allocates no resident production
RAM or new provider buffer and makes no allocator calls in the probe.

The verifier matches the MM_TYPE_PROCESS extent owner and physical
base/length to each live process, independently checks APR `02000`
rounding, and computes the **actual** sum of physical APR padding
minus logical BRK, rather than a 1023-per-process bound. Host
negative fixtures reject lost processes, bad BRK, wrong VM size,
duplicate MM ownership and unmarked u-area. Two development-only
sampler errors were diagnosed and corrected: slot 0 is not a user
process, and PDP-10 AC0 cannot index memory. Neither affected
production source.

## Exact live state at the OOM decision

| Field (36-bit words) | Test OOM enabled | Production OOM off |
| --- | ---: | ---: |
| Real user processes resident | 12 | 12 |
| MM process extent count | 12 | 12 |
| Total live MM descriptors | 31 / 136 | 31 / 136 |
| Managed arena words | 242427 | 242441 |
| Managed MM allocated | 231012 | 231012 |
| Managed MM free | 11415 | 11429 |
| Largest free extent | 10377 | 10391 |
| Three gaps, measured LOW APR placement slack | 151 | 165 |
| Actual logical BRK/APR rounding loss | **5499** | **5499** |
| Shared filesystem workspace | 128 | 128 |
| Original foreign provider ownership | `7,,0` | `7,,0` |

All six extra processes had distinct logical breaks. The original
six process extents were 18432 words each; the six extra images
were 19456 words each, with respective logical breaks 18512,
18523, 18534, 18545, 18556 and 18567 words (944, 933, 922,
911, 900 and 889 wasted APR words). No resident MM extent was
unowned; all child TERM/WAIT and final process/MM accounting passed.
The actual measured memory occupancy is 95.3% of the managed arena.

## Evidence and limitations

`$TMPDIR/daimos-f07-real-mm-v1205r4.tar.gz`

SHA256:
`db2a1e1069fe6b008aa9ef85d4dc79cd953ce47a17a2b30eac9aad0547b44694`

The archive contains full raw SIMH, build, map, address, checksum,
disk-profile and per-policy snapshots, and independently verified
manifest/checksums. The original frozen DAIMOS and testkit commits
are recorded in the guest profiles.

This establishes real concurrent process/APR pressure at high
physical occupancy but **not** near-exhaustion of the descriptor
table: 31/136 used, versus 127/136 in the separate v1204 synthetic
MM-descriptor test. v1206 must combine both in one instant. Also
the configured 256K profile permits up to 64 process slots, not all
of which were populated during v1205. No high-concurrency stack
watermark was sampled. Consequently F07 remains OPEN; E13/F11 also
remain pre-enable gates and F12/F13 deferred. Mandatory OOM
checklist remains **59/64**, with production requester OOM OFF.
