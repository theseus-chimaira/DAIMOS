# v1664: bounded FULL APR restore from a compressed PURE MM owner

Date: 2026-11-18. Parent DAIMOS v1663 commit
`640be3b6d37cba9082cf6f60f24e6b8d90e3ceef`.

## Original MM ownership and APR reconstruction

The opt-in, unlinked `proc_pure_bitmap_restore_v1664.c` accepts a
single exact unpinned mode3 PACKED_PROCESS owner in the existing MM
descriptor array. Its physical encoded length is the existing
extent length; the logical RAW suffix is derived from the immutable
PURE executable text boundary and original APR VM size. No new
per-process compressed-size registry, persistent scratch or KCORE
state is introduced. The fixed production mode assembler continues
to reject mode3 until the entire lifecycle is integrated.

The caller holds PROCESS TRANSITION and is either a stopped/sleeping
process or an SRUN process being restored by current slot 0. The
preflight checks saved immutable executable backing and live mount
identity, stable u-area, zero current APR base, an exact aligned
original APR size, and mode3. The unique encoded MM owner must be
unpinned, physically within 18-bit core, contain at least
`ceil(raw_words/36)` bitmap masks and be strictly shorter than RAW.

After source pin and proof of its exact MM descriptor, the original
source-linked PDP-6 `mm_alloc_aligned_noreclaim` allocates one full
02000-octal-aligned PROCESS APR extent at HIGH preference, with no
recursive swap, MM compaction or cache reclaim. Its exact type,
owner, span, physical boundaries and disjointness are checked
before physical writes or the target pin. The source and target
remain pinned during a capacity-bounded FULL loader callback, which
recreates immutable text from the original backing. The callback
is the existing v1391 four-argument ABI and MUST refuse payload
writes beyond its `allocated_words` capacity. The bounded v1660
bitmap preflight verifies every mask and nonzero literal and exact
stream consumption before the original 30-word decoder writes the
RAW suffix at `apr_base + skip`. No second RAW-sized scratch image
is required. Any loader-created writable words are overwritten by
the authoritative decoded RAW suffix.

## Failure-atomic MM commit

The new opt-in PDP-6 assembly `mm_pure_bitmap_finish_v1664.s`
accepts `(slot, packed_base, encoded_words, apr_base, apr_words)`.
It requires exactly one once-pinned PACKED_PROCESS source and one
once-pinned PROCESS target, correctly aligned and physically
disjoint. All extra process-owned extents, reserved MM types,
unexpected sizes, pins, forged bases and corrupted MM capacity
are refused BEFORE writing any descriptor. MODULE and
KERNEL_DYNAMIC have independent owner namespaces and do not
invalidate the process pairing.

Under PI exclusion it deletes the packed source descriptor, shifts
the original MM array as needed, then consumes the new APR's one
pin by an unconditional store; there is **no fallible MM operation
after source retirement**. The C caller rechecks mode3, PROCESS
TRANSITION, absent APR mapping and saved backing under that same
PI exclusion. After MM success it stores APR base into existing VM
state, clears the saved PURE backing, changes mode3 -> mode0 in the
existing two-bit map, and clears TRANSITION, all before restoring PI.
There is no free/unpin gap after destroying the last compressed
source and no separate resident descriptor or temporary registry.

## Failure handling and retry

Ordinary source pin/target allocation and loader/decode refusal
retains the compressed source. If the APR was allocated, its pin
is released and checked, its exact MM descriptor freed and checked,
and ONLY THEN the source is unpinned and verified. Refusal after
MM finish preflight follows the same order. All cleanup rechecks
mode3 and the original process's TRANSITION, absent APR and saved
backing before releasing anything; unexpected changes do not
authorize a second free, an unsafe unpin or a runnable process.
An uncertain or falsely successful MM call returns BUSY with the
remaining extents quarantined under TRANSITION. There is no blind
MM repair or assumption that an error necessarily mutated nothing.

Untrusted or malformed packed streams never reach the unchecked
decoder. The loader contract is trusted kernel code, not an ABI
that permits arbitrary caller-supplied function pointers. No
physical MM allocation or filesystem I/O is performed with PI
disabled; the final atomic section is only MM descriptor and
publication metadata.

## Native PDP-6 evidence

The testkit chains the actual original source-extracted PDP-6 MM
allocator, find/fit, insert, pin/unpin, free/delete, the permanent
mode2 expectation, v1451 30-word codec, and the v1660/v1661/v1662/
v1663 staged routines. The loader is a bounded test callback, not
an actual executable VFS file read. A second independent managed
arena supplies exactly one full 4,096-word APR reservation; the
original 883-word first arena retains the compressed candidate.

Native success reconstructs all 4,096 APR words, checks the
immutable prefix produced by the loader and the bit-identical
3,456-word RAW suffix, then verifies the sole unpinned PROCESS MM
owner, published APR address, cleared backing, RESIDENT mode and
cleared TRANSITION. An all-zero RAW image (96 encoded masks) also
passes. Other successes cover an independent MODULE sharing the
numeric process ID and SRUN restoration by slot 0. The direct MM
test covers source-before-target and target-before-source layout,
unrelated MODULE ownership, and deletion from a full 136-entry MM
extent table.

The v1664 native matrix has 62 cases: 38 restore-wrapper cases
(including four positives), and 24 direct-ASM cases (four
positives); 54 refusal/corruption conditions are tested. It
includes source/target pin mistakes, loader failure after writes,
malformed masks/literals, full APR allocation NOMEM/descriptor
failure, refused/false-success cleanup, corrupt owner geometry,
stale or externally modified mode/PROCESS TRANSITION/backing/APR,
and forbidden extra PROCESS extents. Two independently executed
dangerous source mutations (skip bitmap decode verification and
ignore duplicate packed MM owner) are rejected by the native PDP-6
oracle. The entire v1663/v1662/v1661/v1660/v1659 native regression
chain is rerun on release.

Unlinked v1664 standalone C restore costs **969 TEXT words**, MM
finish **133 TEXT**, both zero DATA/BSS. These numbers do NOT
represent installed KCORE growth or optimized production MRES cost.
Shared ownership scans and error paths should be folded before any
physical small-memory acceptance.

## Production remains gated

The fixed `proc_storage_runtime_v30.s` still rejects mode3, and
the normal scheduler, `proc_swap_detach`, RAW-only terminal path,
fast PREFIX restore, RUN and WAIT are unchanged. Source-linked
FULL recovery is **not** complete EXIT/WAIT/scheduler integration,
and no real source-built physical 48K/64K whole-guest boot or
continuous peak RAM measurement is claimed. Next is terminal
dispatch and PI6 scheduler/finalizer integration, then matched
physical acceptance with PURE OFF by default. Strict 32K deferred.
