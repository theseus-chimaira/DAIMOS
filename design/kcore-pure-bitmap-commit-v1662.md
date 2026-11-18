# v1662: prepared PURE bitmap MM commit, fail-atomic RAW retirement

Date: 2026-11-18. Exact parents: DAIMOS v1661 commit
`2fe9d857b1152a6d1eeb35e91c57116fc2ae5627`, testkit
`1705321e10b77a7da374efdcbbbe79efe8ec4412`.

## Problem and commit point

An ordinary `mm_unpin(raw)` followed by `mm_free(raw)` is NOT a
failure-atomic compressed process commit: between the calls, MM may
refuse the free. Re-pinning/repairing the old owner after refusal
would be a risky workaround. The v1662 optional PDP-6 assembler
`mm_pure_bitmap_retire_v1662.s` instead validates the exact raw
owner carrying ONE pin and the unique unpinned, disjoint bitmap
candidate, then deletes the raw MM descriptor in one nonfallible
post-validation metadata sequence. It does not first unpin RAW.

The target must have `MM_TYPE_PACKED_PROCESS`, the same process slot,
an exact shorter encoded size, and no pin. The source must have the
original exact RAW suffix size. Third PROCESS/PACKED_PROCESS owners,
invalid/reserved owner types, forged size, unexpected pins, overlap,
out-of-range arguments, count/capacity corruption all fail without
changing either descriptor or pin. MODULE and KERNEL_DYNAMIC have
independent owner namespaces and are left untouched, even when their
numeric owner matches the slot. Deletion uses the original MM
descriptor-shift algorithm and accepts either relative descriptor
order and the exact maximum 136-descriptor table. No separate
per-process registry is installed.

`proc_pure_bitmap_commit_v1662.c` is a **prepared-only** optional
C89 transaction. Its caller already holds PROC_TRANSITION on a
stopped/sleeping process, owns a once-pinned authoritative RAW
suffix and has allocated/encoded/unpinned a separate candidate.
Before dereferencing either physical extent it validates the same
two owners against MM, immutable executable backing and current
mode2. It compares every RAW word to the canonical bitmap, including
nonzero literals, absent zeros, final-mask padding and exact encoded
consumption, with no additional decoded-size buffer. It checks
source and candidate disjointness and 18-bit physical boundaries.

With PI disabled, it checks the current two-bit mode again and
calls the MM retirement leaf. If the leaf refuses, both MM owners,
their pins and mode2 remain intact under TRANSITION. After success,
the C caller writes the existing mode pair from 2 to 3 before
restoring PI, with no fallible operation after RAW deletion.
Backing, u-area, process state and TRANSITION stay unchanged.
The original permanent mode assembler still disallows 3; v1662
does NOT link the service into a runnable process.

## Native source-linked evidence

The testkit links the **actual** PDP-6 source-extracted original
`mm_alloc_aligned_noreclaim`, `mm_find_fit`, `mm_extent_insert`,
`mm_free`, `mm_pin` and `mm_unpin`, the original storage-mode2
expectation leaf, the unmodified 30-word bitmap codec, the v1660
bounded decoder, the v1661 compressed terminal and both new v1662
source objects. It does not simulate MM allocation or MM deletion
in C. Only PI exclusion and an injectable MM-commit refusal use a
controlled fixture.

The v1657 workload geometry is 3,456 RAW words, 65 nonzero words,
96 bitmap masks, exactly 161 encoded words. The original 883-word
high physical gap accepts a real 161-word allocation at octal
`050122`. The new MM leaf commits mode2 -> mode3, leaving one
unlinked compressed MM owner. The **original real allocator** then
successfully allocates an independent 02000-octal-word extent at
octal `040000`, inside the formerly pinned RAW span; after its
release, bounded bitmap decode reconstructs every RAW word and the
v1661 terminal leaf frees the sole compressed owner and clears
mode3. This demonstrates actual reuse of reclaimed physical space
in native original MM, not live-guest compression acceptance.

Twenty-four C transaction refusal cases and twenty-one independent
assembler-leaf refusal cases retain exact MM/pins/mode; valid cases
also cover a same-ID MODULE owner, RAW-first descriptor deletion,
and deletion from a full 136-slot extent table. Independently
assembled/compiled unsafe mutants that bypass canonical verification
or a third PACKED MM owner are both executed and rejected.
The entire v1661/v1660/v1659 native baseline also passes.

Unlinked prototype object sizes: prepared C commit **606 TEXT**,
MM retire **125 TEXT**, combined **731 TEXT**, each zero DATA/BSS.
These are costs of isolated staging code, not added permanent KCORE
words or installed MRES footprint; production should share existing
source checks and minimize installed RAM before acceptance.

## Not yet installed

This stage does NOT implement the bounded allocation/encoding and
cleanup frontend, does not automatically clear TRANSITION for a
compressed sleeper, and does not admit mode3 into the fixed
`proc_storage_runtime_v30.s` or dispatch `proc_swap_detach` to the
compressed terminal leaf. FULL restore of a mode3 process including
immutable text and actual APR publication, PI6 scheduler failure
handling, real physical 48K/64K end-to-end compression and
continuous peak-RAM measurement are still open. A corrupted or
unowned preparatory candidate must stay quarantined until its
own MM rollback proves successful; v1662 never repairs it blindly.

PURE default OFF. Strict 32K physical acceptance deferred. There
is no claimed installed net physical RAM saving yet.
