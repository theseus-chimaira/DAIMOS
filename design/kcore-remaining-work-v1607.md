# KCORE v1607: authoritative remaining-work inventory

Date: 2026-11-15
Baseline: DAIMOS v1606 (`ff249953`), VERSION 0.5, PDP-6 only.
Scope: KCORE and the KINIT/MRES interfaces required to complete KCORE
features. This is an audit/checklist, not a new feature implementation.

## Reading this inventory

This document supersedes *open/closed status* in older sections of
`TODO.MD`, `KCORE.MD`, and dated release notes, without discarding
their evidence. Source evidence must always be matched to its date
and implementation. Historical unchecked tasks are not cumulative.
An item marked accepted is not to be reimplemented because an older
entry says OPEN. No acceptance is inferred from an isolated prototype.

The CPU target remains **PDP-6**. The v1605/v1606 source selector and
portability contracts are accepted preparatory work; KA10, KI10,
segmentation, paging, COW and general CPU support are **not scheduled**.
Complete PDP-6 first. Preserve minimal RAM and fast hot paths.
No featureless abstraction layer, per-process registry or machine
runtime dispatch is justified by this inventory.

## 1. State of the current implementation

| Area | Decision now | Evidence / limitation |
|---|---|---|
| P0-A critical ROOT/BACKSTORE failure | CLOSED | v1548 source-linked and physical matrix; selected root vs swap fail-stop, no false rollback after fatal I/O |
| P0-B DTVOL2 under trusted operator | CLOSED FOR THAT POLICY | v1575 actual RW, physical DIRTY/CLEAN, partial DATA, discard, cold-boot refusal; requires operator to prohibit reel exchange during writable mount |
| P0-B unattended/physical reel-retention guarantee | NOT PROVEN | v1571 silent exchange negative proof; no Type-551 hardware generation or reel interlock. Do not relabel operator policy as hardware safety |
| P0-D/E/F requester-only OOM | ACCEPTED FOR SPECIFIED PROFILE | v1298 F12/F13 64/64 gates, original 256K DSK/KCC release; v1348/v1351 real no-partner F07 and physical MM-owner lifecycle |
| P1-C combined storage/swap/BCACHE | CLOSED | v1603 two real 128K boots, two PI6-backed owners, mapped VOLUME fault/rollback, clean BCACHE, LOGSTORE failed write/retry; direct DEVICE fatal v1600 |
| P1-D dynamic FSINFO/root service matrix | CLOSED | v1601 actual missing DSK/DRM controllers, TSFS 7/8 and D6FS 6-slot physical inventories, high-slot identity/error, native bounds |
| P1-E RAW swapless PURE | OPEN, MAIN CODING TASK | v1381/v1384/v1391/v1395 and v1604 boundary pass isolated native tests; production scheduler does **not** link the transactions |
| Optional mutable RAM compression | DECISION DEFERRED | v1451 bitmap codec is native test-only, 30-word TEXT pair; no whole-system RAM savings shown |
| Final matched-current-source KCORE feature freeze | OPEN | Required after P1-E and any actual corrections |
| Strict 32K footprint and optimization | HOLD | v1486 accepted 48K non-XIP; no 32K feature claim, no shrink gate authorization |

The v1603 closure supersedes the unchecked P1-C entries around
`TODO.MD`'s older outstanding checklist, including v1585 timing
concerns. The v1601 closure supersedes its unchecked P1-D item.
The v1575 P0-B decision is narrower than the physical-hardware
guarantee described by v1571 and reiterated by v1603. These facts
must not be collapsed into one ambiguous OPEN/CLOSED flag.

## 2. P0-B policy disposition (safety scope, NOT a PURE dependency)

- [x] Keep the accepted trusted-operator contract explicit:
      no physical reel replacement during any writable DTFS mount;
      on uncertainty preserve WRITE_UNCERTAIN and old owners, then
      require root's checked destructive discard and operator
      quarantine. DIRTY DTVOL2 must remain rejected on reboot.
- [x] Preserve v1575's genuine physical partial DATA and second
      cold-boot test, verified first-write DIRTY, final CLEAN,
      owner-exclusion and normal fresh mount authentication.
- [ ] If an *unattended/untrusted* writable-removable-media policy
      is required, treat it as a separate hardware/interlock
      requirement and release-policy decision. An on-media
      identity reread or guessed software generation bit does NOT
      prove continuous reel retention (v1571). Do not claim strict
      P0-B closure until retention can physically prevent writes
      across removal/exchange and tests reject adversarial swaps.
