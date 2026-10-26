# Proposal B v412: implementation decision gate and migration checklist

Status: design complete enough for a decision; NO production modification authorized.

## Decision and explicit non-approval

The proposed six-bit packed descriptor from v408 uses a two-word FILE
record: direct restricted canonical vnode plus metadata, and an intact
independent 36-bit offset. A full six-bit mount ID can coexist with
three-bit provider and local-kind fields and six metadata bits. It
cannot preserve the historical four-bit per-OPEN lock-family token.

Two decisions therefore remain explicit gates:

1. Is changing FLOCK from separate OPEN families to process-local,
   vnode-wide state acceptable? This changes existing tested behavior.
   The present code does not implement cross-process lock exclusion;
   do not misrepresent the replacement as a global file lock.
2. May FILE reject providers/local kinds above 7, despite the generic
   VFS six-bit fields? Every vnode producer and consumer must enforce
   the boundary if adopted; module compatibility needs separate review.

Until both are decided, the production candidate is NO-GO. Merely
continuing design notes does not count as authorization.

## Concrete bit allocation (36-bit word zero)

Bits 0..17 index, 18..20 local kind, 21..22 regular/lock state,
23 reserved, 24..29 mount ID, 30..32 provider, 33 DIR/APPEND,
34 WRITE, 35 READ. Vnode mask 077707777777 and metadata mask
700070000000 (octal). Descriptor word one is full 36-bit offset.
Bit 23 remains reserved; packed metadata excludes a family token.

## Implementable migration sequence after approval

1. Add versioned regression cases to daimos-testkit first: each
   descriptor mode; full offset; distinct identical local vnodes on
   mounts 0,8,16,24,26,63; PIPE vs TSFS; DUP/DUP2 and RUN mapping,
   independent offsets; OPEN inheritance and revised FLOCK; forced
   exit/unmount; failed CLOSE, invalid RUN map and PIPE counts.
2. Define shared assembly/C masks and range validation; migrate all
   creators (boot standard I/O, OPEN, FIFO substitution, PIPE devices),
   including non-assembly C fallbacks, as one consistent layout.
3. Replace reconstruction and extension-map callers in FILE runtime,
   syscall fast paths, RUN, PIPE reference helpers and mount/unmount
   operations. Preserve the publication ordering: pipe refs increase
   once only AFTER all fallible RUN descriptor-map work, and existing
   pre-acquisition discard never attempts to decrement unowned refs.
4. Implement vnode-wide process-local OPEN/FLOCK transitions with a
   coherent invariant and no additional dynamic objects. DUP2 retains
   failure atomicity. Child lock state is independent after RUN.
5. Measure actual linked PDP-6/PDP-10 KCORE and KCORE+MRES,
   PERMANENT_LAST, process u-area allocation, MM peak words, and
   representative hot READ/WRITE vs slower OPEN scans. Compare all
   sizes with the same baseline flags and folding stage.
6. Reject/revert any regression in correctness or total physical RAM;
   only accepted changes may be committed/pushed. Record rejected
   experiments. Strict 32K acceptance and JRST folding stay deferred.

## Cost and evidence ledger

The current extension map resides in u-area offset 067. Retiring it
does NOT alone reduce the 0410-octal allocated u-area; physically
reclaimed process words are presently ZERO unless the layout and
allocation size also shrink safely. Potential resident-code savings
include mount extension store/copy/update, vnode extraction,
mount-aware FLOCK comparisons, RUN field copying and PIPE child
table checks. Costs include revised FLOCK/OPEN scans, metadata
validation and migration code. The net word count remains UNKNOWN.

Existing v388 full-link measurements only: KCORE 9109 words,
KCORE+MRES 15657, PERMANENT_LAST 15704. No fresh link, benchmark
or acceptance test was performed for this decision document.

## Final recommendation

Do not convert the descriptor yet. Obtain the two semantic approvals,
then execute one coherent prototype with tests and physical-RAM
measurements; revert if it fails. If either approval is denied, retain
the current packed layout and pursue non-semantic VFS/provider savings.
