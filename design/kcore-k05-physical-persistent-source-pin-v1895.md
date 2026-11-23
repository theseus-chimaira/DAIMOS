# K05 v1895: persistent source pin after real PDP-6 MM free refusal

Date: 2026-11-23. Status: **scoped original 48K PDP-6 physical PASS**.
K05-K09 remain OPEN. RAW PURE/mode3 OFF by default. No production
source or resident RAM growth.

## The remaining persistent refusal gap after v1894

The original v1839 FULL restore uses the authoritative MM extent
table to decide whether a failed `mm_free(PACKED)` reply means the
source truly disappeared, an exact unpinned source survived (safe
provisional-target rollback), or the owner set is ambiguous and
must remain quarantined. v1893 physically demonstrated genuine
successful source deletion with a falsified -5 return; the sole
PROCESS successor was safely published. v1894 physically executed
real original fixed-KCORE `mm_free` under a one-time extra RAW
source pin, returned genuine `MM_ERR_BUSY=-5`, released the
external pin, then verified safe exact two-owner target rollback
and all five original child EXIT/WAIT/HALT completions.

v1895 asks the other question: what if the **external PACKED
source pin is never released**? With a physically pinned source,
the exact-zero-pin owner set cannot be established. Releasing the
temporary complete PROCESS or publishing it would guess the result
of an ambiguous transaction. The original kernel must instead
retain BOTH physical extents and prevent the child from executing,
without blocking unrelated already-resident user work.

## Exact source-linked original PDP-6 experiment

The original source-built 48K v1855 fixed KCORE and opt-in PURE MRES
are byte-identical to v1890-v1894. v1895 uses the accepted v1892
test-only user INIT: through five original child RUN/BRK/PIPE,
secondary DTC3 compressed executable backing, original writable
data checks, and `L` pressure marker it is unchanged. Afterward,
the parent remains in its ORIGINAL already-resident PROCESS APR
and prints repeated user `HB` through real sleep/syscall calls,
instead of waiting for a deliberately quarantined child. Child
source paths and ordinary kernel/driver code are unmodified.

Actual installed PURE base `032417` octal is derived from the
original physical KCORE reclaim JRST. The linked FULL function
and native original `mm_free` DOBJ relocations are checked:

* original bounded FULL loader at physical `036202` octal;
* original source PACKED `PUSHJ 17,mm_free` at `035310`;
* original real `mm_free` return consumer at `035311`;
* original PURE FULL service return at `033067`;
* authoritative physical slot3 source metadata at `024703`.

At pinned FULL, original slot3 owns exact PACKED RAW suffix
`111200..120000` pin1 and independent PROCESS full target
`120000..130000` pin1; MM has 18 descriptors. Original FULL
loads/reconstructs target and performs both successful original
unpins before the source free call. The actual unpinned source
meta at `024703` is `000004000003`. A private test-only simulator
deposit changes **only its physical source pin** to 1,
`000014000003`. The unchanged original fixed-KCORE `mm_free`
physically executes and returns genuine `MM_ERR_BUSY=-5` at
`035311` (AC1 `777777777773`), with source pin1 and target pin0.

**There is no second deposit releasing this external source pin.**
Original v1839 zero-pin owner reconciliation finds a still-pinned
PACKED source plus unpinned complete PROCESS target, cannot safely
prove either successful deletion or safe rollback, and returns
`MM_ERR_BUSY=-5` without releasing either owner, changing saved
backing/storage, or clearing original slot3 TRANSITION. Original
source survives and slot3 has VM base zero. The actual original
MM descriptor count remains 18, and all unrelated owners/pins,
original TSFS ROOT and process table pointer remain unchanged.

At original service return `033067`, and at TWO **new** genuine
user heartbeat stops thereafter, the full authoritative MM
extent table is EXACTLY identical to the original real source
free-refusal snapshot. Slot3 process descriptor at `044723`
is `046031600401`, TRANSITION set; VM word `044724` is
`010000000000`, physical VM base zero. The original TSFS ROOT
is `070101000000` octal. Parent slot1 is genuinely resident
at physical APR `130000..140000` pin0, with original scheduler
state SRUN and `proc_current_slot=1` at BOTH user HB stops.

Actual real PCLK clock measured (octal):

| Original checkpoint | Source pin | Target pin | MM descriptors | Current slot | PCLK |
|---|---:|---:|---:|---:|---:|
| Pinned FULL | 1 | 1 | 18 | 0 | -- |
| Before actual `mm_free` | 0 | 0 | 18 | 0 | -- |
| Actual `mm_free` -5 | 1 | 0 | 18 | 0 | -- |
| Original quarantined FULL service | 1 | 0 | 18 | 0 | `016640` |
| Actual new user HB 1 | 1 | 0 | 18 | 1 | `016640` |
| Actual new user HB 2 | 1 | 0 | 18 | 1 | `016740` |

The first real parent HB occurs within the same PCLK interval as
the service return; the **second** newly emitted HB has a genuinely
later fixed-KCORE PCLK tick. The first test-only verifier was
incorrect to demand a tick *before* the first HB. Corrected
only this verifier, keeping every original machine snapshot and
ownership check unchanged; the original physical trace then PASSed.

The private controller deliberately exits after the two new HBs.
It does NOT observe completion of other PACKED children, all-five
EXIT/WAIT, terminal process cleanup or guest HALT in this
permanently pinned fault configuration.

## Independent audit and exact production budget

Original normalized physical PDP-6 transcript SHA256:
`8f2bb94995d3522b076c845f5e784cb7f682b5cb565ae553b24b717342f62c92`.
Fixed original permanent KCORE DXR SHA256:
`33bf700218a73fefcb36ebb1f1d835c1817f3a291a642898ccb3cbd5d9039d8b`.
Optional original PURE MRES DXR SHA256:
`a0fb25c51cf1c57f4bc70ca1dc83d8b9ea39b1ac3ddb3bba7b9ffcea17e5b3fd`.

The independent physical owner auditor verifies actual original
mm_free PC and `-5` result, original unchanged MM owner extent
descriptors/source pin1/target pin0, slot3 TRANSITION/VM0,
parent PROCESS resident APR/current slot/state, PCLK progress
between two genuinely new user HBs and unchanged ROOT. TEN
separately executed deliberately unsafe physical transcript
mutations are rejected, including a forged MM return, vanished
source pin, changed target pin, missing TRANSITION, false APR,
wrong current user, stale original PCLK, false original FULL
success and missing second user HB.

Real physical evidence:
`$TMPDIR/daimos-k05-persistent-source-pin-v1895-real/`.
Testkit source/reproduction and executable source-linked oracle:
`tests/validation/pure-k05-persistent-source-pin-v1895/`.
Targets `test-pure-k05-persistent-source-pin-physical-v1895` and
`test-pure-k05-persistent-source-pin-audit-v1895`.
Permanent reference:
`pdp10-doc/KERNEL/PURE-K05-PERSISTENT-SOURCE-PIN-V1895.MD`.

## Remaining gates

v1895 closes **persistent genuinely refused source free with
fail-closed MM/process quarantine and autonomous already-resident
user progress**, within the measured postfailure heartbeat window.
It does NOT prove any eventual owner unpin, terminal cleanup of
quarantined processes, original all-five WAIT/HALT, independent
PACKED child re-restoration under permanent memory pressure,
unknown allocator/free/target release outcome, foreign owners,
other PI6 timings, 64K/KA10 or production K05-K09 acceptance.
RAW PURE and mode3 stay OFF by default, with ZERO additional
fixed KCORE or removable PURE MRES words.
