# KCORE v1516: exact physical DECtape root-member ownership

Date: 2026-11-12. Scoped KCORE feature acceptance for the TSFS boot
handoff and optional tape MRES. No automatic device removal detection,
false root crash on ordinary I/O error, or new fixed-KCORE state.

## Problem: unit zero and unused logical slots are indistinguishable

TSFS uses one existing 36-bit `tsfs_file_shape` word per mounted
filesystem. The low 24 bits encode up to eight logical-to-physical
DECtape member selectors, three bits each. An absent trailing logical
member is encoded zero, just like a genuine physical DTC unit zero.
Scanning all eight selectors would therefore falsely classify DTC0 as
critical for a root that uses only nonzero physical units. This is
unsafe if device-loss policy later consults that result to HALT KCORE.

During KINIT, `tsfs_boot_select()` already verifies the complete
multi-member tape set and knows its exact physical units. v1516 builds
an explicit eight-bit member mask there and stores it in bits 24..31
of the *same* `tsfs_handoff[1]` word. The original low 24-bit member
selectors are unchanged and bits 32..35 stay clear. Duplicate physical
units are rejected before the handoff is published. The redundant
second boot-time loop is eliminated; `tsfs_boot_unit_mask_v22()`
derives its selected physical mask directly from the validated handoff.

The optional `tsfs_root_owns_dtc_unit_v1516` PDP-6 assembler routine
validates the queried unit 0..7, the actual `vfs_namespace_root`
TSFS provider and public mount ID, the published `vfs_mount_limit`,
the dynamic per-mount shape pointer and the nonzero stored mask.
It returns +1 only for a selected root member, 0 for an unrelated
physical unit or a non-TSFS namespace root, and -1 for an invalid
request, missing mount, absent shape or malformed zero mask.
It is **not** an I/O status or proof that a tape has been removed.

The normal TSFS file reader still right-shifts the physical-selector
word by at most 21 bits and masks the low three bits. The new upper
eight-bit root mask cannot affect its data/extent unit selections.
No on-media format or public filesystem ABI was changed.

## Accepted tests

The host KINIT source-linked `daimos-tsfs-boot-eight-v1` now PASSes
all physical mask/locator checks for an eight-member permutation of
DTC units 0..7, a single-member root on **unit zero**, and a two-member
root on units **zero and five** with metadata on the second member.
Exact map and original low24 fields are checked in each case.

The native PDP-6 `daimos-tsfs-root-member-v1516` source-linked test
PASSes both selected units in the two-member root, unrelated units,
different root provider, illegal positive/negative DTC units, an
out-of-range mount ID containing otherwise plausible data, invalid
or missing shape, single-unit zero root and full eight-unit root.
Four executed unsafe source mutations are rejected: wrong high-mask
shift, wrong root provider, skipped mount-limit check, and accepting
an invalid zero mask. These are real PDP-6 instruction tests.

Existing source-linked TSFS stored-extent/slab and cross-member
read regressions PASS. The slab test now includes a valid upper
membership mask while continuing to read the same original low-bit
physical selectors. Older standalone tests gained only missing
test-only VFS root/mount symbols and fail-closed unused compressed-
decoder symbols. No fake production success path was introduced.

Both isolated complete tape-root build profiles PASS assembly/link:
32K low-memory GCC KINIT and optional DTFS/TSFS MRES; 256K full
KCC KINIT and optional DTFS/TSFS MRES. These are **build results**,
not physical boot-with-detached-tape acceptance.

An independent strict fixed-KCORE 32K gate reports
`__kcore_low_end=023241` octal, exceeding its `<020000` octal limit.
**The 32K memory gate is not accepted.** v1516 adds no fixed-KCORE
instructions or data; this existing build's overall fixed size issue
must be addressed separately once feature completeness is established.

Controlled old/new optional DTFS/TSFS MRES links use the same current
KCORE map and other objects with independently regenerated fold plans.
The next TSFS function shifts by +27 words and a subsequent read
routine by +25, bounding v1516's added **optional MRES code** at
approximately 25-27 PDP-6 instructions. No new resident **data**
word is required because the selected physical mask occupies the
existing mounted TSFS shape word.

## Still open: trustworthy device-loss indication

SIMH's Type-551 model sets `DTB_ILL` when attempting to select an
unattached DTC unit, but also uses it for an illegal controller/unit
command. That status alone cannot safely mean *confirmed* tape
removal on real hardware. `tape_watchdog_tick_v1089` times out
search/data transfers but a timeout is not a media-presence verdict.
No DTC driver is yet wired to the v1515 fatal HALT leaf. Its future
caller must distinguish authoritative loss of an active TSFS root
member from transient tape errors, then invoke root fatal reason
AC1=1. A missing **non-root DTFS** member must remain nonfatal with
checked same-volume/replacement revalidation. Physical detach and
recovery tests are future P0/P1 work, not claimed in v1516.
