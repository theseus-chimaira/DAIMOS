# K05 v1912: lawful late owner reconciliation and last-user native HALT

Date: 2026-11-24. **Original PDP-6 48K physical PASS (scoped).**

## Question and result

v1911 proved that an unrepaired foreign-claimed FULL child is the
only remaining live user while native slot0 idle retries continue
at the shared `01000` octal backoff cap and true PI6 executes.
v1912 asks whether the original kernel can then complete
terminal shutdown **if authoritative ownership is restored**,
without forced finalization or bypassing the original executive
continuation. The answer is YES for this source-linked fixture.

All original KCORE, PURE and test-only v1910 user DXRs are reused
unchanged. The full v1911 original source-free -5, foreign owner2
claim, refused parent TERM, parent EXIT, siblings 4/5/6 EXIT,
MM10 single orphan, three clock-separated retry loops and real
PI6 sequence is first physically executed. The entire pre-repair
trace passes the independent accepted v1911 positive verifier.

## Correct active MM descriptor after peer EXIT

Original MM extent array compacts when the healthy owners EXIT.
The original foreign claim is injected at source-linked metadata
word `024725` (owner3 -> foreign owner2). **That address is no
longer the active target metadata after MM10 compaction.** In
the accepted MM10 physical table, the unique full PROCESS
`120000..130000` owner2 resides at active word `024707`.
The v1912 controller derives that live word by exact MM owner,
type, geometry, pin and row index from independent accepted
v1911 evidence; the new physical trace verifies owner2 before
and owner3 after the one test-only repair. It never touches
the original kernel return AC1 or any other MM descriptor.

An early v1912/v2 experimental controller had incorrectly
reused old `024725` for the late repair, targeting a different
inactive/stale owner4 descriptor. The invalid run was killed,
excluded from acceptance, and root cause fixed in the test
controller. No production-code defect was inferred from this.

## Genuine source-linked recovery and native terminal result

- Original `proc_pure_mres_service_v1612` calls native
  `mm_free(PROCESS)` at physical PC `033420`; return `033421`
  is **AC1=0**. The exact target `120000..130000` disappears,
  reducing MM extent count **10 -> 9** with RAW source retained.
- Original transition-unlock instruction at `033456` executes
  only after native target release; uarea word067 escrow
  `040000120000` becomes zero. The original private executive
  continuation and surviving RAW still belong to slot3.
- A second FULL allocates original PROCESS `130000..140000`
  owner3. Original native PACKED-source `mm_free` returns
  **AC1=0** at `036175`; RAW `111200..120000` disappears.
- Original completed FULL service at `033737` returns **AC1=0**.
  Slot3 `047017400001` is RESIDENT with VMbase `130000`,
  no TRANSITION and cleared target escrow. MM9, ROOT intact.
- The genuine user prints `w` after reading its pipe and then
  enters original `proc_exit_current=013655` as slot3.
  Original slot0 finalizer retires the last child; real
  PDP-6 simulator reports **HALT PC 000000**. Final MM6,
  `proc_high_slot=1`, current slot0, original slot3 FREE and
  no PROCESS/PACKED extents owned by slot3. This is an actual
  native HALT instruction, not operator exit.

The strict source-linked positive physical verifier checks
MM/ROOT at all stages, unique original target, original AC1
returns, safe unlock after target free, real second FULL,
child context and final normal HALT. FOUR independent forged
physical mutations reject: wrong late owner repair, forged
native target-free result, premature TRANSITION clearance and
false terminal MM count. The original v1911 prefix is also
reverified using its accepted physical oracle.

Accepted physical normalized SHA256:
`d4e4c884b00eb61f455587e60cd8d19033dfbcb220160bf21f4584a51045de96`.
Source-linked physical controller SHA256:
`0ab39813f75552eb623171899ca56b64dbe10d979ce12d40d407da01fec1b26e`.
Byte-identical fixed KCORE SHA256:
`a201a7fdcf481b436b06694e5dd55c9367036ebd9f64c630486f854b1a4f89be`.
Byte-identical optional PURE SHA256:
`09f65d0ca272986b462c63580eae3c4006490004cd060529ce40f10a55949351`.

Testkit: `daimos-testkit/tests/validation/pure-k05-terminal-reconcile-v1912/`.
Permanent docs: `pdp10-doc/KERNEL/PURE-K05-TERMINAL-RECONCILE-V1912.MD`.
Evidence: `$TMPDIR/daimos-k05-terminal-reconcile-v1912-physical-v3/`.

**Scope boundary:** the operator corrected an intentionally
corrupted MM owner. This establishes safe *eventual terminal
completion after trustworthy reconciliation*, NOT automatic
repair, recovery with permanent foreign ownership, or authority
to discard a still-live private executive stack. K05 remains
OPEN for irreversible owner ambiguity, other MM/PI6 timing,
and broader platform validation. K06-K09 OPEN. RAW PURE and
mode3 default OFF. No production ASM/C, fixed RAM, IMAGE or
BSS changes.
