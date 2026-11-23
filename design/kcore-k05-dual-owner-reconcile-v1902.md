# K05 v1902: bounded slot-0 dual-owner FULL reconciliation

Date: 2026-11-23. **Scoped original 48K PDP-6 physical PASS**:
the original native PACKED `mm_free` genuinely refuses -5,
slot3 remains quarantined while its external source pin is held,
and the original idle service automatically removes the temporary
PROCESS extent and clears TRANSITION after the pin is released.
The scheduler then retries FULL with the original PACKED source.
This does not yet prove the final FULL commit, child EXIT or
parent WAIT/HALT. K05-K09 remain OPEN; PURE/mode3 default OFF.

## Original defect and production correction

The original `ppure_retryable_v1612` requires exactly ONE
authoritative unpinned MM owner. A failed FULL can leave two
same-slot owners: the original PACKED RAW suffix and a complete
provisional PROCESS target. The ordinary scheduler excludes
TRANSITION, so this failure previously could not be retried.
v1901 also proved that forcibly delivering a foreign TERM
would release a live private executive continuation unsafely.

The v1902 production patch adds a bounded exact-owner verifier
and scratch-target rollback in the optional PURE MRES. It
checks actual source and target MM extent types, owner slot,
geometry, nonoverlap, count/capacity and pin state, the
original PACKED storage mode, backing identity, VMbase0,
the existing saved u-area and absence of a FINALIZE marker.
The initial admission permits pins, but the PI-atomic MM free
requires both owners unpinned. Only the provisional PROCESS
target may be freed. After native `mm_free`, the MM table is
scanned again; even a lost successful return cannot cause a
double free or prematurely clear TRANSITION. A sole unpinned
PACKED source and unchanged process/backing context are
required before the original TRANSITION is cleared.

The existing permanent KCORE idle-finalizer backoff clock and
non-FINALIZE swap-retry bit in u-area word 067 are reused.
One new KCORE helper arms the existing retry clock for an
exact quarantined PURE owner; the original idle maintenance
loop recognizes this distinct non-FINALIZE marker and calls
the already installed PURE service trampoline with a tagged
AC1 request. The scheduler cursor, normal swap-in ABI and
foreign-event TRANSITION guard remain unchanged. No extra
per-process fields, MM registry or BSS are introduced.

Fixed KCORE DXR IMAGE `024147` -> `024165` octal, an
increase of **14 words**; BSS remains `000415` octal.
Optional PURE MRES DXR IMAGE `004131` -> `004615` octal.
KCORE SHA256:
`aa80ea2d98778298c13ae96de76c52815cfd40ad5486030e5056b436f85072f5`.
Optional PURE MRES SHA256:
`2dd7b7f7340d5b1b9542a97d0e8aca8478ac32afc3c555f0ae1ab38d0cd39a6b`.

## Actual 48K physical experiment

Original source-built 48K compressed TSFS, original five
RUN/PIPE/BRK children and independent resident parent, with
the original FULL code and MM implementation. The installed
PURE base is physically observed as `032435`. Original FULL
loader `036704`, source `mm_free(PACKED)` call `036012`,
return `036013`, original service return `033556`.

The simulator deposits only a TEST-ONLY pin into the real
source MM descriptor at `024721` immediately before original
`mm_free(PACKED)`. The original native allocator returns -5
(`777777777773`) with exact two-owner MM count18, and the
original slot3 stays TRANSITION, VMbase0. The resident parent
emits a real HB while the pin is still held. The controller
then releases only the same test pin. The original kernel
performs every subsequent MM operation and process transition.

The independently source-derived process table in the second
physical trace is `045516`. Original slot3 u-area `046635`
is preserved throughout. The source PACKED extent remains
authoritative at `111200..120000`. The observed sequence:

| Original physical stage | MM count | Slot3 metadata | Source meta |
|---|---:|---|---|
| Genuine MM source refusal | 18 | `046635600401` | `000014000003` |
| Parent HB with external pin | 18 | `046635600401` | `000014000003` |
| Parent HB after pin release | 17 | `046635400401` | `000004000003` |
| Next parent HB / new FULL attempt | 18 | `046635600401` | `000004000003` |

Thus the original idle reaper actually frees the provisional
PROCESS target, clears TRANSITION and permits a new original
FULL restore. The new attempt reacquires TRANSITION and its
temporary target. This trace does **not** establish that the
second FULL ultimately commits or that the child exits.

Original full process-table trace SHA256:
`3bb07f4c9a3429601dd63a3dce4832913e80fac0168fbf8a5d1fd786cb404ede`.
First exploratory original trace SHA256:
`a990a7ca815fce6b06d0749fba5b6112fe8093346caaacce9800f68c8150ea67`.
The native KCC PDP-6 test exercises source/target pins,
actual MM -5, lost-successful-free reply, foreign/duplicate
owner, malformed MM capacity, FINALIZE, backing mismatch,
missing retry tag, PI balance and saved context conservation.
The second full-system read-only audit rejects all **12**
independently corrupted physical refusal/pin/owner/TRANSITION/
u-area witnesses. The original production guest and compressed
TSFS media are source-built; no test-only kernel hooks.

### Independent exact native target-free and saved-context proof

A third original source-built 48K physical controller links the
actual original KCC MM target-free call at `033245` and native
return `033246`, then stops at the actual original C-store
transition-clear instruction `033275`. It observes native
`mm_free(PROCESS)` return **MM_OK=0**, exact MM18 -> MM17,
slot0 as current owner, original source PACKED retained, and
the unchanged original saved executive `CTX_PC` and FDCTL
through the rollback. The original resident parent continues
HB with real PCLK advancement. Its source-linked original
full trace SHA256:
`88e640e3eef12834a2fb6c97d3f0694280092844019c5372e35748a26fab25d5`.
An independent rebuild with identical production KCORE/PURE
SHA256 also passes this exact-PC controller, trace SHA256:
`4fa67d715017fc4e566781c624c3041cf706129a1097fa35216856e1ce5f19fb`.
SIX additional independent unsafe actual native target-free,
MM, TRANSITION, saved-PC and PCLK transcript controls all
reject. Thus v1902 has **18 executed physical negative
controls** across its two independent physical oracles.

## Remaining K05 work

The parent fixture intentionally loops in independent HB
rather than waiting for child EXIT. The physical proof ends
at the *next* FULL attempt. A separate full-system test must
show that this attempt commits RESIDENT, restores the original
saved executive continuation, and that all five original
children complete typed EXIT/WAIT and the parent reaches Z/HALT.
Also test permanently unavailable source pins and foreign MM
ownership through idle backoff. Never bypass the foreign TERM
TRANSITION guard or synthesize a FINALIZE marker. The scoped
v1902 recovery is complete; **K05 remains OPEN**.

Tests: `daimos-testkit/tests/validation/pure-k05-reconcile-v1902/`.
Permanent docs: `pdp10-doc/KERNEL/PURE-K05-RECONCILE-V1902.MD`.
