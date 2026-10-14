- [x] v102 PDP-6 native relocation regression covers stable u-area stop/runqueue restoration and pinned exclusion; PASS, no production code change. See mm/MM-VM-PIN-RUNQUEUE-V102.md.
- [x] v101 production PDP-6 MM+VM relocation checks all 512 payload words and mapped/unaligned rejection, PASS; no code-size change. See mm/MM-VM-FULL-PAYLOAD-V101.md.
- [x] v100 native PDP-6 MM relocation with actual `vm_extent_move` and `fs_move_words` bodies PASS; safe/current/TRANSITION cases, partial data checks. No code-size change; see mm/MM-REAL-VM-REGRESSION-V100.md.
- [x] v099 reviewed real `mm_move_extent` 66 words; native simulator relocation success/rollback + PI fixture PASS. Full VM integration and measured shrinking remain open; see mm/MM-RELOCATION-AUDIT-V099.md.
- [x] v096 evaluated gap/no-reclaim semantics and hoisted one invariant alignment mask: `mm_find_fit` 90 -> 85 words; full MM text 496 -> 493; native regression and DXR links PASS. Second extent scan in no-reclaim allocator remains a measured-candidate only.
- [x] v095 no-reclaim allocator required for packed and block-cache critical paths; 73 -> 71 words (-2), all four native MM tests and full KCORE/KINIT links PASS. See mm/MM-NORECLAIM-OPTIMIZATION-V095.md.
- [x] v094 `mm_find_fit` shares interior/tail gap evaluation, saving 22 PDP-6 words (112 -> 90); native tests and KCORE/KINIT link PASS. See mm/MM-FIND-FIT-OPTIMIZATION-V094.md.
- [x] v092 native PDP-6 `mm_free` and extent-descriptor shift/reuse test passes without full OS; see daimos-testkit/tests/native-kernel/mm/free-v092. Pin-boundary, fragmentation, compaction and reclaim suites remain open.
- [x] v090 begins per-algorithm native PDP-6 MM tests and an optimization/folding register. BLT, canonical instruction ordering and per-algorithm measurements documented in mm/MM-NATIVE-TEST-AND-SIZE-AUDIT-V090.md. No allocator optimization yet.
- [ ] v089 real-MM readiness audit identifies missing native allocator pressure/retry acceptance and outdated host mm-core storage-state fixture; see mm/MM-PACKED-READINESS-V089.md. No MM algorithm changed.
- [x] v088 links packed codec/transactions, slot-0 restore dispatch and VM destroy into KCORE; native relocatable decode PASS; KCORE/KINIT DXR links PASS. Boot and scheduler eviction policy remain unverified.
- [x] v087 removes the packed-registry build toggle, links a permanent
  registry descriptor in KCORE and unconditionally allocates its MM map
  from KINIT. Fresh GCC KCORE and KINIT DXR links PASS. Packed scheduler,
  teardown, and codec activation still pending; disk BACKSTORE remains.
# KCORE - Verified Open Work and Optimization Backlog

Date: 2026-10-13
Basis: current DAIMOS checkout, source code, PROJECT-STATE.MD and
active build notes. A staged, host-tested implementation is not considered
completed until it is linked, measured and accepted on the relevant PDP-6.

## P0: 32K resident limit and correctness

- [ ] Bring the GCC lowmem KCORE exclusive link endpoint to at most 020000
  octal (last resident word at most 017777). A fresh isolated GCC/disk-root
  baseline built on 2026-10-13 gives __kcore_low_end=023503 octal; the older
  PROJECT-STATE.MD v41/v42 021541 result used a different configuration.
  Do not compare these as a same-configuration optimization delta.
- [ ] Measure full permanent KCORE plus committed MRES, irreducible scratch,
  KINIT allocations and process startup headroom; retain the separate general
  below-040000 permanent-address limit where applicable.
- [ ] Pass the strict GCC 32K size gate and native 32K boot/INIT/LOGIN/DSH,
  process-pressure and filesystem tests with the same fresh image.
- [ ] Fix compiler, assembler and kernel correctness bugs at their source;
  add regression tests in daimos-testkit before marking the sweep complete.
