# K12 v1978: exclusive original-prefix lease

Parent 0c2e6077. This is the staged prefix-only preparation/cancel
component, not installed mode3 reclaim. No extra allocation, scratch
extent, descriptor, persistent metadata, or new fixed KCORE word.

## Selected ownership policy

Use the existing sector-rounded reconstructible PURE prefix of the
original full APR extent, not the freed prefix of an existing RAW
owner. Encode the entire canonical v1977 stream at the original
APR base with capacity exactly the PURE skip. Only a strictly
shrinking stream fitting wholly within this prefix is accepted.
If not, preserve resident bytes and allow ordinary RAW omission.
Do not allocate an extra extent merely to compress more images.
No shared VFS/BCACHE scratch lease or dynamic scratch alternative is
introduced. A later alternative must independently prove its bounded
no-reclaim ownership, peak RAM and rollback benefit.

The caller holds TRANSITION, global PI exclusion, and the same
synchronous private executive continuation throughout prepare and
commit/cancel. No I/O, allocation, sleep or callback between them.
The target is noncurrent SLEEP/STOP, no active user executive map or
fatal close, marked PURE, no BACKSTORE, valid live immutable backing
and provider availability. No compression while any RT lease exists,
including another slot, because the staged operation holds global PI
exclusion. Reuse pram_prepare and pram_drop_available_v1926. Before
reading process physical memory, verify the entire MM table and exact
sole user PROCESS owner (base, full size, pin count), and the exact
separate unpinned KERNEL_DYNAMIC u-area (01000+slot,0410 words).
Reject stale slot bounds, other process owners, existing pins, malformed
ranges and wrong storage mode. This is not a new resident owner table.

## Clean versus dirty lifetime

Initially save the original RAW suffix identity in existing word067
using the old DROP kind; the resident text is still intact. Pin under
PI exclusion, prove zero->one ownership, not merely a successful reply.
Lost pin reply may be cleaned up using the original once-pin witness
in this same window. Require exact zero-pin original PROCESS proof
before clearing clean escrow; uncertainty retains TRANSITION.

Before the codec may store, mark both old DROP/PREFIX kind bits in
word067. This combined kind is deliberately refused by existing RAW
idle recovery. RH is zero until successful encode; then it contains
exact encoded length (not a provisional target address). The original
RAW suffix is disjoint from all candidate stores. Successful prepare
returns a four-word private stack lease: original VM word, backing,
u-area base, packed executable identity. Count is already in escrow.
No second copy of RAW or permanent per-slot word. Normal codec refusal
made no store; only then revert to clean DROP escrow, retire the proven
pin, clear escrow on exact unpinned witness. Hard write fault (-6)
retains dirty escrow with RH0 and no valid lease. Do not automatically
cancel or unlock it: a failed write-pass invariant is not a normal RAW
fallback.

After successful prepare the prefix contains compressed data instead of
executable text. Even though VM base and storage map remain RESIDENT,
the TRANSITION-held process must never be dispatched. The only current
cancellation is normal RAW omission via the existing rebase primitive.
Check the original stack identity, exact once-pinned full extent and
dirty escrow; do not discover a substitute owner or acquire a new pin.
Reconcile rebase reply against exact pinned PACKED_PROCESS suffix.
Only after rebase is proved, publish backing, VM base0 and RAW mode2.
Retire pin and require exact unpinned original suffix before clearing
word067 and TRANSITION. If rebase refuses, retain the dirty full pin;
if unpin refuses, retain committed RAW and dirty escrow under TRANSITION.
A lost success reply is accepted only with the exact MM witness. Do not
resume resident execution by unpinning the original full descriptor.
No deferred retry may consume a pin based solely on a count. The new
combined kind is rejected by old recovery, even when no pin remains.

## Measurements and validation

KCC -O PDP6 native DOBJ of v1977 RAW DROP (DROP_ONLY) 691 TEXT,0 DATA/BSS.
Current OFF build byte-identical. With DAIMOS_PBM_PREFIX_V1978=1:
1105 TEXT,0 DATA/BSS (+414); plus v1977 codec395 =809 staged incremental
instruction words, before K13 replacement/commit/restore integration.
No build target enables the new flag; installed RAW and mode3 unchanged.
The four-word caller-stack token is not the entire stack high-water;
additional transient C/ASM call frames remain to be measured.
No physical interrupt-latency, live memory payback or recovery claim.

Testkit ea0be1b. Evidence relative to TMPDIR:

- pure-bitmap-prefix-v1978.1f1RDa: native PDP6 and KA10 each72
  prepare/cancel calls, actual full MM allocator and machine PI gate,
  real rebase, storage-map and v1977 codec. Controlled mount/backing
  publications and explicit pre/post-operation fault wrappers, no
  physical I/O or live scheduler. Healthy SLEEP/STOP, 1024-word full
  extent,256-word prefix,768-word RAW,176-word canonical candidate.
  All RAW, original u-area/owner identities and adjacent foreign words
  verified. Pinned free refuses. Force no-reclaim allocator to reuse
  exactly released prefix and overwrite it; RAW preserved. Dense and
  prefix-capacity refusals execute unchanged real RAW fallback. 27
  admission/owner refusals, lost pin/rebase/unpin success replies,
  held/foreign pin and dirty write error, stale token, failed rebase and
  post-publication unpin. Actual private old RAW recovery/unlock
  predicates extracted unchanged reject dirty escrow even without pin.
  Six executed mutants fail (octal10,10,12,1,25,21): opt-in, u-area
  owner, pin witness, dirty tag, unsafe cancel unpin, stale backing.
- k12-prefix-v1978.8urOE4: matched compiled prior/off/on DOBJ and
  object sizes; prior/off cmp identical, no candidate-source warnings.
- pure-drop-availability-v1926.mmgYAo: prior provider availability and
  six executed unsafe mutants PASS.
- pure-mm-recover-v1919.RbfGzU: prior deferred DROP/PREFIX recovery
  and five executed unsafe mutants PASS. Historical fixture KCC notes
  (long identifiers/unused definitions/undefined disabled flag) remain.

## K13-K16 boundary

Next verify the exact directory candidate against original RAW without
another copy, then commit a single exact MM descriptor to compressed
length at original base and publish mode3 under lifecycle exclusion.
Do not integrate older interleaved bitmap producer/verifier/restore
piecemeal. The existing RAW cancel is the normal alternative after
successful prepare; ordinary no-fit never dirties the prefix. Uncertain
commit, hard write fault, deferred dirty escrow, terminal/restore
ownership and PI6 lifecycle remain unimplemented in this staged path.
Before live activation measure global PI hold and stack high-water;
shorten the exclusion if safely provable, with pin/transition and
revalidation across interrupts. No RT or device-latency guarantee yet.
Keep mode3 fixed destroy refusal and default OFF until full acceptance.
