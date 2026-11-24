# K05 v1905/v1906: failed FULL target owner escrow, foreign MM safety

Date: 2026-11-24. **Original PDP-6 defect reproduced (v1905),
production correction and scoped 48K physical acceptance PASS (v1906)**.

## Exact original defect, not an allocator workaround

During a failed RAW PURE FULL restore, original native source
`mm_free(PACKED)` may return -5 with two surviving extents:
the authoritative slot3 PACKED RAW `111200..120000` and a complete
provisional slot3 PROCESS target `120000..130000`. Since the
temporary target cannot be published, the original slot3 stays
TRANSITION-held with its private executive continuation.

The original v1902 recovery read only MM descriptors whose current
owner equals slot3. Its native fixture deliberately accepted a
sole slot3 RAW owner even if an otherwise scratch-like PROCESS
extent appeared under another owner. That is reasonable only
if the foreign extent is truly unrelated or was legitimately
reallocated after a completed original target free. It cannot
distinguish those circumstances from owner corruption that
retags the same still-live provisional PROCESS target.

The v1905 original-source 48K PDP-6 physical reproduction first
executes genuine native source free -5. One TEST-ONLY MM metadata
deposit at `024725` changes only target PROCESS owner3 -> owner2,
keeping exact base/length/type/pin unchanged. Original idle
recovery then enters the native transition-clear PC `033313`
with source RAW still present, target PHYSICALLY still present
under owner2, MM count18, but slot3 TRANSITION **already clear**,
VMbase0, retry marker retired and saved executive continuation
still alive. No native target free occurred. This is unsafe
publication and a lost-memory owner, not a mere delay.

Original unmodified fixed KCORE SHA256:
`a201a7fdcf481b436b06694e5dd55c9367036ebd9f64c630486f854b1a4f89be`.
Original optional PURE SHA256:
`55be5113d4d15fe5c4a44409dca22afd463c75f05f661b3f8f71451a057c577f`.
Exact v1905 original physical bug trace SHA256:
`660ca0198938bc52cb1a4496f00116d78e390e58905e94062b157c074db70ba1`.

## v1906 correction: retain exact physical target identity

The optional PURE service already computes the full target base
when it arms the v1902 slot-0 deferred retry. v1906 puts that
18-bit address in the right half of the **existing** u-area
word 067, retaining the existing high-half swap-retry bit. While
TRANSITION is held the nonresident scheduler cannot use the
right half as its ordinary swap retry epoch. No permanent KCORE,
new per-process storage or new MM owner registry is added.

Before the slot-0 callback frees the provisional PROCESS target,
it now requires the present same-slot extent at exactly the
escrowed base with correct geometry, type and pin state. After
any `mm_free` result (including lost successful return), its
existing exact RAW owner check is augmented with a read-only
whole-MM-table scan. ANY remaining extent that overlaps the
original target range, including another owner/type, blocks
TRANSITION clearance. It does not free or retag such a foreign
extent and retains all original backing, private stack and
FILE/FDCTL state. Only a proven absent target and sole original
unpinned RAW suffix allow the original retry tag to be retired
and the saved original process to resume normal FULL retry.

This closes the ambiguity identified in the physical v1905
trace without guessing that a foreign owner belongs to slot3.
The scanner safely favors quarantine if a genuinely unrelated
allocation has reused the exact former target range before the
deferred callback. Correctness is preferred to fabricated
progress and an illegal MM free.

## Source-matched native and physical proof

New v1906 KCC PDP-6 native fixture PASS: original normal target
free, actual pin refusals, lost target-free successful reply,
invalid/duplicate owner and capacity, FINALIZE and backing guards,
plus absent/incorrect target escrow, foreign-claimed exact base,
wrong own target and partial foreign overlap. The historical
v1612 KCC prefix/FULL service and unsafe READY mutant tests
also PASS on the changed source.

In the actual source-built 48K PDP-6 guest, compressed TSFS and
five original children execute the original source free -5 at
`036175`; original slot3 remains TRANSITION. The controller
then retags only the target at `024725` to owner2, without a
native MM free, while source remains authoritative. At three
independent parent user HBs, exact MM18 with RAW source pin0
and foreign-claimed PROCESS target pin0 is preserved; PCLK
`024625`, `024725`, `025025`, backoff `0100`, `0200`, `0200`.
Original slot3 TRANSITION/VMbase0 and saved private CTX_PC,
FDCTL and u-area are preserved; u[067] physically contains
`040000120000` on all three checkpoints, with original target
base `120000` retained as a read-only escrow.

The controller then explicitly REPAIRS only the corrupt
test-injected target owner at `024725`, owner2 -> owner3. This
is not automatic foreign-owner correction. The **original**
native PROCESS target `mm_free` at `033420` returns MM_OK=0,
MM18 -> MM17. Original transition clear at `033456` happens
only when the target is gone, and the original escrow clears.
The next real FULL source free at `036175` returns 0 and
publishes RESIDENT at `120000`. The original five child EXITs,
typed parent WAIT, Z and genuine PDP-6 HALT PC0 PASS, with
MM8 at Z, MM6 at HALT, no original slot3 MM owner and ROOT
unchanged. ALL 16 independently mutated physical transcript
controls reject, including the original premature unlock and
missing/wrong saved target identity.

Final v1906 KCORE SHA256:
`a201a7fdcf481b436b06694e5dd55c9367036ebd9f64c630486f854b1a4f89be`.
Final PURE SHA256:
`09f65d0ca272986b462c63580eae3c4006490004cd060529ce40f10a55949351`.
Accepted original physical trace SHA256:
`c22a45e4b89602b2edfbe2daa75c4294d7c4ffd828cf5094584e3f5d5832055a`.

Fixed KCORE remains IMAGE `024167` and BSS `000415` octal,
unchanged. Optional PURE IMAGE grows `004615` -> `004761`
octal (**100 decimal words**), a correctness cost only when
PURE MRES is enabled. No process-table/u-area allocation grows.
The whole-table scan costs bounded time for at most
`MM_MAX_EXTENTS` existing descriptors; future code-size reduction
may merge that scan with the existing owner scan only if it
retains all v1905/v1906 negative controls and physical proof.

Testkit:
`daimos-testkit/tests/validation/pure-k05-foreign-target-v1905/`
and `daimos-testkit/tests/validation/pure-k05-foreign-target-v1906/`.
Permanent documentation:
`pdp10-doc/KERNEL/PURE-K05-FOREIGN-TARGET-V1906.MD`.
Accepted physical evidence:
`$TMPDIR/daimos-k05-foreign-target-v1906-physical-v1/`.

**K05 remains open** for irrecoverably pinned/foreign MM owners,
terminal failure handling, allocator metadata uncertainty, and
other physical PI6 interleavings. This is fail-closed safety and
recovery after *explicitly corrected* owner metadata, not a
general mechanism for forcibly reclaiming foreign allocations.
K06-K09 separately remain open, RAW PURE/mode3 default OFF.