- [ ] Keep the release-facing capability statement unambiguous:
      operator-trust RW accepted, unrestricted mechanical reel
      exchange NOT accepted. This documentation check is part of
      feature freeze, not authorization to redesign DTFS now.

No production software fix for a missing physical transport lock is
in scope. Do not silently disable already accepted v1575 operation,
and do not silently weaken its declared operating assumption.

## 3. P1-E: the required production PDP-6 implementation

Every unchecked E gate below is required for declaring RAW P1-E
complete. Work in small versioned increments, validate before each
production commit, and keep errors fail-closed.

### E1. Swapless eligibility and immutable source

- [ ] Define a reproducible **actually no-disk-swap** build/boot
      profile. Reclamation is forbidden whenever backing disk
      swap or a conflicting nonresident representation owns the
      process; do not silently use disk as a fallback.
- [ ] At RUN/EXEC attachment, distinguish trusted rebuildable PURE
      source from mutable or non-PURE data. Reject malformed DXR/TX2,
      unvalidated text geometry, absent immutable backing or a
      provider that cannot guarantee source lifetime.
- [ ] Preserve vnode/provider/mount identity and any existing
      u-area/transition owner through the whole inactive lifetime.
      Validate executable unmount, replacement, write mutation,
      source loss and failed backing attachment while keeping FILE,
      cwd, PIPE and process ownership consistent.
- [ ] Do not mistake privileged pinned MEMFS XIP for private
      reconstructible PURE. Preserve accepted v1469-v1485 XIP
      owner/pin/rollback semantics; do not write into pinned XIP.

Relevant code: `proc_pure_attach.c`,
`proc_pure_backing_pdp6.s`, `proc_xip_policy_v1394.c`,
`proc_pure_lifecycle_v1604.h`, `exec_load.s`, and VFS mount
guards. Keep source admission policy distinct from PDP-6 APR geometry.

### E2. Correct RAW inactive representation and MM transaction

- [ ] Wire staged `proc_pure_ram_drop_v1381()` and
      `mm_pure_rebase_pinned_v1381` through an actual scheduler/
      slot-0 reclaim path, not merely native test calls.
- [ ] Admit only STOP/SLEEP or otherwise proven quiescent owners.
      Refuse active, user-mapped, pinned, foreign-lease,
      in-transition or otherwise unprovable process images.
      Keep the original MM descriptor until ownership is checked.
- [ ] In the PDP-6 backend compute exactly the sector-rounded
      reconstructible prefix; retain **all** mutable suffix words
      byte-for-byte. Rebase the existing pinned descriptor with
      no unnecessary temporary full-size copy or second registry.
- [ ] Publish process VM nonresident state and storage mode
      atomically, preserving TRANSITION and PI exclusions. The
      existing packed two-bit storage map must be authoritative:
      RESIDENT=0, DISK=1, PACKED/RAW RAM=2. Do not activate unused
      mode 3 for compression yet.
- [ ] On every MM failure (validation, pin, rebase, unpin, free,
      overlap, descriptor capacity), preserve an exact recoverable
      owner or refuse and leave the process safely inactive.

Relevant code: `proc_pure_ram_v1381.c`,
`vm_pdp6_pure_range_v1604.h`,
`mm_pure_rebase_v1381.s`, `proc_storage_runtime_v30.s`,
`mm_pdp6.s` and `proc_pdp6.s`.

### E3. Safe fast and full restoration

- [ ] Use in-place prefix claim/expand from
      `mm_pure_expand_v1384.s` when the former prefix gap is
      actually available, with checked pin/owner/arena bounds.
- [ ] When occupied or fragmented, allocate a complete APR-aligned
      target and perform bounded full reconstruction using the
      staged v1391 transaction. Verify source and destination
      spans, source identity, capacity and non-overlapping writes.
- [ ] Restore omitted immutable code and saved mutable words in
      their exact logical positions. Ensure compressed executable
      decoding and the original EXEC loader do not overwrite the
      sole surviving mutable suffix. Do not link the rejected
      v1487 +180-TEXT shared-loader prototype without independent
      measured justification.
- [ ] Preserve the retained inactive representation on failure.
      Do not install an APR mapping, READY_ONCE state or runnable
      process until a **complete** restored image is owned and
      validated. Release old representation only after safe commit.
- [ ] Test both same-base and relocated restoration, allocated
      prefix reuse, alternate MM arenas, repeated failure/retry,
      no-partner OOM and executable read/short-read refusal.

