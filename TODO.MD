### v916: foreign provider-lock ownership/waiter regression (2026-11-01)

- [x] Native PDP-6 FS gateway test holds another process's provider
  lease through four simulated waiter retries. No early provider
  dispatch, stolen ownership or forced unlock. A fifth wait observes
  release, dispatches and restores request registers and stack.
- [x] Native v915 recursive-owner and v866 owner-encoding regressions
  PASS. No production code, RAM, syscall or module ABI change.
- [ ] Full booted live scheduler test with a truly stalled provider,
  diagnosis of foreign owner death, and allocation-free fatal/OOM
  resolution remain open P0. No forced unlock; OOM killing OFF.

### v915: prevent recursive provider-lock self-deadlock (2026-11-01)

- [x] Detect a DTFS/D6FS/TSFS gateway call by the existing lease owner
  before blocking in `proc_wait_event`; return -5 with the saved
  seven-word request frame unwound and without unlocking its lease.
- [x] Reuse the existing owner_slot+1 LH encoding in `fs_provider_ready`;
  nine additional KCORE instructions, no extra BSS, per-process lock
  table, provider ABI or MEMFS changes.
- [x] Native PDP-6 v915 recursive-owner and other-process waiter tests
  PASS. Existing v866, v326 and v327 tests PASS.
- [x] Full 256K PDP-6 v914 fatal FILE-provider refusal regression rebuilt
  against the final nine-instruction v915 KCORE and PASS (11 provider
  -4 refusals, 11 stable FD checks, 512-tick backoff, normal test HALT).
  Not yet a full-system recursive-provider-lock injection.
- [x] Post-v915 full-system regression: 256K PDP-6 v914 APR fatal-FILE
  provider invalid-refusal workload PASS. 11 VFS refusals, 11 private
  owner sleeps, 512-tick cap, retained descriptor and independent
  runnable user, zero errors, controlled HALT. This does not simulate
  a held-provider-lock deadlock.
- [ ] A different process owning a permanently stalled provider remains
  a separate unresolved progress case. Full-system PI6/clock provider
  lock acceptance and allocation-free fatal/OOM termination remain P0.
  OOM killing stays OFF; TSFS v1.2 and KCORE shrink stay deferred.

### v914: permanent invalid FILE-provider refusal containment (2026-11-01)

- [x] Booted 256K PDP-6; real APR fault causes permanent VFS_SYNC
  error -4 on the victim's FD 0, not the previously tested -5.
- [x] 11 rejected close passes and owner-stack sleeps with a 512-tick
  retry cap; FILE ownership, error-class-4 witness and process identity
  persist while an independent user runs. No premature close or EXIT;
  normal controlled test HALT. No production code or RAM change.
- [x] Native v888 MM error classification, v909 fatal MM stage and
  v885 private FILE backoff regressions PASS.
- [ ] This does not repair corrupted/missing MM extents or ensure
  eventual resource reclaim. Provider-lock refusal, fatal teardown
  completeness and allocation-free OOM killing remain P0. OOM stays
  OFF; TSFS v1.2 and KCORE final shrink remain deferred.

### v913: live PDP-6 double-MM release refusal acceptance (2026-11-01)

- [x] Booted 256K PDP-6 with Type-340 PI6 refresh/PI7 completion and
  real MM pinning of two distinct owner-014 raw-list extents.
- [x] Original list base 19760 equals published `dpy_list_base`;
  unpublished replacement base 19792 equals `dpy_list_orphan_v912`.
  Four rejected frees (replacement, rollback, next WRITE and CLOSE),
  2 pins, 0 unpins, 0 errors, 18 PI6/PI7 cycles and 12 post-error
  refreshes; guest <D and normal HALT. No extra allocation.
- [x] Existing native v912/v874/v904 regressions PASS. No production
  source/ABI/RAM change. Fault hooks remain only in daimos-testkit.
- [ ] Permanent refusal preserves both physical allocations; forced
  reclamation is not permitted. Fatal FILE/MM provider teardown,
  provider locks and allocation-free OOM termination remain P0.
  OOM killing stays OFF; TSFS v1.2 and size optimization stay deferred.

### v912: DPY double-free replacement failure containment (2026-11-01)

- [x] Replace the HALT on consecutive old-list and new-list MM-free
  refusal with one tracked unpublished DPY-MRES extent.
- [x] On every subsequent WRITE_WORDS or STOP, attempt orphan release
  before allocating or mutating the active display. Refusal propagates
  to FILE without losing either owner; recovery drains before proceeding.
- [x] Native PDP-6 v912 double-fault and recovery test PASS. Existing
  v871/v874/v904 and PI7 interrupt-policy regressions PASS.
- [x] Full-system post-change v910 permanent-MM_BUSY regression PASS on
  v912: 3 BUSY, one pin, 18 PI6/PI7 frames, normal HALT; no live-path
  regression. This is not the double-fault injection test.
- [ ] Full booted guest with live PI6/PI7 double-fault injection is not
  yet accepted. Also outstanding: permanent fatal FILE/MM providers,
  provider-lock refusal and safe allocation-free OOM termination.
  OOM killing remains OFF, KCORE shrink and TSFS v1.2 deferred.

### v911: repair resident SYS_MEMINFO FILE counting (2026-11-01)

- [x] Correct the 16-entry FILE scan and reported capacity from octal
  015 (13) to octal 020 (16); do not alter FILE allocator semantics.
- [x] Move the existing count past the temporary MEMFS request whose
  seven-word BLT erased result word 7. No added instructions or BSS.
- [x] Booted 256K PDP-6 C89 guest fills all 16 FILE slots by real DUP,
  checks MEMINFO after every allocation, verifies final FD 15, closes
  the duplicates, checks baseline, reports <F and halts normally.
- [x] Two isolated pre-fix negatives: old capacity yields !1; old
  pre-MEMFS FILE count yields !4. Native v708/v874/v904 PASS. Full v910
  live DPY permanent-MM_BUSY guest also PASSes with meaningful updated
  FILE occupancy accounting.
- [ ] This does not enable automatic OOM victim termination or recover
  permanently refusing FILE/MM providers. Replacement double-failure,
  lock/deadlock acceptance, and fatal FILE teardown stay P0; TSFS v1.2
  and final KCORE shrinking remain deferred.

### v910: persistent DPY/MM refusal across three live CLOSE attempts (2026-11-01)

- [x] Booted 256K PDP-6 Type-340, kept the raw-list extent physically
  pinned across three real MM_BUSY final CLOSE attempts. All returned -5;
  the already-open FILE/process ownership counters stayed unchanged,
  and the guest halted normally without forcibly freeing any memory.
- [x] Measured raw-list owner 014, base 19732 decimal, three MM frees,
  one pin, zero unpins, three MM_BUSY results, 18 PI6 raw refreshes,
  18 PI7 completions, 12 post-first-refusal refreshes and zero errors.
  v871, v874, v904 and the PI7 policy tests remain PASS.
- [ ] A permanently rejecting provider *retains* its physical RAM. This
  proves safe DPY ownership, not eventual cleanup. Still open: injected
  replacement double-fault, provider lock/deadlock handling, fatal FILE
  teardown and zero-allocation OOM victim retirement. OOM stays OFF;
  TSFS v1.2 and KCORE size optimization stay deferred.

### v909: P0 permanent physical-MM refusal and capped idle reaper (2026-11-01)

