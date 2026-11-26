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
incomplete traces. Actual matched physical replay PASSED:

| Physical pressure metric (decimal words) | RAW | Old bitmap | v1985 |
| --- | ---: | ---: | ---: |
| Children admitted | 9 | 8 | 12 |
| RAW children | 9 | 1 | 1 |
| Directory children | 0 | 7 | 4 |
| Resident children | 0 | 0 | 7 |
| Total free | 5908 | 31078 | 1837 |
| Largest 1024-aligned free interval | 1536 | 3584 | 1024 |

Same independently linked executable hashes. 12 is fixture ceiling,
not measured maximum. All33 MM extents valid and nonoverlapping.
New bitmap bases octal061400,062400,063400,064400, each239 words;
actual resident APRs reused former high bitmap locations. Equivalent
RAW saving4*(3584-239)=13380 gross, net11186 after actual2194 resident
overhead. Measured admission +3 vs RAW, +4 vs original bitmap; free
RAM is lower because additional children actually use it.
Actual slot-zero restore slot3, original relocated source061400/239,
disjoint target160000/4096; each pinned once at atomic finish. Peak
image4335 words. All12 complete sparse data/status WAIT checks, parent
Z, exact final child mode0/free, no user or u-area owners, normal HALT
PC0 and Goodbye. Independent verifier testkit6028851 checks old/raw/new
fixture hashes, capacity gain, relocated source, full lifecycle trace
and final MM/HALT; five final-state trace controls reject stale mode,
stale u-area bit, orphan u-area, wrong current and missing HALT.
Evidence pure-placement-physical-v1985.GSp074/physical.clean,
verified-pressure-v1985.txt and verified-final-v1985.txt. Runtime
replay completed with exit0; no running simulator. Close matched 64K
placement/capacity and successful RUN/SLEEP/restore/EXIT/WAIT/final MM
subgates, not all K15 or K16. Actual PI6/READY_ONCE, EXEC/nonresident
terminal, dirty recovery, RT latency/stack and 48K/real DSH remain
open. Default OFF.
