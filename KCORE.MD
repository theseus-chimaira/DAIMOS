## 2026-10-26 Proposal B v459 size-neutral RENAME XOR experiment

Reversing RENAME namespace XOR operands left full linked lowmem KCORE
at **8961**. The experiment was reverted. No production change or
RAM saving is accepted. See v459 design note.

## 2026-10-26 Proposal B v458 rejected FILE RENAME cleanup split

Duplicating the FILE RENAME cleanup tail increased full linked lowmem
KCORE **8961 -> 8963**. Reverted the experiment; the accepted baseline
remains **8961** with no production changes. See v458 design note.

## 2026-10-26 Proposal B v457 native FILE RENAME lifecycle fixture

New daimos-testkit PDP-6 fixture executes production FILE RENAME
assembly with controlled parent, access and provider callbacks.
Success, both lookup failures, both permission failures and provider
error PASS. Native VFS and DTFS RENAME regressions also PASS.
No production edit: linked KCORE **8961**, KCORE+MRES **15508**,
PERMANENT_LAST **15555** remain the accepted v452 values.
See v457 design note.

## 2026-10-26 Proposal B v456 RENAME readonly and provider failures

Native PDP-6 VFS RENAME test covers readonly source/destination,
writable dispatch and negative provider-result propagation. VFS and
DTFS RENAME regressions PASS. No production changes or RAM savings;
fully linked v452 baseline KCORE **8961**, KCORE+MRES **15508**,
PERMANENT_LAST **15555**. See v456 design note.

## 2026-10-26 Proposal B v455 native VFS RENAME dispatch

New native PDP-6 testkit executable exercises production VFS RENAME
dispatch, same-namespace success and cross-provider/cross-mount
rejection. PASS. No production code or permanent RAM changed; linked
baseline remains KCORE **8961**, KCORE+MRES **15508**,
PERMANENT_LAST **15555**. See v455 design note.

## 2026-10-26 Proposal B v454 RENAME ownership guards

Expanded testkit namespace comparisons and added an explicit FILE
RENAME two-parent/permission/frame guard. Static checks, native PDP-6
FILE and pathname-walk, and DTFS RENAME provider tests PASS. No
production edit. Last fully linked KCORE **8961**, KCORE+MRES
**15508**, PERMANENT_LAST **15555**. See v454 design note.

## 2026-10-26 Proposal B v453 native DTFS RENAME regression

Restored the standalone native DTFS RENAME fixture by providing the
production-equivalent `kconst_1_1` stack constant in daimos-testkit.
Native ITS/TENEX, destination-exists and cross-mount cases PASS.
No DAIMOS production change; last fully linked KCORE **8961**,
KCORE+MRES **15508**, PERMANENT_LAST **15555** (v452).
See v453 design note.

## 2026-10-26 Proposal B v452 masked-XOR RENAME identity

Use one XOR and masked nonzero test for provider/mount mismatch.
Full linked GCC PDP-6 lowmem KCORE **8961** (-2), KCORE+MRES
**15508**, PERMANENT_LAST **15555**. Testkit namespace, native
PDP-6 FILE, pathname-walk and provider-lock tests PASS. No new
persistent allocation or ABI change. See v452 design note.

## 2026-10-26 Proposal B v451 RENAME namespace regression closure

Strengthened testkit coverage to detect collisions between distinct
provider/mount identities across all 4096 combinations. Focused
PDP-6 FILE, pathname-walk, provider-lock and identity checks PASS.
No production change or added permanent RAM. Last fully linked
baseline remains v450 KCORE **8963**, KCORE+MRES **15510**,
PERMANENT_LAST **15557**. See v451 design note.

## 2026-10-26 Proposal B v450 RENAME namespace identity

One upper-12-bit provider+mount comparison replaces independent
namespace field decodes. Linked KCORE **8963** (−3); all 4096
provider/mount combinations and focused provider/FILE/PIPE tests PASS.
See v450 design note.

## 2026-10-26 Proposal B v449 TRUNCATE/WRITE safety guard

TRUNCATE and WRITE_WORDS reuse one resident readonly/swap-protection
and provider-dispatch continuation. Linked KCORE **8966** (−6), with
two extra transient stack words for TRUNCATE and no permanent data.
Focused native provider/FILE/PIPE and frame tests PASS. See v449 note.

## 2026-10-26 Proposal B v447 CREATE/SYMLINK E argument

Removed the redundant branch around the SYMLINK-only provider E=0
assignment. Full linked KCORE **8972** (-1); DTFS CREATE, FILE/PIPE,
UMASK, provider lock and updated SYMLINK ABI guard PASS. Native
end-to-end SYMLINK runtime test remains open. See v447 design note.

## 2026-10-26 Proposal B v446 SYMLINK/CREATE policy consolidation

Replaced the separate SYMLINK read-only check and vnode publication
sequence with the existing CREATE policy via exact argument mapping.
Full linked **KCORE 8973** (8 saved), no permanent RAM growth. Native
provider, FILE/PIPE, DTFS CREATE and SYMLINK ABI guard PASS. Dedicated
SYMLINK end-to-end test pending. See v446 design note.

## 2026-10-26 Proposal B v445 CREATE/SYMLINK shared dispatch

Removed duplicated provider call from VFS CREATE/SYMLINK; both now
share call and vnode publication. Fully linked **KCORE 8981** (-1).
FILE/PIPE, provider lock and shared-dispatch guards PASS. See v445 note.

## 2026-10-26 Proposal B v443 consolidated VFS mutations

UNLINK/CHMOD and CHOWN/UTIME now share the readonly-check and physical
provider dispatch continuation. Full linked lowmem **KCORE 8982**
(five words saved); no permanent RAM allocation. Focused provider,
FILE/PIPE and frame-contract tests PASS. See v443 design note.

## 2026-10-26 Proposal B v442 rejected direct-op experiment

FILE/VFS MKDIR/MKFIFO opcode handoff with shared VFS entry compiled,
but full linked KCORE increased **8987 -> 8990** (+3). Both source
files reverted. Baseline KCORE **8987** unchanged. Future consolidation
must delete a full layer rather than insert another dispatch adapter.
See v442 design note.

## 2026-10-26 Proposal B v441 OPEN directory projection

Directory descriptor argument now derives directly from the VFS stat
type, saving one linked KCORE word: **8987**. FILE/PIPE direct-mount,
UMASK, OPEN lifetime and type-equivalence guards PASS. KCC DSH remains
within the original size gate. See v441 design note.

## 2026-10-26 v439 FIFO regression unblocked

GCC full-profile test image avoids the unintended KCC DSH overflow;
PDP-6 FIFO integration PASS verifies v438. No KCORE or DSH source
change, no KCC DSH size reduction. KCORE **8988**. See v439 note.

## 2026-10-26 Proposal B v438 FIFO OPEN completion

FILE OPEN now shares the descriptor result publication between FIFO
and regular-file success paths. Fully linked KCORE **8988**, one word
below v437; no RAM expansion. OPEN lifetime source guard PASS; see
the v438 design note for verification scope.

## 2026-10-26 Proposal B v437 OPEN ownership boundary

Source audit and testkit guard PASS: OPEN's 14-word scratch, retained
node, post-create stat, FIFO rollback and caller VM mapping lease
constrain FILE/syscall consolidation. No production edits, no saving;
accepted linked **KCORE 8989**. See v437 design note.

## 2026-10-26 Proposal B v436 OPEN normalization

Accepted one KCORE word saved by normalizing OPEN flags directly in
the argument register. Full linked KCORE **8989**, no new RAM
allocation. Exhaustive 18-bit flag equivalence and EXEC ABI tests PASS.
The unified FILE/VFS/syscall architecture remains unfinished. See
`design/open-flag-register-abi-20261026-v436.md`.

## 2026-10-26 Proposal B v435 unified pathname ABI design

Identified OPEN's syscall -> FILE -> VFS/provider ownership chain,
including the separate held user mapping and sleeping I/O workspace.
Drafted a single pathname-request contract to replace (not add to)
existing syscall/FILE branches, first targeting OPEN success and
failure publication. No production edit or measured saving; KCORE
**8990**. See v435 design note.

## 2026-10-26 Proposal B v434: larger resident layer census

The linked PDP-6 map shows contiguous spans of 823 words around VFS,
1050 around FILE and 787 from `exec_native_syscall` to the later
syscall/device bridge, versus only 30 for provider serialization.
These spans are not exact exclusive subsystem sizes or guaranteed
savings. Prioritize a cross-layer FILE/VFS/syscall ABI redesign over
further provider bridge tuning. No production change; KCORE **8990**.
See `design/kcore-resident-layer-census-20261026-v434.md`.

## 2026-10-26 Proposal B v433 provider bridge size ceiling

Source census shows the *entire* resident provider call/serialize/wait/
unlock bridge contains 30 instruction words, plus four binding slots
and one lock word. It cannot by itself supply the hundreds of KCORE
words sought, and replacing it safely requires preserving sleeping
workspace leases. No production edit; KCORE **8990**. Redirect major
architectural review to larger resident FILE/VFS and syscall regions.
See `design/provider-bridge-size-ceiling-20261026-v433.md`.

## 2026-10-26 Proposal B v432 direct MEMFS dispatch

Removed the redundant missing-service compare from direct MEMFS
dispatch. The existing stub returns -1. Full linked PDP-6 GCC lowmem
KCORE **8990** (two words below v431); native direct/unbound MEMFS,
provider locking, and lease interleaving tests PASS. See v432 note.

## 2026-10-26 Proposal B v431 provider-bridge KCORE shrink

The serialized provider bridge now calls the missing-service stub
through its ordinary ABI and uses the common unlock. Linked PDP-6
GCC lowmem **KCORE 8992** (three words saved), KCORE+MRES 15539,
PERMANENT_LAST 15586. Native provider lock, missing service and
workspace interleaving PASS. Rejected fast-path experiment increased
KCORE 8995 -> 8999; corrected fall-through failure before acceptance.
See v431 design note. Strict 32K remains deferred.

## 2026-10-26 Proposal B v430 TSFS READDIR projection

Removed redundant record-pointer AC transfer in TSFS READDIR while
retaining caller-owned name/type projection. Full linked PDP-6 GCC
lowmem KCORE **8995**, KCORE+MRES **15542**, PERMANENT_LAST **15589**.
Native TSFS multiextent, D6LZ and projection guards PASS. The global
provider lock remains necessary. See v430 design note.

## 2026-10-26 Proposal B v429 TSFS value-record cost gate

Testkit guard PASS: returning owned copies of TSFS records would cost
at least 8 file-record or 4 extent-record stack words per call, while
sleeping transfers still require shared-buffer serialization. No
unproven value-copy code added to production. Accepted KCORE **8995**;
see v429 design note. Strict 32K acceptance remains deferred.

## 2026-10-26 Proposal B v428 native workspace interleaving

The actual provider bridge passes a new PDP-6 deterministic wait and
shared-buffer lease regression in daimos-testkit. TSFS pointer-return
paths need explicit owned record values before workspace ownership can
be shortened. No kernel change; KCORE remains **8995**. See v428 note.

## 2026-10-26 Proposal B v427: provider I/O lease boundaries

Source tracing proves that shared `fs_block_workspace` remains live
across sleeping DSK/DRM/DTC operations and cache publication. A new
testkit guard checks these specific I/O boundaries. Do not shorten
serialization without replacing the live-buffer ownership contract.
Production unchanged; KCORE **8995**. See v427 design note.

## 2026-10-26 Proposal B v426 provider transaction baseline

Existing PDP-6 provider lock native/static gates PASS. New testkit
inventory counts 60 shared-workspace references across seven files;
blocking I/O is reached via driver wrappers, so transaction lifetimes
are not yet proven separable. Production unchanged; KCORE **8995**.
See `design/provider-transaction-call-boundaries-20261026-v426.md`.

## 2026-10-26 Proposal B v425: new provider transaction candidate

Source audit identifies an untried coordinated redesign of shared
filesystem transfer-workspace ownership and provider serialization.
`fs_provider_reg_call` saves seven ACs during blocking DTFS/D6FS/TSFS
operations; the single 128-word workspace also backs BCACHE slot 0,
so simply removing or splitting the existing lock is unsafe. Proposed
transaction-lifetime experiment and native concurrency gates are in
`design/provider-transaction-ownership-redesign-20261026-v425.md`.
No production edit or new RAM reduction claimed; baseline KCORE 8995.

## 2026-10-26 Proposal B v424 branch-free FLOCK mode replacement

Moving UNLOCK's regular state into the FLOCK mode table removes three
instructions from the full linked PDP-6 GCC lowmem image. KCORE
**8995**, KCORE+MRES **15543**, PERMANENT_LAST **15590**. Native FILE
and directmount/PIPE/UMASK regressions PASS. Total saving against
pre-v417 KCORE 9109 is **114 permanent words**, with no process
u-area growth. Strict 32K acceptance remains deferred.

## 2026-10-26 Proposal B v423 mount-unlock instruction reduction

Retaining the packed descriptor in AC4 while extracting its full
six-bit mount ID into AC5 removes a redundant table reload from
`file_unlock_mount`. Full PDP-6 GCC lowmem linked measurements:
KCORE **8998**, KCORE+MRES **15546**, PERMANENT_LAST **15593**,
one word smaller than v417/v422 and **111 words below v388**.
Native FILE and directmount/PIPE/UMASK tests PASS. No u-area growth.

## 2026-10-26 Proposal B v422 complete root-login validation

Full PDP-6 disk/drum/tape login matrix PASS after raising the tape
interactive response timeout to 15 minutes in daimos-testkit;
disk/drum retain five minutes. No production source change or new size
measurement. Accepted v417 KCORE remains **8999 words** (110 saved).
Strict 32K acceptance remains deferred.

## 2026-10-26 Proposal B v420 provider lifecycle regression

Native PDP-6 D6FS multimount, remount and direct DRM backing tests
PASS against v417 full six-bit FILE mount descriptors. KCORE remains
**8999 words** (last measured full linked image); no new saving is
claimed. Root filesystem boot/login and broad provider testing remain
independent acceptance gates; strict 32K is deferred.

## 2026-10-26 Proposal B v419 native RUN lifecycle regression

Both full PDP-6 simulator fixtures PASS: process lifecycle
(RUN/GETPID/WAIT/nohang/zombies/orphans/explicit fdmap) and late-RUN
rollback (16 mapping failures under three active children with stable
accounting/recovery). `MAKEFLAGS=COMPILER=gcc` uses the documented
smaller DSH without relaxing its EXEC image-size limit. No source
change; previous accepted v417 linked KCORE **8999** remains current.

## 2026-10-26 Proposal B v417 direct-mount descriptor cutover

Accepted full linked GCC PDP-6 lowmem change: KCORE **9109 -> 8999**,
KCORE+MRES **15657 -> 15547**, PERMANENT_LAST **15704 -> 15594**
(110 permanent words saved). FILE now stores full six-bit mount IDs
directly in the first descriptor word, retains independent 36-bit
offsets, and uses process-local vnode-wide FLOCK. The mount-extension
bitmap and per-OPEN lock-family token are removed; VFS CREATE reads
the credential UMASK9. Physical process u-area allocation is unchanged.
Focused PDP-6 FILE, directmount, PIPE identity and UMASK tests PASS;
full boot/multiuser acceptance remains pending. Strict 32K acceptance
and unrelated folding experiments remain deferred. See v417 design note.

## 2026-10-26 Proposal B v400 decisions locked; next-chat handoff v401

Recorded the four approved v400 decisions in
`design/file-descriptor-mount-redesign-20261026-v400.md`:
(1) DUP/DUP2 and RUN descriptor remapping copy offsets independently;
(2) use minimal, demonstrably sufficient descriptor flags;
(3) boot/hardware/provider-dependent capacity must support at least 26
mounts when available, without preallocating 26 slots every boot, and
remain expandable; (4) retain existing locking and provider serialization
initially. Canonical full-vnode plus full 36-bit offset in two words is
the proposed B1 target. No implicit shared offsets, semantic relaxation,
or production change is authorized. Next chat: audit actual metadata
users, propose an exact no-growth encoding, quantify complete RAM/speed,
and prepare the full regression matrix **before seeking approval to edit
code**. Latest measured linked baseline: KCORE 9109; KCORE+MRES 15657;
PERMANENT_LAST 15704. See the design document for detailed handoff.

## 2026-10-26 Proposal B descriptor/mount design review v400

User-selected architecture: at least 26 mount slots with future expansion,
canonical-vnode two-word per-process descriptor (alternative B), retain
useful DUP and shared-offset capabilities. Discussion-first constraint:
no production edit without design agreement and an explicit functionality
and regression review. See `design/file-descriptor-mount-redesign-20261026-v400.md`
for the current code inventory, three candidate formats, offset-sharing
clarification, migration gates and open decisions. This is a design draft;
no production code changed, no new size or test acceptance claimed.
Latest measured baseline remains v388 KCORE 9109, KCORE+MRES 15657,
PERMANENT_LAST 15704.

## 2026-10-26 Cross-proposal architectural redesign policy v399

**Applies to the entire A–F KCORE architectural shrink roadmap, not just
Proposal B.** Existing interfaces, internal and public ABIs, descriptor
semantics, locking protocols, error/rollback rules and historical
compatibility requirements are **not immutable constraints**. The team
may rewrite producers, consumers, callers, filesystem providers,
userland and tests together when a simpler design has a demonstrable
memory/performance benefit. Do not reject a proposal solely because
its behavior or interface differs from the current implementation.

The invariants that **do** remain mandatory are:

- Correctness and explicit resource ownership: no corrupted state,
  leaks, deadlocks, use-after-free or half-published objects. Rollback
  may be redesigned or replaced by a simpler transaction protocol, but
  partial failure must leave a well-defined recoverable state.
- Keep the necessary capabilities available to the OS; alternatives
  need not preserve historical invocation or descriptor semantics.
  EXEC and RUN remain resident essential syscalls, notwithstanding
  permitted changes to their calling interface or internals.
- Optimize **actual physical RAM consumption first**, including KCORE,
  MRES, per-process memory and MM-managed allocations, rather than
  moving memory cost between accounting categories. Execution speed is
  nearly as important; disk size is secondary.
- No workaround-driven redesign or feature creep: fix underlying bugs,
  preserve a readable, documented implementation and measure the
  post-fold fully linked image as well as runtime memory costs.
- Before accepting changed semantics, define the *new* contract,
  migrate all affected call sites and userland, and add appropriate
  tests in `daimos-testkit`. Test both normal operation and concurrency,
  failure, resource cleanup, boot and process lifecycle as applicable.

**A:** process/wait/signal/session and teardown semantics may be
simplified together, but essential scheduling/ownership correctness
remains. **B:** FILE/VFS/MonitorFS APIs, descriptor representation,
DUP/DUP2 and offset semantics, mount policy, locking and rollback
may all change in a coordinated rewrite. Compare packed descriptors,
independent local descriptors and minimal shared handles, including
per-process storage and decode costs. **C:** EXEC/RUN validation,
startup and publication APIs may change, but essential resident
EXEC/RUN and sound replacement/creation semantics stay. **D:** MM
registry and allocation rules may be redesigned based on actual
high-water and fragmentation evidence. **E:** inactive-process
representation and swap interfaces are redesignable if correct state
restoration and total RAM benefit are demonstrated. **F:** scratch
workspace ownership/lock conventions may be rewritten after a proven
lifetime and reentrancy analysis.

This policy supersedes *compatibility-only* restrictions in older
proposal reviews, **not** their factual measurements or correctness
warnings. No production code or ABI was changed by this documentation
update. The latest accepted post-link baseline remains v388: KCORE
**9109**, KCORE+MRES **15657**, PERMANENT_LAST **15704**.

## 2026-10-26 Proposal B bounded structural review v389–v398 (ten passes)

User requested ten further review iterations after v388, except that an
unexpected large-saving opportunity should extend the effort. Ten
independent candidate boundaries were inspected in the current
`system/kernel/fs/vfs.s` and `file_runtime.s`, using the existing
A–F roadmap. **These are source-level review passes, not ten new
compiled patches or runtime acceptance results.** No proposed change
met a credible >=10–20 net resident-word saving threshold with a
proved unchanged ABI and lifetime, so no production edit was made.

1. **v389 LOOKUP namespace table:** both three-record namespaces now
   share loop-count initialization (v387); a further common loop would
   add branching without a demonstrated saving.
2. **v390 READDIR namespace merge:** built-in `/MONITOR` and mounted
   root offsets depend on different input traversal states and
   distinct stack frames. No safe common-entry reduction established.
3. **v391 PARENT_NAME:** MonitorFS device/PROCESSES/DOMAINS paths
   publish different counted names, while provider parent lookup has
   its own mount inheritance and ABI. Existing provider dispatch is
   shared where feasible (v386).
4. **v392 MOUNT slot search:** free-slot selection rejects existing
   mounts and uses MM-backed target/root tables; changing scan order
   risks duplicate target acceptance.
5. **v393 MOUNT publication:** target/root table entries and RO/SWAP/
   LOGSTORE owner policy have distinct publication requirements.
   No redundant full-sized resident table is present.
6. **v394 UNMOUNT retirement:** busy checks, sync, prepare-unmount,
   namespace-root substitution and descriptor unlocking precede
   shared slot retirement. Folding into publication would conflate
   failure and lifetime states.
7. **v395 CREATE/MKDIR/MKFIFO:** common `vfs_create_common` and
   `vfs_create_store_result` already share mode handling, provider
   result publication and error unwind; remaining differences are
   provider/operation policy.
8. **v396 SYMLINK/RENAME:** their readonly checks resemble CREATE,
   but arguments, namespace transitions, provider operations and
   stacked node-result publication differ. No verified net gain from
   a generic new frame helper.
9. **v397 bulk READ/WRITE:** common permission validation and
   provider arguments already exist (v376/v377); device dispatch,
   atomic APPEND and offset publication remain operation-specific.
10. **v398 FILE pathname wrappers:** common parent write-access check,
    one-argument cleanup and descriptor constructors are already in
    place. OPEN/FIFO error cleanup, RENAME two-parent validation and
    UNLINK/RMDIR type/busy checks have different ownership semantics.

**Conclusion:** none of these ten source reviews revealed a large
unimplemented consolidation supported by current evidence. The latest
measured linked baseline (v388) remains KCORE **9109**, KCORE+MRES
**15657**, PERMANENT_LAST **15704**; reaching KCORE <8192 needs
**918** further words. No new size build or test run is claimed,
because there is no production source change. Proposal C EXEC/RUN
is the next distinct roadmap phase. Proposal B can be closed after
any desired final dedicated regression/acceptance pass; avoid
continuing single-word instruction churn as a substitute for an
architectural change. Strict 32K acceptance remains deferred.

## 2026-10-26 Proposal B direct READDIR namespace vnode comparison v388

Removed a redundant AC4 load before `CAME` in the resident
`vfs_readdir` `/MONITOR` special-case test: compare AC1 directly
against `monitorfs_names+7`, which already stores the canonical vnode.
The provider fallback, directory index bounds, special namespace
entries and ABI are unchanged; no persistent state or MRES movement.

Full linked GCC PDP-6 lowmem versus v387: KCORE **9110 -> 9109**
(-1), KCORE+MRES **15658 -> 15657** (-1), PERMANENT_LAST
**15705 -> 15704** (-1). All four native daimos-testkit checks
passed: `target-file-pdp6`, `target-file-walk-pdp6`,
`test-daimos-vfs-stat-frame`, `test-daimos-pre-dsh-fs-closure`.
Global below-8192 goal still needs 918 words. Remaining Proposal B
work should prioritize larger FILE/VFS consolidation over repetitive
single-instruction changes; the original Proposal C addresses
EXEC/RUN separately.

## 2026-10-26 Proposal B VFS LOOKUP common namespace loop count v387

Both built-in MonitorFS namespace branches (`/` and `/MONITOR`)
search exactly three static four-word records. Moved their duplicated
`MOVEI 0,3` to `vfs_lookup_builtin_start`, where both paths converge;
all comparison fields, mount-root behavior and provider fallthrough
remain unchanged. No state, MRES or ABI changes.

Full linked GCC PDP-6 lowmem versus v386: KCORE **9111 -> 9110**
(-1), KCORE+MRES **15659 -> 15658** (-1), PERMANENT_LAST
**15706 -> 15705** (-1). All four daimos-testkit checks pass:
`target-file-pdp6`, `target-file-walk-pdp6`,
`test-daimos-vfs-stat-frame`, `test-daimos-pre-dsh-fs-closure`.
The global below-8192 goal still requires 919 words. The
remaining B work needs larger verified consolidation, not repeated
one-word tuning; Proposal C remains the separately defined EXEC/RUN
phase of the original A–F architectural roadmap.

## 2026-10-26 Proposal B VFS PARENT_NAME provider selection v386

Revisited the earlier invalid-assembler trial without retaining it.
The valid PDP-6 `CAIE 7,2` / `CAIN 7,3` skip pair replaces two
independent equal comparisons and jumps in the resident
`vfs_parent_name_dispatch`. Exactly providers 2 and 3 enter
`monitorfs_parent_name`; other providers continue to the existing
FS_MRES_OP_PARENT_NAME path. Both mount-root resolution and the
provider ABI stay unchanged. The rejected experimental `CAINE`
mnemonic is not used anywhere in this accepted edit.

Full linked GCC PDP-6 lowmem versus v385: KCORE **9112 -> 9111**
(-1), KCORE+MRES **15660 -> 15659** (-1), PERMANENT_LAST
**15707 -> 15706** (-1). Native daimos-testkit targets
`target-file-pdp6`, `target-file-walk-pdp6`,
`test-daimos-vfs-stat-frame` and `test-daimos-pre-dsh-fs-closure`
all PASS. The global below-8192 KCORE goal still needs 920 words;
Proposal B remains distinct from original Proposal C EXEC/RUN.

## 2026-10-26 Proposal B VFS READDIR builtin bounds branch v385

Reconfirmed the original 2026-10-24 A–F architectural roadmap:
Proposal B covers resident FILE/VFS/MonitorFS consolidation, while
Proposal C is distinct EXEC/RUN consolidation, preserving their
resident, distinct publication/replacement transactions. Therefore
v385 changes only resident VFS namespace READDIR, not EXEC/RUN.