- [ ] Audit source-level size hotspots, generated objects and real post-p10fold
  link maps; reject optimizations that increase the 32K resident footprint.

## P1: packed-process lifecycle and compression

- [ ] Choose a compact dense or bounded sparse packed registry using measured
  whole-KCORE code + dynamic data + MM extent cost, not isolated v33-v45
  prototype sizes; preserve MM-authoritative extent ownership.
- [ ] Connect KINIT registry allocation, boot abort, process exit, VM destroy,
  packed publication and teardown; validate exclusion and failure rollback.
- [x] Stage inline KPACK v073 ABI adapters to the v071 encoder and v070
  direct-buffer decoder; native PDP-6 roundtrip and AC10..AC16 checks pass.
  Registry-aware eviction/restore and scheduler connections remain pending.
- [x] v085 expands v084 destruction rollback tests: invalid slots,
  absent TRANSITION, preserved backing on pinned extent, dense/sparse
  reuse and DISK path; host-MM PASS, PDP-6 cross-compile PASS.
- [x] v084 staged VM destroy dispatch retires PACKED MM ownership and
  preserves the original backing record until success, avoiding legacy
  disk-span detach. PDP-6 dispatch regression PASS; production VM linkage
  and KINIT boot registry activation remain open.
- [x] v083 staged slot-0 restore dispatcher routes DISK to legacy swap-in
  and PACKED to inline v082 restore. Native PDP-6 dispatch checks PASS;
  alternate assembly/full-kernel link and boot remain unverified.
- [x] v082 staged lifecycle gate checks process eligibility and storage
  mode, then calls native packed eviction/restore. PDP-6 dense/sparse,
  rollback, current-process and transition rejection PASS. Real assembly
  slot-0 scheduler routing and boot integration remain open.
- [x] v081 native PDP-6 allocation-failure rollback and subsequent
  eviction/restore: source pin/ownership and storage state verified in
  dense/sparse, zero/mixed cases. See proc/D6CODEC-NATIVE-ROLLBACK-V081.md.
- [x] v080 native PDP-6 transaction regression links real v077/v46/v079
  code and v078 codec with mock MM/process records: dense/sparse and
  mixed/zero payloads PASS. Full booted scheduler and physical MM remain
  pending. See proc/D6CODEC-NATIVE-MM-V080.md.
- [x] v079 binds v077 exact-size eviction and v46 restoration to the
  native inline codec callbacks. Host-MM cycle PASS; native PDP-6 public
  entry-point/codec callback test PASS with test-only transaction stubs.
  Native MM scheduler and boot remain open.
- [x] v078 native PDP-6 count-only inline encoder and KPACK measure/
  encode/decode adapters added. Fixed-address roundtrips for mixed data
  and 128 zero words PASS; no decoder-size optimization attempted.
- [x] v077 size-negotiated packed eviction measures compressed words while
  pinned, allocates an exact-sized MM extent, then verifies encode parity.
  Host MM tests cover two stream sizes and dense/sparse restore. Native
  no-allocation sizer and scheduler integration remain open.
- [x] v076 host-MM transaction test uses the real inline v068 codec for
  dense/sparse v45 eviction followed by v46 restore; verifies PURE prefix,
  restored content, ownership and registry retirement. Fixed a v45 leaked
  source pin on precommit allocation failure. Native scheduler and boot
  acceptance still pending; see proc/D6CODEC-PACKED-TRANSACTION-V076.md.
- [ ] Complete packed restore against the selected registry, rather than its
  private descriptor, then integrate packed eviction/restoration with the
  scheduler; do not remove disk swap before full native acceptance.
- [ ] Harden inline v066 further and recover resident code: the guarded
  native decoder is 88 words with unchanged VFS 56, total 144 (+48 over
  baseline). Address bounds and malformed descriptors pass native tests;
  producer parity, overlap/aliasing and whole-KCORE tests remain open.
  V067 eliminates one redundant guard and passes native tests: 87+56=143
  words, still +47; see D6CODEC-INLINE-SIMPLIFICATION-V067.md.
