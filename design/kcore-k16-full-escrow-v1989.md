# K16 v1989: exact directory FULL escrow and bounded rollback

Parent b5a5552d. Native accepted stage only, explicitly compiled with
DAIMOS_PBM_FULL_ESCROW_V1989 alongside directory FULL. Not selected by
the experimental installed package yet. No scheduler/reaper change.

## Geometry and encoding

Directory FULL must remember the exact original encoded length; it is
not derivable from VM words or omitted immutable prefix. Never use a
later MM descriptor length as authority. RUN, EXEC and boot initialize
BRK floor/current to the aligned complete VM. BRK preserves the floor
and rounds only the APR allocation; the current break need not align.
Before borrowing floor bits, check positive 02000-aligned floor,
floor <= current <= VM words, current > words-02000, aligned nonzero
VM length within the 18-bit range.

Existing word067 LH retains original 0200-aligned source sector,
both kind bits and retry tag. RH uses 15 bits for exact encoded words
(maximum040000 from the 14-bit text field plus origin, rounded down
to0200) and three bits for phase. BRK word0407 LH low10 bits:
bit9 version, bit8 reserved poison, low8 destination APR index
(base>>10). Preserve high floor bits and every current-BRK bit.
Invalid allocation address cannot become an absent target; poison the
field and retain HOLD. Ordinary untagged heap and old FULL formats
cannot authorize new rollback. Terminal tags remain disjoint. Both
kind bits still reject original RAW recovery. All edits PI excluded.
Healthy synchronous commit and clean rollback restore the exact
ordinary BRK before clearing escrow and releasing TRANSITION.

## Recovery authority

1 HOLD: no deferred authority. Initial preparation, allocation,
  loading, decode/verify and atomic finish remain HOLD.
2 PAIR: exact original bitmap pin1 and full target pin1; may attempt
  target unpin.
3 FREE: same original bitmap pin1 and full target pin0; may attempt
  target free.
4 SOURCE: target absent (never allocated or exact retirement witnessed)
  and original bitmap pin1; may attempt source unpin.

Only enter the shared rollback helper after a synchronous FULL failure
and exact identity/owner proof, or reconstruct a previously authorized
phase from the saved geometry. Revoke phase to HOLD before any MM
operation. Recheck full stable identity and exact owner set after
each mutation. On unchanged exact refusal, rearm the corresponding
phase; on proven completion advance. A false or lost reply alone
never grants authority. Drop target address only after exact interval
absence proof. Original source pin can then be removed, exact source
pin0 witnessed, BRK restored and escrow cleared. Synchronous rollback
retains TRANSITION for clean unlock; explicit slot-zero recovery clears
TRANSITION and preserves inactive mode3, never READY or resident APR.
No I/O, allocation, decoding or mount dependency in deferred recovery.
No source loss, unknown pin, ambiguous finish or hard codec fault is
authorized for retry. Malformed preflight leaves metadata untouched;
valid preflight followed by ambiguous ownership revokes authority.

Trust boundary: existing stable u-area, coherent backing/credential
records, exact MM descriptor and transaction-owned pin continuity.
Not an allocation generation, content authentication or protection
against coordinated metadata corruption. Do not recover HOLD even
if later descriptors look plausible. A returning executive context
remains excluded until clean rollback; original foreign terminal guard
is unchanged. This stage does not attempt deferred FULL publication.

## Native results

Testkit96addb4. Actual source-extracted encode/phase/clear/heap helpers
host oracle:16,777,216 arithmetic round trips (all16384 lengths,
256 target indices,4 phases; cycling legal heap/source values).
Invalid heap/address rejected. This is bit-field coverage, not a
claim that every possible interval combination physically fits.

GCC/KCC on PDP6/KA10 each82 FULL attempts and80 explicit recovery
calls. Production moved bitmap from original DROP, real codec/loader/
MM/PI; second controlled grown VM04000, floor02000/current02001,
independently encoded by actual codec, not actual BRK syscall.
Normal/lost finish success, clean pin/read/no-fit failures, lost
alloc/cleanup replies. Repeat target unpin/free/source unpin refusal
including false success, exact rollback with backing unavailable,
restore retry; actual allocator reuse after verified target retirement.
Invalid allocation addresses poisoned; unknown pins, false finish,
hard codec fault and identity change remain HOLD. Original source
length/address/owner and target span/pin/owner change, loss and
foreign overlap in all three retry phases reject. No unknown MM
mutation or deferred FULL publication. Nine executed source mutants
per compiler reject. Initial no-post-identity mutant escaped fixture
because it did not check subsequent free; strengthened exact
two-owner/zero-free post-unpin oracle, final mutant rejects036.
Initial test runner generation anchor ambiguous, fixed; no new
production compiler/assembler defect.

Evidence relative TMPDIR:
pure-bitmap-placement-v1985.EUrnBZ KCC,
pure-bitmap-placement-v1985.UVLe58 GCC,
pure-full-escrow-regression-v1989.BGuVtC historical v1988 PASS,
pure-bitmap-placement-v1985.40pXAR original v1985 PASS,
pure-full-escrow-size-v1989.5i7fcn source-built DOBJ accounting.
GCC directory FULL1801->2178 (+377), KCC2240->2698 (+458).
DATA/BSS0, no fixed/per-slot permanent words; one additional private
continuation identity word. Stage OFF directory and RAW-only
objects byte-identical to b5a5552d both compilers. This is optional
leaf TEXT, not measured linked package or stack high-water.

## Remaining K16

Connect only authorized phases to actual slot-zero service command,
arm original shared retry clock without marking terminal or RAW
recovery. Reuse existing backoff; prove no repeated dispatch, no
stale cursor/READY, no accidental HOLD recovery or terminal teardown.
Source-built mode-switch and actual physical FULL failure tests.
Separate dirty DROP recovery and irrecoverable owner/corrupt source
policy, RT latency/stack, real DSH/48K/payback remain open.
No default/profile activation in v1989.