In `vfs_readdir`, the post-provider namespace insertion index is
rejected if negative or at least three. Replaced `CAIGE 4,3; JRST
vfs_readdir_builtin; JRST vfs_readdir_zero` with `CAIL 4,3; JRST
vfs_readdir_zero`, falling through for valid indices 0..2. The
negative index rejection is unchanged; returned dirent records and
mount-root traversal are unchanged. No MRES, ABI or permanent state
changes.

Full linked GCC PDP-6 lowmem versus v384: KCORE **9113 -> 9112**
(-1), KCORE+MRES **15661 -> 15660** (-1), PERMANENT_LAST
**15708 -> 15707** (-1). Native daimos-testkit targets
`target-file-pdp6`, `target-file-walk-pdp6`,
`test-daimos-vfs-stat-frame`, `test-daimos-pre-dsh-fs-closure`
all PASS. KCORE target <8192 still requires 921 words; completion
of Proposal B is assessed by worthwhile structural savings rather
than that independent global target. Proposal C remains separate.

## 2026-10-26 Proposal B resident storage overlay candidate scan v384

Reviewed large explicit resident `.block` declarations, filesystem
workspace call sites, BCACHE slot ownership, MEMFS snapshot entry
points and MonitorFS swapped-process read dispatch. The only obvious
fixed 0200-octal transfer block is `fs_block_workspace` in `vfs.s`;
`bcache_pdp6.s` already overlays slot 0 there while slot 1 is a
**separately allocated 0201-word MM slab** that is needed to preserve
an old clean workspace block across cache misses. The second cache
slot is not an unused permanent block and cannot serve as the VFS
transfer destination without invalidating the cache's recovery model.

`memfs_runtime.s` calls `memfs_snapshot_mount` and
`memfs_snapshot_shutdown`; `monitorfs_runtime.s` dispatches
`mfsproc_swapped_read_words_v63`, which invokes `backstore_read`
with `fs_block_workspace` and invalidates the BCACHE workspace tag
first. These entry paths cannot be assumed to share the physical
provider's DTFS/TSFS/D6FS serialization merely because they operate
on the same scratch address. The audit did **not** establish a
mutually exclusive 128-word replacement allocation or safe buffer
lifetime, so **no storage overlay or memory saving is accepted**.
No code change was made; the latest measured complete linked image
remains KCORE **9113**, KCORE+MRES **15661**, PERMANENT_LAST
**15708**. Next prioritize structural code savings with demonstrable
linked impact, or first prove an exact lock/call graph for the entire
workspace lifetime. Below-8192 KCORE still requires 922 words;
strict 32K acceptance remains deferred.

## 2026-10-26 Proposal B BCACHE/workspace overlay audit v383

Continued the fixed 128-word `fs_block_workspace` lifetime analysis.
`system/kernel/storage/bcache_pdp6.s` **already aliases** BCACHE
slot 0 to that exact VFS scratch region. Its shared slot-0 tag takes
one fixed resident word. The separate MM-managed slot 1 is a
`0201`-word allocation (128 data plus tag), made lazily with
`mm_alloc_aligned_noreclaim`, not a second permanent KCORE buffer.
On workspace-backed cache misses, slot 0's live clean block is copied
to slot 1 before the workspace is overwritten. Reusing the MM slab as
scratch concurrently would destroy that protection; removing the
resident scratch and retaining another permanently allocated 128-word
buffer would merely relocate the RAM cost. The current sharing
already captures the obvious BCACHE overlay opportunity.

The MonitorFS swapped-process read explicitly invalidates the shared
workspace tag before `backstore_read` and uses a per-call stack source
record. The source comments say that swapping and scheduling preserve
executive syscall completion, but this does not prove safe arbitrary
overlap with every other filesystem/snapshot call. Physical provider
serialization through `fs_provider_reg_call` remains an independent
lifetime guarantee; MEMFS and snapshot paths require separate review.

No code change is accepted in v383. Linked v381/v382 baseline remains
KCORE **9113**, KCORE+MRES **15661**, PERMANENT_LAST **15708**;
no additional size saving or runtime test claim. Before a storage
rewrite, require a complete call graph including boot-before-MM,
reentrant provider callbacks, MonitorFS swapped reads and snapshot
mount/shutdown. Search for a *different* existing mutually exclusive
allocation rather than overlaying BCACHE's required backup slot.
Strict 32K acceptance remains deferred, and 922 additional KCORE
words are required for the below-8192 target.

## 2026-10-26 Proposal B shared filesystem workspace lifetime audit v382

Inspected the 0200-octal (128-decimal)-word `.bss` `fs_block_workspace`
owned by resident `vfs.s`. Direct references extend beyond D6FS and
DTFS: TSFS, MEMFS snapshot, MonitorFS swap, the `fs_mres` zero-block
bridge and block-cache operations also depend on this fixed address.
`fs_provider_reg_call` serializes physical providers DTFS, D6FS and
TSFS across their full requests: those requests may sleep, and the
provider lock prevents cross-provider workspace corruption. The
MEMFS fast path deliberately bypasses this lock because it does not
sleep. The MonitorFS swap reader explicitly invalidates the shared
block-cache workspace tag before loading a swap block and keeps a
cached-block key local to its source.

Consequently an unconditional MM relocation is **not accepted**:
fixed-address references across assembler/C, calls in potentially
different contexts, and synchronization/caching assumptions must
first be audited. In particular we have not proved that every
MonitorFS swap or snapshot access is covered by provider locking, or
that any MM block can be released with no persistent replacement.
Merely moving the 128 words outside KCORE would not establish a net
permanent-RAM saving. No source changed and no size reduction claimed;
linked baseline remains KCORE **9113**, KCORE+MRES **15661**,
PERMANENT_LAST **15708**, as measured in v381. Next investigation:
map each user and caller to the lock domain, establish boot-before-MM
requirements, then identify an *existing* mutually exclusive 128-word
area before attempting a zero-extra-memory overlay. Strict 32K
acceptance stays deferred.

## 2026-10-26 Proposal B VFS dispatch / resident transfer workspace audit v381

Tested removal of the apparent redundant JRST immediately before
`vfs_read_words_provider` in resident `vfs.s`; full linked GCC PDP-6
lowmem image remained **KCORE 9113**, **KCORE+MRES 15661**,
**PERMANENT_LAST 15708**. Reverted the size-neutral experiment.
No accepted production code modification or new runtime acceptance.

Resident VFS also contains `fs_block_workspace`, a 0200-octal
(128-decimal)-word BSS transfer buffer, documented as shared between
filesystem providers. This is potentially much more significant than
single-instruction edits, but no reassignment is safe until all
provider references, boot-before-MM use, locking/sleep lifetimes,
concurrency and MM allocation policies are proven. In particular,
replacing it with another permanent 128-word reservation would not
improve total permanent RAM; lowering KCORE alone is not proof of a
net resident saving. Analyze provider calls and lifetimes before
editing, and retain the documented ABI. Strict 32K acceptance stays
deferred. Baseline KCORE requires 922 fewer words to reach 8191.

## 2026-10-26 Proposal B VFS READ_WORDS provider decode audit v380

Reviewed the resident READ_WORDS VFS dispatch and its fast paths for
PIPE and MonitorFS process data. After the packed provider/kind check,
experimentally replaced the explicit halfword shift that extracts the
provider from AC7 with `LDB 7,[POINT 6,1,5]`. The complete linked
GCC PDP-6 lowmem measurement was unchanged: KCORE **9113**,
KCORE+MRES **15661**, PERMANENT_LAST **15708** words. The size-neutral
change was reverted; no production code or MRES source changed.
These paths remain candidates only for larger semantic-preserving
consolidations, not ineffective register decode rewriting. No new
runtime acceptance is claimed. KCORE strictly below 8192 needs 922
additional words; strict 32K acceptance remains deferred.

## 2026-10-26 Proposal B FILE character I/O register-move audit v379

Examined resident `file_readchar` and `file_writechar` for redundant
AC1 descriptor-pointer reloads immediately after saving the same
pointer in AC10 and before `file_node_from_fd`. Removed one source
MOVE from each call path experimentally. The complete linked GCC
PDP-6 lowmem measurement remained unchanged at KCORE **9113**,
KCORE+MRES **15661** and PERMANENT_LAST **15708** words, equal to
v378. This strongly indicates existing assembly folding already
removes the moves. Reverted the size-neutral source experiment;
there is no accepted runtime modification or new test acceptance.
The KCORE below-8192 target still needs 922 words. Future work
should concentrate on demonstrably larger shared request and VFS
blocks, not textual edits already folded by the toolchain. Strict
32K acceptance remains deferred.

## 2026-10-26 Proposal B remove dead bulk FILE offset setup v378

In resident `file_runtime.s`, the shared READ/WRITE bulk entry had
loaded descriptor offset into AC2 and redundantly moved the descriptor
pointer from AC1 back into AC1 just before calling `file_node_from_fd`.
Inspection of the helper confirms that it requires only AC1 (already
holding the descriptor pointer), preserves AC2, and returns the vnode
in AC1. The subsequent shared request setup overwrites AC2 with the
transfer length; the descriptor offset is loaded at the VFS provider
boundary where needed. Both dead moves were removed. No provider
semantics, public ABI, or MRES source changes.

Full linked GCC PDP-6 lowmem against v377: KCORE **9115 -> 9113**
(-2), KCORE+MRES **15663 -> 15661** (-2), PERMANENT_LAST
**15710 -> 15708** (-2). daimos-testkit `target-file-pdp6`,
`target-file-walk-pdp6`, `test-daimos-vfs-stat-frame` and
`test-daimos-pre-dsh-fs-closure` PASS. No new concurrent physical
I/O acceptance is claimed. Below-8192 KCORE still requires 922
further words; strict 32K acceptance remains deferred.

## 2026-10-26 Proposal B shared bulk VFS provider arguments v377

Resident `file_runtime.s` bulk READ and WRITE previously duplicated
four register moves to prepare fallback VFS provider requests:
AC3 buffer, AC4 count, AC1 vnode, AC2 descriptor offset. A common
`file_words_provider_common_v377` block now prepares these once;
WRITE reaches it by fallthrough, READ jumps to it. The signed
READ/WRITE selector in AC5 then dispatches READ directly to
`vfs_read_words` while WRITE retains the original APPEND provider
EOF-lock decision and `vfs_write_words` path. Direct physical-device
shortcuts and shared descriptor-offset handling remain unchanged.
No ABI, MRES code or permanent state is added.

Full linked GCC PDP-6 lowmem vs v376: KCORE **9117 -> 9115**
(-2), KCORE+MRES **15665 -> 15663** (-2), PERMANENT_LAST
**15712 -> 15710** (-2). daimos-testkit `target-file-pdp6`,
`target-file-walk-pdp6`, `test-daimos-vfs-stat-frame`, and
`test-daimos-pre-dsh-fs-closure` all PASS. Physical-provider
concurrency/APPEND end-to-end acceptance remains separate.
KCORE below 8192 needs 924 further words; strict 32K acceptance
remains deferred.

## 2026-10-26 Proposal B shared bulk READ/WRITE argument save v376

In resident `system/kernel/fs/file_runtime.s`, `file_read_words` and
`file_write_words` duplicated three entry PUSH instructions saving
AC10, AC2 buffer and AC3 transfer length. Moved those three identical
PUSHes into common `file_words_save_args_v376`, after the distinct
READ/WRITE permission selector in AC5 is initialized. READ branches
to the shared prologue and WRITE falls through into it. Both enter
`file_words_common` with identical stack layout, original AC1 fd,
arguments, and selector values; the existing result cleanup and
provider-side append semantics remain untouched. No MRES change.

Full linked GCC PDP-6 lowmem versus v375: KCORE 9120 -> **9117**
(-3 words), KCORE+MRES 15668 -> **15665** (-3), PERMANENT_LAST
15715 -> **15712** (-3). daimos-testkit `target-file-pdp6`,
`target-file-walk-pdp6`, `test-daimos-vfs-stat-frame` and
`test-daimos-pre-dsh-fs-closure` PASS. No new physical-device
concurrency acceptance is claimed. The below-8192 KCORE goal requires
926 further words; strict 32K acceptance remains deferred.

## 2026-10-26 Proposal B FILE OPEN directory flag branch v375

In resident `file_open`, replace the conditional branch around
`MOVEI 3,1` (directory argument to `file_new_fd`) with a conditional
`CAIN 1,1` followed by `MOVEI 3,1`: CAIN executes the MOVEI only
for `VFS_TYPE_DIR=1`, leaving zero for every other type. The first
experiment incorrectly used CAIE, which inverted the condition;
it was caught in review and immediately corrected **before testing
or committing**. Only the correct CAIN version is retained.

Full linked GCC PDP-6 lowmem against accepted v374: KCORE
9121 -> **9120** (-1), KCORE+MRES 15669 -> **15668** (-1),
PERMANENT_LAST 15716 -> **15715** (-1). daimos-testkit
`target-file-pdp6`, `target-file-walk-pdp6`,
`test-daimos-vfs-stat-frame` and `test-daimos-pre-dsh-fs-closure`
PASS on the corrected source. Public FILE ABI, directory semantics
and provider behavior unchanged. No MRES code modified. KCORE below
8192 needs 929 more words; strict 32K acceptance deferred.

## 2026-10-26 Proposal B FILE OPEN access-mask consolidation v374

Consolidated the `file_open` READ/WRITE/TRUNC permission-mask builder
in resident `system/kernel/fs/file_runtime.s`. The prior branch tree
set bit 4 if READ and bit 2 if WRITE or TRUNC. Replaced it with two
conditional `TRNE` / `TRO` pairs, checking combined flag mask 022
for WRITE|TRUNC. The resulting mask for all 8 combinations of these
three input bits is identical, as are the subsequent STAT record
access check, CREATE path and failure handling. No public ABI, extra
persistent state or MRES modification.

Full linked GCC PDP-6 lowmem versus v373: KCORE 9125 -> **9121**
(-4 words), KCORE+MRES 15673 -> **15669** (-4), PERMANENT_LAST
15720 -> **15716** (-4). Native daimos-testkit `target-file-pdp6`,
`target-file-walk-pdp6`, `test-daimos-vfs-stat-frame`, and
`test-daimos-pre-dsh-fs-closure` PASS. Full Open flag/error matrix
and physical-provider concurrency have not been separately accepted;
strict 32K acceptance remains deferred. Below-8192 KCORE requires
930 additional words. Continue resident file-runtime and VFS
structural analysis, prioritizing persistent RAM over speed.

## 2026-10-26 Proposal B KCORE FILE OPEN parent permission reuse v373

The resident FILE OPEN create-on-miss branch performed its parent
write/search access check with an inline `MOVEI 2,3` followed by
`PUSHJ file_check_access`, duplicating the contract already
centralized for SYMLINK/RENAME/MKDIR/UNLINK in the existing
`file_parent_write_access_v367` tail-call helper. Reused that helper
in `file_open` with unchanged parent vnode, permission mask and
failure semantics. No MRES changes, new state or new public ABI.

Full linked GCC PDP-6 lowmem versus v372: KCORE 9126 -> **9125**
(-1), KCORE+MRES 15674 -> **15673** (-1), PERMANENT_LAST 15721 ->
**15720** (-1). Native daimos-testkit `target-file-pdp6`,
`target-file-walk-pdp6`, `test-daimos-vfs-stat-frame`, and
`test-daimos-pre-dsh-fs-closure` PASS. KCORE below 8192 requires
934 additional words. Continue larger structural FILE/VFS analysis;
strict 32K acceptance remains deferred.

## 2026-10-26 Proposal B KCORE MOUNT vnode slot encoding v372

In the resident `vfs_mount_found` path, the public mount ID is the
validated zero-based slot plus one. Replaced `MOVE 6,7` followed by
`ADDI 6,1` with `MOVEI 6,1(7)` before the unchanged mount/kind
field shifts and bitwise packing. No state storage, ABI, root vnode
bit layout or provider interface changed.

Full linked GCC PDP-6 lowmem relative to v371: KCORE 9127 ->
**9126** (-1), KCORE+MRES 15675 -> **15674** (-1),
PERMANENT_LAST 15722 -> **15721** (-1). `daimos-testkit` targets
`target-file-pdp6`, `target-file-walk-pdp6`,
`test-daimos-vfs-stat-frame`, `test-daimos-pre-dsh-fs-closure`
PASS. Multi-provider concurrent MOUNT/UNMOUNT acceptance remains
separate; strict 32K acceptance deferred. KCORE below 8192 requires
935 further words. Larger resident structural consolidations remain
the focus of Proposal B.

## 2026-10-26 Proposal B KCORE UNMOUNT mount-ID preparation v371

After restoring DevSpace2 connectivity, inspected the v370 checkout
and optimized two zero-based-slot-to-public-mount-ID preparations in
resident `system/kernel/fs/vfs.s` `vfs_unmount`:
`MOVE 4,2` + `ADDI 4,1` in the protected owner checks, and
`MOVE 1,2` + `ADDI 1,1` before `file_unlock_mount`, each replaced
by one PDP-6 indexed `MOVEI dest,1(2)`. AC2 remains the source slot,
with unchanged public owner tokens, error order and unmount lock
release. No new state and no MRES modifications.

Complete linked GCC lowmem image versus v370: KCORE 9129 ->
**9127** (-2), KCORE+MRES 15677 -> **15675** (-2),
PERMANENT_LAST 15724 -> **15722** (-2). Native daimos-testkit
`target-file-pdp6`, `target-file-walk-pdp6`,
`test-daimos-vfs-stat-frame`, `test-daimos-pre-dsh-fs-closure`
all PASS. No concurrent physical-provider unmount acceptance is
claimed. Another 936 words are needed to reach below 8192; strict
32K acceptance remains deferred. Larger structural VFS/FILE work
remains the main Proposal B priority.

## 2026-10-26 Proposal B VFS unmount provider-field decode v370

In `system/kernel/fs/vfs.s`, after `vfs_sync` and before deciding
whether a provider requires PREPARE_UNMOUNT, replaced `MOVE 3,1` plus
`LSH 3,-036` with the direct `LDB 3,[POINT 6,1,5]` provider-field
extraction already used elsewhere in resident VFS. The existing
MEMFS/D6FS prepare-only dispatch and unmount completion order remain
unchanged; no MRES code or interface was modified.

Full linked GCC lowmem image against v369: KCORE 9130 -> **9129**,
KCORE+MRES 15678 -> **15677**, PERMANENT_LAST 15725 -> **15724**
(all -1 word). `daimos-testkit` native PDP-6 `target-file-pdp6`,
`target-file-walk-pdp6`, `test-daimos-vfs-stat-frame`, and
`test-daimos-pre-dsh-fs-closure` PASS. The test scope does not include
full multi-provider concurrent physical unmount acceptance. KCORE
strictly below 8192 still needs 938 words. Strict 32K boot acceptance
is deferred; further Proposal B work remains on resident MOUNT/UNMOUNT,
FILE OPEN and provider-boundary restructuring.

## 2026-10-26 Proposal B KCORE structural size census v369

Resumed KCORE-only optimization after v368. Inspected the live
`file_runtime.s` and `vfs.s` source and target DOBJ symbol sections,
using the current v368 GCC lowmem linked baseline. No MRES code,
package metadata, or exports were considered optimization targets.

**Pre-link section census, not a claim of additive linked savings:**
`file_runtime.dobj`: text 1127 words, data 0, BSS 0;
`vfs.dobj`: text 770, data 27, BSS 128. The VFS mount
base pointers and active slot-count form three data words. Several
other VFS counters/policies already alias otherwise unused resident
device-state words, and mount target/root tables are MM-backed;
blindly moving them or adding a second allocation would increase
permanent RAM or break boot publication. Thus a simple mount-table
relocation is not a justified candidate.

DOBJ symbol-entry spans (a useful hotspot ranking, **not** exact
function sizes because local routines and helper code may be included):

- FILE: `file_open` 116; `file_write_words` 117 (includes common bulk
  READ/WRITE dispatch); `file_getpath` 93; `file_lock` 79;
  `file_component` 51; `file_dup2` 43; `file_readchar` 43;
  `file_writechar` 43 words.
- VFS: `vfs_mount_prevalidated` 101 (includes mount setup),
  `vfs_readdir` 91; `vfs_unmount` 82; `vfs_parent_name` 81;
  `vfs_lookup` 64; `vfs_create_op` 41 words.

**Next investigation order:** (1) compare VFS MOUNT/UNMOUNT
request and publication phases, proving lifetime and error unwind
semantics before sharing any state; (2) review FILE OPEN and bulk
READ/WRITE dispatch for large mutually exclusive pathways without
changing device ABI, EOF/APPEND handling or offset publication;
(3) check VFS READDIR/PARENT_NAME for duplicated namespace-name
handling while preserving mount-root traversal. Retain the already
MM-backed mount arrays and serialized physical-provider gateway.
Build-native tests in `daimos-testkit`, including concurrency and
error paths, remain required for any accepted change. No production
edit or new saving is claimed in this census: full linked KCORE
remains **9130** words; below 8192 requires 939 more. Strict 32K
acceptance remains deferred.

## 2026-10-26 Proposal B parent/leaf frame helper experiment v368

Inspected resident parent/leaf request setup in `file_runtime.s` for
SYMLINK, MKDIR/MKFIFO, RENAME and UNLINK/RMDIR. Tested a new tail-call
helper deriving the contiguous leaf output pointer from the directory
output pointer, replacing duplicated `MOVEI AC3` setup at two call
sites. A full linked GCC lowmem image measured KCORE **9130**, all
MRES combined **15678**, PERMANENT_LAST **15725**, equal to v367.
The helper saved no RAM and was reverted immediately; no change to
filesystem semantics or production source is retained, and no new
runtime test is claimed for this rejected candidate. Continue
Proposal B with larger resident file-runtime/VFS request handling;
MRES optimization and strict 32K acceptance remain deferred.

## 2026-10-26 Proposal B KCORE parent-mutation access helper v367

Consolidated the repeated parent write-plus-search permission call
in resident `system/kernel/fs/file_runtime.s`. `file_symlink`,
`file_rename` (both parent directories), `file_make_node` and
`file_remove` now invoke the small internal
`file_parent_write_access_v367` leaf; it loads access mask 3 and
uses a tail JRST to `file_check_access` so the original caller's
return address, arguments and error semantics are preserved.
The common helper has no persistent state and no MRES changes.

Complete GCC PDP-6 lowmem linked image versus v366: KCORE
9133 -> **9130** (-3 words), KCORE+all MRES 15681 -> **15678**
(-3), PERMANENT_LAST 15728 -> **15725** (-3). Native
`daimos-testkit` targets `target-file-pdp6`,
`target-file-walk-pdp6`, `test-daimos-vfs-stat-frame` and
`test-daimos-pre-dsh-fs-closure` PASS. This does not demonstrate
all failure branches of RENAME/SYMLINK or physical-provider
concurrency. Below-8192 KCORE target requires 939 additional
words. Strict 32K acceptance remains deferred.

## 2026-10-26 Proposal B KCORE component clear BLT audit v366

Audited the resident `file_component` five-word output `vfs_name`
zero-initialization and its PDP-6 `BLT` source/destination pointer
setup. Experiment replaced `HRLI 0,(3)` with `HRL 0,3` after loading
`1(3)` into AC0; it compiles, but the complete linked GCC lowmem
image is unchanged: KCORE **9133**, KCORE+all MRES **15681**,
PERMANENT_LAST **15728** words. The experiment was reverted rather
than retain a size-neutral rephrasing. No MRES source changes and no
new runtime acceptance are claimed. Follow-on Proposal B work should
favor large resident file-runtime/VFS structural consolidation over
more zero-gain instruction substitutions. The below-8192 KCORE target
requires 942 further words; strict 32K acceptance remains deferred.

## 2026-10-26 Proposal B KCORE indexed path byte-pointer offset v365

In `system/kernel/fs/file_runtime.s`, `file_component` constructs
its packed-SIXBIT input pointer after dividing the current character
position by six. Replaced `ADD 6,1` / `ADDI 6,1` with indexed
`ADDI 6,1(1)` to include both the path address and the one-word
counted-path header offset in one PDP-6 effective-address operation.
Both path pointers lie within the native lowmem address range; the
byte position remainder, SIXBIT pointer LH and path semantics remain
unchanged. No MRES packaging, exports or runtime was changed.

Full linked GCC lowmem KCORE 9134 -> **9133** (-1), all-MRES
combined size 15682 -> **15681** (-1), PERMANENT_LAST 15729 ->
**15728** (-1). `daimos-testkit` native PDP-6 `target-file-pdp6`,
`target-file-walk-pdp6`, `test-daimos-vfs-stat-frame`, and
`test-daimos-pre-dsh-fs-closure` PASS. A further 942 words are
needed to get strictly below 8192. Strict 32K acceptance deferred.

## 2026-10-26 Proposal B KCORE output SIXBIT pointer setup v364

`file_component` initialized its output SIXBIT byte pointer through a
literal pointer MOVE, a separate effective destination address load,
and HRR register merge. Replaced that three-instruction sequence with
`MOVEI 1,1(3)` followed by `HLL 1,[POINT 6,0]`, preserving the
right-half address and installing the same left-half byte-pointer
format. Source record parsing, component count, trailing separator
consumption and public VFS name semantics are unchanged. No MRES
source or export metadata was changed.

Full linked GCC PDP-6 lowmem KCORE 9135 -> **9134** (-1), combined
KCORE+all MRES 15683 -> **15682** (-1), PERMANENT_LAST 15730 ->
**15729** (-1). daimos-testkit `target-file-pdp6`,
`target-file-walk-pdp6`, `test-daimos-vfs-stat-frame` and
`test-daimos-pre-dsh-fs-closure` PASS. This is an accepted resident
KCORE-only size reduction. KCORE below 8192 requires 943 additional
words; strict 32K acceptance remains deferred.

## 2026-10-26 Proposal B KCORE SIXBIT path pointer construction v363

In the resident `file_component` implementation, after `DIVI` and
addition of the path-data base, AC6 already contains the desired
right-half address for the input SIXBIT byte pointer. Replaced the
three-instruction MOVe literal / HRR / MOVe sequence with one
`HLL 6,[POINT 6,0]`, installing the byte-pointer left-half while
preserving the computed right-half address. No change to byte offset,
separator handling, component-size limit or caller ABI; no MRES work.