- [ ] Evaluate inline-token codec v065 as the lower-RAM alternative:
  native fixed decoder 74 words plus unchanged 56-word VFS adapter = 130,
  only +34 over the original 96 (paired grouped v058/v064 is +102).
  Across 81 DXRs the inline layout increases compressed payload by 3.10%.
  This is an inactive proof of concept, not yet malformed-input hardened or
  accepted for Stage-1/KCORE. See D6CODEC-INLINE-LAYOUT-V065.md.
- [ ] Recover the extra resident code cost of the grouped decoder: original
  fixed core spans 000060..000127 (40 decimal words) while the staged v058
  core occupied 129 words (+89) in v058. The guarded v062 standalone core
  measures 139 words (+99), with zero extra static data or decoder stack;
  neither satisfies a zero-net-growth resident KCORE requirement. Measured
  old combined fixed decoder + VFS is 96 words, v058 combined is 200
  (+104), and v062 + unchanged VFS would be 210 (+114). See
  system/kernel/core/D6CODEC-RESIDENT-DELTA-V063.md. The v064 BLT refill
  adapter saves two words (69 vs 71), but still leaves +102 combined with
  v058 or +112 combined with the guarded v062 decoder. Zero BSS/stack alone does NOT satisfy
  the no-extra-resident-RAM requirement. Measure the combined fixed core and
  VFS adapter before an atomic Stage-1 cutover.
- [ ] Implement the single fixed-000060 grouped-control decoder with the
  unchanged AC3/AC4/AC10..AC14/AC0 resumable ABI, atomic <=31-word group
  validation and compaction in the existing 64-word VFS refill window. No
  additional permanent or per-call RAM; staged v058 native tests pass, but
  production linkage and end-to-end acceptance remain open; see
  system/kernel/core/D6CODEC-RESUMABLE-ABI-DESIGN-v057.md.
- [x] Stage C99 host inline-token producer/decoder and DXR CLI v068.
  81-DXR exact parity (159973 raw / 135534 packed), 10000-word roundtrip,
  malformed/truncated input tests PASS in daimos-testkit. This is inactive.
  The 8K KCORE hard limit remains mandatory; defer optimization until the
  implementation and system-wide format migration are otherwise complete.
- [x] Stage transient TSFS v075 validators for MOUNT.TSFS and KINIT tape
  root: exact version 1.3, inline flag 2 in mount scanner. Both cross-compile
  for PDP-6; native mount/read and boot remain unverified.
- [x] Fix inactive TSFS inline runtime extent dispatch in v074: flag 2
  explicitly selects inline decode, flag 1 is rejected. Native host media
  inspection confirms v069 actually emits a compressed flag-2 extent.
  Runtime mount/version validation and live PDP-6 read remain open.
- [x] Stage TSFS 1.3 inline v069 producer/checker, preserving production
  TSFS 1.1 and rejecting legacy compressed images in both directions.
  Host tests PASS; runtime TSFS decode and native boot remain pending.
- [x] Stage v070 inline VFS adapter and named TSFS/EXEC/PURE call-site
  variants, without activating legacy symbols. Standalone PDP-6 link PASS;
  TSFS 1.3 runtime validation and full consumer behavior still pending.
- [x] Stage native inline encoder v071 (PDP-6 standalone literal/zero-run
  roundtrip PASS). Full native 81-DXR parity, KPACK C ABI and VM/MM
  integration remain open; do not activate incomplete codec pairs.
- [x] Stage alternate Stage-1 inline v072 sources for all five devices;
  all five DAS assemblies pass. Actual simulator boot is still pending.
- [ ] Integrate the production D6 codec with the resident KCORE encoder and
  streaming decoder, exact `000060` resumable ABI, bounded VFS refill,
  KPACK and EXEC/VFS/PURE consumers. Keep the detailed cross-subsystem plan
  in system/kernel/core/D6CODEC-INTEGRATION-TODO-v054.md.
- [ ] Switch host producers and checkers together to one grouped-format
  library (the unlinked C99 v060 candidate passed 81-DXR native-policy parity,
  exact decoding, corruption guards and sanitizer checks). Do not link it
  into production until the atomic boot and executable format cutover.
- [ ] Measure combined resident codec text, stack, workspace and peak MM
  descriptors; test direct ABI, malformed input, resumptions, packed
  transaction failures and real PDP-6 execution before activating it.

## P2: kernel data structures and workload-specific sizing

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
