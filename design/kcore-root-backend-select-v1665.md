# v1665: select one swap backend after actual root discovery

Date: 2026-11-18. Parent DAIMOS v1664:
`894615ddd040bf24631c5a8eb733051a48954f1e`.

## Scope and defect

The previous MINIT table invoked `proc_swap_minit_v481` before
`root_select_minit`, forcing a compile-time D6FS test as proxy
for the actual root. With `ROOT=auto`, D6FS can be compiled even
when KINIT selects a DTC/TSFS root without a validated BACKSTORE.
The old compile-time condition therefore excluded PURE from that
valid configuration. Worse, early selection could not distinguish
auxiliary disk/drum hardware from a selected disk/drum root.

v1665 moves `root_select_minit` after physical driver MINITs and
before the swap MINIT. It reads the existing `root_select_class()`
result once; no new resident selector, metadata, mutable mode or
per-call branch is introduced. An unresolved root installs no
swap service. With selected DTC, KINIT calls the existing
`auxstore_boot_discover()`; absent/invalid AUXST1 or validated
zero BACKSTORE capacity enables the optional PURE service, and
only a positively validated nonzero BACKSTORE installs the
unchanged disk-swap MRES. With a selected DSK/DRM root, the
historical disk-swap MRES installs without a TSFS-only probe.
Unsupported root classes and root classes excluded from the
compiled image fail closed. No physical device is formatted or
modified by selection.

The unchanged `mres_install()` relocates exactly ONE selected
MRES into permanent memory and patches the two existing resident
scheduler trampolines. The disk case still patches its original
seven exports; there is no RAM-resident dual implementation.

## Build-time candidate ownership

`MRES_PACKAGE_OBJS` now includes the opt-in PURE package when
both `DAIMOS_PURE_MRES_ENABLED=1` and `KINIT_TAPE_BOOT=1`, for
either `ROOT=tape` or `ROOT=auto`. Dedicated ROOT=disk/drum
builds omit the PURE candidate even if the opt-in flag is 1.
The default flag remains zero. The disk candidate remains linked
in supported configurations: even a dedicated tape-root image
may discover a positively validated AUXST1 BACKSTORE and must
retain a working disk-swap path. Consequently ROOT=auto KINIT
contains BOTH *transient candidates* until the selected root and
BACKSTORE capacity are known. Only the chosen candidate becomes
resident. A strictly swapless-only image excluding the disk
candidate is a separate future build profile, and must reject
positive BACKSTORE rather than silently calling missing exports.

This is transient KINIT policy only: no fixed-KCORE or existing
RAW swap/PURE implementation source was changed. It also does not
admit compressed mode3; v1659..v1664 remain opt-in/unlinked.

## Testing

`daimos-testkit/tests/validation/pure-boot-select-v1665/`
compiles the actual extracted `proc_swap_minit_v481()` as strict
host C89 with isolated discovery/install witnesses in six build
combinations. The oracle checks selected root AUTO, DTC, DSK,
DRM; missing AUXST1, validated zero/positive BACKSTORE, stale
nonzero capacity when discovery fails, exact PURE/disk exports,
the absent-backend case and no undesired installation. Parsing
the real module table proves driver -> root -> swap -> AUXSTORE
ordering. Executed unsafe inverse-order and wrong-root mutations
are rejected. The original v1613 KINIT selection suite, including
its unsafe AUXST1 mutation, also passes. The real PDP-6 Makefile
was evaluated under ROOT=auto/tape/disk/drum and PURE on/off for
six linked-package checks; all pass.

The complete KCC FULL ROOT=auto PURE=1 source-built image links
both candidates successfully. Its KINIT symbol `__kinit_image_end`
is octal `140154`, fixed `__kcore_low_end` octal `024535`, with
both `proc_swap_mres_package_v481` and
`proc_pure_mres_package_v1612` present. Current standalone MRES
section reports: KCC FULL PURE 2,217 resident words; disk swap
1,795 words including 128 BSS. These are candidates, not additive
residency. The dedicated TSFS lowmem GCC build reports fixed
`__kcore_low_end=024255`, KINIT end `113655` octal and PURE MRES
1,773 words. Do not compare these numbers to older v1613 sizes as
if the intervening v1614..v1664 source/toolchain was unchanged.

The 64K source-built PDP-6 TSFS physical boot with DSK/DRM media
attachments removed reaches `MEM 64 K`, `DTC OK`, `PURE LOADED`
and `INIT V1`, remaining alive until a controlled simulator
timeout. There was no fatal `?RT` or INIT RUN/RESPAWN failure.
The original v1613 smoke test first halted at a stale hardcoded
KCORE bound 024254 before launching SIMH; the new v1665 physical
runner uses the current 024255 baseline. Both the failed old
assertion and the successful physical run are retained as evidence.
An auto-root *physical* DTC boot and positive-backstore physical
switch test have not yet been run; the auto-root KCC **link** and
source-linked selection tests passed, not a full auto-root guest.

## Next: remove remaining unused compatibility code

Before enabling compressed mode3 in production, move compressed
terminal release behind a boot-patched memory-backend entry so
fixed `proc_swap_detach` cannot accumulate two different owner
scanners. Then compose v1659..v1664 only into PURE MRES, with
disk-swap code unchanged. Finally implement a dedicated
swapless-only image that omits the disk candidate and rejects
BACKSTORE-positive boot configurations. Compare transient KINIT,
installed MRES, fixed KCORE and physical peak RAM independently.
Strict 32K memory acceptance deferred; PURE defaults OFF.
