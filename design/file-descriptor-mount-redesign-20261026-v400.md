# Proposal B v400 — canonical-vnode descriptors and scalable mounts (design only)

**Status:** DISCUSSION DRAFT. No production API, source, tests, ABI or semantics have been modified. User-approved design constraints: **at least 26 simultaneous mounts**, future expansion possible; **alternative B**, a two-word process-local descriptor with an intact canonical vnode and a full 36-bit word offset; retain duplication and shared-offset functionality **where useful**. Approval of these goals is not approval of a concrete encoding or migration.

## Source-grounded baseline

- `system/kernel/fs/file.h` defines 16 descriptors of two 36-bit words (`node_meta`, `offset`). FILE packs READ/WRITE, DIR/APPEND, fd-family and lock-state information into bits overlapping the canonical vnode. `system/kernel/fs/file_runtime.s` reconstructs mount bits 3 and 4 from two bits per descriptor in the u-area word at `file_table+041` octal, via `file_node_from_fd`; DUP/DUP2, FIFO substitution, OPEN, CLOSE, FLOCK and unmount must maintain this extension.
- In `file_clone_descriptor_v361` and `proc_run_map_loop`, both descriptor words are copied. The observed implementation thus copies an offset value; it does **not** by itself establish a shared mutable open-file offset. Existing owner-family/lock and pipe reference relationships must not be mistaken for shared offset storage.
- `system/kernel/fs/vfs.h` uses a six-bit mount field (`VFS_MOUNT_MASK=077`) but still defines `VFS_NMOUNT=4`; the old four-mount comment is inconsistent with the current `system/kernel/boot/kinit.c` path, which calls `root_select_mount_capacity_v22`, allocates the six-array MM mount slab and publishes `vfs_mount_limit` last. `design/vfs-boot-capacity-20261016-v22.md` documents a capacity planner up to 26, with active profile-dependent counts (typically four to nine). **26 is a representational/planner ceiling, not proof of a 26-mount live boot.**
- The six MM mount arrays currently cost `6*N` words. Policy holds 26 read-only flags plus five-bit SWAP/LOGSTORE ownership in one 36-bit word. PURE backing and optional provider/personality bounds must be audited before accepting higher capacity; do not silently truncate mount IDs.
- `design/file-mount-packing-20261016-v18.md` records **138 additional permanent words for its historical v19 mount-extension integration relative to v18**. This is a historical comparison, **not** a recoverable v400 saving estimate. Latest current full-link baseline from v388: KCORE 9109, KCORE+MRES 15657, PERMANENT_LAST 15704.

## Proposed v400 architecture (not approved at bit level)

1. Keep descriptor words `[canonical vnode36, offset36]`; an empty descriptor uses a defined invalid vnode encoding, not random overlaid metadata. Direct vnode transfer should remove repeated mount-extension extraction and restoration on hot paths.
2. Represent necessary per-fd flags (read/write, directory/append, open state) in **existing** per-process spare fields if feasible; do not increase the u-area or allocate a per-open object merely to move metadata. Before choosing a bit map, inventory all uses of existing credential spare bits, PURE backing, scheduling and historical lock metadata. A 36-bit bitmap is insufficient for arbitrary multibit flags on 16 descriptors; propose either fewer states, computed permissions from vnode/provider, or a measured explicit allocation tradeoff rather than assuming capacity.
3. Preserve 16 fd slots initially; support at least 26 mount IDs without a descriptor mount-extension map. A future wider mount policy is a separate measured design (RO bitmap/owners and provider/PURE formats). Do not reduce mount count to simplify descriptor encoding.
4. Define DUP/DUP2 and RUN remapping explicitly. **Baseline semantics to discuss:** duplication creates an independent fd with a copied offset (as the examined assembly currently does), while pipe/FIFO ownership remains ref-counted. True shared mutable offsets can be offered only via an explicit shared-object/lifetime mechanism or a very narrow modeled relation; it is not free in the two-word format. Determine whether any current application needs shared offsets before paying for such a mechanism.
5. Define locks by canonical vnode and owner identity, rather than embedding mount reconstruction in lock comparisons. Avoid changing concurrency semantics until affected operations and failure/reentry paths are traced; a lock rewrite is a separate approval decision.
6. Make mount policy and provider lifetime independent of descriptor encoding. Keep complete mount ID in the vnode; for future >26 slots, a new reservation-policy representation must be costed and tested, not quietly clamped.

## Three implementation alternatives to benchmark

| Candidate | Descriptor RAM per process | Cost/benefit to establish | Semantics |
|---|---:|---|---|
| Current packed | 32 words + existing extension bits | Baseline native post-link size; frequent vnode reconstruction and lock comparisons | Copy offsets on DUP/RUN; packed lock owner |
| B1 canonical + reduced flags | 32 words + proven reused spare fields | Determine exact flag bitmap and resulting text/data size; reject if it steals live credential/PURE bits | Independent copied offsets; explicit vnode identity |
| B2 canonical + tiny shared-offset capability | At least B1, plus dynamic shared state when requested | Allocator metadata, lookup/close/RUN overhead, peak MM RAM and failure cleanup must be measured | Optional shared offsets with explicit lifetime |

B1 is the **first analysis target**, not an instruction to implement. B2 is reserved for a demonstrated need; arbitrary shared offsets cannot be inferred from plain copied descriptors.

## Migration / acceptance gates

- Trace all readers/writers of `node_meta`, `file_table`, descriptor table offsets, the per-fd extension map and lock-family bits in FILE, proc RUN/EXEC/exit, syscall fast paths, pipes/FIFOs, TTY/device endpoints and alternative C paths. Identify any code that interprets the packed bits without invoking FILE helpers.
- Define a complete packed-to-canonical contract and all failure paths **before editing production code**. Stage changes behind explicit versioned interfaces, with exact total-RAM and hot-path costs; migration must not leave mixed descriptor formats executable.
- Put new tests only in `daimos-testkit`: 26 distinct mount IDs (including 16/26), simultaneous mount/unmount, two mounts with same provider/local vnode, mount owner reservations and readonly flags, open/close, duplicate and DUP2 replacement, independent and optional shared offsets, RUN fd remapping, FIFO/pipe refs, provider-7 TSFS-vs-PIPE distinction, FD exhaustion, lock contention, failed OPEN/CLOSE/RUN/UNMOUNT, swap/PURE lifetime and disk/tape boot.
- Compare PDP-6 and PDP-10 full post-fold KCORE/MRES/permanent images, peak MM allocations, per-process bytes/words, operation timings and simulator regressions. No compatibility-only prohibition, but correctness, real total RAM improvement, efficiency and essential functionality are mandatory.

## Open decisions for review

1. Is copied-offset DUP/RUN sufficient, with shared offsets only when explicitly requested? (Current two-word assembly copies offset values.)
2. Which permissions and descriptor state should survive, and can they be computed or represented without per-process growth?
3. Is the 26-slot requirement satisfied by boot-dependent mount capacity, or must any given supported configuration have 26 slots independently of detected hardware?
4. Should provider-lock semantics change in this redesign, or remain untouched until descriptor correctness is established?

**No implementation is authorized by this document.**