Complete GCC PDP-6 lowmem linked image compared with v362:
KCORE 9137 -> **9135** (-2), combined KCORE+MRES 15685 ->
**15683** (-2), PERMANENT_LAST 15732 -> **15730** (-2).
`daimos-testkit` `target-file-pdp6`, `target-file-walk-pdp6`,
`test-daimos-vfs-stat-frame`, `test-daimos-pre-dsh-fs-closure`
all PASS. KCORE below 8192 requires 944 further words.
Strict 32K acceptance remains deferred.

## 2026-10-26 Proposal B KCORE file_getpath index multiply v362

The resident absolute-path formatter `file_getpath_common` uses a
five-word component array of up to sixteen path parts. Both the
upward parent-walk and reverse output walk computed `5 * depth` with
`LSH 4,2` plus `ADD 4,depth`. Replaced each two-word sequence with
single `IMULI 4,5`, preserving AC4, stack record indexing, operation
order, public ABI and pathname semantics. No MRES source was changed.

Full linked GCC PDP-6 lowmem baseline v361: KCORE 9139 -> **9137**
(-2 words), KCORE+all MRES 15687 -> **15685**, PERMANENT_LAST
15734 -> **15732**. daimos-testkit PDP-6 targets
`target-file-pdp6`, `target-file-walk-pdp6`,
`test-daimos-vfs-stat-frame` and `test-daimos-pre-dsh-fs-closure`
PASS. These are general file/VFS regression tests; a dedicated
native deep GETPATH/GETCWD path-component matrix has not yet been
run, and broader acceptance should include one. The reduction
is entirely resident KCORE. To get below 8192 from 9137 requires
946 additional words; strict 32K acceptance stays deferred.

## 2026-10-26 Proposal B shared descriptor clone v361

Consolidated duplicated resident `file_dup`/`file_dup2` descriptor
cloning in `system/kernel/fs/file_runtime.s` into the internal
`file_clone_descriptor_v361` leaf. The shared sequence copies both
contiguous FILE words, copies the descriptor mount-extension field,
reconstructs the canonical vnode and increments pipe references only
for the PIPE provider. `file_dup` preserves its returned fd across the
helper; `file_dup2` retains close-before-overwrite, equality, and error
semantics. Public FILE ABI and MRES sources remain untouched.

Full linked GCC PDP-6 lowmem disk-root measurement against v360:
KCORE **9148 -> 9139** (-9 words); KCORE+all MRES
15696 -> 15687 (-9); PERMANENT_LAST 15743 -> 15734 (-9).
`daimos-testkit` PDP-6 targets `target-file-pdp6`,
`target-file-walk-pdp6`, and checks `test-daimos-vfs-stat-frame`,
`test-daimos-pre-dsh-fs-closure` all PASS. Targeted native extracted
FILE mount-extension contract `daimos-file-mountext-v19.sh` PASS,
covering DUP/DUP2, 26-mount vnode extensions, locks, close and unmount.
This does not replace full concurrent-provider stress testing. KCORE
below 8192 needs another 948 words from the new baseline. Strict
32K acceptance remains deferred; no MRES optimization was performed.

## 2026-10-26 Proposal B shared bulk-I/O register preparation v360

Reduced permanent KCORE by three words in
`system/kernel/fs/file_runtime.s`. Resident `file_words_common` used
to build identical register arguments separately for READ and WRITE:
AC0 canonical vnode, AC1 user buffer, AC2 word count. These three
moves now execute once before the existing READ/WRITE branch. Device
selection, provider register ABI, append-at-EOF handling, lock and
sleep semantics, and descriptor offset updates remain unchanged.

Full linked GCC lowmem disk-root image versus v359: KCORE
9151 -> **9148**, KCORE+all MRES 15699 -> **15696**, and
PERMANENT_LAST 15746 -> **15743**. All MRES objects are unchanged;
the saving is entirely resident KCORE. PDP-6 `daimos-testkit`
`target-file-pdp6` and `target-file-walk-pdp6` PASS, along with
`test-daimos-vfs-stat-frame` and
`test-daimos-pre-dsh-fs-closure` PASS. This is targeted regression
coverage, not a full physical-provider concurrency stress test.
The next KCORE milestone below 8192 requires 957 further words.
Strict 32K acceptance remains deferred. No MRES size optimization
is part of this change.

## 2026-10-26 Proposal B KCORE-only file-runtime entry audit v359

Resumed Proposal B strictly on KCORE (`file_runtime.s`, its VFS
boundary); no MRES packaging or metadata work. Inspected resident
readchar/writechar/readdir descriptor-to-vnode setup and the path,
permission and create/remove request frames. Three places redundantly
copied AC1 from a saved descriptor register back into AC1 immediately
after saving it. Tested removing one such move (v358) and then all
three (v359) against the complete GCC lowmem linked image, using
the current mandatory MonitorFS MRES build configuration. Both
experiments were **size neutral**: KCORE 9151, KCORE+MRES 15699,
PERMANENT_LAST 15746. Reverted all changes, including the three-move
experiment. No runtime test is claimed for rejected experiments.

This verifies that source-level micro-deletions are insufficient after
current assembler/link folding. Continue with actual structural
file-runtime/VFS opportunities: eliminate substantial duplicated
request preparation or persistent state only with ownership/lifetime
proofs. Keep provider sleeping/serialization contract intact. MonitorFS
MRES/export size work is explicitly out of scope in this KCORE phase.
KCORE goal remains below 8192 words; strict 32K acceptance deferred.

## 2026-10-25 MonitorFS MRES linked-overhead audit v357

Audited the always-loaded MonitorFS ABI trampoline file,
`mfsdev_install_minit` in `module_minit.c`, VFS process/domain name
callers, nine export declarations in the boot MRES link, and the
relocated `monitorfs-mres.map`. Nine fixed KCORE jump words correspond
to distinct ABI services, including `mfsproc_format_slot` called by
resident VFS; nine MRES exports have non-contiguous code offsets. Thus
neither dropping the last formatter entry nor assuming contiguous
function addresses is ABI-safe. A staged C simplification caching
`SIXBIT("MFS   ")` in a local name variable was compiled and measured
against the **complete linked lowmem image**; figures remained exactly
KCORE 9151, KCORE+MRES 15699, PERMANENT_LAST 15746. Because it saved
no permanent words, the experiment was reverted without commit.

No accepted production change from v356. The combined +17 words
compared with the historical resident default remain unresolved. A
future reduction should derive export/trampoline overhead from actual
linked package metadata and preserve independent ABI entry addresses,
then validate a native boot and filesystem semantic regression. The
full-profile permanent-address overflow and strict 32K acceptance
remain deferred.

## 2026-10-25 Mandatory MonitorFS MRES install cleanup v356

Removed the remaining compile-time `MONITORFS_MRES_ENABLED` branches
around the MonitorFS MRES package declarations and boot MINIT table
entry in `system/kernel/modules/module_table.s`, and around
`mfsdev_install_minit` in `module_minit.c`. MonitorFS now has an
unconditional MINIT installer in both configurations; the resident
MonitorFS implementation cannot be selected by disabling a build flag.
The kernel assembly copy selection is also unconditional and always
uses the ABI trampoline-only `fs/monitorfs_dispatch_v14.s`. The nine
resident ABI jump words remain necessary under the current interface.

Full-linked GCC 64K lowmem image metrics: KCORE 9151, KCORE+MRES
15699, PERMANENT_LAST 15746 words, **no RAM growth** from v355.
Native `daimos-testkit` `test-daimos-monitorfs-mres-domains-v346`
PASS with an isolated lowmem PDP-6 image, exercising process stat/read,
domain directory enumeration and numeric-domain lookup under the
forced always-on package. Earlier child/zombie test v354 passed under
the same forced package model in v355. This v356 cleanup does **not**
recover the remaining +17 combined words; do not remove ABI trampolines
without proving external callers and MINIT patch relocation remain
correct. Full-profile permanent-address overflow and strict 32K
acceptance remain deferred.

## 2026-10-25 MonitorFS always-loaded MRES, no resident implementation v355

MonitorFS is now **MRES-only by construction**. The PDP-6 boot build
includes the `monitorfs-mres` package unconditionally in both lowmem
and full MRES package sets, and both boot and kernel Makefiles force
`MONITORFS_MRES_ENABLED=1` even against old command-line requests to
disable it. Kernel build `monitorfs_runtime.s` is always assembled from
`fs/monitorfs_dispatch_v14.s` rather than the old full
`monitorfs_runtime.s` + `monitorfs_swap_v63.c` implementation.
Thus the MonitorFS device, process, domain, and swapped metadata runtime
is linked into the pinned, boot-installed MRES and **not resident as
full KCORE implementation**. The unavoidable nine one-word ABI/MINIT
jump slots remain in KCORE and are patched to the relocated package at
boot; VFS's shared generic provider routing stays resident. Do not
claim literally zero MonitorFS-related KCORE words.

Full-linked lowmem GCC default build measured KCORE=9151,
KCORE+MRES=15699, PERMANENT_LAST=15746. An explicit obsolete
`MONITORFS_MRES_ENABLED=0` argument still produces these figures,
proving resident fallback is disabled. Native daimos-testkit
`test-daimos-procinfo-zombie-v354` **PASS** under this forced default
(including RUN/WAIT, PROCINFO and decimal `/PROC/10/STATE`). Existing
native device, process, and domain MRES tests v341/v343/v344/v346
remain useful. No new permanent RAM change compared with prior
opt-in MRES, but compared with former normal non-MRES default,
KCORE is reduced by 1575 words while combined RAM is increased by 17.

Full-profile GCC size-only build yields KCORE=9151,
KCORE+MRES=18463, PERMANENT_LAST=18510 and fails its existing
`040000` permanent-address gate; full-profile boot acceptance is **not
claimed**. Keep the larger size restriction and strict 32K acceptance
on the deferred backlog; do not change this limit by workaround.
Future optimization may recover the 17 combined resident words and
remove unnecessary jump slots only if ABI-safe, with full linked
measurement before acceptance. Feature is no longer opt-in.

## 2026-10-25 Native zombie PROCINFO safety and RAM-neutral fix v354

Root cause of baseline v348-v351 lowmem child-completion halt found in
`system/kernel/proc/syscall_info.s`: `vm_space_destroy` publishes
`VM_SPACE_NONE` (`-1`) in the zombie's `vm_state`, while
`sys_procinfo_user_comm` previously checked only the nonzero RH
image base and could dereference the invalid `0777777` image address
while reconstructing COMM from argv[0]. A negative VM descriptor is
now treated as unavailable and selects the existing `USER` fallback.
The argv[0] offset is also added directly from memory to AC7,
eliminating one intermediate MOVe and offsetting the one-word safety
check without altering pointer interpretation. The first experimental
explicit zombie-state check would have cost +5 KCORE words and was
replaced; the narrower VM sentinel guard initially cost +1 word; the
final combined fix has **zero full-linked permanent RAM growth**.

Native 64K GCC lowmem regression in daimos-testkit
`test-daimos-procinfo-zombie-v354` **PASS**, including the previously
failing child `PROCINFO`, pipe EOF, WAIT and decimal `/PROC/10/STATE`
S6REC lookup checks. With the final source, full-linked default
KCORE 10726, combined 15682, PERMANENT_LAST 15729 words; optional
MonitorFS MRES KCORE 9151, combined 15699, PERMANENT_LAST 15746,
unchanged against v329/v333 respective baselines. Testkit commit
`c772b54`. Related v348-v351 HALTs are resolved in this tested
configuration, independent of MonitorFS extraction. The broader
MRES swapped-process metadata acceptance remains outstanding;
MonitorFS MRES stays opt-in, strict 32K gate deferred.

## 2026-10-25 Native lowmem SYS_PROCINFO immediately after RUN v351

Adapted the baseline native one-child RUN/PIPE test to call
`dsys_procinfo(pid,&info)` immediately after `SYS_RUN` returns,
before closing parent pipe descriptors or waiting for the child.
Using 64K GCC lowmem, `MONITORFS_MRES_ENABLED=0`, native PDP-6 trace
reached `D6FS LOADED`, then `<RSQT` before halting later at
`PC: 000000 (LUUO00 0,300)`. The `T` marker is emitted after the
first `PROCINFO(2)` returned zero; the fixture also checked that
`info.pid` equals 2. Hence initial child-slot PROCINFO dispatch and
record population work. The subsequent `wait_zombie_by_eof` path
still fails when querying PROCINFO after writer close, EOF read and
reader close, as previously shown in v348/v349. The defect is
associated with child completion/zombie state or with its preceding
lifecycle transition, not ordinary PROCINFO dispatch, and reproduces
without MonitorFS MRES.

The experimental fixture was withdrawn after the diagnostic run;
unrelated dirty testkit state was left untouched. No production
source or permanent RAM changes. Next isolate PROCINFO after each
individual teardown operation and study process slot zombie-layout
in `syscall_info.s`; reject any speculative workaround. MonitorFS
MRES native namespace acceptance v341/v343/v344/v346 remains PASS,
feature opt-in with KCORE=9151 and combined 15699 (+17 default),
strict 32K acceptance deferred.

## 2026-10-25 Native lowmem no-child SYS_PROCINFO acceptance v350

Added `daimos-testkit` native 64K PDP-6 GCC lowmem test
`test-daimos-procinfo-current-v350` with `MONITORFS_MRES_ENABLED=0`.
User INIT calls `dsys_procinfo(1,&info)` without RUN, PIPE, WAIT or
other child lifecycle activity, verifies that the UUO returns zero and
that `info.pid == 1`, then emits the completion marker. Target run
**PASS**. Consequently, the native UUO 057 path is functional for
the initial process with a mapped caller result buffer.

Earlier v348-v349 tests consistently halted inside PROCINFO(2)
after completing child RUN, pipe-writer close, pipe EOF read and
pipe-reader close, regardless of the MonitorFS MRES switch. The
remaining fault is therefore related either to non-initial/zombie
process-table record handling or to state left by the preceding
lifecycle operations, not merely to invoking PROCINFO itself.
Check `sys_procinfo` handling of a zombie slot and its `COMM`
reconstruction, plus native syscall mapping/return invariants;
compare a direct PROCINFO(2) after RUN without pipe teardown.

The accepted testkit commit is `7dc7023`; the test touches no kernel
production source. Fixed KCORE baseline 10726, combined 15682;
optional MonitorFS MRES remains KCORE 9151 and combined 15699 (+17).
Feature stays opt-in, strict 32K acceptance deferred.

## 2026-10-25 Baseline lowmem SYS_PROCINFO comparison v349

Compared the one-child PIPE/RUN/EOF/PROCINFO probe against an isolated
64K GCC lowmem image with `MONITORFS_MRES_ENABLED=0`, matching v348
apart from disabling the extracted MonitorFS MRES. Both builds booted
through `D6FS LOADED`, and both emitted `<RSWabcd` before HALT at
`PC: 000000 (LUUO00 0,0)`. Thus the failure in `dsys_procinfo()`
also occurs in the default lowmem baseline. It is **not** evidence of
a defect introduced by MonitorFS MRES extraction. The probe did not
access MonitorFS namespace paths.

Source inspection locates the syscall ABI in `userland/libc/syscall.s`
(`dsys_procinfo: uuo 057,0(1)`), native dispatch in
`system/kernel/proc/syscall_dispatch_pdp6.s` (map result buffer, call
`sys_procinfo`), and the implementation in
`system/kernel/proc/syscall_info.s`. The root cause of the halt remains
unresolved; next step is a minimal zero-child SYS_PROCINFO probe and
trap/return tracing, with assembler/compiler defects prioritized.
The temporary failing baseline testkit fixture was withdrawn with
unrelated dirty testkit files intact. No production code or RAM change.

Prior accepted MRES tests v341/v343/v344/v346 remain PASS; optional
KCORE 9151 and combined 15699 (+17 versus default). Feature stays
opt-in; strict 32K acceptance deferred.

## 2026-10-25 Proposal B one-child SYS_PROCINFO trap localization v348

Created a temporary daimos-testkit native lowmem GCC 64K PDP-6
`MONITORFS_MRES_ENABLED=1` single-child RUN/PIPE lifecycle probe.
Instrumented the four distinct steps of `wait_zombie_by_eof` after
parent's RUN return: `a` before closing parent writer, `b` after
writer close, `c` after reading EOF, `d` after closing reader, `e`
only after returning from `dsys_procinfo`. PDP-6 trace reached
`D6FS LOADED`, emitted `<RSWabcd`, then halted at
`PC: 000000 (LUUO00 0,0)`. Thus writer close, EOF read, reader
close all completed successfully. The failure occurs within or while
returning from `dsys_procinfo(pid,&info)`, before `e` could print.
The libc entry uses `uuo 057,0(1)` in `userland/libc/syscall.s`.
No MonitorFS path is used by this probe. Diagnose the SYS_PROCINFO
UUO dispatch/return path and compare identical lowmem configuration
with `MONITORFS_MRES_ENABLED=0` before attributing to MRES.

The failed tracing fixture was withdrawn; all other dirty testkit
files were preserved. No production code or memory footprint changed.
Accepted native MRES namespaces v341/v343/v344/v346 remain passing.
Optional KCORE 9151, KCORE+MRES 15699 (+17 vs default), opt-in,
strict 32K acceptance deferred.

## 2026-10-25 Proposal B one-child RUN/WAIT isolation v347

Adapted the existing full-profile process decimal native fixture to an
isolated 64K lowmem GCC image with `MONITORFS_MRES_ENABLED=1`, exactly
one child rather than nine, and markers for pipe creation (`N`),
`SYS_RUN` entry (`R`), return from RUN (`S`), and entering the
zombie-by-pipe/PROCINFO check (`W`). The PDP-6 boot reached D6FS
LOADED; userspace emitted `<NRSW` and then halted at
`PC: 000000 (LUUO00 0,300)`. Thus the pipe creation and first RUN
returned to the parent successfully. The issue occurs inside the
completion routine, which closes the parent's pipe writer, consumes
pipe EOF, closes the reader, then checks `dsys_procinfo` for the zombie.
**No MonitorFS path was accessed** in this probe. The remaining
fault must be separated into those pipe and PROCINFO steps, and
lowmem baseline behavior should be compared before attributing to
MRES. The failed experimental testkit fixture was withdrawn; unrelated
dirty files remain untouched.

No production modification or RAM savings. Accepted v341/v343/v344/v346
MonitorFS MRES namespace tests retain PASS status. Permanent optional
KCORE 9151, combined 15699 (+17 over default). Feature remains
opt-in; strict 32K acceptance deferred.

## 2026-10-25 Proposal B MonitorFS MRES domain lookup acceptance v346

The failed v345 domain enumeration-to-STAT test was caused by a testkit
SIXBIT encoding error: literal `/` requires octal `017`, but v345
constructed the prefix with `057`. Corrected the prefix in a new
versioned daimos-testkit native test `test-daimos-monitorfs-mres-domains-v346`.
With `MONITORFS_MRES_ENABLED=1`, a 64K GCC lowmem PDP-6 image
successfully validates the v343 process and v344 domain-root checks,
reads a numeric domain ID from READDIR, constructs the corresponding
`/MONITOR/DOMAINS/<id>` SIXBIT path with correct separators, then
successfully STATs it. Native test **PASS**. The domain identity
mismatch from v345 was a fixture bug, not evidence of a kernel
namespace defect. Testkit commit `26c6404`.

No kernel production code or RAM changes. The optional MRES remains
KCORE 9151 and combined 15699, +17 words versus default v329.
Child RUN/WAIT and swapped-process metadata acceptance remain open;
strict 32K acceptance deferred.

## 2026-10-25 Proposal B MonitorFS MRES domain identity probe v345

Extended v344's successful 64K native MRES domain-root test to require
one `/MONITOR/DOMAINS` directory entry, validate a one-to-three-digit
SIXBIT domain ID, construct `/MONITOR/DOMAINS/<id>` from that returned
name, and call STAT on the resulting domain directory. The native PDP-6
image booted and passed v343's process checks and the v344 domain-root
checks, but reported `<ABCDE!c`: domain-entry identity was numeric,
yet STAT on the constructed path failed. This may be a fixture
path-encoding error or a real MonitorFS domain lookup inconsistency;
not established. The fixture's new code was withdrawn after failure,
and pre-existing dirty testkit files were preserved. Do not cite this
as native domain-leaf acceptance. Future investigation should compare
an independently encoded literal domain path, the returned directory
name and the VFS lookup chain before modifying the kernel.

No production code changes or RAM savings. The accepted native domain
root v344 remains PASS. Optional MonitorFS MRES is KCORE 9151 words,
KCORE+MRES 15699 (+17 versus default); 32K acceptance remains deferred.

## 2026-10-25 Proposal B MonitorFS MRES domain root access v344

Added `daimos-testkit` native 64K PDP-6 GCC lowmem test
`test-daimos-monitorfs-mres-domains-v344` with
`MONITORFS_MRES_ENABLED=1`. Extends the proven v343 no-child
`/PROC/1/STATE` STAT/open/read/S6REC/close checks with
`STAT /MONITOR/DOMAINS`, open the domain directory, one READDIR
(no negative/error return allowed), and close. Native simulator
run **PASS**. This verifies service dispatch and basic domain-root
access; it does **not** yet verify returned entry identities,
child/decimal PID lifecycle, specific domain leaf data, or swapped
process metadata. Testkit commit `9ed13b0`.

No production code changes and no new RAM savings. Opt-in MonitorFS
MRES remains KCORE 9151, KCORE+MRES 15699 (+17 permanent words)
versus default KCORE 10726, KCORE+MRES 15682. Strict 32K acceptance
deferred. Next acceptance should validate specific domain entries
and metadata values, then isolate child process/RUN regressions.

## 2026-10-25 Proposal B native minimal MonitorFS MRES process access v343

Added `daimos-testkit` target `test-daimos-monitorfs-mres-proc-basic-v343`
with a dedicated GCC lowmem, 64K PDP-6 image and
`MONITORFS_MRES_ENABLED=1`. Unlike the failed full process lifecycle
probe v342, the native test does not RUN or create children. It checks
`STAT /PROC`, `STAT /PROC/1/STATE`, OPEN/READ/CLOSE of that state
leaf and a nonempty S6REC text header. Simulator **PASS**. This proves
basic MonitorFS process namespace services are callable through the
relocated MRES; child/RUN/WAIT, decimal PID 10, domain namespace,
and swapped-process argument metadata still require separate tests.

No production source or permanent RAM change. Optional MRES stays
KCORE 9151 and KCORE+MRES 15699 (+17 over default), remains opt-in;
strict 32K acceptance deferred. The native v343 testkit commit is
`3d82596`; unrelated testkit changes were not staged.

## 2026-10-25 Proposal B MonitorFS MRES process acceptance probe v342

Attempted a native process-namespace regression by adapting the
existing full-profile `daimos-monitorfs-process-decimal-v1` fixture
to a 64K GCC lowmem image with `MONITORFS_MRES_ENABLED=1`.
The fixture exercises child RUN/WAIT, two-digit decimal `/PROC/10/STATE`
lookup, and S6REC zombie-state payload. Image build and boot succeeded
and userspace emitted the initial `<` marker; then SIMH halted with
`PC: 000000 (LUUO00 0,300)` before any normal fixture error or final
`P` marker. Thus the test **did not pass**. The defect has not been
isolated to MonitorFS rather than the child/RUN or lowmem assumptions.
The versioned experimental fixture was removed rather than committed
as accepted test coverage. All pre-existing unrelated testkit changes
were preserved. No DAIMOS production change or new RAM saving.

The accepted native opt-in device namespace test v341 continues to
PASS. Further process acceptance should first isolate minimal
`/PROC` path STAT/open/read from the multi-child RUN/WAIT fixture,
then add child lifecycle and swapped metadata cases individually.
MonitorFS remains opt-in, KCORE=9151, KCORE+MRES=15699 (+17 over
v329 default); strict 32K acceptance remains deferred.

## 2026-10-25 Proposal B native MonitorFS MRES devices acceptance v341

Added `daimos-testkit` native PDP-6 target
`test-daimos-monitorfs-mres-device-v341`, running an isolated GCC
lowmem `MONITORFS_MRES_ENABLED=1` image with matching 64K RAM.
The existing full-profile device-view test expected 0377 octal and
initially failed `!5`, as the lowmem profile omits PTR/OCNSLS/DRM.
The new fixture specifies the lowmem expected present set 0153 octal
(CTY0, CLK0, TTY0, DSK0, D6SET0), with absence checks inherited from
the source fixture. Native PDP-6 run **PASS**: MonitorFS device
namespace works through the relocated MRES entrypoints following the
v340 boot-source ordering correction. This is not yet a test of
process/domain leaves, swapped-process record reads, or module unloading.

No kernel production code changed; no new size saving is claimed.
Continue to use the accepted default v329 KCORE 10726 and combined
15682 words; the opt-in MonitorFS MRES is KCORE 9151 and combined
15699 (+17 words). The MRES mode remains opt-in pending broader
runtime acceptance; strict 32K acceptance remains deferred.

## 2026-10-25 MonitorFS MRES package-source ordering boot fix v340

Confirmed that v339's `REVERS` (six-character SIXBIT diagnostic)
actually appeared in the full simulator log, although the earlier
grep for `REVERSE` omitted it. The underlying fault was a mismatch
between MRES source layout and MINIT execution order: lowmem
`MRES_PACKAGE_OBJS` placed `dtfs-mres` **before** `blockset-mres`,
while `module_table.s` runs `blockset_minit` **before** `dtfs_minit`.
`module_run_minits` requires monotonically increasing nonzero package
source addresses to reclaim consumed boot MRES source intervals.

Fixed the lowmem package list in `system/boot/pdp6/Makefile` to link
BLOCKSET before DTFS, matching MINIT. This is the underlying ownership
ordering fix, not a relaxation of MM overlap checks. Fresh isolated
64K GCC lowmem MonitorFS MRES image built and booted in PDP-6 SIMH:
`BSET LOADED`, `D6FS LOADED`, `INIT V1`. Simulator ran until explicit
17-second timeout (exit 124); **no prior post-BSET `kinit_halt`**.
INIT reported `MEMFS NOT MOUNTED ON /TEMP`, not yet investigated.
Native MonitorFS namespace/swapped-process semantic acceptance remains
open, so the optional feature stays disabled by default.

