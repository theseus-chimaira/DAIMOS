# KCORE open-feature and branch reconciliation v1674

Date: 2026-11-18. Documentation audit only; no production code change or new acceptance claim.

## Authoritative source boundaries

- `main` at `eb3b819f` retains the v1604 KCORE feature baseline. Its current `TODO.MD`, `KCORE.MD`, and `PROJECT-STATE.MD` include a newer operator/48K scope clarification. Later PURE work is **not merged into main**.
- `kcore-pure-reclaim-pressure-v1673-20261118` points at `ae84bb37` (v1672). It descends from v1603 and contains later v1605-v1672 changes. It is a separate development line, not proof of current mainline functionality.
- Evidence below is drawn from source-linked branch history and its `TODO.MD`, `PROJECT-STATE.MD`, `KCORE.MD`; physical test results are *recorded* evidence, not independently rerun by this audit.

## Decisions and feature inventory

| Feature / gate | Main | Later development line | Remaining action |
| --- | --- | --- | --- |
| P0-B DTFS trusted-operator durability | CLOSED v1575 | Later DTFS physical/native work v1655-v1656 | Keep CLOSED; do not revive historical hardware reel-interlock TODOs. Preserve real DIRTY/CLEAN, failure, cache and owner checks. |
| P1-C swap/storage physical matrix | CLOSED v1603 | Retained baseline | No reopen without regression. |
| P1-D dynamic FSINFO/controller matrix | CLOSED v1601 | Retained baseline | No reopen without regression. |
| P1-E cross-machine PURE | v1604 architecture review only, implementation HOLD | PDP-6 RAW PURE implementation and physical process tests through v1672 | Reconcile/integrate only after source and physical regression; KA10 target-specific MM/APR acceptance still not demonstrated. |
| Compressed PURE mode 3 | Not integrated | Staged bitmap codec, bounded decode, FULL restore, terminal MRES v1659-v1667 | Live reclaim, RUN/EXIT/WAIT, PI6 retry, error rollback, and measured net RAM required before enabling. |
| Swapless TSFS KINIT | Not integrated | v1665-v1668 root-selected PURE and strict no-disk-MRES boot | Recheck ROOT=auto/disk/DRM physical matrix and KCC full-profile acceptance. |
| Full-boot memory target | 48K words de facto minimum (policy) | v1669-v1672 48K/64K physical evidence | Continue 48K acceptance and continuous runtime high-water; 32K **kernel-only tests remain mandatory**. Full 32K boot deferred. |
| KINIT stack reserve | No change | v1671 256-word reserve; 130 words touched in sampled boots | Do not shrink without fatal/error/root-path evidence; this is optional optimization after freeze. |
| Final size optimization | Deferred | Various incremental changes already measured | Do not prioritize until feature freeze; preserve RAM-efficient design. |

## Concrete open acceptance gates on the later branch

1. Production bitmap mode3 compressed reclaim and restore across RUN/EXIT/WAIT, scheduler/PI6, terminal release, failures and MM owner recovery. It must remain disabled until complete.
2. Continuous *post-user* MM/process/scheduler peak under 48K/64K pressure and actual RAW PURE reclaim, not just sampled CTY barriers or KINIT allocator-event boundaries.
3. Source-built physical ROOT=auto, DSK/DRM, and KCC full-profile boot regression, especially swapless-vs-backed root selection.
4. Target-specific KA10 PURE hardware/APR/MM validation. PDP-6 physical success does not imply KA10 acceptance.
5. Preserve mandatory 32K **kernel-only** test acceptance. Do not treat older strict full-32K KINIT TODOs as release blockers.

## Recorded physical memory evidence (v1672 branch only)

- Four 48K children: MM free 27441 -> 18193 -> 27441 words.
- Seven 64K children: MM free 43793 -> 27609 -> 43793 words.
- Each child used 2048 APR plus 264 dynamic words; all sampled ownership and pins returned to baseline after WAIT.
- v1670 strict KINIT reduces sampled event-boundary peak by 1877 words at 48K and 64K, but **does not improve steady-state post-user free MM**.
- v1671 highest observed KINIT stack index 129/255; no basis to reduce the default reserve yet.

