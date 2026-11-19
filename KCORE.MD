# DAIMOS KCORE - architecture, accepted boundaries and RAM state (v1676)

Updated 2026-11-19. This is the **current KCORE engineering reference**;
it is not the active task list and not a chronological experiment diary.
The authoritative open backlog is `TODO.MD`, KCORE sections P1/P3 and
relevant P0/P4. The comprehensive, source-linked results are in
`PROJECT-STATE.MD`, `SIZEOPT.MD`, the versioned `design/` reports and
`pdp10-doc/KERNEL/`. This document records only stable design boundaries,
accepted release contracts, validated measurements and their limits.

## Historical record and scope of this cleanup

The original 11,181-line KCORE chronicle, **including the existing local
edits before this cleanup**, remains recoverable without a separate
handoff artifact:

    git show 738c79c5:KCORE.MD

It contains 541 dated milestone headings, intermediate compiler/guest
experiments, implementation measurements and obsolete open checkboxes.
The accepted results remain in Git, `PROJECT-STATE.MD` and individual
versioned design reports. This cleanup changes **documentation only**;
it does not change a kernel object, simulator, ABI or test result.

There are two distinct development states:

- **`main`** is the accepted PDP-6 KCORE baseline through the v1604
  cross-target PURE architecture review, plus later independent toolchain
  work. Its linked default kernel has not merged v1611-v1674 PURE.
- **`kcore-pure-reclaim-pressure-v1673-20261118`** is the unmerged
  PDP-6 PURE development branch. Its accepted v1674 RAW 48K physical
  work is recorded at `ae03d40f` and in
  `design/kcore-pure-runtime-highwater-48k-v1674.md` on that branch.
  A successful feature-branch guest is not evidence that `main` boots
  the same configuration or that KA10 and mode3 are accepted.

## KCORE fixed-residency and machine-specific boundaries

- KCORE holds mandatory, immediately callable kernel/interrupt, process,
  syscall, VM/MM, device and VFS dispatch. Optional policy and provider
  engines belong in MRES where installation is boot-selected and safe.
  A mandatory syscall such as EXEC/RUN is not made removable to achieve
  a cosmetic fixed-KCORE size reduction.
- Physical PDP-6 MM and user address mapping are authoritative. A user
  process requires a complete, aligned APR image before publication as
  runnable. The PDP-6 relocation/protection register does not define a
  KA10 hardware-neutral ABI; the optional KT10A high/low segments require
  a separate target-specific MM/APR contract.
- Process owner/type/size/pin records in original physical MM are the
  authoritative memory state. A PACKED_PROCESS or in-flight transition
  must not be made runnable or freed by reconstructing an owner from an
  unverified pointer or a stale array slot. The MM count determines which
  descriptors are live; stale contents beyond it are not allocations.
- Reclamation, restore, EXEC replacement, failed RUN and terminal release
  require failure-atomic ownership: publish RESIDENT only after complete
  APR contents and cleanup, preserve retained source on uncertain failure,
  and never silently unpin/free a unique owner that is still in use.
- The original PI6 scheduler and slot-0 service protocol own wakeups,
  requests and READY_ONCE publication. Physical fault testing must use
  reachable interrupt/service interleavings, not assume that a user
  process runs concurrently inside a serialized slot-0 FULL restore.
- Boot-selected process table and VFS mount capacities use packed and
  dynamically allocated metadata where the source supports it. Avoid
  introducing second registries, permanently resident scratch slabs or
  per-process fields unless original ownership cannot express a feature.
  Source reference: `system/kernel/proc/`, `system/kernel/mm/`,
  `system/kernel/core/`, and `system/kernel/ipc/`.

## Accepted mainline feature boundaries

| Gate | Accepted contract | Detailed evidence |
| --- | --- | --- |
| P0-A | Critical ROOT/BACKSTORE failure correctly stops the original KCORE, with source-linked real DSK/DRM and TSFS error paths. | `design/kcore-p0a-failstop-acceptance-v1548.md` |
| P0-B | **CLOSED v1575** for trusted-operator DTVOL2 durability/quarantine. The operator is responsible for reel retention/exchange; do not revive the rejected hardware-interlock requirement. | `design/kcore-dtfs-operator-discard-v1575.md` |
| P1-C | **CLOSED v1603** mixed physical swap/BCACHE/LOGSTORE/MEMFS and independent owner/PI6/PI2 matrix. Failed critical BACKSTORE I/O fails closed; cache is clean-only WRITE-through. | `design/kcore-p1c-final-physical-acceptance-v1603.md` |
| P1-D | **CLOSED v1601** dynamic FSINFO/controller/root inventory, including genuinely absent controllers, high mount slots and correct physical capacity. A 26-slot ABI is not proof of 26 attached drives. | `design/kcore-p1d-final-controller-matrix-v1601.md` |
| P1-E | Architecture boundary **reviewed v1604**, not merged or cross-target accepted. PDP-6 RAW implementation lives on a separate branch. | `design/kcore-p1e-pdp6-ka10-architecture-review-v1604.md` |

