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

## 2026-10-25 v311 twenty-child native swap acceptance — PASS

The preserved v311-r2 native testkit run completed successfully:
`daimos-mm-swap-progress-v311: PASS (one post-INIT 20-child pressure cycle)`.
The 256K PDP-6 GCC full-profile guest using a 012000-octal-block
(5120 block) drum backing configuration printed
`<wP0P1P2P3P4P5P6P7P8P9PAPBPCPDPEPFPGPHPIPJSRQ` and a normal
HALT. All twenty children were admitted; writer data integrity,
wait/reap and one full pressure cycle were checked by the guest.
The simulator-only timeout ceiling was 900 seconds; success must not
be read as proof that smaller swap capacity suffices or that every
64-slot simultaneous pressure scenario passes. This completes the
immediate high-concurrency swap correctness regression following the
production swap-in failure-path stack pop fix `3b41e87` and 12-/16-
child passes. No further kernel, KCORE or descriptor-policy change.
Resume the original permanent EXEC/RUN minimization/source sweep.
Further capacity/large concurrency tests remain optional backlog,
not a blocker for EXEC/RUN work.

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

## 2026-10-25 physical fault snapshot invariants v307

Reanalyzed the preserved v297 physical pre-swap-out/post-HALT trace
with versioned testkit tool `daimos-mm-swap-snapshot-invariants-v307.py`.
Of 317 common octal memory words, 30 changed: 27/64 sampled idle-stack
words and one of 83 process/MM metadata words. None of the four sampled
storage-map words changed. Both `mach_kernel_stack_base` and
`mach_kernel_sp` remained `040350` octal, and the first storage-mode
word remained `000005252520` octal. These diffs occurred across
continued execution and therefore do **not** prove stack overflow or
an illegal memory overlap. They also do not rule out unsampled memory
corruption. The known missing stack-local pop was separately repaired
in `3b41e87`; v306 revalidated 12 children. Current unresolved issue
is 16-child swap-space exhaustion, writer TERM handling, and the
higher-capacity v305 timeout. No production change in v307.

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

## 2026-10-25 physical swap-map overlap probe v292

A native 256K PDP-6 16-child failing guest was traced with SIMH
breakpoints examining the storage-state map and 28 MM descriptors.
At the unexpected-UUO aftermath the state-map pointer was physical
`041141` octal; its allocated MM extent was `[040750,041145)`,
`MM_TYPE_KERNEL_DYNAMIC=3`, owner `4` (swap metadata). The captured
neighboring extent boundaries were contiguous and did not overlap
this range. `proc_storage_runtime_v29.slots=0100` (64 slots).
The map's first word was `000005252520` octal, encoding slot 1 as
RESIDENT (0) and slot 2 as DISK (1). The earlier block-count check
matched `0320` octal; the swap-in storage-state expectation for slot 2
reached the good branch. Two failed mode checks observed **after**
the unexpected UUO do not establish the fault's original cause.
The trace covers only the descriptor range requested from SIMH, so
it does not establish a full-table global overlap invariant. It does
rule out a different visible MM extent overlapping this specific
storage-state allocation in the captured snapshot.

The next priority is to trace allocator pressure/compaction calls
nested in the first swap-in and the following swap-out, including
stack bounds and return paths; avoid attributing the UUO to the
storage-state map without new evidence. No production change.

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

## 2026-10-25 post-INIT concurrency escalation v282/v283 — UNRESOLVED

Starting from the passing 12-pressure-child v281 fixture, v282 raised
the guest's post-INIT pressure children to 20. In the 256K PDP-6 GCC
SIMH image the 17th requested pressure child (zero-based index 16)
failed admission (`pG!W`); the run eventually timed out at 360 seconds.
88 MM-insert samples showed maximum pre-insertion count 34, implying
an observed insertion upper bound 35/136. This is **not** proof of a
descriptor exhaustion error or an accepted pressure test.

v283 reduced the same test to 16 children. Instead of completing,
SIMH reported unexpected `UUO 000000777777 340000000003` and halted
at PC 000001. The transcript emitted `<w` but no normal failure or
completion marker. 72 MM-insert samples again gave maximum pre-count
34 and observed insertion upper bound 35/136. Treat this as a possible
runtime correctness fault with higher priority than further capacity
tuning. Determine the trap's origin and instruction corruption or
control-flow issue before declaring high-concurrency swap acceptance.
The v281 12-child PASS and v279 repeated-swap PASS remain valid.
No production code or 32K KCORE size change.

## 2026-10-25 post-INIT twelve-child live swap/high-water v281

The 256K PDP-6 GCC full-profile testkit workload now starts from one
INIT, activates swap and progressively launches twelve 24K-word
pressure children alongside a blocked pipe writer. A single cycle
PASSes writer restoration, pipe integrity, process cleanup and
MM insertion breakpoint sampling: 71 observations, maximum successful
post-insert descriptor occupancy 31 of 136 (105 spare). The initial
three-cycle version made progress through its second round but hit a
300-second SIMH runtime limit; it was not an acceptance pass.
This confirms post-INIT memory pressure at thirteen user processes,
not swapping with all 63 user slots occupied. The latter remains open.
No production code change or KCORE size growth.

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

## 2026-10-25 native 256K swap descriptor high-water v279

Testkit `daimos-mm-swap-highwater-v279.sh` completed the three
256K PDP-6 swap/reclaim rounds from v278 with a native SIMH
breakpoint on `mm_extent_insert`. From 101 pre-insertion count
samples, the greatest observed post-insertion occupancy was
27 of 136 descriptors (109 spare) in this particular workload.
The test passed all existing writer, pipe-integrity and process
reaping assertions. SIMH strips leading octal zeros from printed
addresses, so the trace parser explicitly normalizes addresses;
earlier unsound parser results of 1 were rejected. This does NOT
replace the 63-process workload's separate peak of 132 or prove
peak occupancy under simultaneous 63-process pressure. No
production code or KCORE size change.

## 2026-10-25 repeated 256K native swap cycles v278

The daimos-testkit native PDP-6 GCC full-profile 256K fixture
`daimos-mm-swap-cycles-v278.sh` passed three successive rounds
of blocked pipe writer plus eight sleeping pressure children.
Every round exercised the memory-pressure check, writer resume,
pipe-integrity assertions and reaping of all pressure children.
This validates repeated swap/reclaim and process-slot reuse beyond
the v277 single-cycle acceptance result. It does not yet provide
the 136-descriptor registry high-water under these cycles.
No production source change; the 32K GCC lowmem disk-root
KCORE size reference remains 10742 words, KCORE+MRES 13605.

## 2026-10-25 AUXSTORE/root mapping order fix v277

The v272 eight-pressure-child 256K native PDP-6 GCC workload now
PASSes after moving `auxstore_post_minits()` from before root mounting
to immediately after `kinit_boot()` in `system/kernel/boot/kinit.c`.
Root D6FS mounting updates the shared BLOCKSET direct-tail mapping;
publishing AUXSTORE before root mounting left its backing-store count
advertised while the root mount replaced the live direct-tail extent.
The corrected order permits all eight pressure children and restores
the blocked pipe writer with the fixture's data-integrity checks.
This is live swap-path acceptance, not comprehensive repeated swap
stress. No permanent KCORE data added. Mandatory 32K GCC lowmem
disk-root size: KCORE 10742 words, KCORE+MRES 13605 words,
PERMANENT_LAST 13652 (unchanged).

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

## 2026-10-25 256K eight-child swap-pressure diagnostic v272

A dedicated GCC full-profile 256K PDP-6 testkit guest uses a
24K-word BSS array (inside the DXR BSS limit) and attempts
eight pressure children behind a blocked pipe writer. The
first seven pressure children start and reach PROC_SLEEP;
the eighth `dsys_run` invocation (zero-based child index 7)
fails, emitting `<wp7!W`. This is a **failing diagnostic**,
not swap acceptance. The specific returned error and allocator
state have not yet been observed. Investigate process-create
admission, memory allocation and reclaim rather than raising
MM descriptor limits or declaring a swap bug prematurely.
No production source changed; KCORE measurement remains the
32K GCC lowmem disk-root configuration.

## 2026-10-25 PDP-6 256K executable BSS admission v271

The native exec loader in `system/kernel/proc/exec_load.s` masks
the DXR BSS field to 15 bits and rejects BSS exceeding `077777`
octal (32767) words. This explains v270's 56K-word BSS guest
failing before INIT startup. A separate 256K GCC full-profile
image with a 24K-word pressure array now reaches guest INIT and
prints `<ws!W`: startup succeeds, but the existing two-child
pressure test does not actually force swapping at 256K (`s`).
Testkit v271 records this as an explicitly expected negative
control, not a successful swap stress test; its static bound
check PASSes. The SIMH memory-size increment is 16K words,
while DAIMOS process/extent table policy retains 8K increments.
KCORE size reporting remains at the 32K GCC lowmem disk-root
configuration. No production code change.

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

## 2026-10-25 Native MM descriptor saturation v268

The daimos-testkit daimos-mm-pressure-v268.sh executes
production PDP-6 MM assembler insertion/deletion at the
256K-configured 136-descriptor ceiling. Starting with 132
entries (the measured v267 full-workload peak), four further
insertions reach 136; the 137th rejects with descriptor
exhaustion without mutating the count; a middle deletion
reduces occupancy to 135 and reinsertion restores 136 with
the expected shifted-table content. Native fixture PASS.
This tests allocator mechanics on a synthetic registry, not
live 63-process swap/reclaim transactions. No production
code or 32K KCORE size change. Live high-concurrency swap
pressure is the next acceptance gate.

## 2026-10-25 256K extent high-water acceptance v267

Fresh 63-process 256K GCC PDP-6 SIMH test with execution
breakpoint at the native `mm_extent_insert` entry PASS.
151 entry-point count samples showed pre-insert maximum 131.
Assembler audit verifies the only successful insertion count
increment is inside that routine; the independently observed
v266 count 132 establishes exact first-boot workload peak 132.
Capacity 136 leaves 4 free at peak. This is not acceptance of
additional allocations during high concurrency or process
swap/reclaim transitions. No production memory instrumentation
or KCORE changes; 32K GCC lowmem remains measurement reference.

## 2026-10-25 native MM insertion breakpoint trace v267

The daimos-testkit 256K full-profile GCC native PDP-6
`daimos-mm-256k-highwater-v267.sh` creates a fresh image with
63 guest processes and installs a SIMH execution breakpoint
on the mapped native `mm_extent_insert` entry point. Its
actions EXAMINE `mm_extent_count` and resume, requiring
no permanent kernel code or RAM. The first complete test
PASSed all process start/completion markers and collected
151 insertion-entry count samples. Maximum pre-insertion
count was 131 decimal; adding one gives an insertion-related
ceiling of 132, in agreement with the independent v266
snapshot of 132 allocated of 136 descriptors. A breakpoint
records an attempted insertion, not necessarily successful
insertion, and this run is not a formal global high-water
bound for all workloads or swap/reclaim sequences. The
existing 136-entry policy remains unchanged. Measure KCORE
using the 32K-word GCC low-memory disk-root profile.

## 2026-10-25 PDP-6 MM occupancy observations v266

Native 256K GCC full-profile 63-process first-boot test PASS,
using SIMH EXPECT/EXAMINE for non-production MM observations.
At process 63's start, 132 of 136 descriptors were allocated;
at its completion, 8 remained allocated. The 136 capacity
was independently read from the same guest image. These two
point samples do not establish the lifetime high-water or
adequate reserve for simultaneous swapping/allocations.
Testkit v266 observation parser explicitly supports examination
text appended to guest console marker lines. KCORE source and
the 32K GCC size reference remain unchanged. Next: test
allocation overlap / reclaim behavior and measure peak count.

## 2026-10-25 full 256K numbered process range v265

daimos-testkit `daimos-mm-256k-concurrency-v265.sh` creates
fresh GCC full-profile 256K PDP-6 images with 48 and 63
bootstrap guest processes. Both configurations PASSed:
all numbered start and completion markers appeared, with
private-state corruption reported as a separate error token.
The 64-entry process table reserves slot 0, thus all 63
usable numbered slots have now passed first-boot execution.
The marker protocol uses unambiguous hex slot tokens to
avoid collision with legitimate completions. This is not
an MM high-water, repeated fork/exit/reuse, or swap-pressure
acceptance result. No production code changed. The size
reference remains 32K GCC lowmem disk-root.

## 2026-10-24 PDP-6 256K thirty-two-process concurrency v264

Extended the fresh-image 256K GCC process-private state test
to 32 guest processes. Initial host acceptance accidentally
treated the legitimate slot-24 completion marker 'X' as a
failure marker; the fixture was corrected to reserve '@'
for private-state failure. Rebuilt and reran against new
filesystem media: PASS, all 32 start and completion markers
observed without process-private state corruption.
This does not measure maximum concurrent residency, peak
MM extents, or swap pressure. Capacity policy remains 64
table entries with slot zero reserved (63 numbered slots).
No DAIMOS production source modifications.

## 2026-10-24 PDP-6 256K sixteen-process concurrency v263

New daimos-testkit 256K full-profile GCC first-boot fixture
`daimos-mm-256k-concurrency-v263.sh` runs 16 independent
INIT images. The guest verifies process-private state across
CPU work; the host checks distinct start/completion markers.
Fresh native SIMH PASSed all 16 starts and completions.
This is stronger than v262's five-user acceptance, but is not
an extent high-water measurement or evidence of 32/63 process
operation. Process slot zero is reserved, so a 64-entry table
has at most 63 numbered slots. No production code changed.
The canonical size comparison remains 32K GCC lowmem.

## 2026-10-24 256K MM first-boot acceptance v262

The fresh-image PDP-6 full-profile 256K GCC scheduler fixture
`daimos-mm-256k-scheduler-v262.sh` PASSed timer preemption,
private process context, nice ABI and disk sleep/wakeup. The
configuration explicitly uses `BOOT_PROFILE=full`,
`BOOT_CORE_WORDS=01000000` and `SIMH_CPU_KWORDS=256`, selecting
64 process slots and 136 MM descriptors (272 descriptor words).
This validates basic 256K first-boot and five-user operation,
not simultaneous execution of 64 processes or a high-water
extent count. There is no new production code or claimed RAM
savings in v262. Continue measuring KCORE using the separate
32K-word GCC lowmem disk-root configuration.

## 2026-10-24 RAM-sized MM extent registry v261

Restored dynamic KINIT selection of MM descriptors based on
two per boot-selected process slot plus eight shared/transient
entries: 32K=24 extents, 64K=40, 128K=72, 256K=136.
KINIT reserves exactly `2 * mm_extent_capacity` words just
above the fixed KCORE symbol; the allocator/VM continue to
use the direct-address table. Native PDP-6 24/136 descriptor
boundary tests and the fresh five-process scheduler boot PASS.
The mandatory GCC 32K-profile link reports KCORE=10742 and
KCORE+MRES=13605 words; add the 48-word external table when
comparing real physical usage: 10790 and 13653 respectively,
both 15 words below the old fixed-table baseline. 256K full
workload/peak acceptance remains to be measured.

## 2026-10-24 KINIT external MM backing v260

First stage of restoring boot-sized MM extent descriptors:
`mm_extents` is a zero-sized symbol at `__kcore_low_end`,
with 64 backing words reserved and cleared by KINIT before
low free arenas are published. MRES begins after this region.
Native capacity comparisons use `mm_extent_capacity`, but it
is reset to 32 at boot; larger memory-dependent capacities
are **not yet enabled**. v249/v250 native PDP-6 extent tests
PASS; v260 map/boundary test PASS; lowmem GCC 32K-profile
image build PASS. KCORE link 024766 octal (10742 words),
but KCORE plus 64 descriptor words plus runtime-capacity word
totals 10806 words, one more than the previous 10805-word
fixed table. Full system boot and dynamic-capacity acceptance
are still pending. Do not equate KCORE label shrink with RAM
savings.

## 2026-10-24 measurement convention v259

All reported KCORE resident-size measurements must use the
32K-word GCC low-memory disk-root configuration explicitly:
`BOOT_PROFILE=lowmem ROOT=disk COMPILER=gcc BOOT_CORE_WORDS=0100000
SIMH_CPU_KWORDS=32`. This applies even while a complete 32K
runtime boot remains unsupported. Distinguish a successful
32K-profile link/size measurement from a passing 32K boot.
The v258 lowmem size report already used these 32K defaults:
KCORE 025065 octal (10805 words), KCORE+MRES 032544 octal
(13668 words), PERMANENT_LAST 032623 octal (13715 words).

## 2026-10-24 process-slot policy v258

Implemented the agreed 8K-granularity process-slot policy in
`proc_slots_for_core`: eight slots through 32K, plus two for
each completed additional 8K, capped at 64 from 256K upward.
`PROC_MIN_SLOTS` is now eight so packed/storage validators
accept valid small-system tables. Testkit
`daimos-proc-slot-policy-v258.sh` passes all 8K boundaries,
partial-bank rounding, and the upper cap. GCC lowmem disk
permanent-size link PASS: KCORE+MRES 032544 octal (13668 words),
PERMANENT_LAST 032623 octal (13715). No change to the
separate fixed 32-entry MM extent registry yet.

## 2026-10-24 native first-boot MM observation v257

Added daimos-testkit v257 first-boot PDP-6 scheduler fixture
(testkit commit 32dfb7e) with mapped MM extent-count
examination and standard scheduler acceptance checks.
Two fresh GCC-profile runs passed scheduler assertions.
The second explicitly reported six extents at one checkpoint
(octal count 000000000006 at address 024537).
This is a point sample, not a workload high-water or a safe
smaller-capacity finding. No DAIMOS production changes;
MM registry remains 32 entries/64 words.

## 2026-10-24 MM SIMH replay diagnostic correction v256

An MM replay helper erroneously chose a stale fixture via
lexicographic path sorting. Correct modification-time selection
of the latest October 24 image still produced `?RT`.
KINIT_ERR_RT identifies root mount/validation or missing
/SYSTEM/INIT, not failed scheduler preemption. The official
one-shot scheduler test passes, but reusing its executed media
does not establish MM occupancy. Future trace must attach to
first boot of a fresh isolated build. No production changes,
and the 32-entry extent limit remains unchanged.

## 2026-10-24 fresh-image MM insertion trace v255

Official five-user PDP-6 SIMH scheduler regression PASSes again
with GCC. A retained freshly linked image puts mm_extent_count
at octal 023543 and mm_extent_insert at 001167. Separate
one-shot-breakpoint manual replay of that image emitted `?RT`
before the breakpoint, indicating an unresolved difference
between manual and official simulator launch conditions.
No reliable MM peak measurement, production change, or registry
reduction. Maintain the 32-entry capacity.

## 2026-10-24 scheduler acceptance restored v254

Official `daimos-testkit test-daimos-scheduler-multiproc`
PASSed in native PDP-6 SIMH with `MAKEFLAGS=COMPILER=gcc`,
covering timer preemption, process context, nice ABI and disk
sleep/wakeup. The prior v253 ad hoc replay's `?RT` is not a
reproduced failure of the official acceptance path. Its root
cause remains unproven. Still no validated MM extent high-water;
retain all 32 descriptors. No production changes.

## 2026-10-24 unstepped scheduler validation v253

An unstepped replay of the retained GCC five-process SIMH
image booted but ended with guest output `?RT`, not the
complete scheduler markers required by the test. Thus the
v252 diagnostic run's failure is not solely an EXPECT-stepping
artifact. MM has only three unvalidated-for-peak observations
of 16 live extents; no high-water conclusion. Preserve 32
descriptors, current KCORE baseline and strict image limits.

## 2026-10-24 GCC-profile MM occupancy snapshots v252

The supported COMPILER=gcc lowmem profile cleared the
unchanged DSH 036000-octal gate and booted the isolated
five-process fixture. SIMH diagnostic examinations of
mm_extent_count (address 024537 octal) returned 020 octal
(16 decimal) at three scheduler checkpoints. The stepped
fixture subsequently failed its completion assertion, so
these are only point observations, not a validated high-water
measurement or passing scheduler test. Temporary fixture
removed, no production changes. Keep the MM registry at 32.

## 2026-10-24 runtime extent occupancy attempt v251

Non-invasive mm_extent_count observation using a versioned
five-process PDP-6 scheduler fixture did not reach SIMH:
the isolated image build stopped at the unchanged DSH EXEC size
gate, 036013 octal versus limit 036000. No runtime occupancy
or high-water measurements were obtained. The experimental
testkit script was discarded, pre-existing unrelated changes
preserved, and no DAIMOS production changes were made.
Keep the 32-entry/64-word registry until real measurements exist.

## 2026-10-24 MM extent shifting validation v250

Native daimos-testkit fixture v250 (testkit commit 580e978)
PASSes full 32-entry MM registry insertion/rejection and
middle-slot deletion/reinsertion with displaced descriptor
verification. No real workload extent high-water measurement
or capacity reduction is claimed; KCORE remains 025065 octal.

## 2026-10-24 native MM extent boundary test v249

daimos-testkit commit 11dc124 adds a native PDP-6 test based on
production MM insertion/deletion. PASS: 32 slots filled, 33rd
returns MM_ERR_DESCRIPTORS (-3), deletion enables refill. It is
not workload high-water evidence. No production code change,
and KCORE baseline remains 025065 octal (10805 decimal).

## 2026-10-24 MonitorFS lookup and classifier follow-up v231

Audited MonitorFS lookup, readdir and STAT after the successful v230
device-type sharing. An alternative character-default device classifier
saved only one post-link word (`025114` versus `025115`) but added common
character-device path work; it was rejected and reverted. A restored
GCC lowmem disk-root permanent-size build confirms KCORE=025115 octal
(10829 decimal), KCORE+MRES=13692. No production changes or new runtime
acceptance are claimed for v231; previous v230 device-view native pass
remains applicable. Continue proposal B at larger file/VFS boundaries.

## 2026-10-24 KCORE MonitorFS classification sharing v230

`mfsdev_stat` now reuses `mfsdev_io_type` already called by READDIR to
classify character, block and mount-source devices. A GCC lowmem disk-root
permanent-size link passed: KCORE `025115` octal (10829 decimal),
KCORE+MRES 13692 and PERMANENT_LAST 13739 decimal, net saving 10 words
versus v229. `daimos-testkit` native SIMH MonitorFS device view-online
test PASSed using `COMPILER=gcc` and the unchanged DSH limit. Dedicated
STAT/READDIR classification combinations remain to be checked. No extra
permanent data or provider indirection was introduced.

## 2026-10-24 KCORE A scan audit and B entry v229

Proposal A's process table scans were reviewed after the seven-word-growth
shared-cleanup experiment was rejected. Live-user, session/pgrp, trailing
FREE slot, and intrusive runnable-queue scans have different contracts;
no generic iterator or new resident state is justified. No production
change and no additional size saving in v229. Next source-level shrink
focus is the file/VFS/MonitorFS boundary (proposal B). Available compiled
object section measurements: file_runtime 1175 text; monitorfs_runtime
1565 text + 39 data; vfs 778 text + 27 data + 128 BSS (pre-fold; not
recoverable savings estimates). Native v227 process lifecycle and rollback
acceptance remains valid for the unchanged production source.

## 2026-10-24 process teardown factoring rejected v228

An experimental shared `proc_cleanup_after_vm` service for EXIT and RUN
discard was measured using the production GCC lowmem disk-root link.
KCORE grew from v226/v227 `025127` (10839 decimal) to `025136` (10846),
seven resident words; KCORE+MRES grew 13702 -> 13709. The change was
reverted rather than trading RAM for a source-level abstraction. No native
test was run for the rejected prototype. The supported v227 implementation
and its native lifecycle and rollback passes remain the accepted baseline.

## 2026-10-24 native GCC lifecycle acceptance v227

The recurring native process-fixture blocker was the default KCC-produced
DSH exceeding production `036000`, not a process-runtime failure. Using
the already supported `COMPILER=gcc` policy through `MAKEFLAGS=COMPILER=gcc`
without relaxing the gate, daimos-testkit native SIMH passes both:
`test-daimos-run-late-rollback` (16 post-VM descriptor-map failures, stable
process accounting and recovery) and `test-daimos-process-lifecycle`
(RUN, GETPID, WAIT/nohang, zombie reap, orphan adoption, explicit fd maps).
These exercise the new v226 discard implementation. No production source
changed at v227, no new KCORE saving is claimed; RT-owner fault injection
and full VM teardown fault coverage remain open.

## 2026-10-24 KCORE discard assembler prototype v226

Replaced KCC-generated `proc_slot_discard` in the PDP-6 KCORE build with
an ABI-compatible assembler implementation; the C fallback is guarded by
`PROC_SLOT_DISCARD_PDP6_ASM`. No change to normal EXIT publication, MM
ownership, u-area lifetime or on-error transition clearing was intended.
Lowmem disk-root permanent-size link PASS: KCORE 025127 octal (10839
decimal), KCORE+MRES 13702 and PERMANENT_LAST 13749 decimal; two words
saved against v223. No extra permanent data. Targeted native RUN late
rollback fixture could not start because DSH 036013 exceeds the unchanged
036000 EXEC limit. This implementation remains **native-unaccepted** until
rollback and process lifecycle tests pass on a valid fixture. See KCORE.MD.

## 2026-10-24 KCORE A teardown overlap audit v225

Identified a concrete source-level consolidation candidate: production
KCC-generated `proc_slot_discard` and handwritten `proc_finish_slot` both
perform RT ownership release, VM destruction, TTY/u-area release, run-queue
removal and slot trimming. They differ in FREE handling, session/HUP
semantics, child adoption and FREE versus ZOMB publication; their distinct
rollback/ownership contracts must survive any factoring. The inspected
generated discard body is about 50 instructions gross, not a predicted
post-fold saving. No production source changed and no new native run passed.
KCORE.MD records the gated assembler/common-teardown experiment.

## 2026-10-24 KCORE proposal A cleanup semantics v225

Compared normal/fatal `proc_finish_slot`, failed-creation
`proc_slot_discard` and remote fatal-event file closure. Their shared
VM/u-area/runq calls have different transaction requirements, so no
unproven shared cleanup wrapper was introduced. Existing daimos-testkit
host `proc-abort-transition-v40` PASSed with `CPATH` set to DAIMOS's
`tools/host` header directory, proving transition-flag hold, destroy
failure rollback, retry and slot reuse in the C path. PDP-6 native
assembler cleanup acceptance remains outstanding; there is no new size
measurement or production code change in this checkpoint.

## 2026-10-24 KCORE lifecycle/RT policy audit v224

Compared `proc_finish_slot` in PDP-6 assembler and the C fallback, plus
`proc_slot_discard` and fatal-event termination. Early RT-owner release
precedes VM teardown in both implementations; code review alone does not
establish a rollback bug. No production semantics or source were changed.
Attempted native `test-daimos-process-lifecycle` but the fixture build
stopped before execution because DSH `036013` exceeds EXEC gate `036000`.
Keep the production size gate unchanged and native acceptance open. See
KCORE.MD for the next source-level consolidation candidates.

## 2026-10-24 KCORE process-finish RT ownership v223

`proc_finish_slot` now uses `CAMN/SETZM` rather than
`MOVE/CAME/JRST/SETZM` for conditional RT owner cleanup. The GCC lowmem
disk-root permanent-size link passed: KCORE=025131 octal (10841 decimal),
KCORE+MRES=13704 decimal and PERMANENT_LAST=13751 decimal. This saves two
resident words relative to v222. No new permanent data or process ABI
change. Runtime termination, RT ownership and native hierarchy tests have
not been reaccepted at v223; v222's strict DSH fixture size blocker
remains relevant. Process lifecycle/TTY/teardown architectural audit
continues without collapsing distinct rollback and ownership contracts.

## 2026-10-24 KCORE process-adoption branch reduction v222

In `proc_adopt_children_pdp6` the INIT special case now uses one `CAIN`
branch, removing one unconditional branch word. A lowmem disk-root
`make permanent-size` build completed with KCORE `025133` octal (10843
decimal), a one-word reduction relative to the recorded `025134` profile;
KCORE+MRES = 13706 decimal words for this build. A targeted native
daimos-testkit process-hierarchy run was attempted but its fixture failed
before execution: DSH image `036013` exceeds production gate `036000`.
No hierarchy runtime pass is claimed and the gate was not changed.

## 2026-10-24 KCORE architectural reduction A inventory v221

KCORE.MD now records the ordered architectural shrinking candidates A-F.
Additional JRST folding is explicitly last-effort work, after structural
source reductions, still subject to the existing >20-net-word acceptance.
Proposal A has begun with a source/map inventory of proc_pdp6.s: it mixes
scheduler/PI/context-switch code with wait/reap, process-control, event,
session, and TTY handlers. Available pre-fold object sections are 2164
text + 3 data + 3 BSS words, not a fresh full-link measurement and not all
scheduler. No production code or test assertion changed; no shrink gain is
claimed. Next is an accurate external/indirect dependency graph and one
measured internal consolidation, preserving native process/TTY semantics.

## 2026-10-24 KCORE source/TODO reconciliation v220

Audited KCORE.MD's active backlog against the v219 DAIMOS tree. Verified
the three-word process ABI and boot slot tiers in proc/proc.h/proc_boot.c,
32 static MM extents in mm/mm.h/mm_pdp6.s, and eight-word pipe storage in
ipc/pipe.h. Confirmed EXEC/RUN's resident assembly implementations and that
vm_space_startup still writes to user VM even with empty argument lists.
MEMFS private XIP admission op 027 and VM release op 026 are implemented,
but production EXEC/RUN does not use XIP admission. The restricted raw XIP
linker producer is experimental; native fixtures do not constitute a full
scheduled-process XIP acceptance. Updated KCORE.MD and stale TODO.MD
checkboxes accordingly. No production source or ABI changes.

The most recent recorded GCC lowmem KCORE exclusive endpoint is 025134
octal (10844 words, v213); strict below-020000 would require at least
2653 words of reduction at that profile. The separate full EXEC/RUN profile
reported 025657 octal (11183 words, v205). Neither endpoint was remeasured
at v219 during this documentation-only audit. The strict 32K acceptance
remains deferred and unpassed. The outstanding packed-process, D6 codec,
folding, and size-profiling tasks remain open, not silently promoted.

## 2026-10-23 movable MEMFS XIP admission gateway v219

The existing MEMFS provider register dispatch now accepts private op 027:
AC1=vnode, AC2=&base, AC3=&entry. It calls `memfs_xip_admit` wholly inside
MRES, which validates the raw PURE header and owns pin failure/rollback.
This is the symmetric counterpart of VM XIP release op 026. No new kernel
entry vector, process state or permanent data is required. Native PDP-6
SIMH and AAP MEMFS/XIP lifecycle tests call the real dispatch and PASS.
Normal EXEC/RUN has not yet been changed to request this service, so raw
XIP remains unlaunchable by ordinary users. Privileged, argument-free
admission and no-write startup are still needed before runtime enablement.

## 2026-10-23 experimental linker XIP producer v218

Host `dlink --xip --pure -b 020` now creates a distinct 04000-word RAW
MEMFS image using the existing text-link layout and relocation engine.
Word 0 holds XIP magic, word 1 holds `PURE,,020`, 2..017 are zero, text
begins at logical 020 and the remainder is zero-padded. The extra padding
forces an aligned dedicated MEMFS extent under its existing allocator.
For this experimental generator, accepted images have **no DATA or BSS**
and instruction text is restricted to unindexed, non-indirect `MOVEI`
instructions followed by one `SYS_EXIT` (monitor UUO 040). All other
instructions/syscalls are rejected; this is stricter than the normal DXR
PURE heuristic, which alone cannot prove absence of user stores. Normal
DXR/PURE output is unchanged. Negative/format host tests live in
daimos-testkit. Production EXEC/RUN **does not accept raw XIP yet**:
`vm_space_startup` still writes into the user image. No KCORE change.

## 2026-10-23 reuse DXR PURE for raw MEMFS XIP admission v217

Simplified the experimental XIP image contract to reuse the existing DXR
`PURE` flag (octal 0200000) rather than invent a new immutable-image bit.
The raw image remains a separately laid-out format: word 0 is XIP magic,
and word 1 is `PURE_FLAG,,entry` (LH flag, RH logical entry). MEMFS admission
requires exactly this flag in addition to the existing execute/no-write
file modes and physical alignment. A missing PURE flag must reject without
retaining a pin. This is software metadata only, NOT hardware protection.
Ordinary DXR PURE images remain unchanged and are not automatically XIP.
No EXEC/RUN/VM/KCORE change was made. A privileged loader path with genuine
user-mode no-write constraints remains an open integration task.

## 2026-10-23 MEMFS non-writing XIP user lifecycle v216

daimos-testkit adds `target-memfs-xip-user-lifecycle-v216`. A distinct
non-writable/executable MEMFS raw image is admitted and pinned, then entered
at logical `020` under the PDP-6 APR. The user image executes only `MOVEI`
and a monitor UUO (`040`), without stack or other user-memory stores.
The hardware UUO invokes an executive-only handler, which checks both user
instructions are unchanged and calls the unchanged production
`vm_space_destroy` body. The test requires VM state to be cleared, MM pin
released, and the original MEMFS extent to become reclaimable.

Native SIMH and AAP PDP-6 both PASS. The testkit uses scheduler/storage
stubs and is **not** a fully scheduled process or production EXEC/RUN.
For SIMH, the dedicated runner omits p10run's unconditional `break 20`,
which otherwise intercepts this legitimate user entry before relocation.
This changes no DAIMOS production code or per-process RAM; the last measured
permanent KCORE remains 025134 octal (10844 words).

## 2026-10-23 production VM destroy native XIP rollback v215

daimos-testkit now extracts the unchanged `vm_space_destroy` body from
`system/kernel/proc/vm_pdp6.s` and links it into the native MEMFS fixture.
The fixture uses a real two-half VM descriptor and injects `MM_ERR_BUSY` on
the final XIP unpin. First destruction fails with VM state unchanged and
the pinned MEMFS extent still owned; retry succeeds and clears VM state.
Both SIMH and AAP PDP-6 PASS. Scheduler/storage mode expectations and swap
detach are focused stubs, not a full native process exit. There were no
DAIMOS production changes or KCORE growth in v215. Real process EXEC/exit
and non-writing startup remain open.

## 2026-10-23 XIP release failure injection v214

The daimos-testkit native MEMFS regression now fault-injects `MM_ERR_BUSY`
from `mm_unpin` during private MEMFS XIP release op 026. It verifies the
dispatch reports failure (-1), leaves the MEMFS pin/reference intact, and
permits normal successful release after the injected failure is removed.
Both SIMH and AAP PDP-6 tests PASS. No DAIMOS production source or KCORE
change was necessary. A full process-VM exit/EXEC rollback test remains open.

## 2026-10-23 MEMFS XIP VM teardown dispatch v213

`vm_space_destroy` now probes the existing MEMFS MRES register gateway with
private operation 026, passing the resident physical VM base. The MEMFS
provider uses the existing XIP base provenance and reverse-unpin functions:
result 1 means released; result 0 means ordinary/non-owned VM; -1 means
provider unavailable or release failure. In the latter two cases only the
normal `mm_free(MM_TYPE_PROCESS, owner)` is tried, which cannot free a
MEMFS-owned dynamic extent. No new per-process storage or provider vector.
An MRES-dispatch native test checks unknown, valid, and repeated releases on
SIMH and PDP6-AAP. GCC lowmem image PASS; linked KCORE is now 025134
octal (10844 words), a 9-word increase over v212's 025123.

The existing swap-out path checks `mm_is_pinned`; `vm_space_brk_current`
requires MM_TYPE_PROCESS for resizing. The separately trusted non-writing
startup and true process-XIP launch remain disabled; this commit does not
make arbitrary DXR images safe to map from MEMFS.

## 2026-10-23 MEMFS XIP base provenance v212

MEMFS MRES now exposes `memfs_xip_owns_base(base)` using its existing four
pinned-node reference words. It recognizes exact physical bases only while
an XIP reference is live, requires no extra per-process or persistent MEMFS
data, and becomes false after final unpin. The corresponding native tests
PASS on SIMH and PDP6-AAP. This supplies a prospective VM provenance query,
but **does not yet connect VM teardown or swap to MEMFS MRES**. EXEC/RUN
and resident VM remain unchanged and production XIP launches remain disabled.

## 2026-10-23 MEMFS XIP reverse ownership release v211

Added `memfs_xip_unpin_base(base)` inside MEMFS MRES. It finds the exact
MEMFS-owned physical base using the existing four-entry pinned-node table,
decrements a shared file reference, and releases the underlying MM pin only
on the last reference. Unknown bases return failure without changing pins.
Native SIMH and PDP6-AAP MEMFS target tests PASS. No new persistent MEMFS
reference-table words, no KCORE changes, and no production EXEC/RUN XIP path.
This is the primitive needed before the VM teardown path can distinguish
MEMFS-backed mappings from ordinary MM_TYPE_PROCESS allocations.

## 2026-10-23 MEMFS XIP admission hardening v210

`memfs_xip_admit` now checks mode bits before taking an image pin: at least
one execute bit must be present and all write bits must be absent. A valid
magic/header without safe file modes is rejected without pinning. The native
testkit confirms that both writable and non-executable modes are rejected;
mode 0555 is admitted. SIMH MEMFS and PDP6-AAP MEMFS/user-APR tests PASS.
This is only a file-admission restriction, not hardware write protection:
the PDP-6 can still store into admitted MEMFS physical memory in user mode.
No VM ownership, swap, startup or EXEC/RUN XIP path has been enabled.

## 2026-10-23 MEMFS user-mode APR XIP hardware test v209

daimos-testkit now provides `target-memfs-xip-user-apr-v209`. The test first
admits and pins a separately marked raw MEMFS XIP file; it then programs the
PDP-6 APR relocation base and enters user mode at logical 020 directly into
that file's words. The test user code intentionally executes `MOVEM 2,030`,
and AAP confirms the marker at the corresponding MEMFS physical word and
the correct pinned file base. **AAP PASS.** The native PDP-6 image is built
using p10run, but a completed SIMH user-mode run is not claimed.

This is intentionally a test-only *writable* example. It demonstrates that
the PDP-6 APR cannot prevent writes to mapped MEMFS backing; it does not
prove that ordinary user applications are safe to share or that process
teardown, swapping, and non-writing startup are integrated. EXEC/RUN, KCORE,
and production MEMFS source are unchanged by v209.

## 2026-10-23 MEMFS XIP native execute probe v208

The daimos-testkit MEMFS fixture now uses executive-mode PDP-6 `XCT` to
execute a single instruction at `admitted MEMFS physical base + entry` while
holding the file's pin. Both SIMH and PDP6-AAP PASS and observe the expected
AC result. This verifies native instruction fetch directly from MEMFS-owned
backing without a copied process extent. It does **not** exercise user APR
relocation, startup, process scheduling, swapping, or EXEC/RUN integration.
No production KCORE or EXEC/RUN code changed for v208.

## 2026-10-23 XIP MRES image admission v207

Implemented `memfs_xip_admit` in the separately loadable MEMFS MRES. It
recognizes an explicit raw-image header and entry address, pins backing while
validating, and unpins on rejection; ordinary DXR is never auto-admitted.
Native MEMFS target tests PASS on SIMH and AAP PDP-6. No XIP process launch
is implemented; `vm_space_startup` and MM_TYPE_PROCESS teardown still make
transparent substitution unsafe. No EXEC/RUN or KCORE change in this patch.

## 2026-10-23 post-EXEC-optimization XIP integration audit v206

The optimized `exec_load_process` calls `vm_space_create` followed by
`vm_space_load_file`, `proc_swap_attach`, and the RUN/EXEC commit path calls
`vm_space_startup`. That routine unconditionally writes argc/envc metadata
through the translated VM base (`movem 3,(11)`); it is not safe to replace
the process's MM_TYPE_PROCESS extent with immutable/shared MEMFS backing.
`vm_space_destroy` still unconditionally frees a resident base under
MM_TYPE_PROCESS ownership, and normal swapping assumes resident process
storage. Therefore **no XIP executable is admitted** yet, despite the
completed MEMFS alignment/pinning support. Do not add an unconditional
MEMFS-base substitution to `vm_space_load_file`.

Post-optimization GCC lowmem disk `permanent-size` check PASS:
KCORE 025123 octal (10835 words); KCORE+MRES 032602 octal (13698 words);
PERMANENT_LAST 032661 octal (13745 words). No XIP loader code was added.

The supported next implementation is a privilege-gated, separately marked
non-writing executable ABI, a mapping/teardown provenance mechanism and an
XIP-specific startup path that does not write into backing storage, followed
by testkit native execution on SIMH and AAP. The public EXEC/RUN syscall ABI
need not change, but VM/startup/swap behavior must change before launch.

## 2026-10-23 EXEC UNCOMPRESSED STAGED-VM OOM REGRESSION V205

The outstanding v205 testkit pressure experiment now PASSes natively
on the 256K PDP-6. The test builds an uncompressed, valid DXR of
0100000 octal image words plus 077000 BSS words. The current PID 1
image first grows to 0600000 words, so staging both resident VMs
cannot fit in physical core. EXEC succeeds through the existing
destructive low-memory retry, and the replacement checks PID 1 and
continues execution. The initial failing run before the v201 resident
36-bit ENOMEM comparison correction returned `<H!x`; a corrected
kernel had already passed an early v205 run. A later test-only failure
was caused by expecting `dxrcheck` to print a leading zero in the
octal image word count; the output is `image=100000 bss=077000`.
The v205 fixture now accepts the tool's actual octal formatting while
checking both image and BSS size fields.

Fresh native proof: `$TMPDIR/daimos-exec-create-oom-v205-accepted.log`:
`PASS (staged VM cannot fit physically; low-memory EXEC replaced PID 1)`.
`__kcore_low_end=025657` octal. No additional DAIMOS kernel change,
permanent state, or stack growth. The 32K gate stays deferred, and
the independent full-profile permanent-memory acceptance limit remains
open.

## 2026-10-23 EXEC LOW-MEMORY RETRY CORRECTION V201

The native 256K PDP-6 v201 EXEC pressure regression identified a real
full-word error-comparison bug in resident `exec_replace_current`:
`CAIE 1,EXEC_LOAD_NOMEM` compares against an 18-bit immediate and
cannot recognize the 36-bit `-2` returned by `exec_load_process` when
staged VM allocation fails.  Replace it with `CAME 1,[-2]` so the
existing destructive low-memory EXEC retry is actually invoked.

The previously failing compressed/padded large executable now replaces
PID 1 with physical pressure forcing the staged allocation to fail;
native v201 PASS (`$TMPDIR/daimos-exec-pressure-v201-fullcompare-test.log`).
The same large executable also PASSes without pressure
(`$TMPDIR/daimos-exec-pressure-control-v201-afterfix.log`).  The
independent v200 eight-case PDP-6 instruction-level loader/commit
rollback fixture PASSes after this correction
(`$TMPDIR/daimos-exec-commit-rollback-v201-retest.log`).
No persistent kernel data or added stack words. A fresh full-profile
v201 link map confirms `__kcore_low_end=025657` octal, unchanged from
the pre-fix full-profile KCC/p10fold baseline. This is *not* an
acceptance of the separate 16K permanent-memory, default DSH image
size or 15-second runtime gates. The 32K-specific gate remains
deferred by explicit direction.

## 2026-10-23 MEMFS XIP v204: aligned allocation failure contract

The native testkit MEMFS fixture now injects `MM_ERR_FRAGMENTED` for an
aligned large allocation and confirms there is no fallback to an unaligned
extent, published data pointer, logical quota change, or stale XIP pin.
The subsequent aligned retry succeeds. SIMH and AAP PDP-6 both PASS.
This is a deterministic failure-injection test, not a realistic MM stress
measurement. The production XIP process launch remains disabled.

## 2026-10-23 MEMFS XIP aligned backing v203

Dedicated large MEMFS allocations use 02000-word alignment through MM.
Both SIMH and AAP MEMFS pinning regressions PASS. No XIP process launch
is enabled; executable admission and VM provenance remain open.

## 2026-10-23 MEMFS XIP protection milestone v202

An explicit MEMFS XIP reservation is now supported in the MEMFS MRES, without
enabling any XIP launch path. Four packed (slot+1),,reference-count words
represent the maximum four simultaneously dedicated MEMFS chunks; MM pinning
guards the underlying physical extents. The new API only pins regular files
on large dedicated 02000-aligned resident extents. Pinned file writes,
resizes/truncations, chunk evictions and unmount teardown are denied. Final
unpin releases the MM reference. This avoids additional permanent KCORE data;
MRES size effect has not been measured in a comparable post-fold baseline.
GCC lowmem image build and focused SIMH MEMFS/FIFO native tests PASS.
The identical MEMFS fixture PASSes on AAP PDP-6 as well. This is not
general XIP, and no native XIP process launch has been claimed.

## 2026-10-23 EXEC STAGED COMMIT FAILURE INJECTION V200

The new daimos-testkit `test-daimos-exec-commit-rollback` PASSes eight
instruction-level scenarios against the **actual resident**
`exec_replace_current` implementation extracted from
`system/kernel/proc/exec_lowmem_pdp6.s`; only its external
`exec_load_process`, `vm_space_startup` and `vm_space_destroy` call
boundaries are replaced by deterministic test doubles. Covered cases:
failed image loading, startup creation failure, failed destruction of
the old VM, successful PURE/backing commit with either fifth mount bit,
rollback of a newly claimed RT owner after startup failure, preservation
of an existing RT owner after startup failure, and releasing an existing
owner when a non-RT image commits. The old-VM-destroy failure also
asserts restoration of the user mapping hold. Each failed transaction
must restore the previous backing record/credential bits and return -1.

Evidence: `$TMPDIR/daimos-exec-commit-rollback-v200-final.log`;
testkit commit `de17627`. This is a PDP-6 instruction-level emulator
test of the production transaction with injected call results, **not**
a native full-kernel failure or a real failed `mm_free` invocation.
Successful RT-to-RT replacement is not in the eight-case matrix; its
experimental ninth scenario remained unaccepted and was excluded from
the committed regression. Native v191-v193 owner and decode failure
tests remain the independent real-kernel checks. The 32K gate is
deferred. No kernel code or resident RAM change was made; KCORE
exclusive end remains `025657` octal.

## 2026-10-23 NATIVE RUN CHILD-VM CREATION OUT-OF-MEMORY V199

The new daimos-testkit `test-daimos-run-create-oom` PASSed on a fresh
256K PDP-6 image. The fixture first verifies that a large, valid child
DXR can be RUN and reaped normally. The boot INIT/parent remains the
known-working v197 image; `/SYSTEM/EXEC/TSFSPROBE` is replaced only
in this test image by a separate executable with 077000 octal BSS
words (below the DXR loader's 077777 BSS limit). The child's complete
text/BSS/stack requires at least 0100000 physical words.

The parent then grows its break via bounded 02000-quantum search until
the remaining 256K core cannot accommodate the large child, checking
that the parent process-word allocation plus minimum child requirement
exceeds all installed core. Four otherwise VALID RUN V2 requests for
the already-proven child return -1. After each rejection, the parent's
break and writable sentinel, its live process slots and aggregate
process-word accounting remain unchanged. Shrinking the parent back to
the original break restores the original process accounting, and the
EXACT SAME child RUN/WAIT succeeds again. The fixture then also passes
v197's 16 late descriptor-map rollback failures with three blocked
children, normal reap and final valid RUN/WAIT recovery.

This specifically tests insufficient physical core at RUN's
`exec_load_process -> vm_space_create` stage, without adding kernel
fault switches or reducing memory limits. Early fixture trials failed
for test reasons: an oversized parent image stopped during INIT boot,
and a 0100000-octal BSS child exceeded the documented DXR 077777 limit.
The corrected separate-child fixture PASSed. Evidence:
`$TMPDIR/daimos-run-create-oom-v199-native-c.log`.

All v197/v198/v199 test sources compile with PDP-6 GCC/C99, and runner
shell syntax/diff checks pass. No DAIMOS code, kernel stack, KCORE or
MRES changes; linked KCORE end stays `025657` octal. A forced EXEC
replacement allocation failure and late-commit fault injection are
still separate open scenarios. Strict 32K remains deferred by request.

## 2026-10-23 PDP-6 VM ALLOCATION-FAILURE AND RUN RECOVERY V198

The versioned daimos-testkit v198 regression extends the v197 native
RUN pressure fixture, keeping three children alive while issuing four
valid 18-bit `BRK` requests for `0776000` words each. This is below
the 18-bit logical address ceiling, but the requested resident process
extent cannot fit alongside the kernel and running processes in 256K
physical memory. All four requests returned `-1` through the actual
VM growth/MM allocation failure path rather than a malformed-address
check. After each failure the original process break, own logical VM
word count, existing sentinel data and occupied slot count remained
intact. Resident process-word accounting never increased; it may
decrease when the reclaim path swaps a sleeping process. The fixture
then completed the existing 16 late RUN descriptor-mapping failures,
reaped the three children and launched/reaped one valid recovery child.

The native fresh-image PDP-6 test `test-daimos-run-vm-oom` PASSed;
evidence: `$TMPDIR/daimos-run-vm-oom-v198-native.log`. The baseline
v197 C99 fixture still target-compiles without the v198 extension.
No DAIMOS kernel changes or permanent RAM additions were needed;
post-p10fold `__kcore_low_end=025657` octal. This does *not* yet
force `vm_space_create` to return ENOMEM from an actual RUN or EXEC
loader, nor does it individually audit every transient MM descriptor.
The 32K gate stays deferred; strict full-profile residency and
production image-size/timing requirements are unchanged.

## 2026-10-23 RUN LATE-FDMAP ROLLBACK UNDER LIVE PRESSURE V197

The new native PDP-6 testkit `test-daimos-run-late-rollback` PASSed on
a fresh normal 256K image. While three children remained alive and
blocked on a pipe, sixteen RUN V2 requests passed launch validation,
executable loading, child VM creation, startup construction and u-area
allocation, then failed at the deliberately unavailable parent FD 15
in the final explicit descriptor map. All returned `-1`; each subsequent
`SYS_MEMINFO` sample retained the same process slot count and process
extent accounting as before the failures. After closing the pipe writer,
all three children exited and were reaped; process accounting returned
to the initial baseline. A further valid RUN and WAIT also succeeded.
This tests a late child-creation rollback route and recovery, not a
deliberately exhausted MM allocator or a complete dynamic-MM leak audit.

Testkit fixture: `tests/validation/daimos-run-late-rollback-v197.c`;
runner: `tests/validation/daimos-run-late-rollback-v197.sh`.
Evidence: `$TMPDIR/daimos-run-late-rollback-v197-native-b.log`.
The first diagnostic run failed on an overly strict initial
`process_slots_used == 1` fixture assumption; the corrected fixture
accepts the runtime's actual nonzero baseline and the fresh native
rerun PASSed. DAIMOS source and size are unchanged:
`__kcore_low_end=025657` octal. The 32K gate stays deferred.

## 2026-10-23 RUN PROCESS LIFECYCLE AND ID REUSE RETEST V196

Two existing daimos-testkit native PDP-6 tests PASSed on fresh 256K
images, independent of the deferred 32K boot/image constraints:

* `test-daimos-process-lifecycle`: RUN, GETPID, WAIT/nohang,
  zombie reaping, orphan adoption and explicit descriptor mapping.
* `test-daimos-process-id-lifetime`: process-group, session and domain
  IDs remain protected while referenced after the creator slot is reused.

Versioned logs: `$TMPDIR/daimos-run-lifecycle-v196-native.log` and
`$TMPDIR/daimos-run-id-lifetime-v196-native.log`. DSH functional-only
`DSH_IMAGE_LIMIT=037000` was exported without changing the production
default. No kernel modifications were required. Linked resident KCORE
exclusive end remains `025657` octal. The 32K-specific failures remain
deferred by request as documented separately in v196, not resolved.

## 2026-10-23 EXEC/RUN SCOPE: 32K GATE DEFERRED V196

By explicit project direction, pause **all strict 32K-specific sizing,
configuration-error and native boot acceptance work** while DSH remains
too large for that profile. Do not treat a 32K boot, LOGIN/DSH EXEC,
KINIT reserve or 8K KCORE gate failure as a current EXEC/RUN blocker;
do not relax the architectural limits or consume time engineering
32K-only workarounds. Continue checking resident RAM growth in normal
builds, and continue EXEC/RUN correctness on the 256K PDP-6 profile.

The last exploratory GCC lowmem comparisons (not acceptance) showed
ROOT=auto KINIT end `102552`, above the `076000` stack-safe bound by
`04552` octal (2410 words). ROOT=disk/BOOT=dsk built an image with
KINIT end `074435`, but its KCORE end `025203` still exceeded the
separate `020000` strict size target, and the native disk-root login
reported `LOGIN: EXEC FAILED` while boot, INIT and LOGIN ran. These
observations are *deferred*, not fixed or accepted. Logs:
`$TMPDIR/daimos-lowmem-v195-audit-build.log`,
`$TMPDIR/daimos-lowmem-v195-disk-build.log`, and
`$TMPDIR/daimos-lowmem-v195-disk-native.log`.

The full-profile KCORE/MRES permanent-memory limit is a separate
project-wide size issue. Do not mistake the lowmem deferral for a
claim that the full-profile strict memory gate has passed. The
non-32K EXEC/RUN functional regression program continues below.

## 2026-10-23 RT REQUIRED CHILD CROSS-REGRESSION V194

Following the v193 post-VM EXEC compressed decode failure checks,
the existing `make test-daimos-rt-required` testkit target PASSed with
a fresh native PDP-6 image. It covers compressed RT_REQUIRED child
admission, competing-owner rejection, and eventual release/reacquire.
Evidence: `$TMPDIR/daimos-rt-required-v193-regression.log`.
This verifies the RUN-created child path separately from the v193
same-process EXEC RT rollback tests. Kernel source, KCORE exclusive
end (`025657` octal), and full residency were unchanged. The image
was built with the existing functional `DSH_IMAGE_LIMIT=037000`; the
strict permanent-memory, executable-size and timing gates remain open.

## 2026-10-23 EXEC POST-VM-CREATION RT ROLLBACK V193

The native PDP-6 testkit real-time-owner fixture now additionally
constructs a validly framed compressed DXR2 executable with the
RT_REQUIRED bit, one output image word, a one-word D6LZ payload, and
one relocation word. Its zero control word requires an absent literal,
causing `d6lz36_decode_vfs` to fail after `vm_space_create` on the
normal 256K simulation profile. The test checks both the release of a
newly claimed RT reservation and preservation of an existing owner.
Both checks PASS; the earlier header-failure rollback, RT-required
successful EXEC, and subsequent ordinary EXEC release also PASS.
The stage markers are `<abcdRN` followed by HALT. Testkit commit
`dcaca8c`; native log `$TMPDIR/daimos-exec-rt-postvm-v193-native.log`.

No kernel changes were required after the v191 scratch-overlap fix.
The linked KCORE exclusive end remains `025657` octal. This is
functional-only validation (`DAIMOS_EXEC_OBSERVE_SECONDS=35`,
`DSH_IMAGE_LIMIT=037000`); strict 15-second and `036000` image gates,
native 32K acceptance, and full 16K permanent-memory accounting remain
open. Fault injection after successful startup/VM commit is not part
of this test.

## 2026-10-23 EXEC RT FIX FOLLOW-UP NEGATIVE REGRESSIONS V192

The v191 RT-owner scratch-lifetime correction also passed the native
v190 malformed-launch/ordinary-EXEC fixture when rebuilt against DAIMOS
commit `e2248c0`. All 28 malformed RUN and EXEC requests returned the
expected error; the original process then performed normal EXEC and
preserved its expected state. Evidence:
`$TMPDIR/daimos-exec-v192-malformed-retest.log`.

The tested RT-owner regression separately passed all four transaction
paths and subsequent re-acquisition in v191. No code, stack, KCORE or
MRES growth followed the defect correction (`__kcore_low_end=025657`
octal). Functional-only observation at 35 seconds and the `037000`
DSH cap were used. The strict 15-second timing, `036000` DSH cap,
16K fixed memory, and further post-VM-creation failure injection remain
open; none of those gates has been weakened.

## 2026-10-23 EXEC RT OWNER FLAG STACK OVERLAP FIX V191

Native testkit v191 discovered and reproduced an EXEC loader RT-owner
rollback defect: `exec_load_process` stored its new-RT-claim flag in
`-003(17)`, which is also the seventh/last word (`mtime`) of the
`struct vfs_stat` output at `-011(17)` through `-003(17)`.
Consequently, after a failed RT_REQUIRED EXEC, a process that owned the
reservation before EXEC could lose it despite never claiming a new one.
The v191 PDP-6 fixture verified RT ownership immediately before EXEC
and reproduced the erroneous release after a two-word valid-header,
incomplete-payload DXR failure.

`exec_load.s` now reuses the vnode output scratch at `-012(17)` for the
RT-claim flag **after** copying that vnode to AC12. The early null/path
lookup failures return directly without testing a not-yet-initialized
flag. Seven-word `vfs_stat` output no longer aliases the flag. No stack
growth or persistent allocation is required. KCC/p10fold link PASS:
`__kcore_low_end` remains `025657` octal; full KCORE 11135 words and
KCORE+MRES 18670 words are unchanged. Strict 16K gate still FAILs.

Testkit `test-daimos-exec-rt-owner` native fresh-image PDP-6 PASS:
new RT claim released on failing EXEC; existing owner preserved on a
failing EXEC; RT_REQUIRED successful EXEC acquires ownership; ordinary
replacement releases ownership; subsequent acquisition/release works.
Evidence: `$TMPDIR/daimos-exec-rt-owner-v191-fixed-native.log`.
This uses diagnostic `DAIMOS_EXEC_OBSERVE_SECONDS=35` and functional
`DSH_IMAGE_LIMIT=037000`; default timing and image-size gates remain
unmodified. Additional failure injection after VM creation and native
32K acceptance are still open.

## 2026-10-23 EXEC/RUN MALFORMED LAUNCH REJECTION V190

The native PDP-6 EXEC replacement test in daimos-testkit commit
`9bc72d6` now runs 28 explicit malformed EXEC V1 and RUN V2 launch
probes before its successful replacement path. All 28 invalid calls
returned `-1`, the original process stayed alive, and the subsequent
EXEC replacement passed with preserved PID, credentials, cwd, session,
process group, TTY and descriptors. Cases include incorrect header
version/word count, nonzero upper halves and excessive scalar counts,
invalid counted-SIXBIT path/argv lengths, truncated records and
inconsistent block endings. The RUN probes never submit a valid child
request, avoiding incidental process creation.

The GCC cross-compiled test source passed; the fresh-image PDP-6 native
runner also PASSed. Evidence:
`$TMPDIR/daimos-exec-malformed-v190-native.log`. This is a functional
diagnostic with `DAIMOS_EXEC_OBSERVE_SECONDS=35` and
`DSH_IMAGE_LIMIT=037000`, not strict 15-second/`036000` acceptance.
Dedicated RT owner admission/release/rollback and native 32K testing
remain open. No DAIMOS kernel source or resident-memory change was
needed; linked KCORE exclusive endpoint remains `025657` octal.

## 2026-10-23 EXEC V188 NATIVE REGRESSIONS V189

Both supported fresh-image PDP-6 EXEC testkit targets PASS on the
v188 source revision `6b9ce2a`: current-image replacement preserving
PID, credentials, cwd, session, process group, TTY and descriptors;
and session-leader child EXEC with successful parent reap. Logs:
`$TMPDIR/daimos-exec-v188-replace.log` and
`$TMPDIR/daimos-exec-v188-child.log`.

The same explicit diagnostic settings were used as v186:
`DAIMOS_EXEC_OBSERVE_SECONDS=35`, `DSH_IMAGE_LIMIT=037000`.
This validates ordinary EXEC and RUN/child-EXEC flows after the v188
record-start comparison and low-memory RT owner check. It is not
acceptance of the strict 15-second deadline or `036000` image cap;
malformed argument blocks and real-time-owner rollback remain untested.
Resident KCORE exclusive end stays `025657` octal; permanent-size
figures remain as recorded in v188.

## 2026-10-23 KCORE EXEC SIZE AND FULL PERMANENT ACCOUNTING V188

KCC/p10fold linked resident EXEC v188 saves two further KCORE words:
`__kcore_low_end` `025661` -> `025657` octal. The latest full
`make -C system/boot/pdp6 permanent-size` run (default full/auto
KCC configuration) reports KCORE 11135 words, KCORE+MRES 18670 words,
PERMANENT_LAST `044435`, MULTI_LAST `044476`. Both the single-root
and multi-root strict `040000` permanent-address tests FAIL (report
distances -2333 and -2366 addresses respectively). This is a current
verified size failure, not a successful 16K target. The full report
is `$TMPDIR/daimos-permanent-v188b.log`.

The EXEC record-start comparison and low-memory RT cleanup changes
need native replacement, child-session and malformed-record regression
coverage. Previously validated v186 native tests are not v188 tests.

## 2026-10-23 EXEC/RUN POST-OPTIMIZATION REGRESSIONS V187

Current resident EXEC/RUN optimizations through v186 have KCC/p10fold
link acceptance and two passing fresh-image native PDP-6 diagnostics:
EXEC replacement (preserved PID, credentials, cwd, session, pgrp, TTY,
descriptors) and session-leader child EXEC with successful parent reap.
The preceding v185 EXEC replacement run also passed. Logs:
`$TMPDIR/daimos-exec-v185-replace.log`,
`$TMPDIR/daimos-exec-v186-replace.log`, and
`$TMPDIR/daimos-exec-v186-child.log`.

These tests used `DAIMOS_EXEC_OBSERVE_SECONDS=35` and the existing
functional-only `DSH_IMAGE_LIMIT=037000` without changing the default
15-second or `036000` production gates. The v186 post-p10fold linked
KCORE exclusive end is `025661` octal (28 words below `025715` at v173).
Strict acceptance, malformed-input stress, and targeted real-time-owner
handoff/rollback testing remain open.

## 2026-10-23 EXEC/RUN NATIVE DIAGNOSTIC ACCEPTANCE V183

After the v172-v181 resident EXEC/RUN instruction reductions, the
existing native PDP-6 EXEC-replacement and child-session EXEC tests both
PASS on fresh images using an explicit 35-second observation window and
the functional-only `DSH_IMAGE_LIMIT=037000`. The ABI smoke test also
PASSes after daimos-testkit v182 corrected its stale `proc/exec.c`
reference to the live assembler `exec_replace_current` and
`exec_load_process` entry points. Testkit v183 adds an opt-in
`DAIMOS_EXEC_OBSERVE_SECONDS` parameter; its default remains 15 seconds.
Evidence: `$TMPDIR/daimos-exec-v183-observe.log` and
`$TMPDIR/daimos-exec-child-v183-observe.log`.

This is **functional-only runtime acceptance**, not the strict production
EXEC image-size or 15-second timing gate. The default `036000` image
limit still rejects an oversized generated executable (`036013`), and
the default 15-second simulation test timed out. No production limits
were changed. Current same-profile KCORE exclusive endpoint after v181:
`025671` octal. Further low-memory and malformed-input coverage remains open.

## 2026-10-22 DECtape/TSFS GROUP 027 RETEST V169

Group 027's supported boot-root-only test inventory has seven
independently passing checks on the installed PDP-6 toolchain:
counted DECtape block-stream I/O, userspace-created TSFS images
read through the production Type-551 driver, eight-member scanning
and KINIT boot selection, cross-member extent reads, D6LZ restart
handling and real reverse-direction DECtape transfer. Testkit v169
uses installed `mktsfs` instead of the retired external `make build`
target. The result logs are `$TMPDIR/daimos-group027-v168-*.log`
plus the stream and scanner v167 logs.

The `dtc-online` test originally failed at the deferred DSH image
size cap 036000. Versioned testkit runner v169 runs with the
functional-only cap 037000 and reaches the PDP-6 guest. The first
format/mount cycle runs, but persisted DECtape media fails to mount
on the second boot with `MOUNT.DTFS:` diagnostic output. Group 027
is therefore still open; this is a DTFS compatibility mount test,
not a reason to restore the obsolete `MOUNT.TSFS` interface.
Atomic KCC/GCC matrix retesting has not yet passed.

## 2026-10-22 TSFS BOOT-ROOT-ONLY CONTRACT / TEST REMOVAL V165

By design TSFS is usable only as the **boot filesystem**. Mounting
another TSFS set later is not supported. TSFS filesystem generation
and writing to DECtape belong in userspace tooling and `dd`. Do not
change the root-selected TSFS MRES install policy to satisfy obsolete
secondary-mount tests.

The testkit removed the post-boot `MOUNT.TSFS` runtime tests
(`daimos-tsfs-mount-v1.sh` and `daimos-tsfs-dir-v2.sh`), and
removed their registration from both regression matrices. The
obsolete v1 host scan fixture, which tested the retired mount handoff,
is deleted; the retained eight-member v2 scanner validates member
discovery/ninth-member rejection and passes.

The DAIMOS userspace build and production disk root no longer compile,
package, document or expose `MOUNT.TSFS`. Its source and manual,
userspace syscall stub and declaration, extension dispatch entry,
mount-handoff builder and its unused metadata validation are removed.
The old syscall selector is directed to the existing unsupported
extension handler. KINIT's separate boot-root MRES mount operation
(`tsfs_boot_mount_root` -> `FS_MRES_OP_MOUNT_UNIT` ->
`tsfs_mount_set`) remains intact, as required for TSFS boot.

Group 027 remains active for DTC I/O, TSFS scanning, image generation,
compressed extents and tape-root boot, under KCC and GCC. Strict
32K-size tests remain independent.

## 2026-10-22 GE0 / TYPE-340 TOOLCHAIN ROOT CAUSE V164

GE0 LOGIN failure root-caused to an obsolete *installed*
`$PDP10_PREFIX/bin/p10fold` binary. On `ge_io.dobj` the installed
binary emitted a zero-cost two-word CONSO/JRST fold between distinct
GTYO readiness loops, redirecting a status skip into an unrelated
continuation. The current `tools/host/p10fold.c` source has the
skip-sensitive I/O checks and emits only `P10FOLD1` for the same
object. The source was compiled and installed to `$HOME/cross/bin`.
No P10FOLD source fix was needed in this turn.

On the repaired toolchain, the complete CTY/DCS0/GE0 multi-TTY
runtime v145 PASS, including respawn, routing, resource checks and
STOP isolation. The full native Type-340 DPY login and primary/shifted
refresh test v150 also PASS. Testkit v164 now checks the *installed*
P10FOLD against the real GE assembly source to prevent stale binaries
from silently reintroducing the defect. Logs:
`$TMPDIR/daimos-ge-correct-p10fold-multitty-v161.log`,
`$TMPDIR/daimos-ge-correct-p10fold-dpy-v163.log`.

KCC standalone paths are green; GCC and the full atomic group 030
are not accepted yet. Group 027 DECtape/TSFS is active.

## 2026-10-22 TYPE-340 NATIVE REFRESH V150 BLOCKED BY GE0 LOGIN

The corrected-SIMH, fresh-256K KCC testkit Type-340 native-refresh
runtime v150 boots DAIMOS, connects DCS0 and GE0 and receives DCS0
LOGIN, but receives no GE0 LOGIN prompt; FAIL `remote LOGIN missing`
before running Type-340 refresh assertions. This reproduces the GE0
startup symptom of standalone multi-TTY v144 and does not yet
identify a display-driver bug. Testkit `e1d49ab`; log
`$TMPDIR/daimos-dpy-native-v150.log`. Root-cause the common GE0
LOGIN/session/transport issue with pristine media and unique ports.

## 2026-10-22 GROUP 027 REACTIVATED V149

Group 027 (DECtape/TSFS) is back in the active testkit TODO at the
user's request; it is no longer deferred. Revalidate its ten member,
mount, compressed extent, eight-member boot, and reverse-operation
targets under KCC/GCC. Existing root-selected DTFS/TSFS MRES policy
must remain intact: fix test boot-class assumptions instead of
unconditionally loading resident filesystem code. Strict 32K size
acceptance remains separately deferred. Continue group 030 first,
especially multi-TTY, EXEC timing and Type-340 refresh.

## 2026-10-22 GROUP 030 EXEC / DTFS / MULTI-TTY INVESTIGATION V148

The independently accepted 66-probe KCC/GCC runtime suite and CTY
READCHAR accounting are unchanged. Group 030 as a whole is still open.

Two custom INIT targets, EXEC replacement and child-session EXEC,
complete on corrected SIMH with expected `<E` and `<xEP` HALT
markers. Fresh-image simulation-only timings are 18 and 19 seconds,
respectively, versus the existing 15-second wall-clock acceptance
timeouts. No acceptance timeout or EXEC assertion was weakened.
Versioned testkit diagnostics v137/v140 permit an explicit longer
observation window but default to the original 15 seconds.

DTFS-native testkit v143 boots D6FS-root and fails to mount DECtape.
That is consistent with the intentional root-selected MRES policy:
`dtfs_minits` installs the DTFS/TSFS package only for
`KINIT_ROOT_DTC`. Successful file probes after failed MOUNT.DTFS
target the underlying mountpoint, not DTFS. This test must be
adapted to a supported DECtape-root configuration before any
DTFS acceptance can be claimed. Group 027 is still deferred by
request; this investigation did not modify the root/provider policy.

With corrected SIMH, standalone multi-TTY v144 obtains DCS0 LOGIN
but not GE0 LOGIN. Process-inventory diagnostic v145 could not
distinguish session spawn from transport, because one attempt
reused a dirty disk (HALT `?RT`) and another had a DCS0 TCP
connection timeout. No kernel TTY change is yet justified.

Evidence: `$TMPDIR/daimos-exec-simh-timed-v148.log`,
`$TMPDIR/daimos-exec-child-diagnostic-v141.log`,
`$TMPDIR/daimos-dtfs-group030-v143.log`,
`$TMPDIR/daimos-userspace-multitty-v144.log`, and
`$TMPDIR/daimos-userspace-multitty-pristine-v146.log`.
Strict 32K and full serialized matrix acceptance remain deferred/open.

## 2026-10-21 CTY COUNTER DEFECT / GROUP 030 PARTIAL V136

DAIMOS commit `08625cf` fixes missing completed-CTY I/O counters:
`cty_putchar` now increments the existing MonitorFS CTY0 output slot
only after PI4 confirms transmission, while `cty_getchar` increments
the CTY0 input slot only after actually returning a character.
The driver has no new BSS, buffer, or queue allocation. Before the fix,
the independent CTY READCHAR test read the three correctly formatted
octal STAT lines but returned `<C!4` because output accounting never
advanced. After the fix it passed under KCC and GCC. Evidence:
`$TMPDIR/daimos-monitorfs-cty-kcc-v133.log`,
`$TMPDIR/daimos-monitorfs-cty-gcc-v134.log`.

The original runtime probe suite used obsolete STAT/DF output formats
and attempted to read MonitorFS CTY0/STATS with CAT, which consumes
S6REC word records rather than the READCHAR character API. Testkit
commit `77f7d89` provides versioned `daimos-runtime-probes-v132.txt`
and the separate `test-daimos-monitorfs-cty-stats-v132` target.
All **66/66 runtime probes pass KCC and GCC** on fresh 256K disks:
`$TMPDIR/daimos-group030-runtime-kcc-v135.log` and
`$TMPDIR/daimos-group030-runtime-gcc-v136.log`.

This independently closes only the runtime-build/MonitorFS portion
of regression group 030. Exec, DTFS, display and multi-TTY acceptance
remain OPEN; group 027 is deferred by request, and strict 32K gates
remain deferred.

## 2026-10-21 D6FS REGRESSION GROUP 029 V131 ACCEPTED

The 256K non-size **group 029** D6FS HDD, multimember and recovery
tests now pass independently under both KCC and GCC.

The multimember test fixture had two unrelated defects: its generated
SIMH command files reattached obsolete DSK1-3 media, and its synthetic
D6FS roots omitted production MOUNTALL/configuration and the external
executables required by the 16 file-operation probes. Testkit commit
`552428c` selects versioned `run-multimember-v126.sh`, which seeds
correctly populated roots and attaches only the intended test members.
KCC v126 and GCC v127 each pass all 16 operations on both two-member
and four-member roots, including both rejected invalid-member layouts
and LOGSTORE post-checks. No DAIMOS kernel or D6FS code changed.

The ordinary HDD runtime passed 16 probes and the corrected four-member
LOGSTORE badmap check under KCC v128 and GCC v119. The separate
dual-superblock recovery fixture also passed 16 probes, bootstream
preservation, LOGSTORE inspection and dirty-reboot rejection under
KCC v130 and GCC v129.

Evidence logs are `$TMPDIR/daimos-d6fs-multi-kcc-v126.log`,
`$TMPDIR/daimos-d6fs-multi-gcc-v127.log`,
`$TMPDIR/daimos-d6fs-hdd-runtime-kcc-v128.log`,
`$TMPDIR/daimos-d6fs-recovery-gcc-v129.log` and
`$TMPDIR/daimos-d6fs-recovery-kcc-v130.log`, plus the earlier GCC
`$TMPDIR/daimos-d6fs-logstore-v119/d6fs-hdd-runtime-v119.log`.

Groups **022, 028, 029** are independently accepted; **027** is deferred
by request. **030** and atomic full-matrix acceptance remain open.
Strict 32K and size gates remain deferred.

## 2026-10-21 KERNEL/STANDALONE REGRESSION GROUP 028 V123 ACCEPTED

The independent non-size **group 028** test now passes KCC and GCC on
fresh 256K production PDP-6 disk images. Its previous three-second CTY
watcher incorrectly required an interactive `LOGIN:` prompt on CTY;
the actual interactive LOGIN/DSH transport is DCS0. The versioned testkit
v121 replacement requires the DAIMON version banner and BSET initialization
on CTY, ROOT login through DCS0, a fresh DSH prompt, and successful
external ECHO and PWD commands. Both compiler builds passed both probes.

The standalone PDP-6 bootloader suite was rerun independently and passed
**26/26** DSK/PTR/MTC/DTC cases, with zero failures. This was a test-harness
contract correction; no DAIMOS kernel source changed. The functional
DSH ceiling was explicitly set to `037000`. Strict 32K/size acceptance
remains deferred.

Evidence is `$TMPDIR/daimos-kernel-group028-kcc-v121.log`,
`$TMPDIR/daimos-kernel-group028-gcc-v122.log`, and
`$TMPDIR/daimos-stand-group028-v122.txt`. **Group 027** (DECtape/TSFS)
is deferred by user request. Groups **029/030** and the complete
non-size matrix remain open.

## 2026-10-21 PDP-6 SIMH CONSOLE PC / TRAP GROUP 022 V120 ACCEPTED

The independent 256K non-size group-022 trap-frequency retest now passes
under **both KCC and GCC**, with unchanged syscall-count ceilings. KCC
measured startup/echo/memstat/ls/ps/cat128 as 111/16/19/33/23/18;
GCC measured 90/16/19/33/25/18. Both passed ceilings of
128/35/70/70/110/45, respectively. The GCC run used the isolated
corrected PDP-6 compiler and the same corrected SIMH CPU as KCC.

The root cause was a host ELF symbol collision: the SIMH KX10 CPU's global
`PC` overlapped terminal-library padding `PC`, which interactive readline
overwrote. The corrected symbol `kx10_pc` (sims `1a41f45`) passed direct
script-to-console PC preservation, PDP-6 breakpoint/step execution, and
register sanity for PDP-6, KA10, KI10, KL10, and KS10. The standalone
testkit regression is `test-daimos-simh-console-pc-v115` (commit `58150fd`).
KA10/KI10 also received a separate default `-lm` link correction
(sims `5f6d21b`).

Testkit `0f6ad07` explicitly selects diagnostic `DSH_IMAGE_LIMIT=037000`
for this *functional* trap test; the original default 036000 limit rejected
the current 036021 image before boot. Strict 32K boot and permanent size
acceptance remain deferred, not silently passed. Runtime evidence:
`$TMPDIR/daimos-trap-runtime-fixed-pc-v117.log` and
`$TMPDIR/daimos-trap-gcc-fixed-pc-v118.log`.

This verifies group 022 independently; **full non-size matrix acceptance
is still open**. Groups 027, 028, 029, and 030 require separate repair and
retakes, and the complete matrix has not been rerun atomically. The v111
investigation below is retained as historical evidence, superseded for
the console-PC root cause and group-022 outcome.

## 2026-10-21 D6FS FOUR-MEMBER LOGSTORE RUNTIME V119

The non-size 256K GCC D6FS HDD runtime's `invalid badmap` post-check was
caused by the **testkit** invoking host `logstore --dump -n 1` against the
standard **four-member** D6FS disk set.  The incorrect logical-to-physical
mapping read a non-BADMAP block.  Direct comparison against the same disk
demonstrated `-n 1` FAIL (`invalid badmap`) and `-n 4` PASS (valid metadata).
Testkit commit `16ccd39` corrects the geometry to `-n 4`, without changing
any kernel or filesystem source.  A fresh GCC/256K `d6fs-hdd-runtime` run
then passed **16/16 guest probes** and its host LOGSTORE check (exit 0),
at `$TMPDIR/daimos-d6fs-logstore-v119/d6fs-hdd-runtime-v119.log`.
This resolves that specific group-029 subtest, **not** the complete group
(multi-member and recovery acceptance remain separate).  All dedicated
size and strict 32K tests remain **deferred**.

## 2026-10-21 PDP-6 SIMH STOP/CONTINUE REGRESSION V111 — OPEN

The checkpointed non-size matrix v75 has finished under **both GCC and
KCC**.  Groups 022 (trap), 027 (DECtape/TSFS), 028 (kernel runtime),
029 (D6FS), and 030 (userspace runtime) remain FAIL under both compilers.
They must be repaired independently; neither compiler has achieved full
non-size functional acceptance.  Evidence:
`$TMPDIR/daimos-nonsize-acceptance-v75/{gcc,kcc}-v75.state`.

Trap group 022 fails *before* it measures syscall rates.  The original
test reported `LOGIN INCORRECT` following an asynchronous SIMH Ctrl-E
stop/resume.  The isolated v98 experiment moved the first measurement
until after DSH login: ROOT authentication then succeeded, but the first
stop/resume still caused an invalid UUO and a HALT near PC 000001.
Independently, with **no syscall breakpoint installed**, a tape-root
SIMH session reproduced the stop/resume problem (v103).  In the v108
single-step diagnostic SIMH reported stopping at PC 024162 but a
subsequent `EXAMINE PC` returned 024000; a step ran from 024000 and
the machine halted.  This establishes a simulator console/PC-state
discrepancy, but its exact cause remains unresolved.  Temporarily
restoring the reported PC (v110) did **not** restore a completed DSH
command, so it is not an accepted workaround or fix.

An experimental PDP-6 CPU asynchronous-stop deferral also failed its
v105/v107 runtime controls.  It was removed in sims commit `3ae21bb`,
preserving the independent Type-166 floating-point implementation in
commit `828459c`.  Testkit commit `3ccaaa2` preserves CTY/DCS error
transcripts on request (`DAIMOS_TRAP_KEEP_WORK=1`), without altering the
original syscall-count thresholds.  Diagnostic transcripts:
`$TMPDIR/daimos-trap-frequency-v1-24813/cty.log`,
`$TMPDIR/daimos-simh-resume-v108/cty.log`, and
`$TMPDIR/daimos-simh-resume-v110/driver-v110.log`.

Next: isolate the simulator console's saved PC across a stop and
`CONTINUE` and add a target-executed test to `daimos-testkit`; do not
weaken the trap-frequency oracle.  **All 32K and dedicated size tests
remain deferred** by user direction.

## 2026-10-20 SECONDARY TSFS MOUNT ON TAPE ROOT V96

The independent 256K PDP-6 SIMH tape-root experiment passed **3/3**
secondary-volume probes: `MOUNT.TSFS /DEV/DTC4 /MOUNT/DT0`,
`STAT /MOUNT/DT0` (read-only directory, `TYPE 1`, `MODE 0555`), and
`LS /MOUNT/DT0` (entries `README`, `DOC`).  The bootable TSFS root
was attached on DTC0, and separate three-member TSFS media on DTC4–6.
The kernel's existing tape-root-only provider was used; there were no
additional resident TSFS modules on disk-root systems or kernel changes.
Evidence: `$TMPDIR/daimos-tsfs-tape-root-v96/report-v96.md` and
`pdp10-doc/STATUS/DAIMOS-TSFS-SECONDARY-MOUNT-TAPE-ROOT-20261020-V96.MD`.
This validates the three specific probes only.  The original group-027
disk-root TSFS mount fixtures require adaptation and full retesting.
The 32K and permanent-size gates remain **deferred**.

## 2026-10-20 AUTO ROOT DRM FOUR-MEMBER FIXTURE V91

The v74 non-size GCC AUTO-root run passed DISK and TAPE, but initially
failed DRUM.  Inspection of the live `d6fs_boot_scan_unit` return showed
that the Type-236 read succeeded and the blockset root marker advertised a
four-member D6FS set (membership mask 017).  The testkit fixture had copied
only drum member zero into the AUTO image, leaving its other three drum
volumes from an unrelated build.  Root discovery correctly rejected that
incomplete set; `kinit_boot_handoff` remained unset and mount capacity was
zero.  The testkit now copies **all four** drum image members in commit
`68d5664`.  A focused 256K GCC/PDP-6 SIMH boot with fresh, complete drum
media reached `D6FS LOADED`, `INIT V1`, ROOT login, DSH, and passed
`CAT /CONFIG/SYSTEM` with `ROOTCLASS DRUM` (one of one probes, v91).
No production KINIT or D6FS change was required.  DISK/TAPE from v74 and
DRUM from v91 are separate verified runs, not yet a single full atomic
three-case acceptance.  Evidence:
`$TMPDIR/daimos-root-four-member-v91/report-v91.md` and
`$TMPDIR/daimos-nonsize-acceptance-v74/006.log`.  The 32K boot and all
dedicated size tests remain **deferred** at the user's direction.

## 2026-10-20 MONITORFS SWAP READER CHECKPOINT V71

The experimental MonitorFS swapped-process startup-record reader, its
resident assembler dispatch, and its kernel Makefile integration were
committed as DAIMOS `dc1f96f` at the user's request.  This is a source
checkpoint, **not functional acceptance**.  Previous KCC/GCC source builds
and a 256K KCC image link passed, but the runtime test did not return the
expected swapped-process NAME/CMDLINE/ENVIRONMENT metadata.  Repair and
rerun the unchanged acceptance oracle before declaring the reader ready;
the strict 32K residency/boot gate is also open.  This commit was made
locally, without pushing.

## 2026-10-20 AUTO ROOT THREE-CLASS ACCEPTANCE V70

The corrected `daimos-auto-root-priority-v1.sh` fixture is committed as
daimos-testkit `ab730fe`.  Under a rebuilt Type-551 SIMH with a fresh 256K
KCC AUTO image, independent probes verified the full desired preference:
DSK present selects `ROOTCLASS DISK`, all DSK units absent selects
`ROOTCLASS TAPE`, and both DSK and DTC classes absent selects
`ROOTCLASS DRUM`.  All three root-class assertions passed and reached ROOT
login and DSH.  The initial 120-second per-command TSFS fallback allowance
timed out with the shell already running; the repeat using 360 seconds
completed `CAT /CONFIG/SYSTEM` and produced `ROOTCLASS TAPE`.

Evidence is preserved under `$TMPDIR/daimos-auto-root-priority-v1-26755/`:
`all.md`, `no-dsk-long-v66.md`, and `drm-only-v69.md`.  The drum test used
regenerated clean test media after a diagnostic run left its previous
instance unclean.  These are independently verified cases, not an atomic
end-to-end rerun of the final test script, and they establish neither a
strict 32K boot nor MonitorFS swapped-process metadata correctness.  The
three experimental MonitorFS source/build changes were subsequently
checkpointed in `dc1f96f` without runtime acceptance (see V71 above).

## 2026-10-20 UNCOMMITTED-WORK TRIAGE V64

The pending D6FS host maintenance changes have been independently tested and
committed: `daimos-tools` c79752c and `pdp10-tools` 74ef3aa.  `mkd6fs` now
accepts an empty auxiliary root; `d6fsck` verifies that result as CLEAN.
The `d6maint` helper correctly resolves `d6fsck` through PATH when invoked
as `packfs` without an explicit directory.  The host tests for directory
creation, consistency checking, recovery and PATH-based compaction passed.
The GE lost-carrier output-completion fix was built with a forced SIMH PDP-6
rebuild and its register sanity check passed (SIMH 220f839).

The experimental MonitorFS swapped-process startup-metadata reader
(`system/kernel/fs/monitorfs_swap_v63.c`, the runtime assembler change and
the kernel Makefile integration) compiles with KCC and GCC, and a fresh
256K KCC disk boot image links.  It is **not accepted**: the live MonitorFS
test reaches ROOT/DSH but both relative `CAT 2/NAME` and absolute
`CAT /MONITOR/PROCESSES/2/NAME` return without the expected DSH process
name.  The v64 test re-run failed its existing read oracle.  These three
kernel files were uncommitted at the time of this V64 triage and were later
committed as the unaccepted V71 checkpoint; compilation alone is not
functional acceptance.  Diagnostic
log: `$TMPDIR/daimos-uncommitted-20261020-v64/monitorfs-live-retest-v64.log`.
The testkit diagnostic-preservation change is committed as b3c458a; the
separate automatic-root-priority test remains unverified in this sweep.

The first DECtape retest used an outdated installed SIMH PDP-6 binary and
timed out after DTC startup.  That timeout is invalid as an assessment of
the PI3 kernel experiment, because it did not run the repaired Type-551
simulator.  The repeat with freshly rebuilt `sims/BIN/pdp6`, the same tape
image, and the two PI3 kernel changes passed **both external commands**:
`ECHO RETESTED_OK_V24` produced its output and a prompt, and `PWD` produced
`/` and a prompt.  The v34 harness returned exit zero (2/2 PASS).  Evidence:
`$TMPDIR/daimos-uncommitted-20261020-v64/simh-tape-sims-fixed-v64/report-v34.md`.
The two PI3 files were committed separately as `2c85325`; project-state
acceptance is commit `c1f693a`. The three MonitorFS files were later
checkpointed, without acceptance, as `dc1f96f`.  These checks are
256K runtime acceptance, not strict 32K size/boot acceptance.

## 2026-10-20 GCC STORAGE GROUP 023 VERIFIED V53

The complete non-size, 256K GCC storage-integration group 023 now PASSes
all ten tests, including MonitorFS online controller state, independent
D6FS mount contexts, drum backing, BCACHE, remount/recovery, reservation,
and BADMAP root recovery.  This is the first atomic rerun of the group
since the testkit fixture repairs `2804838` and `81b2610`.  The compiler
is the isolated GCC backend `df27247`; there are no new DAIMOS kernel
changes.  Evidence: `$TMPDIR/daimos-gcc-v53/group023-v53.log` (exit 0),
and pdp10-doc `STATUS/DAIMOS-GCC-STORAGE-INTEGRATION-20261020-V53.MD`.
Other non-size GCC failures are still under investigation; neither full
functional-suite nor 32K size acceptance is implied.

## 2026-10-19 GCC DISK FUNCTIONAL RETEST V52

The focused 256K disk-suite GCC regressions now PASS group 004 (five boot
origins, DSK root discovery, Stage1/KINIT, SIXBIT lifetime and PDP-6
codegen), group 031 (115/115 DSH probes), and group 032 (all host-tools
scripts). Group 023's three D6FS mount oracles and its previously failing
MonitorFS device-state test all pass independently; group 023 has not
yet been rerun atomically.  Testkit commit 81b2610 detaches all four DTC
units before disabling the SIMH controller.  Testkit commit 11feecb
updates the host D6FS repair crash fixture to verify the actual three
durable writes: DIRTY, changed summary, CLEAN; cut 4 never triggers.
The prior fourth-write expectation was stale, not a host checker bug.

The GCC scheduler multiprocess case (017) still fails under patched GCC.
Some group 027 tape mount fixtures request a DTFS/TSFS package in a D6FS
root profile where it is deliberately absent; neither a kernel feature
nor a test skip has been accepted.  Other original matrix failures remain
open.  The corrected compiler has not been installed systemwide.  All
retakes here are 256K, DSH_IMAGE_LIMIT=037000; no 32K claims.  Detailed
evidence: pdp10-doc/STATUS/DAIMOS-GCC-FUNCTIONAL-REGRESSION-REPAIRS-
20261019-V52.MD.

## 2026-10-19 GCC PDP-6 NINE-BIT ARRAY COMPILER REPAIR V46

The GCC backend had a target-executed, reproducible compiler bug at -O1,
-O2 and -Os: a 9-bit char array on the stack returned the wrong character
at a 36-bit word boundary.  A frame-relative 36-bit word access already
selected by the RTL was incorrectly treated as a byte displacement during
final assembly output.  GCC commit df27247 repairs that output path;
the direct PDP-6 testkit regression (commit e24488e) now passes at -O0,
-O1, -O2 and -Os.  The neighboring multidimensional-char regression
passes -O0/-O1/-O2.  No DAIMOS production-code change is involved.

An isolated rebuild with the repaired GCC booted a fresh 256K DAIMOS disk
image and passed **all 115 DSH conformance probes**, including HELP ECHO
and HELP DAS, which were the only two failing GCC probes before the fix.
Reproduction: $TMPDIR/daimos-gcc-v46/dsh-conformance-v46.log, result v46
exit 0.  The patched GCC was tested with an isolated toolchain wrapper;
the installed compiler was not overwritten.  Dedicated 32K and image-size
tests remain explicitly excluded.

The GCC multiprocess scheduler preemption test still fails under the
repaired compiler (v47); its clock/quantum behavior must be diagnosed
independently rather than weakening the test.  The disk regression groups
also retain separate VFS/MonitorFS, storage, DECtape and D6FS failures.
The original 30-group matrix has not been rerun in full after this repair;
do not claim full testkit acceptance.  See
pdp10-doc/TOOLCHAIN/GCC-PDP6-LOCAL-CHAR-ARRAY-20261019-V45.MD.

## 2026-10-19 AAP AND SIMH DECTAPE COMMAND ACCEPTANCE V37

Both supported simulator tape-root paths now reach INIT, ROOT, and DSH
and execute external ECHO and PWD successfully at 256K with the diagnostic
DSH image ceiling 037000.  The repaired SIMH Type-551 selected-transport
case passes 2/2 command probes.  A fresh AAP headless PDP-6 run passes
ECHO and PWD too: DSH was entered after 103.4 seconds, ECHO finished
after 300.2 seconds, and PWD finished after 548.2 seconds total.
The former 240-second AAP test timeout was insufficient for the observed
DECtape seeks and does not establish a permanent command-execution stall.
No new DAIMOS or AAP production change was required for this retest.

The separate full-device SIMH disk/D6FS functional regression sweep is
still under way with 32K and dedicated size tests excluded.  The 12-way
independent boot/root cross-matrix has passed.  The automatic-root-priority
testkit had inconsistent INIT/login expectations; its corrected fixture
is being independently retested.  No full disk-testkit acceptance is yet
claimed, and strict 32K boot and persistence remain separate gates.

Reproductions: daimos-testkit/tests/validation/daimos-aap-boot-basic-v37.py,
daimos-simh-dtc-selected-v34.sh, and
daimos-regression-disk-nosize-v35.tsv.  Details are in
pdp10-doc/STATUS/PDP6-SIMH-TYPE551-REGRESSION-20261019-V34.MD.

## 2026-10-19 SIMH TYPE-551 MULTI-UNIT BOOT REPAIR V34

The high-priority V24 AAP-like SIMH DECtape-root stall is fixed in the
SIMH source tree (local commit `8361359`).  The SIMH Type-551 event routine
was updating a shared `DTSB` from nonselected moving drives; an inactive
drive's end-of-tape could stop the selected drive with a Type-136 read still
in progress (V24 observed 119/128 words, IOWD -9).  A new selected-unit
owner gates status, Type-136 data, and interrupt operations while retaining
physical motion for unselected drives.  The same SIMH commit restores CPU
`PC` as the first register, which is necessary for `GO 020` in rebuilt SIMH.

On the **unchanged experimental DAIMOS PI3** tape driver, AAP-like SIMH
DECtape/TSFS root now reaches INIT, ROOT, DSH and successfully executes
external ECHO and PWD (2/2 tests).  Freshly rebuilt full-device SIMH with
DSK/D6FS root also reaches ROOT/DSH and passes ECHO/PWD (2/2).  A disk-root
`?RT` on reused media remains a distinct unresolved persistence regression;
it did not reproduce after regenerating the image.  All runs here are 256K
with the diagnostic `DSH_IMAGE_LIMIT=037000`, **not** 32K acceptance.

The AAP post-DSH ECHO timeout from V24 has **not** been retested or fixed.
The DAIMOS DECtape PI3 work in `tape_io.s` and `storage_router.s` remains
uncommitted, unchanged by the SIMH repair.  Test reproduction is in
`daimos-testkit/tests/validation/daimos-simh-dtc-selected-v34.sh` and
`pdp10-doc/STATUS/PDP6-SIMH-TYPE551-REGRESSION-20261019-V34.MD`.

## 2026-10-18 CROSS-EMULATOR BOOT RETEST V24

Fresh 256K images from HEAD bc22c27 plus the uncommitted DECtape PI3
driver/router changes were exercised on AAP, comparable-hardware
SIMH with DTC/TSFS root, and full-device SIMH with DSK/D6FS root.
The diagnostic DSH size override 037000 remains necessary.

AAP reaches INIT, ROOT and DSH but ECHO RETESTED_OK_V24 never produces
output or a new prompt within 240 seconds. SIMH with non-AAP disk,
drum, MTC and terminal peripherals disabled reaches DTC OK but does
not reach INIT. A second AAP run, using the headless emulator forcibly
rebuilt from cleaned revision ac603e8, reproduces the same ECHO timeout.
Debugging the SIMH tape run found an active 128-word Type-136 DECtape
read with IOWD count -9 (119 words transferred), no progress on
resumption. This is an open cross-simulator DECtape regression; the
source of the discrepancy has not been assigned to DAIMOS or SIMH.

Full-device SIMH disk boot reaches ROOT/DSH on DCS0 and runs external
ECHO and PWD. A separate fresh-media CTY boot passes the same tests.
The first DCS0 test also logged INIT: RUN FAILED, and reusing its disk
media led to a subsequent ?RT root-mount halt; regenerating the
media made the CTY test pass. Startup consistency and disk persistence
still require investigation.

Reproduction probes are in daimos-testkit. Detailed evidence is in
pdp10-doc STATUS/EMULATOR-BOOT-RETEST-20261018-V24.MD.
No production code or emulator fix was made during the retest.

## 2026-10-18 AAP DEFAULT-INITTAB BOOT RECOVERY V23C

INIT no longer launches LOGIN against missing physical terminal controllers.
The default INITTAB still lists TTY0 (CTY), TTY1 (DCS0) and TTY17 (GE0),
but `init_tty_input_present_v23c` checks the corresponding MINIT-published
MonitorFS `/DEV/CTY0`, `/DEV/DCS0`, or `/DEV/GE0` endpoint before admitting
each entry.  A successfully probed controller permits all its logical lines
even without carrier; unprobed/absent controllers do not allocate a LOGIN
process or enter a failed respawn cycle.  No kernel ABI or resident RAM change.

With the previous v23b packed CTY FIFO and still-**uncommitted** experimental
Type-551 PI3 tape driver, a fresh AAP root-tape boot **using the unchanged
three-entry INITTAB** reaches INIT, ROOT authentication and `DSH V1`.
Evidence: `$HOME/tmp/aap-dtc-boot-v41/aap-boot-console-pi3-v41.log`,
validated by the new testkit v23c trace verifier: 234 forward plus 21 reverse
128-word DECtape audits, zero word errors, no respawn/session/auth messages.
Host C99 tests of the exact production check cover TTY 0/1/16/17/20,
missing hardware, invalid status, and syscall failures.  A previous failed
AAP boot capture is correctly rejected by the verifier.

The transient INIT code text grew from 983 to 1050 words (+67), BSS remains
177 words; KCORE/MRES residency is unchanged. The boot still uses a
diagnostic-only expanded DSH image ceiling, not strict 32K acceptance.

**Separate open issue:** a subsequent AAP DSH `ECHO AAPBOOTOK` request is
delivered and echoed as a command line but does not produce the command's
result or a fresh prompt within the longer v43 experiment. Correct DECtape
word content does not establish provider lookup/search completion. This does
not invalidate the completed default-INITTAB INIT/LOGIN/DSH boot fix.

## 2026-10-18 CTY TYPE-626 INPUT PRESERVATION V23B

The CTY PI4 driver previously used a one-character mailbox which each
incoming DATAI unconditionally overwrote.  Under DECtape I/O latency this
lost queued input: a controlled AAP headless run received all five letters
of `ROOT` and CR, but echoed only `RO`.  V23B replaces the mailbox with a
bounded ten-character, two-word packed seven-bit FIFO and a one-word count.
If full, PI4 masks its input priority without acknowledging the hardware
byte; the foreground dequeue releases a slot and re-enables PI4.  The
Type-626 can then deliver its pending byte.  The producer/consumer use no
unbounded loops and no dynamic allocation; the implementation adds two
resident data words relative to the old one-byte mailbox.  CTY MRES grows
from 58 to 112 words (+54 linked resident words including instructions).

Native PDP-6 exact-assembly FIFO tests pass (NUL, ten-byte capacity, full
rejection, cross-word FIFO order, reuse).  Existing TTY routing tests pass.
An AAP PI3 DECtape + CTY FIFO diagnostic boot with a **CTY-only test
INITTAB** successfully reached ROOT login and DSH.  The original three-
session INITTAB does not: extra LOGIN process/respawn attempts trigger
`INIT: RESPAWN FAILED` and `LOGIN INCORRECT` despite physically correct
DECtape word transfers.  Only the CTY FIFO is accepted here; the PI3
DECtape search changes and the multi-session startup investigation remain
uncommitted.  The diagnostic INITTAB is not the shipped configuration.

The repaired installed `pdp10-objdump` correctly reports MRES resident size
using the current source, packaged as versioned `local.tar.gz` and installed
to `$PDP10_PREFIX`.  The disk permanent gate passes with 170 free words
and tape permanent gate with 568 free words for this *experimental PI3*
working tree; they are not strict 32K KCORE acceptance.

## 2026-10-18 UNCOMMITTED-CODE TRIAGE V23A

The native FILE FLOCK traversal once again computes the owning descriptor's
packed-node difference after extracting the per-fd high mount bits. This
prevents the candidate table displacement from being mistaken for the lock
family comparison. `proc_swap_boot.c` invalidates the shared filesystem
block-cache workspace before a direct persistent BACKSTORE read. The GCC
process swap-out and swap-in paths now publish RESIDENT/DISK state changes
through the production storage map, matching the native assembly path.

The PDP-6 FILE mount-extension and VFS 26-slot policy regressions pass. The
Termux host `mm-core` fixture links the existing storage-map implementation
and supplies an inert block-cache invalidation stub; its full MM suite passes.
These fixes do not enable experimental AAP bootstrap diagnostics or CTY FIFO
changes. The DECtape PI3 search rewrite is reviewed and tested separately.

## 2026-10-16 BOOT-SELECTED MM VFS CAPACITY V22

KINIT now derives its mount slot count from validated physical root-member
masks and the MINIT service directory. D6FS handoff locators expose physical
disk/drum units (including nonconsecutive drives); TSFS selection stores a
transient mask of its validated DECtape units. The installed DSK/DRM/DTC
controllers contribute supported transport slots, not media occupancy.
Controllers without their installed filesystem provider do not consume
mount-table space (D6FS excludes TSFS/DTFS MRES and vice versa in the
current boot profile). MEMFS contributes one slot only in full builds.

KINIT calls the existing 26-slot `vfs_mount_capacity_v1` planner, allocates
and zeroes the exact six-array MM slab, moves the boot reader, publishes all
six table bases and **then** publishes the `vfs_mount_limit`. There is no
fixed-four fallback on inconsistent inventory. A full D6FS/DSK+DRM boot
with one root member allocates nine slots/54 MM words; a detected DSK-only
full boot uses five slots/30 MM words, and a minimal disk-only profile uses
four/24. A full TSFS/DTC one-member root also uses nine slots. The packed
record and policy upper bounds remain 26; current bootable MRES combinations
require at most nine rather than reserving phantom controller/provider slots.

Native PDP-6 full/reduced inventory tests and the existing MM/policy/provider
regressions pass. Disk-root diagnostic ROOT login/ECHO uses the documented
test-only DSH size override. Documentation:
`design/vfs-boot-capacity-20261016-v22.md`.

V22 disk/tape permanent last words remain 037355/036537 octal (disk multi
037416). Static 32K KCORE end 024142 fails the 020000 gate by 2146 words.
The KINIT disk/tape image ends are 073277/074277, leaving 1345/833 words
after reserved bootstrap stacks; these are **not** 32K boot acceptance.

## 2026-10-16 VFS 26-SLOT PROVIDER BOUNDS V21

Replaced the last explicit 4-slot D6FS MRES dispatch and DTFS SPACE bounds
with `vfs_mount_limit`, protecting MM-backed table indexing. Optional
TENEX/ITS DTFS personality lookup no longer uses a four-word self-modifying
XCT table: it reads the existing MM-backed `dtfs_media_base` entry and
validates its slot. Foreign support remains disabled in the shipped build,
but the enabled assembly branch builds and its exact production leaves pass
native PDP-6 tests at mounts 10 and 26, including invalid mount 27.
The separate staged VFS access/parent helpers now also admit 26 slots.

No added resident/process RAM. Disk single/multi/tape permanent last remain
037355/037416/036537 octal; 32K lowmem KCORE end remains 024142 octal,
2146 decimal words above its strict gate. Diagnostic disk-root ROOT login
and ECHO pass under the documented test-only DSH size override.

**Capacity remains four in production.** `vfs_mount_capacity_v1` is a
staged inventory planner, not wired to KINIT. Boot must obtain validated
controller unit counts and root membership before it can publish an MM slab
capacity up to 26. See `design/vfs-provider-26-20261016-v21.md`.

## 2026-10-16 PURE BACKING 26-MOUNT EXTENSION V20

The one-word resident PURE swap record still contains its complete 18-bit
vnode index, four low mount-slot bits, and 14-bit text length. The fifth
mount-slot bit (public IDs 17..26) lives in the existing stable u-area
credential word 066, bit 27, with **no additional process words**. For RUN,
which attaches the executable before allocating the u-area, the bit is staged
in otherwise-unused SIDL scheduler RH bit 0 and published after inheriting
credentials but before setting SRUN. Detach clears the attached/staged bit.
This fixes an actual early RUN boot regression encountered when attachment
was initially made to require an already-allocated u-area.

PURE swap-out/in, raw/compressed reload, optional PURE verifier, vnode and
mount-busy guards all preserve five slot bits. Native unpack validates
`vfs_mount_limit` before indexing the MM-backed root array, and busy guards
skip slots lacking a valid u-area. Ordinary and destructive EXEC preserve
the lifecycle; transactional EXEC snapshots both the old and new extension
in transient stack words and restores the correct bit on failure/commit.

PDP-6 native tests pass for mount IDs 10/26, maximal index/text, corruption
bounds, full raw/compressed PURE swap and verifier, attachment and SIDL
staging, RUN inherited-credential isolation, and EXEC load/startup/VM
rollback and successful commits. Diagnostic disk-root SIMH ROOT login/ECHO
passes after fixing RUN staging. See
`design/pure-backing-mount26-20261016-v20.md`.

Final v20 linked permanent ends (octal): disk single `037355`, disk multi
`037416`, tape single `036537`. This is +116 decimal permanent words over
v19, with no extra per-process memory. GCC lowmem 32K KCORE end `024142`
still fails its strict `020000` limit by 2146 decimal words. Disk/tape KINIT
ends `073042`/`074001`, leaving 1502/1023 words after reserved boot stack.
Diagnostic disk-root PDP-6 ROOT login/ECHO is verified with the existing
test-only DSH size override. No production 32K boot claim is made.

Production still publishes four mounts; remaining 26-mount blockers are
provider-local bounds, optional TENEX/ITS DTFS personality dispatch, and
KINIT's capacity selection. The 32K strict KCORE gate is not yet met.

## 2026-10-16 TWO-WORD FILE DESCRIPTOR 26-MOUNT EXTENSION V19

Active native FILE descriptors now preserve mount bits 3..4 (canonical vnode
bits 27..28) in the existing per-process two-bit-per-fd extension bitmap
at u-area word 067. The packed two-word descriptors, full 36-bit offset,
and `0410`-octal u-area remain unchanged. FILE open/close, canonical-vnode
I/O dispatch, FIFO substitution, DUP/DUP2, FLOCK, unmount lock cleanup and
RUN mapped-descriptor inheritance now maintain the extension. Pipe/TSFS
provider-7 special dispatch was hardened: a TSFS root on mount 8 must not
be mistaken for a PIPE endpoint when duplicated, closed or inherited.
Native PDP-6 tests cover mount IDs 2 versus 26, high-bit preservation, lock
independence, duplicate/close/unmount cleanup, RUN fd remapping, and TSFS
versus PIPE reference ownership; diagnostic disk-root ROOT login/ECHO passes.
Full PDP-6 FIFO and word-FIFO simulator regressions also pass with the
existing diagnostic-only DSH `037000` size override. Without that override
the standard `036000` DSH gate still fails; production acceptance is not
claimed.

Memory costs: +138 linked permanent words versus v18, no per-process data
increase. Disk-root single permanent last 037171, multi 037232, tape root
036353 (octal). GCC 32K lowmem KCORE end 024006 FAILS strict 020000 by
2054 decimal words; KINIT disk/tape ends 072706/073645 leaving 1594/1115
words after reserved stack. Production mount capacity is still **four**.
PURE backing's fifth mount bit, optional TENEX/ITS provider table/bounds,
and KINIT capacity selection remain pending. Details in
`design/file-mount-packing-20261016-v18.md`.

## 2026-10-16 COMPACT CREDENTIAL / FILE EXTENSION RESERVATION V18

UID9, GID9 and UMASK9 now share u-area credential word 066, leaving the
existing word 067 completely available for 16 two-bit FILE mount extensions
(32 of 36 bits). The native GETUID/GETGID/SETUID/SETGID, file credential
checks, PROC_UMASK macro, UMASK syscall and RUN credential inheritance use
the new layout. PDP-6 native tests for packed identity/umask preservation and
the unchanged 0410-octal u-area layout pass. The FILE two-word descriptors
and full 36-bit offsets are unchanged. **Descriptor extension storage is
reserved but not yet used**; KINIT still publishes only four mounts.

Next update all FILE vnode readers/writers and FD lifecycle paths atomically,
including RUN fd-map inheritance, then solve the separate overfull PURE-swap
record. See `design/file-mount-packing-20261016-v18.md`.

An unused high credential bit is a possible home for PURE backing's fifth
mount-slot bit, keeping its 36-bit primary record; attachment/restore/release
lifetimes have not been migrated yet. The repacking adds nine permanent
words over v17: disk/tape last 036757/036141 octal. GCC lowmem 32K KCORE
end 023574 still exceeds 020000 by 1916 decimal words. Boot-image ends
disk/tape 072474/073433, leaving 1732/1253 words after stack reservation.

## 2026-10-16 VFS 26-SLOT POLICY AND SLAB PLANNER V17

The active 36-bit `vfs_mount_policy` word now uses 26 read-only bits (0..25),
five SWAP-owner bits (26..30), and five LOGSTORE-owner bits (31..35). Public
owner IDs 1..26 fit in five bits. Native PDP-6 tests exercise mount ID 26,
its RO flag, exclusive ownership/release, and unmount retirement; the prior
mount-16 regression and mount/storage/D6FS tests pass. No additional
resident words are required; disk/tape permanent last remain 036746/036130.
The six-array KINIT slab planner accepts up to 26 slots (156 dynamic words).
The staged inventory calculator no longer discards the possible seventeenth
device, although its currently described controllers provide at most 17
potential volumes. Full KINIT still allocates exactly four slots and publishes
`vfs_mount_limit=4`, intentionally: production support above four is NOT
enabled or boot-accepted yet.

Two compact runtime formats must be solved before raising the limit:
(1) FILE descriptors overlay metadata on the top three bits of the six-bit
vnode mount field, retaining only THREE mount-ID bits (`file.h` and
`file_unlock_mount`), and (2) resident PURE swap backing packs a full
18-bit index, four-bit mount slot, and 14-bit text length into 36 bits.
Keeping all three full ranges with five mount bits requires 37 bits and is
mathematically impossible in the present one-word swap format. Provider
limits including optional TENEX/ITS personality indexing also remain.
Do not enable >4 production mount slots until these are correctly migrated.

GCC lowmem 32K measurements are unchanged: KCORE ends 023563, failing the
020000 gate by 1907 decimal words; disk/tape KINIT ends 072463/073422,
leaving 1741/1262 words after reserved boot stack. No shell is required for
these size tests, and no 32K boot acceptance is claimed.

## 2026-10-16 COMPACT 16-SLOT VFS POLICY V16

The native VFS policy no longer uses four-slot RO/SWAP/LOGSTORE bit groups.
It retains **one** resident 36-bit word, aliased with the unused MonitorFS
TTY0 input counter: bits 0..15 are per-slot RO flags, bits 16..20 contain
the public SWAP owner mount ID (0=none), and bits 21..25 contain the public
LOGSTORE owner ID (0=none). The owner representation exploits the existing
exclusive reservation rule and avoids allocating three separate bitmaps.
All active users are converted: VFS publish/read/release/unmount/shutdown,
D6FS CLEAN/RW toggling, and FSINFO. The unlinked three-bitmap v2 policy
prototype is not the resident implementation.

Native PDP-6 tests pass for reservation lifetime, atomic rejection of
invalid release, remount CLEAN policy, slot reuse, a reduced runtime bound,
and a synthetic 16-slot table exercising public mount ID 16 (RO and both
exclusive owners, wrong-owner release, correct release, retirement).
Diagnostic disk-root SIMH ROOT login and ECHO also pass under the existing
test-only DSH size override. Production VFS capacity remains four until
the separate provider bounds and hardware-derived KINIT sizing are ready.

The more general owner checks increase permanent instructions by 25 words
over v15, despite adding zero policy data words: disk single last 036746,
multi last 037007, tape last 036130 (octal). GCC 32K KCORE end 023563
FAILS the 020000 gate by 1907 decimal words. KINIT disk/tape ends are
072463/073422, leaving 1741/1262 decimal words after the reserved stack.
Native 32K boot remains unaccepted. Preserve the compact one-word data
representation while optimizing the instruction paths in a later RAM sweep.

## 2026-10-16 MM-BACKED DTFS/TSFS MEDIA TABLES V15

KINIT now publishes all six MM-slab table bases: VFS target/root, D6FS
reader, shared DTFS/TSFS media, TSFS file shape, and TSFS extent locator.
The former `dtfs_media[4]` and TSFS file-shape/extent fixed arrays are no
longer linked. Active tape C/native assembly accesses the MM-backed tables;
PDP-6 TSFS multi-extent, D6LZ, nonzero-slot, and DTFS NATIVE/TENEX/ITS
chain-walk fixtures PASS. Tape-root without D6FS/BLOCKSET/BADMAP links,
and diagnostic disk-root ROOT login and ECHO PASS (DSH size override only).

RAM accounting is not a net saving yet: the new absolute base pointers and
access instructions increase disk-root permanent last from 036712 to 036715
(+3 decimal words), and tape-root last from 036060 to 036077 (+15 decimal).
GCC 32K lowmem KCORE end 023532 still FAILS the strict 020000 gate by 1882
decimal words. Boot streams build without shell: KINIT ends disk 072432,
tape 073371, leaving 1766/1287 decimal words after the 02000-octal KINIT
stack reserve. No 32K runtime acceptance is claimed. The active mount count
remains four until the live policy representation, provider bounds, boot
inventory and optional foreign-format personality indexing are migrated.

## 2026-10-16 KCORE LEGACY TABLE REMOVAL V14

Removed obsolete four-word VFS target/root arrays (eight words) and one
duplicate D6FS boot-reader slot. KINIT already publishes MM-backed VFS
tables and now transfers the D6FS initial reader directly from its active
reader word. Disk/tape size gates and disk-root diagnostic PDP-6 boot pass.
Nine 36-bit permanent words saved over v13: disk permanent last 036712,
tape 036060; lowmem KCORE end 023527, still FAILING the independent 020000
32K gate by 1879 decimal words. Other compatibility-looking paths remain
live, are unlinked, or cost zero words; details in
`design/kcore-legacy-audit-20261016-v14.md`. VFS remains limited to four
mounts until provider metadata and mount-policy migration completes.

## 2026-10-16 VFS D6FS READER SLAB AND 32K MEASUREMENTS V13

D6FS now uses the MM-backed slab reader array via `d6fs_reader_slots_base`.
MINIT stores its initial reader in a single fallback word; KINIT transfers
that pointer into the MM slab before publishing the new base. Native PDP-6
dispatch, multi-mount, remount, and fallback-poisoning tests pass. Diagnostic
disk-root SIMH ROOT login and ECHO pass under the existing test-only DSH size
override. DTFS/TSFS provider metadata and mount policy are not yet converted;
the active VFS mount limit is still four (planned maximum sixteen).

Normal permanent-size last addresses (octal): disk single 036723, disk multi
036764, tape 036071. Fresh GCC lowmem 32K builds without shell produce KINIT
ends 072422 (disk) and 073327 (tape), leaving 1774 and 1321 words after the
02000-octal stack reserve. KCORE end is 023540, failing the independent
020000 KCORE gate by 1888 decimal words in both profiles. KCORE+MRES is
12837/12705 decimal words (disk/tape). These are build/layout measurements,
NOT 32K boot acceptance. See daimos-testkit's versioned 32K footprint test.

## 2026-10-15 VFS LIVE MM TARGET/ROOT TABLES V12

KINIT now allocates the 24-word four-slot VFS slab from MM after MRES
installation and publishes target/root bases before root mount. Production
VFS, FSINFO, and PURE backing use the MM-backed target/root arrays; provider
metadata is still fixed and the live mount capacity remains four (maximum
planned sixteen). The source and complete KINIT image link succeed for the
dedicated disk-root profile. Fixed-symbol test regressions and a new PDP-6
test switching table bases to separate arrays pass. The disk-root permanent
last is 036721, multi-root 036762, and tape-root 036073, octal. Permanent
text grew 35 decimal words compared to v11; MM adds 24 slab words for four
slots, partly unused until provider migration. Remove redundant fixed tables
and optimize pointer hot paths in the final cutover, not by unsafe capacity
changes now. The module-table conditional DTFS import and explicit KINIT
object lists were corrected to make the real image link work.

Diagnostic disk-root PDP-6 SIMH boot reached ROOT login and ran an ECHO
command successfully. The documented DSH_IMAGE_LIMIT=037000 diagnostic
override was necessary because the unmodified production 036000 DSH gate
rejects the 036021 DSH image. This is NOT production-size acceptance.

## 2026-10-15 VFS SHARED RUNTIME SLOT LIMIT V11

The live four-slot VFS now reads one resident `vfs_mount_limit` word instead
of duplicating immediate four-slot bounds in native VFS and FSINFO assembly.
PURE swap attachment also validates against that limit. It starts at four
and cannot be raised until KINIT publishes MM-backed tables for all providers.
PDP-6 execution tests pass with a test-only reduced bound of two and confirm
overflow rejection, storage-reservation bounds, and mount-slot reuse.
Dedicated disk/tape permanent-size gates pass (disk single last 036656,
multi 036717, tape 036030, octal). The change adds three permanent words
over the v10 mount-width fix. Dynamic 16-slot KINIT activation is pending.

## 2026-10-15 SWAP BACKING MOUNT-WIDTH CORRECTION V10

Preparatory VFS work targets boot-configured mount capacity <=16. The active
one-word executable swap/PURE backing record previously encoded only four
mount slots. Its two provider bits have been reassigned to the mount field,
giving index:18, mount:4, PURE-text:14 while retaining exactly one word per
process. Native restore recovers the provider from the mounted root; swapped
PURE processes keep their backing mount busy. PDP-6 tests for raw/compressed
PURE swap and mount-16 reconstruction pass. The disk and tape dedicated-root
permanent-size gates pass; linked permanent footprint is four words smaller
than the preceding VFS mount-mutation v9 build. This does NOT activate VFS
mounts above four: dynamic slab callers and KINIT connection remain pending.

## 2026-10-14 LIBC STRING ASM/C SPLIT V1

DAIMOS libc now uses PDP-6 assembler for `memset`, `memcmp`, `strlen`,
`strcmp`, and `strncmp`, matching the existing KCC freestanding runtime
implementations.  Existing `memcpy` and `memmove` remain assembly.  Only
`strcpy`, `strncpy`, `strcat`, `strncat`, `strchr`, `strrchr`, `strtok`, and
`strcspn` remain in `userland/libc/string.c`.  The libc archive and the
standalone userland MAKE executable both build and link successfully with
this split.  This is a build/link validation, not a new SIMH functional
regression result.  The standalone semantic test for `strchr` still uses
C intentionally; no assembly `strchr` implementation exists.

## 2026-10-13 NEW D6 CODEC NATIVE ABI STAGING V055

Imported the native v053 D6 compressor/decompressor implementation and
the v054 all-producer cutover checklist from the completed codec branch.
Added an inactive AC10..AC16-preserving kernel calling adapter. Its
native PDP-6 test uses the independently recorded v051 darc golden stream
and verifies exact bitstream parity, decode and register preservation:
PASS. The old `000060` resumable decoder, stage-1 installation, image
builders, VFS/EXEC/TSFS consumers and kpack identity API are unchanged.
The v053 complete-buffer decoder cannot simply replace the resumable
fixed core without breaking the old API; a coordinated single-format
image cutover is still required before the user-requested production
activation. See `system/kernel/core/D6CODEC-KCORE-ABI-v055.md` and
`D6CODEC-INTEGRATION-TODO-v054.md`. This step adds no active KCORE words.

## 2026-10-13 REGISTRY-AWARE PACKED RESTORE V46

Staged `proc_packed_restore_transaction_v46` consumes the installed v38
dense/sparse descriptor layout by reconstructing the two-word source from
the MM-authoritative v37 lookup. While lifecycle TRANSITION is held it
pins the packed source, allocates/pins a new aligned resident extent,
zeroes omitted PURE words, reloads executable backing before decoding the
writable suffix, and releases packed MM only after all fallible I/O succeeds.
MM release is the commit point; the matching registry entry and PACKED mode
are retired before VM base publication and TRANSITION release. On a failed
decoder, backing mismatch or MM source-free refusal, the registry, packed
ownership and storage mode remain intact and the destination is freed.

PDP-6 GCC/DAS standalone object assembles at 209 pre-fold text words and
zero BSS. `daimos-testkit/tests/system/packed-restore-v46` passes dense
and sparse success and failure coverage, including PURE-before-writable
ordering. Existing v31 restoration and v45 eviction regressions pass.
The production VM/scheduler and KINIT link remain unchanged. Next test a
complete v45->v46 cycle against one registry with allocation/pin/free fault
injection, then evaluate scheduler entry points and complete linked size.
Do not remove disk swapping or integrate the production codec prematurely.

## 2026-10-13 REGISTRY-AWARE EVICTION TRANSACTION V45

Added inactive `proc_packed_evict_transaction_v45` (separate from the
existing v32 API to preserve previous test coverage). Under TRANSITION
it preflights the configured dense/sparse registry, process state, owned
resident MM extent, and absence of a pre-existing packed extent. It then
pins the resident allocation, allocates and pins packed storage without
reclaim recursion, invokes a caller-provided encoder, and rejects a
nonshrinking or incorrect encoded length. After source MM free, it sets
PACKED mode and zero VM base, publishes the MM-authoritative registry
entry via v44 while TRANSITION is still held, and only then unpins packed
storage and clears TRANSITION. An error after source release is fatal and
leaves TRANSITION set; no scheduler may run such a process. Precommit
failures leave resident ownership and mode intact.

PDP-6 GCC/DAS standalone object: 251 pre-fold text words, no BSS.
daimos-testkit `packed-evict-v45` passes dense/sparse transaction and
Stage 43 release, synthetic lossless pair compression, encoder failure,
nonshrinking output refusal, sparse registry exhaustion, and repeated
slot cleanup. This is a host test, NOT a SIMH scheduler acceptance test.
The original v31 restore still consumes a private descriptor rather than
the selected registry, so scheduler integration and active KCORE linkage
remain disabled. Codec integration and removal of disk swapping remain
deferred. The last measured active KCORE endpoint remains 021541 octal.

## 2026-10-13 MM-AUTHORITATIVE PACKED PUBLICATION V44

The inactive `proc_packed_publish_v44.c/.h` prototype complements compact
v43 teardown using the v38 boot-selected dense/sparse layout. It requires
PACKED mode, a zero resident VM base, lifecycle TRANSITION, a valid prepared
layout, and exactly one MM_TYPE_PACKED_PROCESS extent with matching owner,
base and length. It refuses duplicates and sparse capacity exhaustion
without mutating the registry. Once validation succeeds, it commits the
layout word and (for sparse) its slot index; these are serialized operations
with no remaining fallible steps. The descriptor span stays MM-authoritative.

PDP-6 GCC/DAS standalone compilation passed at 137 text words before
folding (0 BSS). daimos-testkit `packed-publish-v44` passes composed boot,
publish, lookup, release, failure rollback, and slot reuse against both
layout formats. Its companion v43 release is 166 text words standalone;
these are NOT full-image savings and do not include codec, restore, or
KINIT linkage. Active KCORE and boot behavior are unchanged.

IMPORTANT integration mismatch: staged v32 eviction clears TRANSITION on
success before the v44 publication API could run; staged v31 restoration
consumes/clears a private two-word descriptor rather than the selected
registry layout. Before scheduler integration, refactor the transaction
commit boundaries so the packed registry is published while TRANSITION is
held and the restore cleanup retires the same registry entry atomically.
Do not weaken the ownership checks or publish PACKED mode without an
installed descriptor. Disk swap remains operational; production codec
integration remains deferred. The previous live KCORE endpoint was 021541
octal and has not been remeasured by this inactive stage.

## 2026-10-13 COMPACT MM-AUTHORITATIVE PACKED RELEASE V43

The inactive `proc_packed_release_v43.c/.h` prototype performs the packed
teardown commit directly against one MM_TYPE_PACKED_PROCESS extent for the
slot, plus the KINIT-selected v38 dense/sparse layout. It does not construct
or retain an independent packed span descriptor. The routine validates the
explicit PACKED mode, TRANSITION, absent resident VM base, nonempty and
valid writable layout, exact single MM owner, and unique sparse slot entry.
Only after a successful MM free does it clear the selected layout and the
two-bit storage mode. Any MM pin/ownership failure preserves both.

PDP-6 GCC/DAS standalone build: 166 text words before folding, no BSS.
The prior v37 shared owner object was 197 words and v39 guard was 49 words;
these figures are NOT a whole-image saving, since v43 still requires the
boot registry, mode-map, layout and publication machinery. The new testkit
`packed-release-v43` passes both dense and sparse boot-to-teardown lifecycles,
including duplicate sparse entry, ambiguous MM owner, missing TRANSITION,
nonzero VM base, pinned free refusal, MM free failure, and slot reuse.

No live KINIT registry allocation or new KCORE linkage is enabled. The
Stage 41 fail-closed destructor remains active; its last measured endpoint
is 021541 octal. The next implementation decision must compare complete
linked KCORE and total dynamic RAM for an integrated release/publication
path, not merely the isolated object sizes. Production codec and packed
eviction remain deferred.

## 2026-10-13 PACKED BOOT/TEARDOWN INTEGRATION TEST V42

Added a combined host regression in daimos-testkit
`tests/system/packed-boot-teardown-v42`. It runs the existing real
v36/v37/v38/v39 C code in one process, with a controlled MM mock: allocation
and boot-publication of a dense or sparse registry, failed boot allocation,
boot abort and reinitialization, publication and lookup of an MM-owned
packed extent, rejection of wrong ownership or missing lifecycle exclusion,
refusal to release pinned or ambiguous extents, successful packed teardown,
slot reuse, and rejection of registry abort while an entry is live. A failed
registry MM free retains the runtime pointer; a successful abort clears it.
Both 24-slot configurations pass: dense at 24 descriptor words and sparse
with four entries at eight descriptor words. This test establishes the
staged interfaces can be composed without integrating a production codec.

The live GCC 32K image is NOT modified by v42: the Stage 41 VM destructor
continues to fail closed on PACKED rather than treating it as disk-backed.
No additional KCORE or runtime allocation is linked. Connecting the
registered descriptor path to production VM teardown, process restoration,
and KINIT remains blocked on measurable permanent-memory cost and complete
runtime failure-injection coverage. Production codec and scheduler eviction
remain deferred. The most recent measured active KCORE endpoint is 021541
octal (v41), above the required 020000 boundary.

## 2026-10-13 FAIL-CLOSED LIVE VM DESTROY V41

The live PDP-6 `vm_space_destroy` now calls the existing resident
`proc_storage_expect_v30` before any MM free or disk-swap detach. It accepts
RESIDENT and DISK storage only; DISK additionally requires a zero VM base.
PACKED, invalid, or missing map modes fail before changing MM ownership,
the swap record, or the process VM descriptor. This is a minimal safety
boundary while packed-registry activation remains postponed. The corrected
v40 failed-creation path and the exit path both hold TRANSITION during VM
destroy. The future packed teardown handler must be installed together
with boot registration before enabling any PACKED state publication.

The low-memory GCC three-member TSFS image built successfully and booted
in SIMH to DTFS LOADED / INIT V1 without observed HALT or ?RT. On the
current working tree the linked KCORE endpoint moved from v40's 021525
to 021541 octal: **12 decimal resident words** of growth. This does not
solve the hard KCORE shrink target; it is a fail-closed correctness guard.
Existing daimos-testkit packed-teardown-v39, packed-owner-v37 and
proc-abort-transition-v40 host tests still pass. Runtime failure injection
for a deliberately PACKED process is not yet tested; boot smoke covers only
the normal RESIDENT/DISK paths. Full dense/sparse registry code is not
linked into KCORE because that would add substantial permanent code before
packed eviction exists. Production compressor/decompressor integration
remains deferred.

## 2026-10-13 PROCESS ABORT LIFECYCLE TRANSITION V40

The live C `proc_slot_discard()` used to enter `vm_space_destroy()` without
holding `PROC_F_TRANSITION`, unlike the live assembler `proc_finish_slot`
exit path. Since packed teardown requires lifecycle exclusion, that would
make aborted process creation incorrectly reject PACKED cleanup. The abort
path now holds TRANSITION across VM destruction and clears it if VM release
fails; on success the slot is still cleared through its existing teardown.
The live packed registry and teardown hook are NOT activated by this patch.

Testkit `proc-abort-transition-v40` compiles the real `proc.c` against an
instrumented VM destructor and verifies transition-held destruction,
failure rollback, successful cleanup, and no duplicate destruction of a
FREE slot. PDP-6 GCC/DAS compile passed; lowmem GCC three-member TSFS
image built and booted in SIMH to DTFS LOADED / INIT V1 without a reported
HALT or ?RT. The measured KCORE endpoint for this working tree is 021525
octal, versus the previous 021520 snapshot; the tree also contains other
agents' uncommitted boot changes, so do not attribute all five words to
this change without an identical-tree A/B comparison. The 32K target is
still unmet. Codec, packed eviction and KINIT packed registry activation
remain deferred. The next step is paired boot registration and VM destructor
wiring, with a size gate that avoids excessive permanent KCORE growth.

## 2026-10-13 GUARDED PACKED PROCESS TEARDOWN V39

Added an inactive `proc_packed_teardown_guard_v39` interface for the common
VM destruction boundary used by normal process exit and failed-creation
cleanup. It requires process lifecycle TRANSITION, examines the explicit
two-bit per-slot storage mode, and leaves RESIDENT or DISK teardown to the
existing VM code. PACKED requires an installed, matching descriptor layout;
the guard invokes the common v37 MM-authoritative release and returns an
error if ownership, MM pins, or metadata are inconsistent. In that case
the caller MUST NOT clear the VM or reuse the process slot. An invalid
storage mode also fails closed. There is no compressor dependency.

PDP-6 GCC/DAS standalone build passed (49 text words before folding).
Testkit `packed-teardown-v39` passed for dense and sparse layouts, checking
PACKED cleanup and reuse, failed pinned release, absent registration, and
RESIDENT/DISK/invalid discriminators. The guard is NOT currently called
from `vm_space_destroy` or linked into production KCORE: KINIT has not yet
installed a persistent packed registry, and installing the hook alone would
leave an unusable PACKED path. No active KCORE or resident RAM savings
are claimed. Codec integration remains deferred.

## 2026-10-13 PACKED LAYOUT BOOT PREPARATION V38

Added an inactive transient-boot allocator for v36 dense/sparse descriptor
layouts (`proc_packed_boot_v38.c/.h`). It accepts configured slots, maximum
simultaneously packed processes and a descriptor-word budget supplied after
essential allocations, chooses the smaller feasible representation using
the existing v36 planner, then allocates exactly one zero-initialized
MM_TYPE_KERNEL_DYNAMIC extent. The four-word runtime descriptor is only
published once initialization succeeds. The boot preparation is explicitly
opt-in: production KINIT and process exit remain unchanged until complete
packed ownership, rollback and teardown paths are installed. Abort rejects
occupied slots and preserves ownership if MM refuses the free.

PDP-6 GCC/DAS standalone object compiles and assembles (100 text words,
4 BSS words before folding); the host `packed-boot-v38` test covers budget
and allocation failures, sparse and dense selection, duplicate setup,
occupied-table abort refusal, MM free refusal and successful reinitialization.
The v36 layout and v37 ownership tests also pass. These are isolated host
tests; no boot/runtime KCORE size or packed process acceptance is claimed.
Next: connect teardown, aborted creation and slot reuse consistently with
the enabled registry, then add production boot activation without making
the 32K permanent KCORE larger merely to support larger systems. Production
compression and decompression integration remain explicitly deferred.

## 2026-10-13 SHARED DENSE/SPARSE PACKED OWNERSHIP V37

Added an inactive single MM-authoritative ownership engine for either v36
descriptor layout (`proc_packed_owner_v37.c/.h`). It reconstructs packed
physical base/length from exactly one MM_TYPE_PACKED_PROCESS extent owned
by the process slot, validates process transition and PACKED discriminator,
and publishes only the writable layout. Teardown rejects pinned extents,
invalid ownership and an inconsistent mode without freeing the descriptor;
successful MM release clears the selected layout and resets the slot's
storage mode to RESIDENT. Neither operation allocates memory or calls a
compressor. This removes the need for separate duplicate MM validation
engines when selecting dense versus sparse in a future boot policy.

PDP-6 GCC/DAS standalone compilation passed (197 text words before folding
for the ownership engine, excluding v36 layout and other dependencies).
The new testkit `packed-owner-v37` passes for both layout formats, covering
transition/ownership checks, duplicate publication, pinned extent rollback,
ambiguous MM extent rejection and slot reuse. This is still inactive:
KINIT does not allocate a descriptor layout, and process exit/abort paths
do not call these functions. Activating a teardown hook before a supported
registry exists would be unsafe. No compressor integration, active KCORE
growth or saving is claimed. Next: define an explicit inactive-to-active
boot registration contract, then wire all teardown paths at the same time
as activating storage publication and checked restore descriptor consumption.

## 2026-10-13 BOOT-SELECTABLE PACKED LAYOUT PRIMITIVE V36

Added an inactive common descriptor-layout primitive for the dense/sparse
choice. `proc_packed_layout_plan_v36` chooses the smaller feasible data
representation given the configured process-slot count, expected maximum
packed concurrency, and caller-provided descriptor-word RAM budget. Dense
uses one layout word per slot; sparse uses two words per reserved entry
(slot + layout). Tie goes to constant-time dense lookup. The layout routines
implement bounded reservation, lookup, clearing, and slot reuse with no
dynamic allocation, MM mutation, or compiler/compression dependency.

PDP-6 GCC/DAS standalone object: 232 text words before folding. Testkit
`packed-layout-v36` passes layout selection, invalid/budget cases, sparse
exhaustion, duplicate reservation, clearing and slot reuse in both modes.
This is not yet the single shared ownership/teardown engine, and KINIT does
not allocate or select it in the production image. v35 remains a separate
MM-authoritative ownership prototype. Do not claim a 32K resident saving or
turn on packed eviction before lifecycle publication/teardown is unified
and the codec passes target execution tests.

## 2026-10-13 MM-AUTHORITATIVE PACKED DESCRIPTORS V35

The inactive `proc_packed_mm_registry_v35` prototype stores only the two
pieces absent from MM: a slot ID and the retained writable layout. Physical
base and encoded extent length are recovered from exactly one
MM_TYPE_PACKED_PROCESS extent belonging to that slot. Lookup rejects a
duplicate MM owner or duplicate registry slot rather than guessing which
descriptor is authoritative. Publication and discard require lifecycle
exclusion, PACKED mode, matching physical extent ownership, and the
process transition bit. MM release failure leaves the descriptor and mode
unchanged; successful release clears the two-word entry and storage mode.

The PDP-6 GCC/DAS standalone object assembles to 301 text words before
folding; the prior v34 sparse design used 277. Each v35 sparse entry uses
two runtime data words rather than v34's three, but the extra code means
there is no proven overall permanent RAM win. A dense layout-only table
would require one word per process slot but has not been implemented.
The daimos-testkit v35 host tests cover publication, duplicate entries,
ambiguous MM owner, wrong owner, pinned free rejection, teardown, and slot
reuse. No packed codec integration, KINIT allocation, process exit hook,
or KCORE link is enabled. The last measured live KCORE endpoint remains
021520 octal, pending a fresh whole-image build after other agents' work.

## 2026-10-13 SPARSE PACKED DESCRIPTOR REGISTRY V34

An inactive alternative to the dense v33 registry is now available as
`proc_packed_sparse_v34.c/.h`. Its caller-supplied bounded table holds only
three words per reserved packed-process entry (slot ID and the existing
two-word descriptor), not two words for every configured process slot.
Each entry is stable until explicit teardown. Publication validates MM
ownership and rejects duplicate slot claims; disposal checks ownership and
pin state before freeing the extent, clearing PACKED mode and making the
entry reusable. A full table returns MM_ERR_DESCRIPTORS. Calls are permitted
only under process-lifecycle exclusion; no allocations occur in the path.

GCC/DAS PDP-6 standalone assembly passed: 277 text words versus the v33
dense-registry object's 192, prior to folding. For a 24-slot machine the
dense table requires 48 descriptor words, while an eight-entry sparse table
would require 24 words. The sparse table is not necessarily superior: it
adds code, linear lookup, and a configured maximum simultaneous packed count.
Neither implementation is linked into KCORE or allocated by boot yet.
daimos-testkit `packed-sparse-v34` exercises publication, duplicates, MM
ownership mismatch, pinned release refusal, successful cleanup and reuse.
Do not activate packed eviction or integrate the under-development codec.

## 2026-10-13 PACKED DESCRIPTOR LIFETIME REGISTRY V33

Added a separate, inactive slot-indexed packed descriptor registry. Each
descriptor holds two 36-bit words. The owner checked at registration and
teardown is the MM extent type, owner slot, exact physical base and length;
registration rejects already-filled slots and requires PACKED mode plus the
existing process transition exclusion. Teardown checks the same ownership,
refuses pinned extents and preserves the descriptor and PACKED mode on MM
release failure; successful release clears both descriptor words and resets
the mode to RESIDENT for safe reuse of the slot. The registry consumes no
permanent RAM until enabled and allocated for the runtime slot count.

PDP-6 GCC/DAS standalone compilation passed. daimos-testkit
`packed-lifetime-v33` tests duplicate publication, pinned-release refusal,
successful cleanup, and rejection of MM ownership mismatches; the test
passes. These APIs are not yet linked into active KCORE or attached to
vm_space_destroy / process exit, because scheduler PACKED processes and the
production codec are not enabled. Before enabling packed eviction, allocate
the registry at boot, bind the existing v32/v31 descriptor argument to its
slot entry, and ensure all process destroy/abort/slot reuse paths invoke
checked descriptor teardown. No KCORE saving is claimed.

## 2026-10-13 PACKED PROCESS EVICTION TRANSACTION V32

Staged the complement to v31 restore: `proc_packed_evict_transaction_v32`
requires lifecycle exclusion plus PROC_F_TRANSITION, an exactly owned
MM_TYPE_PROCESS extent, RESIDENT mode, and an unpublished descriptor. It
allocates packed MM storage without recursive reclaim, pins the live source
and encoded destination while encoding, and refuses encoded capacities that
do not actually reduce the retained writable suffix. For a successful
fixed-capacity lossless encoder the reported encoded length must exactly
match the allocated span; failure leaves the live VM base, storage mode,
and packed descriptor unchanged. After the source extent is freed, the
descriptor and PACKED mode are published under the transition flag.

The testkit `packed-evict-v32` exercises encoder failure rollback,
non-shrinking rejection, and a synthetic lossless pair-encoding
eviction/restoration round trip with PURE reload. The v31 restore test
continues to pass. The v32 PDP-6 compiler/assembler build succeeded;
the complete inactive packed-process object has 608 pre-fold text words.
Neither eviction nor restoration is linked into active KCORE. The test
codec is only a controlled fixture, NOT a production compressor or a
general-purpose way to pack arbitrary process state. Scheduler integration,
full PURE ownership/pinning, descriptor lifetime across process exit,
real codec benchmarks and SIMH runtime tests remain prerequisites to
removing disk swap. No resident KCORE saving is claimed.

## 2026-10-13 PACKED RESTORE PURE TRANSACTION V31

An inactive `proc_packed_restore_transaction_v31` now performs a complete
prepare/reload/decode/commit sequence under caller-held process lifecycle
exclusion and PROC_F_TRANSITION. It validates the PACKED storage discriminator,
exact source MM ownership, and executable backing, then pins the packed source
and the new APR-aligned destination. It zeroes the omitted prefix, reloads
PURE backing (including compressed DXR that may overwrite the entire image),
then decodes the saved writable suffix. It does not publish process residency
or free packed storage on ordinary decode/reload failure. After successfully
freeing packed storage it publishes VM base and RESIDENT mode before dropping
the destination pin and transition flag. Fatal late MM failures retain the
transition flag, rather than treating a partial commit as runnable.

PDP-6 GCC/DAS standalone build succeeded (438 total text words for the entire
unlinked packed-process object). Host testkit `packed-restore-v31` checks
decode rollback, incorrect executable backing rejection, and PURE-reload-first
ordering when the reloader overwrites the entire destination image. The host
test uses explicit PDP-6 word-address to native byte-address translation.
No KCORE size change or active RAM saving is claimed: the v31 object remains
unlinked. Packed eviction, descriptor lifetime/discriminator integration,
scheduler transactions and PDP-6 SIMH runtime failure tests are not complete.
Do not remove disk swap or enable PACKED process scheduling yet.

## 2026-10-13 DISK SWAP STORAGE-MODE TRANSITIONS V30

The existing PDP-6 assembler `proc_swap_out` and `proc_swap_in` now check
the live two-bit storage map before beginning disk swap transactions, then
publish RESIDENT->DISK or DISK->RESIDENT after successful MM/storage metadata
updates and before clearing PROC_TRANSITION. A late impossible mismatch halts
with the transition still held rather than releasing an incoherent process.
`proc_swap_detach` also clears DISK mode on nonresident-process destruction.
The transition helpers use short PDP-6 assembly with no added process frames
or allocations. No PACKED state is published; no packed eviction is enabled.

The GCC lowmem ROOT=tape BOOT=dtc three-member image builds and reaches
`DTFS LOADED` / `INIT V1` in SIMH with no observed ?RT or HALT. KCORE endpoint
is 021520 octal versus 021412 in the preceding v29 build; these correctness
guards add 70 words after folding. The 100-word planning reserve is a budget,
not permission to treat growth as optimization. Dedicated disk-backed
swap-out/in/detach runtime transitions and failure injection remain untested;
TSFS boot smoke does not exercise them. Leave the packed-process transition
disabled until PURE reload, mode publication and MM release are atomic.

## 2026-10-13 LIVE STORAGE MAP BOOT ALLOCATION V29

The existing proc_swap_boot_init() dynamic swap-record MM extent now also
contains a compact 2-bit-per-slot storage-state map after the backstore
bitmap. It is zero-initialized with the allocation, published via the
resident two-word proc_storage_runtime_v29 descriptor, and requires no
additional MM extent or fixed 256-slot table. At 24 slots, this consumes
two extra dynamic words and two KCORE words for the pointer and slot count.
The GCC lowmem ROOT=tape BOOT=dtc three-member TSFS image built and booted
to `DTFS LOADED` and `INIT V1`. Linked KCORE endpoint is 021412 octal,
versus 021410 in the prior same-tree measurement. The map remains
read-only RESIDENT state in production until transitions can be integrated
with the scheduler and disk swap consistently. No packed eviction is enabled;
do not describe this as an optimization saving or passed login test.

## 2026-10-13 PACKED PROCESS STORAGE DISCRIMINATOR V28

Added an inactive 2-bit-per-process storage-state map with exact slot-count
capacity: 24 lowmem slots need two 36-bit words; 256 slots need 15 words.
States are RESIDENT, DISK, PACKED, and one invalid encoding. This avoids
overloading the legacy swap backing record or the two fully occupied process
metadata flag bits. The API supports caller-owned zero-initialized storage,
bounded lookup, and compare-and-change with validation. Callers must hold the
process lifecycle lock/exclusion: the map itself is not an atomic primitive.
PDP-6 GCC/DAS standalone compilation passed (93 text words). It is not yet
allocated at boot or wired into the scheduler; therefore no KCORE or process
RAM saving is claimed. The descriptor and complete PURE reload/commit
transaction, as well as tests in sibling daimos-testkit, remain outstanding.

## 2026-10-13 PACKED RESTORE MM OWNERSHIP CHECK V27

The inactive packed-process restore preparation now validates that the
compressed source extent has the descriptor's exact physical base and word
length, MM_TYPE_PACKED_PROCESS, and the requested slot as MM owner before
pinning/decoding it. `mm_pin` alone checks existence, not type or ownership;
without this validation a stale or foreign descriptor could read another
MM allocation. The source remains protected by its pin while preparing the
aligned destination. PDP-6 GCC and DAS compile the isolated object (245
text words), but it remains unlinked from KCORE. This is a correctness
prerequisite, not a KCORE saving; full failure-injection tests in testkit
and an explicit lifecycle storage discriminator are still required.

## 2026-10-13 PACKED PROCESS RESTORE PREPARATION V26

An inactive `proc_packed_prepare_restore_v26()` API now allocates a fresh
APR-aligned process extent without invoking recursive reclaim, pins the
packed source across allocation/decoding, and decodes its retained writable
suffix through a caller-provided codec. Decode failures release the new
extent and leave the packed descriptor unchanged. The prepared destination
stays MM-pinned because the unpublished VM base cannot safely be discovered
by compaction. `proc_packed_abort_restore_v26()` unpins and frees it.

This is *not* a complete process restoration transaction: the PURE prefix
still requires executable backing reload, and commit must atomically install
the VM base/storage discriminator and release the packed source and destination
pin. The identity codec is not used for eviction. PDP-6 GCC/DAS standalone
assembly succeeds (206 text words for the whole inactive proc_packed object).
No new KCORE code is linked and no runtime RAM or size savings are claimed.
The planning target continues to reserve 100 extra KCORE words above the
current measured state. Follow-on behavioral and failure tests belong in
daimos-testkit before integration into the live lifecycle.

## 2026-10-13 GCC KCORE SWAP DETACH CONSOLIDATION V24

Consolidated the resident executable-backing clear and legacy nonresident
disk-swap release into the existing `proc_swap_detach()` implementation.
The conditions and release order are preserved; no packed-process eviction
is enabled, and the lifecycle and scheduler remain resident. A paired GCC
lowmem ROOT=tape comparison with identical other working-tree inputs
measured 021416 -> 021410 octal KCORE, six words saved after P10fold.
The complete three-member TSFS image build also succeeds. Runtime swap
detach/restore regression tests and interactive DTFS/TSFS boot acceptance
remain outstanding; do not treat a successful link as runtime acceptance.

## 2026-10-13 GCC 32K THREE-MEMBER TSFS MEDIA V21

The PDP-6 boot image builder now accepts TSFS_ROOT_MEMBERS=1,2,3. A
versioned media-layout stamp regenerates TSFS metadata when the member
count changes. In the BOOT=dtc configuration, Stage1 boots from DTC0 and
the system TSFS members occupy DTC1..DTC3. The SIMH boot template exposes
all four DECtape drives. A fresh GCC lowmem ROOT=tape BOOT=dtc
TSFS_ROOT_MEMBERS=3 MONITORFS_MRES_ENABLED=1 image loads under 32K SIMH,
installs DTFS/TSFS, and reaches INIT V1. KCORE endpoint is still 021375.

Important: this is a **three-member TSFS system boot smoke**, not the
complete requested physical four-drive steady-state arrangement. DTC0
currently holds the separate bootstrap image, so a fourth writable DTFS
`/home` volume cannot occupy a drive simultaneously. A correct solution
needs an in-place bootstrap format coexisting with a filesystem tape (or
another explicitly approved bootstrap medium). Avoid automatic media swaps
or overwriting the user's writable DTFS tape. LOGIN and /home mount tests
remain outstanding. Do not claim the 3+1 acceptance complete.

## 2026-10-13 GCC 32K TSFS ROOT BOOT RESTORATION V20

Decoupled TSFS boot and DTFS/TSFS MRES installation from KINIT_FULL using
KINIT_TAPE_BOOT (derived from TAPE_FS_ENABLED). GCC lowmem ROOT=tape now
assembles the TSFS discovery/mount path, DTC/MTC MINIT entries and the
filesystem MRES package without enabling unrelated full-profile modules.
The DTC Stage1 loader accepts a zero compressed-word count and loads the
raw lowmem KINIT stream directly at 030000, avoiding an image-plus-staging
copy exceeding 32K. BOOT=auto selects DTC for ROOT=tape; lowmem permits
BOOT=dtc. The lowmem MRES source link order matches the MINIT traversal
(TAPE then DSK then DTFS); an earlier reversed order halted at the
module_run_minits package monotonicity check.

Fresh GCC lowmem ROOT=tape BOOT=dtc builds with resident MonitorFS and with
MONITORFS_MRES_ENABLED=1 both boot a single-member TSFS root successfully
through the `DTFS LOADED` message to `INIT V1` under PDP-6 SIMH 32K.
Both variants pass the new daimos-testkit TSFS v20 INIT boot smoke test.
The GCC MRES configuration reports 12612 decimal words of permanent
KCORE+MRES (end 030504 octal) and `__kcore_low_end=021375` octal.
The MonitorFS-MRES KCORE endpoint remains 021375 octal (resident MonitorFS:
023360). LOGIN/MonitorFS VFS acceptance is not yet established. The image
builder currently creates a one-member TSFS root on DTC1, with a separate
DTC0 bootstrap tape. It does NOT yet satisfy the desired four-drive layout
(three TSFS system tapes plus a separate DTFS /home tape). That media/boot
layout and interactive login tests remain required before declaring the
32K DECtape configuration complete.

## 2026-10-13 LOWMEM LOGIN DIAGNOSTIC V19

The MonitorFS v17 testkit runner omitted DCS transport arguments although
the lowmem inittab starts LOGIN on DCS line 0, not CTY. The runner is being
corrected to request TCP relay and DCS port 1101. A manual baseline rerun
with DCS attached established a DCS connection but halted with `?RT` at
PC 062054 before LOGIN. Therefore the prior CTY-only timeout was a harness
error; the new early-boot fatal is a separate unresolved blocker. Do not
claim runtime MonitorFS acceptance. Both root/media state and KINIT boot
failure handling must be checked before attributing this to the MRES change.

## 2026-10-13 MONITORFS GCC BASELINE BOOT COMPARISON V18

A fresh GCC lowmem image was rebuilt with MONITORFS_MRES_ENABLED=0
(original fixed resident MonitorFS); KCORE endpoint is 023360 octal.
The existing daimos-testkit MonitorFS v17 SIMH harness reports the SAME
pre-LOGIN stall as the experimental MONITORFS_MRES_ENABLED=1 image: both
reach INIT V1 and print `INIT: WARNING: MEMFS NOT MOUNTED ON /TEMP`, but
neither reaches LOGIN within the harness timeout. This isolates the LOGIN
blocker from the optional MonitorFS relocation: the observed failure is
not unique to the MRES configuration. It does NOT prove that MonitorFS
MRES runtime VFS behavior works; those post-login tests remain blocked.
Do not enable MonitorFS MRES by default until the baseline LOGIN issue
is diagnosed and the full acceptance probes pass.

## 2026-10-13 MONITORFS MRES BOOT-INSTALL FIX V17

The experimental GCC lowmem MonitorFS MRES configuration now builds a
complete boot image and reaches `INIT V1` on a fresh 32K PDP-6 simulator.
It remains opt-in (`MONITORFS_MRES_ENABLED=1`) until the MonitorFS namespace
probes pass after user login. The 32K and diagnostic 64K lowmem harnesses
both reached INIT but did not obtain a LOGIN prompt within their timeout;
therefore live MonitorFS VFS semantics are NOT yet accepted.

The runtime failure in v16 was diagnosed in two stages: the MRES package
source must appear in ascending MINIT traversal order, and a D6FS MINIT
dynamic 14-word MM allocation prevents a later MRES package from occupying
the required next contiguous address. The correct solution separates early
`mfsdev_install_minit` (after CLK, before dynamic allocations) from the
original late `mfsdev_minit` name publication (after driver discovery).
The MRES package itself appears immediately after CLK in the boot image.

The initial C symbol aliases for writable jump words generated invalid
GCC assembly operands; dedicated ordinary external assembly labels now
provide exact patch addresses. An unnecessary static pointer table that
emitted an unsupported `.long` directive was also eliminated. Build-mode
stamps ensure switching MONITORFS_MRES_ENABLED between 0 and 1 rebuilds
the correct KINIT and KCORE outputs in the same build directory.

GCC lowmem KCORE link is still 021375 octal (versus 023360 resident),
saving 01763 octal (1011 decimal) low-core words. MonitorFS MRES occupies
1030 words, so *net permanent RAM increases by about 19 words*; the
020000 octal KCORE hard target still requires 01376 octal more saving.
The revised size report handles the selected lowmem MRES set. The full
image was built after installing authoritative KCC/DAIMOS toolchain
components under PDP10_PREFIX. Five versioned MonitorFS acceptance probes
and a fresh-build test target are in sibling daimos-testkit; probe syntax
checks pass, but behavioral tests remain blocked before LOGIN.

## 2026-10-13 MONITORFS GCC MRES INTEGRATION V16 (BOOT PENDING)

The opt-in MONITORFS_MRES_ENABLED=1 configuration substitutes nine KCORE
dispatch words for the original MonitorFS implementation. An MRES package
is installed at the final MINIT entry and each VFS-facing jump is patched
to its relocated export. The package comes from the original assembly source.

GCC KCORE low end: 023360 -> 021375 octal, a reduction of 01763 octal
(1011 decimal) words. The installed MonitorFS MRES requires 1030 words,
so total permanent RAM increases by roughly 19 words; this is not net
memory recovery. KINIT image linking succeeds. The profile-aware
permanent-size report gives KCORE+MRES 030704 octal, PERMANENT_LAST
030763 octal. The KCORE 020000 limit still needs 01376 octal more savings.

Full image generation and SIMH testing remain BLOCKED: the external DAS
native self-build manual-check requires sixmd-check, which is absent from
the configured tool prefix. The MRES configuration is opt-in until those
tests validate boot and MonitorFS semantics.

## 2026-10-13 MONITORFS MRES LINK PROBE V15

The standalone MonitorFS MRES link probe succeeds under DAS/DLINK against
the current GCC KCORE map. tools/host/monitorfs-mres-probe-v15.py derives
the probe from the sole authoritative assembler source and renames public
entries for collision-free linking with resident MonitorFS. The resulting
MRES contains 1030 initialized words and zero BSS. This does NOT install
the module or change KCORE residency; MINIT and VFS dispatch integration,
real boot validation, and total permanent-memory accounting remain open.

## 2026-10-13 MONITORFS MRES DISPATCH STAGING V14

Added an inactive nine-entry VFS-to-MonitorFS jump interface in
fs/monitorfs_dispatch_v14.s. Each one-word entry fails safely until MINIT
installs its relocated destination. This is staging source only: it is NOT
linked into KCORE and does not replace the working resident MonitorFS.
Next wire the pinned MRES package, exports and MINIT patching as one atomic
change, then boot-test VFS before removing the resident implementation.

## 2026-10-13 MONITORFS MRES DEPENDENCY ANALYSIS V13

See design/monitorfs-mres-extraction-v13.md. The 1030-word GCC MonitorFS
object is referenced directly from VFS; its MINIT table entry currently has
no MRES package. Moving it needs resident dispatch and pinned service
lifetime. No resident code is removed in this stage, no boot regression is
claimed, and the GCC KCORE endpoint remains 023360 octal.

## 2026-10-13 GCC KCORE REPEATABLE SIZE AUDIT V12

Added tools/host/kcore-size-audit-v1.sh to report a selected fixed-KCORE
object size breakdown from a GCC build independently of the folded link
endpoint. Current endpoint remains 023360 octal. Large savings require
structural work in process control, file/MonitorFS runtime, and disk-backed
swap; the DSK/DRM interface leaf objects are only 37 decimal words combined
before folding. This stage does not change kernel code or claim any savings.

## 2026-10-13 GCC MONITORFS FALLTHROUGH CLEANUP V11

Removed four unconditional jumps to immediately following local labels in
monitorfs_runtime.s. The folded GCC lowmem KCORE link moves from 023361
to 023360 octal: a verified saving of one permanent word after folding.
This is intentionally a small semantics-preserving assembly cleanup; no
filesystem functionality was disabled. Full link passes. Runtime tests
remain outstanding. Next savings must come from larger structural splits.

## 2026-10-13 STRICT GCC 32K KCORE ACCEPTANCE GATE V10

Run make kcore-32k-gate BOOT_PROFILE=lowmem COMPILER=gcc with a clean
GCC-specific build directory. The dedicated check rejects missing/invalid
link-end symbols and requires __kcore_low_end strictly below 020000 octal.
Ordinary development builds intentionally remain possible above the limit.
The gate tests KCORE only; MRES, process availability and actual boot require
separate acceptance tests. The active GCC baseline remains 023361 octal.
The GCC object-level triage is in design/kcore-gcc-size-audit-v10.md;
proc_pdp6, file_runtime and monitorfs_runtime are the largest candidates.

## 2026-10-13 32K GCC KERNEL BUILD POLICY V9

The 32K lowmem boot profile now defaults to GCC. KCC remains selectable
for larger configurations and explicit legacy comparisons. All future 32K
KCORE/KINIT measurements use COMPILER=gcc and a clean compiler-specific
build tree; the previous KCC link endpoint 023617 is historical only.
The strict 32K KCORE goal is a low-core end address below 020000 octal.
The clean GCC lowmem link succeeded with __kcore_low_end=023361 octal,
compared with historical KCC 023617. GCC saves 0236 octal words at this
checkpoint; a further 03362 octal words must be removed to get below 020000.
This is a link result, not a boot/runtime validation.

## 2026-10-13 PACKED MM ALLOCATION HELPERS V8

Staged proc_packed_reserve_v1/proc_packed_release_v1 helpers allocate
unaligned high-core MM type-4 extents with slot ownership, validation,
no reclaim recursion, and release-only descriptor clearing. They do not
publish packed process state or modify legacy swap records. In a trial link
they increased KCORE by 0145 octal words, so they are deliberately NOT linked
in the production resident kernel until the feature is activated. Eviction
and restoration remain disabled pending atomic transitions and rollback.

## 2026-10-13 PACKED PROCESS DESCRIPTOR V7

The inactive proc_packed_v1 two-word descriptor now defines the physical
encoded span and contiguous logical writable suffix, together with distinct
RESIDENT/DISK/PACKED lifecycle tag values. No per-process storage has been
added, no runtime path uses the descriptor, and packed eviction remains off.
Before integration, state tags need a concrete home, atomic transition rules
and MM owner validation; skip the unused middle of memory only with ABI proof.

## 2026-10-13 PACKED PROCESS MM OWNERSHIP RESERVATION V6

MM type 4 is reserved for packed writable process state, distinct from
ordinary aligned process images (type 1), modules (type 2) and dynamic kernel
allocations (type 3). The packed contract is in proc_packed_v1.h. No packed
state is allocated, no ambiguous disk-swap record is reinterpreted, and MM
compaction deliberately does not move the new type without an owner-update
transaction. Runtime behavior is unchanged; no eviction is enabled.

## 2026-10-13 PROCESS BACKING CLEANUP BOUNDARY V5

proc_swap_detach() now separates the legacy backstore-span release from
the executable backing cleanup in named helpers. The existing three-way
ownership decision still depends on VM residency: there is NO new packed
state tag, and compressed eviction remains disabled. KCC and full KCORE
link pass. Next implementation must introduce explicit, validated packed
state ownership before sharing cleanup with compressed in-core processes.

## 2026-10-13 PURE BACKING GUARDS SOURCE SEPARATION V4

Executable vnode mutation and filesystem-unmount busy checks now reside in
proc_pure_backing_pdp6.s with executable unpack/reconstruction.  Existing
entry names and packed backing metadata remain unchanged.  KCC generation and
the PDP-6 DAS proc_swap object pass.  proc_swap_detach still releases disk
backstore for nonresident processes: it must not be reused for compressed
resident pack allocations until a distinct state tag and transaction-safe
cleanup are introduced.  No packed eviction is activated by this patch.

## 2026-10-13 PURE BACKING ATTACH SOURCE SEPARATION V3

The provider-neutral executable backing record encoder proc_swap_attach()
now resides in proc_pure_attach.c. KCC includes it through the existing
vm_pdp6_swap.c translation unit; emitting two independent KCC .s files into
one object caused colliding local labels (%L3), so no fragile label renaming
was accepted. Existing process-swap detach and disk transactions stay put.
The PDP-6 DAS proc_swap object assembles after the change. This does not yet
remove swap from KCORE or enable compressed process packing.

## 2026-10-13 PURE BACKING SOURCE SEPARATION V2

PURE executable vnode unpacking and raw/compressed DXR2 text reconstruction
have moved from proc_swap_pdp6.s into proc_pure_backing_pdp6.s.  The build
still concatenates these leaves into the existing resident proc_swap object,
preserving its entry symbols and swap contract.  The dedicated DAS object
build passes.  This is a source boundary, not yet an MRES extraction or a
measured KCORE reduction.  Next separate the process-state lifecycle from
backstore transactions and ensure compressed resident packing can use the
PURE reload path without disk swap.

## 2026-10-13 DSH V25 OPTIONAL HISTORY FOR 32K

User approved disabling shell history during the current shrink phase,
provided the implementation remains available behind a compile-time
ifdef.  `DSH_ENABLE_HISTORY` defaults to 0; define it as 1 to enable
the existing Ctrl-P and HISTFILE behavior.  The opt-in history cache
is lazily allocated via the shared DSH heap, not static BSS or bare BRK.
GCC image/BSS octal baseline v24: 032104/013742; history-off v25:
031321/012751; history-on v25: 032120/012763.  The history-disabled
default passes full SIMH conformance 115/115; history-on behavior still
requires dedicated regression acceptance.
Strict below-030000 image gap is now 01322 octal in the default mode.

## 2026-10-13 KCORE PACKED PROCESS PREPARATION V1

The first staged interface for future compressed-process eviction is in
system/kernel/core/kpack_codec_v1.[ch].  V1 is an identity codec, is not linked
into the running kernel, and does not reclaim RAM.  No-swap PURE eviction must
NOT be activated until packed-process MM transactions and an actual compressor
are implemented and verified.  Design and next stages are recorded in
design/kcore-packed-process-v1.md.  Existing swap remains unchanged.

Minimum hardware profile revised to three TSFS tapes plus one native DTFS
writable /home tape. CLK is mandatory; the PCLK wall-clock reader should be
split out later. Operator civil-date entry before login is deferred, and
no uninitialized clock substitute may replace the current PCLK call path.

## 2026-10-13 DSH V24 SCRIPT WORKSPACE CLEARING

The 32-token DSH script workspace is now cleared in one word-wise pass
instead of calling `dsh_token_clear` for every element.  All fields are
PDP-6 word-sized, DSH_T_EOF is zero, and required zero-initialization
semantics are preserved.  GCC DXR image shrinks from 032117 to 032104
octal words, BSS remains 013742.  The dedicated nested SOURCE suite
passes 4/4 and full GCC SIMH DSH conformance passes 115/115.  The
strict below-030000 text gate still needs 02105 octal words removed;
physical RAM/BSS/stack/heap work remains open.  No overlays.

## 2026-10-12 DSH V23 STORED-FUNCTION WALKER SHRINK

The AST function-node preflight count and copy use one bounded walker,
with an optional destination.  Preflight remains read-only and occurs
before any existing definition is removed.  GCC executable image drops
from 032147 to 032117 octal words, saving 030; BSS stays 013742.
The full GCC target SIMH conformance passed 115/115; no overlay changes.

## 2026-10-12 DSH V22 PARSER NODE INITIALIZATION SHRINK

An experimental simplification of `dsh_node_clear` replaces repeated
`dsh_s6_clear` calls with one word-wise zeroing pass, then restores the
three DSH_NONE links.  GCC DXR image changes from 032177 to 032147
octal, a reduction of 030 octal words.  BSS remains 013742.  Direct
host parser test and the seven-case live shared-heap SIMH suite pass.
Full GCC SIMH conformance passed 115/115 after the change.  This
source-level reduction is accepted on the experimental branch, but the
strict 030000 text target and physical 32K resident limit remain open.

## 2026-10-12 DSH V21 STRICT IMAGE/BSS/STACK SHRINK PLAN

The accepted optimization target is now strictly below 030000 octal GCC
DXR image words, with further reductions of BSS and verified peak stack
and heap use.  Current shared-heap GCC image 032177, BSS 013742; at
least 02200 octal image words must be eliminated.  The existing 036000
image gate is no longer sufficient as the optimization goal.  GCC `-Oz`
is unsupported (cc1 rejects it); baseline remains GCC `-Os`.  The
prioritized, no-overlay measurement and shrink plan is in
`userland/dsh/TODO.MD`.  No new code optimization is accepted in V21.

## 2026-10-12 DSH V20 GCC-REQUIRED 32K BUILD POLICY

The corrected experimental shared-heap DSH links to image 032177 and
BSS 013742 octal words under GCC, versus image 037000/BSS 013742 under
KCC.  GCC therefore saves 04601 octal executable words and satisfies
the production 036000 DXR gate.  The full 115/115 DSH conformance and
7/7 dedicated shared-heap SIMH regressions pass with COMPILER=gcc.
The agreed configuration policy is that 32K DAIMOS must use GCC; KCC
remains supported for larger memory configurations.  This is not yet
proof of successful 32K DSH launch: INIT residency, process allocation
rounding, fragmentation and actual heap high-water remain open.
See design/dsh-compiler-size-comparison-20261012-v20.md.

## 2026-10-12 DSH V17 SHARED-HEAP DOUBLE-FREE FIX AND ACCEPTANCE

The experimental two-word DSH variable descriptor plus shared heap now
correctly distinguishes occupied variable slots from freed slots.  A new
SET into a slot previously UNSET must not release that slot's stale
payload pointer a second time.  UNSET now clears the pointer, and SET
checks occupancy before releasing an old payload.  Without this fix,
a seven-case target heap stress regression timed out in its final case;
after the fix all 7/7 pass under SIMH.  Final measured DXR image 037000,
BSS 013742 octal, compared to clean-main 036523/015241.  Image exactly
meets the temporary 037000 gate but still fails production 036000.
The corrected implementation also passed the full 115/115 SIMH conformance
suite.  The separate argument-frame suite passed 9/9.  The testkit's
resource-contract fixture was corrected from the obsolete four-entry
history assumption to the implemented sixteen-entry history limit and
passes all eleven fixed-limit checks.  It remains on the experimental branch pending heap high-water
accounting and production image shrink.  No overlays.

## 2026-10-12 DSH VARIABLE EXPERIMENT V17 (NOT ACCEPTED)

An isolated assembly variable-table rewrite plus shared C word heap
compiled and linked with image 036773, BSS 013742 octal against clean
current-main 036523/015241.  Combined image+BSS improves by 01027 octal
words; this is not a demonstrated reduction in physical APR allocations.
The temporary 037000 executable gate passes, while the production 036000
gate does not.  Full target regression is blocked before execution by a
missing native KCC bootstrap at `$HOME/cross/lib/kcc/bootstrap/KCC.dxr`.
V17 must remain experimental until bootstrap, target regression, and peak
memory acceptance are completed.  No overlay code introduced.

## 2026-10-12 DSH VARIABLE ALLOCATOR SIZE EXPERIMENT V16

Clean current-main PDP-6 KCC DSH baseline: DXR image 036523 and BSS 015241
octal.  A shared dynamic allocator for packed variable payloads and the
temporary 20-argument expansion compiled and linked with image 037213 and
BSS 013742.  Combined image+BSS fell by 0607 octal words, but the image
exceeded both the temporary 037000 and production 036000 limits.  No runtime
acceptance was run.  All prototype code was reverted; current production
DSH is unchanged.  The valid shared-lifetime concept requires smaller
assembly implementation and full regression, not the earlier unsafe direct
BRK-restore approach.  Detailed analysis is in
`design/dsh-variable-allocator-shrink-20261012-v16.md`.

## 2026-10-12 P10FOLD PDP-6 I/O SKIP SAFETY AND MULTI-TTY

- Fixed an unsafe zero-cost fold of GE driver CONSO/JRST polling prefixes.
  The CONSO skips the JRST, so identical two-word prefixes with different
  fall-through continuations are not equivalent. P10FOLD now recognizes
  BLKI, BLKO, CONSZ and CONSO I/O skips (encoded in the function field),
  and conservatively treats XCT as capable of propagating a skip.
- The GE-MRES fold plan no longer merges the two unsafe polling loops.
  The full CTY/DCS0/GE0 multi-TTY functional regression passes under both
  GCC and KCC, including terminal routing, LOGIN respawn, memory accounting,
  and STOP isolation. The testkit now discounts the temporary PS process
  from its persistent-process count, while requiring exactly one PS entry.
- The separate GCC/KCC kernel-and-userland compiler-selection regression
  also passes under both selections. The corrected P10FOLD is installed
  under PDP10_PREFIX using a versioned local.tar.gz distribution.
- DSH image-size acceptance is unchanged and remains deferred.

## 2026-10-12 FULL GCC/KCC BUILD SELECTION FOLLOW-UP

- `COMPILER=gcc|kcc` now selects the compiler through boot KINIT, KCORE,
  and native userland translation units, including compiler-specific build
  prerequisites. The previous userland KCC-only recipe path is eliminated.
- GCC native executables link the target libgcc archive. GCC's C-entry
  `__main` ABI has a dedicated one-word startup stub only in GCC builds;
  KCC builds do not include it.
- The versioned testkit compiler-selection probe checks actual compiler
  invocations for kernel and full userland, detects mixed-compiler builds,
  and verifies representative linked DXRs. GCC and KCC both PASS.
- A complete GCC image build passed with diagnostic
  `DSH_IMAGE_LIMIT=037000`; the production `036000` gate is unchanged and
  remains a known independent failure. No DSH-size acceptance was claimed.
- GE0 multi-TTY login is still under investigation; its standalone carrier
  test is not green. This compiler-selection change does not claim to fix it.

## 2026-10-12 TESTKIT COMPILER SELECTION

- `daimos-testkit` now accepts `COMPILER=gcc` and `COMPILER=kcc` (default)
  and forwards the selected compiler through its regression runner.
- `system/kernel/Makefile` honors the exported `COMPILER` while preserving
  explicit command-line `CC` overrides.  The GCC and KCC kernel compile
  commands were verified independently.
- Testkit build roots and regression checkpoints are compiler-specific.
  Both host `check` variants passed, as did representative GCC and KCC
  PDP-6 target probes.  The complete dual-compiler regression suite has
  not yet passed; DSH size acceptance remains excluded from that objective.

## D6FS LIVE PDP-6 APPEND INTERRUPTION AND REBOOT ACCEPTANCE (2026-10-12)

DAIMOS-TESTKIT `9CAF59F` + `91B19FD` IMPLEMENT A TEST-ONLY GENERATED
BACKING-WRITE HALT AFTER A SUCCESSFUL WRITTEN-DATA MARKER. LIVE PDP-6
SIMH RAN AN APPEND TO PRESEEDED `/CONFIG/FCBSEED`, HALTED INSIDE THE
BACKING CALLBACK AT OCTAL PC 032342 BEFORE THE WRITE RETURNED, AND LEFT
THE SELECTED SUPERBLOCK DIRTY (0671, SEQUENCE 2). A FOUR-MEMBER CLONE
WAS REPAIRED OFFLINE WITH D6FSCK -R TO A CLEAN SUPERBLOCK (0670, SEQ 3),
179196 BLOCKS AND 368 FCBS. A FRESH SIMH BOOT FROM THE REPAIRED CLONE
PASSED THE C99 ORIGINAL-WORD/ONE-WORD-EOF READBACK, STAT CONFIRMED
WORDS 1, AND DSH COMPLETED A NORMAL HALT. SOURCE DISK MEMBERS REMAINED
UNCHANGED THROUGH REPAIR AND REBOOT. NO PRODUCTION KERNEL OR DISK FORMAT
WAS ALTERED.

THIS IS ONE LIVE FILE-DATA-WRITE INTERRUPTION BOUNDARY AFTER A SIMULATOR
BACKING CALLBACK, NOT HARDWARE POWER-CUT DURABILITY PROOF. DIRECTORY/FCB
AND EXTENT-ALLOCATION CRASH CUTS, BOOT REPLAY, AND CRASH-ATOMIC HISTFILE
WITH CONCURRENT WRITERS REMAIN OPEN. DO NOT GENERALIZE ONE-WORD EOF
PRESERVATION TO COMPLETE FILE TRANSACTIONS.

## D6FS OFFLINE ORPHAN RECOVERY AND CHANGED-BLOCK REPAIR (2026-10-12)

DAIMOS-TESTKIT `092A67A` EXECUTES REAL FOUR-MEMBER DISK-IMAGE REPAIR,
NOT AN ABSTRACT REPLAY MODEL. THE ORIGINAL HOST D6FSCK RECOVERED FROM
ALL 48 FSYNC-COMPLETE WRITE INTERRUPTIONS WHILE RECLAIMING TWO
ZERO-EXTENT ORPHANS (0451/0452). THE HOST D6FSCK NOW COMPARES STAGED
FCB, FREEMAP AND SUMMARY BLOCKS AGAINST BACKING MEDIA AND WRITES ONLY
CHANGED BLOCKS. THE SAME REPAIR NEEDS THREE DURABLE WRITES (DIRTY
SUPERBLOCK, ONE MODIFIED FCB BLOCK, CLEAN SUPERBLOCK), AND ALL THREE
INTERRUPTION CUTS RECOVER CLEANLY AND IDEMPOTENTLY. NO KERNEL RAM OR
ON-DISK FORMAT CHANGE.

SEPARATE CLONED-IMAGE OWNERSHIP TESTS PASS: A SINGLE-OWNER ORPHAN FCB
WITH AN EIGHT-BLOCK EXTENT IS RECLAIMED AND ITS ALLOCATION BITS REPAIRED;
AN ORPHAN SHARING EXTENTS WITH A REACHABLE FILE IS REPORTED AMBIGUOUS
AND REPAIR WRITES NOTHING. ALL SOURCE IMAGES REMAIN UNMODIFIED.
THESE ARE OFFLINE HOST-TOOL RECOVERY RESULTS AFTER SYNTHETIC FSYNC
INTERRUPTIONS, NOT PROOF OF POWER-LOSS ATOMICITY. BOOT REPLAY,
CRASH-ATOMIC DSH HISTFILE AND CONCURRENT-WRITER TESTS REMAIN OPEN.

## D6FS ACTUAL DIRECTORY-ENTRY IDENTITY AUDIT (2026-10-12)

DAIMOS-TESTKIT `92A9622` INSPECTS THE PRESERVED FAULTED AND CONTROL
DISK IMAGES USING THE EXISTING D6FSCK DIRECTORY WALK. ON THE FAULT
IMAGE FCBS 0451 AND 0452 ARE TYPE-1 ZERO-LENGTH FILES WITH ZERO
DIRECTORY REFERENCES AND ARE UNREACHABLE. ON THE CLEAN CONTROL FCB
0451 HAS SIX WORDS AND ONE REFERENCE: ITS ENTRY IS IN PARENT FCB 0,
SLOT 7, CACHED TYPE 1, WITH NAME WORDS 164463507750:516364576271:
000000000000:000000000000 (OCTAL). FCB 0452 IS FREE. THE CONTROL
CHECKS CLEAN; THE FAULTED IMAGE DOES NOT. THIS DIRECTLY SUPPORTS
MISSING DIRECTORY PUBLICATION UNDER AMBIGUOUS CREATE COMPLETION.
IT DOES NOT YET PROVE PRECISE FAULT HIT COUNT OR CRASH-SAFE REPLAY.
NO KERNEL OR ON-DISK FORMAT CHANGE; PHASE ONE REMAINS OPEN.

## D6FS RECOVERY INTENT IDENTITY CHECKS (2026-10-12)

TESTKIT `7CB07F3` ADDS A HOST-ONLY C99 MODEL COMPARING CHILD FCB INDEX,
PARENT FCB, DIRECTORY SLOT, OBJECT TYPE AND FOUR SIXBIT NAME WORDS.
18 STATE COMBINATIONS PASS; 15 REQUIRE FURTHER INVESTIGATION, ONE
FULLY MATCHING/VISIBLE PAIR IS ALREADY PUBLISHED, AND TWO ALL-ABSENT
CASES ARE ONLY LOCALLY SAFE (ALLOCATIONS REQUIRE SEPARATE CHECKS).
THIS DOES NOT READ A LIVE D6FS IMAGE OR IMPLEMENT BOOT REPLAY, JOURNAL
DURABILITY, POWER-CUT RECOVERY OR CRASH-ATOMIC HISTFILE. PHASE ONE OPEN.

## D6FS CONSERVATIVE REPLAY-DECISION MATRIX (2026-10-12)

DAIMOS-TESTKIT `3759636` ADDS A HOST-ONLY C99 MODEL OF 96 COMBINATIONS
OF INTENT STATE (EMPTY/PREPARE/COMMIT) AND FIVE DURABLE-METADATA FLAGS.
87 REQUIRE FURTHER IDENTITY/CONSISTENCY INVESTIGATION; ONLY A COMPLETE
COMMIT CASE IS ELIGIBLE FOR ROLL-FORWARD UNDER ADDITIONAL IDENTITY
CHECKS. THE EIGHT NO-OP CASES ARE ABSTRACT EMPTY-INTENT CONDITIONS.
THIS IS NOT AN AUTOMATIC RECOVERY ALGORITHM OR DISK-DURABILITY PROOF.
METADATA IDENTITY, REPLAY/UNDO ORDER, POWER-CUT TESTS, AND REAL
CRASH-ATOMIC DSH HISTFILE ARE STILL OPEN. NO KERNEL CHANGE.

## D6FS TWO-COPY INTENT STATE TRANSITION TEST (2026-10-12)

TESTKIT `1B09FF2` MODELS SIX ALTERNATING PREPARE/COMMIT/EMPTY
TRANSITIONS AND ALL 774 WORD-PREFIX INTERRUPTED OVERWRITES. THE FIRST
VERSION FAILED WHEN IT OVERWROTE THE ONLY VALID RECORD; CORRECT
ALTERNATION PRESERVES THE INTACT COPY AND PASSES. STRICT HOST C99
COMPILATION PASS. THIS IS A SELECTOR MODEL WITH WEAK PROVISIONAL
CHECKSUM, NOT PERSISTENT JOURNALING, BOOT REPLAY, POWER-CUT RECOVERY,
OR CRASH-ATOMIC HISTFILE. NO PRODUCTION FORMAT OR KERNEL CODE CHANGED.

## D6FS INTENT RECORD SELECTOR PROTOTYPE (2026-10-12)

TESTKIT `AC9495D` ADDS A C99 HOST-ONLY TWO-COPY INTENT RECORD SELECTOR,
WHICH PASSES 129 WORD-PREFIX OVERWRITE CUTS AND EQUAL-SEQUENCE CONFLICT
REJECTION. THE MODEL USES NO DYNAMIC MEMORY BUT HAS ONLY A PROVISIONAL
36-BIT XOR-ROTATE INTEGRITY DIGEST; IT IS NOT A DEPLOYABLE ON-DISK
FORMAT. NO JOURNAL BLOCKS, BOOT REPLAY, CRASH-ATOMIC CREATE/HISTFILE,
OR KERNEL CODE WERE IMPLEMENTED. POWER-LOSS TESTS REMAIN OPEN.

## D6FS CREATION WRITE-ORDER CRASH MATRIX (2026-10-12)

DAIMOS-TESTKIT `ADF1E65` ADDS AN EXECUTABLE HOST-SIDE MODEL OF CHILD FCB,
OPTIONAL SYMLINK DATA, OPTIONAL PARENT GROWTH, DIRECTORY ENTRY, AND
PARENT FCB PUBLICATION. IT IDENTIFIES ORPHAN FCB WINDOWS AND A POSSIBLY
INVISIBLE DIRECTORY ENTRY BEFORE PARENT EOF PUBLICATION. THIS IS AN
ABSTRACT CRASH-PREFIX AUDIT, NOT A DURABILITY OR RECOVERY TEST. THE
D6FS CREATE RECOVERY DESIGN INCLUDES THE AUDIT RESULTS. IN-PLACE
DIRECTORY AND BITMAP TORN WRITES, COMMIT MARKER AND BOOT REPLAY REMAIN
OPEN. NO PRODUCTION KERNEL PROTOCOL CHANGE WAS MADE.

## D6FS CREATE TRANSACTION RECOVERY DESIGN GATE (2026-10-12)

`SYSTEM/KERNEL/FS/D6FS-CREATE-RECOVERY-DESIGN-20261012-V1.MD` AUDITS THE
EXISTING CREATE PATH AND DEFINES A RECOVERABLE FCB/DIRECTORY PUBLICATION
CONTRACT. IT IS DESIGN ONLY: NO JOURNAL, COW FORMAT, POWER-CUT RECOVERY,
OR PRODUCTION FIX IS IMPLEMENTED. CURRENT BCACHE INVALIDATION PREVENTS
FALSE CLEAN CACHE REUSE AFTER FAILED WRITE, BUT DOES NOT PROVIDE A
CRASH-ATOMIC COMMIT POINT. THE FCB 0451/0452 ORPHAN REGRESSION REMAINS
OPEN. IMPLEMENT ONLY AFTER WRITE-ORDER AUDIT AND TESTKIT FAULT MATRIX;
PRESERVE A SINGLE 128-WORD TRANSFER WORKSPACE AND AVOID PER-OPEN RAM.

## AMBIGUOUS FCB CREATION: DISK FORENSICS (2026-10-12)

TESTKIT `78C094A` ADDS A READ-ONLY D6FSCK-BASED DUMP OF FCBS 0451/0452.
ON THE PRESERVED POST-CALLBACK-ERROR IMAGE BOTH ARE TYPE-1 REGULAR,
SIZE ZERO, ZERO EXTENTS, UNREACHABLE. ON THE CLEAN CONTROL IMAGE FCB
0451 IS A SIX-WORD, ONE-EXTENT REGULAR FILE AND FCB 0452 IS FREE.
THE FAULT IMAGE IS CLEAN-MARKED DESPITE UNREACHABLE FCBS. THIS
STRENGTHENS THE EVIDENCE FOR THE CREATE-OBJECT PATH PUBLISHING THE
FCB BEFORE ITS DIRENT, THEN EXITING ON AN AMBIGUOUS RETURN.
THIS IS NOT YET A KERNEL FIX OR A POWER-LOSS TEST. CORRECT RECOVERY
MUST RESOLVE FCB AND DIRECTORY PUBLICATION TOGETHER, WITHOUT FREEING
POSSIBLY LIVE BLOCKS OR MERELY MASKING RETURN CODES. PHASE ONE OPEN.

## D6FS AMBIGUOUS FCB CREATE ERROR PATH IDENTIFIED (2026-10-12)

DAIMOS-TESTKIT `081DC33` CONFIRMS THAT D6FSCK REPORTS FCB INDICES IN
OCTAL: ORPHANS 451 AND 452 ARE DECIMAL 297 AND 298, BOTH IN LOGICAL
FCB BLOCK 479 (SLOTS 1 AND 2). `AE40D39` ADDS A SOURCE AUDIT:
`D6FS_PROVIDER_CREATE_OBJECT` WRITES THE NEW FCB BEFORE THE DIRECTORY
ENTRY, AND AN ERROR FROM `D6FS_READER_PUT_FCB` BRANCHES DIRECTLY TO
`D6FS_CREATE_FAIL`. A WRITE THAT REACHES BACKING STORAGE BUT RETURNS AN
ERROR CAN THEREFORE LEAVE AN ALLOCATED, UNREFERENCED FCB. THIS IS A
CONCRETE SOURCE PATH CONSISTENT WITH THE FAULT TEST; TARGET HIT COUNT
AND FULL CAUSAL ATTRIBUTION STILL REQUIRE DIRECT OBSERVATION. DO NOT
FREE THE FCB BLINDLY ON ERROR: DURABILITY IS AMBIGUOUS. RECONCILE
RECOVERABLE FCB AND DIRECTORY-ENTRY PUBLICATION AS A CORRECT PROTOCOL
BEFORE CLOSING PHASE ONE. POWER-LOSS/ATOMIC HISTFILE REMAIN OPEN.

## AMBIGUOUS FCB CONTROL IS CLEAN (2026-10-12)

TESTKIT `83A0E6E` ADDED THE MISSING SUCCESS-EXPECTING CONTROL. THE
SAME PRESEEDED FILE APPEND, WITH SYNTHETIC ERROR TARGET SET TO AN
UNREACHABLE LOGICAL BLOCK, PASSED APPEND/READBACK, HALT, AND D6FSCK
(CLEAN, SUPERBLOCK 670 SEQ 3, 179196 BLOCKS, 368 FCBS). BY CONTRAST
INJECTION TARGETING FCB LOGICAL BLOCK 479 REPRODUCED UNREACHABLE FCBS
451 AND 452. THIS CONFIRMS A FAULT-DEPENDENT REGRESSION, BUT TARGET
FCB IDENTITY AND THE EXACT INJECTED WRITE COUNT ARE NOT VERIFIED.
DO NOT ASCRIBE A ROOT CAUSE BEFORE INSTRUMENTING THE TARGET AND HIT
COUNT. POWER-LOSS AND CRASH-ATOMIC RECORD COMMIT ARE STILL OPEN.

## AMBIGUOUS FCB RETURN — NARROWED REPRODUCTION (2026-10-12)

DAIMOS-TESTKIT `E8E66DB` NARROWED THE SYNTHETIC POST-WRITE ERROR
SELECTOR TO LOGICAL FCB-TABLE BLOCK 479. THE APPEND REPORTED AN ERROR,
THE NEW DATA WAS READABLE, AND HALT PASSED. D6FSCK STILL REPORTED
UNREACHABLE FCBS 451 AND 452. THIS NARROWING DOES NOT ELIMINATE THE
REGRESSION. TARGET-FILE FCB IDENTITY AND THE INJECTOR HIT COUNT HAVE
NOT YET BEEN PROVEN. AN UNREACHABLE-BLOCK CONTROL FAILED THE FIXTURE'S
EXPECTED-ERROR PROBE AND DID NOT REACH HALT/FSCK: IT IS INCONCLUSIVE.
BEFORE ROOT-CAUSE CHANGES, BUILD A CLEAN END-TO-END NO-FAULT CONTROL
WITH A SUCCESS-EXPECTING FIXTURE AND PRECISELY TRACE THE TARGET FCB.
CRASH-ATOMIC HISTFILE AND POWER-CUT TESTS REMAIN OPEN.

## D6FS AMBIGUOUS FCB COMPLETION: OPEN FSCK REGRESSION (2026-10-12)

DAIMOS-TESTKIT `2255B02` INTRODUCES A TEST-ONLY POST-BACKING-CALLBACK
ERROR RETURN FOR FCB TABLE WRITES. IN PDP-6 SIMH THE WRITER RECEIVED
AN ERROR WHILE THE SECOND WORD OF A PRESEEDED FILE WAS COMMITTED AND
READ BACK SUCCESSFULLY. HALT PASSED. HOWEVER, D6FSCK FAILED: TWO
UNREACHABLE FCBS (451, 452), EACH WITH ZERO REFERENCES, ON A CLEAN-MARKED
VOLUME. THE INJECTION AFFECTS ALL FCB BLOCKS THROUGHOUT BOOT; THUS
UNRELATED OPERATIONS MAY HAVE CAUSED THE ORPHANS. THE RESULT IS A
REAL FAILING TEST, NOT PROOF OF A SPECIFIC STAGED-GROWTH KERNEL DEFECT.
ISOLATE TARGET PUBLICATION AND TRACE HIT COUNT BEFORE DETERMINING THE
ROOT CAUSE. THIS IS A SYNTHETIC AMBIGUOUS RETURN, NOT A DURABLE WRITE
OR POWER-LOSS ATOMICITY GUARANTEE. KEEP PHASE ONE OPEN.

## D6FS FCB PUBLICATION REJECTION TEST (2026-10-12)

DAIMOS-TESTKIT `C2E8708` TESTED A REJECTED FCB TABLE WRITE ON PDP-6
SIMH. TEST-ONLY BACKING INJECTION TARGETED FCB LOGICAL BLOCKS 442..486,
DETERMINED BY D6FSCK. THE TARGET FILE WAS PRESEEDED IN THE DISK IMAGE;
APPEND RETURNED AN ERROR, AND THE ORIGINAL ONE-WORD CONTENT AND EOF
WERE STILL READABLE AFTER REOPEN. ORDERLY HALT PASSED AND D6FSCK WAS
CLEAN (SUPERBLOCK 670, SEQ 3, 179196 BLOCKS, 368 FCBS). THIS IS A
DEFINITELY REJECTED FCB WRITE, **NOT** AN AMBIGUOUS OR PARTIALLY DURABLE
COMMIT. POWER-LOSS AND CRASH-ATOMIC RECORD COMMIT REMAIN OPEN.

## D6FS ALLOCATION BITMAP RANGE FAULT (2026-10-12)

TESTKIT `8BC8872` EXTENDS THE ISOLATED BACKING-WRITE INJECTOR TO
REJECT A CONTIGUOUS LOGICAL RANGE. ON THE TEST IMAGE (179196 BLOCKS)
THE FREEMAP STARTS AT DECIMAL 487; REJECTING 487..525 CAUSED A DISPOSABLE
ONE-WORD FILE GROWTH TO RETURN AN ERROR, RETAIN ZERO-WORD EOF, AND HALT
NORMALLY. SIMH PASSED BOTH PROBES; D6FSCK REPORTED CLEAN, SUPERBLOCK
670 SEQUENCE 3 AND 360 FCBS. EARLIER REJECTION OF BLOCK 487 ALONE DID
NOT TRIGGER THE ERROR. THIS GEOMETRY IS IMAGE-SPECIFIC. A WRITE-ATTEMPT
COUNTER HAS NOT YET BEEN ADDED, SO TARGETED BITMAP WRITE REJECTION IS
SUPPORTED BY ISOLATION TO THE BITMAP RANGE, NOT BY A RECORDED HIT COUNT.
AMBIGUOUS FCB PUBLICATION, CRASH RECOVERY, AND ATOMIC HISTFILE ARE OPEN.

## D6FS LIVE SECOND-BLOCK FAILURE ACCEPTANCE (2026-10-12)

DAIMOS-TESTKIT `94A4A6B` ADDS A PDP-6 C99 TEST THAT ATTEMPTS TO WRITE
256 WORDS TO A NEW FILE, WITH A TEST-ONLY BACKING DISPATCHER REJECTING
THE SECOND 128-WORD DATA BLOCK VIA ITS FIRST-WORD MARKER. THE WRITE
FAILS, REOPENING REPORTS THE OLD ZERO-WORD EOF, AND ORDERLY HALT PLUS
D6FSCK REPORT CLEAN (179196 BLOCKS, 360 FCBS, SUPERBLOCK 670 SEQ 3).
THIS VALIDATES VISIBLE EOF ROLLBACK AFTER A SECOND-BLOCK FAILURE, NOT
INDEPENDENT PHYSICAL DURABILITY OF THE FIRST BLOCK. ALLOCATION-MAP,
FCB PUBLICATION, POWER CUT, AND ATOMIC HISTFILE REMAIN OPEN.

## D6FS LIVE PAYLOAD-WRITE FAILURE ACCEPTANCE (2026-10-12)

DAIMOS-TESTKIT COMMIT `3D7C045` PROVIDES A TEST-ONLY BACKING-WRITE
INJECTOR SELECTING A DISTINCTIVE FIRST PAYLOAD WORD, AND A NATIVE PDP-6
C99 TEST THAT CREATES A DISPOSABLE D6FS FILE, REQUIRES THE MARKED WRITE
TO FAIL, AND REOPENS THE FILE TO VERIFY THE ORIGINAL ZERO-WORD EOF.
LIVE SIMH ACCEPTANCE PASSED BOTH THE INJECTED-WRITE AND ORDERLY-HALT
PROBES. D6FSCK REPORTED SUPERBLOCK 670, SEQUENCE 3, CLEAN, 179196 BLOCKS
AND 360 FCBS. THE FIRST HARNESS ATTEMPT TIMED OUT AT LOGIN; THE SAME
IMAGE PASSED WITH A LONGER BOOT ALLOWANCE. THE REPEATABLE TEST RUNNER
NOW USES A 120-SECOND BOOT LIMIT.

THIS VALIDATES THE NORMAL ERROR RETURN AND OLD EOF AFTER ONE FAILED
DATA-BLOCK WRITE. ALLOCATION-MAP FAILURE, MULTI-BLOCK PARTIAL WRITES,
AMBIGUOUS FCB COMMIT, POWER-LOSS RECOVERY, AND CRASH-ATOMIC HISTFILE
RECORD COMMITS REMAIN OPEN. THE INJECTOR LIVES ONLY IN DAIMOS-TESTKIT;
PRODUCTION DAIMOS KERNEL SOURCES AND DISK FORMAT ARE UNCHANGED.

## D6FS STAGED GROWTH FAILURE-PATH TESTING STATUS (2026-10-11)

DAIMOS-TESTKIT NOW HAS AN EIGHT-ASSERTION STATIC SOURCE CONTRACT FOR THE
STAGED GROWTH / ABORT FLOW: PAYLOAD BEFORE FCB PUBLICATION, COMPLETE
TRANSFER REQUIRED, ABORT ON WRITE FAILURE, RE-READ OLD DISK FCB, PRESERVE
ITS ALLOCATED EXTENT CAPACITY, RESTORE FCB/INFO, AND PROPAGATE CLEANUP
ERRORS. THE CONTRACT PASSES. THE EXISTING HOST D6FSCK FAULT ABORT HOOK
ONLY EXERCISES FSCK REPAIR, NOT THE LIVE FS_BACKING_WRITE PATH; IT CANNOT
BE USED AS EVIDENCE FOR LIVE STAGED-GROWTH ROLLBACK. SEE THE TESTKIT
`DAIMOS-D6FS-STAGED-WRITE-FAULT-PLAN-20261011-V1.MD` FOR THE BACKING-I/O
FAILURE MATRIX. NO DEVICE-LEVEL FAULT INJECTION HAS BEEN EXECUTED YET.
KEEP FAILURE-PATH VALIDATION AND CRASH-ATOMIC HISTFILE RECORDS OPEN.

## D6FS V2 STAGED REGULAR-FILE GROWTH (2026-10-11, TESTED WRITE PATH)

THE D6FS REGULAR-FILE WRITE PROVIDER NOW STAGES EXPANDED EXTENTS IN A
PROCESS-PRIVATE FCB WITHOUT PUBLISHING THE NEW READABLE SIZE, WRITES THE
PAYLOAD, THEN PUBLISHES THE FINAL FCB. `D6FS_PROVIDER_RESIZE_FCB` USES AN
INTERNAL SIGN-BIT STAGING MODE; ALL EXISTING NON-STAGED CALLERS RETAIN THEIR
PRIOR BEHAVIOR. A FAILED DATA WRITE CALLS THE ASSEMBLER ROLLBACK HELPER,
WHICH RE-READS THE OLD DURABLE FCB, RETAINS ITS FULL ALLOCATED CAPACITY
(RATHER THAN ITS SHORTER LOGICAL EOF), RELEASES ONLY NEW BLOCKS, AND
RESTORES THE OLD IN-MEMORY FCB/INFO. AMBIGUOUS FINAL FCB-COMMIT FAILURE
IS REPORTED WITHOUT FREEING POSSIBLY LIVE BLOCKS. NO EXTRA PER-FILE
RESIDENT RAM OR DISK FORMAT CHANGE IS INTRODUCED.

PDP-6 SIMH PASS: NATIVE D6FS TWO-PREOPENED APPEND (2/2), NATIVE 444-WORD
CROSS-BLOCK GROWTH/READBACK (2/2), BACKGROUND DSH TWO-WRITER APPEND (6/6),
HISTFILE WRITE/HALT (3/3), REBOOT RELOAD (1/1), CLEAN D6FS CHECKS.
SOURCE CONTRACT AUDITS NOW ASSERT STAGED PUBLICATION, NOT THE OLD DEFECT.
BACKING-WRITE FAULT INJECTION AND POWER-CUT RECOVERY REMAIN OPEN; THIS IS
AN ORDERED-GROWTH REPAIR, **NOT** A CRASH-ATOMIC RECORD TRANSACTION.

## D6FS V2 WRITE-GROWTH STAGING SAFETY AUDIT (2026-10-11)

THE EXISTING RESIZE HELPER SERVES TRUNCATE, DIRECTORY, SYMLINK AND REGULAR
FILE PATHS. GLOBAL REORDERING WOULD ALTER THEM ALL. REGULAR WRITES CAN
STAGE EXTENTS IN MEMORY (D6FS_FILE_BLOCK USES EXTENTS, NOT FILE SIZE), BUT
MUST KEEP THE OLD RESERVED EXTENT CAPACITY FOR ERROR ROLLBACK. EARLY
FAILURES MUST NOT RECLAIM OLD CAPACITY; AMBIGUOUS FCB COMMIT MUST NOT FREE
POSSIBLY LIVE BLOCKS. SEE `design/d6fs-v2-growth-staging-20261011-v1.md`.
THE KERNEL IMPLEMENTATION IS STILL UNCHANGED; THE SOURCE-ORDER DEFECT IS
OPEN. NO CRASH-ATOMICITY GUARANTEE IS CLAIMED.

## D6FS V2 ORDERED-WRITE CONTRADICTION AND RECORD-COMMIT DECISION (2026-10-11)

THE D6FS V2 FORMAT DOCUMENT EXPLICITLY EXCLUDES JOURNALS/CHECKSUMS AND
REQUIRES COMPLETE DATA BEFORE NEW FCB EXTENT/SIZE PUBLICATION. THE CURRENT
`D6FS_PROVIDER_WRITE_WORDS`/`D6FS_PROVIDER_RESIZE_FCB` PATH PUBLISHES THE
FCB FIRST. THIS IS AN INDEPENDENT V2 ORDERING BUG AND MUST BE CORRECTED
BEFORE A FORMAT CHANGE. RESTORING ORDER DOES NOT ALONE GIVE CRASH-ATOMIC
MULTIWORD HISTFILE RECORDS; THOSE REQUIRE A SEPARATE VERSION-GATED COMMIT
PROTOCOL OR AN EXPLICITLY WEAKER DURABILITY CONTRACT. SEE
`design/d6fs-record-atomicity-20261011-v1.md`. NO KERNEL FIX IS CLAIMED.

## D6FS RECORD-COMMIT WRITE-ORDER AUDIT (2026-10-11)

THE PROVIDER-LEVEL `O_APPEND` LOCK RESERVES CURRENT EOF BUT DOES NOT
GIVE CRASH-ATOMIC RECORD COMMIT. `D6FS_PROVIDER_WRITE_WORDS` CALLS
`D6FS_PROVIDER_RESIZE_FCB` BEFORE COPYING NEW PAYLOAD. THE RESIZE PATH
UPDATES EXTENTS AND FCB SIZE THROUGH `D6FS_READER_PUT_FCB`; THEN
`D6FS_READER_WRITE_WORDS` WRITES EACH 128-WORD BLOCK SEPARATELY THROUGH
`D6FS_READER_COMMIT_CACHE`. A CRASH CAN THEREFORE EXPOSE A NEW FCB SIZE
WITH MISSING OR PARTIAL PAYLOAD. FINAL MTIME/FCB PUBLICATION IS ANOTHER
FAILURE POINT. A/B CLEAN/DIRTY SUPERBLOCK STATE DOES NOT JOURNAL RECORDS.

DAIMOS-TESTKIT `DAIMOS-D6FS-RECORD-COMMIT-ORDER-AUDIT-20261011-V1.SH`
NOW DETECTS THIS UNSAFE ORDERING AND REPORTS `KNOWN NONATOMIC`; THIS
IS A SOURCE CONTRACT CHECK, NOT A PASSED CRASH-SAFETY TEST.

BEFORE IMPLEMENTATION, SPECIFY AN ON-DISK ATOMIC COMMIT POINT AND
RECOVERY FOR DATA, FCB/EXTENT METADATA AND ALLOCATION MAP; THEN INJECT
I/O FAILURES AT EVERY PUBLICATION STEP. DO NOT CLAIM ATOMICITY BY MOVING
THE EXISTING SIZE WRITE OR BY RELYING ON THE IN-MEMORY PROVIDER LOCK.
NO KERNEL CODE OR DISK FORMAT WAS CHANGED BY THIS AUDIT.

## DSH HISTFILE CORRUPTED-TAIL SIMH ACCEPTANCE (2026-10-11)

THE DAIMOS-TESTKIT NATIVE C99 FIXTURE WRITES A VALID `ECHO SAFE` S6REC
FOLLOWED BY A TEXT HEADER WHOSE PAYLOAD WORDS ARE MISSING. AFTER CLEAN
HALT AND REBOOT OF THE SAME D6FS DISKS, INTERACTIVE DSH EMITS
`HISTFILE DAMAGED; VALID PREFIX LOADED`, AND CTRL-P RECALLS `ECHO SAFE`.
THE SECOND HALT LEAVES SUPERBLOCK 706, SEQUENCE 5, CLEAN. THE TWO-BOOT
SIMH REGRESSION PASSES FOUR PROBES. EXISTING LOADER CODE REQUIRED NO
MODIFICATION. RECOVERING RECORDS AFTER A DAMAGED TAIL OR AUTOMATIC
TRUNCATION REMAINS DISABLED UNTIL CROSS-PROCESS-SAFE RECORD COMMIT/REPAIR
SEMANTICS EXIST. PHASE-ONE IMAGE SIZE OVERRIDE REMAINS TEMPORARY.

## DSH PHASE-ONE HISTFILE CONCURRENT WRITER / DAMAGED TAIL (2026-10-11)

AN ISOLATED PDP-6 SIMH TEST LAUNCHED TWO BACKGROUND DSH GROUPS THAT
APPENDED ALPHA AND BETA TO A D6FS FILE WHILE THE INTERACTIVE PARENT
REMAINED RESPONSIVE. BOTH WORKERS COMPLETED; HEADER, ALPHA, AND BETA
SURVIVED READBACK; CLEAN HALT SUCCEEDED (6/6 PROBES). THIS IS LIVE
MULTIPROCESS ACCEPTANCE BUT NOT DETERMINISTIC EVERY-INTERLEAVING PROOF.

ON HISTFILE LOAD, A MALFORMED OR TRUNCATED S6REC TAIL NOW REPORTS
`DSH: HISTFILE DAMAGED; VALID PREFIX LOADED`, WHILE THE VALID HISTORY
PREFIX REMAINS USABLE. NO AUTOMATIC TRUNCATION IS ATTEMPTED: THE CURRENT
FLOCK VALIDATION IS LOCAL TO A PROCESS FILE TABLE AND CANNOT SAFELY
SERIALIZE REPAIR AGAINST OTHER PROCESSES. NORMAL HISTORY/HALT 3/3 AND
SAME-DISK REBOOT 1/1 PASS; LIVE MALFORMED-TAIL FAULT INJECTION AND
RECOVERY/REPAIR POLICY ARE STILL OPEN. DSH IMAGE 036523 OCTAL WORDS
USES THE TEMPORARY PHASE-ONE BUILD GATE OF 037000.

## DSH PHASE-ONE CONFORMANCE RETEST (2026-10-11)

THE 115-CASE PDP-6 SIMH DSH CONFORMANCE SUITE PASSES 115/115 WITH THE
BOUNDED `$()` S6REC WORKER, HISTFILE WRITE RETRIES AND INTERACTIVE-ONLY
FOREGROUND TTY HANDOFF. A FIRST RETEST RETURNED 106/115 BECAUSE THE
TESTKIT HARNESS MISTOOK THE `# ` SUBSTRING IN ECHOED COMMAND TEXT FOR A
FRESH DSH PROMPT, SUBMITTING LATER COMMANDS TOO EARLY. THE TESTKIT NOW
REQUIRES THE PROMPT TO START A LOGICAL TERMINAL LINE; THE FULL SUITE
PASSES. THE DSH IMAGE IS 036475 OCTAL WORDS IN THE ACCEPTED TEMPORARY
037000 PHASE-ONE BUILD WINDOW. THE DEFAULT 036000 GATE REMAINS IN FORCE
FOR NORMAL BUILDS.

## DSH PHASE-ONE BACKGROUND SESSION REPAIR (2026-10-11)

THE RETURN-TO-LOGIN REGRESSION AFTER `BEGIN ECHO ... END &` WAS CAUSED BY
NONINTERACTIVE DSH WORKERS WITH AN INHERITED CONTROLLING TTY CALLING
`TTY_SETFG` WHILE STARTING THEIR OWN EXTERNAL COMMANDS. THE INTERACTIVE
PARENT REMAINED IN PGRP 2 BUT TTY FG MOVED TO A WORKER CHILD PGRP (5),
SO THE NEXT PARENT `READCHAR` RETURNED -1 AND DSH EXITED. THE FIX REQUIRES
BOTH `st->interactive` AND `st->tty_attached` FOR FOREGROUND HANDOFF AND
RESTORATION IN THE EXTERNAL-COMMAND AND PIPELINE PATHS. NO KERNEL TEARDOWN
CHANGES OR WORKAROUNDS WERE RETAINED.

PDP-6 SIMH: BACKGROUND GROUP LAUNCH, PARENT COMMAND, WAIT, BACKGROUND
GROUP PIPELINE, PARENT COMMAND, WAIT, NORMAL FOREGROUND PIPELINE,
BOUNDED `$()` AFTER THE WORKER AND ORDERLY HALT ALL PASS (9/9 PROBES).
THE DEFAULT DSH IMAGE GATE IS STILL 036000 OCTAL WORDS, WITH USER-APPROVED
TEMPORARY OVERRIDE `DSH_IMAGE_LIMIT=037000` DURING PHASE ONE.

## DSH BOUNDED COMMAND SUBSTITUTION (2026-10-11, PHASE ONE)

THE FIRST BOUNDED `$()` IMPLEMENTATION ACCEPTS ONE STANDALONE SIMPLE
SUBSTITUTION ARGUMENT, WITH AN UNQUOTED SIMPLE COMMAND INSIDE. A SEPARATE
DSH `-S` WORKER EMITS NATIVE S6REC VIA STDOUT, AND THE PARENT DRAINS THE
PIPE WHILE THE WORKER RUNS. IT RETAINS AT MOST 36 SIXBIT CHARACTERS,
DISCARDS TRAILING RECORD NEWLINES, AND REJECTS OUTPUT OVERFLOW AND WORKER
FAILURE. AST SUBSTITUTION POSITION IS PACKED INTO SIMPLE NODE FLAGS;
`extra` REMAINS THE AST CHILD LINK. DIRECT PARSER REGRESSIONS AND SIMH
SUCCESS/FAILURE/OVERFLOW/SYNTAX/SHUTDOWN PROBES PASS.

THE DSH IMAGE IS CURRENTLY 036444 OCTAL WORDS IN THIS EXPERIMENT.
AT USER REQUEST THE BUILD ACCEPTS `DSH_IMAGE_LIMIT=037000` TEMPORARILY
DURING PHASE ONE; ITS DEFAULT 036000 LIMIT IS UNCHANGED. SHRINKING DSH
SOURCE/IMAGE IS DEFERRED UNTIL ALL PHASE-ONE WORK IS COMPLETE.

## MEMFS O_APPEND WRITE-TIME EOF RESOLUTION (2026-10-11)

MEMFS NOW ACCEPTS THE SAME DESCRIPTOR O_APPEND SENTINEL AS D6FS, RESOLVING
THE CURRENT MEMFS SLOT SIZE DURING WRITE_WORDS RATHER THAN AT OPEN TIME.
THE SHARED FILE LAYER DISPATCHES APPEND SENTINELS FOR PROVIDERS 4 AND 6
AND REFRESHES THEIR CURSORS AFTER A SUCCESSFUL WRITE. NO EXTRA DESCRIPTOR
WORDS OR PER-FILE RESIDENT LOCKS WERE ADDED.

THE NATIVE PDP-6 TWO-PREOPENED-DESCRIPTOR MEMFS TEST ON /TEMP PASSES,
INCLUDING SEEK(0), DUPLICATE-APPEND, FOUR-WORD RAW READBACK AND CURSORS.
THE SAME TEST FAILS ON THE ORIGINAL FILE IMPLEMENTATION (SECOND CURSOR 1).
THE NATIVE D6FS TEST STILL PASSES AFTER THE SHARED FILE-LAYER CHANGE.
TRUE SIMULTANEOUS TWO-PROCESS APPENDS AND SHORT-WRITE S6REC INTEGRITY
REMAIN OPEN; A TWO-DESCRIPTOR TEST IS NOT A SCHEDULER CONCURRENCY TEST.

## D6FS ATOMIC O_APPEND IMPLEMENTATION AND SIMH VALIDATION (2026-10-11)

D6FS WRITE_WORDS NOW RESOLVES O_APPEND END-OF-FILE INSIDE THE EXISTING
SERIALIZED PROVIDER CALL, AFTER READING THE CURRENT FCB. THE FILE DESCRIPTOR
KEEPS APPEND INTENT WITHOUT GROWING ITS TWO-WORD FORMAT: THE PACKED DIR BIT
IS REUSED FOR REGULAR FILES WITH NONZERO REGULAR/LOCK STATE. DIRECTORY CHECKS
DISTINGUISH THESE DISJOINT CASES. AN INVALID EXPERIMENTAL LH 000004 FLAG
WAS REJECTED BECAUSE IT CORRUPTED THE CANONICAL VNODE; THE FINAL ENCODING
PASSES PDP-6 SIMH. THE APPEND DESCRIPTOR CURSOR IS REFRESHED AFTER WRITE.

A DETERMINISTIC NATIVE PDP-6 TEST ON /CONFIG (D6FS) OPENS TWO APPEND
DESCRIPTORS BEFORE EITHER WRITE, SEEKS THE SECOND TO ZERO, THEN WRITES
THROUGH BOTH AND A DUPLICATE. ALL FOUR WORDS AND DESCRIPTOR CURSORS PASS;
THE UNMODIFIED BASELINE FAILS (SECOND CURSOR 1 INSTEAD OF 3). THE FIRST
POST-WRITE CURSOR IMPLEMENTATION DECODED AN UNMASKED PROVIDER FIELD AND
WAS FIXED TO MASK METADATA BEFORE LOOKUP. THE TEST ALSO CONFIRMED CLEAN
HALT, SUPERBLOCK 705 SEQUENCE 3. OLDER SEQUENTIAL /TEMP APPEND PROBES
ACTUALLY EXERCISED MEMFS; THEY WERE NOT D6FS VALIDATION. DIRECTORY/HISTFILE
5/5 AND SAME-DISK REBOOT 2/2 PASS, INCLUDING HISTORY ON THE D6FS ROOT.
A SIMULTANEOUS TWO-PROCESS TEST REMAINS OPEN. THE DSH BACKGROUND-GROUP
TEST RETURNS TO LOGIN EVEN ON THE UNMODIFIED BASELINE. MEMFS APPEND
SEMANTICS AND SHORT-WRITE RECORD INTEGRITY ALSO REMAIN OPEN.

## NATIVE MAKE V0.9 FEATURE FREEZE AND SIXBIT SHRINK (2026-10-11)

NATIVE MAKE IS FEATURE-COMPLETE FOR V0.9, WITH UPPERCASE SIXBIT-ONLY
INPUT, SERIAL EXECUTION AND A DUMMY -J OPTION. DOUBLE-COLON RULES AND
PROPAGATION OF COMMAND-LINE VARIABLE OVERRIDES ARE OUT OF SCOPE FOR V0.9.
THE LAST SOURCE AUDIT FOUND NO REMAINING LOWERCASE OR ASCII-COMPATIBILITY
BRANCHES. MAKE STILL NEEDS UNPACKED INTERNAL TEXT FOR S6REC MAKEFILE
PARSING AND COMMAND EXPANSION; REMOVING THAT REPRESENTATION WOULD BE A
MAJOR REWRITE, NOT A SAFE SHRINK. THE RUN V2 MAKEFLAGS ENVIRONMENT FILTER
NOW MATCHES THE SIXBIT RECORD DIRECTLY, ELIMINATING ITS 257-BYTE LOCAL
DECODE BUFFER. KCC BUILD/LINK AND SIXBIT MANUAL VALIDATION PASS.

## NATIVE MAKE MINIMAL MAKEFLAGS (2026-10-11)

NATIVE MAKE NOW IMPORTS UPPERCASE SIXBIT MAKEFLAGS FROM ENVIRONMENT,
MERGES SUPPORTED SERIAL SWITCHES WITH COMMAND-LINE OPTIONS, AND FORWARDS
ONE REPLACEMENT MAKEFLAGS RECORD THROUGH BOTH RUN V2 RECIPE PATHS.
THE NO-OP -J JOB COUNT IS NOT EXPORTED. LITERAL RECURSIVE MAKE COMMANDS
ARE RUN UNDER -N TO ALLOW CHILD DRY-RUN OUTPUT, BUT NOT UNDER -Q.
NATIVE PDP-6 PARENT/CHILD -I AND -N REGRESSIONS PASS (15/15 TOTAL).
NO ASCII ALIASES, PARALLELISM, OR GENERAL ENVIRONMENT REPLACEMENT LAYER
WERE ADDED. COMMAND-LINE VARIABLE OVERRIDES REMAIN NON-PROPAGATING.

## NATIVE MAKE PRECIOUS REGRESSION (2026-10-11)

THE UPPERCASE SIXBIT NATIVE MAKE SIMH SUITE NOW PASSES 13/13 PROBES.
THE NEW .PRECIOUS TEST CREATES A FILE, TRIGGERS A FAILING RECIPE WITH
-B, AND CONFIRMS THE FILE IS RETAINED. THIS IS CONSISTENT WITH THE
EXISTING POLICY OF NEVER REMOVING FAILED OUTPUTS, REGARDLESS OF WHETHER
.PRECIOUS WAS DECLARED. THERE IS NO NEED FOR A NEW DELETION MECHANISM.
DOUBLE-COLON RULES AND AUTOMATIC RECURSIVE MAKEFLAGS PROPAGATION REMAIN
UNIMPLEMENTED. BOTH NEED A SEPARATE SEMANTIC DESIGN BEFORE MODIFICATION.

## D6FS ORDERLY SHUTDOWN AND DSH HISTFILE REBOOT (2026-10-11)

THE PRIVILEGED HALT PATH NOW SYNCS AND TRANSITIONS A WRITABLE D6FS
NAMESPACE ROOT TO CLEAN THROUGH THE EXISTING SERIALIZED VFS REMOUNT.
ALREADY READ-ONLY ROOTS ARE NOT TOGGLED; A FAILED CLEAN PUBLICATION
REFUSES HALT. MEMFS SNAPSHOT SHUTDOWN STILL PRECEDES D6FS PUBLICATION.
AN ISOLATED PDP-6 SIMH TEST PASSED 3/3 INITIAL HISTORY/HALT PROBES.
CURRENT-SOURCE D6FSCK REPORTED CLEAN SUPERBLOCK 705, SEQUENCE 3. THE
SECOND BOOT OF THE SAME DISK PASSED HISTFILE RELOAD 1/1. FAILURE
INJECTION AND CONCURRENT HISTORY APPENDS REMAIN TO BE TESTED.


## NATIVE MAKE UPPERCASE SIMH FOLLOW-UP (2026-10-11)

THE STRICT-UPPERCASE SIXBIT NATIVE MAKE REGRESSION PASSED 7/7 PDP-6 SIMH
PROBES AFTER LOWERCASE ALIASES WERE REMOVED. THE TEST WAS THEN EXPANDED
WITH TARGET-SPECIFIC .SILENT AND .IGNORE AND GLOBAL -I FAILURE HANDLING;
ALL 10/10 PROBES PASS. -J REMAINS A SERIAL NO-OP. .PRECIOUS IS RECOGNIZED
BUT REQUIRES SEPARATE BEHAVIORAL TESTING. PORTABLE DOUBLE-COLON RULES AND
RECURSIVE MAKEFLAGS PROPAGATION REMAIN OUTSTANDING.

## NATIVE MAKE UPPERCASE SIXBIT ONLY (2026-10-11)

NATIVE MAKE NOW ACCEPTS ONLY UPPERCASE SIXBIT OPTION LETTERS AND INCLUDE
DIRECTIVES. LOWERCASE OPTION AND INCLUDE ALIASES HAVE BEEN REMOVED. THIS
REVERSES THE PREVIOUS HOST-STYLE LOWERCASE ACCEPTANCE: THE NATIVE USER ABI
IS SIXBIT, NOT ASCII. THE SERIAL -J OPTION IS RETAINED AS A NO-OP.

## NATIVE MAKE TARGET SIMH ACCEPTANCE (2026-10-11)

THE NEW COMMON-OPTIONS NATIVE MAKE TESTKIT PROBE PASSES ALL SEVEN PDP-6
SIMH CASES: -J WITH SEPARATE/ATTACHED COUNTS, ENABLED/CLEARED .SUFFIXES,
-R IMPLICIT-RULE SUPPRESSION, -T WITHOUT RUNNING A RECIPE, AND STAT OF
THE TOUCHED OUTPUT. DAIMOS SIXBIT COMMAND ARGUMENTS ARRIVE UPPERCASED,
SO MAKE NOW ACCEPTS UPPERCASE -I, -R, -T AS WELL AS LOWERCASE SPELLINGS.
THE JOB-COUNT OPTIONS REMAIN NO-OPS; ALL EXECUTION IS SERIAL.

THE INITIAL IMAGE CHECK FAILURE WAS CAUSED BY MIXING OLDER INSTALLED
MKD6FS/D6FSCK BINARIES WITH THE NEWER CHECKOUT-BUILT TOOLS. THE TEST
NOW BUILDS THE HOST TOOLS AND EXPLICITLY SELECTS THE MATCHING MKD6FS
AND ITS ADJACENT D6FSCK. NO FILESYSTEM CODE WAS CHANGED IN THIS PASS.
ADDITIONAL .SILENT, .IGNORE, .PRECIOUS, -I, AND RECURSIVE MAKEFLAGS
BEHAVIOR REMAINS TO BE COVERED BY NATIVE EXECUTION TESTS.

## NATIVE MAKE SIMH REGRESSION ATTEMPT (2026-10-11)

A NEW DAIMOS-TESTKIT TARGET PROBE COVERS SERIAL -j/-J OPTIONS, ACTIVE AND
CLEARED SUFFIX LISTS, -r, AND -t. NATIVE MAKE HAD PREVIOUSLY COMPILED AND
LINKED, BUT THIS SIMH RUN HAS NOT REACHED TARGET EXECUTION. IMAGE BUILD
FIRST EXPOSED A SIXBIT MANUAL TYPO (FIXED), THEN AN INCORRECT TEST HARNESS
BOOTSTRAP PATH (FIXED); THE CURRENT BLOCKER IS THE EXISTING MKD6FS
POST-FORMAT D6FSCK FAILURE. NO MAKE FEATURE HAS BEEN DECLARED TARGET-VERIFIED.

## NATIVE MAKE SHARED OPTIONS (2026-10-11)

NATIVE MAKE NOW RECOGNIZES .SUFFIXES LISTS AND RESET, TARGET-SCOPED .SILENT
AND .IGNORE, .PRECIOUS DECLARATIONS, AND GLOBAL -i, -r, AND -t. -j AND -J
ACCEPT AN OPTIONAL JOB COUNT WITHOUT ENABLING PARALLEL EXECUTION; MAKE
REMAINS STRICTLY SERIAL. .PRECIOUS REQUIRES NO CLEANUP POLICY CHANGE BECAUSE
NATIVE MAKE DOES NOT DELETE FAILED TARGET OUTPUTS. THE MANUAL NOW DESCRIBES
THE EXISTING INCLUDE HANDLING, $^ EXPANSION, AND DYNAMIC GRAPH STORAGE.

HOST KCC COMPILATION, DAS ASSEMBLY, AND NEW TESTKIT SOURCE CHECKS PASS.
NATIVE PDP-6 SIMH EXECUTION OF THESE NEW PATHS IS NOT YET VERIFIED.
DOUBLE-COLON RULES, AUTOMATIC RECURSIVE MAKEFLAGS PROPAGATION, AND FULL
SUFFIX-PRECEDENCE CONFORMANCE REMAIN OPEN. JOB PARALLELISM IS EXPLICITLY
OUT OF SCOPE.

## DSH SOURCE/RECURSION STACK FRAMES AND FULL TARGET CONFORMANCE (2026-10-10)

THE SOURCE POSITIONAL-RESTORATION AND FUNCTION-RECURSION REGRESSIONS WERE
CAUSED BY NESTED SIMPLE-COMMAND EXECUTION EXHAUSTING THE PDP-6 USER STACK:
EVERY CALL PREVIOUSLY RESERVED TWENTY SEVEN-WORD ARGUMENT RECORDS. DSH NOW
USES A FOUR-RECORD COMMON PATH AND RESERVES THE FULL TWENTY-RECORD SCRATCH
EXTENT THROUGH SYS_BRK ONLY WHEN REQUIRED BY ARITY, ALIAS, OR GLOBBING.
THE SCRATCH EXTENT IS RELEASED IN LIFO ORDER AFTER EXECUTION. NO NEW
PERMANENT BSS OR GENERAL LIBC ALLOCATOR IS REQUIRED; THE TWENTY-WORD
SHELL PARSER LIMIT AND SIXTEEN-ARGUMENT SYS_RUN LIMIT REMAIN DISTINCT.
PIPEDBG DEVELOPMENT OUTPUT IS REMOVED.

THE TESTKIT SIMH HARNESS ALSO NOW MATCHES THE DSH '# ' PROMPT ONLY AT A
TERMINAL LINE START, RATHER THAN MISTAKING AN ECHOED LITERAL '# ' FOR A
PROMPT. WITH BOTH CHANGES, ALL 115 DSH CONFORMANCE PROBES PASS ON PDP-6
SIMH, INCLUDING SOURCE, RECURSION, EIGHT-STAGE PIPELINES, AND FINAL
QUOTE/COMMENT CASES. THE INDEPENDENT REAL-DCS FOREGROUND/TTY TEST PASSES.
SAME-BASE DXR COMPARISON: ORIGINAL IMAGE 035071 WORDS, CORRECTED 035273
(+0202 OCTAL WORDS), BOTH WITH 014445 BSS WORDS. THE EXEC IMAGE REMAINS
BELOW THE 036000-WORD LIMIT. BROADER RESOURCE-LIMIT FEATURE COVERAGE IS
STILL OPEN AND IS TRACKED IN USERLAND/DSH/TODO.MD.

## DSH PIPELINE DISK-PI AND WAKEUP REGISTER FIX (2026-10-10)

PDP-6 TYPE 270 DISK READS FAILED WITH DRL WHILE A PRECEDING PIPELINE CHILD
EXITED: PROC_EXIT_CURRENT HELD THE GLOBAL PI GATE DISABLED THROUGH RESOURCE
TEARDOWN. IT NOW MASKS THE PI6 SCHEDULER CLOCK DURING CLEANUP WHILE ALLOWING
PI3/PI5 STORAGE TRANSFERS. THE PREVIOUS PI6 ENABLE STATE IS RESTORED ON
RETURN TO THE IDLE SCHEDULER, AND THE FINAL-PROCESS HALT DISABLES GLOBAL PI.
THE PI-SAFE PROC_WAKEUP_EVENT ROUTINE ALSO NOW PRESERVES AC7, WHICH IS
MODIFIED BY NESTED PROC_RUNQ_ADD; WITHOUT THAT SAVE, LONG PIPELINES COULD
HALT AT PC 000001. BOTH ERRORS WERE CONFIRMED THROUGH SIMH ISOLATION.

FIVE ISOLATED RUN/PIPE TESTS, 5/6/8-STAGE PIPELINE TESTS, AND REAL-DCS
FOREGROUND/TTY RESTORATION PASS WITH THE CORRECTION. IN THE FULL DSH
CONFORMANCE RUN, 73 PROBES PASSED AND THE TWO INDEPENDENT SOURCE POSITIONAL
RESTORATION AND FUNCTION RECURSION-LIMIT REGRESSIONS REMAIN OPEN. DO NOT
DECLARE FULL DSH CONFORMANCE COMPLETE UNTIL THESE ARE FIXED AND RETESTED.

## NATIVE MAKE DIRECT COMMAND PATH LOOKUP (2026-10-10)

FOLLOW-UP HOST AND NATIVE MAKEFILE CLEANUP: TEN ADDITIONAL DAIMOS
MAKEFILES NOW DEFAULT TO EXECUTABLE NAMES RESOLVED THROUGH PATH INSTEAD
OF THE PREFIX/BIN DIRECTORY OR /OPTION/BASE/EXEC. EXPLICIT TOOL VARIABLE
OVERRIDES REMAIN AVAILABLE. LIBC CHECKS COMMAND AVAILABILITY USING
COMMAND -V; PREFIX-BASED HEADER/LIBRARY PATHS AND ABSOLUTE BOOT-IMAGE
INSTALLATION DESTINATIONS ARE UNCHANGED. TESTKIT VERIFIES 47 DEFAULTS;
GNU MAKE KERNEL AND LIBC DRY RUNS PASS. NATIVE SIMH EXECUTION HAS NOT
BEEN VERIFIED FOR THIS FOLLOW-UP.

NATIVE MAKE DIRECT-RUN RECIPES NOW ACCEPT BARE COMMAND NAMES AND SEARCH THE
IMPORTED PATH VARIABLE FOR REGULAR EXECUTABLE FILES. PREVIOUSLY THESE
RECIPES REQUIRED ABSOLUTE EXECUTABLE PATHS. ABSOLUTE AND EXPLICIT RELATIVE
PATHS REMAIN SUPPORTED. KCC/DAIMOS.MK USES KCC, DAS, DARC, DLINK, INSTALL,
AND RM AS PATH-RESOLVED COMMANDS; THE INSTALLATION DESTINATIONS REMAIN
ABSOLUTE. HOST KCC COMPILATION AND TESTKIT COMMAND-COMPARISON CHECKS PASS.
NATIVE SIMH EXECUTION REMAINS TO BE VERIFIED.

## NATIVE KCC CLEANUP AND RECURSIVE RM (2026-10-10)

THE SYSTEM C IMPLEMENTATION OF RM NOW SUPPORTS BOUNDED RECURSIVE REMOVAL
(-R), OPTIONAL ERROR SUPPRESSION (-F), AND FINAL-COMPONENT SYMLINK HANDLING
THROUGH PARENT DIRECTORY ENTRY LOOKUP. ROOT AND DOT/DOT-DOT PATH COMPONENTS
ARE REFUSED FOR RECURSION. THE OBSOLETE OPTIONAL ASSEMBLER RM IS REMOVED
FROM SOURCE/INSTALL RULES, INCLUDING REMOVAL OF A PREVIOUSLY INSTALLED
ASSEMBLER RM. KCC/DAIMOS.MK NOW USES A COMMON .S.DOBJ SUFFIX RULE AND
CLEAN INVOKES RM -R -F BUILD. THE CURRENT HOST ENTRY POINT USES CROSS.MK,
COMMON.MK AND NATIVE.MK; UNIX.MK WAS REMOVED.

HOST KCC COMPILATION AND TESTKIT MOCK-VFS RECURSION, SUFFIX-INFERENCE,
AND STRUCTURAL TESTS PASS. NATIVE SIMH EXECUTION OF THE NEW SYSTEM RM AND
ON-TARGET MAKE SUFFIX RULE REMAINS UNVERIFIED.

## KCC CANONICAL CROSS/NATIVE TARGETS (2026-10-10)

THE SHARED KCC MAKEFILE STILL SELECTS UNIX.MK ON HOSTS AND
DAIMOS.MK AFTER NATIVE SIXBIT STAGING. UNIX `make` DEFAULTS TO
`all: cross`; `make cross` BUILDS THE UNIX-HOSTED KCC CROSS-
COMPILER AND HOST RUNTIME. `make native` BUILDS THE DAIMOS
COMPILER PHASES/DRIVER, RUNTIME AND LIBC USING THE CROSS TOOLCHAIN.
LEGACY PUBLIC ALIASES HOST-BUILT, NATIVE-BUILT, KCC, ASM,
SELF-ASM, AND NATIVE-* CONVENIENCE TARGETS WERE REMOVED;
INTERNAL ARTIFACT TARGETS AND MAINTENANCE INSTALL/CLEAN TARGETS
REMAIN. UNDER DAIMOS, NATIVE IS THE FIRST BUILD TARGET AND
THE ONLY PUBLIC BUILD TARGET; PHASE ALIASES AND ALL WERE REMOVED.
GNU MAKE AND ISOLATED BSD BMAKE `-n` VERIFIED UNIX DEFAULT=CROSS,
NATIVE TARGET SELECTION, AND REJECTION OF OLD ALIASES. THE
NATIVE PLATFORM GRAPH WAS CHECKED STRUCTURALLY. NO COMPILER
EXECUTION WAS PERFORMED; THIS DOES NOT ESTABLISH THAT KCC WORKS.
REGRESSIONS ARE IN DAIMOS-TESTKIT.

## KCC GNU/BSD MAKE ASCII COMPATIBILITY (2026-10-10)

KCC/DAIMOS.MK IS ALREADY ASCII IN THE REPOSITORY; ONLY THE STAGED
DAIMOS.MK IS CONVERTED TO SIXBIT RECORDS. THE ASCII DAIMOS FILE
PASSES GNU MAKE AND ISOLATED BSD BMAKE CLEAN DRY-RUN PARSING.
IT IS NOT A DIRECT HOST BUILD: NATIVE RECIPES REFER TO SIXBIT
STAGED UPPERCASE SOURCE NAMES AND /OPTION/BASE/EXEC/KCC PATHS.
THE UNIX.MK FILE IS THE HOST/CROSS BUILD CONTRACT. TEN ADJACENT
+= ASSIGNMENT GROUPS WERE WRAPPED WITH PORTABLE BACKSLASHES;
LINES FELL FROM 561 TO 492 WITH NO CHANGE TO VARIABLE INVENTORIES.
GNU MAKE AND BMAKE EXPAND SIX REPRESENTATIVE HOST/NATIVE MODULE
INVENTORIES TO IDENTICAL LISTS AND ACCEPT CLEAN DRY RUNS. CLEAN
COMMAND OUTPUT IS UNCHANGED BEFORE/AFTER THE FORMATTING CHANGE.
BMAKE WAS OBTAINED AS AN EXTRACTED PACKAGE IN A TEMP DIRECTORY;
NO SYSTEM INSTALLATION WAS REQUIRED. THE PORTABILITY CHECKER IS
IN DAIMOS-TESTKIT. NEITHER BACKEND RAN THE BROKEN NATIVE KCC;
NO NEW COMPILER EXECUTABLE WAS PRODUCED OR CLAIMED.

## NATIVE KCC MAKEFILE DRY-RUN VALIDATION BLOCKED (2026-10-10)

A SAFE NATIVE SIMH PREFLIGHT WAS PREPARED WITH MAKE -Q / MAKE -N
ONLY: NO KCC, DAS, LINKER, OR CLEAN RECIPE EXECUTION IS ALLOWED.
IT USES THE NORMALIZED KCC DAIMOS.MK WITH PORTABLE $@/$< DAS
RECIPES. THE HOST SEMANTIC CHECK STILL SHOWS 267 EXPANDED RECIPE
COMMANDS AND 140 RULE TARGETS IDENTICAL TO THE PRECHANGE INPUT.
GNU MAKE -N CLEAN PARSES SUCCESSFULLY. THE NATIVE PREFLIGHT
COULD NOT START SIMH: HOST BOOT IMAGE GENERATION FAILED DURING
MKD6FS POST-FORMAT D6FSCK, REPORTING ALLOCATED LEAKED BLOCKS
14213..14217 AND ABORTING ON DSK0.DSK. THIS IS A SEPARATE BOOT
IMAGE VALIDATION FAILURE, NOT EVIDENCE OF A MAKEFILE REGRESSION.
DO NOT CLAIM A PASS FOR NATIVE -N / -Q UNTIL A VALID IMAGE CAN BE
BUILT; DO NOT INVOKE THE STILL-BROKEN NATIVE KCC COMPILER.
THE NO-RECIPE-EXECUTION PREFLIGHT IS PRESERVED IN DAIMOS-TESTKIT.

## NATIVE KCC PORTABLE AUTOMATIC-VARIABLE RECIPES (2026-10-10)

THE NATIVE MAKE PARSER ALREADY SUPPORTS THE PORTABLE AUTOMATIC
VARIABLES $@ (TARGET), $< (FIRST PREREQUISITE), $? (NEWER
PREREQUISITES), AND $* (STEM) IN RECIPES. NO PARSER CHANGE WAS
NEEDED. THE KCC DAIMOS.MK FILE NOW USES $@ $< IN 53 ONE-INPUT
DAS ASSEMBLER RECIPES IN PLACE OF REPEATED OUTPUT/INPUT NAMES.
THE DAIMOS-TESTKIT HOST CHECK EXPANDS BOTH SIDES AND VERIFIES
THAT ALL 267 RECIPE COMMANDS AND 140 TARGET DECLARATIONS ARE
UNCHANGED. GNU MAKE ACCEPTS THE SOURCE (A FULL NATIVE SIMH
INCREMENTAL BUILD ON THE REVISED SOURCE REMAINS TO BE RUN).
COMPILER AND LINKER RECIPES REMAIN EXPLICIT BECAUSE THEIR
PHASE-SPECIFIC FLAGS AND MULTIPLE INPUTS MAKE THEM DIFFERENT.
ALL FURTHER MAKE WORK SHOULD SERVE THE ACTUAL KCC MAKEFILE; DO
NOT EXTEND THE PARSER WITHOUT A DEMONSTRABLE PORTABLE REQUIREMENT.

## NATIVE KCC MAKEFILE PORTABLE CONTINUATIONS (2026-10-10)

THE KCC DAIMOS.MK FILE NOW COMBINES ADJACENT TARGET DEPENDENCIES
INTO READABLE PHYSICAL LINES (USUALLY <= 96 CHARACTERS), USING
STANDARD BACKSLASH CONTINUATIONS WHEN A TARGET NEEDS MORE THAN ONE
PHYSICAL LINE. NATIVE MAKE ALREADY IMPLEMENTS THIS CONTINUATION
SYNTAX, SO NO MAKE PARSER FEATURE CHANGE WAS NEEDED.
COMPLETED NORMALIZATION REDUCED THE FILE FROM 1038 TO 460 LINES
INCLUDING NEW COMMENTS; THE LONGEST JOINED LOGICAL LINE IS 233
CHARACTERS (NATIVE LIMIT 256). DAIMOS-TESTKIT VALIDATION PRESERVES
ALL ORDERED PREREQUISITES, RECIPE LINES, AND DIRECTIVES FROM THE
ORIGINAL FILE. GNU MAKE -N CLEAN ACCEPTS THE NORMALIZED FILE.
THE TARGET/DEPENDENCY GRAPH IS UNCHANGED, SO A PEAK-RAM REDUCTION
IS NOT CLAIMED. NATIVE SIMH BUILD INVALIDATION REMAINS TO BE
TESTED. CHEAP PORTABLE FEATURES SHOULD BE EVALUATED ONLY WHEN A
REAL MAKEFILE REQUIREMENT EXISTS; .PHONY, .SUFFIXES, AND BASIC
BACKSLASH CONTINUATIONS ARE ALREADY IMPLEMENTED. DO NOT ADD
GNU-ONLY OR BSD-ONLY SYNTAX TO THE SHARED BUILD CONTRACT.

## NATIVE MAKE END-TO-END PEAK BRK VERIFIED (2026-10-10)

MATCHED NATIVE PDP-6 SIMH BEFORE/AFTER RUNS, 13E85C6 VS 5B0E768,
INTERCEPTED ALL MAKE-LINKED LIBC DSYS_BRK SITES, LOGGED HIGHEST
SUCCESSFUL BREAK, AND EMITTED AN ADDITIONAL SNAPSHOT IMMEDIATELY
BEFORE MAKE MAIN RETURNED. ALL FIVE PROBES PASS FOR BOTH VERSIONS.
LARGE-GRAPH MAXIMUM BREAK WORD ADDRESSES WERE 43051 BASELINE AND
37302 OPTIMIZED, DIFFERENCE -5749 WORDS. THE EXIT BREAK AND PEAK
MATCHED IN BOTH VERSIONS; THE EXECUTION PHASE DID NOT INCREASE THE
BREAK ABOVE THE POST-PARSE LEVEL. EXIT FREE-LIST WORDS WERE
10847/5096; LARGEST BLOCKS 7218/2752. BOTH VERSIONS HAVE TWO
ARENA RELOCATIONS; THE LATER RELOCATION IS EARLIER WITH THE
SEPARATE SCRATCH STACK (20000->25000 VS 31250->39062 CHARS).
THESE ARE PROCESS BREAK ADDRESSES, NOT NET ALLOCATED WORDS, AND
PROFILE BUILD LAYOUT CAN SHIFT ABSOLUTE VALUES. THE DIFFERENCE
IS THE MATCHED-END-TO-END MAXIMUM BREAK REDUCTION IN THIS FIXTURE.
NO PRODUCTION PROFILING INSTRUMENTATION IS RETAINED.

## NATIVE MAKE COMPLETE LIBC BREAK-EXTENSION PROFILE (2026-10-10)

AN ISOLATED PDP-6 SIMH BEFORE/AFTER PROFILE INTERCEPTED EVERY
DSYS_BRK CALL SITE IN THE MAKE-LINKED LIBC, INCLUDING THE MALLOC,
REALLOC, BRK AND SBRK PATHS. ALL FIVE NATIVE MAKE TESTS PASSED FOR
EACH BUILD. AT THE LARGE-GRAPH POST-PARSE CHECKPOINT, THE MAXIMUM
BREAK OBSERVED SO FAR WAS 43051 (BASELINE 13E85C6) AND 37302
(SEPARATE SCRATCH 5B0E768), A REDUCTION OF 5749 PDP-6 WORDS.
CURRENT BREAKS EQUALED THOSE MAXIMA AT THE CHECKPOINT. FREE-LIST
TOTALS WERE 10879/5128 WORDS; LARGEST FREE BLOCKS 7218/2752.
ARENA REALLOCATIONS REMAIN TWO IN EACH BUILD. BASELINE RELOCATIONS
OCCUR AT 8192->10240 AND 31250->39062 CHARACTERS; OPTIMIZED
RELOCATIONS AT 8192->10240 AND 20000->25000 CHARACTERS.
THE TEST RECORDS 76/77 LIBC BREAK CALLS AND 18/21 SUCCESSFUL NONZERO
BREAK SET REQUESTS (WHICH ARE NOT NECESSARILY ALL INCREASES).
THE RESULT IS THE PEAK THROUGH THE POST-PARSE CHECKPOINT, NOT A
PROVEN END-TO-END MAXIMUM: LATER DEPENDENCY EXECUTION MAY ALLOCATE.
ABSOLUTE BREAK ADDRESSES ARE NOT TOTAL PROCESS ALLOCATION SIZES.
THE DIFFERENT 37302 VALUE FROM THE EARLIER 36278 SNAPSHOT IS A
PROFILING-BUILD LAYOUT EFFECT; COMPARE MATCHED INSTRUMENTED RUNS,
NOT ABSOLUTE ADDRESSES ACROSS DIFFERENT INSTRUMENTATIONS.
PRODUCTION MAKE AND LIBC REMAIN UNINSTRUMENTED. THE HOST PARSER IN
DAIMOS-TESTKIT VALIDATES THE COMPARISON.

## NATIVE MAKE SCRATCH-SEPARATION HEAP A/B (2026-10-10)

ISOLATED PDP-6 SIMH LARGE-GRAPH PROBES WITH TEMPORARY LIBC HEAP
SNAPSHOTS COMPARE 13E85C6 AGAINST 5B0E768. ALL FIVE NATIVE MAKE
PROBES PASS FOR BOTH BUILDS. AT THE SAME POST-PARSE CHECKPOINT,
PROCESS BREAK WORD ADDRESS FALLS FROM 43051 TO 36278 (6773 WORDS),
FREE-LIST TOTAL FROM 10879 TO 5128 (5751 WORDS), AND LARGEST FREE
BLOCK FROM 7218 TO 2752 (4466 WORDS). THE STORED STRING COUNT IS
IDENTICAL AT 35647 NINE-BIT CHARACTERS, WHILE TRIMMED ARENA
CAPACITY FALLS FROM 42644 TO 35648 CHARACTERS. THIS CONFIRMS A
SUBSTANTIALLY LOWER POST-PARSE HEAP BREAK AND LESS FREE-LIST
FRAGMENTATION, NOT MERELY SMALLER NOMINAL ARENA CAPACITY.
THE BREAK VALUES ARE ABSOLUTE WORD ADDRESSES; THE DIFFERENCE IS
NOT THE TOTAL PROCESS MEMORY OR A DIRECT MEASURE OF ITS HIGH-WATER
MARK. THE TEMPORARY HIGH COUNTER DOES NOT TRACK ALL BRK EXTENSIONS
THROUGH REALLOC, SO IT MUST NOT BE CLAIMED AS AN EXECUTION-WIDE
MAXIMUM. PRODUCTION LIBC AND MAKE WERE NOT INSTRUMENTED.
DAIMOS-TESTKIT HAS A HOST PARSER FOR THESE MATCHED SNAPSHOTS.

## NATIVE MAKE SCRATCH SEPARATION A/B (2026-10-10)

MATCHED NATIVE PDP-6 SIMH A/B EXCLUDED BUILD/BOOT/LOGIN AND
COMPARED BASELINE 13E85C6 VS SEPARATE IMPLICIT SCRATCH 5B0E768.
ALL FIVE PROBES PASSED. FRAGMENT 1.131/1.029 SECONDS, INCLUDE
1.130/1.130, SUFFIX 1.028/1.130, LARGE GRAPH 107.879/106.338
(-1.541 S, -1.43%), INDEX ORDER 1.642/1.539. THESE SMALL SPEED
DIFFERENCES ARE FROM ONE MATCHED PAIR, NOT STATISTICALLY PROVEN.
THE FIXED 33-LEVEL IMPLICIT SCRATCH ARRAY IS ALLOCATED ONCE AFTER
PARSING; IT NO LONGER INFLATES EACH STRING-ARENA GROWTH REQUIREMENT.
NO PER-RECURSION MALLOC IS INTRODUCED. ACTUAL PROCESS HIGH-WATER
BREAK AND FREE-LIST FRAGMENTATION MUST STILL BE PROFILED; LOWER
ARENA CAPACITY ALONE IS NOT PROOF OF LOWER TOTAL PEAK HEAP.
THE REPRODUCER IS IN DAIMOS-TESTKIT TESTS/VALIDATION.

## NATIVE MAKE SEPARATE IMPLICIT SCRATCH STACK (2026-10-10)

THE FIXED (MAKE_MAX_DEPTH+1) IMPLICIT RULE SCRATCH ARRAY IS NOW
ALLOCATED ONCE, AFTER PARSING, RATHER THAN RESERVED AT THE TOP OF
THE GROWING STRING ARENA. MAKE_STORE GROWS FOR STORED STRINGS ONLY;
POST-PARSE ARENA SHRINK RETAINS WORD ALIGNMENT AND ALL STRING
OFFSETS. MAKE_BUILD ADDRESSES A DEDICATED PREALLOCATED SCRATCH
ARRAY BY RECURSION DEPTH; NO PER-RECURSION ALLOCATION IS USED.
A FAILED SCRATCH ALLOCATION REPORTS A NORMAL MAKE STORAGE ERROR.
KCC/DAS COMPILATION AND ALL FIVE NATIVE PDP-6 SIMH MAKE REGRESSIONS
PASS. EFFECT ON PEAK HEAP AND RUNTIME REQUIRES ISOLATED A/B
BENCHMARKING; THE CHANGE IS A TESTED CANDIDATE, NOT PROOF OF A
REDUCED PROCESS HIGH-WATER MARK.

## NATIVE MAKE RELOCATION BLOCK OWNER IDENTIFICATION (2026-10-10)

A TEMPORARY DETACHED WORKTREE WITH WORD-ADDRESS ALLOCATION TRACE
LOGGED EVERY MAKE_GROW AND MAKE_GROW_GRAPH REALLOC RESULT. ALL FIVE
NATIVE PDP-6 SIMH MAKE REGRESSIONS PASS. MATCHING THE ARENA BLOCK
END AGAINST THE START OF THE NEXT LIVE ALLOCATION SHOWS:
AT ARENA CAPACITY 8192 CHARS, END WORD 14338 IS FOLLOWED BY
THE VARIABLE TABLE (INITIAL=16, RECORD SIZE=12 CHARACTERS,
CAPACITY=16), OCCUPYING 50 WORDS INCLUDING MALLOC HEADER.
AT ARENA CAPACITY 31250 CHARS, END WORD 22505 IS FOLLOWED BY
THE DEPENDENCY-LINK TABLE (INITIAL=64, RECORD SIZE=4 CHARACTERS,
CAPACITY=324), OCCUPYING 326 WORDS INCLUDING HEADER.
THIS IDENTIFIES THE OWNERS OF BOTH BLOCKING LIVE ALLOCATIONS.
THE GENERAL LIBC IN-PLACE REALLOC PATH IS FUNCTIONING; THE
FOLLOWING MEMORY IS LIVE, NOT FREE. A STORE-FIRST MAKE EXPERIMENT
PREVIOUSLY LEFT BOTH RELOCATIONS UNCHANGED. FUTURE OPTIONS MUST
BE EVALUATED AGAINST TOTAL PEAK HEAP AND COPYING WORK; SIMPLY
REORDERING AN INSERTION OR INCREASING ALL INITIAL CAPACITIES IS
NOT A PROVEN WIN. TEMPORARY INSTRUMENTATION IS NOT PRODUCTION CODE.

## NATIVE MAKE ARENA NEIGHBOR BLOCK PROFILE (2026-10-10)

DETACHED TEMPORARY LIBC INSTRUMENTATION REPORTED THE ARENA BLOCK'S
ENDING PDP-6 WORD ADDRESS, WHETHER THE PHYSICALLY ADJACENT BLOCK
WAS FREE, ITS FREE-BLOCK SIZE, AND THE CURRENT PROCESS BREAK.
ALL FIVE NATIVE SIMH MAKE PROBES PASSED. LARGE-GRAPH GROWTH AT
8192 CHARS: ARENA END=14338, BREAK=14690, NO ADJACENT FREE BLOCK;
AT 31250 CHARS: END=22505, BREAK=25926, NO ADJACENT FREE BLOCK.
THUS BOTH OBSERVED ARENA RELOCATIONS ARE NECESSARY UNDER THE
EXISTING ALLOCATION LAYOUT: NEITHER DIRECT BREAK EXPANSION NOR
ADJACENT-FREE-BLOCK EXPANSION IS AVAILABLE. IN-PLACE GROWTH AT
10240, 12800, 16000, 20000, 25000, AND 39062 CHARS OCCURRED
WHEN THE ARENA REACHED THE BREAK. THE GENERAL LIBC ALLOCATOR IS
NOT SHOWN DEFECTIVE BY THIS PROFILE; NO LIBC OR PRODUCTION MAKE
MODIFICATIONS WERE JUSTIFIED. FURTHER IMPROVEMENT REQUIRES
IDENTIFYING THE OWNER/LIFETIME OF THE FOLLOWING ALLOCATED BLOCK
AND TESTING A SPECIFIC LAYOUT CHANGE. DAIMOS-TESTKIT HAS A HOST
PARSER FOR THE TEMPORARY NATIVE TRACE.

## NATIVE MAKE STORE-FIRST ALLOCATION EXPERIMENT REJECTED (2026-10-10)

AN ISOLATED EXPERIMENT MOVED MAKE_STORE/MAKE_DEP_INTERN AHEAD OF
GRAPH TABLE CAPACITY GROWTH IN MAKE_GET_RULE AND MAKE_ADD_DEP.
KCC/DAS AND ALL FIVE NATIVE PDP-6 SIMH REGRESSION PROBES PASSED.
A REPEATED INSTRUMENTED LARGE-GRAPH ARENA GROWTH PROFILE HAD THE
SAME TWO RELOCATIONS AS BASELINE: 8192->10240 AND 31250->39062
NINE-BIT CHARACTERS. ALL OTHER ARENA GROWTH WAS IN PLACE.
THERE IS NO CONFIRMED FRAGMENTATION BENEFIT, SO THE SOURCE EXPERIMENT
WAS REVERTED WITHOUT A PRODUCTION CODE CHANGE. FURTHER WORK SHOULD
IDENTIFY THE ACTUAL BLOCK DIRECTLY ADJACENT TO THE ARENA USING A
WORD-ADDRESS-AWARE HEAP TRACE, RATHER THAN INFER THE BLOCKER FROM
NEARBY MAKE-GROW EVENTS. LIBC REALLOC ALREADY SUPPORTS FREE-NEIGHBOR
AND BREAK-ADJACENT IN-PLACE EXPANSION.

## NATIVE MAKE ADDRESS-LEVEL ARENA GROWTH CHECK (2026-10-10)

AN INDEPENDENT DETACHED WORKTREE TRACED RAW ALLOCATOR POINTER VALUES
FOR MAKE_GROW (THE STRING ARENA) ON THE NATIVE PDP-6 SIMH LARGE-GRAPH
FIXTURE. ALL FIVE NATIVE MAKE REGRESSIONS PASS. NINE ALLOCATION
EVENTS OCCUR: ONE INITIAL ALLOCATION; TWO MOVING REALLOCS AT
8192->10240 AND 31250->39062 CHARACTERS; SIX IN-PLACE REALLOCS.
POINTERS ARE KCC BYTE POINTER VALUES, NOT DIRECT WORD ADDRESSES.
THIS TRACE CONFIRMS ARENA MOVEMENT BUT DOES NOT IDENTIFY WHICH
INTERLEAVED RULE/DEPENDENCY/HASH ALLOCATION BLOCKED CONTIGUOUS
GROWTH. DO NOT CHANGE GENERAL LIBC REALLOC OR MAKE'S ALLOCATION
ORDER WITHOUT IDENTIFYING THE INTERVENING BLOCK AND BENCHMARKING
THE RESULT. PRODUCTION SOURCE WAS NOT MODIFIED. TESTKIT CONTAINS
A REUSABLE TRACE PARSER.

## NATIVE MAKE ARENA REALLOC MOVEMENT TRACE (2026-10-10)

TEMPORARY MAKE_GROW INSTRUMENTATION IN A DETACHED WORKTREE LOGGED
EVERY ARENA REALLOC'S OLD CAPACITY, NEW CAPACITY, AND WHETHER THE
ALLOCATED POINTER MOVED. ALL FIVE NATIVE PDP-6 SIMH MAKE PROBES PASS.
LARGE-GRAPH ARENA CHARS: INITIAL 0->8192; RELOCATION 8192->10240;
IN-PLACE 10240->12800->16000->20000->25000->31250;
RELOCATION 31250->39062; IN-PLACE 39062->48827.
TWO OF EIGHT REALLOCATIONS OF AN EXISTING ARENA RELOCATED IT.
THE SECOND RELOCATION FREES AN OLD 31250-CHAR (~7813-WORD) ARENA;
THE PREVIOUSLY REPORTED LARGEST FREE BLOCK WAS 7218 WORDS, SO THE
TWO OBSERVATIONS ARE RELATED CANDIDATES, NOT A PROVEN IDENTICAL
BLOCK. ITS EXACT ORIGIN NEEDS FREE-LIST ADDRESS AND ALLOCATION-
SITE TRACING. DO NOT ALTER THE GENERAL LIBC ALLOCATOR BASED ONLY
ON THIS MAKE-SPECIFIC SNAPSHOT. PRODUCTION MAKE/LIBC ARE UNCHANGED;
DAIMOS-TESTKIT CONTAINS THE MOVEMENT-TRACE PARSER.

## NATIVE MAKE ALLOCATOR FREE-LIST SNAPSHOTS (2026-10-10)

A DETACHED INSTRUMENTED WORKTREE ADDED A TEMPORARY LIBC HEAP-FREE-
LIST PROBE, AND NATIVE MAKE PRINTED BREAK, FREE WORDS, AND LARGEST
FREE BLOCK BEFORE AND AFTER POST-PARSE ARENA SHRINK. ALL FIVE PDP-6
SIMH MAKE PROBES PASS. THE LARGE GRAPH REPORTS BREAK=43051 BOTH
TIMES, FREE WORDS=9333/10879 AND LARGEST FREE BLOCK=7218/7218.
THUS THE TRIM RETURNED 1546 ADDITIONAL 36-BIT WORDS TO THE FREE
LIST; THE PROCESS BREAK DID NOT FALL. THE PRE-TRIM ARENA CAPACITY
WAS 48827 NINE-BIT CHARACTERS AND USED LENGTH 35647. THIS IS
ALREADY A 5/4-GROWTH BUILD. ABSOLUTE BREAK IS AN ADDRESS, NOT A
MEASURE OF HEAP FOOTPRINT, AND THIS SNAPSHOT DOES NOT MEASURE THE
FULL EXECUTION HIGH-WATER MARK OR NUMBER OF REALLOC COPYING EVENTS.
OTHER PROBES RECOVERED 259, 204, 204, AND 389 FREE WORDS FROM
POST-PARSE TRIMMING. THE 7218-WORD LARGEST FREE BLOCK IN THE LARGE
GRAPH REMAINED UNCHANGED; INVESTIGATE TEMPORARY ALLOCATION MOVEMENT
AND FRAGMENTATION BEFORE FURTHER ARENA GROWTH CHANGES. PRODUCTION
MAKE/LIBC WERE NOT INSTRUMENTED. HOST PROFILE PARSER IS IN TESTKIT.

## NATIVE MAKE FIVE-QUARTERS ARENA GROWTH A/B (2026-10-10)

MATCHED NATIVE PDP-6 SIMH A/B MEASURED DSH COMMAND TO PROMPT, NOT
BOOT OR IMAGE BUILD: BASELINE C54ACA1 VS 5/4 ARENA GROWTH 0B24B62.
ALL FIVE PROBES PASS. FRAGMENT 1.028/1.027, INCLUDE 1.131/1.131,
SUFFIX 1.028/1.028, LARGE GRAPH 108.181/107.563 (-0.618 S,
-0.57%), INDEX ORDER 1.641/1.641 SECONDS. THE SMALL SPEED
DIFFERENCE FROM ONE PAIR IS NOT STATISTICALLY ESTABLISHED.
FOR THE PREVIOUSLY MEASURED 42644-CHARACTER ARENA REQUIREMENT,
5/4 GROWTH FROM 8192 REACHES 48827 CHARACTERS RATHER THAN 65536,
A PROJECTED CAPACITY REDUCTION OF 16709 NINE-BIT CHARS OR 4177
36-BIT WORDS (ROUNDED DOWN). THIS IS NOT MEASURED PEAK PROCESS
RAM; HEAP REALLOC FRAGMENTATION IS STILL TO BE CHECKED. THE
ALIGNED POST-PARSE ARENA TRIM REMAINS. SEE DAIMOS-TESTKIT'S NATIVE
MAKE ARENA-125 A/B REPRODUCER.

## NATIVE MAKE FIVE-QUARTERS ARENA GROWTH (2026-10-10)

STRING ARENA GEOMETRIC GROWTH IS REDUCED FROM 2X TO 5/4X TO LOWER
PEAK ALLOCATED ARENA RAM DURING PARSING; GRAPH ARRAYS RETAIN 3/2X.
THE MEASURED LARGE-GRAPH SHRINK TARGET FROM THE PREVIOUS PROFILE IS
42644 NINE-BIT CHARS. STARTING FROM 8192, THE NEW GROWTH SERIES
REACHES 48827 CHARS RATHER THAN 65536: A CAPACITY DIFFERENCE OF
16709 CHARS, OR APPROXIMATELY 4177 PDP-6 WORDS. THIS IS A
CAPACITY PROJECTION FROM THE RECORDED WORKLOAD, NOT A HEAP-BREAK OR
PEAK-PROCESS-RAM MEASUREMENT; ALLOCATOR REALLOC WORK MAY INCREASE.
THE POST-PARSE WORD-ALIGNED ARENA TRIM REMAINS ACTIVE. KCC/DAS AND
ALL FIVE NATIVE SIMH MAKE PROBES PASS. ISOLATED A/B TIMING IS NEXT.

## NATIVE MAKE ACTUAL ARENA CAPACITY PROFILE (2026-10-10)

TEMPORARY INSTRUMENTATION IN A DETACHED DAIMOS WORKTREE PRINTED
ARENA CHARACTERS USED, ALLOCATED BEFORE SHRINK AND ALIGNED TARGET
CAPACITY. THE FIVE NATIVE PDP-6 SIMH PROBES ALL PASS. LARGE GRAPH:
USED=35647, BEFORE CAPACITY=65536, TARGET=42644 9-BIT CHARACTERS.
THE POST-PARSE SHRINK RETURNS 22892 CHARACTERS = 5723 36-BIT WORDS
OF ALLOCATED ARENA PAYLOAD (SUBJECT TO LIBC ALLOCATOR TAIL-SPLITTING).
OTHER PROBES: FRAGMENT 8192->7156 (259 WORDS); INCLUDE AND SUFFIX
8192->7376 (204 WORDS EACH); INDEX-ORDER 16384->8684 (1925 WORDS).
THE TARGET PRESERVES THE FULL 33-LEVEL IMPLICIT SCRATCH RESERVATION;
THIS IS NOT AN INCREASE IN THE HIGH-WATER-MARK BREAK OR AN ACTUAL
PEAK PROCESS-RAM MEASUREMENT. THE ARENA'S PEAK STILL OCCURS WHILE
PARSING. INSTRUMENTATION WAS NOT ADDED TO PRODUCTION MAKE SOURCE.
TESTKIT HOST PARSER: DAIMOS-NATIVE-MAKE-ARENA-ACCOUNTING-20261010-V1.PY.

## NATIVE MAKE POST-PARSE ARENA TRIM A/B (2026-10-10)

MATCHED PDP-6 SIMH A/B (DSH COMMAND TO PROMPT; IMAGE BUILD/BOOT/LOGIN
EXCLUDED) COMPARED BASELINE 937D01F AGAINST POST-PARSE WORD-ALIGNED
ARENA TRIM 9F76395. ALL FIVE PROBES PASS. FRAGMENT 1.029/1.028 S,
INCLUDE 1.028/1.028, SUFFIX 1.028/1.028, LARGE GRAPH
109.534/108.419 (-1.115 S, -1.02%), INDEX 1.540/1.540 S.
A SINGLE MATCHED PAIR IS INSUFFICIENT TO ESTABLISH A RELIABLE SPEEDUP.
THE TRIM RETURNS UNUSED GEOMETRIC ARENA CAPACITY AFTER PARSING WHILE
RETAINING ALL STRINGS AND THE ALIGNED 33-SLOT IMPLICIT SCRATCH STACK.
THE PREVIOUS PROFILE REPORTED 35647 STORED ARENA CHARACTERS, BUT THIS
A/B DID NOT DIRECTLY MEASURE PRE- AND POST-TRIM ALLOCATIONS OR PEAK
PROCESS RAM. NO QUANTIFIED RESIDENT-WORD SAVING IS CLAIMED YET.
THE REPRODUCER IS IN DAIMOS-TESTKIT TESTS/VALIDATION.

## NATIVE MAKE POST-PARSE ARENA TRIM (2026-10-10)

AFTER MAKEFILE PARSING, MAKE RETURNS SPARE GEOMETRIC STRING-ARENA
CAPACITY TO THE DAIMOS LIBC ALLOCATOR WITH REALLOC SHRINK-IN-PLACE.
THE RETAINED ARENA SIZE IS WORD-ALIGNED AND INCLUDES ALL STORED
STRINGS AND THE ENTIRE (MAKE_MAX_DEPTH+1) IMPLICIT-RULE SCRATCH
STACK. NO STRING POINTERS OR GRAPH INDICES ARE INVALIDATED;
SHRINK FAILURE LEAVES THE ORIGINAL ARENA VALID. THE DAIMOS LIBC
REALLOC IMPLEMENTATION RETURNS A FREE TAIL WHEN LARGE ENOUGH.
KCC/DAS AND ALL FIVE NATIVE PDP-6 SIMH MAKE TESTS PASS ON THE FINAL
WORD-ALIGNED IMPLEMENTATION. SAVINGS DEPEND ON LAST GEOMETRIC GROWTH;
ACTUAL PEAK PROCESS RAM AND WALL-TIME A/B REMAIN TO BE MEASURED.

## NATIVE MAKE THREE-HALVES GRAPH GROWTH A/B (2026-10-10)

MATCHED NATIVE SIMH RUN EXCLUDING IMAGE BUILD, BOOT AND LOGIN:
BASELINE 34EF931 VS GRAPH 3/2-GROWTH 594232C; ALL FIVE PROBES PASS.
FRAGMENT 1.028/1.132 S, INCLUDE 1.131/1.028, SUFFIX 1.028/1.028,
LARGE GRAPH 110.553/111.475 (+0.922 S, +0.83%), INDEX 1.643/1.641.
ONE A/B PAIR CANNOT ESTABLISH A PRECISE PERFORMANCE REGRESSION.
RETAIN 3/2 GRAPH GROWTH FOR AN ESTIMATED 2390 FEWER 36-BIT WORDS
OF RULE, ORDER AND DEPENDENCY CAPACITY ON THIS FIXTURE. ACTUAL
PEAK PROCESS RAM AND ALLOCATOR FRAGMENTATION REMAIN UNMEASURED.
THE BYTE STRING ARENA STILL DOUBLES. TESTKIT VALIDATION CONTAINS
THE REPRODUCIBLE MATCHED-NATIVE A/B DRIVER.

## NATIVE MAKE THREE-HALVES GRAPH GROWTH (2026-10-10)

GRAPH TABLES (RULES, SORTED RULE ORDER, DEPENDENCIES, RECIPES,
VARIABLES) NOW GROW BY APPROXIMATELY 3/2 RATHER THAN DOUBLING.
THE STRING ARENA KEEPS DOUBLING TO LIMIT DATA COPYING. FOR THE
681-LHS/1080-LINK LARGE-GRAPH FIXTURE, CAPACITIES CHANGE AS FOLLOWS:
RULES 1024 TO 819 (SIX WORDS EACH, SAVING 1230 WORDS), RULE ORDER
1024 TO 819 (205 WORDS), DEPENDENCIES 2048 TO 1093 (955 WORDS).
TOTAL ESTIMATED ALLOCATED GRAPH-TABLE SAVINGS ARE 2390 36-BIT WORDS
BEFORE RECIPE/VARIABLE TABLES AND ALLOCATOR HEADERS. THESE ARE
CAPACITY ESTIMATES, NOT PEAK USER-RAM MEASUREMENTS. KCC/DAS AND
ALL FIVE NATIVE SIMH MAKE PROBES PASS. ISOLATED A/B TIME IS PENDING.

## NATIVE MAKE ONE-WORD LINK A/B — RAM-FIRST RETAINED (2026-10-10)

MATCHED NATIVE SIMH A/B EXCLUDING IMAGE BUILD/BOOT/LOGIN: BASELINE
48AAB5B VS ONE-WORD DEPENDENCY/RECIPE LINKS EE1F377. ALL FIVE NATIVE
PROBES PASSED IN BOTH. FRAGMENT 1.028/1.030 S; INCLUDE 1.132/1.131;
SUFFIX 1.031/1.030; LARGE GRAPH 113.112/115.187 (+2.075 S, +1.83%);
INDEX ORDER 1.644/1.642 S. ONE A/B RUN CANNOT ESTABLISH A PRECISE
RUNTIME REGRESSION. KEEP THE PACKED LINKS FOR RAM SAVINGS: AT 2048
ALLOCATED DEPENDENCY LINKS, SAVES 2048 36-BIT WORDS; EACH ALLOCATED
RECIPE LINK SAVES ONE FURTHER WORD. THIS IS TABLE CAPACITY, NOT A
MEASURE OF PEAK PROCESS MEMORY OR FULL EXECUTABLE SIZE. BENCHMARK
REPRODUCER IS IN DAIMOS-TESTKIT UNDER TESTS/VALIDATION.

## NATIVE MAKE ONE-WORD DEPENDENCY/RECIPE LINKS (2026-10-10)

MAKE_LINK STORES THE LOW 18-BIT STRING ARENA OFFSET AND HIGH 18-BIT
NEXT-LINK INDEX IN ONE 36-BIT WORD. LINK-INDEX END MARKER REMAINS
0777777 AND THE ORIGINAL OFFSET/RANGE LIMITS ARE UNCHANGED. BOTH
DEPENDENCY AND RECIPE TABLES SHRINK FROM TWO WORDS PER ALLOCATED LINK
TO ONE. THE LARGE-GRAPH 2048-LINK CAPACITY THEREFORE SAVES 2048
WORDS OF DEPENDENCY-TABLE STORAGE, PLUS ANY RECIPE-TABLE CAPACITY.
NO EXTRA INDEX IS USED; ACCESSORS MASK/SHIFT ON TRAVERSAL. KCC/DAS
AND ALL FIVE NATIVE SIMH MAKE TESTS PASSED. THE ISOLATED NATIVE A/B
RUNTIME COST AND PEAK PROCESS RAM ARE STILL TO BE MEASURED.

## NATIVE MAKE SIX-WORD RULE ISOLATED A/B (2026-10-10)

MATCHED NATIVE PDP-6 SIMH A/B TIMED DSH COMMAND TO PROMPT, EXCLUDING
BUILD, BOOT AND LOGIN: BASELINE 91B15D5 VS PACKED 9BB6DC3.
FRAGMENT 1.033/1.030 S, INCLUDE 1.139/1.030 S, SUFFIX 1.136/1.133 S,
LARGE GRAPH 119.344/114.807 S (-4.537 S, -3.8%), AND SORTED INDEX
1.652/1.746 S. ALL FIVE PROBES PASSED. SINGLE PAIR DOES NOT ESTABLISH
A RELIABLE SPEEDUP BUT SHOWS NO LARGE OVERHEAD. THE PRIMARY BENEFIT
IS DEFINITE 7-TO-6-WORD RULE RECORD SHRINKAGE: 1024 WORDS LESS AT
1024-RULE CAPACITY (DOES NOT INCLUDE POSSIBLE EXECUTABLE CODE GROWTH).
PEAK PROCESS RAM REMAINS UNMEASURED. TESTKIT REPRODUCER:
DAIMOS-NATIVE-MAKE-RULE-PACK-AB-20261010-V1.SH.

## NATIVE MAKE SIX-WORD RULE STRUCTURE (2026-10-10)

RULE FLAGS AND FOUR EXECUTION STATES NOW SHARE ONE 36-BIT WORD.
FLAGS USE BITS 0-2 AND STATE USES BITS 3-4. STRUCT MAKE_RULE
SHRINKS FROM SEVEN TO SIX WORDS, WITH NO EXTRA INDEX OR CACHE.
ON THE 1024-CAPACITY LARGE GRAPH THIS SAVES 1024 ALLOCATED RULE-TABLE
WORDS (RAM CAPACITY ESTIMATE, NOT PEAK-RSS MEASUREMENT).
KCC/DAS BUILD AND ALL FIVE NATIVE PDP-6 SIMH MAKE REGRESSIONS PASS.
ISOLATED A/B TIMING IS REQUIRED BEFORE RETAINING THIS AS A RUNTIME
OPTIMIZATION; RAM REDUCTION IS THE PRIMARY PURPOSE.

## NATIVE MAKE POINTER COMPARISON A/B — REVERTED (2026-10-10)

THE POINTER-ITERATION MAKE_RULE_COMPARE FROM C18F3C9 ELIMINATED
%ADJBPH HELPER CALLS FROM ITS GENERATED PDP-6 COMPARISON LOOP,
BUT DID NOT IMPROVE THE MATCHED ISOLATED SIMH COMMAND BENCHMARK.
LARGE GRAPH: 116.873 S AT BASELINE 676B07A VS 118.866 S AT C18F3C9
(+1.993 S, +1.7%). FRAGMENT 1.031/1.033, INCLUDE 1.131/1.137,
SUFFIX 1.032/1.035, INDEX ORDER 1.750/1.644 S. ALL FIVE NATIVE
REGRESSIONS PASSED FOR BOTH IMAGES. THIS IS ONE A/B PAIR, NOT A
STATISTICALLY ESTABLISHED SLOWDOWN, BUT NO SPEED BENEFIT WAS SHOWN.
THE INDEXED IMPLEMENTATION IS RESTORED. DO NOT INFER THAT FEWER
%ADJBPH CALLS ALONE IMPLY FASTER PDP-6 EXECUTION.

## NATIVE MAKE POINTER-BASED RULE COMPARISON (2026-10-10)

MAKE_RULE_COMPARE USES ADVANCING BYTE POINTERS RATHER THAN INDEXED
CHARACTER ACCESS. KCC -O NOW EMITS IBP/LDB IN THE COMPARISON LOOP,
REMOVING EXPENSIVE %ADJBPH CALLS FROM THAT LOOP. SIGNEDNESS AND
LEXICAL COMPARISON ORDER ARE PRESERVED BY EXPLICIT UNSIGNED CHAR
VALUES. NO ADDITIONAL GRAPH RAM IS USED. KCC/DAS AND ALL FIVE
NATIVE PDP-6 SIMH MAKE PROBES PASS. AN ISOLATED A/B PERFORMANCE
CHECK AGAINST 676B07A IS REQUIRED TO ESTABLISH SPEEDUP.

## NATIVE MAKE STRUCTURE AND CODE GENERATION AUDIT (2026-10-09)

SOURCE AND GENERATED PDP-6 ASSEMBLY AUDIT (NO PRODUCTION CODE CHANGE):
MAKE_RULE CONTAINS SEVEN 36-BIT WORD FIELDS; MAKE_LINK TWO; MAKE_VAR
THREE. THE LARGE-GRAPH FIXTURE HAS APPROXIMATELY 681 DISTINCT LHS
LABELS AND 1080 DEPENDENCY LINKS. AT DOUBLING ALLOCATION CAPACITIES
OF 1024 RULES AND 2048 DEPENDENCIES, THE RULE TABLE ALONE COSTS
7168 WORDS, LINKS 4096 WORDS, AND SORTED RULE INDEX 1024 WORDS.
COMBINING FLAGS AND STATE INTO ONE RULE WORD WOULD THEORETICALLY
SAVE 1024 ALLOCATED WORDS ON THIS FIXTURE (681 LIVE RECORD WORDS).
THIS IS CAPACITY-BASED ESTIMATION, NOT MEASURED PEAK PROCESS RAM.
MAKE STRINGS ARE 9-BIT C CHARS PACKED IN 36-BIT WORDS, NOT ONE
36-BIT WORD PER CHARACTER; EARLIER ARENA CHARACTER COUNTS MUST NOT
BE INTERPRETED AS WORD COUNTS.

KCC -O PDP-6 OUTPUT FOR MAKE_RULE_COMPARE PERFORMS EXPENSIVE 9-BIT
INDEXED BYTE-POINTER ADJUSTMENT CALLS TO %ADJBPH AND LDB OPERATIONS
FOR CHARACTER ACCESS. MAKE_STREQ AND MAKE_STRLEN SHOW SIMILAR CODE;
MAKE_DEP_HASH USES DIRECT LDB/IBP AND DOES NOT SHOW THE SAME INDEXED
ADJUSTMENT PATTERN. A HAND-VERIFIED POINTER-WALKING COMPARE (OR
COMPILER OPTIMIZATION) IS A STRONG SPEED CANDIDATE, BUT MUST BE
BENCHMARKED WITHOUT AFFECTING CORRECT UNSIGNED CHARACTER ORDER.
THE NEXT RAM CANDIDATE IS PACKING RULE FLAGS AND STATE INTO ONE WORD;
MEASURE GENERATED CODE GROWTH AND EXECUTION TIME BEFORE RETAINING IT.
TESTKIT HOST AUDIT: DAIMOS-NATIVE-MAKE-STORAGE-AUDIT-20261009-V1.PY.

## NATIVE MAKE PACKED RULE CACHE REJECTED AFTER A/B (2026-10-09)

A MATCHED NATIVE SIMH A/B MEASURED DSH COMMAND EXECUTION (EXCLUDING
BUILD, BOOT AND LOGIN) AT 93FD5EB BASELINE VS C720F2D PACKED-LINK
RULE CACHE. ALL FIVE PROBES PASSED IN BOTH BUILDS. LARGE GRAPH:
112.115 S BASELINE VS 114.322 S CACHED (+2.207 S, +1.97%). OTHER
PROBES: FRAGMENT 1.028/1.031; INCLUDE 1.131/1.131; SUFFIX 1.028/1.030;
INDEX ORDER 1.645/1.641 S. ONE RUN DOES NOT ESTABLISH A PRECISE
REGRESSION MAGNITUDE, BUT THERE IS NO EVIDENCE OF IMPROVEMENT.
A SINGLE MAKE TRAVERSAL USUALLY VISITS EACH DEPENDENCY LINK ONLY ONCE;
LAZY CACHING THEREFORE ADDS MASK/SHIFT/BRANCH COSTS WITHOUT REUSING
THE REFERENCE. GENERATED PDP-6 ASSEMBLY CONFIRMS EXTRA PACK/UNPACK
INSTRUCTIONS AROUND THE LOOKUP. THE SOURCE IS RETURNED TO THE
PRE-CACHE ALGORITHM; DO NOT ADD A PER-LINK CACHE WITHOUT EVIDENCE OF
MULTIPLE LINK TRAVERSALS. THE ORIGINAL TWO-WORD DEPENDENCY LINK AND
SEVEN-WORD RULE STRUCTURES REMAIN UNCHANGED. PEAK PROCESS RAM AND
CODE-WORD DIFFERENCE WERE NOT MEASURED. SEE DAIMOS-TESTKIT VALIDATION
DAIMOS-NATIVE-MAKE-DEPREF-AB-20261009-V1.SH FOR THE REPRODUCER.

## NATIVE MAKE PACKED DEPENDENCY RULE REFERENCES (2026-10-09)

DEP LINK NAME REFERENCES NOW LAZILY CACHE RESOLVED RULE INDEX+1 IN THE
UPPER 18 BITS OF THE EXISTING 36-BIT TEXT WORD. THE LOWER 18 BITS
RETAIN THE ORIGINAL ARENA TEXT OFFSET; ZERO IN THE UPPER HALF MEANS
UNRESOLVED. MAKE_BUILD ACCEPTS A POINTER TO THAT WORD FROM EXPLICIT
DEPENDENCIES AND USES THE RULE INDEX ON LATER TRAVERSALS. IMPLICIT
SOURCE RULES AND COMMAND-LINE GOALS STILL USE NORMAL LOOKUP. $?-LIST
CONSTRUCTION AND DEPENDENCY DIAGNOSTICS MASK TO THE LOW 18-BIT NAME
OFFSET. NO ADDITIONAL PER-RULE OR PER-DEPENDENCY WORDS ARE NEEDED;
THE EXISTING LINK REMAINS TWO 36-BIT WORDS, RULE REMAINS SEVEN WORDS.
THE LAZY CACHE DOES NOT RESOLVE ALL DEPENDENCIES BEFORE BUILDING AND
THEREFORE DOES NOT ADD A WHOLE-GRAPH PREPASS. NATIVE KCC AND DAS
PASSED; ALL FIVE SIMH MAKE-INCLUDE TEST PROBES PASSED. AN ISOLATED
RUNTIME A/B AND MEASURED PEAK PROCESS RAM ARE STILL OPEN; MEMORY
NEUTRALITY HERE REFERS TO THE LINK FIELD LAYOUT, NOT CODE SIZE.

## NATIVE MAKE ISOLATED COMMAND A/B (2026-10-09)

A TESTKIT HARNESS TIMED DSH COMMAND INVOCATION THROUGH RETURN TO DSH
PROMPT, EXCLUDING IMAGE CONSTRUCTION, BOOT AND LOGIN. ON OTHERWISE
MATCHING PDP-6 SIMH IMAGES, NATIVE MAKE BEFORE INTERN PROBE-FIRST
(42A227C) VERSUS AFTER (873A024) RETURNED THESE WALL TIMES (SECONDS):
FRAGMENT 1.029/1.028; INCLUDE 1.132/1.028; SUFFIX SUBSTITUTION
1.031/1.030; LARGE GRAPH 115.724/115.068; SORTED INDEX 1.644/1.644.
ALL FIVE PROBES PASSED IN BOTH RUNS. THE LARGE-GRAPH DIFFERENCE IS
0.656 S (0.57%); ONE RUN IS INSUFFICIENT TO ATTRIBUTE THIS SMALL
DIFFERENCE TO THE OPTIMIZATION. EACH COMMAND INCLUDES DSH DISPATCH,
NATIVE MAKE PARSING AND TRAVERSAL, AND PROMPT-POLLING OVERHEAD;
IT EXCLUDES SIMH BOOT AND BUILD. NATIVE PHASE CYCLE COUNTS, ACTUAL
HASH COLLISION STATISTICS AND PEAK PDP-6 PROCESS RAM WERE NOT MEASURED.
THE OPTIMIZATION IS RETAINED FOR ITS CORRECTNESS AND AVOIDANCE OF
PREMATURE REHASH, BUT NO STRONG SPEEDUP CLAIM IS MADE.

## NATIVE MAKE DEPENDENCY INTERN PROBE BEFORE GROWTH (2026-10-09)

MAKE_DEP_INTERN NOW CHECKS THE EXISTING HASH TABLE FOR DUPLICATES
BEFORE ASSESSING WHETHER INSERTION REQUIRES GROWTH. REPEATED
DEPENDENCY NAMES NO LONGER TRIGGER A PREMATURE REHASH AT THE LOAD
THRESHOLD. ABSENT NAMES STILL GROW THE TABLE BEFORE INSERTION AS
NEEDED; OPEN ADDRESSING AND STRING ARENA OWNERSHIP ARE UNCHANGED.
THE CHANGE ADDS NO PERSISTENT RAM. THE NATIVE PDP-6 KCC/DAS OBJECT
BUILD AND ALL FIVE SIMH MAKE-INCLUDE PROBES PASS. THE FULL HARNESS
TOOK ABOUT 145 S INCLUDING BOOT AND IMAGE CONSTRUCTION; NO ISOLATED
SPEEDUP IS CLAIMED. THE NEXT BENCHMARK SHOULD TIME ONLY NATIVE MAKE
COMMANDS, EXCLUDING IMAGE CONSTRUCTION AND BOOT.

## NATIVE MAKE RULE INSERTION ONE-SEARCH FAST PATH (2026-10-09)

MAKE_GET_RULE PREVIOUSLY DID A BINARY SEARCH THROUGH MAKE_FIND_RULE
AND REPEATED MAKE_RULE_LOWER_BOUND FOR NEW RULES. IT NOW REUSES THE
INITIAL LOWER BOUND FOR BOTH DUPLICATE DETECTION AND INSERTION. THIS
REMOVES ONE O(LOG N) SEARCH PER NEW RULE WITHOUT ANY NEW MEMORY OR
CHANGES TO RULE ORDER. THE 2026-10-09 PARSER PROFILE HAD 2323 RULE
REQUESTS; THIS CHANGE AFFECTS NEW RULES ONLY. KCC/DAS BUILD AND ALL
FIVE NATIVE PDP-6 SIMH MAKE-INCLUDE REGRESSION PROBES PASS. NO
ISOLATED RUNTIME SPEEDUP OR PHASE CYCLE COUNT HAS BEEN MEASURED.

## NATIVE MAKE PARSER OPERATION PROFILE (2026-10-09)

AN INSTRUMENTED COPY OF COMMIT ACEB654, BUILT AND EXECUTED WITH THE
DAIMOS-TESTKIT NATIVE PDP-6 SIMH HARNESS, PASSED ALL FIVE PROBES.
FOR THE 1080-DEPENDENCY LARGE-GRAPH FIXTURE, PARSING PERFORMED:
2042 LITERAL RULE-PARSE CALLS, 0 VARIABLE RULE-PARSE CALLS,
0 MAKE_EXPAND CALLS, 2323 MAKE_GET_RULE CALLS, 1080 MAKE_ADD_DEP CALLS,
AND 32 CAPACITY-GROWTH EVENTS ACROSS THE GRAPH ARRAYS AND ARENA.
STRING ARENA USED WAS 35647 CHARACTERS. THESE ARE COUNTS, NOT
ELAPSED CYCLE ATTRIBUTION. THEY INDICATE THAT VARIABLE EXPANSION IS
NOT THE BOTTLENECK FOR THIS ALL-LITERAL GRAPH, AND ARRAY GROWTH IS
UNLIKELY TO DOMINATE. RULE RESOLUTION AND DEPENDENCY-RECORD INSERTION
ARE THE NEXT ALGORITHMS TO MEASURE, WITH ISOLATED TIMING BEFORE ANY
FURTHER CHANGES. NO PRODUCTION MAKE SOURCE CHANGE WAS JUSTIFIED BY
THIS PROFILE. THE TEMPORARY PROFILER LIVES OUTSIDE THE PRODUCTION
TREE. EXISTING WORKTREE EDITS WERE PRESERVED.

## NATIVE MAKE LITERAL DEPENDENCY PARSING (2026-10-09)

MAKE_PARSE_RULE NOW USES ITS INPUT LINE DIRECTLY WHEN THE RULE CONTAINS
NO DOLLAR SIGN. RULES CONTAINING VARIABLE REFERENCES RETAIN THE
ORIGINAL MAKE_EXPAND PATH. THIS AVOIDS REDUNDANT EXPANSION SCANNING
AND STRING COPYING ON LITERAL DEPENDENCY RULES. THE PDP-6 KCC/DAS
OBJECT COMPILES AND ASSEMBLES; ALL FIVE NATIVE SIMH MAKE-INCLUDE
TESTS PASS. THE FULL RUN TOOK APPROXIMATELY 155 S INCLUDING IMAGE
BUILD AND BOOT; AN ISOLATED A/B SPEEDUP HAS NOT BEEN MEASURED.
PREEXISTING WORKTREE EDITS TO TODO.MD AND MACH_USER_PDP6.S ARE NOT PART
OF THIS CHANGE.

## TEMPORARY INVALID-UUO DIAGNOSTIC (2026-10-09)

THE KCORE MACH_SYSCALL_INVALID EXIT PATH NOW SAVES LOW-CORE 040 (TRAP
INSTRUCTION) AND MACH_SYSCALL_SAVE (JSR RETURN WORD) TO TWO EXPORTED
DIAGNOSTIC WORDS AND PRINTS BOTH AS TWELVE-DIGIT OCTAL VALUES ON THE
PHYSICAL PDP-6 CTY, PREFIXED "UUO ", BEFORE RETURNING STATUS 126.
THE CONSOLE ROUTINE IS SELF-CONTAINED IN KCORE TO AVOID A DEPENDENCY
ON THE CTY MRES. THIS IS TEMPORARY DIAGNOSTIC CODE, TO REMOVE AFTER
IDENTIFYING THE FAULT. DAS AND KCORE LINK/CONVERT PASS. A FRESH BOOT
IMAGE DID NOT COMPLETE: MKD6FS POST-FORMAT D6FSCK REPORTED ALLOCATED
LEAKED BLOCKS 14235-14241; NO SIMH RUNTIME DIAGNOSTIC PASS IS CLAIMED.

## NATIVE MAKE RULE INSERTION / INTERNING A-B PROFILE (2026-10-09)

TWO SEPARATE INSTRUMENTED PDP-6 SIMH BOOT IMAGES FROM 1178F14 (BEFORE)
AND 78FBC43 (AFTER) COMPLETED THE SAME FIVE NATIVE MAKE TEST PROBES.
FOR THE 1080-LINK LARGE GRAPH, BOTH HAD 682 SORTED RULE-INDEX SHIFTS;
DEPENDENCY NAME INTERNING CUT STRING ARENA USE FROM 39647 TO 35647
CHARACTERS (4000, OR 10.1%). THE OPEN-ADDRESSED INTERN INDEX HAS ITS
OWN MEMORY COST AND NO PEAK-RAM MEASUREMENT WAS MADE. ELAPSED TIME FOR
THE COMPLETE BUILD/BOOT/HARNESS SEQUENCES WAS ABOUT 167 S BEFORE AND
162 S AFTER; THIS IS NOT AN ISOLATED MAKE COMMAND TIMING, SO NO
RELIABLE EXECUTION SPEEDUP CAN BE CLAIMED. ONLY 682 RULE SHIFTS WERE
COUNTED; REPLACING THE EXISTING SORTED RULE INDEX IS NOT JUSTIFIED
BY THIS WORKLOAD. RETAIN CURRENT ALGORITHM. NEXT PROFILE SHOULD TARGET
RULE PARSING AND STRING EXPANSION WITH ISOLATED PHASE CLOCKS.

## NATIVE MAKE PHASE PROFILE AND DEPENDENCY INTERNING (2026-10-09)

A FRESH NATIVE SIMH BUILD WITH TEMPORARY PARSE/BUILD MARKERS PASSED ALL
FIVE MAKE-INCLUDE PROBES. AN EXTERNAL WATCHER OBSERVED PARSE DONE AT
T+50.55 S AND BUILD DONE AT T+88.08 S, A 37.53 S TRAVERSAL INTERVAL.
THE WATCHER STARTED AFTER MAKE HAD BEGUN, SO 50.55 S IS ONLY A LOWER
BOUND ON PARSE DURATION, NOT AN ACCURATE FULL PHASE TIME.

THE LARGE-GRAPH FIXTURE HAS 1080 EXPLICIT DEPENDENCY LINKS BUT ONLY
280 DISTINCT NAMES: 5400 CHARACTERS INCLUDING TERMINATORS WOULD BE
STORED AS INDIVIDUAL COPIES, VERSUS 1400 UNIQUE NAME CHARACTERS.
MAKE NOW INTERNS DEPENDENCY NAMES IN A GROWING OPEN-ADDRESSED TABLE
OF ARENA OFFSETS (ONE WORD PER SLOT). IT FREES OLD INDEX TABLES ON
GROWTH AND PRESERVES DEPENDENCY LINK ORDER AND ALL GRAPH SEMANTICS.
THE TABLE ITSELF CONSUMES WORDS, SO THE NET RAM BENEFIT DEPENDS ON
NAME REPETITION. NATIVE KCC/DAS BUILD AND ALL FIVE SIMH MAKE-INCLUDE
PROBES PASS. PRECISE NEW-VERSUS-OLD PHASE TIMING AND PEAK-RAM
BENCHMARKS ARE STILL OPEN.

## NATIVE MAKE LARGE-GRAPH COUNTER PROFILE / PHONY METADATA (2026-10-09)

A TEMPORARY INSTRUMENTED MAKE, BUILT IN AN ISOLATED WORKTREE AND RUN
UNDER THE NATIVE SIMH TESTKIT, COMPLETED ALL FIVE PROBES. THE 280-
TARGET / 800+-LINK LARGE-GRAPH PROBE RECORDED 1081 BUILD INVOCATIONS,
1082 MAKE_STAT CALLS, 3404 RULE LOOKUPS AND 2555 STRING STORES.
THESE ARE CALL COUNTS, NOT CPU TIME OR ATTRIBUTED COSTS. MOST OF THE
GRAPH IS EXPLICITLY PHONY, SO A FOLLOW-UP FAST PATH SKIPS INITIAL
AND POST-RECIPE STAT FOR PHONY TARGETS WHILE PRESERVING ALWAYS-CHANGED
SEMANTICS. THE OPTIMIZED PDP-6 OBJECT COMPILES AND ASSEMBLES; ALL FIVE
NATIVE SIMH MAKE-INCLUDE PROBES PASS. TOTAL HARNESS ELAPSED TIME WAS
ABOUT 162 S (INCLUDING IMAGE BUILD AND BOOT), SO NO END-TO-END SPEEDUP
IS ESTABLISHED FOR THE FAST PATH. PARSING/ALLOCATION PHASE TIMING AND
ACTUAL FILE-DEPENDENCY INVALIDATION REMAIN TO BE MEASURED.

## NATIVE MAKE SIMH A/B BENCHMARK (2026-10-09)

A CONTROLLED NATIVE SIMH RUN COMPARED THE OLD MAKE.C FROM COMMIT
31AC5B3 WITH THE OPTIMIZED MAKE.C FROM 12749AD. BOTH USED THE SAME
CURRENT DAIMOS SOURCE TREE, HOST TOOLCHAIN AND INSTALLED KCC BOOTSTRAP;
ONLY MAKE.C WAS REVERTED IN A DETACHED BASELINE WORKTREE. EACH IMAGE
BOOTED A PDP-6 AND EXECUTED THE SAME MAKE TESTKIT PROBES.

THE FRAGMENT, INCLUDE, AND SUFFIX-SUBSTITUTION PROBES PASSED IN 1 S
EACH ON BOTH BUILDS (WHOLE-SECOND TIMING RESOLUTION). THE 280-TARGET,
800+-DEPENDENCY LARGE-GRAPH PROBE TIMED OUT AT THE 300 S LIMIT IN
THE BASELINE (307 S INCLUDING HARNESS TIMEOUT HANDLING); OPTIMIZED
MAKE PASSED IN 129 S. THE OPTIMIZED BUILD ALSO PASSED THE SORTED-INDEX
PROBE IN 2 S. LARGE-GRAPH TIME IS AT LEAST 2.33 TIMES FASTER BY THE
TIME LIMIT (NOT AN EXACT SPEEDUP: BASELINE DID NOT COMPLETE). MEASURED
TIMES INCLUDE SIMH AND HOST HARNESS OVERHEAD. NO-OP/INCREMENTAL REBUILD
AND SEPARATE MEMORY/STAT-COUNT BENCHMARKS HAVE NOT YET BEEN MEASURED.

## NATIVE MAKE SUFFIX-RULE INDEX (2026-10-09)

IMPLICIT-RULE SEARCH NOW ITERATES ONLY OVER THE SUFFIX RECIPES, USING
ONE INDEX WORD PER SUFFIX RULE IN SOURCE ORDER. ORDINARY TARGET RULES
ARE NO LONGER SCANNED FOR EACH IMPLICIT RESOLUTION. PDP-6 KCC/DAS
COMPILATION AND STRUCTURAL TEST PASS. SIMH RUNTIME AND TIMING TESTS
ARE STILL REQUIRED. COMPLETED-RULE TIMESTAMP CACHING IS DEFERRED:
ANOTHER RECIPE CAN CHANGE A PREVIOUSLY EXAMINED FILE, SO A CORRECT
CACHE MUST ADDRESS INVALIDATION WITHOUT HIDING RECIPE SIDE EFFECTS.

## NATIVE MAKE PACKED DEPENDENCY METADATA (2026-10-09)

THE FIRST DFS PASS NOW STORES EACH EXPLICIT DEPENDENCY'S NEWER STATUS
IN A PACKED 36-BIT BITMAP, ALLOCATED ONCE AFTER PARSING (ONE BIT PER
DEPENDENCY). IMPLICIT SOURCE STATUS USES A LOCAL FLAG. `$?` CONSTRUCTION
NO LONGER RE-STATS PREREQUISITES. PDP-6 KCC/DAS BUILD AND STRUCTURAL
TEST PASS; SIMH INCREMENTAL INVALIDATION AND PERFORMANCE MEASUREMENTS
REMAIN REQUIRED. THE USER'S EXISTING TODO.MD EDITS WERE NOT CHANGED.

## DAIMOS LIBC REALLOC FRAGMENTATION FIX (2026-10-09)

LIBC REALLOC NOW GROWS ALLOCATIONS IN PLACE USING AN ADJACENT ADDRESS-ORDERED
FREE BLOCK, OR EXTENDS THE PROCESS BREAK WHEN THE ALLOCATION (WITH OR
WITHOUT A FREE NEIGHBOR) REACHES THE HEAP END. IT SPLITS EXCESS SPACE
WHEN SHRINKING OR PARTIALLY CONSUMING A FREE BLOCK AND RETURNS SHRUNK
SPACE TO THE COALESCING FREE LIST. THE COPY-AND-FREE FALLBACK IS USED
ONLY WHEN NEITHER IN-PLACE STRATEGY SUCCEEDS; FAILED GROWTH LEAVES THE
ORIGINAL BLOCK VALID. NO ADDITIONAL ALLOCATOR METADATA IS REQUIRED.
LIBC BUILDS WITH /USR/LOCAL/BIN/KCC, AND THE DAIMOS-TESTKIT HOST MOCK
TEST COVERS ADJACENT GROWTH, TOP-OF-HEAP GROWTH, SHRINKING, COALESCING,
AND MOVING GROWTH. KIR'S 16384-TYPE FAILURE IS NOT YET RETESTED ON THE
NATIVE SYSTEM. THE NEW LIBC MUST BE INCORPORATED INTO THE KCC NATIVE
BOOTSTRAP AND BOOT IMAGE BEFORE REPEATING KCC COMPILATION.

## NATIVE MAKE NEWER-PREREQUISITE FAST PATH (2026-10-09)

MAKE NOW SKIPS THE SECOND DEPENDENCY-METADATA WALK FOR TARGETS WITH
NO RECIPE TO EXECUTE, UP-TO-DATE TARGETS, AND RECIPES NOT USING `$?`.
A CONSERVATIVE VARIABLE-VALUE FLAG PRESERVES INDIRECT `$?` EXPANSION.
THE NATIVE MAKE OBJECT COMPILES AND ASSEMBLES. RUNTIME TIMING AND FULL
INCREMENTAL INVALIDATION REGRESSIONS REMAIN PENDING; SOURCE STAT CACHING
AND SUFFIX-INDEXING ARE NOT YET IMPLEMENTED.

## KCC SOURCE RESTAGING READ-ONLY FILE FIX (2026-10-09)

THE KCC SOURCE STAGER PREVIOUSLY USED CP ON INSTALLED BOOTSTRAP INPUTS
WITH READ-ONLY MODE 0444. SECOND AND LATER IMAGE BUILDS FAILED WITH
PERMISSION DENIED WHEN REPLACING EXISTING BOOT/CRT0.DOBJ. IT NOW USES
INSTALL -M 0644 TO CREATE WRITABLE STAGED COPIES. RESTAGING TWICE IN THE
EXISTING BUILD DIRECTORY PASSES, AS DOES THE DAIMOS-TESTKIT HOST REGRESSION
THAT MAKES ALL FIVE STAGED BOOTSTRAP INPUTS READ-ONLY BEFORE RESTAGING.

## NATIVE MAKE SIXBIT INCLUDE SPELLING (2026-10-09)

NATIVE MAKE NOW RECOGNIZES OPTIONAL `-INCLUDE` IN ITS SIXBIT
UPPERCASE MAKEFILES. THE HOST-STAGED SOURCES MAY USE LOWERCASE
`include` AND `-include`, BUT THE NATIVE SIXBIT FILE MUST BE
UPPERCASED BEFORE S6TEXT CONVERSION. THE PDP-6 OBJECT BUILT WITH
THE INSTALLED KCC COMPILER. THE FOCUSED SIMH REGRESSION IS PENDING.

## INSTALLED KCC BOOTSTRAP LOCATION (2026-10-09)

THE HOST KCC AND ALL FIVE DAIMOS BOOTSTRAP DXRS WERE REBUILT FROM
THE KCC DEVSPACE CHECKOUT AND INSTALLED INTO $PDP10_PREFIX.
KCC INSTALL-NATIVE STORES THE BOOTSTRAP UNDER
$PDP10_PREFIX/LIB/KCC/BOOTSTRAP, INCLUDING THE NATIVE RUNTIME AND
LIBC OBJECTS USED BY BOOT SOURCE STAGING. SYSTEM/BOOT/PDP6/MAKEFILE
USES THAT INSTALLED PATH AS ITS DEFAULT, NOT KCC/BUILD-NATIVE.
HOST-ONLY BOOT MAKE DRY-RUN CHECKS PASSED. LONG NATIVE KCC SELF-BUILDS
REMAIN USER-OPERATED AND WERE NOT RUN.

## NATIVE MAKE INCLUDE SYNTAX EXTENSION (2026-10-09)

NATIVE MAKE ACCEPTS THE EXISTING `INCLUDE` SYNTAX AS WELL AS GNU-STYLE
`include` AND OPTIONAL `-include`. AN OPTIONAL INCLUDE IS SKIPPED ONLY
WHEN FILE STATUS CONFIRMS IT IS ABSENT; AN EXISTING FILE WITH A PARSE
ERROR STILL FAILS. THE MODIFIED PDP-6 MAKE OBJECT COMPILES AND ASSEMBLES.
DAIMOS-TESTKIT HAS NEW INCLUDE SYNTAX CASES, BUT SIMH RUNTIME VERIFICATION
IS BLOCKED BY THE MISSING KCC HOST BOOTSTRAP AND NATIVE PHASE DXRS IN
THIS DEVSPACE CHECKOUT. NO LONG NATIVE KCC RUN WAS STARTED.

## NATIVE MAKE RULE LOOKUP OPTIMIZATION (2026-10-09)

RULE LOOKUPS NOW USE A SORTED SINGLE-WORD-PER-RULE INDEX WITH BINARY
SEARCH. RULE OBJECTS RETAIN STABLE IDS FOR DEPENDENCY LINKS, AND THE
SORTED INDEX IS UPDATED ONLY WHEN NEW RULES ARE PARSED. THE PDP-6
OBJECT COMPILES, BUT NATIVE RUNTIME REGRESSION AND LARGE-GRAPH TIMING
ARE STILL REQUIRED. THE USER OPERATES LONG NATIVE KCC BUILDS.

## NATIVE MAKE DYNAMIC GRAPH STORAGE (2026-10-09)

NATIVE MAKE NOW ALLOCATES RULES, DEPENDENCIES, RECIPES, VARIABLES, AND ITS
STRING ARENA ON DEMAND. THE VARIABLE TABLE IS NAME-SORTED FOR BINARY LOOKUPS
WITHOUT A SEPARATE HASH INDEX. THE `$?` PREREQUISITE LIST GROWS ONLY WHEN
NEEDED, AND PHONY RULES SKIP IMPOSSIBLE IMPLICIT-RULE SEARCHES. MAKE NOW
LINKS THE REAL DAIMOS LIBC ALLOCATOR, STRING ROUTINES, AND MEMCPY. THE
DAIMOS-TESTKIT INCLUDE/SUBSTITUTION/CAPACITY REGRESSION PASSED WITH 280
RULES AND 190 VARIABLES, BEYOND THE PREVIOUS FIXED TABLE LIMITS. THIS
TEST IS SLOW UNDER SIMH; PERFORMANCE AND FULL KCC MAKEFILE FEATURES REMAIN
OPEN. LONG NATIVE KCC COMPILATIONS REMAIN USER-OPERATED ONLY.

## PORTABLE DAIMOS LIBC BUILD (2026-10-09)

THE USERLAND/LIBC MAKEFILE NOW USES EXPLICIT DEPENDENCIES AND SHARED
RECIPES COMPATIBLE WITH GNU MAKE AND BSD MAKE. KCC INVOKES IT VIA $(MAKE)
WITHOUT A SEPARATE GNU-ONLY SUBMAKE. HOST AND NATIVE BUILD OUTPUT DIRECTORY
SELECTION IS UNCHANGED. FULL NATIVE TARGET SELF-HOSTING REMAINS OPEN.

## KCC HOST BUILD DIRECTORY TRANSITION (2026-10-09)

KCC'S HOST BUILD ARTIFACTS NOW LIVE IN KCC/BUILD/ AND ITS NATIVE DAIMOS
BOOTSTRAP ARTIFACTS IN KCC/BUILD-NATIVE/ (WITHOUT -V1 SUFFIXES).
SYSTEM/BOOT/PDP6/MAKEFILE'S DEFAULT KCC_BOOT_BUILD NOW POINTS TO
BUILD-NATIVE/. GNU MAKE AND BSD BMAKE PRODUCE IDENTICAL FIVE NATIVE KCC
DXRS IN THE HOST-SIDE BOOTSTRAP. DAIMOS LIBC REMAINS GNU-MAKE-ONLY.
THIS BUILD-SYSTEM CHANGE DOES NOT VALIDATE FULL ON-TARGET SELF-HOSTING.

## NATIVE KCC D6FS RESIZE CORRECTIONS / KPARSE GATE (2026-10-09)

2026-10-09 KIR/PROCESS-EXIT INVESTIGATION: MANUALLY COMPILING CC.C
REPORTED `? KCC - COULD NOT WRITE KIR1 EXTERNAL DEFINITION`, FOLLOWED
BY A SIMH HALT RATHER THAN A RETURN TO DSH. THE KIR WRITER NOW TRACKS
RECURSIVE GRAPH-BUILD FAILURES AND REPORTS ITS FAILING OUTPUT STAGE,
RECORD INDEX, COUNTS, AND STREAM ERROR STATE WITHOUT HIGH-VOLUME TRACES
(KCC NATIVE BRANCH). IN PROC_EXIT_CURRENT, LAST-PROCESS HALT (OPERAND
ZERO) AND FAILED-RELEASE HALT (OPERAND ONE) ARE NOW DISTINCT TO SUPPORT
FUTURE SIMH DIAGNOSIS. THIS IS NOT YET A FIX FOR THE UNDERLYING HALT.
FRESH KPARSE AND BOOT IMAGE BUILDS SUCCEEDED. DAIMOS-TESTKIT PROCESS
HIERARCHY AND PROCESS EVENTS REGRESSIONS PASSED. REPRODUCTION HARNESS
ATTEMPTS DID NOT REACH KCC: ONE STALLED AT LOGIN, THE DCS RETRY FAILED
ROOT VALIDATION WITH `?RT`. DO NOT MARK NATIVE `MAKE ALL` AS PASSING.

2026-10-09 NATIVE COMPILER FOLLOW-UP: KCC CCEVAL.C CHKOVF USED
`JFCL 11`, WHICH TESTS PDP-6 ARITHMETIC OVERFLOW **AND PC-CHANGE**.
REPLACE WITH `JFCL 10`, TESTING ONLY ARITHMETIC OVERFLOW; THE NATIVE
SMALL-SOURCE TESTKIT REGRESSION `DAIMOS-NATIVE-KCC-OVERFLOW-FLAGS-
20261009-V1` PASSES THROUGH KCPP/KPARSE/KGEN/KOPT WITH NO FALSE
OVERFLOW DIAGNOSTICS. THE FULL CC.C KPARSE REPLAY ALSO ELIMINATED THE
PREVIOUS REPEATED FALSE CONSTANT-EXPRESSION OVERFLOW ERRORS.

CRITICAL HOST TOOLCHAIN PROVENANCE: `/USR/LOCAL/BIN/KCC` WAS AN
OLDER OCT-7 EXECUTABLE, WHILE BUILDING `~/GIT/KCC/KCC` FROM CURRENT
SOURCES PRODUCES CORRECT OPTIMIZED PDP-6 LOOPS. THE TESTKIT TARGET
`DAIMOS-KCC-POINTER-SCAN-PDP6-20261009-V1` FAILS WITH THE INSTALLED
COMPILER (RESULT 1), PASSES WITH THE NEW HOST COMPILER (RESULT 0).
THE STALE COMPILER DROPPED THE LOOP INDEX INCREMENT FROM KIR'S
`FINDPTR`, CORRUPTING SYMBOL IDS AND THE PREPROCESSOR/COMPILER PHASES.
ALL NATIVE KCC PHASE DXRS AND DRIVER WERE REBUILT USING THE CURRENT
`~/GIT/KCC/KCC`, NOT THE STALE INSTALLED EXECUTABLE. THE FRESH IMAGE
PASSES FSCK AND KPARSE BUILDS LARGE GRAPHS PAST THE OLD FAILURE AT
CC.C:1015. FULL NATIVE `MAKE ALL` IS NOT YET VALIDATED; THE KIR
GRAPH-WRITER IS NOW CPU-BOUND ON A LARGE SYMBOL GRAPH.

THE EXPERIMENTAL KIR POINTER RANGE NEGATIVE FILTER IS ONLY A TRIAL:
IT ADDS EIGHT BOUNDS WORDS TO A GRAPH/MODULE AND DOES NOT ALTER THE
IDENTITY/ID MATCH CRITERION. KEEP IT ONLY IF A CONTROLLED BENCHMARK
DEMONSTRATES SUFFICIENT SPEEDUP TO JUSTIFY INCREASED USER RAM.

THE REPRODUCIBLE PREPROCESSOR STALL HAD TWO INDEPENDENT D6FS ASSEMBLER
ROOT CAUSES, BOTH FIXED IN FS/D6FS_PROVIDER_MUTATION.S:

1. A PUSHJ TO D6FS_RESIZE_DECODE_EXTENT IMMEDIATELY PRECEDED THE CALLEE
   LABEL. ITS POPJ RE-ENTERED THE SAME HELPER AND CONSUMED A DATA WORD
   (000000000001) AS A RETURN ADDRESS. THE HELPER WAS MOVED AFTER THE
   RESIZE ROUTINE'S FINAL POPJ, AT NO MACHINE-WORD COST.
2. THREE READER.FIELD ACCESSES USED 6(1) (FCB_COUNT) RATHER THAN 3(1)
   (TOTAL_BLOCKS): INITIAL BLOCK LIMIT, ADJACENT EXTENT CHECK, AND
   ALLOCATION-CURSOR WRAP. THEY FORCED PREMATURE EXTENT EXHAUSTION IN
   THE KCPP TOKEN STREAM. EACH WAS CORRECTED WITHOUT CODE-SIZE GROWTH.

THE D6FS MRES DXR REMAINS IMAGE 004043, RELOCS 000072, TOTAL 2144
WORDS. ALL FOUR-DISK/FOUR-DRUM IMAGES BUILT AND PASSED D6FSCK. NATIVE
KCPP NOW PREPROCESSES CC.C THROUGH 220672 INPUT CHARACTERS AND RETURNS
STATUS ZERO. DAIMOS-TESTKIT PASSED INCREMENTAL D6FS APPENDS, A 3000-WORD
LARGE APPEND, THE NEW 12288-WORD/96-BLOCK EXTENT GROWTH/READBACK TEST,
AND A SOURCE CALL-FLOW REGRESSION.

THE NEXT NATIVE SELF-HOSTING GATE IS KPARSE. IT EMITS REPEATED
"CONSTANT EXPRESSION OVERFLOW" DIAGNOSTICS FOR ORDINARY ARRAY-SIZE AND
OTHER EXPRESSIONS (CCSYM.H:41, CC.H:125, CC.C:319, ETC.) AND DID NOT
COMPLETE WITHIN THE FIRST 260-SECOND TEST. ITS EVALBINOP/CHKOVF INLINE
JFCL CHECK IS BEING EXAMINED. DO NOT CLAIM MAKE ALL, INSTALL, OR THE
FULL SELF-HOSTING COMPILER PASSED.

SIMH HISTORY ALSO IDENTIFIED A SEPARATE KPARSE PERFORMANCE HOTSPOT:
USER PC 134041..134057 (APR BASE 350000) MAPS INTO THE NATIVE
CCKIRWRITE-V1.DOBJ ROUTINE FINDPTR(), WHICH DOES A LINEAR SCAN OVER
POINTER ARRAYS. ONE OBSERVED SEARCH HAD N=016777 (7679 DECIMAL), AND
ITS INDEX WAS ADVANCING. THIS IS QUADRATIC GRAPH-INDEX WORK, NOT AN
INFINITE LOOP AT THAT PC. REPLACING IT NEEDS AN EXPLICIT USER-RAM BUDGET;
A LARGE GENERIC HASH TABLE WOULD BE COUNTER TO THE LOW-MEMORY DESIGN.
ANOTHER PAUSED RUN SHOWED A SLEEPING PROCESS AT EVENT WORD 050636 WITH
NO RUNNABLE SLOT AND DSK STORAGE_STATE ZERO; THE CONNECTION TO THE
OVERFLOW DIAGNOSTICS OR KIR PERFORMANCE IS NOT ESTABLISHED.

## HISTORICAL DIAGNOSTICS SUPERSEDED BY D6FS FIX (2026-10-08)

EARLIER INSTRUCTION TRACES WERE MISATTRIBUTED TO KCPP USER TEXT.
THE FAULTING PC WAS ACTUALLY IN THE RELOCATED D6FS MRES; THE
BOGUS RETURN TO ADDRESS 1 FOLLOWED A PUSHJ WHOSE CALLEE LABEL
IMMEDIATELY FOLLOWED IT. INVALID-UUO EXIT CLEANUP THEN WAITED ON
THE SAME PROCESS'S UNRELEASED FILESYSTEM PROVIDER LOCK. THAT
LOCK WAIT WAS SECONDARY TO THE ASSEMBLER CONTROL-FLOW DEFECT.
THE RESIDENT KERNEL, APR RELOCATION, AND KCPP USER IMAGE WERE
NOT THE CAUSE. SEE THE COMPLETED FIX DETAILS ABOVE.

## NATIVE MAKE AND INSTALL (2026-10-08)

THE NATIVE INSTALL PROGRAM IS AVAILABLE UNDER /SYSTEM/EXEC/INSTALL. IT
COPIES UNMODIFIED 36-BIT WORD STREAMS, SUPPORTS -M OCTAL MODE, AND CREATES
DIRECTORIES WITH -D. MAKE SUPPORTS -Q UPDATE QUERY AND AN EXPLICIT '!' DIRECT
RUN V2 RECIPE FORM FOR COMMANDS LONGER THAN THE 102-CHARACTER DSH -C ABI.
DIRECT COMMANDS REQUIRE AN ABSOLUTE EXECUTABLE AND SPACE-SEPARATED ARGUMENTS;
THE ORDINARY SHELL RECIPE FORM IS UNCHANGED. A DAIMOS-TESTKIT SIMULATOR
REGRESSION PASSES INSTALL DIRECTORY/COPY/COMPARE, MAKE -Q IN BOTH STATES,
AND A DIRECT RECIPE LONGER THAN 102 CHARACTERS.
## KCC SOURCE LAYOUT AND BUILD STATUS (2026-10-08)

THE CANONICAL KCC SOURCE INSTALLATION IS /OPTION/SOURCE/KCC, NOT
/OPTION/KCC/SOURCE. THE BOOT IMAGE INSTALLS CONVERTED SOURCE FILES AND THE
NATIVE MAKEFILE. CHANGES TO BOOT-IMAGE PACKAGING RULES INVALIDATE DISK
IMAGE TARGETS AS WELL AS CHANGES TO THE STAGED SOURCE FILES.

THE BOOT IMAGE NOW STAGES ALL FIVE HOST-BUILT NATIVE KCC BOOTSTRAP
EXECUTABLES, PLUS THE NATIVE KCC CRT0, BOOTSTRAP, SYSCALL, AND LIBC
OBJECTS/ARCHIVE UNDER /OPTION/SOURCE/KCC/BOOT. THE ABI HEADER SNAPSHOT
IS UNDER /OPTION/SOURCE/KCC/ABI. KCC'S AUTHORITATIVE MAKEFILE AND
DAIMOS.MK ARE STAGED UNDER
/OPTION/SOURCE/KCC, NOT A GENERATED DEPENDENCY GRAPH. KCC'S SOURCE-TREE
MAKEFILE OWNS SHARED MODULE INVENTORIES; UNIX.MK HANDLES HOST COMPILATION
AND THE CROSS-BOOTSTRAP, WHILE DAIMOS.MK HANDLES NATIVE BUILD RULES.
THE STAGER UPPERCASES THE SHARED MAKEFILE, SELECTS DAIMOS AS THE PLATFORM,
TRANSLATES PORTABLE RECIPE PREFIXES INTO NATIVE DIRECT-RUN PREFIXES,
AND ENCODES BOTH AS S6REC. THE OLD PYTHON MAKEFILE GENERATOR WAS DELETED.
`MAKE ALL` BUILDS INCREMENTALLY INTO THE LOCAL `B` SUBDIRECTORY; `MAKE INSTALL` COPIES THE
NEW DXRS INTO /OPTION/BASE/EXEC AND /OPTION/BASE/LIBEXEC/KCC.

THE PRE-SPLIT HOST IMAGE BUILD PASSED D6FSCK AND THE TARGET-SIDE
MAKE -Q GRAPH PREFLIGHT PASSED ON THE PREVIOUS GENERATED GRAPH. NEITHER RESULT PROVES SUCCESSFUL NATIVE MAKE ALL OR
LINK/INSTALL. THE USER WILL RUN THOSE TARGET ACCEPTANCE TESTS. THE
BOOTSTRAP ARTIFACTS ARE EXPLICIT PREREQUISITES; A MISSING PREBUILT KCC
PHASE OR LIBC OBJECT FAILS IMAGE PACKAGING INSTEAD OF SILENTLY OMITTING IT.

THE ON-TARGET DAIMOS.MK ALSO SUPPORTS MAKE CLEAN. CLEAN REMOVES ONLY
GENERATED B/*.S, B/*.DOBJ, B/*.DARC, AND B/*.DXR ARTIFACTS; IT DOES
NOT TOUCH ORIGINAL SOURCES, BOOTSTRAP PHASES, OR STAGED ABI/LIBC INPUTS.
THE EARLIER NATIVE SIMULATOR PREFLIGHT PASSED MAKE CLEAN, MAKE -Q, AND
MAKE -Q ALL USING THE OLD GRAPH. THE SHARED-FRAGMENT GRAPH STILL NEEDS
A NEW NATIVE RUNTIME ACCEPTANCE RUN.
THE ROOT IMAGE INCLUDES A WRITABLE /SCRATCH MOUNT POINT FOR THE NATIVE
COMPILER'S INTERMEDIATE FILES.
## NATIVE KCC MAKE PARSER/QUERY ACCEPTANCE (2026-10-09)

A FRESH SIMH BOOT IMAGE WITH THE SPLIT KCC MAKEFILES INITIALLY FAILED
NATIVE MAKE PARSING BECAUSE HOST-ORIENTED REPEATED `+=` INVENTORY
ASSIGNMENTS EXCEEDED NATIVE MAKE'S 256-CHARACTER VARIABLE BUFFER.
KCC NOW PLACES THOSE CROSS-BUILD INVENTORIES IN UNIX.MK, WITH ONLY THE
PLATFORM SELECTOR IN THE SHARED ROOT MAKEFILE AND EXPLICIT ON-TARGET
DEPENDENCIES IN DAIMOS.MK. THE CORRECTED SIMH RUN PASSED `MAKE -Q`
(DEFAULT AND ALL), `MAKE -N` FOR B/CC-CPP.S, AND `MAKE CLEAN`.
THE SINGLE NATIVE B/CC-CPP.S COMPILATION REACHED KCC AND PRINTED
KCC000006, BUT THE TEST TIMED OUT; DO NOT CLAIM OBJECT COMPILATION,
FULL MAKE ALL, INSTALL, OR INCREMENTAL REBUILD PASSES. A NEW TESTKIT
V2 PRE-FLIGHT SEPARATES GRAPH CHECKS FROM LONG COMPILER ACCEPTANCE.

## KCC SHARED MAKEFILE SPLIT (2026-10-09)

HOST AND CROSS-BUILD DRY RUNS PASS WITH KCC/MAKEFILE AND UNIX.MK. THE
ON-TARGET DAIMOS.MK HAS EXPLICIT OBJECT, PHASE-ARCHIVE, LINK AND LOCAL-HEADER
PREREQUISITES. HOST STAGING OF MAKEFILE, DAIMOS.MK, SOURCES, ABI AND THE
INSTALLED BOOTSTRAP LIBRARY NOW PASSES. THE STAGER'S OLD ASSUMPTION THAT
BOOTSTRAP RUNTIME OBJECTS HAVE UNSUFFIXED FILENAMES WAS FIXED TO READ
THE INSTALLED -V1.DOBJ FILES. THE DAIMOS-TESTKIT HOST PREFLIGHT CHECKS
THAT ALL KCC PHASE MODULES HAVE OBJECT RULES AND HEADER DEPENDENCIES.
A NEW NATIVE SIMH MAKE ALL / INSTALL ACCEPTANCE HAS NOT BEEN RUN; DO NOT
TREAT KCC SELF-HOSTING AS QUALIFIED YET.

## D6FS HOST CHECKER OWNER WIDTH (2026-10-08)

THE HOST D6FSCK BLOCK-OWNERSHIP MAP USES 32-BIT OWNERS (FCB INDEX PLUS TWO),
NOT EIGHT-BIT BYTES. THE OLD EIGHT-BIT MAP WRAPPED FCB 254'S OWNER TO ZERO,
MISCLASSIFYING VALID FILE BLOCKS AS ALLOCATED LEAKS IN LARGER BOOT IMAGES.
THIS AFFECTED THE KCC-SOURCE-EQUIPPED BOOT IMAGE AND WAS A CHECKER BUG,
NOT LOST FILE DATA. DAIMOS-TESTKIT NOW COVERS 260 FILES AND THE OWNER-WIDTH
BOUNDARY. THE EXISTING FAILED IMAGE PASSES THE CORRECTED CHECKER WITHOUT
REPAIR.
# DAIMOS PROJECT STATE

DATE: 2026-10-07

THIS FILE IS THE COMPACT AUTHORITATIVE SUMMARY OF CURRENT DAIMOS ARCHITECTURE,
IMPLEMENTATION STATE, HARD CONSTRAINTS, AND DEVELOPMENT ORDER.  DETAILED DESIGN
BELONGS IN PDP10-DOC, OPEN WORK IN TODO.MD, AND COMPLETED IMPLEMENTATION HISTORY
IN GIT.

## 1. MACHINE AND MEMORY BASELINE

THE INITIAL/MINIMUM MACHINE IS A PDP-6 WITH 32K WORDS OF CORE.  PDP-10
BACKENDS MAY ADD MACHINE-SPECIFIC FACILITIES WITHOUT RAISING THAT BASELINE.
THE GENERIC VM/MM BOUNDARY MUST REMAIN REPLACEABLE BY A MACHINE-SPECIFIC PAGER.

THE HARD PERMANENT KERNEL LIMIT IS ADDRESSING BELOW 040000.  KCC IS USED FOR
THE OFFICIAL RESIDENT-SIZE MEASUREMENT.  SHRINKAGE SWEEPS ARE REQUIRED ONLY
WHEN THE CONFIGURED HARD LIMIT IS ACTUALLY EXCEEDED.

KCORE IS THE FIXED RESIDENT KERNEL CORE: THE COMMON SYSCALL, PROCESS, VM, VFS,
INTERRUPT-BRIDGE, AND LOW-LEVEL SUPPORT CODE THAT REMAINS PRESENT AFTER BOOT.
KINIT IS THE TRANSIENT BOOT/INITIALIZATION IMAGE.  IT DISCOVERS HARDWARE AND
THE ROOT FILESYSTEM, RUNS MODULE INITIALIZATION, INSTALLS THE RESIDENT PACKAGES
NEEDED BY THE ACTIVE CONFIGURATION, STARTS /SYSTEM/INIT, AND IS THEN RECLAIMED.
MRES MEANS A MODULE'S RESIDENT PACKAGE: THE CODE AND SMALL STATE THAT A MODULE
MUST LEAVE BEHIND AFTER ITS KINIT/MINIT INITIALIZATION HAS FINISHED.  A MODULE
CAN HAVE A MINIT ROUTINE WITHOUT HAVING A SEPARATE MRES PACKAGE.

LAST CLEAN FULL KCC BASELINE BEFORE THE WORD-I/O REDESIGN:

    KCORE               9728 WORDS
    CTY MRES              58 WORDS    CONSOLE TERMINAL I/O
    CLK MRES              22 WORDS    APR LINE-CLOCK SERVICE
    PTR MRES              39 WORDS    PAPER-TAPE READER
    PTP MRES              25 WORDS    PAPER-TAPE PUNCH
    CR MRES               42 WORDS    CARD READER
    CP MRES               44 WORDS    CARD PUNCH
    DCS MRES              63 WORDS    DATA-COMMUNICATION TERMINALS
    GE MRES               97 WORDS    GE TERMINAL INTERFACE
    DPY MRES              14 WORDS    DISPLAY OUTPUT
    TTY MRES              22 WORDS    COMMON TERMINAL/LINE-DISCIPLINE BRIDGE
    WCNSLS MRES            8 WORDS    WCNSLS DISPLAY INTERFACE
    OCNSLS MRES            3 WORDS    OCNSLS DISPLAY INTERFACE
    DSK MRES             212 WORDS    TYPE-270 DISK SERVICE
    TAPE MRES            330 WORDS    SHARED DTC/MTC TAPE SERVICE
    SLV MRES               2 WORDS    SLAVE INTERFACE
    DRM MRES             112 WORDS    DRM236 DRUM SERVICE
    MEMFS MRES           636 WORDS    MEMORY FILESYSTEM
    DTFS/TSFS MRES      1848 WORDS    NATIVE DECTAPE FILESYSTEMS; ITS/TENEX DISABLED
    BLOCKSET MRES          0 WORDS    SINGLE-ROOT BUILD; 38-WORD PACKAGE WHEN
                                      MULTI-MEMBER BLOCK MAPPING IS INSTALLED
    LOGSTORE MRES         101 WORDS    RUNTIME LOG RING SERVICE
    D6FS MRES           2236 WORDS    NATIVE WRITABLE FILESYSTEM

ROOT-SELECTION AND MONITORFS-DEVICE INITIALIZATION HAVE MINIT ENTRIES WITHOUT
STANDALONE MRES PACKAGES.  LPT NOW HAS AN OPTIONAL MRES BECAUSE ITS S6REC
WORD-OUTPUT ADAPTER IS LARGE ENOUGH THAT ABSENT-PRINTER SYSTEMS SHOULD NOT PAY
FOR IT IN KCORE.  BADMAP IS CONDITIONAL: CLEAN MEDIA NEEDS NO BADMAP MRES;
SUPPORT CAN ALSO BE REMOVED AT COMPILE TIME WITH KINIT_BADMAP=0.

THE CURRENT SUPPORTED BUILD ENABLES NATIVE DTFS/TSFS ONLY.  TENEX/ITS FOREIGN
DTFS PERSONALITY SOURCE REMAINS IN THE TREE, BUT THE PRODUCTION PDP-6 BUILD
SETS DTFS_ENABLE_TENEX=0, DTFS_ENABLE_ITS=0, AND DTFS_ENABLE_FOREIGN=0.  THE
AUTHORITATIVE CURRENT RESIDENT-SIZE MEASUREMENT IS RECORDED LATER IN THIS FILE
AND IN PDP10-DOC STATUS/RESIDENT-SIZE.MD.  THE FINAL PRE-DSH KERNEL TASK
REMAINS ONE RESIDENT-SIZE/CORRECTNESS SWEEP TO RESTORE THE REQUIRED 040000
LIMIT, FOLLOWED BY THE COMPLETE REGRESSION/BOOT/LOGIN CHECK.



DAIMOS 0.9/1.0 REGULAR FILES ARE WORD STREAMS.  VFS AND FILESYSTEM PROVIDERS
TRACK ONLY SIZE_WORDS; THE FORMER GENERIC 9-BIT SIZE_CHARS/OFF_CHARS ABI,
REGULAR-FILE READCHAR/WRITECHAR PATHS, WRITE_CHARS UUO, AND D6FS CHARACTER-TAIL
METADATA HAVE BEEN REMOVED.  NATIVE TEXT IS APPLICATION-LEVEL S6REC/SIXBIT.
BUILD TOOLS CONVERT HOST CONFIGURATION TEXT WITH S6TEXT AND IMPORT THE RESULT
AS ORDINARY WORD FILES; D6FS/TSFS HAVE NO TEXT FILE TYPE.  SYMLINKS REMAIN A
DISTINCT VFS OBJECT TYPE AND STORE A SELF-CONTAINED COUNTED PACKED-SIXBIT
TARGET RECORD.

MONITORFS PROCESS/DOMAIN TEXT LEAVES FOLLOW THE SAME WORD-TEXT CONTRACT: THEY
ARE S6REC READ_WORDS STREAMS, WITH NO PROVIDER-3 READCHAR ABI.  LIVE
NAME/CMDLINE/ENVIRONMENT DATA IS DERIVED FROM THE PROCESS IMAGE WITHOUT
PER-PROCESS KERNEL COPIES; CMDLINE, ENVIRONMENT, AND DOMAIN PIDS USE ONE S6REC
RECORD PER LOGICAL ENTRY.  THE OLD CHARACTER-AT-OFFSET SIXBIT/DECIMAL HELPERS
ARE REMOVED; KFMT RETURNS PACKED SIXBIT DECIMAL DIRECTLY.

PIPES AND FILESYSTEM FIFOS ARE NOW ONE OPAQUE 36-BIT WORD-FIFO FACILITY.  EACH
LIVE OBJECT IS 14 WORDS, INCLUDING AN EIGHT-WORD PAYLOAD RING, AND USES THE
EXISTING READ_WORDS/WRITE_WORDS ABI WITH LEGAL SHORT POSITIVE TRANSFERS.  ON
THE FIXED PRE-REDESIGN BASE THIS CHANGED KCORE FROM 9728 TO 9722 WORDS, SIX
WORDS SMALLER THAN THE OLD PACKED-CHARACTER PIPE.  TEXT/BYTE/MEDIA CONVERSION
IS OUTSIDE IPC.  THE COMMON FIXED-WIDTH TOKEN CONTRACT DEFINES SPECIALIZED
6/7/8/12-BIT MSB-FIRST PACKING; S6REC, TTY, PAPER-TAPE, AND CARD LAYERS OWN
THEIR OWN FRAMING.

TTY WORD I/O CONVERTS COOKED S6REC AT THE TTY MRES BOUNDARY WHILE RAW
READCHAR/WRITECHAR REMAINS CHARACTER-ORIENTED.  COOKED READ_WORDS REQUIRES AN
ATTACHED CANONICAL TTY; UNOWNED CTY BOOTSTRAP INPUT USES READCHAR AND IS NOT
SYNTHESIZED INTO ONE-CHARACTER S6REC RECORDS.  TTY AND LPT SHARE ONE MEASURED
20-WORD KCORE S6REC TEXT-FRAME VALIDATOR; THIS IS SMALLER IN THE COMPLETE
PERMANENT IMAGE THAN DUPLICATING THE CHECK IN BOTH MRES PACKAGES.  PTR/PTP USE FOUR 8-BIT TOKENS
PER WORD WITH THE LOW NIBBLE COUNTING A FINAL 1..3-BYTE PARTIAL WORD.  CR/CP
USE 27 WORDTOKEN12 WORDS FOR ONE FIXED 80-COLUMN CARD.  LPT ACCEPTS S6REC WORD
OUTPUT AND PACKS UP TO FIVE SEVEN-BIT CHARACTERS PER LP10 DATAO.  THESE DEVICE
ADAPTERS ARE SPECIALIZED FOR SPEED AND LIVE WITH OPTIONAL MRES PACKAGES WHERE
POSSIBLE; THEY DO NOT ADD PER-STREAM RESIDENT BUFFERS.

CURRENT WORD-I/O MRES MEASUREMENTS AFTER THE LEGACY/API PURGE ARE: PTR 57
WORDS, PTP 62, CR 68, CP 62, TTY 151, AND LPT 64 WORDS OF INSTALLED RESIDENT
PACKAGE TEXT/STATE.  THE OLD DIRECT UUO-044 GETCHAR ALIAS AND THE TTY
NO-CONTROLLING-TTY S6REC RECONSTRUCTION PATH ARE REMOVED; READCHAR/WRITECHAR
REMAIN THE CHARACTER ABI.

THE 2026-10-01 CORE AUDIT REDUCED THE RESIDENT PDP-6 PI TABLE FROM 13 TO THE
PROVEN EIGHT SIMULTANEOUS HANDLERS REQUIRED BY THE COMPLETE CURRENT DEVICE SET,
RECOVERING FIVE KCORE BSS WORDS.  PDP-6 SKIP-FORM SIMPLIFICATION OF THE SEVEN
PI USER-STACK SWITCHES AND PI RESTORE RECOVERED ANOTHER EIGHT KCORE TEXT WORDS
WHILE REMOVING EXECUTED INSTRUCTIONS; THE PI OBJECT IS NOW 105 TEXT + 31 BSS
WORDS.  COMPRESSED EXEC'S VFS D6LZ REFILL WINDOW IS 64 TRANSIENT STACK WORDS,
UP FROM EIGHT.  CURRENT LOGIN/DSH PAYLOADS REQUIRE ABOUT ONE EIGHTH AS MANY VFS
REFILLS; THE PROCESS-KERNEL-STACK WATERMARK REMAINS 0307 OCTAL WORDS IN THE
ACCEPTANCE WORKLOAD.  THE SAME D6LZ RUNTIME ADAPTER NOW ALSO HAS A DIRECT-
BUFFER ENTRY FOR TSFS: TSFS CONTINUES TO AVOID VFS REFILLS, BUT NO LONGER
DUPLICATES DECODER AC SAVE/RESTORE, STATE INITIALIZATION, OR EXACT-CONSUMPTION
CHECKS.  THE ADAPTER GROWS FROM 42 TO 49 KCORE WORDS WHILE TSFS_RUNTIME SHRINKS
FROM 503 TO 489 WORDS, FOR A NET SEVEN-WORD PERMANENT SAVING.  THE CHANGE ALSO
CORRECTS THE TSFS DECODE DESTINATION TO USE THE ACTUAL MM_ALLOC RESULT STACK
SLOT.  BOTH FRONTENDS STILL FEED THE SAME FIXED 40-WORD LOW-CORE TOKEN ENGINE.

MONITORFS DEVICE STATS ARE INTENTIONALLY COMPACT IN 0.9/1.0: EVERY DEVICE
REPORTS ONLY COMPLETED READ REQUESTS, COMPLETED WRITE REQUESTS, AND ERRORS.
THE UNUSED D6SET LOG STATS LEAF AND SEPARATE MTC-WORD/D6SET-BLOCK LIFETIME
VOLUME COUNTERS WERE REMOVED; NONE PARTICIPATED IN I/O, SCHEDULING, RECOVERY,
OR USERSPACE POLICY.

WCNSLS DEVICE 0420 IS NOW EXPOSED TO PRIVILEGED USERSPACE AS RAW
`/DEV/WCNSLS` BULK WORD I/O AND REMAINS COMPLETELY OUTSIDE THE TTY LAYER.
MONITORFS DEVICE MODE 0600 PROVIDES THE OPEN-TIME ROOT PRIVILEGE GATE.  A
NONZERO READ_WORDS REQUEST RETURNS ONE RAW DATAI 0420 SAMPLE.  WRITE_WORDS
EXECUTES A BATCH IN WHICH BIT 35 TAGS CONO AND AN UNTAGGED WORD IS SENT BY
DATAO; PICTURE STORAGE, COLOR SEQUENCING, REFRESH TIMING, AND ANIMATION ALL
BELONG TO USERSPACE.  THE FILE DESCRIPTOR IS THE CAPABILITY AFTER OPEN, SO A
PRIVILEGED RT CLIENT DOES NOT PAY A CREDENTIAL CHECK PER POINT.  THE RETAINED
WCNSLS PACKAGE IS 22 WORDS WITH NO BSS, PI HANDLER, FRAMEBUFFER, DISPLAY LIST,
OR REFRESH SERVICE.  KINIT CONTINUES TO USE ITS DISPOSABLE RAW PRIMITIVES FOR
PROBE AND THE BOOT BANNER.  DAIMOS-TESTKIT TARGET `test-daimos-wcnsls-raw`
PASSES AGAINST SIMH WITH EXACT DATAI/CONO/DATAO TRACE VALIDATION.

THE KCORE LINK RUNS P10FOLD OVER ALL HANDWRITTEN KCORE ASSEMBLER BEFORE
DLINK.  SIX RELOCATION-AWARE ZERO-EXECUTION-COST DONOR FOLDS REMOVE 37 KCORE
WORDS.  THE COMPLETE 2026-09-30 P10SUPER HANDWRITTEN SOURCE SWEEP ALSO APPLIED
14 PROVEN ONE-WORD PEEPHOLES, REDUCING KCORE FROM 9655 TO 9641 WITHOUT
CHANGING THAT FOLD PLAN.  THE FOLLOW-UP SCAN HAS TEN SCHEDULING-EQUIVALENT
PAIRS BUT ZERO PROVEN FOLD VALUE, SO GLOBAL-PLAN SELECTS NONE.  NO FURTHER
PEEPHOLE, ZERO-FORM, OR AC-RENAME NORMALIZATION REMAINS.  KCC-GENERATED
ASSEMBLER REMAINS DELIBERATELY EXCLUDED UNTIL SELF-HOSTING/COMPILER
CANONICALIZATION WORK.

MRES FOLDING IS PACKAGE-LOCAL.  THE DTFS PACKAGE RUNS P10FOLD ONLY OVER ITS
HANDWRITTEN DTFS_RUNTIME/TSFS_RUNTIME INPUTS AND REMOVES ONE 5-WORD ZERO-COST
DUPLICATE BEFORE MRES PACKAGING.  THE SUPPORTED PDP-6 BUILD NOW DISABLES
TENEX/ITS DTFS COMPATIBILITY AND RETAINS ONLY NATIVE DTFS PLUS TSFS; THE
RESULTING PACKAGE IS 1848 WORDS.  FOREIGN-PERSONALITY SOURCE REMAINS DORMANT
REFERENCE CODE AND IS NOT PART OF THE PERMANENT-SIZE BASELINE.  OTHER HANDWRITTEN MRES PACKAGE
INPUTS CURRENTLY HAVE NO ZERO-COST FOLD AT THE FOUR-WORD THRESHOLD.

THE 32K LOWMEM PROFILE IS NOT A DIFFERENT DAIMOS VARIANT.  IT BUILDS THE SAME
SOURCE TREE FOR A PDP-6 LIMITED TO 32K WORDS, USES THE SMALLER TRANSIENT
KINIT_FULL=0 INITIALIZATION PROFILE, DEFAULTS KINIT_BADMAP=0, AND THEN BOOTS
THROUGH THE SAME NORMAL INIT -> LOGIN -> DSH PATH.

## 2. BOOT AND ROOT POLICY

STAGE1 SELECTS THE ROOT CONTROLLER CLASS.  KINIT DISCOVERS AND VALIDATES THE
ROOT SET AND FILESYSTEM, INSTALLS ONLY THE REQUIRED PERMANENT PACKAGES, LOADS
`/SYSTEM/INIT`, AND RECLAIMS BOOT-ONLY STATE.

SUPPORTED V0.9 ROOT PATHS INCLUDE DSK/D6FS, DRM/D6FS, AND DTC/TSFS.  AUTO ROOT
POLICY PREFERS DSK, THEN DECTAPE, THEN DRM.

TSFS V1.1 SETS MAY USE ALL EIGHT DTC UNITS AS LOGICAL MEMBERS 0..7.  BOTH
KINIT ROOT DISCOVERY AND THE TRANSIENT USERSPACE MOUNT SCANNER ACCEPT EIGHT
MEMBERS.  THE RUNTIME HANDOFF REMAINS COMPACT: ITS THREE-BIT-PER-MEMBER MAP
USES 24 OF THE AVAILABLE 36 BITS FOR AN EIGHT-MEMBER SET.  THE SEPARATE
GENERIC BLOCKSET DESCRIPTOR LIMIT REMAINS SEVEN MEMBERS AND IS NOT A TSFS
FORMAT LIMIT.

HARDWARE/ROOT DISCOVERY STAYS IN STAGE1/KINIT.  ADMINISTRATIVE POLICY SUCH AS
ADDITIONAL MOUNTS AND SWAP/LOGSTORE SELECTION BELONGS IN INIT AND `/CONFIG`.
INIT READS `/CONFIG/FSTAB`; MEMFS SCRATCH STORAGE IS CREATED ONLY WHEN A
MEMFS ENTRY REQUESTS IT AND IS NO LONGER INSTANTIATED BY KINIT.

USERSPACE CONFIGURATION USES TWO SMALL STREAMING TEXT GRAMMARS.  PASSWD/FSTAB/
INITTAB-STYLE FILES ARE COLON-DELIMITED RECORDS WHOSE FIELD SEMANTICS BELONG TO
THE CONSUMER.  SIMPLE POLICY FILES USE `KEY = VALUE ...` RECORDS WITH ZERO OR
MORE SPACE-SEPARATED VALUES.  THE SHARED USERLAND TEXT LIBRARY PERFORMS S6REC
READING, WHITESPACE/COMMENT HANDLING, AND ALLOCATION-FREE TOKENIZATION; IT DOES
NOT OWN APPLICATION POLICY OR BUILD AN IN-MEMORY CONFIGURATION TREE.

## 3. PROCESS, VM, IPC, AND TTY

THE PDP-6 VM BACKEND USES CONTIGUOUS USER EXTENTS, RELOCATION/PROTECTION,
PROCESS COMPACTION, AND WHOLE-IMAGE SWAPPING.  CLEAN RECLAIMABLE FILESYSTEM
CACHE IS DROPPED BEFORE PROCESS SWAP.

MEMFS USES DEMAND-BACKED STORAGE INSTEAD OF A FIXED PREALLOCATED/COMPACTED RAM
POOL.  PHYSICAL CORE REMAINS OWNED BY MM EXTENTS; EVICTED
MEMFS DATA SHARES THE EXISTING D6FS SWAP RESERVATION WITH PROCESS SWAP.  THE
SWAP RESERVATION IS ONE DYNAMICALLY SHARED POOL, NOT A PROCESS/MEMFS
PARTITION.  THE COMMON SWAP STORE SHALL REUSE THE COMPACT D6FS-STYLE
BITMAP/CONTIGUOUS-RUN ALLOCATION MECHANISM RATHER THAN ADDING A THIRD EXTENT
ALLOCATOR.  MEMFS RECLAIM IS INSERTED BEFORE PROCESS SWAP WHERE THAT FREES
CORE CHEAPLY; CLEAN MEMFS DATA WITH VALID SWAP BACKING MAY BE DROPPED WITHOUT
ANOTHER WRITE.

MEMFS ALLOCATES ONE BLOCK-ALIGNED 640-WORD METADATA AREA AT MOUNT: 448 WORDS
FOR 64 SEVEN-WORD NODES, 64 PER-NODE BACKING DESCRIPTORS, 64 DIRECT COMPACT
UID9,GID9 OWNER WORDS, AND 64 MTIME36 WORDS.  FILE-DATA CHUNKS STILL COME FROM
MM ONLY ON FIRST USE; MEMFS RELOCATES ONLY THE FILE BEING GROWN AND RETURNS A
COMPLETELY FREE CHUNK TO MM.  THE OLD GLOBAL DATA-POOL COMPACTION PATH IS
REMOVED.  THE CURRENT
FOUR-CHUNK BOUND LIMITS MEMFS TO FOUR MM DESCRIPTORS.  SWAP-STORE ALLOCATION
IS SHARED AND MEMFS ALLOCATION PRESERVES THE ORIGINAL 0200-BLOCK PROCESS-SWAP
RESERVE.

MEMFS EVICTION/FAULT-IN IS NOW IMPLEMENTED.  EACH OF THE AT MOST FOUR RESIDENT
MM CHUNKS IS SECTOR-ALIGNED INTERNALLY; UNDER MM PRESSURE MEMFS WRITES DIRTY
FILES IN A CHUNK TO SHARED SWAP, PUBLISHES ONE FIRST-BLOCK,,BLOCK-COUNT WORD
PER NODE, THEN RELEASES THE WHOLE MM CHUNK.  READ/WRITE/TRUNCATE FAULT DATA
BACK ON DEMAND.  CLEAN BACKING SURVIVES FAULT-IN UNTIL THE FILE IS MODIFIED,
SO A SECOND EVICTION NEEDS NO WRITE.  MM PRESSURE ORDER IS D6FS CLEAN CACHE,
MEMFS EVICTION, THEN PROCESS SWAP.  MEMFS CANNOT SELF-EVICT WHILE ACQUIRING A
NEW MM CHUNK.  THE PER-NODE BACKING WORDS LIVE BESIDE THE DYNAMIC NAMESPACE,
NOT IN PERMANENT MRES DATA.

FILESYSTEM MEMORY PRESSURE NOW USES ONE MM-FACING RECLAIM INTERFACE WITH TWO
CLASSES: CHEAP/DISCARDABLE CACHE FIRST, THEN BACKED FILESYSTEM DATA.  D6FS AND
DTFS/TSFS SHARE THE COMMON CLEAN BCACHE L1; MEMFS SUPPLIES THE SECOND CLASS
WITH SWAP-BACKED CHUNK EVICTION.  THEIR BACKING SEMANTICS AND MOVABLE MRES
LIFETIMES REMAIN SEPARATE.  THIS REMOVES D6FS/MEMFS POLICY NAMES FROM MM
WITHOUT ADDING A THIRD CACHE/EXTENT MANAGER.

THE STORAGE CONSOLIDATION IS IMPLEMENTED.  PROCESS SWAP AND MEMFS SHARE THE
GENERIC BSTORE BITMAP/RUN ALLOCATOR, WHILE D6FS AND DTFS/TSFS USE THE SHARED
CLEAN TWO-BLOCK BCACHE L1.  LOGSTORE REMAINS A FIXED RAW REGION WITH ITS OWN
RING/RECOVERY SEMANTICS.  AUXILIARY STORAGE IS FILESYSTEM-INDEPENDENT; DO NOT
ADD A SECOND MEMFS CACHE/BACKING ALLOCATOR OR A DIRTY GENERAL BLOCK CACHE.  THE
DETAILED CONTRACT IS IN PDP10-DOC `KERNEL/BSTORE.MD`.

THE NORMAL PDP-6 `MAKE BOOT` MEDIA LAYOUT NOW USES THE HDD SET AS THE D6FS
ROOT AND DRM0 AS NON-ROOT AUXILIARY STORAGE.  DRM0 IS FORMATTED AT THE FULL
TYPE-236 CAPACITY OF 020000 0200-WORD BLOCKS.  BLOCK 0 HOLDS THE AUXSTORE
DESCRIPTOR; THE LOW REGION AFTER IT IS AN EMPTY, INDEPENDENT D6FS FILESYSTEM,
AND THE TAIL HOLDS FIXED LOGSTORE, BACKSTORE, AND CACHESTORE REGIONS.  THE
DEFAULT SIZES ARE LOGSTORE 0400 BLOCKS, BACKSTORE 04000 BLOCKS, AND CACHESTORE
02010 BLOCKS, LEAVING 011367 BLOCKS FOR THE NON-ROOT D6FS PARTITION.  NO KINIT
BOOT STREAM OR ROOT PAYLOAD IS COPIED TO THIS NORMAL AUXILIARY DRUM.  DRM1-3
ARE BLANK FULL-CAPACITY MEDIA.  AN EXPLICIT DRM BOOT (`STAGE1_DEVICE=drm`)
RETAINS THE BOOTABLE D6FS DRUM-SET BUILD PATH.

THE TOP-LEVEL `MAKE BOOT` POLICY IS EXPLICIT: IT INVOKES THE IMAGE BUILD WITH
`BOOT=DSK ROOT=DISK`.  IT THEREFORE ALWAYS LOADS STAGE1 FROM DSK AND SELECTS
THE DSK D6FS ROOT, INSTEAD OF RELYING ON AUTO ROOT PROBING.  ALTERNATE BOOT
SOURCE/ROOT COMBINATIONS REMAIN AVAILABLE THROUGH THE LOWER-LEVEL IMAGE BUILD
AND THEIR EXPLICIT `BOOT=`/`ROOT=` SETTINGS.
THE TOP-LEVEL MAKEFILE ALSO PASSES THESE VALUES EXPLICITLY; ITS GENERIC
`ROOT?=AUTO` VARIABLE IS NOT FORWARDED BY `MAKE BOOT`.  `DISK-BOOT` IS AN
ALIAS OF THAT SAME FIXED DSK/D6FS BOOT POLICY.

OPTIONAL PERSISTENT MEMFS IS IMPLEMENTED: `PERSIST` IN `/CONFIG/FSTAB` MAKES
ORDERLY HALT COMMIT A VERSIONED/CHECKED SNAPSHOT INTO AN EXPLICIT SWAP-TAIL
RANGE, AND A PERSISTENT MOUNT REUSES THAT SNAPSHOT AS DEMAND-LOAD BACKING.
ORDINARY UNMOUNT AND
DEFAULT MEMFS REMAIN VOLATILE; THE NORMAL RUNTIME SWAP BITMAP REMAINS EPHEMERAL.
VOLATILE MEMFS DOES NOT REQUIRE SWAP: WITH A ZERO-LENGTH SWAP TAIL ITS RECLAIM
PATH RETURNS IMMEDIATELY AND MEMFS REMAINS RAM-ONLY UNTIL MM EXHAUSTION.
PERSISTENT MEMFS FAILS ITS PERSISTENT MOUNT MODE IF NO EXPLICIT PERSISTENT
BACKING RANGE EXISTS; IT NEVER SILENTLY BECOMES VOLATILE.  THE FINAL SWAP-TAIL
BLOCK IS THE DISCOVERY/COMMIT HEADER.  KINIT RESERVES A VALID RECORDED RANGE
BEFORE PROCESS SWAP CAN USE IT; MEMFS RESTORES THE NAMESPACE AND LEAVES FILE
DATA DEMAND-BACKED THERE.  SHUTDOWN INVALIDATES THE HEADER FIRST, WRITES FILE
DATA AND METADATA, THEN PUBLISHES THE VERSIONED HEADER LAST.

PROCESS GROUPS, SESSIONS, CONTROLLING TERMINALS, FOREGROUND GROUPS,
STOP/CONTINUE, CHILD STATE REPORTING, DESCRIPTOR I/O, PIPES/FIFOS, AND NATIVE
PROCESS EVENTS ARE IMPLEMENTED.

PROCESS-SLOT EXHAUSTION CANNOT LOCK OUT ADMINISTRATION.  THE HIGHEST TWO
RUNTIME PROCESS SLOTS ARE RESERVED FOR UID 0: ORDINARY RUN ALLOCATION STOPS
BEFORE THAT TAIL, WHILE ROOT/INIT MAY USE THE COMPLETE TABLE.  THE POLICY ADDS
NO PER-UID COUNTERS OR QUOTA TABLE; ON THE 24-SLOT MINIMUM CONFIGURATION IT
LEAVES 21 ORDINARY USER SLOTS PLUS PID 1 AND TWO ROOT-RESERVED SLOTS.

THE STABLE PROCESS U-AREA IS 0407 WORDS.  USER-PREEMPTION AND
SLEEPING-EXECUTIVE REGISTER IMAGES SHARE ONE CONTEXT AREA because those
scheduler states are mutually exclusive and the sleeping syscall's eventual
user-return frame already lives on the stable private kernel stack.  This
saves 17 physical RAM words per live process while retaining the full 0316
private-stack words introduced by the earlier overflow fix.  The watermark
acceptance scans all 0316 words, rejects saturation at capacity as well as the
existing 0310 policy limit, and currently measures a 0301-word high water.
AND `LS /TEMP`, `DU /TEMP`, AND PLAIN `DU` PASS AFTER THE CHANGE.

THE TTY LINE DISCIPLINE SITS BETWEEN TERMINAL DEVICE CHARACTERS AND PROCESS
READ/WRITE CALLS.  IN COOKED MODE IT COLLECTS A LINE BEFORE DELIVERING IT TO A
READER, ECHOES INPUT, INTERPRETS ERASE AND KILL EDITING, RECOGNIZES EOF, AND CAN
TURN CTRL-C/CTRL-Z INTO FOREGROUND PROCESS-GROUP SIGNAL EVENTS.  IN RAW MODE IT
DOES NOT PERFORM LINE EDITING OR WAIT FOR A COMPLETE LINE: INPUT IS DELIVERED
DIRECTLY SO INTERACTIVE PROGRAMS SUCH AS EDITORS AND SHELL LINE EDITORS CAN
IMPLEMENT THEIR OWN KEY HANDLING.

LOGICAL TTY INPUT IDENTITY AND OUTPUT ROUTING ARE NOW SEPARATE.  TTY0 REMAINS
THE CTY KEYBOARD INPUT, TTY1..16 REMAIN DCS0..15 INPUT, AND TTY17..20 REMAIN
GE0..3 INPUT; EACH LOGICAL TTY MAY INDEPENDENTLY ROUTE OUTPUT TO CTY, ANY DCS
LINE, ANY GE CONSOLE, OR THE TYPE-340 DPY SINK.  ZERO ROUTE BITS MEAN THE
HISTORICAL NATIVE INPUT/OUTPUT PAIR.  THE FIVE-BIT OUTPUT ROUTE OCCUPIES
PREVIOUSLY UNUSED BITS 29..33 OF EACH EXISTING TTY RECORD, SO ROUTING ADDS NO
FIXED RAM TABLE AND SURVIVES LOGIN/SESSION DETACH.  ROOT MAY CHANGE THE GLOBAL
ROUTE WITH TTYCTL/DSH TTYOUT; GETOUT IS UNPRIVILEGED.

THE TYPE-340 DPY SINK HAS A RETAINED 84-BY-46 SCALE-2 TERMINAL.  THE FIRST
ACTUAL DPY TERMINAL BYTE LAZILY ALLOCATES ONE 1288-WORD
MM_TYPE_KERNEL_DYNAMIC NATIVE-TEXT EXTENT; THIS WORKING AREA IS NOT PERMANENT
KCORE/MRES AND MUST NOT BE ADDED TO PERMANENT-SIZE RESULTS.  EACH ROW HAS
FOURTEEN SIX-CELL BLOCKS, TWO WORDS PER BLOCK.  A ZERO FIRST WORD DENOTES AN
UNINITIALIZED SIX-BLANK-CELL BLOCK AND MAKES THE SECOND WORD IRRELEVANT.  ON
FIRST USE BOTH WORDS ARE INITIALIZED TO SIX DIRECTLY EXECUTABLE TYPE-342
(SI/SO,GLYPH) PAIRS, THREE PAIRS PER WORD.  EVERY TOUCHED CELL THEREFORE CARRIES
ITS OWN CHARACTER-SET SELECTION.  THE SAME BLOCK IMAGE IS EDITED BY THE TTY
PATH AND FED DIRECTLY TO BLKO: THERE IS NO SECOND SIXBIT CHARACTER PLANE, ROW
LENGTH TABLE, OR COMPILED-ROW CACHE.

LOWERCASE STREAM OUTPUT IS NORMALIZED TO TYPE-342 UPPERCASE.  SHIFTED
PUNCTUATION CHANGES ONLY THE SELECTED FIXED PAIR.  IF CURSOR MOTION FIRST
TOUCHES A LATER BLOCK, PREDECESSOR BLOCKS ARE INITIALIZED TO EXPLICIT BLANK
PAIRS SO THE USED ROW PREFIX IS CONTIGUOUS BLKO DATA.  ROW CLEAR STILL ZEROS
ONLY THE FIRST WORD OF EACH BLOCK.  SCROLLING ROTATES A 46-ROW RING WITHOUT
COPYING THE SCREEN, AND dpy_text_rows_used BOUNDS REFRESH AT THE LAST LOGICAL
ROW THAT CAN CONTAIN VISIBLE TEXT.  THE SCALE-2 TYPE-342 LINE HEIGHT IS 22
DISPLAY UNITS; THE FRAME STARTS AT Y=1010, SO 46 ROWS FIT INSIDE THE 0..1023
TYPE-340 RASTER AND THE TOP-ROW GLYPH EXTENT ENDS AT Y=1022.  FORTY-SIX ROWS
ARE THEREFORE THE MAXIMUM SAFE HEIGHT AT THE CURRENT SCALE.

THE 2026-10-05 SCROLL REGRESSION WAS IN THE COMPACT ASSEMBLY REWRITE, NOT THE
RING DESIGN: dpy_text_scroll LOADED THE OLD RING TOP INTO AC5 BUT THEN USED
`AOJ 1,5`, SO IT DID NOT COMPUTE old_top+1.  THE SCROLL PATH NOW FORMS THE
NEXT TOP DIRECTLY WITH `MOVEI 1,1(5)`.  THE PUBLIC /SYSTEM/EXEC/CLEAR COMMAND
WRITES FORM FEED; THE RETAINED DPY TERMINAL ALREADY DEFINES FORM FEED AS
SCREEN CLEAR PLUS CURSOR/RING RESET, SO NO DPY-SPECIFIC USERSPACE ABI WAS
ADDED.

THE SUBSEQUENT BLKO INTEGRATION EXPOSED A SECOND 42-ROW LIMIT: dpy_io.s STILL
USED THE OLD OCTAL 052 DPY_TEXT_ROWS VALUE, AND ITS PRIVATE BANNER REFRESH
SENTINEL ALSO USED 052.  THE RETAINED STORE AND SCROLL PATH COULD HOLD 46 ROWS,
BUT THE BLKO REFRESH STATE MACHINE TERMINATED WHEN dpy_refresh_row REACHED
LOGICAL ROW 42.  dpy_io.s NOW USES OCTAL 056 FOR 46 TEXT ROWS AND RESERVES
OCTAL 056..060 FOR BANNER/ONESHOT/TRAILER REFRESH STATES, STRICTLY ABOVE THE
VALID LOGICAL ROW RANGE 0..055.

THE NORMAL FULL GUI `make boot` NOW KEEPS TTY0 ON ITS NATIVE CTY OUTPUT ROUTE,
SO LOGIN/DSH AND DIAGNOSTICS REMAIN VISIBLE ON CTY WHILE /DEV/DPY0 IS RESERVED
FOR RAW DISPLAY-PROGRAM TESTING SUCH AS DPYVIEW.  DPY_LOGIN=1 REMAINS AN
EXPLICIT OPTIONAL BUILD SETTING; WHEN SELECTED, TTY0 INPUT STILL USES THE CTY
KEYBOARD WHILE OUTPUT IS ROUTED TO THE TYPE-340 DISPLAY.

MONITORFS ALSO PUBLISHES /DEV/TTYDPY0 WHEN BOTH CTY0 AND DPY0 ARE INSTALLED.
IT IS A COMPOSITE LOGICAL TTY0 ENDPOINT: READS USE CTY0, WRITES GO DIRECTLY TO
THE RETAINED DPY TEXT BACKEND, AND ISATTY TREATS IT AS THE CALLER'S
CONTROLLING TTY0.  IT DOES NOT MUTATE TTY0'S PERSISTENT OUTPUT ROUTE.
ECHO TEST > /DEV/TTYDPY0 HAS BEEN ACCEPTED ON A GUI BOOT; AFTER SEVERAL
REFRESH INTERVALS THE FRAMEBUFFER CONTAINED 56 LIT PIXELS OVER BBOX
(0,37)-(45,50), DISTINCT FROM THE 147-PIXEL BOOT BANNER, WITH CPU PIH ZERO.

/DEV/DPY0 ALSO HAS A WORD-ORIENTED NATIVE DISPLAY-PROGRAM ABI.  A
WRITE_WORDS SUBMISSION REPLACES THE ACTIVE PERSISTENT TYPE-340 LIST.  THE
DRIVER COPIES THE WORDS INTO ONE DYNAMIC EXTENT, PUBLISHES AN IMMUTABLE BLKO
IOWD WITH PI7 DISABLED, AND THE LINE-CLOCK HOOK STARTS ONE FINITE BLKO PASS AT
THE NORMAL 30-HZ CADENCE.  RAW-LIST BLKO OVERFLOW NOW ENDS THAT PASS, DISABLES
DPY PI REQUESTS, AND REARMS THE CLOCK DIVIDER INSTEAD OF IMMEDIATELY RELOADING
THE IOWD AT DEVICE SPEED.  WHILE THE RAW LIST EXISTS IT HAS PRIORITY OVER
RETAINED-TEXT REFRESH; A ZERO-LENGTH WRITE/CLOSE RELEASES THE EXTENT AND THE
NEXT 30-HZ TICK RESUMES TTY REFRESH.

/SYSTEM/EXEC/DPYVIEW IS THE INITIAL USERSPACE PICTURE VIEWER.  THE KERNEL DOES
NOT DECODE IMAGE FORMATS OR CONSTRUCT GRAPHICS OBJECTS; IT ONLY RETAINS AND
REPLAYS THE SUPPLIED NATIVE 36-BIT DISPLAY PROGRAM.  DPYVIEW NOW ACCEPTS
NATIVE FRAMES UP TO 32256 WORDS.  ITS BSS IS 32301 WORDS INCLUDING THE FRAME
BUFFER, WHICH REMAINS BELOW THE CURRENT 077777-WORD DXR BSS FIELD LIMIT.
DPYVIEW SUBMITS THE FRAME ONCE AND, WHEN STDIN IS ITS CONTROLLING TTY, BLOCKS
IN THE NORMAL READCHAR PATH WHILE THE KERNEL REFRESHES IT.  THIS LETS THE
EXISTING COOKED-TTY CTRL-C PATH DELIVER INT TO THE FOREGROUND PROCESS GROUP;
OTHER INPUT IS IGNORED.  NON-TTY STDIN RETAINS THE LOW-COST SLEEP FALLBACK.
THE FORMER TWO-TICK USERSPACE RESUBMISSION LOOP WAS REMOVED BECAUSE IT RESET
AND REALLOCATED THE SAME PERSISTENT LIST WHILE THE DRIVER WAS ALREADY
REFRESHING IT.

DXR V1 USES ALL 15 BSS BITS AVAILABLE BELOW ITS THREE WORD-1 FLAG BITS.  THE
HISTORICAL 020000-WORD BSS ADMISSION CAP HAS BEEN REMOVED; BSS MAY NOW RANGE
THROUGH 077777 WORDS, WHICH IS THE FORMAT'S NATURAL LIMIT and remains far
below the PDP-6 256K-word process-address-space ceiling even with the current
maximum image and fixed stack.  This is required by large static-buffer tools
such as DPYVIEW and does not enlarge any resident kernel allocation.
HOST-SIDE IMG2DPIC IS NOW A C99/MAGICKWAND HOST PROGRAM WITH TWO LOSSLESS
TYPE-340 ENCODERS.  THE VECTOR-RUN ENCODER MEASURES HORIZONTAL AND VERTICAL
SERPENTINE RUNS, INCLUDING BOTH INITIAL DIRECTIONS.  THE INCREMENTAL ENCODER
GREEDILY COVERS THE SAME LIT PIXELS WITH 8-CONNECTED ONE-PIXEL PATHS AND PACKS
FOUR PATH STEPS PER TYPE-340 INCREMENTAL HALFWORD.  EACH PATH START IS
INTENSIFIED EXACTLY ONCE; ONE-TO-THREE RESIDUAL STEPS ARE EMITTED AS ORDINARY
ONE-PIXEL VECTORS RATHER THAN ZERO-PADDED INCREMENT WORDS, SO ENDPOINTS ARE NOT
ARTIFICIALLY BRIGHTENED.  IMG2DPIC MEASURES BOTH REPRESENTATIONS AND EMITS THE
SHORTER PROGRAM.

THE OPTIMIZATION IS LOSSLESS WITH RESPECT TO THE THRESHOLDED 1024X1024 BITMAP.
RENDER-BACK VALIDATION OF ALL 25 CURRENT PICTURES PRODUCED BIT-FOR-BIT IDENTICAL
LIT-PIXEL MAPS.  AGAINST THE PRE-OPTIMIZATION DISPLAY PROGRAMS THE FINAL
ENCODER REDUCES THE CORPUS FROM 359396 TO 239465 WORDS, ABOUT 33.4 PERCENT.
PICTURE08 FALLS FROM 30324 TO 15092 WORDS, WHICH REDUCES THE CURRENT SIMH
FIXED-PER-WORD REPLAY ESTIMATE FROM ABOUT 3.03 TO 1.51 SECONDS.

IMG2DPIC ALSO HAS AN AUTOMATIC LINE-ART STROKE CANDIDATE FOR THE 100-MS
PICTURE-REFRESH TARGET.  IT PERFORMS TOPOLOGY-PRESERVING THINNING, REMOVES
FALSE DIAGONAL CORNER CONNECTIONS, TRACES THE RESULTING STROKE GRAPH, APPLIES
ONE-PIXEL-TOLERANCE DOUGLAS-PEUCKER VECTOR SIMPLIFICATION, AND ORDERS/REVERSES
STROKES TO MINIMIZE DARK BEAM TRAVEL.  THIS APPROXIMATE MODE IS NOT UNIVERSAL
BY ITSELF: IT IS ELIGIBLE ONLY WHEN THE GENERATED STROKES PASS BOUNDED-ERROR
CHECKS AGAINST THE ORIGINAL THRESHOLDED RASTER.  AT LEAST 99.5 PERCENT OF
SOURCE LIT PIXELS MUST LIE WITHIN TWO PIXELS OF THE STROKE RESULT, EVERY SOURCE
LIT PIXEL MUST LIE WITHIN FIVE PIXELS, AND EVERY GENERATED STROKE PIXEL MUST
LIE WITHIN ONE PIXEL OF THE SOURCE.  OTHERWISE IMG2DPIC AUTOMATICALLY FALLS
BACK TO THE LOSSLESS VECTOR/INCREMENTAL ENCODERS.

ON THE CURRENT PICTURE CORPUS FIVE IMAGES PASS THAT LINE-ART GATE.  THEIR
ESTIMATED TYPE-340 EXECUTION TIMES ARE PICTURE08 96.18 MS, PICTURE20 16.28 MS,
PICTURE21 48.50 MS, PICTURE24 50.21 MS, AND PICTURE25 20.29 MS.  PICTURE08 IS
REDUCED FROM THE ORIGINAL 30324 WORDS TO 1923 WORDS IN STROKE MODE.

IMAGE PROCESSING USES A 1024X1024 HOST-SIDE BILEVEL RASTER AFTER IMAGEMAGICK
RESIZE/GRAYSCALE/50-PERCENT THRESHOLDING; THERE IS NO PYTHON OR
INTERMEDIATE-PBM DEPENDENCY.  WHITE INPUT PIXELS ARE INTENSIFIED AND BLACK
PIXELS REMAIN DARK.  RASTER Y IS REFLECTED BECAUSE IMAGE ROW ZERO IS THE TOP
EDGE WHILE TYPE-340 Y=0 IS THE BOTTOM EDGE.

SYS_PROCINFO COMM NOW DERIVES UP TO SIX SIXBIT BASENAME CHARACTERS DIRECTLY
FROM THE LIVE PROCESS STARTUP ARGV[0] RECORD.  THIS REPLACES THE FORMER
HARD-CODED USER NAME WITHOUT ADDING A PER-PROCESS COMM WORD; SWAPPED OR
OTHERWISE UNAVAILABLE IMAGES STILL FALL BACK TO USER.  /SYSTEM/EXEC/KILL PID
USES THE EXISTING PROCCTL EVENT_PID ABI TO SEND TERM, SO PROCESS TERMINATION
NEEDS NO NEW KERNEL SIGNAL MECHANISM.

THE SOURCE-TREE `pictures/` DIRECTORY IS A LOCAL, GIT-IGNORED MEDIA-STAGING
AREA.  D6FS ROOT-IMAGE BUILDS CREATE `/PICTURES` AND IMPORT EACH REGULAR
FILE FOUND DIRECTLY IN THAT DIRECTORY AS A MODE-0444
36-BIT BINARY-WORD FILE.  HOST BASENAMES ARE FOLDED TO UPPERCASE FOR THE
SIXBIT NAMESPACE; STAGED NAMES ARE LIMITED TO 24 CHARACTERS FROM A-Z, 0-9,
DOT, UNDERSCORE, PLUS, OR MINUS.  CHANGING A STAGED FILE MAKES IT A ROOT-IMAGE
DEPENDENCY, SO A SUBSEQUENT MEDIA BUILD REIMPORTS IT.
TSFS ROOT-IMAGE DEPLOYMENT EXCLUDES `/PICTURES` ENTIRELY, INCLUDING ITS
DIRECTORY, ALL PICTURE FILES, AND THE PICTURE-FILE BUILD PREREQUISITES.

TYPE-340 DATA REQUESTS RUN ON LOWEST-PRIORITY PI7, WHILE APR CLOCK AND
SCHEDULER KICKS REMAIN ON PI6.  THE 2026-10-05 ITS COMPARISON CORRECTED THE
EARLIER BLKO CONCLUSION: DEVICE 0130 DOES SUPPORT THE PDP-6 BLOCK-OUTPUT DATA-
CHANNEL MODEL, BUT BLKO MUST BE THE EVEN PI VECTOR INSTRUCTION.  THE FAILED
2026-10-04 EXPERIMENT ISSUED BLKO FROM INSIDE AN ORDINARY HANDLER AND DID NOT
REPRODUCE ITS CONTROL FLOW.  KINIT NOW INSTALLS `BLKO 0130,dpy_refresh_iowd`
IN LOW-CORE PI7 WORD 056 AND A DIRECT JSR TO THE DPY PACKAGE'S PRIVATE
STACKLESS COMPLETION ENTRY IN WORD 057.  BLKO COUNT OVERFLOW SELECTS THAT ODD
VECTOR; THE GENERIC PI7 DISPATCHER IS DELIBERATELY BYPASSED.  SLV IS MOVED TO
ORDINARY PI6 DISPATCH ONLY when DPY owns PI7.  NO-DPY SYSTEMS RETAIN THE
UNMODIFIED GENERIC PI7 VECTORS AND PAY NO DISPLAY RUNTIME COST.

THE RETAINED TEXT FORMAT WAS SIMPLIFIED WITH THE BLKO CONVERSION.  EVERY
INITIALIZED SIX-CELL BLOCK IS TWO DIRECT FIXED-PAIR TYPE-342 WORDS, SO THE
INITIALIZED PREFIX OF ONE ROW IS ONE CONTIGUOUS BLKO SPAN.  ONE CR/LF WORD IS
A SECOND SPAN; THE COMPLETED IOWD ADDRESS ITSELF IS THE ROW-END PHASE MARKER.
THIS REMOVES THE OLD PER-BLOCK SIMPLE/COMPLEX REFRESH DECODER, SHIFT-STATE
TRACKING, AND BLOCK CURSOR FROM THE INTERRUPT HOT PATH.  A FINITE FRAME ENDS
WITH ONE NONPRINTING SI TRAILER SO dpy_pending IS CLEARED ONLY AFTER THE LAST
VISIBLE WORD HAS ACTUALLY COMPLETED.

THE LINKED DPY MRES IS NOW 439 PERMANENT WORDS, DOWN FROM THE 534-WORD
PRE-BLKO CURRENT BASELINE, A 95-WORD PERMANENT SAVING.  THE 46-ROW LAZY TEXT EXTENT IS 1288 WORDS AND STILL SAVES 560 DYNAMIC WORDS
VERSUS THE FORMER 1848-WORD SIXBIT+COMPILED-CACHE DESIGN.  p10super FINDS NO FURTHER REORDERING
OR PEEPHOLE SAVINGS; p10fold FINDS ONLY FOUR-WORD RUNS REQUIRING EXTRA JRSTs,
SO NO AUTOMATIC DPY FOLD WAS APPLIED.

THE CURRENT 2026-10-06 MEASUREMENT AFTER THE PURE-TEXT PROCESS-SWAP
OPTIMIZATION IS KCORE 9691 WORDS, CLK MRES 45, DPY MRES 444, DTFS MRES 1827,
D6FS MRES 2091, MEMFS MRES 1363, KCORE+MRES 16980, PERMANENT_LAST 041203,
MULTI_ROOT 041165, AND MULTI_LAST 041244.  MULTI_LAST IS THEREFORE 676
ADDRESSES ABOVE THE 040000 LIMIT, SO THE FINAL OPTIMIZATION SWEEP REMAINS
ACTIVE.  A DIAGNOSTIC BUILD WITH PROC_SWAP_VERIFY_PURE=1 MEASURES KCORE 9793
WORDS AND MULTI_LAST 041412; THOSE EXTRA 102 KCORE WORDS ARE COMPILED OUT OF
NORMAL KERNELS.  THE PRE-SWAP APR BASELINE WAS KCORE 9487 AND MULTI_LAST
040730.  THE APR CORRECTNESS FIX ITSELF ADDS 20 WORDS TO CLK MRES VERSUS THE
IMMEDIATELY PRECEDING 25-WORD PACKAGE; ITS SIMULTANEOUS-CLOCK PATH SHARES THE
NORMAL TICK BODY RATHER THAN DUPLICATING IT.

THE 2026-10-07 FINAL SHRINK RUN IS ACTIVE.  P10FOLD NOW ACCEPTS RELOCATION-
SAFE TWO-WORD ZERO-COST RUNS IN KCORE, AND GE/TTY/DTFS/D6FS APPLY PACKAGE-
LOCAL POST-LINK FOLD PLANS AT THE SAME THRESHOLD.  THE PERMANENT-SIZE REPORTER
WAS ALSO FIXED TO READ THE LINKED MRES1 RESIDENT EXTENT RATHER THAN SUMMING
UNFOLDED INPUT DOBJS.  WITH THOSE CHANGES KCORE IS 9669 WORDS, D6FS 2083,
DTFS/TSFS 1823, MEMFS 1337, AND MULTI_LAST IS 041144: 612 ADDRESSES ABOVE
040000.  MEMFS REMAINS THE PRIMARY SOURCE-LEVEL SHRINK TARGET; THE RUN IS NOT
COMPLETE UNTIL MULTI_LAST IS BELOW 040000 AND THE ACCEPTANCE SUITE PASSES.

PDP-6 APR FAULT DELIVERY IS NOW PART OF THE PI6 CLK MRES.  APR CONI IS
QUALIFIED BEFORE CLOCK/SOFTWARE-KICK HANDLING FOR NON-EXISTENT MEMORY,
ILLEGAL/PROTECTED ADDRESS, AND PUSHDOWN-LIST OVERFLOW.  THE HANDLER CLEARS
THOSE HARDWARE CONDITIONS WHILE PRESERVING APR PIA6 AND THE 60-HZ CLOCK.
WHEN THE PI6 SAVED RETURN SHOWS USER MODE, THE HANDLER REDIRECTS THE NORMAL
PI DISMISSAL TO AN EXECUTIVE TRAMPOLINE, RESTORES THE PROCESS'S PRIVATE KERNEL
STACK, CLOSES ITS FILES, AND TERMINATES IT WITH THE EXISTING TERM-CLASS WAIT
STATUS.  THIS AVOIDS BOTH LEAVING PI6 HELD AND FREEING THE U-AREA WHILE STILL
EXECUTING ON ITS STACK.  THE SAME APR CONDITIONS INTERRUPTING EXECUTIVE MODE
ARE FAIL-STOP KERNEL FAULTS.  ARITHMETIC-OVERFLOW AND PC-CHANGE APR INTERRUPTS
REMAIN DISABLED.  SIMH IS CONFIRMED TO IMPLEMENT THE PDP-6 NXM, MEMPROT, AND
PUSHOVER APR FLAGS; DAIMOS-TESTKIT COVERS A REAL USER PROTECTION FAULT, NXM,
A REAL USER PUSH OVERFLOW, AND EXECUTES THE PRODUCTION CLK/APR MRES PATH ON
THE INDEPENDENT AAP PDP-6 EMULATOR.

THE EARLIER PERMANENT-SIZE REPORTER COUNTED ONLY MEMFS_RUNTIME FOR MEMFS EVEN
THOUGH THE INSTALLED PACKAGE ALSO CONTAINS MEMFS_DATA AND MEMFS_SNAPSHOT, AND
IT OMITTED BADMAP.  THE REPORTER NOW COUNTS THE COMPLETE INSTALLED MEMFS AND
BADMAP RESIDENT SET.  THE PREVIOUS 56-WORD-BELOW RESULT WAS THEREFORE AN
ACCOUNTING UNDERESTIMATE, NOT A REAL PERMANENT-IMAGE MARGIN.

THE FIRST PDP-6 MEMFS DATA/SNAPSHOT REWRITE IS COMPLETE.  MEMFS_DATA TEXT IS
477 WORDS INSTEAD OF KCC'S 796 AND MEMFS_SNAPSHOT IS 216 INSTEAD OF 343, A
COMBINED 446-WORD TEXT REDUCTION BEFORE LINK-TIME EFFECTS.  MEMFS REMAINS THE
PRIMARY SOURCE-LEVEL SHRINK TARGET.

THE COMMON BCACHE NOW REUSES THE PERMANENT 0200-WORD FS_BLOCK_WORKSPACE AS
CACHE SLOT 0 AND ALLOCATES ONLY ONE PRIVATE 0200-WORD DATA SLOT PLUS ITS TAG
IN MANAGED CORE.  THE PRIVATE SLAB IS 0201 OCTAL WORDS (129 DECIMAL), DOWN
FROM THE FORMER 0403-OCTAL (259-DECIMAL) PRIVATE TWO-BLOCK SLAB.  ONE FIXED
WORKSPACE TAG AND THE REQUIRED INVALIDATION/PRESERVATION CODE ADD 21 PERMANENT
WORDS, SO THE NET ACTIVE-CACHE SAVING IS 109 PHYSICAL RAM WORDS WHILE RETAINING
TWO CLEAN CACHE ENTRIES.

THE MM FIT SEARCH NOW ACCUMULATES TOTAL FREE SPACE WHILE IT WALKS THE ARENA
GAPS.  ON FAILURE THE EXISTING BASE RESULT SLOT RETURNS THAT TOTAL, ALLOWING
ALLOCATION AND COMPACTION TO DISTINGUISH TRUE NO-MEMORY FROM FRAGMENTATION
WITHOUT A SECOND mm_total_free() PASS.  THIS BOTH SHORTENS THE PRESSURE PATH
AND REMOVES 15 LINKED KCORE WORDS.  THE COLD proc_swap_service_one() AND
proc_swap_reclaim() ORCHESTRATION PATHS ARE ALSO PDP-6 ASSEMBLY LEAVES, SAVING
20 MORE KCORE WORDS WHILE LEAVING THE SWAP TRANSFER TRANSACTIONS IN C.

THE DEFERRED PDP-6 RUNTIME-MODULE MOVER NO LONGER HARD-CODES THE FIXED-BINDING
COUNT.  ITS 37-ENTRY BINDING TABLE NOW LIVES BESIDE THE ASSEMBLY MOVER AND THE
LOOP COUNT IS DERIVED FROM LABELS, PREVENTING NEW BINDINGS FROM SILENTLY
FALLING OUTSIDE THE RETARGET LOOP.

THE PDP-6 PROCESS U-AREA NOW OVERLAYS USER-PREEMPTION AND SLEEPING-EXECUTIVE
REGISTER IMAGES.  THOSE TWO SCHEDULER STATES ARE MUTUALLY EXCLUSIVE, AND A
SLEEPING SYSCALL'S USER-RETURN FRAME ALREADY LIVES ON THE STABLE PRIVATE
KERNEL STACK.  AC0..AC17 AND THE SAVED PI RETURN WORD ARE THEREFORE SHARED.
THIS REDUCES PROC_UAREA_WORDS FROM 0430 TO 0407, SAVING 17 PHYSICAL RAM WORDS
PER LIVE PROCESS WHILE RETAINING THE FULL 0316-WORD PRIVATE KERNEL STACK.
THE STACK WATERMARK SCANNER WAS ALSO FIXED TO COVER THE FULL 0316-WORD RANGE;
THE ACCEPTANCE WORKLOAD CURRENTLY MEASURES 0301 WORDS HIGH-WATER.

THE COLD MEMORY-PRESSURE proc_swap_victim() POLICY IS NOW IMPLEMENTED IN
proc_pdp6.s FOR THE PDP-6 BUILD.  THE ASSEMBLY SCAN KEEPS ITS SLOT, DESCRIPTOR,
BEST-SLOT, AND SCORE STATE IN AC10..AC16 INSTEAD OF KCC'S LARGE LOCAL FRAME,
WHILE PRESERVING THE C POLICY EXACTLY: SLEEP/STOP AGE AND WAIT BIAS, RUNNABLE
NICE/CPU BIAS, CURRENT/EXCLUDED SLOT FILTERS, U-AREA/MAPPING/TRANSITION FILTERS,
SWAP-BACKING PRESENCE, AND PHYSICAL-PIN REJECTION.  THE PDP-6 PATH ALSO FOLDS
THE ONE-USE vm_space_can_swap() WRAPPER INTO THE SELECTOR WHILE RETAINING THE
CANONICAL mm_is_pinned() CHECK.  proc.dobj FALLS 870->763 TEXT; THE ASSEMBLY
ROUTINE ADDS 86 WORDS TO proc_pdp6 AND REMOVING THE VM WRAPPER SAVES THE
REMAINDER, FOR 34 LINKED KCORE WORDS NET.  THE PORTABLE C FALLBACK REMAINS FOR
HOST/REFERENCE BUILDS; ITS proc_child_report() HELPER WAS RESTORED UNDER THE
NON-PDP6 EVENT GUARD AND COSTS ZERO TARGET WORDS.  HOST MM VICTIM-RANKING,
32K SWAP-WAIT, 32K SWAP-STRESS, MULTIPROCESS, REALTIME, AND RT-REVOKE
REGRESSIONS PASS.

PROCESS SWAP NOW USES THE EXISTING DXR2 PURE-TEXT FIELD AS AN IMMUTABILITY
CONTRACT RATHER THAN TRYING TO TRACK A HARDWARE DIRTY BIT WHICH PDP-6 DOES NOT
PROVIDE.  FOR PURE IMAGES, COMPLETE 0200-WORD SECTORS COVERED BY THE EXECUTABLE
TEXT PREFIX ARE OMITTED FROM BACKSTORE; THE TEXT/DATA BOUNDARY SECTOR AND ALL
WRITABLE DATA/HEAP/STACK STATE REMAIN IN THE SWAPPED SUFFIX.  SWAP-IN RELOADS
PLAIN TEXT DIRECTLY FROM THE EXECUTABLE.  COMPRESSED PURE DXR2 REUSES THE
NORMAL D6LZ36 VFS DECODER AGAINST THE ALREADY-VALIDATED IMMUTABLE BACKING, THEN
OVERLAYS THE SWAPPED SUFFIX.  NON-PURE IMAGES RETAIN WHOLE-IMAGE SWAP AND MAY
SELF-MODIFY.

WHILE A NONRESIDENT PURE PROCESS DEPENDS ON AN EXECUTABLE, WRITE/TRUNCATE/
UNLINK OF THAT VNODE AND UNMOUNT OF ITS FILESYSTEM ARE REJECTED.  THE CHECK
SCANS THE EXISTING PROCESS TABLE AND ADDS NO PER-VNODE RESIDENT TABLE.
PROC_SWAP_VERIFY_PURE=1 ADDS A DIAGNOSTIC-ONLY RUNTIME ASSERTION WHICH SAVES
THE COMPLETE IMAGE TEMPORARILY, RECONSTRUCTS THE DISCARDABLE TEXT, COMPARES IT
THROUGH THE SHARED 0200-WORD FILESYSTEM WORKSPACE, RESTORES THE IMAGE, AND
HALTS ON A PURE-CONTRACT VIOLATION.  NORMAL BUILDS COMPILE THAT VERIFIER OUT.
DLINK --PURE REMAINS THE FIRST STATIC CHECK AND REFUSES DEFINITE TEXT WRITES;
PURE ENABLEMENT IS THEREFORE PER EXECUTABLE, NOT A BLANKET USERLAND POLICY.

THE SWAP VERIFICATION PASS ALSO FIXED TWO LATENT LOW-LEVEL ERRORS: THE PDP-6
ASSEMBLY TRANSACTION NOW UPDATES proc_swap_blocks_used WITH ADDM ON BOTH
SWAP-OUT AND SWAP-IN, AND d6lz36_decode_vfs TREATS A NEGATIVE VFS READ RESULT
AS AN ERROR RATHER THAN AS INPUT.  THE DIRECT PDP-6 PURE TARGET COVERS RAW AND
COMPRESSED PURE RELOAD, NON-PURE FULL SWAP, BACKING/MOUNT PINS, AND THE
OPTIONAL VERIFIER ASSERTION.  THE HOST MM AGGREGATE, 32K SWAP-WAIT, AND 32K
SWAP-STRESS REGRESSIONS PASS.

COLD FILESYSTEM METADATA PATHS NOW SHARE THREE SMALL SEQUENCES WITHOUT ADDING
OVERHEAD TO NORMAL READ/WRITE I/O.  D6FS RESIZE GROW/SHRINK SHARE THE PACKED
EXTENT DECODER; D6FS TRUNCATE REUSES THE IDENTICAL CHMOD/CHOWN/UTIME FCB COMMIT
AND 035-WORD FRAME UNWIND; AND TSFS MOUNT USES ONE LOOP TO VALIDATE THE PACKED
FILE AND OPTIONAL EXTENT MEDIA RECORDS.  D6FS_PROVIDER FALLS 828->822 TEXT,
D6FS_RUNTIME 1179->1173, AND TSFS_RUNTIME 512->502.  LINKED D6FS MRES FALLS
2211->2199 AND DTFS MRES 1848->1838, SAVING 22 PERMANENT WORDS TOTAL.  THE
PRE-DSH FILESYSTEM CLOSURE, D6FS CONTEXT/MULTIMOUNT/SUPER-RECOVERY, TSFS SCAN,
MULTI-EXTENT, D6LZ, AND LIVE HEADLESS TSFS MOUNT REGRESSIONS PASS.

FILE'S HAND-WRITTEN CURRENT-CREDENTIAL HELPER NOW USES ONLY AC5..AC7 SCRATCH,
PRESERVING NORMAL SYSCALL ARGUMENTS IN AC2..AC4 ACROSS file_check_root().
ROOT-ONLY DTFS MOUNT, TTY ROUTING, STORAGECTL, DTC WRITE, AND RTCTL THEREFORE NO
LONGER SPILL AND RESTORE ARGUMENTS AROUND THE PRIVILEGE CHECK.  FILE_RUNTIME IS
UNCHANGED AT 890 TEXT WORDS; SYSCALL_DISPATCH_PDP6 FALLS FROM 825 TO 802 TEXT
WORDS AND LINKED KCORE FALLS BY THE SAME 23 WORDS.  MULTIUSER AUTH/CREDENTIAL,
RT-REQUIRED, TTY-ROUTING, LOGSTORE, STORAGE-RESERVATION, AND LIVE DTC FORMAT/
WRITE/REBOOT/MOUNT/READ REGRESSIONS PASS.

THE PDP-6 CONTEXT SWITCHER NOW SHARES THE AC0..AC17 COPY CORE BETWEEN USER
AND SLEEPING-EXECUTIVE CONTEXTS.  THE TWO U-AREA IMAGES ALREADY HAVE IDENTICAL
RELATIVE REGISTER SHAPES; USER SAVE/RESTORE USE THE U-AREA BASE AND KERNEL
SAVE/RESTORE USE `CTX_K_AC0` AS THE ADJUSTED BASE.  THIS REMOVES DUPLICATE HOT
MOVEM/MOVE SEQUENCES WITHOUT CHANGING THE U-AREA ABI OR SAVED REGISTER SET.
PROC_PDP6 DROPS BY 33 TEXT WORDS AND THE LINKED KCORE BY 33 WORDS.  THE
RESTORE PATH ALSO REUSES THE U-AREA POINTER ALREADY SUPPLIED BY THE SCHEDULER,
AVOIDING A SECOND SLOT-TO-U-AREA LOOKUP.  SLEEP, PROCESS-EVENT, MULTIPROCESS,
REALTIME, RT-REVOKE, JOB-PIPELINE, AND EXEC-REPLACE REGRESSIONS PASS.

PROC EVENT APPLICATION IS NOW HAND-WRITTEN PDP-6 ASSEMBLY IN proc_pdp6.s.
THE HOT VALIDATED EVENT PATH KEEPS SLOT/EVENT/DESCRIPTOR STATE IN AC10..AC12,
REUSES proc_finish_slot FOR FATAL REMOTE TEARDOWN AND proc_notify_parent FOR
STOP/CONT REPORTING, AND PERFORMS TSTP/CONT/ALRM PACKED-STATE TRANSITIONS
DIRECTLY.  THE C proc_child_report HELPER IS RETAINED ONLY INSIDE THE PORTABLE
NON-PDP6 FALLBACK AND EMITS NO TARGET CODE.  THIS REDUCED KCORE BY 55 WORDS
(10313 -> 10258) WHILE THE PROCESS-EVENT, JOB-PIPELINE,
MULTIPROCESS, REALTIME, RT-REVOKE, AND 60-HZ SLEEP REGRESSIONS ALL PASS.

TARGET-SIDE DPY ACCEPTANCE COVERS FIXED-PAIR CELL EDITS, SPARSE-PREFIX FILL,
ROW CLEAR/REUSE, UPPERCASE NORMALIZATION, SHIFTED PUNCTUATION, AND RING WRAP.
A HEADLESS TYPE-340 FULL BOOT WITH WCNSLS/OCNSLS DISABLED REACHES DSH, WRITES
BOTH PRIMARY TEXT AND `[\]^_` THROUGH /DEV/TTYDPY0, AND REMAINS LIVE ACROSS
MULTIPLE REFRESH PERIODS.  THE INTERRUPT-POLICY REGRESSION NOW REQUIRES THE
PI7 BLKO VECTOR AND DIRECT ODD-VECTOR COMPLETION PATH.

TARGET TESTING OF THE NEW BLOCK LOCATOR EXPOSED A PDP-6 DIVI PAIR-REGISTER
HAZARD: DIVI 2,6 CONSUMES AC2:AC3 AS A DOUBLE-WORD DIVIDEND.  THE FORMER CELL
HELPER HAD BEEN DIVIDING COLUMN,,CHARACTER STATE.  THE NATIVE BLOCK LOCATOR
NOW MOVES THE COLUMN INTO THE LOW DIVIDEND AC AND CLEARS THE HIGH AC BEFORE
DIVI; THE THREE-WAY COMPLEX PAIR SELECTOR AVOIDS DIVI ENTIRELY.  PACKED
TYPE-342 OCTAL WORD CONSTANTS ALSO REQUIRE THE LEADING `0` ASSEMBLER PREFIX;
WITHOUT IT DAS INTERPRETS THE DIGITS AS DECIMAL.

THE BANNER/TEXT REFRESH TRANSITION USES dpy_refresh_row=DPY_TEXT_ROWS AS A
NON-TEXT SENTINEL.  TEXT FRAMES USE -1 FOR THEIR SETUP SPAN.  THIS IS
REQUIRED BECAUSE THE FIRST LAZY TEXT ALLOCATION MAY COMPLETE WHILE A BOOT
BANNER FRAME IS STILL IN FLIGHT; WITHOUT THE SENTINEL THE FINAL BANNER DONE
INTERRUPT CAN BE MISREAD AS TEXT-SETUP COMPLETION, FEED NATIVE TEXT BLOCK
WORDS IN THE WRONG TYPE-340 MODE, STOP THE DISPLAY, AND LEAVE dpy_pending SET.

THE 2026-10-04 RUNTIME DEBUGGING ALSO FOUND TWO LOWER-LEVEL REFRESH FAILURES.
FIRST, PI6 MAY NEST INSIDE THE LOWEST-PRIORITY PI7 DISPLAY HANDLER.  THE PI6
SCHEDULER MUST NOT CONTEXT-SWITCH WHILE CONI PI REPORTS PI7 HELD (CONI HOLD
BIT 0400); OTHERWISE IT CAN SAVE THE NESTED PI7 FRAME AS AN ORDINARY EXECUTIVE
CONTINUATION AND RESUME USER MODE WITH PI7 STILL HELD.  SECOND, dpy_pending IS
FRAME OWNERSHIP, NOT MERELY ONE-WORD OWNERSHIP.  IT REMAINS SET FROM THE FIRST
DATAO THROUGH THE FINAL DONE OF A RETAINED FRAME.  CLEARING IT AFTER EACH DONE
CREATED A PI6 WINDOW IN WHICH THE CLOCK COULD RESET/START A SECOND FRAME WHILE
THE OLD PI7 HANDLER WAS STILL CHAINING THE PREVIOUS FRAME.

THE CLOCK/DISPLAY PATH NO LONGER REPLACES THE LIVE PI6 HANDLER. CLK REMAINS
THE REGISTERED PI6 OWNER AND EXPOSES A POST-SERVICE JRST WHICH DPY MINIT
PATCHES TO A STACKLESS DISPLAY HOOK. THIS AVOIDS AN EMPTY PI6 SPAN AND ALSO
AVOIDS EXTRA PUSHJ USE WHILE KINIT'S EXECUTIVE STACK IS ACTIVE. CLK SAVES THE
PI DISPATCHER'S AC2/AC3 IN TWO RESIDENT WORDS RATHER THAN ON AC17, SO A
SCHEDULER STACK CHANGE CANNOT RESTORE THE DISPATCH CURSOR FROM THE WRONG STACK.

THE SAME DEBUGGING EXPOSED AN INDEPENDENT SLOT-0 SCHEDULER AMBIGUITY. BEFORE
USERSPACE EXISTS, KINIT RUNS WITH proc_current_slot=0, WHICH IS ALSO THE VALUE
USED LATER BY THE PERMANENT EXECUTIVE IDLE LOOP. A PI6 TICK MUST NOT TREAT
BOOTSTRAP/KINIT SLOT 0 AS AN IDLE CONTEXT AND SWITCH AWAY FROM IT. THE PI6
SCHEDULER NOW ALLOWS SLOT-0 SWITCHING ONLY WHEN THE SAVED PI6 RETURN PC LIES
INSIDE proc_idle_loop..proc_idle_loop_end. THIS FIXES THE REPRODUCIBLE HANG
AFTER `DTC OK`, WHERE A BOOT-TIME PI6 SWITCH LEFT THE PI RETURN STATE
CORRUPTED. THE FIXED GUI BOOT PROCEEDS THROUGH MTC, DSK270, DRM236, BSET,
MEMFS, DTFS, SLV, D6FS, AND INIT V1; AFTER ROOT LOGIN/DSH THE TYPE-340
FRAMEBUFFER HAS 234 LIT PIXELS OVER BBOX (0,37)-(129,94) AND CPU PIH IS ZERO.

THE 2026-10-04 DTC BOOT HANG WAS A SEPARATE KINIT LIFETIME BUG EXPOSED BY THE
DISPLAY TIMING. module_run_minits() COPIES THE MINIT TABLE TO THE KINIT STACK
AND THEN RECLAIMS ITS SOURCE PREFIX. THE DTC/MTC/DSK SHARED ENTRY STUBS HAD
BEEN LINKED IMMEDIATELY BEFORE THAT TABLE, SO THE COPIED TABLE RETAINED ENTRY
ADDRESSES INTO RECLAIMED MEMORY. AFTER DTC INSTALLATION, MTC ENTERED AN
OVERWRITTEN STUB AND FELL INTO THE MONITOR-UUO PATH. THE THREE TYPE-136 STUBS
NOW LIVE WITH NON-RECLAIMED KINIT MODULE I/O CODE.

THE EARLIER EXPLICIT GUI DPY_LOGIN ACCEPTANCE BOOT SUSTAINED THE LOGIN
DISPLAY, ACCEPTED ROOT INPUT THROUGH CTY, EXECUTED DSH, AND REMAINED VISIBLE
AFTER MULTIPLE SECONDS OF REFRESH.  THAT REMAINS A VALID OPTIONAL ROUTING TEST,
BUT IT IS NO LONGER THE DEFAULT `make boot` CONFIGURATION; NORMAL GUI BOOT NOW
KEEPS LOGIN OUTPUT ON CTY SO RAW DPYVIEW TESTS HAVE AN UNAMBIGUOUS DISPLAY
OWNER.

LOADABLE/UNLOADABLE/RELOCATABLE MODULES AND USERSPACE OVERLAYS ARE NOT
IMPLEMENTED YET.

THE GENERAL KERNEL FEATURE/ACCEPTANCE WORK REQUIRED BEFORE USERLAND IS
COMPLETE.  THE PURE-EXECUTABLE-TEXT SWAP-OUT OPTIMIZATION IS COMPLETE.  THE
ONLY REMAINING PRE-DSH KERNEL WORK IS THE FINAL PERMANENT-SIZE/CORRECTNESS RUN
DESCRIBED IN TODO.MD.  AFTER THAT RUN IS GREEN, NORMAL DEVELOPMENT MOVES TO
DSH AND USERLAND; FURTHER KERNEL CHANGES SHOULD BE DRIVEN BY A REAL USERLAND
REQUIREMENT OR A REPRODUCIBLE DEFECT.

## 4. VFS AND NAMESPACE

VFS OWNS PATH WALKING, MOUNT CROSSING, FILESYSTEM DISPATCH, DESCRIPTORS,
CREDENTIAL/MODE CHECKING, AND COMMON FILE/FIFO OPERATIONS.  DAIMOS V0.9 UID
AND GID VALUES ARE NINE BITS (0..0777); UID 0 IS PRIVILEGED, EACH PROCESS HAS
ONE PRIMARY GID, AND THERE ARE NO SUPPLEMENTARY GROUPS.  OWNER/GROUP/OTHER RWX
SELECTION, ROOT BYPASS, OWNER-OR-ROOT CHMOD/UTIME, ROOT-ONLY CHOWN, AND THE
PROCESS UMASK ARE COMMON POLICY RATHER THAN FILESYSTEM-SPECIFIC CHECKS.  CHMOD,
CHOWN, AND UTIME SHARE ONE COMPACT VFS SETATTR PROVIDER SLOT.

D6FS PERSISTS MODE, UID/GID, AND MTIME PER NODE.  MEMFS STORES THE SAME
SEMANTICS IN ITS DYNAMIC METADATA AND VERSION-2 PERSISTENT SNAPSHOT.  THE
SUPPORTED DTFS BUILD PRESERVES NATIVE MEDIA UNCHANGED AND EXPOSES THE
MOUNTER'S UID/GID AS MOUNT OWNERSHIP; DORMANT TENEX/ITS PERSONALITY SOURCE
FOLLOWS THE SAME NO-CONVERSION RULE WHEN BUILT FOR DEVELOPMENT.  FILE-LEVEL
CHOWN AND MTIME ARE NOT REPRESENTABLE THERE.  TSFS
V1.1 STORES IMMUTABLE MODE12 AND UID9,GID9 IN ITS EXISTING EIGHT-WORD FILE
RECORD FROM MKTSFS MANIFEST DATA, WITH NO PER-FILE TIMESTAMP.  MONITORFS STAYS
SYNTHETIC/ROOT-OWNED.  DTFS, TSFS, AND MONITORFS REPORT MTIME 0; DAIMOS HAS NO
ATIME OR CTIME.

MONITORFS IS THE ONLY KERNEL-STATE FILESYSTEM:

    /MONITOR/PROCESSES
    /MONITOR/DEVICES
    /PROC                  ALIAS OF /MONITOR/PROCESSES
    /DEV/NAME              DEVICE I/O ENDPOINT VIEW

## 5. D6FS, BLOCKSET, AND BADMAP

D6FS IS THE NATIVE WRITABLE FILESYSTEM.  CURRENT V0.9 SUPPORT INCLUDES
MULTI-MOUNT, DSK/DRM BACKING, REMOUNT, CRASH-RECOVERY SELECTION, UID/GID/MODE,
MTIME, SEEK, CHOWN, RMDIR, A SMALL RECLAIMABLE WRITE-THROUGH BLOCK CACHE, SWAP
RESERVATION, LOGSTORE RESERVATION, AND COMMON BADMAP TRANSLATION.

D6FS TRUNCATE/SHRINK NOW PRESERVES THE RETAINED EXTENT COUNT WHILE CLEARING
DISCARDED EXTENT SLOTS, AND FILE-BLOCK LOOKUP RETURNS -1 AFTER THE FINAL
EXTENT INSTEAD OF FALLING THROUGH INTO THE NEXT ROUTINE.  THE TARGET
TRUNCATE-TO-ZERO THEN REGROW REGRESSION PASSES; THE PRE-FIX TREE FAILS THE
FIRST O_TRUNC REOPEN.

BLOCKSET PROVIDES GENERIC LOGICAL-TO-PHYSICAL MAPPING FOR MULTI-MEMBER SETS.
SINGLE-MEMBER ROOTS BYPASS ITS MRES.  SEMANTIC OWNERSHIP OF SWAP/LOGSTORE DOES
NOT LIVE IN BLOCKSET.

BADMAP HANDLING IS OPTIONAL.  MEDIA CAN BE CREATED WITHOUT A BADMAP, IN WHICH
CASE NO BADMAP PACKAGE IS INSTALLED FOR THAT MEDIA, AND BADMAP SUPPORT CAN ALSO
BE OMITTED FROM THE KERNEL BUILD WITH KINIT_BADMAP=0.

## 6. DSK270 AND DRM236 SCHEDULING

DSK270 USES ITS BOUNDED PER-UNIT CYLINDER ELEVATOR.  DRM236 USES A MINIMAL
ONE-PENDING FIFO.  DRM SCHEDULING MAKES NO ROTATIONAL OR TIMING ASSUMPTIONS,
BECAUSE THE CURRENT SIMULATOR DOES NOT PROVIDE USEFUL ROTATIONAL DATA.


## 7. TSFS

TSFS V1.1 IS A READ-ONLY DECTAPE FILESYSTEM.  `MOUNT.TSFS` PERFORMS TRANSIENT
MEMBER/TABLE/CHECKSUM/EXTENT VALIDATION AND PASSES A COMPACT HANDOFF TO THE
RESIDENT PROVIDER.  LOOKUP, READDIR, STAT, PARENT TRAVERSAL, MULTI-EXTENT AND
CROSS-MEMBER READS ARE IMPLEMENTED.  ITS EIGHT-WORD FILE RECORD NOW CARRIES
IMMUTABLE MODE12 AND UID9,GID9 WITHOUT GROWING THE RECORD; MKTSFS MANIFESTS
SUPPLY THESE ATTRIBUTES AND TSFSCHECK VALIDATES THE V1.1 FORMAT.  TSFS MTIME
REMAINS 0 BY DESIGN.

REGULAR DATA USES 0400-WORD RESTART EXTENTS.  EACH EXTENT IS D6LZ36 WHEN THAT
SAVES DECTAPE BLOCKS AND STORED OTHERWISE; METADATA REMAINS DIRECT.  TSFS MAY
ALSO SERVE AS THE BOOT ROOT.

## 8. LOGSTORE

LOGSTORE V1 IS COMPLETE.  D6FS PERSISTS THE RAW LOGICAL RESERVATION; KINIT
RECOVERS THE RING and publishes the compact runtime state.  THE RESIDENT MRES
PROVIDES STATUS, BOUNDED RAW I/O, COMMIT-LAST APPEND, AND RACE-SAFE WAIT/WAKE
WITHOUT A PERMANENT 128-WORD RECORD BUFFER.

THE ROOT-ONLY LOGCTL TRANSPORT SERVES `/SYSTEM/EXEC/LOGDRAIN`.  LOGDRAIN
IMPLEMENTS NULL, D6FS FILE/ROTATION, SEVERITY-FILTERED CONSOLE, AND MTC
ARCHIVAL SINKS WITH REDUNDANT PERSISTENT DRAIN STATE, LOSS/GAP ACCOUNTING,
EOT/REEL RESUME, AND FINITE-DRAIN FILEMARK.  INIT AND LOGIN PRODUCE NORMAL
OPERATIONAL RECORDS.

## 9. CLOCK AND REAL-TIME MODE

THE STANFORD PHIL PETIT PCLK AT DEVICE 0730 IS THE STANDARD WALL-CLOCK SOURCE.
GETTIME RETURNS UTC TIME36 FOR 2026-01-01 THROUGH 2125-12-31.  THE APR 60 HZ
LINE CLOCK REMAINS THE MONOTONIC SCHEDULER/TIMER CLOCK.

USERSPACE MAY BLOCK FOR A BOUNDED 17-BIT NUMBER OF APR TICKS THROUGH
SYS_EXT_SLEEP.  THE DEADLINE IS KEPT IN THE SLEEPING PROCESS DESCRIPTOR AND A
SINGLE GLOBAL FRONTIER WORD AVOIDS A PROCESS-TABLE SCAN ON ORDINARY CLOCK
TICKS.
THE 2026-10-05 TIMER COMPACTION PRESERVES THAT FRONTIER ALGORITHM AND THE FULL
17-BIT SLEEP RANGE WHILE REMOVING REDUNDANT CLOCK/DEADLINE RECONSTRUCTION.
PROC_PDP6 DROPPED BY 18 TEXT WORDS; THE LINKED KCORE CHECKPOINT IS 10328 WORDS
AND MULTI_LAST IS 041477, 832 WORD ADDRESSES ABOVE THE 040000 LIMIT.
A FOLLOWING MEMFS NAMESPACE PASS SHARES WRITABLE-PARENT/NAME/CHILD VALIDATION
ACROSS CREATE, UNLINK, AND RENAME.  MEMFS_RUNTIME DROPPED BY 23 TEXT WORDS AND
THE LINKED MEMFS MRES BY 24 WORDS, TO 688.  MULTI_LAST IS NOW 041447, 808 WORD
ADDRESSES ABOVE 040000.
THE FILE BULK-WORD I/O PASS THEN SHARED READ/WRITE DESCRIPTOR VALIDATION AND
RESULT UNWIND WITHOUT ADDING A SUBROUTINE CALL TO THE HOT DATA PATH.
FILE_RUNTIME DROPPED BY 15 TEXT WORDS; KCORE IS NOW 10313 WORDS AND MULTI_LAST
IS 041430, 793 WORD ADDRESSES ABOVE 040000.
D6FS NOW RELIES EXCLUSIVELY ON BCACHE FOR RUNTIME BLOCK IDENTITY.  THE FORMER
PRIVATE READER CACHE TAG HAD BECOME WRITE-ONLY; REMOVING ITS DEAD LOAD/COMMIT
STORES CUTS THE LINKED D6FS MRES BY 6 WORDS, TO 2211.  MULTI_LAST IS 041422,
787 WORD ADDRESSES ABOVE 040000.

THE MINIMAL SINGLE-OWNER REAL-TIME USER PROCESS MODE AND RT_REQUIRED EXEC
ADMISSION ARE IMPLEMENTED.  ACQUIRING RT MODE OR LAUNCHING AN RT_REQUIRED
IMAGE REQUIRES UID 0; DISABLE/YIELD BY THE CURRENT OWNER REMAIN NORMAL OWNER
OPERATIONS.

MULTI-USER ADMINISTRATIVE BOUNDARIES ARE ENFORCED IN THE KERNEL: HALT,
FILESYSTEM MOUNT/UNMOUNT, AND RT ACQUISITION ARE ROOT-ONLY; PROCESS EVENT
DELIVERY REQUIRES THE SAME UID UNLESS THE CALLER IS UID 0; AND ORDINARY USERS
MAY LOWER BUT NOT RAISE THEIR NORMAL SCHEDULER PRIORITY.

THE HIGHEST TWO RUNTIME PROCESS SLOTS ARE RESERVED FOR UID 0.  ORDINARY RUN
ALLOCATION STOPS BEFORE THAT TAIL, WHILE ROOT/INIT MAY USE THE COMPLETE TABLE.
THIS PROVIDES ADMINISTRATIVE HEADROOM WITHOUT A PER-UID QUOTA TABLE OR
ADDITIONAL PER-PROCESS RESIDENT STATE.

## 10. NORMAL USERSPACE

NORMAL BOOT IS KINIT -> `/SYSTEM/INIT` -> LOGIN -> CONFIGURED SHELL.  INIT
READS `/CONFIG/INITTAB`; LOGIN USES `/CONFIG/PASSWD`, ESTABLISHES CREDENTIALS,
SESSION/PROCESS-GROUP/TTY STATE, HOME/ENVIRONMENT, AND EXECUTES THE CONFIGURED
SHELL.

RUN/EXEC USE THE BOUNDED COUNTED-SIXBIT STARTUP ABI.  NORMAL USERSPACE BUILDS
WITH KCC/DAS.

DSH HAS BEEN PORTED ONTO THE CURRENT RUN/EXEC/PROCESS-GROUP ABI, INCLUDING THE
FIXED-ARENA PARSER, FUNCTIONS, SCRIPT/-C EXECUTION, REDIRECTION, CONCURRENT
PIPELINES, BACKGROUND JOBS, AND JOB CONTROL.  CURRENT USERLAND PORT WORK ALSO
RESTORES THE DAIMOSV1 CORE-UTILITY SET USING NATIVE WORD/S6REC INTERFACES.
NATIVE MAKE IS NOW PRESENT AT `/SYSTEM/EXEC/MAKE`: IT USES REAL VFS MTIME36
DEPENDENCY DATA, A BOUNDED RULE/DEPENDENCY/VARIABLE ARENA, NATIVE S6REC
MAKEFILES, AND DSH FOR RECIPE EXECUTION.  TARGET ACCEPTANCE COVERS INITIAL
BUILD, OUTPUT, UP-TO-DATE SUPPRESSION, AND REBUILD SELECTION AFTER TOUCHING A
PREREQUISITE.

THE DAIMOSV1 `DU` COMMAND HAS BEEN PORTED TO THE CURRENT VFS AS A BOUNDED
12-LEVEL WORD-COUNT WALKER.  DAIMOSV1 `PACK` IS NOT PORTED: D6FS V2 REMOVED
THE ONLINE PACKER AND `TOOLS/HOST/PACKFS` IS THE AUTHORITATIVE OFFLINE
COMPACTOR.  DAIMOSV1 `LOGARC` IS ALSO RETIRED WITH ITS OLD SYSLOG ARCHIVE ABI;
CURRENT LOGSTORE USES THE CRASH-PERSISTENT `LOGDRAIN MTC` CURSOR/SINK PATH,
WHICH ADVANCES DRAIN STATE ONLY AFTER A COMPLETE TAPE RECORD IS ACCEPTED.  THE
OLD VOLUME/FILE CATALOG AND SEGMENT FORMAT ARE NOT PART OF CURRENT LOGSTORE.

NATIVE `/OPTION/BASE/EXEC/DARC`, `DLINK`, AND `OBJDUMP` ARE BUILT FROM
`USERLAND/TOOLCHAIN`.  THEIR HOST COUNTERPARTS AND THE OTHER CROSS-DEVELOPMENT
TOOLS LIVE IN `TOOLS/HOST`.  DARC STREAMS DOBJ1 MEMBERS INTO DARC1 OBJECT LIBRARIES;
DLINK IS A FIXED-METADATA, MULTI-PASS STREAMING DOBJ/DARC LINKER AND SUPPORTS
THE USERLAND SELF-HOSTING OPTIONS INCLUDING BASE ADDRESS, MAP OUTPUT, PURE/RT
FLAGS, AND DAIMOS UUO RELAXATION; OBJDUMP INSPECTS DOBJ HEADERS, SYMBOLS,
RELOCATIONS, AND TEXT.  TARGET ACCEPTANCE CREATES A MULTI-MEMBER DARC LIBRARY,
LINKS A NEW DXR AT BASE 020 THROUGH THAT LIBRARY, AND EXECUTES THE RESULT.
THE FORMER `PDP10-TOOLS` AND `DAIMOS-TOOLS` SOURCE TREES ARE NOW INTEGRATED:
`TOOLS/HOST` IS THE SINGLE HOST-SIDE SOURCE TREE AND BUILDS OUT OF TREE UNDER
`BUILD/TOOLS/HOST`, WHILE NATIVE DARC/DLINK/OBJDUMP ARE ORDINARY USERLAND
PROGRAMS.  NATIVE DOBJ SYMBOL SCANS ARE SEQUENTIAL, AND DARC PRE-SIZES ITS
EXACT KNOWN OUTPUT AND USES A 16-WORD INDEX BUFFER, AVOIDING REPEATED D6FS
FILE GROWTH DURING ARCHIVE CREATION.  THE NATIVE LINK-AND-EXECUTE ACCEPTANCE
PASSES WITH THE NORMAL 15-SECOND COMMAND LIMIT.
HOST-ONLY DLINK FOLD/MRES/ABSOLUTE-IMPORT IMAGE-CONSTRUCTION OPTIONS REMAIN
HOST-SIDE BECAUSE THEY ARE NOT REQUIRED BY NATIVE USERLAND SELF-HOSTING.
ORDINARY NON-PURE DXR IMAGES MAY NOW CONTAIN UP TO 0300000 WORDS OF STORED
TEXT+DATA, MATCHING THE 96K-CLASS SELF-HOSTING PROFILE.  THIS IS DISTINCT FROM
THE COMPACT PROCESS-SWAP PURE-TEXT FIELD, WHICH REMAINS 037777 WORDS; AN
OVERSIZED PURE TEXT PREFIX IS REJECTED RATHER THAN GROWING THE ONE-WORD SWAP
RECORD.  NATIVE DLINK STREAMS LARGE OUTPUT AND SIZES ONLY ITS RELOCATION
BITMAP FOR THE 0300000-WORD CEILING.  THE EXEC/PROC_SWAP OBJECT SIZES ARE
UNCHANGED BY THIS LOADER-POLICY EXTENSION.

THE FINAL DAIMOSV1 SOURCE AUDIT FOUND NO REMAINING ACTIVE IMPLEMENTATION
DEPENDENCY.  DMAKE/DSED ARE SUPERSEDED BY MAKE/SED; MINIDIAG IS COVERED BY
THE NORMAL PS/FREE/STAT/LOGSTORE DIAGNOSTICS; PIDCHECK/TTYCHECK WERE TEST
HELPERS; CONFIGFS/PROCFS/STATUSFS/DEVICEFS/MOUNTFS WERE REPLACED BY THE
CURRENT /CONFIG + MONITORFS/VFS DESIGN; AND THE STANDALONE TYPE-136 LAYER WAS
REPLACED BY THE CURRENT SHARED STORAGE ROUTER.  DAIMOSV1 IS THEREFORE RETIRED
AS AN ACTIVE DEVELOPMENT REFERENCE.  ITS ARCHIVE MAY BE KEPT AS HISTORICAL
SOURCE ONLY; EXPLICITLY DEFERRED FEATURES SUCH AS THE SECOND JOYSTICK/SWITCH
INTERFACE REMAIN TRACKED IN THE CURRENT TODO INSTEAD OF DEPENDING ON V1.

THE CURRENT DSH CONFORMANCE BASELINE IS 106/106 TARGET SIMH PROBES.  IT COVERS
THE FROZEN QUOTING/COMMENT RULES, CASE/GLOB, VARIABLES AND STATUS, SCRIPT/-C,
SOURCE FRAME RESTORATION, FUNCTIONS/COMPOUND CONTROL FLOW, STATUS 126/127,
REDIRECTION, PIPELINE DATA/STATUS, BOUNDED FUNCTION LIMITS, AND THE FOUR-SLOT
JOB TABLE INCLUDING STABLE `%N`, DONE RECLAIM, STOPPED BACKGROUND TTY READS,
WAIT/BG RE-STOP, AND EXHAUSTION.  LOGIN `-L` NOW LOADS THE SYSTEM PROFILE
BEFORE THE HOME PROFILE, IGNORES MISSING PROFILES, DIAGNOSES PROFILE ERRORS
WITHOUT ABORTING THE LOGIN SHELL, AND PRESERVES A NONZERO STATUS; NON-LOGIN
SHELLS LOAD NO PROFILE IMPLICITLY.  PIPELINES CREATE ONE PIPE AT A TIME AND
CLOSE PARENT ENDS AS SOON AS EACH STAGE IS LAUNCHED, SO THE BOUNDED EIGHT-STAGE
MAXIMUM FITS THE 16-DESCRIPTOR PROCESS ABI. THE HISTORICAL DSH PIPELINE
ACCEPTANCE PEAK USED 21 MM EXTENT DESCRIPTORS. NESTED NATIVE
DSH -> MAKE -> KCC -> KCPP EXHAUSTS THAT CAPACITY: KCPP'S IMAGE LOADS,
BUT THE SUBSEQUENT 0410-WORD CHILD U-AREA ALLOCATION FAILS WITH
MM_ERR_DESCRIPTORS (-3), EVEN THOUGH CONTIGUOUS CORE REMAINS FREE.
THE TABLE NOW HOLDS 32 DESCRIPTORS, COSTING 22 ADDITIONAL RESIDENT WORDS.
A SMALL DIRECT NATIVE KCC COMPILE PASSES WITH THAT CAPACITY. A SEPARATE
NESTED MAKE -> KCC TEST REACHES THE COMPILER BUT HALTS AT PC 000000;
THIS SECOND REGRESSION REMAINS OPEN, AND FULL NATIVE MAKE ALL IS NOT YET
ACCEPTED. A SEPARATE REAL-DCS FOREGROUND ACCEPTANCE PASSES:
DSH SAVES A STOPPED JOB'S RAW/COOKED TTY MODE, REFRESHES IT AFTER A SECOND
STOP, RESTORES IT BEFORE FG/CONTINUE, RECLAIMS FOREGROUND OWNERSHIP, AND
RESTORES SHELL RAW MODE.  MODE CHANGES OCCUR BEFORE FOREGROUND PGRP HANDOFF,
AS REQUIRED BY THE KERNEL'S FOREGROUND-OWNER TTY_SETMODE RULE.  THE SAME
REAL-DCS ACCEPTANCE NOW COVERS A TWO-PROCESS FOREGROUND PIPELINE STOPPED AND
RESUMED AS ONE PGRP, PLUS DEFAULT `FG` SELECTING THE MOST RECENT OF MULTIPLE
LIVE STOPPED JOBS.  WHILE DSH ITSELF OWNS THE RAW/NO-ECHO EDITOR TTY, CTRL-C
NOW CANCELS THE COMPLETE PARTIAL LOGICAL INPUT AND RETURNS A FRESH PROMPT;
FOREGROUND CHILDREN STILL RUN UNDER THE KERNEL'S COOKED/SIGNAL PATH, SO THEIR
CTRL-C PROCESS-GROUP SEMANTICS ARE UNCHANGED.
NONINTERACTIVE DSH STDIN NOW CONSUMES NATIVE S6REC TEXT RATHER THAN CHARACTER
I/O, SO `DSH < SCRIPT` WORKS ON DAIMOS FILES/PIPES.  EXPLICIT
`BEGIN ... END &` PRESERVES A GROUP AST NODE AND LAUNCHES ONE NONINTERACTIVE
DSH WORKER IN A NEW PGRP; THE PARENT SERIALIZES THE BOUNDED AST INTO NATIVE
S6REC RECORDS OVER A PIPE.  THIS USES THE EXISTING DSH LANGUAGE/RUN ABI, ADDS
NO PERMANENT COMMAND BUFFER OR PRIVATE WORKER PROTOCOL, INHERITS EXPORTED
ENVIRONMENT ONLY, AND KEEPS CHILD SHELL-STATE CHANGES OUT OF THE PARENT.

THE PUBLIC COMMAND NAMESPACE NOW CONTAINS ORDINARY EXECUTABLE FILES.  `LS`,
`DF`, `MAN`, `WHICH`, `LOGD`, AND THE OTHER PUBLIC COMMANDS ARE INDEPENDENT
DXR PROGRAMS UNDER `/SYSTEM/EXEC`; THERE IS NO RUNTIME COMMAND MAP AND NO
PUBLIC-COMMAND MULTIPLEXER.  COMMON IMPLEMENTATION CODE IS A LINK-TIME DETAIL:
THE BUILD PLACES ONE COMMAND BACKEND PER MEMBER IN STATIC DARC ARCHIVES, SO
DLINK EXTRACTS ONLY THE SELECTED COMMAND AND THE COMMON LIBRARY MEMBERS IT
ACTUALLY REFERENCES.  THIS KEEPS SMALL COMMANDS SMALL (FOR EXAMPLE `TRUE` IS
ONLY 000300 WORDS) WHILE PRESERVING A NORMAL EXECUTABLE NAMESPACE.  THE BOOT
IMAGES STORE THE INDIVIDUAL DXRS IN D6LZ FORM; THE TSFS ROOT IMAGE USES THE
SAME STANDALONE PROGRAM MODEL.  DSH, XARGS, AND `WHICH` NOW USE ORDINARY
PATH/FILESYSTEM LOOKUP ONLY.  `/SYSTEM/LIBEXEC` IS RESERVED FOR GENUINELY
PRIVATE HELPERS SUCH AS `DSHCOMP`.  A FUTURE SHARED-LIBRARY ABI MAY REPLACE
THE STATIC ARCHIVES WITHOUT CHANGING THIS COMMAND NAMESPACE.

THE OPTIONAL HANDWRITTEN ASSEMBLER UTILITIES NOW SHIP AS NATIVE-BUILDABLE
SOURCE UNDER `/OPTION/BASE/SOURCE/ASMUTILS`.  THERE IS ONE CANONICAL HOST
`Makefile`; THE BOOT BUILD DERIVES THE NATIVE S6REC `MAKEFILE` BY CHANGING A
RECIPE-LEADING TAB TO `>`, NORMALIZING TO UPPERCASE/SIXBIT-SAFE TEXT, AND
S6REC-ENCODING THE RESULT.  NO PARALLEL NATIVE-ONLY MAKEFILE IS MAINTAINED.
THE SAME FILE USES `DASFLAGS = -B`: HOST DAS AUTO-DETECTS ASCII VERSUS S6REC
SOURCE, WHILE RESIDENT DAS READS THE NATIVE S6REC REPRESENTATION.  -A/-S ARE
ONLY EXPLICIT DAS COMPATIBILITY/DIAGNOSTIC OVERRIDES.  NATIVE `MAKE -C
/OPTION/BASE/SOURCE/ASMUTILS INSTALL` BUILDS SEPARATE DXR PROGRAMS FOR THE
ASSEMBLER UTILITIES AND INSTALLS THEM UNDER `/OPTION/BASE/EXEC`.  DXR IS THE
EXECUTABLE FORMAT RATHER THAN A REQUIRED FILE-NAME SUFFIX; ASMUTILS NOW BUILDS
THE FINAL EXTENSIONLESS COMMAND NAMES THROUGH MAKE'S CLASSIC SINGLE-SUFFIX
`.S:` RULE.  IMAGE-SEEDED SOURCE FILES WITH MTIME 0 ARE TREATED AS UNDATED
PREREQUISITES: THEY BUILD A MISSING/UNDATED TARGET ONCE BUT DO NOT FORCE A
VALID-TIMESTAMP TARGET TO REBUILD FOREVER.  INVALID NONZERO MTIMES REMAIN
CONSERVATIVE, AND `MAKE -B` STILL FORCES REBUILD.  NATIVE PDP-6 ACCEPTANCE
CONFIRMS `ARGS.S` WITH MTIME 0 BUILDS `ARGS` ONCE, AN IMMEDIATE NORMAL SECOND
MAKE IS QUIET, AND `-B -N` SELECTS THE DAS RECIPE AGAIN.

THIS NATIVE MAKE PATH EXPOSED AND FIXED A KCC PDP-6 SIMULATED-ADJBP BUG.  A
PEEPHOLE-FOLDED BYTE POINTER WITH AN INDEX REGISTER WAS PREVIOUSLY HANDED TO
`%ADJBPH` WITHOUT FIRST MATERIALIZING ITS EFFECTIVE WORD ADDRESS.  LOCAL
CHAR-ARRAY WRITES COULD THEREFORE DISAGREE WITH THE POINTER PRODUCED BY ARRAY
DECAY; MAKE'S `$(DAS)` VARIABLE NAME WAS A CONCRETE FAILURE.  KCC NOW
MATERIALIZES THE EFFECTIVE ADDRESS AND THEN APPLIES THE BYTE-POINTER BITS.
THE MINIMIZED PDP-6 TARGET REGRESSION PASSES WITH THE FIX.

SIXMD IS THE AUTHORITATIVE PUBLIC COMMAND DOCUMENTATION SOURCE.  `HELP NAME`
RENDERS COMPACT COMMAND HELP FROM THE NAME/SUMMARY, USAGE, OPTIONS, AND
OPERANDS; `MAN NAME` RENDERS THE COMPLETE MANUAL; `APROPOS WORD` SEARCHES THE
ONE-LINE COMMAND SUMMARY INDEX.  DAIMOS DOES NOT RESERVE A PER-COMMAND `-H`
HELP OPTION.  MISSING OR CORRUPT HELP/MANUAL METADATA FAILS CLEANLY WITH
`HELP NOT AVAILABLE` OR `MANUAL NOT AVAILABLE` AND NEVER PREVENTS THE COMMAND
ITSELF FROM RUNNING.  NORMAL USERLAND BUILDS VALIDATE THAT EVERY PUBLIC
COMMAND HAS A SIXMD MANUAL; THE OPTIONAL ASSEMBLER-UTILITY PACKAGE ENFORCES
THE SAME RULE FOR ALL OF ITS PUBLIC PROGRAMS.

INTERACTIVE DSH NOW HAS A SMALL BOUNDED RAW-TTY EDITOR WITH CURSOR INSERTION,
BOUNDED HISTORY RECALL, CTRL-C LINE CANCELLATION, AND GENERIC COMMAND/PATH TAB
COMPLETION.  COMPLETION POLICY RUNS IN THE PRIVATE `/SYSTEM/LIBEXEC/DSHCOMP`
HELPER SO DIRECTORY/COMMAND-INDEX SCANNING DOES NOT BECOME PERMANENT SHELL
STATE.  REAL-DCS ACCEPTANCE COVERS CTRL-B INSERTION, CTRL-P HISTORY, COMMAND
AND PATH COMPLETION, CTRL-C RECOVERY, AND FOREGROUND JOB TTY RESTORATION.  THE
DSH VARIABLE TABLE NOW HAS 64 FIXED SLOTS INSTEAD OF 16.  EACH SLOT IS 13
WORDS: ONE PACKED METADATA WORD PLUS SIX SIXBIT NAME WORDS AND SIX SIXBIT
VALUE WORDS, PRESERVING THE 36-CHARACTER NAME/VALUE LIMIT WHILE REMOVING THE
SEPARATE USED/EXPORTED/LENGTH WORDS.  THE HOT SET/GET/UNSET/ENUMERATION PATH
IS SMALL HAND-WRITTEN PDP-6 CODE; EXTERNAL-LAUNCH ENUMERATION USES STATIC
SCRATCH SO THE DEEP SOURCE/POSITIONAL PATH DOES NOT LOSE USER-STACK MARGIN.
THE FINAL DSH DXR IMAGE IS 035551 WORDS WITH 007261 WORDS OF BSS, BELOW DSH'S
RETAINED 036000-WORD BUILD ACCEPTANCE CEILING AND SMALLER IN IMAGE TEXT THAN
THE FORMER 035773 BUILD.  P10FOLD IS APPLIED ONLY TO DSH'S HAND-WRITTEN
ASSEMBLER OBJECTS;
FOLDING KCC-GENERATED DSH OBJECTS WAS FOUND TO CORRUPT FOREGROUND EXTERNAL
COMMAND EXECUTION AND IS EXCLUDED.  THE FOLD PLAN DEPENDS ON THE USERLAND
MAKEFILE SO A CHANGE OF FOLD SCOPE CANNOT LEAVE A STALE PLAN IN AN INCREMENTAL
BUILD.  A SECOND LOGIN FAILURE WAS TRACED TO A STALE KCC INSTALLED UNDER THE
DEFAULT `$(HOME)/cross` PREFIX: THAT COMPILER PRODUCED A 036200-WORD DSH,
WHICH DSH'S 036000-WORD BUILD GUARD REJECTS.  THE CURRENT KCC INSTALLED INTO
THAT PREFIX PRODUCES THE VERIFIED 035551-WORD IMAGE.  THE PDP-6 BOOT BUILD
NOW DEPENDS EXPLICITLY ON `dsh_var_pdp6.s` AND USES DXRCHECK TO REJECT AN
OVERSIZED DSH BEFORE IT CAN BE COMPRESSED OR INSTALLED INTO D6FS.  THE 106/106
TARGET SUITE, REAL-DCS FOREGROUND/EDITOR
ACCEPTANCE, AND A DIRECT NORMAL-BOOT ROOT-LOGIN/EXTERNAL-COMMAND SMOKE TEST
PASS WITH THIS REPRESENTATION.

THE INTERACTIVE DSH EDITOR NOW NORMALIZES ASCII LOWERCASE TO UPPERCASE BEFORE
APPLYING THE NATIVE SIXBIT PRINTABLE-RANGE CHECK.  THE PREVIOUS ORDER MADE THE
LOWERCASE CONVERSION UNREACHABLE, SO ORDINARY LOWERCASE INPUT WAS SILENTLY
DISCARDED WHILE ENTER AND CTRL-D STILL WORKED.  THE NORMAL PDP-6 SIMH CONFIG
ALSO SELECTS `SET GE ASCII`; SIMS HAS SUPPORTED THIS TRANSPARENT GE INPUT MODE
SINCE COMMIT 0f0d530, BUT DAIMOS HAD LEFT GE IN THE HISTORICAL LEGACY
TRANSLATION MODE, WHERE ASCII `root` ARRIVED AS `2//4`.  MULTITTY ACCEPTANCE
NOW SENDS LOWERCASE SHELL COMMANDS THROUGH CTY, DCS0, AND GE0 AND PASSES ON
ALL THREE PATHS.

DAS REMAINS AN EXTERNAL SOURCE TREE.  THE PDP-6 BOOT BUILD CONSUMES ITS
RESIDENT DAS/DAS1/DAS2 ARTIFACTS AND AUTHORITATIVE SIXMD MANUAL.  NORMAL
`MAKE BOOT` REGENERATES THE FOUR-MEMBER HDD ROOT SET AND THE SEPARATE AUXILIARY
DRUM MEDIA; IT DOES NOT DUPLICATE THE ROOT FILESYSTEM ON THE DRUM.
THE PUBLIC COMMAND LIVES AT `/OPTION/BASE/EXEC/DAS`; ITS PRIVATE DAS1/DAS2
PHASES LIVE UNDER `/OPTION/BASE/LIBEXEC/DAS` AND ARE INVOKED ONLY BY ABSOLUTE
PATH, SO THEY DO NOT POLLUTE THE COMMAND NAMESPACE.
THE PUBLIC DAS DRIVER AND BOTH PRIVATE PHASES ARE KCC-BUILT.  THE
PEEPHOLE/FOLDING OPTIMIZER IS HOST ONLY AND IS COMPILED OUT OF DAS_NATIVE TO
SAVE RESIDENT CODE AND STATE.  STRUCTURAL PHASE-LIFETIME/PARSER/RUNTIME
SHRINKAGE PLUS KCC SHARED FIXED-FRAME RETURN EPILOGUES PUT DAS1 AT 033244
WORDS (BSS 002032, PROCESS 037276) AND DAS2 AT 027501 WORDS (BSS 002342,
PROCESS 034043), SO BOTH PASS THE 034000-WORD IMAGE
BUDGET AND 040000-WORD PROCESS BUDGET WITH THE STANDARD 02000-WORD STACK.  THE
TRANSITIONAL PDP-10 GCC PHASE BUILD HAS BEEN REMOVED.  NATIVE DAS
ASSEMBLE/VERIFY/EXECUTE ACCEPTANCE PASSES.

THE DAS BRING-UP EXPOSED AND FIXED A KCC CORRECTNESS BUG IN LOOP INDUCTION:
A DERIVED LOCAL-ARRAY POINTER COULD BE SPILLED ON ONLY ONE ARM OF A GENERAL IF
AND RESTORED AT THE JOIN, CORRUPTING THE USER STACK.  KCC NOW RESTRICTS THAT
STRENGTH REDUCTION TO BRANCH-SAFE LOOP BODIES; THE PDP-6/KA10/KI10/KS10
SEMANTIC OPTIMIZER REGRESSION AND THE TARGET DAS REGRESSION PASS.

THE KCC-ONLY DAS2 TRANSITION ALSO EXPOSED AND FIXED AN OBJECT-OPTIMIZER BUG:
NEGATION COULD BE PUSHED BACKWARD THROUGH P_UIDIV, WHICH IS INVALID FOR
UNSIGNED QUOTIENT/REMAINDER.  DAS'S PACKED OPCODE LOOKUP THEN SELECTED JUMPN
0326 INSTEAD OF MOVEI 0201 FOR MNEMONIC INDEX 319.  P_UIDIV NOW BLOCKS THAT
NEGATION TRANSFORM; THE MINIMIZED REGRESSION PASSES ON PDP-6, KA10, KI10, AND
KS10, AND THE FULL TARGET DAS REGRESSION PASSES.

THE BUILD GRAPH NOW TREATS THE SELECTED PDP-10 C COMPILER EXECUTABLE AS AN
INPUT.  REPLACING KCC INVALIDATES KINIT/KCORE BOOT STAMPS, KERNEL C-GENERATED
ASSEMBLY, USERLAND C OBJECTS, AND THE PRIVATE NATIVE DAS PHASE ASSEMBLY.
HANDWRITTEN ASSEMBLER INPUTS DO NOT REBUILD MERELY BECAUSE KCC CHANGED.  THIS

DAIMOS NOW PROVIDES A REAL USER BREAK FACILITY FOR SELF-HOSTING USERLAND.
UUO-077 EXTENSION 052 IMPLEMENTS BRK; LIBC IMPLEMENTS SBRK, MALLOC, CALLOC,
REALLOC, AND FREE ON TOP OF IT.  PDP-6 PROCESS EXTENTS GROW IN 02000-WORD APR
QUANTA; HIGH-PLACED PROCESSES CAN REBASE DOWNWARD WITH AN OVERLAP-SAFE COPY SO
GROWTH DOES NOT NORMALLY REQUIRE TWO COMPLETE PROCESS IMAGES IN CORE.  THE
EXACT FLOOR/CURRENT BREAK PAIR COSTS ONE WORD IN THE STABLE U-AREA AND DOES NOT
GROW THE THREE-WORD RESIDENT PROCESS DESCRIPTOR.  RUN AND BOTH EXEC REPLACEMENT
PATHS INITIALIZE/RESET THE BREAK.  A TARGET SIMH TEST HAS GROWN THROUGH AN APR
BOUNDARY, VERIFIED 9000 ALLOCATED C CHARACTERS, SHRUNK BACK TO THE INITIAL
BREAK, AND EXERCISED SBRK SUCCESSFULLY.
PREVENTS STALE PREVIOUS-COMPILER OUTPUT FROM SURVIVING AN INCREMENTAL BUILD.

## 12. DEFERRED V0.9 NON-GATES

THE FOLLOWING ARE EXPLICITLY NOT PRE-USERLAND V0.9 GATE ITEMS:

- RECOVERY/SINGLE-USER DECTAPE ENVIRONMENT;
- GENERAL USERSPACE OVERLAYS UNTIL REAL WORKING SETS ARE MEASURED;
- LOADABLE/UNLOADABLE/RELOCATABLE RUNTIME MODULES UNTIL AFTER OVERLAYS.


## KPARSE NATIVE COMPILATION INVESTIGATION — 2026-10-10 V1

STATUS: OPEN.  THIS SECTION RECORDS THE REPRODUCED DEBUGGING EVIDENCE, NOT
A CLAIM THAT NATIVE KPARSE NOW COMPLETES OR THAT ITS RUNTIME IMPROVED.

- THE INSTALLED KPARSE.DXR WAS REBUILT BYTE-FOR-BYTE FROM THE CURRENT
  KCC SOURCE TO OBTAIN A TRUSTWORTHY KPARSE.MAP.  THE REPEATED USER PC
  133306-133324 MAPS TO FINDPTR() IN CCKIRWRITE.C.  APR RELOCATION WAS
  CAPTURED AT THE SAME BREAKPOINT: SAVED APR WORD AT 023431, RELOCATION
  BASE 0340000, LOGICAL PC 0133324, PHYSICAL ADDRESS 0473324.
- FINDPTR PERFORMS LINEAR POINTER-IDENTITY SEARCHES FOR KIR GRAPH OBJECTS.
  A REVERSE SCAN, KCC COMMIT A081463, RETAINS DISCOVERY IDS, USES ZERO
  EXTRA LOOKUP RAM, AND REDUCES THE KPARSE IMAGE FROM OCTAL 150027 TO
  150020 WORDS (MINUS SEVEN).  THE HOST IDENTITY TEST AND NATIVE LINK
  PASS.  AN END-TO-END SPEEDUP IS NOT PROVEN; THE 185-SECOND SIMH TEST
  TIMED OUT AFTER KCC000006.  FUTURE OPTIMIZATION MUST ACCOUNT FOR RAM.
- THE EARLIER CPP INCLUDE TEXT-MODE AND SINGLE-PASS S6REC EXPERIMENTS
  WERE REVERTED AFTER NATIVE SIMH VERIFICATION FAILED; DO NOT REAPPLY
  WITHOUT TESTING BINARY INPUT/CCSRC AND THE NATIVE STDIO REWIND PATH.
- LONG COMPILER TESTS MUST NOT USE A SIMH DEBUGGER PROMPT AS A SUCCESS
  SIGNAL.  DAIMOS-TESTKIT'S LONGRUN V2 WAITS FOR THE DSH PROMPT;
  HARNESS V6 SUPPORTS --TIMEOUT-SNAPSHOT (CPU HISTORY, PC, REGISTERS)
  AND RETAINS OUTPUTS UNDER A UNIQUE WORK DIRECTORY.  A CONTROLLED
  FIVE-SECOND TIMEOUT VERIFIED SNAPSHOT CAPTURE AND FAIL REPORTING.
- SEPARATE OPEN GATES: SPURIOUS KPARSE CONSTANT-OVERFLOW DIAGNOSTICS
  AFTER SUCCESSFUL KCPP PREPROCESSING (SEE TODO.MD), AN EARLIER
  INVALID-UUO FAULT NOT YET EXPLAINED, AND INTERMITTENT NATIVE READ
  FAILURES THAT REQUIRE INDEPENDENT REPRODUCTION.  BUILDING FRESH D6FS
  TEST DISKS HAS ALSO EXPOSED POST-FORMAT LEAKED-BLOCK CHECK FAILURES;
  REPAIRS OF ISOLATED TEST MEDIA ARE NOT A SOURCE-LEVEL FIX.
- REPRODUCTION SCRIPT: DAIMOS-TESTKIT/TESTS/VALIDATION/
  DAIMOS-KPARSE-LONGRUN-20261010-V2.SH.  THE DEFAULT LIMIT IS 1200
  SECONDS, NOT A MEASURED COMPILATION DURATION.


## KPARSE KIR STACK CORRUPTION AND FOLLOW-ON ALLOCATION — 2026-10-10 V2

STATUS: THE PREVIOUS INVALID-UUO FAILURE WAS LOCALIZED, AND A BOUNDED-STACK
KIR WALK WAS IMPLEMENTED; THE FULL NATIVE COMPILER BUILD STILL FAILS.

- SIMH BREAK AT THE REAL INVALID-UUO HANDLER, PHYSICAL PC OCTAL 021202,
  CAUGHT AN OPCODE-ZERO TRAP.  THE INSTRUCTION HISTORY SHOWED CCKIRWRITE.C
  ADDSYM() RETURNING THROUGH POPJ 17,0 AT LOGICAL 134341.  ITS SAVED RETURN
  SLOT AT LOGICAL 176203 HELD OCTAL 227277, WHICH POINTED INTO NONCODE
  DATA CONTAINING 000000000402.  THE ACTUAL INCORRECT RETURN WAS THEREFORE
  STACK CONTENT CORRUPTION, NOT A MALFORMED SYSCALL GENERATED AT THAT PC.
- INITIAL USER BRK IS ABOVE THE ALLOCATED IMAGE AND FIXED 02000-WORD STACK
  RESERVATION; GRAPH RECURSION REACHED WELL BEYOND THAT AREA.  HEAP/STACK
  OVERLAP IS A STRONG CAUSAL EXPLANATION, BUT THE PRECISE OVERWRITING STORE
  WAS NOT ISOLATED.  KCC COMMIT DD4ACFE REPLACES RECURSIVE TYPE, SYMBOL,
  AND NODE DISCOVERY WITH A QUEUE WALK IN THE EXISTING IDENTITY ARRAYS.
  TESTKIT COMMIT BC4CB5C VERIFIES 12000 NODES, 9000 SYMBOLS AND 5000 TYPES
  WITH A 64-KB HOST STACK, INCLUDING CYCLES.  NATIVE KPARSE LINKS.
- IN AN ISOLATED NATIVE SIMH BUILD THE PREVIOUS UUO DID NOT RECUR.  KCC
  ADVANCED TO AN EXPLICIT ERROR AFTER 7 MINUTES 13 SECONDS: 'KIR GRAPH
  ALLOCATION/IDENTITY FAILURE: TYPES=6389 SYMBOLS=8192 NODES=296'.
  THE KIR1 EXTERNAL DEFINITION COULD NOT BE WRITTEN; MAKE CORRECTLY FAILED.
  THIS IS NOT A PASSING COMPILER BUILD.  IDENTIFY THE EXACT FAILED ARRAY
  OR MODULE-SYMBOL ALLOCATION AND REDUCE PEAK RAM BEFORE RETESTING.
- BOUNDED DISCOVERY INCREASES THE NATIVE KPARSE TEXT IMAGE FROM OCTAL
  150020 TO 150167 WORDS (+0147 OCTAL, 103 DECIMAL), WITH UNCHANGED BSS.
  IT ALLOCATES NO SEPARATE WORKLIST; THREE INDICES WALK EXISTING VECTORS.
- REPRODUCTION LOG: $HOME/tmp/kcc-kir-queue-simh-20261010-v1/evidence/run/simh.log
  (HOME-RELATIVE LOCAL DEVSPACE PATH; THE FILE IS NOT A REPOSITORY ARTIFACT).

## NATIVE KPARSE KIR IDENTITY AND UNSIGNED TARGET WIDTH — 2026-10-10 V3

STATUS: HOST CROSS-COMPILER AND SPLIT-PHASE REGRESSIONS PASS; NATIVE
SELF-COMPILATION IS NOT YET CONFIRMED COMPLETE.

- THE ORIGINAL INVALID UUO WAS REMOVED BY BOUNDED-STACK GRAPH DISCOVERY.
  A SEPARATE INTRUSIVE RUN-QUEUE SELF-LINK WAS FIXED IN DAIMOS 2162614.
- KIR ADDRESS-SPAN REJECTION WAS INVALID ON THE NATIVE PDP-6 POINTER
  REPRESENTATION: DIAGNOSTICS SHOWED REPEATED REGISTRATION OF THE SAME
  TYPE POINTER.  KCC F93D904 REMOVED NUMERIC NEGATIVE FILTERS AND USES
  EXACT POINTER EQUALITY.  THE NEXT NATIVE RUN HAD 208 PERSISTENT SYMBOLS
  RATHER THAN 17,000+, MATCHING THE HOST CENSUS, BUT STILL REPORTED
  65,536 NODES AND 25,192 TYPES AFTER ARRAY INITIALIZER ERRORS.
- HOST SPLIT KCPP/KPARSE ON CC.C PRODUCES 210 GRAPHS, AT MOST 55 SYMBOLS,
  36 TYPES AND 982 NODES PER GRAPH; THE LATEST HOST RUN HAS 207 PERSISTENT
  SYMBOLS AND MATCHES THE PRIOR HOST KIR OUTPUT BYTE FOR BYTE.
- KCC 7F6B59D RESTRICTS HOSTED UNSIGNED CONSTANT FOLDING TO TARGET 36-BIT
  WORDS, AND CORRECTS PIZARRAY'S UNSIZED-ARRAY LIMIT TO 0377777777777.
  HOST TESTS CHECK RIGHT SHIFTS, DESIGNATED INITIALIZERS, AND THE ACTUAL
  CCDECL ASSEMBLY.  TESTKIT F20525A AND CEB7A85 PROVIDE THESE REGRESSIONS.
- THE EARLIER LONGRUN V3 SCRIPT COULD PASS WITHOUT COMPILING BECAUSE IT
  REQUESTED OBSOLETE B/CC-CPP.S.  TESTKIT LONGRUN V4 INSTEAD INVOKES
  MAKE -C /OPTION/SOURCE/KCC -F /OPTION/SOURCE/KCC/DAIMOS.MK
  BUILD/CC-CPP.S, REQUIRES KCC OUTPUT AND REJECTS UNKNOWN TARGETS,
  RECIPE FAILURES, BAD ARRAY DESIGNATORS AND INVALID UUOS.
- A REAL NATIVE SIMH V4 RUN IS IN PROGRESS AT
  $HOME/tmp/daimos-array-bound-verified-20261010-v3/evidence/run/.
  NO NATIVE SUCCESS CLAIM SHOULD BE MADE WITHOUT ITS FINAL RESULT.
  THE INDEPENDENT MKD6FS POST-FORMAT LEAK REMAINS UNFIXED; ISOLATED
  TEST-DISK REPAIR DOES NOT COUNT AS A SOURCE-LEVEL FIX.

## 2026-10-12 GCC / ROOT-SELECTED DECtape FILESYSTEM CHECK

- GCC 3.2 WITH -Os CAN COMPILE AND LINK KINIT/KCORE FOR THE PDP-6.  THE
  INSTALLED GCC PDP10-SIXBIT.H SHADOWS THE PROJECT HEADER, SO A PRIVATE
  INCLUDE DIRECTORY WITH THE CURRENT SIXBIT PACKERS WAS USED FOR THIS TEST.
- DTFS AND TSFS STILL SHARE ONE MRES PACKAGE.  EXPLICIT ROOT=DISK AND
  ROOT=DRUM IMAGES NOW OMIT THAT PACKAGE ENTIRELY.  ROOT=AUTO AND ROOT=TAPE
  RETAIN IT SO DECtape BOOT REMAINS POSSIBLE; AT RUNTIME DTFS_MINIT ONLY
  INSTALLS THE PACKAGE FOR A SELECTED DTC ROOT.  THIS TEMPORARILY REMOVES
  BOTH DECtape FILESYSTEM PROVIDERS FROM D6FS-ROOTED SYSTEMS.
- GCC -Os D6FS-ONLY LINKED KCORE+MRES=15412 DECIMAL WORDS, PERMANENT
  SINGLE-ROOT=15459 AND MULTI-ROOT=15492 (LIMIT 16384).  GCC DISK-ROOT
  AND TAPE-ROOT KINIT DXR LINKS PASS.  A BOOT/RUNTIME PASS HAS NOT BEEN
  ESTABLISHED; DSH IMAGE SIZE AND OTHER BASELINE BUILD FAILURES BLOCKED IT.
- THE 2026-10-12 DAIMOS-TESTKIT FULL REGRESSION SWEEP COMPLETED WITH
  30 FAILED ROWS AND 2 PASSING ROWS.  HOST COVERAGE, TEST-STUB SYMBOLS,
  P10FOLD SIZE PARSING, AND BUILD DEPENDENCY/IMAGE CREATION ARE AMONG
  THE FAILURES.  THESE ARE NOT GCC RUNTIME TEST RESULTS.  SEE
  $HOME/tmp/daimos-userland-regression-20261012-v1.log.

## 2026-10-12 REGRESSION TEST REPAIRS (PARTIAL)

- MAKE DEPENDENCIES FOR MKBOOTBANNER, P10FOLD AND THE NATIVE DAS KCC
  COMPILER NOW RESOLVE THEIR EXECUTABLE PATHS WITH COMMAND -V INSTEAD OF
  TREATING A BARE COMMAND NAME AS A SOURCE FILE.  THIS REMOVES A COMMON
  EARLY BUILD BLOCKER ACROSS THE SIMULATOR MATRIX.
- DAIMOS-TESTKIT NOW FIXES DUPLICATE FILE TARGET STUB DEFINITIONS AND
  MISSING WCNSLS MODULE RUNTIME STUBS.  THE DIRECT TARGET SUITE PASSES.
  INSTALL/KILL NEGATIVE PROBES PROVIDE HOST COMMAND-COVERAGE ENTRIES.
- THE P10FOLD ASSERTION NOW ALLOWS INDEPENDENT MRES FOLD SAVINGS IN
  ADDITION TO KCORE; 65 KCORE WORDS AND 81 TOTAL WORDS WERE MEASURED.
  PERMANENT-SIZE ACCEPTANCE NOW MEASURES ROOT=DISK, WHICH PASSES WITH
  15644 DECIMAL WORDS ON THE PRESENT KCC INSTALLATION.
- BOOT-SOURCE MATRIX REACHED THE DSH EXEC IMAGE GUARD BUT FAILED:
  KCC-BUILT DSH IMAGE 036523 OCTAL EXCEEDS 036000.  DO NOT RAISE THE
  LIMIT TO MAKE THE TEST PASS.  DSH SIZE OPTIMIZATION IS NOT FINISHED;
  THIS FAILURE IS EXPECTED AND DEFERRED, NOT A TESTKIT DEFECT.
  NO COMPLETE BOOT/RUNTIME PASS YET.

## 2026-10-12 FULL REGRESSION RESWEEP

- FRESH DAIMOS-TESTKIT 32-ROW MATRIX WITH THE BUILD DEPENDENCY AND
  TARGET-STUB FIXES: 4 PASS (001 HOST, 002 TARGET, 024 LOGSTORE,
  032 TOOLS), 28 FAIL.  THE FAILURES CANNOT ALL BE ATTRIBUTED TO DSH.
- MOST SIMULATOR ROWS HIT THE EXPECTED UNOPTIMIZED DSH IMAGE LIMIT
  (036523 > 036000 OCTAL).  KEEP THIS GATE AS IS UNTIL DSH OPTIMIZATION.
- AN INDEPENDENT MKD6FS POST-FORMAT D6FSCK FAILURE REPEATED THROUGH THE
  BOOT/FILESYSTEM/SIMULATOR ROWS.  ROOT CAUSE WAS A STALE INSTALLED
  /USR/LOCAL/BIN/MKD6FS FROM OCTOBER 8, PREDATING THE COMMITTED
  D6M_RUN_FSCK EXECVP FIX.  REBUILDING/INSTALLING THE CURRENT MKD6FS
  AND D6FSCK MADE BOTH DISK AND DRUM POST-FORMAT CHECKS PASS.
- THE SWAP WAIT/STRESS TARGETS ALSO LACKED THE TESTKIT'S DEFAULT DAS_REPO;
  ADDING ITS SIBLING CHECKOUT DEFAULT EXPOSED THE EXPECTED DSH BUILD GATE.
- D6FS REMOUNT'S SOURCE EXTRACTOR HAD STARTED INCLUDING VFS_SHUTDOWN_ROOT;
  RESTRICTING IT TO VFS_REMOUNT RESTORED ITS STANDALONE PASS.
- LOG: $HOME/tmp/daimos-regression-20261012-v2.log; ROW LOGS:
  $HOME/tmp/daimos-regression-20261012-v2.logs/.
- MIXED BOOT=DSK, ROOT=DRUM PREVIOUSLY BUILT A BLANK AUXILIARY DRUM,
  SO KINIT COULD NOT FIND /SYSTEM/INIT AND HALTED ?RT.  ROOT=DRUM NOW
  ALWAYS BUILDS POPULATED D6FS DRUM MEDIA, INDEPENDENT OF STAGE1 DEVICE.
  DIAGNOSTIC ROOT-LOGIN LIFECYCLE PASSED DISK AND DRUM PHASES.
- TEMPORARY DSH_IMAGE_LIMIT=037000 IS USED ONLY FOR FURTHER DIAGNOSTIC
  RUNTIME ISOLATION; THE DEFAULT ACCEPTANCE LIMIT STAYS 036000.
  THE SINGLE-MEMBER TSFS ROOT IMAGE STILL FAILS MKTSFS CAPACITY WITH THE
  CURRENT USERLAND PAYLOAD.  THIS IS NOT YET PROVEN TO BE DSH-ONLY.
- DIAGNOSTIC MATRIX ($HOME/tmp/daimos-regression-diagnostic-20261012-v3.log)
  WAS STOPPED BEFORE ROW 017 COMPLETED AFTER CHECKING ROWS 001..016.
  TEN ROWS PASSED: 001,002,003,009,010,012,013,014,015,016.
  SIX FAILED: 004..008 AND 011.  THIS IS NOT A FULL ACCEPTANCE RESULT.
  ROW 011'S ONLY INITIAL FAILURE WAS AN OBSOLETE 0407-WORD U-AREA
  EXPECTATION; THE FOCUSED FIX NOW PASSES FOR 0410 WORDS, INCLUDING THE
  BREAK STATE AT 0407 AND SWAP BACKING AT 0406.
- ROW 004 STILL HAS AN OUTDATED DSK-ROOT-DISCOVERY FIXTURE WHICH EXPECTS
  DSK1..3 UNATTACHED, IN CONFLICT WITH THE DEFAULT FOUR-MEMBER ROOT.
  THE OTHER BOOT CHECKS IN THAT ROW PASSED.  ROWS 005..008 ARE BLOCKED
  IN PART BY THE ONE-MEMBER TSFS MEDIA CAPACITY FAILURE; DISK/DRUM
  ROOT CLASS, CROSS-BOOT, LOGIN, AND COMPRESSED-USERLAND SUBTESTS PASS.

## 2026-10-12 INDEPENDENT REGRESSION CONTINUATION

- DSH IMAGE SIZE AND SINGLE-MEMBER TSFS CAPACITY ARE EXPLICITLY DEFERRED.
  KEEP THE DEFAULT DSH LIMIT 036000 AND EXISTING TSFS CAPACITY UNCHANGED.
  TEMPORARY MAKEFLAGS=DSH_IMAGE_LIMIT=037000 WAS USED ONLY TO DIAGNOSE
  OTHERWISE BLOCKED TESTS.
- DSK-ROOT-DISCOVERY FIXTURE NOW CHECKS ALL FOUR CONFIGURED DSK MEMBERS
  AND ITS FULL SIMULATOR LOGIN/MOUNT REGRESSION PASSES.  THE OLD
  SINGLE-MEMBER ATTACHMENT ASSUMPTION WAS INVALID.
- 12-ROW NON-TSFS DIAGNOSTIC MATRIX ROWS 017-023,025-026,028-030:
  018 STACK, 019 IPC, 020 SWAP, 021 UUO ISOLATION, 022 TRAP COUNTS,
  AND 028 KERNEL/STANDALONE PASSED (SIX).  017 SCHEDULER, 023 STORAGE,
  025 DSK270, 026 DRM236 AND 029 D6FS RECOVERY FAILED (FIVE).
  ROW 030 REPORTED RUNTIME FAILURES BUT WAS STOPPED BEFORE IT FINISHED;
  DO NOT COUNT IT AS A COMPLETED ROW OR ACCEPTANCE PASS.
- SCHEDULER: APR SIMH, MULTIPROCESS, RT REVOKE, RT ADMISSION AND SLEEP
  SUBTESTS PASS.  AAP APR PASSES ON FOCUSED RETEST AFTER SETTING THE
  DEFAULT AAP PDP6 CHECKOUT.  RT YIELD DID NOT SCHEDULE THE NORMAL
  PROCESS BEFORE RESUME.  THE TIMER-FRONTIER FIXTURE NOW LINKS AGAINST
  THE PRODUCTION ASSEMBLER ABI BUT FAILS ITS STOPPED-TIMER ASSERTION
  (__TEST_EXIT=3); DO NOT ALTER THE EXPECTATION WITHOUT ROOT-CAUSE ANALYSIS.
- STORAGE: NINE SUBTESTS PASS, BUT BADMAP-ROOT FAILS WHEN D6SWAP REPORTS
  INVALID BADMAP METADATA.  DSK270-ONLINE ALSO REPORTS INVALID BADMAP
  METADATA AND CANNOT VERIFY RECOVERABLE DIRTY ROOT.
- DRM236: CAPACITY PASSES.  ONLINE NO LONGER FAILS FOR A MISSING SIMH
  VARIABLE, BUT FAILS WITH DRUM READ ACCOUNTING COUNT (FAIL_ID=021).
  MINIT FAILS THE PERMANENT-SIZE GATE FOR ITS CONFIGURATION.  STAGE1
  TEST EXPECTS A LEGACY DR0 MEDIA PATH AND FAILS.
- D6FS RECOVERY: DCS0 TRANSPORT RESET DURING SIMULATOR TEST; UNKNOWN
  WHETHER CAUSE IS BOOT OR TRANSPORT.  RUNTIME ROW 030 ALSO HAS
  STAT/DF/MONITORFS AND NATIVE DTFS MOUNT ASSERTION FAILURES; ITS
  MULTI-TTY TEST DID NOT REPROMPT AFTER REMOTE CARRIER.
- RETAIN ROW LOGS AT $HOME/tmp/daimos-independent-20261012-v1.logs/
  AND RUNNER LOG $HOME/tmp/daimos-independent-20261012-v1.log.

## 2026-10-12 BADMAP / DSK270 / TIMER REGRESSION REPAIRS

- THE BADMAP-ROOT TEST HAD TWO OBSOLETE ASSUMPTIONS: IT OPENED THE CURRENT
  FOUR-MEMBER D6FS ROOT WITH -N 1, AND CHOSE THE *LAST* MKD6FS DIAGNOSTIC,
  WHICH BELONGED TO THE EMPTY AUXILIARY DRUM (SUPER=0/1).  THE TEST NOW
  SELECTS THE DISK FORMAT AND MAPS EACH LOGICAL SUPERBLOCK TO ITS OWN DSK
  MEMBER/PHYSICAL SECTOR.  HOST D6FSCK AND SIMULATOR RECOVERY THROUGH BADMAP
  TO LOGIN BOTH PASS AFTER THE ORIGINAL SUPERBLOCK SECTORS ARE DESTROYED.
- DSK270-ONLINE NOW OPENS THE FOUR-MEMBER SET WITH -N 4.  ORDERLY HALT
  PUBLISHES CLEAN D6FS (VFS_SHUTDOWN_ROOT), NOT DIRTY.  THE TEST NOW REQUIRES
  A CLEAN, CONSISTENT POST-HALT ROOT AND PASSES TARGET READ/WRITE AND D6FSCK.
- PROC_TIMER_SERVICE KEPT THE FRONTIER CLOCK/BASE IN AC6/AC7 WHILE CALLING
  PROC_RUNQ_ADD, WHICH CLOBBERS BOTH.  THE KERNEL NOW SAVES/RESTORES THESE
  TWO ACS ACROSS RUN-QUEUE INSERTION.  THE DIRECT TIMER-FRONTIER TEST PASSES
  DUE/FUTURE, STOP, WRAPAROUND, AND QUEUE CHAINING.  KCC -O D6FS-ROOT
  PERMANENT SIZE = 15648 DECIMAL WORDS, BELOW THE 16384-WORD LIMIT.
- RTCTL YIELD REMAINS OPEN: THE SIMH RT REGRESSION DOES NOT OBSERVE PEER
  EXECUTION BEFORE OWNER RETURN; A TEMPORARY CURSOR-PRIORITY EXPERIMENT DID
  NOT CORRECT THIS AND WAS REVERTED.  TIMER-FRONTIER FIX IS RETAINED.
  DO NOT WEAKEN YIELD ACCEPTANCE OR CLAIM THE ENTIRE SCHEDULER ROW PASSES.
- DSH EXECUTABLE SIZE AND SINGLE-MEMBER TSFS CAPACITY REMAIN DEFERRED;
  PRODUCTION ACCEPTANCE LIMITS ARE UNCHANGED.  TARGETED SIMH TESTS USED
  TEMPORARY DSH_IMAGE_LIMIT=037000 TO ISOLATE UNRELATED FAILURES.

## 2026-10-12 DRM236 REGRESSION CLOSURE

- ALL FOUR DRM236 TARGETS NOW PASS TOGETHER: CAPACITY, ONLINE TYPE-167/236
  DMA AND QUEUED I/O, MINIT/MRES, AND DIRECT DRM STAGE1 TO LOGIN WITH
  DSK0 UNATTACHED.
- REAL DRIVER DEFECT: DRM236_IO.S NEVER UPDATED MFSDEV_DRM_READS OR
  MFSDEV_DRM_WRITES.  BOTH COUNTERS WERE ZERO DESPITE SUCCESSFUL BLOCK
  TRANSFERS.  COMPLETION ACCOUNTING NOW INCREMENTS ONCE AT EACH SUCCESSFUL
  POLLED OR PI2 DRUM COMPLETION (NOT AT THE INTERMEDIATE DP INTERRUPT OR
  WHEN ENQUEUING A REQUEST).  FAILED TRANSFERS INCREMENT THE EXISTING
  DRM STORAGE ERROR COUNTER.  NO NEW KCORE STATE WORDS ARE ALLOCATED.
- TESTKIT MINIT NOW MEASURES AN EXPLICIT ROOT=DRUM PROFILE INSTEAD OF
  AUTO-ROOT INCLUDING THE UNNEEDED TSFS PACKAGE.  THE STAGE1 TEST NOW
  CHECKS THE FOUR-MEMBER DRUM D6FS IMAGE AND REAL BOOT, NOT A REMOVED
  MEDIA/DR0.DRM PATH OR BYTE EQUALITY WITH A LARGER DISK IMAGE.
- LOG: $HOME/tmp/daimos-drm-regression-20261012-v3.log.
- FINAL TARGETED MATRIX RERUN: STORAGE ROW 023, DSK270 ROW 025, AND DRM236
  ROW 026 ALL PASS (3/3) ON THE CURRENT SOURCES WITH THE DIAGNOSTIC
  DSH_IMAGE_LIMIT=037000.  THE NORMAL DSH LIMIT IS STILL 036000.
  LOG: $HOME/tmp/daimos-repaired-storage-20261012-v1.log.
- DISK-ROOT PERMANENT-SIZE RECHECK AFTER DRM ACCOUNTING: 15663 DECIMAL
  WORDS, 721 WORDS BELOW THE 16384-WORD ADDRESS LIMIT.

## 2026-10-12 D6FS HDD RECOVERY FOLLOW-UP

- THE RECOVERY TESTKIT FIXTURE BUILDS A PRIVATE ONE-MEMBER D6FS IMAGE,
  BUT WAS STILL ATTACHING DSK1..3 FROM THE PRODUCTION FOUR-MEMBER IMAGE.
  THE PRIVATE BOOT CONFIGURATION MUST OMIT THOSE EXTRA ATTACHMENTS.
- EVEN WITH THAT FIXTURE CORRECTION, THE RECOVERY ACCEPTANCE IS NOT GREEN.
  PRESERVED SIMULATOR TRANSCRIPT FROM THE ONE-MEMBER TEST SHOWS DAIMON,
  DSK270, AND D6FS INITIALIZATION SUCCEEDING, FOLLOWED BY
  `HALT ... (MUUO114 7,10(16))` BEFORE INIT/LOGIN.
  THE DCS0 SOCKET RESET IS A CONSEQUENCE OF THE SIMULATOR HALT, NOT AN
  INDEPENDENT TRANSPORT ROOT CAUSE.  DETERMINE WHY THE PRIVATE ONE-MEMBER
  BOOT/USERLAND FIXTURE EXECUTES AN INVALID MONITOR UUO BEFORE REPAIRING
  THE RECOVERY ACCEPTANCE.  DO NOT MARK THIS TEST PASSED.
- DIAGNOSTIC LOG: $HOME/tmp/daimos-d6fs-recovery-debug-20261012-v2.log;
  PRESERVED SIMULATOR TRANSCRIPT UNDER
  $HOME/tmp/daimos-d6fs-recovery-v1-28068/run/simh.log.
- UNCORRUPTED CONTROL: A SEPARATE DIAGNOSTIC RUN LEFT BOTH D6FS
  SUPERBLOCKS UNMODIFIED AND STILL HALTED AT THE SAME `MUUO114 7,10(16)`
  BEFORE LOGIN.  THUS SUPERBLOCK CORRUPTION IS NOT THE CAUSE OF THIS
  BOOT/USERLAND FAILURE.  CONTROL TRANSCRIPT:
  $HOME/tmp/daimos-d6fs-recovery-v1-1078/run/simh.log.

## 2026-10-12 SINGLE-MEMBER RECOVERY CONTROL FOLLOW-UP

- THE RECOVERY IMAGE WAS MISSING /SYSTEM/EXEC/MOUNTALL, WHICH CURRENT
  /SYSTEM/INIT ATTEMPTS TO EXEC BEFORE STARTING LOGIN.  THE TESTKIT NOW
  INCLUDES THE REQUIRED EXECUTABLE; THIS IS A NECESSARY FIXTURE UPDATE,
  NOT YET A RECOVERY PASS.
- A FRESH UNCORRUPTED ONE-MEMBER CONTROL WITH MOUNTALL INSTALLED STILL
  HALTS AFTER D6FS MODULE INITIALIZATION AT `MUUO114 7,10(16)` BEFORE
  INIT'S BANNER.  THE FILESYSTEM FORMATTER AND HOST D6FSCK PASS.
  THE ONE-MEMBER ROOT MASK IS SUPPORTED BY D6FS_BOOT_SELECT; AN UNSUPPORTED
  SINGLE-MEMBER ROOT HAS NOT BEEN ESTABLISHED AS THE CAUSE.
- PRESERVED LOG: $HOME/tmp/daimos-d6fs-recovery-v1-23953/run/simh.log;
  WRAPPER LOG: $HOME/tmp/daimos-d6fs-recovery-clean-mountall-20261012-v1.log.
  CONTINUE BY COMPARING FIRST USER EXEC/BOOT CONTEXT WITH THE PASSING
  FOUR-MEMBER BOOT, NOT BY SUPPRESSING THE HALT OR THE RECOVERY ASSERTIONS.
- RT YIELD IS SEPARATELY OPEN.  DSH SIZE AND TSFS CAPACITY ARE DEFERRED.

## 2026-10-12 TSFS-ROOT D6FS ELIMINATION

- ROOT=TAPE / BOOT=DTC BUILDS NOW OMIT THE D6FS MRES PACKAGE AND D6FS
  MINIT ENTIRELY.  THE KINIT ROOT SELECTOR CANNOT PROBE/MOUNT D6FS,
  INCLUDING WHEN DSK/DRM HARDWARE IS ATTACHED.  DISK AND DRUM CONTROLLERS
  REMAIN AVAILABLE FOR RAW SWAP, LOGSTORE AND MEMFS AUXILIARY BACKING.
  TSFS-ONLY BUILDS NOW OMIT THE UNUSED BLOCKSET AND BADMAP MRES PACKAGES,
  SINCE ONLY ONE RAW AUXILIARY DISK/DRUM IS NEEDED.
- IN THE MIXED ROOT=AUTO PROFILE, D6FS MINIT REFUSES TO INSTALL ITS
  PACKAGE IF ROOT SELECTION CHOSE DTC/TSFS.  MIXED AUTO PROFILES STILL
  INCLUDE BOTH PACKAGES IN THEIR MAXIMUM STATIC SIZE REPORT; THE AUTO
  PROFILE IS NOT YET UNDER THE 16K MAXIMUM AND IS NOT CLAIMED GREEN.
- THE TSFS-SPECIFIC RESIDENT SIZE REPORT NOW EXCLUDES D6FS AND COUNTS
  THE RETAINED RAW AUXILIARY MODULES.  ISOLATED PDP-6 KCC BUILD/LINK:
  KCORE+MRES 15322 DECIMAL WORDS; WITH BLOCKSET 15355; LAST ADDRESS
  15402; 981 FREE ADDRESSABLE WORDS BELOW 040000 OCTAL.
- DAIMOS-TESTKIT TARGET `TEST-DAIMOS-TSFS-ROOT-NO-D6FS` PASSES.
  DISK-ROOT PERMANENT-SIZE GATE ALSO PASSES (15663 RESIDENT WORDS).
  LOG: $HOME/tmp/daimos-tsfs-and-disk-regression-20261012-v1.log.
- THE SINGLE-MEMBER TSFS MEDIA-CAPACITY ISSUE REMAINS DEFERRED AND ITS
  CAPACITY LIMIT UNCHANGED.  DSH IMAGE-SIZE OPTIMIZATION IS ALSO DEFERRED.

## 2026-10-12 D6FS RECOVERY RECHECK AFTER DSH WORK

- FETCHED AND FAST-FORWARD PULLED THE CURRENT DAIMOS AND DAIMOS-TESTKIT
  ORIGIN/MAIN.  THE NEW COMMITS ADDED DOCUMENTATION AND SEPARATE D6FS
  HOST REGRESSIONS; THE CORE BOOT/EXEC/PATHNAME FILES WERE UNCHANGED.
- PRESERVED THE SIX-FILE UNCOMMITTED RECOVERY DIAGNOSTICS IN A VERSIONED
  PATCH/STASH AND REMOVED THEM FROM THE WORKTREE BEFORE REBUILDING.
- A FRESH ISOLATED PDP-6 BUILD REACHED LOGIN WITHOUT THE EARLIER MUUO114
  HALT.  THEREFORE THE PREVIOUS BOOT FAILURE IS NOT REPRODUCIBLE ON THE
  CLEAN CURRENT TREE; INTERFERENCE FROM SHARED/STALE BUILD ARTIFACTS IS
  SUSPECTED BUT NOT CONCLUSIVELY PROVEN.
- THE RECOVERY TEST STILL FAILED SIX RUNTIME ASSERTIONS INITIALLY BECAUSE
  ITS PRIVATE IMAGE INSTALLED DSH BUT NONE OF THE EXTERNAL COMMANDS USED
  BY ITS PROBES.  "DSH: NOT FOUND" HAD ALSO ALLOWED SOME NEGATIVE PROBES
  TO PASS SPURIOUSLY.  THE TESTKIT NOW INSTALLS THE NINE REQUIRED PROGRAMS
  (TOUCH, STAT, ECHO, CAT, CP, MV, MKDIR, RM, LS) AND ALL 16 PROBES
  EXPLICITLY REJECT MISSING-COMMAND DIAGNOSTICS.
- THE FULL D6FS RECOVERY REGRESSION NOW PASSES 16/16 PDP-6 USERLAND PROBES,
  VERIFIES THE BOOTSTREAM DID NOT CHANGE, AND REJECTS THE SUBSEQUENT DIRTY
  REBOOT WITH ?RT.  TEST EXIT CODE 0.  THE EARLIER RECOVERY-FAIL STATUS
  ABOVE IS SUPERSEDED; NO DAIMOS KERNEL SOURCE CHANGE WAS NEEDED.
- ISOLATED ACCEPTANCE LOG AND REPORT:
  $HOME/tmp/daimos-recovery-recheck-20261012-v2/run.log
  $HOME/tmp/daimos-recovery-recheck-20261012-v2/report.md
- RT YIELD REMAINS SEPARATELY OPEN.  DSH IMAGE SIZE AND SINGLE-MEMBER
  TSFS CAPACITY REMAIN DEFERRED WITH THEIR PRODUCTION LIMITS UNCHANGED.

## 2026-10-12 RTCTL YIELD REGRESSION CORRECTION

- THE RT YIELD ACCEPTANCE FAILURE WAS CAUSED BY AN INVALID TEST ASSERTION,
  NOT BY A VERIFIED KERNEL/ASSEMBLER BUG.  THE OLD PROBE REQUIRED B/C PEER
  STARTUP MARKERS (ALREADY EMITTED WHILE THE OWNER WAS BLOCKED) OR FULL
  PEER WORKLOAD COMPLETION BEFORE THE YIELDING OWNER RETURNED.
- THE UNMODIFIED PRODUCTION SCHEDULER WAS REBUILT AND TESTED WITH PERIODIC
  PEER-PROGRESS MARKERS.  THE CONTROL SHOWED 156 NORMAL-PEER PROGRESS EVENTS
  AFTER THE OWNER'S W MARKER AND BEFORE ITS YIELD-RETURN Y MARKER.  THIS
  SATISFIES THE DOCUMENTED CONTRACT: A NORMAL PEER MUST EXECUTE, BUT NEED
  NOT FINISH ITS CPU-BOUND JOB BEFORE YIELD RETURNS.
- THE TESTKIT RT TEST NOW EMITS BOUNDED P MARKERS AFTER PEER WORK CHUNKS
  AND REQUIRES A P BETWEEN W AND Y.  THE DIRECT REAL PDP-6 SIMH REGRESSION
  PASSES ON UNCHANGED PRODUCTION SCHEDULER CODE; EXPERIMENTAL CURSOR/
  RESCHEDULER/PI TRACE CHANGES WERE NOT RETAINED.
- LOG: $HOME/tmp/daimos-scheduler-realtime-20261012-v3.log.
- SINGLE-MEMBER TSFS CAPACITY AND DSH SIZE OPTIMIZATION REMAIN DEFERRED.

## 2026-10-12 SCHEDULER ROW 017 FOLLOW-UP

- RE-RAN ALL TEN CONSTITUENT TESTS IN THE SCHEDULER MATRIX WITH THE
  CORRECTED RT YIELD PROBE.  NINE PASS: APR SIMH PROTECTION AND NXM/PDL
  INJECTION, AAP APR, MULTIPROCESS SCHEDULING, RT YIELD, RT REVOCATION,
  RT-REQUIRED ADMISSION, SLEEP, AND TIMER FRONTIER.
- ROW 017 IS NOT FULLY GREEN: THE SEPARATE 32K LOWMEM BOOT TEST ACCEPTS
  ROOT CREDENTIALS BUT REPORTS `LOGIN: EXEC FAILED` INSTEAD OF STARTING
  DSH.  THE EXACT CAUSE HAS NOT BEEN PROVEN; THE ONGOING/DEFERRED DSH
  EXECUTABLE-SIZE WORK IS A POSSIBLE CONTRIBUTOR.  DO NOT MISATTRIBUTE
  THIS FAILURE TO RT YIELD, AND DO NOT RELAX THE LOWMEM TEST.
- LOG: $HOME/tmp/daimos-scheduler-matrix-20261012-v1/logs/017.log.

## 2026-10-12 TSFS COMPACT ROOT MEDIA AND RAW BACKING

- TSFS ROOT DEPLOYMENT NO LONGER CREATES /PICTURES, IMPORTS STAGED
  PICTURE FILES, OR DEPENDS ON PICTURE_FILES.  THE D6FS ROOT IMAGE
  CONTINUES TO INCLUDE THE PREVIOUS /PICTURES PAYLOAD UNCHANGED.
- A REAL MKTSFS BUILD NOW PACKS THE ONE-MEMBER TSFS ROOT SUCCESSFULLY
  WITHOUT THE ~248 KIB OF STAGED PICTURE FILES.  OUTPUT IMAGE SIZE WAS
  ABOUT 578 KIB.  THIS SOLVES THE PREVIOUS MKTSFS PACKING ERROR FOR THE
  TESTED COMPACT CONTENT; NO TAPE GEOMETRY/CAPACITY LIMIT WAS INCREASED.
- ROOT=TAPE NOW ALSO EXCLUDES BLOCKSET MINIT/MRES AND BADMAP MINIT/MRES.
  AUXSTORE DISCOVERY, SWAP, LOGSTORE AND MEMFS BACKING USE RAW DSK/DRM0
  DIRECTLY.  ROOT=DISK/DRUM/AUTO PROFILES RETAIN BLOCKSET/BADMAP.
  THE PREVIOUS TSFS SINGLE-ROOT SIZE REPORT ALREADY OMITTED 33 BLOCKSET
  RESIDENT WORDS; REMOVING THE UNUSED BADMAP PACKAGE SAVES 65 MORE.
  KCORE+MRES = 15257 WORDS, PERMANENT_LAST = 15304, 1079 FREE ADDRESSABLE
  WORDS BELOW 040000 OCTAL.  BOTH THE COMPACT ROOT-IMAGE TEST AND THE
  D6FS/BLOCKSET/BADMAP-FREE LINK TEST PASS; DISK-ROOT SIZE TEST STILL PASSES.
- FULL TSFS ROOT SIMULATOR BOOT IS NOT YET ACCEPTED: WITH DISK STAGE1
  AND TSFS SELECTED AS ROOT, MODULES INCLUDING LOGSTORE/MEMFS/DTFS LOAD
  BUT KINIT HALTS BEFORE LOGIN (MUUO105).  THE BOOT HANDOFF MUST BE
  INVESTIGATED SEPARATELY; DO NOT ATTRIBUTE THE FAILURE TO BLOCKSET
  REMOVAL WITHOUT A CONTROL RUN.  TRANSCRIPT:
  $HOME/tmp/daimos-tsfs-boot-no-pictures-20261012-v1/sim-v2/simh.log.
- THE SEPARATE DSH EXECUTABLE-SIZE GATE REMAINS 036000 OCTAL AND IS NOT
  RELAXED.  THE ROOT IMAGE PACKING TEST USED DIAGNOSTIC DSH_IMAGE_LIMIT=037000
  TO EXERCISE MEDIA GENERATION, NOT TO ACCEPT AN OVERSIZED DSH.

## 2026-10-15 SINGLE-DISK 32K/48K LOWMEM BOOT MEASUREMENT

- A GENUINE ONE-MEMBER D6FS ROOT WAS GENERATED WITH `mkdsk -n 1` AND
  `mkd6fs -n 1` (44799 filesystem blocks; `d6fsck -n 1` CLEAN).
  SIMH ATTACHED ONLY DSK0 FOR THE ROOT; CTY WAS THE LOGIN CONSOLE.
  THE BOOTSTRAP STILL USES THE SIMULATOR PTR READ-IN PATH.  THE STOCK
  LOWMEM KERNEL STILL CONTAINS DCS/GE/TTY RESIDENT PACKAGES EVEN WITHOUT
  SERIAL DEVICES ATTACHED: THIS WAS NOT A CTY-ONLY KERNEL LINK.
- SIMH ACCEPTS `set cpu 48k`.  KINIT'S PREVIOUS MEMORY DETECTION ONLY
  PROBED 32K MULTIPLES, AND REPORTED 32K ON 48K MACHINES.  KINIT_DIAG.C
  NOW PROBES CONTIGUOUS 8K BOUNDARIES ABOVE THE 32K MINIMUM AND PRINTS
  THE ACTUAL DETECTED SIZE.  VERIFIED 48K BANNER: `MEM 48 K`.
- A FRESH 48K LOWMEM BUILD (`SIMH_CPU_KWORDS=48`,
  `BOOT_CORE_WORDS=0140000`) BOOTED D6FS, INIT, CTY LOGIN ROOT, AND
  DSH V1, AND EXECUTED MEMSTAT SUCCESSFULLY.  RUNTIME COUNTERS:
  TOTAL=49152, RESIDENT=12725, PROCESS-WORDS=28440, MEMFS-USED=0,
  MEMFS-CAPACITY=0.  THE DIFFERENCE, 7987 WORDS, IS UNALLOCATED
  ACCORDING TO THESE COUNTERS (NOT NECESSARILY A CONTIGUOUS EXTENT).
  TRANSCRIPT: $HOME/tmp/daimos-48k-single-v4-memstat.log.
- THE SAME OBSERVED RESIDENT+PROCESS FOOTPRINT IS 41165 WORDS,
  EXCEEDING 32K BY 8397 WORDS.  THIS IS A WORKLOAD COMPARISON, NOT A
  PRECISE MINIMUM DSH BOOT REQUIREMENT: MEMSTAT MAY CREATE ADDITIONAL
  PROCESS ALLOCATIONS, AND MM FRAGMENTATION/ALIGNMENT MATTERS.
- SEPARATE STATIC CTY+CLK+DSK+D6FS ESTIMATE, EXCLUDING UNUSED DCS/GE/TTY
  AND BLOCKSET: KCORE=10020, MRES=2478, TOTAL=12498 WORDS.
  THIS IS AN ESTIMATE FOR A STRIPPED LINK, NOT THE RUNNING 48K IMAGE.
  DSH DXR: IMAGE=031321, BSS=012751 (OCTAL); WITH USER ORIGIN/STACK
  ITS VM NEEDS 19658 WORDS, ROUNDED TO 20480 AT 02000 ALIGNMENT.
  THE STRIPPED STATIC KERNEL PLUS THIS DSH VM WOULD NEED 32978 WORDS,
  210 MORE THAN 32K, BEFORE OTHER ALLOCATIONS.
- 32K SINGLE-DISK BOOT REACHES LOGIN; LOGIN->DSH STILL FAILS IN THE
  EARLIER FULL LOWMEM CONFIGURATION.  DO NOT CLAIM 32K DSH ACCEPTANCE.
  A SEPARATE LOWMEM EXEC FRAME-OVERLAP CORRECTION IS IN THE WORKTREE.
- NEXT STAGE (SEPARATE CHAT): OPTIMIZE PERMANENT KCORE TO BELOW 8192
  WORDS, THEN REASSESS THE 32K BOOT GAP WITH ACTUAL LIVE MEASUREMENTS.
  DO NOT SIMPLY DISABLE FEATURES OR RAISE THE 32K MINIMUM; THE 8397-WORD
  WORKLOAD DIFFERENCE CANNOT BE ASSUMED RECOVERABLE FROM KCORE ALONE.

## 2026-10-24 file CHDIR STAT reuse v232

CHDIR now supplies its existing validated VFS STAT frame to the access
policy, rather than performing another provider STAT. Search permissions
and file ABI remain unchanged. GCC lowmem link PASS: KCORE 025115 octal
(10829 decimal), KCORE+MRES=13692, no resident size change. The
PDP-6 target file-walk fixture PASSes, including CHDIR. Continue
proposal B looking for resident text and data reduction.

## 2026-10-24 VFS failure-epilogue audit v238

On the v237 025075-octal (10813 decimal) GCC lowmem full-link baseline,
sharing SETATTR's three-word unwind was size-neutral; sharing SYMLINK
and RENAME's four-word unwind with WRITE_WORDS unexpectedly increased
KCORE to 025077 octal (10815 decimal), KCORE+MRES to 13678, and
PERMANENT_LAST to 13725. Both experiments were reverted. No new
production savings or runtime acceptance; continue proposal B at
larger VFS/provider boundaries. JRST folding and strict 32K work
remain deferred.

## 2026-10-24 VFS/provider architectural audit v239

Reviewed VFS lookup/parent resolution, CREATE/MKDIR/SYMLINK
publication, and MonitorFS leaf lookup/READDIR. Existing common
helpers already cover the main overlapping operations. A tentative
leaf-length loop-invariant move was reverted immediately because AC5
is clobbered by name comparison on subsequent iterations. No
production change, unverified size claim or new runtime acceptance.
KCORE remains 025075 octal (10813 decimal). Proposal B remains open.

## 2026-10-24 file mount-extension update sharing v240

The direct-store and descriptor-copy paths now share the two-bit mount
extension bitmap update sequence in file_runtime.s, preserving the
destination-clearing semantics. GCC lowmem disk-root full image PASS:
KCORE 025067 octal (10807 decimal), KCORE+MRES 13670,
PERMANENT_LAST 13717, six permanent words saved from v239.
PDP-6 file and file-walk tests PASS. Extended native descriptor
mount-extension lifecycle acceptance remains open. JRST folding
and strict 32K acceptance remain deferred.

## 2026-10-24 compact FILE duplicate v241

Two contiguous FILE descriptor words now copy by one bounded BLT
instead of two MOVE/MOVEM pairs in file_dup. GCC lowmem disk-root
full linked size KCORE=025066 octal (10806 decimal), KCORE+MRES=13669,
PERMANENT_LAST=13716, one resident word below v240. Native
daimos-file-mountext-v19 tests PASS 26-mount vnode, lock, DUP/DUP2,
close and unmount; PDP-6 file/file-walk fixtures PASS. Strict 32K
and JRST folding remain deferred.

## 2026-10-24 compact FILE DUP2 v242

The DUP2 descriptor pair uses a bounded BLT, and pipe reference
handling reads the copied destination metadata. The full GCC
lowmem disk-root link PASSes: KCORE 025065 octal (10805 decimal),
KCORE+MRES=13668 and PERMANENT_LAST=13715, one more permanent
word saved. Native daimos-file-mountext-v19 and PDP-6 file and
file-walk fixtures PASS. No ABI, resident data or stack growth.
Strict 32K acceptance and JRST folding remain deferred.

## 2026-10-24 descriptor and GETCWD audit v243

Reviewed descriptor-copy follow-up and the variable-length
GETCWD output zeroing loop. DUP/DUP2 copies and the mount extension
bit update already share compact implementations. A proposed BLT
zero-fill was reverted before measurement because setup and overlap
semantics offered no demonstrated advantage. No production changes.
KCORE baseline remains 025065 octal (10805 decimal).

## 2026-10-24 file path/provider request-frame audit v244

Reviewed resident SYMLINK, RENAME, CHDIR, MKDIR/MKFIFO,
UNLINK/RMDIR, TRUNCATE/STAT and VFS CREATE request staging.
The principal request frames and failure unwinds are already
shared where their lifetimes agree; no safe additional reduction
was established. No production code changes. GCC testkit
vfs-stat-frame and pre-dsh-fs-closure tests PASS; linked baseline
remains KCORE 025065 octal (10805 decimal). Proposal B open,
JRST folding and strict 32K acceptance deferred.

## 2026-10-24 filesystem transfer workspace lifetime audit v245

Reviewed `fs_block_workspace` (0200 octal words) and the
`fs_provider_serialized` lifecycle. DTFS, TSFS and D6FS already
share the resident transfer buffer while holding the serialization
lock over sleeping I/O; MEMFS uses a distinct non-sleeping path.
No second large permanent transfer buffer or safe unprotected
overlap was identified. No source changes; baseline KCORE remains
025065 octal (10805 decimal). Proposal F remains open subject to
explicit concurrency/lifetime proof. Proposal B remains open.

## 2026-10-24 static allocator and VM storage audit v246

Inspected BSS allocations across MM, MEMFS, IPC, PROC and VM.
The MM extent registry is 0100 octal (64 decimal) words of live
descriptors; MEMFS chunk/XIP registries and VM/IPC/PROC singleton
words preserve distinct live state. No safe scratch overlay was
demonstrated. No production change; preserve KCORE 025065 octal
(10805 decimal). Measure MM extent high-water before revisiting
the descriptor count.

## 2026-10-24 MM extent registry validation gate v247

PDP-6 MM permits 040 octal (32) live extent descriptors occupying
0100 octal (64) words. Full-registry insertions return
MM_ERR_DESCRIPTORS (-3), so a smaller limit needs measured workload
high-water and fragmentation evidence. Testkit `mm-core` cannot
currently compile because it references a nonexistent
`system/kernel/proc/vm_pdp6.c`. The existing testkit tree has
unrelated local edits and was not modified. No production change
or accepted memory reduction.

## 2026-10-24 MM host test dependency investigation v248

Temporarily excluded the missing vm_pdp6.c from the testkit
mm-core build, revealing unresolved host symbols
vm_extent_move, vm_space_can_swap and vm_space_destroy. Real
implementations are in the target-side PDP-6 VM code; replacing
them with trivial host stubs would invalidate the test's
movement/compaction/swap assertions. The testkit Makefile was
restored and unrelated pre-existing modifications preserved.
The MM registry remains 32 entries/64 words pending validated
workload high-water data. No production change or size saving.

## 2026-10-24 KCORE bulk device I/O sharing v233

Both resident bulk I/O paths now prepare mapped-buffer arguments once
before device selection and preserve the vnode for ordinary VFS fallthrough.
The full linked GCC lowmem disk-root KCORE decreased 025115 -> 025103
octal (10829 -> 10819 decimal), KCORE+MRES 13692 -> 13682,
PERMANENT_LAST 13739 -> 13729. No static data, stack or ABI growth.
Target PDP-6 file and file-walk fixtures PASS. Native PTR/PTP simulator
fixture stopped with SIMH status 139 and SDL2 window-event warnings; it is
not recorded as a pass. Native card WORDTOKEN12/CR/CP round trip PASSed
on an independent SIMH test with SDL_VIDEODRIVER=dummy. Further native
PTR/PTP and other device I/O acceptance is open.
Strict 32K acceptance and JRST folding remain deferred.

## 2026-10-24 KCORE MonitorFS name copy v235

Two VFS three-word static name-record copies use bounded BLT
instead of six scalar instructions each. GCC lowmem disk-root
full link KCORE 025075 octal (10813 decimal), KCORE+MRES 13676,
PERMANENT_LAST 13723; saving six permanent words from v234.
Target PDP-6 file and file-walk fixtures PASS. A size-neutral
VFS provider fallthrough branch experiment was reverted.
Native SIMH MonitorFS device view-online PASSes (GCC profile).
Strict 32K acceptance and JRST folding remain deferred.

## 2026-10-24 PTR/PTP native verification v237

The v233 bulk I/O pathway's previously incomplete paper-tape native
acceptance is now verified: `test-daimos-pt-wordio` PASS on PDP-6 SIMH
using GCC and SDL dummy video/audio drivers. Both full and partial
PTR/PTP WORDTOKEN8 physical round trips passed. No production changes;
last KCORE measurement remains 025075 octal (10813 decimal), with
KCORE+MRES=13676. The strict 32K gate and JRST folding stay deferred.