## Documentation reconciliation rules

1. Treat historical dated checkboxes as evidence, not a live backlog. The earliest top-level `TODO.MD` block on `main` is not authoritative for later *unmerged* feature branches.
2. Never conflate the v1604 review HOLD with v1672 PDP-6 implementation, or v1672 PDP-6 acceptance with KA10 acceptance.
3. Keep branch-specific results and build flags explicit. Mode3 staged linkage is not production admission.
4. The operator-trust P0-B contract, 48K full-boot target, 32K kernel-only requirement and deferred final optimization supersede older conflicting status text.
5. This report does not authorize merging the newer feature line, and does not assert a new full source-tree or physical test run.

## Production-link and implementation audit (source inspected)

The following is a direct source-level check of `kcore-pure-reclaim-pressure-v1673-20261118` at `ae84bb37`, not just an interpretation of historical checkboxes:

- `system/kernel/Makefile` defaults `DAIMOS_PURE_MRES_ENABLED=0`, `DAIMOS_PBM_V1667=0`, `DAIMOS_SWAPLESS_ONLY_V1668=0`, `VM_BACKEND=pdp6`. `DAIMOS_PBM_V1667=1` requires PURE MRES and conditionally adds `proc_pure_term_v1667`, bitmap terminal, restore and decode sources to **MRES_ASM**. It does not by itself install a working bitmap compressor into the scheduler.
- `system/kernel/proc/proc_pure_bitmap_compress_v1663.c` explicitly states that its bounded RAW-to-bitmap commit is **not installed in the scheduler, KCORE, KINIT or opt-in PURE MRES**. Its entry `proc_pure_bitmap_compress_v1663` performs MM owner and source preflight. The missing live reclaim call path is therefore an actual integration gate, not merely missing documentation.
- `system/kernel/proc/proc_pure_term_v1667.c` dispatches mode2 RAW to `pdp6_pure_detach_v1609` and mode3 bitmap to `proc_pure_bitmap_terminal_v1661`, but explicitly says the permanent `vm_space_destroy` still rejects mode3 and forbids runnable production admission before scheduler/lifecycle acceptance.
- `system/kernel/proc/proc_pure_bitmap_restore_v1664.c` includes the staged `proc_pure_bitmap_retry_v1667` owner validation and bounded FULL restore. Neither is proof of actual compressed-process RUN/WAIT or physical PI6 recovery.
- `system/kernel/proc/vm_pdp6_swap.c` routes terminal backing release through `proc_swap_term_v1666` into the selected MRES, retaining a small fixed-kernel cleanup path. This explains the code-placement optimization without asserting the bitmap path is live.
- The Makefile explicitly says no KA10 or KI10 VM backend has been validated; the selected backend defaults to PDP-6. Cross-machine acceptance remains a distinct gate.

## pdp10-doc cross-check

`pdp10-doc/KERNEL/SWAPLESS-MEMORY.MD` begins at v1488 and `pdp10-doc/KERNEL/MEMORY-MANAGEMENT.MD` begins at v1375. Both contain accurate historical baseline descriptions but their top sections **cannot describe the newer unmerged v1611-v1672 development as current mainline behavior**. Keep them historical until the relevant implementation is completed and integrated; use the branch-specific DAIMOS design and test reports for development status. `pdp10-doc/TODO.MD` currently has the newer policy note, while its v1604 feature baseline remains the accepted mainline boundary.

## Suggested next implementation order (not performed)

1. Finish the RAW PURE 48K physical reclaim and post-user high-water gates on the development branch, with the existing `daimos-testkit` as the sole test home.
2. Close the source-built ROOT=auto/DSK/DRM and KCC full-profile regressions so the optional PURE path cannot break disk-backed systems.
3. Implement and prove mode3 compressor invocation plus atomic MM release, restore, EXIT/WAIT, scheduler/PI6 retries, and source/media failures; leave default OFF until accepted.
4. Perform a controlled mainline integration review with separate baseline and feature tests. Do not merge the branch solely because its documentation is newer.
5. Pursue KA10 hardware-specific MM/APR implementation independently; do not make it a prerequisite for truthful PDP-6-only status reporting.