Accepted native/physical cases are **version- and source-qualified**;
a historical note saying P0-B, P1-C or P1-D was OPEN does not reopen
these completed feature contracts. New tests may detect regressions,
but are not authorization to recreate superseded design layers.

The production codec is the canonical v1421/v1464 D6 block
format. D6LZ36 was removed from the active Stage-1, runtime and host
producer paths; `system/kernel/core/D6CODEC-INTEGRATION-TODO-v054.md`
contains an older staged plan and a newer production decision. Do not
reintroduce dual-format runtime fallback. Restricted privileged MEMFS
XIP is production-integrated within the v1469-v1485 accepted supported
profiles; this does not assert that arbitrary or writable DXR2 user
images can execute zero-copy.

## PDP-6 PURE development branch: what is actually proven

On the unmerged branch, RAW PURE is an opt-in **swapless** process-storage
mechanism. A sleeping/stopped noncurrent process can relinquish its
immutable PURE text prefix while an exact uncompressed mutable suffix
remains a physically owned PACKED_PROCESS MM extent. A later scheduler
service restores the complete APR image from authenticated immutable
backing and the retained mutable suffix, then completes publication.
Disk-backed swap continues to use its independent existing contract.

- The opt-in `DAIMOS_PURE_MRES_ENABLED=1` path, shared immutable source
  guard, RAM-only owner, bounded FULL fallback and terminal cleanup
  have staged native tests and accepted PDP-6 physical guest evidence.
- The strict `DAIMOS_SWAPLESS_ONLY_V1668=1` TSFS/PURE boot omits the
  unused DISK SWAP MRES candidate from KINIT, without removing the
  DSK/DRM LOGSTORE controller services. Its 48K/64K original KINIT
  experiments show **1,877 fewer transient image words** for matched
  GCC lowmem configurations; installed fixed KCORE/PURE and steady
  post-user free memory are unchanged.
- The KINIT diagnostic stack watermark touched index 129 of the
  256-word reserve in tested 48K/64K general/strict boots. This is not
  sufficient to reduce the reserve before abnormal error/root/fatal
  paths are measured; diagnostic instrumentation is OFF by default.
- v1672 sampled 48K/64K RUN/EXIT/WAIT and SYS_MEMINFO/PROCINFO zombie
  accounting. Its 48K four-child MM free was
  27,441 -> 18,193 -> 27,441; 64K seven-child was
  43,793 -> 27,609 -> 43,793. These are checkpoint values, not
  continuous post-user high-water.
- v1673 physically accepted nine RAW PACKED children under APR
  fragmentation, a refused tenth RUN, 128-word prefix savings per
  child and exact owner/pin recovery on EXIT/WAIT, for source-matched
  48K general/strict PDP-6 boots.

### v1674 48K physical committed-MM high-water: PASS

The 2026-11-19 general and strict swapless 48K PDP-6 tests used
**identical KCORE/PURE MRES binaries**, a deterministic PIPE-controlled
nine-child fixture, and 71 original physical MM write watches. Each
source-linked independent trace replayed 3,122 physical MM descriptor
writes across A-to-B-to-C and matched its original physical snapshots.

| 48K physical MM observation | General | Strict |
| --- | ---: | ---: |
| Free before children (A) | 27,339 | 27,339 |
| Free under nine-owner pressure (B) | 2,692 | 2,692 |
| Free after complete WAIT (C) | 27,339 | 27,339 |
| Lowest free **committed** MM count | **1,255** | **1,255** |
| Highest nonfree of 49,152 physical words | **47,897** | **47,897** |
| Source-linked physical MM write events | 3,122 | 3,122 |
| Original extent-count commits | 194 | 194 |
| In-place BRK span changes | 5 | 5 |
| PURE 128-word drop / restore changes | 9 / 9 | 9 / 9 |

The original v1673 owner/PROCINFO/MEMINFO verifier, v1674 trace
verifier, paired general/strict comparison and five physical
write-watch mutations plus forged-report rejection all PASS on both
source-matched recordings. The measured low point is a **committed-MM
event high-water** for this specific workload, not a timing-neutral
instruction/scheduler worst-case bound. SIMH write watches can alter
clock interleavings; intermediate descriptor shifts are not genuine
committed memory consumers. Full detail and binary identities:
`design/kcore-pure-runtime-highwater-48k-v1674.md` on the PURE branch,
and `daimos-testkit/tests/validation/pure-runtime-highwater-v1674/`.

