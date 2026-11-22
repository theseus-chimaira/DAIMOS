# v1663: bounded PURE bitmap compression frontend, test-only

Date: 2026-11-18. Exact parent DAIMOS v1662
`06f34cc23d4e5a16e6c735801930089bf0583e5f`.

## Owner transition and RAM policy

The opt-in source `proc_pure_bitmap_compress_v1663.c` completes
the v1662 prepared-only MM commit's missing bounded frontend. It
requires a noncurrent sleeping/stopped process with TRANSITION, a
stable u-area and matching immutable executable backing, zero APR
base, storage mode2 (RAW), and precisely one unpinned RAW
PACKED_PROCESS owner. It uses the existing process and MM metadata:
no persistent registry, additional per-process word, RAW-sized
scratch buffer, KCORE or installed MRES dependency.

The frontend pins RAW, verifies it is exactly once pinned, counts
all nonzero 36-bit words and calculates the exact original v1451
bitmap length as `ceil(raw_words/36) + nonzero`. If the encoded
length is not strictly smaller, it unpins RAW and returns the
distinct no-gain code (`-6`) without allocating anything.

The original `mm_alloc_aligned_noreclaim` reserves precisely that
many words with type PACKED_PROCESS and the same slot, using HIGH
placement. It does not invoke MM compaction, swap or reclaim. It
confirms the two exact MM owner descriptors, one RAW pin, disjoint
positive 18-bit physical bounds and an unpinned candidate BEFORE
pinning the candidate or writing anything. The original 30-TEXT-word
v1451 assembler encodes RAW directly into the pinned candidate.
After candidate unpin and exact MM verification, the v1662 commit
independently rechecks immutable backing, canonical bitmap content,
the exact two MM owners and mode2, then under PI exclusion deletes
the once-pinned RAW MM descriptor and publishes mode3 without a
separate fallible RAW unpin/free interval.

All successful compressed processes remain under TRANSITION. The
new entrypoint does **not** make a mode3 sleeper runnable and does
not attach itself to the scheduler. The future service must integrate
FULL restore and EXIT/WAIT before clearing TRANSITION.

## Fail-closed cleanup contract

On ordinary allocation error, exact owner validation followed by
RAW unpin returns the original MM error. On verified target pin
refusal, the candidate is freed first and RAW unpinned last. If
encoding is corrupted, the v1662 canonical full-stream verifier
refuses commit and the same bounded cleanup applies. The frontend
does not duplicate the v1662 full RAW stream scan after encoding.

Before freeing a refused candidate it rechecks both exact owner
descriptors AND mode2. After candidate free it proves the RAW
descriptor is still the sole once-pinned PROCESS owner, unpins it,
then confirms the exact sole unpinned owner. No error or false
success is allowed to silently restore execution with uncertain
MM. If free, pin, unpin or mode verification is uncertain, the
frontend returns BUSY, leaves TRANSITION held and does not attempt
blind ownership repair or a second unsafe free. In particular,
an overlapping/forged allocator witness never authorizes encoding
or cleanup. No writable RAW owner is retired on refusal.

## Native evidence (real original MM)

The testkit source-links the actual source-extracted PDP-6
`mm_alloc_aligned_noreclaim`, `mm_find_fit`, descriptor insertion,
`mm_pin`, `mm_unpin`, `mm_free` and MM deletion, plus the permanent
mode2 expectation leaf. It runs the actual 30-word v1451 encoder,
the unmodified v1662 atomic commit/retirement, the v1660 bounded
decoder and the v1661 compressed terminal MM release. Only
injected MM failure/witness faults and PI-state observations use
controlled testkit wrappers. The original v1662/v1661/v1660/v1659
native regression chain is executed before the v1663 fixture.

The 3,456-word RAW image with 65 nonzero words gives 96 bitmap
masks plus 65 literals, exactly 161 encoded words. Real original
MM allocates at octal `050122` in the established 883-word gap;
after commit a separate real MM allocation reuses 02000 octal
words at the former RAW address `040000`. Complete native bounded
decode and compressed terminal release PASS. An all-zero 3,456-word
RAW image instead encodes to 96 masks alone, allocated at octal
`050223`; its complete reconstruction, reuse and terminal release
also PASS. Dense RAW rejects with no allocation.

Forty-four source-linked native cases comprise three positive
scenarios (sparse, sparse plus independent MODULE owner, all zero)
and 41 refusal/no-gain and MM uncertainty states. These include
actual physical NOMEM and descriptor shortage; false-success pin,
unpin and free, false-failure allocation/pin/unpin, duplicate or
reserved MM owner, malformed encoder output, concurrent mode-map
corruption, invalid physical allocation, and no-gain thresholds.
In every ordinary refusal, RAW is exact and mode2 remains published;
uncertain cleanup retains TRANSITION and MM ownership for diagnosis.
Two independently compiled dangerous source mutations (skip target
geometry verification; skip real target unpin) are executed in PDP-6
and rejected by the native oracle. The source also compiles with
KCC warning-free.

Isolated frontend C prototype: **776 TEXT words**, **0 DATA/BSS**.
The existing v1662 C commit is 606 TEXT and v1662 MM retire leaf
125 TEXT (also unlinked). These are not net installed MRES size or
permanent KCORE words; minimize shared code and test actual guest
memory before considering production installation.

## Not installed, no premature savings claim

The original fixed KCORE storage-mode assembler still rejects
mode3; no scheduler, process RUN, `proc_swap_detach`, executable
restore or WAIT path dispatches a compressed owner. No integrated
mode3 FULL APR restoration has been built, so no sleeping process
is yet eligible to resume with compressed PURE memory. Native MM
reallocation demonstrates real physical reuse under the isolated
fixture, **not** an accepted complete guest or continuous peak-RAM
measurement. Next: implement fail-atomic FULL restore, then terminal
dispatch, PI6 fault handling, and 48K/64K physical net RAM
acceptance. PURE default OFF. Strict 32K deferred.
