# K05 v1904: original PDP-6 target pin through idle retry and FULL commit

Date: 2026-11-24. **Scoped original 48K PDP-6 physical PASS.** The
v1902 autonomous rollback and v1903 complete FULL/EXIT/WAIT/HALT
remain intact when the temporary PROCESS target of a failed FULL
is held externally pinned through three real parent heartbeats.
There are no production source, fixed KCORE, BSS, process-table,
or optional PURE MRES changes. RAW PURE and mode3 remain OFF.

## Original fault and exact owner identity

The original secondary-backed five-child TSFS/BRK/PIPE workload
reaches a genuine PACKED RAW source `mm_free` refusal (-5) with two
full slot3 owners and TRANSITION held, as established in v1903.
The test-only controller releases the original temporary PACKED
source pin and *before advancing the machine* adds one external pin
to the remaining slot3 complete PROCESS target. Physical native
MM metadata entries, two words each:

- PACKED source at `024723`, owner3 type4, base `111200..120000`;
- PROCESS target at `024725`, owner3 type1, base `120000..130000`;
- independent parent at `024727`, owner1 type1, base `130000..140000`.

The original target metadata changes **only**
`000001000003` -> `000011000003` (pin0 to pin1).
An exploratory controller mistakenly selected `024727` by adding
four words rather than two; both those runs are REJECTED. A later
attempt to identify a source-linked folded service-return PC was
also not accepted as native-refusal proof. The underlying correct
test idea was retained and its physical owner identity fixed.

## Measured physical checkpoints

At THREE independently executed user-mode parent heartbeat stages,
the MM extent table remains at 18 with exactly the original RAW
source pin0 and PROCESS target pin1. Slot3 stays TRANSITION-held,
VMbase0 and PACKED; saved executive CTX_PC/FDCTL, backing, root
and unrelated parent/foreign extents are unchanged. Parent slot1
remains runnable, with real PCLK `025256`, `025356`, `025456` octal.

The original single shared post-VM retry word at `024532` has its
pending flag set and bounded power-of-two intervals `0100`, `0200`,
`0200` octal. The original slot0 recovery does **not** invoke the
native destructive target-free while the complete target is pinned.
This is a physical MM/backoff invariant, not a direct observation of
a particular C return instruction during the pinned interval.

When the external target pin is released, the actual native
`mm_free(PROCESS)` call and return are observed with `MM_OK=0` after
`000060` octal additional PCLK ticks. Only the provisional target
is removed, original sole RAW source ownership is verified and
TRANSITION clears. The original subsequent FULL again frees the
PACKED source successfully, publishes RESIDENT VMbase `120000`,
and restores all five child continuations. Typed EXIT/WAIT, parent
`Z` and real hardware HALT PC0 PASS; MM count 8 at Z and 6 at
HALT, no remaining slot3 PROCESS/PACKED extent, ROOT unchanged.

The complete accepted original physical transcript SHA256 is
`f912b6cae1c470a876a404cd2e49af708b727f9bb423fbc2eaefe1d0777f16de`.
Identical accepted v1903 KCORE/PURE images:

- fixed KCORE `a201a7fdcf481b436b06694e5dd55c9367036ebd9f64c630486f854b1a4f89be`
- optional PURE `55be5113d4d15fe5c4a44409dca22afd463c75f05f661b3f8f71451a057c577f`
- KCORE DXR IMAGE `024167`, BSS `000415`; PURE IMAGE `004615`

The full original v1903 strict physical validator re-PASSes on the
v1904 trace. **12 independently executed unsafe real transcript
controls** all reject, including the actual preceding wrong-parent
owner bug, target/raw pin corruption, early TRANSITION/RESIDENT
publication, changed saved CTX_PC, dropped retry epoch, stale PCLK,
early parent gate and false physical target release.

Testkit: `daimos-testkit/tests/validation/pure-k05-target-pin-idle-v1904/`.
Permanent docs: `pdp10-doc/KERNEL/PURE-K05-TARGET-PIN-IDLE-V1904.MD`.
Accepted local evidence:
`$TMPDIR/daimos-k05-target-pin-v1904-physical-v4/`.

**K05 remains OPEN**: this tests prolonged, *eventually released*
external PROCESS pin, not an indefinitely pinned or foreign-owned
MM descriptor, ambiguous MM/allocator return, unsafe terminal
teardown or all PI6 interleavings. It does not close K06-K09.