Relevant code: `proc_pure_ram_restore_v1391.c`,
`proc_pure_restore_v1384.c`, `mm_pure_expand_v1384.s`,
`exec_image_common_v1386.s`, `vm_pdp6.s`.

### E4. Scheduler, process lifecycle, PI and teardown

- [ ] Integrate with original PI6 wakeup, slot-0 service,
      timer/event scheduler and MM pressure; never block on
      filesystem I/O inside PI or while PI is globally disabled.
- [ ] Verify SLEEP/STOP -> selected runnable -> restored APR
      activation, concurrent independent owners, no duplicate
      wakeup or repeated reload, and blocked transfer/
      `vm_user_mapping_hold` scope across rescheduling.
- [ ] Verify RUN/EXEC/EXIT/WAIT, invalid or failed EXEC,
      BRK growth, signal/TERM/HUP/PIPE, finalizer PREVM,
      parent status/PID reuse and checked u-area release.
      Concurrent EXEC/unmount must not retire immutable backing.
- [ ] Keep existing ordinary raw disk/drum swap and XIP
      behavior exactly unchanged on configurations that
      have backing. A process never simultaneously owns an
      untracked disk record and a RAW inactive RAM suffix.
- [ ] Fail closed on repeated ENOMEM, missing immutable
      provider, MM_BUSY, partial restoration, and process
      cancellation. Preserve a retryable exact owner; avoid
      unsafe zombie/VM double-release or idle-loop spin.

Relevant code: `proc_pdp6.s`, `proc_run.s`,
`proc_swap_pdp6.s`, `proc_storage_runtime_v30.s`,
`mach_user_pdp6.s`, `vm_pdp6.s`.

### E5. Acceptance tests and RAM measurements

- [ ] Add versioned tests **only in daimos-testkit**.
      Rerun native v1604 policy/geometry, v1381 zero-copy,
      v1384/v1385 prefix, v1395/v1397 rollback and actual
      source-sensitive VM/MM/PI6/EXEC regressions.
- [ ] Boot original KCC PDP-6 KCORE without swap backing;
      demonstrate real independent inactive owners, memory
      pressure, PURE drop, recovery and correct program
      output/state. Capture authentic MM descriptors, APR,
      PI6, storage mode, vnode holds and source bytes.
- [ ] Include actual refusal/rollback negatives: pinned
      extent, stolen prefix, full-image fragmentation,
      interrupted provider, failed unpin/free/claim,
      source mutation/unmount, canceled process and
      simultaneous owner activity. Independent source-linked
      negative witnesses must reject false publication.
- [ ] Recheck 48K **non-XIP** and appropriate larger
      96K/128K/256K configurations; 48K v1486 was only
      an earlier boot acceptance, NOT a P1-E run. A
      mandatory *strict 32K optimization/placement* test
      remains HOLD pending separate authorization.
- [ ] Record before/after KCORE TEXT/DATA/BSS, MRES TEXT,
      allocator descriptors, peak scratch, successful
      reclaimed words, pressure-induced additional allocations,
      scheduler timing, and total peak resident RAM.
      A feature acceptance cannot be inferred solely from
      an isolated two-word MM descriptor rebase.

P1-E is closed only after **all** real production lifecycle and
native/full-guest evidence is accepted. v1604 staging alone is
not completion.

## 4. Optional compression: make a measured yes/no decision

- [ ] AFTER RAW P1-E, measure representative mutable suffix
      snapshots under actual scheduler pressure. Include
      codec instructions, staging buffers, MM descriptors,
      source/destination overlap restrictions, and peak restore
      headroom. Compare real 36-bit words saved with code/data
      and transient costs.
- [ ] If and only if the net RAM benefit is positive, consider
      the already tested v1451 bitmap codec. Add an exclusive
      separately owned output extent, strict length/bounds
      preflight and all-or-nothing publication; fall back
      to RAW when size does not shrink. Only then decide
      whether packed storage-map mode 3 is suitable.
- [ ] If benefit is nonpositive, explicitly accept
      **NO COMPRESSION**. No unfinished optional compression
      experiment should block RAW P1-E feature completion.

The trusted v1451 codec is not an untrusted file decoder and
its standalone 30-word TEXT count is not total system cost.

## 5. Final PDP-6 KCORE feature freeze (required)

- [ ] Freeze exact DAIMOS, pdp10-doc, daimos-testkit, KCC,
      assembler, linker, simulator and build-option revisions;
      keep physical evidence with the matching linked symbol
      map and full media identity.
