# KCORE P1-E v1657: real PDP-6 48K RAW suffix bitmap density

Date: 2026-11-17. **Actual physical 36-bit PDP-6 memory
capture PASS; exact source-linked bitmap word arithmetic PASS.**
No production KCORE, MRES, MM, VFS, process, boot or ABI changes.
No new physical memory saving is claimed or installed.

## Original source and MM geometry

The original v1654 rebuilt GCC 48K/no-AUX/selected-TSFS-root
PDP-6 boot executes real compressed PURE children from separate
native secondary DTC3 DTFS. The original v1654 source-linked
accepted MM snapshot at parent B has six distinct, unpinned
MM_TYPE_PACKED_PROCESS owners in slots2..7, each exactly
006600 octal = 3,456 decimal retained RAW words.
The user program has genuine writable DATA/private stack.
The seventh RUN correctly refuses, all original child PIPE,
private DATA, EXIT-057, exact parent WAIT and normal HALT pass.

The v1657 testkit derives the six original physical addresses
from the v1654 accepted B MM table using the v1638 exact
two-word extent parser. It privately copies all four original
Type-551 DTC reels and reboots the UNMODIFIED original
48K KCORE/user image. At genuine source marker aaaaaax[B,
the simulator examines EVERY 36-bit word of all six RAW
suffixes, 20,736 words total, before allowing the remaining
six-child lifecycle through normal HALT.

No raw words are guessed from BSS sizes. Each complete physical
word/address pair is checked, each private suffix is SHA256-
authenticated against this specific original source-built
run, and the v1451 exact 36-word presence-bitmap/literal
representation is reconstructed in both directions.
There are TWO original private suffix contents:
slot2 differs from slots3..7 in three nonzero private
control words. They have exactly equal density but are
NOT falsely assumed byte-identical.

## Measured actual bitmap data sizes

| Physical owner | RAW words | Nonzero literals | Presence masks | Packed words | Potential saved |
| --- | ---: | ---: | ---: | ---: | ---: |
| Slot2 | 3,456 | 65 | 96 | 161 | 3,295 |
| Slot3 | 3,456 | 65 | 96 | 161 | 3,295 |
| Slot4 | 3,456 | 65 | 96 | 161 | 3,295 |
| Slot5 | 3,456 | 65 | 96 | 161 | 3,295 |
| Slot6 | 3,456 | 65 | 96 | 161 | 3,295 |
| Slot7 | 3,456 | 65 | 96 | 161 | 3,295 |
| **Total** | **20,736** | **390** | **576** | **966** | **19,770** |

For each source 3,456/36=96 groups; every group is one
36-bit mask, followed by its nonzero literal words.
Therefore 96+65=161 encoded words, versus original 3,456.
This is an EXACT candidate-data measurement of the already
PDP-6-native-tested 30-word v1451 zero-bitmap codec format.
It is NOT general-purpose LZ or DXR compression.
Zero words per owner: 3,391. The six-source candidate
representation is 966 words against 20,736 RAW.

The original 48K v1651 B checkpoint largest available free
contiguous gap is 883 words. A single disjoint 161-word
candidate would FIT that gap numerically, but does not prove
MM descriptor availability, accepted concurrent allocator
ownership, PI atomic commit, source release, mode3 state,
no intermediate full-image copy, memory-pressure progress,
full decode+RAW overlay correctness or total net savings.
Likewise it does not claim compression was performed
inside the physical KCORE: the original running image
continues to own full RAW suffixes.

## Source validation and false witnesses

The real physical run finishes all six EXIT/WAIT and normal
PC000000 HALT. The new testkit auditor rejects three separate
false original trace witnesses: changing one retained word,
deleting one physical RAW address, and forging a normal HALT.
The former naive assumption that ALL six private byte strings
must be identical was rejected against actual evidence;
the auditor now checks the TWO distinct original content
hashes and all six correct decoded representations.

Testkit branch test-kcore-pure-raw-density-v1657-20261117,
commit 5cbe205. Test entry:
tests/diagnostics/pure-suffix-entropy-v1657/run-v1657.sh.

Versioned physical trace/manifest/symbol-base/invalid witnesses:
$TMPDIR/daimos-kcore-v1657-evidence-20261117.tar.gz
SHA256 c12efb2caa5d489b3638d8d34f95fec6ca4ecf92fd0327ca10e798a90a24e3f7.

## Architecture decision and remaining work

**Decision: the optional v1451 bitmap transaction merits an
actual failure-atomic PDP-6/MM prototype**. It is not justified
to dismiss it as NO COMPRESSION on codec-TEXT overhead alone
when a REAL physical six-owner pressure workload exhibits
19,770 potential words of candidate data savings. RAM remains
the primary optimization objective, but this is an optional
measured prototype authorization, not release acceptance.

Before production activation, require exclusive source and
output ownership, output length bounds, source/non-overlap
preflight, failure-atomic mode3 storage publication, no-pinning
leaks, lossless 36-bit private data decode, original loader
fast/FULL transitions, SLEEP/PI6/EXIT/WAIT/EXEC/unmount guard,
OOM, and complete original physical ON/OFF net/continuous
peak RAM accounting including codec, temporary MM descriptors,
installed optional MRES and provider scratch.
If these total costs erase the gain or a correct transaction
cannot be safely completed, remain RAW and record NO COMPRESSION.

No 32K boot/footprint acceptance. It is explicitly DEFERRED
until after remaining functional KCORE development and freeze.
PDP-6 only; KA10/KI10 not silently treated as APR backends.