### Feature exclusions that remain in the TODO

- PI6/scheduler physical fault pressure during simultaneous RAW owners,
  including uncertain MM/source errors, safe TRANSITION/pin recovery,
  READY_ONCE publication and final reaping, remains unaccepted.
- ROOT=auto, disk/drum BACKSTORE and source-matched KCC FULL regression
  with the newer PURE path remains unaccepted, including the distinction
  between a truly swapless tape boot and a disk-backed system.
- Mode3 bitmap compression has staged bounded encoder/restore/terminal
  components but **no accepted live scheduler compression admission**,
  in-place commit/release or full compressed RUN/EXIT/WAIT/PI6.
  `DAIMOS_PBM_V1667=0` remains the production default.
- The KA10 backend and optional KT10A high segment are not validated by
  PDP-6 success. Mainline integration is a separate controlled step.

## Boot and memory targets

- The current target is **48K physical words for a complete boot**;
  physical 64K/96K/128K/192K/256K profiles are accepted only within
  their individual release/test contracts. Do not derive a broad
  minimum from an isolated native fixture or larger-memory boot.
- **32K kernel-only correctness tests are mandatory.** Full 32K
  ROOT/LOGIN/DSH boot and an 8K-address permanent KCORE shrink are
  deferred, not required for the current functional feature freeze.
- Measure permanent KCORE, installed MRES, KINIT transient image,
  original MM owner extents, APR alignment/rounding, process u-area,
  startup stack, and largest physical free extent separately. A smaller
  KINIT package does not necessarily yield more post-user free RAM.
- Historical linked sizes in the old chronicle are conditional on
  GCC/KCC, root type, RAM and source/toolchain revision. They are not
  a live size report: obtain fresh identical-link maps before claiming
  savings or changing strict placement.
- Preserve the proven KINIT 256-word default stack until real exceptional
  control-flow traces support a safe smaller value.

## Post-feature-freeze RAM optimization

RAM footprint is the primary optimization metric, execution speed
second, disk footprint third. First eliminate unnecessary permanent
state and duplicate operations at the source, then evaluate relocatable
MRES, dynamic mount/MM metadata, source-level EXEC/RUN consolidation,
APR placement and process/file descriptors. Do not move mandatory
syscalls into removable modules or introduce RAM-expensive indexes
without measured benefit.

`SIZEOPT.MD` contains the detailed deferred source and link census.
When P1-E is accepted or explicitly limited, remeasure each candidate
in matched KCORE/MRES/KINIT maps and physical pressure boots. For
p10fold/p10super, preserve relocation/anchor integrity, natural MRES
dependencies and hot JRST paths. The older exploratory >20-net-word
cold-path threshold is a selection heuristic, not authorization to
fold live code without correctness tests. Runtime MRES unload/reload
is **post-1.0** and cannot compromise installed mandatory services.

## Reference index and update policy

- **Active work:** root `TODO.MD` (53 source-audited open items as of
  v1675r2), and specialist `userland/dsh/TODO.MD` for shell tasks.
- **Detailed implementation state:** root `PROJECT-STATE.MD`, which
  records versions, acceptance provenance and late regressions.
- **Current mainline/PURE branch reconciliation:**
  `design/kcore-open-feature-audit-v1674.md`, interpreted with the
  *newer* v1674 physical result in the PURE branch. Older text in this
  audit predates v1673/v1674 acceptance and is not a current gate.
- **KCORE accepted physical contracts:** `design/kcore-p0a-*`,
  `design/kcore-dtfs-operator-discard-v1575.md`,
  `design/kcore-p1c-final-physical-acceptance-v1603.md`,
  `design/kcore-p1d-final-controller-matrix-v1601.md`, and
  `design/kcore-p1e-pdp6-ka10-architecture-review-v1604.md`.
- **Long-term architecture:** `pdp10-doc/KERNEL/` and hardware-
  specific PDP-6, KA10, DECtape, D6FS and MM documentation. Those
  documents should be updated on completed, accepted development,
  not with unverified handoff plans.
- **Complete original KCORE milestone diary:**
  `git show 738c79c5:KCORE.MD` (mainline) and historical snapshots
  in the PURE branch. Do not re-add DONE/OPEN chronological entries
  to this file; update the accepted contract or remove the finished
  item from `TODO.MD` instead.