## 2026-11-19 follow-up: documentation cleanup and native regression

The active `TODO.MD` index was rewritten as a short accepted/deferred/open
feature list. Historical dated sections were retained unchanged as evidence,
including their now-superseded P0-B and full-32K requirements. `KCORE.MD`
and `PROJECT-STATE.MD` now explicitly identify the mainline/unmerged-branch
boundary. `pdp10-doc/TODO.MD` records only the accepted scope and historical
mainline status, not a claim that unmerged PURE is released.

The original testkit branch `test-pure-reclaim-pressure-v1673-20261118`
contains a **pre-existing untracked** `pure-runtime-reclaim-v1673/` test
fixture. Do not overwrite or assume its test results; it may be active work.
The v1672 native test was independently rerun against the existing PURE
source worktree on 2026-11-19:

- `tests/validation/pure-runtime-pressure-v1672/run-meminfo-native-v1672.sh`
  PASS: original SYS_MEMINFO/PROCINFO zombie accounting and both executed
  unsafe assembler mutations rejected.
- This is native assembler verification only. No v1673 forced-reclaim
  physical guest, continuous high-water or bitmap mode3 acceptance was run.

Next implementation step: inspect the existing v1673 fixture and its status
before changing any test or kernel source. Preserve the untracked fixture
and its independent worktree; do not duplicate concurrent test development.

## v1673 integration inspection (2026-11-19)

The dedicated `daimos-testkit` development worktree
`test-pure-reclaim-pressure-v1673-20261118` now has a **staged**,
not-yet-committed `Makefile` opt-in target and ten-file
`tests/validation/pure-runtime-reclaim-v1673/` fixture. It is not
present on testkit `main` and should not be described as integrated.
The fixture builds a source-matched disposable 48K TSFS image with nine
sleeping PURE children, a refused tenth RUN, three CTY/MM checkpoints,
PACKED RAW owner accounting and exact post-WAIT owner recovery. It
requires general and strict profiles to agree and executes negative
oracle mutations. Mode3 stays OFF. Static shell/Python syntax and
staged Git whitespace checks PASS.

Review detail: README says the child sleeps 600 simulated seconds, but
`proc-pressure-v1673.c` currently calls `dsys_sleep(1800U)` for children;
the later PID1 idle loop uses 600U. The v1673 README was corrected to describe the actual 1800U child
sleep and distinguish the 900-second host timeout. The test runner's
900-second timeout is host wall-clock, not necessarily simulated time.
No kernel modification or forced-reclaim physical PASS is asserted here.

## 2026-11-19: v1673 RAW reclaim testkit integration

The existing staged opt-in test suite was source-reviewed, checked for shell
syntax, Python syntax and Git whitespace, then committed/pushed **on the
separate daimos-testkit development branch**
`test-pure-reclaim-pressure-v1673-20261118` as commit `1848722`.
It adds `make test-pure-runtime-reclaim-v1673` and ten files (Makefile
plus nine fixture files) under `tests/validation/pure-runtime-reclaim-v1673/`.
It does not modify DAIMOS production source or enable bitmap mode3.

One retained **general-profile** 48K physical JSON report was inspected:
free MM `27441 -> 2794 -> 27441`, process APR owners `1 -> 1 -> 1`,
PACKED RAW owners `0 -> 9 -> 0`, all nine packed owners exactly 128
words smaller than their logical images (1152 total), and exact baseline
MM descriptors after WAIT. At pressure, total free is 2794 but the
largest contiguous free extent is 1641, below a 2048-word APR request;
there is a spare process slot. This supports genuine fragmented-memory
RAW reclaim in that retained **single** run, not paired acceptance.

A separate existing v1673 physical guest run was already in progress.
A duplicate newly launched build was interrupted to avoid competing for
SIMH/build resources. The ongoing run was not modified. **Paired
48K general/strict comparison, five physical transcript mutations and
forged-summary rejection remain unverified in this audit** until that
run completes and the saved reports are checked. No continuous peak,
KA10, mode3, disk-root or PI6 fault acceptance is inferred.
