# K16 v1993: exact directory dirty DROP recovery

Parent ca107f7e; testkit90ee89c. Staged experimental flag
DAIMOS_PBM_DIRTY_DROP_V1993 defaults OFF, requires directory idle
recovery. No default activation.

## Authority and state

The directory DROP encoder writes its presence masks and literals
inside the still-owned PURE prefix; the entire original writable
suffix is disjoint and read-only during this encode. For a bounded
write-pass failure in that contract, the original synchronous lease
may discard the damaged prefix by the existing zero-copy RAW rebase.
It may also cancel a verified candidate whose MM commit refuses.
Never publish the dirty resident text. General physical memory
corruption outside the codec store contract is not covered.

Capture the original four-word caller-stack lease before encoding.
Recheck exact PROCESS/u-area, original VM, executable backing and
credential, and the original once-pinned owner in the same PI window.
Reserve the original backing in the existing u-area backing word
before RAW publication; the process remains TRANSITION-held.
Existing word067 carries the exact original suffix sector, original
APR length /02000, five executable credential bits, and phase:
REBASE (original full PROCESS pin1), UNPIN (published mode2 exact
RAW suffix pin1), or HOLD (no deferred authority). RH17..16 set
and bit15 reserved, length14..7, credential6..2, phase1..0.
Neither valid FULL phases nor directory candidate lengths can use
the two-bit marker. No BRK borrowing or added per-slot metadata.

Before either mutation revoke retry authority. Rebase is metadata
only; reconcile lost replies by exact owner witnesses. Revalidate
process/u-area/VM/mode/backing/credential and HOLD after each MM call.
Publish mode2 and base0 only on exact rebased pin1. Retire only the
original suffix pin; exact unpinned RAW clears escrow/TRANSITION.
No READY, new pin, allocation, codec, loader or backing I/O during
rollback. An exact unchanged refusal alone reauthorizes its phase.
New synchronous failure arms the original shared retry clock;
slot-zero recovery never resets backoff. Recognized exact tag is
changed to HOLD before current metadata validation, retaining
original geometry/credential. Invalid or uncertain ownership is not
recovered by guessing; a stable process/u-area and agreeing backing
records remain the trust boundary. Malformed table/u-area refuses
without dereferencing escrow. Old RAW unlock must reject both kind
bits even if HOLD has no retry tag. Legacy RAW and mode3 FULL paths
retain their original routing. No controller loss/return redesign.

## Native qualification

Both PDP6/KA10 and GCC/KCC: 25 counted direct DROP calls plus actual
pressure/reclaim, original idle scan/deadline/armer, controlled outer
interrupt handoff. Test partial prefix write, healthy directory,
no-gain RAW, old RAW retry, placement refusal, rebase/unpin failure
and lost replies, wake to SRUN, recovery without backing mount,
exact bytes/owner/BRK/credential, no new pin, retry epoch/delay,
preserved unrelated scheduler cursor. Refuse changed pins, owner,
length, backing, provider/mount credential and even coherent changed
VM+MM geometry; repairing metadata cannot reactivate HOLD.
Post-call identity loss blocks further MM or publication.
Six executed unsafe mutants per compiler reject. A deliberately
inconsistent mode2 with a FULL SOURCE tag reproduced collision of
the initial single-bit spelling (native exit040); final marker
0600000 fixes it, both CPUs/compilers PASS.
Original v1990 KCC OFF regression (82 FULL/80 recovery/127 idle
calls, eight mutants) PASS. Existing GCC include-order and KCC
unused historical fixture warnings only; no new toolchain defect.

Evidence relative TMPDIR:
pure-bitmap-placement-v1985.kXbsn1 (GCC),
pure-bitmap-placement-v1985.KOjzbR (KCC),
pure-bitmap-placement-v1985.2qseo5 (v1990 OFF KCC regression),
pure-bitmap-placement-v1985.AqGoY4 (provisional tag collision).

## Linked package

GCC 5917->6377 resident words (+460), KCC 7055->7620 (+565),
BSS0. Both actual boot/kernel build systems select the staged
flag only with directory idle recovery; invalid/missing dependency
modes refuse. OFF/ON/OFF/ON repeatable package and map, fixed KCORE
byte-identical between switches. GCC OFF MRES and fixed KCORE
also byte-identical to the v1992 physical build. Initial package
matrix spanned the deliberate marker correction and failed
repeatability as expected; discarded that mixed-source run.
Fresh final matrix PASS, evidence relative TMPDIR
pure-directory-dirty-package-v1993.kfIv8O. No physical guest claim.

## Boundary

Native staged dirty DROP and original recovery integration accepted.
Physical dirty prefix write/rebase/unpin replay, ambiguous ownership
physical faults, irrecoverable source-loss/corruption disposition,
RT latency/stack, real DSH/48K and optional KA10 boot remain open.
K15/K16 stay open, default OFF. Controller loss/return discussion
deferred until K16 completion. TODO.MD deduplication follows closure.