Full linked optional lowmem `permanent-size` unchanged by reordering:
KCORE 021677 (9151), KCORE+MRES 036523 (15699),
PERMANENT_LAST 036602 (15746), +17 words combined versus default v329
(10726 / 15682 / 15729). The +17 cost is provisionally acceptable;
optimization of that overhead follows functional acceptance. Strict
32K gate deferred.

## 2026-10-25 MonitorFS MRES MINIT halt probes v338-v339

Investigated v337's post-BLOCKSET `kinit_halt` with two separate
temporary diagnostic builds, both using matching 64K PDP-6 SIMH
configuration. The first printed `GAPERR` only on failed
`mm_add_free(previous_package, package - previous_package)`; the
second additionally printed `REVERSE` for descending source-package
addresses. Neither marker appeared before the HALT, so neither branch
was conclusively identified as the caller. A failed debug-output
routine or different `kinit_halt` caller has not been excluded.
Both builds still halted after `BSET LOADED`. The instrumentation
was removed entirely; source is unchanged. The previous v337
inference that `mm_add_free` was specifically rejecting an interval
remains **unproven** and must not be used as the basis for an MM fix.
Next: capture the PDP-6 halted CPU's return chain or trace calls to
`kinit_halt` directly, then inspect the implicated MM/initializer
condition. MonitorFS MRES remains opt-in; v333 measured fixed KCORE
9151 words and combined KCORE+MRES 15699 versus default 10726/15682.
No runtime PASS or additional saving claimed; strict 32K deferred.

## 2026-10-25 MonitorFS opt-in MINIT source-reclaim localization v337

Built an isolated 64K lowmem MonitorFS-MRES image with temporary
SIXBIT `NEXT` and `DONE` trace markers around each call in
`module_run_minits` (not retained in production). The PDP-6 simulator
trace shows `BSET LOADED`, then `DONE`, followed by a HALT without the
next `NEXT`. The failure therefore follows successful return from the
BLOCKSET initializer and occurs before the next initializer call.
The intervening branch validates/free-publishes the previous package
source between two successive MRES package addresses. In the
instrumented linker map `blockset_mres_package=075633` and
`d6fs_mres_package=075672` octal, so the expected reclaimed gap is
037 octal words. Since the next `NEXT` marker follows the reclamation
branch and the addresses are increasing, `mm_add_free` rejecting this
source interval is the leading diagnosis; confirm the exact
allocation/arena overlap or MM return code before any fix. `mm_add_free`
explicitly rejects overlapping extents and arenas, and no bypass is
acceptable. The diagnostic KINIT edits were reverted immediately and
no kernel production source was changed. The option remains off by
default; +17 combined permanent words remains provisionally acceptable
if full runtime acceptance is achieved. Strict 32K gate deferred.

## 2026-10-25 MonitorFS MRES post-BSET boot localization v336

Using the existing optional `MONITORFS_MRES_ENABLED=1` image from
v333, compared unmodified boot media under SIMH configured for 64K
and 256K physical memory (versioned simulator INI copies). Both
executions reached the DAIMON KINIT banner and reported
`BSET LOADED`, then halted through `kinit_halt` at octal PC 063116.
This excludes the original 32K/64K configuration mismatch as the
sole reason for the *later* halt; simply increasing simulator RAM does
not resolve it. The module table places `d6fs_minit` after BLOCKSET,
so the next investigation should instrument the exact pre-D6FS/MRES
call boundary, plus early MM reservations, without asserting that D6FS
or MonitorFS caused the halt. A generic `kinit_halt` PC alone cannot
attribute the caller. No production code changed and the default
MonitorFS MRES option remains off. The +17 permanent-word difference
is provisionally acceptable contingent on real boot and service tests;
no additional RAM saving or runtime PASS is claimed.

## 2026-10-25 MonitorFS opt-in boot diagnosis v335

Examined the opt-in 64K MonitorFS-MRES image boot from v334. The
original `?B`, `HALT 000544` was produced with an inconsistent boot
configuration: the lowmem profile's generated `boot.ini` still set
`cpu 32k` while `BOOT_CORE_WORDS=0200000` built a 64K image.
An isolated boot-configuration copy using `set cpu 64k` proceeds past
Stage1 and emits the DAIMON V0.5 / PDP6 / MEM 64K KINIT banner and
MINIT statuses through `BSET LOADED`, but then halts at `kinit_halt`
(PC 063116). Thus there is still a later boot/installation failure;
no functional MonitorFS runtime acceptance is claimed. The boot failure
has not been attributed to the MRES itself. A matching CPU-size option
must accompany larger-image tests; do not weaken stack-reserve checks.

The opt-in MonitorFS MRES link fix from v333 is retained and remains
disabled by default. The proposed +17 combined permanent words is
provisionally acceptable if runtime validation succeeds, but should
be optimized afterward. Memory figures remain KCORE=9151,
KCORE+MRES=15699 (enabled) versus 10726/15682 (default).
The strict 32K gate remains deferred.

## 2026-10-25 MonitorFS MRES build and boot gate v334

The optional MonitorFS MRES full link (v333) saves 1575 KCORE words
but increases KCORE+MRES and PERMANENT_LAST by 17 words. The 17-word
cost is an acceptable provisional tradeoff if native behavior is correct;
seek to recover it only after the runtime acceptance gate, without
adding state or compromising module boundaries.

`BOOT_PROFILE=lowmem MONITORFS_MRES_ENABLED=1` with default 32K
`BOOT_CORE_WORDS` failed `mkimage` because KINIT left less than the
configured stack reserve. Without relaxing that protection, rebuilt
with isolated `BOOT_CORE_WORDS=0200000` (64K) and the image target
completed. A SIMH boot using its generated `boot.ini` then printed
`?B` and halted at PC `000544` (`HALT 544`). Consequently, successful
MRES linking/image creation is *not* successful runtime installation
or MonitorFS namespace acceptance. The failure needs diagnosis before
enabling MRES by default. The default v329 configuration is unchanged.
Strict 32K acceptance stays deferred.

## 2026-10-25 Proposal B MonitorFS MRES missing swap-helper build fix v333

Fixed the opt-in `MONITORFS_MRES_ENABLED=1` package link in
`system/boot/pdp6/Makefile`: compile the existing
`fs/monitorfs_swap_v63.c` with PDP-10 GCC into a distinct MRES object
and link it alongside the generated namespace service object. The
normal non-MRES KCORE build remains unchanged. This resolves the
previous `undefined symbol: mfsproc_swapped_read_words_v63` failure
without dropping functionality, stubbing a syscall, or adding KCORE
resident storage. A fresh isolated GCC lowmem `permanent-size` build
now **links successfully** with MonitorFS MRES enabled.

Measured default v329 versus experimental enabled profile:
KCORE 024746 (10726) -> 021677 (9151), a 1575-word reduction;
MonitorFS MRES adds 003070 (1592) words; KCORE+MRES 036502
(15682) -> 036523 (15699), an increase of 17 permanent words;
PERMANENT_LAST 036561 (15729) -> 036602 (15746), likewise +17.
This is a large **fixed-KCORE size reduction**, but **not a total
permanent occupancy reduction**. The feature stays opt-in, not the
accepted default permanent-RAM optimization baseline.

A successful link does not prove boot, MINIT export patching, pinned
module lifetime, swapped-process reads, or namespace semantics; those
are outstanding native runtime acceptance gates. Existing
`mfsdev_install_minit` already patches nine dispatch entries (contrary
to the earlier assumption that the installer was absent). Do not
mark MonitorFS extraction complete until those gates pass. Strict 32K
acceptance remains deferred.

## 2026-10-25 Proposal B MonitorFS MRES configuration investigation v332

Re-examined the effective build rather than inferring residency from the
KCORE object names. `MONITORFS_MRES_ENABLED` defaults to 0; its normal
KCORE build concatenates `monitorfs_runtime.s` with the compiled
`monitorfs_swap_v63.c` implementation. With the option set to 1,
`system/kernel/Makefile` **instead substitutes**
`monitorfs_dispatch_v14.s` for the resident MonitorFS object, leaving
nine one-word `JRST kret_neg1` service entrypoints. Thus the KCORE
object list alone does not show which implementation actually resides
there. This corrects the earlier interpretation of the Makefile.

Attempted isolated GCC lowmem full-link build with
`MONITORFS_MRES_ENABLED=1` in a distinct TMPDIR build root. KCORE
assembly/link succeeded, but the optional MonitorFS package link failed
with `undefined symbol: mfsproc_swapped_read_words_v63`. The
`monitorfs-mres-probe-v15.py` generator rewrites the full assembler
source, which references that helper; the helper is supplied by
`monitorfs_swap_v63.c` in the normal resident build but was not included
in this package. The generator explicitly says its output is a
**non-installable probe**. `monitorfs_dispatch_v14.s` explicitly says
MINIT must still patch its nine JRST destinations after MRES relocation
and pin the MRES while MonitorFS is reachable. Neither link repair nor
runtime installation was attempted; the enabled configuration is NOT
a working MonitorFS extraction. No complete permanent-size comparison
or native enabled-mode acceptance can be claimed. No source changes
were made. Last accepted standard lowmem baseline remains v329:
KCORE 024746 (10726), KCORE+MRES 036502 (15682), PERMANENT_LAST
036561 (15729), octal values followed by decimal in parentheses.

Next B stage: supply the swapped-reader code/dependencies to the MRES
package properly, verify package relocation/link and resident entries,
implement and test MINIT patching and lifetime/pinning, measure both
profiles, and test MonitorFS device/process/domain namespace on the
PDP-6. Strict 32K acceptance remains deferred.

## 2026-10-25 Proposal B MonitorFS device/namespace audit v331

Inspected the resident `monitorfs_runtime.s` device-name lookup,
READDIR, device-ID validation, special TTYDPY0 name handling,
leaf selection and IO type classification. The already shared
`mfsdev_name_length`, `mfsdev_validate_id`, `mfsleaf_lookup` and
`mfsdev_readdir_store` leaves account for the obvious overlapping
operations. The D6SET0 special leaf count and TTYDPY0 seventh character
must remain exceptional. No safe additional resident reduction was
established: no production code changed, and no RAM saving is claimed.

`daimos-testkit` GCC `test-daimos-monitorfs-devices-online` PASSed on
PDP-6 after its independent boot-image build (roughly 200 seconds).
This is a native MonitorFS device-visibility test, not a comprehensive
process-namespace or multi-process provider test. The attempted v330
UNLINK/RMDIR type-matrix fixture remains unaccepted; no false regression
PASS was recorded. Preserve the last measured v329 GCC lowmem full link:
KCORE 024746 (10726 decimal), KCORE+MRES 036502 (15682 decimal),
PERMANENT_LAST 036561 (15729 decimal). Strict 32K acceptance remains
deferred. Further B source work should target MonitorFS process/domain
namespace lifetime and file-path permission/lookup interactions, with
whole-image measurement before acceptance.

## 2026-10-25 Proposal B remove-type native fixture investigation v330

Attempted a dedicated PDP-6 UNLINK/RMDIR type-matrix fixture by extracting
v328's actual `file_remove` type check and iterating 16 STAT-type / action
cases. The initial fixture had undefined link symbols and incorrect test
expectation/loop instructions; those were corrected, but `p10run`
continued to time out (SIMH status 124) without readable test exit.
No native acceptance is claimed, and the testkit experiment was removed
rather than committing an unverified fixture. Existing unrelated testkit
edits were preserved. No production code changed; retain the last v329
full-image measurement (KCORE 024746, KCORE+MRES 036502,
PERMANENT_LAST 036561 octal). Dedicated native UNLINK/RMDIR semantics
remain unverified. Strict 32K acceptance remains deferred.

## 2026-10-25 Proposal B CREATE/SYMLINK result publication v329

The shared `vfs_create_store_result` epilogue formerly loaded the
caller's output-node pointer from `(17)` into AC4 before storing the
provider-result vnode. PDP-6 indirect addressing can publish the
result directly with `MOVEM 3,@(17)`, eliminating the intermediary
register load. The preceding success check, mount-ID inheritance,
caller-owned pointer and three-word stack unwind remain unchanged;
CREATE/MKDIR/MKFIFO and SYMLINK retain the existing provider ABI.

Fresh full GCC lowmem disk-root linked image compared with v328:
KCORE 024747 (10727 decimal) -> 024746 (10726), KCORE+MRES
036503 (15683) -> 036502 (15682), PERMANENT_LAST 036562 (15730)
-> 036561 (15729). One permanent word saved, no added state.

GCC daimos-testkit `target-file-pdp6`, `target-file-walk-pdp6`,
`test-daimos-vfs-stat-frame`, and `test-daimos-pre-dsh-fs-closure`
PASS. `test-daimos-d6fs-context-dispatch` and
`test-daimos-vfs-storage-reservation` also PASSed in this review.
The existing tests do not separately prove every CREATE/SYMLINK
failure-mode combination. Strict 32K acceptance remains deferred.

## 2026-10-25 Proposal B UNLINK/RMDIR type normalization v328

Consolidated `file_remove`'s duplicated directory and non-directory
STAT result branches. Normalizing `VFS_TYPE_DIR` (1) to a boolean and
comparing it with the existing requested-directory flag preserves UNLINK
(reject directories) and RMDIR (accept directories only), without
changing the parent permission, backing-busy or provider removal paths.
No ABI, new storage or provider dispatch changes.

Full linked GCC lowmem disk-root image compared with accepted v324:
KCORE 024752 (10730 decimal) -> 024747 (10727), KCORE+MRES
036506 (15686) -> 036503 (15683), PERMANENT_LAST 036565 (15733)
-> 036562 (15730). Three permanent words saved, no MRES growth.

GCC daimos-testkit `target-file-pdp6`, `target-file-walk-pdp6`,
`test-daimos-vfs-stat-frame` and `test-daimos-pre-dsh-fs-closure`
all PASS. A dedicated native UNLINK/RMDIR type matrix is still useful
for stronger semantic acceptance. The strict 32K acceptance gate
remains deferred.

## 2026-10-25 Proposal B expanded native provider gateway test v327

Expanded daimos-testkit's v326 PDP-6 gateway oracle into a distinct
`test-daimos-provider-lock-native-v327` target. Native simulator PASS
covers the v326 locked-wait/register-recovery and MEMFS bypass cases,
plus unlocked D6FS and TSFS table dispatch and rejection of provider
IDs outside the 4..7 range. This executes the extracted production
assembly gateway with test-controlled service jumps and wait/wakeup;
it is not an actual competing-process filesystem I/O test. The
multi-process physical-provider contention acceptance remains open.

No production source changed and no new RAM saving is claimed.
Last fully linked GCC lowmem v324 baseline remains KCORE 024752
(10730 decimal), KCORE+MRES 036506 (15686), PERMANENT_LAST
036565 (15733). No strict 32K acceptance work was performed.

## 2026-10-25 Proposal B PDP-6 provider lock native probe v326

Added `daimos-testkit` `test-daimos-provider-lock-native-v326` and ran it
successfully on PDP-6 via `p10run`. The fixture extracts the real
resident provider gateway and uses a controlled wait oracle that initially
holds the lock, releases it through `proc_wait_event`, and deliberately
clobbers request registers before retry. The production save/restore path
returns the original arguments to the provider, preserves its result across
wakeup and releases the lock. The MEMFS path bypasses serialization.
This is native execution of production gateway instructions with oracle
wait/wakeup services, **not** proof of true multi-process scheduler contention
or real physical-provider I/O. No production source changed. The accepted
v324 full linked baseline remains KCORE 024752 (10730 decimal),
KCORE+MRES 036506 (15686 decimal), PERMANENT_LAST 036565 (15733).
Strict 32K acceptance remains deferred.

## 2026-10-25 Proposal B provider lock invariants gate v325

Audited the v324 resident provider dispatch and shared transfer-workspace
lock. The private 7-word request save survives the `proc_wait_event`
loop, the ready lock is acquired before registers are restored, and
provider completion releases the lock and wakes waiters while preserving
the returned AC1. MEMFS still bypasses serialization; DTFS/D6FS/TSFS
continue to serialize over potentially sleeping I/O. No production code
change or memory saving is claimed. The accepted full-linked baseline
remains v324: KCORE 024752 (10730 decimal), KCORE+MRES 036506
(15686 decimal), PERMANENT_LAST 036565 (15733 decimal).

Added `daimos-testkit` target `test-daimos-provider-lock-contract-v325`
(PASS). It enforces assembly source-level register/lock invariants only;
it is not a scheduler contention test. The testkit had unrelated local
edits which were preserved. Further reentrancy changes require actual
multi-process sleep/wakeup and concurrent-provider native acceptance.
Strict 32K acceptance remains deferred.

## 2026-10-25 Proposal B provider table index normalization v324

Normalized the provider number (4 MEMFS through 7 TSFS) to a 0..3
service-table index on entry to `fs_provider_reg_call`. The MEMFS
fast path remains direct and non-sleeping. The other providers retain
the seven-register process-private save, lock wait, dispatch and wakeup;
the index is now saved instead of the original provider number, removing
the redundant subtraction after a lock acquisition. Invalid provider
numbers still return `fs_mres_no_service`. No ABI or persistent storage
changes were made.

Full GCC lowmem disk-root linked image versus accepted v322:
KCORE 024755 (10733) -> 024752 (10730), KCORE+MRES 036511
(15689) -> 036506 (15686), PERMANENT_LAST 036570 (15736) ->
036565 (15733). Saving: three permanent words, unchanged MRES sizes.

GCC daimos-testkit `target-file-pdp6`, `target-file-walk-pdp6`,
`test-daimos-vfs-stat-frame`, and `test-daimos-pre-dsh-fs-closure`
PASS. These tests do not exhaustively exercise concurrent lock waiters;
a targeted stress regression remains useful. Strict 32K acceptance
and further JRST folding remain deferred.

## 2026-10-25 Proposal B SETATTR unwind experiment v323 (reverted)

Re-tested reusing `vfs_mutate3_ro` for `vfs_setattr_ro` in the
current v322 GCC lowmem configuration. Although the source contains
one fewer inline cleanup instruction, the full linked image **grew**:
KCORE 024755 -> 024756 octal (10733 -> 10734 decimal),
KCORE+MRES 036511 -> 036512 octal (15689 -> 15690),
PERMANENT_LAST 036570 -> 036571 octal (15736 -> 15737).
This is a one-word permanent RAM regression. The experiment was
reverted immediately; no semantic acceptance claimed. Retain v322 as
the measured baseline. Continue Proposal B with structural
file-runtime/VFS/provider consolidation, not SETATTR cleanup sharing.
Strict 32K acceptance remains deferred.

## 2026-10-25 Proposal B VFS CREATE mode staging v322

The common CREATE/MKDIR/MKFIFO request staging in `vfs_create_common`
carried the umask-filtered mode through an unnecessary intermediate AC3
assignment before moving it into AC4. Passing the masked value directly
from AC6 into AC4 retains the fifth argument, original call conventions,
provider request order and mode semantics. No new state or dispatch.

Full GCC lowmem disk-root linked image against v321: KCORE 024756
(10734 decimal) to 024755 (10733); KCORE+MRES 036512 (15690) to
036511 (15689); PERMANENT_LAST 036571 (15737) to 036570 (15736).
Savings: one permanent word. Component MRES sizes are unchanged.

`daimos-testkit` GCC `target-file-pdp6`, `target-file-walk-pdp6`,
`test-daimos-vfs-stat-frame`, and `test-daimos-pre-dsh-fs-closure`
all PASS. No dedicated native CREATE/mode matrix was added in this
step. Architectural follow-up should seek meaningful savings in
request frames and provider interactions rather than new dispatch
abstractions. Strict 32K acceptance and JRST folding remain deferred.

## 2026-10-25 Proposal B file lock mount decode v321

Consolidated the identical two-bit mount-extension lookup in the
resident FLOCK conflict and update passes into a single internal leaf
helper, preserving AC4, the public descriptor/mount ABI, lock ownership,
and shared/exclusive/unlock semantics. This is not a new provider dispatch
or permanent storage allocation. The experiment was present as an
uncommitted workspace change before this audit; it was independently
measured against committed v318 source with full GCC lowmem disk-root
links. Baseline: KCORE 024764 (10740 decimal), KCORE+MRES 036520
(15696), PERMANENT_LAST 036577 (15743). Accepted candidate: KCORE
024756 (10734), KCORE+MRES 036512 (15690), PERMANENT_LAST 036571
(15737). Net saving: 6 permanent words; MRES component sizes unchanged.

GCC daimos-testkit target-file-pdp6, target-file-walk-pdp6,
test-daimos-vfs-stat-frame and test-daimos-pre-dsh-fs-closure all PASS.
The file-runtime native fixture exercises shared/exclusive locking,
conflict, unlock and descriptor duplicate behavior. An additional
explicit cross-mount high-extension collision scenario is still a
valuable follow-up; do not claim that narrow scenario was separately
tested here. The 32K acceptance gate and JRST folding remain deferred.

## 2026-10-25 v318 EXEC RT cleanup native acceptance v320

The previously linked and measured v318 two-word EXEC-loader cleanup
is now native regression accepted for the covered failure and ownership
paths. `make COMPILER=gcc test-daimos-exec-rt-owner` in `daimos-testkit`
PASSed with `DAIMOS_EXEC_OBSERVE_SECONDS=120`, verifying RT rollback
at header and post-VM decoder failure, existing-owner preservation,
RT admission and release. The default 15-second run timed out at SIMH
boot without reaching assertions; the extended run completed.
The separate `test-daimos-exec-create-oom` PASS is recorded at v319.

Accepted comparative lowmem KCORE size: v316 `024766` to v318
`024764` octal, saving two words post-fold, with no additional
persistent data. This does not constitute strict 32K acceptance.
The unrelated working-tree file-runtime experiment is not included.

## 2026-10-25 EXEC allocation-failure native retest v319

`make -C ../daimos-testkit COMPILER=gcc test-daimos-exec-create-oom`
PASSed against the current v318 production EXEC loader. The independent
PDP-6 test image printed `__kcore_low_end=025036` and
`daimos-exec-create-oom-v205: PASS (staged VM cannot fit physically;
low-memory EXEC replaced PID 1)`. This is positive native evidence for
the low-memory EXEC retry after a staged allocation failure. The test
image has a different build configuration; its endpoint must **not**
be compared with the v318 `024764` lowmem permanent-size image.

The native `test-daimos-exec-rt-owner` timeout described under v318
remains unresolved; this retest alone does not establish full v318 RT
release semantics. No production change or new permanent-size gain was
made here. An unrelated uncommitted `file_runtime.s` experiment in the
workspace was left untouched.

## 2026-10-25 EXEC loader RT cleanup fold v318

`exec_load.s` now shares the post-lookup RT-reservation release sequence
for ordinary loader failure (`-1`) and VM allocation failure (`-2`).
The early argument/lookup failure still bypasses RT cleanup and returns
`-1`. This avoids changing reservation ownership or error semantics.
The PDP-6 boot Makefile's KERNEL_INPUTS was also missing the included
`exec_load.s` source: add it to force recompilation when that source
changes. A first build without this fix was stale and is not evidence.

After correcting the dependency, GCC lowmem `permanent-size` PASSed:
KCORE `024764` octal (10740 words), KCORE+MRES `036520` (15696),
PERMANENT_LAST `036577` (15743). This is a **two-word post-fold
reduction** from the comparable v316 KCORE `024766` build. The
targeted native `test-daimos-exec-rt-owner` assembled and launched its
256K PDP-6 SIMH guest, but timed out during boot before assertions
(`simulator failed: 124`); `test-daimos-exec-create-oom` did not run
because Make stopped on that failure. Therefore **runtime acceptance is
pending**, despite successful assembly and size measurement. Rerun
native failure-path testing before treating the change as complete.

## 2026-10-25 EXEC/RUN linked-fold audit v317

Inspected the GCC lowmem linked map and `kcore.fold` after the v316
fresh build. Resident entries appear at `exec_replace_current=000130`,
`exec_replace_current_lowmem=000407`, `sixbit_record_words=000540`,
and `proc_run_block=020332` (octal). The fold-plan text has no entries
for either `proc_run.dobj` or `exec_lowmem_pdp6.dobj`: there is no
existing cross-block folding in these two units to harvest directly.

Checked both startup-record loops against the decoder contract.
Their pre-call address test protects the count-word read, and their
post-call address test protects payload extent. The count-zero path
is necessary for argc/envc=0. Preserve these checks, even though
the loops superficially resemble each other. No safe positive-size
candidate was verified, so this audit makes **no production change**.

Retain the v316 `024766` GCC lowmem size measurement as baseline.
Next investigate executable-loader and startup helpers for proved
redundancy, with exact before/after post-fold and native regression
tests; do not infer code savings from source-level similarity.

## 2026-10-25 EXEC/RUN measurement checkpoint v316

Fresh `make -C system/boot/pdp6 BOOT_PROFILE=lowmem COMPILER=gcc
permanent-size` PASSed on the current checkout without production edits.
Its report gives KCORE `024766` octal (10742 words), KCORE+MRES
`036522` (15698), PERMANENT_LAST `036601` (15745), and the single-root
limit `040000` with 638 free words. This is 101 words below the older
documented `025133` lowmem number, but profile/build inputs and tool
versions have not yet been demonstrated equivalent; **do not attribute
the difference to EXEC/RUN optimization**.

The scanned EXEC/RUN record loops both require their input-range checks:
`sixbit_record_words` reads the pointed-to count word, so removing the
pre-call `pointer < end` check permits an out-of-range read even though
the post-call span check remains. Likewise the post-call span bound is
necessary to reject a record claiming data beyond its mapped block.
Do not remove either check as a size shortcut. RUN's descriptors,
hierarchy and publication require distinct cleanup ordering from EXEC.

Next: compare linked instruction sequences and call graphs for proven
intra-path redundancies; use this exact build command as the baseline
for before/after size tests. No code change, runtime retest or new
size-saving claim follows from this measurement alone.

## 2026-10-25 resident EXEC/RUN sharing audit v315

Source-only audit of `exec_lowmem_pdp6.s`, `proc_run.s`, and
`exec_load.s`; no code or native acceptance change. Both entry points
already call `sixbit_record_words` for bounded counted-SIXBIT records
and `exec_load_process` for DXR loading. The remaining record scans
use different persistent registers and failure targets; a separate
shared loop would add call/return and register adaptation, so no
positive post-fold saving has been demonstrated. Do not introduce it
without a measured whole-image reduction.

