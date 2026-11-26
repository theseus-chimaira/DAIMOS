# K13 v1979: staged atomic directory bitmap DROP

Parent 9a7c16a2. K12 is the still-owned prefix lease. K13 adds exact
candidate verification and an MM commit before releasing original RAW.
No build target enables DAIMOS_PBM_COMMIT_V1979. The staged reclaim
branch is source-linked for validation; old v1667 interleaved restore
is forbidden with this flag. No live mode3 scheduler/terminal support.

## Commit

Reuse K12 private four-word identity and dirty word067 escrow. Share
lease validation between cancellation and commit. Exact original
resident PROCESS base/length/pin, u-area owner, slot, identity, backing
and mode are required. No extra persistent word or descriptor.
Use pbmd_verify_v1979, which invokes the existing canonical decode
walker in read-only comparison mode (bit4), comparing every decoded
word, including zeros, to original RAW. It verifies canonical masks,
nonzero literals and exact consumption. No temporary decoded buffer.

mm_pure_bitmap_commit_v1979 accepts base, slot, full words and packed
prefix,,encoded lengths. It proves APR alignment, nonzero bounded
sector-rounded prefix, strictly smaller encoded than RAW, sufficient
mask directory, complete valid MM table and unique exact original
once-pinned PROCESS. Numeric MODULE/KERNEL_DYNAMIC IDs are independent.
No write on refusal. On success two descriptor stores retain original
base, shrink length, set PACKED_PROCESS and consume the sole pin.
No allocator, separate unpin, physical copy or fallible mutation after
that leaf commit point. Caller holds global PI exclusion and process
TRANSITION continuously. Compare and proof must precede leaf call.

The process wrapper reconciles the result using the exact sole
unpinned PACKED_PROCESS at original base and encoded length, not rc.
Publish original backing in u-area, VM base0, mode3 in existing
2-bit cell (preserving other slots), then clear dirty escrow and
TRANSITION. Original RAW is no longer owned and is not read again.
A lost success reply can be accepted with this exact witness. False
success without witness, changed owner or unexpected pins quarantines.
If the candidate is not equal, or the MM commit refuses and the exact
original lease survives, cancel by normal RAW omission, not resident
unlock. Failed cancellation keeps dirty escrow and TRANSITION, even
if the RAW rebase has committed. No deferred dirty recovery yet.

The staged reclaim wrapper holds PI throughout preparation, exact
verification and commit/cancel. Only clean no-fit/no-gain/admission
refusal with exact original unpinned resident owner and zero escrow
can use ordinary RAW DROP. Hard write fault is not normal fallback.
A live RT lease currently refuses the staged path (including RAW
fallback); default RAW service is unchanged. Before live use, assess
interrupt latency, stack high-water and the RT policy. No asynchronous
lease continuation or borrowing released words.

## Measurements

Matched KCC -O PDP6 DOBJ counts: RAW DROP691 (OFF byte-identical to
v1978); staged prefix-only after shared lease refactor1140; full
candidate1281 (+590 vs RAW). Shared codec frontend318, walker138
(456 vs v1977 395, +61); independent MM commit89. Total incremental
TEXT1135 vs ordinary RAW DROP; service901 both selections. Zero new
DATA/BSS, descriptors or per-slot metadata. This is staged code, not
an installed MRES delta or live net-memory/latency measurement.

The synthetic full1024/prefix256/RAW768 fixture encodes176 words.
Compared with RAW omission, saves592 words; releases848 from full
resident allocation. Actual allocator reuses all848 and overwrites
them before the retained bitmap decodes to the exact original RAW.
Two such owners only cover current staged cost by49 words, before
restore/recovery overhead; real DSH sample acceptance remains open.

## Tests

Testkit d3759f8. Evidence relative to TMPDIR:

- pure-bitmap-commit-v1979.p8dyar: PDP6/KA10 each47 commit/reclaim
  calls, actual MM allocator/PI, codec, lease and production reclaim
  function extracted unchanged along with its legacy recovery gates.
  Controlled provider publication and faults. Candidate corruption
  (wrong literal, zero literal, mask, padding, count and same-popcount
  corruption); lost/false success, foreign owner, pin witness, failed
  RAW cancellation and lost replies. 17 direct invalid MM leaf cases
  preserve descriptor/payload. Real pressure with full arenas, RAW
  fallback for unmarked/dense/no-fit/refused commit, dirty hard fault
  quarantine. All released words reused before exact decode.
  Six executed unsafe variants rejected (octal11,2,2,20,20,20): omit
  comparison, trust reply, retain pin, ignore duplicate owner, omit
  capacity, omit pin check.
- pure-bitmap-prefix-v1978.7Mr3Np: previous72 calls each CPU, six
  unsafe mutants PASS after shared lease refactor.
- daimos-bitmap-directory-v1977.h0Ruw7: previous2760 cases each CPU,
  four unsafe mutants and injected hard write fault PASS.
- k13-v1979.SvAhCA: matched object counts; OFF DOBJ identical to
  prior v1978 OFF. No candidate build warnings.

## Remaining boundary

K14 must use new directory representation for bounded FULL restore,
preferably retaining original bitmap ownership until complete target
and commit are verified. No legacy interleaved decoder may consume
this stream. K15 owns actual scheduler and terminal lifecycle; K16
owns deferred dirty/uncertain recovery including lost owner or pin
provenance. Do not enable installed mode3 until full physical acceptance,
net RAM benefit and PI latency/stack costs are established. RAW PURE and
mode3 remain default OFF; fixed destroy refusal unchanged.