- [ ] Re-run current-source native/source-mutant and genuine
      physical tests for: PI6 and context switching; VM/APR,
      RUN/EXEC/WAIT/BRK/OOM and F07; normal raw DISK swap,
      BACKSTORE fail-stop and recoverable owner rollback; DTFS
      DTVOL2 operator-trust failure/return; D6FS clean BCACHE,
      LOGSTORE/MEMFS persistent two-boot; dynamic FSINFO
      root/controller absence; and completed P1-E.
- [ ] Preserve distinct selected-root vs BACKSTORE fatal
      reasons and healthy unrelated-provider progress.
      The v1599/v1600 fatal cases must never be described
      as post-HALT recovery or concurrent DMA completions.
- [ ] Reconcile source-sensitive stale testkit fixtures:
      v1518 predates v1561's authenticated DTFS write gate
      and currently does **not** pass fixture generation.
      Repair it or explicitly replace its coverage with
      accepted modern tests; do not bypass the gate or
      silently count it as PASS.
- [ ] Report permanent KCORE low end and TEXT/DATA/BSS,
      KINIT/boot, all required MRES footprint, MM slab/
      per-process RAM, peak pressure/restore scratch and
      performance. Current pre-P1-E low end is
      `024364` octal (v1606), not a future size target.
- [ ] Update active state/TODO/docs only on accepted test
      evidence, preserving earlier chronological notes as
      history; declare one well-defined release profile.
      A clean compile/link alone is not a full feature freeze.

No new KCORE optimization, speculative CPU abstraction or
32K shrink should be interleaved with unfinished safety gates.

## 6. Deferred: explicitly NOT current KCORE tasks

- Strict 32K resident footprint, P10FOLD/P10SUPER or JRST
  tuning, cross-MRES folding and final permanent-RAM minimization:
  follow the feature freeze, only by separate authorization.
  Existing v1605/v1606 binary-identical modularity stays.
- KA10/KT10A segmented user VM, shared high code, KI10
  page frames/fast-AC/paging, later CPU boot and drivers:
  source portability concerns are documented, but none is
  scheduled for implementation before PDP-6 completion.
- Unified TSFS, D6FS online packer, special DECtape
  profile/`TAPE_AUX` options and generic userland work:
  separate OS/0.9 workstreams, not reasons to reopen
  completed KCORE P1-C/P1-D.
- Dynamic MRES runtime unload/reload: POST-1.0 only.
- Obsolete compressed *disk* swap, old 32K XIP/32K PURE
  proposals, general page allocator or speculative COW:
  superseded or out of scope. Ordinary raw disk swap
  remains the accepted implementation.

## 7. Short execution order

1. Preserve and explicitly state v1575 trusted-operator P0-B
   release scope; do not pretend a physical reel interlock exists.
2. P1-E E1: no-backing policy and immutable source owner.
3. P1-E E2: atomic zero-copy RAW drop.
4. P1-E E3: fast/full complete restoration and rollback.
5. P1-E E4: live scheduler/PI6/EXEC/EXIT/WAIT integration.
6. P1-E E5: native and real PDP-6 matrix with peak-RAM evidence.
7. Conditional compression yes/no decision.
8. Full source-matched feature freeze and release documentation.
9. Only then, separately authorized footprint reduction/32K work.

The above is the authoritative active KCORE checklist from v1607.
Do not turn its postponed alternatives into implied implementation
work. Every source change remains versionized, reviewed,
tested in daimos-testkit, committed and pushed.

## Evidence anchors

- `PROJECT-STATE.MD` entries v1606/v1604/v1603/v1601,
  v1575, v1548 and v1298/v1351.
- `KCORE.MD` current v1605/v1603/v1601 entries.
- `design/kcore-p1e-ka10-portable-boundary-v1604.md` and
  `design/kcore-pdp6-first-cpu-contract-v1606.md`.
- `design/kcore-p1c-final-physical-acceptance-v1603.md`;
  `design/kcore-p1d-final-controller-matrix-v1601.md`.
- `design/kcore-dtfs-operator-discard-v1575.md` and
  `design/kcore-dtfs-media-retention-v1571.md`.
- pdp10-doc `KERNEL/SWAPLESS-MEMORY.MD`,
  `KERNEL/MULTI-CPU-PORTABILITY-V1606.MD`,
  `STATUS/KCORE-FEATURE-STATE-V1603.MD`,
  `STATUS/KCORE-FEATURE-STATE-V1601.MD`,
  `STATUS/KCORE-FEATURE-STATE-V1575.MD`.
