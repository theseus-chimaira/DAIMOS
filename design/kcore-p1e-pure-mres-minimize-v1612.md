# P1-E v1612: measured PURE MRES footprint reduction

Date: 2026-11-16
Baseline: committed v1611 opt-in PDP-6 PURE MRES, 2275 KCC words.
Status: source-linked native PDP-6 tests PASS. The experimental build
remains disabled by default; a physical no-AUXST1 guest and positive
whole-system RAM proof are still required.

## Accepted changes

The existing optional PURE service retains exactly two MRES exports,
and the fixed scheduler trampolines, v1381 zero-copy DROP, v1384
prefix restoration, v1391 capacity-bounded FULL fallback, storage
map and v1609 terminal teardown remain unchanged.

1. Source-selected PDP-6 policy
   `proc_pure_service_pdp6_v1612.c` replaces the v1611 service object.
   An MM owner proof now handles resident (explicit nonzero VM base)
   and PACKED (zero APR base; physical base derived from the one MM
   descriptor) in **one** strict descriptor scan. It rejects a
   missing/zero base, nonmatching base when resident, wrong MM type,
   wrong owner, pins, wrong word count or a second process extent.
   A failed restore cannot release TRANSITION without this proof.
   The previous PACKED retry check scanned the MM extent list twice.
2. The existing `p10fold` performs a relocation-safe *zero-cost*
   fold inside the removable PURE module. It merges an identical
   49-word sequence in the v1381 DROP and v1391 FULL objects.
   The generated v1612 plan uses minimum run 20 and only accepts
   the zero-cost equivalence group: no extra JRST or hot-path branch
   is introduced. The fold plan is a generated build artifact,
   never a handwritten assembly patch and never a permanent KCORE
   optimization.
3. The pure service source remains selected through the current
   PDP-6 architecture manifest. This does not add a KA10/KI10
   backend or any speculative CPU dispatch; a different physical
   memory architecture must replace the MM-specific implementation.

## Exact KCC size measurements

The same compiler and KCORE map were used to compare linked MRES
objects under `ROOT=tape BOOT_PROFILE=full COMPILER=kcc`.
All figures below are PDP-10 *words*, not bytes:

| Item | v1611 | v1612 |
|---|---:|---:|
| KCC PURE policy object TEXT | 412 | 382 |
| KCC v1381 DROP object TEXT | 443 | 443 |
| KCC v1391 FULL object TEXT | 520 | 520 |
| KCC zero-cost inter-object fold | 0 | -49 |
| **Installed KCC PURE MRES** | **2275** | **2196** |
| KCC reduction | baseline | **-79 (-3.47%)** |
| **Installed GCC lowmem PURE MRES** | **1864** | **1758** |
| GCC reduction | baseline | **-106 (-5.69%)** |
| New MRES or KCORE BSS | 0 | 0 |
| New per-process words | 0 | 0 |
| Fixed KCORE DXR | v1611 image | **byte-identical** |

Fixed KCC `__kcore_low_end` remains octal `024534`;
GCC lowmem fixed KCORE remains byte-identical to v1611
at `024254` octal. GCC's compiler-specific fold plan
also has only zero-cost equivalences.
Neither the historical DISK MRES nor default KINIT uses the
experimental PURE service.

### Rejected source-sharing experiment

The shared lifecycle, immutable-source admission and MM-extent
checks in DROP and bounded FULL are indeed duplicates. However,
consolidating them into a standalone KCC object caused extra
call/argument-handling code: DROP shrank from 443 to 312,
FULL from 520 to 388, but the new helper cost 344 words.
The resulting MRES was **2356 words**, 81 more than v1611
and 160 more than the accepted v1612. The idea was
semantically sound; it was rejected on measured RAM size,
not because of test failure. That experiment is absent
from the committed source tree. No alternative per-process
registry or extra C abstraction was introduced.

## Test evidence and constraints

DAIMOS-testkit `pure-service-v1612/run-v1612.sh` executes
original KCC PDP-6 source-linked DROP, prefix/FULL
restore, failed BACKSTORE admission, missing executable
source and failed FULL loader, the new v1612 policy and
the actual MM/type/mode machine code. Additional corrupt
duplicate-owner and pinned-PACKED cases verify refusal
without READY_ONCE or unsafe TRANSITION release. The
existing unsafe READY_ONCE source mutation is still
rejected by executed PDP-6 assertions. The complete
v1611 native service test still runs against the unchanged
historical policy object.

The v1612 `boot-mode-v1612.sh` checks the same KCC build
directory across OFF -> ON -> OFF, requires the v1612
package and both exports in the opt-in image, verifies
that a generated fold plan exists, pins the 2196-word
installed footprint, and compares fixed KCORE bytes across
all three states. The independent GCC lowmem size test
pins the 1758-word installed PURE module and `024254`
octal fixed-KCORE boundary. Together they catch stale
build configuration, compiler-size regressions or an
accidental permanent-RAM dependency.

No full physical PDP-6 no-AUXST1 boot, PI6 scheduler
acceptance, immutable executable lifetime/unmount
concurrency, peak RAM/fragmentation proof, or positive
net RAM saving has been established. In particular,
nine 256-word omitted prefixes only reclaim 2304 words:
barely more than the 2196-word installed KCC MRES;
seven such prefixes barely exceed the GCC 1758-word
module. Neither includes transient allocations. Keep
`DAIMOS_PURE_MRES_ENABLED=0` in normal builds until
real physical/net-RAM acceptance. Compression remains
deferred until uncompressed RAW P1-E is complete.
