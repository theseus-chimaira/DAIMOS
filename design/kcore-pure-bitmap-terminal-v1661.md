# v1661: exact compressed PURE MM terminal release, test-only

Date: 2026-11-18. Based on the pushed v1660 commit
`6a78fb79fa2fa19b4e986a7e085fb2d4c3776134`.

## Ownership contract

Storage mode3 is reserved to distinguish a compressed PURE owner
from mode2's uncompressed RAW suffix. No new process descriptor or
registry is necessary: with TRANSITION held, exactly one unpinned
`MM_TYPE_PACKED_PROCESS` extent owned by the process slot supplies
its physical base and encoded length. Derive the logical RAW length
from the saved executable text boundary and original APR size.

`proc_pure_bitmap_terminal_v1661.c` is explicitly opt-in and unlinked.
Its terminal preflight accepts only an existing non-FREE process with
stable u-area, TRANSITION, absent user APR base, matching saved PURE
backing, aligned original APR, valid omitted prefix, current mode3,
and precisely one PACKED_PROCESS owner. Require the physical encoded
extent to cover at least `ceil(raw_words/36)` masks, to be shorter
than RAW, to be unpinned, and to fit the 18-bit physical address
space. It refuses even a second owner of another PROCESS type.

Original `mm_free` is called under PI exclusion after validation.
If MM free refuses, mode and MM remain unchanged. After successful
free the same PI-excluded section clears the mode3 bits to mode0,
with no subsequent fallible operation. The caller must still own
TRANSITION and retire its own swap record/u-area; this leaf does
not silently clear either. No executable mount is needed to release
the dying process: correct backing identity and MM ownership suffice.

The terminal path deliberately does not decode data or require a
canonical bitmap (no user data are read after death). FULL restore
must independently preflight the stream before decoding.

## Native evidence and boundary correction

The native KCC/PDP-6 fixture links the EXACT original assembler
`mm_delete`, `mm_find_base` and `mm_free` leaves, extracted from the
source, rather than emulating MM release in C. It verifies real
release of a 161-word compressed owner representing 3,456 RAW words
and zeroing of only the mode pair, while original backing/u-area and
TRANSITION remain owned by the caller. Eighteen refusal cases check
the unchanged mode/extent table: RAW mode, wrong owner, duplicate,
pin, RAW-size, missing mask capacity, backing mismatch, live APR,
missing transition, MM-free refusal, wrong type, FREE process,
missing extent, physical overflow, descriptor-capacity corruption
and negative extent count, plus invalid owner type and an additional
reserved-type owner. A separate MODULE owner with the same numeric
ID is correctly preserved. Exact last-word physical boundary
`01000000` octal also succeeds. An independently assembled unsafe
duplicate-owner mutant is executed and rejected by the native test.

The standalone terminal C object is 314 TEXT words, zero DATA/BSS;
this is not installed KCORE or MRES overhead. Source-matched tests
use a controlled PI-state stub with an original MM-free ASM leaf:
the actual hardware PI instruction and complete swap finalizer are
not yet exercised in the physical guest.

The review also found a one-past-last-word check in the unlinked
v1660 bitmap preflight that rejected a physically legal extent
ending exactly at `01000000` octal. The v1661 source corrects both
the decoder and terminal checks to accept `base + length <= 01000000`.
The original v1660 negative decoding regression is rerun.

## What is NOT enabled

Permanent `proc_storage_runtime_v30.s` still rejects mode3 and
original `pdp6_pure_detach_v1609` still handles only RAW mode2.
The new leaf is not wired into `proc_swap_detach`. It cannot be
installed until a fully failure-atomic compressed DROP transaction
and bounded FULL restore are available, including reclaim/scheduler
failure-state handling. No compressed process is made runnable or
released in production. No physical 48K/64K RAM saving is claimed.
PURE default stays OFF; strict 32K size acceptance remains deferred.