EXEC replacement stages a VM, creates startup data, preserves the old
swap/RT/extension state on failure and only then commits. Its low-memory
retry deliberately destroys the old VM before loading again and has a
distinct fatal-after-commit result. RUN instead claims a child slot,
creates a new u-area, inherits hierarchy and descriptors, then publishes
the runnable child; all post-claim errors use `proc_slot_discard`.
Merging these transaction tails would risk ownership and rollback
semantics and is not recommended.

Next optimization unit: count individual validation/check sequences
against actual linked object and p10fold output; prioritize proven
redundant work within a single path over generic cross-path helpers.
Preserve the deferred strict-32K policy. The existing comparable lowmem
size reference remains `025133` octal; no fresh measurement was made.

## 2026-10-25 verified whole-core swap-out return snapshots v314

The PDP-6 SIMH debugger stops at the successor instruction when a
breakpoint is requested on an instruction. The v313 attempt stopped
at reclaim's `PUSHJ proc_swap_out` address 003266 and its 003267 return,
producing two identical 256K dumps because the call had already
returned at the first snapshot. Its zero-difference output is an
**invalid before/after transaction comparison**, not proof of no change.

v314 fixes the pre-call stop to 003265 (SIMH displays the following
`PUSHJ 17,2623`) while retaining 003267 for the post-return stop.
The DXR call, prior branch and return instructions are verified before
SIMH runs. Captures contained all 262144 PDP-6 physical words apiece,
identical address coverage, and **75 changed words**: 12 below KCORE,
9 in permanent KCORE code/data range, 2 MM descriptors, and 52 beyond
the descriptor table. `mm_extent_count` changed 031 -> 030 octal;
there were zero unmatched addresses. This is a genuine pre-call to
post-return memory difference; expected kernel stack, MM extent and
swap metadata updates are present. Not all 75 changes have been
semantically proven correct, so it is **not a global non-corruption
proof**. The 20-child native swap guest remains PASS under v311.
The testkit fixture is `daimos-mm-swap-snapshot-v314.sh`, using the
v312 splitter/comparator. No production modification or KCORE growth.
With swap correctness restored, return to EXEC/RUN permanent kernel
minimization; additional memory investigations are optional unless a
new regression appears.

## 2026-10-25 native full-core swap snapshots v312

The testkit captured both 256K-word (262144-word) PDP-6 physical
RAM images for a swap-out, from `proc_swap_out` entry to
`proc_storage_publish_v30` entry (the latter is **pre-publication**, not
post-commit). The two complete, equally addressed dumps differ in
82 words: 20 below KCORE, nine in permanent code/data below the MM
extent table, two MM descriptor words, and 51 words above it.
`mm_extent_count` moved from `031` to `030` octal, consistent with an
extent release, and the swap record/accounting changed as expected.
This is a reproducible memory comparison tool, **not a global
non-corruption proof**, an after-return snapshot, or another guest
acceptance run. In particular, some AC/low-memory words naturally
change and must not be called corruption. The fixed 20-child v311
acceptance run still passes, so no speculative kernel fix is needed.
The testkit scripts `daimos-mm-swap-snapshot-v312.sh`,
`daimos-mm-swap-snapshot-split-v312.py`, and
`daimos-mm-swap-snapshot-compare-v312.py` capture, extract, and
classify the images without consuming guest RAM. The comparator
requires identical full 256K address coverage. More granular
semantic invariants and optional post-return capture can be pursued
separately if another swap regression warrants them.

### Post-INIT swap pressure acceptance v311

The native 256K PDP-6 GCC 20-pressure-child single-cycle fixture
passed with 012000 octal (5120) drum backing blocks, guest completion
marker `Q` and normal HALT. The 16-child fixture already passed with
010000 octal (4096) backing blocks. This resolves the immediate swap
correctness investigation following the real failed-swap-in local-stack
pop repair `3b41e87`; it does not validate smaller backing or all
possible 64-slot pressure workloads. No KCORE or descriptor-size
increase. Return to resident EXEC/RUN minimization and low-RAM
correctness analysis; 32K strict acceptance remains deferred.

## 2026-10-25 v311 twenty-child native swap acceptance — RUNNING / NOT ACCEPTED

The production kernel remains unchanged after the successful v310
16-child 256K PDP-6 GCC full-profile one-cycle acceptance using
`DRUM_BACKSTORE_BLOCKS=010000` (4096 decimal blocks) and a 900-second
SIMH-only ceiling. v311 extends the existing instrumented guest from
16 to 20 pressure children, retaining writer pipe integrity, swapped
writer restore, child wait/reap, and guest completion checks. The
original requested `020000`-octal backstore could not fit the single
`020000`-sector drum with logstore and cachestore and failed at
`mkdsk` before SIMH. v311 was corrected to `012000` octal backing
(5120 decimal blocks); the drum image built and `d6fsck` passed.

The no-breakpoint simulator run has so far emitted
`<wP0P1P2P3P4P5P6P7P8P9PAPBPCPDPEPFPGPHPIPJSR`:
all 20 children were admitted (`P0`..`PJ`), reached sleep (`S`),
and the parent reached its first writer-pipe read (`R`). At this
checkpoint the simulator process is **still running**; no `Q`, guest
HALT, or shell PASS result has been observed. This is *not* 20-child
acceptance, nor proof of a deadlock. The fixture has a 900-second
SIMH ceiling and retains its work directory for post-run inspection.
The two testkit commits are `f775ae4` (fixture) and `96d8711`
(drum geometry correction). Do not alter production swap policy,
KCORE size or descriptor capacity on this incomplete evidence.

Previous v307 snapshot analysis found expected live stack changes,
not a demonstrated memory overlap. v294 measured 28 of 64 idle-stack
words at the nested swap-out entry; v293 doubling the idle stack did
not remove the old UUO. Later `3b41e87` fixed a genuine swap-in
failure-path stack pop error, and v310 passed with sufficient backing.
Further tests belong only in daimos-testkit.

## 2026-10-25 extended 16-child swap acceptance v310 — PASS

The earlier 360-second high-capacity 16-child tests (v304/v305/v308/
v309) ended in host timeout, even though v309 reached marker `R`
just before the parent's first writer-pipe read. The unchanged v309
instrumented guest was rerun with `DRUM_BACKSTORE_BLOCKS=010000`
(4096 octal blocks), 256K PDP-6 full GCC image, no SIMH debugger
breakpoints and a **900-second simulator timeout**. Versioned testkit
fixture `daimos-mm-swap-progress-v310.sh` passed its one-cycle guest
completion checks (`<`, `w`, `Q`, guest HALT): writer pipe payload
integrity, sixteen pressure children, wait/reap and cleanup succeeded.
The full command including image build took roughly 569 seconds;
that duration is **not** the simulator-only elapsed time.

The v309 `proc_wait_event` snapshot was thus insufficient to conclude
a deadlock: under this configuration the program completed with a
longer execution budget. Original 2048-block backing remains a
separate capacity-exhaustion case and is not accepted at 16 children.
The 12-child current-kernel passing baseline remains valid.
No production code change, linked KCORE growth or descriptor-capacity
increase. Do not add debugger-heavy traces to this acceptance fixture.

## 2026-10-25 expanded swap writer-pipe blocking boundary v309

An additional one-byte progress marker `R` was emitted by the native
16-child 4096-block backing-store guest only after `dsys_close(gate_r)`
and `target_is_swapped()` both succeeded, immediately before the
first writer-pipe `dsys_read_words` call. The SIMH run printed
`<wP0P1P2P3P4P5P6P7P8P9PAPBPCPDPEPFSR` then timed out at kernel
`proc_wait_event` PC `017440` after the configured 360 seconds.
All 16 pressure children were admitted and asleep. The parent reached
the first pipe read but never returned from it. This strongly
localizes the high-capacity failure to the writer process resuming,
pipe output availability/wakeup, or related scheduling, not to
pressure-child admission or the prior swap-state guard. The marker
does not itself prove whether the writer ran or attempted to write.
Next inspect the writer's PROC_SLEEP reason, SWAP residency and
run-queue state at the blocked pipe read without high-volume SIMH
breakpoints; compare with 12-child passing trace. No production code
change or KCORE growth.

## 2026-10-25 expanded backstore 16-child progress localization v308

The unmodified 256K PDP-6 kernel, with `DRUM_BACKSTORE_BLOCKS=010000`
(octal, 4096 blocks), ran a lightly marked v308 guest (no SIMH
breakpoints). Its guest output was `<wP0P1P2P3P4P5P6P7P8P9PAPBPCPDPEPFS`.
Each `P` index proves a pressure child was admitted; `S` is printed
only after all sixteen reached PROC_SLEEP. The simulator nonetheless
timed out after 360 seconds at `proc_wait_event` PC `017440` without
`Q`, `a` or `s` error markers. Thus the long-run stall is **after**
all sixteen children sleep, in the `dsys_close(gate_r)` /
`target_is_swapped` check or first writer-pipe read before its
return. It is NOT an admission bottleneck. A v309 check should place
low-cost markers around this boundary, avoiding MM breakpoints.
No production code change or KCORE growth.

### Physical fault snapshots v307

The retained v297 snapshot compared 317 memory words, changing 27 of
64 sampled idle-stack words, 1 of 83 sampled process/MM metadata words,
and 0 of 4 sampled storage-map words. Both idle-stack base pointers
remain `040350` octal. Execution continued between captures, so these
changes cannot by themselves establish illegal writes or a stack
overflow. The v307 testkit report classifies physical changes without
assuming that a changed stack word is a corrupted word. No KCORE
growth or production change.

## 2026-10-25 current-kernel twelve-child regression v306 — PASS

After the production swap-in allocation-failure local-pop correction
(commit `3b41e87`), the original 256K PDP-6 GCC v281 twelve-child
post-INIT guest was rebuilt and rerun unchanged. Native SIMH reported
`PASS (one post-INIT 12-child pressure cycle)`, with allocator
insertion peak 31/136 across 71 samples. This reconfirms writer
restoration, pipe data integrity and child completion at 12 children
under the current code. It does **not** prove the 16-child case,
which still fails due to swap-space exhaustion and an observed writer
TERM event, or resolve the 4096-block v305 timeout. No production
source or KCORE-size change in this regression.

## 2026-10-25 expanded swap capacity experiment v305 — FAIL/timeout

A native 256K full-profile PDP-6 GCC 16-child acceptance attempt
with `DRUM_BACKSTORE_BLOCKS=010000` (octal; 4096 blocks), doubled
from the default `04000` (2048), used the v302 instrumented guest
without SIMH debugger breakpoints. The guest emitted `<w` but never
reached `Q`; the simulator stopped at its 360-second timeout
(`SIGTERM`, PC `017441` in the wait path). The run did **not** show
the previous unexpected UUO, but did not satisfy writer completion
or child reaping. The larger backing area therefore does not yet
establish 16-child acceptance or settle whether the failure is
capacity-only. A concurrent separate v305 run may also exist;
this result belongs to the run preserved as
`$TMPDIR/daimos-mm-swap-capacity-v305.log`.
Next distinguish slow forward progress from blocked writer / scheduler
wait with a lower-overhead trace, and retain correct behavior under
insufficient backing capacity. No production change or KCORE growth.

## 2026-10-25 writer termination event confirmation v303

A diagnostic SIMH run of the 16-child v302 guest placed one breakpoint
at `proc_event_apply` (linked address `015370`). At the breakpoint,
AC1=2, AC2=1 (`SYS_EVENT_TERM`), and `proc_sched_cursor=2`.
The guest emitted `<w!2101c!W` and halted, matching the earlier
v302 observation of writer PID low digit 2, EXITED kind 1, exit status
001 octal instead of 050. This establishes a kernel TERM event for
slot 2 in the failing run, rather than a normal writer exit.
The trace does not yet establish the event call's return address nor
that every exhausted swap-in should be terminated. Previously
captured swap backing space was insufficient for the tenth allocation;
the correct treatment of a temporarily nonresident process requires
investigation before changing scheduler semantics.
The event-only breakpoint took substantially longer than a plain
guest run. No production source or permanent memory change.

## 2026-10-25 writer WAIT exit status capture v302

The versioned 16-child guest v302 instrumented `wait_report()` with
`!` plus low PID octal digit, status kind, and 2-digit octal status.
The unmodified native PDP-6 GCC 256K kernel ran to guest HALT,
producing `<w!2101c!W`, **not** the former unexpected UUO.
The writer wait returned a PID with low three bits 2, EXITED kind 1,
and **exit status 001 octal** rather than the expected writer exit
status 050 octal. The writer source explicitly exits with 041, 042
or 050 only; this implicates kernel-side process termination.
`proc_swap_service_failed` on an unsuccessful restore invokes
`proc_event_apply(...,SYS_EVENT_TERM)`, a fatal event; termination
status 001 is consistent with this path, although no direct event
breakpoint has yet established the causal call for this writer.

Earlier v297 established 2048 swap blocks total, 1872 used, and a
208-block request with only 176 remaining. The 16-child case thus
probes real backing-store exhaustion, not simply MM descriptor or
storage map overlap. Further work must distinguish required graceful
capacity failure from incorrect scheduler termination and evaluate
whether the acceptance test should use an adequately sized swap tail.
Do not claim a 16-child PASS or change swap policy without an observed
failure path. Production code unchanged in v302.

## 2026-10-25 lightweight writer outcome isolation v300/v301

The 16-child 256K PDP-6 fixture used a versioned guest (v300) to
replace the compound `d` writer-failure marker with distinct markers:
`a` first pipe read, `b` first word mismatch, `c` writer WAIT exit
status mismatch, `d` pipe drain, `e` pipe close. A verbose v300 SIMH
breakpoint run timed out before any of these guest markers, so its
result was inconclusive. Version v301 removed the high-volume MM
breakpoints and ran the same guest with a 360-second SIMH timeout.
It completed with `<wc!W` and HALT, not the old unexpected UUO.
The first pipe read returned one word with the expected `A` value;
`wait_report(writer, SYS_WAIT_EXITED, WRITER_STATUS)` failed. The
actual wait PID, status kind and value remain unmeasured. It is not
established whether the writer was terminated on swap-in failure,
returned the wrong status, or produced some other wait result.
This run is still a FAIL, not 16-child swap acceptance. Next obtain
exact wait result and writer state, then repair the verified error
path, preserving correct backing-store exhaustion behavior.
No production kernel modifications in v300/v301.

## 2026-10-25 swap-in failed allocation return correction v299 test — partial

After commit `3b41e87` corrected the missing one-word stack-local
pop on the `proc_swap_in_alloc_fail` path, the existing native 256K
PDP-6 GCC 16-pressure-child v298 diagnostic was rerun. The original
`UUO 000000777777 340000000003` was **not** seen. Instead the
fixture emitted `d!W`: the writer output/read or wait verification
failed after all pressure children had been created. SIMH then ran
until its 300-second timeout (`SIGTERM`, PC `017441`, within
`proc_wait_event` boot wait), so the test is still **FAIL**, not an
acceptance run. `d` comes from the fixture's pipe/read/wait/drain
validation, not the swap allocator directly. The existing trace
shows recurrent `proc_swap_in` failure-return handling and the old
control-flow UUO is no longer reproduced under this workload.

The validated defect is the missing local pop. Do not interpret the
absence of a UUO in one run as complete repair: next identify which
writer outcome caused `d`, distinguish legitimate swap-capacity
failure from bad process termination semantics, and rerun the 12-child
PASS fixture to detect regressions. The old 16-child case cannot be
accepted until pipe integrity and complete child reaping succeed.
No additional production changes in v299.

## 2026-10-25 nested swap and backing-store exhaustion v294-v297

v294 captured the actual idle stack base `040350` and AC17
`000034040404` octal at the failing nested swap-out, i.e. 28
pushed words of the configured 0100-octal (64-word) idle stack.
At v295, execution reached the nested `proc_swap_out` call to
`proc_swap_skip_words` and the subsequent `mm_pin` check, with AC17
`000043040413` (35 pushed words). v296 traced the later
`backstore_alloc` return of -1 with AC17 `000045040415`
(37 pushed words). These snapshots do **not** demonstrate an idle
stack overflow at the sampled points; future deeper use is not
excluded. The separate 128-word experiment in v293 still failed.

v297 directly confirms backing-store exhaustion: total
`backstore_blocks=04000` octal (2048), in-use
`backstore_blocks_used=03520` octal (1872),
`backstore_enabled=1`, and new allocation AC1=`0320` octal
(208). Only `0260` octal (176) blocks remain. Nine prior 208-block
swap writes used 1872 blocks; the tenth allocation correctly returns
-1. This is a capacity refusal, not a detected overlapping MM
extent or a proven stack overflow. The subsequent unexpected UUO
`000000777777 340000000003` remains a **kernel error-path defect**
to diagnose. In particular, trace the propagation of the allocation
error from `proc_swap_out` through `proc_swap_reclaim`,
`mm_alloc_aligned` and `proc_swap_in` into `proc_swap_service_one`.
Do not increase KCORE, the idle stack or backing-store capacity as
a substitute for fixing error handling. The 12-child acceptance
fixture remains valid, the 16-child fixture still fails.

## 2026-10-25 v296/v297 physical pre/post swap-out snapshots

Native 256K PDP-6 full-image, sixteen-pressure-child SIMH breakpoints
captured 9 successful swap-outs as matching pre-entry and pre-storage-
publication snapshots (v296). A tenth swap-out entered but never
reached publication before `UUO 000000777777 340000000003`.
The v297 repeat added checkpoints at `mm_pin`, `backstore_alloc` and
a final dump after the UUO halt. On the failing tenth swap-out,
`mach_kernel_stack_base=040350` octal. AC17 at `proc_swap_out` entry
was `000034040404`, then `000044040414` at `mm_pin`, but at
`backstore_alloc` it was `000001040351`, nearly reset to the idle
stack base. The after-HALT dump again had AC17=`000001040351`.
Comparing the tenth swap-out entry with the post-HALT dump across
captured memory found 28 changed words in the inspected idle-stack
region, one process-table word and one MM extent-descriptor word.
The first packed storage-state word at `041141` remained
`000005252520` octal. These differences span continued execution
and an asynchronous fault, so they do not on their own prove illegal
memory writes. The abrupt stack-pointer change is strong evidence to
inspect PI6/PI7 handler and scheduler stack reset/return handling;
there is not yet proof of the specific mechanism.

Diagnostic files: daimos-testkit `daimos-mm-swap-snapshot-v295.sh`
(reuses boot, but a previously run disk image stopped at `?RT`),
`daimos-mm-swap-snapshot-v296.sh` and
`daimos-mm-swap-fault-snapshot-v297.sh` (fresh image), plus
`daimos-mm-swap-snapshot-diff-v297.py`. No production code change.

## 2026-10-25 nested allocator stack trace v293 — unresolved

The unchanged 16-pressure-child PDP-6 guest was traced through
`mm_alloc_aligned`, `mm_compact`, `proc_swap_reclaim`,
`proc_swap_victim`, `proc_swap_out`, and `proc_swap_in` with AC17.
At the first swap-in for slot 2, `mm_alloc_aligned` entered reclaim,
selected resident slot 1, and entered a nested `proc_swap_out` before
`UUO 000000777777 340000000003`. The trace captured AC17 as
`000034040404` octal at that swap-out entry. The configured idle stack
was 0100 octal (64 words), making stack overrun a hypothesis but not
proof: the full frame depth and actual allocated stack boundary were
not verified from that trace.

A controlled source experiment doubled the configured KINIT and
scheduler idle-stack size from 0100 to 0200 octal words. The same
16-child test still failed with the same UUO and the same captured
AC17. The two source changes were restored. This does not demonstrate
that expanding the idle stack fixed anything, and the unchanged SP
merits independent verification of stack selection/boot initialization.
Next check actual `mach_kernel_stack_base`, its owner-2 MM extent and
saved scheduler stack pointer under SIMH, and test stack bounds rather
than increasing permanent RAM or guessing at a larger stack.
No production code was retained.

### Swap metadata extent overlap inspection v292

At the 16-child failing guest's UUO aftermath, the two-bit storage
map at `041141` octal is within kernel-dynamic owner-4 extent
`[040750,041145)`. Captured neighboring MM extents do not overlap.
Slot 1 is RESIDENT, slot 2 DISK in its first map word
`000005252520` octal. Earlier slot-2 mode and block count validation
succeeded. Post-UUO checks that failed are not evidence of the
initial cause. Full descriptor-table overlap validation and nested
allocator/stack instrumentation remain open. No KCORE size change.

## 2026-10-25 first swap-read return probe v287 — inconclusive

Three iterations of the existing 16-child reproducer attempted
breakpoints around the linked `proc_swap_in` read return and rollback.
The base object-file disassembly exposes instructions after
`PUSHJ backstore_read`, but the inferred relocated breakpoints did
not trigger; early iterations also contained incorrect octal
address arithmetic. The native trace still reproduced the UUO.
A separate direct-image probe attempted to examine the linked code
but halted early at `?RT` and supplied no usable instruction evidence.
Therefore the read return code and the exact rollback cause remain
**unmeasured**. Do not conclude a read failure or a specific code
corruption mechanism from this probe. Next obtain authoritative final
linked disassembly or a simulator return-stack/trace snapshot and
verify each breakpoint address before launching a costly reproducer.
No production code change or KCORE growth.

## 2026-10-25 first swap-in rollback trace v286

The unmodified 16-pressure-child PDP-6 GCC reproducer again emitted
`UUO 000000777777 340000000003` and halted at PC 000001. A new
SIMH-only breakpoint fixture traced storage-state checks and
publication, backstore reads/writes, unpin, and MM-free operations.
The run recorded **one** `proc_swap_in` entry, **one**
`backstore_read` entry, **no** storage-publication entry associated
with that swap-in, and an `mm_free` entry before the subsequent
`proc_swap_out`. This is consistent with the first swap-in falling
into its rollback path; the exact failing return value was not
sampled. Over the entire run there were 10 swap-out entries,
9 backstore-write entries, 9 publication entries, 19 MM-free
entries and 10 unpin entries. Following the first swap-in rollback,
`proc_swap_out` was entered again, then the unexpected UUO appeared.
The 16-child workload remains a **FAIL**. No kernel source was
modified, no instruction-corruption root cause is claimed and
KCORE size/descriptor policy remain unchanged.
Next: sample the return from `backstore_read`, distinguish read
failure from unpin failure, and check saved process state after
failed restore.

## 2026-10-25 native swap-transition event trace v285

The unchanged 16-pressure-child test reproduced the unexpected UUO.
A new SIMH breakpoint fixture sampled `proc_current_slot`,
`proc_sched_cursor`, and AC1 on each `proc_swap_out`, `proc_swap_in`,
`proc_swap_service_one`, `backstore_read`, `backstore_write` entry,
without changing KCORE. The execution logged 10 `proc_swap_out`
entries, one `proc_swap_service_one`, one `proc_swap_in`, one
`backstore_read` and 9 `backstore_write` entries. The final sequence
was swap service request `proc_sched_cursor=0402` (octal),
`proc_swap_in` AC1=2 in slot-0 context, then `proc_swap_out`
AC1=1 in slot-0 context, followed by the same unexpected
`UUO 000000777777 340000000003` and PC 000001 HALT.
These are entry observations, not verified successful transitions.
The process-control corruption origin is still unresolved. Prioritize
return-path/instruction-state tracing of the first swap-in and the
following swap-out; do not increase process or descriptor limits.
No production code or permanent memory change.

## 2026-10-25 v284 unexpected UUO simulator breakpoint diagnostic

The unchanged v283 16-child guest was rerun under a new testkit
SIMH script with a memory-insert breakpoint and an additional PC=000001
breakpoint intended to examine `proc_current_slot`,
`proc_sched_cursor`, `mm_extent_count`, and selected ACs. The
same abnormal terminal output recurred:
`UUO 000000777777 340000000003` followed by
`HALT instruction, PC: 000001`. The new PC=1 breakpoint did **not**
fire, so no process/AC snapshot was captured. The fault remains
unresolved. In particular, PC 000001 in the terminal line must not
be interpreted as a successfully executed instruction at that PC.
No production change or swap high-water acceptance follows from v284.
Further debugging requires an emulator-side trap/event trace or an
earlier breakpoint on the actual UUO entry path, rather than relying
on post-fault PC=1 execution.

### Post-INIT stress escalation v282/v283: unexpected UUO unresolved

v282 (20 pressure children) fails creation at the 17th requested
child and exceeds simulator time. v283 (16 children) instead reaches
an unexpected UUO and halts at PC 000001 before normal guest markers.
SIMH descriptor insert observations for both runs were below 36/136;
neither run passed. A native trap/control-flow investigation now takes
precedence over increasing process counts or descriptor capacity.
No production changes or KCORE size growth.

### Post-INIT twelve-child swap/high-water acceptance v281

The 256K native PDP-6 test launches twelve 24K-word pressure children
after enabling swap and verifies a blocked writer resumes with intact
pipe data. One complete cycle PASS; 71 MM insert-entry samples give
peak descriptor occupancy 31/136. A three-cycle variant exceeded its
300-second simulator timeout during the second cycle and was not
counted as PASS. Full 63-slot concurrent swap remains unverified.
No KCORE source or size change.

## 2026-10-25 256K preload-pressure admission probe v280

Test-only PDP-6 GCC user image enlarged each guest with a private
`010000`-octal (4096-word) BSS array, checked at its first and last
word. The original 63-process preload and a reduced 48-process preload
both built valid boot images but halted without any guest start marker.
Consequently the test never reached runtime swap/reclaim; no high-water
value or allocator defect is established. This is a failed diagnostic,
not a passing or high-water acceptance fixture. The v265 63-process
small-image concurrency PASS and v278/v279 independently exercised
runtime swap remain the supported results.

The next high-concurrency swap fixture should create or enlarge user
memory after INIT has activated swap, not preload 48-63 large images
before the boot handoff. All simulator core settings remain in 16K-word
increments, the DAIMOS process policy in 8K increments.
No production changes or KCORE size increase.

### Three-cycle 256K swap descriptor high-water v279

