# K15/K16 v1985: lower-sector placement during original bitmap DROP

Parent 4b4982bb. v1984 measured substantial savings but lower admitted
capacity (8 directory vs 9 RAW) because small bitmaps at original APR
boundaries split free space into unusable aligned gaps. Avoid changing
fixed mm_compact: type4 is shared by RAW and bitmap, and RAW geometry
must remain tied to its original APR prefix. A separate generic
published-owner relocation protocol would require independent
ownership/recovery handling and more code.

## Selected approach

Use existing synchronous PI-excluded K12/K13 prefix lease. After the
candidate has been verified against intact original RAW, search for
lowest free 0200-aligned interval with mm_find_fit (new exported symbol
only, no fixed instructions). Use the gap only if below original base;
otherwise old in-place commit. The original full extent remains pinned
throughout; no additional owner, descriptor, allocation, or scratch.
The optional assembly entry mm_pure_bitmap_place_v1985 shares v1979
full owner and geometry proof. Require exact lower disjoint target,
positive sector alignment, valid arena containment, sorted complete
MM table and sole once-pinned original PROCESS. Backward scan from
original descriptor proves every preceding interval is disjoint and
identifies insertion index. After all checks, BLT candidate to target,
shift intervening descriptors backward, replace original descriptor
with exact PACKED_PROCESS, zero pins. Count unchanged. No fallible
software operation or callback after first store. Same-base entry
retains original in-place semantics. This is not a hardware memory
fault recovery mechanism.

Before placing, save word067 bit31 plus both kind bits and retry tag,
LH proposed source sector, RH exact encoded length. Original full VM
base/words and both backing identities remain until publication.
The old dirty-prefix escrow instead records original RAW suffix;
bit31 distinguishes placement. Combined kind rejects old RAW recovery.
MM success requires exact proposed bitmap owner, not return code;
lost success accepted, false success quarantined. On reported refusal,
only exact original pinned full owner can revert placement marker to
original prefix marker and use existing RAW cancellation. Dirty
placement recovery is NOT implemented; no speculative pin release.
FULL source discovery, clean unlock and terminal geometry require
0200 instead of 02000 alignment only with placement selected. Full
APR target, original RAW geometry and all exact owner/range/pin proof
unchanged. Existing word067 source sector and terminal BRK escrow
already represent this alignment. No new per-slot storage.

## Native and package gate

Testkit4df3cf2, make test-pure-bitmap-placement-v1985, KCC/GCC each
25 DROP/commit/direct MM, 4 FULL, 2 terminal calls per CPU (PDP6/KA10).
Full descriptor capacity, exact lower gap, original no-fit fallback,
refused/false/lost reply, malformed table/duplicate/wrong pins,
invalid overlap/arena/alignment/direction, one/three intervening
foreign descriptors incl independent owner namespaces, original full
APR allocator reuse and complete overwrite. Real bounded plain and
compressed backing loader, clean failed FULL+unlock+retry, exact
moved terminal PREVM refusal and retry through staged VM destroy.
Six unsafe controls per compiler fail native assertions (no copy,
no overlap proof, no shift, no target alignment, trust reply, no
placement escrow). Initial two fixture issues: exact candidate helper
required original one pin in deliberately bad owner case; OR of pin
bit did not actually create second pin. Corrected fixture, no proven
kernel defect. Final evidence pure-bitmap-placement-v1985.f5IRAH and
pure-bitmap-placement-v1985.97yGI3. Placement-OFF K12/K13/FULL/service/
terminal regressions PASS, pure-placement-regression-v1985.uvVqvy.

Use existing experimental DAIMOS_PBM_DIRECTORY_V1983=1 to select
placement producer, atomic leaf and matching restore/terminal readers.
No new user-facing flag; default and historical interleaved unaffected.
Both compiler directory->RAW->directory reproducible, linked source
package checks pass, pure-directory-package-v1983.zYDnuc. GCC directory
5460 vs5359, KCC6504 vs6401 (+101/+103 incl v1984 RT+2); RAW3258/3885
and both fixed KCORE DXRs byte-identical to v1983. Zero BSS or new
per-process words. New optional MM leaf132 vs89 words. Original code
and placement-OFF tests remain available to prove regressions.
No new toolchain defect observed; existing GCC include-order notes.

## Physical gate

Matched original v1984 INIT/COMP bytes and 64K strict TSFS root, actual
v1985 source-built image pure-placement-build-v1985.s17eYq/build;
three members validated. Existing replay recognizes new alignment
and independent verifier accepts exact sleeping resident children
if admitted without pressure. Final MM/HALT verification now requires
all child storage cells, user extents and u-areas retired. Reject old
incomplete traces. Physical aligned admission gain remains a mandatory
gate; do not infer capacity from gross free words. K15 actual PI6,
READY_ONCE, EXEC/nonresident terminal and K16 uncertain recovery,
RT latency/stack, 48K/real DSH remain open. Default OFF.
