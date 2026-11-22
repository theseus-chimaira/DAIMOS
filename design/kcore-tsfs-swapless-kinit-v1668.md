# v1668: strict TSFS swapless-only KINIT without DISK SWAP candidate

Date: 2026-11-18. Exact DAIMOS parent v1667:
`9d9d310526a363caf43430e18a84b5720cc08872`.

## Purpose

General ROOT=auto cannot safely omit either process-backing
candidate: KINIT selects its actual root class, then checks
validated BACKSTORE. However, a dedicated TSFS root configured
to NEVER use disk swap should not carry the unused disk-swap
MRES in its transient KINIT boot image. v1665 already installed
only ONE permanent MRES; v1666 moved disk/RAW terminal release
out of fixed KCORE; v1667 staged compressed mode3 exclusively in
PURE MRES. v1668 removes the unused DISK SWAP candidate from
the specialized ROOT=tape image, not from general ROOT=auto.

## Build contract and physical fail-closed selection

Set `ROOT=tape DAIMOS_PURE_MRES_ENABLED=1` and explicitly set
`DAIMOS_SWAPLESS_ONLY_V1668=1`. The flag defaults to zero and
accepts only 0 or 1. Non-TSFS roots, general ROOT=auto and
PURE-disabled images reject the strict flag at Make configuration
time. The transient kernel C source also requires DTC support,
compiled-out D6FS and enabled PURE when this flag is 1. No
silent fallback is possible. `DAIMOS_PBM_V1667=1` remains a
separate staged, test-only PURE codec option and defaults OFF.

The KINIT module table's SWAP MINIT entry uses a **zero** MRES
package address and does not reference
`proc_swap_mres_package_v481` when the strict option is on.
`MRES_PACKAGE_OBJS` excludes `swapv481-mres.dobj` from both
lowmem and full profiles. The KINIT selector therefore cannot
install DISK SWAP and must never enter a disk-swap export path.
The regular `ROOT=tape`, `ROOT=auto`, disk and drum images retain
their historical candidate selection and disk-swap exports.

In the strict image, `proc_swap_minit_v481()` checks the actual
selected root after physical driver MINITs. Only DTC/TSFS is
accepted. It calls the existing read-only AUXST1 descriptor
discovery, which positively validates every claimed BACKSTORE
region before reporting its size. No descriptor, an invalid
descriptor or zero BACKSTORE selects the ordinary opt-in PURE
MRES with three KINIT-patched scheduler/terminal exports. A
**positively validated nonzero BACKSTORE** instead calls the
original `minit_fatal(SWAP)` HALT BEFORE any PURE installation,
disk-swap publication or root transition. Unexpected/unresolved
root classes also fail closed before probing or installing.

Full-profile `auxstore_minit()` runs a second discovery for
LOGSTORE. A positive BACKSTORE found on this later probe is
again fatal, before LOGSTORE setup. Zero-BACKSTORE LOGSTORE
remains supported and is not confused with process swap. The
physical DSK/DRM drivers and their independent MRES packages
remain available for auxiliary LOGSTORE and other services;
only the DISK SWAP memory-management package is omitted.

The kernel's fixed `proc_swap_reclaim`, `proc_swap_service_one`,
`proc_swap_term_v1666` and disk `backstore_*` default-refusing
entries remain unchanged. The installed PURE MRES is unchanged.
No new process field, compatibility backend selector, MM owner
registry or permanent KCORE data is created.

## Source-linked, linker and physical evidence

The testkit `pure-swapless-kinit-v1668/run-v1668.sh` extracts
the genuine `proc_swap_minit_v481()` and `auxstore_minit()` C
from production sources and compiles strict host C89. Absent,
zero and stale-negative AUXST1 cases install only PURE; positive
validated BACKSTORE, wrong root and a second positive discovery
are fatal. The tests check all three PURE trampoline patches,
that no DISK service is installed, and that no BACKSTORE exports
are published. Two deliberately unsafe compiled source mutants
(skip first positive fatal; skip rediscovery fatal) are executed
and rejected. The real top-level Makefile candidate policy is
tested for strict tape, normal tape/auto/disk/drum, optional
v1667 staging and incompatible ROOT/PURE/flag cases. Original
v1665/v1613 normal-root selector regression passes.

The testkit `run-physical-v1668.sh` boots the SAME source-built
64K GCC lowmem PDP-6 TSFS root image twice. With all DSK/DRM
media detached, the actual kernel reaches `MEM 64 K`, `DTC OK`,
`PURE LOADED`, `INIT V1` and survives until a controlled timeout.
With the separately generated, validated DRM0 AUXST1 BACKSTORE
image attached, KINIT prints `SWAP FAIL` and executes an actual
HALT (observed PC 065774 octal), without PURE or INIT. The
positive result is a guest-device probe, not a mocked fixture.

### Source-matched GCC lowmem link-size comparison

| Measure | General tape / PURE | Strict swapless tape / PURE |
| --- | ---: | ---: |
| KCORE fixed end (octal) | 024076 | 024076 |
| Installed PURE MRES (words) | 1863 | 1863 |
| DISK SWAP candidate in KINIT | 1837 words | **absent** |
| `__kinit_image_end` (octal) | 113732 | 110205 |

**Exactly 1877 transient image words saved**, including the
1837-word disk-swap candidate and a net 40-word change in KINIT
selector code, table and packaging overhead (not separately
attributed). The KINIT module table has no disk-swap package
symbol relocation, and the strict linked map lacks the package
entirely. The companion `run-size-v1668.sh` compares independently
linked, source-matched images and rejects any change to fixed
KCORE or the installed PURE MRES. This is a boot image/end-address
reduction, **not** 1877 new permanent resident process words or
a measured whole-system physical high-water reduction.

The independently built source-matched **KCC FULL** ROOT=tape
profile also preserves fixed `__kcore_low_end=024353` octal
and installed PURE MRES **2307** words. General tape KINIT end
is `131475` octal, strict swapless KINIT end is `125775`
octal: **1856 transient words saved**. Both supported compiler
profiles pass linked-package ownership and size tests.

## Remaining work

Complete 48K/64K physical boot transient peak and MM owner/pin
accounting with comparable guest workloads, and strict 32K
physical acceptance only after resolving the known KINIT peak
geometry. Source-built full-profile KCC link PASS; physical
full KCC disk/auto-root regressions remain separate gates.
Live bitmap mode3 remains disabled: compression/reclaim, EXIT,
WAIT, RUN/PI6 and physical net RAM must be accepted before
production mode3 admission. The strict flag does not enable
compression; ordinary PURE and bitmap compression both default
OFF. Keep existing normal disk/drum swap implementation intact.
