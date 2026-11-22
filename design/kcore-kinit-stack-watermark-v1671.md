# v1671: actual KINIT stack high-water on physical PDP-6 48K/64K

Date: 2026-11-18. Exact accepted DAIMOS parent v1670:
`c3867558cd05d0d8be6a179529342cfcda1075f6`.

## Why this diagnostic was run

The v1670 physical MM event trace established that a dedicated
TSFS swapless-only KINIT saves 1877 transient GCC lowmem words
at the highest sampled allocator event boundary, while the
general/strict configurations have identical free MM at the
first user VM entry. This does not bound KINIT's native
pushdown-list use: the separate reserved stack could run deep
even when managed MM arenas have free words. v1671 therefore
measures the original KINIT physical stack and compares the
actual used depth to its `0400` octal (256-word) reserve.

The existing v639/v655 `KINIT_STACK_WATERMARK=1` implementation
fills unused upper stack with the fixed octal sentinel
`525252525252`, then scans for the highest touched index after
KINIT completion. This flag is for **diagnostic images only**;
normal production KINIT and fixed KCORE remain unchanged. Its
single `kinit_stack_highwater` word increases fixed
`__kcore_low_end` from octal 024076 to 024077 in diagnostic
GCC lowmem images. The added sentinel initializer/scanner is
in transient KINIT; the reserve itself remains unchanged.

## Source-matched real physical test

The testkit's `pure-kinit-stack-v1671/run-physical-v1671.sh`
builds or uses two independent GCC lowmem TSFS/PURE images:
general ROOT=tape and strict v1668 swapless-only ROOT=tape.
Bitmap stage v1667 is disabled in both. The host simulator
runs each image with actual PDP-6 48K and 64K physical core,
separate copies of the four DECtape media, no attached DSK/DRM
swap backing, and a controlled stop at the FIRST real
`vm_enter_initial_user` handoff.

Every breakpoint comes from the exact linked symbols in the
corresponding image, not guessed offsets. The test requires
exactly one original `kinit_stack_watermark_begin` execution
and a successful PURE-selected root. At the final handoff,
SIMH examines all 256 actual physical stack words starting at
`__kinit_image_end`, the resident high-water counter, and the
patched permanent resident immediate. `measure-v1671.py`
rejects missing or duplicate physical words, incorrect
simulator core size, an uninitialized stack, an inconsistent
stored/physical maximum or any overrun of the upper reserve.

| Physical core | General highest touched offset | Strict offset | Upper untouched words |
| --- | ---: | ---: | ---: |
| 48K | 129 | 129 | 126 |
| 64K | 129 | 129 | 126 |

The highest modified stack index is offset **129 decimal**
relative to the start of its stack reserve, meaning **130
physical stack words** through the highest touched address.
All four native watermark values agree with an independent
scan of the complete 256-word stack. Exactly **126 upper words**
remain intact sentinel values. Thus the strict package pruning
does NOT make the exercised TSFS/PURE boot stack any deeper.
The diagnostic source-matched linked KINIT image end still
differs by 1877 words between general and strict images.

The diagnostic-only one-word KCORE increase also yields
low-resident RAM 48K 17726 words and 64K 17742 words, exactly
one above normal v1670 (17725 and 17741 respectively). There
is no production KCORE size change or new per-process field.

## Failure-proof tests and limitations

Four independently executed unsafe evidence mutations are
rejected: a forged original high-water counter, a corrupted
last stack word that destroys the untouched upper sentinel,
removal of the original sentinel initialization breakpoint,
and a falsified postprocessed high-water report. The normal
source-matched comparator also refuses unequal diagnostic
resident KCORE size or impossible stack-reserve accounting.

This result is valid for the tested TSFS/PURE successful path,
not necessarily for all other root modes, KINIT fatal error
paths or unforeseen nested calls. **Do not reduce the default
`KINIT_STACK_RESERVE_WORDS=0400` based on this workload alone.**
The difference between highest-touched and reserve addresses
is physical headroom, not an automatic permanent memory saving.
Next acceptance includes fail-stop/error-path stack bounds,
post-user RUN/EXIT/WAIT/PI6 process-memory pressure at 48K/64K,
real bitmap-mode3 workloads, and strict physical 32K geometry.
No DAIMOS production source code was changed for v1671.
