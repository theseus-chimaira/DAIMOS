# MonitorFS extraction into resident MRES: implementation boundary v13

Status: design analysis; no runtime ABI changed.

## Measured starting point

The GCC 32K fixed KCORE link ends at 023360 octal. The existing
monitorfs_runtime.dobj is 991 text + 39 data = 1030 decimal words before
folding. Its entire size is NOT removable: VFS has direct entry points and
runtime MonitorFS data is shared with the filesystem namespace.

## Current call and initialization paths

`fs/vfs.s` calls `mfsdev_lookup`, `mfsproc_lookup`, `mfsdev_readdir`,
`mfsproc_readdir`, `mfsdev_stat`, `mfsproc_stat`, `mfsproc_read_words`,
`mfsdev_readchar`, and `mfsproc_format_slot`. Domain operations are reached
through the process view. `module_table.s` currently records
`mfsdev_minit,,0` (no MRES package); `mfsdev_minit` in `module_minit.c`
publishes present device names after driver registration.

## Required design before moving code

1. Preserve the VFS ABI and stable MonitorFS vnode encoding.
2. Introduce a compact resident interface consisting of the existing VFS
   entry symbols forwarding to a boot-installed service table, or their
   equivalent direct patchable calls. Never dereference an absent service.
3. Put cold namespace enumeration, formatted statistics and domain reports
   in a nonremovable MonitorFS MRES package. The package must remain pinned
   while any VFS operation or open vnode can reach its code. Removable
   modules are not appropriate for a filesystem with persistent open handles.
4. Retain immutable/small shared MonitorFS data where lifetime requires it.
   Do not move process scheduler, exit/wait/fork or core MM into the package.
5. Extend the MINIT table entry from `mfsdev_minit,,0` to a real MRES package
   only after the package loader, jump patching and all early VFS consumers
   can be verified for the ordering used by both 32K and full profiles.
6. Test initial boot, the MonitorFS /DEV /PROC /MONITOR views, open handles,
   missing devices, and memory-pressure behavior in daimos-testkit.

## Acceptance

Compare fully linked GCC `__kcore_low_end` before and after extraction; the
object byte count alone is not a saving. Also report added MRES footprint,
fixed dispatch words and total permanently occupied core. No kernel overlays,
no hot-path process lifecycle indirection, and no silent feature removal.

The next patch should establish the dispatch/lifetime contract before any
MonitorFS functions are moved. Keeping the implementation unchanged until
that contract is tested avoids unsafe VFS calls into unloaded code.

## Baseline comparison (v18)

A new GCC lowmem build with resident MonitorFS (`MONITORFS_MRES_ENABLED=0`)
also reaches INIT V1 but not LOGIN in the same testkit simulator harness.
This matches the v17 MRES-enabled failure and therefore does not establish
a MonitorFS-specific regression. The fixed resident baseline still links at
023360 octal. MRES activation remains opt-in pending independent LOGIN
diagnosis and MonitorFS VFS acceptance tests.

## Isolated package link probe (v15)

The existing MonitorFS assembler has now been linked experimentally as a
relocatable MRES package against the GCC KCORE map. Its nine VFS-facing
exports were renamed to avoid collision with the current resident exports.
Use tools/host/monitorfs-mres-probe-v15.py to generate this variant from the
single source of truth; do not maintain a divergent copy. DLINK reports
1030 initialized words and no BSS. Installation, KCORE removal and functional
tests are still required; this result alone saves no RAM.

## Experimental integrated GCC build (v16)

MONITORFS_MRES_ENABLED=1 installs the MonitorFS MRES at the final MINIT
table entry and patches nine KCORE-resident VFS entry jumps to its exports.
The default resident implementation is unchanged until boot validation.

The GCC fixed KCORE endpoint drops from 023360 to 021375 octal, while the
MRES adds 1030 resident words. Consequently the change saves 1011 words
of low-core address space but increases total permanent RAM by about 19
words. GCC KINIT linking and the profile-aware permanent-size report pass.
Runtime/boot testing is blocked by the missing external sixmd-check tool
during native DAS image production. Do not claim functional acceptance yet.

## GCC 32K boot and lifetime verification (v17)

The missing prefix utilities were installed from the maintained toolchain
sources, allowing complete image generation. A simulator boot with fresh
media now reaches INIT V1 with MONITORFS_MRES_ENABLED=1. KINIT must install
the MonitorFS MRES early, immediately after CLK: a later D6FS MINIT dynamic
allocation breaks the mandatory contiguous placement invariant. Device-name
publication still happens after all driver MINIT routines, independently of
the early package installation. Both MINIT/package ordering and dispatch
symbol address generation are now corrected.

The GCC KCORE endpoint remains 021375 octal. The package accounts for 1030
permanent MRES words and therefore does not represent a net RAM saving.
Repeated make mode-switch checks (0 -> 1) produce the correct KCORE endpoint
on each build: 023360 for resident MonitorFS and 021375 for MRES.

The DAIMOS testkit supplies a dedicated GCC lowmem fresh-image acceptance
target and five VFS probes. Both 32K and 64K lowmem simulator runs reached
INIT but did not reach LOGIN within the harness timeout, so the VFS probes
were not yet executed. Until that blocker is resolved, keep MRES opt-in and
do not declare full MonitorFS regression acceptance.