Native PDP-6 SIMH `mm_extent_insert` breakpoints sampled 101
pre-insertion descriptor counts across three passing pressure
cycles. Maximum resulting descriptor occupancy was 27/136;
109 entries remained unused in that particular test. This is
distinct from the 63-process boot peak of 132/136; combined
63-process swapping has not been measured. No KCORE change.

### Repeated native swap/reclaim and process reuse v278

The 256K PDP-6 GCC full-profile test completed three successive
pressure rounds (blocked writer, eight pressure sleepers, swap
recovery, pipe data checks and child reaping). Testkit v278 PASS.
No permanent KCORE change. Descriptor high-water tracing during
these cycles and longer repeated evictions remain open; 32K GCC
lowmem disk-root is still the size measurement baseline.

### Correct AUXSTORE/root boot ordering and live swap recovery v277

`auxstore_post_minits()` now publishes dedicated backing-store mapping
after `kinit_boot()` mounts D6FS. Previously root mounting could
overwrite the shared direct-tail mapping, leaving backing allocation
capacity inconsistent with the active I/O span. With the corrected
order the 256K PDP-6 eight-child pressure fixture completes, including
blocked-writer restoration and pipe data checks. The standard 32K GCC
lowmem disk-root size run reports unchanged KCORE 10742 and
KCORE+MRES 13605 words; PERMANENT_LAST is 13652.

## 2026-10-25 live backing-tail measurements v276

Corrected test-only SIMH breakpoint examination at `backstore_write`
entry on the 256K PDP-6 pressure failure reads these **octal**
values: `backstore_blocks=04000` (2048),
`backstore_blocks_used=0320` (208), and
`blockset_direct_tail=02000` (1024). The allocator's advertised
backing extent exceeds the active direct-tail span. A first
experimental reading before boot incorrectly showed zero and was
replaced by readings within the execution breakpoint. The guest
still fails at child seven (`p7!W`) after swap-out rollback.
This is a verified configuration inconsistency; the test did not
capture the failing write's exact logical range or return code,
so the inconsistency is not yet proved to cause this particular
failure. Next trace configuration ownership and the write return
status; do not change production memory limits as a workaround.
32K GCC lowmem disk-root stays the KCORE size reference.

## 2026-10-25 backing-store rollback trace v275

On the fresh 256K PDP-6 eighth-pressure-child reproducer,
SIMH execution breakpoints recorded one call each to
`backstore_write`, `mm_unpin`, and `backstore_free`, with no
call to `proc_storage_publish_v30`. The guest still emitted
`<wp7!W`. This matches the `proc_swap_out_unpin` rollback
branch following a failed `backstore_write`, but the return
register after the write was not sampled, so the precise
failure remains to be proved. The absence of storage-state
publication confirms that this attempt did not complete the
normal swap-out commit sequence. The fixture remains a failing
diagnostic, not a swap acceptance test. Next inspect the native
backstore write return code and the blockset/direct-tail I/O
mapping under the tested 256K image. No production code or 32K
KCORE size-reference changes.

## 2026-10-25 256K pressure admission native traces v273/v274

SIMH breakpoints on the unchanged native PDP-6 guest captured one
`proc_swap_reclaim` entry, one `proc_swap_victim` entry, one
`proc_swap_out` entry, one `backstore_alloc` entry and one
`backstore_write` entry before the `p7` admission failure.
The deeper trace also observed 11 storage-state checks and
21 MM-free entries over the boot. Counts are entry observations,
not successful return results. In particular, whether the backing
write or subsequent transition fails remains unresolved.
The v272 eighth-child reproducer still fails; no new production
code or claimed swap acceptance. Continue at backing-store
return/rollback analysis, maintaining the 32K GCC KCORE baseline.

### Live 256K swap pressure: eighth child fails admission v272

The new test-only PDP-6 fixture holds a pipe writer blocked,
then starts sleeping 24K-word pressure children. Seven children
enter PROC_SLEEP; spawning the eighth reports failure (`<wp7!W`).
This reproduces pressure on the process creation path but does
not yet establish why admission failed or exercise successful
swap-out/resume. No KCORE modification or size change.

### 256K test BSS admission and SIMH granularity v271

SIMH physical-core selections use 16K-word increments; the
DAIMOS boot-slot and MM capacity formulas still use complete
8K-word banks. The 32K GCC lowmem disk-root profile remains
the KCORE size reference. The DXR loader rejects BSS values
above `077777` octal (32767 words). A 56K-word test array
therefore prevented INIT startup; a 24K-word test array
successfully reaches INIT on a fresh 256K image, then hits
the expected insufficient-swap-pressure assertion. This is
an admission/negative-control test, not swap acceptance.

## 2026-10-25 256K large-image swap-pressure probe v270

A disposable test-only derivative of the original 32K swap-stress
fixture enlarged each process's pressure array from 8K words to
56K words and built a fresh 256K full-profile GCC PDP-6 image.
The image linked and reached device initialization but halted
before the INIT guest's first `<` marker. The test thus yielded
**no swap-path result**. Simply enlarging each guest is not a
validated way to create pressure; startup / VM image sizing must
be examined before retrying. The failed experimental source was
not installed as an acceptance test or merged into production.
The correct next test should employ more moderate guest images,
additional sleeping pressure processes, verified physical process
residency, and explicit swapped-process resume checks. Existing
v267/v268 saturation and high-water results remain valid.
No production source or 32K KCORE-size reference changed.

## 2026-10-25 swap-pressure acceptance investigation v269

The older daimos-testkit daimos-swap-stress-v1 workload assumes
32K physical memory and forces a blocked pipe writer to swap
using two pressure children. Naively running that identical guest
at 256K does NOT reproduce the needed pressure: the guest emits
`<ws!W`, with `s` identifying failure of the explicit target-swapped
assertion in `run_writer_case`. This is not evidence of a broken
swap implementation: pressure requirements changed. Independently,
rerunning the original 32K profile fails at image construction:
`mkimage: KINIT image leaves less than the configured stack reserve`.
No 32K guest swap result can therefore be claimed from v269.

Next: construct a dedicated 256K test with enough concurrently held
process images to force real reclaim, explicit procinfo-backed
nonresidency checks, and resume/integrity verification. Preserve
32K GCC lowmem disk-root as the KCORE *size* reference even while
its runtime image does not fit. No production code changed in v269.

### Native descriptor saturation and reclaim v268 (2026-10-25)

The PDP-6 production mm_extent_insert/mm_delete test
fills the registry to 132 entries, adds four entries to
reach the 136-entry ceiling, verifies rejection of the
137th allocation without count mutation, and tests a middle
deletion followed by successful reinsertion at full capacity.
Testkit v268 PASS. This verifies the registry boundary and
slot reuse, not physical-memory pressure or process swap.
No KCORE source change; the 32K GCC lowmem disk-root size
baseline remains in force.

### 256K 63-process MM insertion high-water v267 (2026-10-25)

Fresh-image 256K GCC PDP-6 SIMH successfully completed all 63
process starts/finishes while an execution breakpoint sampled
`mm_extent_count` at each `mm_extent_insert` entry. There were
151 observations and a maximum **pre-insertion count of 131**
decimal. The PDP-6 production MM assembler increments this
count only once, at successful insertion. Therefore occupancy
cannot exceed 132 from these observed insertion entries. The
independent v266 point sample directly observed 132 allocated
descriptors, establishing the **132-descriptor peak** for this
specific first-boot workload (capacity 136, spare 4). The
breakpoint-action mechanism adds no persistent KCORE storage.
This does not prove adequacy for concurrent new allocations,
process replacement, repeated reuse or swapping/reclaim under
pressure. Do not change the eight-entry safety allowance before
testing those operations. No production code changed.

### MM native insertion trace v267 (2026-10-25)

A fresh 256K GCC PDP-6 first-boot workload exercising all
63 numbered process slots PASSed with a SIMH execution
breakpoint at `mm_extent_insert`. Breakpoint actions read
`mm_extent_count` at entry and resume. The test captured
151 pre-insert observations, peaking at 131 existing extents
(at most 132 immediately after that insertion); independent
v266 point sampling confirms 132. Capacity was 136 and no
descriptor-pressure failure was reported. This is a workload
trace, not a universal guarantee or validated swap/reclaim
peak; breakpoint actions do not establish whether each
insertion succeeded. No permanent kernel instrumentation
or new KCORE RAM. Size reference remains the 32K GCC lowmem
disk-root profile.

### 256K MM descriptor point observations v266 (2026-10-25)

The fresh-image 63-process 256K GCC PDP-6 SIMH test sampled MM
metadata at the last process's start and completion markers,
using test-only SIMH EXPECT and EXAMINE (no resident instrumentation).
The raw transcript shows `mm_extent_capacity=0210` octal
(136 decimal), `mm_extent_count=0204` octal (132) at
`[3F]`, and `mm_extent_count=0010` octal (8) at
`{3F}`. All 63 guest start/completion tokens were present.
There were only **four spare descriptors** at the last-start
sample; the eight-descriptor non-process/transient allowance
has not been demonstrated sufficient for further overlap.
These are instantaneous point observations, not a measured
peak, and do not prove descriptor safety during concurrent
allocation or swap/reclaim transitions. The simulator places
EXAMINE text on the same line as prior guest console markers;
the v266 fixture therefore extracts embedded address/value
pairs rather than anchoring them to the beginning of lines.
Production KCORE remains unchanged; use the established
32K-word GCC lowmem disk-root configuration for its size.

### Full 256K numbered process range v265 (2026-10-25)

The fresh-image 256K GCC PDP-6 concurrency fixture PASSed
both 48 and 63 guest process start/completion sets, including
guest private-state checks. Hexadecimal slot markers avoid
the earlier single-character marker collision. Slot zero is
reserved, so 63 is the largest numbered process count in the
64-entry table. This does not establish peak MM extent count
or sustained swap/descriptor reuse. KCORE code and the
required 32K-word measurement reference are unchanged.

### PDP-6 thirty-two-process 256K acceptance v264 (2026-10-24)

Fresh 256K full-profile GCC PDP-6 SIMH boot completed 32
process start and completion markers with private-state checks.
An initial false positive was an acceptance-script collision
between slot 24's valid 'X' completion and an error marker;
using '@' for errors resolved the fixture, and the clean
rerun PASSed. The test does not measure MM high-water,
simultaneous residency or swapping. Production KCORE was
unchanged; the 32K GCC lowmem disk-root reference remains.

### PDP-6 sixteen-process 256K acceptance v263 (2026-10-24)

First-boot 256K GCC full-profile native SIMH PASSed 16 process
start markers and 16 completion markers with per-process private
state checks. Test lives in daimos-testkit. The 64-slot policy
includes reserved slot zero, hence at most 63 numbered slots.
No MM peak occupancy was measured, and no KCORE change occurred.
KCORE size reference remains the 32K GCC lowmem disk-root build.

### 256K first-boot scheduler acceptance v262 (2026-10-24)

The separate daimos-testkit `daimos-mm-256k-scheduler-v262.sh`
builds a new GCC full-profile 256K PDP-6 image with five boot
users. Native SIMH PASSed preemption, process context, nice ABI
and disk sleep/wakeup with boot-selected 136-descriptor backing.
This checks real boot/scheduler functionality, but does not
establish 64-process stress or a peak extent occupancy value.
Production source remains unchanged. Continue using 32K-word
lowmem disk-root builds for KCORE size measurements.

### KINIT RAM-sized MM extent capacity v261 (2026-10-24)

Extended the v260 external backing to a boot-selected capacity:
`mm_extent_capacity = 2 * proc_slots_for_core(core_words) + 8`.
This yields 24 descriptors at 32K, 40 at 64K, 72 at 128K,
and 136 at 256K; capacity grows by four for each complete
8K bank. The fixed address of `mm_extents` and the direct
PDP-6 MM/VM memory operands remain unchanged. KINIT reserves
exactly two words per selected descriptor before publishing
its first free arena, and MRES starts afterward.
Native PDP-6 production-assembler tests at 24 and 136 extents
PASS, along with v249/v250 32-entry boundaries, the 32K
layout test and the **fresh-image five-process scheduler**
(preemption, context, nice ABI and disk sleep/wakeup). GCC
32K-profile KCORE is 024766 octal (10742 linked words), but
its 24-entry external registry occupies another 48 words:
actual fixed KCORE-plus-registry is 10790 words, 15 fewer
than the old 10805-word fixed-KCORE baseline. Likewise,
the linked KCORE+MRES report 13605 excludes these 48 words;
combined physical footprint is 13653 versus old 13668.
This is not a guarantee of 64 fully resident processes:
physical memory, pinned allocations and pressure still limit
simultaneous residency. High-load 256K boot acceptance is
still outstanding; only the maximum-capacity native allocator
boundary has been exercised at 136 extents.

### KINIT external MM backing restoration v260 (2026-10-24)

Restored the historical direct-address table boundary design as
an **intermediate 32-descriptor stage**: `mm_extents` aliases
`__kcore_low_end` without linked BSS storage. KINIT reserves and
zeroes the following 64 words before releasing any low arena;
MRES starts after that reserved metadata. The native PDP-6
insertion guard now reads `mm_extent_capacity`, currently reset
to 32 in `mm_boot_init` until dynamic higher capacities are
boot-validated. Native extent v249/v250 insertion/deletion tests
PASS; the 32K GCC disk-root image links and layout test v260
PASS. This change does **not** yet permit more than 32 extents.
32K-profile linked KCORE decreases from 10805 to 10742 words,
but the external 64-word reservation remains resident; with
one added capacity word the actual KCORE-plus-table footprint
is 10806 rather than 10805 words. Do not count moving the
table past the KCORE label as physical-RAM savings. Full
boot acceptance and memory-tier capacity selection remain open.

### KCORE measurement configuration v259 (2026-10-24)

For every KCORE size comparison, use a **32K-word** GCC
low-memory disk-root build, explicitly setting
`BOOT_PROFILE=lowmem ROOT=disk COMPILER=gcc
BOOT_CORE_WORDS=0100000 SIMH_CPU_KWORDS=32`.
The physical 32K boot may remain unsupported; linking and
measuring this profile is nonetheless the mandatory optimization
reference. Report KCORE, KCORE+MRES and PERMANENT_LAST separately.

### Process-slot 8K policy v258 (2026-10-24)

The boot process-slot count now grows from eight at 32K by two
for each complete extra 8K bank up to 64 slots at 256K.
The min-slot contract used by packed/storage metadata is eight.
Testkit checks 8K boundaries and incomplete banks; full GCC
lowmem disk link keeps KCORE+MRES at 032544 octal (13668).
This is process-slot sizing, not restoration of runtime MM
extent capacity; that remains a separate boot-layout task.

## 2026-10-23 MEMFS XIP alignment v203

Large dedicated MEMFS allocations now use `mm_alloc_aligned` with `02000`
alignment. Small packed chunks keep their existing path. Focused native
MEMFS pinning tests PASS on SIMH and AAP. EXEC/RUN remains unchanged.

## 2026-10-23 MEMFS XIP reservation foundation v202

MEMFS MRES has a four-word per-file XIP reference table and reuses the MM
extent pin count. The privileged reservation API currently admits only an
existing dedicated large MEMFS chunk with a `02000`-aligned physical base;
regular DXR files are not launched from this storage. Write, truncate,
reclaim and unmount paths now respect active reservations. No EXEC/VM
ownership changes have been made. The GCC lowmem image links, and focused
SIMH MEMFS and FIFO native target tests pass. AAP also passes the same
MEMFS pinning target fixture; actual AAP XIP process launch is pending.

# KCORE - Verified Open Work and Optimization Backlog

## 2026-10-23 native uncompressed EXEC physical OOM v205

daimos-testkit v205 PASSes uncompressed executable replacement on a
256K PDP-6 where old+new VM extents exceed physical memory. The
replacement executes as PID 1 by the v201-corrected low-memory EXEC
path. A false-negative DXR metadata assertion was fixed in the
testkit. No new KCORE text or static data; fresh link endpoint
`025657` octal. 32K acceptance remains deferred.

## 2026-10-23 resident EXEC -2 comparison correction v201

The EXEC staged-loader out-of-memory return must be tested with a
full 36-bit compare (`CAME 1,[-2]`), not `CAIE` with an 18-bit immediate.
The latter bypassed the already implemented low-memory replacement
retry. Native PDP-6 v201 pressured and unpressured compressed EXEC
replacement PASS after the fix; the v200 eight-case staged transaction
test also PASSes. No persistent RAM growth; the fresh full-profile
`__kcore_low_end=025657` octal. 32K acceptance remains deferred and
the full-profile permanent-memory limit remains open.

## 2026-10-23 EXEC late transaction boundary tests v200

daimos-testkit v200 executes the production `exec_replace_current`
assembler under PDP-6 instruction-level fault injection. Eight modes
PASS: load, startup and old-VM destruction failure; PURE backing-bit
commit variants; RT newly-claimed rollback, old-owner preservation and
non-RT successful release. Restored user mapping hold, swap backing
and prior credential mount bit are also checked. External call stubs
are injected, so these are not native MM allocation/destroy failures;
native full-image replacement/rollback tests are tracked separately.
No KCORE growth (`__kcore_low_end=025657`). Successful RT-to-RT EXEC
and direct EXEC ENOMEM under physical pressure remain to be verified.

## 2026-10-23 RUN loader VM creation resource exhaustion v199

Native 256K PDP-6 testkit v199 **PASS**: a separately installed valid
large child DXR RUNs before and after pressure, but four identical RUN
requests fail while the parent's live, unswappable VM consumes enough
physical core that `vm_space_create` cannot allocate another child.
Each rejection preserves caller break/data, live process slots and
process-word accounting. Shrink restores the old baseline, and v197
late descriptor-map rollback/reap/recovery tests also pass. Source
change is test-only: `__kcore_low_end=025657` octal, no added stack or
persistent RAM. EXEC replacement failure under deliberate pressure
remains unverified; 32K acceptance remains explicitly deferred.

## 2026-10-23 256K physical MM exhaustion regression v198

New daimos-testkit v198 native fixture PASSes four unallocatable but
well-formed `BRK(0776000)` growth requests with three live children.
After each failed VM/MM growth attempt, caller break, VM word count,
sentinel contents and process slot count remain intact. The combined
fixture subsequently passes v197's late RUN failure rollback and
healthy child recovery. Kernel unchanged: `__kcore_low_end=025657`
octal. Direct RUN/EXEC loader allocation failure injection and
fine-grained MM descriptor leakage tests remain unverified. The 32K
gate is deferred, not passed.

## 2026-10-23 RUN late descriptor-map rollback regression v197

Native 256K PDP-6 RUN V2 fixture PASSes sixteen late-stage failures
after child VM, startup and u-area setup, with three other children
live. Process slots and reported process words remain stable across
each failed RUN; after releasing/reaping the children, both return to
their baseline, and a subsequent valid RUN/WAIT succeeds. This is
not forced MM allocation exhaustion and does not individually audit
orphaned dynamic MM allocations. No kernel text or persistent data
changed; linked `__kcore_low_end` remains `025657` octal. The 32K
gate remains deferred by request.

## 2026-10-23 RUN native lifecycle/ID reuse pass v196

Existing 256K native PDP-6 process-lifecycle and process-ID-lifetime
testkit targets both PASS: RUN/WAIT/reap/orphan/descriptor cases and
creator-slot reuse protection for pgrp, session and domain IDs.
No resident code change; `__kcore_low_end=025657` octal. Continue
non-32K EXEC/RUN validation; the 32K gate is explicitly deferred.

## 2026-10-23 EXEC/RUN 32K acceptance deferred v196

Per current project direction, stop investigating 32K-only configuration,
KINIT packaging and oversized DSH EXEC errors for now. Keep the 8K/32K
targets documented as deferred, not passed; do not reduce memory reserves
or weaken size gates. Prioritize 256K native EXEC/RUN correctness,
rollback and process lifecycle tests, while preserving the resident RAM
footprint. The most recent 256K linked KCORE end remains `025657`
octal. The independent full-profile permanent-size overage remains open.

## 2026-10-23 native RT_REQUIRED RUN child follow-up v194

The existing compressed RT_REQUIRED child admission/rejection/release
native PDP-6 test passed after v193 post-VM EXEC rollback coverage.
This independent RUN path check introduced no KCORE modifications.
The linked `__kcore_low_end` remains `025657` octal. Functional-only
image-size settings are not strict permanent-memory acceptance.

## 2026-10-23 EXEC compressed decode failure RT cleanup v193

Native PDP-6 testkit v193 PASSes new RT owner cleanup and existing
owner preservation when an RT_REQUIRED compressed DXR2 has valid
header geometry but a truncated D6LZ control/literal stream. The
loader's normal path allocates the staged VM before invoking the
decoder; failure then destroys that staged VM and rolls back only a
new reservation. The earlier header-failure and successful EXEC
scenarios also PASS. No new kernel code or resident memory; current
post-p10fold KCORE end `025657` octal. Production timing, executable
image cap and 16K residency are still unaccepted.

## 2026-10-23 EXEC RT correction cross-regression v192

After correcting the live RT-claim marker/VFS stat overlap, the
versioned native fixture with 28 malformed EXEC/RUN blocks still PASSes
and the normal EXEC replacement completes successfully. Its full log
is `$TMPDIR/daimos-exec-v192-malformed-retest.log`. The v191 RT owner
transaction test independently PASSes. KCC/p10fold linked KCORE and
full permanent-memory measurements are unchanged; strict production
timing and memory acceptance are still open.

## 2026-10-23 EXEC loader RT claim scratch lifetime correction v191

The seven-word VFS stat output at `-011..-003(17)` was overwriting the
RT-claim marker previously stored at `-003(17)`. An existing RT owner
could therefore be released by a failed EXEC. Reuse `-012(17)` only
after its vnode output has been copied to AC12, initialize it before
the VFS stat call, and bypass marker-based cleanup on earlier failures.
The v191 native RT-owner transaction regression reproduces the prior
failure and PASSes after correction (new-claim rollback, old-owner
preservation, admission and normal-EXEC release). Both KCORE text/end
and transient stack usage are unchanged; `__kcore_low_end=025657`
octal after KCC/p10fold. Full memory still exceeds the strict limit.

## 2026-10-23 EXEC/RUN malformed launch regression v190

The PDP-6 native EXEC replacement fixture in daimos-testkit v190
now also PASSes 28 rejected malformed EXEC/RUN launch requests before
completing normal replacement. Invalid headers, count upper halves,
oversized counts, bad SIXBIT length fields and out-of-block records
were rejected without destroying the caller. Functional-only image
size and 35-second observation settings were retained. RT-owner
rollback, strict 15-second/`036000` gates and the full permanent
memory deficit remain open. This test-only checkpoint does not change
the `025657`-octal post-p10fold KCORE exclusive endpoint.

## 2026-10-23 EXEC/RUN native functional retest v189

Native PDP-6 replacement and child-session EXEC tests PASS on v188
(`6b9ce2a`), covering normal image replacement and child-process reap.
This follows the v188 linked `025657`-octal KCORE measurement.
The 35-second test observation and `037000` functional image cap do
not satisfy the strict timing/size gates. Malformed launch-block and
RT reservation failure cases remain open; this documentation-only
checkpoint does not alter the resident size.

## 2026-10-23 EXEC strict record start and low-memory RT cleanup v188

In `exec_replace_current` the argv/env record scanner tests that AC15
points strictly before the validated block end with `CAML/JRST`,
replacing the unused `MOVE/SUB/JUMPLE` difference. A nonzero record
count still requires a valid first word before calling the record
parser; record-end validation is unchanged. In low-memory EXEC the old
RT reservation is released only if AC14 equals `proc_rt_owner`, using
`CAMN/SETZM` directly instead of loading the owner in AC1 first.
The same-checkout KCC/p10fold exclusive KCORE endpoint changed from
`025661` to `025657` octal, two resident words saved. Native runtime
and malformed-block testing on v188 are still pending.

Full KCC `make permanent-size` accounting on the same profile reports:
KCORE 11135 words (`025577`), KCORE+MRES 18670 words (`044356`),
PERMANENT_LAST `044435`, MULTI_LAST `044476`. The strict `040000`
permanent-address limit still fails by 2333 addresses (single root)
or 2366 (multi root), as shown by the report's distance metric.
The KCORE exclusive-end address is not interchangeable with the
report's standalone KCORE size; keep both measurements distinct.

## 2026-10-23 EXEC/RUN native regression follow-up v187

Fresh-image PDP-6 simulator diagnostics for v186 PASS both EXEC current
image replacement and session-leader child EXEC/parent reap. The v185
EXEC replacement diagnostic also PASSed. Observation window was
explicitly extended to 35 seconds, with the functional-only `037000`
DSH image cap; original production timing and executable-size gates
remain open. Targeted RT-owner rollback and malformed launch-block
coverage also remain open. No new code or resident-word change here;
the current exclusive KCORE end remains `025661` octal.

## 2026-10-23 EXEC real-time owner cleanup v186

In the successful old-real-time reservation release and the failure
rollback, compare the saved process slot directly against
`proc_rt_owner` using `CAMN` and conditionally execute `SETZM`.
This replaces each `MOVE/CAME/JRST/SETZM` sequence with two words,
without clearing a reservation transferred to another owner. The
same-checkout post-p10fold KCORE exclusive end changed from `025665`
to `025661` octal (four resident words), without additional RAM or
stack. The v185 native EXEC replacement diagnostic PASSed using the
existing 35-second functional-only observation window; v186 native
runtime regression remains pending at this size measurement.

## 2026-10-23 EXEC direct validated scalar counts v185

`exec_replace_current` now loads the argc and envc launch words
directly into AC13 and AC14 for their existing upper-half-zero and
maximum-count checks. The formerly separate `HRRZ` copies are
redundant: both values are already validated pure 18-bit counts before
their use in the packed count pair or record total. Same-checkout
KCC/p10fold relink reduced exclusive `__kcore_low_end` from `025667`
to `025665` octal (two permanent words), with no additional RAM or
stack requirement. Native EXEC/RUN regression tests for v184-v185
remain pending at the time of this size measurement.

## 2026-10-23 EXEC old real-time ownership comparison v184

`exec_replace_current` now uses `CAMN 13,proc_rt_owner` followed by
`MOVEI 14,1` to set its existing old-RT flag only when the current
slot owns the real-time reservation. This avoids a temporary AC1 load
and a branch, preserving the original owner-equality behavior and
unchanged rollback logic. A same-checkout KCC/p10fold link moved
`__kcore_low_end` from `025671` to `025667` octal (two resident words).
No additional RAM or stack; v184 native EXEC regression pending.