- [x] Reproduced a true CPU-intensive leak of retries under permanent
  physical `mm_pin`: the PDP-6 slot-0 idle loop retried a still-owned
  u-area every clock tick (1,281 rejected `mm_free` calls, 1,280 reaper
  passes across the guest's 02400-octal-tick observation period). The
  timer-driven scheduler admission path had a cap, but the already-idle
  loop ignored it. Negative control and original simulator transcript
  retained in v909 test evidence.
- [x] The idle loop now checks wrapped elapsed ticks against the existing
  shared retry interval before trying deferred MM release. No new BSS,
  per-process retry table, forced MM free or module dependency. The real
  guest records 7 refused frees, 6 reaper passes, 1 physical pin and
  no test errors; the independent bystander keeps running.
- [x] Once fatal FILE closure and VM destruction have committed, clear
  stale `PROC_FATAL_CLOSE_BIT` on the post-VM pending-u-area owner. A
  TRANSITION+VM_NONE+FINALIZE marker still quarantines it; GETTEARDOWN
  now correctly reports MM stage and the capped 01000-octal (512-tick)
  interval rather than the obsolete FILE stage. Native v909 verifies
  exact-once finalization and the old-code negative control fails.
- [x] Full-system v909 on 256K PDP-6: real APR fault, permanent pinned
  u-area, `GETTEARDOWN` MM and physical pin diagnostics, nonblocking
  WAIT refusal, independent runnable child, controlled test HALT PASS.
- [ ] Permanent failure still retains physical RAM indefinitely; this
  is safe ownership and bounded work, NOT successful recovery or safe
  automatic OOM termination. Continue locked-provider, nested fatal
  teardown and allocation-free OOM policy work. KCORE shrinking and
  TSFS v1.2 remain deferred.

### v908: live DPY raw-list MM_BUSY and PI7 completion (2026-11-01)

- [x] Full-system 256K PDP-6 headless Type-340 (`DPY OK`) verifies the
  *actual* raw-list PI6 refresh and PI7 BLKO completion path, not just
  static source/isolated assembly. A disposable test-kernel MM hook pins
  the exact dynamic owner 014 on final CLOSE. The native MM refuses
  release with -5; FILE and raw-list ownership remain live and clock
  refresh resumes. Retry physically unpins/frees; a new DPY program
  installs and closes; FILE/process counts recover; guest HALT normal.
- [x] v908 guest counters: MM base 19730 decimal, raw-list MM frees 3,
  pins/unpins 1/1, MM_BUSY 1, raw PI6 refresh 18, raw PI7 completion 18,
  *six* refreshes after the failed CLOSE but before unpin, errors 0.
  Standalone target C89 build and shell syntax PASS. Native v709, v871,
  v874, v904 and the DPY PI7 policy regressions also PASS.
- [ ] Indefinitely refusing FILE/MM providers, fatal process isolation,
  and allocation-free automatic OOM termination are NOT accepted by a
  single recoverable MM_BUSY. Do not force-release or enable OOM killing.
  DPY-specific persistent-refusal and double-fault behavior remain P0.
  TSFS development and KCORE shrinking remain deferred.

### v907: full-system HUP partial failure with physical MM (2026-11-01)

- [x] Disposable copied-kernel hooks pin the first real session child's
  u-area before normal fatal HUP, reject a second child's first HUP,
  then retry the leader's fatal HUP through the real PROCCTL syscall.
  The first child's VM_NONE+FINALIZE ownership remains physically pinned
  during the second scan and is skipped only by production v905 logic.
- [x] Booted 256K PDP-6 PASS: first child PID 3, leader PID 2, exact
  pinned u-area 20020 decimal; 1 MM pin/unpin, 1 injected HUP rejection,
  1 second-member delivery, 0 duplicate first-member HUPs, 1 observed
  deferred finalization, 0 errors. Parent WAIT and TTY reattach pass;
  normal HALT. No production kernel change or KCORE size work.
- [ ] A persistently rejecting FILE/MM provider still must never be
  force-released. The DPY live interrupt/MM_BUSY acceptance is covered
  by v908; the allocation-free OOM safe-victim gate remains closed.

### v906: reconcile older TSFS entries after KCORE deferral (2026-11-01)

- [x] Supersede the remaining unchecked cross-provider TSFS mount and
  DTFS/TSFS metadata/capacity experiments in older V13/V15/V22 lists.
  They are post-KCORE work, not active KCORE feature or size gates.
- [x] Preserve existing TSFS boot and VFS/codec compatibility checks
  whenever KCORE changes their shared contracts; this does not reopen
  TSFS v1.2 feature work. D6FS, generic VFS and backing work remain
  KCORE tasks. The TSFS feature sequence resumes only after KCORE.

### v905: P0 partial session-HUP retry (2026-11-01)

- [x] During session-leader teardown, skip an already terminal member
  only if its descriptor is TRANSITION, VM is NONE, and the original
  u-area contains PROC_FINALIZE_MARK_LH. This is the precise committed
  fatal-event/deferred-MM owner; `proc_event_apply` correctly rejects a
  second HUP, so retry must not try it again after another member fails.
- [x] Native PDP-6 partial HUP regression v905 PASS: first child commits
  with deferred u-area; second member fails, TTY remains; retry skips the
  committed child, HUPs only the failed member, then detaches. Cases with
  non-NONE VM or absent finalize mark cannot be skipped. Pre-v905 negative
  control fails as expected (guest failure code 010). Historical v875 PASS.
- [x] Existing full-system `daimos-session-teardown-v1.sh` on the v905
  kernel PASS: booted PDP-6 session-leader exit, HUP of running/stopped
  children, controlling-TTY detach and later reattach.
- [x] Full-system injected HUP partial-fault/actual-MM rollback PASS
  as v907, including one pinned terminal victim and later TTY handoff.
- [ ] Indefinitely refusing FILE/MM providers still require safe ownership
  handling; automatic OOM termination remains disabled.

### v904: P0 DPY teardown quiesce-before-unpublish (2026-11-01)

- [x] STOP disables the Type-340 controller and PI7 before removing the
  active raw-list base. This closes an interrupt window where PI7 could
  treat the still-running raw-list completion as a retained-text frame.
  No new instructions or permanent allocation; instruction order only.
- [x] v904 native PDP-6 checks actual MM pin/unpin/free: pinned extent
  refuses CLOSE without losing FILE or DPY ownership; invalid MM owner
  also refuses; correct owner then frees exact extent, and empty CLOSE
  succeeds. Pre-v904 STOP-order negative control rejects the old order.
  Existing v709, v871 and v874 tests PASS.
- [ ] Full-system DPY STOP under a live PI7 replay and permanent MM provider
  refusal remain separate P0 acceptance. Continue session/HUP partial
  failure and safe automatic OOM teardown; do not force-release MM.

### v903: TSFS scheduled strictly after KCORE (2026-11-01)

- [x] Remove TSFS v1.2 format/metadata implementation and its acceptance
  from the active KCORE feature and size backlog. Complete KCORE features,
  their regressions and the deferred KCORE size/freeze work first. Then
  resume the separate TSFS v1.2 sharded-metadata effort before 0.9 release.
- [x] Preserve existing TSFS boot compatibility and regression coverage
  while changing other subsystems. Such preservation is a KCORE invariant,
  not permission to start new TSFS work during KCORE completion.
- [ ] Next active P0: DPY raw-list failure-safe teardown and real-MM
  acceptance; then session/HUP partial-failure recovery, permanent FILE/MM
  provider repair, and safe allocation-free OOM victim termination.

### v902: historical anonymous PIPE regression rerun (2026-11-01)

- [x] The full-system historical `daimos-pipe-anon-v1.c` user workload
  now passes on the v901 kernel with only headless simulator settings
  changed. Verified PIPE create, character I/O, EOF, EPIPE, duplicated
  references, `RUN` descriptor inheritance and blocking wakeups.
  The versioned v902 harness preserves the legacy test program.
- [ ] Keep feature completion and allocation-free teardown as the
  priority. OOM killing remains disabled until permanent FILE/MM
  provider faults, DPY cleanup and session-HUP failure graphs have
  safe ownership and recovery semantics.

### v901: PIPE character-stream feature restored (2026-11-01)

- [x] Native `READCHAR` and `WRITECHAR` on PIPE and named FIFO
  descriptors now dispatch to the existing one-word PIPE queue
  operations. These syscalls exchange one complete character word
  rather than packed SIXBIT; no new pipe buffer or permanent state.
- [x] Native PDP-6 positive test: WRITECHAR/READCHAR transfer,
  pending queue count, EOF after last writer, broken-pipe error and
  unchanged MonitorFS device dispatch PASS. Pre-v901 VFS source
  rejects PIPE WRITECHAR; negative control FAILS as expected.
- [x] Full 256K PDP-6 user syscall test PASS: writechar/readchar,
  cross character/word I/O in both directions, EOF after writer
  close, exact FILE-slot accounting and normal HALT. The underlying
  VFS gap also explains the earlier v899 character-read assertion;
  its separate real-MM word-I/O ownership test remains valid.
- [ ] Remaining feature work includes DPY raw-list teardown,
  session/HUP partial failure, permanent provider repair and safe
  allocation-free OOM victim termination. No size sweep attempted.

### v900: named FIFO MM refusal and list-ownership acceptance (2026-11-01)

- [x] In a booted 256K PDP-6, inject one actual `mm_pin` of the
  named FIFO's final physical pipe object. Its regular last CLOSE
  receives real MM_ERR_BUSY and restores the named object to its
  active linked list, retaining the original FILE descriptor.
- [x] A second O_RDWR OPEN joins that same object after the failure;
  writing through the original descriptor and reading from the new
  one proves cross-descriptor stream identity. The later final CLOSE
  unpins and frees the exact extent. A new FIFO OPEN/CLOSE and
  normal HALT pass. Counters: target 19687 decimal, calls 3,
  physical pins 1, unpins 1, busy 1, errors 0.
- [ ] Automatic OOM killing still requires complete allocation-free
  fatal FILE/MM/TTY/session teardown. An indefinitely refusing or
  invalid provider must not be forcibly released.

### v899: real PIPE final-close MM_BUSY recovery (2026-11-01)

- [x] Native v870 final PIPE close retry remains PASS.
- [x] Full-system 256K PDP-6 acceptance with actual MM pin,
  `mm_free` MM_ERR_BUSY and `mm_unpin` before retry PASS. The
  failed final close retains the original read FILE and PIPE
  reference, which can still report EOF. Later close releases
  the exact extent, and a new PIPE can be created/closed.
  Isolated guest counters: target 19687 (decimal), final-close
  calls 3 (including reuse), pins 1, unpins 1, busy 1, errors 0.
  The test kernel alone contains injection/counters, and the
  production KCORE is unchanged. No shrinking work attempted.
- [ ] Still open: named FIFO lifetime under real MM refusal, DPY
  raw-list cleanup, session-HUP partial commit, persistent FILE/MM
  provider refusal, and allocation-free automatic OOM retirement.
  An indefinitely pinned or inconsistent provider cannot be
  forcibly discarded.

### v898: booted PDP-6 real TTY MM_BUSY release acceptance (2026-11-01)

- [x] Production v897 control path accepted with actual physical
  `mm_alloc`, `mm_pin`, rejected `mm_free`, `mm_unpin`, and successful
  retry, all in a booted 256K PDP-6 guest. The test kernel is an
  isolated overlay; the production source is unchanged.
- [x] Session owner stays attached through the first rejected
  `TTY_DETACH`; the second attempt succeeds only after unpin.
  Subsequent attach/child session activity and normal HALT PASS.
  Test counters: four checked operations, one physical pin, one
  MM_BUSY, one unpin and zero errors. No KCORE size work was run.
- [ ] Remaining feature gates: refused FILE/MM providers, nonreturning
  fatal cleanup, session HUP partial-commit cases, and allocation-free
  OOM victim retirement. Automatic OOM termination stays disabled.

### Feature completion priority and TTY ownership v897 (2026-11-01)

- [x] Defer the KCORE 8,192-word size-reduction sweep until functional
  work and its real-system regression gates are complete. Memory
  footprint remains an invariant to avoid unnecessary growth, not
  the present optimization target.
- [x] v897: prevent `TTY_DETACH` from dropping controlling-session state
  if the canonical SIXBIT line MM extent is pinned or has invalid owner/
  type metadata. Reject `TTY_ATTACH` by a new session until a leftover
  line is successfully released; preserve existing TTY output routing.
  `proc_tty_mode_set` reuses the same checked, allocation-free helper.
- [x] v897 native PDP-6 `procctl` tests PASS failure/retained session,
  output-route preservation, retry after unpin, and successful handoff.
  The pre-v897 negative control incorrectly transfers a pinned line,
  as expected. A second native harness builds the actual C helper and
  executes real PDP-6 `mm_free` and `mm_unpin` for pinned, wrong-owner,
  missing-extent and successful-release cases: PASS.
- [x] v897 production full-system PDP-6 session attach/detach/reattach
  and independent child regression PASS. The headless test harness
  uses the original session-state user workload and normal kernel.
- [ ] Continue allocation-free teardown audit, persistent FILE/MM
  provider recovery, fatal process isolation, and safe OOM admission.
  Do not enable automatic OOM killing on a failed-provider path.

### v896: RUN startup owns allocated u-area before fallible work (2026-11-01)

- [x] Move the existing u-area descriptor publication before
  `vm_space_startup`, preserving SIDL exclusion and the same three-word
  process descriptor. A failed startup must enter `proc_slot_discard`
  with actual ownership and release the exact MM extent, including
  v892/v894 deferred cleanup on downstream provider errors.
- [x] Full-system PDP-6 startup-branch fault injection PASS: one
  unpublished RUN discard, exact MM u-area probe confirms 0 leaks,
  valid second RUN/WAIT, and normal HALT. The v895-ordering negative
  control independently detects exactly 1 leaked MM extent while
  process slot/VM accounting appears normal. Same-profile GCC lowmem
  KCORE 9,426, KCORE+MRES 15,912, endpoint 15,959, all unchanged.
  Native v885/v892/v894 cleanup regressions PASS.
- [x] Original v197 full-system regression PASS after the v896 change:
  16 post-VM RUN fdmap failures under three live children, stable
  accounting and recovery. Its test harness now disables SDL window
  consoles on headless hosts; the first GUI attempt crashed inside
  the simulator before guest assertions, not in DAIMOS.
- [ ] Persistent FILE/MM provider owner inconsistencies and safe OOM
  termination remain open. No forced MM release is authorized.

## Size refresh and pending work (v513, 2026-10-27)

### v864: OOM handling for already-running processes (2026-10-31)

- [x] v895: real full-system PDP-6 pre-VM RUN MM_BUSY recovery.
  Test-only `mm_pin` of the actual child process VM before a normal
  destructor produces two genuine MM_ERR_BUSY failures, then one
  `mm_unpin` permits the third attempt to complete. Slot-zero retry
  calls 2, independent child runs, no unpublished-child WAIT,
  zero errors and normal HALT. No production size or ABI changes.
- [x] v894: failed VM destruction during unpublished RUN rollback,
  **including before u-area allocation**, retains SIDL+TRANSITION
  and one spare scheduler RH bit 02 for reaper retry. A later
  successful VM destroy clears that mark and continues to v892
  post-VM cleanup if u-area free fails. Read-only GETTEARDOWN stage
  3 reports the ownership and shared delay. Native PDP-6 failure/
  retry, stage-3/older query and deferred-cleanup regressions PASS;
  pre-v894 negative control FAILS. Same-profile GCC KCORE +45
  code words, no static RAM, ABI syscall slot or process-table growth.
- [x] v894 full-system VM-pin follow-up: v895 now verifies actual
  booted PDP-6 `vm_space_destroy` MM_BUSY, retry/unpin, independent
  runnable work, and absence of a fabricated parent WAIT. v893's
  separate post-VM u-area busy case remains accepted.
- [x] v893: full-system PDP-6 RUN rollback with actual u-area
  `mm_pin`, two genuine MM_ERR_BUSY (-5) releases, one `mm_unpin`,
  two slot-zero retries and final FREE. Test-only kernel overlay;
  independent child runs and exits, no unpublished-child WAIT.
  SIMH counters 1/3/2/1/2, zero errors, normal HALT. No production
  change, size growth or static data.
- [x] v892: retain ownership after an unpublished RUN child has its
  VM destroyed but its u-area MM free fails. Register FINALIZE|DISCARD
  in existing word 067, reuse deferred slot-0 retries, and publish
  FREE (never a zombie or parent WAIT) only after successful free.
  Native fault-injection and prior deferred-EXIT tests PASS; the
  full-system v197 RUN fdmap rollback test PASSes 16 cases. Old
  v891 code FAILS the negative control; same-profile GCC KCORE +30
  words with zero new static words.
- [x] v892 full-system follow-up: real pinned u-area MM rejection,
  independent process, retry, exact-once discard and no parent event
  accepted by v893. Keep automatic OOM termination disabled until
  all provider and transaction ownership gates are met.
- [x] v891: full-system *permanent* FILE provider refusal containment.
  Genuine APR fault, copied-kernel VFS-sync rejects FD 0 indefinitely,
  while an independent user runs. Through 02400 octal clock ticks the
  private owner performs 11 rejected passes/sleeps, retains FD and
  real ERR 5 diagnostic, and reaches the 01000 octal (512-tick)
  exponential backoff cap with no forced release or premature exit.
  Actual PDP-6 SIMH PASS, no DAIMOS changes or additional kernel RAM.
- [ ] v891 follow-up: classify and repair invalid/missing/type/owner MM
  extents and other permanently failing FILE providers in their owning
  subsystems. A saturated retry delay does not authorize forced free.
  Define what an administrator can safely do without invalidating
  outstanding provider references. Automatic OOM termination remains
  disabled until complete allocation-free teardown is guaranteed.
- [x] v890: real full-system post-commit EXEC fatal FILE failure
  recovery. Valid user `EXEC` enters true low-memory fallback
  after injected pre-stage NOMEM; the production VM destroy
  commits `VM_SPACE_NONE` before test-only replacement-load
  rejection. Simulated VFS close fails twice; private kernel
  continuation retries after 1/2 ticks, retains FD and recorded
  ERR 5, clears witness on success, and parent WAIT returns
  fatal EXEC status 1. Booted PDP-6 SIMH PASS, no production
  code/data changes (KCORE 9,300).
- [ ] v890 next: investigate permanently refusing FILE/MM
  providers while another user remains runnable; retention,
  diagnostic availability, bounded CPU usage, and a safe
  administrative remedy. Do not enable automatic OOM killing
  without proving resource-lifetime and admission guarantees.
- [x] v889 follow-up: booted post-commit EXEC fatal failure
  acceptance completed in v890; APR path accepted in v889.
- [x] v889: full-system, actual user APR fault to private-stack
  fatal FILE cleanup acceptance. Disposable test kernel injects
  -5 from VFS-sync on the first two FD-0 close attempts.
  SIMH PASS: 1 real APR fault, 3 VFS calls, 2 rejections,
  3 fatal close passes, 1/2-tick sleeps, FD retained across
  failures, ERR class 5 visible, FD/witness cleared on
  success, correct parent WAIT and clean HALT. Zero production
  code/data changes; KCORE remains 9,300 words.
- [x] v889 EXEC follow-up: booted PDP-6 post-commit fatal
  provider-failure acceptance completed in v890.
  Persistent rejection diagnosis/recovery and the admission
  policy for automatic OOM termination remain open.
- [x] v888: preserve the first actual rejecting FILE provider
  result and FD during private fatal close using otherwise-unused
  u-area word 067. Expose a compact 0..6 result class through
  existing GETTEARDOWN detail bits 15..17 and a new numeric
  `ps ERR` column. A full successful close clears the record;
  nonfatal FILE closes leave it untouched. MM mismatches remain
  distinguishable through the v887 exact-extent snapshot. Native
  source/negative tests PASS. Real MM allocator busy/invalid
  distinction test PASS and full-system ordinary session/HUP/TTY
  regression PASS. KCORE +26 text words, no static data.
- [x] v888 transient FILE regression: proven in a full-system
  private-stack APR fatal continuation (v889), followed by an
  irreversible EXEC continuation (v890).
  Continue investigating invalid MM owner/type metadata failures
  rather than forcibly reclaiming such allocations.
- [x] v887: add on-demand exact provider diagnostics to the existing
  GETTEARDOWN(pid) syscall using PID flag 0400. FILE stage reports
  count, first remaining descriptor and VFS/PIPE/DPY class. MM
  stage reports verified exact pin count or missing/type/owner
  inconsistency. The existing stage/retry query is unchanged.
  Native PDP-6 tests PASS, zero new permanent RAM; KCORE +72
  text words over v886. No forced owner release is permitted.
- [ ] v887 follow-up: diagnose actual rejecting provider operations,
  not merely snapshots of surviving references. A persistent
  mm_free MM_ERR_INVAL needs a different recovery investigation
  from a legitimate pin. Automatic OOM killing remains disabled.
- [x] v886: user-visible readonly teardown state, via
  `procctl(GETTEARDOWN, pid)` in unused UUO-077 sub-op 041 and `ps`
  TD/RETRY columns. Distinguish normal/zombie (0), private
  fatal FILE owner (1), and deferred VM-free MM owner (2 with
  the existing shared 16..512-tick retry delay). No new
  permanent data or privileged forced-release operation.
  Native PDP-6 leaf/dispatch tests, KCORE/PS builds, and
  full-system actual-UUO/MM_BUSY/spinner/HUP tests PASS.
- [ ] v886 administrative follow-up: no claim that a provider is
  permanently failed can be made solely from a stage or retry
  interval. Investigate provider-specific recovery and diagnostic
  checks before allowing any automatic OOM victim termination.
- [x] v885: bound fatal FILE-close and quarantined MM u-area retry
  frequency without discarding any owned resource. FILE sleeps
  exponentially from 1 to 01000 octal ticks on its private stack.
  The shared MM reaper uses the existing last-service word for a
  020..01000 octal tick exponential delay, reset for a new owner
  and cleared when no pending u-area remains. Native tests and the
  real full-system transient MM-busy spinner regression PASS.
  No permanent RAM growth; KCORE +30 words over v884.
- [ ] v885 remaining: permanent MM/FILE provider rejection cannot
  guarantee eventual reclamation. Define diagnostic/administrative
  recovery policy without an unsafe forced free. Automatic OOM
  victim killing remains disabled.
- [x] v884: slot-0 MM cleanup is now eligible for one throttled
  maintenance turn after 020 octal timer ticks despite another
  runnable user; runnable realtime and disk-swap service retain
  priority, while dormant RT leases cannot starve cleanup.
  Pending is packed into the existing last-reaper-tick word, no
  permanent data growth. Native scheduler fairness and timer-wrap
  test PASS; pre-v884 negative control FAILs as expected. Full-system
  real MM-busy injection with a continuously spinning companion
  PASSes (reaper runs twice with nonempty run queue, 2 busy frees,
  1 pin/unpin pair and normal HUP/TTY completion).
- [x] v884 follow-up: permanently refusing MM/FILE providers now
  have bounded retry frequency in v885. They still retain ownership,
  so reclamation itself remains explicitly unbounded.
- [x] v883: non-current fatal HUP returns success once VM destruction
  commits and u-area release is explicitly deferred under the
  TRANSITION/VM_NONE/word-067 marker; duplicate events and precommit
  failures remain rejected. Native PDP-6 test PASS; old code FAILs
  the negative-control fixture. Full-system non-current MM-busy
  injection PASS (3 frees / 2 busy / 1 pin / 1 unpin / no error);
  the older current-EXIT injected test also PASSes.
- [x] v883/v884 follow-up: prove reaper progress alongside an
  independently spinning user in a booted guest (v884 PASS).
- [ ] v883 follow-up: genuine pre-VM fatal-event error propagation
  in a booted guest and permanent MM/resource-provider failure policy.
- [x] v882 full-system transient MM_BUSY test for process-owned EXIT:
  real u-area MM pin, two rejected frees, clock-yielded idle cleanup,
  one unpin, and eventual normal session/HUP/TTY HALT PASS. All
  instrumentation stays in the disposable daimos-testkit kernel
  overlay, zero production resident words.
- [x] v882/v883 follow-up: non-current fatal teardown now reports
  committed delivery correctly, and a full-system MM-busy/HUP
  fault-injection regression PASSes. Sustained continuously runnable
  companion progress and permanent-MM-error policy remain open
  under the v883 follow-up above.
- [x] v881 defers a failed post-VM stable u-area MM free to the
  clock-throttled slot-0 reaper, using unused u-area word 067 for
  marker/status. No per-process queue, unchanged session bits,
  one new permanent retry-clock word. Native retry/compiled EXIT
  and old teardown tests PASS; v882/v883 now demonstrate genuine
  full-system MM-busy recovery for current and non-current victims.
  Continuously runnable competing-user PI6 fairness remains open.
  The idle-only mechanism is not a bounded OOM killer.
- [x] v880 makes disk-swap detach ownership-preserving under a
  missing/invalid backing span: the complete bitmap run is verified
  before free, and failed detach prevents VM_SPACE_NONE publication.
  Native VM and actual C detach tests PASS with expected failure
  against pre-v880 code. The SWAP MRES verifier is reused through
  a patched KCORE entry (+16 resident words); full-system ordinary
  session HUP/TTY regression also PASSes. Post-VM u-area failure is
  handled by the v881/v882/v883 deferred-finalizer series.
- [ ] v879: APR and post-commit EXEC now use a process-owned kernel
  FILE-close continuation with timer-yielded retries; duplicate fatal
  events, native direct swap-out/exchange and OOM selection exclude
  that owner. VM_SPACE_NONE (-1) is now correctly excluded by native/C
  scheduling unless it has a tagged executive continuation. Native
  selection, direct swap guard and APR-safe restoration PASS; the
  pre-change swap guard FAILs the negative control. Obtain genuine
  full-system failing-provider/PI6/clock acceptance (the ordinary
  full-system session regression already PASSes), bound *total*
  cleanup latency or define unrecoverable-provider policy, and fix
  post-VM-destroy finalization errors before enabling OOM termination.
- [x] v878: post-commit fatal EXEC now enters FILE cleanup; v879 moves
  it onto the process-owned retry continuation. Native EXEC and prior
  pre-v878 negative control passed. Full-system injected failure stays
  open under v879, not under this completed wiring prerequisite.
- [ ] v877: ordinary full-system session teardown PASSes on the post-v876
  code under headless CTY/DPY test isolation. This does *not* validate
  injected HUP/VM failures during the full-system run or guarantee
  asynchronous cleanup after a nonreturning APR/EXEC fault.
- [ ] v876 now excludes a victim from scheduling before possibly sleeping
  session-HUP callbacks, and preserves RT ownership on failed HUP or VM
  destruction. Native failure/rollback test PASS; old-code negative
  control FAIL as expected. Prove remaining teardown stages can either
  complete or keep a nonreturning process safely quarantined before
  enabling any OOM kill.
- [ ] v875 delays controlling-TTY detach until all session HUP calls
  succeed; failed HUP leaves original attachment for retry. Native
  fault-injection and pre-change negative control PASS/expected FAIL.
  Earlier members may still exit before a later HUP failure, so audit
  recursive teardown and nonreturning fatal EXIT before OOM killing.
- [ ] v874 closes the Type 340 replacement old-extent ownership leak.
  Native transactional rollback tests PASS, including a failing old-code
  negative control. Still verify replay after an injected free failure
  in a full-system DPY guest, and explicitly test the MM invariant HALT
  when even the new allocation cannot be rolled back.
- [ ] v873: the KCC full-system named-FIFO workload times out both
  before v870 (v869) and on v871 after D6FS startup. Diagnose guest
  progress and the failing stage; do not weaken the test or claim the
  v870 native MM-rollback check provides full FIFO acceptance.
- [ ] v871 fixes DPY CLOSE error propagation and loss of the Type 340
  raw-list base after failed MM release (native fault injection PASS).
  Verify native refresh after a failed stop before declaring Type 340
  teardown fully allocation-free and retry-safe.
- [ ] v870 fixes final PIPE/FIFO close retry after an MM free error;
  native fault-injection PASS. Audit and prove remaining release helpers,
  and test full process cleanup with PIPE descriptors before activating
  OOM victim termination.
- [ ] v868-v869 now preserve current-process FILE resources on ordinary
  close failure and reject `proc_finish_slot` while any FD remains live.
  Native PDP-6 branch tests PASS. Resolve nonreturning APR/EXEC fatal exits
  and other cleanup failures without orphaning resources or hanging before
  using this path as an OOM killer.
- [ ] v867 closes a resource-orphaning hole in fatal-other cleanup: a failed
  VFS sync now prevents freeing the victim u-area. It does not guarantee
  OOM progress. Finish the allocation-free termination graph, including
  failed/sleeping provider operations, current-process exit and nested
  session HUP, before enabling automatic kills.
- [ ] Apply the same allocation-free, static KCORE OOM policy to **every
  memory allocation or growth requested by an already-running process**,
  including BRK/heap, process-VM expansion, and other user-triggered MM
  allocations. Do not limit OOM recovery to EXEC/SYS_RUN admission.
- [ ] First retry normal allocation after packing/compaction and permitted
  reclaim. If sufficient realizable, aligned free RAM exists but packing
  fails, fix the allocator regression; do not use OOM to hide it. Measure
  available words, alignment, pinning and fragmented gaps before deciding.
- [ ] If the running request still cannot be satisfied because resources
  are genuinely exhausted, invoke the **minimal static KCORE OOM path**.
  Terminate the requesting process when required rather than letting it
  spin or leave a half-updated address space. All kill, cleanup, backing
  release, parent notification, and zombie/WAIT bookkeeping must run
  without new RAM, new backing sectors, dynamic buffers, or removable
  modules; never swap in the victim merely to terminate it.
- [ ] Bound victim selection and retry attempts, preserve protection for
  INIT, kernel, current in-flight transitions, pinned or privileged critical
  processes where applicable, and define the requesting-process fatal case
  explicitly. Verify failure atomicity and zero-allocation teardown with
  native daimos-testkit fault injection for both RUN and live BRK/heap
  expansion, with RAM-only, backing-only, simultaneous exhaustion and
  no-killable-victim tests. OOM must not require a victim-held provider
  lock to make progress.


The **fresh GCC linked/package sizes** for the current source are:

| PDP-6 profile | KCORE | KCORE + packaged MRES | Permanent endpoint | Below `040000` |
| --- | ---: | ---: | ---: | ---: |
| 32K lowmem TSFS/tape | 8548 | 13463 | 13510 | 2873 free |
| 32K lowmem D6FS/disk | 8548 | 13539 | 13586 | 2797 free |
| 256K full/auto | 8548 | 18271 | 18318 | **1934 over** |

The full/auto `permanent-size` target **fails** its `040000` limit; its
multi-root endpoint is 18351 (1967 over). Package size includes optional
MRES components that may not be simultaneously installed; it is **not** a
measured runtime RAM high-water mark. Dynamic MM allocations, actual loaded
MRES combinations, stacks, and process footprints must be measured separately.
The strict lowmem KCORE `<020000` target still requires at least 357 words
removed (current KCORE 8548 versus octal 020000 = 8192).

- [ ] Resolve or correctly gate the **full/auto packaged permanent-image
  overflow** (1934 words single-root, 1967 multi-root). Audit which MRES
  packages really require permanent residency before changing the loader;
  do not hide genuinely always-resident RAM as a nominally removable module.
- [ ] Measure runtime physical MM allocations and peak working set for both
  lowmem roots and the actual full/auto loaded-module combinations; keep
  linked-package totals separate from peak resident RAM.

### Remaining XIP integration (active checklist; v201-v219 tests completed)

- [ ] Add an **opt-in root-only** RUN/EXEC admission branch for the distinct
  raw MEMFS XIP format (`PURE` header, mode no-write, exact aligned resident
  base), using MRES op `027` and retaining its pin through process lifetime.
  Ordinary DXR/PURE executables must continue through the normal loader.
- [ ] Enforce the **no-user-memory-write contract**, not just a PURE header:
  only admit the restricted verified instruction/exit class, refuse any
  unsupported or mutable image, and explicitly define the credential/trust
  boundary. Never treat a user-writeable PURE file as trusted executable.
- [ ] Add no-stack/no-argv/no-environment XIP startup/entry; bypass the
  ordinary `vm_space_startup` write into user memory, initialize APR and
  scheduler-owned state without modifying shared MEMFS bytes. Reject
  requests needing a private stack, data, BSS, or writable brk.
- [ ] Keep XIP mappings unswappable and non-growable. Verify MM pins,
  credential inheritance, provider unmount/reclaim resistance, scheduler
  transitions, and the existing process-descriptor provenance under load.
- [ ] Complete failure-safe RUN/EXEC replacement (admission rollback,
  no partial process publication, old/new image replacement, unpin/retry
  on failures), plus normal EXIT/wait/reap teardown via the tested VM path.
- [ ] Add **actual production-kernel** SIMH and PDP6-AAP tests for root
  launch, scheduled user instruction fetch, syscall EXIT, WAIT/reap,
  reuse/unmount, denied non-root/XIP-bad-image launches, swap/BRK refusal,
  and EXEC success/failure rollback. Existing v216 tests use stubs, not
  a scheduled production process. Measure KCORE and dynamic RAM on both.
- [ ] Keep XIP disabled for production until these tests pass; 32K lowmem
  must not acquire an automatic MEMFS dependency just to enable XIP.

### P0: efficient physical RAM management using native PDP-6 APR (pre-sweep gate)

POLICY REVISED 2026-10-27 v522: 512-word process mapping is no longer a
target. Preserve the PDP-6 APR's native `02000`-octal (1024 decimal word)
relocation and protection granularity. Never bypass, simulate or weaken APR
to obtain smaller process extents. General MM allocation is already exact
word-granular. Optimize measured physical RAM utilization, allocation
placement, fragmentation, compaction, extent lifetime and reuse, with CPU
cost as the next criterion; reject added metadata or indirection without
proven net benefit. Native 1K process granularity is an accepted outcome.

V524 SEQUENCING (2026-10-27): current fragmentation readings are useful
as correctness diagnostics, not final optimization evidence. First validate
native APR isolation, MM ownership, pin/release, compaction and transactional
swap/BRK failure recovery without resident telemetry. BSTORE, production XIP
and compressed swap change occupancy and lifetimes. After those features
are accepted, measure representative 32K/48K/256K physical high-water,
free gaps, APR rounding and fragmentation and only then optimize placement
or compaction based on net physical RAM and CPU measurements.

SOURCE AUDIT 2026-10-27 v521: `mm_alloc()` already requests precise word
counts with alignment 1; `mm_alloc_aligned()` and the extent descriptors do
not impose a global 02000-word allocation quantum. `vm_pdp6.s` and
`vm_pdp6_swap.c` enforce 02000-octal alignment for user APR images.
Therefore a global 512-word allocator conversion would be a regression,
not an optimization. Next measure rounding waste per user VM and enumerate
any software-managed consumers that actually overallocate at 02000 octal.
Only implement reductions where supported by hardware and measurements.

- [ ] After BSTORE/XIP/compressed swap integration, measure physical allocation high-water, total free words, largest free
  gap, process APR rounding waste and extent-descriptor overhead on 32K,
  48K and 256K workloads in daimos-testkit.
- [ ] Evaluate placement and compaction against measured fragmentation;
  eliminate only proven redundant allocations or retention. Keep APR
  process granularity intact and avoid new persistent metadata.
- [ ] Run protection, swap, BRK, XIP, reclaim and compaction regression
  tests after any accepted change; record total resident RAM and CPU cost.

## Pre-final-KCORE functional freeze inventory (2026-10-27 v512)

This is the authoritative functional checklist before the final linked-image
RAM sweep.  Historical unchecked v13-v21 mount and v208-v219 XIP substeps
below are evidence, not independent commands to reimplement accepted work.
`SIZEOPT.MD` tracks size-only experiments; `KCORE.MD` and
`PROJECT-STATE.MD` record accepted chronological evidence.  Check source
and testkit immediately before implementing an item.

Release-critical or prerequisite correctness/acceptance:

- [x] Regression row 028 (v580): `kernel-runtime` and `stand-runtime` PASS.
- [x] Regression row 029 (v581–v583): D6FS HDD runtime, two/four-member
  HDD tests and dual-superblock recovery PASS. Existing multimember fixture
  omitted external DSH commands; accepted v582 testkit fixture packages
  nine required binaries without altering the kernel or weakening assertions.
- [x] Regression row 030 runtime-build (v589-v590): the dedicated native
  v586-v588 SIXBIT pathname accidentally encoded `/MONITOR/DEVICES/DTY0/STATS`
  rather than `CTY0/STATS`. Corrected v589 OPEN/READCHAR/CLOSE test passes
  on PDP-6 SIMH. The v590 probe manifest preserves all 65 valid command
  probes (65 PASS, zero FAIL), replacing only the inapplicable CAT/STATS
  oracle with mandatory native character-stream acceptance in runtime-build.
  Complete runtime-build PASS; no kernel source change or optimization.

- [ ] **Preserve and diagnose every future `?RT` boot failure before
  rebuilding/resetting media.** Save SIMH transcript, exact boot image and
  mutable media, selected-root/handoff evidence and filesystem state;
  distinguish root selection, root mount/validation, and missing/nonregular
  `/SYSTEM/INIT`. Reproduce against the failing image and repair DAIMOS if
  defective. A clean-image rebuild is only a comparison, not a fix.

- [ ] **P0 efficient native-APR memory management**: preserve 1024-word
  PDP-6 process mapping alignment, measure real waste and fragmentation,
  improve placement/compaction/retention only where total RAM decreases,
  and verify execution cost and regressions in daimos-testkit.
- [ ] Settable UTC wall clock without PCLK: privileged SETTIME, DATE SET,
  safe PCLK presence detection, APR 50/60-Hz calendar accumulation, atomic
  GETTIME/SETTIME, and TIME36 timestamp preservation.
  Native milestone v592-v593: absent SIMH PCLK now produces unknown TIME36
  rather than a fabricated date; native PCLK-on/off GETTIME tests PASS.
  v595-v599: shared TIME36 service now falls back to APR-tick-driven UTC
  when PCLK is absent; root-only SETTIME validates TIME36 and rejects
  attempts to modify physical PCLK. Native PDP-6 SETTIME acceptance PASS
  in both PCLK modes. SETTIME extension opcode moved from the conflicting
  PROCCTL SETGID 025 to unused 037. Native ROOT LOGIN on an unset clock
  invokes DATE, initializes UTC, then enters DSH (testkit v597 PASS).
  Native v604 leap-day rollover, invalid calendar and non-root SETTIME
  denial PASS in both PCLK modes with restored pristine boot media.
  The targeted v602 SETTIME handler size pass saves three linked lowmem
  words (KCORE+MRES 15983 -> 15980), with no new data structure.
  50-Hz configuration, concurrency/timestamp stress and physical-device
  presence policy remain acceptance work; repeated-boot ?RT forensics
  are tracked separately.
  v606 source audit: PDP-6 APR CONI exposes clock flag/enable, **not**
  line frequency. SIMH's `rtc_tps` is simulator-internal. No automatic
  50/60-Hz discovery is possible from APR ticks alone; use a verified
  independent reference or explicit platform/boot configuration instead.
  v607: `WALL_CLOCK_HZ=50|60` hardcodes the operator's installation-time
  choice in the software UTC accumulator, default 60. Both cross-builds
  and lowmem links pass; invalid rate rejected. Native v608 configured
  50- and 60-tick UTC rollover/privilege tests PASS. Physical 50-Hz
  accuracy and system-wide scheduler/device timeout rate audit remain
  open; SIMH's native APR source itself stays 60 Hz.
  Native v609 D6FS mtime regression PASS at 60 Hz, both PCLK modes;
  verifies timestamp propagation after SETTIME and leap-day rollover.
  Native rerun with WALL_CLOCK_HZ=50 PASS in both PCLK modes as well.
  Source concurrency audit found PI-masked snapshots and guarded
  publication; native v612 multiprocess GETTIME/SETTIME race PASS at
  50 and 60 configured Hz, with three concurrent reader processes and
  a privileged clock writer. Native v610 MEMFS mtime
  regression PASS (both PCLK modes, leap-day rollover); `memfs_touch_slot`
  now preserves caller-scratch AC5 across the software clock gateway.
  Native v610 rerun at WALL_CLOCK_HZ=50 PASS in both PCLK modes too.
- [ ] Finish tested, non-destructive AUXSTORE swap provisioning (MKSWAP),
  including explicit destructive confirmation and existing-format/overlap
  protections. No automatic formatting at boot.
- [ ] Complete combined tape+drum, disk+drum, BLOCKSET member and non-root
  storage inventories, including other safely discoverable DRM units, and
  real supported mixed-provider mount/capacity tests.
- [x] Revalidate swapped-process MonitorFS NAME/CMDLINE/ENVIRONMENT reads
  and subsequent wake/WAIT: fresh 48K PDP-6 SIMH testkit v562 PASS
  (2026-10-28). The earlier v74 restore stall no longer reproduces.
- [x] Recheck GCC multiprocess timer preemption using the unchanged native
  five-user scheduler test with SIMH rate 8M: 2026-10-28 fresh run PASS,
  all users start before slot-1 checkpoint `A`; process-context and disk
  sleep/wakeup assertions also pass. No test-oracle or kernel modification.
- [ ] Rerun remaining GCC/KCC grouped acceptance; the focused GCC
  preemption PASS does not establish that the complete groups pass.
- [ ] Complete additional KINIT failure-path stack reserve checks;
  32K kernel-only and 96K/256K full-profile DSH acceptance are PASS.
  Repair stale MM
  testkit linking and secondary DTFS/TSFS mount test assumptions.
  v638 recovered 30 transient KINIT words: 32K image end `077236` ->
  `077200` at unchanged octal `0600` stack reserve. Native TSFS+AUXSTORE
  v493 and independent PDP-6 32K diagnostic/INIT boot PASS; the image
  exactly meets the limit without extra margin. Failure-path stack
  watermark, kernel-only and other-profile acceptance remain open.
  v639 fixes the watermark fill/scan exclusive-endpoint off-by-one; a
  native 48K watermark-enabled boot PASS reached INIT V1. The 32K
  failure-path stack high-water measurement remains outstanding.
  v640 adds an instrumented fatal-HALT stack sample. A native 48K
  invalid-root-selector test measured octal `0164` (116 decimal) words
  against the unchanged octal `0600` (384 decimal) reserve. Other
  failure paths remain open. v641 native 32K kernel-only boot PASS:
  SIMH breaks at the map-derived `proc_boot_init` entry after KINIT
  initializes DTC/DSK/DTFS, before running `/SYSTEM/INIT` or DSH.
  v642 additionally verifies the same instrumented lowmem KINIT image
  at 48K, 96K and 256K core: all three native SIMH kernel-only runs
  PASS through the process-bootstrap boundary. These are not full
  profile or DSH functional acceptance tests.
  v643 native GCC 96K full-profile disk-root KINIT PASS: image end
  `107421` octal, full device markers and `proc_boot_init` breakpoint.
  The 96K userland/DSH, KCC and 256K full-profile gates remain open.
  v644: 96K KCC full-profile disk-root KINIT also PASS, with image
  end `113573` octal and process-init breakpoint at `066510`. KCC
  96K kernel-initialization gate is closed; userland/DSH and 256K
  functional gates remain open.
  v645: native 256K KCC full-profile KINIT PASS; exact memory banner,
  DSK/DRM and filesystem markers and process-init breakpoint verified.
  256K GCC and interactive DSH functional acceptance remain open.
  v646 closes the 256K GCC full-profile kernel-only boot gate with a
  native SIMH PASS at `proc_boot_init` (`063067`), image end `107421`.
  Full-profile 96K/256K GCC/KCC kernel initialization is complete;
  interactive DSH and remaining KINIT failure paths stay open.
  v647: interactive ROOT login, DSH ECHO, external TRUE, STATUS:0,
  orderly HALT, and postflight CLEAN D6FS all PASS for GCC and KCC
  at both 96K and 256K. Source media must be CLEAN before reuse;
  a stop at `proc_boot_init` leaves writable root DIRTY, so it is
  unsuitable as a second-boot fixture. See
  `design/kinit-dsh-full-profile-v647.md`.
  v648: instrumented 48K tape-root boot with explicit disk selector
  `1` reaches HALT with stack high-water `0164` octal (116 words),
  below the unchanged `0600` reserve. This adds selector coverage;
  independent D6FS mount and MM allocation failures remain open.
  v649: independent D6FS superblock-validation failure is now measured:
  native 96K GCC full-profile boot with both superblock magic words
  invalidated in disposable disk copies emits `?RT`, HALTs and records
  `0172` octal (122 words) against `0600` reserve. Allocation, I/O
  and missing-INIT error paths remain unmeasured.
  v650: targeted native KINIT idle-stack `mm_alloc` failure PASS:
  inject only an impossible request into simulator RAM after Stage1
  loads, then measure fatal HALT high-water `0172` octal (122 words)
  against `0600` reserve. Disk media remains CLEAN. Independent
  I/O and missing-INIT fatal paths remain open.
  v651: BLOCKSET read-service I/O error injection during full-profile
  D6FS root mount PASS: `?RT`, HALT, `0172` octal (122-word) stack
  watermark, CLEAN source media before/after. Physical controller
  errors and absent/nonregular `/SYSTEM/INIT` still need coverage.
  v652: KINIT bootstrap reserve reduced from `0600` to `0400` octal
  (256 words); prior failure-path high-water maximum is `0172`
  octal (122 words). Fresh native 32K lowmem watermark-enabled image
  boots through `proc_boot_init`, and invalid-root HALT records `0164`.
  v653: missing and nonregular `/SYSTEM/INIT` acceptance PASS after
  successful D6FS initialization on native PDP-6 SIMH, using clean
  disposable disk copies. Physical-controller errors remain a
  separate coverage item.
  v655: physical DSK0 media-loss after Stage1 now PASS in native 96K
  GCC full-profile SIMH: KINIT HALT, validated watermark `0172`
  octal (122 words) inside the `0400` reserve. A prior `0577` reading
  was invalid because a reused build mixed watermark-disabled KINIT C
  with watermark-enabled assembly. KINIT reserve/watermark changes now
  invalidate both generated files; mode-toggle 1/0/1 regression PASS.
  Other transient physical-controller error modes remain separate.
  Remaining `/SYSTEM/INIT` validation is separate from this size change.

Architecture-changing features requiring an explicit pre-sweep decision:

The active KCORE decisions are 1, 3, 4 and 5; decision 2 is explicitly
POST-KCORE. An unchecked active item means implementation/acceptance is
outstanding, NOT that scheduling is undecided.

- [ ] **DECISION 1 ACCEPTED (2026-10-27):** Complete the D6 codec production
  streaming low-core decoder and atomic producer/consumer cutover before the
  final KCORE sweep. This is a version 0.9 release prerequisite, not an
  optional future experiment. Keep the old D6LZ36 system bootable until
  the new format passes native regression; do not ship mixed formats or a
  permanent legacy decoder. The decision is recorded, NOT implemented.
- [ ] **DECISION 3 ACCEPTED (2026-10-27):** Complete the remaining BSTORE
  migration to generic shared backing/BCACHE/LOGSTORE BEFORE final KCORE
  optimization as a version 0.9 prerequisite. Audit remaining gaps against
  the accepted AUXST1/BACKSTORE implementation; preserve raw AUXSTORE and
  existing D6FS-root validation contracts. Verify all production consumers,
  removable MRES boundaries, dynamic RAM and native rollback/recovery before
  freezing the service ABI. Decision recorded; implementation not complete.
  v657 first feature increment: shared bounded raw AUXSTORE region
  read/write interface for BACKSTORE/LOGSTORE/CACHESTORE landed;
  LOGSTORE bootstrap I/O now delegates through that interface.
  Host partition-bounds regression and earlier AUXSTORE choice/overlap
  checks PASS. BACKSTORE conversion, recovery and native acceptance
  remain open; no KCORE optimization or completion claim.
  v658: direct DEVICE regions are now named/configured separately from
  multi-member VOLUME tails, keeping the legacy BLOCKSET entry alias
  and one shared physical DSK/DRM transfer path. Source routing
  regression PASS. Remaining full storage feature cutover is open.
  v659: a configured multi-member VOLUME now fails I/O if its mapper
  is unavailable; it no longer silently falls back to DEVICE 0
  addressing. Source regression and fresh native 32K D6FS boot PASS.
  Native missing-mapper fault injection remains open.
  v660: the existing three resident backing-region state words now
  have explicit DEVICE/VOLUME/REGION source names, while legacy
  symbol aliases share exactly the same storage and entry point.
  Process swap boot uses `backing_region_blocks()`.
  v661: LOG boot I/O directly selects the bounded AUXSTORE LOG REGION;
  redundant LOG-only AUXSTORE wrappers were removed. Legacy root-backed
  LOG remains supported. Persistent LOG recovery and native fault
  injection remain independent acceptance work.
  v662: MonitorFS SWAP statistics uses `backing_region_blocks`, as
  process swap already does. Legacy BLOCKSET query/transfer symbols
  remain ABI aliases; runtime mapping fault injection remains open.
  v663: native multi-member D6FS HDD suite PASS on two-/four-member
  volumes after rebuilding AArch64 host relay helpers. New testkit
  v663 launcher builds versioned native helpers and runs the unchanged
  v582 suite.
  v664: targeted native PDP-6 BACKING gateway test PASS: unavailable
  multi-member mapper fails without device I/O; direct DEVICE still
  succeeds. Full-system in-flight failure tests remain outstanding.
  v665: native PDP-6 partial BACKING write test PASS (second write
  fails, third never issued). Swap/MEMFS publication-order source
  regression PASS. Full-system in-flight fault and late MM rollback
  remain open; BACKING media writes are not physically atomic.
  v666: late swap-out MM unpin/free error branches audited; source
  regression PASS. Real native MM failure injection and complete
  scheduler/recovery acceptance remain outstanding.
  v667: subsequent audit found `mm_unpin` errors were not handled
  safely by normal rollback. Both unpin failure paths now halt via
  the existing storage invariant instead of unlocking the process;
  v667 source check and native PDP-6 branch fault tests PASS.
  Full-system MM fault injection remains open.
  v668: isolated native PDP-6 `mm_free` failure after successful unpin
  PASS: resident process record preserved, provisional BACKING released,
  staging word and TRANSITION cleared. Full scheduler fault injection
  and live I/O recovery remain open.
  v669: full-system swap scheduler regression is BLOCKED: historic
  v298 expects `proc_swap_out` in KCORE rather than SWAP MRES; updated
  v669 simulator run reaches startup `?RT` and HALT at `065206` without
  workload completion. Determine root cause, then run fault injection.
  v670: native D6FS boot trace confirms pre-VFS failure after
  BLOCKSET discovery/reads and superblock select entry. Capture
  precise failing return/check and root media fields; do not
  infer swap failure or claim full-system rollback acceptance.
- [ ] **DECISION 4 ACCEPTED (2026-10-27):** Complete production raw MEMFS
  XIP BEFORE the final KCORE optimization as a version 0.9 prerequisite.
  Implement privileged non-writing EXEC/RUN, provenance, swap/BRK exclusion,
  unpin and rollback; validate scheduled process lifetime on native PDP-6
  simulators. Existing admission and isolated VM tests are not production
  process-launch acceptance. Decision recorded, implementation outstanding.
- [ ] **DECISION 5 ACCEPTED (2026-10-27):** Implement and benchmark opt-in
  compressed process swap BEFORE final KCORE optimization, restricted to
  explicitly allowed executable images (for example, the shell). Mark
  eligibility in a validated executable-header flag at EXEC; unmarked or
  unapproved images MUST use raw swap without attempting compression.
  Fork inherits the opt-in only as appropriate to the image contract; EXEC
  replaces it. Keep an independent swap-record compressed bit recording
  actual encoding, plus atomic failure/non-shrinking raw fallback. Confirm
  eligibility cannot be forged by untrusted executables or writable media.
  Benchmark PDP-6 ticks, compressed blocks, transient RAM and full linked
  resident size. Accept production only with no net permanent RAM growth
  and passing mixed raw/packed rollback and process tests in daimos-testkit.
  Decision recorded; not yet implemented or benchmarked.

### Deferred post-KCORE work: TSFS v1.2 (not a KCORE gate)

- [ ] **DECISION 2 ACCEPTED (2026-10-27; deferred v903):** After ALL
  KCORE feature completion, regression acceptance and KCORE optimization,
  implement TSFS v1.2 sharded metadata, reader, boot/format compatibility
  and native tests, before the 0.9 release. Keep TSFS-specific changes in
  provider MRES and host tools wherever possible; audit any necessary
  KCORE/VFS/KINIT ABI impact when the TSFS effort actually starts.
  Do not make TSFS v1.2 a blocker for KCORE completion or its size sweep.

Explicitly outside the 0.9 final KCORE functional freeze unless a verified
release dependency is found: new dynamic/unloadable MRES framework, user
overlays, runtime module movement, hardware pager,
Type-340 input/chess, PDP-6 TS 1.4 foreign DECtape, and full 32K DSH
command execution (separate DSH shrink effort). Never auto-load MEMFS
based on AUXSTORE presence. Do not treat KCORE `proc_swap`, `bcache`,
`fs_mres`, p10fold or dead-data audits as functional prerequisites.

The **online D6FS packer and other agreed storage/admin extensions are
required for 0.9**, not post-0.9. This release-scope correction does not
bring the deferred DTFS/TSFS work into the current KCORE feature sequence.
TSFS v1.2 is explicitly scheduled after completion of KCORE.
Implement these features before release acceptance; KCORE feature
implementation takes precedence over any new size-optimization sweep.

## TSFS-root auxiliary backing and command tools (2026-10-27)

### P-1 KCORE TODO reconciliation (v511)

Before beginning any unchecked KCORE item, inspect current source, latest
commits, linked maps and tests to decide whether it is already implemented,
superseded, or no longer needed. Preserve historical evidence in Git; do
not implement stale proposals. `SIZEOPT.MD` is the active, ordered RAM-size
worklist; `KCORE.MD` and `PROJECT-STATE.MD` are chronological evidence.

Run kernel-only 32K boot/initialization tests, including no-DSH device and
swap tests, at 32K. Functional tests requiring DSH use 48K, 96K or 256K.
The complete ROOT -> DSH -> command workload at 32K is explicitly deferred
to a separate DSH optimization run and is not a gate for this sweep.
Keep the KCORE below-020000 size goal as a tracked, deferred acceptance
milestone while exhausting verified no-growth RAM optimizations.

- [ ] DEFERRED: design explicit loadable MRES modules only after KINIT size
  optimization makes module loading feasible. Do not automatically load
  MEMFS when a backingstore is detected. The earlier automatic lowmem
  MEMFS proposal was cancelled on 2026-10-27.

- [x] Keep the 32K `lowmem` profile free of MEMFS: it currently omits the
  MEMFS MRES and `memfs_minit` through `KINIT_FULL=0`, and deliberately
  leaves `/TEMP` unmounted as MEMFS. Do not introduce a MEMFS dependency
  into tape-only optional-swap discovery.
- [ ] Extend the existing **DATE** utility (`userland/exec/COMMANDS`,
  `commands.c`, and `manual/DATE.SIXMD`) to **set** as well as display the
  UTC date/time. DATE must be able to initialize/correct wall time when the
  optional PCLK device is absent; require a privileged SETTIME operation,
  strict calendar validation (including leap years), and native 32K TSFS-root
  coverage. Preserve the existing no-argument display behavior.
- [ ] Add a **settable software wall clock** driven by the existing APR
  50/60-Hz scheduler-clock interrupt when PCLK is unavailable. Detect PCLK
  availability safely at boot in KINIT; never blindly issue PCLK I/O against
  absent hardware. Advance the UTC calendar clock using the actual tick
  frequency with correct subsecond accumulation and rollover (including
  day/month/year and leap years); handle concurrent GETTIME and SETTIME
  atomically. Keep the 50/60-Hz monotonic scheduler clock independent of
  wall-clock adjustments, preserve TIME36 encoding and existing GETTIME/
  filesystem timestamp semantics, and minimize KCORE resident RAM/CPU cost.
  Test both PCLK-present and PCLK-absent boots and both 50/60-Hz settings
  exclusively in daimos-testkit; document behavior after passing tests.
- [ ] Add **MKSWAP** as a userspace provisioning utility for deliberately
  selected DSK/DRM swap and optional LOGSTORE reserved regions. Provide a
  non-destructive inspection/dry-run mode, exact controller/unit/range
  identification, explicit destructive confirmation, and strong overlap and
  existing-format checks. No automatic formatting or overwrite at boot;
  INIT may offer an interactive provisioning action, but unattended boot
  must safely continue with no swap.
- [x] Reject unsafe AUXST1 overlap/range layouts without installing D6FS;
  preserve unformatted optional D6FS partition compatibility (v507-v510).
  D6FS-root dual-superblock and BLOCKSET validation already exists. The
  proposed generic KINIT-only superblock parser is **superseded**: add
  validation for a specifically selected non-root D6FS/BLOCKSET only when
  a real consumer needs it; do not parse D6FS for raw AUXSTORE.
- [x] Have KINIT select tape-only (no physical swap) versus tape with validated
  AUXSTORE (DSK/DRM), using existing resident service bindings and backing
  capacity rather than adding a new KCORE boot-mode word. Lowmem DRM was
  enabled in v506; the 32K DSK AUXST1 positive swap regression passed v507.
  The positive 32K TSFS+DRM0 AUXST1 swap/wake/WAIT regression passed v508.
  Continue separate combined-provider/BLOCKSET coverage below.
- [ ] Validate combined tape+drum, disk+drum and complete BLOCKSET member
  inventories when corresponding supported boot profiles exist; verify
  installed MRES and persistent dynamic allocation footprints separately.

## Deferred acceptance gate: strict 32K (not active priority)

- [ ] Pass strict GCC PDP-6 KCORE endpoint **below** octal `020000`.
  Accepted v507 lowmem KCORE = 8548 words, 357 words above the
  strict below-8192 target. Do not merely move required resident
  code to an always-loaded MRES to mask permanent RAM usage.
- [ ] Keep native 32K **boot without DSH** and KINIT stack/headroom checks.
  v503-v508 passed native 32K TSFS/AUXST1 tests with a 384-word KINIT
  stack; 129/136/201-word highwaters are not exhaustive failure-path
  maxima. DSH-dependent commands, filesystem and process-pressure tests
  run at 48K, 96K or 256K in this sweep. The full 32K ROOT -> DSH ->
  command acceptance is deferred to a separate DSH optimization effort.
- [ ] Repeat all size and correctness gates after each accepted source
  reduction; fix compiler/assembler/kernel correctness bugs first.

**Strict 32K acceptance is explicitly deferred; current storage work and
source-level RAM optimization continue independently.** See
`design/p0-strict-32k-gates-20261027-v474.md`.

## XIP v219 MRES gateway

- [x] Publish raw PURE XIP admission through existing private MEMFS MRES
  register gateway (op 027), tested on SIMH and AAP, with no new KCORE slot.
- [ ] Connect a privileged loader path only with zero argv/environment and
  no writes to the shared VM during RUN/EXEC startup.
- [ ] Test scheduled process EXIT and low-memory EXEC rollback end to end;
  reject any trust/pin failure before changing process VM state.

## XIP restricted linker producer v218

- [x] Implement an opt-in, distinct dlink raw XIP output mode reusing PURE
  and existing relocation resolution; reject writable/op-unsafe images.
- [x] Cover exact format, alignment padding, plain DXR compatibility,
  writable instructions, alternative UUOs, DATA/BSS, and invalid options
  in daimos-testkit host regressions.
- [ ] Add an opt-in **privileged** EXEC/RUN path for raw XIP with no argv,
  environment, writable startup, or stack; preserve ordinary PURE/DXR.
- [ ] Verify a production scheduled RUN/EXIT and EXEC replacement rollback
  on both PDP-6 simulators before declaring XIP runtime enabled.

## MEMFS XIP PURE reuse v217

- [x] Require the existing PURE flag in the distinct raw-XIP file header;
  reject absent PURE without leaving a pin. No new permanent process state.
- [x] Implement the restricted raw-image producer for statically pure code
  (v218), preserving logical origin 020 without loader-time relocation;
  its intentionally narrow instruction subset is not general PURE XIP.
- [ ] Add narrowly gated EXEC/RUN use only when both trust and no-writing
  runtime conditions are met; ordinary DXR PURE keeps its existing behavior.

## XIP v216 user-mode lifecycle

- [x] Native-test strict non-writing MEMFS-backed user instruction fetch,
  monitor-UUO handoff, and production VM teardown on SIMH and AAP PDP-6.
- [x] Verify image immutability, pin lifetime, VM state reset, and reclaim
  after termination without adding KCORE or per-process state.
- [ ] Test real scheduler process exit and EXEC replacement/rollback for a
  trusted XIP image, including storage-map state and swap interactions.
- [ ] Define and enforce the privileged no-user-write contract before
  enabling this distinct image format in production EXEC/RUN.

## XIP v215 native VM teardown

- [x] Native-test production `vm_space_destroy` on a pinned MEMFS VM
  descriptor with injected MM-unpin failure and successful cleanup retry.
- [ ] Exercise a full native process exit/EXEC replacement, including MM
  ownership checks and non-writing user startup, on both PDP-6 emulators.

## XIP v214 release rollback proof

- [x] Inject an MM unpin failure through the MEMFS MRES release dispatcher;
  verify retained reference, exact-base ownership, and successful retry.
- [ ] Test staged process VM teardown (not only its MRES dispatcher) and
  both EXEC commit/rollback, with a trusted non-writing user startup.

## MEMFS XIP teardown v213

- [x] Route VM destroy through the movable MEMFS provider's private release
  operation, falling back to MM owner/type validation for regular processes.
- [x] Native-test the release dispatcher and quantify KCORE growth (+9 words).
- [ ] Prove teardown of a staged XIP-backed *process VM* on both emulators,
  including EXEC rollback, then implement verified non-writing startup.
- [ ] Enable separately trusted process XIP launches only after lifecycle
  and swap/heap guards are covered by process-level tests.

## MEMFS XIP v212 provenance

- [x] Implement exact-base pinned MEMFS ownership lookup in MRES without
  a new process descriptor or persistent reference-table word; native tested.
- [x] Route VM destroy to MRES unpin for XIP-owned extents (v213), with unloaded-MRES
  and failure/rollback handling. Never free a MEMFS extent as MM_TYPE_PROCESS.
- [ ] Prevent XIP swap and brk changes; implement non-writing startup and
  actual privileged EXEC/RUN process lifecycle tests.

## MEMFS XIP v211 lifetime primitive

- [x] Implement native-tested exact-base reverse MEMFS XIP unpin for VM
  teardown without a vnode in the three-word process descriptor.
- [ ] Add explicit process VM provenance, swap exclusion and brk rejection,
  then call the reverse release instead of `mm_free(MM_TYPE_PROCESS)`.
- [ ] Validate non-writing startup and actual EXEC/RUN lifetime on PDP-6.

## MEMFS XIP admission gate v210

- [x] Require executable/non-writable file mode before XIP pin/admission;
  test mode 0000, 0444, writable mode, and successful 0555 on PDP-6.
- [ ] Establish process owner/provenance and MEMFS unpin on VM teardown;
  exclude XIP allocations from normal process swap and brk growth.
- [ ] Implement a guaranteed non-writing user startup/termination contract,
  then integrate a separately trusted XIP image path into EXEC/RUN.

## MEMFS user APR XIP validation v209

- [x] Validate PDP-6 user-mode APR instruction fetch and physical relocation
  from admitted, pinned MEMFS backing on AAP. Demonstrate that user stores
  also modify backing (test-only). Add repeatable testkit Make target.
- [ ] Implement safe XIP process provenance and teardown/unpin, swap
  exclusion, and non-writing startup without copying the executable.
- [ ] Test a strictly non-writing user program and syscall handoff from
  pinned storage on both SIMH and AAP before production EXEC/RUN admission.

## MEMFS XIP v208

- [x] Execute a real PDP-6 instruction from pinned MEMFS backing on SIMH
  and AAP using a testkit-only executive XCT trampoline.
- [ ] Add user-mode APR instruction-fetch/native process test with an
  explicitly non-writing startup and guaranteed teardown/unpin.
- [ ] Only after this passes enable privilege-gated production EXEC/RUN XIP.

## XIP integration gate v207 (2026-10-23)

- [x] Implement distinct MEMFS raw-image admission with stable pinned
  backing and rollback, native-tested on SIMH and AAP.
- [ ] Implement privileged non-writing image contract, VM MEMFS provenance,
  swap exclusion, and startup/teardown without shared-memory stores.
- [ ] Only then enable the new image kind through the existing EXEC/RUN
  loading interface and test native process launches on SIMH and AAP.

## Post-optimization MEMFS XIP safety gate v206

- [x] Audit optimized EXEC/RUN VM call sites and establish linked KCORE size.
- [ ] Implement restricted image admission and non-writing startup: current
  `vm_space_startup` writes into memory even for empty argv/envp.
- [ ] Add VM MEMFS ownership and unpin-on-teardown; reject ordinary swap,
  writable break growth, and mutable text before switching physical base.
- [ ] Only then connect the MEMFS XIP path and test actual launches on both
  native PDP-6 simulators. Do not substitute MEMFS base in ordinary DXR.

## MEMFS XIP v204 (2026-10-23)

- [x] Native aligned-allocation `MM_ERR_FRAGMENTED` failure injection on
  SIMH and AAP, followed by successful retry without leaked storage.
- [ ] Run real MM fragmentation/compaction pressure tests and measure wasted
  free words caused by `02000` alignment.
- [ ] Integrate safe XIP VM provenance, process teardown and image admission;
  do not enable ordinary DXR shared writable images.

## MEMFS XIP alignment milestone v203 (2026-10-23)

- [x] Request 02000 alignment for dedicated large MEMFS allocations; both
  SIMH and AAP MEMFS pinning target fixtures PASS.
- [ ] Test aligned allocation fragmentation under pressure, then implement
  executable admission and an XIP-aware VM teardown without changing normal
  EXEC/RUN behavior.

## MEMFS XIP implementation progress (2026-10-23 v202)

- [x] Add per-file XIP pin/refcount reservation with existing MM chunk pins,
  plus write/truncate/reclaim/unmount exclusion. Target MEMFS/FIFO fixture
  passes on SIMH and AAP; ordinary EXEC/RUN remains unchanged.
- [ ] Add dedicated `02000`-aligned MEMFS image allocation, image admission,
  VM provenance/lifetime and read-only execution contract; then validate
  native XIP process behavior on both SIMH and AAP. No general DXR XIP yet.

## MEMFS execute-in-place (XIP) requirement (2026-10-23)

- [ ] Support genuine zero-copy MEMFS executable launch: execute from the
  resident MEMFS backing allocation rather than copying the image into a
  separate process VM allocation. Treat this as a MEMFS design requirement,
  not as completed functionality.
- [ ] Determine a viable PDP-6 mapping strategy before implementing XIP.
  The current APR exposes one contiguous relocated/protected process space
  (02000-word-aligned physical base); it does not independently map shared
  text and private writable data. Do not claim shared-text XIP until private
  data, stack, and relocation semantics are demonstrated.
- [ ] Require executable-backend pin/refcount ownership, immutability while
  mapped, and release at exit/exec failure. Coordinate with MEMFS resize,
  unlink, reclaim/spill, swap, and process teardown; prevent dangling
  physical addresses and mutation of live executable instructions.
- [ ] Check executable-format alignment, header offset, relocation, writable
  text, RT-required mode, and compressed DXR2 behavior. Preserve conventional
  EXEC/RUN fallback for images not eligible for direct execution.
- [ ] Add native tests in daimos-testkit covering repeated/shared launches,
  simultaneous execution, unlink/truncate/write attempts, memory pressure,
  failed EXEC rollback, process exit, and swap interaction. Measure total
  RAM savings and KCORE/MRES word changes before enabling the path.

## V64 validation follow-ups (2026-10-20)

- [x] Swapped-process NAME/CMDLINE/ENVIRONMENT reads in MonitorFS:
  live 48K PDP-6 SIMH v562 verifies exact records for a swapped process,
  its wake, expected child exit and WAIT completion. Existing reader retained;
  no persistent per-process metadata added. The v74 wake/WAIT stall from
  2026-10-20 does not reproduce on the current source. This does not close
  the separate GCC preemption or grouped regression gates.
- [x] Validate the corrected three-root AUTO/login testkit fixture at 256K:
  all-media selects DISK, no-DSK selects TAPE, and drum-only selects DRUM.
  The three individual fresh-media probes pass; tape `CAT /CONFIG/SYSTEM`
  needs the measured 360-second allowance.  Testkit commit `ab730fe`.
  An atomic rerun of the updated three-case script is still separate work.

## GCC PDP-6 functional regression recovery (2026-10-19)

- [x] GCC group 004 fresh targeted run passes all five boot origins,
  four-member DSK root discovery, Stage1 KINIT entry, SIXBIT lifetime,
  and corrected compiler-codegen validation.
- [x] Restore MonitorFS online-device test: detach SIMH DTC3 as well as
  DTC0-DTC2 before disabling the controller. Direct GCC test PASS; all
  other originally failing group-023 subtests pass independently.
- [x] Repair D6FS host checker power-cut regression: current optimal repair
  uses exactly three durable writes, with CLEAN already published after
  cut 3. Verify DIRTY recovery, final CLEAN, no fourth write and ambiguous
  no-modification. Full group-032 host-tools suite PASS, testkit 11feecb.
- [x] Recheck complete regression row 023 (ten storage targets): v564
  repaired the MonitorFS device test's SDL2 crash by applying the existing
  headless SIMH policy, then the unchanged guest oracle passed. V565 ran
  the other nine row-023 targets in matrix order; all PASS (2026-10-28).
  This is row-level evidence, not full GCC/KCC matrix acceptance.
- [x] Recheck matrix row 024 (LOGSTORE and MTC516 online): both targets
  PASS in v566 (2026-10-28); no source or fixture changes.
- [x] Recheck matrix row 025 (DSK270 online and elevator): both targets
  PASS in v567 (2026-10-28); no source or fixture changes.
- [x] Recheck matrix row 026 (DRM236 capacity, online, MINIT/MRES, Stage1
  boot): all four targets PASS in v568–v570. The MINIT-only test fixture
  now uses supported `BOOT_PROFILE=lowmem ROOT=drum` rather than the known
  oversized default full profile; all original module checks remain.
- [x] Row 027 has all ten targets passing in focused, ordered continuation
  runs v573–v579 (not a single clean-sweep invocation). V573 proves
  native DTFS media survives
  two boots and passes second-boot FSCK; disk-root `MOUNT.DTFS` refusal is
  intentional because `dtfs_minit()` installs its MRES only when the selected
  root is DTC. Testkit v573 now asserts this exact supported contract.
  v575 updates the eight-member TSFS scanner regression for its current
  direct-result API (old SYS_TSFS handoff ABI retired). V577 verifies TSFS
  root directory lookup/listing/STAT on a real tape-root SIMH image, replacing
  the obsolete secondary `MOUNT.TSFS` oracle in the matrix. V578 verifies native DTFS format,
  FSCK, secondary mount, root STAT and unmount on a DTC-selected root;
  v579 verifies production reverse-direction single-block read/write.
  Retain strict complete-matrix release sweep as a separate gate.
- [x] Rework secondary DTFS mount acceptance to match the intentionally
  root-selected MRES package: v578 native secondary mount on tape root PASS;
  v573 disk-root mount refusal PASS. Retired MOUNT.TSFS secondary fixture
  replaced by v577 root-selected TSFS directory acceptance. No extra MRES.
- [x] Repair GCC local 9-bit char-array loads across 36-bit words at
  -O1/-O2/-Os; preserve the selected stack word rather than interpreting
  the word address as a byte offset.  Testkit v45 target execution passes
  at -O0/-O1/-O2/-Os, compiler commit df27247.
- [x] Retest full GCC DSH conformance with a fresh 256K disk image:
  115/115 PASS, including HELP ECHO and HELP DAS previously failing.
- [x] Update three outdated D6FS assembly test oracles, TSFS eight-member
  test compilation and SIXBIT headers, and the PDP-6 stack-adjustment
  codegen oracle.  All six focused cases pass, testkit commit 2804838.
- [x] Reproduce GCC multiprocess scheduler preemption on current code:
  the unchanged `daimos-scheduler-multiproc-v1.sh` returned PASS with
  `<abB2cCxxxxdDeEyyyyAxxxxyyyy314D5` (v563). All boot users run before
  slot-1's first `A`. The historical failing sequence is not reproduced
  using the runner's documented 8M SIMH throttle; no compiler/kernel fix
  is attributed to this observation. GCC/KCC group matrix remains open.
- [ ] Address remaining GCC and KCC disk-testkit group failures without
  re-running already passing groups.  32K and size gates are separate.

## Native MAKE V0.9 feature freeze (2026-10-11)

- Native MAKE is feature-complete for V0.9. No new features unless required
  for correctness or demonstrated native self-hosting compatibility.
- Retain SIXBIT-only uppercase parsing and serial execution; -J remains
  an accepted no-op.
- The final RAM sweep removes redundant MAKEFLAGS environment decoding;
  native SIMH verification is tracked in daimos-testkit.

## Native MAKE compatibility follow-up (2026-10-11)

- Recursive -N and inherited -I MAKEFLAGS pass on PDP-6 SIMH (15/15).
- Command-line variable override propagation is deliberately deferred.

- Maintain strict uppercase SIXBIT-native syntax; never add lowercase ASCII
  aliases to native MAKE.

- Target-side PDP-6 SIMH verification passed for .SUFFIXES reset/filtering,
  -r, -t, and accepted-but-serial -j/-J (7/7 probes).
- Target-side .SILENT, .IGNORE, AND -I PASS (10/10 TOTAL SIMH PROBES).
- Target-side .PRECIOUS preservation passed (13/13 TOTAL SIMH PROBES).
- Decide whether actual native makefiles require double-colon rules or
  recursive MAKEFLAGS propagation before accepting additional complexity.
- Verify suffix precedence/order with BSD Make and GNU Make test cases.
- Evaluate portable double-colon rule semantics and automatic MAKEFLAGS
  propagation for recursive native builds.
- Never add parallel execution for native MAKE: -j/-J are compatibility-only.

# DAIMOS TODO

## AAP PDP-6 boot and CTY work (2026-10-18)

- [x] V23B CTY PI4 10-byte packed FIFO: eliminate one-character mailbox
  overwrite; native PDP-6 FIFO and TTY routing tests pass.  AAP diagnostic
  boot with CTY-only INITTAB reaches DSH.  Permanent CTY MRES cost +54 words,
  including +2 resident data words.  Optimize only after proving equivalent
  lossless handoff and PI4 wakeup behavior.
- [x] V23C recover **default** three-session INITTAB startup on AAP without
  reducing the configuration: INIT tests the MINIT-published CTY/DCS/GE
  MonitorFS input endpoint and starts LOGIN only for detected controllers.
  ROOT reaches DSH; no failed respawn/auth lines, correct 255 audited tape
  blocks. C99 source-leaf and AAP evidence tests pass.
- [x] Retest AAP DSH external command execution after ROOT boot: v37
  headless AAP passes ECHO at 300.2 seconds and PWD at 548.2 seconds;
  the former 240-second command timeout was insufficient for DECtape seeks.
  The comparable SIMH Type-551 tape-root path also passes both probes.
  This is 256K functional acceptance only, not 32K or persistence testing.
- [x] Accept the Type-551 PI3 DECtape block-mark search/transfer rewrite:
  rebuilt SIMH with the corrected transport controller booted the 256K
  TSFS root, reached ROOT/DSH, and passed external ECHO and PWD (2/2).
  The independent PI3 driver/router commit is `2c85325`.
- [ ] Complete strict 32K residency/boot and combined-provider persisted
  media acceptance.  The 256K PI3 command test is not either of these gates.


## KINIT storage inventory and VFS mount sizing (2026-10-15)

- [x] V20: fifth PURE backing mount-slot bit in spare credential bit 27,
  transiently staged in SIDL scheduler RH bit0 until RUN allocates u-area.
  Full 18-bit index, 14-bit PURE text length and one-word swap record are
  retained. EXEC rollback, swap raw/compressed/verify, busy guards and mount
  26 native tests pass. No additional process words allocated.
- [x] V21 provider bound phase: replaced D6FS dispatch/DTFS SPACE fixed
  four-slot checks and removed optional TENEX/ITS four-word personality XCT
  table; validated mount 26 and invalid 27. No RAM growth.
- [x] V22 capacity activation: KINIT derives selected physical root members
  and installed provider/controller slots from MINIT state, calls the
  26-slot planner, allocates the exact six-array MM slab, then publishes
  `vfs_mount_limit` last. Native full/reduced inventory tests pass.
- [x] Historical V22 multi-provider experiment deferred post-KCORE (v906):
  exercising simultaneously mounted D6FS/DTFS/TSFS volumes above slot four
  requires an MRES provider matrix the current profiles do not install.
  Keep existing boot-selected mount bounds safe; do not advertise phantom
  slots or make cross-provider TSFS development a KCORE acceptance gate.
- [ ] Reduce the accrued permanent KCORE descriptor/dispatch cost after the
  VFS activation path is stable; no workarounds or unsupported 32K claims.
- [x] Repair stale daimos-testkit `mm-core` host fixture: its Makefile
  still references removed `system/kernel/proc/vm_pdp6.c`. VM lifecycle
  functions now live in native PDP-6 assembly; simply dropping that
  source leaves `vm_extent_move`, `vm_space_can_swap` and
  `vm_space_destroy` unresolved on the host. Implement a verified host
  reference matching current production VM semantics, or restructure
  the fixture to test only available host policy functions. Earlier
  TODO claims of missing `vfs_mount_limit`, `proc_storage_publish_v30`
  and `proc_swap_storage_invariant_v30` definitions were stale: the
  fixture already provides all three. Native PDP-6 swap/EXEC/RUN
  acceptance passes independently. Source audit v655; no outdated
  VM C implementation restored.
  v656: completed in testkit with C99 host reference for the assembly
  VM lifecycle calls and corrected dynamic MM descriptor/process-slot
  expectations. `make mm-core` now reports `mm-v1: PASS`. Production
  VM assembly remains authoritative; MEMFS XIP provider teardown is
  not covered by this host fixture.

- [ ] V19: native FILE mount-extension map now preserves mount IDs up to 26
  across file operations, DUP/DUP2, FLOCK, FIFO substitution, unmount, pipe
  dispatch and RUN fd-map inheritance, with unchanged two-word descriptors
  and u-area size. Restore the +138 permanent-word instruction cost in a
  dedicated source-level KCORE optimization sweep once remaining consumers
  are stable. Next: PURE backing fifth-bit lifetime and optional DTFS
  personality/bounds, then safe boot-selected mount count.

- [x] V18/V19: credentials UID9/GID9/UMASK9 are packed into word 066;
  formerly dedicated umask word 067 can hold all 16 two-bit FILE mount
  extensions at no process-memory increase. Native FILE open/read/write,
  dup/close/lock/FIFO/unmount and RUN-inheritance paths now use word 067;
  separately resolve PURE backing's 37-bit requirement.
  One of credential word 066's nine remaining spare bits may hold PURE's
  additional mount-slot bit if its entire process lifecycle is audited.
  See `design/file-mount-packing-20261016-v18.md`.

- [ ] V17: one-word 26-mount policy and 26-slot six-array slab layout tested
  natively; boot capacity stays four. Next resolve the two-bit FILE descriptor
  mount-ID overlay collision and the mathematically overfull PURE backing
  record (18 index + 5 mount + 14 PURE-text = 37 bits). Preserve all file
  metadata and PURE length semantics; never silently truncate IDs. Audit
  optional TENEX/ITS personality arrays and the remaining provider bounds
  before KINIT activates larger mount counts. Current 32K gate still fails.

- [ ] V16 compact mount policy now handles sixteen slots in the original
  one-word MonitorFS counter alias. Finish native provider bounds and the
  optional foreign DTFS personality dispatch, then connect the boot inventory
  to select a 1..16 limit. Full v16 policy adds 25 instruction words over v15:
  recover the instruction cost in the post-cutover RAM optimization sweep.

- [x] Historical V15 four-slot mount activation proposal superseded by
  the completed V21/V22 boot-selected capacity and native bounds checks.
  The remaining DTFS/TSFS media, shape and optional foreign-personality
  enhancements are post-KCORE. Retain existing provider isolation and
  mount safety while working on generic VFS or D6FS functionality.

- [ ] V14 KCORE audit eliminated nine redundant resident words. See
  `design/kcore-legacy-audit-20261016-v14.md` for protected compatibility
  paths and remaining table candidates. Optimize dynamic-pointer hot paths
  after the other provider tables have migrated; the 32K KCORE end still
  exceeds its strict gate by 1879 words.

- [x] Historical V13 four-slot proposal superseded by V22: D6FS now
  uses the MM slab and VFS boot-selected capacity is live. DTFS/TSFS
  media, shape/extent and format enhancements follow KCORE (v906).
  The older v13 32K KCORE size measurements are historical evidence,
  not current size-sweep authorization.

- [ ] Fix drum D6FS BLOCKSET discovery separately: root selection currently
  discovers a selected DSK/DRM root set, while AUXSTORE discovery checks only
  DRM0 (then DSK0). Audit other drum units and complete/non-root BLOCKSET
  detection without unsafe probes of unattached hardware. Preserve existing
  root-selection and AUXSTORE semantics until native tests establish the fix.
- [ ] For the current VFS capacity plan, count every disk/drum unit outside
  *known* BLOCKSET membership as potentially independently mountable. Do not
  assume an unexamined drum is reserved for swap or belongs to a BLOCKSET.
  Count a known BLOCKSET as one logical filesystem. Exclude independently
  impossible non-root TSFS sets; the boot TSFS root counts as one mount.
- [ ] Audit generic VFS mount tables, mount-ID dispatch, D6FS/MEMFS
  mount state and packed policy against the active boot-selected capacity.
  Preserve existing DTFS/TSFS boot/mount ABI compatibility, but defer new
  DTFS/TSFS provider metadata and format work until after KCORE.
  Enforce a maximum of 16 mounts and size dynamically at boot; a theoretical
  17th simultaneously mountable volume is not supported. Account for future/removable
  media rather than sizing solely to currently mounted filesystems. Audit and
  implementation sequence: `design/vfs-mount-capacity-20261015-v1.md`.
  V12 progress: active target/root arrays now reside in a KINIT-allocated MM
  slab and pass diagnostic native boot; provider-specific reader, media,
  shape/extent state and policy still use four-slot layouts. Retire the
  redundant resident fallback tables and measure the pointer hot paths after
  all consumers are converted; do not raise `vfs_mount_limit` prematurely.
- [ ] Defer block-cache policy changes pending a separate design discussion.


## LIBC ASSEMBLY OPTIMIZATION CANDIDATES — DEFERRED (2026-10-14)

- Investigate `qsort` first: profile the existing generic comparator-based
  implementation, generated PDP-6 code, swap cost, code size, and worst-case
  auxiliary RAM/stack use. Prefer bounded-memory sorting over a literal
  translation of the C algorithm; compare native SIMH timing and words.
- Then evaluate `strcpy`, `strncpy`, `strcat`, `strncat`, `strchr`, and
  `strrchr` for compact byte-pointer assembly. Measure actual savings before
  replacing C implementations.
- Leave `strcspn`, `strtok`, `atoi`, `abs`, `calloc`, `brk`, `sbrk`,
  `stat`, `time`, `exit`, and general heap management in C unless testing
  demonstrates worthwhile memory or performance gains. `calloc` should
  continue to benefit from assembly `memset`.
- Keep regression tests in `pdp10-testkit`. Do not interrupt the ongoing
  upstream semantic-test import for these optimizations.

## D6 CODEC PRODUCTION CUTOVER — STREAMING LOW-CORE DECODER (V057 PLAN)

The production-format v053 encoder/complete-buffer decoder are staged; the
v055 kernel ABI wrapper is a *test milestone*, not the production design.
The active fixed-address D6LZ36 decoder and its callers remain unchanged.
Before any production cutover, replace the decoder **implementation**, not
its external use. The old `system/stand/pdp6/common/decompressor.inc` at
executive address `000060` is the definitive calling-contract reference.

- Implement a directly callable new-format decoder at `000060` with the
  existing resumable register state/entry behavior: AC12 destination cursor,
  AC13 output words remaining, AC3 input cursor, AC4 input words remaining,
  AC14 output origin, and AC0 result (0 complete, +1 NEED_INPUT, -1 invalid).
  Preserve caller-visible state across NEED_INPUT. Keep existing Stage-1
  installation and runtime `d6lz36_decode_vfs` / `d6lz36_decode_buffer`
  interfaces intact; names may change only together with all callers.
- Decode the **new** grouped-control / paired-18-bit-descriptor stream,
  including word copies, zero runs and supported halfword matches, without
  materializing a complete compressed image. The current v053 whole-buffer
  decoder is NOT suitable as a runtime EXEC/VFS replacement.
- A group contains at most 30 tokens. With `m` descriptor-bearing tokens,
  its complete encoded size is `1 + (30 - m) + ceil(m/2) <= 31` 36-bit
  words, including control. Reuse the existing **64-word VFS refill
  window**: retain/compact a truncated group at a window boundary and
  refill the available space. Do not allocate a second group buffer,
  image-sized workspace, or permanent decoder BSS. Validate the 31-word
  bound and the final partial group against actual encoder output.
- Preserve the low-core decoder ABI **directly**. Do not link a generic
  wrapper or permanently retain the v055 AC-save adapter: it exists solely
  for native ABI tests. Implement the encoder's required ABI and callee-save
  behavior directly as well. Measure combined code words, stack high-water
  and persistent RAM; prioritize RAM, then execution speed. Decoder RAM
  should stay close to the old implementation, not grow with image size.
- Validate resumptions at every possible control/descriptor/literal boundary,
  exact compressed-stream consumption, truncated/malformed input, output
  bounds, grouped descriptor padding, odd/even halfword-distance behavior,
  overlap, and direct-buffer decoding. Test against host-produced golden
  streams on native PDP-6 in **daimos-testkit**; do not assume the existing
  v053 decoder's even-distance-only restriction is acceptable.
- Compare the old and replacement `000060` footprints, Stage-1 relocation
  and overlap, VFS stack frames, full linked KCORE text/data/BSS and peak
  process RAM. Avoid extra hot-path dispatch or an unnecessary buffer copy.
- Perform the **single-format atomic cutover** only after tests pass:
  update all five Stage-1 loaders, KINIT compression, host/userland DXR and
  TSFS producers, VFS/EXEC/PURE/TSFS consumers, and replace the dummy kpack
  codec with the real resident encoder/decoder. Rebuild every compressed
  artifact; do not ship mixed D6LZ36/new-format images or runtime fallback.
  Retain disk-swap fallback until packed eviction/restoration and scheduler
  integration pass their own end-to-end tests.

Detailed producer/consumer checklist:
`system/kernel/core/D6CODEC-INTEGRATION-TODO-v054.md`.
Acceptance is **not** claimed by this plan; no production codec or decoder
was activated as part of the TODO update.

## Experimental user overlays and shared text (planned, not implemented)

Implementation plan: `design/user-overlay-shared-text-experiments-20261012-v1.md`.
Start with measured/optional user-space overlays; subsequently benchmark
PDP-6 shared execution areas and optimized private-state copying. Keep
the architecture replaceable by the selected VM backend. No acceptance
or production enablement is claimed.

DATE: 2026-10-07

PROJECT-STATE.MD DEFINES THE CURRENT ARCHITECTURE.  THIS FILE CONTAINS OPEN
WORK ONLY; COMPLETED WORK BELONGS IN GIT HISTORY AND THE CURRENT DOCUMENTATION.
KCORE WORK IS TRACKED EXCLUSIVELY IN KCORE.MD.

## NATIVE KCC BUILD AND INSTALL GATE

- COMPLETED (HOST/STAGING): KCC'S ROOT MAKEFILE NOW HOLDS THE SHARED
  NATIVE MODULE INVENTORIES AND INCLUDES UNIX.MK OR DAIMOS.MK. THE OLD
  PYTHON PHASE MAKEFILE GENERATOR WAS REMOVED. THE DAIMOS STAGER CONVERTS
  THESE TWO SOURCE FILES DIRECTLY INTO SIXBIT-COMPATIBLE S6REC AND STAGES
  THEM ALONG WITH THE INSTALLED BOOTSTRAP. HOST AND CROSS DRY RUNS AND
  STAGING/PREPREREQUISITE GRAPH TESTS PASS.
- PARTIAL PASS: SIMH NATIVE MAKE -Q (DEFAULT/ALL), MAKE -N FOR THE FIRST
  OBJECT, AND MAKE CLEAN PASSED WITH THE SPLIT GRAPH. THE REAL FIRST-OBJECT
  COMPILATION TIMED OUT AFTER REACHING KCC; RETEST IT SEPARATELY.
- REMAINING: RUN A FOCUSED NATIVE MAKE INVALIDATION REGRESSION
  ON THE CONVERTED SHARED GRAPH; MEASURE ITS PEAK MEMORY AND RULE LOOKUP
  TIME. THEN PERFORM A USER-OPERATED NATIVE KCC MAKE ALL/INSTALL ONCE THE
  KPARSE COMPILER CORRECTNESS AND PERFORMANCE GATES ARE CLEARED. DO NOT
  START LONG NATIVE KCC SELF-BUILDS AUTOMATICALLY.
- NEXT FOR NATIVE MAKE: VERIFY GROWTH BEYOND 768 DEPENDENCY LINKS AND 384
  RECIPE LINKS; MEASURE THE NEW SORTED-INDEX RULE LOOKUP ON LARGE GRAPHS
  (PDP-6 OBJECT COMPILES; NATIVE RUNTIME TEST STILL REQUIRED); THEN PROVIDE THE
  REMAINING RECIPE/VARIABLE SYNTAX REQUIRED BY A COMMON KCC MAKEFILE.

- COMPLETED: PDP-6 NATIVE CONSTANT-FOLD OVERFLOW FLAG MASK CORRECTED
  (`JFCL 10`, NOT `JFCL 11`), NATIVE COMPILE REGRESSION PASSES.
  THE STALE INSTALLED HOST KCC MISCOMPILED A SIMPLE INDEXED POINTER
  LOOP; THE REBUILT REPOSITORY HOST COMPILER PASSES A NEW PDP-6
  RUNTIME TEST, AND REBUILDING ALL NATIVE PHASES WITH IT ELIMINATES
  THE OLD SMALL/MALFORMED KIR GRAPHS. BEFORE SELF-HOSTING, ENSURE
  THE INSTALLED HOST COMPILER IS REFRESHED FROM CURRENT SOURCE.
- CURRENT: COMPLETE NATIVE `MAKE ALL` WITH THE FRESH HOST-BOOTSTRAPPED
  KCPP/KPARSE/KGEN/KOPT; MEASURE KPARSE KIR GRAPH BUILD TIME AND RAM.
  REJECT ANY PER-OBJECT CACHE/INDEX UNTIL ITS SAVINGS JUSTIFY RAM.
  THE POINTER-BOUNDS EXPERIMENT IS NOT YET AN ACCEPTED OPTIMIZATION.
- NATIVE KCPP AND D6FS RESIZE CALL-FLOW/READER-FIELD DEFECTS ARE FIXED;
  KCPP PREPROCESSES CC.C COMPLETELY (220672 INPUT CHARACTERS, STATUS 0).
  FIRST REMAINING GATE IS KPARSE: IT REPORTS SPURIOUS-LOOKING CONSTANT
  EXPRESSION OVERFLOW ON ORDINARY IDENTWDS AND ARRAY-SIZE EXPRESSIONS,
  THEN DID NOT COMPLETE WITHIN A 260-SECOND NATIVE TEST. USE INSTRUMENTED
  KPT4 TOKEN PROGRESS AND ANALYZE CHKOVF/JFCL HARDWARE FLAGS OR A BAD
  KCC OPTIMIZATION. FIX THE COMPILER'S UNDERLYING SEMANTICS; DO NOT
  DISABLE OVERFLOW DETECTION AS A WORKAROUND. TESTS BELONG IN DAIMOS-TESTKIT.
- KPARSE SPEED AND RAM: USER PC 134041..134057 MAPS TO CCKIRWRITE.C'S
  LINEAR FINDPTR POINTER LOOKUP (CONFIRMED VIA KPARSE.MAP AND OBJECT
  DISASSEMBLY). A CAPTURED SCAN HAD 7679 ENTRIES, GIVING QUADRATIC
  GRAPH WRITER WORK. DESIGN A BOUNDED-MEMORY INDEX BEFORE OPTIMIZING;
  MEASURE EXTRA RAM AND EXECUTION TIME UNDER NATIVE 0.25 MIPS CONDITIONS.
  ALSO RECHECK THE SEPARATE STALLED-STORAGE-EVENT SNAPSHOT (EVENT 050636,
  STORAGE_STATE ZERO) BEFORE CHANGING I/O WAKEUP SEMANTICS.
- VALIDATE KPARSE, THEN KOPT AND KGEN THROUGH NATIVE MAKE ALL/INSTALL;
  FOLLOW WITH A NATIVE REBUILD AND SINGLE-SOURCE INVALIDATION TEST.

## 1. SUBSTANTIAL DSH

THE LANGUAGE/INVOCATION CONTRACT IS FROZEN IN PDP10-DOC USER/DSH_DESIGN.MD.
THE DETAILED OPEN-WORK LIST IS `USERLAND/DSH/TODO.MD`; KEEP THIS TOP-LEVEL
SECTION AS THE ROADMAP POINTER RATHER THAN DUPLICATING A FAST-MOVING CHECKLIST.

- FIRST PRIORITY IS DSH-SPECIFIC TEST COVERAGE AND DOCUMENTATION SYNCHRONIZATION.
- THEN CLOSE FROZEN LANGUAGE/CASE/GLOB CORRECTNESS, JOB-CONTROL TTY SEMANTICS,
  THE BOUNDED INTERACTIVE EDITOR, AND ADVISORY SIXMD COMPLETION.
- BASIC COMMAND EXECUTION MUST NEVER DEPEND ON INSTALLED HELP/COMPLETION
  METADATA.

## 2. CORE USERSPACE UTILITIES

PORT ALGORITHMS AND COMMAND BEHAVIOR FROM DAIMOSV1 WHERE USEFUL, NOT OBSOLETE
KERNEL ABIS.

- PRIORITIZE THE MINIMUM COMMAND SET REQUIRED BY THE DAIMOS BUILD AND NORMAL
  ADMINISTRATION.
- PORT/FINISH ECHO, BASENAME, DIRNAME, TRUE, FALSE, EXPR, CAT, CP, CMP, GREP,
  HEAD, WC, TEE, STRINGS, LS, FIND, STAT, MKDIR, RM, MV, PWD, CHMOD, XARGS,
  TAIL, DD, CHOWN, RMDIR, AND TOUCH AS NEEDED.
- REWRITE PS/FREE/DEVSTAT/MEMSTAT/ADMINISTRATION TOOLS FOR MONITORFS;
  DO NOT RECREATE DAIMOSV1 SYSCTL/DEVIO INTERFACES.

## 3. NATIVE BUILD AND SELF-HOSTING PATH

- NATIVE MAKE, DAS, DARC, DLINK, AND OBJDUMP ARE PRESENT.  KEEP THEIR
  LINK-AND-EXECUTE TARGET REGRESSION PASSING.
- DEFER: COMPLETE A DAIMOS-NATIVE MAKEDEPEND.  AN X.ORG 1.0.10 HOST
  REFERENCE IS ALREADY IMPORTED IN `USERLAND/MAKEDEPEND`; SEE ITS
  `PORT-STATUS.MD` FOR NATIVE LIBC, SIXBIT FILE/PATH, AND MEMORY BLOCKERS.
  FIRST CHECK WHETHER NATIVE KCC PREPROCESSING CAN ASSIST DEPENDENCY
  GENERATION.  IF THE UPSTREAM PORT IS TOO LARGE, CONSIDER A SMALL C89
  CONSERVATIVE INCLUDE SCANNER WITH RECURSIVE `#INCLUDE`, `-I`, CYCLE
  DETECTION, AND MAKEFILE OUTPUT; CONDITIONAL BRANCHES MAY OVERAPPROXIMATE
  DEPENDENCIES BUT MUST NOT MISS THEM.  VERIFY THE RESULT ON TERMUX AND
  DAIMOS.  DO NOT IMPLEMENT DURING THE CURRENT NATIVE BUILD WORK.
- REBUILD A CONTROLLED USERLAND COMPONENT WITH THE NATIVE BUILD TOOLS, THEN THE
  FULL USERLAND.
- MEASURE REAL DSH/MAKE/DAS/DLINK/EDITOR/KCC WORKING SETS BEFORE FREEZING
  USERSPACE OVERLAY BOUNDARIES.
- IMPLEMENT USERSPACE OVERLAYS ONLY AFTER THE NON-OVERLAY PROGRAMS WORK AND THE
  MEASUREMENTS JUSTIFY THEM.
- AFTER OVERLAYS, REVISIT LOADABLE/UNLOADABLE/RELOCATABLE RUNTIME MODULES.
  DO NOT RESTORE RESIDENT MOVEMENT/RETARGETING MACHINERY BEFORE THEN.
- PORT/BUILD KCC/CPP, LIBC, CRT0, AND LIBSYS TO RUN UNDER DAIMOS; THEN REBUILD
  THE KERNEL WITH THE NATIVE TOOLCHAIN.
- AFTER THE CORE BUILD PATH WORKS, PORT ELVIS VI/EX WITH SPELLING AND FOLDING;
  KEEP COLOR, SYNTAX HIGHLIGHTING, AND URL FEATURES OUT.

## 4. STORAGE / ADMINISTRATION

- ADD A TRUE ONLINE D6FS PACKER WHICH OPERATES ON A LIVE, MOUNTED FILESYSTEM.
  IT MUST COORDINATE WITH VFS/D6FS ALLOCATION AND OPEN-FILE STATE, MOVE EXTENTS
  WITHOUT INVALIDATING ACTIVE FILE REFERENCES, PRESERVE SWAP/LOGSTORE AND
  BADMAP RESERVED RANGES, UPDATE METADATA CRASH-SAFELY, AND ALLOW NORMAL I/O
  TO CONTINUE EXCEPT FOR THE MINIMUM NECESSARY PER-FILE/PER-EXTENT EXCLUSION.
  THIS IS DISTINCT FROM `TOOLS/HOST/PACKFS`, WHICH REMAINS THE OFFLINE
  MAINTENANCE COMPACTOR.

## 5. HOST TOOLING

### 32K LOWMEM BOOT FOLLOW-UP (NEXT CHAT)

- TARGET PERMANENT KCORE BELOW 8192 DECIMAL WORDS (CURRENT MEASURED
  LOWMEM KCORE: 10020).  MEASURE EVERY CHANGE AGAINST THE SINGLE-DISK
  CTY-LOGIN CONFIGURATION; KEEP THE BOOT MINIMUM AT 32K.
- THE 48K SINGLE-DISK BOOT REACHES DSH AND MEMSTAT.  MEASURED LIVE
  RESIDENT=12725, PROCESS-WORDS=28440, FREE BY DIFFERENCE=7987;
  THE IDENTICAL WORKLOAD EXCEEDS 32K BY 8397 WORDS.  REVISIT USER
  PROCESSES/VM AFTER KCORE REDUCTION; THE GAP IS NOT ALL KCORE.
- PRESERVE THE 8K-WORD CORE-BOUNDARY PROBE IN KINIT.  ADD COVERAGE FOR
  32K, 40K, 48K, AND OTHER SUPPORTED BOUNDARIES IN DAIMOS-TESTKIT.
- BUILD A TRUE CTY-ONLY KERNEL IMAGE, NOT JUST A SIMH CONFIGURATION WITH
  DCS/GE UNATTACHED, AND RECHECK RESIDENT SIZE AND 32K DSH EXEC.
- DO NOT RELAX THE EXISTING DSH IMAGE-SIZE GATE OR COUNT A 48K SUCCESS
  AS PROOF OF 32K BOOT SUCCESS.

- ADD DIRECT PDP10-TESTKIT REGRESSION COVERAGE FOR `TOOLS/HOST/MKDT`,
  `TOOLS/HOST/MKSTREAM`, AND `TOOLS/HOST/WORDS2PT`, USING FORMAT VECTORS OR
  ROUND-TRIP CHECKS WHERE A READER EXISTS.

## 6. POST-0.9 / EXPLICITLY DEFERRED

- PORT MIT MAC HACK VI (`ITS SRC/RG/C.562`) TO DAIMOS AFTER REDESIGNING THE
  TYPE-340 APPLICATION INTERFACE.  DO NOT REUSE THE REJECTED EXPERIMENTAL
  PER-WORD/PERSISTENT KERNEL DISPLAY-LIST FEEDER.  START FROM THE ACTUAL
  TYPE-340/344 HARDWARE MODEL AND FIND A FUNDAMENTALLY SMALLER DESIGN THAT
  PRESERVES DAIMOS'S RESIDENT-MEMORY GOALS, THEN PORT THE CHESS ENGINE AND ITS
  ORIGINAL TYPE-340 PIECE ARTWORK ON TOP OF THAT NATIVE INTERFACE.  KEEP ITS
  ITS-SPECIFIC BOOK/HASH/UTAPE AND MONITOR DEPENDENCIES SEPARATE FROM THE
  CHESS/DISPLAY CORE AND ADD THEM ONLY WHEN THEY ARE JUSTIFIED.

- COMPLETE THE AAP-EMULATED TYPE-340/344 INTERACTIVE HARDWARE PATH.  AAP
  IMPLEMENTS THE LIGHT PEN, LATCHED LIGHT-PEN X/Y DATAI COORDINATES, A SEPARATE
  SPECIAL-CONDITION PIA, HORIZONTAL/VERTICAL EDGE FLAGS, PROGRAMMED STOP, AND
  RESUME.  DAIMOS CURRENTLY USES THE SPECIAL PIA ONLY WHILE PROBING AND KEEPS
  IT DISABLED DURING NORMAL OPERATION.  ADD THE SMALLEST USEFUL RUNTIME
  INTERFACE FOR LIGHT-PEN/SPECIAL-CONDITION INPUT WITHOUT GROWING THE ORDINARY
  DISPLAY-REFRESH HOT PATH; KEEP DATA/DONE PI7 HANDLING SEPARATE FROM THE
  SPECIAL-CONDITION PIA AND EXPOSE RAW HARDWARE SEMANTICS TO USERSPACE RATHER
  THAN INVENTING A LARGE GRAPHICS INPUT SUBSYSTEM.  COVER LIGHT-PEN HIT, DATAI
  COORDINATES, EDGE/STOP CONDITIONS, ACKNOWLEDGEMENT, AND RESUME ON AAP-PDP6.

- ADD DEC PDP-6 TIMESHARING SYSTEM 1.4 DECTAPE FORMAT COMPATIBILITY TO DTFS AS
  ANOTHER FOREIGN MEDIA PERSONALITY.  START READ-ONLY: DETECT/VALIDATE THE
  MONITOR 1.4 FORMAT, READ ITS BLOCK-1 DIRECTORY AND FOUR-WORD DIRECTORY
  ENTRIES, AND MAP LOOKUP/READDIR/STAT/FILE READS THROUGH THE EXISTING
  DTC/BCACHE/VFS PATH.  REUSE THE EXISTING PACKED PERSONALITY STATE; `030` IS
  CURRENTLY AVAILABLE AFTER NATIVE `000`, TENEX `010`, AND ITS `020`.
  CONSIDER WRITABLE COMPATIBILITY ONLY AFTER READ-ONLY SUPPORT HAS GOLDEN-IMAGE
  TESTS FOR DIRECTORY, BLOCK-CHAIN, AND ALLOCATION SEMANTICS.  DO NOT IMPLEMENT
  THIS BEFORE 0.9.
- RECOVERY/SINGLE-USER MODE: BOOTABLE RECOVERY DECTAPE MEDIA USING THE NORMAL
  MAINTENANCE TOOLS; DO NOT ADD AN INITFS ROOT.
- SLAVE REVERSE INTERRUPTS AND PCLK INTERRUPT DELIVERY.
- A HARDWARE-PAGER VM BACKEND FOR LATER PDP-10 MACHINES.

## Deferred: executable opt-in for compressed process swap (2026-10-15)

### v763: variable-sized in-place swap exchange (critical correctness)

- [ ] Establish the bounded, lossless sector-transfer order for unequal
  resident/swap sizes and unequal PURE boundaries; see
  `design/swap-exchange-v763.md`. RAM capacity, disk extent capacity,
  contiguous allocation, and workspace lifetime are distinct invariants.
  No persistent journal/crash recovery is required for volatile swap.
- [ ] Implement and test exchange as a new transaction only when source
  preservation, MM ownership, BACKSTORE bitmap accounting, and process
  descriptor publication are proven. Keep the current disk format unless
  variable-sized exchange demonstrably requires a replacement. Accept only
  after native writer WAIT/EXIT and failure-injection regression pass.

### v761: optional link-time compression of PURE executable backing


- [ ] Evaluate opting PURE DXR2 executables into the already-supported
  compressed executable-payload format at link time. Measure raw versus
  encoded storage blocks and estimate PDP-6 decode time for disk and DECtape;
  select compressed backing only when it pays off. Reuse existing executable
  size/PURE-boundary metadata and swap-in reload; do not confuse executable
  payload compression with runtime process-state compression.
- [ ] Validate original-image reconstruction and repeated PURE eviction /
  reload for compressed and raw DXR2, without storing another compressed
  text copy in RAM by default. Keep this strictly deferred and independent
  from the currently failing native raw-swap pressure regression.


### v760 design separation: capacity recovery first; compression later

- [ ] Treat RESIDENT, PACKED-in-RAM, and DISK as *alternative storage
  representations*, not a mandatory RESIDENT -> PACKED -> DISK sequence.
  A disk-backed image may be raw or compressed. The reclaim policy must
  select a representation using measured RAM savings, BACKSTORE blocks,
  compression CPU cost, restore latency, and scratch/rollback headroom.
  On a PDP-6, do not assume compressing before each disk swap is faster.
- [ ] Preserve the original *logical process image size* in the stable
  process descriptor across every storage transition, independently of
  the encoded payload length. For PACKED, also retain the omitted PURE
  boundary and decoded writable extent; for compressed DISK, retain the
  encoded block count and exact decoded word count, including any padding.
  First audit the existing three-word process descriptor, one-word
  overloaded disk record, stable u-area, and two-bit storage mode map;
  do not allocate another per-slot word or steal an encoding bit without
  proving non-overlap. Reject mismatching sizes on restore.
- [ ] Before activation, benchmark PURE prefix omission alone against
  raw disk swap, compressed disk swap, and PACKED-in-RAM using actual
  process data and PDP-6 timing; record worst-case incompressible images
  and temporary peak RAM, not only compression ratios. Reuse lossless
  codec primitives where safe, not disk/RAM transaction semantics.
- [ ] Keep the native v758/v759 writer deadlock as a separate critical
  correctness problem. Fix resource-pressure admission/forward progress
  without making either compressed swap or PACKED a prerequisite. First
  establish which eligible victims require 208 blocks after PURE omission;
  then specify an admission/headroom invariant covering the case with no
  runnable resident producer of free RAM or swap space. Preserve valid
  swapped processes on retryable pressure and require passing native
  writer WAIT/EXIT acceptance. A cooldown is not a capacity guarantee.

- [ ] Reserve an unused, backwards-compatible DXR executable-header flag
  (`COMPRESSIBLE` or equivalent) after auditing existing DXR flag allocations.
  This flag describes the expected compressibility of *runtime process memory*;
  it is distinct from the existing DXR `COMPRESSED` executable-payload flag.
- [ ] On `exec`, initialize the process compressibility bit from the executable;
  inherit it across `fork`, and replace it on a subsequent `exec`. Keep the
  default uncompressed for unmarked executables; no adaptive statistics,
  heuristics, size-only prepass, or runtime policy controls initially.
- [ ] Swap-out: bypass the codec entirely for unmarked processes. For marked
  processes, attempt the native compressor; if it cannot encode safely or
  produces no useful reduction, retain the existing raw swap transaction.
  Preserve atomic allocation/write/rollback semantics on any failure.
- [ ] Keep a separate *swap-record* `COMPRESSED` bit to describe the actual
  stored representation, independent of the process opt-in bit. Swap-in must
  decode according to the record and validate the exact restored word count.
- [ ] Before activation, benchmark representative non-text 36-bit process
  images with AAP logical ticks for encode/decode, compressed block count,
  temporary stack/RAM, and I/O tradeoffs. Test roundtrip, incompressible
  fallback, allocation failure, rollback, `fork`/`exec` inheritance and
  mixed compressed/raw swap records in daimos-testkit. Do not modify the
  running parallel kernel implementation as part of this TODO entry.

- [x] v618 mutually exclusive PCLK/SCLK MRES: KINIT boot selection,
  shared APR scheduler, target GETTIME/SETTIME acceptance, and measured
  linked low-memory occupancy. Both calendar providers are optional.

- [x] v618 native exclusive-provider gate: testkit
  `daimos-clock-mres-v618` PASS at 50 and 60 Hz, with PCLK enabled
  and disabled in separate clean boots. Lowmem GCC links PASS at both rates.
