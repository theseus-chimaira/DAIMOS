# KCORE feature-phase size baseline v735 (2026-10-30)

Source HEAD: `eb7a407` (v734), with pre-existing working-tree changes
in KCORE.MD, PROJECT-STATE.MD and userland/dsh/dsh_exec.c left untouched.

Command: `make -C system/boot/pdp6 permanent-size BOOT=dsk ROOT=disk`.
This is a fresh linked GCC/package measurement; the report exits nonzero
because the complete optional package inventory exceeds address 040000.

| Item | Octal | Decimal words |
| --- | ---: | ---: |
| KCORE endpoint | 021030 | 8728 |
| KCORE + all included MRES packages | 041406 | 17158 |
| Single-root permanent endpoint | 041465 | 17205 |
| Multi-root permanent endpoint | 041526 | 17238 |

The report marks the single-root endpoint as 822 words past the allowed
040000 boundary (distance -821, free -822), and the multi-root endpoint
as 855 words past it (distance -854, free -855). These figures refer to
linked package footprints, not the real simultaneously installed MRES
set or runtime physical RAM high-water.

The v513 historical baseline is 8548 KCORE words: net +180 linked words.
Do not attribute this difference to any one optimization; intervening
changes also add features and bootstrap functionality. The strict KCORE
under-020000 target is deferred until the KCORE feature freeze.

## Feature and correctness gates (not size-optimization tasks)

1. Diagnose the native v669/v670 full-system scheduler/`?RT` D6FS root
   bootstrap blocker; isolated swap failure tests do not replace this gate.
2. Complete shared AUXSTORE/BACKING integration, native provisioning and
   failure acceptance; retain fail-closed VOLUME behavior.
3. Complete D6 codec integration and TSFS v1.2 sharded metadata against
   the agreed VFS/KCORE interfaces.
4. Complete the privileged raw MEMFS XIP process path and full native
   process-lifecycle acceptance before enabling it in production.
5. Complete and benchmark opt-in compressed process swap, including
   rollback and mixed compressed/uncompressed records.
6. Revalidate all KINIT/MRES direct bindings and driver watchdog failure
   paths, including removable-provider lifetime behavior.
7. Measure actual loaded-module sets and peak physical RAM independently
   of packaged linked totals, and rerun GCC/KCC native acceptance.

There is no claimed feature completion in this measurement-only record.
No code-size reduction should be attempted until these functional gates
are accepted or explicitly scoped out of the feature freeze.

The existing `daimos-testkit/tests/validation/daimos-dsk-watchdog-fix-v734.md`
records a native corrected-image countdown regression PASS: the left-half
count changes `000454` to `000453` while the right-half descriptor remains
`035024`. This does not establish full timeout or scheduler-pressure PASS.

## v736: Full-system scheduler fixture reproduction attempt

Ran the native `daimos-mm-swap-scheduler-v669.sh` runner against the current
checkout, with the pressure INIT compiled anew and an isolated `BUILD`
directory. An outer 115-second deadline expired while the full userland
image was still compiling. Make terminated and removed unfinished output;
the simulator did not start. **No scheduler or root-mount result is claimed.**
The compiler emitted repeated cross-include search-order and libc builtin
prototype warnings, but no identified fatal compiler diagnostic preceding
the timeout. A subsequent run needs a longer build allowance or a verified
prebuilt CLEAN image with the matching pressure-INIT hash; do not bypass
the fixture's hash and CLEAN-media preflight checks.

Source review confirms `d6fs_super_select` intentionally rejects DIRTY
generations for a normal mount; the explicit read-only recovery path uses
`d6fs_super_select_recovery`. Do not change that policy based solely on
the historical v670 trace. The earlier v670 failure is still unresolved.

## v737: Full image built; pressure writer fails (not accepted)

Re-ran `daimos-mm-swap-scheduler-v669.sh` from a fresh isolated BUILD with
a 540-second outer limit. The image and separate test INIT compiled, disk
creation completed, and `d6fsck` reported `state=CLEAN` for the generated
root. The PDP-6 SIMH run reached the KINIT device banner and reported
`D6FS LOADED`, so the historical pre-VFS `?RT` did not recur in this image.
The workload emitted `<wd!W` and remained without the required success
markers until the runner's own 300-second simulator timeout (exit 124).
The runner returned failure; **this is not scheduler/swap acceptance**.

The existing testkit `daimos-mm-swap-writer-diagnostic-v741.md` records
more detailed independent follow-up: the writer's WAIT sees TERM rather
than the expected normal EXIT. The source includes a TERM-on-swap-in-failure
branch, but the cause of TERM has not yet been established. Continue at
that narrower fault boundary, rather than weakening D6FS mount policy or
repeating the original coarse v283 workload without extra instrumentation.

## v745: TERM tracing must use the linked kernel map

The retained v744 event-entry trace shows AC1=2 (target slot) and AC2=1
(TERM), but its simulator script uses a fixed breakpoint at octal 011215.
The currently linked KCORE map instead resolves `proc_event_apply` to
011472. These are different build addresses; the old breakpoint cannot be
assumed to identify the event handler in a different image. The v743
swap-in failure probe also uses fixed instruction addresses and did not
observe its expected failure breakpoint. Neither observation proves which
caller sent TERM in the current linked build.