## 2026-10-23 EXEC/RUN runtime verification v183

Native PDP-6 fresh-image EXEC replacement and child-session EXEC both
PASS after v181, using daimos-testkit's opt-in 35-second observation
window and functional-only `DSH_IMAGE_LIMIT=037000`. ABI inspection
PASSes using the live assembler rather than the retired C fallback.
Original 15-second timing acceptance and `036000` production executable
size remain outstanding, as do malformed-input and strict low-memory
regressions. No further code or resident-word change in this test pass.

## 2026-10-23 EXEC in-place path advancement v181

The first path record parse in `exec_replace_current` now advances AC15
directly to the first argv/environment record with `ADD 15,1` instead of
constructing the address in AC2 and copying it back. The existing
`CAMLE` end-of-block check still validates the computed address before
use. In a same-checkout KCC/p10fold KCORE relink, exclusive
`__kcore_low_end` moved from `025673` to `025671` octal, saving two
resident words. No extra stack or persistent RAM; syscall ABI unchanged.
Native EXEC/RUN launch and malformed-input regressions remain pending.

## 2026-10-23 EXEC argument-record end comparison v180

`exec_replace_current` now checks each argument/environment record end
directly (`CAMLE/JRST`) against the validated EXEC block endpoint rather
than subtracting the two addresses and testing the sign. This completes
the interrupted v180 edit without altering error handling or record
semantics. The same-checkout KCC/p10fold link reduced the exclusive
`__kcore_low_end` from `025674` to `025673` octal: one resident word.
Native EXEC/RUN malformed-input and process-launch tests are pending.

## 2026-10-22 EXEC path record endpoint check v179

`exec_replace_current` now checks the parsed path-record end directly
against the validated launch-block end (`CAMLE/JRST`) instead of using
`MOVE/SUB/JUMPL`. The `sixbit_record_words` parse and its nonzero check
remain in place. The same-checkout KCC/p10fold KCORE relink reduced the
exclusive `__kcore_low_end` from `025675` to `025674` octal, saving one
resident word without changing the syscall ABI, stack, or persistent state.
Native malformed-record and EXEC/RUN regression tests remain pending.

## 2026-10-22 EXEC launch size comparison v178

In `exec_replace_current`, replaced `MOVE/SUB/JUMPL` for the declared
launch-block length against available mapped words with the equivalent
`CAMLE/JRST` bound. The earlier minimum-length validations are preserved;
declared words greater than mapped words still fail before record parsing.
Same-checkout KCC/p10fold KCORE link reduced `__kcore_low_end` from
`025676` to `025675` octal, saving one resident word. A separate attempt
to change the RUN slot increment was reverted because it saved no
post-fold words. Native EXEC/RUN malformed-input testing remains pending.

## 2026-10-22 RUN record count combination v177

Following the existing validation of the RUN V2 argc and envc count
fields as pure 18-bit values, the record scanner now sums the two source
words directly (`MOVE` and `ADD`) instead of extracting both RH fields
into separate registers. This retains the exact nonnegative count bounds
and record-scanning semantics, and reduces the post-p10fold linked KCORE
exclusive end from `025677` to `025676` octal (one resident word).
KCC/p10fold link succeeded; native malformed-argument and startup tests
remain pending. No additional RAM or stack requirement.

## 2026-10-22 RUN initial path bound reduction v176

RUN V2 verifies `RUN_MIN_WORDS=010` before computing the path position
at `RUN_FIXED_WORDS=6`; consequently the path record starts strictly
before the validated block end. Removed the redundant `CAML/JRST`
pair in `proc_run_block`, retaining the `sixbit_record_words` validation
and the subsequent record-end bounds check. Same-checkout KCC/p10fold
link reduced exclusive `__kcore_low_end` from `025701` to `025677`
octal (two resident words). No additional stack or permanent storage.
Native malformed RUN block tests remain outstanding.

## 2026-10-22 RUN scalar count checks v175

RUN now checks the three scalar count fields (fd-map, argc, envc) with
`TLNE` on the full loaded value instead of using `HRRZ` followed by
`CAME`.  The left-half nonzero rejection and existing maximum-count
comparisons preserve the validation ABI. A KCC/p10fold relink in the
same checkout reduced `__kcore_low_end` from `025704` to `025701`
octal (three resident words). No extra permanent or stack state.
Native launch and malformed-block regression tests remain pending.

## 2026-10-22 RUN loader error cleanup v174

`proc_run_block` now routes all negative `exec_load_process` return values
directly to the existing claimed-slot discard path. The former comparison
against `EXEC_LOAD_NOMEM` and `MOVEI AC2,011` did not affect that cleanup
or its return value. A same-checkout KCC/p10fold relink moves the exclusive
KCORE endpoint from `025711` to `025704` octal: five resident words saved.
No permanent data or ABI changes; native process-launch regression remains
pending. A separate experimental merge of EXEC loader error rollback paths
linked successfully but yielded zero post-fold savings and was reverted.

## 2026-10-22 RUN validated pointer reuse v173

RUN previously called `sixbit_record_words` again after successful block
validation, to reconstruct the argv/environment record address for
`vm_space_startup`. `proc_run.s` now saves that address in one additional
temporary stack word before scanning the records and reuses it after
child image loading. This removes redundant parsing and its failure branch.
Same-checkout KCC/p10fold relink changed the `__kcore_low_end` exclusive
address from `025715` to `025711` octal (four resident words saved).
Peak RUN kernel stack increases by one word; permanent tables, EXEC/RUN
ABI and process descriptor size are unchanged. Native RUN/EXEC regression
and full image boot acceptance remain pending.

## 2026-10-22 EXEC validation pointer reuse v172

`exec_replace_current` in `proc/exec_lowmem_pdp6.s` now retains the
already-validated argv/environment record pointer in AC14 across its
record scan, rather than calling `sixbit_record_words` again on the path.
AC14 is available after argc/envc are merged and is repurposed for the
old RT-owner flag only after the scan. The next linked EXEC symbol moved
from `000430` to `000425` (octal), corresponding to five fewer words
in the leading EXEC section. The current linked `__kcore_low_end` is
`025715` octal; it is not compared against a freshly rebuilt matching
baseline, and no whole-image net saving or runtime acceptance is claimed.
Keep EXEC/RUN permanently resident; continue with RUN input-validation
and error-cleanup audit after native tests.

## 2026-10-22 additional unused C source sweep v171

The production MEMFS MRES builds `memfs_data_pdp6.s` and
`memfs_snapshot_pdp6.s` directly. Neither portable C source was
compiled by the active rule. Both superseded C files and their stale
source dependencies were removed; the assembly sources remain unchanged
except for a comment describing the replacement. Kernel `kcore` source
generation and boot integration dry run passed. This eliminates obsolete
source, with **no demonstrated resident KCORE or MRES word saving**.

Retain `d6fs_provider.c`: external daimos-testkit validation references
it even though the production D6FS provider uses handwritten assembly.
Also retain the live C portions of the mixed `proc`, `file`, module-runtime,
and swap implementations. No native machine execution is claimed.

## 2026-10-22 redundant C implementation sweep v170

The PDP-6 build now constructs EXEC exclusively from
`proc/exec_lowmem_pdp6.s` and `proc/exec_load.s`; the entirely disabled
`proc/exec.c` fallback was removed.  Likewise `proc/vm_pdp6.c` was removed:
all six functions were already compiled out by `VM_SPACE_*_PDP6_ASM`
defines, and the live routines remain in `proc/vm_pdp6.s`.  The PDP-6
`vm_backend.s` target is now an empty `.text` section; other backend
selections retain their C compilation rule.  These changes eliminate dead
source and compiler work, NOT linked KCORE instructions.  Do not count
the source-line reduction as resident RAM savings.

The same-basename kernel C/assembly inventory identified `vm_pdp6` as
the only exact pair; a build-rule examination additionally found EXEC's
differently named assembly replacement.  Other mixed units must remain:
`proc.c` and `file.c` still contribute active C functions after their
assembly-specific feature guards, and `module_runtime.c` contributes
descriptor/binding storage absent from its assembly implementation.
No runtime acceptance or new size measurement is claimed for this sweep.

## 2026-10-15 GCC 32K disk-root measurement and MM recovery v138

Isolated baseline at commit `37fba7e`, with `BOOT_PROFILE=lowmem`,
`COMPILER=gcc`, `ROOT=disk`, `BOOT=dsk`, `BOOT_CORE_WORDS=0100000`,
`SIMH_CPU_KWORDS=32`, and unchanged `02000` KINIT stack reserve:
complete image build PASS; KCORE exclusive endpoint `023503` octal
(10051 decimal); KINIT image end `072203` octal. The MM object is
520 text + 70 BSS words. The baseline is above the strict 8K target
by 1860 decimal words (exclusive endpoint must be below 020000).

Current committed MM assembler had lost optimizations from v094, v095,
v096, v106, v109, v110 and v111 compared with commit `9d52bb0`.
Restoring that exact v111 assembly in isolation builds a complete GCC
32K image: KCORE endpoint `023435` octal (10013 decimal), KINIT end
`072135` octal, MM object 482 text + 70 BSS words. Net whole-image
improvement is 38 decimal words after folding/layout;
the MM text contribution also falls by 38 words. The optimized image still
requires 1822 more words for a strictly below-8K exclusive endpoint.

This is a GCC link-and-package measurement, NOT native runtime
acceptance. `daimos-testkit` host `mm-core` cannot currently link due to
missing `proc_storage_publish_v30` and
`proc_swap_storage_invariant_v30`; fix the test fixture and rerun native
PDP-6 MM allocation, freeing, relocation and compaction regressions
before calling the recovered code runtime accepted. The older v136
external-descriptor branch additionally changes the insertion capacity
guard; it was not imported into the v138 image.

Date: 2026-10-24 (source reconciliation at v219, a405005)
Basis: checked DAIMOS source, PROJECT-STATE.MD, TODO.MD, and committed
native-test evidence. This is a documentation audit, not a new full link or
native simulator acceptance run. A staged, host-tested implementation is not
completed until linked, measured and accepted on the relevant PDP-6.

### Verified implementation baseline (v219)

- `proc/proc.h` defines three-word descriptors and 24/40/64/88/128/192/256
  boot-selected slot tiers; `proc/proc_boot.c` provides tier selection.
- `mm/mm.h` and `mm/mm_pdp6.s` still define 32 two-word MM extents.
  `ipc/pipe.h` still defines an eight-word FIFO buffer.
- `proc/exec_lowmem_pdp6.s` and `proc/proc_run.s` are the resident EXEC/RUN
  paths. Production `vm_space_startup` in `proc/vm_pdp6.s` writes user VM
  metadata even with zero arguments; it is not XIP-safe.
- The production VM destruction path in `proc/vm_pdp6.s` invokes the MEMFS
  provider's XIP release operation (026), with normal MM owner/type fallback.
  Private MEMFS operation 027 supplies restricted raw PURE XIP admission.
  Ordinary EXEC/RUN does not invoke that admission operation.
- The restricted linker XIP producer is experimental, with a limited
  instruction format. v215-v219 native fixtures validate selected admission,
  release, rollback and non-writing user-mode paths, **not** fully scheduled
  production EXEC/RUN, process exit, or swap interaction.
- The most recent documented **GCC lowmem** linked KCORE exclusive endpoint
  is `025134` octal (10844 decimal words, v213). Reaching a *strictly below*
  `020000` octal endpoint requires at least 2653 fewer words. The distinct
  EXEC/RUN **full-profile** checkpoint is `025657` octal (11183 words,
  v205); profiles must not be mixed. Neither is a newly measured v219 link.
- v190-v205 recorded native malformed EXEC/RUN, staged transaction, RUN
  failure rollback, and compressed/uncompressed low-memory EXEC regressions.
  This is positive targeted evidence, not 32K boot acceptance.
- The 32K strict gate is explicitly deferred by v196; do not report it as
  passing or relax it. The v138 GCC lowmem result (`023435` octal) and the
  older v41/v42 (`021541`) are historical, not current size baselines.

## P0: 32K resident limit and correctness

- [ ] When the deferred strict 32K work resumes, shrink the GCC lowmem KCORE
  exclusive endpoint below `020000` octal. The latest documented comparable
  lowmem link is `025134` (v213); first repeat a fresh v219 full link and
  full accounting, not just an object-level estimate.
- [ ] Measure full permanent KCORE plus committed MRES, irreducible scratch,
  KINIT allocations and process startup headroom; retain the separate general
  below-040000 permanent-address limit where applicable.
- [ ] [Deferred at v196] Pass the strict GCC 32K size gate and native 32K
  boot/INIT/LOGIN/DSH, process-pressure and filesystem tests with the same
  fresh image. Do not conflate 256K EXEC/RUN passes with this acceptance.
- [ ] Fix compiler, assembler and kernel correctness bugs at their source;
  add regression tests in daimos-testkit before marking the sweep complete.
- [ ] Audit source-level size hotspots, generated objects and real post-p10fold
  link maps; reject optimizations that increase the 32K resident footprint.

## P1: packed-process lifecycle and compression

The v33-v45 packed registry/layout and transaction sources are staged
experiments, not active production packed processes; retain ordinary swap.

- [ ] Choose a compact dense or bounded sparse packed registry using measured
  whole-KCORE code + dynamic data + MM extent cost, not isolated v33-v45
  prototype sizes; preserve MM-authoritative extent ownership.
- [ ] Connect KINIT registry allocation, boot abort, process exit, VM destroy,
  packed publication and teardown; validate exclusion and failure rollback.
- [ ] Complete packed restore against the selected registry, rather than its
  private descriptor, then integrate packed eviction/restoration with the
  scheduler; do not remove disk swap before full native acceptance.
- [ ] Integrate the production D6 codec with the resident KCORE encoder and
  streaming decoder, exact `000060` resumable ABI, bounded VFS refill,
  KPACK and EXEC/VFS/PURE consumers. Keep the detailed cross-subsystem plan
  in system/kernel/core/D6CODEC-INTEGRATION-TODO-v054.md.
- [ ] Measure combined resident codec text, stack, workspace and peak MM
  descriptors; test direct ABI, malformed input, resumptions, packed
  transaction failures and real PDP-6 execution before activating it.

## P2: kernel data structures and workload-specific sizing

These are profiling/selection tasks, not claims that the existing MM,
scheduler, pipe or slot-tier implementations are absent.

- [ ] Evaluate runtime-sized MM extent descriptors versus the current static
  `MM_MAX_EXTENTS=32` two-word table (mm/mm.h, mm/mm.c, mm/mm_pdp6.s);
  measure nested MAKE/KCC/KCPP load and relocation-sensitive assembly cost.
- [ ] Compare sorted linear MM scans with optional lookup indexes only for
  measured large-core workloads; avoid permanent small-core dispatch cost.
- [ ] Profile D6FS/DTFS/TSFS cache footprint, replacement and DECtape seek
  cost under native compilation; evaluate boot-selected budgets based on
  actual free memory after essential allocations.
- [ ] Profile VFS directory lookup under native MAKE/KCC and test compact
  entry caching or optional indexes, reusing D6FS hashing if advantageous.
- [ ] Benchmark dense versus sparse packed layout capacity and lookup cost
  under 24/40/64+ process slots, keeping the inactive experiments isolated
  until an accepted resident implementation is ready.
- [ ] Profile intrusive scheduler queue scans at larger process counts before
  adding priority indexing or extra branches to the common context-switch path.
- [ ] Benchmark the existing fixed eight-word, word-only pipe FIFO ring
  (ipc/pipe.h and pipe.s); change capacity only with demonstrated RAM or
  throughput wins. Do not revive obsolete character-width pipe formats.
- [ ] Confirm boot-selected process slot tiers (proc/proc.h and proc_boot.c)
  against real maximum-concurrency tests; do not reimplement already working
  boot-time selection or add runtime resizing without evidence.
- [ ] Audit remaining permanently resident device/accounting state and shared
  filesystem scratch for outright deletion or genuine buffer reuse; avoid
  moving always-required storage to MM merely to disguise its cost.

## P3: folding and compiler optimization

Priority decision (2026-10-24): further one-JRST folding is a **last-effort**
optimization after source and architectural shrinking opportunities are
exhausted. Preserve existing p10fold behavior, but do not spend current
engineering time expanding cross-domain JRST folding. Reconsider only after
a fresh final linked census and the existing >20-net-word/cold-path gate.

- [ ] Rerun p10fold read-only cross-domain census after final handwritten
  KCORE objects and fold plans stabilize; use explicit `-a` anchors, `-x`
  fold-plan exclusions and `-C` verified cold donors.
- [ ] Implement DLINK external-anchor folding only if nonoverlapping net
  savings exceed approximately 20 resident words with no KCORE growth,
  target-side metadata or hot-path JRST regressions.
- [ ] After self-hosting, use p10super results to improve KCC generator
  canonicalization (common tails, branches, AC allocation and equivalent
  forms), not by editing generated assembly.
- [ ] Defer native streaming low-memory p10fold and larger semantic/SMT
  searches until self-hosting and simpler optimizations justify their cost.

## P1 follow-up: restricted MEMFS XIP production integration

- [x] Provide aligned dedicated MEMFS XIP storage, pin lifetime, restricted
  PURE admission (private provider op 027) and VM release (op 026); native
  SIMH/AAP fixtures validate the individual mechanisms (v203-v219).
- [x] Provide an experimental raw PURE XIP linker mode limited to a verified
  non-writing instruction subset (v218); ordinary DXR/PURE is not XIP.
- [ ] Design a privileged argument-free, environment-free EXEC/RUN admission
  path, preserving existing ABI and no additional permanent process fields.
- [ ] Make XIP startup genuinely non-writing: zero argc/envc alone is not
  sufficient because `vm_space_startup` unconditionally stores VM metadata.
- [ ] Native-test *production* scheduler entry, failed EXEC replacement and
  rollback, process exit, swap exclusion, storage-map state, and memory
  pressure on both simulators before claiming XIP enabled.

## 2026-10-24 architectural shrink roadmap and proposal A start v221

### Proposal D first-boot MM checkpoint fixture v257 (2026-10-24)

Added `tests/validation/daimos-mm-firstboot-v257.sh` to
`daimos-testkit` (commit `32dfb7e`). The fixture builds its
own disposable five-process GCC-profile PDP-6 boot image,
checks the existing scheduler preemption/context/nice and
disk sleep/wakeup assertions, and examines the mapped
`mm_extent_count` during its **first** SIMH boot.
The first acceptance run PASSed the scheduler but deleted its
transcript before the observation was extracted. A second
independent fresh-build run PASSed and emitted one explicit
checkpoint sample of `000000000006` (six decimal) at
octal address 024537. The fixture now derives this address
from the fresh link map and requires an observation.
The test demonstrates successful non-production observation
without replaying modified media. **One point sample is not
the high-water value**: no MM registry reduction or permanent
kernel instrumentation is justified. KCORE stays 025065
octal (10805 words).

### Proposal D simulator replay/root diagnostic correction v256 (2026-10-24)

Revisited the v255 simulator failure. Manual artifact discovery had
used lexicographic sort of `daimos-scheduler-multiproc-v1-<pid>`
paths, selecting an October 20 image rather than the October 24
fixture. Corrected the selection to modification-time ordering and
replayed the newest saved October 24 image (MM count at 024537,
insert entry at 001145 octal), preserving its 8M SIMH throttle.
The subsequent replay also exited with `?RT` before the insertion
breakpoint. Kernel `KINIT_ERR_RT` documentation establishes that
`?RT` means root mount/validation failure or absence of a valid
`/SYSTEM/INIT`; it is **not a scheduler failure code**. The official
single-use scheduler build/run still PASSes, so a repeat execution
of saved mutable media is not a valid workload measurement.
Fresh-build breakpoint telemetry must be installed before the
first boot (with reproducible disposable media), not introduced
into a previously executed image. No MM high-water result or
production change is claimed.

### Proposal D fresh-image insertion tracing attempt v255 (2026-10-24)

Reran the unmodified official five-user scheduler regression with
`MAKEFLAGS=COMPILER=gcc` and retained its isolated build; the
native SIMH acceptance **PASSed**. Its freshly linked `kcore.map`
identified `mm_extent_count` at 023543 and
`mm_extent_insert` at 001167 octal. A separate manual SIMH
replay of that same build, with a one-shot breakpoint on
`mm_extent_insert`, instead emitted `?RT` and never reached
the breakpoint. Therefore the replay environment is not yet
equivalent to the official test invocation. No occupancy or
high-water data were accepted from that replay and there were
no kernel or testkit source changes. Resolve the launch-path
discrepancy before using debugger-based telemetry; keep all
32 extents.

### Proposal D official scheduler rerun v254 (2026-10-24)

After DevSpace2 became reachable again, reran the unmodified
`daimos-testkit` target `test-daimos-scheduler-multiproc`
with `MAKEFLAGS=COMPILER=gcc`. Its full isolated PDP-6 build
and SIMH regression **PASSed**, including preemption, process
context, nice ABI and disk sleep/wakeup. This supersedes the
earlier v253 implication of a persistent scheduler failure
from the ad hoc replay, without establishing a specific cause
for that `?RT` output. It does not turn v252's three extent
snapshots into a high-water measurement: the 32-entry MM
capacity remains unchanged. No kernel or testkit changes.

### Proposal D unstepped scheduler cross-check v253 (2026-10-24)

Replayed the preserved GCC five-process image without SIMH
EXPECT stepping, using the fixture's 8M instruction/second
throttle. SIMH booted the filesystem providers and halted
normally, but the guest emitted `?RT` instead of the expected
multichild scheduler progress markers. Therefore the earlier
v252 scheduler completion failure cannot be attributed solely
to intrusive diagnostic stepping. The only confirmed extent
data remain three point samples of 020 octal (16) from v252;
there is **no validated workload high-water result**. Do not
downsize the 32-entry MM table on these observations. No
production or testkit source changes were accepted.

### Proposal D GCC-profile MM occupancy checkpoints v252 (2026-10-24)

The earlier `036013 > 036000` DSH gate failure was caused by
building the five-process diagnostic with the default KCC
configuration. The previously documented lowmem `COMPILER=gcc`
configuration builds past the unchanged gate and reaches SIMH.
An isolated scheduler fixture inspection of mapped symbol
`mm_extent_count` at octal address 024537 yielded 020 octal
(16 decimal) at each of three scripted checkpoints. However the
diagnostic-stepped scheduler run then reported that timer
preemption did not complete all boot users; this is **not a
passing scheduler regression or an extent high-water measurement**.
No persistent instrumentation, production code or MM allocation
changes were made; the experimental fixture was removed.
Retain the 32-descriptor limit until representative full
workloads are measured with nonintrusive peak tracking.

### Proposal D real workload MM occupancy attempt v251 (2026-10-24)

Attempted a non-invasive PDP-6 runtime snapshot of `mm_extent_count`
at scheduler checkpoints by adapting the existing five-process SIMH
fixture in a separately versioned testkit-only script. The build
failed before simulator execution at the existing strict DSH user
image size gate: `036013` octal exceeds `036000`.
Consequently **no runtime extent occupancy samples or high-water
measurements exist from this attempt**. The DSH acceptance limit
was not relaxed, the experimental test script was discarded,
and unrelated testkit working-tree modifications were preserved.
The tested production MM registry remains 32 entries (64 words),
with no justified shrink or new permanent RAM saving.

### Proposal D native MM sorted insertion verification v250 (2026-10-24)

Added a separately versioned `daimos-mm-extents-v250` test to
daimos-testkit, extending v249's full 32-entry boundary and -3
rejection coverage with deletion followed by insertion into the
middle of the live descriptor array. The fixture checks both the
inserted two-word slot and the displaced old slot. It PASSes
using production PDP-6 `mm_extent_insert`/`mm_delete` leaves,
testkit commit `580e978`. This is deterministic correctness
coverage, not workload high-water telemetry; the 32-entry MM
limit stays unchanged and KCORE remains 025065 octal.

### Proposal D native extent registry boundary fixture v249 (2026-10-24)

Added a native PDP-6 fixture to daimos-testkit:
`tests/validation/daimos-mm-extents-v249.s` and
`daimos-mm-extents-v249.sh`. It extracts the production
`mm_delete` and `mm_extent_insert` leaves, fills all 040 octal
(32) slots, confirms that insertion 33 returns -3, deletes one
slot and successfully refills. It PASSes with p10run/PDP-6;
testkit commit `11dc124`. This tests the boundary, not real
workload high-water, fragmentation or VM compaction. No DAIMOS
production allocation or code change is justified. Preserve the
025065-octal (10805-word) KCORE baseline.

### Proposal D host MM fixture dependency investigation v248 (2026-10-24)

The `mm-core` failure described in v247 is not only an obsolete
Makefile path. A temporary testkit-only build experiment omitted
the missing `vm_pdp6.c` dependency. The host linker then
reported three unresolved VM functions: `vm_extent_move`,
`vm_space_can_swap`, and `vm_space_destroy`. These production
functions belong to the PDP-6 VM implementation, including
`vm_pdp6.s`, and host C tests cannot link the assembly directly.
The fixture asserts actual movement, pinning, live mapping and
swap-destruction behaviors; no dummy symbol implementations
would be valid acceptance evidence. The testkit Makefile was
restored byte-for-byte from a temporary backup, preserving
all unrelated pre-existing testkit edits. A proper test repair
requires an equivalent host implementation with verified semantics
or a PDP-6-native MM fixture. No production code or extent
registry limit changed; baseline remains 025065 octal KCORE.

### Proposal D extent registry acceptance gate v247 (2026-10-24)

The PDP-6 MM implementation has `MM_MAX_EXTENTS=040` (32)
two-word descriptors, occupying 0100 octal (64) resident words.
`mm_extent_insert` explicitly returns MM_ERR_DESCRIPTORS (-3)
when full; shrinking the registry would change real failure behavior.
No representative runtime high-water readings are available here,
so do not reduce this count merely to reclaim static words.

The daimos-testkit `mm-core` target currently fails to compile
before test execution: its Makefile references removed/missing
`system/kernel/proc/vm_pdp6.c`; the checkout contains
`vm_pdp6.s` and `vm_pdp6_swap.c` instead. The testkit checkout
also contains unrelated pre-existing working-tree changes; these
were preserved. Repairing the fixture requires identifying the
correct host-side VM implementation/link requirements, not simply
replacing the path with assembly. No source changes or new
permanent-memory savings were accepted.

