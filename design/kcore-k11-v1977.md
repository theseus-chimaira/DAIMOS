# K11 v1977: bounded bidirectional bitmap directory

Parent 27f69d33 (K10). No mode3 activation or installed link change.
This closes the source-linked codec leaf, not ownership or lifecycle.

## Representation and traversal

For N positive mutable RAW words, G=ceil(N/36), L=nonzero count.
Canonical stream is G masks followed by L nonzero literals in original
RAW order. Each mask uses bit35 for the first RAW word of its group;
unused low bits of the final partial group must be zero. Every literal
must be nonzero, and total consumption must be exact. Size G+L is
identical to v1451; no new marker, footer, group-index array or scratch
word in the stream. It must shrink strictly. There is no persistent
mode3 disk format. Directory format is NOT interleaved v1451: staged
v1662/v1663/v1664 producer, verifier and restore must change together
before mode3 can be installed. Keep v1975/v1451 regressions intact.

The encoder first counts literals read-only. Plan then walks the exact
prospective stores in either direction. A literal is loaded before its
store; a group's mask is accumulated in an AC and stored only after
all group RAW words have been read. For forward RAW cursor r, unread
interval is [r,end); for reverse it is [base,r]. A store may only hit
outside unread input. Decoder consumes the group mask and each needed
literal before output, checking two independently shrinking unread
intervals (mask directory and literal suffix). It continues canonical
validation after detecting overlap, so malformed data is invalid, not
merely a geometry refusal. Public bounds reject AC0..17 aliases,
invalid spans/wrap, zero counts, nonshrinking streams, insufficient
capacity, invalid direction and null count. The caller must guarantee
exclusive stable physical spans and an independent count cell. The
same ASM schedule is replayed with writes enabled only after preflight.

Normal refusal: no store to either span, unchanged count. Unexpected
write-pass failure returns PBMD_WRITE_FAULT_V1977 (-6); the spans and
count may have changed. This is a hard invariant failure for future
quarantine/recovery, not safe RAW fallback. It must not be interpreted
as a successful transformation or publish mode3. The exact leaf
schedule does not prove a nonfailing ownership commit. No scratch array,
permanent BSS, or extra per-process words. ASM walker saves AC10..16 and
uses two local words (nine-word frame, one transient guard call).

The measured C prototypes were 625 TEXT (separate traversal), 621 with
cursor step, 521 shared traversal, 518 bounded signed cursors. The native
ASM walker and C frontend originally 389 TEXT; explicit hard error
propagation makes final frontend262 + walker133 =395 TEXT, zero DATA/BSS.
This is +79 versus historical v1975 forward wrapper286 + codec30, not
an installed MRES delta. Replace old checks instead of accumulating
both codec implementations when integrating. No standalone speed
benchmark, no live net RAM claim.

## Native and host validation

Testkit fcd228f. Evidence relative to TMPDIR:

- daimos-bitmap-directory-v1977.bcA6nu: final KCC/DAS source-linked
  PDP6 and KA10, identical 2760 matrix outcomes each, 1202 success,
  862 unsafe-overlap, 696 no-gain. Independent reference stream and
  per-word consumed-input ledger, whole-arena comparison, independent
  read-only planner on every encode, 9 lengths, 8 density patterns,
  11 offsets plus exact end alignment. 50 encode and 99 decode
  geometries succeed in reverse only; 288 geometries refuse both.
  Synthetic5248-word zero in-place forward encode/reverse expansion
  and sparse reverse encode/forward decode, 146 masks, exact output,
  excess capacity and guard checks PASS. Canonical and invalid range
  tests PASS. Executed mutants missing encoder guard (exit1), decoder
  mask/literal guards (each exit3), exact consumption (exit5) rejected.
  Injected late write-pass error returns hard -6 after real stores,
  both encode/decode PASS. This is a status-propagation test, not
  evidence of recovery of a torn real process image.
- daimos-bitmap-overlap-v1975.rIUsCn: unchanged interleaved 612-case
  matrix and both unsafe hazard mutants PASS.
- dlink-rh18-test-v1977.R7Kf4y: original integrated host failed linking
  the codec test due to valid negative LOCAL_RH18 displacement. Restore
  previously accepted standalone daimos-tools commit48fc3df rule for
  both LOCAL/SYMBOL_RH18: representable negative result with negative
  addend wraps modulo 2^18; positive overflow and LH18 underflow still
  rejected. Same rule in actual native linker. Dedicated native loads
  using both actual DOBJ relocation types PASS; host negative guards
  PASS. Native linker negative guard cases not separately exercised.
- daimos-native-tools-v1-24018: actual native DLINK archive link,
  marked executable self-reads exact TX2 and checks both negative
  relocations (MUTABLE TX2 OK, RH18 WRAP OK, NATIVE LINK OK), all six
  shell probes PASS. Full distribution retains pre-existing notes.
- dlink-rh18-v1977.nLmQxv/install-v1977/local.tar.gz: updated host
  dlink installed at PDP10_PREFIX/bin. SHA256
  b6182a58fa5b8cef331bfa03e5cb8ad69a71c0e9fb07ee27a4b04e1abbe1cf74.

## K12 boundary

Next select a real exclusive source/target lease. Prefer a complete
encoded stream within still-owned reconstructible PURE prefix, so RAW
remains authoritative through commit. Other geometries require explicit
recoverable ownership transition before source overwrite. A pinned
no-reclaim scratch extent is fallback only when bounded and justified;
never borrow released words or create a permanent codec slab. Direction
selection cannot make every overlap safe. Use RAW when both refuse.
Prove exact MM source/target owner/length/pin, no foreign-owner access,
source/backing survival, cancellation and uncertain commit recovery.
Current fixed VM destroy refusal and default RAW PURE/mode3 OFF remain.