The new independent testkit `daimos-mm-swap-term-event-v745.sh` resolves
the `proc_event_apply` entry point from the fixture's own `kcore.map` and
requires a real breakpoint hit, rather than trusting an absolute address.
Its shell syntax check PASSed; no native execution of this new probe is
claimed yet. Next acquire a verified CLEAN, pressure-INIT-matched fixture,
run the probe and capture the caller/return address before changing swap
transaction behavior. Testkit commit: `85f7e81`.

## v745-v746: Native event entry and stack captured

Found retained CLEAN original pressure fixture
`daimos-mm-swap-scheduler-v669-22735/build/system/boot/pdp6` with
`kcore.map` and the pressure-INIT hash. The old v744 trace copy is DIRTY
and was not reused. The testkit v745 run against a copied disk PASSed its
map-resolved event-entry test: `proc_event_apply` at 011215, AC1=2 (slot),
AC2=1 (TERM). The next versioned testkit v746 run captured the current
executive stack on the same path: AC17=000003040117, memory at 040117
contains 340000027740, and 040116 contains 000000000002. This provides
a return-site candidate at 027740; it is **not yet confirmed** as a
specific instruction or provider-module call. The event cause remains
unknown. Neither run is a complete scheduler-pressure PASS.

Testkit v746 commit `b4367e8` includes the map-resolved native stack
probe. Subsequent investigation must resolve 027740 using the actual
runtime-installed module placement, not assume the MRES map's package
offsets are absolute runtime addresses.

## v747-v749: Failure localized to resident allocation during swap-in

Native v747 captured runtime instructions 027734-027740. Instruction
027737 is `PUSHJ 17,011215`, calling the linked `proc_event_apply`.
This matches the `proc_swap_service_failed` sequence in the installed
SWAP MRES: the swap-in service itself delivers TERM after a failed
`proc_swap_in`. The actual loaded-module placement was verified against
the assembled package offsets, not assumed from a fixed historical map.

On the same CLEAN pressure fixture, v748 broke at 027662, the
`proc_swap_in_alloc_fail`/common local cleanup entry. It did *not*
first hit the separate pin-failure or backing-read/restore-failure
breakpoints. Native v749 captured the pre-call register arguments at
027562, the `mm_alloc_aligned` call site: AC1=000000064000 (octal
words requested), AC2=000000002000 (octal alignment), AC3=1 (process
extent type), AC4=2 (process slot), AC13=000000064000. These are
26,624 words with 1,024-word alignment, for slot 2. The request is
plausible for a large resident user image, but whether allocation
failure reflects exhaustion, fragmentation, pinning or MM state is
**not yet known**. Do not add a fallback or weaken APR alignment.

v747/v748 and v749 are committed to `daimos-testkit` as `8a90685`
and `6b4d3b2`. All three traces are diagnostic PASS only, not
full-system scheduler-pressure acceptance. Next capture the allocator
free-list/extent state and reason for failure before changing production
MM or scheduling code. No KCORE reduction was performed.

## v750-v754: Physical RAM and swap-capacity exhaustion quantified

The native v750 allocator return at 027563 is AC1=-1 (`MM_ERR_NOMEM`),
not a descriptor error. Extent capacity is octal 0210 (136) and count
octal 042 (34), so descriptor exhaustion is excluded. The v751 captured
extent table reconstructs one managed arena from octal 040032 to
01000000, containing 245734 decimal words. Nine resident process
extents each occupy octal 064000 (26624) words. All 34 extents occupy
244827 words; total free is just **907 words**, with a largest gap of
**760 words** (147 in the interior and 760 at the arena end). There is
no free octal 064000-word aligned extent. This is real physical RAM
exhaustion at the observed swap-in allocation, not an APR-alignment or
extent-descriptor defect.

Native v752 confirms the SWAP MRES `proc_swap_reclaim_fail` branch is
entered; v753 independently observes an earlier victim-selection return
of slot 2, so victim choice sometimes succeeds but that observation is
not necessarily from the final failing transaction. At the v754
reclaim-failure breakpoint, AC10=064000 (request words), AC11=02000
(required alignment), AC12=2 (excluded requester slot), and backstore
capacity/usage are 04000/03520 octal blocks, i.e. 2048 total,
1872 used, **176 blocks free**. A complete 26624-word image requires
26624/128 = **208 blocks**, larger than the remaining backing capacity.
The process's restore needs another resident extent, and reclaim cannot
create one by writing another full-size victim into the available swap
space. The failure is a combined resource-capacity deadlock; it is not
evidence of a corrupted allocator.

The existing scheduler nevertheless converts every failed `proc_swap_in`
to fatal TERM, including resource shortage. This is a **correctness and
admission/liveness policy issue**, not a code-size issue. Do not simply
retry from the slot-0 idle loop without ensuring a resident runnable
process can make progress (or the system may spin indefinitely). Do not
change the return code of all swap-in failures to recoverable: invalid
swap records and I/O integrity failures must remain distinguishable.
Before a production patch, design/verify a recoverable resource-pressure
result, a scheduler path that can run existing resident processes, and an
admission/headroom invariant for mixed process extents and BACKSTORE.

Native diagnostic probes v750-v751, v752-v753 and v754 live exclusively
in daimos-testkit, commits `fbed786`, `acc0fa4` and `64ffbdb`.
They are bounded observed-state tests, **not** scheduler-pressure PASS.
The KCORE feature-phase measurement remains unchanged; no size reduction
or production kernel edit was made in this diagnostic series.
