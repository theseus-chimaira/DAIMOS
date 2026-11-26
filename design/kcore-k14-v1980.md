# K14 v1980: staged bounded directory FULL restore

Parent 6d053097. No installed build target enables
DAIMOS_PBM_RESTORE_V1980. No old interleaved bitmap restore is used.
The existing v1391 RAW FULL implementation compiles byte-identically
with the staged flag absent. New code shares its bounded DXR loader,
pram_prepare, v1917 two-owner proof and existing v1664 atomic finish.

## Selected restoration strategy

Use a separate full aligned PROCESS destination, preserving the
original mode3 bitmap until after complete restoration. No second
RAW-sized scratch, VFS buffer lease or extra persistent metadata.
This requires a free contiguous 02000-octal aligned APR extent and
one temporary descriptor. A future in-place expansion would need a
separate source-survival and backing-failure rollback proof; no such
claim is made here. Retaining the source across failed immutable I/O
is more important than destroying it to force a restore.

Entry requires slot zero, TRANSITION held, valid live slot, exact
unpinned stable u-area owner, mode3, no active map/fatal close, saved
backing and marked executable identity, swapless mounted backing.
Check the entire MM table before reading physical u-area or source.
Discover original sole unpinned PACKED_PROCESS only once; K13's
bitmap base must retain APR alignment, its length fit inside derived
PURE skip and shrink logical RAW while containing all directory masks.
Retain exact base/length and private six-word identity (backing,
full length, u-area, executable identity, escrow and mode-map cell).
No new per-slot state. Word067 stores original aligned bitmap base
and original target address with both old kind bits set; mode3
identifies FULL versus dirty mode0 DROP. Old RAW recovery must refuse.

With PI excluded prove source pin zero->one; reserve full APR using
no-reclaim allocation, escrow its returned identity, prove disjoint
original source and provisional PROCESS, pin target and prove both.
Lost pin reply is acceptable only after exact zero->one witness in
this same uninterrupted window. Neither payload I/O nor physical
write precedes exact target pin and identity proof. Restore previous
PI state, zero omitted PURE prefix and invoke the existing
exec_image_full_bounded_v1391(backing,base,uarea,allocated_words).
The loader enforces original immutable backing, capacity and exact
plain or compressed DXR payload. Recheck identity, exact owners and
backing admission after I/O. Decode the directory into target+skip
with read-only preflight and write pass; compare every decoded word
against retained source with pbmd_verify_v1979. These two codec
operations run with normal PI behavior; both spans are protected by
pins and lifecycle exclusion. No I/O/callback during codec passes.
Canonical shape and equality do not authenticate a valid-shaped
altered source; there is no checksum or persistent integrity metadata.

## Commit and rollback

Recheck exact original process/escrow and owner pair with PI excluded.
Existing mm_pure_bitmap_finish_v1664 validates the two once-pinned
owners, deletes bitmap and consumes target pin atomically. The shared
v1917 proof checks complete table/arena/foreign range geometry before
and after the leaf; no independent raw/bitmap owner table. Reconcile
reply against exact sole unpinned APR with original bitmap absent,
and recheck process identity. Only then publish VM base, clear saved
backing, clear mode3 bits (preserve other slots), clear escrow and
TRANSITION. Lost successful finish reply cannot revoke publication.
False success or changed witnesses quarantine instead of freeing an
uncertain owner or publishing a corrupt runnable process.

On ordinary I/O, decode, allocation or commit refusal, prove original
identity and exact owner pair before cleanup. Unpin target if held,
prove exact unpinned target and pinned source, free target, prove
its absence including absence of a foreign overlapping extent.
Then clear target from escrow and retire original source pin. Verify
sole unpinned original bitmap before clearing escrow. Keep TRANSITION
held on all refusals; live retry/terminal selection is K15/K16. A
known unavailable backing does not prevent release of an exact failed
target. Lost cleanup replies reconcile by exact witnesses. Hard codec
write fault, ambiguous pin/owner/identity or refused cleanup retains
escrow/TRANSITION. No deferred dirty pin provenance is assumed.

## RAM and evidence

Matched KCC -O PDP6 DOBJ: previous/off RAW FULL1046 TEXT, byte-identical;
on1785 (+739). Existing v1664 finish133 TEXT, total incremental K14
872, K11-K14 combined staged TEXT2007. All zero DATA/BSS. Private
identity is six caller-stack words, not total stack high-water.
Synthetic full1024/prefix256/RAW768/bitmap176: disjoint restore image
peak1200 words versus RAW FULL1792; one temporary extra descriptor.
No fixed KCORE or installed MRES delta. Actual stack high-water, MM
PI hold, RT/device latency and real compression payback still open.

Testkit 333f07f; evidence relative to TMPDIR:

- pure-bitmap-full-v1980.2vmz4f: PDP6/KA10 each52 calls, five successful
  restore variants per CPU, actual v1979 DROP, real MM and PI, bounded
  DXR loader/backing unpacker, real v1458 fixed low-core decoder copied
  to 062, directory decode and compare. Controlled synthetic VFS
  metadata/payload, no mounted physical filesystem. Plain and compressed
  marked DXR2; initialized file data crosses PURE boundary and mutable
  overlay replaces it. Overwrite original released suffix before
  restoration; prefill target nonzero; verify every APR word including
  zero words, adjacent guards, unrelated owner/mode, exact pins.
  Force real no-reclaim allocator to reuse original freed bitmap and
  overwrite it without changing restored APR. Actual no-fit and
  descriptor exhaustion; partial I/O, oversize header before payload,
  changed mount, malformed executable or bitmap stream; stale identity,
  changed/extra pins and foreign owners, lost/false pin/alloc/finish
  replies, target/source cleanup failures and hard codec write fault.
  Loader/decode/compare verify PI enabled, MM mutations PI excluded.
  Six executed mutants fail (octal3,3,3,10,10,21): no comparison,
  trust finish reply, no pin proof, no post-I/O identity/owner proof,
  ignore target-free refusal. Early fixture corrections: truncated
  bitmap witness expected original length, and a one-word neighboring
  descriptor move into a free gap was not a malformed table. Corrected
  fixtures; no production bug demonstrated by those early failures.
- k14-v1980.7LDnyb: matched prior/off/on DOBJ and finish counts;
  OFF byte-identical, new source compiles without warnings.
- pure-full-free-reply-v1839.kmDJoW: RAW source-free lost reply and
  foreign refusal, two executed unsafe mutants PASS.
- pure-k05-pinned-commit-v1855.D7khZr: RAW pin and allocator pressure
  acceptance, two executed unsafe mutants PASS.
- pure-mm-recover-v1919.3bXTLk: prior RAW deferred DROP/PREFIX recovery
  and five unsafe mutants PASS. Historical fixture KCC undefined
  disabled-flag and unused/long-identifier notes remain.

K15 integrates directory dispatch and terminal accounting; K16 handles
uncertain deferred recovery, physical and scheduler acceptance, latency,
stack and net RAM. No installed mode3 support, no production boot claim.
Default RAW PURE/mode3 OFF, permanent mode3 destroy refusal retained.
