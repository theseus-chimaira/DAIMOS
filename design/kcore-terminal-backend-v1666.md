# v1666: one KINIT-patched terminal release, no dual KCORE owner scan

Date: 2026-11-18. Parent DAIMOS v1665:
`08a31f16c1c5a7793e8bc59995efaed238954f31`.

## Problem and memory ownership

The fixed `proc_swap_detach()` previously contained the DISK
allocation release and two-bit mode transition and depended on a
second fixed PDP-6 `pdp6_pure_detach_v1609` RAW MM-owner scanner.
The two incompatible memory backend implementations were therefore
carried in KCORE even though v1665 already installs exactly one
disk-swap or optional PURE MRES at boot. The unneeded RAW scanner
wasted permanent words on disk-swap and swapless-disabled systems.

v1666 adds ONE default-refusing fixed entry,
`proc_swap_term_v1666`, alongside existing `proc_swap_reclaim`
and `proc_swap_service_one`. KINIT patches it once with the third
PURE MRES export or eighth DISK MRES export. The COMMON fixed
`proc_swap_detach` only checks whether the existing two-bit mode is
RESIDENT; if not, it calls the installed terminal release service
with the existing `(slot, process_descriptor, backing_word)` ABI.
It never interprets a disk block address as a PURE executable
backing and never scans PACKED_PROCESS extents. On successful
service return it clears existing swap-record/u-area/mount-staging
metadata. A refused backend call retains every such field for
retry. Mode0/resident metadata cleanup works without any installed
memory backend; mode1/mode2 must fail closed via the default stub
when no MRES has been installed.

The disk-only service `proc_swap_mres_terminal_v1666` is an
assembler leaf inside the existing proc_swap_pdp6.s MRES. It
requires exact mode1, absent APR, nonzero disk block count and
sufficient physical swap-accounting words before asking the
original `backstore_free` provider to release the validated
allocation. Only after successful free does it subtract the
count and publish mode1->mode0. A post-free mode-map mismatch
uses the pre-existing fatal invariant, not a second free.
New allocations may be disabled; legitimate release remains valid.

The PURE terminal service is the ORIGINAL
`pdp6_pure_detach_v1609` assembler body, now assembled into the
PURE MRES rather than concatenated into the fixed proc_swap.s.
The MRES exports it directly at index 2, avoiding an additional
function wrapper. Before touching its RAW MM descriptor, it now
validates exact mode2, in addition to its original TRANSITION,
u-area saved backing identity, absent APR, PURE suffix geometry,
unique unpinned PACKED_PROCESS MM owner and physical MM free.
As before, mode2->mode0 follows only successful release, with
fatal invariant on a theoretically impossible late publication
failure. A mode3 compressed owner cannot enter this RAW path;
the fixed `vm_space_destroy` continues to reject mode3.

There is NO new permanent per-process field, extra registry,
dynamic dispatch table or resident backend selection branch.
The relocation of the existing RAW leaf is accounted for by the
MRES loader's normal relocation and export validation. PURE remains
OFF by default, and compressed mode3 stays isolated/unlinked.

## Exact full-image size comparison against accepted v1665

KCC FULL ROOT=auto PURE=1 v1665:

* fixed `__kcore_low_end=024535` octal;
* DISK MRES 1795 resident words (including 128 BSS);
* PURE MRES 2217 resident words (zero BSS);
* `__kinit_image_end=140154` octal.

KCC FULL ROOT=auto PURE=1 v1666:

* fixed `__kcore_low_end=024353` octal: **114 words less**;
* DISK MRES 1837 resident words (including 128 BSS): +42;
* PURE MRES 2307 resident words (zero BSS): +90;
* `__kinit_image_end=140230` octal: +44 transient words.

Because exactly one backend installs, net permanent change is
**-72 words with disk swap** and **-24 words with PURE** in this
KCC full profile; a no-swap/no-PURE boot retains the whole
114-word fixed saving. Transient KINIT image high-water is a
separate metric and increases by 44 words in the auto image.

The source-built GCC lowmem TSFS PURE image changes fixed
`__kcore_low_end=024255` -> `024076` octal (**111 fewer**)
and PURE MRES 1773 -> 1863 (**90 more**). This gives a net
**21-word permanent saving** when PURE is installed, not a claim
about peak physical RAM or the entire process system.

## Validation and accepted limits

The testkit uses the genuine KCORE `proc_swap_detach` C body and
source-extracted PDP-6 `proc_swap_mres_terminal_v1666` assembler
under the original storage mode leaf. It runs disk release
success/idempotence and refused free, insufficient block count,
zero block length, retained VM APR, wrong RAW/mode3, SIDL stage
bit and RESIDENT bypass. A separate run compiles the actual
default-refusing JRST to prove that no installed MRES cannot
discard a disk allocation but can release harmless mode0 metadata.
Two independently executed unsafe mutants (default trampoline
returns success; disk MRES ignores live APR) are rejected by the
native PDP-6 oracle. The historical v1609 RAW terminal native
suite, including its saved-backing mutation, passes using the
PURE MRES target via a native patched-entry fixture. The original
v880 and v957 disk/RESIDENT host regressions pass against the
common C entry using a host-only disk service fixture. KINIT v1613
and v1665 C89 selector tests now check the third/eighth export
and remain passing. The original v1659..v1664 compression/restore
test chain is rerun unchanged in release acceptance.

An actual source-built 64K PDP-6 with three-member TSFS root,
no attached DSK/DRM AUXSTORE and PURE enabled reaches `MEM 64 K`,
`DTC OK`, `PURE LOADED`, and `INIT V1`, remaining alive until the
controlled simulator timeout. The physical boot also checks that
the KCORE map exports `proc_swap_term_v1666` but NOT the old
`pdp6_pure_detach_v1609` RAW scanner, which is present in the
PURE MRES map. This is a real boot acceptance distinct from
source-extracted native tests, but does NOT exercise live mode3.
No disk-root physical v1666 boot, 48K/32K physical acceptance or
physical peak-RAM reduction is claimed.

## Remaining work

v1667: add mode3-only compressed release behind the same PURE
terminal service, but only after fully integrating v1663 compression
and v1664 bounded FULL restore into PURE MRES and validating RUN,
EXIT and WAIT under scheduler/PI6. The disk backend must remain
unaware of mode3. A dedicated TSFS swapless-only KINIT profile
could later omit the disk-swap *candidate* as well, with an
explicit refusal on a positive validated BACKSTORE. PURE default
OFF; strict physical 32K deferred.