### Proposal F static allocator and VM lifetime inventory v246 (2026-10-24)

Following the committed v245 filesystem scratch audit, inspected
resident BSS declarations in MM, MEMFS, IPC, PROC and VM. MM's
`mm_extents` is 0100 octal words (64 decimal) for its live
extent descriptors, not a disposable scratch buffer. MEMFS
`memfs_data_chunks` and `memfs_xip_refs` track allocator
and XIP ownership; the separate VM APR word and IPC/PROC
singletons hold live state. None can be aliased safely to the
128-word filesystem transfer workspace without first proving
non-overlapping use and ownership. No production change is
proposed; retain the v245 GCC lowmem KCORE baseline 025065
octal (10805 decimal). Measure peak extent occupancy before
any MM descriptor count reduction.

### Proposal F filesystem workspace ownership check v245 (2026-10-24)

Inspected resident filesystem static allocations and provider
serialization for possible buffer lifetime overlap. The only
128-word resident VFS transfer block is already `fs_block_workspace`.
DTFS, TSFS and D6FS share it, and `fs_provider_serialized` holds
the provider lock over potentially sleeping physical I/O. Arguments
are held on private process stacks during lock waits. MEMFS bypasses
that serialization because it does not sleep or use this block.
Reusing the transfer block for unprotected path/lookup scratch
would permit an unrelated process to corrupt an in-flight request.
The remaining assembly storage declarations inspected in this
area are small state variables or MEMFS-owned allocations rather
than obvious duplicate permanent transfer blocks. No safe
ownership-preserving static overlay was identified, and no source
change or new saving is claimed. Retain v244 KCORE baseline 025065
octal (10805 words); revisit only with a documented lifetime proof.

### Proposal B file-path request-frame inspection v244 (2026-10-24)

Audited the resident `file_symlink`, `file_rename`, `file_chdir`,
`file_make_node`, `file_remove`, `file_truncate` and
`file_stat_path` paths together with `vfs_create_common` and
`vfs_create_op`. MKDIR/MKFIFO already share their parent/access
frame, UNLINK/RMDIR share a typed removal frame, and the one-argument
TRUNCATE/STAT wrappers already share stack cleanup. The CREATE
provider request and returned-vnode publication are already common
for the relevant operations. A new cross-operation frame would have
to preserve distinct permissions, argument lifetimes and stack
offsets, with no identified positive linked-size candidate. No
production source changes were made.

`test-daimos-vfs-stat-frame` and `test-daimos-pre-dsh-fs-closure`
PASS with GCC. Retain v243 KCORE baseline 025065 octal (10805
decimal). Continue looking for a concrete full-image reduction,
without touching deferred JRST folding or 32K acceptance.

### Proposal B descriptor and GETCWD clear follow-up v243 (2026-10-24)

After v242, the descriptor duplication paths both use bounded BLT
and the mount-extension replacement operation is shared. Examined
the variable-length output zeroing in `file_getpath_common` and
its `file_getcwd_clear` loop. A prospective BLT-based fill needs
additional pointer, destination and end-address preparation and
would add overlap/lifetime complexity. The exploratory source edit
was reverted immediately without accepting or benchmarking it.
No production code change and no new RAM reduction was claimed.
Do not force an additional dispatcher or BLT on this short loop
without a positive complete-link result and path regression.

### Proposal B DUP2 descriptor pair copy v242 (2026-10-24)

`file_dup2` now uses the same bounded two-word BLT as `file_dup`
to copy the packed vnode/metadata and offset. Its subsequent pipe
reference operation obtains the packed metadata from the completed
destination descriptor, rather than relying on the AC3 scratch
value from the former scalar-copy sequence. This retains the
descriptor-copy, mount-extension, pipe reference and old==new
semantics without extra resident state or stack.
Full linked GCC lowmem disk-root KCORE 025066 -> 025065 octal
(10806 -> 10805 decimal), KCORE+MRES 13669 -> 13668,
PERMANENT_LAST 13716 -> 13715. The existing native PDP-6
`daimos-file-mountext-v19` fixture PASSes 26-mount vnode, lock,
DUP/DUP2, close and unmount; file and file-walk tests both PASS.
Strict 32K acceptance and JRST folding remain deferred.

### Proposal B contiguous FILE descriptor duplication v241 (2026-10-24)

`file_dup` copies the packed descriptor node and offset from two
adjacent words. One bounded two-word BLT replaces four MOVE/MOVEM
instructions with a three-instruction pointer setup and BLT, without
changing destination extension replacement, lock-family semantics,
or pipe reference handling. No resident data or stack increase.
GCC lowmem disk-root full linked KCORE 025067 -> 025066 octal
(10807 -> 10806 decimal), KCORE+MRES 13670 -> 13669,
PERMANENT_LAST 13717 -> 13716. The existing daimos-testkit
`daimos-file-mountext-v19` PDP-6 test PASSes its 26-mount vnode,
lock, DUP/DUP2, close and unmount assertions; target-file-pdp6 and
target-file-walk-pdp6 PASS. No JRST folding or 32K acceptance work.

### Proposal B file mount-extension bitmap update v240 (2026-10-24)

`file_mount_ext_store` and `file_mount_ext_copy` previously duplicated
the final two-bit descriptor mount-extension clear/set sequence. Both
paths now enter `file_mount_ext_update` with the destination bit offset
in AC7 and already-positioned replacement bits in AC6. Clearing
outdated destination bits precedes ORing the replacement, as before.
The canonical vnode, descriptor layout, AC1/AC2 preservation and
zero-to-clear behavior remain unchanged. This adds no resident
state or stack frame.

Full linked GCC lowmem disk-root KCORE: 025075 -> 025067 octal
(10813 -> 10807 decimal). KCORE+MRES 13676 -> 13670;
PERMANENT_LAST 13723 -> 13717. PDP-6 target-file-pdp6 and
target-file-walk-pdp6 PASS. Specific full native descriptor mount
extension lifecycle verification remains open; the size result
does not imply that broader acceptance.

### Proposal B provider boundary and leaf-loop audit v239 (2026-10-24)

The v238 follow-up reviewed `vfs_lookup_provider`,
`vfs_parent_return`, `vfs_create_op`, `vfs_symlink` and MonitorFS
`mfsleaf_lookup`/`mfsleaf_readdir`. CREATE/MKDIR already share the
request formatter and SYMLINK already shares vnode publication.
Lookup must additionally resolve mounted targets, unlike PARENT.
MonitorFS leaf lookup must retain the requested count across the
entire loop; AC5 is reused for packed-name comparison, so naively
hoisting the length load would be incorrect. That source experiment
was immediately reverted without a misleading size claim.
No ABI-safe larger common routine was identified, and no production
code or tests were changed. Preserve the 025075-octal KCORE baseline;
further investigation should target a concrete duplicated data
lifetime or full provider request frame, not an extra dispatcher.

### Proposal B VFS mutation epilogue experiments v238 (2026-10-24)

From the clean v237 checkout (KCORE 025075 octal, 10813 decimal),
two cold error-path sharing candidates were compiled and measured
against the complete GCC lowmem disk-root image. Redirecting
`vfs_setattr` errors through the existing three-word unwind
`vfs_mutate3_ro` was size-neutral at 025075, so it was reverted.
Redirecting `vfs_symlink` and `vfs_rename` four-word failure
unwinds through `vfs_write_words_ro` **increased** the full linked
KCORE to 025077 (10815 decimal), KCORE+MRES to 13678 and
PERMANENT_LAST to 13725. It added an extra cold-path branch and was
also reverted. The linker/folder already removes or shares more than
the source-level instruction count suggests. No production changes
were accepted and no native runtime acceptance is claimed for these
discarded experiments. Continue proposal B with larger structural
opportunities rather than tiny branch/epilogue rewrites.

### Proposal B MonitorFS namespace BLT copies v235 (2026-10-24)

Two fixed three-word name-record copies in resident VFS, one in
`vfs_readdir_table` and one in `monitorfs_name_record`, now use
bounded PDP-10 BLT transfers instead of three MOVE/MOVEM pairs.
The source/destination record layout and length are unchanged.
Full GCC lowmem disk-root image: KCORE 025103 -> 025075 octal
(10819 -> 10813 decimal); KCORE+MRES 13682 -> 13676 and
PERMANENT_LAST 13729 -> 13723. No new static data or stack.
Both target-file-pdp6 and target-file-walk-pdp6 PASS on PDP-6.
The preceding one-branch provider fallthrough experiment was
size-neutral after linking and reverted. Further native directory
enumeration and getcwd acceptance remains useful. The native
test-daimos-monitorfs-devices-online on SIMH also PASSes with
COMPILER=gcc and SDL_VIDEODRIVER=dummy.

### Proposal B bulk device argument consolidation v233 (2026-10-24)

The resident file-runtime bulk READ/WRITE dispatch previously reloaded
the buffer pointer and word count separately in ten individual device
leaves (six WRITE, four READ). Both dispatchers now save their canonical
vnode in AC0 and establish the device-call arguments once before selecting
the device. The ordinary VFS provider fallback restores the vnode and
original word offset before calling VFS, preserving descriptor advancement,
append handling, provider ABI and the mapped-buffer contract. No data,
extra stack slots, or provider dispatch layers are added.

Full GCC lowmem disk-root linked measurement: KCORE 025115 -> 025103
(10829 -> 10819 decimal), KCORE+MRES 13692 -> 13682 and PERMANENT_LAST
13739 -> 13729: ten permanent words saved. PDP-6 target-file-pdp6 and
target-file-walk-pdp6 PASS. Native PTR/PTP test compilation completed but
SIMH terminated with status 139 after SDL2 window-event warnings. This
is **not** a native device I/O acceptance pass; separate device validation
remains open. A separate native card WORDTOKEN12/CR/CP round-trip test
PASSed with SDL_VIDEODRIVER=dummy. No changes to JRST folding or the
deferred 32K gate.

### Proposal B CHDIR stat reuse v232 (2026-10-24)

`file_chdir` already obtains a seven-word `vfs_stat` frame to check that
the target is a directory. Its subsequent `file_check_access` call repeated
`vfs_stat` on the same vnode. Use `file_access_stat` directly on that
validated frame for the same search-permission mask (1). This removes a
redundant provider STAT invocation from successful CHDIR, with no ABI,
permission-policy or persistent-state change. GCC lowmem disk-root full
link PASS: KCORE `025115` (10829 decimal), KCORE+MRES=13692; size-neutral.
`daimos-testkit target-file-walk-pdp6` PASSes on PDP-6 including CHDIR.
This is a runtime-path simplification rather than a KCORE shrink. Further
proposal B work must find actual text/data reduction.

### Proposal B MonitorFS namespace/classifier audit v231 (2026-10-24)

Reviewed MonitorFS device lookup, directory enumeration and STAT. They
already share `mfsdev_name_length`, `mfsdev_validate_id`, leaf table
lookup/readdir and (since v230) `mfsdev_io_type`; lookup and enumeration
retain distinct return layouts and device-name matching requirements.
Tested making character devices the default in `mfsdev_io_type` and
checking mount/block exceptions first. The lowmem full link saved only
**one word** (`025114`, 10828 decimal), while adding instructions to the
ordinary character-device path. Rejected the tradeoff and reverted it;
restored full link PASSes at `025115` (10829 decimal), KCORE+MRES=13692.
No production source change was retained. Next proposal B work should
concentrate on larger file/VFS semantics or duplicated provider operations,
not device-classification instruction rearrangements. Dedicated native
STAT/READDIR type-matrix coverage is still desirable.

### Proposal B MonitorFS shared device classification v230 (2026-10-24)

`mfsdev_stat` independently duplicated the device-type classification
already implemented by `mfsdev_io_type` for READDIR. It now passes the
device ID in AC0, calls that existing classifier, and uses returned AC7
as the STAT type. No new data or provider dispatch is introduced. GCC
lowmem disk-root permanent-size link PASS: KCORE `025115` octal (10829
decimal), KCORE+MRES=13692, PERMANENT_LAST=13739 decimal, **10 words**
below the v229 accepted baseline. Native MonitorFS device tests remain
required before calling the change runtime accepted. The daimos-testkit
`test-daimos-monitorfs-devices-online` native SIMH test subsequently
**PASSed** using the supported `COMPILER=gcc` configuration and the unchanged
DSH executable size gate. This confirms the online device view; a separate
STAT/READDIR mode/type matrix remains useful to directly exercise all
device-classification categories.

### Proposal A process-table scan census and next boundary v229 (2026-10-24)

Current `proc_pdp6.s` has distinct hand-written scans with different
contracts: `proc_has_live_user` checks only an active state, whereas
`proc_tty_session_has` excludes a slot and conditionally checks pgrp,
u-area presence and session. `proc_trim_high` traverses backward only over
FREE trailing descriptors; `proc_runq_add/remove` operate the intrusive
runnable queue. Sharing these through a general scan helper would add
calling convention, predicates and/or persistent state to short fast paths;
there is no measured benefit. **Do not implement a generic iterator.**
The existing boot-selected process capacity and three-word descriptors
remain the baseline; no new process metadata is proposed.

After the failed common teardown prototype (v228), proposal A has no
currently justified large source-level rewrite. Treat A's first pass as
measured but not exhausted forever; revisit only with a concrete duplicate
instruction block or demonstrable lifetime/state simplification. Turn to
proposal B (file/VFS/MonitorFS) next rather than spend time on one-word
instruction substitutions. No source change or additional saving at v229.

Initial B measurement from the current available compiled lowmem objects
(section totals **before** final link/folding; not additive potential):
`file_runtime` text=1175, `monitorfs_runtime` text=1565 data=39,
`vfs` text=778 data=27 BSS=128 words. Prioritize the MonitorFS dispatch
surface and file permissions/lookup interactions, but preserve provider
semantics and measure the post-fold whole-image result before editing.

### Proposal A teardown sharing investigation v225 (2026-10-24)

### Proposal A discard assembler prototype v226 (2026-10-24)

### Proposal A supported GCC native rollback verification v227 (2026-10-24)

### Proposal A shared post-VM cleanup experiment v228 (2026-10-24)

Tested one common `proc_cleanup_after_vm` routine to serve both normal EXIT
and failed RUN slot discard after successful `vm_space_destroy`. It shared
TTY session release, u-area release and runnable-queue removal while keeping
distinct VM transitions and process publication. The GCC lowmem disk-root
link **grew** from v226/v227 KCORE `025127` (10839 decimal) to `025136`
(10846 decimal), an increase of seven resident words; KCORE+MRES likewise
grew from 13702 to 13709 words. The additional calls and register saves
outweighed eliminated local operations. **Rejected and reverted**: no
production code change retained and no native test claimed for this
discarded experiment. Do not attempt the same wrapper without a new
measurable reason to expect savings. Further A candidates must eliminate
actual work or data, not merely factor assembly source.

The production DSH size gate of `036000` octal must remain unchanged.
Investigation found the default KCC userland fixture generated DSH `036013`,
but DAIMOS already documents `COMPILER=gcc` as the supported smaller-DSH
policy for low-memory operation (`design/dsh-compiler-size-comparison-20261012-v20.md`).
Without bypassing the gate, reran daimos-testkit's
`test-daimos-run-late-rollback` with inherited `MAKEFLAGS=COMPILER=gcc`.
Native target **PASS**: 16 post-VM RUN descriptor-map failures under three
live children, stable process accounting and successful recovery. This
validates an important failure path of the v226 assembler discard replacement,
not every termination, signal, hierarchy or RT-ownership case. The separate
full process-lifecycle regression is tracked independently.

The independent `test-daimos-process-lifecycle` was also rerun with the
supported GCC configuration and **PASSed** native SIMH execution: RUN,
GETPID, WAIT/nohang, zombie reap, orphan adoption and explicit descriptor
maps. Neither test required a DSH image size override. This lifts the
previous general lifecycle acceptance blocker for these covered paths,
but does not prove RT-owner failure injection or all VM teardown faults.

Replaced the production KCC-generated `proc_slot_discard` with a dedicated
PDP-6 assembler implementation, guarded by `PROC_SLOT_DISCARD_PDP6_ASM` in
the C fallback build. It preserves separate discard semantics: FREE early
return, conditional RT owner release, transition setup and failure rollback,
VM destruction, TTY/u-area release, run-queue removal, descriptor zeroing
and trailing slot trim. This is NOT yet a shared teardown middle with EXIT.
The lowmem disk-root permanent-size build PASSes: KCORE=`025127` octal
(10839 decimal), KCORE+MRES=13702, PERMANENT_LAST=13749 decimal. Net
post-link saving versus v223: **two words**. No persistent data added.

The targeted native RUN late-rollback fixture was attempted but stopped
before executing: DSH `036013` exceeded the unchanged production `036000`
EXEC image gate. Thus the prototype is linked and size-measured, but native
RUN rollback and resource ownership remain unaccepted. Fix the independent
DSH image-size problem, or use a genuinely independent targeted fixture,
before claiming native completion. Do not bypass the gate.

Source comparison identifies a real, but not yet safely merged, code overlap:
`proc_slot_discard` in `proc/proc.c` is a production KCC-compiled resident
function (`build/system/boot/pdp6/kcore-asm/proc.s`), whereas
`proc_finish_slot` is handwritten in `proc/proc_pdp6.s`. Both release RT
ownership, set process TRANSITION, call `vm_space_destroy`, release session
TTY ownership and u-area, remove the runnable slot and trim process slots.

The generated `proc_slot_discard` is about 50 assembly instructions in the
inspected build, including save/restore and local labels. This is *gross*
code to investigate, not 50 words of recoverable savings. Its implementation
differs from finish-slot materially: discard checks FREE, does not perform
session-leader HUP teardown or adopt children, and frees the slot instead
of publishing ZOMB. Preserve VM-failure transition clearing and the exact
MM/u-area release order. `proc_exit_finish` is a separate small generated
wrapper; fatal-event termination can enter `proc_finish_slot` directly.

Candidate A architecture: a single carefully delimited teardown *middle*
operating on a validated slot and explicit caller contract, with distinct
normal-exit and failed-creation publication tails. First measure whether
replacing the generated discard path with an ABI-compatible assembler
implementation can reduce the full post-fold KCORE without an extra hot-path
dispatch or more permanent state. Do not merge paths by inferring lifetime
semantics merely from similar instructions. Native lifecycle/RT/failed-RUN
injection acceptance is a prerequisite. No source change or saving is
claimed from this inventory.

### Proposal A lifecycle/RT audit v224 (2026-10-24)

### Proposal A cleanup ownership and rollback v225 (2026-10-24)

Inspected three distinct cleanup entries: `proc_finish_slot` (normal/fatal
termination), `proc_slot_discard` (failed creation) and
`proc_event_fatal_other` (closes the target's file table before finishing).
Although all involve VM/u-area/runq release, their ordering and ownership
requirements differ. No shared lifecycle state machine or MRES dispatch
has been introduced; a new wrapper is not justified without actual net
post-fold saving and proof of failure behavior.

Existing daimos-testkit host `proc-abort-transition-v40` PASSes against
current `proc.c`, including held TRANSITION during VM destroy, failed
destroy rollback, successful retry and slot reuse. The first invocation
could not find `pdp10-sixbit.h`; a rerun with `CPATH` pointing to the
existing DAIMOS `tools/host` headers PASSed. No test source changes.
This is focused C host coverage, not native PDP-6 assembler acceptance.
The DSH image gate continues to block the full lifecycle simulator fixture.


The RT owner release at the beginning of `proc_finish_slot` is intentional
in the existing C fallback (`proc.c`) as well as PDP-6 assembler. The same
early release is used by `proc_slot_discard` before VM destruction. This
ordering cannot be unilaterally changed in assembler as a size optimization:
failure-path policy must be investigated in both paths and with the event
fatal-process caller. No RT rollback bug is established by code inspection
alone, and no behavior change was made.

The v223 runtime verification attempt using daimos-testkit
`test-daimos-process-lifecycle` failed at image construction, before any
native process assertions: DSH `036013` exceeds the unchanged `036000`
EXEC gate. This repeats the independent v222 fixture blocker; it is not
a pass or evidence of a new process-management failure. A dedicated
failure-injection test of RT ownership during teardown remains open.

Further A work should target duplicated resource acquisition/release
mechanics across process exit, failed slot creation and event termination,
while preserving their distinct ownership and commit points. Do not add a
common service wrapper without proving lower post-fold memory consumption.

### Proposal A first source experiment v222 (2026-10-24)

### Proposal A process-finish RT ownership v223 (2026-10-24)

Replaced the four-instruction `MOVE/CAME/JRST/SETZM` real-time-owner
conditional in `proc_finish_slot` with `CAMN/SETZM`. This preserves the
ownership test and termination order without a new branch or storage.
The GCC lowmem disk-root `permanent-size` link passes with KCORE
`025131` octal (10841 decimal), KCORE+MRES=13704 and
PERMANENT_LAST=13751 decimal words; net reduction from v222 is two
resident words. A native process termination / RT-owner regression still
must be run; linker success is not runtime acceptance. Architectural
cleanup remains the priority over further micro-optimizations.


`proc_adopt_children_pdp6` now uses `CAIN` to branch directly for the
INIT-parent case, eliminating the former `CAIE/JRST/JRST` arrangement.
The production lowmem disk-root `make permanent-size` completed and reports
KCORE `025133` (10843 decimal words), one word below the previously
recorded `025134` baseline; KCORE+MRES is 13706 words in this build.
The lowmem map is profile-dependent, so do not compare it to the stale
full-profile map in another build directory.

Native process-hierarchy regression was attempted via daimos-testkit, but
its fixture build stopped at the *unchanged* strict DSH image size limit:
`036013` > `036000` octal. It did not execute the runtime assertion stage.
Do not call the behavior native-accepted; keep the production DSH size gate
unchanged. The next A step remains a measured call-site/lifetime audit of
process cleanup, signal/session and TTY responsibilities.

Measure total physical RAM (KCORE + MRES + dynamic allocations) first, and
the permanent KCORE endpoint separately. Execution cost is nearly as
important. None of these candidate savings is yet measured or additive.

1. **A: process-management decomposition.** Keep interrupt, scheduler,
   context-switch, runnable-queue, essential sleep/wakeup and exit-state
   paths in KCORE. Audit child hierarchy, terminal/session management,
   signal policy, wait/reap and resource teardown for duplicate cold logic.
   Eliminate code before considering MRES relocation; do not put another
   dispatch on the context-switch or tick path.
2. **B: file/VFS/MonitorFS consolidation.** Share repeated validation,
   exceptional dispatch and error handling where actual paths overlap;
   preserve existing provider ABI, direct fast paths and compatibility.
3. **C: EXEC/RUN consolidation.** Loader code is already shared. Fold only
   further proven duplicated validation and startup paths, while retaining
   distinct resident EXEC replacement and RUN publication transactions.
4. **D: MM architecture.** Preserve MM-authoritative ownership/pinning;
   profile real fragmentation and extent high-water before redesign. The
   current 32 two-word descriptors total only 64 words.
5. **E: packed inactive processes.** Long-term total-RAM candidate; assess
   registry, codec, stack and swap costs; retain disk swap until verified.
6. **F: scratch lifetime sharing.** Reuse nonoverlapping buffers only with
   proved reentrancy/ownership and a positive measured total-RAM benefit.
7. Revisit assembler/compiler cleanup and then additional JRST folding only
   after architecture opportunities are exhausted (subject to >20-word gate).

Proposal A initial source inventory (not an implementation):

- `system/kernel/proc/proc_pdp6.s` is one mixed assembly unit, with
  2164 text, 3 data and 3 BSS words in the available
  `build/system/boot/pdp6/proc_pdp6.dobj` (pre-fold, not a freshly
  rebuilt v221 object). Its size is NOT solely scheduler cost.
- Scheduler selection is near source lines 120-315, runnable queue near
  589-640, sleep/wakeup near 2345-2541 and PI/context-switch/timer work
  near 2597-2944. These are likely hot or latency-sensitive paths.
- Child adoption and finish/reap, u-area/session release and wait/status
  occur near lines 329-1024. Events/signal control and TTY/canonical
  input/output occupy substantial additional code in the same object.
- The current map exports `proc_finish_slot`, `proc_wait_status`,
  `proc_event_apply`, `proc_tty_canon_input`, `proc_sched_pi_resched` and
  `proc_swap_victim`; no cold-path move can yet be declared safe.
- Next: construct an entry/call-site graph including PI/indirect paths,
  then identify *one* duplicated internal routine or policy path to simplify.
  Compare full post-fold RAM and native scheduler/process/TTY tests; do not
  implement dispatch-only movement which merely hides memory in MRES.

## Acceptance and documentation

- [ ] For every candidate record post-p10fold KCORE text/data/BSS, total
  permanent/MRES RAM, boot dynamic words, MM descriptor high-water, peak
  process RAM, worst-case cost and actual maximum concurrency.
- [ ] Update PROJECT-STATE.MD and pdp10-doc's resident limits, GCC/KCC build
  policy and dated 32K acceptance evidence only after corresponding tests pass.

## Completed items removed from the active backlog

The current source already provides boot-selected process-slot tiers starting
at 24 (`proc/proc.h`, `proc_boot.c`), a compact three-word process descriptor,
an intrusive runnable queue, a word-only eight-word pipe, a p10fold build
path, and an explicit strict GCC 32K size gate. These are baselines to test
and optimize, not absent features to implement again. The v33-v45 packed
registry/transaction components are staged prototypes, not production KCORE.

### Proposal B native PTR/PTP acceptance v237 (2026-10-24)

Re-ran `daimos-testkit test-daimos-pt-wordio` using the supported GCC
userland configuration and `SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy`.
Native SIMH **PASS**: PTR/PTP WORDTOKEN8 full and partial physical
round trip. The earlier status-139 window-event failure is not reproduced
with the headless SDL configuration. This strengthens native acceptance
of the v233 bulk-device argument consolidation without changing any kernel
source, persistent data, or the 036000 DSH image limit. KCORE remains
025075 octal (10813 decimal), KCORE+MRES=13676, based on the prior
full lowmem link. Continue proposal B on file/VFS structural savings.
